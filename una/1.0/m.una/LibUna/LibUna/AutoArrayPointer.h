// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoArrayPointer.h -- AutoArrayPointer の定義ファイル
// 
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
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

#ifndef __AUTOARRAYPOINTER__HEADER__
#define __AUTOARRAYPOINTER__HEADER__

#include "Module.h"
#include "ModVector.h"

_UNA_BEGIN 

//
//	CLASS
//		AutoArrayPointer -- ポインタ型の要素を持つ配列
//							デストラクト時に要素を delete する
//
template < class T >
class AutoArrayPointer : public ModVector< T* >
{
public:
	typedef ModVector< T* > Super;
	// コンストラクタ
	AutoArrayPointer();

	// デストラクタ
	~AutoArrayPointer();

protected:
private:

};

template < class T >
inline
AutoArrayPointer< T >::AutoArrayPointer() {}

template < class T >
inline
AutoArrayPointer< T >::~AutoArrayPointer() {
	ModTypename Super::Iterator it  = Super::begin();
	ModTypename Super::Iterator fin = Super::end();
	for ( ; it != fin; ++it ) {
		delete (*it);
	}
}

_UNA_END

#endif // __AUTOARRAYPOINTER__HEADER__

//
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
