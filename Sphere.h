#ifndef SPHERE_H
#define SPHERE_H

#include "Commons.h"
#include "HeapManager.h"
#include "GlobalMemory.h"

class Sphere
{
public:
	Vec3f center;                           /// position of the sphere
	float radius, radius2;                  /// sphere radius and radius^2
	Vec3f surfaceColor, emissionColor;      /// surface color and emission (light)
	float transparency, reflection;         /// surface transparency and reflectivity
	bool allocated = false; //will not render
	int poolIndex = -1;
	Animation* anim = nullptr;

	Sphere();
	Sphere(const Vec3f& c, const float& r, const Vec3f& sc,
		const float& refl = 0, const float& transp = 0, const Vec3f& ec = 0);
	~Sphere();

	void SetRadius(const float& r);
	void SetPosition(const Vec3f& v) { this->center = v; }
	void SetSurfaceColor(const Vec3f& v) { this->surfaceColor = v; }
	void SetRender(bool val) { allocated = val; }

	// Compute a ray-sphere intersection using the geometric solution
	bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const;

	static void* operator new(size_t size);
	static void operator delete(void* pMem, size_t size);
};
#endif