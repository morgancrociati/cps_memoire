#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[]) {
	//Initialisation de notre mémoire
	mem_init(get_memory_adr(), get_memory_size());

	int* i = mem_alloc(sizeof(int));
	
	int* tab= mem_alloc(sizeof(int)*10);

	for(*i = 0; *i < 10; (*i)++){
		tab[*i] = *i;
	}

	for(*i = 0; *i < 10; (*i)++){
		printf("%d  ", tab[*i]);
	}
	printf("\n");

	mem_free(tab);

	char* chaine = mem_alloc(sizeof(char)*27);

	for(*i = 0; *i < 26; (*i)++){
		chaine[*i] = 'A' + *i;
	}
	chaine[*i] = '\0';

	printf("%s\n", chaine);

	mem_free(chaine);

	printf("Etes vous satisfait des tests ? (y/n)\n");

	char* reponse = mem_alloc(sizeof(char));

	scanf("%c", reponse);

	if(*reponse == 'y'){
		printf("Cool\n");
	}
	else{
		printf("Dommage\n");
	}

	return 0;
}