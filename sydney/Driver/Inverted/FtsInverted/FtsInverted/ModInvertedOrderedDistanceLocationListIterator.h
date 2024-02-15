// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedOrderedDistanceLocationListIterator.h -- 文書内出現位置リストの反復子
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedOrderedDistanceLocationListIterator_H__
#define __ModInvertedOrderedDistanceLocationListIterator_H__

#include "ModInvertedLocationListIterator.h"
#include "ModVector.h"

//
// CLASS
// ModInvertedOrderedDistanceLocationListIterator -- 位置のつき合わせによる位置反復子
//
// NOTES
//
class
ModInvertedOrderedDistanceLocationListIterator :
	public ModInvertedLocationListIterator
{
public:
	// typedef
	typedef ModInvertedLocationListIterator	LocationIterator;
	typedef ModVector<ModInvertedLocationListIterator*> LocationIterators;

	ModInvertedOrderedDistanceLocationListIterator(
		ModInvertedQueryInternalNode* node);
	~ModInvertedOrderedDistanceLocationListIterator();

	void initialize();
	void pushIterator(ModSize position_,
					  LocationIterator* location_)
	{
		pathPosition.pushBack(position_);
		if (pathPosition.getSize() == 1)
			firstPathPosition = position_;
		childLocations.pushBack(location_);
	}
	void reserve(ModSize n)
	{
		pathPosition.reserve(n);
		childLocations.reserve(n);
	}
	ModSize getSize() const
	{
		// childLocations.getSize()も同じ値
		return pathPosition.getSize();
	}

	void next();
	void reset();
	ModBoolean isEnd() const;
	ModSize getLocation();

	// マッチする部分の末尾位置を返す
	ModSize getEndLocation();

	void release()
	{
		LocationIterators::Iterator i = childLocations.begin();
		for (; i != childLocations.end(); ++i)
			(*i)->release();
		childLocations.erase(childLocations.begin(), childLocations.end());
		pathPosition.erase(pathPosition.begin(), pathPosition.end());
		LocationIterator::release();
	}
	
	// メンバ変数endのアクセサ関数
	virtual LocationIterator* getEnd();	
	virtual void setEnd(LocationIterator* end_);

#ifdef DEBUG
	static int countRawNext;
#endif

private:
	void rawNext();
	
	ModBoolean isEndStatus;
	ModSize leftLocation;

	ModVector<ModSize> pathPosition;
	LocationIterators childLocations;

	// pathPosition[0]の値
	ModSize firstPathPosition;

	// マッチする部分の最後尾のsimpleTokenLeafNodeの位置情報へのポインタ
	// RankingTermLeafNode/TermLeafNodeのreevaluateでセット
	LocationIterator* end;	
};

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::ModInvertedOrderedDistanceLocationListIterator -- コンストラクタ
//
// NOTES
// コンストラクタ
//
// ARGUMENTS
// ModInvertedQueryInternalNode* node
//		ノード
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedOrderedDistanceLocationListIterator::
ModInvertedOrderedDistanceLocationListIterator(
	ModInvertedQueryInternalNode* node)
	: LocationIterator(node)
{
}

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::~ModInvertedOrderedDistanceLocationListIterator -- デストラクタ
//
// NOTES
// デストラクタ
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
ModInvertedOrderedDistanceLocationListIterator::
~ModInvertedOrderedDistanceLocationListIterator()
{
}

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::initialize -- コンストラクタ
//
// NOTES
// 最初の該当位置に移動する。
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
ModInvertedOrderedDistanceLocationListIterator::initialize()
{
	end = 0;
	isEndStatus = ModFalse;

	//
	// 先頭子ノードの初期化
	//
	ModVector<LocationIterator*>::Iterator i = childLocations.begin();
	leftLocation = (*i)->getLocation();
	while ((*i)->isEnd() != ModTrue)
	{
		// 先頭子ノードの位置リスト中の参照位置を進める。
		// 少なくとも、文書内における位置が検索語内における位置より
		// 後ろ以降でないと、条件を満たさない。
		
		if (leftLocation < firstPathPosition)
		{
			// 現在参照中の文書内における位置が、検索語内の位置より前
			
			// 参照位置を更新 (文書内における位置を後ろに進める)
			(*i)->next();
			leftLocation = (*i)->getLocation();
			continue;
		}
		break;
	}

	//
	// 各子ノードの確認
	//
	for (; i != childLocations.end(); ++i)
	{
		// 各子ノードの位置リスト中の参照位置が終わりまできているか
		
		if ((*i)->isEnd() == ModTrue)
		{
			// 終わりまで来ていた (条件を満たさないことは明らか)
			isEndStatus = ModTrue;
			break;
		}
	}

	//
	// 各子ノードの位置リストの中に、条件を満たす組み合わせは存在するか
	// (存在するならisEndStatusはModFalseが維持される)
	//
	rawNext();
}

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::rawNext -- 次の位置に進む
//
// NOTES
// 現在位置から、距離の制約条件を満たす次の位置に自分を進める。
// 現在位置が距離の制約条件を満たしている場合は動かない。
//
// 制約条件を満たさない場合、isEndStatus==ModTrueを設定する。
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
ModInvertedOrderedDistanceLocationListIterator::rawNext()
{
#ifdef DEBUG
	++countRawNext;
#endif

	// アルゴリズム概要
	// * 検索文字列の後ろから調べる。
	// * BM法のように照合失敗時のスキップテーブルはない。
	// * 照合に失敗した索引語を基準に後ろにずらすのみ。

	// 改善点
	// * 照合失敗時に、次に照合する子ノードは基本的に先頭からなので、
	//  二回照合してしまう可能性がある。
	// * 各索引語のTFはわかっているので、後ろから調べる必要はない。
	//  TFが小さい順に調べた方が速いはず。
	
	ModVector<LocationIterator*>::Iterator cb = childLocations.begin();
	ModVector<LocationIterator*>::Iterator e = childLocations.end();
	ModVector<ModSize>::Iterator pb = pathPosition.begin();

	ModVector<LocationIterator*>::Iterator child = cb;
	ModVector<ModSize>::Iterator p = pb;
	
	// 先頭子ノードの索引語の検索語内における位置
	const ModSize leftPosition = *p;
	// 第二子ノードから
	++child;
	++p;

	while (isEndStatus == ModFalse)
	{
		if (child == e)
			// 全ての条件を満足
			break;

		// 検索語内の差と文書内の差が一致するtmpを探す。
		
		// 先頭子ノードと参照中子ノードにおける検索語内の差(leftPosition - *p)
		// ※ 子ノードは、検索語内での出現位置が後ろのものが前に設定される。
		//    ModInvertedOrderedDistanceNode::reevaluate、
		//    ModInvertedTermLeafNode::getBestPath等を参照のこと。
		// 先頭子ノードにおける文書内の位置(leftLocation)
		// ※ leftLocationはメンバ変数で、先頭子ノードの索引語を
		//    どこまで調べたかを保持している。
		// 参照中の子ノードにおける文書内の位置(tmp)
		ModSize tmp = leftLocation - (leftPosition - *p);
		
		if ((*child)->lowerBound(tmp) == ModFalse)
		{
			// lowerBound で見つからなければ、照合しないということ
			// ※ lowerBoundは、tmp以上の中で最少の位置を返す。
			//    それで見つからないということは、
			//    leftLocationを後ろにずらし、tmpも後ろにずらしても、
			//    条件を満たすものが存在しないことは明らか。
			isEndStatus = ModTrue;
			break;
		}
		else if (tmp < (*child)->getLocation())
		{
			// tmpと等しくなかった(条件を満たさない)
			// ※ LocationIterator::lowerBoundでみつかった位置は
			//    getLocationで取得できる。
			
			// 条件を満たす位置が他に存在するかもしれないので、
			// 各変数を更新する。
			
			// 先頭子ノードの位置を、条件を満たす位置が存在するなら、
			// この位置以降に先頭子ノードの検索語が存在しなければならない
			// という位置に更新
			leftLocation = (*child)->getLocation() - *p + leftPosition;
			
			// 次に調査する子ノードを更新
			if (child == cb)
			{
				// 現在、参照中の子ノードは先頭子ノード
				
				// *p == leftPosition なので、leftLocationの値は
				// 先頭子ノードの検索語の位置と一致する。
				// 従って、二番目の子ノードの調査を続ければよい。
				
				// 二番目の子ノードへ
				++child;
				++p;
			}
			else
			{
				// 現在、参照中の子ノードは先頭子ノード以外

				// leftLocationの値が示す位置に、
				// 先頭子ノードの検索語が存在しないことは明らか。
				// 存在していたら、tmp == (*child)->getLocation()のはず。
				// 従って、先頭子ノードの新たな位置を取得する必要がある。
				
				// 先頭子ノードへ
				child = cb;
				p = pb;
			}
		}
		else
		{
			// tmp == (*child)->getLocation()
			// 条件を満たすtmpが見つかった

			// 次へ
			++child;
			++p;
		}
	}
}

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::next -- 次の位置に進む
//
// NOTES
// 現在位置の次の距離の制約条件を満たす位置に自分を進める。
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
ModInvertedOrderedDistanceLocationListIterator::next()
{
	if (isEndStatus == ModFalse)
	{
		LocationIterator* loc = *(childLocations.begin());
		loc->next();
		isEndStatus = (ModBoolean)(loc->isEnd() == ModTrue);
		if (isEndStatus == ModFalse)
		{
			leftLocation = loc->getLocation();
			rawNext();
		}
	}
}

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::reset -- 先頭に戻る
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
ModInvertedOrderedDistanceLocationListIterator::reset()
{
	isEndStatus = ModFalse;
	ModVector<LocationIterator*>::Iterator i = childLocations.begin();
	for (; i != childLocations.end(); ++i)
	{
		(*i)->reset();
	}
	i = childLocations.begin();
	leftLocation = (*i)->getLocation();
	while ((*i)->isEnd() != ModTrue)
	{
		if (leftLocation < firstPathPosition)
		{
			(*i)->next();
			leftLocation = (*i)->getLocation();
			continue;
		}
		break;
	}
	for (; i != childLocations.end(); ++i)
	{
		if ((*i)->isEnd() == ModTrue)
		{
			isEndStatus = ModTrue;
			break;
		}
	}
	rawNext();
}

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::isEnd -- 末尾か調べる
//
// NOTES
// 現在位置が末尾か調べる
//
// ARGUMENTS
// なし
//
// RETURN
// 末尾ならば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedOrderedDistanceLocationListIterator::isEnd() const
{
	return isEndStatus;
}

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::getLocation -- 位置の取得
//
// NOTES
// 現在位置を取得する
//
// ARGUMENTS
// なし
//
// RETURN
// 現在位置
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedOrderedDistanceLocationListIterator::getLocation()
{
	return leftLocation - firstPathPosition + 1;
}

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::getEndLocation -- マッチする部分の末尾位置を返す
//
// NOTES
// マッチする部分の末尾位置を返す。
//
// ARGUMENTS
// なし
//
// RETURN
// 末尾位置
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedOrderedDistanceLocationListIterator::getEndLocation()
{
	return getEnd()->getEndLocation();
}

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::getEnd -- マッチする部分の末尾のsimpleTokenLeafNodeの位置情報へのポインタをendを返す。
//
// NOTES
// マッチする部分の末尾のsimpleTokenLeafNodeの位置情報へのポインタendを返す。
// 
// ARGUMENTS
// なし
//
// RETURN
// マッチする部分の末尾のsimpleTokenLeafNodeの位置情報へのポインタend
//
// EXCEPTIONS
// なし
//
inline ModInvertedLocationListIterator*
ModInvertedOrderedDistanceLocationListIterator::getEnd()
{
	if (end == 0)
	{
		// 設定されていないので、設定する
		ModVector<ModSize>::Iterator p = pathPosition.begin();
		ModSize maxPosition = *p;
		ModSize element = 0;
		++p;
		while (p != pathPosition.end())
		{
			if (maxPosition < *p)
			{
				maxPosition = *p;
				element = p - pathPosition.begin();
			}
		}
		end = childLocations[element];
	}
	return end;
}

//
// FUNCTION
// ModInvertedOrderedDistanceLocationListIterator::setEnd -- マッチする部分の末尾のsimpleTokenLeafNodeの位置情報へのポインタをendをセット
//
// NOTES
// マッチする部分の末尾のsimpleTokenLeafNodeの位置情報へのポインタendをセットする。
// 
// ARGUMENTS
// LocationIterator* end_
//		セットするポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOrderedDistanceLocationListIterator::setEnd(LocationIterator* end_)
{
	this->end = end_;
}

#endif	// __ModInvertedOrderedDistanceLocationListIterator_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
