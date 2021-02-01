#ifndef SPHEREPOOL_H
#define SPHEREPOOL_H

#include "GlobalMemory.h"
#include "Sphere.h"
#include "json.hpp"

constexpr size_t POOL_SIZE = 10;

class SpherePool
{
public:
	SpherePool();
	~SpherePool();

	void AllocateSphere();
	void DeallocateSphere(unsigned int index);

	Sphere* GetSphere(int index) { return m_pool[index]; }
	unsigned int GetAllocatedNum() { return m_numAllocated; }

	void ReadFromJson();
	void WriteToJson();

	static SpherePool* GetInstance();

private:
	static SpherePool* m_instance;

	Sphere* m_pool[POOL_SIZE];
	unsigned int m_numAllocated;
};
#endif