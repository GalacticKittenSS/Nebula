#pragma once

#include "../basic.h"

namespace Nebula {
	template <typename T>
	struct vec<1, T> {
		union { T x, r, s; };

		// -- Component accesses --

		static constexpr length_t Length() { return 1; }
		
		inline constexpr T& operator[](length_t i) {
			return x;
		}

		inline constexpr const T& operator[](length_t i) const {
			return x;
		}

		// -- Implicit Constructors --

		constexpr vec(): x(0) { }
		constexpr vec(vec<1, T> const& v): x(v.x) { }

		// -- Explicit Constructors --
		constexpr explicit vec(T scalar): x(scalar) { }

		// -- Explicit Coversion Constructors --

		template<typename U>
		inline constexpr vec(vec<1, U> const& v) : x(static_cast<T>(v.x)) { }
		
		template<typename U>
		inline constexpr vec(vec<2, U> const& v) : x(static_cast<T>(v.x)) { }
		
		template<typename U>
		inline constexpr vec(vec<3, U> const& v) : x(static_cast<T>(v.x)) { }
		
		template<typename U>
		inline constexpr vec(vec<4, U> const& v) : x(static_cast<T>(v.x)) { }
		

		// -- Arithmetic operators --
		
		template<typename U>
		inline constexpr vec<1, T>& operator=(vec<1, U> const& v) {
			this->x = static_cast<T>(v.x);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator+=(U scalar) {
			this->x += static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator+=(vec<1, U> const& v) {
			this->x += static_cast<T>(v.x);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator-=(U scalar) {
			this->x -= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator-=(vec<1, U> const& v) {
			this->x -= static_cast<T>(v.x);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator*=(U scalar) {
			this->x *= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator*=(vec<1, U> const& v) {
			this->x *= static_cast<T>(v.x);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator/=(U scalar) {
			this->x /= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator/=(vec<1, U> const& v) {
			this->x /= static_cast<T>(v.x);
			return *this;
		}

		// -- Increment and decrement operators --

		inline constexpr vec<1, T>& operator++() {
			++this->x;
			return *this;
		}

		inline constexpr vec<1, T>& operator--() {
			--this->x;
			return *this;
		}

		inline constexpr vec<1, T> operator++(int) {
			vec<1, T> Result(*this);
			++* this;
			return Result;
		}

		inline constexpr vec<1, T> operator--(int) {
			vec<1, T> Result(*this);
			--* this;
			return Result;
		}



		// -- Bit Operators --


		template<typename U>
		inline constexpr vec<1, T>& operator%=(U scalar) {
			this->x %= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator%=(vec<1, U> const& v) {
			this->x %= static_cast<T>(v.x);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator&=(U scalar) {
			this->x &= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator&=(vec<1, U> const& v) {
			this->x &= static_cast<T>(v.x);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator|=(U scalar) {
			this->x |= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator|=(vec<1, U> const& v) {
			this->x |= U(v.x);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator^=(U scalar) {
			this->x ^= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator^=(vec<1, U> const& v) {
			this->x ^= static_cast<T>(v.x);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator<<=(U scalar) {
			this->x <<= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator<<=(vec<1, U> const& v) {
			this->x <<= static_cast<T>(v.x);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator>>=(U scalar) {
			this->x >>= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<1, T>& operator>>=(vec<1, U> const& v) {
			this->x >>= static_cast<T>(v.x);
			return *this;
		}


	};
	
	// -- Operators --

	template<typename T>
	inline constexpr vec<1, T> operator+(vec<1, T> const& v) {
		return v;
	}

	template<typename T>
	inline constexpr vec<1, T> operator-(vec<1, T> const& v) {
		return vec<1, T>(-v.x);
	}

	// -- Binary operators --

	//Operator +
	template<typename T>
	inline constexpr vec<1, T> operator+(vec<1, T> const& v, T scalar) {
		return vec<1, T>(v.x + scalar);
	}

	template<typename T>
	inline constexpr vec<1, T> operator+(T scalar, vec<1, T> const& v) {
		return vec<1, T>(scalar + v.x);
	}

	template<typename T>
	inline constexpr vec<1, T> operator+(vec<1, T> const& v1, vec<1, T> const& v2) {
		return vec<1, T>(v1.x + v2.x);
	}

	//Operator -
	template<typename T>
	inline constexpr vec<1, T> operator-(vec<1, T> const& v, T scalar) {
		return vec<1, T>(v.x - scalar);
	}

	template<typename T>
	inline constexpr vec<1, T> operator-(T scalar, vec<1, T> const& v) {
		return vec<1, T>(scalar - v.x);
	}

	template<typename T>
	inline constexpr vec<1, T> operator-(vec<1, T> const& v1, vec<1, T> const& v2) {
		return vec<1, T>(v1.x - v2.x);
	}

	//Operator *
	template<typename T>
	inline constexpr vec<1, T> operator*(vec<1, T> const& v, T scalar) {
		return vec<1, T>(v.x * scalar);
	}

	template<typename T>
	inline constexpr vec<1, T> operator*(T scalar, vec<1, T> const& v) {
		return vec<1, T>(scalar * v.x);
	}

	template<typename T>
	inline constexpr vec<1, T> operator*(vec<1, T> const& v1, vec<1, T> const& v2) {
		return vec<1, T>(v1.x * v2.x);
	}

	//Operator /
	template<typename T>
	inline constexpr vec<1, T> operator/(vec<1, T> const& v, T scalar) {
		return vec<1, T>(v.x / scalar);
	}

	template<typename T>
	inline constexpr vec<1, T> operator/(T scalar, vec<1, T> const& v) {
		return vec<1, T>(scalar / v.x);
	}

	template<typename T>
	inline constexpr vec<1, T> operator/(vec<1, T> const& v1, vec<1, T> const& v2) {
		return vec<1, T>(v1.x / v2.x);
	}

	//Operator %
	template<typename T>
	inline constexpr vec<1, T> operator%(vec<1, T> const& v, T scalar) {
		return vec<1, T>(v.x % scalar);
	}

	template<typename T>
	inline constexpr vec<1, T> operator%(T scalar, vec<1, T> const& v) {
		return vec<1, T>(scalar % v.x);
	}

	template<typename T>
	inline constexpr vec<1, T> operator%(vec<1, T> const& v1, vec<1, T> const& v2) {
		return vec<1, T>(v1.x % v2.x);
	}

	//Operator &
	template<typename T>
	inline constexpr vec<1, T> operator&(vec<1, T> const& v, T scalar) {
		return vec<1, T>(v.x & scalar);
	}

	template<typename T>
	inline constexpr vec<1, T> operator&(T scalar, vec<1, T> const& v) {
		return vec<1, T>(scalar & v.x);
	}

	template<typename T>
	inline constexpr vec<1, T> operator&(vec<1, T> const& v1, vec<1, T> const& v2) {
		return vec<1, T>(v1.x & v2.x);
	}

	//Operator |
	template<typename T>
	inline constexpr vec<1, T> operator|(vec<1, T> const& v, T scalar) {
		return vec<1, T>(v.x | scalar);
	}

	template<typename T>
	inline constexpr vec<1, T> operator|(T scalar, vec<1, T> const& v) {
		return vec<1, T>(scalar | v.x);
	}

	template<typename T>
	inline constexpr vec<1, T> operator|(vec<1, T> const& v1, vec<1, T> const& v2) {
		return vec<1, T>(v1.x | v2.x);
	}

	//Operator ^
	template<typename T>
	inline constexpr vec<1, T> operator^(vec<1, T> const& v, T scalar) {
		return vec<1, T>(v.x ^ scalar);
	}

	template<typename T>
	inline constexpr vec<1, T> operator^(T scalar, vec<1, T> const& v) {
		return vec<1, T>(scalar ^ v.x);
	}

	template<typename T>
	inline constexpr vec<1, T> operator^(vec<1, T> const& v1, vec<1, T> const& v2) {
		return vec<1, T>(v1.x ^ v2.x);
	}

	//Operator <<
	template<typename T>
	inline constexpr vec<1, T> operator<<(vec<1, T> const& v, T scalar) {
		return vec<1, T>(v.x << scalar);
	}

	template<typename T>
	inline constexpr vec<1, T> operator<<(T scalar, vec<1, T> const& v) {
		return vec<1, T>(scalar << v.x);
	}

	template<typename T>
	inline constexpr vec<1, T> operator<<(vec<1, T> const& v1, vec<1, T> const& v2) {
		return vec<1, T>(v1.x >> v2.x);
	}

	//Operator >>
	template<typename T>
	inline constexpr vec<1, T> operator>>(vec<1, T> const& v, T scalar) {
		return vec<1, T>(v.x >> scalar);
	}

	template<typename T>
	inline constexpr vec<1, T> operator>>(T scalar, vec<1, T> const& v) {
		return vec<1, T>(scalar >> v.x);
	}

	template<typename T>
	inline constexpr vec<1, T> operator>>(vec<1, T> const& v1, vec<1, T> const& v2) {
		return vec<1, T>(v1.x >> v2.x);
	}

	//Operator ~
	template<typename T>
	inline constexpr vec<1, T> operator~(vec<1, T> const& v) {
		return vec<1, T>(~v.x);
	}

	// -- Boolean operators --

	template<typename T>
	inline constexpr bool  operator==(vec<1, T> const& v1, vec<1, T> const& v2) {
		return v1.x == v2.x;
	}

	template<typename T>
	inline constexpr bool operator!=(vec<1, T> const& v1, vec<1, T> const& v2) {
		return !(v1 == v2);
	}

	inline std::ostream& operator<<(std::ostream& os, const vec<1, float>& vector) {
		return os << "Vector 1: (" << vector.x << ")";
	}
}