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

/* La seule variable globale autorisée
 * On trouve à cette adresse la taille de la zone mémoire
 */
static void* memory_addr;

static inline void *get_system_memory_adr() {
	return memory_addr;
}

/* struct for free block */
struct free_bloc {
	size_t size;
	struct free_bloc* next;
};

typedef struct free_bloc* pfree_block;

struct first_bloc {
	size_t sizeG;
	pfree_block begin;
};

typedef struct first_bloc first_bloc;

struct bloc_used_ {
	size_t sizeUsed;
};

typedef struct bloc_used_ bloc_used;


static inline size_t get_system_memory_size() {
	return ((first_bloc*)memory_addr)->sizeG;
}


void mem_init(void* mem, size_t taille)
{
	/* Création du bloc jaune */
        memory_addr = mem;
        ((first_bloc*)memory_addr)->sizeG = taille;
        ((first_bloc*)memory_addr)->begin = memory_addr + sizeof(first_bloc);
        ((first_bloc*)memory_addr)->begin->next = NULL;
        ((first_bloc*)memory_addr)->begin->size = taille - sizeof(struct first_bloc);
	assert(mem == get_system_memory_adr());
	assert(taille == get_system_memory_size());
	mem_fit(&mem_fit_first);
}

void mem_show(void (*print)(void *, size_t, int)) {
	pfree_block free_bloc = ((first_bloc*)memory_addr)->begin;
	bloc_used* moving = memory_addr + sizeof(first_bloc);
	void* end_mem = get_system_memory_adr() + get_system_memory_size();

	while((void*)moving < end_mem){
		if ((pfree_block)moving != free_bloc){
			print(moving, moving->sizeUsed, 0);
		} else {
			print(moving, moving->sizeUsed, 1);
			free_bloc = free_bloc->next;
		}
		moving = (bloc_used*)((void*)moving + moving->sizeUsed);
	}
}

static mem_fit_function_t *mem_fit_fn;

void mem_fit(mem_fit_function_t *f) {
	mem_fit_fn=f;
}

void *mem_alloc(size_t taille) {
	/* __attribute__((unused))  juste pour que gcc compile ce squelette avec -Werror */
																/* size for the user + the metadata needed */
	struct free_bloc *free_bloc=mem_fit_fn(((first_bloc*)memory_addr)->begin, taille + sizeof(bloc_used));
	/* free_bloc means free_bloc; here i get either the address to allocate or NULL which means do nothing*/
	if(free_bloc != NULL){
		pfree_block previous = ((first_bloc*)memory_addr)->begin;
		/* previous means the block that were pointing at that free block*/
		if (free_bloc != ((first_bloc*)memory_addr)->begin){

		/* check if the first free block is the one returned */
			while(previous->next != free_bloc){
				previous = previous->next;
			}

			/* update the pointer of free block */
			previous->next = free_bloc + taille + sizeof(bloc_used);
			((bloc_used*)free_bloc)->sizeUsed = taille;
			((pfree_block)previous->next)->size -= taille + sizeof(bloc_used); 
			/**/ 

		} else {
			((first_bloc*)memory_addr)->begin += taille + sizeof(bloc_used); 
		}
	}
	return free_bloc;
}

void mem_free(void* mem) {
}

struct free_bloc* mem_fit_first(struct free_bloc *list, size_t size) {
	while(size > list->size + sizeof(bloc_used)){
		if (list->next == NULL){
			return NULL;
		}
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
size_t mem_get_size(void *zone) {
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
	return 0;
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct free_bloc* mem_fit_best(struct free_bloc *list, size_t size) {
	return NULL;
}

struct free_bloc* mem_fit_worst(struct free_bloc *list, size_t size) {
	return NULL;
}