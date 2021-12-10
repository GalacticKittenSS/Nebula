#pragma once

#include "Basic.h"

namespace Nebula {
	template<typename T>
	inline T const* value_ptr(vec<2, T> const& v)
	{
		return &(v.x);
	}

	template<typename T>
	inline T* value_ptr(vec<2, T>& v)
	{
		return &(v.x);
	}

	template<typename T>
	inline T const* value_ptr(vec<3, T> const& v)
	{
		return &(v.x);
	}

	template<typename T>
	inline T* value_ptr(vec<3, T>& v)
	{
		return &(v.x);
	}

	template<typename T>
	inline T const* value_ptr(vec<4, T> const& v)
	{
		return &(v.x);
	}

	template<typename T>
	inline T* value_ptr(vec<4, T>& v)
	{
		return &(v.x);
	}

	template<typename T>
	inline T const* value_ptr(mat<2, 2, T> const& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T* value_ptr(mat<2, 2, T>& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T const* value_ptr(mat<3, 3, T> const& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T* value_ptr(mat<3, 3, T>& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T const* value_ptr(mat<4, 4, T> const& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T* value_ptr(mat<4, 4, T>& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T const* value_ptr(mat<2, 3, T> const& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T* value_ptr(mat<2, 3, T>& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T const* value_ptr(mat<3, 2, T> const& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T* value_ptr(mat<3, 2, T>& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T const* value_ptr(mat<2, 4, T> const& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T* value_ptr(mat<2, 4, T>& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T const* value_ptr(mat<4, 2, T> const& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T* value_ptr(mat<4, 2, T>& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T const* value_ptr(mat<3, 4, T> const& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T* value_ptr(mat<3, 4, T>& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T const* value_ptr(mat<4, 3, T> const& m)
	{
		return &(m[0].x);
	}

	template<typename T>
	inline T* value_ptr(mat<4, 3, T>& m)
	{
		return &(m[0].x);
	}

	template <typename T>
	vec<1, T> make_vec1(vec<1, T> const& v)
	{
		return v;
	}

	template <typename T>
	vec<1, T> make_vec1(vec<2, T> const& v)
	{
		return vec<1, T>(v);
	}

	template <typename T>
	vec<1, T> make_vec1(vec<3, T> const& v)
	{
		return vec<1, T>(v);
	}

	template <typename T>
	vec<1, T> make_vec1(vec<4, T> const& v)
	{
		return vec<1, T>(v);
	}

	template <typename T>
	vec<2, T> make_vec2(vec<1, T> const& v)
	{
		return vec<2, T>(v.x, static_cast<T>(0));
	}

	template <typename T>
	vec<2, T> make_vec2(vec<2, T> const& v)
	{
		return v;
	}

	template <typename T>
	vec<2, T> make_vec2(vec<3, T> const& v)
	{
		return vec<2, T>(v);
	}

	template <typename T>
	vec<2, T> make_vec2(vec<4, T> const& v)
	{
		return vec<2, T>(v);
	}

	template <typename T>
	vec<3, T> make_vec3(vec<1, T> const& v)
	{
		return vec<3, T>(v.x, static_cast<T>(0), static_cast<T>(0));
	}

	template <typename T>
	vec<3, T> make_vec3(vec<2, T> const& v)
	{
		return vec<3, T>(v.x, v.y, static_cast<T>(0));
	}

	template <typename T>
	vec<3, T> make_vec3(vec<3, T> const& v)
	{
		return v;
	}

	template <typename T>
	vec<3, T> make_vec3(vec<4, T> const& v)
	{
		return vec<3, T>(v);
	}

	template <typename T>
	vec<4, T> make_vec4(vec<1, T> const& v)
	{
		return vec<4, T>(v.x, static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));
	}

	template <typename T>
	vec<4, T> make_vec4(vec<2, T> const& v)
	{
		return vec<4, T>(v.x, v.y, static_cast<T>(0), static_cast<T>(1));
	}

	template <typename T>
	vec<4, T> make_vec4(vec<3, T> const& v)
	{
		return vec<4, T>(v.x, v.y, v.z, static_cast<T>(1));
	}

	template <typename T>
	vec<4, T> make_vec4(vec<4, T> const& v)
	{
		return v;
	}

	template<typename T>
	inline vec<2, T> make_vec2(T const* const ptr)
	{
		vec<2, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(vec<2, T>));
		return Result;
	}

	template<typename T>
	inline vec<3, T> make_vec3(T const* const ptr)
	{
		vec<3, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(vec<3, T>));
		return Result;
	}

	template<typename T>
	inline vec<4, T> make_vec4(T const* const ptr)
	{
		vec<4, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(vec<4, T>));
		return Result;
	}

	template<typename T>
	inline mat<2, 2, T> make_mat2x2(T const* const ptr)
	{
		mat<2, 2, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(mat<2, 2, T>));
		return Result;
	}

	template<typename T>
	inline mat<2, 3, T> make_mat2x3(T const* const ptr)
	{
		mat<2, 3, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(mat<2, 3, T>));
		return Result;
	}

	template<typename T>
	inline mat<2, 4, T> make_mat2x4(T const* const ptr)
	{
		mat<2, 4, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(mat<2, 4, T>));
		return Result;
	}

	template<typename T>
	inline mat<3, 2, T> make_mat3x2(T const* const ptr)
	{
		mat<3, 2, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(mat<3, 2, T>));
		return Result;
	}

	template<typename T>
	inline mat<3, 3, T> make_mat3x3(T const* const ptr)
	{
		mat<3, 3, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(mat<3, 3, T>));
		return Result;
	}

	template<typename T>
	inline mat<3, 4, T> make_mat3x4(T const* const ptr)
	{
		mat<3, 4, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(mat<3, 4, T>));
		return Result;
	}

	template<typename T>
	inline mat<4, 2, T> make_mat4x2(T const* const ptr)
	{
		mat<4, 2, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(mat<4, 2, T>));
		return Result;
	}

	template<typename T>
	inline mat<4, 3, T> make_mat4x3(T const* const ptr)
	{
		mat<4, 3, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(mat<4, 3, T>));
		return Result;
	}

	template<typename T>
	inline mat<4, 4, T> make_mat4x4(T const* const ptr)
	{
		mat<4, 4, T> Result;
		memcpy(value_ptr(Result), ptr, sizeof(mat<4, 4, T>));
		return Result;
	}

	template<typename T>
	inline mat<2, 2, T> make_mat2(T const* const ptr)
	{
		return make_mat2x2(ptr);
	}

	template<typename T>
	inline mat<3, 3, T> make_mat3(T const* const ptr)
	{
		return make_mat3x3(ptr);
	}

	template<typename T>
	inline mat<4, 4, T> make_mat4(T const* const ptr)
	{
		return make_mat4x4(ptr);
	}
}