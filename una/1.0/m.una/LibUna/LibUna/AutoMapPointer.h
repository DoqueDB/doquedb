// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoMapPointer.h -- AutoMapPointer の定義ファイル
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

#ifndef __AUTOMAPPOINTER__HEADER__
#define __AUTOMAPPOINTER__HEADER__

#include "Module.h"
#include "ModMap.h"

_UNA_BEGIN 

/////////////////////////////////////////////////////////////////////////////
//
//	CLASS
//		AutoMapFirstPointer
//			-- first が ポインタ型の map
//			   デストラクト時に要素を delete する
//
template <class KeyType, class MappedType, class Compare >
class AutoMapFirstPointer : public ModMap< KeyType, MappedType, Compare >
{
public:
	typedef ModMap< KeyType, MappedType, Compare > Super;
	// コンストラクタ、デストラクタ
	AutoMapFirstPointer();
	~AutoMapFirstPointer();
protected:
private:

};

template <class KeyType, class MappedType, class Compare >
AutoMapFirstPointer< KeyType, MappedType, Compare >::~AutoMapFirstPointer()
	{
		ModTypename Super::Iterator it  = this->begin();
		ModTypename Super::Iterator fin = this->end();
		for ( ; it != fin; ++it ) {
			delete (*it).first;
		}
	}

//
// FUNCTION public
//	AutoMapFirstPointer::AutoMapFirstPointer
//		-- AutoMapFirstPointer クラスのコンストラクタ
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
template <class KeyType, class MappedType, class Compare >
AutoMapFirstPointer< KeyType, MappedType, Compare >::AutoMapFirstPointer()
{
}

/////////////////////////////////////////////////////////////////////////////
//
//	CLASS
//		AutoMapSecondPointer
//			-- second が ポインタ型の map
//			   デストラクト時に要素を delete する
//
template <class KeyType, class MappedType, class Compare >
class AutoMapSecondPointer : public ModMap<KeyType, MappedType, Compare>
{
public:
	typedef ModMap<KeyType, MappedType, Compare> Super;
	// コンストラクタ、デストラクタ
	AutoMapSecondPointer();
	~AutoMapSecondPointer();

protected:
private:

};

template <class KeyType, class MappedType, class Compare >
AutoMapSecondPointer< KeyType, MappedType, Compare >::~AutoMapSecondPointer()
	{
		ModTypename Super::Iterator it  = this->begin();
		ModTypename Super::Iterator fin = this->end();
		for ( ; it != fin; ++it ) {
			delete (*it).second;
		}
	}

//
// FUNCTION public
//	AutoMapSecondPointer::AutoMapSecondPointer
//		-- AutoMapSecondPointer クラスのコンストラクタ
//
// NOTES
//
// ARGUMENTS
//		なし
//
// RETURN
//		なし
//
// EXCEPTIONS
//
template <class KeyType, class MappedType, class Compare >
AutoMapSecondPointer< KeyType, MappedType, Compare >::AutoMapSecondPointer()
{
}

_UNA_END

#endif // __AUTOMAPPOINTER__HEADER__

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
