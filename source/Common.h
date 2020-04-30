﻿#pragma once

#include <stdlib.h>

#include <algorithm>
#include <vector>
#include <map>
#include <string>

#define PI_CONST		3.14159265f
#define INV_PI_CONST	0.31830989f
#define TWO_PI_CONST	6.2831853f
#define ANG2RAD(x)		(x*PI_CONST/180.0f)
#define PI_ON_180		0.01745f
#define PI_UNDER_180	57.29578f		// 180_OVER_PI

using std::vector;
using std::map;
using std::string;

//
namespace LaplataRayTracer
{
	enum EPlaneOritention
	{
		PLANE_XOZ = 0,
		PALNE_XOY,
		PLANE_YOZ,
	};

}
