// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedDataPage.h -- 
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

#ifndef __SYDNEY_LOB_COMPRESSEDDATAPAGE_H
#define __SYDNEY_LOB_COMPRESSEDDATAPAGE_H

#include "Lob/Module.h"
#include "Lob/DataPage.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

//
//	CLASS
//	Lob::CompressedDataPage -- DATAページ
//
//	NOTES
//
class CompressedDataPage : public DataPage
{
public:
	//
	//	STRUCT
	//	Lob::CompressedDataPage::CompressedHeader -- ヘッダー
	//
	struct CompressedHeader : public DataPage::Header
	{
		ModSize			m_uiCompressedLength;		// 圧縮データサイズ
		ModSize			m_uiLastUnitLength;			// 最終ユニットサイズ
	};

	//
	//	STRUCT
	//	Lob::CompressedDataPage::CompressedData -- データ
	//
	struct CompressedData
	{
		unsigned short	m_usCompressedLength;
		char			m_pCompressedData[2];
	};

	//
	//	ENUM
	//	Lob::CompressedDataPage::BlockSize -- ブロックサイズ
	//
	enum { BlockSize = 2000 };

	//
	//	TYPEDEF
	//	Lob::CompressedDataPage::Iterator -- イテレータ
	//
	typedef ModVector<CompressedData*>::Iterator Iterator;
	
	// コンストラクタ(1)
	CompressedDataPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_);
	// コンストラクタ(2)
	CompressedDataPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_,
					   PhysicalFile::PageID uiPrevPageID_,
					   PhysicalFile::PageID uiNextPageID_);
	// デストラクタ
	virtual ~CompressedDataPage();

	// データを得る
	char* getData()
	{
		return getBuffer() + sizeof(CompressedHeader);
	}

	// データを得る
	Iterator getData(ModSize uiPosition_, ModSize& uiPrevSize_);

	// データを追加する
	bool append(const char* pBuffer_, ModSize uiLength_);
	// データを削除する
	void truncate(ModSize uiLength_);
	// データを書き換える
	void replace(ModSize uiPosition_,
				 const char* pBuffer_, ModSize uiLength_);

	// データサイズを得る
	ModSize getUsedSize()
	{
		return sizeof(CompressedHeader)
			+ m_pCompressedHeader->m_uiCompressedLength;
	}

	// 先頭を得る
	Iterator begin() { return m_vecEntry.begin(); }
	// 最後を得る
	Iterator end() { return m_vecEntry.end() - 1; }

	// 伸長サイズを得る
	ModSize getLength(Iterator i_);
	// 圧縮サイズを得る
	ModSize getCompressedLength(Iterator i_)
	{
		return (*i_)->m_usCompressedLength;
	}

	// 圧縮されているかどうか
	bool isCompressed(Iterator i_)
	{
		return getLength(i_) > getCompressedLength(i_);
	}

	// データを伸長する
	void uncompress(void* dst_, ModSize& dstLength_, Iterator i);

private:
	// データをロードする
	void load();

	// ユニット長を計算する
	static ModSize calcUnitLength(unsigned short uiLength_)
	{
		return (static_cast<ModSize>(uiLength_) + 1) / 4 * 4 + 4;
	}
	static ModSize calcUnitLength(ModSize uiLength_)
	{
		return calcUnitLength(static_cast<unsigned short>(uiLength_));
	}
	
	// 圧縮する
	static bool compress(void* dst_, ModSize& dstLength_,
						 const void* src_, ModSize srcLength_);
	// 伸長する
	static void uncompress(void* dst_, ModSize& dstLength_,
						   const void* src_, ModSize srcLength_);
	// ヘッダー
	CompressedHeader* m_pCompressedHeader;

	// ファイル
	LobFile& m_cFile;

	// データ
	ModVector<CompressedData*> m_vecEntry;
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_COMPRESSEDDATAPAGE_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
