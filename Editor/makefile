all: prog format

%.o: %.c
	gcc -std=c89 -Wall -ansi -pedantic -c $<
	astyle --style=ansi $(CUR_DIR)/$<

prog: editor.o
	gcc -o editor $^

CUR_DIR = $(shell pwd)

%.form: %.c
	astyle --style=ansi $(CUR_DIR)/$<

format: editor.form