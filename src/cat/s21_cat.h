#ifndef SRC_CAT_S21_CAT_H_
#define SRC_CAT_S21_CAT_H_

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Структура для хранения флагов программы */
typedef struct {
  bool number_nonempty;  // Флаг -b (нумерует непустые строки)
  bool show_ends;  // Флаг -e (показывает концы строк как $)
  bool show_nonprinting;  // Флаг -v (показывает непечатаемые символы)
  bool squeeze_blank;  // Флаг -s (сжимает несколько пустых строк)
  bool show_tabs;   // Флаг -t (показывает табы как ^I)
  bool number_all;  // Флаг -n (нумерует все строки)
} ProgramFlags;

ProgramFlags flags;

void initialize_flags(void);

void parse_command_line_options(int argc, char **argv);

void print_usage_error(const char *invalid_option);

void process_input_files(int argc, char **argv);

void print_file_error(const char *filename);

void process_file_contents(FILE *file, size_t *line_counter,
                           int *consecutive_empty_lines);

void handle_nonprinting_characters(int *character);

void print_line_number(size_t *line_counter, bool *is_new_line);

void print_tab_character(void);

void print_end_of_line(void);

bool should_skip_repeated_empty_lines(int character, int *empty_line_count);

#endif  // SRC_CAT_S21_CAT_H_
