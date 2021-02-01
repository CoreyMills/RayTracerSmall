#ifndef GLOBALMEMORY_H
#define GLOBALMEMORY_H

#include "Commons.h"
#include "HeapManager.h"

void operator delete(void* pMem);
void* operator new(size_t size);
void* operator new(size_t size, Heap* heap);
#endif