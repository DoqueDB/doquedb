// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PageManagePage.cpp --
//		物理ページ管理機能付き物理ファイルの物理ページ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"

#include "PhysicalFile/PageManagePage.h"
#include "PhysicalFile/PageManageFile.h"
#include "PhysicalFile/PageManageFile2.h"

#include "Exception/Unexpected.h"

_SYDNEY_USING

using namespace PhysicalFile;

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManagePageクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::PageManagePage::PageManagePage -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Trans::Transaction&					Transaction_
//		トランザクション記述子
//	const PhysicalFile::File*					File_
//		物理ファイル記述子
//	const PhysicalFile::PageID					PageID_
//		物理ページ識別子
//	const Buffer::Page::FixMode::Value			FixMode_
//		フィックスモード
//	const Buffer::ReplacementPriority::Value	ReplacementPriority_
//																= Middle
//		バッファリング内容の破棄されにくさ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageManagePage::PageManagePage(
	const Trans::Transaction&					Transaction_,
	File*										File_,
	const PageID								PageID_,
	const Buffer::Page::FixMode::Value			FixMode_,
	const Buffer::ReplacementPriority::Value	ReplacementPriority_
															// = Middle
	)
	: Page(Transaction_,
		   File_,
		   PageID_,
		   FixMode_,
		   ReplacementPriority_),
	  m_File(File_)
{
	this->m_VersionPageTop = this->m_PhysicalPageTop;
}

//
//	FUNCTION private
//	PhysicalFile::PageManagePage::PageManagePage -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	File*								File_
//		物理ファイル記述子
//	const PageID						PageID_
//		物理ページ識別子
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	Admin::Verification::Progress&		Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//
//	EXCEPTIONS
//	[YET!]
//
PageManagePage::PageManagePage(
	const Trans::Transaction&			Transaction_,
	File*								File_,
	const PageID						PageID_,
	const Buffer::Page::FixMode::Value	FixMode_,
	Admin::Verification::Progress&		Progress_)
	: Page(Transaction_,
		   File_,
		   PageID_,
		   FixMode_,
		   Progress_),
	  m_File(File_)
{
	this->m_VersionPageTop = this->m_PhysicalPageTop;
}

//	FUNCTION public
//	PhysicalFile::PageManagePage::getPageDataSize --
//		物理ページデータサイズを返す
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
PageSize
PageManagePage::getPageDataSize(const AreaNum	AreaNum_) const
{
	switch(m_File->m_Type)
	{
	case PageManageType:
		return _SYDNEY_DYNAMIC_CAST(PageManageFile*, m_File)
			->getPageDataSize(AreaNum_);
	case PageManageType2:
		return _SYDNEY_DYNAMIC_CAST(PageManageFile2*, m_File)
			->getPageDataSize(AreaNum_);
	default:
		_SYDNEY_THROW0(Exception::Unexpected);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManagePageクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::PageManagePage::~PageManagePage --
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
PageManagePage::~PageManagePage()
{
}

//
//	FUCNTION private
//	PhysicalFile::PageManagePage::reset -- メモリー再利用
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManagePage::reset(
	const Trans::Transaction&			Transaction_,
	PageID								PageID_,
	Buffer::Page::FixMode::Value		FixMode_,
	Buffer::ReplacementPriority::Value	ReplacementPriority_)
{
	Page::reset(Transaction_,
				m_File,
				PageID_,
				FixMode_,
				ReplacementPriority_);
	m_VersionPageTop = m_PhysicalPageTop;
}

#ifdef DEBUG
//	FUNCTION private
//	PhysicalFile::PageManagePage::getUserAreaSize --
//		利用者に公開する領域のサイズを返す
//
//	NOTES
//	利用者に公開する領域のサイズを返す。
//	物理ページ管理機能付き物理ファイルの場合、
//	物理ページ内に物理ファイルマネージャが使用する領域は
//	存在しないので、バージョンページデータサイズをそのまま返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageSize
//		利用者に公開する領域のサイズ [byte]
//
//	EXCEPTIONS
//	なし

PageSize
PageManagePage::getUserAreaSize() const
{
	return m_File->m_VersionPageDataSize;
}
#endif

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
