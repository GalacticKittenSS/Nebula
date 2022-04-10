#pragma once

#include <initializer_list>

namespace Nebula {
	template<class T>
	class Array {
	public:
		typedef T* iterator;
		typedef const T* const_iterator;
		typedef size_t count_t;

		Array() = default;
		Array(const count_t& size) : head(size > 0 ? new T[size] : nullptr), elems(size) { }
		Array(const Array<T>& arr) {
			head  = arr.head;
			elems = arr.elems;
		}
		Array(const T arr[]) {
			elems = sizeof(arr) / sizeof(T);
			head = new T[elems];
			
			iterator pointer = begin();
			for (const_iterator i = &arr[0]; i != &arr[0] + elems; i++)
				*pointer++ = *i;
		}
		Array(std::initializer_list<T> values) {
			elems = values.size();
			head = new T[elems];
			
			iterator pointer = begin();
			for (const T& i : values)
				*pointer++ = i;
		}

		void push_back(const T& value) {
			T* copy = head;
			head = new T[elems + 1];
			
			for (count_t i = 0; i < size(); i++) { head[i] = copy[i]; }
			delete[] copy;
			
			head[elems] = value;
			elems++;
		}

		bool remove_index(count_t index) {
			T* newArr = new T[elems];

			count_t newI = 0;
			bool found = false;
			for (count_t i = 0; i < size(); i++) {
				if (i != index) {
					newArr[newI] = head[i];
					newI++;
				} else {
					found = true;
				}
			}

			if (found) {
				head = newArr;
				elems--;
			}

			return found;
		}

		bool remove(const T& value) {
			T* newArr = new T[elems];

			uint32_t newI = 0;
			bool found = false;
			for (uint32_t i = 0; i < size(); i++) {
				if (head[i] != value) {
					newArr[newI] = head[i];
					newI++;
				} else {
					found = true;
				}
			}

			if (found) {
				head = newArr;
				elems--;
			}

			return found;
		}

		void move(count_t from, count_t to) {
			T value = head[from];
			
			if (from < to) {
				for (count_t i = from; i < to; i++)
					head[i] = head[i + 1];
			} else if (from > to) {
				for (count_t i = from; i > to; i--)
					head[i] = head[i - 1];
			}
			
			head[to] = value;
		}

		void swap(count_t a, count_t b) {
			T value = head[b];

			head[b] = head[a];
			head[a] = value;
		}

		count_t FindIndex(const T& e) {
			for (uint32_t i = 0; i < elems; i++) {
				if (head[i] == e)
					return i;
			}

			return FLT_MAX;
		}

		count_t size() { return elems; }
		const count_t size() const { return elems; }

		iterator begin() const { return head; }
		iterator end() const { return head != nullptr ? &head[elems] : nullptr; }

		T* data() { return head; }

		void clear() { head = nullptr; elems = 0; }
		void resize(count_t size) { head = new T[size]; elems = 0; }
		bool empty() { return elems == 0; }

		T& getIndex(count_t index) {
			if (index < 0 || index > size())
				NB_ERROR("Index {0} is Greater than the Size of Array", index);

			return head[index];
		}

		const T& getIndex(count_t index) const {
			if (index < 0 || index > size())
				NB_ERROR("Index {0} is Greater than the Size of Array", index);

			return head[index];
		}

		T& operator[](count_t index) { return getIndex(index); }
		const T& operator[](count_t index) const { return getIndex(index); }
		
		bool operator==(const Array<T>& right) const {
			if (size() != right.size())
				return false;

			for (const_iterator i = 0; i < size(); i++) {
				if (this[i] != right[i])
					return false;
			}

			return true;
		}
		bool operator!=(const Array<T>& right) const { return !(this == right); }

		Array<T> operator=(const Array<T>& right) {
			if (this != &right) {
				delete[] head;

				head  = right.head;
				elems = right.elems;
			}
			return *this;
		}
	private:
		iterator head = nullptr;
		count_t elems = 0;
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