#pragma once

#ifndef ALLOCATOR_H
#define ALLOCATOR_H


#include <stddef.h>
#include <new>
#include <stdexcept>
#include <stdlib.h>
#include <memory>


template<class T, int align_bytes>
class allocator_aligned
{

public:

	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	// пустые ctor, copy ctor, rebind copy ctor и dtor для stateless аллокатора
	allocator_aligned() throw() {}

	allocator_aligned(const allocator_aligned<T, align_bytes>&) throw() {}

	template<class U, int align_bytes>
	allocator_aligned(const allocator_aligned<U, align_bytes>&) throw() {}

	~allocator_aligned() {}

	/*
	pointer allocate(const size_type& n, const void* hint = 0) const
	{
		return reinterpret_cast< __declspec(align(16)) pointer >( operator new(n*sizeof(value_type)) );
	}
	
	void deallocate(pointer const p, const size_type& n) const
	{
		operator delete(p);
	}
	*/

	pointer allocate(const size_type& n, const void* hint = 0) const
	{
		if(0 == n) return NULL;

		if(n > max_size()) throw std::length_error("Requested size exceeds maximally allowed");

		//void* mem = _aligned_malloc(n*sizeof(value_type), 16);
		void* mem = _aligned_malloc(n*sizeof(T), align_bytes);

		if(NULL == mem) throw std::bad_alloc();
		
		return reinterpret_cast<pointer>(mem);
	}
	
	void deallocate(pointer const p, const size_type& n) const
	{
		_aligned_free(p);
	}

	pointer address(reference r) const
	{
		return &r;
	}

	const_pointer address(const_reference r) const
	{
		return &r;
	}

	size_type max_size() const
	{
		return (static_cast<size_type>(0)-static_cast<size_type>(1)) / sizeof(value_type);
	}


	template<class U>
	struct rebind
	{
		typedef allocator_aligned<U, align_bytes> other;
	};

	// для stateless аллокатора операторы сравнения возвращают true и false без выполнения непосредственно сравнения
	/*
	bool operator==(const allocator<T>& other) const throw()
	{
		return true;
	}

	bool operator!=(const allocator_aligned<T>& other) const throw()
	{
		return false;
	}
	
	template<class U>
	bool operator==(const allocator<U>& other) const throw()
	{
		return false;
	}

	template<class U>
	bool operator!=(const allocator_aligned<U>& other) const throw()
	{
		return true;
	}
    */

	void construct(pointer const mem, const_reference val) const
	{
		new(reinterpret_cast<void*>(mem)) value_type(val);
	}

	 
	// для C++ 11
	// вызов конструктора с многими аргументами
	template<class U, class... Args>
	void construct(U* mem, Args&&... args)
	{
		::new(reinterpret_cast<void*>(mem)) U(std::forward<Args>(args)...); // глобальный оператор new (::new) использ-ся, т.к. у класса U
																			// может быть своя перегруженная версия
	}
	

	void destroy(pointer p) const
	{
		p->~T();
	}

	template<class U>
	void destroy(U* p) const
	{
		p->~U();
	}

	/*
private:

	allocator_aligned& operator=(const allocator_aligned&);
	
	template<class U>
	allocator_aligned& operator=(const allocator_aligned<U>&);
	*/
	
};


template<class T1, int align_bytes, class T2>
inline bool operator==(const allocator_aligned<T1, align_bytes>&, const allocator_aligned<T2, align_bytes>&)
{
	return true;
}

template<class T1, int align_bytes, class T2>
inline bool operator!=(const allocator_aligned<T1, align_bytes>&, const allocator_aligned<T2, align_bytes>&)
{
	return false;
}


template<class T, int align_bytes, class other_allocator>
inline bool operator==(const allocator_aligned<T, align_bytes>&, const other_allocator&)
{
	return false;
}


template<class T, int align_bytes, class other_allocator>
inline bool operator!=(const allocator_aligned<T, align_bytes>&, const other_allocator&)
{
	return true;
}


#endif

