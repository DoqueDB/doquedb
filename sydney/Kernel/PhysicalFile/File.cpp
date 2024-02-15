// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp --
//		物理ファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Exception/BadArgument.h"
#include "Exception/IllegalFileAccess.h"
#include "Exception/FileManipulateError.h"
#include "Exception/NotSupported.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/Unexpected.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/ObjectIDData.h"

//#include "Os/AutoCriticalSection.h"

#include "Schema/File.h"

#include "Version/File.h"

#include "PhysicalFile/File.h"
#include "PhysicalFile/Manager.h"
#include "PhysicalFile/AreaManageFile.h"
#include "PhysicalFile/AreaManagePage.h"
#include "PhysicalFile/PageManageFile.h"
#include "PhysicalFile/PageManagePage.h"
#include "PhysicalFile/NonManageFile.h"
#include "PhysicalFile/NonManagePage.h"
#include "PhysicalFile/DirectAreaFile.h"
#include "PhysicalFile/DirectAreaPage.h"
#include "PhysicalFile/PageManageFile2.h"
#include "PhysicalFile/Message_InitializeFailed.h"
#include "PhysicalFile/Message_NotManagePage.h"
#include "PhysicalFile/Message_CanNotFixHeaderPage.h"
#include "PhysicalFile/Message_CanNotFixAreaManageTable.h"
#include "PhysicalFile/Message_CanNotFixPageTable.h"
#include "PhysicalFile/Message_DiscordPageUseSituation1.h"
#include "PhysicalFile/Message_DiscordPageUseSituation2.h"
#include "PhysicalFile/Message_DiscordPageUseSituation3.h"
#include "PhysicalFile/Message_CanNotCorrectPageUseSituation.h"
#include "PhysicalFile/Message_CorrectedPageUseSituation.h"

#include <algorithm>

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Fileクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//
//	CONST protected
//	PhysicalFile::File::DiscardableAllocateFixMode --
//		版を破棄可能な生成フィックスモード
//
//	NOTES
//	版を破棄可能な生成フィックスモード
//
// static
Buffer::Page::FixMode::Value
File::DiscardableAllocateFixMode =
static_cast<Buffer::Page::FixMode::Value>(
	static_cast<int>(Buffer::Page::FixMode::Allocate) |
	static_cast<int>(Buffer::Page::FixMode::Discardable));

//
//	CONST protected
//	PhysicalFile::File::DiscardableWriteFixMode --
//		版を破棄可能な書き込みフィックスモード
//
//	NOTES
//	版を破棄可能な書き込みフィックスモード
//
// static
Buffer::Page::FixMode::Value
File::DiscardableWriteFixMode =
static_cast<Buffer::Page::FixMode::Value>(
	static_cast<int>(Buffer::Page::FixMode::Write) |
	static_cast<int>(Buffer::Page::FixMode::Discardable));

//
//	FUNCTION public
//	PhysicalFile::File::VersionPage::VersionPage -- コンストラクタ
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
PhysicalFile::File::VersionPage::VersionPage()
	: _reference(0), _next(0), _prev(0)
{}

//
//	FUNCTION public
//	PhysicalFile::File::VersionPage::~VersionPage -- デストラクタ
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
PhysicalFile::File::VersionPage::~VersionPage()
{}

//
//	FUNCTION public
//	PhysicalFile::File::VersionPage::operator = -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::File::VersionPage& src
//		コピー元
//
//	RETURN
//	PhysicalFile::File::VersionPage&
//		自分自身
//
//	EXCEPTIONS
//
PhysicalFile::File::VersionPage&
PhysicalFile::File::VersionPage::operator = (const VersionPage& src)
{
	static_cast<Version::Page::Memory*>(this)->operator = (src);
	return *this;
}

//
//	FUNCTION public
//	PhysicalFile::File::VersionPage::operator = -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
//	const Version::Page::Memory& src
//		コピー元
//
//	RETURN
//	PhysicalFile::File::VersionPage&
//		自分自身
//
//	EXCEPTIONS
//
PhysicalFile::File::VersionPage&
PhysicalFile::File::VersionPage::operator = (const Version::Page::Memory& src)
{
	static_cast<Version::Page::Memory*>(this)->operator = (src);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Fileクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::File::create --
//		物理ファイルを生成する
//
//	NOTES
//	物理ファイルを生成する。
//	物理ファイルは、バージョンファイルを用いて実装される。
//	この関数を呼び出すことにより、マスタデータファイルと
//	バージョンログファイルが生成される。
//
//	-----------------------------------------------------
//
//	以下、物理ページ管理機能付き物理ファイル、および、
//	空き領域管理機能付き物理ファイルについて…
//
//	物理ファイル生成時には、
//		1. 物理ファイルヘッダ
//		2. 先頭の空き領域管理表／物理ページ表
//		3. 先頭の物理ページ
//	の3つのバージョンページが確保される。
//
//	※ 先頭の物理ページはバージョンページ確保するだけで、
//	　 まだ“未使用の物理ページ”である。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ファイルを生成できなかった
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
File::create(const Trans::Transaction&	Transaction_)
{
	try {

		// バージョンファイルを生成する
		this->m_VersionFile->create(Transaction_);

	} catch (Exception::Object&) {

		_SYDNEY_RETHROW;

#ifndef NO_CATCH_ALL
	} catch (...) {
		// バージョンファイルを生成できなかった…

		// 物理ファイルを生成できなかった
		throw Exception::FileManipulateError(moduleName,
											 srcFile,
											 __LINE__);
#endif
	}

	Version::Page::ID	versionPageID = FileHeader::VersionPageID;

	try {

		AutoUnfix cUnfix(this);
		
		// 確保モードで物理ファイルヘッダのバッファリング内容を得る
		// （フィックスする）

		//
		// ここではエラーが発生した場合には、
		// ファイルごと破棄してしまうので、
		// Discardableモードは指定しない。
		//

		PagePointer fileHeaderPage
			= fixVersionPage(Transaction_,
							 versionPageID++,
							 Buffer::Page::FixMode::Allocate,
							 Buffer::ReplacementPriority::Middle);

		// 物理ファイルヘッダのバッファリング内容へのポインタを得る
		void*	fileHeaderTop = (void*)(*fileHeaderPage);

		// 物理ファイルヘッダを初期化する
		FileHeader::initialize(fileHeaderTop, this->m_Type);

		// 各タイプの物理ファイルごとの初期化
		this->initialize(Transaction_, fileHeaderTop);

		// すべての変更を確定する
		cUnfix.success();

	} catch (Exception::Object&) {

		// 物理ファイルの初期化ができなかった…

		this->undoCreate(Transaction_);

		_SYDNEY_RETHROW;

#ifndef NO_CATCH_ALL
	} catch (...) {

		// 物理ファイルの初期化ができなかった…

		this->undoCreate(Transaction_);

		// 物理ファイルを生成できなかった
		throw Exception::FileManipulateError(moduleName,
											 srcFile,
											 __LINE__);
#endif
	}
}

//
//	FUNCTION public
//	PhysicalFile::File::destroy --
//		物理ファイルを消去する
//
//	NOTES
//	物理ファイルをディスク上から消去する。
//	物理ファイルは、バージョンファイルを用いて実装される。
//	この関数を呼び出すことにより、
//	マスタデータファイル、バージョンログファイル、
//	同期ログファイルがディスク上から消去される。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ファイルを消去できなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
// virtual
void
File::destroy(const Trans::Transaction&	Transaction_)
{
	try
	{
		// キャッシュしている管理ページをunfixする
		unfixVersionPage(true);

		// バージョンファイルを消去する
		this->m_VersionFile->destroy(Transaction_);
	}
	catch (Exception::Object&)
	{
		_SYDNEY_RETHROW;
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		// バージョンファイルを消去できなかった…

		// 物理ファイルを消去できなかった
		throw Exception::FileManipulateError(moduleName,
											 srcFile,
											 __LINE__);
	}
#endif
}

//
//	FUNCTION public
//	PhysicalFile::File::move --
//		物理ファイルを移動する
//
//	NOTES
//	物理ファイルを移動する。
//
//	ARGUMENTS
//	const Trans::Transaction&					Transaction_
//		トランザクション記述子への参照
//	const Version::File::StorageStrategy::Path&	FilePath_
//		バージョンファイルパス構造体への参照
//		（移動後のバージョンファイルパスを指定する）
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
File::move(const Trans::Transaction&					Transaction_,
		   const Version::File::StorageStrategy::Path&	FilePath_)
{
	this->m_VersionFile->move(Transaction_, FilePath_);
}

//
//	FUNCTION public
//	PhysicalFile::File::isAccessible --
//		物理ファイルを構成する OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//	bool	Force_ = false
//		OSファイルの存在を実際に調べるかどうか
//			true  : OSファイルの存在を実際に調べる
//			false : OSファイルの存在を必要に応じて調べる
//
//	RETURN
//	bool
//		生成されているかどうか
//			true  : 生成されている
//			false : 生成されていない
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
bool
File::isAccessible(bool	Force_) const
{
	return this->m_VersionFile->isAccessible(Force_);
}

//	FUNCTION public
//	PhysicalFile::File::isMounted --
//		物理ファイルがマウントされているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&		trans
//			マウントされているか調べる
//			トランザクションのトランザクション記述子
//
//	RETURN
//		true
//			マウントされている
//		false
//			マウントされていない
//
//	EXCEPTIONS

bool
File::isMounted(const Trans::Transaction& trans) const
{
	; _SYDNEY_ASSERT(m_VersionFile);
	if (m_bMounted == false)
	{
		// falseだったら下位に聞く
		// trueだったらdetachするまでfalseになることはない
		
		m_bMounted = m_VersionFile->isMounted(trans);
	}
	return m_bMounted;
}

//
//	FUNCTION public
//	PhysicalFile::File::getSize --
//		物理ファイルの実体であるバージョンファイルを構成する
//		OS ファイルの総サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS

FileSize
File::getSize() const
{
	; _SYDNEY_ASSERT(m_VersionFile);
	return m_VersionFile->getSize();
}

#ifdef OBSOLETE
//	FUNCTION public
//	PhysicalFile::File::getVersionSize --
//		物理ファイルの実体であるバージョンファイルが使用するサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS

FileSize
File::getVersionSize(const Trans::Transaction& trans)
{
	; _SYDNEY_ASSERT(m_VersionFile);
	return m_VersionFile->getBoundSize(trans);
}
#endif

//	FUNCTION public
//	PhysicalFile::File::getUsedSize --
//		物理ファイルが使用中の総バージョンページサイズを得る
//
//	NOTES
//		物理ファイルが管理する物理ページのうち使用中のものと、
//		物理ページ表、物理ファイルヘッダーに使用している
//		バージョンページの総サイズを求める
//
//		ちなみに、バージョンページの全てを物理ページが使えるわけではないので、
//		実際は、管理ページ等を含めた使用中の物理ページの総サイズを求める。
//
//	ARGUMENTS
//		Trans::Transaction&		trans
//			サイズを求めるトランザクションのトランザクション記述子
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS

// virtual
FileSize
File::getUsedSize(const Trans::Transaction& trans)
{
	AutoUnfix cUnfix(this);
	
	// 物理ファイルヘッダーをフィックスし、
	// 記録されている使用中と未使用のそれぞれのページ数を得る

	PagePointer header =
		fixVersionPage(trans,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	PageNum used = 0;
	PageNum unuse = 0;
	FileHeader::fetchOutPageNum(
		static_cast<const VersionPage&>(*header).operator const void*(),
		used, unuse);

	// 使用中と使用済のページを管理するために必要な物理ページ表の数を求める

	const PageNum tableCount = getManageTableNum(used + unuse - 1);

	cUnfix.success();

	// 使用中のページと物理ページ表と
	// ファイルヘッダーに使用しているサイズを計算する

	return static_cast<FileSize>(used + tableCount + 1) * m_VersionPageDataSize;
}

#ifdef OBSOLETE
//	FUNCTION public
//	PhysicalFile::File::getTotalSize --
//		物理ファイルが確保中の総バージョンページサイズを得る
//
//	NOTES
//		物理ファイルが管理する物理ページ、
//		物理ページ表、物理ファイルヘッダーに使用している
//		バージョンページの総サイズを求める
//
//	ARGUMENTS
//		Trans::Transaction&		trans
//			サイズを求めるトランザクションのトランザクション記述子
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS

FileSize
File::getTotalSize(const Trans::Transaction& trans)
{
	// 物理ファイルヘッダーをフィックスし、
	// 記録されている使用中と使用済のそれぞれのページ数を得る

	const Version::Page::Memory& header =
		File::fixVersionPage(trans, this,
							 FileHeader::VersionPageID,
							 Buffer::Page::FixMode::ReadOnly,
							 Buffer::ReplacementPriority::Middle);

	PageNum used = 0;
	PageNum unuse = 0;
	FileHeader::fetchOutPageNum(
		static_cast<const VersionPage&>(*header).operator const void*(),
		used, unuse);

	// 使用中と使用済のページを管理するために必要な物理ページ表の数を求める

	const PageNum total = used + unuse;
	const PageNum tableCount = getManageTableNum(total - 1);

	// 使用中と使用済のページと物理ページ表と
	// ファイルヘッダーに使用しているサイズを計算する

	return static_cast<FileSize>(total + tableCount + 1) * m_VersionPageDataSize;
}
#endif

//
//	FUNCTION public
//	PhysicalFile::File::allocatePage --
//		物理ページを確保する
//
//	NOTES
//	物理ページを確保する。
//	引数PageID_を指定した場合には、
//	その識別子が示す物理ページを確保し、
//	省略した場合には、もっとも物理ファイル先頭に近い
//	未使用の物理ページを確保する。
//
//	※ detachPageAll() を呼ぶこと。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		[IN]		トランザクション記述子への参照
//	PhysicalFile::PageID		PageID_
//		[IN]		確保する物理ページの識別子
//
//	RETURN
//	PhysicalFile::PageID
//		確保した物理ページの識別子
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ページを確保できなかった
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageID
File::allocatePage(const Trans::Transaction&	Transaction_,
				   PageID						PageID_)
{
	Page* pPage = allocatePage2(Transaction_,
								File::DiscardableAllocateFixMode,
								PageID_);
	PageID id = pPage->getID();
	detachPage(pPage, Page::UnfixMode::Dirty);
	return id;
}

//
//	FUNCTION public
//	PhysicalFile::File::allocatePage2 --
//		物理ページを確保する
//
//	NOTES
//	物理ページを確保する。
//	引数PageID_を指定した場合には、
//	その識別子が示す物理ページを確保し、
//	省略した場合には、もっとも物理ファイル先頭に近い
//	未使用の物理ページを確保する。
//
//	※ detachPageAll() を呼ぶこと。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	Buffer::Page::FixMode::Value		eFixMode_
//		FIXモード(ReadOnlyは指定不可)
//	const PhysicalFile::PageID			PageID_ = UndefinedPageID
//		確保する物理ページの識別子
//	Buffer::ReplacementPriority::Value	ePriority =
//									Buffer::ReplacementPriority::Low
//		プライオリティー
//
//	RETURN
//	PhysicalFile::PageID
//		確保した物理ページの識別子
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ページを確保できなかった
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
Page*
File::allocatePage2(
	const Trans::Transaction&			Transaction_,
	Buffer::Page::FixMode::Value		eFixMode_,
	PageID								PageID_/* = UndifnedPageID */,
	Buffer::ReplacementPriority::Value	ePriority_/* = Low */)
{
	switch (this->m_Type) {
	case AreaManageType:
	case PageManageType:
	case PageManageType2:
		// thru
		break;
	default:

		// 管理機能なし物理ファイルまたは物理エリア管理機能付き物理ファイル…

		// 管理機能なし物理ファイル NonManageFile では、
		// 物理ファイル生成時に物理ページが確保されるので、
		// 明記的に物理ページを確保する必要がない。

		// 物理エリア管理機能付き物理ファイル DirectAreaFile では、
		// 物理ページという概念を利用者側に見せないため、
		// 物理ページを確保する必要がない。

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	PageID	pageID = PageID_;
	
	bool	discardable =
		((eFixMode_ & Buffer::Page::FixMode::Discardable) != 0);
	Buffer::Page::FixMode::Value	allocateFixMode =
		(discardable ?
			File::DiscardableAllocateFixMode :
			Buffer::Page::FixMode::Allocate);
	Buffer::Page::FixMode::Value	writeFixMode =
		(discardable ?
			File::DiscardableWriteFixMode :
			Buffer::Page::FixMode::Write);

	// 物理ファイルヘッダのバッファリング内容を得る（フィックスする）
	PagePointer	fileHeaderPage
		= fixVersionPage(Transaction_,
						 FileHeader::VersionPageID,
						 writeFixMode,
						 Buffer::ReplacementPriority::Middle);

	// 物理ファイルヘッダのバッファリング内容へのポインタを得る
	void*	fileHeaderTop = (void*)(*fileHeaderPage);

	// 物理ファイルが管理している物理ページ数を求める
	FileHeader::Item_Type1*	fileHeader =
		static_cast<FileHeader::Item_Type1*>(fileHeaderTop);
	PageNum	totalPageNum =
		fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum;

	; _SYDNEY_ASSERT(totalPageNum > 0);

	// 物理ファイルが管理している最終物理ページの識別子を求める
	PageID	lastPageID = totalPageNum - 1;
	if (PageID_ == ConstValue::UndefinedPageID) {

		// 確保する物理ページの識別子が指定されていない

		// 物理ページを割り当てる
		pageID = this->assignPage(Transaction_,
								  fileHeaderTop,
								  totalPageNum,
								  fileHeader->m_UnusePageNum);
	}

	Page* pPage = 0;
	bool bReuse = false;

	if (pageID > lastPageID) {

		// 物理ファイルが管理していない
		// 物理ページの識別子が指定された…

		// 物理ページ追加の準備

		if (pageID == lastPageID + 1) {
			// 直後か
			prepareAppendPage(Transaction_,
							  fileHeaderTop,
							  lastPageID,
							  allocateFixMode);
		} else {
			// 直後より後ろか
			prepareAppendPage(Transaction_,
							  fileHeaderTop,
							  lastPageID,
							  pageID,
							  allocateFixMode,
							  writeFixMode);
		}

	} else {

		// 物理ファイルが管理している
		// 物理ページの識別子が指定された…

		bReuse = true;
	}

	// 以下の管理表をフィックス
	//		AreaManageFile  … 空き領域管理表
	//		PageManageFile  … 物理ページ表
	//		PageManageFile2 … リーフ
	PagePointer table
		= fixVersionPage(Transaction_,
						 getManageTableVersionPageID(pageID),
						 writeFixMode,
						 Buffer::ReplacementPriority::Middle);

	// 空き領域管理表／物理ページ表の
	// バッファリング内容へのポインタを得る
	void*	tablePointer = (void*)(*table);

	// 確保モードで物理ページを得る
	// ※ 物理ページをまっさらな状態にするために確保モードを指定する
	pPage = attachPage(Transaction_,
					   pageID,
					   allocateFixMode,
					   ePriority_);

	try {

		// 確保した物理ページを初期化する
		initializePage(pPage->m_VersionPageTop);

		switch (this->m_Type) {
		case AreaManageType:

			// 空き領域管理機能付き物理ファイル…

			// 領域率ビットマップを更新する
			updateAreaBitmap(
				tablePointer,
				pageID,
				BitmapTable::ToBitmapValue
				[BitmapTable::Rate80_]
				[BitmapTable::Rate80_]);

			// 未使用領域率別の物理ページ数配列の要素を
			// 更新する
			updatePageArray(tablePointer,
							true,  // 未使用
							100,
							true); // インクリメント

			// 空き領域率別の物理ページ数配列の要素を
			// 更新する
			updatePageArray(tablePointer,
							false, // 空き
							100,
							true); // インクリメント
			break;

		case PageManageType2:

			this->updateTree(Transaction_,
							 fileHeaderTop,
							 tablePointer,		// リーフ
							 pageID,
							 true,				// 物理ページを使用状態とする
							 bReuse == false,	// append mode / update mode
							 discardable);
			break;

		case PageManageType:

			// 物理ページ管理機能付き物理ファイル…

			// 物理ページ使用状態ビットマップの
			// ビットをONにする
			updatePageBitmap(tablePointer,
							 pageID,
							 true); // ビットON
			break;

		default:
			;
		}

		if (m_Type != PageManageType2) {

			// 空き領域管理表ヘッダ／物理ページ表ヘッダの
			// 「使用中の物理ページ数」をインクリメントする
			this->updateUsedPageNum(tablePointer, 1);

			if (bReuse) {

				// 空き領域管理表ヘッダ／物理ページ表ヘッダの
				// 「未使用の物理ページ数」をデクリメントする
				this->updateUnusePageNum(tablePointer, -1);
			}
		}

		// 物理ファイルヘッダの「使用中の物理ページ数」を
		// インクリメントする
		(fileHeader->m_UsedPageNum)++;

		if (bReuse) {

			// 物理ファイルヘッダの「未使用の物理ページ数」を
			// デクリメントする
			(fileHeader->m_UnusePageNum)--;
		}

#ifdef NO_CATCH_ALL
	} catch (Exception::Object&) {
#else
	} catch (...) {
#endif
		//detachPage(pPage, Page::UnfixMode::Dirty);
		_SYDNEY_RETHROW;
	}

	return pPage;
}

//
//	FUNCTION public
//	PhysicalFile::File::freePage2 --
//		物理ページを解放する
//
//	NOTES
//	物理ページを解放する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::Page*& pPage_
//		解放する物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ページを解放できなかった
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
File::freePage2(const Trans::Transaction&	Transaction_,
				Page*&						pPage_)
{
	switch (this->m_Type) {
	case AreaManageType:
	case PageManageType:
	case PageManageType2:
		// thru
		break;
	default:

		// 管理機能なし物理ファイル NonManageFile では、
		// 物理ページを解放しない（できない）。

		// 物理エリア管理機能付き物理ファイル DirectAreaFile では、
		// 物理ページという概念を利用者側に見せないため、
		// 物理ページを解放することはない。

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	; _SYDNEY_ASSERT(pPage_ != 0);

	PageID id = pPage_->getID();
	
	pPage_->dirty();
	detachPage(pPage_);

	// 古いfreePageを呼ぶ
	freePage(Transaction_, id);
}

//
//	FUNCTION public
//	PhysicalFile::File::freePage --
//		物理ページを解放する
//
//	NOTES
//	物理ページを解放する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID	PageID_
//		解放する物理ページの識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ページを解放できなかった
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
File::freePage(const Trans::Transaction&	Transaction_,
			   const PageID					PageID_)
{
	switch (this->m_Type) {
	case AreaManageType:
	case PageManageType:
	case PageManageType2:
		// thru
		break;
	default:

		// 管理機能なし物理ファイル NonManageFile では、
		// 物理ページを解放しない（できない）。

		// 物理エリア管理機能付き物理ファイル DirectAreaFile では、
		// 物理ページという概念を利用者側に見せないため、
		// 物理ページを解放することはない。

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	; _SYDNEY_ASSERT(PageID_ != ConstValue::UndefinedPageID);

	// 物理ファイルヘッダのバッファリング内容を
	// 得る（フィックスする）

	//
	// 何らかのエラー発生時に、
	// フィックスした版を破棄できるように
	// Discardableモードも指定する。
	//

	PagePointer fileHeaderPage =
		fixVersionPage(Transaction_,
					   FileHeader::VersionPageID,
					   File::DiscardableWriteFixMode,
					   Buffer::ReplacementPriority::Middle);

	// 物理ファイルヘッダのバッファリング内容へのポインタを得る
	FileHeader::Item_Type1*	fileHeader =
		static_cast<FileHeader::Item_Type1*>((void*)(*fileHeaderPage));

	// 物理ファイルが管理している物理ページ数を求める
	PageNum	totalPageNum =
		fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum;

	; _SYDNEY_ASSERT(totalPageNum > 0);

	// 物理ファイルが管理している最終物理ページの識別子を求める
	PageID	lastPageID = totalPageNum - 1;

	; _SYDNEY_ASSERT(PageID_ <= lastPageID);

	// 以下の管理表をフィックス
	//		AreaManageFile  … 空き領域管理表
	//		PageManageFile  … 物理ページ表
	//		PageManageFile2 … リーフ

	//
	// 何らかのエラー発生時に、
	// フィックスした版を破棄できるように
	// Discardableモードも指定する。
	//

	PagePointer table
		= fixVersionPage(Transaction_,
						 getManageTableVersionPageID(PageID_),
						 File::DiscardableWriteFixMode,
						 Buffer::ReplacementPriority::Middle);

	// 空き領域管理表／物理ページ表の
	// バッファリング内容へのポインタを得る
	void*	tablePointer = (void*)(*table);

	switch (this->m_Type) {
	case AreaManageType:
		{
		// 空き領域管理機能付き物理ファイル…

		// 物理ページのバッファリング内容を得る
		//（フィックスする）

		const Version::Page::Memory&	page =
			Version::Page::fix(
				Transaction_,
				*m_VersionFile,
				this->convertToVersionPageID(PageID_),
				Buffer::Page::FixMode::ReadOnly,
				Buffer::ReplacementPriority::Low);

		// 空き領域管理表を更新する
		this->updateManageTable(tablePointer,
								PageID_,
								totalPageNum,
								false, // for free
								(const void*)page);
		break;
		}
	case PageManageType:

		// 物理ページ管理機能付き物理ファイル…

		// 物理ページ表を更新する
		this->updateManageTable(tablePointer,
								PageID_,
								totalPageNum,
								false); // for free
		break;

	case PageManageType2:

		// 物理ページ管理機能付き物理ファイル PageManageFile2

		// 木構造を更新する
		this->updateTree(Transaction_,
						 fileHeader,
						 tablePointer,
						 PageID_,
						 false,	// unuse
						 false,	// update mode
						 true);	// discardable
		break;

	default:
		;
	}

	// 物理ファイルヘッダの「使用中の物理ページ数」を
	// デクリメントする
	(fileHeader->m_UsedPageNum)--;

	// 物理ファイルヘッダの「未使用の物理ページ数」を
	// インクリメントする
	(fileHeader->m_UnusePageNum)++;
}

//
//	FUNCTION public
//	PhysicalFile::File::reusePage --
//		物理ページを再利用する
//
//	NOTES
//	PhysicalFile::File::freePageにより
//	解放した物理ページを再利用する。
//	未使用状態となっている物理ページを
//	使用中の物理ページに変更するだけであり、
//	物理ページ内の初期化は行わない。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID	PageID_
//		再利用する物理ページの識別子
//
//	RETURN
//	PhysicalFile::PageID
//		再利用した物理ページの識別子
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ページを再利用できなかった
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageID
File::reusePage(const Trans::Transaction&	Transaction_,
				const PageID				PageID_)
{
	switch (this->m_Type) {
	case AreaManageType:
	case PageManageType:
	case PageManageType2:
		// thru
		break;
	default:

		// 管理機能なし物理ファイルまたは物理エリア管理機能付き物理ファイル…

		// 管理機能なし物理ファイル NonManageFile では、
		// 物理ページを再利用しない（できない）。

		// 物理エリア管理機能付き物理ファイル DirectAreaFile では、
		// 物理ページという概念を利用者側に見せないため、
		// 物理ページを再利用することはない。

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	; _SYDNEY_ASSERT(PageID_ != ConstValue::UndefinedPageID);

	PagePointer fileHeaderPage
		 = fixVersionPage(Transaction_,
						  FileHeader::VersionPageID,
						  File::DiscardableWriteFixMode,
						  Buffer::ReplacementPriority::Middle);

	// 物理ファイルヘッダのバッファリング内容へのポインタを得る
	FileHeader::Item_Type1*	fileHeader =
		static_cast<FileHeader::Item_Type1*>((void*)(*fileHeaderPage));

	// 物理ファイルが管理している物理ページ数を求める
	PageNum	totalPageNum =
		fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum;

	; _SYDNEY_ASSERT(totalPageNum > 0);

	// 物理ファイルが管理している最終物理ページの識別子を求める
	PageID		lastPageID = totalPageNum - 1;

	; _SYDNEY_ASSERT(PageID_ <= lastPageID);

	// 以下の管理表をフィックス
	//		AreaManageFile  … 空き領域管理表
	//		PageManageFile  … 物理ページ表
	//		PageManageFile2 … リーフ

	//
	// 何らかのエラー発生時に、
	// フィックスした版を破棄できるように
	// Discardableモードも指定する。
	//

	PagePointer table
		 = fixVersionPage(Transaction_,
						  this->getManageTableVersionPageID(PageID_),
						  File::DiscardableWriteFixMode,
						  Buffer::ReplacementPriority::Middle);

	// 空き領域管理表／物理ページ表の
	// バッファリング内容へのポインタを得る
	void*	tablePointer = (void*)(*table);

	switch (this->m_Type) {
	case AreaManageType:
		{
		// 空き領域管理機能付き物理ファイル…

		// 物理ページのバッファリング内容を
		// 得る（フィックスする）
		const Version::Page::Memory&	page =
			Version::Page::fix(
				Transaction_,
				*m_VersionFile,
				this->convertToVersionPageID(PageID_),
				Buffer::Page::FixMode::ReadOnly,
				Buffer::ReplacementPriority::Middle);

		// 物理ページのバッファリング内容へのポインタを得る
		const void*	pagePointer = (const void*)page;

		// 空き領域管理表を更新する
		this->updateManageTable(tablePointer,
								PageID_,
								totalPageNum,
								true,
								pagePointer);
		break;
		}
	case PageManageType:

		// 物理ページ管理機能付き物理ファイル…

		// 物理ページ表を更新する
		this->updateManageTable(tablePointer,
								PageID_,
								totalPageNum,
								true);
		break;

	case PageManageType2:

		// 物理ページ管理機能付き物理ファイル PageManageFile2

		// 木構造を更新する
		this->updateTree(Transaction_,
						 fileHeader,
						 tablePointer,
						 PageID_,
						 true,	// used
						 false,	// update mode
						 true);	// discardable

		break;

	default:
		;
	}

	// 物理ファイルヘッダの「使用中の物理ページ数」を
	// インクリメントする
	(fileHeader->m_UsedPageNum)++;

	// 物理ファイルヘッダの「未使用の物理ページ数」を
	// デクリメントする
	(fileHeader->m_UnusePageNum)--;

	return PageID_;
}

//
//	FUNCTION public
//	PhysicalFile::File::clear --
//		物理ファイルを空の状態にする
//
//	NOTES
//	物理ファイルを生成直後（空）の状態に戻す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const bool					Force_ = false
//		一旦物理ファイルを削除した後に生成するかどうか
//			true  : 一旦物理ファイルを削除した後に生成する
//			false : 物理ファイルは削除せず物理ファイルヘッダや管理表を
//			        生成直後の状態に戻す
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ファイルを空の状態にできなかった
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある

void
File::clear(const Trans::Transaction&	Transaction_,
			const bool					Force_/* = false */)
{
	// 管理機能なし物理ファイルの場合、空にする必要がない。
	if (this->m_Type == NonManageType) return;

	if (Force_)
	{
		//
		// 一旦物理ファイルを削除した後に生成する。
		// もう、元には戻せない！
		//

		try
		{
			this->destroy(Transaction_);
			this->create(Transaction_);
			return;
		}
		catch (Exception::Object&)
		{
			_SYDNEY_RETHROW;
		}
#ifndef NO_CATCH_ALL
		catch (...)
		{
			throw Exception::FileManipulateError(moduleName,
												 srcFile,
												 __LINE__);
		}
#endif
	}

	// 物理エリア管理機能付き物理ファイルは、
	// 作り直しのみサポート
	if (this->m_Type == DirectAreaType) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	//
	// 以下が、物理ファイルは削除せずに
	// 物理ファイルを生成直後の状態に戻す処理。
	// 利用者側が以前の物理ページの使用状況を把握していると
	// いう前提だが、こちらでは、
	// reusePageが可能なはず。
	//

	// × ファイルヘッダも初期化するので reusePage できない。

	// 物理ファイルが管理している物理ページ数
	PageNum	totalPageNum = 0;

	//
	// 物理ファイルヘッダを再初期化する。
	//

	try
	{
		// 物理ファイルヘッダのバッファリング内容を
		// 得る（フィックスする）

		//
		// ここでは物理ファイルヘッダをアンフィックスするまでの間、
		// エラーは発生しないので、フィックスモードに
		// Discardableモードは指定しない。
		//

		Version::Page::Memory	fileHeaderPage
			= Version::Page::fix(Transaction_,
								 *m_VersionFile,
								 FileHeader::VersionPageID,
								 Buffer::Page::FixMode::Write,
								 Buffer::ReplacementPriority::Middle);

		// 物理ファイルヘッダのバッファリング内容へのポインタを得る
		FileHeader::Item_Type1*	fileHeader =
			static_cast<FileHeader::Item_Type1*>((void*)fileHeaderPage);

		totalPageNum = fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum;

		// 物理ファイルヘッダを再初期化する
		FileHeader::initialize(fileHeader, this->m_Type);

		// 物理ファイル生成直後の状態にするために、
		// 物理ファイルヘッダを再初期化後、
		// “未使用の物理ページ数”をインクリメントする。
		(fileHeader->m_UnusePageNum)++;

		if (this->m_Type == PageManageType2) {

			// ノードのビットマップ内の先頭ビットを ON
			this->clearNode(fileHeader);
			this->updateNode(Transaction_, fileHeader, 0, true, false);
		}

		// 物理ファイルヘッダをアンフィックスする
		fileHeaderPage.unfix(true);
	}
	catch (Exception::Object&)
	{
		_SYDNEY_RETHROW;
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		throw Exception::FileManipulateError(moduleName,
											 srcFile,
											 __LINE__);
	}
#endif

	//
	// 物理ページ表／空き領域管理表を再初期化する。
	//

	try
	{
		; _SYDNEY_ASSERT(totalPageNum > 0);

		if (this->m_Type == PageManageType2) {

			// ノード（物理ファイルヘッダと同居のものは除く）とリーフをクリア
			this->clearTree(Transaction_, totalPageNum);

		} else {

			// 再初期化する物理ページ表／空き領域管理表の
			// バージョンページ識別子
			Version::Page::ID	tableVersionPageID = 1;

			// 物理ファイル内に存在する最終物理ページ表／空き領域管理表の
			// バージョンページ識別子を取得する
			Version::Page::ID	lastTableVersionPageID =
				this->getManageTableVersionPageID(totalPageNum - 1);

			// 前後の物理ページ表／空き領域管理表の間隔（ページ数）を求める
			PageNum	skipNum = this->getPagePerManageTable() + 1;

			// 物理ファイル内に存在する物理ページ表／空き領域管理表を
			// 再初期化する。
			while (tableVersionPageID <= lastTableVersionPageID)
			{
				// 確保モードで物理ページ表／空き領域管理表の
				// バッファリング内容を得る（フィックスする）
				// 確保モードなので、0クリアされる。
				// ということは、基本的にこれで再初期化を
				// していることになる。

				//
				// ここでは物理ページ表／空き領域管理表を
				// アンフィックスするまでの間、エラーは発生しないので、
				// フィックスモードに
				// Discardableモードは指定しない。
				//

				Version::Page::Memory	pageTable
					= Version::Page::fix(Transaction_,
										 *m_VersionFile,
										 tableVersionPageID,
										 Buffer::Page::FixMode::Allocate,
										 Buffer::ReplacementPriority::Low);

				if (tableVersionPageID == 1)
				{
					// 先頭物理ページ表／空き領域管理表…

					// 確保モードで物理ページ表／空き領域管理表を
					// フィックスすることにより、再初期化されているが、
					// 先頭物理ページ表／空き領域管理表だけは、
					// “未使用の物理ページ数”を
					// インクリメントする必要がある。
					// これで物理ファイル生成直後の状態となる。
					this->updateUnusePageNum((void*)pageTable, 1);
				}

				// 物理ページ表／空き領域管理表をアンフィックスする
				pageTable.unfix(true);

				tableVersionPageID += skipNum;
			}
		}
	}
	catch (Exception::Object&)
	{
		// 物理ファイルヘッダは再初期化できたのに、
		// 物理ページ表／空き領域管理表が再初期化できなかった…

		this->setAvailability(false);

		_SYDNEY_RETHROW;
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		// 物理ファイルヘッダは再初期化できたのに、
		// 物理ページ表／空き領域管理表が再初期化できなかった…

		this->setAvailability(false);

		throw Exception::FileManipulateError(moduleName,
											 srcFile,
											 __LINE__);
	}
#endif
}

//
//	FUNCTION public
//	PhysicalFile::File::attachPage -- 物理ページ記述子を生成する
//
//	NOTES
//	※ 管理機能なし物理ファイルのみ利用可能。
//	　 （管理機能なし物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
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
// virtual
Page*
File::attachPage(
	const Trans::Transaction&					Transaction_,
	const Buffer::Page::FixMode::Value			FixMode_,
	const Buffer::ReplacementPriority::Value	ReplacementPriority_/* = Low */)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::attachPage -- 物理ページ記述子を生成する
//
//	NOTES
//	物理ページ記述子を生成し、返す。
//
//	ARGUMENTS
//	const Trans::Transaction&					Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID					PageID_
//		記述子を生成する物理ページの識別子
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
// virtual
Page*
File::attachPage(
	const Trans::Transaction&					Transaction_,
	const PageID								PageID_,
	const Buffer::Page::FixMode::Value			FixMode_,
	const Buffer::ReplacementPriority::Value	ReplacementPriority_/* = Low */)
{
	if (this->m_Check)
	{
		//
		// 整合性検査中は、attachPageによる物理ページアタッチを
		// 禁止する。物理ページをアタッチしたいのであれば、
		// verifyPageを呼び出すこと。
		//

		throw Exception::IllegalFileAccess(moduleName,
										   srcFile,
										   __LINE__);
	}

	Page*	page = 0;

	// 物理ページ記述子のリンクを保護するためにラッチする

//	Os::AutoCriticalSection	latch(_latch);

	if (this->m_Page != 0)
	{
		// 既にアタッチ中の物理ページが存在する…

		page = this->getAttachedPage(Transaction_,
									 PageID_,
									 FixMode_,
									 &ReplacementPriority_,
									 0);

		if (page == 0)
		{
			// 既にアタッチ中の物理ページには、
			// 識別子が等しいものはなかった…

			// 物理ページ記述子を生成する
			page = this->attachPage(Transaction_,
									this,
									PageID_,
									FixMode_,
									ReplacementPriority_);

			// アタッチ中の物理ページのリンクを張り替える

			//
			// ※ 上で this->m_Page が 0 かどうかをチェックしているが、
			// 　 this->attachPage()で下位モジュールから
			// 　 MemoryExhaustがthrowされた場合、
			// 　 リトライのためにthis->m_Pageにキャッシュされている
			// 　 物理ページのうち、不要なものに関しては
			// 　 デタッチされてしまい、結果として、
			// 　 this->m_Page が 0 になってしまうこともある。
			// 　 したがって、再度 this->m_Page が 0 かどうかの
			// 　 チェックが必要となる。
			//

			if (this->m_Page != 0) {
				page->m_Next = this->m_Page->m_Next;
				page->m_Prev = this->m_Page;

				this->m_Page->m_Next->m_Prev = page;
				this->m_Page->m_Next = page;
			} else {
				page->m_Next = page;
				page->m_Prev = page;
			}
		}
	}
	else
	{
		// アタッチ中の物理ページが存在しない…

		// 物理ページ記述子を生成する
		page = this->attachPage(Transaction_,
								this,
								PageID_,
								FixMode_,
								ReplacementPriority_);

		// アタッチ中の物理ページのリンクを初期化する

		page->m_Next = page;
		page->m_Prev = page;
	}

	// 物理ページ記述子の参照カウンタを更新する
	page->m_ReferenceCounter++;

	// 生成した物理ページ記述子を保持する
	this->m_Page = page;

	return page;
}

//
//	FUNCTION public
//	PhysicalFile::File::detachPage --
//		物理ページ記述子を破棄する
//
//	NOTES
//	物理ページ記述子を破棄する。
//
//	ARGUMENTS
//	PhysicalFile::Page*&					Page_
//		物理ページ記述子への参照
//	PhysicalFile::Page::UnfixMode::Value	UnfixMode_ = Omit
//		アンフィックスモード
//	const bool								SavePage_ = false
//		利用者からは破棄したように見えても、実はキャッシュしておくか
//			true  : キャッシュしておく
//			false : 本当に破棄してしまう
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
File::detachPage(Page*&					Page_,
				 Page::UnfixMode::Value	UnfixMode_/* = Omit */,
				 const bool				SavePage_/* = false */)
{
	; _SYDNEY_ASSERT(Page_ != 0);

	// 物理ページ記述子のリンクを保護するためにラッチする

//	Os::AutoCriticalSection	latch(_latch);

	// 物理ページ記述子の参照カウンタを更新する
	if (Page_->m_ReferenceCounter > 0)
	{
		Page_->m_ReferenceCounter--;
	}

	if (Page_->m_ReferenceCounter == 0)
	{
		// 物理ページ記述子がどこからも参照されなくなった…

		if (SavePage_ == false)
		{
			// 本当にデタッチしても構わない…

			if (UnfixMode_ != Page::UnfixMode::Omit)
			{
				// 明示的にアンフィックスモードが指定された…

				// 物理ページのバッファリング内容をアンフィックスする
				Page_->m_Memory.unfix(
					UnfixMode_ == Page::UnfixMode::Dirty);

			} else {

				// 物理ページのバッファリング内容をアンフィックスする
				Page_->m_Memory.unfix(
					Page_->m_UnfixMode == Page::UnfixMode::Dirty);
			}

			// アタッチ中の物理ページのリンクを張り替える

			Page_->m_Prev->m_Next = Page_->m_Next;

			Page_->m_Next->m_Prev = Page_->m_Prev;

			bool	isLastPage = false;
			if (Page_ == Page_->m_Next)
			{
				// 他にアタッチ中の物理ページが存在しない…

				; _SYDNEY_ASSERT(Page_ == Page_->m_Prev);

				isLastPage = true;
			}

			Page*	prevPage = isLastPage ? 0 : Page_->m_Prev;

			// デタッチしたページはファイルがdetachされるまで保存する
			Page_->m_Free = m_DetachPage;
			m_DetachPage = Page_;
			Page_ = 0;

			this->m_Page = prevPage;
		}
		else
		{
			if (Page_->m_UnfixMode != Page::UnfixMode::Dirty)
			{
				Page_->m_UnfixMode = UnfixMode_;
			}

			// ページ内容を更新する
			this->savePage(Page_,
						   Page_->m_UnfixMode == Page::UnfixMode::Dirty);
		}
	}
}

//
//	FUNCTION public
//	PhysicalFile::File::savePage --
//		物理ページ記述子を破棄せずにページ内容を更新する
//
//	NOTES
//	物理ページ記述子を破棄せずにページ内容を更新する。
//	版を破棄可能なフィックスモードでアタッチした物理ページでも、
//	版を破棄するように物理ページをデタッチしようとしても、
//	この関数が呼び出された場合には、その時点のページ内容にしか戻らなくなる。
//
//	ARGUMENTS
//	PhysicalFile::Page*	Page_
//		物理ページ記述子
//	const bool			Dirty_ = false
//		物理ページのページ内容を更新したかどうか
//		falseが指定された場合や省略時には、Page_のメンバm_UnfixModeを参照し、
//		PhysicalFile::Page::UnfixMode::Dirtyならば、
//		Version::Page::Memory::touch()への引数をtrueとする。
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
File::savePage(Page*		Page_,
			   const bool	Dirty_/* = false */)
{
	if (this->m_Type == DirectAreaType) {

		// 物理エリア管理機能付き物理ファイル…

		// 物理エリア管理機能付き物理ファイル DirectAreaFile では、
		// 物理ページという概念を利用者側に見せないため、
		// 物理ページ単位で更新することはない。

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	; _SYDNEY_ASSERT(Page_ != 0);

	bool	dirty = Dirty_;

	if (dirty == false) {
		dirty = (Page_->m_UnfixMode == Page::UnfixMode::Dirty);
	}

	if (dirty) {

		// ページ内容を更新する
		Page_->m_Memory.touch(dirty);

		// ページ内容を更新したのだから、
		// アンフィックスモードを初期値に戻す。
		Page_->m_UnfixMode = Page::UnfixMode::NotDirty;
	}
}

//
//	FUNCTION public
//	PhysicalFile::File::detachPageAll --
//		生成されている全物理ページ記述子を破棄する
//
//	NOTES
//	生成されている全物理ページ記述子を破棄する。
//
//	ARGUMENTS
//	なし
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
File::detachPageAll()
{
	// 物理ページ記述子のリンクを保護するためにラッチする

//	Os::AutoCriticalSection	latch(_latch);

	while (this->m_Page != 0)
	{
		this->detachPage(this->m_Page, this->m_Page->m_UnfixMode);
	}

	// キャッシュしている管理ページをunfixする
	unfixVersionPage(true);

	m_bMounted = false;
}

//
//	FUNCTION public
//	PhysicalFile::File::recoverPage --
//		空き領域管理機能付き物理ファイルの物理ページ記述子を破棄し、
//		ページ内容を元に戻す
//
//	NOTES
//	空き領域管理機能付き物理ファイルの物理ページ記述子を破棄し、
//	ページ内容をアタッチ前の状態に戻す。
//	※ FixMode::WriteとFixMode::Discardableを
//	　 マスクしたモードでアタッチした物理ページにのみ有効。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子
//	Page*&						Page_
//		破棄する物理ページの記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
File::recoverPage(const Trans::Transaction&	Transaction_,
				  Page*&					Page_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::recoverPage --
//		物理ページ管理機能付き物理ファイルの物理ページ記述子を破棄し、
//		ページ内容を元に戻す
//
//	NOTES
//	物理ページ管理機能付き物理ファイルの物理ページ記述子を破棄し、
//	ページ内容をアタッチ前の状態に戻す。
//	※ FixMode::WriteとFixMode::Discardableを
//	　 マスクしたモードでアタッチした物理ページにのみ有効。
//
//	※ 物理ページ管理機能付き物理ファイルおよび
//	　 管理機能なし物理ファイルで利用可能。
//	　 （物理ページ管理機能付き物理ファイル記述子クラスおよび
//	　 　管理機能なし物理ファイル記述子クラスで
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	Page*&	Page_
//		破棄する物理ページの記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
File::recoverPage(Page*&	Page_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::recoverPageAll --
//		生成されている空き領域管理機能付き物理ファイルの
//		全物理ページ記述子を破棄し、ページ内容を元に戻す
//
//	NOTES
//	生成されている空き領域管理機能付き物理ファイルの
//	全物理ページ記述子を破棄し、ページ内容を元に戻す。
//	※ FixMode::WriteとFixMode::Discardableを
//	　 マスクしたモードでアタッチした物理ページにのみ有効。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans:Transaction&	Transaction_
//		トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
File::recoverPageAll(const Trans::Transaction&	Transaction_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::recoverPageAll --
//		生成されている物理ページ管理機能付き物理ファイルの
//		全物理ページ記述子を破棄し、ページ内容を元に戻す
//
//	NOTES
//	生成されている物理ページ管理機能付き物理ファイルの
//	全物理ページ記述子を破棄し、ページ内容をアタッチ前の状態に戻す。
//	※ FixMode::WriteとFixMode::Discardableを
//	　 マスクしたモードでアタッチした物理ページにのみ有効。
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
void
File::recoverPageAll()
{
	// キャッシュしている管理ページをunfixする
	unfixVersionPage(false);

	m_bMounted = false;
}

//
//	FUNCTION public
//	PhysicalFile::File::getPageSearchableThreshold -- 
//		物理ページを高速検索可能な閾値を返す
//
//	NOTES
//	File::searchFreePage()により、
//	物理ページを高速検索可能な閾値（バイト数）を返す。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページを高速検索可能な閾値 [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
File::getPageSearchableThreshold() const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::searchFreePage --
//		物理ページを検索する
//
//	NOTES
//	引数Size_以上の未使用領域または空き領域をもつ
//	物理ページを検索し、
//	該当する物理ページが存在する場合には
//	その物理ページの識別子を、
//	存在しない場合には
//	PhysicalFile::ConstValue::UndefinedPageIDを返す。
//
//	検索は、引数PageID_で指定される識別子が示す
//	物理ページから物理ファイル終端に向かって行われる。
//	ただし、引数PageID_にPhysicalFile::ConstValue::UndefinedPageIDが
//	指定された場合には
//	物理ファイル先頭から物理ファイル終端に向かって
//	検索が行われる。
//
//	空き領域管理機能付き物理ファイルの場合、
//	物理ページ内の領域のうち利用者が利用可能な領域は、
//	その物理ページ内に存在する物理エリア数により変わる。
//	このため、利用者は物理ファイル記述子に対して
//		「引数AreaNum_個の物理エリアを生成するために、
//		　引数Size_分の未使用領域（または空き領域）が
//		　存在する物理ページを検索する」
//	というような指示を出す必要がある。
//
//	物理ファイル格納戦略で指定される
//	物理ページ内の使用率上限も考慮し、検索を行う。
//
//	検索高速化のため、実際には該当する物理ページが
//	物理ファイル内に存在するにもかかわらず、
//	PhysicalFile::ConstValue::UndefinedPageIDを返すこともある。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageSize	Size_
//		未使用領域サイズ／空き領域サイズ [byte]
//	const PhysicalFile::PageID		PageID_
//		検索開始物理ページの識別子
//	const bool						IsUnuseArea_
//		引数Size_が未使用領域サイズ／空き領域サイズどちらか
//			true  : 引数Size_が未使用領域サイズ
//			false : 引数Size_が空き領域サイズ
//	const PhysicalFile::AreaNum		AreaNum_ = 1
//		生成物理エリア数
//		（見つかった物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageID
//		十分な未使用領域または空き領域をもつ物理ページの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
PageID
File::searchFreePage(const Trans::Transaction&	Transaction_,
					 const PageSize				Size_,
					 const PageID				PageID_,
					 const bool					IsUnuseArea_,
					 const AreaNum				AreaNum_ /* = 1 */)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::searchFreePage2 --
//		物理ページを検索する
//
//	NOTES
//	引数Size_以上の未使用領域または空き領域をもつ
//	物理ページを検索し、
//	該当する物理ページが存在する場合には
//	その物理ページの識別子を、
//	存在しない場合には0を返す。
//
//	検索は、引数PageID_で指定される識別子が示す
//	物理ページから物理ファイル終端に向かって行われる。
//	ただし、引数PageID_にPhysicalFile::ConstValue::UndefinedPageIDが
//	指定された場合には
//	物理ファイル先頭から物理ファイル終端に向かって
//	検索が行われる。
//
//【仕様変更】
//	これだと、ファイルが大きくなった場合リニアにコストが増大してしまうので、
//	よろしくない。この不具合に対応するため、検索時に利用する管理テーブルページ
//	の上限を3とした。管理テーブルをランダムに3つ見て、空いていなかったら
//	あきらめる。
//	本来なら、管理テーブルを管理するページを作るべきだが、ファイルフォーマット
//	が変更されてしまうので、とりあえずこの修正で様子を見る。
//
//	空き領域管理機能付き物理ファイルの場合、
//	物理ページ内の領域のうち利用者が利用可能な領域は、
//	その物理ページ内に存在する物理エリア数により変わる。
//	このため、利用者は物理ファイル記述子に対して
//		「引数AreaNum_個の物理エリアを生成するために、
//		　引数Size_分の未使用領域（または空き領域）が
//		　存在する物理ページを検索する」
//	というような指示を出す必要がある。
//
//	物理ファイル格納戦略で指定される
//	物理ページ内の使用率上限も考慮し、検索を行う。
//
//	検索高速化のため、実際には該当する物理ページが
//	物理ファイル内に存在するにもかかわらず、0を返すこともある。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::PageSize	Size_
//		未使用領域サイズ／空き領域サイズ [byte]
//	Buffer::Page::FixMode::Value eFixMode
//		見つかったページをattachするときのFIXモード
//	PhysicalFile::PageID		PageID_
//		検索開始物理ページの識別子
//		PhysicalFile::ConstValue::UndefinedPageIDが指定された場合には、
//		物理ファイル先頭から検索を行う。
//	bool						IsUnuseArea_
//		true  : 引数Size_が未使用領域サイズ
//		false : 引数Size_が空き領域サイズ
//	PhysicalFile::AreaNum		AreaNum_ = 1
//		生成物理エリア数
//		（見つかった物理ページ内に
//		　いくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::Page*
//		十分な未使用領域または空き領域をもつ物理ページ
//
//	EXCEPTIONS
//	なし
//
// virtual
Page*
File::searchFreePage2(const Trans::Transaction& Transaction_,
					  PageSize	Size_,
					  Buffer::Page::FixMode::Value	eFixMode_,
					  PageID PageID_,
					  bool	IsUnuseArea_,
					  AreaNum AreaNum_/* = 1 */)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::getPageDataSize --
//		物理ページデータサイズを返す
//
//	NOTES
//	物理ページデータイサイズを返す。
//
//	ARGUMENTS
//	const PhysicalFile::Type	PhysicalFileType_
//		物理ファイルタイプ
//	const Os::Memory::Size		VersionPageSize_
//		バージョンページサイズ
//	const PhysicalFile::AreaNum	AreaNum_ = 1
//		生成物理エリア数
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページデータサイズ [byte]
//
//	EXCEPTIONS
//	Exception::BadArgument
//		不正な引数
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
// static
PageSize
File::getPageDataSize(const Type				PhysicalFileType_,
					  const Os::Memory::Size	VersionPageSize_,
					  const AreaNum				AreaNum_/* = 1 */)
{
	PageSize	pageDataSize = 0;

	switch (PhysicalFileType_) {
	case AreaManageType:
		pageDataSize = AreaManageFile::getPageDataSize(VersionPageSize_,
													   AreaNum_);
		break;
	case PageManageType:
		pageDataSize = PageManageFile::getPageDataSize(VersionPageSize_,
													   AreaNum_);
		break;
	case NonManageType:
		pageDataSize = NonManageFile::getPageDataSize(VersionPageSize_,
													  AreaNum_);
		break;
	case DirectAreaType:
		pageDataSize = DirectAreaFile::getPageDataSize(VersionPageSize_,
													   AreaNum_);
		break;
	case PageManageType2:
		pageDataSize = PageManageFile2::getPageDataSize(VersionPageSize_,
														AreaNum_);
		break;
	default:
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	return pageDataSize;
}

//
//	FUNCTION public
//	PhysicalFile::File::isUsedPage --
//		使用中の物理ページかどうかをチェックする
//
//	NOTES
//	引数PageID_が使用中の物理ページの識別子かどうかをチェックする。
//	物理ファイルヘッダからチェックが行われる。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID	PageID_
//		チェックする物理ページの識別子
//
//	RETURN
//	bool
//		使用中の物理ページかどうか
//			true  : 引数PageID_が使用中の物理ページの識別子である
//			false : 引数PageID_が未使用の物理ページの識別子である
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
bool
File::isUsedPage(const Trans::Transaction&	Transaction_,
				 const PageID				PageID_)
{
	if (this->m_Type == DirectAreaType) {

		// 物理エリア管理機能付き物理ファイル…

		// 物理エリア管理機能付き物理ファイル DirectAreaFile では、
		// 物理ページという概念を利用者側に見せないため、
		// 物理ページ単位で使用中かどうかをチェックすることはない。

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	if (this->m_Type == NonManageType) {

		// 管理機能なし物理ファイル…

		// 管理機能なし物理ファイルの場合、
		// 先頭の物理ページのみ必ず使用中である。

		return PageID_ == NonManagePage::ID;
	}

	// 物理ファイルヘッダのバッファリング内容を得る（フィックスする）
	PagePointer fileHeaderPage =
		fixVersionPage(Transaction_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	// 物理ファイルが管理している最終物理ページの識別子を求める
	const FileHeader::Item_Type1*	fileHeader =
		static_cast<const FileHeader::Item_Type1*>(
			static_cast<const VersionPage&>(
				*fileHeaderPage).operator const void*()); 
	PageID lastPageID =
		fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum - 1;

	if (PageID_ > lastPageID) return false;

	// 指定された物理ページを管理している空き領域管理表／物理ページ表の
	// バージョンページ識別子を得る
	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(PageID_);

	// 空き領域管理表／物理ページ表のバッファリング内容を得る
	// （フィックスする）
	PagePointer table =
		fixVersionPage(Transaction_,
					   this->getManageTableVersionPageID(PageID_),
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	// 空き領域管理表／物理ページ表のバッファリング内容へのポインタを得る
	const void*	tablePointer =
		static_cast<const VersionPage&>(*table).operator const void*();

	// 空き領域管理表／物理ページ表内で使用中の物理ページかどうかを
	// チェックする
	return this->isUsedPage(tablePointer, PageID_);
}

//
//	FUNCTION public
//	PhysicalFile::File::fetchOutPageNum --
//		物理ファイル内で使用中と未使用の物理ページ数を取り出す
//
//	NOTES
//	物理ファイル内で使用中の物理ページ数と未使用の物理ページ数を
//	取り出す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::PageNum&		UsedPageNum_
//		使用中の物理ページ数への参照
//	PhysicalFile::PageNum&		UnusePageNum_
//		未使用の物理ページ数への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
File::fetchOutPageNum(const Trans::Transaction&	Transaction_,
					  PageNum&					UsedPageNum_,
					  PageNum&					UnusePageNum_)
{
	if (this->m_Type == DirectAreaType) {

		// 物理エリア管理機能付き物理ファイル…

		// 物理エリア管理機能付き物理ファイル DirectAreaFile では、
		// 物理ページという概念を利用者側に見せないため、
		// 物理ページ数を取り出すことはない。

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	AutoUnfix cUnfix(this);

	if (this->m_Type == NonManageType) {

		// 管理機能なし物理ファイル…

		// 管理機能なし物理ファイルでは、
		// 使用中／未使用のページ数は固定である。

		UsedPageNum_ = 1;
		UnusePageNum_ = 0;

	} else {

		// 空き領域管理機能付き物理ファイル
		// または
		// 物理ページ管理機能付き物理ファイル…

		PagePointer fileHeaderPage =
			fixVersionPage(Transaction_,
						   FileHeader::VersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);
		const FileHeader::Item_Type1*	fileHeader =
			static_cast<const FileHeader::Item_Type1*>(
				static_cast<const VersionPage&>(
					*fileHeaderPage).operator const void*());
		UsedPageNum_ = fileHeader->m_UsedPageNum;
		UnusePageNum_ = fileHeader->m_UnusePageNum;
	}

	cUnfix.success();
}

//
//	FUNCTION public
//	PhysicalFile::File::getUsedPageNum --
//		物理ファイル内で使用中の物理ページ数を返す
//
//	NOTES
//	物理ファイル内で管理している物理ページのうち、
//	使用中の物理ページの総数を返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::PageNum
//		物理ファイル内で使用中の物理ページ数
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageNum
File::getUsedPageNum(const Trans::Transaction&	Transaction_)
{
	if (this->m_Type == DirectAreaType) {

		// 物理エリア管理機能付き物理ファイル…

		// 物理エリア管理機能付き物理ファイル DirectAreaFile では、
		// 物理ページという概念を利用者側に見せないため、
		// 物理ページ数を返すことはない。

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	AutoUnfix cUnfix(this);
	
	// 管理機能なし物理ファイルでは、
	// 使用中の物理ページ数は固定である。

	PageNum	usedPageNum = 1;

	if (this->m_Type != NonManageType) {

		// 空き領域管理機能付き物理ファイル
		// または
		// 物理ページ管理機能付き物理ファイル…

		PagePointer fileHeaderPage =
			fixVersionPage(Transaction_,
						   FileHeader::VersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);
		const FileHeader::Item_Type1*	fileHeader =
			static_cast<const FileHeader::Item_Type1*>(
				static_cast<const VersionPage&>(
					*fileHeaderPage).operator const void*());
		usedPageNum = fileHeader->m_UsedPageNum;
	}

	cUnfix.success();

	return usedPageNum;
}

//
//	FUNCTION public
//	PhysicalFile::File::getUnusePageNum --
//		物理ファイル内で未使用の物理ページ数を返す
//
//	NOTES
//	物理ファイル内で管理している物理ページのうち、
//	未使用の物理ページの総数を返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::PageNum
//		物理ファイル内で未使用の物理ページ数
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageNum
File::getUnusePageNum(const Trans::Transaction&	Transaction_)
{
	if (this->m_Type == DirectAreaType) {

		// 物理エリア管理機能付き物理ファイル…

		// 物理エリア管理機能付き物理ファイル DirectAreaFile では、
		// 物理ページという概念を利用者側に見せないため、
		// 物理ページ数を返すことはない。

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	AutoUnfix cUnfix(this);
	
	// 管理機能なし物理ファイルでは、
	// 未使用の物理ページは存在しない。

	PageNum	unusePageNum = 0;

	if (this->m_Type != NonManageType) {

		// 空き領域管理機能付き物理ファイル
		// または
		// 物理ページ管理機能付き物理ファイル

		PagePointer fileHeaderPage =
			fixVersionPage(Transaction_,
						   FileHeader::VersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);
		const FileHeader::Item_Type1*	fileHeader =
			static_cast<const FileHeader::Item_Type1*>(
				static_cast<const VersionPage&>(
					*fileHeaderPage).operator const void*());
		unusePageNum = fileHeader->m_UnusePageNum;
	}

	cUnfix.success();

	return unusePageNum;
}

//
//	FUNCTION public
//	PhysicalFile::File::getTopPageID --
//		先頭の使用中の物理ページの識別子を返す
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
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
// virtual
PageID
File::getTopPageID(const Trans::Transaction&	Transaction_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::getTopPageID --
//		最後の使用中の物理ページの識別子を返す
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
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
// virtual
PageID
File::getLastPageID(const Trans::Transaction&	Transaction_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::getNextPageID --
//		次の使用中の物理ページの識別子を返す
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	PhysicalFile::PageID
//		次の使用中の物理ページの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
PageID
File::getNextPageID(const Trans::Transaction&	Transaction_,
					const PageID				PageID_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::getPrevPageID --
//		前の使用中の物理ページの識別子を返す
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	PhysicalFile::PageID
//		前の使用中の物理ページの識別子
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
// virtual
PageID
File::getPrevPageID(const Trans::Transaction&	Transaction_,
					const PageID				PageID_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::equals --
//		参照しているバージョンファイル記述子が同じかどうかを調べる
//
//	NOTES
//	参照しているバージョンファイル記述子が同じかどうかを調べる。
//
//	ARGUMENTS
//	const PhysicalFile::File*	DstFile_
//		比較対象の物理ファイル記述子
//
//	RETURN
//	bool
//		参照しているバージョンファイル記述子が同じかどうか
//			true  : 同じ
//			false : 違う
//
//	EXCEPTIONS
//	なし
//
bool
File::equals(const File*	DstFile_) const
{
	return this->m_VersionFile == DstFile_->m_VersionFile;
}

//
// ↓↓↓↓↓ for DirectAreaFile ↓↓↓↓↓
//

//	FUNCTION public
//	PhysicalFile::File::allocateArea -- 物理エリアを確保する
//
//	NOTES
//	物理エリア先頭アドレスと確保サイズは 4 [byte] 区切りでアライメントされる。
//	The size of DirectArea is fixed.
//	When you want to resize it, you can do it using DirectArea::changeSize().
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		トランザクション記述子
//	const PhysicalFile::AreaSize			uiSize_
//		[IN]		要求サイズ [byte]
//
//	RETURN
//	PhysicalFile::DirectArea&
//		物理エリアオブジェクトへの参照
//
//	EXCEPTIONS
//	Exception::NotSupported
//		未サポート

// virtual
DirectArea
File::allocateArea(const Trans::Transaction&		cTransaction_,
				   AreaSize							uiSize_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// Free area
// virtual
void
File::freeArea(const Trans::Transaction&		cTransaction_,
			   const DirectArea::ID&			cID_,
			   Admin::Verification::Progress*	pProgress_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//	FUNCTION pubilc
//	PhysicalFile::File::attachArea -- 物理エリアをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&			Trans_
//		[IN]		トランザクション記述子
//	const PhysicalFile::DirectArea::ID&	DirectAreaID_
//		[IN]		アタッチする物理エリアの識別子
//	Buffer::Page::FixMode::Value		eFixMode_
//		[IN]		fix モード
//
//	RETURN
//	PhysicalFile::DirectArea&
//		物理エリアオブジェクトへの参照
//
//	EXCEPTIONS
//	Exception::NotSupported
//		未サポート

// virtual
DirectArea
File::attachArea(const Trans::Transaction&		cTransaction_,
				 const DirectArea::ID&			cID_,
				 Buffer::Page::FixMode::Value	eFixMode_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	PhysicalFile::File::detachAllAreas --
//		アタッチしているすべての物理エリアをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	Exception::NotSupported
//		未サポート

// virtual
void
File::detachAllAreas()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	PhysicalFile::File::recoverAllAreas --
//		アタッチしているすべての物理エリアをデタッチし、データを元に戻す
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	Exception::NotSupported
//		未サポート

// virtual
void
File::recoverAllAreas()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}


// virtual
AreaSize
File::getMaxStorableAreaSize() const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// Convert ObjectIDData to DirectArea::ID.
// static
void
File::convertToDirectAreaID(const Common::ObjectIDData&	cObjectID_,
							DirectArea::ID&				cID_)
{
	cID_.m_uiPageID = static_cast<PageID>(cObjectID_.getFormerValue());
	cID_.m_uiAreaID = static_cast<AreaID>(cObjectID_.getLatterValue());
}

// Convert DirectArea::ID to ObjectIDData.
// static
void
File::convertToObjectID(const DirectArea::ID&	cID_,
						Common::ObjectIDData&	cObjectID_)
{
	if ((cID_.m_uiAreaID & ConstValue::AreaIDUpperMask) > 0)
	{
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
	
	cObjectID_.setValue(
		static_cast<Common::ObjectIDData::FormerType>(cID_.m_uiPageID),
		static_cast<Common::ObjectIDData::LatterType>(cID_.m_uiAreaID));
}

//
// ↑↑↑↑↑ for DirectAreaFile ↑↑↑↑↑
//

//
//	FUNCTION public
//	PhysicalFile::File::mount -- 物理ファイルをマウントする
//
//	NOTES
//	物理ファイルをマウントする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
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
File::mount(const Trans::Transaction&	Transaction_)
{
	this->m_VersionFile->mount(Transaction_);
}

//
//	FUNCTION public
//	PhysicalFile::File::unmount --
//		物理ファイルをアンマウントする
//
//	NOTES
//	物理ファイルをアンマウントする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
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
File::unmount(const Trans::Transaction&	Transaction_)
{
	// キャッシュしている管理ページをunfixする
	unfixVersionPage(true);

	this->m_VersionFile->unmount(Transaction_);
	m_bMounted = false;
}

//
//	FUNCTION public
//	PhysicalFile::File::flush --
//		物理ファイルをフラッシュする
//
//	NOTES
//	物理ファイルをフラッシュする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
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
File::flush(const Trans::Transaction&	Transaction_)
{
	this->m_VersionFile->flush(Transaction_);
}

//
//	FUNCTION pubilc
//	PhysicalFile::File::startBackup --
//		物理ファイルに対してバックアップ開始を通知する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const bool					Restorable_ = true
//		版が最新版になるように変更可能とするかどうか
//			true  : バックアップされた内容をリストアしたとき、
//			        あるタイムスタンプの表す時点に開始された
//			        版管理するトランザクションの参照する版が
//			        最新版になるように変更可能にする。
//			false : バックアップされた内容をリストアしたとき、
//			        バックアップ開始時点に障害回復可能にする。
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
File::startBackup(const Trans::Transaction&	Transaction_,
				  const bool				Restorable_ // = true
				  )
{
	this->m_VersionFile->startBackup(Transaction_, Restorable_);
}

//
//	FUNCTION pubilc
//	PhysicalFile::File::endBackup --
//		物理ファイルに対してバックアップ終了を通知する
//
//	NOTES
//	物理ファイルに対してバックアップ終了を通知する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
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
File::endBackup(const Trans::Transaction&	Transaction_)
{
	this->m_VersionFile->endBackup(Transaction_);
}

//
//	FUNCTION public
//	PhysicalFile::File::recover --
//		物理ファイルを障害回復する
//
//	NOTES
//	物理ファイルを障害回復する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		バージョンファイルを戻す時点のタイムスタンプ
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
File::recover(const Trans::Transaction&	Transaction_,
			  const Trans::TimeStamp&	Point_)
{
	this->m_VersionFile->recover(Transaction_, Point_);
}

//
//	FUNCTION public
//	PhysicalFile::File::restore --
//		あるタイムスタンプの表す時点に開始された
//		版管理するトランザクションの参照する版が
//		最新版になるようにバージョンファイルを変更する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		このタイムスタンプの表す時点に開始された
//		版管理するトランザクションの参照する版が
//		最新版になるようにバージョンファイルを変更する
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
File::restore(const Trans::Transaction&	Transaction_,
			  const Trans::TimeStamp&	Point_)
{
	this->m_VersionFile->restore(Transaction_, Point_);
}

//	FUNCTION public
//	PhysicalFile::File::sync -- 物理ファイルの同期を取る
//
//	NOTES
//	物理ファイルの同期を取る。
//	==== v12.0(1.0.12.5) 〜 ===========================
//		空き領域管理機能付き物理ファイルまたは
//		物理ページ管理機能付き物理ファイルの場合、
//		Version::File::sync()を呼び出す前に、
//		物理ファイル末尾にある未使用の物理ページを
//		トランケートする。
//	===================================================
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			物理ファイルの同期を取る
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理で物理ファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理で物理ファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、物理ファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理で物理ファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理で物理ファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、物理ファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ファイルの同期を取れなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある

void
File::sync(const Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	// まず、可能な限り物理ファイルの末尾から
	// 使用されていない部分をトランケートする

//	truncate(trans, modified);

	// バージョンファイルのあるバージョンページの版のうち、
	// 同期できるものがあれば、同期する

	; _SYDNEY_ASSERT(m_VersionFile);
	m_VersionFile->sync(trans, incomplete, modified);
}

//
//	FUNCTION public
//	PhysicalFile::File::startVerification -- 整合性検査開始を指示する
//
//	NOTES
//	物理ファイルマネージャに、整合性検査開始を指示するためのメソッド。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const unsigned int				Treatment_
//		整合性検査の検査方法（“可能ならば修復するか”など）
//		const Admin::Verification::Treatment::Valueを
//		const unsigned intにキャストした値
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
File::startVerification(
	const Trans::Transaction&		Transaction_,
	const unsigned int				Treatment_,
	Admin::Verification::Progress&	Progress_)
{
	this->m_VersionFileStarted = false;

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// バージョンファイルに整合性検査の開始を通知する。
	// ★ できる限り下位モジュールから順に検査を実施するために、
	// 　 これが最初に行うべきことである。
	//

	this->m_VersionFile->startVerification(
		Transaction_,
		this->m_Treatment,
		Progress_,
		false); // 物理ファイルマネージャから、
		        // Version::File::startVerificationを呼び出す場合、
		        // overallはfalse固定。

	// バージョンファイルのstartVerificationを呼んで
	// 不整合を検出しなかったことを覚えておく。
	this->m_VersionFileStarted = true;

	if (Progress_.isGood() == false)
	{
		return;
	}

	// 管理機能なし物理ファイルでは、
	// 以下の処理が不要。

	if (this->m_Type != NonManageType) {

		//
		// 既にアタッチされている物理ページが存在すれば、
		// それらに対応するバージョンページの整合性検査を行う。
		//

		this->verifyAttachedPage(Transaction_, Progress_);

		if (Progress_.isGood() == false)
		{
			return;
		}
	}

	//
	// 物理ファイルマネージャレベルでの整合性検査のための準備をする。
	//

	this->initializeVerification(Transaction_, Treatment_, Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// 物理ファイルヘッダを参照しただけで
	// 検査可能な検査を行う。
	//

	//
	// 現状では、物理ファイルヘッダを参照しただけで
	// 検査できる項目はない。
	//
}

//
//	FUNCTION public
//	PhysicalFile::File::notifyUsePage -- 使用中の物理ページを通知する
//
//	NOTES
//	物理ファイルマネージャに、使用中の物理ページを通知するためのメソッド。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//	const PhysicalFile::PageID	PageID_
//		利用者が“使用中である”とする物理ページの識別子
//	const AreaNum				AreaNum_ = 0
//		引数AreaIDs_の要素数
//		※ 物理ページ管理機能付き物理ファイルの場合は指定不要。
//		　 空き領域管理機能付き物理ファイルの場合は必須。
//		　 （その物理ページ内で使用している物理エリアがない(=0)
//		　 　というのならば、省略しても良いが…）
//	const PhysicalFile::AreaID*	AreaIDs_ = 0
//		利用者が使用中とする物理エリアの識別子が
//		記録されている配列へのポインタ
//		※ 物理ページ管理機能付き物理ファイルの場合は指定不要。
//		　 空き領域管理機能付き物理ファイルの場合は必須。
//		　 （その物理ページ内で使用している物理エリアがない
//		　 　というのならば、省略しても良いが…）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
File::notifyUsePage(const Trans::Transaction&		Transaction_,
					Admin::Verification::Progress&	Progress_,
					const PageID					PageID_,
					const AreaNum					AreaNum_, // = 0
					const AreaID*					AreaIDs_  // = 0
					)
{
	if (this->m_Type == NonManageType || this->m_Type == DirectAreaType) {

		// 管理機能なし物理ファイル…

		// 物理ページはただ一つで、しかも利用者が明示的に確保する
		// ものではないので、通知してくれなくてよい。
		return;
	}

	if (Progress_.isGood() == false)
	{
		return;
	}

	if (PageID_ > this->m_LastManagePageID)
	{
		// 管理していない物理ページの識別子を指定された…
		
		//
		// 管理していない物理ページの識別子を指定されても、
		// notifyUsePage内で、これから先の検査は不可能。
		//

		_SYDNEY_VERIFY_INCONSISTENT(Progress_,
									this->m_FilePath,
									Message::NotManagePage(PageID_));

		return;
	}

	if (this->isAttachedPage(PageID_) == false)
	{
		// 利用者が過去に記述子を生成していないと思われる
		// 物理ページの識別子が指定された…

		// それならば、ページをフィックスしてみる。

		//
		// ※ 例外は、catchしない。
		//
		Version::Page::Memory	versionPage =
			File::fixVersionPage(Transaction_,
								 this,
								 this->convertToVersionPageID(PageID_),
								 Buffer::Page::FixMode::ReadOnly,
								 Progress_);

		versionPage.unfix(false);

		if (Progress_.isGood() == false)
		{
			// バージョンページの不整合を検出…

			//
			// バージョンページの不整合のために
			// ページをフィックスできないのであれば、
			// ビットマップを更新しても意味がないので、
			// ここで、notifyUsePageを終了。
			//

			return;
		}
	}

	//
	// 各ビットマップのビットをONにする。
	//

	this->m_PageIDs.set(PageID_, true);

	if (this->m_Type == AreaManageType)
	{
		this->setAreaIDBitmap(PageID_, AreaNum_, AreaIDs_);
	}

	this->m_NotifiedUsePage = true;
}

//
//	FUNCTION public
//	PhysicalFile::File::endVerification -- 整合性検査終了を指示する
//
//	NOTES
//	物理ファイルマネージャに、整合性検査終了を指示するためのメソッド。
//	実際の物理ファイルの整合性検査は、このメソッド内で行う。
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
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
File::endVerification(const Trans::Transaction&			Transaction_,
					  Admin::Verification::Progress&	Progress_)
{
	// 念のためキャッシュしているバージョンページをunfixする
	unfixVersionPage(true);

	if (this->m_Check == false)
	{
		// initializeVerification() has NOT been executed.
		// So, NOT need to execute anything.
		
		// But m_VersionFile->startVerification() may have been executed.
		if (m_VersionFileStarted == true)
		{
			m_VersionFile->endVerification(Transaction_, Progress_);
		}
		
		return;
	}

	if (this->existAttachPage())
	{
		// アタッチ中の物理ページが存在する…

		//
		// これは整合性検査とは関係なく、利用者側のバグなので、
		// Progress_には何も設定しない。
		//

		; _SYDNEY_ASSERT(false);

		SydErrorMessage << "Exist attached page." << ModEndl;

		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	}
	// For DirectAreaFile, NOT check whether attached areas exist.
	// Because, the areas of the file is implemented with Page.

	try
	{
		//
		// 物理ファイルヘッダと
		// 空き領域管理表／物理ページ表／ノードをフィックスしてみる。
		//

		if (Progress_.isGood())
		{
			this->verifyFileHeader(Transaction_, Progress_);
		}

		if (Progress_.isGood()) {

			if (this->m_Type == DirectAreaType) {
				verifyAllPages(Transaction_, Progress_);
			} else if (this->m_Type != NonManageType) {
				this->verifyAllTable(Transaction_, Progress_);
			}
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		//
		// statusがどうなっていようと、
		// Version::File::startVerificationを呼び出し
		// その時点では不整合がなく正常終了したのであれば、
		// Version::File::endVerificationは、呼ばなければ。
		//

		if (this->m_VersionFileStarted)
		{
			this->m_VersionFile->endVerification(Transaction_, Progress_);
		}

		//
		// 整合性検査の後処理
		//

		this->terminateVerification();

		_SYDNEY_RETHROW;
	}

	//
	// statusがどうなっていようと、
	// Version::File::startVerificationを呼び出し
	// その時点では不整合がなく正常終了したのであれば、
	// Version::File::endVerificationは、呼ばなければ。
	//

	if (this->m_VersionFileStarted)
	{
		this->m_VersionFile->endVerification(Transaction_, Progress_);
	}

	this->m_Check = false;

	// 管理機能なし物理ファイルでは、
	// 以下の処理が不要。

	if (this->m_Type != NonManageType) {

		try
		{
			//
			// まずは、利用者と自身の物理ページ使用状況が
			// 一致しているかどうかをチェックする。
			//

			if (this->m_Type != DirectAreaType && Progress_.isGood())
			{
				this->correspondUsePage(Transaction_, Progress_);
			}

			//
			// 空き領域管理機能付き物理ファイルならば、
			// 次に、利用者と自身の物理エリア使用状況が
			// 一致しているかどうかをチェックする。
			//

			if ((this->m_Type == AreaManageType) &&
				Progress_.isGood())
			{
				this->correspondUseArea(Transaction_, Progress_);
			}

			//
			// 最後に、物理ファイルの整合性検査を行う。
			//

			if (Progress_.isGood())
			{
				this->checkPhysicalFile(Transaction_, Progress_);
			}
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			//
			// 整合性検査の後処理
			//

			this->terminateVerification();

			_SYDNEY_RETHROW;
		}
	}

	//
	// 整合性検査の後処理
	//

	this->terminateVerification();
}

//
//	FUNCTION public
//	PhysicalFile::File::verifyPage --
//		物理ページ記述子を生成する（バージョンページの整合性検査付き）
//
//	NOTES
//	バージョンページの整合性検査を行い、不整合がなければ、
//	物理ページ記述子を生成し、返す。
//
//	※ 管理機能なし物理ファイルのみ利用可能。
//	　 （管理機能なし物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID			PageID_
//		記述子を生成する物理ページの識別子
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
// virtual
Page*
File::verifyPage(const Trans::Transaction&			Transaction_,
				 const Buffer::Page::FixMode::Value	FixMode_,
				 Admin::Verification::Progress&		Progress_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//	PhysicalFile::File::verifyPage --
//		物理ページ記述子を生成する（バージョンページの整合性検査付き）
//
//	NOTES
//	バージョンページの整合性検査を行い、不整合がなければ、
//	物理ページ記述子を生成し、返す。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID			PageID_
//		記述子を生成する物理ページの識別子
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
File::verifyPage(const Trans::Transaction&			Transaction_,
				 const PageID						PageID_,
				 const Buffer::Page::FixMode::Value	FixMode_,
				 Admin::Verification::Progress&		Progress_)
{
	// 物理エリア管理機能付き物理ファイル DirectAreaFile では、
	// 物理ページという概念を利用者側に見せないが、
	// 内部的には物理ページをアタッチしている。

	if (this->m_Check == false)
	{
		//
		// 整合性検査中しか、verifyPageによる物理ページアタッチは
		// できない。整合性検査中ではないときに、
		// 物理ページをアタッチしたいのであれば、
		// attachPageを呼び出すこと。
		//

		throw Exception::IllegalFileAccess(moduleName, srcFile, __LINE__);
	}

	Page*	page = 0;

	// 物理ページ記述子のリンクを保護するためにラッチする

	Os::AutoCriticalSection	latch(_latch);

	if (this->m_Page != 0)
	{
		// 既にアタッチ中の物理ページが存在する…

		page = this->getAttachedPage(Transaction_,
									 PageID_,
									 FixMode_,
									 0,
									 &Progress_);

		if (page == 0)
		{
			// 既にアタッチ中の物理ページには、
			// 識別子が等しいものはなかった…

			// 物理ページ記述子を生成する
			page = this->verifyPage(Transaction_,
									this,
									PageID_,
									FixMode_,
									Progress_);

			// アタッチ中の物理ページのリンクを張り替える

			//
			// ※ 上で this->m_Page が 0 かどうかをチェックしているが、
			// 　 this->attachPage()で下位モジュールから
			// 　 MemoryExhaustがthrowされた場合、
			// 　 リトライのためにthis->m_Pageにキャッシュされている
			// 　 物理ページのうち、不要なものに関しては
			// 　 デタッチされてしまい、結果として、
			// 　 this->m_Page が 0 になってしまうこともある。
			// 　 したがって、再度 this->m_Page が 0 かどうかの
			// 　 チェックが必要となる。
			//

			if (this->m_Page != 0) {
				page->m_Next = this->m_Page->m_Next;
				page->m_Prev = this->m_Page;

				this->m_Page->m_Next->m_Prev = page;
				this->m_Page->m_Next = page;
			} else {
				page->m_Next = page;
				page->m_Prev = page;
			}
		}
	}
	else
	{
		// アタッチ中の物理ページが存在しない…

		// 物理ページ記述子を生成する
		page = this->verifyPage(Transaction_,
								this,
								PageID_,
								FixMode_,
								Progress_);

		// アタッチ中の物理ページのリンクを初期化する

		page->m_Next = page;
		page->m_Prev = page;
	}

	// 物理ページ記述子の参照カウンタを更新する
	page->m_ReferenceCounter++;

	// 生成した物理ページ記述子を保持する
	this->m_Page = page;

	return page;
}

//	FUNCTION public
//	PhysicalFile::File::verifyArea --
//		物理エリアをアタッチする（整合性検査付き）
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&			cTransaction_
//		[IN]		トランザクション記述子
//	const PhysicalFile::DirectArea::ID&	cID_
//		[IN]		アタッチする物理エリアの識別子
//	Buffer::Page::FixMode::Value		eFixMode_
//		[IN]		フィックスモード
//	Admin::Verification::Progress&		cProgress_
//		[IN/OUT]	整合性検査の途中経過
//
//	RETURN
//	PhysicalFile::DirectArea
//		物理エリアオブジェクト
//
//	EXCEPTIONS

// virtual
DirectArea
File::verifyArea(const Trans::Transaction&		cTransaction_,
				 const DirectArea::ID&			cID_,
				 Buffer::Page::FixMode::Value	eFixMode_,
				 Admin::Verification::Progress&	cProgress_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

#ifdef DEBUG

Version::Page::ID
File::getTableID(const PageID	PageID_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

void
File::getTableHeader(const Trans::Transaction&	Transaction_,
					 const Version::Page::ID	TableVersionPageID_,
					 PageNum&					UsedPageNum_,
					 PageNum&					UnusePageNum_,
					 PageNum*					DummyPageNumByUnuseAreaRate_,
					 PageNum*					DummyPageNumByFreeAreaRate_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

void
File::getTableBitmap(const Trans::Transaction&	Transaction_,
					 const Version::Page::ID	TableVersionPageID_,
					 unsigned char*				BitmapBuffer_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

AreaSize
File::getParentIndexValue(const Trans::Transaction&	cTransaction_,
						  PageID					uiPageID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

void
File::setParentIndexValue(const Trans::Transaction&	cTransaction_,
						  PageID					uiPageID_,
						  AreaSize					uiSize_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

void
File::setLeafHeaderNumber(const Trans::Transaction&	cTransaction_,
						  PageID					uiPageID_,
						  AreaNum					uiNum_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

void
File::setLeafHeaderOffset(const Trans::Transaction&	cTransaction_,
						  PageID					uiPageID_,
						  AreaOffset				uiOffset_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

void
File::setLeafIndexAreaID(const Trans::Transaction&	cTransaction_,
						 PageID						uiPageID_,
						 AreaID						uiAreaID_,
						 AreaNum					uiIndex_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

void
File::setLeafIndexOffset(const Trans::Transaction&	cTransaction_,
						 PageID						uiPageID_,
						 AreaOffset					uiOffset_,
						 AreaNum					uiIndex_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

#endif // DEBUG

//
//	FUNCTION public
//	PhysicalFile::File::unfixVersionPage
//		-- キャッシュしているすべてのバージョンページをunfixする
//
//	NOTES
//
//	ARGUMENTS
//	bool
//		dirtyでunfixするかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::unfixVersionPage(bool dirty_)
{
	PageList::Iterator i = m_cPageList.begin();
	while (i != m_cPageList.end())
	{
		// deleteしなくてはいけないので、ポインタをとっておく
		VersionPage* p = &(*i);
		++i;

		if (p->isUpdatable()
			&& (dirty_ || p->isDiscardable() == false))
			// dirtyなので、trueでunfix
			// または、WriteなのにDiscardableじゃないとき
			p->unfix(true);
		else
			p->unfix(false);

		m_cPageList.erase(*p);

		// eraseしてからdeleteする
		delete p;
	}

	// ヘッダーページをunfixする
	if (m_pHeaderPage)
	{
		if (m_pHeaderPage->isUpdatable()
			&& (dirty_ || m_pHeaderPage->isDiscardable() == false))
			m_pHeaderPage->unfix(true);
		else
			m_pHeaderPage->unfix(false);
		
		delete m_pHeaderPage, m_pHeaderPage = 0;
	}
}

//
//	FUNCTION public
//	PhysicalFile::File::unfixNotDirtyPage
//		-- dirtyではないページを解放する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize iCacheSize_ = 0
//		残しておくページ数
//
//	RETURN
//	bool
//		解放したページがあった場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
File::unfixNotDirtyPage(ModSize iCacheSize_)
{
	bool result = false;

	if (m_cPageList.getSize() > iCacheSize_)
	{
		ModSize n = 0;
		PageList::Iterator i = m_cPageList.begin();
		while (i != m_cPageList.end())
		{
			VersionPage* p = &(*i);
			++i;
			
			if (p->isUpdatable() == true || p->_reference != 0)
			{
				// dirtyなページか、参照されているページは飛ばす
				continue;
			}

			// dirtyじゃなく、参照もされていないページ数
			n++;
		
			if (n <= iCacheSize_)
			{
				// これは、とっておく
				continue;
			}

			// dirtyじゃないのでunfixする
			p->unfix(false);

			m_cPageList.erase(*p);

			// eraseしてからdeleteする
			delete p;

			result = true;
		}
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Fileクラスのprotectedメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION protected
//	PhysicalFile::File::File --
//		コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	バージョンファイル記述子を生成する。
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
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
File::File(const File::StorageStrategy&		FileStorageStrategy_,
		   const File::BufferingStrategy&	BufferingStrategy_,
		   const Lock::FileName*			LockName_,
		   bool								batch_)
	: Common::Object(),
	  m_Type(FileStorageStrategy_.m_PhysicalFileType),
	  m_Page(0),
	  m_DetachPage(0),
	  m_VersionPageSize(0),
	  m_VersionPageDataSize(0),
	  m_UserAreaSizeMax(0),
	  m_PageIDs(),
	  m_ManagePageNum(0),
	  m_LastManagePageID(ConstValue::UndefinedPageID),
	  m_Treatment(Admin::Verification::Treatment::None),
	  m_Check(false),
	  m_NotifiedUsePage(false),
	  m_VersionFileStarted(false),
	  m_FilePath(),
	  m_BufferingCategory(BufferingStrategy_.m_VersionFileInfo._category),
	  m_bMounted(false),
	  m_cPageList(&VersionPage::_next, &VersionPage::_prev),
	  m_pHeaderPage(0)
{
	if (LockName_ != 0)
	{
		this->m_VersionFile =
			Version::File::attach(FileStorageStrategy_.m_VersionFileInfo,
								  BufferingStrategy_.m_VersionFileInfo,
								  *LockName_,
								  batch_);
	}
	else
	{
		this->m_VersionFile =
			Version::File::attach(FileStorageStrategy_.m_VersionFileInfo,
								  BufferingStrategy_.m_VersionFileInfo,
								  batch_);
	}
}

//
//	FUNCTION protected
//	PhysicalFile::File::~File --
//		デストラクタ
//
//	NOTES
//	デストラクタ。
//	バージョンファイル記述子を破棄する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
File::~File()
{
	// キャッシュしているすべてのページを解放する
	while (m_DetachPage)
	{
		Page* n = m_DetachPage;
		m_DetachPage = m_DetachPage->m_Free;
		delete n;
	}

	// キャッシュしているページを破棄する
	unfixVersionPage(true);

#ifdef DEBUG
	if (this->m_Page != 0)
	{
		// デタッチし忘れの物理ページが存在する！！！

		SydErrorMessage << "Exist not detach page!" << ModEndl;

		SydErrorMessage << "page id = ";

		Page*	page = this->m_Page;
		Page*	top = page;

		do
		{
			SydErrorMessage << page->m_ID;

			if (page == page->m_Next)
			{
				break;
			}

			SydErrorMessage << ", ";

			page = page->m_Next;
		}
		while (page != top);

		SydErrorMessage << ModEndl;
	}
#endif // DEBUG
	// バージョンファイル記述子を破棄する

	Version::File::detach(m_VersionFile, true);
	; _SYDNEY_ASSERT(!m_VersionFile);
}

//
//	FUNCTION protected
//	PhysicalFile::File::getCachedPage --
//		キャッシュされている物理ページ記述子を返す
//
//	NOTES
//	引数PageID_で示される物理ページの記述子が
//	キャッシュされている場合、その物理ページ記述子を返す。
//	キャッシュされていないのであれば、0（ヌルポインタ)を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	PhysicalFile::Page*
//		物理ページ記述子（キャッシュされていない場合には、0）
//
//	EXCEPTIONS
//	なし
//
Page*
File::getCachedPage(const PageID	PageID_) const
{
	//【注意】	_latch はラッチされている必要がある

	Page*	page = 0;

	if (this->m_Page != 0)
	{
		page = this->m_Page;

		bool	exist = false;

		do
		{
			if (page->m_ID == PageID_)
			{
				exist = true;

				break;
			}

			page = page->m_Next;
		}
		while (page != this->m_Page);

		if (exist == false)
		{
			page = 0;
		}
	}

	return page;
}

//
//	FUNCTION protected
//	PhysicalFile::File::fixVersionPage --
//		バージョンページをフィックスする（リトライ付き）
//
//	NOTES
//	バージョンページをフィックスする。
//	下位モジュールからMemoryExhaustが送出された場合、
//	2回までリトライする。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*					PhysicalFile_
//		物理ファイル記述子
//	Version::Page::ID					VersionPageID_
//		バージョンページ識別子
//	Buffer::Page::FixMode::Value		FixMode_
//		フィックスモード
//	Buffer::ReplacementPriority::Value	ReplacementPriority_
//		バッファリング内容の破棄されにくさ
//		After v15.0 this value is ignored, so default value is set.
//
//	RETURN
//	Version::Page::Memory
//		バージョンページのバッファリング内容
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
File::PagePointer
File::fixVersionPage(const Trans::Transaction&			Transaction_,
					 Version::Page::ID					VersionPageID_,
					 Buffer::Page::FixMode::Value		FixMode_,
					 Buffer::ReplacementPriority::Value	ReplacementPriority_)
{
	int	retryCnt = 0;
	PagePointer pPage = 0;

	while (retryCnt < 2)
	{
		try
		{
			pPage = getVersionPage(Transaction_,
								   VersionPageID_,
								   FixMode_,
								   ReplacementPriority_);
			break;
		}
		catch (Exception::MemoryExhaust&)
		{
			// 管理機能なし物理ファイルの場合、
			// その物理ファイルにはバージョンページは一つしかないので
			// step1はすっ飛ばす。
			if (m_Type == NonManageType && retryCnt == 0) retryCnt = 1;

			switch (retryCnt)
			{
			case 0:
				if (retryStepSelf() == false) _SYDNEY_RETHROW;
				break;
			default:
				_SYDNEY_RETHROW;
			}

			retryCnt++;
		}
	}

	return pPage;
}

//	FUNCTION protected
//	PhysicalFile::File::existAttachPage --
//		アタッチ中の物理ページが存在するかどうかを知らせる
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		true  : アタッチ中の物理ページが存在する
//		false : アタッチ中の物理ページが存在しない
//
//	EXCEPTIONS
//	なし

bool
File::existAttachPage() const
{
	// 物理ページ記述子のリンクを保護するためにラッチする

//	Os::AutoCriticalSection	latch(_latch);

	if (this->m_Page != 0)
	{
		// 物理ページ記述子がキャッシュされている…

		//
		// キャッシュされているからといって、
		// アタッチされているとも限らない。
		//

		Page*	page = this->m_Page;

		Page*	lastPage = page->m_Prev;

		bool	isLastPage = false;

		do
		{
			isLastPage = (page == lastPage);

			Page*	nextPage = page->m_Next;

			if (page->m_ReferenceCounter > 0)
			{
				// アタッチされていた…

				return true;
			}

			page = nextPage;
		}
		while (isLastPage == false);
	}

	// アタッチ中の物理ページは存在しない。

	return false;
}

//
//	FUNCTION protected
//	PhysicalFile::File::isCorrect --
//		可能であれば修復するように指定されているかどうかを知らせる
//
//	NOTES
//	可能であれば修復するように指定されているかどうかを知らせる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		可能であれば修復するように指定されているかどうか
//			true  : 指定されている
//			false : 指定されていない
//
//	EXCEPTIONS
//	なし
//
bool
File::isCorrect() const
{
	return
		(this->m_Treatment & Admin::Verification::Treatment::Correct) != 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::Fileクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION private
//	PhysicalFile::File::initialize -- 物理ファイル生成時の初期化
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	void*						FileHeader_
//		[IN]		物理ファイルヘッダ
//
//	RETURN
//
//	EXCEPTIONS

// virtual
void
File::initialize(const Trans::Transaction&	Trans_,
				 void*						FileHeader_)
{
	// 未処理

	// 現状では NonManageFile は未処理。
	// 他のタイプの物理ファイルはサブクラスで実装。
}

//
//	FUNCTION private
//	PhysicalFile::File::undoCreate -- 物理ファイル生成を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
File::undoCreate(const Trans::Transaction&	Transaction_)
{
	try
	{
		this->m_VersionFile->destroy(Transaction_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// バージョンファイルを破棄できなかった…

		this->setAvailability(false);
	}
}

//	FUNCTION private
//	PhysicalFile::File::setAvailability --
//		物理ファイルの利用可能性を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const bool	Availability_
//		[IN]		物理ファイルを利用可能とするか不可能とするか
//						true  : 利用可能とする
//						false : 利用不可能とする
//
//	RETURN
//
//	EXCEPTIONS

void
File::setAvailability(const bool	Availability_)
{
	if (Availability_ == false) {
		SydErrorMessage << "Physical file update failed. FATAL." << ModEndl;
	}
	Schema::File::setAvailability(this->m_VersionFile->getLockName(),
								  Availability_);
}

//
//	FUNCTION private
//	PhysicalFile::File::getAttachedPage --
//		既にアタッチしてある物理ページの記述子を返す
//
//	NOTES
//	引数PageID_で示される物理ページが既にアタッチされているのであれば、
//	その物理ページの記述子を返す。
//	アタッチされていなければ、0（ヌルポインタ）を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	PhysicalFile::Page*
//		物理ページ記述子
//
//	EXCEPTIONS
//	Exception::IllegalFileAccess
//		異なるフィックスモードで既にアタッチ中の物理ページの
//		記述子を生成しようとした
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
Page*
File::getAttachedPage(
	const Trans::Transaction&					Transaction_,
	const PageID								PageID_,
	const Buffer::Page::FixMode::Value			FixMode_,
	const Buffer::ReplacementPriority::Value*	ReplacementPriority_,
	Admin::Verification::Progress*				Progress_)
{
	//【注意】	_latch はラッチされている必要がある

	; _SYDNEY_ASSERT(this->m_Page != 0);
	Page*	page = this->m_Page;

	do
	{
		if (page->m_ID == PageID_)
		{
			// 既にアタッチ中の物理ページで
			// 識別子が等しい物理ページの記述子が存在した…

			if (page->getFixMode() != FixMode_)
			{
				// 識別子は等しいが、フィックスモードが異なっている…

				if (page->m_ReferenceCounter != 0)
				{
					// どこからかアタッチされている…

					// 不正なファイルアクセス
					throw Exception::IllegalFileAccess(moduleName,
													   srcFile,
													   __LINE__);
				}
		
				//
				// どこからもアタッチされていないので
				// 一度アンフィックスして、
				// 今回指定されたモードでフィックスし直す
				//

				Page*	nextPage = page->m_Next;
				Page*	prevPage = page->m_Prev;

				bool	onlyOnePage = (page == nextPage);

				if (page->m_UnfixMode != Page::UnfixMode::Omit)
				{
					page->m_Memory.unfix(
						page->m_UnfixMode == Page::UnfixMode::Dirty);
				}

				delete page;

				if (onlyOnePage)
				{
					this->m_Page = 0;
				}
				else
				{
					nextPage->m_Prev = prevPage;
					prevPage->m_Next = nextPage;

					this->m_Page = nextPage;
				}

				page =
					(ReplacementPriority_ != 0) ?
					this->attachPage(Transaction_,
									 this,
									 PageID_,
									 FixMode_,
									 *ReplacementPriority_) :
					this->verifyPage(Transaction_,
									 this,
									 PageID_,
									 FixMode_,
									 *Progress_);

				if (onlyOnePage)
				{
					page->m_Next = page;
					page->m_Prev = page;
				}
				else
				{
					page->m_Next = nextPage;
					page->m_Prev = prevPage;

					nextPage->m_Prev = page;

					prevPage->m_Next = page;
				}
			}

			return page;
		}

		page = page->m_Next;
	}
	while (page != this->m_Page);

	return 0;
}

//
//	FUNCTION private
//	PhysicalFile::File::attachPage --
//		物理ページ記述子を生成する
//
//	NOTES
//	物理ページ記述子を生成し、返す。
//	下位モジュールからMemoryExhaustが送出された場合、
//	2回までリトライする。
//
//	ARGUMENTS
//	const Trans::Transaction&					Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*							File_
//		物理ファイル記述子
//	const PhysicalFile::PageID					PageID_
//		記述子を生成する物理ページの識別子
//	const Buffer::Page::FixMode::Value			FixMode_
//		フィックスモード
//	const Buffer::ReplacementPriority::Value	ReplacementPriority_
//		バッファリング内容の破棄されにくさ
//
//	RETURN
//	PhysicalFile::Page*
//		物理ページ記述子
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
Page*
File::attachPage(
	const Trans::Transaction&					Transaction_,
	File*										File_,
	const PageID								PageID_,
	const Buffer::Page::FixMode::Value			FixMode_,
	const Buffer::ReplacementPriority::Value	ReplacementPriority_)
{
	Page*	page = 0;

	int	retryCnt = 0;

#ifdef OBSOLETE
	while (retryCnt < 3)
#else
	while (retryCnt < 2)
#endif
	{
		try
		{
			if (m_DetachPage)
			{
				// キャッシュがある
				
				page = m_DetachPage;
				m_DetachPage = m_DetachPage->m_Free;

				try
				{

					// バージョンページをfixする
				
					page->reset(Transaction_,
								PageID_,
								FixMode_,
								ReplacementPriority_);

				}
#ifdef NO_CATCH_ALL
				catch (Exception::Object&)
#else
				catch (...)
#endif
				{
					// 何か例外が発生したので戻す
					page->m_Free = m_DetachPage;
					m_DetachPage = page;

					_SYDNEY_RETHROW;
				}

				break;
			}

			// 物理ページ記述子を生成する
			page = allocatePageInstance(Transaction_, PageID_, FixMode_);
			break;
		}
		catch (Exception::MemoryExhaust&)
		{
			// 管理機能なし物理ファイルの場合、
			// その物理ファイルにはバージョンページは一つしかないので
			// step1はすっ飛ばす。
			if (this->m_Type == NonManageType && retryCnt == 0) retryCnt = 1;

			switch (retryCnt)
			{
			case 0:
				if (retryStepSelf() == false) _SYDNEY_RETHROW;
				break;
#ifdef OBSOLETE
			case 1:
				if (retryStepOthers() == false) _SYDNEY_RETHROW;
				break;
#endif
			default:
				_SYDNEY_RETHROW;
			}

			retryCnt++;
		}
	}

	return page;
}

//	FUNCTION protected
//	PhysicalFile::File::retryStepSelf --
//		物理ページアタッチのリトライのための処理を行う
//
//	NOTES
//	物理ページアタッチの初回リトライのために、
//	引数PhysicalFile_で指定された物理ファイルがキャッシュしている
//	すべての物理ページを可能なだけデタッチする。
//
//	ARGUMENTS
//		bool	others
//			true または指定されないとき
//				自分自身の管理する物理ページをひとつも破棄できなかったとき、
//				他の物理ファイル記述子について調べてみる
//			false
//				他の物理ファイル記述子は調べない
//
//	RETURN
//	bool
//		物理ページアタッチをリトライする意味があるかどうか
//			true  : 意味がある
//			false : 意味がない
//
//	EXCEPTIONS
//	なし

bool
File::retryStepSelf(bool others)
{
	bool detached = false;
	PhysicalFile::Page* page;
	{
	// 物理ページ記述子のリンクを保護するためにラッチを試みる
#ifdef OBSOLETE
	Os::AutoTryCriticalSection	latch(_latch);

	if (latch.isLocked() && (page = m_Page)) {
#else
	if (page = m_Page) {
#endif

		Page* lastPage = page->m_Prev;

		bool isLastPage;

		do {
			isLastPage = (page == lastPage);

			Page* nextPage = page->m_Next;

			if (page->m_ReferenceCounter == 0)
			{
				detachPage(page, page->m_UnfixMode);
				detached = true;
			}

			page = nextPage;

		} while (!isLastPage) ;
	}
	}
	// ひとつも物理ページ記述子を破棄できなければ、
	// 他の物理ファイル記述子で管理されているものも探してみる

#ifdef OBSOLETE
	return detached || (others && retryStepOthers()) || unfixNotDirtyPage();
#else
	return detached || unfixNotDirtyPage();
#endif
}
#ifdef OBSOLETE
//	FUNCTION private
//	PhysicalFile::File::retryStepOthers --
//		物理ページアタッチのリトライのための処理を行う
//
//	NOTES
//	物理ページアタッチの2回目のリトライのために、
//	アタッチ中のすべての物理ファイルがキャッシュしている
//	すべての物理ページを可能なだけデタッチする。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	bool
//		物理ページアタッチをリトライする意味があるかどうか
//			true  : 意味がある
//			false : 意味がない
//
//	EXCEPTIONS
//	なし

bool
File::retryStepOthers()
{
	bool detached = false;
	File* file = this;

	Os::AutoCriticalSection	latch(Manager::getLatch());

	if (file != m_Prev)
		do {
			file = file->m_Next;

			// この物理ファイル記述子で管理されている
			// 物理ページ記述子が破棄できるか試みる

			if (file->m_BufferingCategory == m_BufferingCategory &&
				file->retryStepSelf(false))
				detached = true;

		} while (file != m_Prev) ;

	return detached;
}
#endif
//
//	FUNCTION private
//	PhysicalFile::File::assignPage --
//		物理ページを割り当てる
//
//	NOTES
//	物理ファイル先頭から未使用の物理ページを検索し、
//	該当する物理ページを新たに確保する物理ページに割り当てる。
//	物理ファイル内に未使用の物理ページが存在しない場合には、
//	物理ファイルが管理している最終物理ページの直後に、
//	物理ページを新たに確保し、割り当てる。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		[IN]		トランザクション記述子への参照
//	const void*					FileHeader_
//		[IN]		物理ファイルヘッダ
//	const PhysicalFile::PageNum	TotalPageNum_
//		[IN]		物理ファイル内で管理している物理ページ数
//	const PhysicalFile::PageNum	UnusePageNum_
//		[IN]		物理ファイル内に存在する未使用の物理ページ数
//
//	RETURN
//	PhysicalFile::PageID
//		次に割り当てる物理ページの識別子
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ページを割り当てることができなかった
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageID
File::assignPage(const Trans::Transaction&	Transaction_,
				 const void*				FileHeader_,
				 const PageNum				TotalPageNum_,
				 const PageNum				UnusePageNum_)
{
	; _SYDNEY_ASSERT(TotalPageNum_ > 0);

	PageID	pageID = ConstValue::UndefinedPageID;

	// 物理ファイルが管理している最終物理ページの識別子を求める
	PageID	lastPageID = TotalPageNum_ - 1;

	if (UnusePageNum_ == 0) {
		// 物理ファイルが“未使用の物理ページ”を管理していない…

		// 物理ファイル末尾に物理ページを追加する
		pageID = lastPageID + 1;

	} else if (this->m_Type == PageManageType2) {

		// 物理ページ割り当て
		pageID = this->searchNextAssignPage(Transaction_,
											FileHeader_,
											TotalPageNum_);

	// 以下 this->m_Type == PageManageType || this->m_Type == AreaManageType
	} else if (this->getManageTableNum(lastPageID) > 3) {

		// ※ 管理表が多いときはランダムで管理表を選択して
		// 　 選ばれた管理表で未使用の物理ページを管理しているかを
		// 　 調べる。

		pageID = this->searchNextAssignPage2(Transaction_, TotalPageNum_);

		// 空いている物理ページを軽く探してみたものの見つからなかった場合には、
		// 物理ファイル末尾に物理ページを追加する
		if (pageID == ConstValue::UndefinedPageID) pageID = lastPageID + 1;

	} else {

		// 物理ページ割り当て
		pageID = this->searchNextAssignPage(Transaction_,
											TotalPageNum_);
	}

	return pageID;
}

//	FUNCTION private
//	PhysicalFile::File::prepareAppendPage -- 物理ページ追加のための準備を行う
//
//	NOTES
//	現在の物理ファイル末尾にひとつの物理ページを追加するための準備を行う。
//
//	ARGUMENTS
//	const Trans::Transaction&			Trans_
//		[IN]		トランザクション記述子への参照
//	void*								FileHeader_
//		[IN]		物理ファイルヘッダのバッファリング内容へのポインタ
//					※ この関数内で物理ファイルヘッダを更新するため、
//					　 呼び出し側では物理ファイルヘッダを更新モードで
//					　 フィックスする必要がある
//	const PhysicalFile::PageID			LastPageID_
//		[IN]		物理ファイルが管理している最終物理ページの識別子
//	const Buffer::Page::FixMode::Value	AllocateFixMode_
//		[IN]		バージョンページ確保時の fix モード
//
//	RETURN
//
//	EXCEPTIONS

// virtual
void
File::prepareAppendPage(const Trans::Transaction&			Trans_,
						void*								FileHeader_,
						const PageID						LastPageID_,
						const Buffer::Page::FixMode::Value	AllocateFixMode_)
{
	// 追加する物理ページの識別子を求める
	PageID	pageID = LastPageID_ + 1;

	// 追加する物理ページの直前が
	// 空き領域管理表／物理ページ表かもしれない！
	if (this->getManageTableVersionPageID(LastPageID_) <
		this->getManageTableVersionPageID(pageID)) {

		// 追加する物理ページの直前が
		// 空き領域管理表／物理ページ表…

		// 確保モードで空き領域管理表／物理ページ表の
		// バッファリング内容を得る（フィックスする）
		PagePointer table
			= fixVersionPage(
				Trans_,
				this->convertToVersionPageID(LastPageID_) + 1,
				AllocateFixMode_,
				Buffer::ReplacementPriority::Middle);
	}
}

//	FUNCTION private
//	PhysicalFile::File::prepareAppendPage -- 物理ページ追加のための準備を行う
//
//	NOTES
//	現在の物理ファイル末尾に複数の物理ページを追加するための準備を行う。
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子への参照
//	void*						FileHeader_
//		[IN]		物理ファイルヘッダのバッファリング内容へのポインタ
//					※ この関数内で物理ファイルヘッダを更新するため、
//					　 呼び出し側では物理ファイルヘッダを更新モードで
//					　 フィックスする必要がある
//	const PhysicalFile::PageID	LastPageID_
//		[IN]		物理ファイルが管理している最終物理ページの識別子
//	const PhysicalFile::PageID	PageID_
//		[IN]		追加する物理ページの識別子
//	const Buffer::Page::FixMode::Value	AllocateFixMode_
//		[IN]		バージョンページ確保時の fix モード
//	const Buffer::Page::FixMode::Value	WriteFixMode_
//		[IN]		バージョンページ更新時の fix モード
//
//	RETURN
//
//	EXCEPTIONS

// virtual
void
File::prepareAppendPage(const Trans::Transaction&			Trans_,
						void*								FileHeader_,
						const PageID						LastPageID_,
						const PageID						PageID_,
						const Buffer::Page::FixMode::Value	AllocateFixMode_,
						const Buffer::Page::FixMode::Value	WriteFixMode_)
{
	bool	allocated = false;

	try {

		// バージョンファイルを拡張するために、
		// 最終物理ページに続く物理ページから、
		// 追加する物理ページまでを確保モードでバッファリング内容を得る
		// （フィックスする）

		Version::Page::ID	startVersionPageID =
			this->convertToVersionPageID(LastPageID_) + 1;
		Version::Page::ID	endVersionPageID =
			this->convertToVersionPageID(PageID_);
		for (Version::Page::ID	versionPageID = startVersionPageID;
			 versionPageID < endVersionPageID;
			 versionPageID++) {

			Version::Page::Memory	versionPage
				= Version::Page::fix(Trans_,
									 *m_VersionFile,
									 versionPageID,
									 AllocateFixMode_,
									 Buffer::ReplacementPriority::Low);

			if (allocated == false) allocated = true;
		}

		// 最終物理ページを管理している空き領域管理表／物理ページ表の
		// バージョンページ識別子を得る
		Version::Page::ID	tableVersionPageID =
			this->getManageTableVersionPageID(LastPageID_);

		// 追加する物理ページを管理する空き領域管理表／物理ページ表の
		// バージョンページ識別子を得る
		Version::Page::ID	appendTableVersionPageID =
			this->getManageTableVersionPageID(PageID_);

		// 1つの空き領域管理表／物理ページ表で管理可能な物理ページ数を得る
		PageNum	pagePerTable = this->getPagePerManageTable();

		// 中間の空き領域管理表／物理ページ表を更新または確保＋初期化する
		while (tableVersionPageID < appendTableVersionPageID) {

			// 空き領域管理表／物理ページ表のバッファリング内容を得る
			// （フィックスする）

			PagePointer table
				 = fixVersionPage(Trans_,
								  tableVersionPageID,
								  WriteFixMode_,
								  Buffer::ReplacementPriority::Low);

			// 空き領域管理表／物理ページ表の
			// バッファリング内容へのポインタを得る
			void*	tablePointer = (void*)(*table);

			// 空き領域管理表ヘッダ／物理ページ表ヘッダから
			// 「使用中の物理ページ数」と
			// 「未使用の物理ページ数」を取り出す
			PageNum	usedPageNum = 0;
			PageNum	unusePageNum = 0;
			this->fetchOutPageNumFromManageTable(tablePointer,
												 usedPageNum,
												 unusePageNum);

			// 空き領域管理表／物理ページ表で管理している
			// 物理ページ数を求める
			PageNum	totalPageNum = usedPageNum + unusePageNum;

			// 空き領域管理表ヘッダ／物理ページ表ヘッダの
			// 「未使用の物理ページ数」を更新する
			this->updateUnusePageNum(tablePointer,
									 pagePerTable - totalPageNum);

			// 次の空き領域管理表／物理ページ表の
			// バージョンページ識別子を設定する
			tableVersionPageID += pagePerTable + 1;
		}

		// 追加した物理ページを管理する空き領域管理表／物理ページ表の
		// バッファリング内容を得る（フィックスする）

		// ここでは新たに確保するページ以外に内部的に増やしたページのみを
		// 未使用の物理ページとして管理表に加える
		// 使用中の物理ページ数の管理は上位のallocatePage2で行う

		PagePointer table
			 = fixVersionPage(Trans_,
							  tableVersionPageID,
							  WriteFixMode_,
							  Buffer::ReplacementPriority::Middle);

		// 空き領域管理表／物理ページ表の
		// バッファリング内容へのポインタを得る
		void*	tablePointer = (void*)(*table);

		// 追加した物理ページを管理する空き領域管理表／物理ページ表の
		// ヘッダに記録されている「使用中の物理ページ数」と
		// 「未使用の物理ページ数」を取り出す
		PageNum	usedPageNum = 0;
		PageNum	unusePageNum = 0;
		this->fetchOutPageNumFromManageTable(tablePointer,
											 usedPageNum,
											 unusePageNum);

		// 追加した物理ページを管理する空き領域管理表／物理ページ表で
		// 管理している物理ページ数を求める
		PageNum	totalPageNum = usedPageNum + unusePageNum;

		// 追加した物理ページを管理する空き領域管理表／物理ページ表の
		// ヘッダの「未使用の物理ページ数」への加算値を求める
		int	addNum = PageID_ % pagePerTable - totalPageNum;

		// 追加した物理ページを管理する空き領域管理表／物理ページ表の
		// ヘッダの「未使用の物理ページ数」を更新する
		this->updateUnusePageNum(tablePointer, addNum);

		// 物理ファイルが管理している物理ページ数を求める
		FileHeader::Item_Type1*	fileHeader =
			static_cast<FileHeader::Item_Type1*>(FileHeader_);
		totalPageNum = fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum;

		// 物理ファイルヘッダの「未使用の物理ページ数」への加算値を求める
		addNum = PageID_ - totalPageNum;

		// 物理ファイルヘッダの「未使用の物理ページ数」を更新する
		FileHeader::updateUnusePageNum(FileHeader_, addNum);

	} catch (Exception::Object&) {

		if (allocated) {

			this->m_VersionFile->truncate(
				Trans_,
				this->convertToVersionPageID(LastPageID_) + 1);
		}

		_SYDNEY_RETHROW;

#ifndef NO_CATCH_ALL
	} catch (...) {

		if (allocated)
		{
			// バージョンファイルを拡張した後に例外が発生した…

			// バージョンファイルを元の状態に縮める
			this->m_VersionFile->truncate(
				Trans_,
				this->convertToVersionPageID(LastPageID_) + 1);
		}

		throw Exception::FileManipulateError(moduleName,
											 srcFile,
											 __LINE__);
#endif
	}
}

//	FUNCTION private
//	PhysicalFile::File::truncate -- トランケートする
//
//	NOTES
//	空き領域管理機能付き物理ファイルまたは
//	物理ページ管理機能付き物理ファイルの場合、
//	物理ファイル末尾にある未使用の物理ページを
//	トランケートする。
//
//	ARGUMENTS
//		const Trans::Transaction&	Transaction_
//			トランザクション記述子への参照
//		bool&				modified
//			true
//				今回の処理で物理ファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の処理で物理ファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				トランケートの結果、物理ファイルが更新されたかを設定する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ファイルをトランケートできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある

// virtual
void
File::truncate(const Trans::Transaction& Transaction_, bool& modified)
{
	if (m_Type == NonManageType ||
		(m_VersionFile->isMounted(Transaction_) == false)) {

		// 管理機能なし物理ファイルか、
		// 実体であるバージョンファイルがマウントされていない
		// 物理ファイルはトランケートする必要はない

		// DirectAreaType も不要。
		// -> やはり必要。

		return;
	}

	//
	// 物理ファイル末尾にある未使用の物理ページを
	// トランケートする。
	//

	try {
		PageNum usedPageNum = 0;
		PageNum unusePageNum = 0;
		{
		// 物理ファイルヘッダーをフィックスし、
		// 使用中の物理ページ数と未使用の物理ページ数を求める
		//
		//【注意】	物理ファイルヘッダーを
		//			Buffer::Page::FixMode::Write モードでフィックスすると、
		//			新しいバージョンが生成されてしまい、
		//			物理ファイルが空でも、
		//			実体であるバージョンファイルがいつまでたっても空にならない
		//
		//			この関数の呼び出し中は論理ファイル単位に
		//			ラッチされているはずなので、まず、物理ファイルヘッダーを
		//			Buffer::Page::FixMode::ReadOnly モードでフィックスして、
		//			新たなバージョンを生成せずに、
		//			トランケートの必要かを確認し、必要であれば、
		//			Buffer::Page::FixMode::Write モードでフィックスし直して、
		//			トランケート処理を続けることにする

		const Version::Page::Memory& fileHeaderPage =
			Version::Page::fix(Transaction_,
							   *m_VersionFile,
							   FileHeader::VersionPageID,
							   Buffer::Page::FixMode::ReadOnly,
							   Buffer::ReplacementPriority::Low);

		FileHeader::fetchOutPageNum(
			static_cast<const void*>(fileHeaderPage), usedPageNum, unusePageNum);
		}
		if (!unusePageNum)

			// 未使用の物理ページは存在しないのでトランケート不要である

			return;

		if (usedPageNum) {

			// いずれかの物理ページが使用中…

			// 物理ファイルが管理している最終物理ページの識別子を求める

			; _SYDNEY_ASSERT(usedPageNum + unusePageNum);
			PageID lastPageID = usedPageNum + unusePageNum - 1;

			// 最終物理ページを管理している空き領域管理表／物理ページ表の
			// バージョンページ識別子を得る
			Version::Page::ID	tableVersionPageID =
				this->getManageTableVersionPageID(lastPageID);
			{
			// 最後の空き領域管理表／物理ページ表のバッファリング内容を得る
			// （フィックスする）
			const Version::Page::Memory&	lastTable =
				Version::Page::fix(
					Transaction_, *m_VersionFile, tableVersionPageID,
					Buffer::Page::FixMode::ReadOnly,
					Buffer::ReplacementPriority::Low);

			// 最後の空き領域管理表／物理ページ表のバッファリング内容への
			// ポインタを得る
			const void*	lastTablePointer = (const void*)lastTable;

			// 最後の物理ページが使用中かどうかをチェックする

			if (isUsedPage(lastTablePointer, lastPageID))

				// 最後の物理ページは使用中なので、トランケートできない

				return;
			}

			// 以降、物理ファイルは必ず更新されてしまう

			modified = true;

			// 物理ファイルヘッダのバッファリング内容を得る（フィックスする）

			Version::Page::Memory	fileHeader(
				Version::Page::fix(
					Transaction_, *m_VersionFile, FileHeader::VersionPageID,
					File::DiscardableWriteFixMode,
					Buffer::ReplacementPriority::Low));
			try {
				// 物理ファイルヘッダのバッファリング内容へのポインタを得る
				void*	fileHeaderPointer = (void*)fileHeader;

				//
				// 最後の物理ページが未使用なので、
				// 不要な物理ページをトランケートする。
				//

				// 最後の使用中の物理ページの識別子を得る
				PageID	lastUsedPageID = this->getLastPageID(Transaction_,
															 usedPageNum,
															 unusePageNum);
				// getLastPageID()は参照したページをキャッシュするので、
				// 直後にキャッシュをクリアする
				unfixNotDirtyPage();

				// トランケートする
				this->m_VersionFile->truncate(
					Transaction_,
					this->convertToVersionPageID(lastUsedPageID) + 1);

				//
				// 物理ファイルヘッダを更新する
				//

				// トランケートする物理ページ数を求める
				int	truncatePageNum = lastPageID - lastUsedPageID;

				// 物理ファイルヘッダに記録されている
				// “未使用の物理ページ数”を更新する
				// （トランケートする物理ページ数分減らす）
				FileHeader::updateUnusePageNum(fileHeaderPointer,
											   -truncatePageNum);

				//
				// 最後の使用中の物理ページを管理している
				// 空き領域管理表／物理ページ表を更新する
				//

				// 最後の使用中の物理ページを管理している
				// 空き領域管理表／物理ページ表のバージョンページ識別子を得る
				tableVersionPageID =
					this->getManageTableVersionPageID(lastUsedPageID);

				// 空き領域管理表／物理ページ表のバッファリング内容を得る
				// （フィックスする）
				Version::Page::Memory	table =
					Version::Page::fix(Transaction_,
									   *m_VersionFile,
									   tableVersionPageID,
									   File::DiscardableWriteFixMode,
									   Buffer::ReplacementPriority::Low);

				// 空き領域管理表／物理ページ表のバッファリング内容への
				// ポインタを得る
				void*	tablePointer = (void*)table;

				// 空き領域管理表ヘッダ／物理ページ表ヘッダから
				// 「使用中の物理ページ数」と「未使用の物理ページ数」を取り出す
				this->fetchOutPageNumFromManageTable(tablePointer,
													 usedPageNum,
													 unusePageNum);

				// 空き領域管理表／物理ページ表が管理している物理ページ数を
				// 求める
				PageNum	managePageNum = usedPageNum + unusePageNum;

				//
				// 空き領域管理表／物理ページ表で管理している物理ページのうち、
				// 先頭と最後の物理ページの識別子を設定する
				//

				PageID	topPageID = 0;
				PageNum	pagePerManageTable = this->getPagePerManageTable();
				while (this->getManageTableVersionPageID(topPageID) !=
					   tableVersionPageID) {
					topPageID += pagePerManageTable;
				}
				lastPageID = topPageID + managePageNum - 1;

				//
				// 空き領域管理表／物理ページ表で管理している物理ページのうち、
				// トランケートする物理ページ数を求める
				//

				truncatePageNum = 0;

				PageID	pageID;
				for (pageID = lastPageID; pageID >= topPageID; pageID--) {

					if (this->isUsedPage(tablePointer, pageID)) break;

					truncatePageNum++;

					if (pageID == 0) break;
				}

				//
				// 空き領域管理表／物理ページ表に記録されている
				// “未使用の物理ページ数”を更新する
				// （トランケートする物理ページ数分減らす）
				//

				bool	tableUnfixMode = false;

				if (truncatePageNum > 0) {
						
					this->updateUnusePageNum(tablePointer, -truncatePageNum);

					tableUnfixMode = true;
				}

				// 空き領域管理表／物理ページ表をアンフィックスする
				table.unfix(tableUnfixMode);

			}
#ifdef NO_CATCH_ALL
			catch (Exception::Object&)
#else
			catch (...)
#endif
			{
				fileHeader.unfix(false);
				_SYDNEY_RETHROW;
			}
		} else if (unusePageNum > 1) {

			// 使用中の物理ページがなく、
			// 未使用の物理ページが 2 つ以上あるとき、
			// 先頭の物理ページを残し、それ以降をすべてトランケートする
			//
			//【注意】	未使用の物理ページが 1 つのときは、
			//			物理ファイル生成直後の状態のなので、なにもしない

			// 以降、物理ファイルは必ず更新されてしまう

			modified = true;

			// 物理ファイルヘッダーをフィックスする

			Version::Page::Memory fileHeaderPage(
				Version::Page::fix(
					Transaction_, *m_VersionFile, FileHeader::VersionPageID,
					File::DiscardableWriteFixMode,
					Buffer::ReplacementPriority::Low));
			try {
				void* header = static_cast<void*>(fileHeaderPage);

				// 実体であるバージョンファイルをトランケートする
				//
				//【注意】	先頭物理ページのバージョンページ識別子は、
				//			物理ファイルヘッダー、
				//			先頭の空き領域管理表/物理ページ表、
				//			の次なので、物理ファイルヘッダーのものの + 2 になる

				m_VersionFile->truncate(
					Transaction_, FileHeader::VersionPageID + 3);

				// 物理ファイルヘッダーを再初期化する

				FileHeader::initialize(header, m_Type);

				// 未使用の物理ページ数を 1 にする

				FileHeader::updateUnusePageNum(header, 1);

				// 先頭の空き領域管理表/物理ページ表を再確保する

				Version::Page::Memory tableMemory(
					Version::Page::fix(
						Transaction_, *m_VersionFile,
						FileHeader::VersionPageID + 1,
						Buffer::Page::FixMode::Allocate,
						Buffer::ReplacementPriority::Low));

				// 未使用の物理ページ数を 1 にする

				updateUnusePageNum(static_cast<void*>(tableMemory), 1);

			}
#ifdef NO_CATCH_ALL
			catch (Exception::Object&)
#else
			catch (...)
#endif
			{
				fileHeaderPage.unfix(false);
				_SYDNEY_RETHROW;
			}
		}
	} catch (Exception::Object&) {
		_SYDNEY_RETHROW;
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		_SYDNEY_THROW0(Exception::FileManipulateError);
	}
#endif
}

#ifdef DEBUG
bool
File::truncate(const Trans::Transaction&	Trans_)
{
	bool	modified = false;
	this->truncate(Trans_, modified);
	return modified;
}
#endif

//
//	FUNCTION private
//	PhysicalFile::File::getLastPageID --
//		最後の使用中の物理ページの識別子を返す
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::PageNum		usedPageNum_
//		使用中ページ数
//	PhysicalFile::PageNum		unusePageNum_
//		未使用ページ数
//
//	RETURN
//	PhysicalFile::PageID
//		最後の使用中の物理ページの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
PageID
File::getLastPageID(const Trans::Transaction&	Transaction_,
					PageNum						usedPageNum_,
					PageNum						unusePageNum_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::fetchOutPageNumFromManageTable --
//		空き領域管理表／物理ページ表から使用中の物理ページ数・未使用の
//		物理ページ数を取り出す
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	const void*	TablePointer_
//		空き領域管理表／物理ページ表のバッファリング内容へのポインタ
//	PageNum&	UsedPageNum_
//		使用中の物理ページ数への参照
//	PageNum&	UnusePageNum_
//		未使用の物理ページ数への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::fetchOutPageNumFromManageTable(const void*	TablePointer_,
									 PageNum&		UsedPageNum_,
									 PageNum&		UnusePageNum_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::updateUsedPageNum --
//		空き領域管理表／物理ページ表に記録されている使用中の物理ページ数を
//		更新する
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	void*		TablePointer_
//		空き領域管理表／物理ページ表のバッファリング内容へのポインタ
//	const int	AddNum_
//		加算数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::updateUsedPageNum(void*		TablePointer_,
						const int	AddNum_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::updateUnusePageNum --
//		空き領域管理表／物理ページ表に記録されている未使用の物理ページ数を
//		更新する
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	void*		TablePointer_
//		空き領域管理表／物理ページ表のバッファリング内容へのポインタ
//	const int	AddNum_
//		加算数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::updateUnusePageNum(void*		TablePointer_,
						const int	AddNum_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::isUsedPage --
//		使用中の物理ページかどうかをチェックする
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	const void*					TablePointer_
//		空き領域管理表／物理ページ表のバッファリング内容へのポインタ
//	const PhysicalFile::PageID	PageID_
//		チェックする物理ページの識別子
//
//	RETURN
//	bool
//		使用中の物理ページかどうか
//			true  : 引数PageID_が使用中の物理ページの識別子である
//			false : 引数PageID_が未使用の物理ページの識別子である
//
//	EXCEPTIONS
//	なし
//
// virtual
bool
File::isUsedPage(const void*	TablePointer_,
				 const PageID	PageID_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::getPagePerManageTable --
//		1つの空き領域管理表／物理ページ表で管理可能な物理ページ数を返す
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageNum
//		1つの空き領域管理表／物理ページ表で管理可能な物理ページ数
//
//	EXCEPTIONS
//	なし
//
// virtual
PageNum
File::getPagePerManageTable() const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::searchNextAssignPage --
//		次に割り当てる物理ページを検索する
//
//	NOTES
//	次に割り当てる物理ページを検索し、識別子を返す。
//	指定された空き領域管理表／物理ページ表で未使用の物理ページを
//	管理していない場合にはPhysicalFile::ConstValue::UndefinedPageIDを返す。
//
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	const void*	TablePointer_
//		空き領域管理表／物理ページ表のバッファリング内容へのポインタ
//
//	RETURN
//	PhysicalFile::PageID
//		次に割り当てる物理ページの識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
PageID
File::searchNextAssignPage(const void*	TablePointer_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//	FUNCTION private
//	PhysicalFile::File::searchNextAssignPage --
//		次に割り当てる物理ページを検索する
//
//	NOTES
//	※ for 物理ページ管理機能つき物理ファイル PageManageFile2
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	const void*					FileHeader_
//		[IN]		ファイルヘッダ
//	const PhysicalFile::PageNum	ManagePageNum_
//		[IN]		物理ファイル内で管理している物理ページ数
//
//	RETURN
//	PhysicalFile::PageID
//		次に割り当てる物理ページの識別子
//
//	EXCEPTIONS

// virtual
PageID
File::searchNextAssignPage(const Trans::Transaction&	Trans_,
						   const void*					FileHeader_,
						   const PageNum				ManagePageNum_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION private
//	PhysicalFile::File::updateManageTable --
//		空き領域管理表／物理ページ表を更新する
//
//	NOTES
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	void*						TablePointer_
//		空き領域管理表／物理ページ表のバッファリング内容へのポインタ
//	const PhysicalFile::PageID	PageID_
//		確保／解放した物理ページの識別子
//	const PhysicalFile::PageNum	PageNum_
//		物理ファイルが管理している物理ページ数
//	const bool					ForReuse_
//		物理ページ再利用のために物理ページ表を更新するかどうか
//			true  : 物理ページ再利用のために更新する
//			false : 物理ページ解放のために更新する
//	const void*					PagePointer_ = 0
//		物理ページのバッファリング内容へのポインタ
//		（空き領域管理機能付き物理ファイルのための引数）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::updateManageTable(void*			TablePointer_,
						const PageID	PageID_,
						const PageNum	PageNum_,
						const bool		ForReuse_,
						const void*		PagePointer_ // = 0
						)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::searchNextAssignPage --
//		次に割り当てる物理ページを検索する
//
//	NOTES
//	次に割り当てる物理ページを検索し、識別子を返す。
//	検索は先頭空き領域管理表／物理ページ表から順に行う。
//
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルについては、
//	　 この仮想関数で良いが、
//	　 今後、異なる物理ファイルタイプの物理ファイルを
//	　 実装することがあれば、この仮想関数を
//	　 オーバーライドする必要がある。
//	　 （不要となる可能性もあるが…）
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageNum	TotalPageNum_
//		物理ファイルで管理している物理ページ数
//
//	RETURN
//	PhysicalFile::PageID
//		次に割り当てる物理ページの識別子
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
// virtual
PageID
File::searchNextAssignPage(const Trans::Transaction&	Transaction_,
						   const PageNum				TotalPageNum_)
{
	// 最終空き領域管理表／物理ページ表のバージョンページ識別子を得る
	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(TotalPageNum_ - 1);

	// 先頭の空き領域管理表／物理ページ表の
	// バージョンページ識別子を求める
	Version::Page::ID	tableVersionPageID =
		FileHeader::VersionPageID + 1;

	// 基準となる物理ページ識別子を設定
	PageID	basePageID = 0;

	// 残りの管理している物理ページ数を設定
	PageNum	remainPageNum = TotalPageNum_;

	// 1つの空き領域管理表／物理ページ表で管理可能な物理ページ数を得る
	PageNum	pagePerManageTable = this->getPagePerManageTable();

	PageID	nextAssignPageID = ConstValue::UndefinedPageID;

	while (1)
	{
		// 空き領域管理表／物理ページ表のバッファリング内容を得る
		// （フィックスする）
	   	PagePointer manageTable =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Low);

		PageNum	managePageNum = pagePerManageTable;
		if (managePageNum > remainPageNum)
		{
			managePageNum = remainPageNum;
		}

		const void*	tableTop = static_cast<const VersionPage&>(
			*manageTable).operator const void*();

		// 次に割り当てる物理ページを検索し、
		// その物理ページの識別子を得る
		nextAssignPageID =
			this->searchNextAssignPage(tableTop);
		
		// 次に割り当てる物理ページが見つかったか
		// 最後の空き領域管理表／物理ページ表まで検索し終わったら
		// 検索終了
		if (nextAssignPageID != ConstValue::UndefinedPageID)
		{
			nextAssignPageID += basePageID;
			break;
		}
		if (tableVersionPageID == lastTableVersionPageID)
		{
			break;
		}

		// 残りの管理している物理ページ数を更新する
		if (remainPageNum >= pagePerManageTable)
		{
			remainPageNum -= pagePerManageTable;
		}

		// 次の空き領域管理表／物理ページ表の
		// バージョンページ識別子を設定する
		tableVersionPageID += pagePerManageTable + 1;

		// 基準となる物理ページ識別子を更新する
		basePageID += pagePerManageTable;
	}

	// 物理ファイルヘッダに記録されている未使用の物理ページ数を参照し、
	// 未使用の物理ページが存在することを確認したのに、
	// 該当する物理ページが存在しないのはおかしい
	; _SYDNEY_ASSERT(nextAssignPageID != ConstValue::UndefinedPageID);

	return nextAssignPageID;
}

//	FUNCTION private
//	PhysicalFile::File::searchNextAssignPage2 --
//		次に割り当てる物理ページを検索する
//
//	NOTES
//	※ 空き領域管理表／物理ページ表が 4 つ以上の場合にのみ呼び出すこと。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageNum	TotalPageNum_
//		物理ファイルで管理している物理ページ数
//
//	RETURN
//	PhysicalFile::PageID
//		次に割り当てる物理ページの識別子
//		見つからなかったら ConstValue::UndefinedPageID
//
//	EXCEPTION

PageID
File::searchNextAssignPage2(const Trans::Transaction&	Transaction_,
							const PageNum				TotalPageNum_)
{
	PageNum	pagePerTable = getPagePerManageTable();

	// 先頭から最終のひとつ手前までの空き領域管理表／物理ページ表のうちから
	// 3 つだけ選ぶ
	// File::getLockManageTable() は checkTableIDs の最後の要素に、
	// 指定した最終空き領域管理表／物理ページ表のバージョンページ識別子を
	// 埋めてくれる
	Version::Page::ID	checkTableIDs[3];
	ModSize				checkCount;
	checkCount =
		File::getLookManageTable(
			pagePerTable,
			FileHeader::VersionPageID + 1,
				// 先頭空き領域管理表／物理ページ表
			this->getManageTableVersionPageID(TotalPageNum_ - 1),
				// 最終空き領域管理表／物理ページ表
			checkTableIDs,
			3);
	; _SYDNEY_ASSERT(checkCount == 3);

	PageID	nextAssignPageID = ConstValue::UndefinedPageID;

	for (ModSize i = 0; i < checkCount; i++) {

		// 空き領域管理表／物理ページ表のバッファリング内容を得る
		// （フィックスする）
	   	PagePointer	table =
			fixVersionPage(Transaction_,
						   checkTableIDs[i],
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Low);

		const void*	tableTop =
			static_cast<const VersionPage&>(*table).operator const void*();

		if ((nextAssignPageID = searchNextAssignPage(tableTop)) !=
			ConstValue::UndefinedPageID) {
			// nextAssignPageID はこのままでは、
			// 空き領域管理表／物理ページ表内での先頭からの識別子のままなので
			// 物理ファイル内での正しい物理ページ識別子に変換する
			nextAssignPageID +=
				(((checkTableIDs[i] - 1) / (pagePerTable + 1)) * pagePerTable);
			break;
		}
	}

	return nextAssignPageID;
}

//
//	FUNCTION private
//	PhysicalFile::File::initializePage --
//		物理ページを初期化する
//
//	NOTES
//	物理ページを初期化する。
//
//	※ 物理ページの初期化が必要な物理ファイル
//	　 （現状では空き領域管理機能付き物理ファイルのみ）は、
//	　 この仮想関数をオーバーライドする必要がある。
//
//	ARGUMENTS
//	void*	PagePointer_
//		物理ページのバッファリング内容へのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::initializePage(void*	PagePointer_)
{
}

//
//	FUNCTION private
//	PhysicalFile::File::getManageTableVersionPageID --
//		空き領域管理表／物理ページ表のバージョンページ識別子を返す
//
//	NOTES
//	引数PageID_で示される物理ページを管理している
//	空き領域管理表／物理ページ表のバージョンページ識別子を返す。
//
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	Version::Page::ID
//		空き領域管理表／物理ページ表のバージョンページ識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
Version::Page::ID
File::getManageTableVersionPageID(const PageID	PageID_) const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::updateAreaBitmap --
//		空き領域管理表内の領域率ビットマップを更新する
//
//	NOTES
//	空き領域管理表内の領域率ビットマップを更新する。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	void*						TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const PhysicalFile::PageID	PageID_
//		領域率を更新する物理ページの識別子
//	const unsigned char			BitmapValue_
//		更新後の領域率ビットマップの値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::updateAreaBitmap(void*				TablePointer_,
					   const PageID			PageID_,
					   const unsigned char	BitmapValue_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}
//
//	FUNCTION private
//	PhysicalFile::File::updatePageBitmap --
//		物理ページ表内の物理ページ使用状態ビットマップを更新する
//
//	NOTES
//	物理ページ表内の物理ページ使用状態ビットマップを更新する。
//
//	※ 物理ページ管理機能付き物理ファイルのみ利用可能。
//	　 （物理ページ管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	void*						TablePointer_
//		物理ページ表のバッファリング内容へのポインタ
//	const PhysicalFile::PageID	PageID_
//		使用状態を更新する物理ページの識別子
//	const bool					BitON_
//		物理ページを使用状態にするかどうか
//			true  : 使用状態とする
//			false : 未使用状態とする
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::updatePageBitmap(void*		TablePointer_,
					   const PageID	PageID_,
					   const bool	BitON_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::updatePageArray --
//		未使用領域率別／空き領域率別の物理ページ数配列を更新する
//
//	NOTES
//	空き領域管理表ヘッダ内に記録されている
//	未使用領域率別の物理ページ数配列または
//	空き領域率別の物理ページ数配列を更新する。
//
//	※ 空き領域管理機能付き物理ファイルと
//	　 物理ページ管理機能付き物理ファイルでは
//	　 この関数が必要なので、Fileクラスでは仮想関数としてある。
//
//	ARGUMENTS
//	void*	TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const bool	ByUnuseAreaRate_
//		未使用領域率別、空き領域率別いずれの物理ページ数配列を更新するか
//			true  : 未使用領域率別の物理ページ数配列を更新
//			false : 空き領域率別の物理ページ数配列を更新
//	const unsigned int	AreaRateValue_
//		未使用領域率／空き領域率 [%]
//	const bool			Increment_
//		インクリメントするのかデクリメントするのか
//			true  : インクリメントする
//			false : デクリメントする
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::updatePageArray(void*					TablePointer_,
					  const bool			ByUnuseAreaRate_,
					  const unsigned int	AreaRateValue_,
					  const bool			Increment_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//	FUNCTION private
//	PhysicalFile::File::updateTree -- 木構造を更新する
//
//	NOTES
//	引数 PageID_ で示される物理ページの使用状況を更新する。
//
//	※ PageManageFile2 専用
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	void*						FileHeader_
//		[IN]		物理ファイルヘッダ
//	void*						Leaf_
//		[IN]		リーフ
//	const PhysicalFile::PageID	PageID_
//		[IN]		管理表更新が必要となった原因である物理ページの識別子
//	const bool					IsUse_
//		[IN]		引数 PageID_ で示される物理ページが使用中か
//						true  : 使用中
//						false : 未使用
//	const bool					AppendPage_
//		[IN]		物理ページ追加に伴う管理表の更新かどうか
//						true  : 物理ページ追加に伴う管理表の更新
//						false : 物理ページの使用状況の変更に伴う管理表の更新
//	const bool					Discardable_
//		[IN]		版を破棄可能な状態でバージョンページを生成するか
//						true  : 版を破棄可能な状態で生成する
//						false : 版を破棄不可能な状態で生成する
//
//	RETURN
//
//	EXCEPTIONS

// virtual
void
File::updateTree(const Trans::Transaction&	Trans_,
				  void*						FileHeader_,
				  void*						Leaf_,
				  const PageID				PageID_,
				  const bool				IsUse_,
				  const bool				AppendPage_,
				  const bool				Discardable_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//	FUNCTION private
//	PhysicalFile::File::clearNode -- ノードの全ビット OFF
//
//	NOTES
//	※ PageManageFile2 専用
//
//	ARGUMENTS
//	void*	Node_
//		[IN]		ノード
//
//	RETURN
//
//	EXCEPTIONS

// virtual
void
File::clearNode(void*	Node_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//	FUNCTION private
//	PhysicalFile::File::updateNode -- ノードのビット ON/OFF
//
//	NOTES
//	※ PageManageFile2 専用
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	void*						FileHeader_
//		[IN]		物理ファイルヘッダ
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//	const bool					BitON_
//		[IN]		ビットを ON とするか OFF とするか
//						true  : ビット ON
//						false : ビット OFF
//	const bool					Discardable_
//		[IN]		版を破棄可能な状態でバージョンページを生成するか
//						true  : 版を破棄可能な状態で生成する
//						false : 版を破棄不可能な状態で生成する
//
//	RETURN
//
//	EXCEPTIONS

// virtual
void
File::updateNode(const Trans::Transaction&	Trans_,
				 void*						FileHeader_,
				 const PageID				PageID_,
				 const bool					BitON_,
				 const bool					Discardable_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//	FUNCTION private
//	PhysicalFile::File::clearTree -- ノードとリーフをクリアする
//
//	NOTES
//	※ PageManageFile2 専用
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	const PhysicalFile::PageNum	TotalPageNum_
//		[IN]		物理ファイル内で管理している物理ページ数
//
//	RETURN
//
//	EXCEPTIONS

// virtual
void
File::clearTree(const Trans::Transaction&	Trans_,
				const PageNum				TotalPageNum_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION private
//	PhysicalFile::File::verifyPage --
//		物理ページ記述子を生成する（バージョンページの整合性検査付き）
//
//	NOTES
//	バージョンページの整合性検査を行い、不整合がなければ、
//	物理ページ記述子を生成し、返す。
//	下位モジュールからMemoryExhaustが送出された場合、
//	2回までリトライする。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*					File_
//		物理ファイル記述子
//	const physicalFile::PageID			PageID_
//		記述子を生成する物理ページの識別子
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	Admin::Verification::Progress&		Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	PhysicalFile::Page*
//		物理ページ記述子
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
Page*
File::verifyPage(const Trans::Transaction&			Transaction_,
				 File*								File_,
				 const PageID						PageID_,
				 const Buffer::Page::FixMode::Value	FixMode_,
				 Admin::Verification::Progress&		Progress_)
{
	Page*	page = 0;

	int	retryCnt = 0;

#ifdef OBSOLETE
	while (retryCnt < 3)
#else
	while (retryCnt < 2)
#endif
	{
		try
		{
			// 物理ページ記述子を生成する
			page = allocatePageInstance(
				Transaction_, PageID_, FixMode_, &Progress_);
			break;
		}
		catch (Exception::MemoryExhaust&)
		{
			// 管理機能なし物理ファイルの場合、
			// その物理ファイルにはバージョンページは一つしかないので
			// step1はすっ飛ばす。
			if (this->m_Type == NonManageType && retryCnt == 0) retryCnt = 1;

			switch (retryCnt)
			{
			case 0:
				if (retryStepSelf() == false) _SYDNEY_RETHROW;
				break;
#ifdef OBSOLETE
			case 1:
				if (retryStepOthers() == false) _SYDNEY_RETHROW;
				break;
#endif
			default:
				_SYDNEY_RETHROW;
			}

			retryCnt++;
		}
	}

	return page;
}

//
//	FUNCTION private
//	PhysicalFile::File::verifyAttachedPage --
//		アタッチ中の物理ページに対応する
//		バージョンページの整合性検査をする
//
//	NOTES
//	アタッチ中の物理ページに対応するバージョンページの整合性検査をする。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	Admin::Verification::Progress	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある

// virtual
void
File::verifyAttachedPage(const Trans::Transaction&		Transaction_,
						 Admin::Verification::Progress&	Progress_) const
{
	// 物理ページ記述子のリンクを保護するためにラッチする

//	Os::AutoCriticalSection	latch(_latch);

	if (this->m_Page != 0)
	{
		Page*	page = this->m_Page;

		Page*	lastPage = page->m_Prev;

		bool	isLastPage = false;

		do
		{
			isLastPage = (page == lastPage);

			Version::Page::verify(Transaction_,
								  *this->m_VersionFile,
								  page->m_Memory.getPageID(),
								  Progress_);

			if (Progress_.isGood() == false)
			{
				break;
			}

			page = page->m_Next;
		}
		while (isLastPage == false);
	}
}

//
//	FUNCTION private
//	PhysicalFile::File::isAttachedPage --
//		物理ページが過去にアタッチされたかどうかを確認する
//
//	NOTES
//	引数PageID_で示される物理ページが、
//	利用者によってアタッチされたことがあるかどうかを確認する。
//	既に利用者がデタッチしていたとしても、
//	物理ファイルマネージャがキャッシュしているのであれば、
//	その物理ページに関しても、アタッチされたことが確認できる。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		アタッチされたかどうかを確認する物理ページの識別子
//
//	RETURN
//	bool
//		過去にアタッチされたかどうか
//			true  : 過去にアタッチされた
//			false : 過去にアタッチされたことを検出できなかった
//
//	EXCEPTIONS
//	なし
//
bool
File::isAttachedPage(const PageID	PageID_) const
{
	// 物理ページ記述子のリンクを保護するためにラッチする

//	Os::AutoCriticalSection	latch(_latch);

	if (this->m_Page != 0)
	{
		Page*	page = this->m_Page;

		Page*	lastPage = page->m_Prev;

		bool	isLastPage = false;

		do
		{
			isLastPage = (page == lastPage);

			if (page->m_ID == PageID_)
			{
				return true;
			}

			page = page->m_Next;
		}
		while (isLastPage == false);
	}

	return false;
}

//
//	FUNCTION private
//	PhysicalFile::File::initializeVerification -- 整合性検査の前処理を行う
//
//	NOTES
//	整合性検査の前処理を行う。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const unsigned int				Treatment_
//		整合性検査の検査方法（“可能ならば修復するか”など）
//		const Admin::Verification::Treatment::Valueを
//		const unsigned intにキャストした値
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある

// virtual
void
File::initializeVerification(const Trans::Transaction&		Transaction_,
							 const unsigned int				Treatment_,
							 Admin::Verification::Progress&	Progress_)
{
	; _SYDNEY_ASSERT(this->m_Check == false);

	this->m_Check = true;

	// 管理機能なし物理ファイルでは、
	// 以下の処理が不要。

	if (this->m_Type != NonManageType) {

		bool	initializedAreaIDBitmap = false;

		try
		{
			//
			// 整合性検査の準備のために、
			// 物理ファイルヘッダから、
			//     使用中の物理ページ数
			//     未使用の物理ページ数
			// を読み出す。
			//

			const Version::Page::Memory&	fileHeader =
				File::fixVersionPage(Transaction_,
									 this,
									 FileHeader::VersionPageID,
									 Buffer::Page::FixMode::ReadOnly,
									 Progress_);

			if (Progress_.isGood() == false)
			{
				return;
			}

			const void*	fileHeaderTop = (const void*)fileHeader;

			PageNum	usedPageNum = FileHeader::getUsedPageNum(fileHeaderTop);

			PageNum	unusePageNum = FileHeader::getUnusePageNum(fileHeaderTop);

			//
			// 整合性検査用データメンバの初期化
			//

			this->m_Treatment = Treatment_;

			this->m_ManagePageNum = usedPageNum + unusePageNum;

			this->m_LastManagePageID = this->m_ManagePageNum - 1;

			this->m_PageIDs.reset();

			if (this->m_Type == AreaManageType)
			{
				this->initializeAreaIDBitmap(this->m_ManagePageNum);

				initializedAreaIDBitmap = true;
			}

			this->m_NotifiedUsePage = false;
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			if (initializedAreaIDBitmap)
			{
				this->terminateAreaIDBitmap(this->m_ManagePageNum);
			}

			// 終了処理
			unfixVersionPage(true);
			m_VersionFile->endVerification(Transaction_, Progress_);

			_SYDNEY_VERIFY_ABORTED(
				Progress_,
				m_VersionFile->getStorageStrategy()._path._masterData,
				Message::InitializeFailed());

			_SYDNEY_RETHROW;
		}
	}

	Version::File::StorageStrategy
		storageStrategy(this->m_VersionFile->getStorageStrategy());

	this->m_FilePath = storageStrategy._path._masterData;
}

//
//	FUNCTION private
//	PhysicalFile::File::terminateVerification -- 整合性検査の後処理を行う
//
//	NOTES
//	整合性検査の後処理を行う。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

// virtual
void
File::terminateVerification()
{
	// 管理機能なし物理ファイルでは後処理不要。

	if (this->m_Type == NonManageType) return;

	this->m_PageIDs.reset();

	if (this->m_Type == AreaManageType)
	{
		this->terminateAreaIDBitmap(this->m_ManagePageNum);
	}
}

//
//	FUNCTION private
//	PhysicalFile::File::initializeAreaIDBitmap --
//		物理エリア識別子を記録するためのビットマップの列を初期化する
//
//	NOTES
//	物理エリア識別子を記録するためのビットマップの列を初期化する。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const PhysicalFile::PageNum	ManagePageNum_
//		物理ファイル内で管理している物理ページ数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::initializeAreaIDBitmap(const PageNum	ManagePageNum_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::terminateAreaIDBitmap --
//		物理エリア識別子を記録するためのビットマップの列を解放する
//
//	NOTES
//	物理エリア識別子を記録するためのビットマップの列を解放する。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const PhysicalFile::PageNum	ManagePageNum_
//		物理ファイル内で管理している物理ページ数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::terminateAreaIDBitmap(const PageNum	ManagePageNum_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::setAreaIDBitmap --
//		物理エリア識別子を記録するためのビットマップを設定する
//
//	NOTES
//	物理エリア識別子を記録するためのビットマップを設定する。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//	const PhysicalFile::AreaNum	AreaNum_
//		引数AreaIDs_の要素数
//	const PhysicalFile::AreaID*	AreaIDs_
//		利用者が使用中とする物理エリアの識別子が
//		記録されている配列へのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// virtual
void
File::setAreaIDBitmap(const PageID	PageID_,
					  const AreaNum	AreaNum_,
					  const AreaID*	AreaIDs_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION protected
//	PhysicalFile::File::fixVersionPage --
//		整合性検査のためにバージョンページをフィックスする（リトライ付き）
//
//	NOTES
//	整合性検査のためにバージョンページをフィックスする。
//	下位モジュールからMemoryExhaustが送出された場合、
//	2回までリトライする。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*					PhysicalFile_
//		物理ファイル記述子
//	const Version::Page::ID				VersionPageID_
//		バージョンページ識別子
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	Admin::Verification::Status::Value&	Progress
//		整合性検査の途中結果への参照
//
//	RETURN
//	Version::Page::Memory
//		バージョンページのバッファリング内容
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
// static
Version::Page::Memory
File::fixVersionPage(const Trans::Transaction&			Transaction_,
					 File*								PhysicalFile_,
					 const Version::Page::ID			VersionPageID_,
					 const Buffer::Page::FixMode::Value	FixMode_,
					 Admin::Verification::Progress&		Progress_)
{
	int	retryCnt = 0;

	while (retryCnt < 3)
	{
		try
		{
			return Version::Page::verify(Transaction_,
										 *PhysicalFile_->m_VersionFile,
										 VersionPageID_,
										 FixMode_,
										 Progress_);
		}
		catch (Exception::MemoryExhaust&)
		{
			// 管理機能なし物理ファイルの場合、
			// その物理ファイルにはバージョンページは一つしかないので
			// step1はすっ飛ばす。
			if (PhysicalFile_->m_Type == NonManageType && retryCnt == 0) retryCnt = 1;

			switch (retryCnt)
			{
			case 0:
				if (PhysicalFile_->retryStepSelf() == false) _SYDNEY_RETHROW;
				break;
#ifdef OBSOLETE
			case 1:
				if (PhysicalFile_->retryStepOthers() == false) _SYDNEY_RETHROW;
				break;
#endif
			default:
				_SYDNEY_RETHROW;
			}

			retryCnt++;
		}
	}

	// ここには、絶対に来ない!
	return Version::Page::verify(Transaction_,
								 *PhysicalFile_->m_VersionFile,
								 VersionPageID_,
								 FixMode_,
								 Progress_);
}

//
//	FUNCTION private
//	PhysicalFile::File::verifyFileHeader --
//		整合性検査のために、物理ファイルヘッダをフィックスする
//
//	NOTES
//	整合性検査のために、物理ファイルヘッダをフィックスる。
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
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある

// virtual
void
File::verifyFileHeader(const Trans::Transaction&		Transaction_,
					   Admin::Verification::Progress&	Progress_)
{
	try
	{
		const Version::Page::Memory	fileHeader =
			File::fixVersionPage(Transaction_,
								 this,
								 FileHeader::VersionPageID,
								 Buffer::Page::FixMode::ReadOnly,
								 Progress_);

		// 管理機能なし物理ファイルでは、以下の処理が不要。

		if (this->m_Type == NonManageType) return;

		if (this->m_NotifiedUsePage == false)
		{
			// 使用中の物理ページを通知されなかった…

			const void*	fileHeaderTop = (const void*)fileHeader;

			//
			// では、全然物理ページを使っていないの？
			//

			if (FileHeader::getUsedPageNum(fileHeaderTop) > 0) {

				// 物理ファイルマネージャとしては、
				// いずれかの物理ページが使用中と認識している…

				_SYDNEY_VERIFY_INCONSISTENT(
					Progress_,
					this->m_FilePath,
					Message::DiscordPageUseSituation1());
			}
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 物理ファイルヘッダをフィックスできなかった…

		_SYDNEY_VERIFY_ABORTED(Progress_,
							   this->m_FilePath,
							   Message::CanNotFixHeaderPage());

		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	PhysicalFile::File::verifyAllTable --
//		整合性検査のために、
//		すべての空き領域管理表／物理ページ表をフィックスする
//
//	NOTES
//	整合性検査のために、
//	すべての空き領域管理表／物理ページ表をフィックスする。
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
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
// virtual
void
File::verifyAllTable(const Trans::Transaction&		Transaction_,
					 Admin::Verification::Progress&	Progress_)
{
	; _SYDNEY_ASSERT(Progress_.isGood());

	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(0);

	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(this->m_LastManagePageID);

	PageNum	pagePerTable = this->getPagePerManageTable();

	while (tableVersionPageID <= lastTableVersionPageID &&
		   Progress_.isGood())
	{
		//
		// 使用中の空き領域管理表／物理ページ表ならば、
		// フィックスしてみる。
		//

		try
		{
			Version::Page::Memory	table =
				File::fixVersionPage(Transaction_,
									 this,
									 tableVersionPageID,
									 Buffer::Page::FixMode::ReadOnly,
									 Progress_);

			table.unfix(false);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			// 空き領域管理表／物理ページ表を
			// フィックスできなかった…

			if (this->m_Type == AreaManageType)
			{
				// 空き領域管理機能つき物理ファイル…

				_SYDNEY_VERIFY_ABORTED(
					Progress_,
					this->m_FilePath,
					Message::CanNotFixAreaManageTable());
			}
			else
			{
				// 物理ページ管理機能つき物理ファイル…

				_SYDNEY_VERIFY_ABORTED(Progress_,
									   this->m_FilePath,
									   Message::CanNotFixPageTable());
			}

			_SYDNEY_RETHROW;
		}

		tableVersionPageID += pagePerTable + 1;
	}
}

//	FUNCTION private
//	PhysicalFile::File::verifyAllPages --
//		整合性検査のために、すべてのノードをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&		cTransaction_
//		[IN]		トランザクション記述子
//	Admin::Verification::Progress&	cProgress_
//		[IN/OUT]	整合性検査の途中経過
//
//	RETURN
//
//	EXCEPTIONS
//	Exception::NotSupported
//		未サポート

// virtual
void
File::verifyAllPages(const Trans::Transaction&		cTransaction_,
					 Admin::Verification::Progress&	cProgress_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION private
//	PhysicalFile::File::correspondUsePage --
//		利用者と自身の物理ページの使用状況が一致するかどうかをチェックする
//
//	NOTES
//	利用者と自身の物理ページの使用状況が一致するかどうかをチェックする。
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
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
File::correspondUsePage(const Trans::Transaction&		Transaction_,
						Admin::Verification::Progress&	Progress_)
{
	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(0);

	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(this->m_LastManagePageID);

	PageNum	pagePerTable = this->getPagePerManageTable();

	PageID	pageID = 0;

	while (tableVersionPageID <= lastTableVersionPageID)
	{
		AutoUnfix cUnfix(this);
		
		Admin::Verification::Progress
			tableProgress(Progress_.getConnection());

		//
		// 可能であれば修復するように指定されているのであれば、
		// 空き領域管理表／物理ページ表を更新する可能性があるため、
		// Writeモードでフィックスする。
		//
		Buffer::Page::FixMode::Value	fixMode =
			Buffer::Page::FixMode::Unknown;

		if (this->isCorrect())
		{
			// 可能ならば修復する…

			//
			// 何らかのエラー発生時に、
			// フィックスした版を破棄できるように
			// Discardableモードも指定する。
			//

			fixMode = File::DiscardableWriteFixMode;
		}
		else
		{
			// 修復しない…

			fixMode = Buffer::Page::FixMode::ReadOnly;
		}

		PagePointer table =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   fixMode,
						   Buffer::ReplacementPriority::Low);

		const Version::Page::Memory&	tableConstRef = *table;

		void*	tableTop = 0;
		if (fixMode == Buffer::Page::FixMode::ReadOnly)
		{
			//
			// ReadOnlyならば、修復しないので、
			// const_cast<void*>しても安全なはず。
			//

			tableTop =
				const_cast<void*>((const void*)tableConstRef);
		}
		else
		{
			tableTop = (void*)(*table);
		}

		while (this->getManageTableVersionPageID(pageID) ==
			   tableVersionPageID)
		{
			bool	used = this->isUsedPage(tableTop, pageID);

			if (used != this->m_PageIDs.test(pageID))
			{
				Admin::Verification::Progress
					pageProgress(Progress_.getConnection());

				if (used)
				{
					// 物理ファイルマネージャとしては、
					// 使用中として認識していたのに、
					// 利用者は未使用と指定していた…

					//
					// この場合は、直せる。
					// 修復指示をされていれば、直して、検査を続ける。
					//

					if ((this->m_Treatment &
						 Admin::Verification::Treatment::Correct)
						!= 0)
					{
						// 修復指示をされている…

						// では、直す。

						this->correctUsePage(Transaction_,
											 pageID,
											 tableTop,
											 pageProgress);
					}
					else
					{
						// 修復指示をされていない…

						_SYDNEY_VERIFY_INCONSISTENT(
							pageProgress,
							this->m_FilePath,
							Message::DiscordPageUseSituation2(pageID));
					}
				}
				else
				{
					// 物理ファイルマネージャとしては、
					// 未使用として認識していたのに、
					// 利用者は、使用中と指定していた…

					// これは直せない。

					_SYDNEY_VERIFY_INCONSISTENT(
						pageProgress,
						this->m_FilePath,
						Message::DiscordPageUseSituation3(pageID));
				}

				tableProgress += pageProgress;

				if (tableProgress.isGood() == false)
				{
					break;
				}
			}

			pageID++;
		}

		table = 0;

		// 修復したのであれば、表を更新したことを伝えなければ。
		if (tableProgress.getStatus() ==
			Admin::Verification::Status::Corrected)
		{
			cUnfix.success();
		}

		if (tableProgress.getStatus() !=
			Admin::Verification::Status::Consistent)
		{
			Progress_ += tableProgress;
		}

		if (Progress_.isGood() == false)
		{
			break;
		}

		tableVersionPageID += pagePerTable + 1;
	}
}

//
//	FUNCTION private
//	PhysicalFile::File::correctUsePage -- 物理ページの使用状態を修復する
//
//	NOTES
//	物理ファイルマネージャでは使用中と認識していた物理ページを
//	未使用状態に修復する。
//	修復できれば引数Progress_にCorrectedを設定し、
//	修復できなければ引数Progress_にInconsistentを設定する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID		PageID_
//		修復する物理ページの識別子（この物理ページを未使用状態にする）
//	void*							TableTop_
//		空き領域管理表／物理ページ表先頭へのポインタ
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//	
void
File::correctUsePage(const Trans::Transaction&		Transaction_,
					 const PageID					PageID_,
					 void*							TableTop_,
					 Admin::Verification::Progress&	Progress_)
{
	bool	corrected = false;

	try
	{
		//
		// 何らかのエラー発生時に、
		// フィックスした版を破棄できるように
		// Discardableモードも指定する。
		//

		PagePointer fileHeader =
			fixVersionPage(Transaction_,
						   FileHeader::VersionPageID,
						   File::DiscardableWriteFixMode,
						   Buffer::ReplacementPriority::Low);

		void*	fileHeaderTop = (void*)(*fileHeader);

		PageNum	usedPageNum = 0;
		PageNum	unusePageNum = 0;
		FileHeader::fetchOutPageNum(fileHeaderTop,
									usedPageNum,
									unusePageNum);

		PageNum	totalPageNum = usedPageNum + unusePageNum;

		; _SYDNEY_ASSERT(totalPageNum > 0);

		if (this->m_Type == AreaManageType)
		{
			// 空き領域管理機能付き物理ファイル…

			//
			// 空き領域管理表を更新する。
			//

			const Version::Page::Memory&	page =
				Version::Page::fix(
					Transaction_,
					*m_VersionFile,
					this->convertToVersionPageID(PageID_),
					Buffer::Page::FixMode::ReadOnly,
					Buffer::ReplacementPriority::Low);

			const void*	pageTop = (const void*)page;

			this->updateManageTable(TableTop_,
									PageID_,
									totalPageNum,
									false, // for free
									pageTop);
		}
		else
		{
			// 物理ページ管理機能付き物理ファイル…

			//
			// 物理ページ表を更新する。
			//

			this->updateManageTable(TableTop_,
									PageID_,
									totalPageNum,
									false); // for free
		}

		//
		// 物理ファイルヘッダを更新する。
		//

		FileHeader::updateUsedPageNum(fileHeaderTop, -1);

		FileHeader::updateUnusePageNum(fileHeaderTop, 1);

		corrected = true;
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 修復できなかった…

		_SYDNEY_VERIFY_INCONSISTENT(
			Progress_,
			this->m_FilePath,
			Message::CanNotCorrectPageUseSituation(PageID_));
	}

	if (corrected)
	{
		// 修復できた…

		_SYDNEY_VERIFY_CORRECTED(
			Progress_,
			this->m_FilePath,
			Message::CorrectedPageUseSituation(PageID_));
	}
}

//
//	FUNCTION private
//	PhysicalFile::File::correspondUseArea --
//		利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする
//
//	NOTES
//	利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする。
//
//	※ 空き領域管理機能付き物理ファイルのみ利用可能。
//	　 （空き領域管理機能付き物理ファイル記述子クラスでのみ
//	　 　この関数をオーバーライドしている。）
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
//
// virtual
void
File::correspondUseArea(const Trans::Transaction&		Transaction_,
						Admin::Verification::Progress&	Progress_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION private
//	PhysicalFile::File::getHeaderPage -- ヘッダーページを得る
//
//	NOTES
//
//	ARGUMENTS
//	Buffer::Page::FixMode::Value eFixMode_
//		FIXモード
//
//	RETURN
//	PhysicalFile::File::VersionPage*
//		ヘッダーページへのポインタ
//
//	EXCEPTIONS
//
File::VersionPage*
File::getHeaderPage(const Trans::Transaction& cTransaction_,
					Buffer::Page::FixMode::Value eFixMode_)
{
	if (m_pHeaderPage)
	{
		// すでにfixされている
		if (eFixMode_ != Buffer::Page::FixMode::ReadOnly
			&& m_pHeaderPage->isUpdatable() == false)
		{
			// 前回のfixがReadOnlyだったので、unfixして
			// あたらしいFIXモードでfixしなおす

			m_pHeaderPage->unfix(false);
		}
	}
	else
	{
		m_pHeaderPage = new VersionPage;
	}

	if (m_pHeaderPage->isOwner() == false)
	{
		// まだfixされていない
		*m_pHeaderPage
			= Version::Page::fix(cTransaction_,
								 *m_VersionFile,
								 FileHeader::VersionPageID,
								 eFixMode_,
								 Buffer::ReplacementPriority::Middle);
	}

	return m_pHeaderPage;
}

//
//	FUNCTION private
//	PhysicalFile::File::getVersionPage -- バージョンページを得る
//
//	NOTES
//	PageListを検索し、存在していればそれを返す。ただしFixModeが異なる場合は、
//	fixしなおす。存在しえいない場合は新たにfixして返す。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Version::Page::ID uiVersionPageID_
//		バージョンページID
//	Buffer::Page::FixMode::Value eFixMode_
//		FIXモード
//	Buffer::ReplacementPriority::Value eReplacementPriority_
//		バッファリング内容の破棄されにくさ
//
//	RETURN
//	PhysicalFile::File::PagePointer
//		バージョンページ
//
//	EXCEPTIONS
//
File::PagePointer
File::getVersionPage(const Trans::Transaction& cTransaction_,
					 Version::Page::ID uiVersionPageID_,
					 Buffer::Page::FixMode::Value eFixMode_,
					 Buffer::ReplacementPriority::Value eReplacementPriority_)
{
	PagePointer pPage;

	if (uiVersionPageID_ == FileHeader::VersionPageID)
	{
		pPage = getHeaderPage(cTransaction_, eFixMode_);
		return pPage;
	}

	ModSize readOnly = 0;
	PageList::Iterator i = m_cPageList.begin();
	PageList::Iterator j = m_cPageList.end();
	for (; i != m_cPageList.end(); ++i)
	{
		if ((*i).getPageID() == uiVersionPageID_)
		{
			j = i;
			break;
		}
		if ((*i).isUpdatable() ==  false && (*i)._reference == 0)
		{
			// ReadOnlyなページは3ページまで、それ以上ある場合、
			// 最後の要素をunfixして再利用する
			
			readOnly++;
			if (readOnly > 3)
				j = i;
		}
	}

	if (j != m_cPageList.end())
	{
		// 検索でヒットした
		if ((eFixMode_ != Buffer::Page::FixMode::ReadOnly
			 && (*j).isUpdatable() == false)
			|| (*j).getPageID() != uiVersionPageID_)
		{
			// 以前のfixがReadOnlyだったので、unfixして
			// あたらしいFIXモードでfixしなおす
			
			(*j).unfix(false);
			(*j) = Version::Page::fix(cTransaction_,
									  *m_VersionFile,
									  uiVersionPageID_,
									  eFixMode_,
									  eReplacementPriority_);
		}

		// 先頭にする
		m_cPageList.splice(m_cPageList.begin(), m_cPageList, j);

		pPage = &(*j);
	}
	else
	{
		// 見つからなかった
		ModAutoPointer<VersionPage> p = new VersionPage;
		*p = Version::Page::fix(cTransaction_,
								*m_VersionFile,
								uiVersionPageID_,
								eFixMode_,
								eReplacementPriority_);

		// 先頭に追加する
		m_cPageList.pushFront(*(p.release()));

		pPage = p.get();
	}

	return pPage;
}

//	FUNCTION private static
//	PhysicalFile::File::getLookManageTable --
//		ランダムで管理テーブルを列挙する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageNum	PagePerTable_
//		ひとつの管理テーブルで管理可能な物理ページ数
//	Version::Page::ID		FirstTableID_
//		先頭の管理テーブルのバージョンページ識別子
//	Version::Page::ID		LastTableID_
//		最後の管理テーブルのバージョンページ識別子
//	Version::Page::ID*		LookPages_
//		参照するべき管理テーブルのバージョンページ識別子
//	ModSize					Size_
//		LookPages_ の容量（要素数）（ 2 以上であること）
//
//	RETURN
//	ModSize
//		LookPages_ に格納したバージョンページ識別子の数
//
//	EXCEPTIONS

// static
ModSize
File::getLookManageTable(	PageNum				PagePerTable_,
							Version::Page::ID	FirstTableID_,
							Version::Page::ID	LastTableID_,
							Version::Page::ID*	LookPages_,
							ModSize				Size_)
{
	if (FirstTableID_ > LastTableID_) return 0;
	
	// シャッフルするページの合計数を求める
	// 最終ページはシャッフルの対象ではない
	ModSize n = ((LastTableID_ - FirstTableID_) / (PagePerTable_ + 1));

	ModSize j = 0;

	if (n == 1)
	{
		// シャッフルの対象ページが1ページなので、シャッフルする必要がない
		LookPages_[j++] = FirstTableID_;
	}
	else if (n > 1)
	{
		// WindowsとLinux, Solaris共通で使用できる乱数ジェネレータがないので、
		// STLのrandom_shuffleを利用する。
		// そのため、まずすべてのエントリを配列に格納する
		// (必要以上に内部で乱数が発生するので、遅いかも...)
		ModVector<Version::Page::ID> v;
		v.reserve(n);
		for (ModSize i = 0; i < n; ++i)
			v.pushBack(FirstTableID_ + (PagePerTable_ + 1) * i);

		// STLのrandom_shuffleで配列をシャッフルする
		std::random_shuffle(&*v.begin(), &*v.end());

		// 要求している数だけ返す
		ModSize count = (n < (Size_ - 1)) ? n : (Size_ - 1);
		while (j < count)
		{
			LookPages_[j] = v[j];
			j++;
		}
	}
	// 最終ページを最後に加える
	LookPages_[j++] = LastTableID_;
	
	return j;
}


///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::FileHeaderクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//	CONST public
//	PhysicalFile::FileHeader::VersionPageID --
//		物理ファイルヘッダのバージョンページ識別子
//
//	NOTES

// static
const Version::Page::ID
FileHeader::VersionPageID = 0;

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::FileHeaderクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	common
//

//	FUNCTION public
//	PhysicalFile::FileHeader::initialize --
//		物理ファイルヘッダを初期化する
//
//	NOTES
//
//	ARGUMENTS
//	void*				FileHeaderPointer_
//		物理ファイルヘッダへのポインタ
//	PhysicalFile::Type	FileType_
//		物理ファイルタイプ
//
//	RETURN
//
//	EXCEPTIONS

// static
void
FileHeader::initialize(void*	FileHeaderPointer_,
					   Type		FileType_)
{
	Item_Common*	item = static_cast<Item_Common*>(FileHeaderPointer_);
	item->m_Version = File::CurrentVersion;	// 物理ファイルバージョン
	item->m_FileType = FileType_;			// 物理ファイルタイプ

	switch (FileType_) {
	case AreaManageType:
	case PageManageType:
	case PageManageType2:
		{
		Item_Type1*	item = static_cast<Item_Type1*>(FileHeaderPointer_);
		item->m_UsedPageNum = 0;	// 使用中の物理ページ数
		item->m_UnusePageNum = 0;	// 未使用の物理ページ数
		}
		break;
	case DirectAreaType:
		{
		Item_Type2*	item = static_cast<Item_Type2*>(FileHeaderPointer_);
		// これが呼ばれる時点で、すでにルートはフィックスされている。
		item->m_ManagePageNum = 1;	// 管理している物理ページ数
		}
		break;
	}
}

//	FUNCTION public
//	PhysicalFile::FileHeader::getSize --
//		物理ファイルヘッダの記録サイズを返す
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::Type	FileType_
//		物理ファイルタイプ
//
//	RETURN
//	Os::Memory::Size
//		物理ファイルヘッダの記録サイズ [byte]
//
//	EXCEPTIONS
//	Exception::BadArgument
//		不正な引数

// static
Os::Memory::Size
FileHeader::getSize(const Type	FileType_)
{
	Os::Memory::Size	sz = 0;

	switch (FileType_) {
	case NonManageType:
		sz = sizeof(Item_Common);
		break;
	case AreaManageType:
	case PageManageType:
	case PageManageType2:
		sz = sizeof(Item_Type1);
		break;
	case DirectAreaType:
		sz = sizeof(Item_Type2);
		break;
	default:
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	return sz;
}

//
//	for type 1
//

//	FUNCTION public
//	PhysicalFile::FileHeader::updateUsedPageNum --
//		使用中の物理ページ数を更新する
//
//	NOTES
//
//	ARGUMENTS
//	void*		FileHeaderPointer_
//		物理ファイルヘッダへのポインタ
//	const int	AddNum_
//		使用中の物理ページ数への加算値
//
//	RETURN
//	PhysicalFile::PageNum
//		更新後の使用中の物理ページ数
//
//	EXCEPTIONS

// static
PageNum
FileHeader::updateUsedPageNum(void*		FileHeaderPointer_,
							  const int	AddNum_)
{
	Item_Type1*	item = static_cast<Item_Type1*>(FileHeaderPointer_);
	if (AddNum_) {

		// 使用中の物理ページ数を更新する
#ifdef DEBUG
		if (AddNum_ < 0) {
			; _SYDNEY_ASSERT(
				static_cast<unsigned int>(-AddNum_) <= item->m_UsedPageNum);
		}
#endif // DEBUG
		item->m_UsedPageNum += AddNum_;
	}
	return item->m_UsedPageNum;
}

//	FUNCTION public
//	PhysicalFile::FileHeader::updateUnusePageNum --
//		未使用の物理ページ数を更新する
//
//	NOTES
//
//	ARGUMENTS
//	void*		FileHeaderPointer_
//		物理ファイルヘッダへのポインタ
//	const int	AddNum_
//		未使用の物理ページ数への加算値
//
//	RETURN
//	PhysicalFile::PageNum
//		更新後の未使用の物理ページ数
//
//	EXCEPTIONS

// static
PageNum
FileHeader::updateUnusePageNum(void*		FileHeaderPointer_,
							   const int	AddNum_)
{
	Item_Type1*	item = static_cast<Item_Type1*>(FileHeaderPointer_);
	if (AddNum_) {

		// 未用中の物理ページ数を更新する
#ifdef DEBUG
		if (AddNum_ < 0) {
			; _SYDNEY_ASSERT(
				static_cast<unsigned int>(-AddNum_) <= item->m_UnusePageNum);
		}
#endif // DEBUG
		item->m_UnusePageNum += AddNum_;
	}
	return item->m_UnusePageNum;
}

//	FUNCTION public
//	PhysicalFile::FileHeader::fetchOutPageNum --
//		使用中／未使用物理ページ数を取り出す
//
//	NOTES
//
//	ARGUMENTS
//	const void*				FileHeaderPointer_
//		物理ファイルヘッダへのポインタ
//	PhysicalFile::PageNum&	UsedPageNum_
//		使用中の物理ページ数への参照
//	PhysicalFile::PageNum&	UnusePageNum_
//		未使用の物理ページ数への参照
//
//	RETURN
//
//	EXCEPTIONS

// static
void
FileHeader::fetchOutPageNum(const void*	FileHeaderPointer_,
							PageNum&	UsedPageNum_,
							PageNum&	UnusePageNum_)
{
	const Item_Type1*	item =
		static_cast<const Item_Type1*>(FileHeaderPointer_);
	UsedPageNum_ = item->m_UsedPageNum;
	UnusePageNum_ = item->m_UnusePageNum;
}

//	FUNCTION public
//	PhysicalFile::FileHeader::getUsedPageNum --
//		使用中の物理ページ数を返す
//
//	NOTES
//
//	ARGUMENTS
//	const void*	FileHeaderPointer_
//		物理ファイルヘッダへのポインタ
//
//	RETURN
//	PhysicalFile::PageNum
//		使用中の物理ページ数
//
//	EXCEPTIONS

// static
PageNum
FileHeader::getUsedPageNum(const void*	FileHeaderPointer_)
{
	const Item_Type1*	item =
		static_cast<const Item_Type1*>(FileHeaderPointer_);
	return item->m_UsedPageNum;
}

//	FUNCTION public
//	PhysicalFile::FileHeader::getUnusePageNum --
//		未使用の物理ページ数を返す
//
//	NOTES
//
//	ARGUMENTS
//	const void*	FileHeaderPointer_
//		物理ファイルヘッダへのポインタ
//
//	RETURN
//	PhysicalFile::PageNum
//		未使用の物理ページ数
//
//	EXCEPTIONS

// static
PageNum
FileHeader::getUnusePageNum(const void*	FileHeaderPointer_)
{
	const Item_Type1*	item =
		static_cast<const Item_Type1*>(FileHeaderPointer_);
	return item->m_UnusePageNum;
}

//
//	for type 2
//

//	FUNCTION public
//	PhysicalFile::FileHeader::setManagePageNum --
//		Set the number of managed pages
//
//	NOTES
//	If the argument of the PageNum represents the difference,
//	PageNum is not enough.
//	Because, the difference needs the number of plus or minus 4G.
//
//	ARGUMENTS
//	void*		pFileHeaderPointer_
//		Pointer to the file header
//	const PageNum	uiNum_
//		The number of pages which is managed in the file from now.
//		
//	RETURN
//
//	EXCEPTIONS

// static
void
FileHeader::setManagePageNum(void*			pFileHeaderPointer_,
							 const PageNum	uiNum_)
{
	Item_Type2*	item = static_cast<Item_Type2*>(pFileHeaderPointer_);
	item->m_ManagePageNum = uiNum_;
}

//	FUNCTION public
//	PhysicalFile::FileHeader::getManagePageNum --
//		管理している物理ページ数を返す
//
//	NOTES
//
//	ARGUMENTS
//	const void*	pFileHeaderPointer_
//		物理ファイルヘッダへのポインタ
//
//	RETURN
//	PhysicalFile::PageNum
//		管理している物理ページ数
//
//	EXCEPTIONS

// static
PageNum
FileHeader::getManagePageNum(const void*	pFileHeaderPointer_)
{
	const Item_Type2*	item =
		static_cast<const Item_Type2*>(pFileHeaderPointer_);
	return item->m_ManagePageNum;
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
