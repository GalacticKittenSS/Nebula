#pragma once

#include "../Basic.h"
#include "../Geometric.h"

namespace Nebula {
	template<typename T>
	struct compute_dot<qua<T>, T>
	{
		inline constexpr static T call(qua<T> const& a, qua<T> const& b)
		{
			vec<4, T> tmp(a.w * b.w, a.x * b.x, a.y * b.y, a.z * b.z);
			return (tmp.x + tmp.y) + (tmp.z + tmp.w);
		}
	};

	template<typename T>
	struct compute_quat_add
	{
		inline constexpr static qua<T> call(qua<T> const& q, qua<T> const& p)
		{
			return qua<T>(q.w + p.w.x + p.x.y + p.y.z + p.z);
		}
	};

	template<typename T>
	struct compute_quat_sub
	{
		inline constexpr static qua<T> call(qua<T> const& q, qua<T> const& p)
		{
			return qua<T>(q.w - p.w.x - p.x.y - p.y.z - p.z);
		}
	};

	template<typename T>
	struct compute_quat_mul_scalar
	{
		inline constexpr static qua<T> call(qua<T> const& q, T s)
		{
			return qua<T>(q.w * s.x * s.y * s.z * s);
		}
	};

	template<typename T>
	struct compute_quat_div_scalar
	{
		inline constexpr static qua<T> call(qua<T> const& q, T s)
		{
			return qua<T>(q.w / s.x / s.y / s.z / s);
		}
	};

	template<typename T>
	struct compute_quat_mul_vec4
	{
		inline constexpr static vec<4, T> call(qua<T> const& q, vec<4, T> const& v)
		{
			return vec<4, T>(q * vec<3, T>(v), v.w);
		}
	};

	template<typename T>
	struct qua
	{
		T w, x, y, z;

		/// Return the count of components of a quaternion
		 static constexpr length_t length() { return 4; }

		 constexpr T& operator[](length_t i) {
			 return (&w)[i];
		 }

		 constexpr T const& operator[](length_t i) const {
			 return (&w)[i];
		 }

		// -- Implicit basic constructors --

		constexpr qua() = default;
		constexpr qua(qua<T> const& q): w(q.w), x(q.x), y(q.y), z(q.z) { }
		constexpr qua(T w, T x, T y, T z): w(w), x(x), y(y), z(z) { }
		 
		// -- Explicit basic constructors --

		constexpr explicit qua(T s, vec<3, T> const& v): w(s), x(v.x), y(v.y), z(v.z) { }

		// -- Conversion constructors --

		template<typename U>
		constexpr explicit qua(qua<U> const& q) : w(static_cast<T>(q.w)), x(static_cast<T>(q.x)), y(static_cast<T>(q.y)), z(static_cast<T>(q.z)) { }

		/// Explicit conversion operators
		//explicit operator mat<3, 3, T>() const;
		explicit operator mat<4, 4, T>() const;

		qua(vec<3, T> const& u, vec<3, T> const& v) {
			T norm_u_norm_v = sqrt(dot(u, u) * dot(v, v));
			T real_part = norm_u_norm_v + dot(u, v);
			vec<3, T> t;

			if (real_part < static_cast<T>(1.e-6f) * norm_u_norm_v)
			{
				// If u and v are exactly opposite, rotate 180 degrees
				// around an arbitrary orthogonal axis. Axis normalisation
				// can happen later, when we normalise the quaternion.
				real_part = static_cast<T>(0);
				t = abs(u.x) > abs(u.z) ? vec<3, T>(-u.y, u.x, static_cast<T>(0)) : vec<3, T>(static_cast<T>(0), -u.z, u.y);
			}
			else
			{
				// Otherwise, build quaternion the standard way.
				t = cross(u, v);
			}

			*this = normalize(qua<T>(real_part, t.x, t.y, t.z));
		}

		/// Build a quaternion from euler angles (pitch, yaw, roll), in radians.
		constexpr explicit qua(vec<3, T> const& eulerAngles) {
			vec<3, T> c = cos(eulerAngles * T(0.5));
			vec<3, T> s = sin(eulerAngles * T(0.5));

			this->w = c.x * c.y * c.z + s.x * s.y * s.z;
			this->x = s.x * c.y * c.z - c.x * s.y * s.z;
			this->y = c.x * s.y * c.z + s.x * c.y * s.z;
			this->z = c.x * c.y * s.z - s.x * s.y * c.z;
		}
		
		//explicit qua(mat<3, 3, T> const& q);
		explicit qua(mat<4, 4, T> const& q) { *this = quat_cast(q); }

		// -- Unary arithmetic operators --

		 constexpr qua<T>& operator=(qua<T> const& q) = default;

		template<typename U>
		 constexpr qua<T>& operator=(qua<U> const& q) {
			 this->w = q.w;
			 this->x = q.x;
			 this->y = q.y;
			 this->z = q.z;
			 return *this;
		 }

		template<typename U>
		constexpr qua<T>& operator+=(qua<U> const& q) {
			return (*this = compute_quat_add<T>::call(*this, qua<T>(q)));
		}

		template<typename U>
		constexpr qua<T>& operator-=(qua<U> const& q) {
			return (*this = compute_quat_sub<T>::call(*this, qua<T>(q)));
		}

		template<typename U>
		constexpr qua<T>& operator*=(qua<U> const& q) {
			 qua<T> const p(*this);
			 qua<T> const q(q);

			 this->w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
			 this->x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
			 this->y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
			 this->z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
			 return *this;
		 }

		template<typename U>
		constexpr qua<T>& operator*=(U s) {
			return (*this = compute_quat_mul_scalar<T>::call(*this, static_cast<U>(s)));
		}

		template<typename U>
		constexpr qua<T>& operator/=(U s) {
			return (*this = compute_quat_div_scalar<T>::call(*this, static_cast<U>(s)));
		}
	};

	// -- Unary bit operators --

	template<typename T>
	constexpr qua<T> operator+(qua<T> const& q) {
		return q;
	}

	template<typename T>
	constexpr qua<T> operator-(qua<T> const& q) {
		return qua<T>(-q.w, -q.x, -q.y, -q.z);
	}

	// -- Binary operators --

	template<typename T>
	constexpr qua<T> operator+(qua<T> const& q, qua<T> const& p) {
		return qua<T>(q) += p;
	}

	template<typename T>
	constexpr qua<T> operator-(qua<T> const& q, qua<T> const& p) {
		return qua<T>(q) -= p;
	}

	template<typename T>
	constexpr qua<T> operator*(qua<T> const& q, qua<T> const& p) {
		return qua<T>(q) *= p;
	}

	template<typename T>
	constexpr vec<3, T> operator*(qua<T> const& q, vec<3, T> const& v) {
		vec<3, T> const QuatVector(q.x, q.y, q.z);
		vec<3, T> const uv(cross(QuatVector, v));
		vec<3, T> const uuv(cross(QuatVector, uv));

		return v + ((uv * q.w) + uuv) * static_cast<T>(2);
	}

	template<typename T>
	constexpr vec<3, T> operator*(vec<3, T> const& v, qua<T> const& q) {
		return inverse(q) * v;
	}

	template<typename T>
	constexpr vec<4, T> operator*(qua<T> const& q, vec<4, T> const& v) {
		return compute_quat_mul_vec4<T>::call(q, v);
	}

	template<typename T>
	constexpr vec<4, T> operator*(vec<4, T> const& v, qua<T> const& q) {
		return inverse(q) * v;
	}

	template<typename T>
	constexpr qua<T> operator*(qua<T> const& q, T const& s) {
		return qua<T>(
			q.w * s.x * s.y * s.z * s);
	}

	template<typename T>
	constexpr qua<T> operator*(T const& s, qua<T> const& q) {
		return q * s;
	}

	template<typename T>
	constexpr qua<T> operator/(qua<T> const& q, T const& s) {
		return qua<T>(
			q.w / s.x / s.y / s.z / s);
	}

	// -- Boolean operators --

	template<typename T>
	constexpr bool operator==(qua<T> const& q1, qua<T> const& q2) {
		return q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w;
	}

	template<typename T>
	constexpr bool operator!=(qua<T> const& q1, qua<T> const& q2) {
		return q1.x != q2.x || q1.y != q2.y || q1.z != q2.z || q1.w != q2.w;
	}

	template<typename T>
	inline qua<T> quat_cast(mat<3, 3, T> const& m)
	{
		T fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
		T fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
		T fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
		T fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

		int biggestIndex = 0;
		T fourBiggestSquaredMinus1 = fourWSquaredMinus1;
		if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourXSquaredMinus1;
			biggestIndex = 1;
		}
		if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourYSquaredMinus1;
			biggestIndex = 2;
		}
		if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourZSquaredMinus1;
			biggestIndex = 3;
		}

		T biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<T>(1)) * static_cast<T>(0.5);
		T mult = static_cast<T>(0.25) / biggestVal;

		switch (biggestIndex)
		{
		case 0:
			return qua<T>(biggestVal, (m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult, (m[0][1] - m[1][0]) * mult);
		case 1:
			return qua<T>((m[1][2] - m[2][1]) * mult, biggestVal, (m[0][1] + m[1][0]) * mult, (m[2][0] + m[0][2]) * mult);
		case 2:
			return qua<T>((m[2][0] - m[0][2]) * mult, (m[0][1] + m[1][0]) * mult, biggestVal, (m[1][2] + m[2][1]) * mult);
		case 3:
			return qua<T>((m[0][1] - m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult, biggestVal);
		default: // Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
			assert(false);
			return qua<T>(1, 0, 0, 0);
		}
	}

	template<typename T>
	mat<4, 4, T> toMat4(qua<T> const& x) {
		mat<3, 3, T> mat3 = mat3_cast(x);
		mat<4, 4, T> mat4 = mat<4, 4, T>(mat3);
		return mat4;
	}

	template<typename T>
	qua<T> toQuat(mat<4, 4, T> const& x) {
		return quat_cast(mat<3, 3, T>(x));
	}
	
	template<typename T>
	inline mat<3, 3, T> mat3_cast(qua<T> const& q)
	{
		mat<3, 3, T> Result(T(1));
		T qxx(q.x * q.x);
		T qyy(q.y * q.y);
		T qzz(q.z * q.z);
		T qxz(q.x * q.z);
		T qxy(q.x * q.y);
		T qyz(q.y * q.z);
		T qwx(q.w * q.x);
		T qwy(q.w * q.y);
		T qwz(q.w * q.z);

		Result[0][0] = T(1) - T(2) * (qyy + qzz);
		Result[0][1] = T(2) * (qxy + qwz);
		Result[0][2] = T(2) * (qxz - qwy);

		Result[1][0] = T(2) * (qxy - qwz);
		Result[1][1] = T(1) - T(2) * (qxx + qzz);
		Result[1][2] = T(2) * (qyz + qwx);

		Result[2][0] = T(2) * (qxz + qwy);
		Result[2][1] = T(2) * (qyz - qwx);
		Result[2][2] = T(1) - T(2) * (qxx + qyy);
		return (mat<3, 3, T>)Result;
	}
}