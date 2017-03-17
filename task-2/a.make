all: prog format

%.o: %.c
	gcc -std=c89 -Wall -ansi -pedantic -c $<

prog: main.o
	gcc -o my_cp $^

CUR_DIR = $(shell pwd)

%.form: %.c
	astyle --style=ansi $(CUR_DIR)/$<

format: main.form