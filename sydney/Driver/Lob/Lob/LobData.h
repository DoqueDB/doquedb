// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LobData.h -- 
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

#ifndef __SYDNEY_LOB_LOBDATA_H
#define __SYDNEY_LOB_LOBDATA_H

#include "Lob/Module.h"
#include "Lob/LobFile.h"
#include "Lob/BlockPage.h"
#include "Lob/DataPage.h"
#include "Lob/AutoPointer.h"
#include "Lob/ObjectID.h"

#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

//
//	CLASS
//	Lob::LobData -- DIRページ
//
//	NOTES
//
class LobData : public Common::Object
{
public:
	// コンストラクタ
	LobData(LobFile& cFile,
			BlockPage::PagePointer pBlockPage_, BlockPage::Block* pBlock_);
	// デストラクタ
	virtual ~LobData();

	// 取得
	virtual AutoPointer<void> get(ModSize uiPosition_, ModSize& uiLength_);

	// 挿入
	virtual void insert(const void* pBuffer_, ModSize uiLength_);
	// 削除(本当の削除)
	virtual void expunge();

	// 追加
	virtual void append(const void* pBuffer_, ModSize uiLength_);
	// 縮める
	virtual void truncate(ModSize uiLength_);
	// 書き換え
	virtual void replace(ModSize uiPosition_,
						 const void* pBuffer_, ModSize uiLength_);

	// 整合性検査を行う
	virtual void verify(const ObjectID& cObjectID_);

protected:
	// 指定ポジションのページを取り出す
	DataPage::PagePointer attachDataPage(ModSize uiPosition_,
										 ModSize& uiPrevSize_);
	// 最終データページを取り出す
	DataPage::PagePointer attachLastDataPage(ModSize& uiPrevSize_);
	// 新しいページを得る
	DataPage::PagePointer allocateDataPage(DataPage::PagePointer& pPrevPage_);
	// ページを開放する
	void freeDataPage(DataPage::PagePointer& pDataPage_);

	// ファイル
	LobFile& m_cFile;
	// ブロックページ
	BlockPage::PagePointer m_pBlockPage;
	// ブロック
	BlockPage::Block* m_pBlock;
	
private:
	// DATAページのページIDを得る
	PhysicalFile::PageID getDataPageID(ModSize uiPosition_,
									   ModSize& uiPrevSize_);
	// DIRページの最後にデータを挿入する
	void insertDirPage(PhysicalFile::PageID uiPageID_,
					   ModSize uiDataSize_);
	// DIRページの最後のデータを削除する
	void deleteDirPage(PhysicalFile::PageID uiPageID_,
					   ModSize uiDataSize_);
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_LOBDATA_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
