// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NonManagePage.cpp --
// 
// Copyright (c) 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
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

#include "PhysicalFile/NonManagePage.h"

_SYDNEY_USING

using namespace PhysicalFile;

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::NonManagePageクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//
//	CONST private
//	PhysicalFile::NonManagePage::ID --
//		管理機能なし物理ファイルの物理ページ識別子
//
//	NOTES
//	管理機能なし物理ファイルは、
//	ただ一つの物理ページのみで構成される。
//	したがって、その物理ページの識別子は固定である。
//
// static
PageID
NonManagePage::ID = 0;

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::NonManagePageクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::NonManagePage::NonManagePage -- コンストラクタ
//
//	NOTES
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
NonManagePage::NonManagePage(
	const Trans::Transaction&					Transaction_,
	NonManageFile*								File_,
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
	this->m_VersionPageTop = this->m_PhysicalPageTop
		- FileHeader::getSize(NonManageType);
}

//
//	FUNCTION private
//	PhysicalFile::NonManagePage::NonManagePage -- コンストラクタ
//
//	NOTES
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
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
NonManagePage::NonManagePage(
	const Trans::Transaction&			Transaction_,
	NonManageFile*						File_,
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

//
//	FUNCTION private
//	PhysicalFile::NonManagePage::~NonManagePage -- デストラクタ
//
//	NOTES
//	デストラクタ
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
NonManagePage::~NonManagePage()
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
NonManagePage::reset(
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
	m_VersionPageTop = m_PhysicalPageTop - FileHeader::getSize(NonManageType);
}

#ifdef DEBUG
//	FUNCTION private
//	PhysicalFile::NonManagePage::getUserAreaSize --
//		利用者に公開する領域のサイズを返す
//
//	NOTES
//	利用者に公開する領域のサイズを返す。
//	管理機能なし物理ファイルの場合、
//	物理ページ内に物理ファイルヘッダが記録されているので
//	バージョンページデータサイズから
//	物理ファイルヘッダの領域サイズ分をそのまま返す。
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
NonManagePage::getUserAreaSize() const
{
	return this->m_File->m_UserAreaSizeMax;
}
#endif

//
//	Copyright (c) 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
