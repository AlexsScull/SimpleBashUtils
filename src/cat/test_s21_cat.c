#include <stdbool.h>  // Для использования булевых значений (true/false)
#include <stdio.h>    // Стандартный ввод/вывод
#include <stdlib.h>  // Стандартная библиотека (EXIT_SUCCESS и др.)
#include <string.h>  // Функции работы со строками
#include <unistd.h>  // Для системных вызовов (popen и др.)

// Максимальное количество флагов для тестирования
#define MAX_FLAGS 8
// Размер буфера для хранения вывода команд
#define BUFFER_SIZE 8192
#define TEST_FILES "1.txt 2.txt 3.txt 4.txt 5.txt"
// Массив тестируемых флагов программы cat
const char *flags[] = {"-b", "-e", "-n", "-s", "-t", "-v", "-E", "-T"};
// Строка с именами тестовых файлов через пробел

/**
 * Создает тестовые файлы с различными типами содержимого:
 * - Обычный текст с пустыми строками
 * - Файл с множественными пустыми строками
 * - Файл с непечатаемыми символами
 * - Пустой файл
 * - Файл с очень длинными строками
 */
void create_test_files() {
  FILE *f;

  // 1.txt - базовый тестовый файл с разными типами строк
  f = fopen("1.txt", "w");
  fprintf(f, "Line 1\n\nLine 3\n\tLine 4\nLine 5\n");
  fclose(f);

  // 2.txt - проверка обработки множественных пустых строк
  f = fopen("2.txt", "w");
  fprintf(f, "\n\n\nSingle line\n\n");
  fclose(f);

  // 3.txt - содержит непечатаемые символы (0x01) и символ DEL (0x7F)
  f = fopen("3.txt", "w");
  fprintf(f, "Normal\ttext\nText with \x01\nEnd\x7F\n");
  fclose(f);

  // 4.txt - полностью пустой файл для проверки граничных случаев
  f = fopen("4.txt", "w");
  fclose(f);

  // 5.txt - файл с одной очень длинной строкой (проверка обработки длинных
  // строк)
  f = fopen("5.txt", "w");
  for (int i = 0; i < 100; i++) fprintf(f, "Long line %d ", i);
  fprintf(f, "\n");
  fclose(f);
}

/**
 * Выполняет команду в shell и возвращает ее вывод
 * @param cmd Команда для выполнения
 * @return Указатель на статический буфер с выводом команды
 *
 * Особенности:
 * - Использует статический буфер (не потокобезопасно)
 * - Возвращает "CMD ERROR" при ошибке выполнения
 * - Читает вывод команды порциями по 256 байт
 */
char *run_cmd(const char *cmd) {
  // Статический буфер для хранения вывода (общий для всех вызовов)
  static char output[BUFFER_SIZE];
  FILE *fp = popen(cmd, "r");  // Открываем pipe для чтения вывода команды

  output[0] = '\0';  // Обнуляем буфер вывода

  if (!fp) return "CMD ERROR";  // Если не удалось выполнить команду

  char buf[256];  // Буфер для чтения вывода порциями
  // Читаем вывод команды построчно до конца
  while (fgets(buf, sizeof(buf), fp))
    strcat(output, buf);  // Добавляем прочитанное в общий вывод

  pclose(fp);  // Закрываем pipe
  return output;  // Возвращаем указатель на результат
}

/**
 * Тестирует все возможные комбинации флагов с тестовыми файлами
 * @param verbose Режим подробного вывода (true - выводить все тесты, false -
 * только ошибки)
 *
 * Алгоритм работы:
 * 1. Перебираем все возможные комбинации флагов (2^8 - 1 вариантов)
 * 2. Для каждой комбинации:
 *    - Формируем команды для system cat и s21_cat
 *    - Выполняем обе команды
 *    - Сравниваем вывод
 * 3. Подсчитываем и выводим статистику
 */
void test_all_combinations(bool verbose) {
  int passed = 0, total = 0;  // Счетчики успешных и всех тестов

  // Перебираем все комбинации флагов (от 1 до 2^MAX_FLAGS - 1)
  for (int mask = 1; mask < (1 << MAX_FLAGS); mask++) {
    char flags_str[100] = "";  // Буфер для хранения комбинации флагов

    // Формируем строку флагов для текущей комбинации
    for (int i = 0; i < MAX_FLAGS; i++) {
      if (mask & (1 << i)) {  // Если флаг включен в текущую комбинацию
        strcat(flags_str, flags[i]);  // Добавляем флаг
        strcat(flags_str, " ");       // И пробел после него
      }
    }

    // Формируем команды для system cat и s21_cat
    char sys_cmd[BUFFER_SIZE], custom_cmd[BUFFER_SIZE];
    snprintf(sys_cmd, sizeof(sys_cmd), "cat %s %s", flags_str, TEST_FILES);
    snprintf(custom_cmd, sizeof(custom_cmd), "./s21_cat %s %s", flags_str,
             TEST_FILES);

    // В подробном режиме выводим текущую тестируемую команду
    if (verbose) printf("Testing: %s\n", custom_cmd);

    // Выполняем обе команды и получаем их вывод
    char *sys_out = run_cmd(sys_cmd);
    char *custom_out = run_cmd(custom_cmd);

    total++;  // Увеличиваем счетчик всех тестов

    // Сравниваем вывод system cat и s21_cat
    if (strcmp(sys_out, custom_out) == 0) {
      passed++;  // Если вывод совпадает, увеличиваем счетчик успешных
    } else {
      printf("FAIL: %s\n", custom_cmd);
      printf("Expected (system cat):\n%s\nGot (s21_cat):\n%s\n\n", sys_out,
             custom_out);
    }
  }

  // Выводим итоговую статистику
  printf("\nResults: %d/%d passed (%.1f%%)\n", passed, total,
         (float)passed / total * 100);
}

/**
 * Точка входа в программу
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 *
 * Использование:
 *   ./test      - обычный режим (вывод только ошибок)
 *   ./test +    - подробный режим (вывод всех тестов)
 */
int main(int argc, char **argv) {
  // Создаем тестовые файлы (если их нет)
  create_test_files();

  // Запускаем тестирование в выбранном режиме
  // Если есть аргумент "+" - подробный режим, иначе - обычный
  test_all_combinations(argc > 1 && strcmp(argv[1], "+") == 0);

  return 0;  // Успешное завершение
}