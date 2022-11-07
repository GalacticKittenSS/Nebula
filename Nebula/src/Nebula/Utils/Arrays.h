#pragma once

#include <initializer_list>
#include <algorithm>
#include <vector>

namespace Nebula {
	template<class T>
	class Array : public std::vector<T> {
	public:
		size_t find(const T& item) {
			return std::find(begin(), end(), item) - begin();
		}

		void swap(size_t a, size_t b) {
			iter_swap(begin() + a, begin() + b);
		}

		void move(size_t from, size_t to) {
			if (from >= size() || to >= size())
				return;

			if (from > to)
				std::rotate(rend() - from - 1, rend() - from, rend() - to);
			else
				std::rotate(begin() + from, begin() + from + 1, begin() + to + 1);
		}

		void reverse() {
			std::reverse(begin(), end());
		}
		
		void remove(const T& item) {
			iterator it = std::find(begin(), end(), item);
			if (it != end())
				erase(it);
		}

		void remove_index(size_t index) {
			iterator it = begin() + index;
			if (it != end())
				erase(it);
		}
	};

	template <typename T>
	std::ostream& operator<<(std::ostream& stream, const Array<T>& arr) {
		for (uint32_t i = 0; i < arr.size(); i++) {
			stream << ", " << arr[i];

			if ((i + 1) % 4 == 0)
				stream << std::endl;
		}

		if (arr.size() % 4 != 0)
			stream << std::endl;

		return stream;
	}

	template <typename T>
	std::istream& operator>>(std::istream& stream, Array<T>& arr) {
		for (uint32_t i = 0; i < arr.size(); i++)
			stream >> arr[i];

		return stream;
	}
}