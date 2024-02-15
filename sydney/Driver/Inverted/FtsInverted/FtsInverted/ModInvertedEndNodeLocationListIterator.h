// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedEndNodeLocationListIterator.h -- OperatorEnd用文書内出現位置リストの反復子
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

#ifndef	__ModInvertedEndNodeLocationListIterator_H__
#define __ModInvertedEndNodeLocationListIterator_H__

#include "ModAlgorithm.h"
#include "ModInvertedLocationListIterator.h"

//
// CLASS
// ModInvertedEndNodeLocationListIterator
//		-- OperatorEnd用文書内出現位置リストの反復子
//
// NOTES
// OperatorEnd用文書内出現位置リストの反復子。
//
class
ModInvertedEndNodeLocationListIterator :
	public ModInvertedLocationListIterator
{
public:
	// typedef
	typedef ModInvertedLocationListIterator	LocationIterator;
	typedef ModVector<ModInvertedLocationListIterator*> LocationIterators;
	typedef ModInvertedOperatorEndNode EndNode;

	ModInvertedEndNodeLocationListIterator(ModInvertedQueryInternalNode*);
	~ModInvertedEndNodeLocationListIterator();

	void initialize(LocationIterator*,
					ModSize endLocation_,
					ModSize distance_);

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
	LocationIterator* iterator;

	// 終端位置
	ModSize endLocation;
	ModSize distance;
};

//
// FUNCTION
// ModInvertedEndNodeLocationListIterator::ModInvertedEndNodeLocationListIterator -- コンストラクタ
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
ModInvertedEndNodeLocationListIterator::ModInvertedEndNodeLocationListIterator(
	ModInvertedQueryInternalNode* node_)
	: ModInvertedLocationListIterator(node_)
{
}

//
// FUNCTION
// ModInvertedEndNodeLocationListIterator::~ModInvertedEndNodeLocationListIterator -- デストラクタ
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
ModInvertedEndNodeLocationListIterator::
~ModInvertedEndNodeLocationListIterator()
{
}

//
//	FUNCTION
//	ModInvertedEndNodeLocationListIterator::initialize -- 初期化
//
//	NOTES
//
//	
// ARGUMENTS
// LocationIterator* iterator_
//		処理対象の位置反復子
// ModSize endLocation_
//		終端位置（先頭からの文字数）
// ModSize distance_
//		終端からの距離（終端からの文字数）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
// 
inline void
ModInvertedEndNodeLocationListIterator::initialize(
	LocationIterator* iterator_,
	ModSize endLocation_,
	ModSize distance_) 
	
{
	iterator = iterator_;
	endLocation = endLocation_;
	distance = distance_;
	
	rawNext();
}

//
// FUNCTION
// ModInvertedEndNodeLocationListIterator::rawNext -- 次の位置に進む
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
ModInvertedEndNodeLocationListIterator::rawNext()
{
	while (iterator->isEnd() == ModFalse) {
		if (iterator->getEndLocation() + distance - 1 == endLocation) {
			break;
		}
		iterator->next();
	}
}
//
// FUNCTION
// ModInvertedEndNodeLocationListIterator::next -- 次の位置に進む
//
// NOTES
// 現在位置の次の距離の制約条件を満たす位置に自分を進める。
// EndNodeの場合はヒットするのは一個所だけなのでendになるはず。
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
ModInvertedEndNodeLocationListIterator::next()
{
	// current の要素を進める
	iterator->next();
	rawNext();
}

//
// FUNCTION
// ModInvertedEndNodeLocationListIterator::reset -- 先頭に戻る
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
ModInvertedEndNodeLocationListIterator::reset()
{
	iterator->reset();
	rawNext();
}
	
//
// FUNCTION
// ModInvertedEndNodeLocationListIterator::isEnd -- 末尾か調べる
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
ModInvertedEndNodeLocationListIterator::isEnd() const
{
	return iterator->isEnd();
}

//
// FUNCTION
// ModInvertedEndNodeLocationListIterator::getLocation -- 位置の取得
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
ModInvertedEndNodeLocationListIterator::getLocation()
{
	return iterator->getLocation();
}

//
// FUNCTION
// ModInvertedEndNodeLocationListIterator::getEndLocation -- マッチする部分の末尾位置を返す
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
ModInvertedEndNodeLocationListIterator::getEndLocation()
{
	return iterator->getEndLocation();
}

#endif	// __ModInvertedEndNodeLocationListIterator_H__

//
// Copyright (c) 1999, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
