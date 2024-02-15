// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiVectorFile.cpp -- 固定長データを格納するベクターファイル
// 
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/MultiVectorFile.h"

#include "FullText2/FakeError.h"

#include "Common/Assert.h"
#include "Common/Data.h"

#include "Version/File.h"

#include "Os/File.h"
#include "Os/Memory.h"

#include "Schema/File.h"

#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	FUNCTION local
	//	_$$::_IsNull -- 該当するビットがnullかどうか
	//
	//	NOTES
	//	新しく確保されたページはすべて0xffで初期化されているので、
	//	１つも挿入されていないページのnullビットマップはすべて立っている
	//
	bool _IsNull(const char* bitmap_, ModSize pos_)
	{
		bitmap_ += (pos_ / 8);
		return (*bitmap_ & (1 << (pos_ % 8))) ? true : false;
	}

	//
	//	FUNCTION local
	//	_$$::_BitOn -- 該当するビットをONにする
	//
	void _BitOn(char* bitmap_, ModSize pos_)
	{
		bitmap_ += (pos_ / 8);
		*bitmap_ |= (1 << (pos_ % 8));
	}

	//
	//	FUNCTION local
	//	_$$::_BitOff -- 該当するビットをOFFにする
	//
	void _BitOff(char* bitmap_, ModSize pos_)
	{
		bitmap_ += (pos_ / 8);
		*bitmap_ &= ~(1 << (pos_ % 8));
	}
}

//
//	FUNCTION public
//	FullText2::MultiVectorFile::MultiVectorFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		このファイルを格納するパス
//	const ModVector<Common::DataType::Value>& vecElement_
//		各要素の型
//	bool bBatch_
//		バッチモードか否か
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
MultiVectorFile::
MultiVectorFile(const FullText2::FileID& cFileID_,
				const Os::Path& cPath_,
				const ModVector<Common::DataType::Type>& vecElement_,
				bool bBatch_)
	: VectorFile(cFileID_, cPath_, bBatch_),
	  m_uiFieldCount(0), m_uiElementSize(0)
{
	// 要素数
	m_uiFieldCount = vecElement_.getSize();

	// サイズ
	ModVector<Common::DataType::Type>::ConstIterator i = vecElement_.begin();
	for (; i < vecElement_.end(); ++i)
	{
		// 固定長しか与えられないはずなので、それをチェックしたりしない

		ModSize size = Common::Data::getDumpSize(*i);
		m_vecElementSize.pushBack(size);
		
		m_uiElementSize += size;	// 合計サイズ
	}

	// 1ページに格納できるエントリ数
	ModSize totalBit
		= Version::Page::getContentSize(m_pVersionFile->getPageSize()) * 8;
	ModSize elementBit = m_uiElementSize * 8 + m_uiFieldCount;
	m_uiCountPerPage = totalBit / elementBit;
}

//
//	FUNCTION public
//	FullText2::MultiVectorFile::~MultiVectorFile -- デストラクタ
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
MultiVectorFile::~MultiVectorFile()
{
}

//
//	FUNCTION public
//	FullText2::MultiVectorFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		キー値
//	const Common::DataArrayData& cValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiVectorFile::insert(ModUInt32 uiKey_,
						const Common::DataArrayData& cValue_)
{
	; _SYDNEY_ASSERT(cValue_.getCount() == (int)getFieldCount());
		  
	char* bitmap;
	char* buf = getBuffer(uiKey_, bitmap);
	int n = cValue_.getCount();
	for (int i = 0; i < n; ++i)
	{
		const Common::Data::Pointer& p = cValue_.getElement(i);
		if (p->isNull())
		{
			// nullビットマップをONする
			bitOn(bitmap, uiKey_, i);
		}
		else
		{
			// nullビットマップをOFFする
			bitOff(bitmap, uiKey_, i);
			// データを書き出す
			p->dumpValue(buf);
		}
		buf += m_vecElementSize[i];
	}

	readHeader();
	// 登録件数を更新する
	++m_cHeader.m_uiCount;
	// 最大キー値を更新する
	if (m_cHeader.m_uiMaxKey < uiKey_)
		m_cHeader.m_uiMaxKey = uiKey_;
	m_bDirtyHeaderPage = true;

	; _FULLTEXT2_FAKE_ERROR(MultiVectorFile::insert);
}

//
//	FUNCTION public
//	FullText2::MultiVectorFile::update -- 更新する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		キー値
//	int iField_
//		フィールド番号
//	const Common::Data& cValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiVectorFile::update(ModUInt32 uiKey_,
						int iField_,
						const Common::Data& cValue_)
{
	char* bitmap;
	char* buf = getBuffer(uiKey_, bitmap);
	int n = static_cast<int>(getFieldCount());
	for (int i = 0; i < n; ++i)
	{
		if (i == iField_)
		{
			// 更新フィールド番号に一致

			if (cValue_.isNull())
			{
				// nullビットマップをONにする
				bitOn(bitmap, uiKey_, i);
			}
			else
			{
				// nullビットマップをOFFする
				bitOff(bitmap, uiKey_, i);
				cValue_.dumpValue(buf);
			}

			break;
		}
		
		buf += m_vecElementSize[i];
	}
	
	; _FULLTEXT2_FAKE_ERROR(MultiVectorFile::update);
}

//
//	FUNCTION public
//	FullText2::MultiVectorFile::expunge -- 削除する
//
//	NOTES
//
//	ARUGMENTS
//	ModUInt32 uiKey_
//		キー値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiVectorFile::expunge(ModUInt32 uiKey_)
{
	char* bitmap;
	char* buf = getBuffer(uiKey_, bitmap);
	int n = static_cast<int>(getFieldCount());
	for (int i = 0; i < n; ++i)
	{
		// nullビットマップをONする
		bitOn(bitmap, uiKey_, i);
	}
		
	// 登録件数を更新する
	readHeader();
	--m_cHeader.m_uiCount;
	m_bDirtyHeaderPage = true;
	
	; _FULLTEXT2_FAKE_ERROR(MultiVectorFile::expunge);
}

//
//	FUNCTION public
//	FullText2::MultiVectorFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		キー値
//	int iField_
//		取得するフィールド
//	Common::Data& cValue_
//		取得した値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiVectorFile::get(ModUInt32 uiKey_, int iField_, Common::Data& cValue_)
{
	; _SYDNEY_ASSERT(iField_ < (int)getFieldCount());

	const char* bitmap;
	const char* buf = getConstBuffer(uiKey_, bitmap);
	if (buf == 0)
	{
		// そのキーのページは存在していないので、null に設定する
			
		cValue_.setNull();

		return;
	}
		
	int n = static_cast<int>(getFieldCount());
	for (int i = 0; i < n; ++i)
	{
		if (i == iField_)
		{
			// 取得依頼フィールド番号に一致
				
			if (isNull(bitmap, uiKey_, i))
			{
				// null
				cValue_.setNull();
			}
			else
			{
				// not null
				cValue_.setDumpedValue(buf);
			}

			break;
		}
		buf += m_vecElementSize[i];
	}
}

//
//	FUNCTION public
//	FullText2::MultiVectorFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		キー値
//	Common::DataArrayData& cValue_
//		取得した値
//	const ModVector<int>& vecGetFields_
//		取得するフィールド
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiVectorFile::get(ModUInt32 uiKey_, Common::DataArrayData& cValue_,
					 const ModVector<int>& vecGetFields_)
{
	; _SYDNEY_ASSERT(cValue_.getCount() == (int)vecGetFields_.getSize());
	; _SYDNEY_ASSERT(cValue_.getCount() <= (int)getFieldCount());

	const char* bitmap;
	const char* buf = getConstBuffer(uiKey_, bitmap);
	int n = static_cast<int>(getFieldCount());
	ModVector<int>::ConstIterator j = vecGetFields_.begin();
	int k = 0;
	for (int i = 0;j != vecGetFields_.end() && i < n; ++i)
	{
		if (i == *j)
		{
			// 取得依頼フィールド番号に一致
				
			Common::Data::Pointer p = cValue_.getElement(k++);
			if (isNull(bitmap, uiKey_, i))
			{
				// null
				p->setNull();
			}
			else
			{
				// not null
				p->setDumpedValue(buf);
			}
				
			++j;	// 次の依頼へ
		}
		buf += m_vecElementSize[i];
	}
}

//
//	FUNCTION public
//	FullText2::MultiVectorFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		キー値
//	int iField_
//		取得するフィールド
//	ModUInt32& uiValue_
//		取得した値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiVectorFile::get(ModUInt32 uiKey_, int iField_, ModUInt32& uiValue_)
{
	; _SYDNEY_ASSERT(iField_ < (int)getFieldCount());

	//
	//	null はないということが前提
	//

	const char* bitmap;
	const char* buf = getConstBuffer(uiKey_, bitmap);
	if (buf == 0)
	{
		// そのキーのページは存在していない
		_SYDNEY_THROW0(Exception::BadArgument);
	}
		
	int n = static_cast<int>(getFieldCount());
	for (int i = 0; i < n; ++i)
	{
		if (i == iField_)
		{
			// 取得依頼フィールド番号に一致
			Os::Memory::copy(&uiValue_, buf, sizeof(ModUInt32));
			break;
		}
		buf += m_vecElementSize[i];
	}
}

//
//	FUNCTION private
//	FullText2::MultiVectorFile::getConstBuffer -- キー値から格納領域を得る
//	FullText2::MultiVectorFile::getBuffer
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		キー値
//
//	RETURN
//	const char*
//	char*
//		格納領域へのポインタ
//
//	EXCEPTIONS
//
const char*
MultiVectorFile::getConstBuffer(ModUInt32 uiKey_, const char*& bitmap_)
{
	// キー -> ページID+オフセットへ変換
	int offset = 0;
	Version::Page::ID id = convertToPageID(uiKey_, offset);

	// ページを得る
	fixPage(id);

	// メモリーを得る
	const char* buf = m_pCurrentPage->getConstBuffer();
    
	// ビットマップ領域の先頭を得る
	bitmap_ = buf + getCountPerPage() * getElementSize();

	return buf + offset;
}
char*
MultiVectorFile::getBuffer(ModUInt32 uiKey_, char*& bitmap_)
{
	// キー -> ページID+オフセットへ変換
	int offset = 0;
	Version::Page::ID id = convertToPageID(uiKey_, offset);

	if (id > getMaxPageID())
	{
		// 最大ページIDより大きい -> 必要なところまでallocateする
		allocatePage(id);
	}

	// ページを得る
	fixPage(id);

	// メモリーを得る
	char* buf = m_pCurrentPage->getBuffer();
	// 更新するために取得しているので dirty にする
	m_pCurrentPage->dirty();

	// ビットマップ領域の先頭を得る
	bitmap_ = buf + getCountPerPage() * getElementSize();

	return buf + offset;
}

//
//	FUNCTION private
//	FullText2::MultiVectorFile::isNull -- ビットマップをチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const char* bitmap_
//		このページのビットマップ領域の先頭
//	ModUInt32 uiKey_
//		キー値
//	int n_
//		要素番号
//
//	RETURN
//	bool
//		該当するフィールドがnullの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MultiVectorFile::isNull(const char* bitmap_, ModUInt32 uiKey_, int n_) const
{
	ModSize pos = uiKey_ % getCountPerPage();
	pos = pos * getFieldCount() + n_;
	return _IsNull(bitmap_, pos);
}

//
//	FUNCTION private
//	FullText2::MultiVectorFile::bitOn -- ビットをONにする
//
//	NOTES
//
//	ARGUMENTS
//	char* bitmap_
//		このページのビットマップ領域の先頭
//	ModUInt32 uiKey_
//		キー値
//	int n_
//		要素番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiVectorFile::bitOn(char* bitmap_, ModUInt32 uiKey_, int n_)
{
	ModSize pos = uiKey_ % getCountPerPage();
	pos = pos * getFieldCount() + n_;
	_BitOn(bitmap_, pos);
}

//
//	FUNCTION private
//	FullText2::MultiVectorFile::bitOff -- ビットをOFFする
//
//	NOTES
//
//	ARGUMENTS
//	char* bitmap_
//		このページのビットマップ領域の先頭
//	ModUInt32 uiKey_
//		キー値
//	int n_
//		要素番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiVectorFile::bitOff(char* bitmap_, ModUInt32 uiKey_, int n_)
{
	ModSize pos = uiKey_ % getCountPerPage();
	pos = pos * getFieldCount() + n_;
	_BitOff(bitmap_, pos);
}

//
//	Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
