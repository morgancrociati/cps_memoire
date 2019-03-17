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

void test_fonction(int i)
{
	assert(i == 12);
}

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

	char **mat = mem_alloc(sizeof(char *) * 128);

	for (*i = 0; *i < 128; (*i)++)
	{
		mat[*i] = mem_alloc(sizeof(char) * 128);
	}

	int *j = mem_alloc(sizeof(int));

	for (*i = 0; *i < 128; (*i)++)
	{
		for (*j = 0; *j < 128; (*j)++)
		{
			mat[*i][*j] = 'a' + (*j) % 26;
		}
	}

	for (*i = 0; *i < 128; (*i)++)
	{
		for (*j = 0; *j < 128; (*j)++)
		{
			assert(mat[*i][*j] == 'a' + (*j) % 26);
		}
	}

	test_struct *struc = mem_alloc(sizeof(test_struct));
	struc->a = 'a';
	struc->b = 2;
	struc->c = 0x1212121212;

	assert(struc->a == 'a');
	assert(struc->b == 2);
	assert(struc->c == 0x1212121212);

	*i = 12;
	test_fonction(*i);

	char *passed = mem_alloc(sizeof(char) * 128);
	strcpy(passed, "mem_alloc_test passed\n");

	printf("%s", passed);

	return 0;
}