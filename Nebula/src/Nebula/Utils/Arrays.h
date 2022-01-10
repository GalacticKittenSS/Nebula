#pragma once

#include <initializer_list>

namespace Nebula {
	template<class T>
	class Array {
	public:
		Array() = default;

		Array(std::initializer_list<T> values) {
			for (auto v : values)
				push_back(v);

			_first = &mem[0];
		}

		void push_back(const T& value) {
			T* newArr = new T[count + 1];
			
			for (uint32_t i = 0; i < count; i++)
				newArr[i] = mem[i];

			newArr[count] = value;
			mem = newArr;
			count++;

			_last = &mem[count];
		}

		bool remove_index(uint32_t index) {
			T* newArr = new T[count];

			uint32_t newI = 0;
			for (uint32_t i = 0; i < count; i++) {
				if (i != index) {
					newArr[newI] = mem[i];
					newI++;
				}
			}

			if (found) {
				mem = newArr;
				count--;

				_first = &mem[0];
				_last = &mem[count];
			}

			return found;
		}

		bool remove(const T& value) {
			T* newArr = new T[count];

			uint32_t newI = 0;
			bool found = false;
			for (uint32_t i = 0; i < count; i++) {
				if (mem[i] != value) {
					newArr[newI] = mem[i];
					newI++;
				}
				else {
					found = true;
				}
			}

			if (found) {
				mem = newArr;
				count--;
			
				_first = &mem[0];
				_last =  &mem[count];
			}

			return found;
		}

		void move(uint32_t from, uint32_t to) {
			T value = mem[from];
			
			if (from < to) {
				for (uint32_t i = from; i < to; i++)
					mem[i] = mem[i + 1];
			} else if (from > to) {
				for (uint32_t i = from; i > to; i--)
					mem[i] = mem[i - 1];
			}
			
			mem[to] = value;
		}

		void swap(uint32_t a, uint32_t b) {
			T value = mem[b];

			mem[b] = mem[a];
			mem[a] = value;
		}

		uint32_t size() { return count; }

		_NODISCARD constexpr const T* begin() const noexcept { return _first; }
		_NODISCARD constexpr const T* end() const noexcept { return _last; }

		T* data() { return mem; }

		void clear() { mem = new T[32]; count = 0; }
		void resize(uint32_t size) { mem = new T[size]; count = 0; }

		bool empty() { return count == 0; }

		T  operator[](uint32_t index) { return mem[index]; }
	private:
		T* mem = new T[32];
		T* _first = nullptr;
		T* _last = nullptr;
		uint32_t count = 0;
	};
}