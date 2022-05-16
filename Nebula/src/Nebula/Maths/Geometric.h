#pragma once

#include "Basic.h"

namespace Nebula {
	template<length_t L, typename T>
	struct compute_length
	{
		inline static T call(vec<L, T> const& v)
		{
			return sqrt(dot(v, v));
		}
	};

	template<length_t L, typename T>
	struct compute_distance
	{
		inline static T call(vec<L, T> const& p0, vec<L, T> const& p1)
		{
			return length(p1 - p0);
		}
	};

	template<typename V, typename T>
	struct compute_dot {};

	template<typename T>
	struct compute_dot<vec<1, T>, T>
	{
		inline constexpr static T call(vec<1, T> const& a, vec<1, T> const& b)
		{
			return a.x * b.x;
		}
	};

	template<typename T>
	struct compute_dot<vec<2, T>, T>
	{
		inline constexpr static T call(vec<2, T> const& a, vec<2, T> const& b)
		{
			vec<2, T> tmp(a * b);
			return tmp.x + tmp.y;
		}
	};

	template<typename T>
	struct compute_dot<vec<3, T>, T>
	{
		inline constexpr static T call(vec<3, T> const& a, vec<3, T> const& b)
		{
			vec<3, T> tmp(a * b);
			return tmp.x + tmp.y + tmp.z;
		}
	};

	template<typename T>
	struct compute_dot<vec<4, T>, T>
	{
		inline constexpr static T call(vec<4, T> const& a, vec<4, T> const& b)
		{
			vec<4, T> tmp(a * b);
			return (tmp.x + tmp.y) + (tmp.z + tmp.w);
		}
	};

	template<typename T>
	struct compute_cross
	{
		inline constexpr static vec<3, T> call(vec<3, T> const& x, vec<3, T> const& y)
		{
			NB_ASSERT(std::numeric_limits<T>::is_iec559, "'cross' accepts only floating-point inputs");

			return vec<3, T>(
				x.y * y.z - y.y * x.z,
				x.z * y.x - y.z * x.x,
				x.x * y.y - y.x * x.y);
		}


		inline constexpr static float call(vec<2, T> const& a, vec<2, T> const& b)
		{
			NB_ASSERT(std::numeric_limits<T>::is_iec559, "'cross' accepts only floating-point inputs");
			return a.x * b.y - a.y * b.x;
		}

		inline constexpr static vec<2, T> call(float const& a, vec<2, T> const& v)
		{
			NB_ASSERT(std::numeric_limits<T>::is_iec559, "'cross' accepts only floating-point inputs");
			return vec<2, T>(-a * v.y, a* v.x);
		}
	};

	template<length_t L, typename T>
	struct compute_normalize
	{
		inline static vec<L, T> call(vec<L, T> const& v)
		{
			NB_ASSERT(std::numeric_limits<T>::is_iec559, "'normalize' accepts only floating-point inputs");

			return v * inversesqrt(dot(v, v));
		}
	};

	template<length_t L, typename T>
	struct compute_faceforward
	{
		inline static vec<L, T> call(vec<L, T> const& N, vec<L, T> const& I, vec<L, T> const& Nref)
		{
			NB_ASSERT(std::numeric_limits<T>::is_iec559, "'normalize' accepts only floating-point inputs");

			return dot(Nref, I) < static_cast<T>(0) ? N : -N;
		}
	};

	template<length_t L, typename T>
	struct compute_reflect
	{
		inline static vec<L, T> call(vec<L, T> const& I, vec<L, T> const& N)
		{
			return I - N * dot(N, I) * static_cast<T>(2);
		}
	};

	template<length_t L, typename T>
	struct compute_refract
	{
		inline static vec<L, T> call(vec<L, T> const& I, vec<L, T> const& N, T eta)
		{
			T const dotValue(dot(N, I));
			T const k(static_cast<T>(1) - eta * eta * (static_cast<T>(1) - dotValue * dotValue));
			vec<L, T> const Result =
				(k >= static_cast<T>(0)) ? (eta * I - (eta * dotValue + std::sqrt(k)) * N) : vec<L, T>(0);
			return Result;
		}
	};

	template<typename genType>
	inline genType length(genType x)
	{
		NB_ASSERT(std::numeric_limits<genType>::is_iec559, "'length' accepts only floating-point inputs");

		return abs(x);
	}

	template<length_t L, typename T>
	inline T length(vec<L, T> const& v)
	{
		NB_ASSERT(std::numeric_limits<T>::is_iec559, "'length' accepts only floating-point inputs");

		return compute_length<L, T>::call(v);
	}

	// distance
	template<typename genType>
	inline genType distance(genType const& p0, genType const& p1)
	{
		NB_ASSERT(std::numeric_limits<genType>::is_iec559, "'distance' accepts only floating-point inputs");

		return length(p1 - p0);
	}

	template<length_t L, typename T>
	inline T distance(vec<L, T> const& p0, vec<L, T> const& p1)
	{
		return compute_distance<L, T>::call(p0, p1);
	}

	// dot
	template<typename T>
	inline constexpr T dot(T x, T y)
	{
		NB_ASSERT(std::numeric_limits<T>::is_iec559, "'dot' accepts only floating-point inputs");
		return x * y;
	}

	template<length_t L, typename T>
	inline constexpr T dot(vec<L, T> const& x, vec<L, T> const& y)
	{
		NB_ASSERT(std::numeric_limits<T>::is_iec559, "'dot' accepts only floating-point inputs");
		return compute_dot<vec<L, T>, T>::call(x, y);
	}

	// cross
	template<typename T>
	inline constexpr vec<3, T> cross(vec<3, T> const& x, vec<3, T> const& y)
	{
		return compute_cross<T>::call(x, y);
	}

	template<typename T>
	inline constexpr float cross(vec<2, T> const& x, vec<2, T> const& y)
	{
		return compute_cross<T>::call(x, y);
	}

	template<typename T>
	inline constexpr vec<2, T> cross(float const& x, vec<2, T> const& y)
	{
		return compute_cross<T>::call(x, y);
	}

	// normalize
	template<typename genType>
	inline genType normalize(genType const& x)
	{
		NB_ASSERT(std::numeric_limits<genType>::is_iec559, "'normalize' accepts only floating-point inputs");

		return x < genType(0) ? genType(-1) : genType(1);
	}

	template<length_t L, typename T>
	inline vec<L, T> normalize(vec<L, T> const& x)
	{
		NB_ASSERT(std::numeric_limits<T>::is_iec559, "'normalize' accepts only floating-point inputs");

		return compute_normalize<L, T>::call(x);
	}

	// faceforward
	template<typename genType>
	inline genType faceforward(genType const& N, genType const& I, genType const& Nref)
	{
		return dot(Nref, I) < static_cast<genType>(0) ? N : -N;
	}

	template<length_t L, typename T>
	inline vec<L, T> faceforward(vec<L, T> const& N, vec<L, T> const& I, vec<L, T> const& Nref)
	{
		return compute_faceforward<L, T>::call(N, I, Nref);
	}

	// reflect
	template<typename genType>
	inline genType reflect(genType const& I, genType const& N)
	{
		return I - N * dot(N, I) * genType(2);
	}

	template<length_t L, typename T>
	inline vec<L, T> reflect(vec<L, T> const& I, vec<L, T> const& N)
	{
		return compute_reflect<L, T>::call(I, N);
	}

	// refract
	template<typename genType>
	inline genType refract(genType const& I, genType const& N, genType eta)
	{
		NB_ASSERT(std::numeric_limits<genType>::is_iec559, "'refract' accepts only floating-point inputs");
		genType const dotValue(dot(N, I));
		genType const k(static_cast<genType>(1) - eta * eta * (static_cast<genType>(1) - dotValue * dotValue));
		return (eta * I - (eta * dotValue + sqrt(k)) * N) * static_cast<genType>(k >= static_cast<genType>(0));
	}

	template<length_t L, typename T>
	inline vec<L, T> refract(vec<L, T> const& I, vec<L, T> const& N, T eta)
	{
		NB_ASSERT(std::numeric_limits<T>::is_iec559, "'refract' accepts only floating-point inputs");
		return compute_refract<L, T>::call(I, N, eta);
	}

	//Absoulute
	using std::abs;
	template<typename genType>
	inline genType abs(genType x) {
		return abs(x);
	}
}