#include "HeapManager.h"

HeapManager* HeapManager::m_instance = 0;

HeapManager::HeapManager()
{
}

HeapManager::~HeapManager()
{
	//free(m_defaultHeap);
	//free(m_sphereHeap);
	free(m_instance);
}

void HeapManager::Init()
{
	m_defaultHeap = (Heap*)malloc(sizeof(Heap));
	m_defaultHeap->Init();
	m_sphereHeap = (Heap*)malloc(sizeof(Heap));
	m_sphereHeap->Init();
}

void HeapManager::PrintAllocations()
{
	std::cout << "Default Heap-" << std::endl;
	std::cout << "Size: " << m_defaultHeap->allocatedSize << std::endl;
	
	std::cout << "Sphere Heap-" << std::endl;
	std::cout << "Size: " << m_sphereHeap->allocatedSize << std::endl;
}

//bool HeapManager::WalkTheHeap(Header* h)
//{
//	void* fAddress = h + sizeof(Header) + h->dataSize;
//	Footer* f = (Footer*)fAddress;
//	if (h->checkVal != "0xDEADCODE" || f->checkVal != "0xDEADFEET")
//		return false;
//	
//	if (h->pNext)
//		WalkTheHeap(h->pNext);
//	return true;
//}

HeapManager* HeapManager::GetInstance()
{
	if (m_instance == 0)
		m_instance = (HeapManager*)malloc(sizeof(HeapManager));
	return m_instance;
}