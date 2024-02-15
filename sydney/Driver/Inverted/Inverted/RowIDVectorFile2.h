// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RowIDVectorFile2.h --
// 
// Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_ROWIDVECTORFILE2_H
#define __SYDNEY_INVERTED_ROWIDVECTORFILE2_H

#include "Inverted/Module.h"
#include "Inverted/VectorFile.h"
#include "Inverted/FileID.h"
#include "ModPair.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	CLASS
//	Inverted::RowIDVectorFile2 --
//
//	NOTES
//
//
class RowIDVectorFile2 : public VectorFile<ModPair<ModUInt32, ModInt32> >
{
public:
	//コンストラクタ
	RowIDVectorFile2(const FileID& cFileID_, bool batch_);
	//デストラクタ
	virtual ~RowIDVectorFile2();

	// 移動する
	void move(const Trans::Transaction& cTransaction_, const Os::Path& cPath_);
	
	// 挿入
	void insert(ModUInt32 uiRowID_,
				ModUInt32 uiDocumentID_,
				ModInt32 iElement_);

	// 検索
	bool find(ModUInt32 uiRowID_,
			  ModUInt32& uiDocumentID_, ModInt32& iElement_);

};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_ROWIDVECTORFILE2_H

//
//	Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
