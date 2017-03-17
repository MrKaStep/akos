#include <stdio.h>
#include <unistd.h>

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
	while(true)
	{

	}
}