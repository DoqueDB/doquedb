// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedBatch.cpp --
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
#include "SyReinterpretCast.h"
#include "FullText2/InvertedBatch.h"
#include "FullText2/InvertedSection.h"
#include "FullText2/BatchList.h"
#include "FullText2/BatchListManager.h"
#include "FullText2/BatchNolocationList.h"
#include "FullText2/BatchNolocationNoTFList.h"

#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//  FUNCTION public
//  FullText2::InvertedBatch::InvertedBatch -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
// 	FullText2::InvertedSection& cSection_
//		転置ファイルセクション
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
InvertedBatch::InvertedBatch(InvertedSection& cSection_)
	: InvertedUpdateFile(cSection_, Os::Path())	// 実体はないので空文字列
{
	m_pListMap = new BatchListMap(cSection_.getFile());
}

//
//  FUNCTION public
//  FullText2::InvertedBatch::~InvertedBatch -- デストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
InvertedBatch::~InvertedBatch()
{
	delete m_pListMap;
}

//
//	FUNCTION public
//	FullText2::InvertedBatch::clear -- ファイルの内容をクリアする
//
//	NOTES
//	マージした後などに利用する
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
InvertedBatch::clear()
{
	delete m_pListMap;
	m_pListMap = new BatchListMap(m_cSection.getFile());
}

//
//	FUNCTION public
//	FullText2::InvertedBatch::getUpdateListManager
//		-- 更新用のリストマネージャーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::UpdateListManager*
//		更新用のリストマネージャー
//
//	EXCEPTIONS
//
UpdateListManager*
InvertedBatch::getUpdateListManager()
{
	return new BatchListManager(m_cSection.getFile(), this);
}

//
//	FUNCTION public
//	FullText2::InvertedBatch::addList -- 空のリストを追加する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		キー
//
//	RETURN
//	FullText2::BatchListMap::Iterator
//		追加した要素へのイテレータ
//
//	EXCEPTIONS
//
BatchListMap::Iterator
InvertedBatch::addList(const ModUnicodeString& cstrKey_)
{
	// 空の転置リストを作る
	BatchBaseList* p = makeList(cstrKey_);

	// リストに格納する
	ModList<BatchBaseList*> list;
	list.pushBack(p);

	// マップに挿入する
	ModPair<BatchListMap::Iterator, ModBoolean> r
		= m_pListMap->insertEntry(cstrKey_, list);

	if (r.second == ModFalse)
	{
		// すでに同じキーが存在していたので、メモリを解放する
		delete p;
	}

	return r.first;
}

//
//	FUNCTION public
//	FullText2::InvertedBatch::makeList -- 新しい転置リストを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//
//	RETURN
//	FullText2::BatchBaseList*
//		転置リスト
//
//	EXCEPTIONS
//
BatchBaseList*
InvertedBatch::makeList(const ModUnicodeString& cstrKey_)
{
	BatchBaseList* p;
	
	if (isNoTF() == true)
	{
		p = new BatchNolocationNoTFList(*this, *m_pListMap, cstrKey_);
	}
	else if (isNolocation() == true)
	{
		p = new BatchNolocationList(*this, *m_pListMap, cstrKey_);
	}
	else
	{
		p = new BatchList(*this, *m_pListMap, cstrKey_);
	}

	return p;
}

//
//  Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
