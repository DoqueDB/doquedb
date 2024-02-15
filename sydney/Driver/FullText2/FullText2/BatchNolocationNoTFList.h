// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchNolocationNoTFList.h --
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

#ifndef __SYDNEY_FULLTEXT2_BATCHNOLOCATIONNOTFLIST_H
#define __SYDNEY_FULLTEXT2_BATCHNOLOCATIONNOTFLIST_H

#include "FullText2/Module.h"
#include "FullText2/BatchBaseList.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::BatchNolocationNoTFList --
//
//	NOTES
//
//
class BatchNolocationNoTFList : public BatchBaseList
{
public:
	// コンストラクタ(1)
	BatchNolocationNoTFList(InvertedUpdateFile& cInvertedFile_,
							BatchListMap& cBatchListMap_,
							const ModUnicodeChar* pszKey_);
	// コンストラクタ(2)
	BatchNolocationNoTFList(InvertedUpdateFile& cInvertedFile_,
							LeafPage::Area* pArea_);
	// デストラクタ
	virtual ~BatchNolocationNoTFList();

	// 位置情報を格納していないか
	bool isNolocation() const { return true; }
	// TFを格納していないか (TFを格納しない時は位置情報も格納しない)
	bool isNoTF() const { return true; }
	
	// 転置リストの挿入 - 1文書挿入用
	bool insert(ModUInt32 uiDocumentID_,
				const SmartLocationList& cLocationList_);

	// イテレータを得る -- 得られたインスタンスは呼び出し元で開放する
	InvertedIterator* getIterator();

protected:
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_BATCHNOLOCATIONNOTFLIST_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
