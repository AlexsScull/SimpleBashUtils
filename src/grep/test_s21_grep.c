#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Конфигурация тестирования
#define MAX_FLAGS 8
#define BUFFER_SIZE 8192
#define TEST_FILES " 1.txt 2.txt 3.txt 4.txt 5.txt"
#define TEST "\"test\""
#define TEST_E "-e \"TEST\" -e \"line\""
#define TEST_F "-f patterns.txt"

// Тестируемые флаги grep
const char *flags[] = {"-i", "-v", "-c", "-l", "-n", "-h", "-s", "-o"};
const char *patterns[] = {"test", "TEST", "line", "pattern", "[a-z]"};

/**
 * Создает тестовые файлы и файл с шаблонами для флага -f
 * - 1.txt: базовый тестовый файл
 * - 2.txt: файл с одним совпадением
 * - 3.txt: файл без совпадений
 * - 4.txt: файл со спецсимволами
 * - 5.txt: пустой файл
 * - patterns.txt: файл с шаблонами для -f
 */
void create_test_files() {
  // Создаем основные тестовые файлы
  FILE *f = fopen("1.txt", "w");
  fprintf(f, "test line 1\nline 2\nTEST line 3\ntest line 4\n");
  fclose(f);

  f = fopen("2.txt", "w");
  fprintf(f, "no match\nTEST match\nno match\n");
  fclose(f);

  f = fopen("3.txt", "w");
  fprintf(f, "no matches here\njust text\nnothing to find\n");
  fclose(f);

  f = fopen("4.txt", "w");
  fprintf(f, "line with \x01\nmulti\npattern\nmatch\n");
  fclose(f);

  f = fopen("5.txt", "w");
  fclose(f);

  // Создаем файл с шаблонами для флага -f
  f = fopen("patterns.txt", "w");
  for (size_t i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
    fprintf(f, "%s\n", patterns[i]);
  }
  fclose(f);
}

/**
 * Выполняет команду и возвращает ее вывод
 * @param cmd Команда для выполнения
 * @return Указатель на статический буфер с выводом
 */
char *run_cmd(const char *cmd) {
  static char output[BUFFER_SIZE];
  FILE *fp = popen(cmd, "r");

  output[0] = '\0';
  if (!fp) return "CMD ERROR";

  char buf[256];
  while (fgets(buf, sizeof(buf), fp)) strcat(output, buf);

  pclose(fp);
  return output;
}

/**
 * Формирует команду grep с указанными параметрами
 * @param dest Буфер для команды
 * @param prefix "grep" или "./s21_grep"
 * @param flags_str Строка с флагами
 * @param pattern Шаблон для поиска (NULL для -f)
 * @param use_f Флаг использования -f вместо прямого шаблона
 * @param use_e Флаг использования -e для шаблона
 */
void build_grep_cmd(char *dest, const char *prefix, const char *flags_str,
                    int i) {
  char pattern_part[256] = "";

  if (i == 0) {
    snprintf(pattern_part, sizeof(pattern_part), "%s", TEST);
  } else if (i == 1) {
    snprintf(pattern_part, sizeof(pattern_part), "%s", TEST_F);
  } else {
    snprintf(pattern_part, sizeof(pattern_part), "%s", TEST_E);
  }

  snprintf(dest, BUFFER_SIZE, "%s %s %s %s", prefix, flags_str, pattern_part,
           TEST_FILES);
}

/**
 * Тестирует все комбинации флагов для трех режимов:
 * 1. Обычный поиск (просто шаблон)
 * 2. Поиск с -f (шаблоны из файла)
 * 3. Поиск с -e (несколько шаблонов)
 */
void test_all_combinations(bool verbose) {
  int passed = 0, total = 0;

  // Тестируем все три режима
  for (size_t m = 0; m < 3; m++) {
    // Перебираем все комбинации флагов
    for (int mask = 1; mask < (1 << MAX_FLAGS); mask++) {
      // Формируем строку флагов
      char flags_str[100] = "";
      for (int i = 0; i < MAX_FLAGS; i++) {
        if (mask & (1 << i)) {
          strcat(flags_str, flags[i]);
          strcat(flags_str, " ");
        }
      }

      char sys_cmd[BUFFER_SIZE], custom_cmd[BUFFER_SIZE];

      // Формируем команды
      build_grep_cmd(sys_cmd, "grep", flags_str, m);
      build_grep_cmd(custom_cmd, "./s21_grep", flags_str, m);

      if (verbose) printf("Testing: %s\n", custom_cmd);

      // Выполняем и сравниваем
      char *sys_out = run_cmd(sys_cmd);
      char *custom_out = run_cmd(custom_cmd);

      total++;
      if (strcmp(sys_out, custom_out) == 0) {
        passed++;
      } else {
        printf("FAIL: %s\n", custom_cmd);
        printf("Expected:\n%s\nGot:\n%s\n\n", sys_out, custom_out);
      }
    }
  }

  printf("\nResults: %d/%d passed (%.1f%%)\n", passed, total,
         (float)passed / total * 100);
}

int main(int argc, char **argv) {
  create_test_files();
  test_all_combinations(argc > 1 && strcmp(argv[1], "+") == 0);
  return 0;
}