#ifndef VECTOR_H
#define VECTOR_H

#include <memory>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <iterator>
#include <iostream>
//#include <cstddef>

template <typename T>
struct vector {
private:
	union {
		size_t *q;
		T val;
	};
	bool small;

	void control() {
		if (!small && q != nullptr && refs() != 1) {
			size_t i = 0, n = size_();
			size_t *p = reinterpret_cast<size_t*>(operator new[] (3 * sizeof(size_t) + capacity_() * sizeof(T)));
			*p = *q;
			*(p + 1) = *(q + 1);
			*(p + 2) = 1;
			try {
				T* st = reinterpret_cast<T*>(q + 3);
				for (size_t i = 0; i < n; i++, st++)
					new (reinterpret_cast<T*>(p + 3) + i) T(*st);
			} catch (...) {
				for (size_t j = 0; j < i; j++)
					(*(reinterpret_cast<T*>(p + 3) + j)).~T();
				operator delete[] (p);
				throw;
			}
			refs()--;
			q = p;
		}
	}

	void del() noexcept {
		if (!small) {
			if (q != nullptr) {
				if (refs() == 1) {
					size_t n = size_();
					for (size_t i = 0; i < n; i++)
						(*this)[i].~T();
					operator delete[] (q);
				} else {
					refs()--;
				}
			}
		} else {
			val.~T();
		}
		small = false;
		q = nullptr;
	}

	void fig(size_t n) {
		if (n == 0) {
			del();
			return;
		}
		size_t i = 0, N = (small ? 1 : q == nullptr ? 0 : size_());
		if (n == 1) {
			if (N > 0) {
				T vw(front());
				del();
				small = true;
				val = vw;
			} else {
				del();
				small = true;
			}
			return;
		}
		size_t *p = reinterpret_cast<size_t*>(operator new[] (3 * sizeof(size_t) + n * sizeof(T)));
		try {
			control();
			*p = std::min(N, n);
			*(p + 1) = n;
			*(p + 2) = 1;
			if (small) {
				if (N > 0)
					new (reinterpret_cast<T*>(p + 3)) T(val);
			} else {
				T* st = reinterpret_cast<T*>(p + 3);
				for (i = 0; i < std::min(N, n); i++, st++)
					new (st) T((*this)[i]);
			}
			del();
			q = p;
		} catch (...) {
			for (size_t j = 0; j < i; j++)
				(*(reinterpret_cast<T*>(p + 3) + j)).~T();
			operator delete [] (p);
		}
	}
	
	void make() {
		if (!small && q != nullptr && size_() == 1) {
			T w((*this)[0]);
			del();
			push_back(w);
		}
	}

	size_t& size_() noexcept {
		return *q;
	}
	size_t& capacity_() noexcept {
		return *(q + 1);
	}
	size_t& refs() noexcept {
		return *(q + 2);
	}

public:
	typedef T value_type;
	typedef T* iterator;
	typedef const T* const_iterator;
	typedef std::reverse_iterator<T*> reverse_iterator;
	typedef std::reverse_iterator<const T*> const_reverse_iterator;

    vector() : q(nullptr), small(false) {}
	vector(vector const &w) : small(w.small) {
		if (small) {
			new (&val) T(w.val);
		} else {
			q = w.q;
			if (q != nullptr)
				refs()++;
		}
	}

    ~vector() {
		del();
	}

    vector(size_t n, T w) {
		if (n == 1) {
			small = true;
			new (&val) T(w);
		} else {
			small = false;
			if (n == 0) {
				q = nullptr;
			} else {
				q = operator new[] (3 * sizeof(size_t) + n * sizeof(T));
				size_() = n;
				capacity_() = n;
				refs() = 1;
				T* st = q + 3;
				size_t i;
				try {
					for (i = 0; i < n; i++, st++)
						new(st) T(w);
				} catch(...) {
					for (size_t j = 0; j < i; j++)
						(*this)[j].~T();
					operator delete[] (q);
					throw;
				}
			}
		}
	}

    vector &operator=(vector const &w) noexcept {
		del();
		small = w.small;
		if (small) {
			new (&val) T(w.val);
		} else {
			q = w.q;
			if (q != nullptr)
				refs()++;
		}
		return *this;
	}

    template <typename InputIterator>
    vector(InputIterator first, InputIterator last) {
		small = false;
		q = nullptr;
		while (first != last) {
			push_back(*first);
			first++;
		}
	}
    template <typename InputIterator>
    void assign(InputIterator first, InputIterator last) {
		del();
		while (first != last) {
			push_back(*first);
			first++;
		}
	}

    T& back() noexcept {
		if (small) {
			return val;
		} else {
			return (*this)[size_() - 1];
		}
	};

	T const &back() const noexcept {
		if (small) {
			return val;
		} else {
			return (*this)[size_() - 1];
		}
	}
	
	T& front() noexcept {
		if (small) {
			return val;
		} else {
			return (*this)[0];
		}
	};

	T const &front() const noexcept {
		if (small) {
			return val;
		} else {
			return (*this)[0];
		}
	}

    T &operator[](size_t i) noexcept {
		if (small)
			return val;
		control();
		return *(reinterpret_cast<T*>(q + 3) + i);
	}

    T const &operator[](size_t i) const noexcept {
		if (small)
			return val;
		control();
		return *(reinterpret_cast<T*>(q + 3) + i);
	}
	
	T* data() noexcept {
		if (small) {
			return *val;
		} else {
			control();
			if (q == nullptr)
				return nullptr;
			else
				return q + 3;
		}
	}

	const T* data() const noexcept {
		if (small) {
			return *val;
		} else {
			control();
			if (q == nullptr)
				return nullptr;
			else
				return q + 3;
		}
	}

	bool empty() const noexcept {
		return !small && q == nullptr;
	}

	size_t size() const noexcept {
		if (small)
			return 1;
		else if (q == nullptr)
			return 0;
		else
			return *q;
	}

	size_t capacity_() const noexcept {
		if (small)
			return 1;
		else if (q == nullptr)
			return 0;
		else
			return *(q + 1);
	}

	void reserve(size_t n) {
		if (n <= 1 || (!small && q != nullptr && capacity_() >= n))
			return;
		fig(n);
	}

	void shrink_to_fit() {
		if (small || (!small && q == nullptr))
			return;
		fig(size_());
	}
	
	void resize(size_t n, const T &w) {
		size_t N = (small ? 1 : q == nullptr ? 0 : *q);
		fig(std::min(N, n));
		while (N < n)
			push_back(w);
	}
	
	void clear() noexcept {
		if (small)
			small = false;
		else if (q != nullptr)
			size_() = 0;
	}

	void push_back(const T &w) {
		if (!small && q == nullptr) {
			new (&val) T(w);
			small = true;
			return;
		}
		size_t n = (small ? 1 : size_());
		if (small || capacity_() == n)
			reserve(2 * n);
		new (reinterpret_cast<T*>(q + 3) + n) T(w);
		size_()++;
	}

    void pop_back() noexcept {
		if (small || q == nullptr) {
			del();
			return;
		}
		control();
		size_()--;
		make();
	}

	iterator begin() noexcept {
		if (small)
			return &val;
		if (q == nullptr)
			return nullptr;
		return &(*this)[0];
	}

	iterator end() noexcept {
		if (small) {
			iterator it = &val;
			it++;
			return it;
		}
		if (q == nullptr)
			return nullptr;
		control();
		return reinterpret_cast<T*>(q + 3) + size_();
	}
	
	reverse_iterator rbegin() noexcept {
		if (small)
			return reverse_iterator(&val);
		if (q == nullptr)
			return reverse_iterator(nullptr);
		return reverse_iterator(&back());
	}

	reverse_iterator rend() noexcept {
		if (small) {
			iterator it = &val;
			it++;
			return reverse_iterator(it);
		}
		if (q == nullptr)
			return reverse_iterator(nullptr);
		control();
		return reinterpret_cast<T*>(q + 3) - 1;
	}
	
	iterator insert(const_iterator pos, T const& w) {
		control();
		size_t id = 0;
		const_iterator cpos = begin();
		while (cpos != pos)
			cpos++, id++;
		size_t n = (small ? 1 : q == nullptr ? 0 : size_());
		if (id == n) {
			push_back(w);
			iterator it = end();
			it--;
			return it;
		}
		size_t *p = reinterpret_cast<size_t*>(operator new[] (3 * sizeof(size_t) + (n + 1) * sizeof(T)));
		*p = n + 1;
		*(p + 1) = *p;
		*(p + 2) = 1;
		size_t i;
		try {
			for (i = 0; i < id; i++)
				new (reinterpret_cast<T*>(p + 3) + i) T((*this)[i]);
			new (reinterpret_cast<T*>(p + 3) + id) T(w);
			for (i = id; i < n; i++)
				new (reinterpret_cast<T*>(p + 3) + i + 1) T((*this)[i]);
			del();
			q = p;
		} catch (...) {
			for (size_t j = 0; j < i; j++)
				(*(reinterpret_cast<T*>(p + 3) + j)).~T();
			operator delete[] (p);
			throw;
		}
		make();
		return begin() + id;
	}

	iterator erase(const_iterator pos) {
		return erase(pos, pos + 1);
	}

	iterator erase(const_iterator L, const_iterator R) {
		control();
		size_t le = 0, re = 0;
		const_iterator cpos = begin();
		while (cpos != L)
			cpos++, le++;
		re = le;
		while (cpos != R)
			cpos++, re++;
		size_t n = (small ? 1 : q == nullptr ? 0 : size_());
		if (re - le == n) {
			del();
			return begin();
		}
		size_t *p = reinterpret_cast<size_t*>(operator new[] (3 * sizeof(size_t) + (n - (re - le)) * sizeof(T)));
		*p = n - (re - le);
		*(p + 1) = *p;
		*(p + 2) = 1;
		size_t i;
		try {
			for (i = 0; i < le; i++)
				new (reinterpret_cast<T*>(p + 3) + i) T((*this)[i]);
			for (i = re; i < n; i++)
				new (reinterpret_cast<T*>(p + 3) + i - (re - le)) T((*this)[i]);
			del();
			q = p;
		} catch (...) {
			if (i >= re)
				i -= re - le;
			for (size_t j = 0; j < i; j++)
				(*(reinterpret_cast<T*>(p + 3) + j)).~T();
			operator delete[] (p);
			throw;
		}
		make();
		return begin() + le;
	}
	
	void swap(vector &w) {
		if (small) {
			if (w.small) {
				std::swap(val, w.val);
			} else { 
				T s(val);
				val.~T();
				swap(q, w.q);
				new(&w.val) T(s);
				swap(small, w.small);
			}
		} else {
			if (w.small) {
				T s(w.val);
				w.val.~T();
				swap(q, w.q);
				new(&val) T(s);
				swap(small, w.small);
			} else {
				swap(q, w.q);
			}
		}
	}
	bool operator ==(vector const &w) noexcept {
		size_t x = (small ? 1 : q == nullptr ? 0 : size_()), y = w.size();
		if (x != y)
			return false;
		for (size_t i = 0; i < x; i++)
			if ((*this)[i] != w[i])
				return false;
		return true;
	}
	bool operator !=(vector const &w) noexcept {
		return !(*this == w);
	}
	bool operator <(vector const &w) noexcept {
		size_t x = (small ? 1 : q == nullptr ? 0 : size_()), y = w.size();
		for (size_t i = 0; i < std::min(x, y); i++)
			if ((*this)[i] != w[i])
				return (*this)[i] < w[i];
		return x < y;
	}
	bool operator >(vector const &w) noexcept {
		return w < *this;
	}
	bool operator <=(vector const &w) noexcept {
		return !(*this > w);
	}
	bool operator >=(vector const &w) noexcept {
		return !(*this < w);
	}
};

#endif // VECTOR_H
