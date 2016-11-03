#include <stdio.h>
#include <stdint.h>

#include "diymm.h"

/* Different tests, simply being run from 'main' */

char * items[3];

static void reset()
{
	for (int i=0; i<3;i++)
	{
		if (items[i] != NULL)
			items[i] = release(items[i]);
	}
}

static void setup_3 ()
{
	reset();
	printf("\nSetup 3 items (17, 25, 35): ");
	items[0] = allocate(17);
	items[1] = allocate(25);
	items[2] = allocate(35);

	sprintf(items[0], "1234567890123456");
	sprintf(items[1], "toedeloe");
	sprintf(items[2], "mazzel");
	visualize();
}

static void delete(int which)
{
	printf("Delete item %d             : ", which);
	items[which-1] = release(items[which-1]);
	visualize();
}

static void insert(uint32_t size)
{
	printf("Insert item of size %2d    : ", size);
	items[1] = allocate(size);
	visualize();
}

static void rethink(int which, int what)
{
	printf("Reallocate item %d         : ", which);
	items[which-1] = reallocate(items[which-1], what);
	visualize();
}

int main (int argc, char ** argv)
{
	visualize();
	setup_3(); delete(1);
	setup_3(); delete(2);
	setup_3(); delete(3);

	setup_3(); delete(1); delete(2); delete(3);
	setup_3(); delete(3); delete(2); delete(1);

	setup_3(); rethink(1, 2);
	setup_3(); rethink(2, 2);
	setup_3(); rethink(3, 2);

	setup_3(); delete(2); insert(20);
	setup_3(); delete(2); insert(32);
	setup_3(); delete(2); insert(33);

	setup_3(); rethink(1, 33);
printf(items[0]);
	setup_3(); rethink(2, 40);
	setup_3(); rethink(3, 50);
	// This one is expected to perform sub-optimal.
	// The algorithm is unaware that the last item
	// can be enlarged so reallocs instead.
	setup_3(); rethink(3, 90);

	return 0;
}
