// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchListMap.cpp --
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
#include "FullText2/BatchListMap.h"
#include "FullText2/FullTextFile.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::BatchListMap::BatchListMap -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::FullTextFile& cFile_
//		全文索引ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchListMap::BatchListMap(FullTextFile& cFile_)
	: m_uiListSize(0), m_uiLastDocumentID(0), m_uiMaxRowID(0), m_cFile(cFile_)
{
}

//
//	FUNCTION public
//	FullText2::BatchListMap::~BatchListMap -- デストラクタ
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
//	なし
//
BatchListMap::~BatchListMap()
{
	Iterator i = begin();
	for (; i != end(); ++i)
	{
		ModList<BatchBaseList*>::Iterator j = (*i).second.begin();
		for (; j != (*i).second.end(); ++j)
			delete (*j);
	}
}

//
//	FUNCTION public
//	FullText2::BatchListMap::insertEntry -- リストを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		キー
//	const ModList<FullText2::BatchBaseList*>& cList_
//		リスト
//
//	RETURN
//	ModPair<FullText2::BatchListMap::Iterator, ModBoolean>
//		エントリのイテレータ
//
//	EXCEPTIONS
//
ModPair<BatchListMap::Iterator, ModBoolean>
BatchListMap::insertEntry(const ModUnicodeString& cstrKey_,
						  const ModList<BatchBaseList*>& cList_)
{
	ModPair<Iterator, ModBoolean> r = Super::insert(cstrKey_, cList_);
	if (r.second == ModTrue)
	{
		addListSize(cstrKey_.getBufferSize());
		addListSize(sizeof(ModValueNode<
						   ModPair<
						   ModUnicodeString, ModList<BatchBaseList*> > >));
	}
	return r;
}

//
//	FUNCTION public
//	FullText2::BatchListMap::addListSize -- サイズを増やす
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiSize_
//		増やすサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BatchListMap::addListSize(ModSize uiSize_)
{
	m_cFile.addBatchSize(uiSize_);
	m_uiListSize += uiSize_;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
