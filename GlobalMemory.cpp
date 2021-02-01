#include "GlobalMemory.h"

const bool gUseDoubleLinkedList = false;

void operator delete(void* pMem)
{
	//std::cout << "delete called" << std::endl;

	Header* pHeader = (Header*)((char*)pMem - sizeof(Header));
	Footer* pFooter = (Footer*)((char*)pMem + pHeader->dataSize);

	//std::cout << pHeader->heap->allocatedSize << std::endl;
	pHeader->heap->Decrease(pHeader->dataSize + sizeof(pFooter) + sizeof(pHeader));
	
	if (gUseDoubleLinkedList)
	{
		if (pHeader->pPrev != nullptr) //is there a previous
		{
			if (pHeader->pNext != nullptr)
			{
				pHeader->pPrev->pNext = pHeader->pNext;
				pHeader->pNext->pPrev = pHeader->pPrev;
			}
			else
			{
				pHeader->pPrev->pNext = nullptr;
				pHeader->heap->pLastAssigned = pHeader->pPrev;
			}
		}
		else
		{
			if (pHeader->pNext != nullptr)
				pHeader->pNext->pPrev = nullptr;
			else
				pHeader->heap->pLastAssigned = nullptr;
		}
	}
	
	//std::cout << pHeader->heap->allocatedSize << std::endl;
	//std::cout << std::endl;
	free(pHeader);
}

void* operator new(size_t size)
{
	//std::cout << "new called" << std::endl;

	size_t requestedBytes = size + sizeof(Header) + sizeof(Footer);
	char* pMem = (char*)malloc(requestedBytes);
	Header* pHeader = (Header*)pMem;
	pHeader->Init();

	void* pStartMemBlock = pMem + sizeof(Header);

	pHeader->dataSize = size;
	pHeader->heap = HeapManager::GetInstance()->GetDefaultHeap();

	void* pFooterAddress = pMem + sizeof(Header) + size;
	Footer* pFooter = (Footer*)pFooterAddress;
	pFooter->Init();

	//std::cout << pHeader->heap->allocatedSize << std::endl;
	pHeader->heap->Increase(pHeader->dataSize + sizeof(pFooter) + sizeof(pHeader));
	if (gUseDoubleLinkedList)
	{
		if (pHeader->heap->pLastAssigned != nullptr)
		{
			pHeader->heap->pLastAssigned->pNext = pHeader;
			pHeader->pPrev = pHeader->heap->pLastAssigned;
		}
		pHeader->heap->pLastAssigned = pHeader;
	}
	//std::cout << pHeader->heap->allocatedSize << std::endl;

	return pStartMemBlock;
}

void* operator new(size_t size, Heap* heap)
{
	//std::cout << "special new called" << std::endl;

	size_t requestedBytes = size + sizeof(Header) + sizeof(Footer);
	char* pMem = (char*)malloc(requestedBytes);
	Header* pHeader = (Header*)pMem;
	pHeader->Init();

	void* pStartMemBlock = pMem + sizeof(Header);

	pHeader->dataSize = size;
	pHeader->heap = heap;

	void* pFooterAddress = pMem + sizeof(Header) + size;
	Footer* pFooter = (Footer*)pFooterAddress;
	pFooter->Init();

	//std::cout << pHeader->heap->allocatedSize << std::endl;
	pHeader->heap->Increase(pHeader->dataSize + sizeof(pFooter) + sizeof(pHeader));
	if (gUseDoubleLinkedList)
	{
		if (pHeader->heap->pLastAssigned != nullptr)
		{
			pHeader->heap->pLastAssigned->pNext = pHeader;
			pHeader->pPrev = pHeader->heap->pLastAssigned;
		}
		pHeader->heap->pLastAssigned = pHeader;
	}
	//std::cout << pHeader->heap->allocatedSize << std::endl;

	return pStartMemBlock;
}