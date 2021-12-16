#pragma once

typedef int length_t;

namespace Nebula {
	template <length_t l, typename T> struct vec;
	template <length_t R, length_t C, typename T> struct mat;
	template <typename T> struct qua;
}