// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Map.h -- 排他制御つきのmapテンプレートクラス
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2007, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_MAP_H
#define __SYDNEY_SYDTEST_MAP_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "ModAutoMutex.h"
#include "ModCriticalSection.h"
#include "ModMap.h"
#include "SydTestMessage.h"

_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::Map -- 排他制御つきのmapテンプレートクラス
//
//	NOTES
//
//
//##ModelId=3A9B4747033B
template <class V, class T>
class Map 
{
public:
	typedef ModMap<V, T, ModLess<V> > mapType;
	typedef typename mapType::Iterator iterator;
	//コンストラクタ
	Map();
	//デストラクタ
	virtual ~Map();

	bool exists(V cKey_);

	T& operator [] (V cKey_);
	//##ModelId=3A9B47470359
	bool insert(V cKey_, T cValue_);
	ModSize erase(V cKey_);

	iterator begin();
	iterator end();

	bool isEmpty();

private:
	//##ModelId=3A9B47470352
	mapType m_mapValue;
	//##ModelId=3A9B47470349
	ModCriticalSection m_cCriticalSection;
};

//
//	FUNCTION public
//	SydTest::Map::Map -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
template <class V, class T>
inline
Map<V, T>::Map()
{
}

//
//	FUNCTION public
//	SydTest::Map::~Map -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
template <class V, class T>
inline
Map<V, T>::~Map()
{
}

//
//	FUNCTION public
//  SydTest::Map::exists -- キーに対応する要素があるかどうか調べる
//
//	NOTES
//    キーに対応する要素があるかどうか調べる
//
//	ARGUMENTS
//    V cKey
//      キー
//
//	RETURN
//    bool
//      true: ある、 false: ない
//
template <class V, class T>
inline
bool
Map<V, T>::exists(V cKey_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCriticalSection);
	cAuto.lock();

	return (m_mapValue.find(cKey_) != m_mapValue.end());
}

//
//	FUNCTION public
//  SydTest::Map::operator [] -- キーに対応する値を取得する
//
//	NOTES
//    キーに対応する値を取得する
//
//	ARGUMENTS
//    V cKey_
//      キー
//
//	RETURN
//    T&
//      値の参照
//
template <class V, class T>
inline
T&
Map<V, T>::operator [] (V cKey_)
{	
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCriticalSection);
	cAuto.lock();

	return m_mapValue[cKey_];
}

//
//	FUNCTION public
//  SydTest::Map::insert -- キーと値の組を挿入する
//
//	NOTES
//    キーと値の組を挿入する
//
//	ARGUMENTS
//    V cKey_
//      キー
//    T cValue_
//      値
//
//	RETURN
//    bool
//      true: 挿入成功、 false: 挿入失敗
//
template <class V, class T>
inline
bool
Map<V, T>::insert(V cKey_, T cValue_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCriticalSection);
	cAuto.lock();

	ModPair<iterator, ModBoolean> p = m_mapValue.insert
		(typename mapType::ValueType(cKey_, cValue_));
	return p.second == ModTrue ? true : false;
}

//
//	FUNCTION public
//  SydTest::Map::erase -- キーと値の組を削除する
//
//	NOTES
//    キーと値の組を削除する
//
//	ARGUMENTS
//    V cKey_
//      キー
//
//	RETURN
//    ModSize
//      キーにマッチして削除された要素の個数
//
template <class V, class T>
inline
ModSize
Map<V, T>::erase(V cKey_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCriticalSection);
	cAuto.lock();

	return m_mapValue.erase(cKey_);
}

//
//	FUNCTION public
//  SydTest::Map::begin -- 先頭要素へのイテレータを得る
//
//	NOTES
//    先頭要素へのイテレータを得る
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    Map::iterator
//      先頭要素へのイテレータ
//
template <class V, class T>
inline
typename Map<V, T>::iterator
Map<V, T>::begin()
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCriticalSection);
	cAuto.lock();
	return m_mapValue.begin();
}

//
//	FUNCTION public
//  SydTest::Map::end -- 末尾要素へのイテレータを得る
//
//	NOTES
//    末尾要素へのイテレータを得る
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    Map::iterator
//      末尾要素へのイテレータ
//
template <class V, class T>
inline
typename Map<V, T>::iterator
Map<V, T>::end()
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCriticalSection);
	cAuto.lock();
	return m_mapValue.end();
}

// TEMPLATE FUNCTION public
//	SydTest::Map<V, T>::isEmpty -- 
//
// TEMPLATE ARGUMENTS
//	
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

template <class V, class T>
inline
bool
Map<V, T>::
isEmpty()
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCriticalSection);
	cAuto.lock();
	return m_mapValue.isEmpty();
}

} // end of the namespace SydTest

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_MAP_H

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
