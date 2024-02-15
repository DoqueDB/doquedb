// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NonManageFile.cpp --
//		管理機能なし物理ファイル関連の関数定義
// 
// Copyright (c) 2002, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#include "PhysicalFile/NonManageFile.h"
#include "PhysicalFile/NonManagePage.h"

#include "Common/Assert.h"
#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::NonManageFileクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::NonManageFile::recoverPage --
//		物理ページ記述子を破棄し、ページ内容を元に戻す
//
//	NOTES
//	※ FixMode::WriteとFixMode::Discardableを
//	　 マスクしたモードでアタッチした物理ページにのみ有効。
//
//	ARGUMENTS
//	PhysicalFile::Page*&	Page_
//		破棄する物理ページの記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
NonManageFile::recoverPage(Page*&	Page_)
{
	this->detachPage(Page_, Page::UnfixMode::NotDirty);
}

//
//	FUNCTION public
//	PhysicalFile::NonManageFile::attachPage -- 物理ページ記述子を生成する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&					Transaction_
//		トランザクション記述子への参照
//	const Buffer::Page::FixMode::Value			FixMode_
//		フィックスモード
//	const Buffer::ReplacementPriority::Value	ReplacementPriority_
//																= Low
//		バッファリング内容の破棄されにくさ
//
//	RETURN
//	PhysicalFile::Page*
//		物理ページ記述子
//
//	EXCEPTIONS
//	Exception::IllegalFileAccess
//		異なるフィックスモードで既にアタッチ中の物理ページの
//		記述子を生成しようとした
//		または、
//		整合性検査中に呼び出された
//	Exception::MemoryExhaust
//		メモリ不足のため、物理ページ記述子を生成できなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
Page*
NonManageFile::attachPage(
	const Trans::Transaction&					Transaction_,
	const Buffer::Page::FixMode::Value			FixMode_,
	const Buffer::ReplacementPriority::Value	ReplacementPriority_
	                                                         // = Low
	)
{
	return File::attachPage(Transaction_,
							NonManagePage::ID,
							FixMode_,
							ReplacementPriority_);
}

//
//	FUNCTION public
//	PhysicalFile::NonManageFile::getTopPageID --
//		先頭の使用中の物理ページの識別子を返す
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::PageID
//		先頭の使用中の物理ページの識別子
//
//	EXCEPTIONS
//	なし
//
PageID
NonManageFile::getTopPageID(const Trans::Transaction&	Transaction_)
{
	return NonManagePage::ID;
}

//
//	FUNCTION public
//	PhysicalFile::NonManageFile::getLastPageID --
//		最後の使用中の物理ページの識別子を返す
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::PageID
//		最後の使用中の物理ページの識別子
//
//	EXCEPTIONS
//	なし
//
PageID
NonManageFile::getLastPageID(const Trans::Transaction&	Transaction_)
{
	return NonManagePage::ID;
}

//
//	FUNCTION public
//	PhysicalFie::NonManageFile::verifyPage --
//		物理ページ記述子を生成する（バージョンページの整合性検査付き）
//
//	NOTES
//	バージョンページの整合性検査を行い、不整合がなければ、
//	物理ページ記述子を生成し、返す。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	Admin::Verification::Progress&		Progress_
//		整合性検査のへの参照
//		(output)
//
//	RETURN
//	PhysicalFile::Page*
//		物理ページ記述子
//
//	EXCEPTIONS
//	Exception::IllegalFileAccess
//		異なるフィックスモードで既にアタッチ中の物理ページの
//		記述子を生成しようとした
//		または、
//		整合性検査中でないときに呼び出された
//	Exception::MemoryExhaust
//		メモリ不足のため、物理ページ記述子を生成できなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
Page*
NonManageFile::verifyPage(
	const Trans::Transaction&			Transaction_,
	const Buffer::Page::FixMode::Value	FixMode_,
	Admin::Verification::Progress&		Progress_)
{
	return File::verifyPage(Transaction_,
							NonManagePage::ID,
							FixMode_,
							Progress_);
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::NonManageFileクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::NonManageFile::NonManageFile -- コンストラクタ
//
//	NOTES
//	管理機能なし物理ファイルの記述子を生成する。
//
//	ARGUMENTS
//	const PhysicalFile::File::StorageStrategy&		FileStorageStrategy_
//		物理ファイル格納戦略への参照
//	const PhysicalFile::File::BufferingStrategy&	BufferingStrategy_
//		物理ファイルバッファリング戦略への参照
//	const Lock::FileName*							LockName_
//		物理ファイルが存在する論理ファイルのロック名へのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::BadArgument
//		不正な引数
//
NonManageFile::NonManageFile(
	const File::StorageStrategy&	FileStorageStrategy_,
	const File::BufferingStrategy&	BufferingStrategy_,
	const Lock::FileName*			LockName_,
	bool							batch_)
	: File(FileStorageStrategy_,
		   BufferingStrategy_,
		   LockName_,
		   batch_)
{
	// バージョンファイル記述子は、
	// NonManageFileクラスの親クラスである
	// Fileクラスのコンストラクタ内で生成している。

	//
	// 以下が、管理機能なし物理ファイル記述子を生成するための
	// 固有の処理
	//

	// バージョンページデータサイズを取得する
	try {

		this->m_VersionPageSize =
			Version::File::verifyPageSize(FileStorageStrategy_.m_VersionFileInfo._pageSize);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// （多分、）不正な引数…
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
	this->m_VersionPageDataSize = Version::Page::getContentSize(this->m_VersionPageSize);

	// 物理ページデータサイズ（公開領域最大サイズ）を設定する
	this->m_UserAreaSizeMax = this->m_VersionPageDataSize - FileHeader::getSize(NonManageType);
}

//
//	FUNCTION private
//	PhysicalFile::NonManageFile::~NonManageFile -- デストラクタ
//
//	NOTES
//	管理機能なし物理ファイルの記述子を破棄する。
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
NonManageFile::~NonManageFile()
{
	// バージョンファイル記述子は、
	// NonManageFileクラスの親クラスである
	// Fileクラスのデストラクタ内で破棄している。
}

//	FUNCTION private
//	PhysicalFile::NonManageFile::allocatePageInstance -- Allocate Page instance
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Page*
NonManageFile::allocatePageInstance(
	const Trans::Transaction&			cTransaction_,
	PageID								uiPageID_,
	Buffer::Page::FixMode::Value		eFixMode_,
	Admin::Verification::Progress*		pProgress_,
	Buffer::ReplacementPriority::Value	eReplacementPriority_)
{
	if (pProgress_ == 0)
	{
		return new NonManagePage(
			cTransaction_, this, uiPageID_, eFixMode_, eReplacementPriority_);
	}
	else
	{
		// For verify
		return new NonManagePage(
			cTransaction_, this, uiPageID_, eFixMode_, *pProgress_);
	}
}

//
//	FUNCTION private
//	PhysicalFile::NonManageFile::getPageDataSize --
//		物理ページデータサイズを返す
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Memory::Size		VersionPageSize_
//		バージョンページサイズ [byte]
//	const PhysicalFile::AreaNum	DummyAreaNum_
//		ダミーの物理エリア数
//		※ public版のgetPageDataSizeと
//		　 インタフェースを分けるために必要。
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページデータサイズ [byte]
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる
// static
PageSize
NonManageFile::getPageDataSize(const Os::Memory::Size	VersionPageSize_,
							   const AreaNum			DummyAreaNum_)
{
	// バージョンページに格納可能な内容のサイズを得る
	Os::Memory::Size	verifyVersionPageSize =
		Version::File::verifyPageSize(VersionPageSize_);
	Os::Memory::Size	versionPageContentSize =
		Version::Page::getContentSize(verifyVersionPageSize);

	// 物理ファイルヘッダの分を引いて返す
	return versionPageContentSize - FileHeader::getSize(NonManageType);
}

//
//	FUNCTION private
//	PhysicalFile::NonManageFile::convertToVersionPageID --
//		物理ページ識別子からバージョンページ識別子へ変換する
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	Version::Page::ID
//		バージョンページ識別子
//
//	EXCEPTIONS
//	なし
//
Version::Page::ID
NonManageFile::convertToVersionPageID(const PageID	PageID_) const
{
	; _SYDNEY_ASSERT(PageID_ == NonManagePage::ID);

	return NonManagePage::ID;
}

//
//	FUNCTION private
//	PhysicalFile::NonManageFile::checkPhysicalFile --
//		管理機能なし物理ファイルの整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
void
NonManageFile::checkPhysicalFile(const Trans::Transaction&		Transaction_,
								 Admin::Verification::Progress&	Progress_)
{
	//
	// 管理機能なし物理ファイルでは、検査する項目がない。
	//

	return;
}

//
//	Copyright (c) 2002, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
