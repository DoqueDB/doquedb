// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleFile.h -- 
// 
// Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR2_SIMPLEFILE_H
#define __SYDNEY_VECTOR2_SIMPLEFILE_H

#include "Vector2/Module.h"
#include "Vector2/VectorFile.h"

_SYDNEY_BEGIN
_SYDNEY_VECTOR2_BEGIN

//
//	CLASS
//	Vector2::SimpleFile --
//
//	NOTES
//	【SimpleFileとは】
//
class SimpleFile : public VectorFile
{
public:
	// コンストラクタ
	SimpleFile(const FileID& cFileID_);
	// デストラクタ
	virtual ~SimpleFile();

	// 整合性検査をする
	void verify();
	//void verify(const Trans::Transaction& cTransaction_,
	//			const unsigned int uiTreatment_,
	//			Admin::Verification::Progress& cProgress_);

	// 指定されたエントリを得る
	bool fetch(ModUInt32 uiKey_,
			   Common::DataArrayData& cTuple_,
			   const int* pField_,
			   //@@const ModVector<int>& vecField_,
			   const ModSize uiFieldCount_);

	// 次のエントリを得る
	ModUInt32 next(ModUInt32 uiKey_,
				   Common::DataArrayData& cTuple_,
				   const int* pField_,
				   const ModSize uiFieldCount_,
				   bool bGetByBitset_ = false);
	// 前のエントリを得る
	ModUInt32 prev(ModUInt32 uiKey_,
				   Common::DataArrayData& cTuple_,
				   const int* pField_,
				   const ModSize uiFieldCount_);

	// 挿入する
	void insert(ModUInt32 uiKey_,
				const Common::DataArrayData& cTuple_);
	// 削除する
	void expunge(ModUInt32 uiKey_);
	// 更新する
	void update(ModUInt32 uiKey_,
				const Common::DataArrayData& cTuple_,
				const int* pUpdateField_,
				//@@const ModVector<int>& vecUpdateField_,
				const ModSize uiFieldCount_);

	// ページをリセットする
	void resetPage(Version::Page::Memory& page);

private:
	// データ領域の先頭を得る
	static char* getPageData(Version::Page::Memory& page_);
	static const char* getConstPageData(const Version::Page::Memory& page_);
	
	// ファイルを作成する
	void create(const Trans::Transaction& cTransaction_);
	
	// ページのデータ領域のサイズを得る
	Os::Memory::Size getPageDataSize() const
		{ return m_cPageManager.getPageDataSize() - sizeof(PageHeader); }
};

_SYDNEY_VECTOR2_END
_SYDNEY_END

#endif // __SYDNEY_VECTOR2_SIMPLEFILE_H

//
//	Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
