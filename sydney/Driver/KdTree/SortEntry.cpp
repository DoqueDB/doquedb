// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SortEntry.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/SortEntry.h"

#include "Os/Limits.h"

#include "Exception/BadArgument.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::SortEntry::SortEntry -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Common::LargeVector<Entry*>::Iterator& b_
//		ソート対象の先頭
//	const Common::LargeVector<Entry*>::Iterator& e_
//		ソート対象の終端
//	int iSortKey_
//		ソート対象の次元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SortEntry::SortEntry(const Common::LargeVector<Entry*>::Iterator& b_,
					 const Common::LargeVector<Entry*>::Iterator& e_,
					 int iSortKey_)
	: m_b(b_), m_e(e_), m_cLess(iSortKey_), m_iCountPerThread(0)
{
}

//
//	FUNCTION public
//	KdTree::SortEntry::~SortEntry -- デストラクタ
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
SortEntry::~SortEntry()
{
}

//
//	FUNCTION public
//	KdTree::SortEntry::prepare -- 準備する
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
void
SortEntry::prepare()
{
	// スレッド数
	int size = getThreadSize();
	
	// スレッドあたりの数
	m_iCountPerThread = ((m_e - m_b) + (size - 1)) / size;

	// 配列を割り当てる
	m_cEntryPerThread.assign(size);
}

//
//	FUNCTION public
//	KdTree::SortEntry::parallel -- 平行実行する
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
void
SortEntry::parallel()
{
	// スレッド番号
	int n = getThreadNum();
	
	// 自スレッドの割り当てを得る
	Common::LargeVector<Entry*>& v = m_cEntryPerThread[n];

	if ((m_e - m_b) < (m_iCountPerThread * n))
		// このスレッドで実行するものはない
		return;
		
	Common::LargeVector<Entry*>::Iterator b
		= m_b + (m_iCountPerThread * n);
	Common::LargeVector<Entry*>::Iterator e
		= ((m_e - b) < m_iCountPerThread) ? m_e : (b + m_iCountPerThread);

	v.assign(b, e);

	// ソートする
	ModSort(v.begin(), v.end(), m_cLess);
}

//
//	FUNCTION public
//	KdTree::SortEntry::dispose -- 後処理する
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
void
SortEntry::dispose()
{
	if (isException())
		// どこかで例外が発生しているのでマージしない
		return;
	
	// マージする

	Common::LargeVector<Entry*>::Iterator b = m_b;

	int size = getThreadSize();
	ModVector<
		ModPair<Common::LargeVector<Entry*>::Iterator,
			    Common::LargeVector<Entry*>::Iterator> > vecIte;
	vecIte.reserve(size);

	ModVector<Common::LargeVector<Entry*> >::Iterator i
		= m_cEntryPerThread.begin();
	for (; i != m_cEntryPerThread.end(); ++i)
	{
		if ((*i).getSize() == 0)
			continue;
		
		vecIte.pushBack(
			ModPair<Common::LargeVector<Entry*>::Iterator,
				    Common::LargeVector<Entry*>::Iterator>
			((*i).begin(), (*i).end()));
	}

	for (; b != m_e; ++b)
	{
		if (vecIte.getSize() == 0)
		{
			// ありあえない
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		
		ModVector<
			ModPair<Common::LargeVector<Entry*>::Iterator,
				    Common::LargeVector<Entry*>::Iterator> >::Iterator
			j = vecIte.begin();
		ModVector<
			ModPair<Common::LargeVector<Entry*>::Iterator,
				    Common::LargeVector<Entry*>::Iterator> >::Iterator
			min = j;
		
		// 最小値を得る
		for (++j; j != vecIte.end(); ++j)
		{
			if (m_cLess(*((*j).first), *((*min).first)) == ModTrue)
				min = j;
		}
		
		*b = *((*min).first);
		++((*min).first);

		if ((*min).first == (*min).second)
		{
			// これは終わり
			vecIte.erase(min);
		}
	}
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
