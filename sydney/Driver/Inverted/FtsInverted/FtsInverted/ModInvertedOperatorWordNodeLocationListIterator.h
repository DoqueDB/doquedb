// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedOperatorWordNodeLocationListIterator.h -- OperatorWordNode用書内出現位置リストの反復子
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedOperatorWordNodeLocationListIterator_H__
#define	__ModInvertedOperatorWordNodeLocationListIterator_H__

#ifdef V1_4	// 単語単位検索

#include "ModInvertedQueryNode.h"
#include "ModInvertedLocationListIterator.h"
#include "ModInvertedTermLeafNode.h"

#define USE_LOWER

//
// CLASS
// ModInvertedOperatorWordNodeLocationListIterator -- wordNode用位置反復子
//
// NOTES
// WordNodeのsimpleWordMode/exactWordModeの時に使用する位置反復子。
//
//
class
ModInvertedOperatorWordNodeLocationListIterator :
	public ModInvertedLocationListIterator
{
public:
	typedef ModInvertedLocationListIterator	LocationIterator;

	ModInvertedOperatorWordNodeLocationListIterator(
		ModInvertedQueryInternalNode* node,
		ModInvertedTermMatchMode mode,
		ModSize _wordLength,
		ModInvertedSmartLocationList* emptyLoationList);
	~ModInvertedOperatorWordNodeLocationListIterator();

	void initialize(
		LocationIterator* _termLoc,
		LocationIterator* _emptyStringLoc);

	void next();
	void reset();
	ModBoolean isEnd() const;
	ModSize getLocation();
	ModSize getEndLocation();

	void release()
	{
		if (termLoc) termLoc->release(), termLoc = 0;
		if (emptyStringLoc) emptyStringLoc->release(), emptyStringLoc = 0;
		LocationIterator::release();
	}

	// マッチの位置にiteratorが時以外は使用禁止
	// appModeで使用する
	// 現在の照合がどのような照合かを返す
	ModInvertedTermMatchMode getCurrentMatchType();

protected:
	// 検索語の文書内位置情報
	LocationIterator* termLoc;

	// 空文字列ノード(単語区切)の文書内位置情報
	LocationIterator* emptyStringLoc;

	// マッチモード
	ModInvertedTermMatchMode matchMode;

	// 検索語の長さ
	ModSize	wordLength;

	// 検索語内区切り文字位置情報(exactWordModeで使用する)
	ModInvertedSmartLocationList* tokenBoundary;
	// 検索語内区切り文字位置情報へアクセスするiterator(exactWordModeで使用する)
	ModInvertedLocationListIterator* boundary;

	// 次の位置に進む
	void rawNext();
	void rawNextExact();
	void rawNextSimple();
	void rawNextWordHead();
	void rawNextWordTail();
#ifndef MOD_DIST // APPMODE
	void rawNextApproximate();
#endif

private:
#ifdef USE_LOWER
	ModBoolean isEndStatus;
#endif
};

//
//	FUNCTION
//	ModInvertedOrderedOperatorWindowLocationListIterator::ModInvertedOrderedOperatorWindowLocationListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	ModInvertedQueryInternalNode* node
//		ノード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
inline
ModInvertedOperatorWordNodeLocationListIterator::
ModInvertedOperatorWordNodeLocationListIterator(
	ModInvertedQueryInternalNode* node,
	ModInvertedTermMatchMode mode,
	ModSize _wordLength,
	ModInvertedSmartLocationList* emptyLocationList)
	: LocationIterator(node), matchMode(mode), wordLength(_wordLength),
	  tokenBoundary(emptyLocationList), boundary(0)
{
}

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::~ModInvertedOrderedOperatorWindowLocationListIterator -- デストラクタ
//
// NOTES
// 	WordNode(単語検索用ノード)用位置反復子を削除する
//
// ARGUMENTS
//	なし
//
// RETURN
// 	なし
//
// EXCEPTIONS
//	なし
//
inline
ModInvertedOperatorWordNodeLocationListIterator::
~ModInvertedOperatorWordNodeLocationListIterator()
{
	if (boundary) delete boundary;
}

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::reset -- リセット
//
// NOTES
// 	リセット
//
// ARGUMENTS
//	なし
//
// RETURN
// 	なし
//
// EXCEPTIONS
//	なし
//
inline void
ModInvertedOperatorWordNodeLocationListIterator::reset()
{
#ifdef USE_LOWER
	isEndStatus = ModFalse;
#endif
	termLoc->reset();
	emptyStringLoc->reset();
	rawNext();
}

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::isEnd -- 終端チェック
//
// NOTES
// 	終端のチェック
//
// ARGUMENTS
//	なし
//
// RETURN
//  ModBoolean 終端に達している場合はModTrue/達していない場合はModFalse
//
// EXCEPTIONS
//	なし
//
inline ModBoolean
ModInvertedOperatorWordNodeLocationListIterator::isEnd() const
{
#ifdef USE_LOWER
	return isEndStatus;
#else
	if (termLoc->isEnd() == ModTrue) {
		return ModTrue;
	}
	return ModFalse;
#endif
}

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::next -- 次の位置へ移動
//
// NOTES
// 	次の位置へ移動
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
inline void
ModInvertedOperatorWordNodeLocationListIterator::next()
{
	termLoc->next();
	rawNext();
}

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::getLocation -- 位置取得
//
// NOTES
// 	位置取得
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModSize	現在の位置
//
// EXCEPTIONS
//	なし
//
inline ModSize
ModInvertedOperatorWordNodeLocationListIterator::getLocation()
{
	return termLoc->getLocation();
}


//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::getLocation -- マッチしている部分の後ろの位置を返す
//
// NOTES
// 	マッチしている部分の後ろの位置を返す
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModSize	マッチしている部分の後ろの位置
//
// EXCEPTIONS
//	なし
//
inline ModSize
ModInvertedOperatorWordNodeLocationListIterator::getEndLocation()
{
	// termLoc->getLocation() + wordLength()で良い？
	return termLoc->getEndLocation();
}

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::rawNext -- 次の位置に進む
//
// NOTES
// 現在位置から、距離の制約条件を満たす次の位置に自分を進める。
// 現在位置が距離の制約条件を満たしている場合は動かない。
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
ModInvertedOperatorWordNodeLocationListIterator::rawNext()
{
#ifndef MOD_DIST // APPMODE
	if (matchMode == ModInvertedTermApproximateMode) {
		rawNextApproximate();
	}
#endif
	if (matchMode == ModInvertedTermExactWordMode &&
		tokenBoundary != 0) {
		// exactModeでtokenBoundaryがセットされている
		// (tokenBoundaryがセットされていないのはshortWordのケース
		//					→simpleWordの動作)
		rawNextExact();
	} else if (matchMode == ModInvertedTermWordHead) {
		rawNextWordHead();
	} else if (matchMode == ModInvertedTermWordTail) {
		rawNextWordTail();
	} else {
		rawNextSimple();
	}
}

#endif // V1_4	単語単位検索

#endif	__ModInvertedOperatorWordNodeLocationListIterator_H__

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
