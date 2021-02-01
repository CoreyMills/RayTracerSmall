#ifndef COMMONS_H
#define COMMONS_H

#include <fstream>
#include <iostream>
#include <algorithm>

template<typename T>
class Vec3
{
public:
	T x, y, z;
	Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
	Vec3(T xx) : x(xx), y(xx), z(xx) {}
	Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
	Vec3& normalize()
	{
		T nor2 = length2();
		if (nor2 > 0) {
			T invNor = 1 / sqrt(nor2);
			x *= invNor, y *= invNor, z *= invNor;
		}
		return *this;
	}
	Vec3<T> operator * (const T& f) const { return Vec3<T>(x * f, y * f, z * f); }
	Vec3<T> operator * (const Vec3<T>& v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
	T dot(const Vec3<T>& v) const { return x * v.x + y * v.y + z * v.z; }
	Vec3<T> operator - (const Vec3<T>& v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
	Vec3<T> operator + (const Vec3<T>& v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
	Vec3<T> operator / (const float f) const { return Vec3<T>(x / f, y / f, z / f); }
	Vec3<T>& operator += (const Vec3<T>& v) { x += v.x, y += v.y, z += v.z; return *this; }
	Vec3<T>& operator *= (const Vec3<T>& v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
	Vec3<T>& operator /= (const float f) { x /= f, y /= f, z /= f; return *this; }
	Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); }

	Vec3<T> MaxVec(const float f)
	{
		if (x > f) x = f;
		if (y > f) y = f;
		if (z > f) z = f;

		return *this;
	}
	Vec3<T> MinVec(const float f)
	{
		if (x < f) x = f;
		if (y < f) y = f;
		if (z < f) z = f;

		return *this;
	}

	T length2() const { return x * x + y * y + z * z; }
	T length() const { return sqrt(length2()); }
	friend std::ostream& operator << (std::ostream& os, const Vec3<T>& v)
	{
		os << "[" << v.x << " " << v.y << " " << v.z << "]";
		return os;
	}
};

typedef Vec3<float> Vec3f;

enum class AnimationType
{
	Position = 0,
	Colour,
	Radius,
	Max
};

struct Animation
{
	AnimationType aType;
	Vec3f changeTo; //if radius just use x value

	Animation()
	{
		aType = AnimationType::Max;
		changeTo = Vec3f();
	}

	Animation(AnimationType aType, Vec3f changeTo)
	{
		this->aType = aType;
		this->changeTo = changeTo;
	}
};

//Use for further expansion
/*struct Animation
{
	Vec3f changePos; 
	Vec3f changeColour;
	float radius;

	Animation()
	{
		changePos = Vec3f();
		changeColour = Vec3f();
		radius = 0;
	}

	Animation(Vec3f changePos, 
		Vec3f changeColour, float radius)
	{
		this->changePos = changePos;
		this->changePos = changeColour;
		this->radius = radius;
	}
};*/

struct Header;
struct Heap
{
	size_t allocatedSize;
	Header* pLastAssigned;

	void Init()
	{
		this->allocatedSize = 0;
		this->pLastAssigned = nullptr;
	}

	void Increase(size_t size) { allocatedSize += size; }
	void Decrease(size_t size) { allocatedSize -= size; }
};

struct Header
{
	const char* checkVal;
	size_t dataSize; //size of main data
	Heap* heap;
	Header* pNext;
	Header* pPrev;

	void Init()
	{
		checkVal = "0xDEADCODE";
		dataSize = 0;
		heap = nullptr;
		pNext = nullptr;
		pPrev = nullptr;
	}
};

struct Footer
{
	const char* checkVal;
	size_t reserved;

	void Init()
	{
		this->checkVal = "0xDEADFEET";
		reserved = 0;
	}
};
#endif