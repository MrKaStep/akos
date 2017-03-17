#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define E_MALLOC 1
#define E_CORPT  2


int expand_string(char** str, int len)
{
	int add = len;
	char* temp;
	while(add > 0)
	{
		temp = realloc(*str, (len + add) * sizeof(char));
		if(temp != NULL)
		{
			*str = temp;
			break;
		}
		add >>= 1;
	}
	return add;
}

int read_string(char** str, FILE* f)
{
	int fd = fileno(f);
	char* s = malloc(4 * sizeof(char));
	int buf = 4;
	int sz = 0;
	while(1)
	{
		if(sz == buf - 1)
		{
			if(expand_string(&s, buf) == 0)
			{
				free(s);
				return E_MALLOC;
			}
		}
		int rd = read(fd, s + sz);
		if(rd == -1)
		{
			if(rd == E_AGAIN)
			{
				continue;
			}
			perror("read_string: ");
			return -1;
		}
		if(rd == 0)
		{
			s[sz] = 0;
			*str = s;
			return sz;
		}
		if(rd != sizeof(char))
		{
			s[sz] = 0;
			*str = s;
			return E_CORPT;
		}
		if(s[sz] == '\n')
		{
			s[sz] = 0;
			*str = s;
			return sz;
		}
		++sz;
	}
	*str = s;
	return sz;
}