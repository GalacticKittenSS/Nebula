#pragma once

#include "../Basic.h"

namespace Nebula {
	template <typename T>
	struct mat<4, 4, T> {
		vec<4, T> value[4];

		// -- Component accesses --
		static constexpr length_t length() { return 4; }

		vec<4, T>& operator[](length_t i) {
			return this->value[i];
		}
		constexpr vec<4, T> const& operator[](length_t i) const {
			return this->value[i];
		}

		// -- Implicit Constructors --
		constexpr mat() {
			this->value[0] = vec<4, T>(1, 0, 0, 0);
			this->value[1] = vec<4, T>(0, 1, 0, 0);
			this->value[2] = vec<4, T>(0, 0, 1, 0);
			this->value[3] = vec<4, T>(0, 0, 0, 1);
		}
		constexpr mat(const mat<4, 4, T>& m) { 
			this->value[0] = m[0];
			this->value[1] = m[1];
			this->value[2] = m[2];
			this->value[3] = m[3];
		}
		
		constexpr mat(
			T const& x0, T const& y0, T const& z0, T const& w0,
			T const& x1, T const& y1, T const& z1, T const& w1,
			T const& x2, T const& y2, T const& z2, T const& w2,
			T const& x3, T const& y3, T const& z3, T const& w3) { 
			this->value[0] = vec<4, T>(x0, y0, z0, w0);
			this->value[1] = vec<4, T>(x1, y1, z1, w1);
			this->value[2] = vec<4, T>(x2, y2, z2, w2);
			this->value[3] = vec<4, T>(x3, y3, z3, w3);
		}

		constexpr mat(
			vec<4, T> const& v0,
			vec<4, T> const& v1,
			vec<4, T> const& v2,
			vec<4, T> const& v3) {
			this->value[0] = v0;
			this->value[1] = v1;
			this->value[2] = v2;
			this->value[3] = v3;
		}

		// -- Explicit Constructors --
		explicit constexpr mat(T const& s) {
			this->value[0] = vec<4, T>(s, 0, 0, 0);
			this->value[1] = vec<4, T>(0, s, 0, 0);
			this->value[2] = vec<4, T>(0, 0, s, 0);
			this->value[3] = vec<4, T>(0, 0, 0, s);
		}

		template<
			typename X1, typename Y1, typename Z1, typename W1,
			typename X2, typename Y2, typename Z2, typename W2,
			typename X3, typename Y3, typename Z3, typename W3,
			typename X4, typename Y4, typename Z4, typename W4>
			constexpr mat(
				X1 const& x1, Y1 const& y1, Z1 const& z1, W1 const& w1,
				X2 const& x2, Y2 const& y2, Z2 const& z2, W2 const& w2,
				X3 const& x3, Y3 const& y3, Z3 const& z3, W3 const& w3,
				X4 const& x4, Y4 const& y4, Z4 const& z4, W4 const& w4) {
			this->value[0] = vec<4, T>(x1, y1, z1, w1);
			this->value[1] = vec<4, T>(x2, y2, z2, w2);
			this->value[2] = vec<4, T>(x3, y3, z3, w3);
			this->value[3] = vec<4, T>(x4, y4, z4, w4);
		}

		template<typename V1, typename V2, typename V3, typename V4>
		constexpr mat(
			vec<4, V1> const& v1,
			vec<4, V2> const& v2,
			vec<4, V3> const& v3,
			vec<4, V4> const& v4) {
			this->value[0] = vec<4, T>(v1);
			this->value[1] = vec<4, T>(v2);
			this->value[2] = vec<4, T>(v3);
			this->value[3] = vec<4, T>(v4);
		}

		// -- Matrix conversions --

		template<typename U>
		explicit constexpr mat(mat<4, 4, U> const& m) {
			this->value[0] = vec<4, T>(m[0]);
			this->value[1] = vec<4, T>(m[1]);
			this->value[2] = vec<4, T>(m[2]);
			this->value[3] = vec<4, T>(m[3]);
		}
		
		explicit constexpr mat(mat<2, 2, T> const& m) {
			this->value[0] = vec<4, T>(m[0], 0, 0);
			this->value[1] = vec<4, T>(m[1], 0, 0);
			this->value[2] = vec<4, T>(0, 0, 1, 0);
			this->value[3] = vec<4, T>(0, 0, 0, 1);
		}

		explicit constexpr mat(mat<3, 3, T> const& m) {
			this->value[0] = vec<4, T>(m[0], 0);
			this->value[1] = vec<4, T>(m[1], 0);
			this->value[2] = vec<4, T>(m[2], 0);
			this->value[3] = vec<4, T>(0, 0, 0, 1);
		}

		explicit constexpr mat(mat<2, 3, T> const& m) {
			this->value[0] = vec<4, T>(m[0], 0);
			this->value[1] = vec<4, T>(m[1], 0);
			this->value[2] = vec<4, T>(0, 0, 1, 0);
			this->value[3] = vec<4, T>(0, 0, 0, 1);
		}

		explicit constexpr mat(mat<3, 2, T> const& m) {
			this->value[0] = vec<4, T>(m[0], 0, 0);
			this->value[1] = vec<4, T>(m[1], 0, 0);
			this->value[2] = vec<4, T>(m[2], 1, 0);
			this->value[3] = vec<4, T>(0, 0, 0, 1);
		}

		explicit constexpr mat(mat<2, 4, T> const& m) {
			this->value[0] = m[0];
			this->value[1] = m[1];
			this->value[2] = vec<4, T>(0, 0, 1, 0);
			this->value[3] = vec<4, T>(0, 0, 0, 1);
		}

		explicit constexpr mat(mat<4, 2, T> const& m) {
			this->value[0] = vec<4, T>(m[0], 0, 0);
			this->value[1] = vec<4, T>(m[1], 0, 0);
			this->value[2] = vec<4, T>(m[2], 1, 0);
			this->value[3] = vec<4, T>(m[3], 0, 1);
		}

		explicit constexpr mat(mat<3, 4, T> const& m) {
			this->value[0] = m[0];
			this->value[1] = m[1];
			this->value[2] = m[2];
			this->value[3] = vec<4, T>(0, 0, 0, 1);
		}

		explicit constexpr mat(mat<4, 3, T> const& m) {
			this->value[0] = vec<4, T>(m[0], 0);
			this->value[1] = vec<4, T>(m[1], 0);
			this->value[2] = vec<4, T>(m[2], 0);
			this->value[3] = vec<4, T>(m[3], 1);
		}

		template<typename U>
		inline mat<4, 4, T>& operator=(mat<4, 4, U> const& m)
		{
			//memcpy could be faster
			//memcpy(&this->value, &m.value, 16 * sizeof(valType));
			this->value[0] = m[0];
			this->value[1] = m[1];
			this->value[2] = m[2];
			this->value[3] = m[3];
			return *this;
		}

		template<typename U>
		inline mat<4, 4, T>& operator+=(U s)
		{
			this->value[0] += s;
			this->value[1] += s;
			this->value[2] += s;
			this->value[3] += s;
			return *this;
		}

		template<typename U>
		inline mat<4, 4, T>& operator+=(mat<4, 4, U> const& m)
		{
			this->value[0] += m[0];
			this->value[1] += m[1];
			this->value[2] += m[2];
			this->value[3] += m[3];
			return *this;
		}

		template<typename U>
		inline mat<4, 4, T>& operator-=(U s)
		{
			this->value[0] -= s;
			this->value[1] -= s;
			this->value[2] -= s;
			this->value[3] -= s;
			return *this;
		}

		template<typename U>
		inline mat<4, 4, T>& operator-=(mat<4, 4, U> const& m)
		{
			this->value[0] -= m[0];
			this->value[1] -= m[1];
			this->value[2] -= m[2];
			this->value[3] -= m[3];
			return *this;
		}

		template<typename U>
		inline mat<4, 4, T>& operator*=(U s)
		{
			this->value[0] *= s;
			this->value[1] *= s;
			this->value[2] *= s;
			this->value[3] *= s;
			return *this;
		}

		template<typename U>
		inline mat<4, 4, T>& operator*=(mat<4, 4, U> const& m)
		{
			return (*this = *this * m);
		}

		template<typename U>
		inline mat<4, 4, T>& operator/=(U s)
		{
			this->value[0] /= s;
			this->value[1] /= s;
			this->value[2] /= s;
			this->value[3] /= s;
			return *this;
		}

		template<typename U>
		inline mat<4, 4, T>& operator/=(mat<4, 4, U> const& m)
		{
			return *this *= inverse(m);
		}

		// -- Increment and decrement operators --

		inline mat<4, 4, T>& operator++()
		{
			++this->value[0];
			++this->value[1];
			++this->value[2];
			++this->value[3];
			return *this;
		}

		inline mat<4, 4, T>& operator--()
		{
			--this->value[0];
			--this->value[1];
			--this->value[2];
			--this->value[3];
			return *this;
		}

		inline mat<4, 4, T> operator++(int)
		{
			mat<4, 4, T> Result(*this);
			++* this;
			return Result;
		}

		inline mat<4, 4, T> operator--(int)
		{
			mat<4, 4, T> Result(*this);
			--* this;
			return Result;
		}
	};

	// -- Unary constant operators --

	template<typename T>
	inline mat<4, 4, T> operator+(mat<4, 4, T> const& m)
	{
		return m;
	}

	template<typename T>
	inline mat<4, 4, T> operator-(mat<4, 4, T> const& m) {
		return mat<4, 4, T>(
			-m[0],
			-m[1],
			-m[2],
			-m[3]);
	}

	// -- Binary arithmetic operators --

	template<typename T>
	inline mat<4, 4, T> operator+(mat<4, 4, T> const& m, T const& s) {
		return mat<4, 4, T>(
			m[0] + s,
			m[1] + s,
			m[2] + s,
			m[3] + s);
	}

	template<typename T>
	inline mat<4, 4, T> operator+(T const& s, mat<4, 4, T> const& m) {
		return mat<4, 4, T>(
			m[0] + s,
			m[1] + s,
			m[2] + s,
			m[3] + s);
	}

	template<typename T>
	inline mat<4, 4, T> operator+(mat<4, 4, T> const& m1, mat<4, 4, T> const& m2) {
		return mat<4, 4, T>(
			m1[0] + m2[0],
			m1[1] + m2[1],
			m1[2] + m2[2],
			m1[3] + m2[3]);
	}

	template<typename T>
	inline mat<4, 4, T> operator-(mat<4, 4, T> const& m, T const& s) {
		return mat<4, 4, T>(
			m[0] - s,
			m[1] - s,
			m[2] - s,
			m[3] - s);
	}

	template<typename T>
	inline mat<4, 4, T> operator-(T const& s, mat<4, 4, T> const& m) {
		return mat<4, 4, T>(
			s - m[0],
			s - m[1],
			s - m[2],
			s - m[3]);
	}

	template<typename T>
	inline mat<4, 4, T> operator-(mat<4, 4, T> const& m1, mat<4, 4, T> const& m2) {
		return mat<4, 4, T>(
			m1[0] - m2[0],
			m1[1] - m2[1],
			m1[2] - m2[2],
			m1[3] - m2[3]);
	}

	template<typename T>
	inline mat<4, 4, T> operator*(mat<4, 4, T> const& m, T const& s) {
		return mat<4, 4, T>(
			m[0] * s,
			m[1] * s,
			m[2] * s,
			m[3] * s);
	}

	template<typename T>
	inline typename vec<4, T> operator*(mat<4, 4, T> const& m, vec<4, T> const& v) {
		vec<4, T> const Mov0(v[0]);
		vec<4, T> const Mov1(v[1]);
		vec<4, T> const Mul0 = m[0] * Mov0;
		vec<4, T> const Mul1 = m[1] * Mov1;
		vec<4, T> const Add0 = Mul0 + Mul1;
		vec<4, T> const Mov2(v[2]);
		vec<4, T> const Mov3(v[3]);
		vec<4, T> const Mul2 = m[2] * Mov2;
		vec<4, T> const Mul3 = m[3] * Mov3;
		vec<4, T> const Add1 = Mul2 + Mul3;
		vec<4, T> const Add2 = Add0 + Add1;
		return Add2;
	}

	template<typename T>
	inline vec<4, T> operator*(typename vec<4, T> const& v, mat<4, 4, T> const& m) {
		return vec<4, T>(
			m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2] + m[0][3] * v[3],
			m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2] + m[1][3] * v[3],
			m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2] + m[2][3] * v[3],
			m[3][0] * v[0] + m[3][1] * v[1] + m[3][2] * v[2] + m[3][3] * v[3]);
	}

	template<typename T>
	inline mat<2, 4, T> operator*(mat<4, 4, T> const& m1, mat<2, 4, T> const& m2) {
		return mat<2, 4, T>(
			m1[0][0] * m2[0][0] + m1[1][0] * m2[0][1] + m1[2][0] * m2[0][2] + m1[3][0] * m2[0][3],
			m1[0][1] * m2[0][0] + m1[1][1] * m2[0][1] + m1[2][1] * m2[0][2] + m1[3][1] * m2[0][3],
			m1[0][2] * m2[0][0] + m1[1][2] * m2[0][1] + m1[2][2] * m2[0][2] + m1[3][2] * m2[0][3],
			m1[0][3] * m2[0][0] + m1[1][3] * m2[0][1] + m1[2][3] * m2[0][2] + m1[3][3] * m2[0][3],
			m1[0][0] * m2[1][0] + m1[1][0] * m2[1][1] + m1[2][0] * m2[1][2] + m1[3][0] * m2[1][3],
			m1[0][1] * m2[1][0] + m1[1][1] * m2[1][1] + m1[2][1] * m2[1][2] + m1[3][1] * m2[1][3],
			m1[0][2] * m2[1][0] + m1[1][2] * m2[1][1] + m1[2][2] * m2[1][2] + m1[3][2] * m2[1][3],
			m1[0][3] * m2[1][0] + m1[1][3] * m2[1][1] + m1[2][3] * m2[1][2] + m1[3][3] * m2[1][3]);
	}

	template<typename T>
	inline mat<3, 4, T> operator*(mat<4, 4, T> const& m1, mat<3, 4, T> const& m2) {
		return mat<3, 4, T>(
			m1[0][0] * m2[0][0] + m1[1][0] * m2[0][1] + m1[2][0] * m2[0][2] + m1[3][0] * m2[0][3],
			m1[0][1] * m2[0][0] + m1[1][1] * m2[0][1] + m1[2][1] * m2[0][2] + m1[3][1] * m2[0][3],
			m1[0][2] * m2[0][0] + m1[1][2] * m2[0][1] + m1[2][2] * m2[0][2] + m1[3][2] * m2[0][3],
			m1[0][3] * m2[0][0] + m1[1][3] * m2[0][1] + m1[2][3] * m2[0][2] + m1[3][3] * m2[0][3],
			m1[0][0] * m2[1][0] + m1[1][0] * m2[1][1] + m1[2][0] * m2[1][2] + m1[3][0] * m2[1][3],
			m1[0][1] * m2[1][0] + m1[1][1] * m2[1][1] + m1[2][1] * m2[1][2] + m1[3][1] * m2[1][3],
			m1[0][2] * m2[1][0] + m1[1][2] * m2[1][1] + m1[2][2] * m2[1][2] + m1[3][2] * m2[1][3],
			m1[0][3] * m2[1][0] + m1[1][3] * m2[1][1] + m1[2][3] * m2[1][2] + m1[3][3] * m2[1][3],
			m1[0][0] * m2[2][0] + m1[1][0] * m2[2][1] + m1[2][0] * m2[2][2] + m1[3][0] * m2[2][3],
			m1[0][1] * m2[2][0] + m1[1][1] * m2[2][1] + m1[2][1] * m2[2][2] + m1[3][1] * m2[2][3],
			m1[0][2] * m2[2][0] + m1[1][2] * m2[2][1] + m1[2][2] * m2[2][2] + m1[3][2] * m2[2][3],
			m1[0][3] * m2[2][0] + m1[1][3] * m2[2][1] + m1[2][3] * m2[2][2] + m1[3][3] * m2[2][3]);
	}

	template<typename T>
	inline mat<4, 4, T> operator*(mat<4, 4, T> const& m1, mat<4, 4, T> const& m2)
	{
		const vec<4, T> SrcA0 = m1[0];
		const vec<4, T> SrcA1 = m1[1];
		const vec<4, T> SrcA2 = m1[2];
		const vec<4, T> SrcA3 = m1[3];
			  
		const vec<4, T> SrcB0 = m2[0];
		const vec<4, T> SrcB1 = m2[1];
		const vec<4, T> SrcB2 = m2[2];
		const vec<4, T> SrcB3 = m2[3];

		mat<4, 4, T> Result;
		Result[0] = SrcA0 * SrcB0[0] + SrcA1 * SrcB0[1] + SrcA2 * SrcB0[2] + SrcA3 * SrcB0[3];
		Result[1] = SrcA0 * SrcB1[0] + SrcA1 * SrcB1[1] + SrcA2 * SrcB1[2] + SrcA3 * SrcB1[3];
		Result[2] = SrcA0 * SrcB2[0] + SrcA1 * SrcB2[1] + SrcA2 * SrcB2[2] + SrcA3 * SrcB2[3];
		Result[3] = SrcA0 * SrcB3[0] + SrcA1 * SrcB3[1] + SrcA2 * SrcB3[2] + SrcA3 * SrcB3[3];
		return Result;
	}

	template<typename T>
	inline mat<4, 4, T> operator/(mat<4, 4, T> const& m, T const& s) {
		return mat<4, 4, T>(
			m[0] / s,
			m[1] / s,
			m[2] / s,
			m[3] / s);
	}

	template<typename T>
	inline mat<4, 4, T> operator/(T const& s, mat<4, 4, T> const& m)
	{
		return mat<4, 4, T>(
			s / m[0],
			s / m[1],
			s / m[2],
			s / m[3]);
	}

	template<typename T>
	inline vec<4, T> operator/(mat<4, 4, T> const& m, vec<4, T> const& v)
	{
		return inverse(m) * v;
	}

	template<typename T>
	inline vec<4, T> operator/( vec<4, T> const& v, mat<4, 4, T> const& m)
	{
		return v * inverse(m);
	}

	template<typename T>
	inline mat<4, 4, T> operator/(mat<4, 4, T> const& m1, mat<4, 4, T> const& m2)
	{
		mat<4, 4, T> m1_copy(m1);
		return m1_copy /= m2;
	}

	// -- Boolean operators --

	template<typename T>
	inline bool operator==(mat<4, 4, T> const& m1, mat<4, 4, T> const& m2) {
		return (m1[0] == m2[0]) && (m1[1] == m2[1]) && (m1[2] == m2[2]) && (m1[3] == m2[3]);
	}

	template<typename T>
	inline bool operator!=(mat<4, 4, T> const& m1, mat<4, 4, T> const& m2) {
		return (m1[0] != m2[0]) || (m1[1] != m2[1]) || (m1[2] != m2[2]) || (m1[3] != m2[3]);
	}
}