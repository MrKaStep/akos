CUR_DIR = $(shell pwd)
OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
FORMS = $(patsubst %.c, %.form, $(wildcard *.c))

my_shell: $(OBJS)
	gcc -g -o my_shell $^

%.o: %.c
	gcc -g -std=c89 -Wall -Wextra -ansi -pedantic -c $<
	astyle --style=ansi $(CUR_DIR)/$<


%.form: %.c
	astyle --style=ansi $(CUR_DIR)/$<

format: $(FORMS)

clean:
	rm -f *.c.orig *.o my_shell