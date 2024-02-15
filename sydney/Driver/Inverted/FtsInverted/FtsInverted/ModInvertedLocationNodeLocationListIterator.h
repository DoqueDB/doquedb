// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedLocationNodeLocationListIterator.h -- OperatorLocation用文書内出現位置リストの反復子
// 
// Copyright (c) 1999, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedLocationNodeLocationListIterator_H__
#define __ModInvertedLocationNodeLocationListIterator_H__

#include "ModAlgorithm.h"
#include "ModInvertedLocationListIterator.h"

//
// CLASS
// ModInvertedLocationNodeLocationListIterator
//		-- OperatorLocation用文書内出現位置リストの反復子
//
// NOTES
// OperatorLocation用文書内出現位置リストの反復子
//
class
ModInvertedLocationNodeLocationListIterator :
	public ModInvertedLocationListIterator
{
public:
	// typedef
	typedef ModInvertedLocationListIterator	LocationIterator;
	typedef ModVector<ModInvertedLocationListIterator*> LocationIterators;
	typedef ModVector<ModPair<ModSize,ModSize> > LocationIndexVector;
	typedef ModInvertedOperatorLocationNode LocationNode;

	ModInvertedLocationNodeLocationListIterator(ModInvertedQueryInternalNode*);
	~ModInvertedLocationNodeLocationListIterator();

	void initialize(LocationIterator*, ModSize);
	void initialize(LocationIterator*, ModSize, ModSize);

	void next();
	void reset();
	ModBoolean isEnd() const;
	ModSize getLocation();

	// マッチする部分の末尾位置を返す
	ModSize getEndLocation();

	void release()
	{
		if (iterator) iterator->release();
		iterator = 0;
		LocationIterator::release();
	}

private:
	void rawNext();

	// 子要素の文書内位置情報
	LocationIterator*		iterator;

	// 位置制約条件
	ModSize location;
};

//
// FUNCTION
// ModInvertedLocationNodeLocationListIterator::ModInvertedLocationNodeLocationListIterator -- コンストラクタ
//
// NOTES
// コンストラクタ。
//
// ARGUMENTS
// ModInvertedQueryInternalNode* node_
//		ノード
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
// 
inline
ModInvertedLocationNodeLocationListIterator::ModInvertedLocationNodeLocationListIterator(
	ModInvertedQueryInternalNode* node)
	: ModInvertedLocationListIterator(node)
{
}

//
// FUNCTION
// ModInvertedLocationNodeLocationListIterator::~ModInvertedLocationNodeLocationListIterator -- デストラクタ
//
// NOTES
// デストラクタ。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedLocationNodeLocationListIterator::
~ModInvertedLocationNodeLocationListIterator()
{
}

//
// FUNCTION
// ModInvertedLocationNodeLocationListIterator::initialize -- 初期化
//
// NOTES
//
// ARGUMENTS
// LocationIterator* iterator_
//		処理対象の位置反復子
// const ModSize location_
//		先頭からの距離（先頭からの文字数）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
// 
inline void
ModInvertedLocationNodeLocationListIterator::initialize(
	LocationIterator* iterator_, ModSize location_) 
{
	iterator = iterator_;
	location = location_;
	rawNext();
}

//
// FUNCTION
// ModInvertedLocationNodeLocationListIterator::initialize -- 初期化
//
// NOTES
// EndNodeLocationListIteratorのコンストラクタと同じインタフェースにするため
// ダミーのパラメータを追加した初期化関数
//
// ARGUMENTS
// LocationIterator* iterator_
//      処理対象の位置反復子
// const ModSize dummy
//		ダミー
// const ModSize location_
//      先頭からの距離（先頭からの文字数）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedLocationNodeLocationListIterator::initialize(
		LocationIterator* iterator_, ModSize dummy, ModSize location_)
{
	initialize(iterator_, location_);
}

//
// FUNCTION
// ModInvertedLocationNodeLocationListIterator::rawNext -- 次の位置に進む
//
// NOTES
// 現在位置の次の距離の制約条件を満たす位置に自分を進める。
// ただし現在の位置が条件を満たしている場合は移動しない。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
// 
inline void
ModInvertedLocationNodeLocationListIterator::rawNext()
{
	while(iterator->isEnd() != ModTrue) {
		if(iterator->getLocation() == location) {
			break;
		}
		iterator->next();
	}
}
//
// FUNCTION
// ModInvertedLocationNodeLocationListIterator::next -- 次の位置に進む
//
// NOTES
// 現在位置の次の距離の制約条件を満たす位置に自分を進める。
// locationNodeの場合はヒットするのは一個所だけなのでendになるはず。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedLocationNodeLocationListIterator::next()
{
	// current の要素を進める
	iterator->next();
	rawNext();
}

//
// FUNCTION
// ModInvertedLocationNodeLocationListIterator::reset -- 先頭に戻る
//
// NOTES
// 先頭に戻る。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedLocationNodeLocationListIterator::reset()
{
	iterator->reset();
	rawNext();
}
	
//
// FUNCTION
// ModInvertedLocationNodeLocationListIterator::isEnd -- 末尾か調べる
//
// NOTES
// iteratorが end だったら ModTrue をかえす。それ以外はModFalseを返す。
//
// ARGUMENTS
// なし
//
// RETURN
// endならModTrue、end以外はModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedLocationNodeLocationListIterator::isEnd() const
{
	return iterator->isEnd();
}

//
// FUNCTION
// ModInvertedLocationNodeLocationListIterator::getLocation -- 位置の取得
//
// NOTES
// iteratorのgetLocation()を返す。
//
// ARGUMENTS
// なし
//
// RETURN
// iteratorの現在の位置
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedLocationNodeLocationListIterator::getLocation()
{
	return iterator->getLocation();
}

//
// FUNCTION
// ModInvertedLocationNodeLocationListIterator::getEndLocation -- マッチする部分の末尾位置を返す
//
// NOTES
// iteratorのgetEndLocation()を返す。
//
// ARGUMENTS
// なし
//
// RETURN
// マッチする部分の末尾位置
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedLocationNodeLocationListIterator::getEndLocation()
{
	return iterator->getEndLocation();
}

#endif	// __ModInvertedLocationNodeLocationListIterator_H__

//
// Copyright (c) 1999, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
