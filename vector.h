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

	void del() {
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

	void fig() {
		fig(capacity());
	}

	void fig(size_t n) {
		if (n == 0) {
			del();
			return;
		}
		if (n == capacity() && (small || (q != nullptr && refs() == 1)))
			return;
		size_t N = size();
		if (n == 1) {
			if (N > 0) {
				T vw(front());
				vector q1(*this);
				del();
				try {
					new (&val) T(vw);
					small = true;
				} catch (...) {
					q = nullptr;
					*this = q1;
					throw;
				}
			} else {
				del();
			}
			return;
		}
		size_t i = 0, *p = reinterpret_cast<size_t*>(operator new[] (3 * sizeof(size_t) + n * sizeof(T)));
		*p = std::min(N, n);
		*(p + 1) = n;
		*(p + 2) = 1;
		try {
			if (small) {
				new (reinterpret_cast<T*>(p + 3)) T(val);
			} else {
				T* st = reinterpret_cast<T*>(p + 3);
				for (i = 0; i < std::min(N, n); i++, st++)
					new (st) T(this->at(i));
			}
			del();
			q = p;
		} catch (...) {
			for (size_t j = 0; j < i; j++)
				(*(reinterpret_cast<T*>(p + 3) + j)).~T();
			operator delete [] (p);
			throw;
		}
	}

	T at(size_t i) const {
		if (small)
			return val;
		return *(reinterpret_cast<T*>(q + 3) + i);
	}

	size_t& size_() const noexcept {
		return *q;
	}
	size_t& capacity_() const noexcept {
		return *(q + 1);
	}
	size_t& refs() const noexcept {
		return *(q + 2);
	}

public:
	typedef T value_type;
	typedef T* iterator;
	typedef const T* const_iterator;
	typedef std::reverse_iterator<T*> reverse_iterator;
	typedef std::reverse_iterator<const T*> const_reverse_iterator;

    vector() noexcept : q(nullptr), small(false) {}
	vector(vector const &w) : small(w.small) {
		if (small) {
			try {
				new (&val) T(w.val);
			} catch (...) {
				small = false;
				q = nullptr;
				throw;
			}
		} else {
			q = w.q;
			if (q != nullptr)
				refs()++;
		}
	}

    ~vector() {
		del();
	}

    vector(size_t n, T w) : q(nullptr), small(false) {
		if (n == 1) {
			small = true;
			try {
				new (&val) T(w);
			} catch (...) {
				small = false;
				q = nullptr;
				throw;
			}
		} else {
			fig(n);
			for (size_t i = 0; i < n; i++)
				push_back(w);
		}
	}

    vector &operator=(vector const &w) {
		if (!w.small) {
			del();
			q = w.q;
			if (q != nullptr)
				refs()++;
		} else if (small) {
			val = w.val;
		} else {
			vector q1(*this);
			del();
			try {
				new (&val) T(w.val);
				small = true;
			} catch (...) {
				*this = q1;
				throw;
			}
		}
		return *this;
	}

    template <typename InputIterator>
    vector(InputIterator first, InputIterator last) : q(nullptr), small(false) {
		while (first != last) {
			push_back(*first);
			first++;
		}
	}
    template <typename InputIterator>
    void assign(InputIterator first, InputIterator last) {
		vector w(first, last);
		*this = w;
	}

    T& back() {
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
	
	T& front() {
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

    T &operator[](size_t i) {
		if (small)
			return val;
		fig();
		return *(reinterpret_cast<T*>(q + 3) + i);
	}

    T const &operator[](size_t i) const noexcept {
		if (small)
			return val;
		return *(reinterpret_cast<T*>(q + 3) + i);
	}
	
	T* data() {
		return begin();
	}

	const T* data() const noexcept {
		return begin();
	}

	bool empty() const noexcept {
		return !small && q == nullptr;
	}

	size_t size() const noexcept {
		return small ? 1 : q == nullptr ? 0 : *q;
	}

	size_t capacity() const noexcept {
		return small ? 1 : q == nullptr ? 0 : *(q + 1);
	}

	void reserve(size_t n) {
		if (n <= 1 || capacity() >= n)
			return;
		fig(n);
	}

	void shrink_to_fit() {
		if (capacity() == size())
			return;
		fig(size());
	}

	void resize(size_t n, const T w) {
		if (n <= size()) {
			fig(n);
			return;
		}
		vector g;
		g.reserve(n);
		for (size_t i = 0; i < size(); i++)
			g.push_back((*this)[i]);
		while (g.size() < n)
			g.push_back(w);
		*this = g;
	}

	void clear() noexcept {
		del();
	}

	void push_back(const T w) {
		fig();
		if (!small && q == nullptr) {
			small = true;
			try {
				new (&val) T(w);
			} catch (...) {
				q = nullptr;
				small = false;
				throw;
			}
			return;
		}
		size_t n = size();
		if (small || capacity() == n)
			reserve(2 * n);
		new (reinterpret_cast<T*>(q + 3) + n) T(w);
		size_()++;
	}

    void pop_back() {
		if (small || q == nullptr) {
			del();
			return;
		}
		fig();
		(*this)[size_() - 1].~T();
		size_()--;
		if (size_() == 0)
			del();
	}

	iterator begin() {
		fig();
		return small ? &val : q == nullptr ? nullptr : &(*this)[0];
	}

	iterator end() {
		fig();
		if (small) {
			iterator it = &val;
			it++;
			return it;
		}
		return q == nullptr ? nullptr : reinterpret_cast<T*>(q + 3) + size_();
	}
	
	reverse_iterator rbegin() {
		return reverse_iterator(end());
	}

	reverse_iterator rend() {
		return reverse_iterator(begin());
	}
	
	const_iterator begin() const noexcept {
		return const_iterator(small ? &val : q == nullptr ? nullptr : &(*this)[0]);
	}
	
	const_iterator end() const noexcept {
		if (small) {
			const_iterator it = &val;
			it++;
			return it;
		}
		return const_iterator(q == nullptr ? nullptr : reinterpret_cast<T*>(q + 3) + size_());
	}
	
	const_reverse_iterator rbegin() const noexcept {
		return const_reverse_iterator(end());
	}

	const_reverse_iterator rend() const noexcept {
		return const_reverse_iterator(begin());
	}

	
	iterator insert(const_iterator pos, T const w) {
		fig();
		size_t id = 0;
		const_iterator cpos = begin();
		while (cpos != pos)
			cpos++, id++;
		size_t n = (small ? 1 : q == nullptr ? 0 : size_());
		size_t *p = reinterpret_cast<size_t*>(operator new[] (3 * sizeof(size_t) + (n + 1) * sizeof(T)));
		*p = n + 1;
		*(p + 1) = *p;
		*(p + 2) = 1;
		size_t i;
		try {
			for (i = 0; i < id; i++)
				new (reinterpret_cast<T*>(p + 3) + i) T((*this)[i]);
			new (reinterpret_cast<T*>(p + 3) + id) T(w);
			for (i = id + 1; i <= n; i++)
				new (reinterpret_cast<T*>(p + 3) + i) T((*this)[i - 1]);
			del();
			q = p;
		} catch (...) {
			for (size_t j = 0; j < i; j++)
				(*(reinterpret_cast<T*>(p + 3) + j)).~T();
			operator delete[] (p);
			throw;
		}
		return begin() + id;
	}

	iterator erase(const_iterator pos) {
		return erase(pos, pos + 1);
	}

	iterator erase(const_iterator L, const_iterator R) {
		fig();
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
		return begin() + le;
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
	template<typename U>
	friend bool operator <(vector<U> const &a, vector<U> const &b) noexcept;
	template<typename U>
	friend bool operator <=(vector<U> const &a, vector<U> const &b) noexcept;
	template<typename U>
	friend bool operator >(vector<U> const &a, vector<U> const &b) noexcept;
	template<typename U>
	friend bool operator >=(vector<U> const &a, vector<U> const &b) noexcept;
	template<typename U>
	friend void swap(vector<U> &a, vector<U> &b);
};
template<typename T>
bool operator <(vector<T> const &a, vector<T> const &b) noexcept {
	size_t x = a.size(), y = b.size();
	for (size_t i = 0; i < std::min(x, y); i++)
		if (a[i] != b[i])
			return a[i] < b[i];
	return x < y;
}

template<typename T>
bool operator >(vector<T> const &a, vector<T> const &b) noexcept {
	return b < a;
}
template<typename T>
bool operator <=(vector<T> const &a, vector<T> const &b) noexcept {
	return !(a > b);
}
template<typename T>
bool operator >=(vector<T> const &a, vector<T> const &b) noexcept {
	return !(a < b);
}
template<typename T>
void swap(vector<T> &a, vector<T> &b) {
	if (a.small) {
		if (b.small) {
			using namespace std;
			swap(a.val, b.val);
		} else {
			vector<T> c(b);
			try {
				new (&b.val) T(a.val);
			} catch (...) {
				b.q = c.q;
				throw;
			}
			a.val.~T();
			b.small = true;
			a.q = c.q;
			a.small = false;
		}
	} else {
		if (b.small) {
			vector<T> c(a);
			try {
				new (&a.val) T(b.val);
			} catch (...) {
				a.q = c.q;
				throw;
			}
			b.val.~T();
			a.small = true;
			b.q = c.q;
			b.small = false;
		} else {
			std::swap(a.q, b.q);
		}
	}
}

#endif // VECTOR_H
