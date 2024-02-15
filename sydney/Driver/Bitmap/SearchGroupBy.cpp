// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchGroupBy.cpp --
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
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Bitmap/SearchGroupBy.h"
#include "Bitmap/BitmapIterator.h"

#include "Os/AutoCriticalSection.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace {
}

//
//	FUNCTION public
//	Bitmap::SearchGroupBy::SearchGroupBy -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::FileID& cFileID_
//		ファイルID
//	Bitmap::BitmapFile* pBitmapFile_
//		ビットマップファイル
//	ModUInt32 last_
//		最大のROWIDのUnitポジション
//	const Common::BitSet* pNarrowingBitSet
//		絞り込みのためのビットマップ
//	Bitmap::LogicalInterface::BitSetMap& cResult_
//		group by の検索結果
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchGroupBy::SearchGroupBy(FileID& cFileID_,
							 BitmapFile* pBitmapFile_,
							 ModUInt32 last_,
							 const Common::BitSet* pNarrowingBitSet_,
							 LogicalInterface::BitSetMap& cResult_)
	: m_cFileID(cFileID_), m_pBitmapFile(pBitmapFile_), m_last(last_),
	  m_pNarrowingBitSet(pNarrowingBitSet_), m_cResult(cResult_)
{
}

//
//	FUNCTION public
//	Bitmap::SearchGroupBy::~SearchGroupBy -- デストラクタ
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
SearchGroupBy::~SearchGroupBy()
{
}

//
//	FUNCTION public
//	Bitmap::SearchGroupBy::prepare -- 前準備をする
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
SearchGroupBy::prepare()
{
	// スレッド数分のバリューを得る
	// BitmapFile はスレッドで共有するためコピーしない

	int s = getThreadSize();
	m_vecpValue.reserve(s);
	for (int i = 0; i < s; ++i)
	{
		m_vecpValue.pushBack(m_cFileID.createValueData());
	}
}

//
//	FUNCTION public
//	Bitmap::SearchGroupBy::parallel -- マルチスレッドで実行する
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
SearchGroupBy::parallel()
{
	// イテレータを、それが尽きるまで取出し、
	// 絞り込み条件と論理積を取って、集合がある場合に結果に格納する

	int n = getThreadNum();
	Common::Data& v = *(m_vecpValue[n]);
	
	while (true)
	{
		// キー格納用のCommon::Dataを得る
		Common::Data::Pointer pKey = m_cFileID.createKeyData();
		// B木から検索結果を得る
		if (m_pBitmapFile->getWithLatch(*pKey, v) == false)
			// もう結果がないので終了
			break;
		
		// ビットマップイテレータを得る
		ModAutoPointer<BitmapIterator> i = m_pBitmapFile->getIteratorForGroupBy(v);

		// ビットセットのインスタンスを確保
		Common::ObjectPointer<Common::BitSet> pBitSet = new Common::BitSet();

		// イテレータが得られたので、論理積を取る
		ModUInt32 pos = 0;
		do
		{
			// Common::BitSet::UnitType ごとにビットセット取り出す
			Common::BitSet::UnitType unit = (*i).getNextUnitType();

			if (m_pNarrowingBitSet)
			{
				// 絞り込み条件があるので、論理積を取る
				unit &= m_pNarrowingBitSet->getUnitType(pos);
			}

			if (unit.none() == false)
			{
				// 1ビットも立っていないものは挿入しない
				pBitSet->insertUnitType(pos, unit);
			}

			++pos;
		}
		while (pos <= m_last);

		if (pBitSet->any())
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			
			// 結果があるので、挿入する
			m_cResult.insert(pKey, pBitSet);
		}
	}
}

//
//	FUNCTION public
//	Bitmap::SearchGroupBy::dispose -- 後処理を行う
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
SearchGroupBy::dispose()
{
	m_vecpValue.clear();
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
