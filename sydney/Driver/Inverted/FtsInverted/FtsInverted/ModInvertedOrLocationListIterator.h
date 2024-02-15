// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedOrLocationListIterator.h -- OperatorOr用文書内出現位置リストの反復子
// 
// Copyright (c) 1997, 1998, 2001, 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedOrLocationListIterator_H__
#define __ModInvertedOrLocationListIterator_H__

#include "ModAlgorithm.h"
#include "ModInvertedLocationListIterator.h"

//
// CLASS
// ModInvertedOrLocationListIterator
// -- OperatorOr用文書内出現位置リストの反復子
//
// NOTES
// OperatorOr用文書内出現位置リストの反復子
// 複数の位置リスト(LocationIteratorsでアクセス)を持ち、
// そのリストの中で前方にある順に位置を返すことができる。
//
class
ModInvertedOrLocationListIterator :
	public ModInvertedLocationListIterator
{
public:
	// typedef
	typedef ModInvertedLocationListIterator	LocationIterator;
	typedef ModVector<ModInvertedLocationListIterator*> LocationIterators;
	typedef ModVector<ModPair<ModSize,ModSize> > LocationIndexVector;

	ModInvertedOrLocationListIterator(ModInvertedQueryInternalNode* node);
	~ModInvertedOrLocationListIterator();

	void initialize();
	void pushIterator(LocationIterator* i)
	{
		iterators.pushBack(i);
	}
	void reserve(ModSize n)
	{
		iterators.reserve(n);
		headLocations.reserve(n);
	}
	ModSize getSize() const
	{
		return iterators.getSize();
	}

	void next();
	void reset();
	ModBoolean isEnd() const;
	ModSize getLocation();

	// マッチする部分の末尾位置を返す
	ModSize getEndLocation();

	void release()
	{
		LocationIterators::Iterator i = iterators.begin();
		for (; i != iterators.end(); ++i)
			if (*i) (*i)->release();
		iterators.erase(iterators.begin(), iterators.end());
		headLocations.erase(headLocations.begin(), headLocations.end());
		LocationIterator::release();
	}

private:
	void rawNext();
	// 各要素の文書内位置情報
	LocationIterators		iterators;
	LocationIterators::Iterator iteratorBegin;
	// 現在のLocationIterator
	int						current;
	// 各要素の先頭位置
	ModVector<ModSize>		headLocations;
	// 現在の位置
	ModSize					currentPos;

	// headLocationsの先頭位置の反復子
	ModVector<ModSize>::Iterator headBegin;
	// headLocationsの終端位置の反復子
	ModVector<ModSize>::Iterator headEnd;
};

//
// FUNCTION
// ModInvertedOrLocationListIterator::ModInvertedOrLocationListIterator -- コンストラクタ
//
// NOTES
// コンストラクタ。
// 各要素の先頭位置を保存しておく headLocations を初期化し、最初の位置
// へ移動しておく。
//
// ARGUMENTS
// LocationIterators& iterators_
//		もととなる反復子の配列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline
ModInvertedOrLocationListIterator::ModInvertedOrLocationListIterator(
	ModInvertedQueryInternalNode* node)
	: LocationIterator(node)
{}

//
// FUNCTION
// ModInvertedOrLocationListIterator::~ModInvertedOrLocationListIterator -- デストラクタ
//
// NOTES
// デストラクタ。
// コンストラクト時に渡された反復子を解放する。
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
ModInvertedOrLocationListIterator::~ModInvertedOrLocationListIterator()
{
}

//
//	FUNCTION
//	ModInvetedOrLocationListIterator::initialize -- 初期化
//
//	NOTES
//	pushIteratorで子ノードのiteratorは設定済みのこと
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCETIONS
//
inline void
ModInvertedOrLocationListIterator::initialize()
{
	current = 0;
	int NumberOfIterator = iterators.getSize();
	iteratorBegin = iterators.begin();

	// Vectorのメモリ領域を確保するため0でfillしておく
	headLocations.insert(headLocations.begin(), NumberOfIterator, 0);

	// 最初の位置情報を headLocationsに設定
	for (int i = 0; i < NumberOfIterator; ++i){
		if (iterators[i]->isEnd() != ModTrue){
			headLocations[i] = iterators[i]->getLocation();
		} else {
			headLocations[i] = 0;		// end の時は 0
		}
	}

	headBegin = headLocations.begin();
	headEnd = headLocations.end();

	// 最少の値をもつ iterators へ移動
	rawNext();
}


//
// FUNCTION
// ModInvertedOrLocationListIterator::rawNext -- 次の位置に進む
//
// NOTES
// 次の位置を求める。
// 各反復子の出現位置のなかから最小のものを選択する。
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
ModInvertedOrLocationListIterator::rawNext()
{
	currentPos = ModUInt32Max;

	// headLocations の中から最小値を探し出す
	ModVector<ModSize>::Iterator headLocation = headBegin;
	for (ModSize n(0); headLocation != headEnd; ++headLocation, ++n) {
		if (*headLocation != 0 && *headLocation < currentPos) {
			// 最小値を持っていた interators の index を current にセット
			current = n;
			currentPos = *headLocation;
		}
	}
}

//
// FUNCTION
// ModInvertedOrLocationListIterator::next -- 次の位置に進む
//
// NOTES
// 現在位置の次の距離の制約条件を満たす位置に自分を進める。
//
// current が指していた iterators を次に進る。iterators を進めた結果 
// end に達っした場合、headLocationsには0をセットする。
// また、currentがさしていたiteratorsと同じ位置を指しているiteratorsも同様に
// 次に進め(但しheadLocationsが0(すでにendに達しているもの)以外のもの)、
// endに達した場合はheadLoationsに0をセットする。
//
// end に達っしていなっかた場合は rawNext() を呼び出してheadLocations 
// 内の最小値をもっている iterators をさがし出し、current にセットする。
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
ModInvertedOrLocationListIterator::next()
{
	if (currentPos == ModUInt32Max)
		// iteratorがendの場合は何もしない
		return;

	LocationIterators::Iterator iterator = iteratorBegin;
	ModVector<ModSize>::Iterator headLocation = headBegin;

	for (; headLocation != headEnd; ++headLocation, ++iterator) {
		if (*headLocation != 0) {
			// headLocations[n] が 0(既にendに達している)以外のもの

			if ((*iterator)->getLocation() == currentPos) {
				// iterators[n]が指している位置が現在の位置と一致
				// このiterators[n]も次にすすめる
				(*iterator)->next();

				if ((*iterator)->isEnd() == ModTrue){
					*headLocation = 0;	// end の場合 0 をセット
				} else {
					// end ではないので位置情報を取り出して headLocations
					// へセット
					*headLocation = (*iterator)->getLocation();
				}
			}
		}
	}

	// headLocations から最小値を探し出す
	rawNext();
}

//
// FUNCTION
// ModInvertedOrLocationListIterator::reset -- 先頭に戻る
//
// NOTES
// 先頭に戻る
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
ModInvertedOrLocationListIterator::reset()
{
	int i, NumberOfIterator(iterators.getSize());

	// iteratorsにreset()をかける
	for (i = 0; i < NumberOfIterator; ++i) {
		iterators[i]->reset();
	}

	// 最初の位置情報を headLocationsに設定
	for (i = 0; i < NumberOfIterator; ++i) {
		if (iterators[i]->isEnd() != ModTrue) {
			headLocations[i] = iterators[i]->getLocation();
		} else {
			headLocations[i] = 0;		// end の時は 0
		}
	}

	// 最少の値をもつ iterators へ移動
	rawNext();

}
	
//
// FUNCTION
// ModInvertedOrLocationListIterator::isEnd -- 末尾か調べる
//
// NOTES
// 反復子の現在位置が末尾に達したかを調べる。
//
// ARGUMENTS
// なし
//
// RETURN
// 末尾であれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedOrLocationListIterator::isEnd() const
{
	return (currentPos == ModUInt32Max) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModInvertedOrLocationListIterator::getLocation -- 位置の取得
//
// NOTES
// 反復子の現在位置における、出現位置の先頭位置を返す。
// （currentが指しているiteratorsのgetLocation()を返す。）
//
// ARGUMENTS
// なし
//
// RETURN
// 出現の先頭位置
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedOrLocationListIterator::getLocation()
{
	if (currentPos == ModUInt32Max)
		return 0;

	return currentPos;
}

//
// FUNCTION
// ModInvertedOrLocationListIterator::getEndLocation -- マッチする部分の末尾位置を返す
//
// NOTES
// 反復子の現在位置における、出現位置の末尾位置を返す。
// （currentが指しているiteratorsのgetEndLocation()を返す。）
//
// ARGUMENTS
// なし
//
// RETURN
// 出現の末尾位置
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedOrLocationListIterator::getEndLocation()
{
	if (currentPos == ModUInt32Max)
		return 0;

	return iterators[current]->getEndLocation();
}

#endif	__ModInvertedOrLocationListIterator_H__

//
// Copyright (c) 1997, 1998, 2001, 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//





