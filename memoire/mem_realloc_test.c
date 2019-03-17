#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct test_struct
{
	char a;
	int b;
	long c;
} test_struct;

int main(int argc, char *argv[])
{
	//Initialisation de notre mémoire
	mem_init(get_memory_adr(), get_memory_size());

	int *i = mem_alloc(sizeof(int));

	int *tab = mem_alloc(sizeof(int) * 128);

	for (*i = 0; *i < 128; (*i)++)
	{
		tab[*i] = *i;
	}

	for (*i = 0; *i < 128; (*i)++)
	{
		assert(tab[*i] == *i);
	}

	int *save = tab;
	tab = mem_realloc(tab, sizeof(int) * 256);

	assert(save == tab);

	for (*i = 0; *i < 128; (*i)++)
	{
		assert(tab[*i] == *i);
	}

	int *j = mem_alloc(sizeof(int));

	tab = mem_realloc(tab, sizeof(int) * 512);

	assert(save != tab);

	for (*i = 0; *i < 128; (*i)++)
	{
		assert(tab[*i] == *i);
	}

	for (*i = 128; *i < 512; (*i)++)
	{
		tab[*i] = *i;
	}

	for (*j = 0; *j < 512; (*j)++)
	{
		assert(tab[*j] == *j);
	}

	char *passed = mem_alloc(sizeof(char) * 128);
	strcpy(passed, "mem_realloc_test passed\n");

	printf("%s", passed);

	return 0;
}