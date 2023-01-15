#pragma once

namespace Nebula::Maths {
	template <typename T>
	T Min(const T& A, const T& B) {
		if (A > B)
			return B;

		return A;
	}

	template <typename T>
	T Max(const T& A, const T& B) {
		if (A < B)
			return B;

		return A;
	}
}