#pragma once

#include <limits>
#include <cmath>

#include "Functor.h"

namespace Nebula {
	template<typename genType>
	genType log2(genType Value)
	{
		return std::log(Value) * static_cast<genType>(1.4426950408889634073599246810019);
	}

	template<length_t L, typename T>
	struct compute_log2
	{
		inline static vec<L, T> call(vec<L, T> const& v) {
			NB_ASSERT(std::numeric_limits<T>::is_iec559, "'log2' only accept floating-point inputs. Include <glm/gtc/integer.hpp> for integer inputs.");

			return functor1<vec, L, T, T>::call(log2, v);
		}
	};

	template<length_t L, typename T>
	struct compute_sqrt
	{
		inline static vec<L, T> call(vec<L, T> const& x) {
			return functor1<vec, L, T, T>::call(std::sqrt, x);
		}
	};

	template<length_t L, typename T>
	struct compute_inversesqrt
	{
		inline static vec<L, T> call(vec<L, T> const& x)
		{
			return static_cast<T>(1) / sqrt(x);
		}
	};

	template<length_t L>
	struct compute_inversesqrt<L, float>
	{
		inline static vec<L, float> call(vec<L, float> const& x)
		{
			vec<L, float> tmp(x);
			vec<L, float> xhalf(tmp * 0.5f);
			vec<L, uint32_t>* p = reinterpret_cast<vec<L, uint32_t>*>(const_cast<vec<L, float>*>(&x));
			vec<L, uint32_t> i = vec<L, uint32_t>(0x5f375a86) - (*p >> vec<L, uint32_t>(1));
			vec<L, float>* ptmp = reinterpret_cast<vec<L, float>*>(&i);
			tmp = *ptmp;
			tmp = tmp * (1.5f - xhalf * tmp * tmp);
			return tmp;
		}
	};

	// pow
	using std::pow;
	template<length_t L, typename T>
	inline vec<L, T> pow(vec<L, T> const& base, vec<L, T> const& exponent)
	{
		return functor2<vec, L, T>::call(pow, base, exponent);
	}

	// exp
	using std::exp;
	template<length_t L, typename T>
	inline vec<L, T> exp(vec<L, T> const& x)
	{
		return functor1<vec, L, T, T>::call(exp, x);
	}

	// log
	using std::log;
	template<length_t L, typename T>
	inline vec<L, T> log(vec<L, T> const& x)
	{
		return functor1<vec, L, T, T>::call(log, x);
	}

	//exp2, ln2 = 0.69314718055994530941723212145818f
	template<typename genType>
	inline genType exp2(genType x)
	{
		NB_ASSERT(std::numeric_limits<genType>::is_iec559, "'exp2' only accept floating-point inputs");

		return std::exp(static_cast<genType>(0.69314718055994530941723212145818) * x);
	}

	template<length_t L, typename T>
	inline vec<L, T> exp2(vec<L, T> const& x)
	{
		return FuncArg1<vec, L, T, T>::call(exp2, x);
	}

	// log2, ln2 = 0.69314718055994530941723212145818f
	template<length_t L, typename T>
	inline vec<L, T> log2(vec<L, T> const& x)
	{
		return compute_log2<L, T, std::numeric_limits<T>::is_iec559>::call(x);
	}

	// sqrt
	using std::sqrt;
	template<length_t L, typename T>
	inline vec<L, T> sqrt(vec<L, T> const& x)
	{
		NB_ASSERT(std::numeric_limits<T>::is_iec559, "'sqrt' only accept floating-point inputs");
		return compute_sqrt<L, T>::call(x);
	}

	// inversesqrt
	template<typename genType>
	inline genType inversesqrt(genType x)
	{
		return static_cast<genType>(1) / sqrt(x);
	}

	template<length_t L, typename T>
	inline vec<L, T> inversesqrt(vec<L, T> const& x)
	{
		NB_ASSERT(std::numeric_limits<T>::is_iec559, "'inversesqrt' only accept floating-point inputs");
		return compute_inversesqrt<L, T>::call(x);
	}
}