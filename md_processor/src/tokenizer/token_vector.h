#ifndef token_vector_h
#define token_vector_h

#include <iterator>
#include <string>
#include <vector>

/**
 * @brief Token container support class aimed at higher performance.
 *
 * Wraps up awkward iteration types for use in the parser.
 * Splits a delimited line into a vector of indices.
 * Each index holds a pointer to the place in the string where the token is found.
 * It tries to minimise iteration passes of the line in one construction and does
 * not copy the original parts of the string.
 *
 * Exposes iterator class for use with token content
 */
template <typename T, T delim=',', class A = std::allocator<T*>>
struct token_vector
{
    typedef A allocator_type;
    typedef typename A::value_type value_type;
    typedef typename A::reference reference;
    typedef typename A::const_reference const_reference;
    typedef typename A::difference_type difference_type;
    typedef typename A::size_type size_type;
    typedef typename A::const_pointer const_pointer;
    typedef typename A::pointer pointer;

    typedef std::vector<value_type> value_index_type;
    typedef typename value_index_type::iterator value_index_type_iter;

     class iterator {
     public:
         typedef typename A::difference_type difference_type;
         typedef typename A::value_type value_type;
         typedef typename A::reference reference;
         typedef typename A::pointer pointer;
         typedef std::random_access_iterator_tag iterator_category;

         iterator () = delete;
         iterator (const iterator&) = default;
         ~iterator() = default;

         explicit iterator(value_index_type_iter it) : current(it) {}

         iterator& operator=(const iterator&) = default;

         bool operator==(const iterator& rhs) const {
         	return current == rhs.current;
         }

         bool operator!=(const iterator& rhs) const {
         	return (current < rhs.current) ? -1 : (rhs.current < current) ? +1 : 0;
         }

         iterator& operator ++()
         {
             ++current;
             return *this;
         }

         iterator operator ++(int)
         {
         	iterator  tmp(*this);
             operator ++();
             return tmp;
         }

         iterator& operator --()
         {
             --current;
             return *this;
         }

         iterator operator --(int)
         {
         	iterator  tmp(*this);
            operator --();
            return tmp;
         }

         iterator& operator+=(size_type n) {
         	current+=n;
         	return *this;
         }

         iterator operator+(size_type n) const {
         	return iterator(current+n);
         }

         iterator& operator-=(size_type n) {
            current+=n;
            return *this;
         }

         iterator operator-(size_type n) const {
         	return iterator(current-n);
         }

         difference_type operator-(iterator rhs) const {
         	return current-rhs.current;
         }

         reference operator*() const noexcept {
         	return *current;
         }

         pointer operator->() const noexcept {
         	return &(operator*());
         }

         reference operator[](size_type i) const {
         	return current[i];
         }

     private:
         value_index_type_iter current;
     };

     typedef std::reverse_iterator<iterator> reverse_iterator;

     token_vector(std::string &line) {
    	std::string::iterator end = line.end();
    	std::string::iterator c = line.begin();
    	token_index.push_back(&c[0]);
    	for (; c != end; ++c)
			if (*c == delim) {
				 token_index.push_back(&c[1]);
				 *c = '\0';
			}
    }

    iterator begin() noexcept {
    	return iterator(token_index.begin());
    }

    iterator end() noexcept {
    	return iterator(token_index.end());
    }

    reverse_iterator rbegin() noexcept {
    	return reverse_iterator(token_index.rbegin());
    }

    reverse_iterator rend() noexcept {
    	return reverse_iterator(token_index.rend());
    }

    reference front() noexcept {
    	return token_index.front();
    }

    reference back() noexcept {
    	return token_index.back();
    }

    reference operator[](size_type i) {
    	return token_index[i];
    }

    reference at(size_type i) {
    	return token_index.at(i);
    }

	private:
    	value_index_type token_index;
};

#endif
