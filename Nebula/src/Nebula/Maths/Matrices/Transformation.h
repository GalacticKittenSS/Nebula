#include "../basic.h"
#include "../Geometric.h"

namespace Nebula {
	template<length_t C, length_t R, typename T>
	struct compute_inverse {};

	template<typename T>
	struct compute_inverse<2, 2, T>
	{
		inline static mat<2, 2, T> call(mat<2, 2, T> const& m)
		{
			T OneOverDeterminant = static_cast<T>(1) / (
				+m[0][0] * m[1][1]
				- m[1][0] * m[0][1]);

			mat<2, 2, T> Inverse(
				+m[1][1] * OneOverDeterminant,
				-m[0][1] * OneOverDeterminant,
				-m[1][0] * OneOverDeterminant,
				+m[0][0] * OneOverDeterminant);

			return Inverse;
		}
	};

	template<typename T>
	struct compute_inverse<3, 3, T>
	{
		inline static mat<3, 3, T> call(mat<3, 3, T> const& m)
		{
			T OneOverDeterminant = static_cast<T>(1) / (
				+m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
				- m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
				+ m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]));

			mat<3, 3, T> Inverse;
			Inverse[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]) * OneOverDeterminant;
			Inverse[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * OneOverDeterminant;
			Inverse[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * OneOverDeterminant;
			Inverse[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * OneOverDeterminant;
			Inverse[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * OneOverDeterminant;
			Inverse[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * OneOverDeterminant;
			Inverse[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * OneOverDeterminant;
			Inverse[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * OneOverDeterminant;
			Inverse[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * OneOverDeterminant;

			return Inverse;
		}
	};

	template<typename T>
	struct compute_inverse<4, 4, T>
	{
		inline static mat<4, 4, T> call(mat<4, 4, T> const& m)
		{
			T Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
			T Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
			T Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

			T Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
			T Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
			T Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

			T Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
			T Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
			T Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

			T Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
			T Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
			T Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

			T Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
			T Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
			T Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

			T Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
			T Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
			T Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

			vec<4, T> Fac0(Coef00, Coef00, Coef02, Coef03);
			vec<4, T> Fac1(Coef04, Coef04, Coef06, Coef07);
			vec<4, T> Fac2(Coef08, Coef08, Coef10, Coef11);
			vec<4, T> Fac3(Coef12, Coef12, Coef14, Coef15);
			vec<4, T> Fac4(Coef16, Coef16, Coef18, Coef19);
			vec<4, T> Fac5(Coef20, Coef20, Coef22, Coef23);

			vec<4, T> Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
			vec<4, T> Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
			vec<4, T> Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
			vec<4, T> Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

			vec<4, T> Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
			vec<4, T> Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
			vec<4, T> Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
			vec<4, T> Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

			vec<4, T> SignA(+1, -1, +1, -1);
			vec<4, T> SignB(-1, +1, -1, +1);
			mat<4, 4, T> Inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

			vec<4, T> Row0(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);

			vec<4, T> Dot0(m[0] * Row0);
			T Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

			T OneOverDeterminant = static_cast<T>(1) / Dot1;

			return Inverse * OneOverDeterminant;
		}
	};


	template<length_t C, length_t R, typename T>
	mat<C, R, T> inverse(mat<C, R, T> const& m) {
		NB_ASSERT(std::numeric_limits<T>::is_iec559 || 0, "'inverse' only accept floating-point inputs");
		return compute_inverse<C, R, T>::call(m);
	}
	
	template<typename T>
	inline mat<4, 4, T> translate(mat<4, 4, T> const& m, vec<3, T> const& v) {
		mat<4, 4, T> Result(m);
		Result[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
		return Result;
	}


	template<typename T>
	inline mat<4, 4, T> translate(vec<3, T> const& v) {
		const mat<4, 4, T> m(1.0f);

		return translate(m, v);
	}
	
	template<typename T>
	inline mat<4, 4, T> rotate(mat<4, 4, T> const& m, T angle, vec<3, T> const& v) {
		T const a = angle;
		T const c = cos(a);
		T const s = sin(a);

		vec<3, T> axis(normalize(v));
		vec<3, T> temp((T(1) - c) * axis);

		mat<4, 4, T> Rotate;
		Rotate[0][0] = c + temp[0] * axis[0];
		Rotate[0][1] = temp[0] * axis[1] + s * axis[2];
		Rotate[0][2] = temp[0] * axis[2] - s * axis[1];

		Rotate[1][0] = temp[1] * axis[0] - s * axis[2];
		Rotate[1][1] = c + temp[1] * axis[1];
		Rotate[1][2] = temp[1] * axis[2] + s * axis[0];

		Rotate[2][0] = temp[2] * axis[0] + s * axis[1];
		Rotate[2][1] = temp[2] * axis[1] - s * axis[0];
		Rotate[2][2] = c + temp[2] * axis[2];

		mat<4, 4, T> Result;
		Result[0] = m[0] * Rotate[0][0] + m[1] * Rotate[0][1] + m[2] * Rotate[0][2];
		Result[1] = m[0] * Rotate[1][0] + m[1] * Rotate[1][1] + m[2] * Rotate[1][2];
		Result[2] = m[0] * Rotate[2][0] + m[1] * Rotate[2][1] + m[2] * Rotate[2][2];
		Result[3] = m[3];
		return Result;
	}
	
	template<typename T>
	inline mat<4, 4, T> rotate(T angle, vec<3, T> const& v)
	{
		const mat<4, 4, T> m(1.0f);

		return rotate(m, angle, v);
	}

	template<typename T>
	inline mat<4, 4, T> scale(mat<4, 4, T> const& m, vec<3, T> const& v)
	{
		mat<4, 4, T> Result;
		Result[0] = m[0] * v[0];
		Result[1] = m[1] * v[1];
		Result[2] = m[2] * v[2];
		Result[3] = m[3];
		return Result;
	}

	template<typename T>
	inline mat<4, 4, T> scale(vec<3, T> const& v)
	{
		mat<4, 4, T> const m(1.0f);

		return scale(m, v);
	}
	
	template<typename T>
	inline mat<4, 4, T> ortho(T left, T right, T bottom, T top, T zNear, T zFar)
	{
		mat<4, 4, T> Result(0);
		Result[0][0] =  static_cast<T>(2) / (right - left);
		Result[1][1] =  static_cast<T>(2) / (top - bottom);
		Result[2][2] = -static_cast<T>(2) / (zFar - zNear);
		Result[3][3] =  static_cast<T>(1);
		Result[3][0] = -(right + left) / (right - left);
		Result[3][1] = -(top + bottom) / (top - bottom);
		Result[3][2] = -(zFar + zNear) / (zFar - zNear);
		return Result;
	}

	template<typename T>
	inline mat<4, 4, T> perspective(T fov, T aspect, T zNear, T zFar) {
		T const tanHalfFovy = tan(fov / static_cast<T>(2));

		mat<4, 4, T> Result(static_cast<T>(0));
		Result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
		Result[1][1] = static_cast<T>(1) / (tanHalfFovy);
		Result[2][2] = -(zFar + zNear) / (zFar - zNear);
		Result[2][3] = -static_cast<T>(1);
		Result[3][2] = -(static_cast<T>(2) * zFar * zNear) / (zFar - zNear);
		return Result;
	}
}