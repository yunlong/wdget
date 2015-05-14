#ifndef __SLAB_ALLOC_H__
#define __SLAB_ALLOC_H__

namespace matrix {

struct SLAB;

struct OBJ {
	OBJ *next;
	SLAB *slab;
};

struct SLAB {
	ListHead item;
	int inuse;
	OBJ *free;
};

class Cache {
public:
	Cache(int size, int n);
	~Cache();

	void *allocObj();
	void freeObj(void *p);

	static int reclaimAll();

private:
	SLAB *newSlab();
	int reclaim(int n = 0xffff);

	Cache *nextCache;
	ListHead slabList;
	ListHead *firstNotFull;

	int objSize;
	int numObjs;
	int numFreeSlabs;
	int slabSize;

	static Cache *cacheList;
};

}

#define DECLARE_SLAB(type)		\
private:	\
	static Cache type##_cache;	\
public:	\
	void *operator new(size_t) {	\
		return type##_cache.allocObj();	\
	}	\
	void operator delete(void *p) {	\
		type##_cache.freeObj(p);	\
	}

#define IMPLEMENT_SLAB(type, num)	\
	Cache type::type##_cache(sizeof(type), num);


#endif
