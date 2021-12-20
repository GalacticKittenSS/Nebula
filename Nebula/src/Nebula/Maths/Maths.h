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
#include "Matrices/Mat3.h"

//Quaternions
#include "Quaternions/Quaternion.h"

//Calculations
#include "Exponential.h"
#include "Geometric.h"
#include "Trigonometric.h"

#include "value_ptr.h"
#include "ToString.h"

namespace Nebula {
	typedef vec<1, float> vec1;
	typedef vec<2, float> vec2;
	typedef vec<3, float> vec3;
	typedef vec<4, float> vec4;
	typedef mat<4, 4, float> mat4;
	typedef mat<3, 3, float> mat3;
	typedef qua<float> quat;
	
	inline constexpr float epsilon() { return std::numeric_limits<float>::epsilon(); }
	inline constexpr float pi() { return 3.14159265358979323846264338327950288f; }

	inline bool epsilonEqual(float const& x, float const& y, float const& epsilon) { return abs(x - y) < epsilon; }
	inline bool epsilonNotEqual(float const& x, float const& y, float const& epsilon) { return abs(x - y) >= epsilon; }
	
	template<typename T>
	inline vec<3, T> scaleVec(vec<3, T> const& v, T desiredLength) {
		return v * desiredLength / length(v);
	}

	bool DecomposeTransform(mat4& transform, vec3& outTranslation, vec3& outRotation, vec3& outScale);

	template<typename OStream, length_t L, typename T>
	inline OStream& operator<<(OStream& os, const vec<L, T>& vector) {
		return os << to_string(vector);
	}

	template<typename OStream, length_t C, length_t R, typename T>
	inline OStream& operator<<(OStream& os, const mat<C, R, T>& matrix) {
		return os << to_string(matrix);
	}

	template<typename OStream, typename T>
	inline OStream& operator<<(OStream& os, qua<T> quaternion) {
		return os << to_string(quaternion);
	}
}