all: prog format

%.o: %.c
	gcc -std=c89 -Wall -ansi -pedantic -c $<
	astyle --style=ansi $(CUR_DIR)/$<

prog: my_gets.o
	gcc -o my_gets $^

CUR_DIR = $(shell pwd)

%.form: %.c
	astyle --style=ansi $(CUR_DIR)/$<

format: my_gets.form