#pragma once

#include "../basic.h"

namespace Nebula {
	template <typename T>
	struct vec<4, T> {
		union { T x, r, s; };
		union { T y, g, t; };
		union { T z, b, p; };
		union { T w, a, q; };

		// -- Component accesses --

		static constexpr length_t length() { return 2; }

		inline constexpr T& operator[](length_t i) {
			switch (i) {
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			case 3:
				return w;
			}

			NB_ASSERT(false, "Invalid Value Given!");
			return x;
		}

		inline constexpr const T& operator[](length_t i) const {
			switch (i) {
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			case 3:
				return w;
			}

			NB_ASSERT(false, "Invalid Value Given!");
			return x;
		}

		// -- Implicit Constructors --

		constexpr vec() : x(0), y(0), z(0), w(0) { }
		constexpr vec(vec const& v) : x(v.x), y(v.y), z(v.z), w(v.w) { }
		constexpr vec(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) { }


		// -- Explicit Constructors --
		constexpr explicit vec(T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) { }

		template <typename A, typename B, typename C, typename D>
		inline explicit constexpr vec(const vec<1, A>& _x, vec<1, B> const& _y, vec<1, C> const& _z, const vec<1, D>& _w) :
			x(static_cast<T>(_x.x)),
			y(static_cast<T>(_y.x)),
			z(static_cast<T>(_z.x)),
			w(static_cast<T>(_w)) { }

		template <typename A, typename B, typename C>
		inline explicit constexpr vec(const vec<2, A>& _v, vec<1, B> const& _z, const vec<1, C>& _w) :
			x(static_cast<T>(_v.x)),
			y(static_cast<T>(_v.y)),
			z(static_cast<T>(_z.x)),
			w(static_cast<T>(_w.x)) { }

		template <typename A, typename B, typename C>
		inline explicit constexpr vec(const vec<2, A>& _v, B const& _z, C const& _w) :
			x(static_cast<T>(_v.x)),
			y(static_cast<T>(_v.y)),
			z(static_cast<T>(_z)),
			w(static_cast<T>(_w)) { }

		template <typename A, typename B>
		inline explicit constexpr vec(const vec<2, A>& _v1, vec<2, B> const& _v2) :
			x(static_cast<T>(_v1.x)),
			y(static_cast<T>(_v1.y)),
			z(static_cast<T>(_v2.x)),
			w(static_cast<T>(_v2.y)) { }

		template <typename A, typename B>
		inline explicit constexpr vec(const vec<3, A>& _v, vec<1, B> _w):
			x(static_cast<T>(_v.x)),
			y(static_cast<T>(_v.y)),
			z(static_cast<T>(_v.z)),
			w(static_cast<T>(_w.x)) { }

		template <typename A, typename B>
		inline explicit constexpr vec(const vec<3, A>& _v, B _w):
			x(static_cast<T>(_v.x)),
			y(static_cast<T>(_v.y)),
			z(static_cast<T>(_v.z)),
			w(static_cast<T>(_w)) { }

		// -- Explicit Coversion Constructor --

		template<typename U>
		inline constexpr vec(vec<1, U> const& v) : x(static_cast<T>(v.x)), y(0), z(0), w(0) { }

		template<typename U>
		inline constexpr vec(vec<2, U> const& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(0), w(0) { }

		template<typename U>
		inline constexpr vec(vec<3, U> const& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z)), w(0) { }

		template<typename U>
		inline constexpr vec(vec<4, U> const& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z)), w(static_cast<T>(v.w)) { }

		inline vec<2, T> xy() { return vec<2, T>(this->x, this->y); }
		inline vec<3, T> xyz() { return vec<3, T>(this->x, this->y, this->z); }

		// -- Arithmetic operators --

		template<typename U>
		inline constexpr vec<4, T>& operator=(vec<4, U> const& v) {
			this->x = static_cast<T>(v.x);
			this->y = static_cast<T>(v.y);
			this->z = static_cast<T>(v.z);
			this->w = static_cast<T>(v.w);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator+=(U scalar) {
			this->x += static_cast<T>(scalar);
			this->y += static_cast<T>(scalar);
			this->z += static_cast<T>(scalar);
			this->w += static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator+=(vec<4, U> const& v) {
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.y);
			this->z += static_cast<T>(v.z);
			this->w += static_cast<T>(v.w);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator-=(U scalar) {
			this->x -= static_cast<T>(scalar);
			this->y -= static_cast<T>(scalar);
			this->z -= static_cast<T>(scalar);
			this->w -= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator-=(vec<4, U> const& v) {
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.y);
			this->z -= static_cast<T>(v.z);
			this->w -= static_cast<T>(v.w);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator*=(U scalar) {
			this->x *= static_cast<T>(scalar);
			this->y *= static_cast<T>(scalar);
			this->z *= static_cast<T>(scalar);
			this->w *= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator*=(vec<4, U> const& v) {
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.y);
			this->z *= static_cast<T>(v.z);
			this->w *= static_cast<T>(v.w);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator/=(U scalar) {
			this->x /= static_cast<T>(scalar);
			this->y /= static_cast<T>(scalar);
			this->z /= static_cast<T>(scalar);
			this->w /= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator/=(vec<4, U> const& v) {
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.y);
			this->z /= static_cast<T>(v.z);
			this->w /= static_cast<T>(v.w);
			return *this;
		}

		// -- Increment and decrement operators --

		inline constexpr vec<4, T>& operator++() {
			++this->x;
			++this->y;
			++this->z;
			++this->w;
			return *this;
		}

		inline constexpr vec<4, T>& operator--() {
			--this->x;
			--this->y;
			--this->z;
			--this->w;
			return *this;
		}

		inline constexpr vec<4, T> operator++(int) {
			vec<4, T> Result(*this);
			++* this;
			return Result;
		}

		inline constexpr vec<4, T> operator--(int) {
			vec<4, T> Result(*this);
			--* this;
			return Result;
		}

		// -- Bit Operators --

		template<typename U>
		inline constexpr vec<4, T>& operator%=(U scalar) {
			this->x %= static_cast<T>(scalar);
			this->y %= static_cast<T>(scalar);
			this->z %= static_cast<T>(scalar);
			this->w %= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator%=(vec<4, U> const& v) {
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.y);
			this->z %= static_cast<T>(v.z);
			this->w %= static_cast<T>(v.w);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator&=(U scalar) {
			this->x &= static_cast<T>(scalar);
			this->y &= static_cast<T>(scalar);
			this->z &= static_cast<T>(scalar);
			this->w &= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator&=(vec<4, U> const& v) {
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.y);
			this->z &= static_cast<T>(v.z);
			this->w &= static_cast<T>(v.w);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator|=(U scalar) {
			this->x |= static_cast<T>(scalar);
			this->y |= static_cast<T>(scalar);
			this->z |= static_cast<T>(scalar);
			this->w |= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator|=(vec<4, U> const& v) {
			this->x |= U(v.x);
			this->y |= U(v.y);
			this->z |= U(v.z);
			this->w |= U(v.w);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator^=(U scalar) {
			this->x ^= static_cast<T>(scalar);
			this->y ^= static_cast<T>(scalar);
			this->z ^= static_cast<T>(scalar);
			this->w ^= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator^=(vec<4, U> const& v) {
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.y);
			this->z ^= static_cast<T>(v.z);
			this->w ^= static_cast<T>(v.w);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator<<=(U scalar) {
			this->x <<= static_cast<T>(scalar);
			this->y <<= static_cast<T>(scalar);
			this->z <<= static_cast<T>(scalar);
			this->w <<= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator<<=(vec<4, U> const& v) {
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.y);
			this->z <<= static_cast<T>(v.z);
			this->w <<= static_cast<T>(v.w);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator>>=(U scalar) {
			this->x >>= static_cast<T>(scalar);
			this->y >>= static_cast<T>(scalar);
			this->z >>= static_cast<T>(scalar);
			this->w >>= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<4, T>& operator>>=(vec<4, U> const& v) {
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.y);
			this->z >>= static_cast<T>(v.z);
			this->w >>= static_cast<T>(v.w);
			return *this;
		}
	};

	// -- Operators --

	template<typename T>
	inline constexpr vec<4, T> operator+(vec<4, T> const& v) {
		return v;
	}

	template<typename T>
	inline constexpr vec<4, T> operator-(vec<4, T> const& v) {
		return vec<4, T>(-v.x, -v.y, -v.z, -v.w);
	}

	// -- Binary operators --

	//Operator +
	template<typename T>
	inline constexpr vec<4, T> operator+(vec<4, T> const& v, T scalar) {
		return vec<4, T>(v.x + scalar, v.y + scalar, v.z + scalar, v.w + scalar);
	}

	template<typename T>
	inline constexpr vec<4, T> operator+(T scalar, vec<4, T> const& v) {
		return vec<4, T>(scalar + v.x, scalar + v.y, scalar + v.z, scalar + v.w);
	}

	template<typename T>
	inline constexpr vec<4, T> operator+(const vec<4, T>& v1, const vec<4, T>& v2) {
		return vec<4, T>(v1) += v2;
	}

	//Operator -
	template<typename T>
	inline constexpr vec<4, T> operator-(vec<4, T> const& v, T scalar) {
		return vec<4, T>(v.x - scalar, v.y - scalar, v.z - scalar, v.w - scalar);
	}

	template<typename T>
	inline constexpr vec<4, T> operator-(T scalar, vec<4, T> const& v) {
		return vec<4, T>(scalar - v.x, scalar - v.y, scalar - v.z, scalar - v.w);
	}

	template<typename T>
	inline constexpr vec<4, T> operator-(vec<4, T> const& v1, vec<4, T> const& v2) {
		return vec<4, T>(v1) -= v2;
	}

	//Operator *
	template<typename T>
	inline constexpr vec<4, T> operator*(vec<4, T> const& v, T scalar) {
		return vec<4, T>(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
	}

	template<typename T>
	inline constexpr vec<4, T> operator*(T scalar, vec<4, T> const& v) {
		return vec<4, T>(scalar * v.x, scalar * v.y, scalar * v.z, scalar * v.w);
	}

	template<typename T>
	inline constexpr vec<4, T> operator*(vec<4, T> const& v1, vec<4, T> const& v2) {
		return vec<4, T>(v1) *= v2;
	}

	//Operator /
	template<typename T>
	inline constexpr vec<4, T> operator/(vec<4, T> const& v, T scalar) {
		return vec<4, T>(v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar);
	}

	template<typename T>
	inline constexpr vec<4, T> operator/(T scalar, vec<4, T> const& v) {
		return vec<4, T>(scalar / v.x, scalar / v.y, scalar / v.z, scalar / v.w);
	}

	template<typename T>
	inline constexpr vec<4, T> operator/(vec<4, T> const& v1, vec<4, T> const& v2) {
		return vec<4, T>(v1) /= v2;
	}

	//Operator %
	template<typename T>
	inline constexpr vec<4, T> operator%(vec<4, T> const& v, T scalar) {
		return vec<4, T>(v.x % scalar, v.y % scalar, v.z % scalar, v.w % scalar);
	}

	template<typename T>
	inline constexpr vec<4, T> operator%(T scalar, vec<4, T> const& v) {
		return vec<4, T>(scalar % v.x, scalar % v.y, scalar % v.z, scalar % v.w);
	}

	template<typename T>
	inline constexpr vec<4, T> operator%(vec<4, T> const& v1, vec<4, T> const& v2) {
		return vec<4, T>(v1) %= v2;
	}

	//Operator &
	template<typename T>
	inline constexpr vec<4, T> operator&(vec<4, T> const& v, T scalar) {
		return vec<4, T>(v.x & scalar, v.y & scalar, v.z & scalar, v.w & scalar);
	}

	template<typename T>
	inline constexpr vec<4, T> operator&(T scalar, vec<4, T> const& v) {
		return vec<4, T>(scalar & v.x, scalar & v.y, scalar & v.z, scalar & v.w);
	}

	template<typename T>
	inline constexpr vec<4, T> operator&(vec<4, T> const& v1, vec<4, T> const& v2) {
		return vec<4, T>(v1) &= v2;
	}

	//Operator |
	template<typename T>
	inline constexpr vec<4, T> operator|(vec<4, T> const& v, T scalar) {
		return vec<4, T>(v.x | scalar, v.y | scalar, v.z | scalar, v.w | scalar);
	}

	template<typename T>
	inline constexpr vec<4, T> operator|(T scalar, vec<4, T> const& v) {
		return vec<4, T>(scalar | v.x, scalar | v.y, scalar | v.z, scalar | v.w);
	}

	template<typename T>
	inline constexpr vec<4, T> operator|(vec<4, T> const& v1, vec<4, T> const& v2) {
		return vec<4, T>(v1) |= v2;
	}

	//Operator ^
	template<typename T>
	inline constexpr vec<4, T> operator^(vec<4, T> const& v, T scalar) {
		return vec<4, T>(v.x ^ scalar, v.y ^ scalar, v.z ^ scalar, v.w ^ scalar);
	}

	template<typename T>
	inline constexpr vec<4, T> operator^(T scalar, vec<4, T> const& v) {
		return vec<4, T>(scalar ^ v.x, scalar ^ v.y, scalar ^ v.z, scalar ^ v.w);
	}

	template<typename T>
	inline constexpr vec<4, T> operator^(vec<4, T> const& v1, vec<4, T> const& v2) {
		return vec<4, T>(v1) ^= v2;
	}

	//Operator <<
	template<typename T>
	inline constexpr vec<4, T> operator<<(vec<4, T> const& v, T scalar) {
		return vec<4, T>(v.x << scalar, v.y << scalar, v.z << scalar, v.w << scalar);
	}

	template<typename T>
	inline constexpr vec<4, T> operator<<(T scalar, vec<4, T> const& v) {
		return vec<4, T>(scalar << v.x, scalar << v.y, scalar << v.z, scalar << v.w);
	}

	template<typename T>
	inline constexpr vec<4, T> operator<<(vec<4, T> const& v1, vec<4, T> const& v2) {
		return vec<4, T>(v1) <<= v2;
	}

	//Operator >>
	template<typename T>
	inline constexpr vec<4, T> operator>>(vec<4, T> const& v, T scalar) {
		return vec<4, T>(v.x >> scalar, v.y >> scalar, v.z >> scalar, v.w >> scalar);
	}

	template<typename T>
	inline constexpr vec<4, T> operator>>(T scalar, vec<4, T> const& v) {
		return vec<4, T>(scalar >> v.x, scalar >> v.y, scalar >> v.z, scalar >> v.w);
	}

	template<typename T>
	inline constexpr vec<4, T> operator>>(vec<4, T> const& v1, vec<4, T> const& v2) {
		return vec<4, T>(v1) >>= v2;
	}

	//Operator ~
	template<typename T>
	inline constexpr vec<4, T> operator~(vec<4, T> const& v) {
		return vec<4, T>(~v.x, ~v.y, ~v.z, ~v.w);
	}

	// -- Boolean operators --

	template<typename T>
	inline constexpr bool  operator==(vec<4, T> const& v1, vec<4, T> const& v2) {
		return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
	}

	template<typename T>
	inline constexpr bool operator!=(vec<4, T> const& v1, vec<4, T> const& v2) {
		return !(v1 == v2);
	}
}