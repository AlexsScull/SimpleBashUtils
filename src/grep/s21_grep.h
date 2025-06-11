#ifndef SRC_GREP_S21_GREP_H_
#define SRC_GREP_S21_GREP_H_

#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096

/* Структура для хранения опций программы */
typedef struct {
  bool use_extended_pattern;  // Флаг -e
  bool case_insensitive;      // Флаг -i
  bool invert_match;          // Флаг -v
  bool count_only;            // Флаг -c
  bool files_name_only;       // Флаг -l
  bool line_numbers;          // Флаг -n
  bool no_filename;           // Флаг -h
  bool no_errors_file;        // Флаг -s
  bool patterns_from_file;    // Флаг -f
  bool only_matching;         // Флаг -o
  int files_count;  // Количество файлов для обработки
} ProgramOptions;

ProgramOptions options;

void initialize_options(void);
void parse_arguments(int argc, char** argv, char* search_pattern);
void process_files(int argc, char** argv, const char* pattern);
void search_in_file(char** argv, const char* pattern, FILE* file);
void print_matching_line(char** argv, const char* line, int match_result,
                         regmatch_t match[], regex_t* regex, int line_number);
void print_matches_only(const char* line, int match_result, regmatch_t match[],
                        regex_t* regex, char** argv, int line_number);
void print_line_header(char** argv, int line_number);
void print_file_summary(char** argv, int match_count);

void ensure_proper_newline(const char* line);
int create_regex_flags(bool ignore_case);
void handle_extended_pattern(int* pattern_count, char* search_string);
void handle_pattern_from_file(int* pattern_count, char* search_string);
void handle_default_pattern(char** argv, char* search_string);
void remove_trailing_newline(char* line);
char get_last_character(const char* line);
void add_pattern_separator(int* pattern_count, char* pattern);

#endif  // SRC_GREP_S21_GREP_H_
