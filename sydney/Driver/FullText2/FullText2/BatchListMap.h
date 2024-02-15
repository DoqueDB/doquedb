// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchListMap.h --
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

#ifndef __SYDNEY_FULLTEXT2_BATCHLISTMAP_H
#define __SYDNEY_FULLTEXT2_BATCHLISTMAP_H

#include "FullText2/Module.h"
#include "FullText2/BatchBaseList.h"

#include "Common/LargeVector.h"

#include "ModList.h"
#include "ModMap.h"
#include "ModPair.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class FullTextFile;

//
//	CLASS
//	FullText2::BatchListMap --
//
//	NOTES
//
//
class BatchListMap
	: public ModMap<ModUnicodeString,
					ModList<BatchBaseList*>,
					ModLess<ModUnicodeString> >
{
public:
	//
	//	TYPDEF
	//	FullText2::BatchListMap::Super -- 親クラス
	//
	typedef ModMap<ModUnicodeString, ModList<BatchBaseList*>,
		ModLess<ModUnicodeString> >	Super;
	
	//
	//	STRUCT
	//	FullText2::BatchListMap::Element
	//
	struct Element
	{
		Element() {}
		Element(ModUInt32 uiDocumentID_,
				ModUInt32 uiRowID_,
				ModUInt32 uiLength_)
			: m_uiDocumentID(uiDocumentID_),
			  m_uiRowID(uiRowID_),
			  m_uiLength(uiLength_) {}

		ModUInt32 m_uiDocumentID;	// 文書ID
		ModUInt32 m_uiRowID;		// ROWID
		ModUInt32 m_uiLength;		// 文書長
	};

	//
	//	TYPEDEF
	//	FullText2::BatchListMap::Vector
	//
	typedef Common::LargeVector<Element> Vector;

	// コンストラクタ
	BatchListMap(FullTextFile& cFile_);
	// デストラクタ
	virtual ~BatchListMap();

	// 挿入する
	ModPair<Iterator, ModBoolean>
	insertEntry(const ModUnicodeString& cstrKey_,
				const ModList<BatchBaseList*>& cList_);

	// サイズを増やす
	void addListSize(ModSize uiSize_);
	// サイズを得る
	ModSize getListSize() const { return m_uiListSize; }

	// 最終文書IDを得る
	ModUInt32 getLastDocumentID() { return ++m_uiLastDocumentID; }

	// 最大ROWIDを得る
	ModUInt32 getMaxRowID() const { return m_uiMaxRowID; }

	// 文書ID-ROWID-文書長の組を登録する
	void insertVector(ModUInt32 uiDocumentID_,
					  ModUInt32 uiRowID_, ModUInt32 uiDocumentLength_)
	{
		m_cVector.pushBack(Element(uiDocumentID_, uiRowID_, uiDocumentLength_));
		addListSize(sizeof(Element));
		if (m_uiMaxRowID < uiRowID_)
			m_uiMaxRowID = uiRowID_;
	}

	// ベクターを得る
	Vector& getVector() { return m_cVector; }

private:
	// サイズ
	ModSize m_uiListSize;
	// 最終文書ID
	ModUInt32 m_uiLastDocumentID;
	// 最大ROWID
	ModUInt32 m_uiMaxRowID;
	// ベクター
	Vector m_cVector;

	// 全文索引ファイル
	FullTextFile& m_cFile;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_BATCHLISTMAP_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
