#pragma once

#define PI 3.14159265358979323846

#include "Vectors/Vec1.h"
#include "Functor.h"

#include <cmath>

namespace Nebula {
	//radians
	template<typename genType>
	inline constexpr genType radians(genType degrees) {
		if (!std::numeric_limits<genType>::is_iec559) {
			NB_ASSERT(false, "'Radians' only accepts a floating-point number");
		}

		return degrees * static_cast<genType>(0.01745329251994329576923690768489);
	}

	template<length_t L, typename T>
	inline constexpr vec<L, T> radians(vec<L, T> const& v) {
		return FuncArg1<vec, L, T, T>::call(radians, v);
	}


	//degrees
	template<typename genType>
	inline constexpr genType degrees(genType radians) {
		if (!std::numeric_limits<genType>::is_iec559) {
			NB_ASSERT(false, "'Degrees' only accepts a floating-point number");
		}

		return radians * static_cast<genType>(57.295779513082320876798154814105);
	}

	template<length_t L, typename T>
	inline constexpr vec<L, T> degrees(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(degrees, v);
	}

	//Factorial (x!)
	inline int factorial(int x) {
		int f = 1;
		for (int i = 1; i <= x; i++)
			f *= i;

		return f;
	}

	// sin
	using std::sin;

	template<length_t L, typename T>
	inline vec<L, T> sin(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(sin, v);
	}

	// cos
	using std::cos;

	template<length_t L, typename T>
	inline vec<L, T> cos(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(cos, v);
	}

	// tan
	using std::tan;

	template<length_t L, typename T>
	inline vec<L, T> tan(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(tan, v);
	}

	// asin
	using std::asin;

	template<length_t L, typename T>
	inline vec<L, T> asin(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(asin, v);
	}

	// acos
	using std::acos;

	template<length_t L, typename T>
	inline vec<L, T> acos(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(acos, v);
	}

	// atan
	template<typename genType>
	inline genType atan(genType y, genType x)
	{
		NB_ASSERT(std::numeric_limits<genType>::is_iec559, "'atan' only accept floating-point input");

		return ::std::atan2(y, x);
	}

	template<length_t L, typename T>
	inline vec<L, T> atan(vec<L, T> const& a, vec<L, T> const& b)
	{
		return FuncArg2<vec, L, T>::call(::std::atan2, a, b);
	}

	using std::atan;

	template<length_t L, typename T>
	inline vec<L, T> atan(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(atan, v);
	}

	// sinh
	using std::sinh;

	template<length_t L, typename T>
	inline vec<L, T> sinh(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(sinh, v);
	}

	// cosh
	using std::cosh;

	template<length_t L, typename T>
	inline vec<L, T> cosh(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(cosh, v);
	}

	// tanh
	using std::tanh;

	template<length_t L, typename T>
	inline vec<L, T> tanh(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(tanh, v);
	}

	// asinh
	using std::asinh;

	template<length_t L, typename T>
	inline vec<L, T> asinh(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(asinh, v);
	}

	// acosh
	using std::acosh;

	template<length_t L, typename T>
	inline vec<L, T> acosh(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(acosh, v);
	}

	// atanh
	using std::atanh;

	template<length_t L, typename T>
	inline vec<L, T> atanh(vec<L, T> const& v)
	{
		return FuncArg1<vec, L, T, T>::call(atanh, v);
	}
}