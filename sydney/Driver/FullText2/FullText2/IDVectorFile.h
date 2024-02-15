// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IDVectorFile.h -- ROWID <-> 文書IDの変換を行うベクターファイル
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

#ifndef __SYDNEY_FULLTEXT2_IDVECTORFILE_H
#define __SYDNEY_FULLTEXT2_IDVECTORFILE_H

#include "FullText2/Module.h"
#include "FullText2/FileID.h"
#include "FullText2/VectorFile.h"

#include "Version/File.h"
#include "Version/Page.h"

#include "LogicalFile/OpenOption.h"

#include "Common/DataArrayData.h"
#include "Common/LargeVector.h"

#include "Os/Path.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//	CLASS
//	FullText2::IDVectorFile -- ROWID <-> 文書IDの変換を行うベクターファイル
//
//	NOTES
//
class IDVectorFile : public VectorFile
{
public:
	//コンストラクタ
	IDVectorFile(const FileID& cFileID_,
				 const Os::Path& cPath_,
				 bool bBatch_);
	//デストラクタ
	virtual ~IDVectorFile();

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);

	// 挿入する
	void insert(ModUInt32 uiKey_, ModUInt32 uiValue_);
	// 削除する
	void expunge(ModUInt32 uiKey_);
	
	// 取得する
	bool get(ModUInt32 uiKey_, ModUInt32& uiValue_);
	// すべての値を取得する(appendする)
	void getAll(Common::LargeVector<ModUInt32>& vecValue_);

private:
	// キー値から格納領域を得る
	const char* getConstBuffer(ModUInt32 uiKey_);
	char* getBuffer(ModUInt32 uiKey_);

	// キー値からページIDとオフセットを得る
	Version::Page::ID convertToPageID(ModUInt32 uiKey_, int& offset_) const
	{
		Version::Page::ID id = uiKey_ / getCountPerPage() + 1;
		offset_ = static_cast<int>(
			(uiKey_ % getCountPerPage()) * sizeof(ModUInt32));
		return id;
	}

	// 1ページに格納できる個数を得る
	ModSize getCountPerPage() const
	{
		ModSize totalSize
			= Version::Page::getContentSize(m_pVersionFile->getPageSize());
		return totalSize / sizeof(ModUInt32);
	}
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_IDVECTORFILE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
