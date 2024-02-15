// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleResultSet.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "FullText2/SimpleResultSet.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace {
	//
	//	複数の配列をマージする関数
	//	引数の vbegin のサイズは変更される
	//	また、空の配列が与えられることは想定していない
	//	
	template<class Iterator, class Container, class Compare>
	void
	_merge(Iterator first1, Iterator last1,
		   Iterator first2, Iterator last2,
		   Compare comp, Container& container, ModSize limit)
	{
		ModSize n = 0;
		for (; first1 != last1 && first2 != last2 && n < limit; ++n)
		{
			if (comp(*first2, *first1)) {
				container.pushBack(*first2);
				++first2;
			} else {
				container.pushBack(*first1);
				++first1;
			}
		}
		for (; first1 != last1 && n < limit; ++n, ++first1)
			container.pushBack(*first1);
		
		for (; first2 != last2 && n < limit; ++n, ++first2)
			container.pushBack(*first2);
	}

}

//
//	FUNCTION public
//	FullText2::SimpleResultSet::SimpleResultSet -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SimpleResultSet::SimpleResultSet()
	: Super()
{
}

//
//	FUNCTION public
//	FullText2::SimpleResultSet::~SimpleResultSet -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SimpleResultSet::~SimpleResultSet()
{
}

//
//	FUNCTION public
//	FullText2::SimpleResultSet::SimpleResultSet -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::SimpleResultSet& cSrc_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SimpleResultSet::SimpleResultSet(const SimpleResultSet& cSrc_)
	: Super(cSrc_)
{
}

//
//	FUNCTION public
//	FullText2::SimpleResultSet::operator = -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::SimpleResultSet& cSrc_
//		代入元
//
//	RETURN
//	FullText2::SimpleResultSet&
//		自身への参照
//
//	EXCEPTIONS
//
SimpleResultSet&
SimpleResultSet::operator = (const SimpleResultSet& cSrc_)
{
	Super::operator = (cSrc_);
	return *this;
}

//
//	FUNCTION public
//	FullText2::SimpleResultSet::sort -- ソートする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ResultSet::SortKey::Value eKey_
//		キー
//	FullText2::ResultSet::Order::Value eOrder_
//		降順 or 昇順
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleResultSet::sort(SortKey::Value eKey_, Order::Value eOrder_)
{
	if (eKey_ == SortKey::Score)
	{
		if (eOrder_ == Order::Asc)
			ModSort(begin(), end(), ScoreLess());
		else
			ModSort(begin(), end(), ScoreGreater());
	}
	else if (eKey_ == SortKey::DocID)
	{
		if (eOrder_ == Order::Asc)
			ModSort(begin(), end(), IDLess());
		else
			ModSort(begin(), end(), IDGreater());
	}
}

//
//	FUNCTION public
//	FullText2::SimpleResultSet::sort -- ソートする
//
//	NOTES
//
//	ARGUMENTS
//	int iStart_
//		ソート開始位置
//	int iEnd_
//		ソート範囲終端
//	FullText2::ResultSet::SortKey::Value eKey_
//		キー
//	FullText2::ResultSet::Order::Value eOrder_
//		降順 or 昇順
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleResultSet::sort(int iStart_, int iEnd_,
					  SortKey::Value eKey_, Order::Value eOrder_)
{
	Iterator b = begin();
	Iterator e = b + iEnd_;
	b += iStart_;
	
	if (eKey_ == SortKey::Score)
	{
		if (eOrder_ == Order::Asc)
			ModSort(b, e, ScoreLess());
		else
			ModSort(b, e, ScoreGreater());
	}
	else if (eKey_ == SortKey::DocID)
	{
		if (eOrder_ == Order::Asc)
			ModSort(b, e, IDLess());
		else
			ModSort(b, e, IDGreater());
	}
}

//
//	FUNCTION public
//	FullText2::SimpleResultSet::merge
//		-- マージする
//
//	NOTES
//
//	ARGUMENTS
//	const SimpleResultSet& cResultSet_
//		マージ対象の集合。ソートされていること
//	FullText2::ResultSet::SortKey::Value eKey_
//		ソートのキー
//	FullText2::ResultSet::Order::Value eOrder_
//		昇順 or 降順
//
//	RETURN
//	なし
//
//	EXCEPIONS
//
void
SimpleResultSet::merge(const SimpleResultSet& cResultSet_,
					   SortKey::Value eKey_, Order::Value eOrder_)
{
	ModSize size = cResultSet_.getSize() + getSize();

	// 現在の配列をコピーして、中身をクリアする
	SimpleResultSet original = *this;
	const SimpleResultSet& org = original;
	erase(begin(), end());

	// 領域を確保する
	reserve(size);

	if (eKey_ == SortKey::DocID)
	{
		if (eOrder_ == Order::Asc)
			_merge(org.begin(), org.end(),
				   cResultSet_.begin(), cResultSet_.end(),
				   SimpleResultSet::IDLess(), *this, size);
		else
			_merge(org.begin(), org.end(),
				   cResultSet_.begin(), cResultSet_.end(),
				   SimpleResultSet::IDGreater(), *this, size);
	}
	else if (eKey_ == SortKey::Score)
	{
		if (eOrder_ == Order::Asc)
			_merge(org.begin(), org.end(),
				   cResultSet_.begin(), cResultSet_.end(),
				   SimpleResultSet::ScoreLess(), *this, size);
		else
			_merge(org.begin(), org.end(),
				   cResultSet_.begin(), cResultSet_.end(),
				   SimpleResultSet::ScoreGreater(), *this, size);
	}
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
