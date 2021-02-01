#include "SpherePool.h"

using json = nlohmann::json;

namespace ns {
	void to_json(json& j, const Sphere& s) {
		j = json{ {"centerX", s.center.x}, {"centerY", s.center.y}, {"centerZ", s.center.z},
			{"radius", s.radius}, {"radius2", s.radius2},
			{"surfaceColorX", s.surfaceColor.x}, {"surfaceColorY", s.surfaceColor.y}, {"surfaceColorZ", s.surfaceColor.z},
			{"emissionColorX", s.emissionColor.x}, {"emissionColorY", s.emissionColor.y}, {"emissionColorZ", s.emissionColor.z},
			{"transparency", s.transparency}, {"reflection", s.reflection} };
	}

	void from_json(const json& j, Sphere& s) {
		j.at("centerX").get_to(s.center.x);
		j.at("centerY").get_to(s.center.y);
		j.at("centerZ").get_to(s.center.z);
		j.at("radius").get_to(s.radius);
		j.at("radius2").get_to(s.radius2);
		j.at("surfaceColorX").get_to(s.surfaceColor.x);
		j.at("surfaceColorY").get_to(s.surfaceColor.y);
		j.at("surfaceColorZ").get_to(s.surfaceColor.z);
		j.at("emissionColorX").get_to(s.emissionColor.x);
		j.at("emissionColorY").get_to(s.emissionColor.y);
		j.at("emissionColorZ").get_to(s.emissionColor.z);
		j.at("transparency").get_to(s.transparency);
		j.at("reflection").get_to(s.reflection);
	}
}

SpherePool* SpherePool::m_instance = 0;

SpherePool::SpherePool()
{
	m_numAllocated = 0;
	for (unsigned int i = 0; i < POOL_SIZE; i++)
		m_pool[i] = new Sphere();

	ReadFromJson();

	/*m_pool[0] = Sphere(Vec3f(-5.0, 1, -20), 2, Vec3f(0.20, 0.20, 0.20), 0, 0.0);
	m_pool[1] = Sphere(Vec3f(0.0, 0, -20), 0.1f, Vec3f(1.00, 0.32, 0.36), 1, 0.5);
	m_pool[2] = Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0);
	m_pool[3] = Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0);
	for (unsigned int i = 4; i < POOL_SIZE; i++)
	{
		m_pool[i] = Sphere(Vec3f((rand() % 11) - 5, (rand() % 4) - 2, (rand()% 16) - 35),
			fminf((float)(rand() % 21)/ 10, 0.1f), Vec3f((float)(rand() % 100) / 100, (float)(rand() % 100) / 100, (float)(rand() % 100) / 100),
			(float)(rand()% 100) / 100, (float)(rand() % 100) / 100);
	}
	WriteToJson();*/
}

SpherePool::~SpherePool()
{
	delete [] &m_pool;
	free(m_instance);
}

void SpherePool::AllocateSphere()
{
	for (unsigned int i = m_numAllocated; i < POOL_SIZE; i++)
	{
		if (!m_pool[i]->allocated)
		{
			m_pool[i]->allocated = true;
			m_numAllocated++;
			return;
		}
	}
}

void SpherePool::DeallocateSphere(unsigned int index)
{
	if (index < 0 || index >= POOL_SIZE ||
		index > m_numAllocated - 1)
		return;

	// if deallocating last sphere in the order of allocated spheres
	if (index == m_numAllocated - 1)
	{
		m_pool[index]->allocated = false;
		m_numAllocated--;
		return;
	}

	//if deallocating sphere from anywhere else in the order of allocated spheres
	m_pool[index]->allocated = false;
	Sphere* deallocatedS = m_pool[index];

	for (unsigned int i = index; i < m_numAllocated; i++)
	{
		//Sphere temp = m_pool[i];
		m_pool[i] = m_pool[i + 1];
		m_pool[i]->poolIndex = i;
		//m_pool[i + 1] = temp;
	}

	m_numAllocated--;
	m_pool[m_numAllocated] = deallocatedS;
}

void SpherePool::ReadFromJson()
{
	std::ifstream inFile("file.json");
	json j = json::array();

	inFile >> j;
	for (unsigned int i = 0; i < POOL_SIZE; i++)
	{
		ns::from_json(j[i], *m_pool[i]);
		m_pool[i]->poolIndex = i;
	}
	inFile.close();
}

void SpherePool::WriteToJson()
{
	std::ofstream outFile("file.json");
	json j = json::array();

	for (unsigned int i = 0; i < POOL_SIZE; i++)
	{
		ns::to_json(j[i], *m_pool[i]);
	}
	outFile << j << std::endl;
	outFile.close();
}

SpherePool* SpherePool::GetInstance()
{
	if (m_instance == 0)
		m_instance = new SpherePool();
	return m_instance;
}