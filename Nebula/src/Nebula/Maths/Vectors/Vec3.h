#pragma once

#include "../basic.h"

namespace Nebula {
	template <typename T>
	struct vec<3, T> {
		union { T x, r, s; };
		union { T y, g, t; };
		union { T z, b, p; };

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
			}

			NB_ASSERT(false, "Invalid Value Given!");
			return x;
		}

		// -- Implicit Constructors --

		constexpr vec() : x(0), y(0), z(0) { }
		constexpr vec(vec<3, T> const& v) : x(v.x), y(v.y), z(v.z) { }
		constexpr vec(T _x, T _y, T _z) : x(_x), y(_y), z(_z) { }


		// -- Explicit Constructors --
		constexpr explicit vec(T scalar) : x(scalar), y(scalar), z(scalar) { }

		template <typename A, typename B, typename C>
		inline constexpr vec(const vec<1, A>& _x, vec<1, B> const& _y, vec<1, C> const& _z) :
			x(static_cast<T>(_x.x)),
			y(static_cast<T>(_y.x)),
			z(static_cast<T>(_z.x)) { }

		template <typename A, typename B>
		inline constexpr vec(const vec<2, A>& _v, vec<1, B> const& _z) :
			x(static_cast<T>(_v.x)),
			y(static_cast<T>(_v.y)),
			z(static_cast<T>(_z.x)) { }

		template <typename A, typename B>
		inline constexpr vec(const vec<2, A>& _v1, vec<2, B> const& _v2) :
			x(static_cast<T>(_v1.x)),
			y(static_cast<T>(_v1.y)),
			z(static_cast<T>(_v2.x)) { }

		template <typename A, typename B>
		inline constexpr vec(const vec<1, A>& _x, vec<2, B> const& _v) :
			x(static_cast<T>(_x.x)),
			y(static_cast<T>(_v.x)),
			z(static_cast<T>(_v.y)) { }

		template <typename A, typename B>
		inline constexpr vec(const vec<2, A>& _v, B _z) :
			x(static_cast<T>(_v.x)),
			y(static_cast<T>(_v.y)),
			z(static_cast<T>(_z)) { }

		template <typename A, typename B>
		inline constexpr vec(A _x, vec<2, B> const& _v) :
			x(static_cast<T>(_x)),
			y(static_cast<T>(_v.x)),
			z(static_cast<T>(_v.y)) { }

		// -- Explicit Coversion Constructor --

		template<typename U>
		inline constexpr vec(vec<1, U> const& v) : x(static_cast<T>(v.x)), y(0), z(0) { }

		template<typename U>
		inline constexpr vec(vec<2, U> const& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(0) { }

		template<typename U>
		inline constexpr vec(vec<3, U> const& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z)) { }
		
		template<typename U>
		inline constexpr vec(vec<4, U> const& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z)) { }

		inline vec<2, T> xy() { return vec<2, T>(this->x, this->y); }

		// -- Arithmetic operators --

		template<typename U>
		inline constexpr vec<3, T>& operator=(vec<3, U> const& v) {
			this->x = static_cast<T>(v.x);
			this->y = static_cast<T>(v.y);
			this->z = static_cast<T>(v.z);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator+=(U scalar) {
			this->x += static_cast<T>(scalar);
			this->y += static_cast<T>(scalar);
			this->z += static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator+=(vec<3, U> const& v) {
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.y);
			this->z += static_cast<T>(v.z);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator-=(U scalar) {
			this->x -= static_cast<T>(scalar);
			this->y -= static_cast<T>(scalar);
			this->z -= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator-=(vec<3, U> const& v) {
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.y);
			this->z -= static_cast<T>(v.z);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator*=(U scalar) {
			this->x *= static_cast<T>(scalar);
			this->y *= static_cast<T>(scalar);
			this->z *= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator*=(vec<3, U> const& v) {
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.y);
			this->z *= static_cast<T>(v.z);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator/=(U scalar) {
			this->x /= static_cast<T>(scalar);
			this->y /= static_cast<T>(scalar);
			this->z /= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator/=(vec<3, U> const& v) {
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.y);
			this->z /= static_cast<T>(v.z);
			return *this;
		}

		// -- Increment and decrement operators --

		inline constexpr vec<3, T>& operator++() {
			++this->x;
			++this->y;
			++this->z;
			return *this;
		}

		inline constexpr vec<3, T>& operator--() {
			--this->x;
			--this->y;
			--this->z;
			return *this;
		}

		inline constexpr vec<3, T> operator++(int) {
			vec<3, T> Result(*this);
			++* this;
			return Result;
		}

		inline constexpr vec<3, T> operator--(int) {
			vec<3, T> Result(*this);
			--* this;
			return Result;
		}

		// -- Bit Operators --

		template<typename U>
		inline constexpr vec<3, T>& operator%=(U scalar) {
			this->x %= static_cast<T>(scalar);
			this->y %= static_cast<T>(scalar);
			this->z %= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator%=(vec<3, U> const& v) {
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.y);
			this->z %= static_cast<T>(v.z);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator&=(U scalar) {
			this->x &= static_cast<T>(scalar);
			this->y &= static_cast<T>(scalar);
			this->z &= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator&=(vec<3, U> const& v) {
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.y);
			this->z &= static_cast<T>(v.z);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator|=(U scalar) {
			this->x |= static_cast<T>(scalar);
			this->y |= static_cast<T>(scalar);
			this->z |= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator|=(vec<3, U> const& v) {
			this->x |= U(v.x);
			this->y |= U(v.y);
			this->z |= U(v.z);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator^=(U scalar) {
			this->x ^= static_cast<T>(scalar);
			this->y ^= static_cast<T>(scalar);
			this->z ^= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator^=(vec<3, U> const& v) {
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.y);
			this->z ^= static_cast<T>(v.z);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator<<=(U scalar) {
			this->x <<= static_cast<T>(scalar);
			this->y <<= static_cast<T>(scalar);
			this->z <<= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator<<=(vec<3, U> const& v) {
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.y);
			this->z <<= static_cast<T>(v.z);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator>>=(U scalar) {
			this->x >>= static_cast<T>(scalar);
			this->y >>= static_cast<T>(scalar);
			this->z >>= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator>>=(vec<3, U> const& v) {
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.y);
			this->z >>= static_cast<T>(v.z);
			return *this;
		}
	};

	// -- Operators --

	template<typename T>
	inline constexpr vec<3, T> operator+(vec<3, T> const& v) {
		return v;
	}

	template<typename T>
	inline constexpr vec<3, T> operator-(vec<3, T> const& v) {
		return vec<3, T>(-v.x, -v.y, -v.z);
	}

	// -- Binary operators --

	//Operator +
	template<typename T>
	inline constexpr vec<3, T> operator+(vec<3, T> const& v, T scalar) {
		return vec<3, T>(v.x + scalar, v.y + scalar, v.z + scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator+(T scalar, vec<3, T> const& v) {
		return vec<3, T>(scalar + v.x, scalar + v.y, scalar + v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator+(vec<3, T> const& v1, vec<3, T> const& v2) {
		return vec<3, T>(
			v1.x + v2.x, 
			v1.y + v2.y, 
			v1.z + v2.z);
	}

	//Operator -
	template<typename T>
	inline constexpr vec<3, T> operator-(vec<3, T> const& v, T scalar) {
		return vec<3, T>(v.x - scalar, v.y - scalar, v.z - scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator-(T scalar, vec<3, T> const& v) {
		return vec<3, T>(scalar - v.x, scalar - v.y, scalar - v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator-(vec<3, T> const& v1, vec<3, T> const& v2) {
		return vec<3, T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	}

	//Operator *
	template<typename T>
	inline constexpr vec<3, T> operator*(vec<3, T> const& v, T scalar) {
		return vec<3, T>(v.x * scalar, v.y * scalar, v.z * scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator*(T scalar, vec<3, T> const& v) {
		return vec<3, T>(scalar * v.x, scalar * v.y, scalar * v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator*(vec<3, T> const& v1, vec<3, T> const& v2) {
		return vec<3, T>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
	}

	//Operator /
	template<typename T>
	inline constexpr vec<3, T> operator/(vec<3, T> const& v, T scalar) {
		return vec<3, T>(v.x / scalar, v.y / scalar, v.z / scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator/(T scalar, vec<3, T> const& v) {
		return vec<3, T>(scalar / v.x, scalar / v.y, scalar / v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator/(vec<3, T> const& v1, vec<3, T> const& v2) {
		return vec<3, T>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
	}

	//Operator %
	template<typename T>
	inline constexpr vec<3, T> operator%(vec<3, T> const& v, T scalar) {
		return vec<3, T>(v.x % scalar, v.y % scalar, v.z % scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator%(T scalar, vec<3, T> const& v) {
		return vec<3, T>(scalar % v.x, scalar % v.y, scalar % v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator%(vec<3, T> const& v1, vec<3, T> const& v2) {
		return vec<3, T>(v1.x % v2.x, v1.y % v2.y, v1.z % v2.z);
	}

	//Operator &
	template<typename T>
	inline constexpr vec<3, T> operator&(vec<3, T> const& v, T scalar) {
		return vec<3, T>(v.x & scalar, v.y & scalar, v.z & scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator&(T scalar, vec<3, T> const& v) {
		return vec<3, T>(scalar & v.x, scalar & v.y, scalar & v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator&(vec<3, T> const& v1, vec<3, T> const& v2) {
		return vec<3, T>(v1.x & v2.x, v1.y & v2.y, v1.z & v2.z);
	}

	//Operator |
	template<typename T>
	inline constexpr vec<3, T> operator|(vec<3, T> const& v, T scalar) {
		return vec<3, T>(v.x | scalar, v.y | scalar, v.z | scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator|(T scalar, vec<3, T> const& v) {
		return vec<3, T>(scalar | v.x, scalar | v.y, scalar | v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator|(vec<3, T> const& v1, vec<3, T> const& v2) {
		return vec<3, T>(v1.x | v2.x, v1.y | v2.y, v1.z | v2.z);
	}

	//Operator ^
	template<typename T>
	inline constexpr vec<3, T> operator^(vec<3, T> const& v, T scalar) {
		return vec<3, T>(v.x ^ scalar, v.y ^ scalar, v.z ^ scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator^(T scalar, vec<3, T> const& v) {
		return vec<3, T>(scalar ^ v.x, scalar ^ v.y, scalar ^ v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator^(vec<3, T> const& v1, vec<3, T> const& v2) {
		return vec<3, T>(v1.x ^ v2.x, v1.x ^ v2.x, v1.z ^ v2.z);
	}

	//Operator <<
	template<typename T>
	inline constexpr vec<3, T> operator<<(vec<3, T> const& v, T scalar) {
		return vec<3, T>(v.x << scalar, v.y << scalar, v.z << scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator<<(T scalar, vec<3, T> const& v) {
		return vec<3, T>(scalar << v.x, scalar << v.y, scalar << v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator<<(vec<3, T> const& v1, vec<3, T> const& v2) {
		return vec<3, T>(v1.x << v2.x, v1.x << v2.x, v1.z << v2.z);
	}

	//Operator >>
	template<typename T>
	inline constexpr vec<3, T> operator>>(vec<3, T> const& v, T scalar) {
		return vec<3, T>(v.x >> scalar, v.y >> scalar, v.z >> scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator>>(T scalar, vec<3, T> const& v) {
		return vec<3, T>(scalar >> v.x, scalar >> v.y, scalar >> v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator>>(vec<3, T> const& v1, vec<3, T> const& v2) {
		return vec<3, T>(v1.x >> v2.x, v1.y >> v2.y, v1.z >> v2.z);
	}

	//Operator ~
	template<typename T>
	inline constexpr vec<3, T> operator~(vec<3, T> const& v) {
		return vec<3, T>(~v.x, ~v.y, ~v.z);
	}

	// -- Boolean operators --

	template<typename T>
	inline constexpr bool  operator==(vec<3, T> const& v1, vec<3, T> const& v2) {
		return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
	}

	template<typename T>
	inline constexpr bool operator!=(vec<3, T> const& v1, vec<3, T> const& v2) {
		return !(v1 == v2);
	}
}