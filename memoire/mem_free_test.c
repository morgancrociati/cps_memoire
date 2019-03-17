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

	size_t *i = mem_alloc(sizeof(size_t));

	int *tab1 = mem_alloc(sizeof(int) * 128);

	mem_free(tab1);

	int *tab2 = mem_alloc(sizeof(int) * 128);

	//tab2 took the place of tab1
	//This work for all mem_fit
	assert(tab1 == tab2);

	mem_free(tab2);

	char **mat = mem_alloc(sizeof(char *) * 128);

	for (*i = 0; *i < 128; (*i)++)
	{
		mat[*i] = mem_alloc(sizeof(char) * 128);
	}

	int *j = mem_alloc(sizeof(int));

	for (*i = 0; *i < 128; (*i)++)
	{
		mem_free(mat[*i]);
	}
	mem_free(mat);

	mem_free(j);
	j = mem_alloc(sizeof(int));

	int *save = j;
	for (*i = 0; *i < get_memory_size(); (*i)++)
	{
		mem_free(j);
		j = mem_alloc(sizeof(int));
		assert(save == j);
	}

	test_struct *struc = mem_alloc(sizeof(test_struct));
	mem_free(struc);

	char *passed = mem_alloc(sizeof(char) * 128);
	strcpy(passed, "mem_free_test passed\n");

	printf("%s", passed);

	return 0;
}