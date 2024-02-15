// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedLobData.h -- 
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOB_COMPRESSEDLOBDATA_H
#define __SYDNEY_LOB_COMPRESSEDLOBDATA_H

#include "Lob/Module.h"
#include "Lob/LobData.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

//
//	CLASS
//	Lob::CompressedLobData -- DIRページ
//
//	NOTES
//
class CompressedLobData : public LobData
{
public:
	// コンストラクタ
	CompressedLobData(LobFile& cFile,
					  BlockPage::PagePointer pBlockPage_,
					  BlockPage::Block* pBlock_);
	// デストラクタ
	virtual ~CompressedLobData();

	// 取得
	AutoPointer<void> get(ModSize uiPosition_, ModSize& uiLength_);

	// 追加
	void append(const void* pBuffer_, ModSize uiLength_);
	// 書き換え
	void replace(ModSize uiPosition_,
				 const void* pBuffer_, ModSize uiLength_);

	// 整合性検査を行う
	void verify(const ObjectID& cObjectID_);
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_COMPRESSEDLOBDATA_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
