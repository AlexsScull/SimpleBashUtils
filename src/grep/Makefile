CC=gcc
CFLAGS=-std=c11 -Wall -Werror -Wextra -pedantic


all: s21_grep test_s21_grep clean_peace


s21_grep: s21_grep.c
	$(CC) $(CFLAGS) s21_grep.c -o s21_grep

test_s21_grep: test_s21_grep.c
	$(CC) $(CFLAGS) test_s21_grep.c -o test_s21_grep -D_GNU_SOURCE -fsanitize=address

rebuild: clean all

clean_peace:
	rm -rf *.o

clean:
	rm -rf *.o *.txt s21_grep test_s21_grep




