#ifndef HEAPMANAGER_H
#define HEAPMANAGER_H

#include <vector>
#include "Commons.h"

class HeapManager
{
public:
	void Init();
	
	void PrintAllocations();
	
	bool WalkTheHeap(Header* h);

	static HeapManager* GetInstance();
	Heap* GetDefaultHeap() { return m_defaultHeap; }
	Heap* GetSphereHeap() { return m_sphereHeap; }

private:
	HeapManager();
	~HeapManager();

	static HeapManager* m_instance;

	Heap* m_defaultHeap;
	Heap* m_sphereHeap;
};
#endif