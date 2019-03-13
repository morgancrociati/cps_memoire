#include "mem.h"
#include "common.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

//A SUPPRIMER
#include <stdio.h>

// constante définie dans gcc seulement
#ifdef __BIGGEST_ALIGNMENT__
#define ALIGNMENT __BIGGEST_ALIGNMENT__
#else
#define ALIGNMENT 16
#endif

/*
mlmlmlm
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
	*((size_t *)memoire) = (size_t)memoire + tailleHeader; //OU ((size_t *)memoire)[0] = memoire + align(sizeof(size_t), ALIGNMENT);

	//Création et initialisation du premier fb
	fb *premierFB = (fb *)(((size_t *)memoire)[0]); // OU (fb*)(*((size_t *)memoire)) OU (fb*)(memoire + align(sizeof(size_t), ALIGNMENT))
	premierFB->size = taille - tailleHeader;
	premierFB->next = NULL; //Pas d'autre block libre pour l'instant

	mem_fit(&mem_fit_first);
}

void mem_show(void (*print)(void *, size_t, int))
{
	size_t *memoire = get_memory_adr();
	size_t tailleMemoire = get_memory_size();

	fb *tmp = (fb *)memoire[0];
	size_t posInMem = (size_t)memoire + align(sizeof(size_t), ALIGNMENT);

	while (posInMem < (size_t)memoire + tailleMemoire)
	{
		if (posInMem == (size_t)tmp)
		{
			print((size_t *)posInMem, *((size_t *)posInMem), 1);
			tmp = tmp->next;
		}
		else
		{
			print((size_t *)(posInMem + sizeof(size_t)), *((size_t *)posInMem) - sizeof(size_t), 0);
		}
		posInMem += *((size_t *)posInMem);
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
	//assert(premierFB != NULL); //Si cela arrive alors la mémoire est pleine
	if (premierFB == NULL)
	{
		return NULL;
	}

	size_t actualSize = align(taille + sizeof(size_t), ALIGNMENT);

	__attribute__((unused)) /* juste pour que gcc compile ce squelette avec -Werror */
	//On ajoute à la taille assez de place pour stocker la taille dans le bloc de mémoire
	fb *emplacement = mem_fit_fn(premierFB, actualSize);

	//assert(emplacement != NULL); //Si cela arrive alors il n'y pas assez de place dans la mémoire
	if (emplacement == NULL)
	{
		return NULL;
	}

	fb *tmp;

	/*L'emplacement trouvé est plus gros que ce que l'on veut
	stocker, on crée donc un autre emplacement libre avec la
	place restante.*/
	if (emplacement->size > actualSize)
	{
		fb *prochainEmplacement = (fb *)(((size_t)emplacement) + actualSize);
		prochainEmplacement->size = emplacement->size - actualSize;
		prochainEmplacement->next = emplacement->next;

		//Si l'emplacement libre trouvé est en faites aussi le premier emplacement libre
		if (premierFB == emplacement)
		{
			memoire[0] = (size_t)prochainEmplacement;
		}
		//Sinon l'emplacement libre trouvé n'est pas le premier emplacement libre
		else
		{
			tmp = premierFB;
			//On cherche l'emplacement libre avant l'emplacement que l'on avait trouvé
			while (tmp->next != NULL && tmp->next < emplacement)
			{
				tmp = tmp->next;
			}
			tmp->next = prochainEmplacement;
		}
	}
	//L'emplacement trouvé fait exactement la taille demandé
	else
	{
		//Si l'emplacement libre trouvé est le le premier bloc libre
		if (premierFB == emplacement)
		{
			memoire[0] = (size_t)emplacement->next;
		}
		//Sinon l'emplacement libre trouvé n'est pas le premier bloc libre
		else
		{
			tmp = premierFB;
			while (tmp->next != emplacement)
			{
				tmp = tmp->next;
			}
			tmp->next = tmp->next->next;
		}
	}

	*((size_t *)emplacement) = actualSize;

	fb *resultat = (fb *)((size_t)emplacement + sizeof(size_t));

	return resultat;
}

void mem_free(void *mem)
{
	//On fait - sizeof(size_t) car on a stocké au début du bloc mémoire la taille du bloc
	size_t *emplacement = mem - sizeof(size_t);
	size_t taille = *emplacement;
	assert(taille > 0); //Il y a une erreur si la taille du bloc est égal à 0


	size_t *memoire = get_memory_adr();

	//On doit ici voir si il y a un bloc libre avant et/ou après

	fb *beforeTmp = NULL;
	fb *tmp = (fb *)memoire[0];

	//Tant que notre bloc libre courrante existe
	//et que son adresse est plus petite que notre emplacement
	while (tmp != NULL && tmp < (fb *)emplacement)
	{
		//Si le bloc libre courant est pile avant notre emplacement
		if (((size_t)tmp + tmp->size) == (size_t)emplacement)
		{
			//On augmente alors la taille de ce bloc libre
			tmp->size += taille;
			break;
		}
		beforeTmp = tmp;
		tmp = tmp->next;
	}

	//Si il n'y a aucun bloc libre collé avant notre emplacement et après notre emplacement
	if (tmp == NULL)
	{
		((fb *)emplacement)->size = taille;
		((fb *)emplacement)->next = NULL;
		//Si la mémoire était pleine et qu'il n'y avait aucun bloc libre
		if (beforeTmp == NULL)
		{
			memoire[0] = (size_t)emplacement;
		}
		//Sinon il y avait des blocs libres avant notre emplacement
		//beforeTmp représente ici le dernier bloc libre situé avant nous
		else
		{
			beforeTmp->next = (fb *)emplacement;
		}
	}
	//Sinon si il y avait des bloc libre collé avant notre emplacement
	//tmp représente ici le premier bloc libre après notre emplacement
	else if (tmp > (fb *)emplacement)
	{
		((fb *)emplacement)->size = taille;

		//Si il n'y avait aucun bloc libre avant notre emplacement
		if (beforeTmp == NULL)
		{
			memoire[0] = (size_t)emplacement;
		}
		//Sinon il y avait un bloc libre avant notre emplacement
		else
		{
			beforeTmp->next = (fb *)emplacement;
		}

		//Si le bloc libre suivant est pile après
		if ((size_t)emplacement + taille == (size_t)tmp)
		{
			((fb *)emplacement)->size += tmp->size;
			((fb *)emplacement)->next = tmp->next;
		}
		else
		{
			((fb *)emplacement)->next = tmp;
		}
		tmp = (fb *)emplacement;
	}
	//Sinon si il y avait un bloc libre pile avant notre emplacement
	else if (tmp < (fb *)emplacement)
	{
		if ((size_t)tmp + tmp->size == (size_t)tmp->next)
		{
			tmp->size += tmp->next->size;
			tmp->next = tmp->next->next;
		}
	}
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
	size_t *size = zone - sizeof(size_t);
	return *size;
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
	fb *worst = list;
	register int value = abs(size - worst->size);
	while (list != NULL)
	{
		if (abs(size - list->size) > value)
		{
			value = abs(size - list->size);
			worst = list;
		}
		list = list->next;
	}
	return worst;
}

struct fb *mem_fit_balanced(struct fb *list, size_t size)
{
	fb *worst = list;
	fb *best = list;
	register int valueW = abs(size - worst->size);
	register int valueB = abs(size - best->size);
	while (list != NULL)
	{
		if (abs(size - list->size) > valueW)
		{
			valueW = abs(size - list->size);
			worst = list;
		}
		else if (abs(size - list->size) < valueB)
		{
			valueB = abs(size - list->size);
			best = list;
		}
		list = list->next;
	}
	if(valueB == 0){
		return best;
	}
	else{
		return worst;
	}
}

//Il faut peut être supprimer l'ancienne emplacement quand on renvoie NULL
void *mem_realloc(void *old, size_t new_size)
{
	size_t *memoire = get_memory_adr();
	//Il faut aligner la nouvelle taille ainsi que de ne pas oublier de garder la taille du bloc
	new_size = align(new_size + sizeof(size_t), ALIGNMENT);
	size_t old_size = mem_get_size(old);
	size_t emplacement = (size_t)old - sizeof(size_t);
	fb *premierFB = (fb *)memoire[0];
	//Si il n'y a plus d'espace libre on renvoie NULL
	if (premierFB == NULL)
	{
		return NULL;
	}
	//L'utilisation veut une plus grande zone mémoire
	if (new_size > old_size)
	{
		fb *beforetmp = NULL;
		fb *tmp = premierFB;
		while (tmp != NULL)
		{
			//Si il existe un bloc libre après notre emplacement qui permet de contenir la nouvelle taille
			if (emplacement + old_size == (size_t)tmp)
			{
				//Le bloc libre permet d'avoir exactement la taille demandé
				if (old_size + tmp->size == new_size)
				{
					//Si le bloc libre suivant notre emplacement était le premier bloc libre
					if (beforetmp == NULL)
					{
						memoire[0] = (size_t)tmp->next;
					}
					//Sinon il y avait d'autre bloc libre avant
					else
					{
						beforetmp->next = tmp->next;
					}
					*((size_t *)emplacement) = new_size;
					//On a pu juste augmenter la taille de l'ancien emplacement
					return old;
				}
				//Le bloc libre permet d'avoir une taille plus grande que celle demandé
				else if (old_size + tmp->size > new_size)
				{
					fb *nouveauFB = (fb *)(emplacement + new_size);
					nouveauFB->next = tmp->next;
					nouveauFB->size = (old_size + tmp->size) - new_size;
					if (beforetmp == NULL)
					{
						memoire[0] = (size_t)nouveauFB;
					}
					//Sinon il y avait d'autre bloc libre avant
					else
					{
						beforetmp->next = nouveauFB;
					}
					//On a pu juste augmenter la taille de l'ancien emplacement
					return old;
				}
				break;
			}
			beforetmp = tmp;
			tmp = tmp->next;
		}
		//Si on arrive ici alors il n'y avait pas de bloc libre pile après (ou du moins pas assez grand)
		//On cherche un nouvelle emplacement mémoire avec notre nouvelle taille
		octet *nouvelleEmplacement = mem_alloc(new_size - sizeof(size_t)); //On avait ajouté la taille de la taille
		//Si il n'y a pas la place de stocké ça en mémoire on renvoie NULL
		if (nouvelleEmplacement == NULL)
		{
			return NULL;
		}
		//On copie les anciennes données
		for (int i = sizeof(size_t); i < old_size; i++)
		{
			nouvelleEmplacement[i] = ((octet *)old)[i];
		}
		//Il faut maintenant supprimer l'ancien bloc
		mem_free(old);
		return nouvelleEmplacement;
	}
	//Peut être faut il faire le cas si l'utilisateur veut diminuer de taille
	return old;
}