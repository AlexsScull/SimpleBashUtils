CC=gcc
CFLAGS=-std=c11 -Wall -Werror -Wextra -pedantic


all: s21_cat test_s21_cat clean_peace

s21_cat: s21_cat.c 
	$(CC) $(CFLAGS) s21_cat.c -o s21_cat

test_s21_cat: test_s21_cat.c
	$(CC) $(CFLAGS) test_s21_cat.c -o test_s21_cat -D_GNU_SOURCE -fsanitize=address

rebuild: clean all

clean_peace:
	rm -rf *.o

clean:
	rm -rf *.o *.txt s21_cat test_s21_cat



