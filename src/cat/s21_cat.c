#include "s21_cat.h"

/**
 * Точка входа в программу
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return Код завершения программы
 */
int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Использование: %s [-beEnstTv] [файл ...]\n", argv[0]);
    return EXIT_FAILURE;
  }

  initialize_flags();
  parse_command_line_options(argc, argv);
  process_input_files(argc, argv);

  return EXIT_SUCCESS;
}

/**
 * Инициализация флагов программы значениями по умолчанию
 */
void initialize_flags(void) {
  flags.number_nonempty = false;
  flags.show_ends = false;
  flags.show_nonprinting = false;
  flags.squeeze_blank = false;
  flags.show_tabs = false;
  flags.number_all = false;
}

/**
 * Парсинг аргументов командной строки
 * @param argc Количество аргументов
 * @param argv Массив аргументов
 */
void parse_command_line_options(int argc, char **argv) {
  int option;
  const char *valid_options = "+beEnstTv";

  opterr = 0;  // Отключаем стандартные сообщения об ошибках

  while ((option = getopt_long(argc, argv, valid_options, NULL, NULL)) != -1) {
    switch (option) {
      case 'b':
        flags.number_nonempty = true;
        flags.number_all = false;  // -b имеет приоритет над -n
        break;
      case 'e':
        flags.show_ends = true;
        flags.show_nonprinting = true;
        break;
      case 'E':
        flags.show_ends = true;
        break;
      case 'n':
        if (!flags.number_nonempty) {
          flags.number_all = true;
        }
        break;
      case 's':
        flags.squeeze_blank = true;
        break;
      case 't':
        flags.show_tabs = true;
        flags.show_nonprinting = true;
        break;
      case 'T':
        flags.show_tabs = true;
        break;
      case 'v':
        flags.show_nonprinting = true;
        break;
      default:
        print_usage_error(argv[optind - 1]);
        exit(EXIT_FAILURE);
    }
  }
}

/**
 * Вывод сообщения об ошибке использования
 * @param invalid_option Неверный параметр командной строки
 */
void print_usage_error(const char *invalid_option) {
  fprintf(stderr, "Ошибка: Недопустимая опция -- %s\n", invalid_option + 1);
  fprintf(stderr, "Использование: s21_cat [-beEnstTv] [файл...]\n");
}

/**
 * Обработка входных файлов
 * @param argc Количество аргументов
 * @param argv Массив аргументов
 */
void process_input_files(int argc, char **argv) {
  size_t line_counter = 1;
  int consecutive_empty_lines = -1;
  for (int i = optind; i < argc; i++) {
    FILE *input_file = fopen(argv[i], "r");
    if (input_file == NULL) {
      print_file_error(argv[i]);
      continue;
    }

    process_file_contents(input_file, &line_counter, &consecutive_empty_lines);
    fclose(input_file);
  }
}

void print_file_error(const char *filename) {
  fprintf(stderr, "s21_cat: Ошибка: Не удалось открыть файл %s\n", filename);
}

/**
 * Обработка содержимого одного файла
 * @param file Указатель на файл для обработки
 */
void process_file_contents(FILE *file, size_t *line_counter,
                           int *consecutive_empty_lines) {
  int current_char;

  bool is_new_line = true;

  while ((current_char = fgetc(file)) != EOF) {
    // Обработка флага -s (сжатие пустых строк)
    if (flags.squeeze_blank && should_skip_repeated_empty_lines(
                                   current_char, consecutive_empty_lines)) {
      continue;
    }

    // Обработка флагов нумерации строк (-n и -b)
    if (is_new_line) {
      if (flags.number_nonempty && current_char != '\n') {
        print_line_number(line_counter, &is_new_line);
      } else if (flags.number_all) {
        print_line_number(line_counter, &is_new_line);
      }
    }

    // Обработка флага -t (отображение табов)
    if (flags.show_tabs && current_char == '\t') {
      print_tab_character();
      continue;
    }

    // Обработка флага -v (отображение непечатаемых символов)
    if (flags.show_nonprinting && current_char != '\n' &&
        current_char != '\t') {
      handle_nonprinting_characters(&current_char);
    }

    // Обработка флага -e (отображение конца строк)
    if (flags.show_ends && current_char == '\n') {
      print_end_of_line();
    }

    // Вывод текущего символа
    putchar(current_char);

    // Обновление состояния новой строки
    is_new_line = (current_char == '\n');
  }
}

/**
 * Обработка непечатаемых символов (для флага -v)
 * @param character Указатель на обрабатываемый символ
 */
void handle_nonprinting_characters(int *character) {
  if (*character >= 0 && *character <= 31) {
    putchar('^');
    *character += 64;
  } else if (*character == 127) {
    putchar('^');
    *character = '?';
  }
}

/**
 * Печать номера строки (для флагов -n и -b)
 * @param line_counter Счетчик строк
 * @param is_new_line Флаг новой строки
 */
void print_line_number(size_t *line_counter, bool *is_new_line) {
  printf("%6zu\t", (*line_counter)++);
  *is_new_line = false;
}

/**
 * Печать табуляции в виде ^I (для флага -t)
 */
void print_tab_character(void) { printf("^I"); }

/**
 * Печать символа конца строки $ (для флага -e)
 */
void print_end_of_line(void) { putchar('$'); }

/**
 * Проверка необходимости пропуска пустых строк (для флага -s)
 * @param character Текущий символ
 * @param empty_line_count Счетчик пустых строк
 * @return true если строку нужно пропустить, иначе false
 */
bool should_skip_repeated_empty_lines(int character, int *empty_line_count) {
  bool fl = false;
  // printf("%d",*empty_line_count);
  if (character == '\n') {
    if (*empty_line_count == -1) (*empty_line_count) = 1;
    (*empty_line_count)++;

    if (*empty_line_count > 2) {
      fl = true;
    }
  } else {
    *empty_line_count = 0;
  }
  // printf("%d",*empty_line_count);
  return fl;
}
