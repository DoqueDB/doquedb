// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchBaseList.cpp --
// 
// Copyright (c) 2010, 2017, 2023 Ricoh Company, Ltd.
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
#include "FullText2/BatchBaseList.h"
#include "FullText2/BatchListMap.h"
#include "FullText2/Parameter.h"

#include "ModOsDriver.h"
#include "ModList.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	VARIABLE
	//	_$$::_cAllocateUnitSize -- エリアの初期ユニット数
	//
	ParameterInteger
	_cAllocateUnitSize("FullText2_BatchListInitialUnitSize", 32,	// 128B
					   false);	// reload不可

	//
	//	VARIABLE
	//	_$$::_cRegularUnitSize -- 一定間隔でアロケートするユニット数
	//
	ParameterInteger
	_cRegularUnitSize("FullText2_BatchListRegularUnitSize", 1024,	// 4KB
					  false);	// reload不可

	//
	//	VARIABLE
	//	_$$::_cMaxUnitSize -- エリアの最大ユニット数
	//
	ParameterInteger
	_cMaxUnitSize("FullText2_BatchListMaxUnitSize", 16384,			// 64KB
				  false);	// reload不可
}

//
//	FUNCTION public
//	FullText2::BatchBaseList::BatchBaseList -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile& cInvertedFile_
//		転置ファイル
//	FullText2::BatchListMap& cBatchListMap_
//		バッチリストマップ
//	const ModUnicodeChar* pKey_
//		索引単位
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchBaseList::BatchBaseList(InvertedUpdateFile& cInvertedFile_,
							 BatchListMap& cBatchListMap_,
							 const ModUnicodeChar* pszKey_)
	: InvertedList(cInvertedFile_, pszKey_, ListType::Batch),
	  m_pMap(&cBatchListMap_), m_pArea(0), m_uiMaxDocumentID(0)
{
	m_pArea = LeafPage::Area::allocateArea(getKey(), getAllocateUnitSize());

	// 大きさを求める
	ModSize size = m_pArea->getUnitSize() * sizeof(ModUInt32);
	size += sizeof(BatchBaseList);			// BatchBaseList自体の大きさ
	size += getKey().getBufferSize();		// ModUnicodeStringの大きさ
	size += sizeof(ModListNode<BatchBaseList*>);		// ListNodeの大きさ

	m_pMap->addListSize(size);
}

//
//	FUNCTION public
//	FullText2::BatchBaseList::BatchBaseList -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile& cInvertedFile_
//		転置ファイル
//	LeafPage::Area* pArea_
//		リーフページのエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchBaseList::BatchBaseList(InvertedUpdateFile& cInvertedFile_,
							 LeafPage::Area* pArea_)
	: InvertedList(cInvertedFile_,
				   pArea_->getKey(),
				   pArea_->getKeyLength(),
				   ListType::Batch),
	  m_pMap(0), m_pArea(pArea_)
{
	m_uiMaxDocumentID = pArea_->getLastDocumentID();
}

//
//	FUNCTION public
//	FullText2::BatchBaseList::~BatchBaseList -- デストラクタ
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
BatchBaseList::~BatchBaseList()
{
}

//
//	FUNCTION protected
//	FullText2::BatchBaseList::getAllocateUnitSize
//		-- エリアの初期ユニット数を得る
//
int
BatchBaseList::getAllocateUnitSize()
{
	return _cAllocateUnitSize.get();
}

//
//	FUNCTION protected
//	FullText2::BatchBaseList::getRegularUnitSize
//		-- 一定間隔でアロケートするユニット数
//
int
BatchBaseList::getRegularUnitSize()
{
	return _cRegularUnitSize.get();
}

//
//	FUNCTION public
//	FullText2::BatchBaseList::getMaxUnitSize
//		-- エリアの最大ユニット数
//
int
BatchBaseList::getMaxUnitSize()
{
	return _cMaxUnitSize.get();
}

//
//	Copyright (c) 2010, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
