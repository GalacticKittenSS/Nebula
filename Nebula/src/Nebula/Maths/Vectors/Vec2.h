#pragma once

#include "../basic.h"

namespace Nebula {
	template <typename T>
	struct vec<2, T> {
		union { T x, r, s; };
		union { T y, g, t; };

		// -- Component accesses --

		static constexpr length_t Length() { return 2; }
		
		inline constexpr T& operator[](length_t i) {
			NB_ASSERT(i >= 0 && i < this->Length(), "'i' must be Less than 2 and more than 0");

			switch (i) {
			case 0:
				return x;
			case 1:
				return y;
			}
		}

		inline constexpr const T& operator[](length_t i) const {
			NB_ASSERT(i >= 0 && i < this->Length(), "'i' must be Less than 2 and more than 0");
			
			switch (i) {
			case 0:
				return x;
			case 1:
				return y;
			}
		}

		// -- Implicit Constructors --

		constexpr vec(): x(0), y(0) { }
		constexpr vec(vec<2, T> const& v): x(v.x), y(v.y) { }
		constexpr vec(T _x, T _y): x(_x), y(_y) { }


		// -- Explicit Constructors --
		constexpr explicit vec(T scalar): x(scalar), y(scalar) { }

		template <typename A, typename B>
		inline explicit constexpr vec(vec<1, A> const& _x, B _y) : x(static_cast<T>(_x.x)), y(static_cast<T>(_y)) { }
		
		template <typename A, typename B>
		inline explicit constexpr vec(A _x, vec<1, B> const& _y) : x(static_cast<T>(_x)), y(static_cast<T>(_y.x)) { }

		template <typename A, typename B>
		inline explicit constexpr vec(const vec<1, A>& _x, vec<1, B> const& _y) : x(static_cast<T>(_x.x)), y(static_cast<T>(_y.x)) { }

		// -- Coversion Constructor --

		template<typename U>
		inline constexpr vec(vec<1, U> const& v) : x(static_cast<T>(v.x)), y(0) { }

		template<typename U>
		inline constexpr vec(vec<2, U> const& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) { }
		
		template<typename U>
		inline constexpr vec(vec<3, U> const& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) { }
		
		template<typename U>
		inline constexpr vec(vec<4, U> const& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) { }
		
		// -- Arithmetic operators --
		
		template<typename U>
		inline constexpr vec<2, T>& operator=(vec<2, U> const& v) {
			this->x = static_cast<T>(v.x);
			this->y = static_cast<T>(v.y);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator+=(U scalar) {
			this->x += static_cast<T>(scalar);
			this->y += static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator+=(vec<2, U> const& v) {
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.y);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator-=(U scalar) {
			this->x -= static_cast<T>(scalar);
			this->y -= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator-=(vec<2, U> const& v) {
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.y);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator*=(U scalar) {
			this->x *= static_cast<T>(scalar);
			this->y *= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator*=(vec<2, U> const& v) {
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.y);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator/=(U scalar) {
			this->x /= static_cast<T>(scalar);
			this->y /= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator/=(vec<2, U> const& v) {
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.y);
			return *this;
		}

		// -- Increment and decrement operators --

		inline constexpr vec<2, T>& operator++() {
			++this->x;
			++this->y;
			return *this;
		}

		inline constexpr vec<2, T>& operator--() {
			--this->x;
			--this->y;
			return *this;
		}

		inline constexpr vec<2, T> operator++(int) {
			vec<2, T> Result(*this);
			++*this;
			return Result;
		}

		inline constexpr vec<2, T> operator--(int) {
			vec<2, T> Result(*this);
			--*this;
			return Result;
		}

		// -- Bit Operators --

		template<typename U>
		inline constexpr vec<2, T>& operator%=(U scalar) {
			this->x %= static_cast<T>(scalar);
			this->y %= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator%=(vec<1, U> const& v) {
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.y);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator&=(U scalar) {
			this->x &= static_cast<T>(scalar);
			this->y &= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator&=(vec<1, U> const& v) {
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.y);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator|=(U scalar) {
			this->x |= static_cast<T>(scalar);
			this->y |= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator|=(vec<1, U> const& v) {
			this->x |= U(v.x);
			this->y |= U(v.y);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator^=(U scalar) {
			this->x ^= static_cast<T>(scalar);
			this->y ^= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator^=(vec<1, U> const& v) {
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.y);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator<<=(U scalar) {
			this->x <<= static_cast<T>(scalar);
			this->y <<= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator<<=(vec<1, U> const& v) {
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.y);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator>>=(U scalar) {
			this->x >>= static_cast<T>(scalar);
			this->y >>= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<2, T>& operator>>=(vec<1, U> const& v) {
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.y);
			return *this;
		}


		inline float LenSqr() const {
			return x * x + y * y;
		}
	};
	
	// -- Operators --

	template<typename T>
	inline constexpr vec<2, T> operator+(vec<2, T> const& v) {
		return v;
	}

	template<typename T>
	inline constexpr vec<2, T> operator-(vec<2, T> const& v) {
		return vec<2, T>(-v.x, -v.y);
	}

	// -- Binary operators --

	//Operator +
	template<typename T>
	inline constexpr vec<2, T> operator+(vec<2, T> const& v, T scalar) {
		return vec<2, T>(v.x + scalar, v.y + scalar);
	}

	template<typename T>
	inline constexpr vec<2, T> operator+(T scalar, vec<2, T> const& v) {
		return vec<2, T>(scalar + v.x, scalar + v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator+(vec<2, T> const& v1, vec<2, T> const& v2) {
		return vec<2, T>(v1.x + v2.x, v1.x + v2.x);
	}

	//Operator -
	template<typename T>
	inline constexpr vec<2, T> operator-(vec<2, T> const& v, T scalar) {
		return vec<2, T>(v.x - scalar, v.y - scalar);
	}

	template<typename T>
	inline constexpr vec<2, T> operator-(T scalar, vec<2, T> const& v) {
		return vec<2, T>(scalar - v.x, scalar - v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator-(vec<2, T> const& v1, vec<2, T> const& v2) {
		return vec<2, T>(v1.x - v2.x, v1.y - v2.y);
	}

	//Operator *
	template<typename T>
	inline constexpr vec<2, T> operator*(vec<2, T> const& v, T scalar) {
		return vec<2, T>(v.x * scalar, v.y * scalar);
	}

	template<typename T>
	inline constexpr vec<2, T> operator*(T scalar, vec<2, T> const& v) {
		return vec<2, T>(scalar * v.x, scalar * v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator*(vec<2, T> const& v1, vec<2, T> const& v2) {
		return vec<2, T>(v1.x * v2.x, v1.y * v2.y);
	}

	//Operator /
	template<typename T>
	inline constexpr vec<2, T> operator/(vec<2, T> const& v, T scalar) {
		return vec<2, T>(v.x / scalar, v.y / scalar);
	}

	template<typename T>
	inline constexpr vec<2, T> operator/(T scalar, vec<2, T> const& v) {
		return vec<2, T>(scalar / v.x, scalar / v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator/(vec<2, T> const& v1, vec<2, T> const& v2) {
		return vec<2, T>(v1.x / v2.x, v1.y / v2.y);
	}

	//Operator %
	template<typename T>
	inline constexpr vec<2, T> operator%(vec<2, T> const& v, T scalar) {
		return vec<2, T>(v.x % scalar, v.y % scalar);
	}

	template<typename T>
	inline constexpr vec<2, T> operator%(T scalar, vec<2, T> const& v) {
		return vec<2, T>(scalar % v.x, scalar % v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator%(vec<2, T> const& v1, vec<2, T> const& v2) {
		return vec<2, T>(v1.x % v2.x, v1.y % v2.y);
	}

	//Operator &
	template<typename T>
	inline constexpr vec<2, T> operator&(vec<2, T> const& v, T scalar) {
		return vec<2, T>(v.x & scalar, v.y & scalar);
	}

	template<typename T>
	inline constexpr vec<2, T> operator&(T scalar, vec<2, T> const& v) {
		return vec<2, T>(scalar & v.x, scalar & v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator&(vec<2, T> const& v1, vec<2, T> const& v2) {
		return vec<2, T>(v1.x & v2.x, v1.y & v2.y);
	}

	//Operator |
	template<typename T>
	inline constexpr vec<2, T> operator|(vec<2, T> const& v, T scalar) {
		return vec<2, T>(v.x | scalar, v.y | scalar);
	}

	template<typename T>
	inline constexpr vec<2, T> operator|(T scalar, vec<2, T> const& v) {
		return vec<2, T>(scalar | v.x, scalar | v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator|(vec<2, T> const& v1, vec<2, T> const& v2) {
		return vec<2, T>(v1.x | v2.x, v1.y | v2.y);
	}

	//Operator ^
	template<typename T>
	inline constexpr vec<2, T> operator^(vec<2, T> const& v, T scalar) {
		return vec<2, T>(v.x ^ scalar, v.y ^ v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator^(T scalar, vec<2, T> const& v) {
		return vec<2, T>(scalar ^ v.x, scalar ^ v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator^(vec<2, T> const& v1, vec<2, T> const& v2) {
		return vec<2, T>(v1.x ^ v2.x, v1.y ^ v2.y);
	}

	//Operator <<
	template<typename T>
	inline constexpr vec<2, T> operator<<(vec<2, T> const& v, T scalar) {
		return vec<2, T>(v.x << scalar, v.y << scalar);
	}

	template<typename T>
	inline constexpr vec<2, T> operator<<(T scalar, vec<2, T> const& v) {
		return vec<2, T>(scalar << v.x, scalar << v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator<<(vec<2, T> const& v1, vec<2, T> const& v2) {
		return vec<2, T>(v1.x << v2.x, v1.y << v2.y);
	}

	//Operator >>
	template<typename T>
	inline constexpr vec<2, T> operator>>(vec<2, T> const& v, T scalar) {
		return vec<2, T>(v.x >> scalar, v.y >> scalar);
	}

	template<typename T>
	inline constexpr vec<2, T> operator>>(T scalar, vec<2, T> const& v) {
		return vec<2, T>(scalar >> v.x, scalar >> v.y);
	}

	template<typename T>
	inline constexpr vec<2, T> operator>>(vec<2, T> const& v1, vec<2, T> const& v2) {
		return vec<2, T>(v1.x >> v2.x, v1.y >> v2.y);
	}

	//Operator ~
	template<typename T>
	inline constexpr vec<2, T> operator~(vec<2, T> const& v) {
		return vec<2, T>(~v.x, ~v.y);
	}

	// -- Boolean operators --

	template<typename T>
	inline constexpr bool operator==(vec<2, T> const& v1, vec<2, T> const& v2) {
		return v1.x == v2.x && v1.y == v2.y;
	}

	template<typename T>
	inline constexpr bool operator!=(vec<2, T> const& v1, vec<2, T> const& v2) {
		return !(v1 == v2);
	}

	template<typename T>
	inline constexpr bool operator>=(vec<2, T> const& v1, vec<2, T> const& v2) {
		return v1.x >= v2.x || v1.y >= v2.y;
	}

	template<typename T>
	inline constexpr bool operator<=(vec<2, T> const& v1, vec<2, T> const& v2) {
		return v1.x <= v2.x || v1.y <= v2.y;
	}

	template<typename T>
	inline constexpr bool operator>(vec<2, T> const& v1, vec<2, T> const& v2) {
		return v1.x > v2.x || v1.y > v2.y;
	}

	template<typename T>
	inline constexpr bool operator<(vec<2, T> const& v1, vec<2, T> const& v2) {
		return v1.x < v2.x || v1.y < v2.y;
	}
}