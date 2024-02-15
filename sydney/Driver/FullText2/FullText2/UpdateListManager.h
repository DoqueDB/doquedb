// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UpdateListManager.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_UPDATELISTMANAGER_H
#define __SYDNEY_FULLTEXT2_UPDATELISTMANAGER_H

#include "FullText2/Module.h"
#include "FullText2/ListManager.h"
#include "FullText2/SmartLocationList.h"

#include "Admin/Verification.h"
#include "Os/Path.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedList;

//
//	CLASS
//	FullText2::UpdateListManager -- 
//
//	NOTES
//	本クラスは更新処理が必要とするインターフェース
//	を規定しているクラスである。
//
class UpdateListManager : public ListManager
{
public:
	// コンストラクタ
	UpdateListManager(FullTextFile& cFile_);
	// デストラクタ
	virtual ~UpdateListManager();
	// コピーコンストラクタ
	UpdateListManager(const UpdateListManager& src_);

	//
	//	以下は検索には利用しないインターフェース
	//
	
	// 転置リストを得る
	virtual InvertedList* getInvertedList() = 0;
	
	// 挿入する
	virtual void insert(ModUInt32 uiDocumentID_,
						const SmartLocationList& cLocationList_) = 0;
	virtual void insert(InvertedList& cInvertedList_) = 0;

	// 削除する
	virtual void expunge(ModUInt32 uiDocumentID_) = 0;
	virtual int expunge(InvertedList& cInvertedList_) = 0;

	// 削除を取り消す
	virtual void undoExpunge(ModUInt32 uiDocumentID_,
							 const SmartLocationList& cLocationList_) = 0;

	// 整合性検査を行う
	virtual void verify(Admin::Verification::Treatment::Value uiTreatment_,
						Admin::Verification::Progress& cProgress_,
						const Os::Path& cRootPath_) = 0;

	// 同じものが登録されているかチェックする
	virtual bool check(ModUInt32 uiDocumentID_,
					   const SmartLocationList& cLocationList_) = 0;

	// 削除対象のIDブロックを削除する
	virtual void expungeIdBlock(const ModVector<ModUInt32>& vecDocumentID_) = 0;

	// バキュームが必要かどうか
	virtual bool isNeedVacuum(int iNewExpungeCount_);
	// バキュームする
	virtual void vacuum();
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_UPDATELISTMANAGER_H

//
//  Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
