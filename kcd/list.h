/*
	list.h		Linked-List Functions Header
	Copyright (c) 1996-8,2000,2001,2002,2003 Kriang Lerdsuwanakij
	email:		lerdsuwa@users.sourceforge.net

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __K_LIST_H
#define __K_LIST_H

#include "config.h"

#include CXX__HEADER_cstring
#include CXX__HEADER_list

USING_NAMESPACE_STD;

/*************************************************************************
	Smart pointer class with reference counting
*************************************************************************/

template <class T> class sptr {
		T *	ptr;
		size_t *	count_ptr;
		bool *	own;
	public:
				// Create new smart pointer from scratch
				// ptr_ is now own by sptr
		sptr (T *ptr_) : ptr (ptr_), count_ptr (new size_t), own (new bool) {
			*count_ptr = 1;
			*own = true;
		}
				// Clone another copy of smart pointer
		sptr (const sptr &s) : ptr (s.ptr), count_ptr (s.count_ptr), own (s.own) {
			++*count_ptr;
		}
		~sptr () { 
			--*count_ptr;
				// This object is the last one that reference ptr
			if (*count_ptr == 0) {
				if (*own)
					delete ptr;
				delete count_ptr;
			}
		}
				// Reassign with new smart pointer/ordinary pointer
		sptr& operator =(const sptr &s) {
			--*count_ptr;
			if (*count_ptr == 0) {
				if (*own)
					delete ptr;
				delete count_ptr;
			}
			
			ptr = s.ptr;
			count_ptr = s.count_ptr;
			own = s.own;
			++*count_ptr;
			return *this;
		}

				// Release ptr from our pointer management
		T* release() {
			*own = false;
			return ptr;
		}

		T* operator ()()	{ return ptr; }
		T* operator ->()	{ return ptr; }
		T& operator *() const	{ return *ptr; }

		const T* operator ()() const	{ return ptr; }
		const T* operator ->() const	{ return ptr; }
};

template <class T> bool operator == (const sptr<T> &s1, const sptr<T> &s2)
{
	return *s1 == *s2;
}

template <class T> bool operator < (const sptr<T> &s1, const sptr<T> &s2)
{
	return *s1 < *s2;
}

/*************************************************************************
	Smart pointer to array class with reference counting
	It is same as sptr<T> except delete [] is used instead.
*************************************************************************/

template <class T> class saptr {
		T *	ptr;
		size_t *	count_ptr;
		bool *	own;
	public:
				// Create new smart pointer from scratch
				// ptr_ is now own by saptr
		saptr (T *ptr_) : ptr (ptr_), count_ptr (new size_t), own (new bool) {
			*count_ptr = 1;
			*own = true;
		}
				// Clone another copy of smart pointer
		saptr (const saptr &s) : ptr (s.ptr), count_ptr (s.count_ptr), own (s.own) {
			++*count_ptr;
		}
		~saptr () { 
			--*count_ptr;
				// This object is the last one that reference ptr
			if (*count_ptr == 0) {
				if (*own)
					delete [] ptr;
				delete count_ptr;
			}
		}
				// Reassign with new smart pointer/ordinary pointer
		saptr& operator =(const saptr &s) {
			--*count_ptr;
			if (*count_ptr == 0) {
				if (*own)
					delete [] ptr;
				delete count_ptr;
			}
			
			ptr = s.ptr;
			count_ptr = s.count_ptr;
			own = s.own;
			++*count_ptr;
			return *this;
		}

				// Release ptr from our pointer management
		T* release() {
			*own = false;
			return ptr;
		}

		T* operator ()()	{ return ptr; }
		T* operator ->()	{ return ptr; }
		T& operator *()	const	{ return *ptr; }
		T& operator [](size_t i)	{ return ptr[i]; }

		const T* operator ()() const	{ return ptr; }
		const T* operator ->() const	{ return ptr; }
};

/*************************************************************************
	Smart STL list class
*************************************************************************/

template <class T> class sptr_list : public list<sptr<T> > {
};

/*************************************************************************
	Loop iterator
*************************************************************************/

template <class T> inline void next_loop (T &t, typename T::iterator &iter)
{
	++iter;
	if (iter == t.end())
		iter = t.begin();
}

template <class T> inline void prev_loop (T &t, typename T::iterator &iter)
{
	if (iter == t.begin())
		iter = t.end();
	--iter;
}

template <class T> inline void next_loop (const T &t, typename T::const_iterator &iter)
{
	++iter;
	if (iter == t.end())
		iter = t.begin();
}

template <class T> inline void prev_loop (const T &t, typename T::const_iterator &iter)
{
	if (iter == t.begin())
		iter = t.end();
	--iter;
}

#endif	/* __K_LIST_H */
