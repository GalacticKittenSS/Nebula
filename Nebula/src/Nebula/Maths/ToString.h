#pragma once

#include "Basic.h"

#include <string>
#include <cstdarg>
#include <cstdio>

namespace Nebula {
	template <typename T>
	struct cast {
		typedef T value_type;
	};

	template <>
	struct cast<float> {
		typedef double value_type;
	};

	inline std::string format(const char* msg, ...) {
		std::size_t const STRING_BUFFER(4096);
		char text[STRING_BUFFER];
		va_list list;

		if (msg == nullptr)
			return std::string();

		va_start(list, msg);
		std::vsprintf(text, msg, list);
		va_end(list);

		return std::string(text);
	}

	static const char* LabelTrue = "true";
	static const char* LabelFalse = "false";

	template<typename T, bool isFloat = false>
	struct literal
	{
		inline static char const* value() { return "%d"; }
	};

	template<typename T>
	struct literal<T, true>
	{
		inline static char const* value() { return "%f"; }
	};

	template<typename T>
	struct prefix {};

	template<>
	struct prefix<float>
	{
		inline static char const* value() { return ""; }
	};

	template<>
	struct prefix<double>
	{
		inline static char const* value() { return "d"; }
	};

	template<>
	struct prefix<bool>
	{
		inline static char const* value() { return "b"; }
	};

	template<>
	struct prefix<uint8_t>
	{
		inline static char const* value() { return "u8"; }
	};

	template<>
	struct prefix<int8_t>
	{
		inline static char const* value() { return "i8"; }
	};

	template<>
	struct prefix<uint16_t>
	{
		inline static char const* value() { return "u16"; }
	};

	template<>
	struct prefix<int16_t>
	{
		inline static char const* value() { return "i16"; }
	};

	template<>
	struct prefix<uint32_t>
	{
		inline static char const* value() { return "u"; }
	};

	template<>
	struct prefix<int32_t>
	{
		inline static char const* value() { return "i"; }
	};

	template<>
	struct prefix<uint64_t>
	{
		inline static char const* value() { return "u64"; }
	};

	template<>
	struct prefix<int64_t>
	{
		inline static char const* value() { return "i64"; }
	};

	template<typename matType>
	struct compute_to_string
	{};

	template<>
	struct compute_to_string<vec<1, bool> >
	{
		inline static std::string call(vec<1, bool> const& x)
		{
			return format("bvec1(%s)",
				x[0] ? LabelTrue : LabelFalse);
		}
	};

	template<>
	struct compute_to_string<vec<2, bool> >
	{
		inline static std::string call(vec<2, bool> const& x)
		{
			return format("bvec2(%s, %s)",
				x[0] ? LabelTrue : LabelFalse,
				x[1] ? LabelTrue : LabelFalse);
		}
	};

	template<>
	struct compute_to_string<vec<3, bool> >
	{
		inline static std::string call(vec<3, bool> const& x)
		{
			return format("bvec3(%s, %s, %s)",
				x[0] ? LabelTrue : LabelFalse,
				x[1] ? LabelTrue : LabelFalse,
				x[2] ? LabelTrue : LabelFalse);
		}
	};

	template<>
	struct compute_to_string<vec<4, bool> >
	{
		inline static std::string call(vec<4, bool> const& x)
		{
			return format("bvec4(%s, %s, %s, %s)",
				x[0] ? LabelTrue : LabelFalse,
				x[1] ? LabelTrue : LabelFalse,
				x[2] ? LabelTrue : LabelFalse,
				x[3] ? LabelTrue : LabelFalse);
		}
	};

	template<typename T>
	struct compute_to_string<vec<1, T> >
	{
		inline static std::string call(vec<1, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%svec1(%s)",
				PrefixStr,
				LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0]));
		}
	};

	template<typename T>
	struct compute_to_string<vec<2, T> >
	{
		inline static std::string call(vec<2, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%svec2(%s, %s)",
				PrefixStr,
				LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0]),
				static_cast<typename cast<T>::value_type>(x[1]));
		}
	};

	template<typename T>
	struct compute_to_string<vec<3, T> >
	{
		inline static std::string call(vec<3, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%svec3(%s, %s, %s)",
				PrefixStr,
				LiteralStr, LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0]),
				static_cast<typename cast<T>::value_type>(x[1]),
				static_cast<typename cast<T>::value_type>(x[2]));
		}
	};

	template<typename T>
	struct compute_to_string<vec<4, T> >
	{
		inline static std::string call(vec<4, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%svec4(%s, %s, %s, %s)",
				PrefixStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0]),
				static_cast<typename cast<T>::value_type>(x[1]),
				static_cast<typename cast<T>::value_type>(x[2]),
				static_cast<typename cast<T>::value_type>(x[3]));
		}
	};


	template<typename T>
	struct compute_to_string<mat<2, 2, T> >
	{
		inline static std::string call(mat<2, 2, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%smat2x2((%s, %s), (%s, %s))",
				PrefixStr,
				LiteralStr, LiteralStr,
				LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0][0]), static_cast<typename cast<T>::value_type>(x[0][1]),
				static_cast<typename cast<T>::value_type>(x[1][0]), static_cast<typename cast<T>::value_type>(x[1][1]));
		}
	};

	template<typename T>
	struct compute_to_string<mat<2, 3, T> >
	{
		inline static std::string call(mat<2, 3, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%smat2x3((%s, %s, %s), (%s, %s, %s))",
				PrefixStr,
				LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0][0]), static_cast<typename cast<T>::value_type>(x[0][1]), static_cast<typename cast<T>::value_type>(x[0][2]),
				static_cast<typename cast<T>::value_type>(x[1][0]), static_cast<typename cast<T>::value_type>(x[1][1]), static_cast<typename cast<T>::value_type>(x[1][2]));
		}
	};

	template<typename T>
	struct compute_to_string<mat<2, 4, T> >
	{
		inline static std::string call(mat<2, 4, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%smat2x4((%s, %s, %s, %s), (%s, %s, %s, %s))",
				PrefixStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0][0]), static_cast<typename cast<T>::value_type>(x[0][1]), static_cast<typename cast<T>::value_type>(x[0][2]), static_cast<typename cast<T>::value_type>(x[0][3]),
				static_cast<typename cast<T>::value_type>(x[1][0]), static_cast<typename cast<T>::value_type>(x[1][1]), static_cast<typename cast<T>::value_type>(x[1][2]), static_cast<typename cast<T>::value_type>(x[1][3]));
		}
	};

	template<typename T>
	struct compute_to_string<mat<3, 2, T> >
	{
		inline static std::string call(mat<3, 2, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%smat3x2((%s, %s), (%s, %s), (%s, %s))",
				PrefixStr,
				LiteralStr, LiteralStr,
				LiteralStr, LiteralStr,
				LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0][0]), static_cast<typename cast<T>::value_type>(x[0][1]),
				static_cast<typename cast<T>::value_type>(x[1][0]), static_cast<typename cast<T>::value_type>(x[1][1]),
				static_cast<typename cast<T>::value_type>(x[2][0]), static_cast<typename cast<T>::value_type>(x[2][1]));
		}
	};

	template<typename T>
	struct compute_to_string<mat<3, 3, T> >
	{
		inline static std::string call(mat<3, 3, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%smat3x3((%s, %s, %s), (%s, %s, %s), (%s, %s, %s))",
				PrefixStr,
				LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0][0]), static_cast<typename cast<T>::value_type>(x[0][1]), static_cast<typename cast<T>::value_type>(x[0][2]),
				static_cast<typename cast<T>::value_type>(x[1][0]), static_cast<typename cast<T>::value_type>(x[1][1]), static_cast<typename cast<T>::value_type>(x[1][2]),
				static_cast<typename cast<T>::value_type>(x[2][0]), static_cast<typename cast<T>::value_type>(x[2][1]), static_cast<typename cast<T>::value_type>(x[2][2]));
		}
	};

	template<typename T>
	struct compute_to_string<mat<3, 4, T> >
	{
		inline static std::string call(mat<3, 4, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%smat3x4((%s, %s, %s, %s), (%s, %s, %s, %s), (%s, %s, %s, %s))",
				PrefixStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0][0]), static_cast<typename cast<T>::value_type>(x[0][1]), static_cast<typename cast<T>::value_type>(x[0][2]), static_cast<typename cast<T>::value_type>(x[0][3]),
				static_cast<typename cast<T>::value_type>(x[1][0]), static_cast<typename cast<T>::value_type>(x[1][1]), static_cast<typename cast<T>::value_type>(x[1][2]), static_cast<typename cast<T>::value_type>(x[1][3]),
				static_cast<typename cast<T>::value_type>(x[2][0]), static_cast<typename cast<T>::value_type>(x[2][1]), static_cast<typename cast<T>::value_type>(x[2][2]), static_cast<typename cast<T>::value_type>(x[2][3]));
		}
	};

	template<typename T>
	struct compute_to_string<mat<4, 2, T> >
	{
		inline static std::string call(mat<4, 2, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%smat4x2((%s, %s), (%s, %s), (%s, %s), (%s, %s))",
				PrefixStr,
				LiteralStr, LiteralStr,
				LiteralStr, LiteralStr,
				LiteralStr, LiteralStr,
				LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0][0]), static_cast<typename cast<T>::value_type>(x[0][1]),
				static_cast<typename cast<T>::value_type>(x[1][0]), static_cast<typename cast<T>::value_type>(x[1][1]),
				static_cast<typename cast<T>::value_type>(x[2][0]), static_cast<typename cast<T>::value_type>(x[2][1]),
				static_cast<typename cast<T>::value_type>(x[3][0]), static_cast<typename cast<T>::value_type>(x[3][1]));
		}
	};

	template<typename T>
	struct compute_to_string<mat<4, 3, T> >
	{
		inline static std::string call(mat<4, 3, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%smat4x3((%s, %s, %s), (%s, %s, %s), (%s, %s, %s), (%s, %s, %s))",
				PrefixStr,
				LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0][0]), static_cast<typename cast<T>::value_type>(x[0][1]), static_cast<typename cast<T>::value_type>(x[0][2]),
				static_cast<typename cast<T>::value_type>(x[1][0]), static_cast<typename cast<T>::value_type>(x[1][1]), static_cast<typename cast<T>::value_type>(x[1][2]),
				static_cast<typename cast<T>::value_type>(x[2][0]), static_cast<typename cast<T>::value_type>(x[2][1]), static_cast<typename cast<T>::value_type>(x[2][2]),
				static_cast<typename cast<T>::value_type>(x[3][0]), static_cast<typename cast<T>::value_type>(x[3][1]), static_cast<typename cast<T>::value_type>(x[3][2]));
		}
	};

	template<typename T>
	struct compute_to_string<mat<4, 4, T> >
	{
		inline static std::string call(mat<4, 4, T> const& x)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%smat4x4((%s, %s, %s, %s), (%s, %s, %s, %s), (%s, %s, %s, %s), (%s, %s, %s, %s))",
				PrefixStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(x[0][0]), static_cast<typename cast<T>::value_type>(x[0][1]), static_cast<typename cast<T>::value_type>(x[0][2]), static_cast<typename cast<T>::value_type>(x[0][3]),
				static_cast<typename cast<T>::value_type>(x[1][0]), static_cast<typename cast<T>::value_type>(x[1][1]), static_cast<typename cast<T>::value_type>(x[1][2]), static_cast<typename cast<T>::value_type>(x[1][3]),
				static_cast<typename cast<T>::value_type>(x[2][0]), static_cast<typename cast<T>::value_type>(x[2][1]), static_cast<typename cast<T>::value_type>(x[2][2]), static_cast<typename cast<T>::value_type>(x[2][3]),
				static_cast<typename cast<T>::value_type>(x[3][0]), static_cast<typename cast<T>::value_type>(x[3][1]), static_cast<typename cast<T>::value_type>(x[3][2]), static_cast<typename cast<T>::value_type>(x[3][3]));
		}
	};


	template<typename T>
	struct compute_to_string<qua<T> >
	{
		inline static std::string call(qua<T> const& q)
		{
			char const* PrefixStr = prefix<T>::value();
			char const* LiteralStr = literal<T, std::numeric_limits<T>::is_iec559>::value();
			std::string FormatStr(format("%squat(%s, {%s, %s, %s})",
				PrefixStr,
				LiteralStr, LiteralStr, LiteralStr, LiteralStr));

			return format(FormatStr.c_str(),
				static_cast<typename cast<T>::value_type>(q.w),
				static_cast<typename cast<T>::value_type>(q.x),
				static_cast<typename cast<T>::value_type>(q.y),
				static_cast<typename cast<T>::value_type>(q.z));
		}
	};

	template<class matType>
	inline std::string to_string(matType const& x)
	{
		return compute_to_string<matType>::call(x);
	}

}//namespace glm
