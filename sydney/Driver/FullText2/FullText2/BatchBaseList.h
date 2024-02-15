// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchBaseList.h --
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

#ifndef __SYDNEY_FULLTEXT2_BATCHBASELIST_H
#define __SYDNEY_FULLTEXT2_BATCHBASELIST_H

#include "FullText2/Module.h"
#include "FullText2/InvertedList.h"
#include "FullText2/LeafPage.h"
#include "FullText2/AutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class BatchListMap;

//
//	CLASS
//	FullText2::BatchBaseList --
//
//	NOTES
//
//
class BatchBaseList : public InvertedList
{
public:
	// コンストラクタ(1)
	BatchBaseList(InvertedUpdateFile& cInvertedFile_,
				  BatchListMap& cBatchListMap_,
				  const ModUnicodeChar* pszKey_);
	// コンストラクタ(2)
	BatchBaseList(InvertedUpdateFile& cInvertedFile_,
				  LeafPage::Area* pArea_);
	// デストラクタ
	virtual ~BatchBaseList();

	// 整合性検査を行う
	void verify(Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_,
				const Os::Path& cRootPath_) {}

	// 格納されている文書数を得る
	ModUInt32 getCount() const { return m_pArea->getDocumentCount(); }

	// エリアを得る
	LeafPage::Area* getArea() { return m_pArea; }
	const LeafPage::Area* getArea() const { return m_pArea; }

	// 最大文書IDを得る
	DocumentID getMaxDocumentID() const { return m_uiMaxDocumentID; }

	// コンバート
	InvertedList* convert() { return 0; }

protected:
	// エリアの初期ユニット数
	static int getAllocateUnitSize();
	// 一定間隔でアロケートするユニット数
	static int getRegularUnitSize();
	// エリアの最大ユニット数
	static int getMaxUnitSize();
	
	// バッチリストマップ
	BatchListMap* m_pMap;

	// エリア
	AutoPointer<LeafPage::Area> m_pArea;
	// 最大文書ID
	DocumentID m_uiMaxDocumentID;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_BATCHBASELIST_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
