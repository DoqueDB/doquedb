// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedDataPage.cpp --
// 
// Copyright (c) 2003, 2009, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Lob";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Lob/CompressedDataPage.h"
#include "Lob/LobFile.h"

#include "Exception/NotSupported.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"

#include "ModOsDriver.h"

#include "zlib.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

//
//	FUNCTION public
//	Lob::CompressedDataPage::CompressedDataPage -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	Lob::LobFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
CompressedDataPage::CompressedDataPage(LobFile& cFile_,
									   PhysicalFile::Page* pPhysicalPage_)
	: DataPage(cFile_, pPhysicalPage_), m_cFile(cFile_)
{
	m_pCompressedHeader = syd_reinterpret_cast<CompressedHeader*>(getBuffer());
	load();
}

//
//	FUNCTION public
//	Lob::CompressedDataPage::CompressedDataPage -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	Lob::LobFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//	PhysicalFile::PageID uiPrevPageID_
//		前のページ
//	PhysicalFile::PageID uiNextPageID_
//		次のページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
CompressedDataPage::CompressedDataPage(LobFile& cFile_,
									   PhysicalFile::Page* pPhysicalPage_,
									   PhysicalFile::PageID uiPrevPageID_,
									   PhysicalFile::PageID uiNextPageID_)
	: DataPage(cFile_, pPhysicalPage_, uiPrevPageID_, uiNextPageID_),
	  m_cFile(cFile_)
{
	m_pCompressedHeader = syd_reinterpret_cast<CompressedHeader*>(getBuffer());
	m_pCompressedHeader->m_uiCompressedLength = 0;
	m_pCompressedHeader->m_uiLastUnitLength = 0;
	load();
}

//
//	FUNCTION public
//	Lob::CompressedDataPage::~CompressedDataPage -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
CompressedDataPage::~CompressedDataPage()
{
}

//
//	FUNCTION public
//	Lob::CompressedDataPage::getData -- データを得る
//
//	NOTES
//
//	ARGUMENS
//	ModSize uiPosition_
//		開始位置
//	ModSize& uiPrevSize_
//		ブロック以前のデータサイズ
//
//	RETURN
//	Lob::CompressedDataPage::Iterator
//		該当ブロックへのイテレータ
//
//	EXCEPTIONS
//
CompressedDataPage::Iterator
CompressedDataPage::getData(ModSize uiPosition_, ModSize& uiPrevSize_)
{
	uiPrevSize_ = 0;
	Iterator i = begin();
	for (ModSize n = 0; n < (uiPosition_ / BlockSize); ++i, ++n)
	{
		uiPrevSize_ += getLength(i);
	}
	return i;
}

//
//	FUNCTION public
//	Lob::CompressedDataPage::append -- データを追加する
//
//	NOTES
//	BlockSize以下でのappendしかサポートしない
//
//	ARGUMENTS
//	const char* pBuffer_
//		追加するデータ
//	ModSize uiLength_
//		追加するデータの長さ
//
//	RETURN
//	bool
//		格納できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
CompressedDataPage::append(const char* pBuffer_, ModSize uiLength_)
{
	if (m_pCompressedHeader->m_uiCompressedLength != 0
		&& m_pCompressedHeader->m_uiLastUnitLength != BlockSize)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	if (uiLength_ > BlockSize)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	ModSize clength = uiLength_ - 1;
	char data[BlockSize];
	if (compress(data, clength, pBuffer_, uiLength_) == true)
	{
		// 圧縮できた
		pBuffer_ = data;
	}
	else
	{
		// 圧縮できなかった
		clength = uiLength_;
	}

	// 収まるかのチェック
	ModSize totalLength = calcUnitLength(clength);
	if (totalLength > getFreeSize())
	{
		// 収まらない
		return false;
	}

	dirty();
	Iterator i = end();
	(*i)->m_usCompressedLength = static_cast<unsigned short>(clength);
	ModOsDriver::Memory::copy((*i)->m_pCompressedData, pBuffer_, clength);
	m_pCompressedHeader->m_uiLength += uiLength_;
	m_pCompressedHeader->m_uiCompressedLength += totalLength;
	m_pCompressedHeader->m_uiLastUnitLength = uiLength_;
	
	CompressedData* p = syd_reinterpret_cast<CompressedData*>(
		syd_reinterpret_cast<char*>(*i) + totalLength);
	m_vecEntry.pushBack(p);
	
	return true;
}

//
//	FUNCTION public
//	Lob::CompressedDataPage::truncate --  データを削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLength_
//		削除する長さ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedDataPage::truncate(ModSize uiLength_)
{
	if (uiLength_ == 0) return;
	
	dirty();
	Iterator i = end();
	--i;
	while (uiLength_ != 0)
	{
		CompressedData* p = (*i);
		
		ModSize s = calcUnitLength(p->m_usCompressedLength);
		m_pCompressedHeader->m_uiCompressedLength -= s;
		ModSize uiLastUnitLength = m_pCompressedHeader->m_uiLastUnitLength;
		m_pCompressedHeader->m_uiLength -= uiLastUnitLength;
		m_pCompressedHeader->m_uiLastUnitLength = BlockSize;
		
		if (uiLength_ <= uiLastUnitLength)
		{
			char data[BlockSize];
			if (uiLength_ < uiLastUnitLength)
			{
				// このユニットの途中なので消しすぎた分を追加する
				ModSize length = uiLastUnitLength;
				uncompress(data, length, i);
			}
			
			// 終わり
			m_vecEntry.erase(i + 1, m_vecEntry.end());
			
			if (uiLength_ < uiLastUnitLength)
			{
				// このユニットの途中なので消しすぎた分を追加する
				append(data, uiLastUnitLength - uiLength_);
			}

			uiLength_ = 0;
		}
		else
		{
			uiLength_ -= uiLastUnitLength;
			--i;
		}
	}
}

//
//	FUNCTION public
//	Lob::CompressedDataPage::replace -- データを書き換える
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		書き換える先頭位置
//	const char* pBuffer_
//		書き換えるデータ
//	ModSize uiLength_
//		書き換えるデータの長さ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedDataPage::replace(ModSize uiPosition_,
							const char* pBuffer_, ModSize uiLength_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Lob::CompressedDataPage::getLength -- 伸長後サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	Lob::CompressedDataPage::Iterator i_
//		伸長後サイズを得るイテレータ
//
//	RETURN
//	ModSize
//		伸長後サイズ
//
//	EXCEPTIONS
//
ModSize
CompressedDataPage::getLength(Iterator i_)
{
	if ((end() - 1) == i_)
		return m_pCompressedHeader->m_uiLastUnitLength;
	return BlockSize;
}

//
//	FUNCTION public
//	Lob::CompressedDataPage::uncompress -- 伸長する
//
//	NOTES
//
//	ARGUMENTS
//	void* destination
//		伸長後のデータ
//	ModSize& destinationLength
//		伸長後のデータ長
//	Lob::CompressedDataPage::Iterator i
//		伸長するデータへのイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedDataPage::uncompress(void* destination, ModSize& destinationLength,
							   Iterator i)
{
	if (isCompressed(i) == true)
	{
		uncompress(destination, destinationLength,
				   (*i)->m_pCompressedData, (*i)->m_usCompressedLength);
	}
	else
	{
		destinationLength = (*i)->m_usCompressedLength;
		ModOsDriver::Memory::copy(destination, (*i)->m_pCompressedData,
								  destinationLength);
	}
}

//
//	FUNCTION private
//	Lob::CompressedDataPage::load -- データをロードする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedDataPage::load()
{
	m_vecEntry.clear();
	char* p = getData();
	ModSize uiSize = 0;
	while (uiSize < m_pCompressedHeader->m_uiCompressedLength)
	{
		CompressedData* pData = syd_reinterpret_cast<CompressedData*>(p);
		m_vecEntry.pushBack(pData);
		ModSize s = calcUnitLength(pData->m_usCompressedLength);
		uiSize += s;
		p += s;
	}
	m_vecEntry.pushBack(syd_reinterpret_cast<CompressedData*>(p));
}

//
//	FUNCTION private static
//	Lob::CompressedDataPage::compress -- 圧縮する
//
//	NOTES
//
//	ARGUMENTS
//	void* destination
//		圧縮後のデータ
//	ModSize& destinationLength
//		圧縮後のデータ長
//	const void* source
//		圧縮前のデータ
//	ModSize sourceLength
//		圧縮前のデータ長
//
//	RETURN
//	bool
//		圧縮できた場合はtrue、できなかった場合はfalse
//
//	EXCEPTIONS
//
bool
CompressedDataPage::compress(void* destination, ModSize& destinationLength,
							 const void* source, ModSize sourceLength)
{
	uLongf len = destinationLength;
	int error = ::compress((Byte*)destination, &len,
						   (const Bytef*)source, sourceLength);
	switch(error) {
	case Z_OK:
		break;
	case Z_MEM_ERROR:
		_SYDNEY_THROW0(Exception::MemoryExhaust);
	case Z_BUF_ERROR:
		return false;
	default:
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	destinationLength = static_cast<ModSize>(len);
	return true;
}

//
//	FUNCTION private static
//	Lob::CompressedDataPage::uncompress -- 伸長する
//
//	NOTES
//
//	ARGUMENTS
//	void* destination
//		伸長後のデータ
//	ModSize& destinationLength
//		伸長後のデータ長
//	const void* source
//		伸長前のデータ
//	ModSize sourceLength
//		伸長前のデータ長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedDataPage::uncompress(void* destination, ModSize& destinationLength,
							   const void* source, ModSize sourceLength)
{
	uLongf len = destinationLength;
	int error = ::uncompress((Byte*)destination, &len,
							 (const Bytef*)source, sourceLength);
	switch(error) {
	case Z_OK:
		break;
	case Z_MEM_ERROR:
		_SYDNEY_THROW0(Exception::MemoryExhaust);
	case Z_BUF_ERROR:
	case Z_DATA_ERROR:
		_SYDNEY_THROW0(Exception::BadArgument);
	default:
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	destinationLength = static_cast<ModSize>(len);
}

//
//	Copyright (c) 2003, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
