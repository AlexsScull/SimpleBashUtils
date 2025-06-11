#include "s21_grep.h"

/**
 * Точка входа в программу
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return Код завершения программы
 */
int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr,
            "Использование: %s [-ivclnhso] ( [-e шаблон] [-f файл] || "
            "[шаблон]) [файл ...]\n",
            argv[0]);
    return EXIT_FAILURE;
  }

  initialize_options();
  char search_pattern[BUFFER_SIZE] = {0};
  parse_arguments(argc, argv, search_pattern);
  process_files(argc, argv, search_pattern);

  return EXIT_SUCCESS;
}

/* Инициализация опций программы значениями по умолчанию*/
void initialize_options(void) {
  options.use_extended_pattern = false;
  options.case_insensitive = false;
  options.invert_match = false;
  options.count_only = false;
  options.files_name_only = false;
  options.line_numbers = false;
  options.no_filename = false;
  options.no_errors_file = false;
  options.patterns_from_file = false;
  options.only_matching = false;
  options.files_count = 0;
}

/**
 * Парсинг аргументов командной строки
 * @param argc Количество аргументов
 * @param argv Массив аргументов
 * @param search_pattern Буфер для сохранения шаблона поиска
 */
void parse_arguments(int argc, char** argv, char* search_pattern) {
  int option;
  int pattern_count = 0;

  opterr = 0;

  while ((option = getopt_long(argc, argv, "e:f:ivclnhso", NULL, NULL)) != -1) {
    switch (option) {
      case 'e':
        options.use_extended_pattern = true;
        handle_extended_pattern(&pattern_count, search_pattern);
        break;
      case 'f':
        options.patterns_from_file = true;
        handle_pattern_from_file(&pattern_count, search_pattern);
        break;
      case 'i':
        options.case_insensitive = true;
        break;
      case 'v':
        options.invert_match = true;
        break;
      case 'c':
        options.count_only = true;
        break;
      case 'l':
        options.files_name_only = true;
        break;
      case 'n':
        options.line_numbers = true;
        break;
      case 'h':
        options.no_filename = true;
        break;
      case 's':
        options.no_errors_file = true;
        break;
      case 'o':
        options.only_matching = true;
        break;
      case '?':
        fprintf(stderr, "Ошибка: Некорректный флаг -%c\n", (char)optopt);
        exit(EXIT_FAILURE);
      default:
        break;
    }
  }

  if (options.files_name_only == true)
    options.no_filename = false;  // Флаг -l подразумевает отсутствие -h

  if (!options.use_extended_pattern && !options.patterns_from_file) {
    handle_default_pattern(argv, search_pattern);
  }
}

/**
 * Обработка файлов для поиска
 * @param argc Количество аргументов
 * @param argv Массив аргументов
 * @param search_pattern Буфер для сохранения шаблона поиска
 */
void process_files(int argc, char** argv, const char* search_pattern) {
  options.files_count =
      argc -
      optind;  // вычисление количества файлов, переданных в командной строке

  for (; optind < argc; optind++) {
    FILE* file = fopen(argv[optind], "r");
    if (file == NULL) {
      if (!options.no_errors_file) {
        fprintf(stderr, "Ошибка: Не удалось открыть файл %s\n", argv[optind]);
      }
      continue;
    }

    search_in_file(argv, search_pattern, file);
    fclose(file);
  }
}

/**
 * Поиск шаблона в файле
 * @param argv Аргументы командной строки
 * @param pattern Шаблон для поиска
 * @param file Файл для обработки
 */
void search_in_file(char** argv, const char* pattern, FILE* file) {
  regex_t regex;  // структура для регулярного выражения
  char buffer[BUFFER_SIZE];
  int line_number = 1;  // номер строки (отчет с первой)
  int match_count = 0;  // количество совпадающих строк
  regmatch_t match[1];

  if (regcomp(&regex, pattern, create_regex_flags(options.case_insensitive)) !=
      0) {
    fprintf(stderr, "Ошибка: Некорректное регулярное выражение\n");
    return;
  }

  while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
    int result = regexec(&regex, buffer, 1, match, 0);

    if (options.invert_match) result = !result;

    if (result != REG_NOMATCH) {  // проверяет, было ли совпадение
      if (!options.count_only && !options.files_name_only) {
        print_matching_line(argv, buffer, result, match, &regex, line_number);
      }
      match_count++;
    }
    line_number++;
  }

  print_file_summary(argv, match_count);
  regfree(&regex);
}

/**
 * Вывод строки с совпадением согласно флагам
 * @param argv Аргументы командной строки
 * @param line Строка с совпадением
 * @param match_result Результат сопоставления
 * @param match Информация о совпадении
 * @param regex Скомпилированное регулярное выражение
 * @param line_number Номер строки
 */
void print_matching_line(char** argv, const char* line, int match_result,
                         regmatch_t match[], regex_t* regex, int line_number) {
  if (options.only_matching && !options.invert_match) {
    print_matches_only(line, match_result, match, regex, argv, line_number);

  } else if (!options.only_matching) {
    print_line_header(argv, line_number);
    printf("%s", line);
    ensure_proper_newline(line);
  }
}

/**
 * Вывод только совпадающих частей строки (для флага -o)
 * @param line Обрабатываемая строка
 * @param match_result Результат сопоставления
 * @param match Информация о совпадении
 * @param regex Скомпилированное регулярное выражение
 * @param argv Аргументы командной строки
 * @param line_number Номер строки
 */
void print_matches_only(const char* line, int match_result, regmatch_t match[],
                        regex_t* regex, char** argv, int line_number) {
  const char* ptr = line;

  while (!match_result) {
    if (match[0].rm_eo == match[0].rm_so) {
      break;  // если совпадение не найдено
    }

    print_line_header(argv, line_number);

    /*Код выводит подстроку заданной длины, начинающуюся с определенной позиции
     первый аргумент printf() "%.*s\n" указывает на то, что будет выведена
     строка, длина которой будет следующим аргументом (т.е. * означает, что
     длина строки будет определена аргументом) (int)(match[0].rm_eo -
     match[0].rm_so) вычисляет длину подстроки, извлеченной из исходной строки
     ptr match[0].rm_eo и match[0].rm_so указывают на конец и начало найденного
     соответствия регулярного выражения ptr + match[0].rm_so указывает на начало
     подстроки */

    printf("%.*s\n", (int)(match[0].rm_eo - match[0].rm_so),
           ptr + match[0].rm_so);

    ptr += match[0].rm_eo;
    match_result = regexec(regex, ptr, 1, match, REG_NOTBOL);
  }
}

/**
 * Вывод заголовка строки (имя файла и номер строки)
 * @param argv Аргументы командной строки
 * @param line_number Номер строки
 */
void print_line_header(char** argv, int line_number) {
  if (options.files_count > 1 && !options.no_filename) {
    printf("%s:", argv[optind]);
  }

  if (options.line_numbers) {
    printf("%d:", line_number);
  }
}

/**
 * Вывод сводной информации по файлу (для флагов -c и -l)
 * @param argv Аргументы командной строки
 * @param match_count Количество совпадений
 */
void print_file_summary(char** argv, int match_count) {
  if (options.count_only) {
    if (options.no_filename) {
      printf("%d\n", match_count);
    } else if (!options.files_name_only) {
      if (options.files_count > 1) {
        printf("%s:%d\n", argv[optind], match_count);
      } else {
        printf("%d\n", match_count);
      }
    }
  }

  if (options.files_name_only && match_count > 0) {
    printf("%s\n", argv[optind]);
  }
}

/**
 * Проверка и добавление перевода строки при необходимости
 * @param line Проверяемая строка
 */
void ensure_proper_newline(const char* line) {
  if (get_last_character(line) != '\n') {
    printf("\n");
  }
}

/**
 * Создание флагов для регулярного выражения
 * @param ignore_case Флаг игнорирования регистра
 * @return Флаги для компиляции регулярного выражения
 */
int create_regex_flags(bool ignore_case) {
  int flags = REG_EXTENDED;
  if (ignore_case) {
    flags |= REG_ICASE;
  }
  return flags;
}

/**
 * Обработка шаблона из аргумента -e
 * @param pattern_count Счетчик шаблонов
 * @param search_string Буфер для сохранения шаблона
 */
void handle_extended_pattern(int* pattern_count, char* search_string) {
  if (optarg == NULL || *optarg == '\0') {
    optarg = ".";
  }

  add_pattern_separator(pattern_count, search_string);
  strcat(search_string, optarg);
  (*pattern_count)++;
}

/**
 * Обработка шаблонов из файла (флаг -f)
 * @param pattern_count Счетчик шаблонов
 * @param search_string Буфер для сохранения шаблонов
 */
void handle_pattern_from_file(int* pattern_count, char* search_string) {
  FILE* pattern_file = fopen(optarg, "r");
  if (pattern_file == NULL) {
    fprintf(stderr, "Ошибка: Не удалось открыть файл с шаблонами %s\n", optarg);
    exit(EXIT_FAILURE);
  }

  char buffer[BUFFER_SIZE];
  while (fgets(buffer, BUFFER_SIZE, pattern_file) != NULL) {
    remove_trailing_newline(buffer);

    add_pattern_separator(pattern_count, search_string);
    if (*buffer == '\0') {
      strcat(search_string, ".");
    } else {
      strcat(search_string, buffer);
    }
    (*pattern_count)++;
  }

  fclose(pattern_file);
}

/**
 * Обработка шаблона по умолчанию (без флагов -e/-f)
 * @param argv Аргументы командной строки
 * @param search_string Буфер для сохранения шаблона
 */
void handle_default_pattern(char** argv, char* search_string) {
  if (argv[optind] == NULL) {
    argv[optind] = ".";
  }
  strcpy(search_string, argv[optind]);
  optind++;
}

/**
 * Удаление символа новой строки в конце строки
 * @param line Обрабатываемая строка
 */
void remove_trailing_newline(char* line) {
  if (get_last_character(line) == '\n') {
    line[strlen(line) - 1] = '\0';
  }
}

/**
 * Получение последнего символа строки
 * @param line Входная строка
 * @return Последний символ строки
 */
char get_last_character(const char* line) {
  size_t len = strlen(line);
  return len > 0 ? line[len - 1] : '\0';
}

/**
 * Добавление разделителя шаблонов (|) при необходимости
 * @param pattern_count Счетчик шаблонов
 * @param pattern Буфер с шаблонами
 */
void add_pattern_separator(int* pattern_count, char* pattern) {
  if (*pattern_count > 0) {
    strcat(pattern, "|");
  }
}