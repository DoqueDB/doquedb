// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiVectorFile.h -- 固定長データを格納するベクターファイル
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

#ifndef __SYDNEY_FULLTEXT2_MULTIVECTORFILE_H
#define __SYDNEY_FULLTEXT2_MULTIVECTORFILE_H

#include "FullText2/Module.h"
#include "FullText2/FileID.h"
#include "FullText2/VectorFile.h"

#include "Version/File.h"
#include "Version/Page.h"

#include "LogicalFile/OpenOption.h"

#include "Common/DataArrayData.h"

#include "Os/Path.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//	CLASS
//	FullText2::MultiVectorFile -- 固定長データを格納するファイル
//
//	NOTES
//
class MultiVectorFile : public VectorFile
{
public:
	//コンストラクタ
	MultiVectorFile(const FileID& cFileID_,
					const Os::Path& cPath_,
					const ModVector<Common::DataType::Type>& vecElement_,
					bool bBatch_);
	//デストラクタ
	virtual ~MultiVectorFile();

	// 挿入する
	void insert(ModUInt32 uiKey_, const Common::DataArrayData& cValue_);
	// 更新する
	void update(ModUInt32 uiKey_, int iField_, const Common::Data& vValue_);
	// 削除する
	void expunge(ModUInt32 uiKey_);
	
	// 取得する
	void get(ModUInt32 uiKey_, int iField_, Common::Data& cValue_);
	void get(ModUInt32 uiKey_, Common::DataArrayData& cValue_,
			 const ModVector<int>& vecGetFields_);

	// 32ビット固定長専用
	void get(ModUInt32 uiKey_, int iField_, ModUInt32& uiValue_);

private:
	// キー値から格納領域を得る
	const char* getConstBuffer(ModUInt32 uiKey_, const char*& bitmap_);
	char* getBuffer(ModUInt32 uiKey_, char*& bitmap_);

	// キー値からページIDとオフセットを得る
	Version::Page::ID convertToPageID(ModUInt32 uiKey_, int& offset_) const
	{
		Version::Page::ID id = uiKey_ / getCountPerPage() + 1;
		offset_ = static_cast<int>(
			(uiKey_ % getCountPerPage()) * getElementSize());
		return id;
	}

	// 1ページに格納できる個数を得る
	ModSize getCountPerPage() const { return m_uiCountPerPage; }
	// 全要素の合計サイズを得る
	ModSize getElementSize() const { return m_uiElementSize; }
	// フィールド数を得る
	ModSize getFieldCount() const { return m_uiFieldCount; }

	// ビットマップをチェックする
	bool isNull(const char* bitmap_, ModUInt32 uiKey_, int n_) const;

	// ビットマップをONにする
	void bitOn(char* bitmap_, ModUInt32 uiKey_, int n_);
	// ビットマップをOFFにする
	void bitOff(char* bitmap_, ModUInt32 uiKey_, int n_);

	// 要素数
	ModSize m_uiFieldCount;
	// 合計サイズ
	ModSize m_uiElementSize;
	// １ページに格納できるエントリ数
	ModSize m_uiCountPerPage;
	// 各要素のサイズ
	ModVector<ModSize> m_vecElementSize;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MULTIVECTORFILE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
