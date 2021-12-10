#pragma once

/*	
	This is an unfinished basic prototype of
	a math library based on GLM (linked as
	submodule). 

	At this moment, the library is barely
	working in the context of the Nebula
	Engine. Expansion of this Math
	Library will expanded to a similar
	point of GLM.

	The GLM submodule will be linked to git
	until this Math Library is mostly
	complete.
*/

//Vectors
#include "Vectors/Vec1.h"
#include "Vectors/Vec2.h"
#include "Vectors/Vec3.h"
#include "Vectors/Vec4.h"

//Matrices
#include "Matrices/Transformation.h"
#include "Matrices/Mat4.h"

//Calculations
#include "Exponential.h"
#include "Geometric.h"
#include "Trigonometric.h"

#include "value_ptr.h"

namespace Nebula {
	typedef vec<1, float> vec1;
	typedef vec<2, float> vec2;
	typedef vec<3, float> vec3;
	typedef vec<4, float> vec4;
	typedef mat<4, 4, float> mat4;
}