#ifndef md_helper_h
#define md_helper_h

#include <memory>

/**
 * Missing in c++11 wait for c++14
 *
 * @param args
 * @return
 */
namespace std {
template<typename T, typename... Args>
	unique_ptr<T> make_unique(Args&&... args)
	{
		return unique_ptr<T>(new T(forward<Args>(args)...));
	}
}

/**
 * Check if items a and b match
 *
 * @param a
 * @param b
 * @return the match boolean
 */
template <typename T,typename First>
bool contains(T a, First b) {
	return static_cast<First>(a) == b;
}

/**
 * Variadic helper with takes and expands variable
 * lists of items we with to compare with the reference item
 *
 * @param a reference
 * @param b first in remaining list
 * @param rest the rest of the list
 * @return combined truth to see if a is b or in rest
 */
template <typename T, typename First, typename...Rest>
bool contains(T a, First b, Rest... rest) {
	return contains(a,b) || contains(a,rest...);
}

/**
 * Support for constexpr strings
 */
class str_const { // constexpr string
private:
	const char* const p_;
	const std::size_t sz_;
public:
	template<std::size_t N>
	constexpr str_const(const char (&a)[N]) : // ctor
			p_(a), sz_(N - 1) {
	}
	constexpr char operator[](std::size_t n) { // []
		return n < sz_ ? p_[n] : throw std::out_of_range("");
	}
	constexpr std::size_t size() {
		return sz_;
	} // size()
};

/**
 * Helper to reverse iteration without using rend, rbegin
 */
template<typename T>
struct reverse_range {
private:
	T& x_;

public:
	reverse_range(T& x) :
			x_(x) {
	}

	auto begin() const -> decltype (this->x_.rbegin ()) {
		return x_.rbegin();
	}

	auto end() const -> decltype (this->x_.rend ()) {
		return x_.rend();
	}
};

template<typename T>
reverse_range<T> reverse_iterate(T& x) {
	return reverse_range<T>(x);
}

// Branch prediction
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#endif
