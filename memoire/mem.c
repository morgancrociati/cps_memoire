#include "mem.h"
#include "common.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

// constante définie dans gcc seulement
#ifdef __BIGGEST_ALIGNMENT__
#define ALIGNMENT __BIGGEST_ALIGNMENT__
#else
#define ALIGNMENT 16
#endif

/*
v <- nombre à aligner
a <- alignement
*/
#define align(v, a) \
	(((v) + (a)-1) & ~((a)-1))

/*
#define align(a) a + (ALIGNMENT - a%ALIGNMENT)%ALIGNEMENT
*/

#define abs(a) (a < 0) ? (-a) : (a)

typedef char octet;

/*
fb <- block de mémoire libre
size <- taille du block libre
next <- adresse du prochain block libre
Penser à trier les blocks libres par ordre croissant
*/
typedef struct fb
{
	size_t size;
	struct fb *next;
} fb;

void mem_init(void *mem, size_t taille)
{
	assert(mem == get_memory_adr());
	assert(taille == get_memory_size());

	/*memoire contient l'adresse où débute notre mémoire*/
	octet *memoire = mem;

	//Initialisation de la mémoire à 0 /* /!\ Etape non obligatoire /!\ */
	for (size_t i = 0; i < taille; i++)
	{
		memoire[i] = 0;
	}

	/*On stock dans notre header la position du premier block libre,
	il faut donc laisser la place pour l'adresse du premier block libre!
	De plus il faut faire attention à l'algnement*/
	size_t tailleHeader = align(sizeof(size_t), ALIGNMENT);

	//Pointeur sur le premier fb
	*((size_t *)memoire) = memoire + tailleHeader; //OU ((size_t *)memoire)[0] = memoire + align(sizeof(size_t), ALIGNMENT);

	//Création et initialisation du premier fb
	fb *premierFB = (fb *)(((size_t *)memoire)[0]); // OU (fb*)(*((size_t *)memoire)) OU (fb*)(memoire + align(sizeof(size_t), ALIGNMENT))
	premierFB->size = taille - tailleHeader;
	premierFB->next = NULL; //Pas d'autre block libre pour l'instant

	mem_fit(&mem_fit_first);
}

void mem_show(void (*print)(void *, size_t, int))
{
	/* ... */
	while (/* ... */ 0)
	{
		/* ... */
		print(/* ... */ NULL, /* ... */ 0, /* ... */ 0);
		/* ... */
	}
}

static mem_fit_function_t *mem_fit_fn;
void mem_fit(mem_fit_function_t *f)
{
	mem_fit_fn = f;
}

void *mem_alloc(size_t taille)
{
	//On obtient l'adresse de notre mémoire
	size_t *memoire = get_memory_adr();

	fb *premierFB = (fb *)memoire[0];
	assert(premierFB != NULL); //Si cela arrive alors la mémoire est pleine

	size_t actualSize = align(taille + sizeof(size_t), ALIGNMENT);

	__attribute__((unused)) /* juste pour que gcc compile ce squelette avec -Werror */
	//On ajoute à la taille assez de place pour stocker la taille dans le bloc de mémoire
	fb *emplacement = mem_fit_fn(premierFB, actualSize);

	assert(emplacement != NULL); //Si cela arrive alors il n'y pas assez de place dans la mémoire

	fb *tmp;

	/*L'emplacement trouvé est plus gros que ce que l'on veut
	stocker, on crée donc un autre emplacement libre avec la
	place restante.*/
	if (emplacement->size > actualSize)
	{
		fb *prochainEmplacement = (fb *)(emplacement + actualSize);
		prochainEmplacement->size = emplacement->size - actualSize;

		//On veut ici supprimer l'ancien block libre
		//Cas où le bloc libre n'était pas le premier block libre
		if (premierFB != emplacement)
		{
			tmp = premierFB;
			while (tmp->next != emplacement)
			{
				tmp = tmp->next;
			}
			tmp->next = tmp->next->next;
		}

		//On ajoute notre nouveau bloc libre à notre liste chaînée de bloc libre
		prochainEmplacement->next = premierFB->next;
		memoire[0] = prochainEmplacement;
	}
	//L'emplacement trouvé fait exactement la taille demandé
	else
	{
		tmp = premierFB;
		if (tmp == emplacement)
		{
			memoire[0] = emplacement->next;
		}
		else
		{
			while (tmp->next != emplacement)
			{
				tmp = tmp->next;
			}
			tmp->next = tmp->next->next;
		}
	}

	*((size_t *)emplacement) = actualSize;

	return emplacement + sizeof(size_t);
}

void mem_free(void *mem)
{
}

struct fb *mem_fit_first(struct fb *list, size_t size)
{
	while (list != NULL && list->size < size)
	{
		list = list->next;
	}
	return list;
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone)
{
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
	return 0;
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb *mem_fit_best(struct fb *list, size_t size)
{
	fb *best = list;
	register int value = abs(size - best->size);
	while (list != NULL)
	{
		if (abs(size - list->size) < value)
		{
			value = abs(size - list->size);
			best = list;
		}
		list = list->next;
	}
	return best;
}

struct fb *mem_fit_worst(struct fb *list, size_t size)
{
	fb *best = list;
	register int value = abs(size - best->size);
	while (list != NULL)
	{
		if (abs(size - list->size) > value)
		{
			value = abs(size - list->size);
			best = list;
		}
		list = list->next;
	}
	return best;
}
