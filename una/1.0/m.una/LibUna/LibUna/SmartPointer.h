// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SmartPointer.h -- SmartPointer の定義ファイル
// 
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#ifndef __SMARTPOINTER__HEADER__
#define __SMARTPOINTER__HEADER__

#include "LibUna/Module.h"

_UNA_BEGIN 

//
//	CLASS
//		SmartPointer -- 参照カウンタを持つ AutoPointer
//
	template < class T >
	class SmartPointer
	{
	public:
		// コンストラクタ、デストラクタ
		SmartPointer(T* p_ = 0);
			
		SmartPointer(const T* p_);
		
		SmartPointer(const SmartPointer< T >& t_);

		~SmartPointer();

		SmartPointer&	operator=(const SmartPointer< T >& t_ );
		
		SmartPointer&	operator=(T* t_);

		SmartPointer&	operator=(const T* t_);

		T&	operator* () const;
		T*	operator-> () const;
		T*	get() const;

		void release();

	protected:
	private:

		void			increaseCount();
		int			decreaseCount();

		T*			_pointer;
		bool			_const;
		/*mutable*/ int		_counter;
	};
	
	
	// Constructor
	template < class T >
	SmartPointer< T >::SmartPointer(T* p_)
	    : _pointer(p_), _const(false), _counter(0)
	{
		increaseCount();
	}
	
	template < class T >
	SmartPointer< T >::SmartPointer(const T* p_)
		: _pointer(p_), _const(true), _counter(0)
	{
		increaseCount();
	}
	
	template < class T >
	SmartPointer< T >::SmartPointer(const SmartPointer< T >& t_)
		: _pointer(t_.get()), _const(t_._const), _counter(0)
	{
		_pointer = t_.get();
		_const = t_._const;
		increaseCount();
	}
	
	// Destructor
	template < class T >
	SmartPointer< T >::~SmartPointer()	{	release();	}
	
	// 
	template < class T >
	SmartPointer< T >&	
	SmartPointer< T >::operator=(const SmartPointer< T >& t_ )
	{
		if ( get() != t_.get() ) {
			release();
			_pointer = t_.get();
			_const = t_._const;
			_counter = t_._counter;
			increaseCount();
		}
		return *this;
	}
	
	template < class T >
	SmartPointer< T >&
	SmartPointer< T >::operator=(T* t_)
	{
		if ( get() != t_ ) {
			release();
			_pointer = t_;
			_const = false;
			increaseCount();
		}
		return *this;
	}
	template < class T >
	SmartPointer< T >&
	SmartPointer< T >::operator=(const T* t_)
	{
		if ( get() != t_ ) {
			release();
			_pointer = const_cast<T*>(t_);
			_const = true;
			increaseCount();
		}
		return *this;
	}
	template < class T >
	T&
	SmartPointer< T >::operator* () const	{	return *get();		}
	
	template < class T >	
	T*
	SmartPointer< T >::operator-> () const	{	return get();		}
	
	template < class T >
	T*
	SmartPointer< T >::get() const		{	return _pointer;	}

	template < class T >
	void
	SmartPointer< T >::release()
	{
		if ( _pointer && _const == false) {
			if ( decreaseCount() == 0 ) {
				delete _pointer;
			}
		}
	}
	
	template < class T >
	void
	SmartPointer< T >::increaseCount() { ++_counter; }
		
	template < class T >
	int
	SmartPointer< T >::decreaseCount() { return (--_counter); }

_UNA_END

#endif // __SMARTPOINTER__HEADER__

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
