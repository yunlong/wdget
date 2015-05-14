
#define USE_LIST_H
#define USE_SLAB_H
#include "matrix.h"

#define MAX_FREE_SLABS		8

namespace matrix {

Cache *Cache::cacheList = NULL;

int Cache::reclaimAll()
{
	int ret = 0;
	for (Cache *p = cacheList; p; p = p->nextCache)
		ret += p->reclaim();
	return ret;
}

Cache::Cache(int size, int n)
{
	objSize = size + sizeof(OBJ);
	numObjs = n;
	numFreeSlabs = 0;
	firstNotFull = &slabList;

	slabSize = sizeof(SLAB) + objSize * n;

	// Add it to the cache chain
	nextCache = cacheList;
	cacheList = this;
}

Cache::~Cache()
{
	while (!slabList.isEmpty()) {
		ListHead *pos = slabList.removeHead();
		SLAB *slab = LIST_ENTRY(pos, SLAB, item);
		free(slab);
	}
}

/*
 * Reclaim empty slabs in this cache
 */
int Cache::reclaim(int n)
{
	SLAB *slab;
	ListHead *pos, *t = firstNotFull;

	int i = 0;
	while (i < n && (pos = slabList.prev) != &slabList) {
		slab = LIST_ENTRY(pos, SLAB, item);
		if (slab->inuse)
			break;

		i++;
		if (firstNotFull == pos)
			firstNotFull = pos->prev;
		pos->remove();
		free(slab);
	}

	if (firstNotFull != t && firstNotFull != &slabList) {
		slab = LIST_ENTRY(firstNotFull, SLAB, item);
		if (slab->inuse == numObjs)
			firstNotFull = &slabList;
	}

	numFreeSlabs += i;
	return i;
}

/*
 * Alloc a new slab
 */
SLAB *Cache::newSlab()
{
	SLAB *slab = (SLAB *) malloc(slabSize);
	if (!slab) {
		if (reclaimAll() > 0) {
			slab = (SLAB *) malloc(slabSize);
			if (!slab)
				return NULL;
		}
	}

	slab->inuse = 0;

	OBJ *obj = (OBJ *) (slab + 1);
	slab->free = obj;
	for (int i = 0; i < numObjs - 1; ++i) {
		OBJ *next = (OBJ *) ((char *) obj + objSize);
		obj->next = next;
		obj->slab = slab;
		obj = next;
	}
	obj->next = NULL;
	obj->slab = slab;

	slabList.add(&slab->item);
	firstNotFull = &slab->item;

	return slab;
}

/*
 * Alloc an object from the cache
 */
void *Cache::allocObj()
{
	void *obj = NULL;
	SLAB *slab;

	if (firstNotFull == &slabList)
		slab = newSlab();
	else {
		slab = LIST_ENTRY(firstNotFull, SLAB, item);
		if (!slab->inuse)
			numFreeSlabs--;
	}

	if (slab) {
		slab->inuse++;
		obj = slab->free + 1;
		slab->free = slab->free->next;

		if (!slab->free)
			firstNotFull = slab->item.next;
	}

	return obj;
}

/*
 * Free an object in the cache
 */
void Cache::freeObj(void *p)
{
#ifdef _DEBUG
	memset(p, 0xdd, objSize - sizeof(OBJ));
#endif

	OBJ *obj = (OBJ *) p - 1;
	SLAB *slab = obj->slab;

	obj->next = slab->free;
	slab->free = obj;

	ListHead *pos;
	if (slab->inuse-- == numObjs) {
		ListHead *t = firstNotFull;
		pos = &slab->item;
		firstNotFull = pos;
		if (pos->next != t) {
			pos->remove();
			t->add(pos);
		}
	} else if (!slab->inuse) {
		int n = ++numFreeSlabs - MAX_FREE_SLABS;
		if (n > 0)
			reclaim(n);

		ListHead *t = firstNotFull->prev;
		pos = &slab->item;

		pos->remove();
		slabList.add(pos);
		if (firstNotFull == &slab->item)
			firstNotFull = t->next;
	}
}

}

