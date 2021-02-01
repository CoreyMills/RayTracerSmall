// [header]
// A very basic raytracer example.
// [/header]
// [compile]
// c++ -o raytracer -O3 -Wall raytracer.cpp
// [/compile]
// [ignore]
// Copyright (C) 2012  www.scratchapixel.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// [/ignore]
#include <stdlib.h>
#include <cstdio>
#include <cmath>
#include <cassert>

#include <thread>
#include <mutex>
#include <ctime>
#include <chrono>
// Windows only
#include <sstream>
#include <string.h>

#include "HeapManager.h"
#include "SpherePool.h"
#include "Sphere.h"
#include "Commons.h"
#include "GlobalMemory.h"

#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else
// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif

float Mix(const float& a, const float& b, const float& mix)
{
	return b * mix + a * (1 - mix);
}

float Maxf(float val, float max)
{
	if (val > max)
		val = max;
	return val;
}
float Minf(float val, float min)
{
	if (val < min)
		val = min;
	return val;
}

// This variable controls the maximum recursion depth
#define MAX_RAY_DEPTH 5

std::mutex gMutex;
// Recommended Testing Resolution
//const unsigned int gWidth = 640, gHeight = 480;
// Recommended Production Resolution
const unsigned gWidth = 1920, gHeight = 1080;

//[comment]
// This is the main trace function. It takes a ray as argument (defined by its origin
// and direction). We test if this ray intersects any of the geometry in the scene.
// If the ray intersects an object, we compute the intersection point, the normal
// at the intersection point, and shade this point using this information.
// Shading depends on the surface property (is it transparent, reflective, diffuse).
// The function returns a color for the ray. If the ray intersects an object that
// is the color of the object at the intersection point, otherwise it returns
// the background color.
//[/comment]
Vec3f Trace(
	const Vec3f &rayorig, const Vec3f &raydir,
	Sphere** spheres, const unsigned int allocatedNum, const int &depth)
{
	//if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
	float tnear = INFINITY;
	const Sphere* sphere = NULL;

	// find intersection of this ray with the sphere in the scene
	for (unsigned i = 0; i < allocatedNum; ++i)
	{
		float t0 = INFINITY, t1 = INFINITY;
		if (spheres[i]->intersect(rayorig, raydir, t0, t1))
		{
			if (t0 < 0) t0 = t1;
			if (t0 < tnear) 
			{
				tnear = t0;
				sphere = spheres[i];
			}
		}
	}

	// if there's no intersection return black or background color
	if (!sphere) return Vec3f(2);
	Vec3f surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray
	Vec3f phit = rayorig + raydir * tnear; // point of intersection
	Vec3f nhit = phit - sphere->center; // normal at the intersection point
	nhit.normalize(); // normalize normal direction
					  // If the normal and the view direction are not opposite to each other
					  // reverse the normal direction. That also means we are inside the sphere so set
					  // the inside bool to true. Finally reverse the sign of IdotN which we want
					  // positive.
	float bias = 1e-4; // add some bias to the point from which we will be tracing
	bool inside = false;
	if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true;
	if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH) 
	{
		float facingratio = -raydir.dot(nhit);
		// change the mix value to tweak the effect
		float fresneleffect = Mix(pow(1 - facingratio, 3), 1, 0.1);
		// compute reflection direction (not need to normalize because all vectors
		// are already normalized)
		Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit);
		refldir.normalize();
		Vec3f reflection = Trace(phit + nhit * bias, refldir, spheres, allocatedNum, depth + 1);
		Vec3f refraction = 0;
		// if the sphere is also transparent compute refraction ray (transmission)
		if (sphere->transparency) 
		{
			float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface?
			float cosi = -nhit.dot(raydir);
			float k = 1 - eta * eta * (1 - cosi * cosi);
			Vec3f refrdir = raydir * eta + nhit * (eta *  cosi - sqrt(k));
			refrdir.normalize();
			refraction = Trace(phit - nhit * bias, refrdir, spheres, allocatedNum, depth + 1);
		}
		// the result is a mix of reflection and refraction (if the sphere is transparent)
		surfaceColor = (reflection * fresneleffect +
			refraction * (1 - fresneleffect) * sphere->transparency) * sphere->surfaceColor;
	}
	else 
	{
		// it's a diffuse object, no need to raytrace any further
		for (unsigned i = 0; i < allocatedNum; ++i)
		{
			if (spheres[i]->emissionColor.x > 0)
			{
				// this is a light
				Vec3f transmission = 1;
				Vec3f lightDirection = spheres[i]->center - phit;
				lightDirection.normalize();
				for (unsigned j = 0; j < allocatedNum; ++j)
				{
					if (i != j) 
					{
						float t0, t1;
						if (spheres[j]->intersect(phit + nhit * bias, lightDirection, t0, t1))
						{
							transmission = 0;
							break;
						}
					}
				}
				surfaceColor += sphere->surfaceColor * transmission *
					std::max(float(0), nhit.dot(lightDirection)) * spheres[i]->emissionColor;
			}
		}
	}
	
	return surfaceColor + sphere->emissionColor;
}

Animation* GetAnimInput(int id)
{
	Animation* animation = new Animation();

	std::cout << "Select what you want to change for sphere " << id << ":" << "\n" <<
		"0. Position" << "\n" <<
		"1. Color" << "\n" <<
		"2. Radius" << "\n" <<
		"3. No Animation" << std::endl;

	int type;
	Vec3f v;
	std::cin >> type;
	switch (type)
	{
	case 0:
		std::cout << "Please input how much you want the sphere to move for each second." << "\n" <<
			"Limit each axis input between 1.0 & -1.0" << std::endl;
		std::cin >> v.x;
		std::cin >> v.y;
		std::cin >> v.z;
		std::cin.clear();

		animation->aType = (AnimationType)type;
		animation->changeTo = v / 30.0f;
		break;
	case 1:
		std::cout << "Please input how much you want the sphere color to change by each second." << "\n" <<
			"Limit each axis input between 0.3 & -0.3" << std::endl;
		std::cin >> v.x;
		std::cin >> v.y;
		std::cin >> v.z;
		std::cin.clear();

		animation->aType = (AnimationType)type;
		animation->changeTo = v / 30.0f;
		break;
	case 2:
		std::cout << "Please input how much you want the sphere' radius to change by each second." << "\n" <<
			"Limit each axis input between 30.0 & -30.0" << std::endl;
		std::cin >> v.x;
		std::cin.clear();

		animation->aType = (AnimationType)type;
		animation->changeTo = v / 30.0f;
		break;
	default:
		std::cout << "No Animation" << std::endl;
	}
	return animation;
}

Animation* GetRandomAnim()
{
	Animation* animation = new Animation();
	int randAnim = rand() % 4;
	switch (randAnim)
	{
	case 0:
		animation->aType = AnimationType::Position;
		animation->changeTo = Vec3f((float)(rand() % 60 - 30) / 100.0f,
			(float)((rand() % 200) - 100) / 100.0f, (float)((rand() % 200) - 100) / 100.0f) / 30.0f;
		break;
	case 1:
		animation->aType = AnimationType::Colour;
		animation->changeTo = Vec3f((float)(rand() % 100) / 100.0f,
			(float)(rand() % 100) / 100.0f, (float)(rand() % 100) / 100.0f) / 30.0f;
		break;
	case 2:
		animation->aType = AnimationType::Radius;
		animation->changeTo = Vec3f((float)(rand() % 60) - 20, 0, 0) / 30.0f;
		break;
	}
	return animation;
}

//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
//[/comment]
void Render(Sphere** spheres, const unsigned int allocatedNum, int iteration)
{
	Vec3f* image = new Vec3f[gWidth * gHeight], * pixel = image;
	float invWidth = 1 / float(gWidth), invHeight = 1 / float(gHeight);
	float fov = 30, aspectratio = gWidth / float(gHeight);
	float angle = tan(M_PI * 0.5 * fov / 180.);
	
	// Trace rays
	for (unsigned int y = 0; y < gHeight; ++y)
	{
		for (unsigned int x = 0; x < gWidth; ++x, ++pixel)
		{
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();
			*pixel = Trace(Vec3f(0), raydir, spheres, allocatedNum, 0);
		}
	}

	// Save result to a PPM image (keep these flags if you compile under Windows)
	std::stringstream ss;
	ss << "./video/spheres" << iteration << ".ppm";
	std::string tempString = ss.str();
	char* filename = (char*)tempString.c_str();

	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
	ofs << "P6\n" << gWidth << " " << gHeight << "\n255\n";
	for (unsigned i = 0; i < gWidth * gHeight; ++i) {
		ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
			(unsigned char)(std::min(float(1), image[i].y) * 255) <<
			(unsigned char)(std::min(float(1), image[i].z) * 255);
	}
	ofs.close();
	delete[] image;
}

void RenderScreenQuad(unsigned int startHeight, unsigned int endheight, Vec3f* pixel,
	Sphere** spheres, const unsigned int allocatedNum, 
	float invWidth, float invHeight, float aspectRatio, float angle)
{
	for (unsigned int y = startHeight; y < endheight; ++y)
	{
		for (unsigned int x = 0; x < gWidth; ++x, ++pixel)
		{
			//if(y == 0 && x == 0)
			//	std::cout << "Quad: " << pixel << std::endl;

			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectRatio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();

			*pixel = Trace(Vec3f(0), raydir, spheres, allocatedNum, 0);
		}
	}
}

void RenderThreaded(Sphere** spheres, const unsigned int allocatedNum, int iteration)
{
	Vec3f* image = new Vec3f[gWidth * gHeight], * pixel = image;
	float invWidth = 1 / float(gWidth), invHeight = 1 / float(gHeight);
	float fov = 30, aspectratio = gWidth / float(gHeight);
	float angle = tan(M_PI * 0.5 * fov / 180.);

	std::thread t[4];
	int quadHeight = gHeight / 4;
	// Trace rays
	for (unsigned int i = 0; i < 4; i++)
	{
		int pixelMoveBy = i * (quadHeight * gWidth);
		t[i] = std::thread(RenderScreenQuad, i * quadHeight, (i + 1) * quadHeight, 
			pixel + pixelMoveBy, spheres, allocatedNum,
			invWidth, invHeight, aspectratio, angle);
	}
	for (unsigned int i = 0; i < 4; i++)
	{
		t[i].join();
	}

	// Save result to a PPM image (keep these flags if you compile under Windows)
	std::stringstream ss;
	ss << "./video/spheres" << iteration << ".ppm";
	std::string tempString = ss.str();
	char* filename = (char*)tempString.c_str();

	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
	ofs << "P6\n" << gWidth << " " << gHeight << "\n255\n";
	for (unsigned i = 0; i < gWidth * gHeight; ++i) {
		ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
			(unsigned char)(std::min(float(1), image[i].y) * 255) <<
			(unsigned char)(std::min(float(1), image[i].z) * 255);
	}
	ofs.close();
	delete[] image;
}

void BasicRender(Sphere** spheres, const unsigned int allocatedNum)
{
	Render(spheres, allocatedNum, 0);
	std::cout << "Rendered and saved spheres0.ppm" << std::endl;
}

void SimpleShrinking(Sphere** spheres, const unsigned int allocatedNum)
{
	for (int i = 0; i < 4; i++)
	{
		switch (i)
		{
		case 1:
			spheres[1]->SetRadius(3);
			break;
		case 2:
			spheres[1]->SetRadius(2);
			break;
		case 3:
			spheres[1]->SetRadius(1);
			break;
		}
		
		Render(spheres, allocatedNum, i);
		std::cout << "Rendered and saved spheres" << i << ".ppm" << std::endl;
	}
}

void SmoothScaling(Sphere** spheres, const unsigned int allocatedNum)
{
	for (int r = 0; r < 100; r++)
	{
		spheres[0]->SetRadius((float)r / 100);
		Render(spheres, allocatedNum, r);
		std::cout << "Rendered and saved spheres" << r << ".ppm" << std::endl;
	}
}

void AnimsApplied(std::thread* t, Sphere** spheres, 
	unsigned int allocatedNum, int count, int maxCount)
{
	/*if (count == 50)
	{
		SpherePool::GetInstance()->DeallocateSphere(0);
		allocatedNum = SpherePool::GetInstance()->GetAllocatedNum();
		
		Sphere** overrideSpheres = new Sphere * [allocatedNum];
		for (unsigned int i = 0; i < allocatedNum; i++)
		{
			overrideSpheres[i] = SpherePool::GetInstance()->GetSphere(i);
		}
		spheres = *overrideSpheres;
	}

	if (count == 30)
	{
		SpherePool::GetInstance()->AllocateSphere();
		allocatedNum = SpherePool::GetInstance()->GetAllocatedNum();
		
		Sphere** overrideSpheres = new Sphere * [allocatedNum];
		for (unsigned int i = 0; i < allocatedNum; i++)
		{
			overrideSpheres[i] = SpherePool::GetInstance()->GetSphere(i);
		}
		overrideSpheres[allocatedNum - 1]->anim = GetRandomAnim();
		spheres = *overrideSpheres;
	}*/

	for (unsigned int i = 0; i < allocatedNum; i++)
	{
		if (!spheres[i]->anim)
			continue;

		if (spheres[i]->anim->aType == AnimationType::Max)
			continue;

		//Vec3f changeAmount = spheres[i]->anim->changeTo / 30.0f;
		switch (spheres[i]->anim->aType)
		{
		case AnimationType::Position:
			spheres[i]->SetPosition(spheres[i]->center + 
				spheres[i]->anim->changeTo);
			break;
		case AnimationType::Colour:
			spheres[i]->SetSurfaceColor((spheres[i]->surfaceColor + 
				spheres[i]->anim->changeTo).MaxVec(1.0f));
			break;
		case AnimationType::Radius:
			spheres[i]->SetRadius(Minf(spheres[i]->radius + 
				(spheres[i]->anim->changeTo.x / 100), 0.1f));
			break;
		}
	}

	Sphere** s = new Sphere*[allocatedNum];
	for (unsigned int i = 0; i < allocatedNum; i++)
	{
		s[i] = new Sphere(*spheres[i]);
	}

	t[count] = std::thread(Render, s, allocatedNum, count);
	//t[count] = std::thread(RenderThreaded, s, allocatedNum, count);
	std::cout << "Rendered and saved spheres" << count << ".ppm" << std::endl;
	
	count++;
	if(count < maxCount)
		AnimsApplied(t, spheres, allocatedNum, count, maxCount);
	t[count - 1].join();

	for (unsigned int i = 0; i < allocatedNum; i++)
	{
		delete s[i];
		s[i] = nullptr;
	}
	delete[] s;
	s = nullptr;
}

//[comment]
// In the main function, we will create the scene which is composed of 5 spheres
// and 1 light (which is also a sphere). Then, once the scene description is complete
// we render that scene, by calling the render() function.
//[/comment]
int main(int argc, char** argv)
{
	srand(time(NULL));

	HeapManager::GetInstance()->Init();

	const int maxImgCount = 100; //same number of threads as images
	std::thread t[maxImgCount];
	std::chrono::time_point<std::chrono::system_clock> start;
	std::chrono::time_point<std::chrono::system_clock> end;

	unsigned int allocated = 0;
	bool chosen = false;
	while (!chosen)
	{
		std::cout << "Spheres allocated: " << allocated << std::endl;
		std::cout << "Please choose from the options:" << "\n" <<
			"1. Add Sphere" << "\n" <<
			"2. Remove Sphere" << "\n" <<
			"3. Done" << std::endl;

		int input;
		std::cin >> input;
		switch (input)
		{
		case 1:
			if (allocated == POOL_SIZE)
				std::cout << "Sphere array full" << std::endl;
			else
			{
				SpherePool::GetInstance()->AllocateSphere();
				allocated++;
			}
			break;
		case 2:
			SpherePool::GetInstance()->DeallocateSphere(allocated - 1);
			allocated = SpherePool::GetInstance()->GetAllocatedNum();
			break;
		case 3:
			chosen = true;
			break;
		}
	}

	Sphere** spheres = new Sphere * [allocated];
	for (unsigned int i = 0; i < allocated; i++)
	{
		spheres[i] = SpherePool::GetInstance()->GetSphere(i);
	}

	//HeapManager::GetInstance()->WalkTheHeap((Header*)(spheres - sizeof(Header)));

	std::cout << "Choose your output type:" << "\n" <<
		"1. Basic Render" << "\n" <<
		"2. Simple Shrinking" << "\n" <<
		"3. SmoothScaling" << "\n" <<
		"4. Use Animations" << std::endl;

	int renderType;
	std::cin >> renderType;
	switch (renderType)
	{
	case 1:
		std::cout << "Basic" << std::endl;
		BasicRender(spheres, allocated);
		break;
	case 2:
		std::cout << "Simple" << std::endl;
		SimpleShrinking(spheres, allocated);
		break;
	case 3:
		std::cout << "Smooth" << std::endl;
		SmoothScaling(spheres, allocated);
		break;
	case 4:
		std::cout << "Do you want random animations to be appllied:" << "\n" <<
			"1. Yes" << "\n" <<
			"2. No" << std::endl;
		int randAnim;
		std::cin >> randAnim;
		if (randAnim == 1)
		{
			for (unsigned int i = 0; i < allocated; i++)
			{
				if (spheres[i]->allocated)
					spheres[i]->anim = GetRandomAnim();
			}
		}
		else
		{
			for (unsigned int i = 0; i < allocated; i++)
			{
				if (spheres[i]->allocated)
					spheres[i]->anim = GetAnimInput(i);
			}
		}

		std::cout << "Chrono Start-" << std::endl;
		start = std::chrono::system_clock::now();
		AnimsApplied(t, spheres, allocated, 0, maxImgCount);
		end = std::chrono::system_clock::now();

		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << "Chrono End-" << std::endl;
		std::cout << "Time taken = " << elapsed.count() << std::endl;
		break;
	}

	system("ffmpeg -y -r 60 -f image2 -s 1920*1080 -i video/spheres%d.ppm -vcodec libx264 -crf 25 -pix_fmt yuv420p video/RaytracingOutput.mp4");

	return 0;
}