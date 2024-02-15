// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Content.cpp -- 物理ページのバッファリング内容関連の関数定義
// 
// Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "PhysicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "PhysicalFile/AreaManagePage.h"
#include "PhysicalFile/Content.h"
#include "PhysicalFile/File.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Contentクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFlie::Content::Content -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]

Content::Content()
	: m_Memory(0),
	  m_PhysicalFileType(AreaManageType),
	  m_PageSize(0),
	  m_PageDataSize(0)
{}

//
//	FUNCTION public
//	PhysicalFile::Content::~Content --
//		デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Content::~Content()
{
}

//
//	FUNCTION public
//	PhysicalFile::Content::operator void* --
//		void*へのキャスト演算子
//
//	NOTES
//	void*へのキャスト演算子。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	void*
//		物理ページのバッファリング内容へのポインタ
//
//	EXCEPTIONS
//	なし
//
Content::operator void*() const
{
	// 物理ページのバッファリング内容へのポインタを得る

	// [YET!]

	char*	pagePointer = 0;

	if (this->m_FixMode == Buffer::Page::FixMode::ReadOnly)
	{
		pagePointer =
			const_cast<char*>(static_cast<const Version::Page::Memory&>(*(this->m_Memory)).operator const char*());
			//const_cast<char*>(this->m_Memory->operator const char*());
	}
	else
	{
		pagePointer = this->m_Memory->operator char*();
	}

	if (this->m_PhysicalFileType == AreaManageType) {

		// 空き領域管理機能付き物理ファイル…

		// 物理ページのバッファリング内容へのポインタを
		// 利用者に公開する領域先頭へ移動する
		// （物理ページヘッダを非公開とする）

		pagePointer += AreaManagePageHeader::getSize(m_PageSize);


	} else if (this->m_PhysicalFileType == NonManageType) {

		// 管理機能なし物理ファイル…

		pagePointer += FileHeader::getSize(NonManageType);
	}

	return pagePointer;
}

//
//	FUNCTION public
//	PhysicalFile::Content::operator char* --
//		char*へのキャスト演算子
//
//	NOTES
//	char*へのキャスト演算子。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	char*
//		物理ページのバッファリング内容へのポインタ
//
//	EXCEPTIONS
//	なし
//
Content::operator char*() const
{
	return static_cast<char*>(this->operator void*());
}

//
//	FUNCTION public
//	PhysicalFile::Content::operator const void* --
//		const void*へのキャスト演算子
//
//	NOTES
//	void*へのキャスト演算子。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const void*
//		物理ページのバッファリング内容へのポインタ
//
//	EXCEPTIONS
//	なし
//
Content::operator const void*() const
{
       //const char*	pagePointer = this->m_Memory->operator const char*();
        const char*	pagePointer =static_cast<const Version::Page::Memory&>(*(this->m_Memory)).operator const char*();

	if (this->m_PhysicalFileType == AreaManageType) {

		pagePointer += AreaManagePageHeader::getSize(m_PageSize);

	} else if (this->m_PhysicalFileType == NonManageType) {

		pagePointer += FileHeader::getSize(NonManageType);
	}

	return pagePointer;
}

//
//	FUNCTION public
//	PhysicalFile::Content::operator const char* --
//		const char*へのキャスト演算子
//
//	NOTES
//	const char*へのキャスト演算子。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const char*
//		物理ページのバッファリング内容へのポインタ
//
//	EXCEPTIONS
//	なし
//
Content::operator const char*() const
{
	return static_cast<const char*>(this->operator const void*());
}

//
//	FUNCTION public
//	PhysicalFile::Content::getSize --
//		バッファリング内容のサイズを返す
//
//	NOTES
//	バッファリング内容のサイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Os::Memory::Size
//		バッファリング内容のサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
Os::Memory::Size
Content::getSize() const
{
	// バージョンページのバッファリング内容サイズを得る
	Os::Memory::Size	contentSize = this->m_Memory->getSize();

	if (this->m_PhysicalFileType == AreaManageType)	{

		// 空き領域管理機能付き物理ファイル…

		// 物理ページヘッダの物理サイズを引く
		contentSize -= (contentSize - m_PageDataSize);

		// 物理ページヘッダ内に記録されている
		// 「管理している物理エリア数」を得る
		AreaNum	areaNum =
			AreaManagePageHeader::getManageAreaNum(
				m_PageSize,
				this->m_Memory->operator void*());

		// 物理エリア管理ディレクトリの物理サイズを引く
		contentSize -= Area::Directory::getSize(m_PageSize, areaNum);

	} else if (this->m_PhysicalFileType == NonManageType) {

		// 管理機能なし物理ファイル…

		// 物理ファイルヘッダの物理サイズを引く
		contentSize -= FileHeader::getSize(NonManageType);
	}

	return contentSize;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Contentクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION private
//	PhysicalFile::Content::Content --
//		コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Version::Page::Memory&	VersionPageContent_
//		バージョンページのバッファリング内容への参照
//	PhysicalFile::Type		PhysicalFileType_
//		物理ファイルタイプ
//	PhysicalFile::PageSize	PageSize_
//		バージョンページサイズ [byte]
//	PhysicalFile::PageSize	PageDataSize_
//		バージョンページデータサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

Content::Content(Version::Page::Memory*			Memory_,
				 Type							PhysicalFileType_,
				 Buffer::Page::FixMode::Value	FixMode_,
				 PageSize					   	PageSize_,
				 PageSize						PageDataSize_)
	: m_Memory(Memory_),
	  m_PhysicalFileType(PhysicalFileType_),
	  m_FixMode(FixMode_),
	  m_PageSize(PageSize_),
	  m_PageDataSize(PageDataSize_)
{}

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
