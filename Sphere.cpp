#include "Sphere.h"

Sphere::Sphere() :
	center(Vec3f()), radius(0), radius2(0), surfaceColor(Vec3f()),
	emissionColor(Vec3f()), transparency(0), reflection(0)
{
}

Sphere::Sphere(const Vec3f& c, const float& r, const Vec3f& sc,
	const float& refl, const float& transp, const Vec3f& ec) :
	center(c), radius(r), radius2(r* r), surfaceColor(sc),
	reflection(refl), transparency(transp), emissionColor(ec)
{
}

Sphere::~Sphere()
{
}

void Sphere::SetRadius(const float& r)
{
	this->radius = r;
	this->radius2 = r * r;
}

bool Sphere::intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const
{
	Vec3f l = center - rayorig;
	float tca = l.dot(raydir);
	if (tca < 0) return false;
	float d2 = l.dot(l) - tca * tca;
	if (d2 > radius2) return false;
	float thc = sqrt(radius2 - d2);
	t0 = tca - thc;
	t1 = tca + thc;

	return true;
}

void* Sphere::operator new(size_t size)
{
	//std::cout << "Class New" << std::endl;
	return ::operator new(size, HeapManager::GetInstance()->GetSphereHeap());
}

void Sphere::operator delete(void* pMem, size_t size)
{
	::operator delete(pMem);
}