#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_MEM 400

unsigned char * memory = NULL; // DO NOT REALLOC
uint32_t memory_size = 0;

unsigned short granularity = 8; // should be header_size or larger
unsigned short header_size = 8;

typedef struct {
	uint32_t * size_total; // times granularity, plus current index = index of next
	uint32_t * size_used; // times granularity = size in bytes; includes header
} extracted_header;


/* mem layout:

   [1|1 root] -> [2|2 value] <- memory_size

   There is always an (empty) root.
*/

static void extract_header(extracted_header * h, uint32_t index)
{
	h->size_used = (uint32_t *) &memory[index];
	h->size_total = (uint32_t *) &memory[index+4];
}

static unsigned char * initialize()
{
	if (memory != NULL) return memory;

	memory_size += granularity;
	memory = malloc(MAX_MEM); // DO NOT REALLOC

	extracted_header h;
	extract_header(&h, 0);
	* h.size_used = 1;
	* h.size_total = 1; // idea: set to total in case of fixed size?

	return memory;
}

static uint32_t calc_full_size(uint32_t size)
{
	uint32_t full_size = size + header_size;
	uint32_t remainder = full_size % granularity;
	if (remainder != 0) full_size += granularity - remainder;

	return full_size;
}

static uint32_t allocate_as_new(uint32_t full_size)
{

	uint32_t index = memory_size;
	memory_size += full_size;
	//memory = realloc(memory, memory_size);

	extracted_header h;
	extract_header(&h, index);
	(* h.size_used) = full_size / granularity;
	(* h.size_total) = full_size / granularity;

	return index;
}

static uint32_t allocate_internal(uint32_t size)
{
	initialize();

	uint32_t full_size = calc_full_size(size);

	uint32_t index = 0;
	extracted_header h;

	while (index < memory_size)
	{
		extract_header(&h, index);

		uint32_t difference = (* h.size_total) - (* h.size_used);

		if (difference >= full_size / 8)
		{
			extracted_header new_h;
			extract_header(&new_h, index + (* h.size_used) * granularity);
			* new_h.size_used = full_size / 8;
			* new_h.size_total = difference;

			* h.size_total = *h.size_used;
			return index + (* h.size_used) * granularity;
		}

		index += (* h.size_total) * granularity;
	}

	return allocate_as_new(full_size);
}

void * allocate(uint32_t size)
{
	uint32_t index = allocate_internal(size);
	return &memory[index+header_size];
}

static void free_internal(uint32_t target_index)
{
	uint32_t index = 0;
	extracted_header h, target_header;
	extract_header(&target_header, target_index);

	while (index < target_index)
	{
		extract_header(&h, index);
		index += (* h.size_total) * granularity;
	}

	(* h.size_total) += (* target_header.size_total);
}

// always returns null, as a service: bla = release(bla); // bla is null
void * release(void * value)
{
	unsigned char * val = (unsigned char *) value;
	uint32_t index = val - memory;
	index -= header_size;

	free_internal(index);

	return NULL;
}

void copy_mem(uint32_t old_index, uint32_t new_index, uint32_t size_used)
{
	// skip first four bits; they are size_total, which is specific
	// to the container and should not be copied
	for(int i=header_size;i<size_used * granularity;i++)
	{
		memory[new_index+i] = memory[old_index+i];
	}
}

void * reallocate(void * value, int size)
{
	unsigned char * val = (unsigned char *) value;
	uint32_t index = val - memory;
	index -= header_size;

	uint32_t full_size = calc_full_size(size);

	extracted_header h;
	extract_header(&h, index);

	uint32_t full_size_g = full_size / granularity;

	if (full_size_g < (* h.size_used))
	{
		// we're shrinking; do this at the same spot
		(* h.size_used) = full_size / granularity;
	}

	if (full_size_g > (* h.size_used))
	{
		bool at_end = index + ((* h.size_total) * 8) == memory_size;

		// we're growing
		if (full_size_g > (* h.size_total) && !at_end)
		{
			uint32_t old_index = index;
			index = allocate_internal(size);
			copy_mem(old_index, index, (* h.size_used));
			free_internal(old_index);
		} else
		{
			if (at_end)
			{
				memory_size = index + full_size;
				(* h.size_total) = full_size_g;
			 }
			(* h.size_used) = full_size_g;
		}


	}

	return &memory[index+header_size];
}

void visualize()
{
	initialize();

	uint32_t index = 0;
	extracted_header h;

	while (index < memory_size)
	{
		extract_header(&h, index);
/*
		printf("\nst: %d\n", (* h.size_total) * granularity);
		printf("su: %d\n", (* h.size_used) * granularity);
*/
		printf("#");
		int32_t used = (* h.size_used);
		for(int i=0;i<used-1;i++) printf("*");
		for(int i=0;i<(* h.size_total)-used;i++) printf("-");

		if ((* h.size_total) == 0)
		{
			printf("error\n");
		       	exit(-1);
		}

		index += (* h.size_total) * granularity;
	}

	printf("\n");
}
