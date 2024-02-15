// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PageManageFile2.cpp --
//		物理ページ管理機能付き物理ファイル関連の関数定義
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

#include "Common/Assert.h"

#include "Exception/FileManipulateError.h"

#include "PhysicalFile/PageManageFile2.h"
#include "PhysicalFile/PageManagePage.h"
#include "PhysicalFile/Message_CanNotFixPageTable.h"
#include "PhysicalFile/Message_DiscordPageUseSituation2.h"
#include "PhysicalFile/Message_DiscordPageUseSituation3.h"
#include "PhysicalFile/Message_CanNotCorrectPageUseSituation.h"
#include "PhysicalFile/Message_CorrectedPageUseSituation.h"
#include "PhysicalFile/Message_DiscordManagePageNum.h"
#include "PhysicalFile/Message_DiscordUsePageNum.h"
#include "PhysicalFile/Message_DiscordUnusePageNumInTable.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFile2 クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::PageManageFile2::recoverPage --
//		物理ページ記述子を破棄し、ページ内容を元に戻す
//
//	NOTES
//	物理ページ記述子を破棄し、ページ内容をアタッチ前の状態に戻す。
//	※ FixMode::WriteとFixMode::Discardableを
//	　 マスクしたモードでアタッチした物理ページにのみ有効。
//
//	ARGUMENTS
//	PhysicalFile::Page*&	Page_
//		[IN]		破棄する物理ページの記述子
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::recoverPage(Page*&	Page_)
{
	this->detachPage(Page_, Page::UnfixMode::NotDirty);
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::recoverPageAll --
//		生成されている全物理ページ記述子を破棄し、ページ内容を元に戻す
//
//	NOTES
//	生成されている全物理ページ記述子を破棄し、
//	ページ内容をアタッチ前の状態に戻す。
//	※ FixMode::WriteとFixMode::Discardableを
//	　 マスクしたモードでアタッチした物理ページにのみ有効。
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::recoverPageAll()
{
	while (this->m_Page != 0) {
		this->detachPage(this->m_Page, Page::UnfixMode::NotDirty);
	}

	File::recoverPageAll();
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::getTopPageID --
//		先頭の使用中の物理ページの識別子を返す
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//
//	RETURN
//	PhysicalFile::PageID
//		先頭の使用中の物理ページの識別子
//
//	EXCEPTIONS

PageID
PageManageFile2::getTopPageID(const Trans::Transaction&	Trans_)
{
	PagePointer	fileHeaderPage =
		fixVersionPage(Trans_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	const FileHeader::Item_Type1*	fileHeader =
		static_cast<const FileHeader::Item_Type1*>(
			static_cast<const VersionPage&>(*fileHeaderPage).operator const void*());
			//fileHeaderPage->operator const void*());
	if (fileHeader->m_UsedPageNum == 0) {

		// 物理ファイル内に使用中の物理ページが存在しないならば
		// “先頭の使用中の物理ページ”など存在しない
		return ConstValue::UndefinedPageID;
	}

	PageID	topPageID = ConstValue::UndefinedPageID;

	//
	// ノードには、
	// “リーフが未使用物理ページを管理しているか？”
	// ということだけを記録している。
	// したがって、ノードを参照しても
	// “リーフが使用中の物理ページを管理しているか？”
	// ということはわからない。
	//
	// 未使用物理ページを管理していない”リーフの場合は
	// 必ず“使用中の物理ページを管理している”ことになるが、
	// “未使用物理ページを管理している”リーフでは
	// “使用中の物理ページを管理している”かもしれないし
	// “管理していない”かもしれないので。
	//
	// このため、ここではリーフを順に参照していく。
	//

	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	PageID	lastPageID =
		fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum - 1;
	for (PageID pageID = 0; pageID <= lastPageID; pageID += pagePerLeaf) {

		Version::Page::ID	leafVPID =
			this->m_Leaf.getVersionPageID(pageID);
		PagePointer	leafPage =
			fixVersionPage(Trans_,
						   leafVPID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		// 使用中の物理ページを検索する
		int	pageIndex =
			this->m_Leaf.searchUsedPage(static_cast<const VersionPage&>(*leafPage).operator const void*());
			//this->m_Leaf.searchUsedPage(leafPage->operator const void*());
		if (pageIndex >= 0) {

			// 使用中の物理ページが見つかった
			topPageID = pageID + pageIndex;
			break;
		}
	}

	return topPageID;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::getLastPageID --
//		最後の使用中の物理ページの識別子を返す
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//
//	RETURN
//	PhysicalFile::PageID
//		最後のの使用中の物理ページの識別子
//
//	EXCEPTIONS

PageID
PageManageFile2::getLastPageID(const Trans::Transaction&	Trans_)
{
	PagePointer	fileHeaderPage =
		fixVersionPage(Trans_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	const FileHeader::Item_Type1*	fileHeader =
		static_cast<const FileHeader::Item_Type1*>(
			static_cast<const VersionPage&>(*fileHeaderPage).operator const void*());
			//fileHeaderPage->operator const void*());
	if (fileHeader->m_UsedPageNum == 0) {

		// 物理ファイル内に使用中の物理ページが存在しないならば
		// “最後の使用中の物理ページ”など存在しない
		return ConstValue::UndefinedPageID;
	}

	PageID	lastPageID = ConstValue::UndefinedPageID;

	// ※ ノードを参照しない理由は上の getTopPageID() のコメントを参照

	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	PageID	pageID =
		fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum - 1;

	// 各リーフの最初のビットで管理している物理ページをチェックするように
	// 物理ページ識別子を更新する
	pageID -= (pageID % pagePerLeaf);

	for (;
		 lastPageID == ConstValue::UndefinedPageID;
		 pageID -= pagePerLeaf) {

		Version::Page::ID	leafVPID =
			this->m_Leaf.getVersionPageID(pageID);
		PagePointer	leafPage =
			fixVersionPage(Trans_,
						   leafVPID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		// 使用中の物理ページを検索する
		int	pageIndex =
			this->m_Leaf.searchUsedPageBehind(
				static_cast<const VersionPage&>(*leafPage).operator const void*());
				//leafPage->operator const void*());
		if (pageIndex >= 0) {

			// 使用中の物理ページが見つかった
			lastPageID = pageID + pageIndex;

		} else if (pageID == 0) {

			// 先頭のリーフまで検索したが見つからなかった
			break;
		}
	}

	return lastPageID;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::getNextPageID --
//		次の使用中の物理ページの識別子を返す
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	PhysicalFile::PageID
//		次の使用中の物理ページの識別子
//
//	EXCEPTIONS

PageID
PageManageFile2::getNextPageID(const Trans::Transaction&	Trans_,
							   const PageID					PageID_)
{
	; _SYDNEY_ASSERT(PageID_ != ConstValue::UndefinedPageID);

	PagePointer	fileHeaderPage =
		fixVersionPage(Trans_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	const FileHeader::Item_Type1*	fileHeader =
		static_cast<const FileHeader::Item_Type1*>(
			static_cast<const VersionPage&>(*fileHeaderPage).operator const void*());
			//fileHeaderPage->operator const void*());
	if (fileHeader->m_UsedPageNum == 0) {

		// 物理ファイル内に使用中の物理ページが存在しないならば
		// “次の使用中の物理ページ”など存在しない
		return ConstValue::UndefinedPageID;
	}

	PageID	lastPageID =
		fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum - 1;
	if (PageID_ >= lastPageID) {

		// 物理ファイルで管理していない物理ページや
		// 管理している最後の物理ページの識別子が指定されたのなら
		// “次の使用中の物理ページ”など存在しない
		return ConstValue::UndefinedPageID;
	}

	PageID	nextPageID = ConstValue::UndefinedPageID;

	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	PageID	pageID = PageID_ - (PageID_ % pagePerLeaf);
	int	startPageIndex = PageID_ % pagePerLeaf;
	for (; pageID <= lastPageID; pageID += pagePerLeaf) {

		Version::Page::ID	leafVPID =
			this->m_Leaf.getVersionPageID(pageID);
		PagePointer	leafPage =
			fixVersionPage(Trans_,
						   leafVPID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		// 使用中の物理ページを検索する
		int	pageIndex =
			//this->m_Leaf.searchUsedPage(leafPage->operator const void*(),
			this->m_Leaf.searchUsedPage(static_cast<const VersionPage&>(*leafPage).operator const void*(),
										startPageIndex);
		if (pageIndex >= 0) {

			// 使用中の物理ページが見つかった
			nextPageID = pageID + pageIndex;
			break;
		}

		if (startPageIndex >= 0) startPageIndex = -1;
	}

	return nextPageID;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::getPrevPageID --
//		前の使用中の物理ページの識別子を返す
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	PhysicalFile::PageID
//		前の使用中の物理ページの識別子
//
//	EXCEPTIONS

PageID
PageManageFile2::getPrevPageID(const Trans::Transaction&	Trans_,
							   const PageID					PageID_)
{
	; _SYDNEY_ASSERT(PageID_ != ConstValue::UndefinedPageID);

	PagePointer	fileHeaderPage =
		fixVersionPage(Trans_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	const FileHeader::Item_Type1*	fileHeader =
		static_cast<const FileHeader::Item_Type1*>(
			static_cast<const VersionPage&>(*fileHeaderPage).operator const void*());
			//fileHeaderPage->operator const void*());
	if (fileHeader->m_UsedPageNum == 0) {

		// 物理ファイル内に使用中の物理ページが存在しないならば
		// “前の使用中の物理ページ”など存在しない
		return ConstValue::UndefinedPageID;
	}

	PageID	lastPageID =
		fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum - 1;
	if (PageID_ > lastPageID) {

		// 物理ファイルで管理していない物理ページが指定されたのなら
		// “前の使用中の物理ページ”は“最後の使用中の物理ページ”である
		return this->getLastPageID(Trans_);
	}

	PageID	prevPageID = ConstValue::UndefinedPageID;

	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	PageID	pageID = PageID_ - (PageID_ % pagePerLeaf);
	int	startPageIndex = PageID_ % pagePerLeaf;
	for (;
		 prevPageID == ConstValue::UndefinedPageID;
		 pageID -= pagePerLeaf) {

		Version::Page::ID	leafVPID =
			this->m_Leaf.getVersionPageID(pageID);
		PagePointer	leafPage =
			fixVersionPage(Trans_,
						   leafVPID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		int	pageIndex =
			this->m_Leaf.searchUsedPageBehind(
				static_cast<const VersionPage&>(*leafPage).operator const void*(),
				//leafPage->operator const void*(),
				startPageIndex);
		if (pageIndex >= 0) {

			prevPageID = pageID + pageIndex;

		} else if (pageID == 0) {

			break;
		}

		if (startPageIndex >= 0) startPageIndex = -1;

	}

	return prevPageID;
}


///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFile2 クラスの private メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION private
//	PhysicalFile::PageManageFile2::PageManageFile2 -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::File::StorageStrategy&		FileStorageStrategy_
//		[IN]		物理ファイル格納戦略
//	const PhysicalFile::File::BufferingStrategy&	BufferingStrategy_
//		[IN]		物理ファイルバッファリング戦略
//	const Lock::FileName*							LockName_
//		[IN]		物理ファイルが存在する論理ファイルのロック名
//	bool											Batch_
//		[IN]		バッチインサートモードかどうか
//						true  : バッチインサートモード
//						false : バッチインサートモードではない
//
//	RETURN
//
//	EXCEPTIONS

PageManageFile2::PageManageFile2(
	const File::StorageStrategy&	FileStorageStrategy_,
	const File::BufferingStrategy&	BufferingStrategy_,
	const Lock::FileName*			LockName_,
	bool							Batch_)
	: File(FileStorageStrategy_,
		   BufferingStrategy_,
		   LockName_,
		   Batch_),
	  m_Tree(),
	  m_Node(m_Tree, m_Leaf),
	  m_Leaf(m_Tree, m_Node)
{
	try {
		this->m_VersionPageSize =
			Version::File::verifyPageSize(
				FileStorageStrategy_.m_VersionFileInfo._pageSize);

#ifdef NO_CATCH_ALL
	} catch (Exception::Object&) {
#else
	} catch (...) {
#endif
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	this->m_VersionPageDataSize =
		Version::Page::getContentSize(this->m_VersionPageSize);

	// ノードのビットマップサイズを設定
	this->m_Node.setBitmapSize(this->m_VersionPageDataSize,
							   FileHeader::getSize(PageManageType2));

	// リーフのビットマップサイズを設定
	this->m_Leaf.setBitmapSize(this->m_VersionPageDataSize);

	// ひとつの木構造あたりのページ数を設定
	this->m_Tree.setPageNum(this->m_Node.getManageLeafMax(),
							this->m_Leaf.getManagePageMax());

	// 公開領域最大サイズ
	this->m_UserAreaSizeMax = this->m_VersionPageDataSize;
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::~PageManageFile2 -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

PageManageFile2::~PageManageFile2()
{
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::allocatePageInstance --
//		Allocate Page instance
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
PageManageFile2::allocatePageInstance(
	const Trans::Transaction&			cTransaction_,
	PageID								uiPageID_,
	Buffer::Page::FixMode::Value		eFixMode_,
	Admin::Verification::Progress*		pProgress_,
	Buffer::ReplacementPriority::Value	eReplacementPriority_)
{
	if (pProgress_ == 0)
	{
		return new PageManagePage(
			cTransaction_, this, uiPageID_, eFixMode_, eReplacementPriority_);
	}
	else
	{
		// For verify
		return new PageManagePage(
			cTransaction_, this, uiPageID_, eFixMode_, *pProgress_);
	}
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::initialize -- 物理ファイル生成時の初期化
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

void
PageManageFile2::initialize(const Trans::Transaction&	Trans_,
							void*						FileHeader_)
{
	// 先頭リーフを確保
	PagePointer	topLeaf = fixVersionPage(Trans_,
										 FileHeader::VersionPageID + 1,
										 Buffer::Page::FixMode::Allocate,
										 Buffer::ReplacementPriority::Middle);

	// 先頭物理ページを確保
	this->createTopPage(Trans_,
						FileHeader_,
						topLeaf->operator void*());
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::truncate -- トランケートする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	bool&						Modified_
//		[OUT]		物理ファイル更新フラグ
//					（トランケート処理により物理ファイルが更新されたかどうか）
//						true  : 更新された
//						false : 更新されていない
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::truncate(const Trans::Transaction&	Trans_,
						  bool&						Modified_)
{
	PageID	lastManagePageID, lastUsedPageID;
	if (this->needTruncate(Trans_,
						   Modified_,
						   lastManagePageID,
						   lastUsedPageID) == false) return;

	// 最後の使用中の物理ページを管理しているリーフを更新
	// （それ以降のリーフはトランケートされるので更新不要）

	Version::Page::Memory	fileHeaderPage =
		Version::Page::fix(Trans_,
						   *this->m_VersionFile,
						   FileHeader::VersionPageID,
						   File::DiscardableWriteFixMode,
						   Buffer::ReplacementPriority::Middle);
	FileHeader::Item_Type1*	fileHeader =
		static_cast<FileHeader::Item_Type1*>(fileHeaderPage.operator void*());
	Version::Page::ID	leafVPID =
		this->m_Leaf.getVersionPageID(lastUsedPageID);
	Version::Page::Memory	leafPage =
		Version::Page::fix(Trans_,
						   *this->m_VersionFile,
						   leafVPID,
						   File::DiscardableWriteFixMode,
						   Buffer::ReplacementPriority::Middle);
	bool	leafUnfixMode = false;

	try {

		void*	leaf = leafPage.operator void*();
		Leaf::Header*	leafHeader = static_cast<Leaf::Header*>(leaf);

		PageNum	pagePerLeaf =
			leafHeader->m_UsedPageNum + leafHeader->m_UnusePageNum;
		int	endIndex = pagePerLeaf - 1;
		int	startIndex =
			this->m_Leaf.searchUsedPageBehind(leaf);

		if (startIndex < endIndex) {

			// リーフヘッダを更新
			PageNum	truncatePageNum = endIndex - startIndex;
			; _SYDNEY_ASSERT(leafHeader->m_UnusePageNum >= truncatePageNum);
			leafHeader->m_UnusePageNum -= truncatePageNum;
			leafUnfixMode = true;
		}

		// トランケート
		this->m_VersionFile->truncate(
			Trans_,
			this->convertToVersionPageID(lastUsedPageID) + 1);

		// ノードのビットマップを更新
		Version::Page::ID	nodeVPID =
			this->m_Node.getVersionPageID(lastUsedPageID);
		if (nodeVPID == FileHeader::VersionPageID) {

			if (leafHeader->m_UnusePageNum == 0) {
				this->m_Node.clear(Trans_,
								   fileHeader,
								   lastUsedPageID,
								   lastManagePageID,
								   leafHeader->m_UnusePageNum);
			}

		} else {

			Version::Page::Memory	nodePage =
				Version::Page::fix(Trans_,
								   *this->m_VersionFile,
								   nodeVPID,
								   File::DiscardableWriteFixMode,
								   Buffer::ReplacementPriority::Middle);
			if (leafHeader->m_UnusePageNum == 0) {
				this->m_Node.clear(Trans_,
								   nodePage.operator void*(),
								   lastUsedPageID,
								   lastManagePageID,
								   leafHeader->m_UnusePageNum);
				nodePage.unfix(true);
			} else {
				nodePage.unfix(false);
			}
		}

		// 物理ファイルヘッダを更新
		PageNum	truncatePageNum = lastManagePageID - lastUsedPageID;
		fileHeader->m_UnusePageNum -= truncatePageNum;

		Modified_ = true;

		fileHeaderPage.unfix(true);
		leafPage.unfix(leafUnfixMode);

	} catch (Exception::Object&) {

		fileHeaderPage.unfix(false);
		leafPage.unfix(false);
		_SYDNEY_RETHROW;

#ifndef NO_CATCH_ALL
	} catch (...) {
		fileHeaderPage.unfix(false);
		leafPage.unfix(false);
		_SYDNEY_THROW0(Exception::FileManipulateError);
#endif
	}
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::needTruncate --
//		トランケートが必要かどうかを調べる
//
//	NOTES
//	呼び出し側でトランケートが必要ならば
//	引数 LastManagePageID_ と LastUsedPageID_ に値が設定される。
//	ファイルを空の状態にする場合にのみ、本メソッド内で
//	更新（トランケート）を行い、その場合には呼び出し側での
//	トランケートは不要となる。
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	bool						Modified_
//		[OUT]		物理ファイル更新フラグ
//					（トランケート処理により物理ファイルが更新されたかどうか）
//						true  : 更新された
//						false : 更新されていない
//	PhysicalFile::PageID&		LastManagePageID_
//		[OUT]		物理ファイル内で管理している最後の物理ページの識別子
//	PhysicalFile::PageID&		LastUsedPageID_
//		[OUT]		物理ファイル内で管理している最後の使用中の物理ページの識別子
//
//	RETURN
//	bool
//		呼び出し側でトランケートの処理が必要かどうか
//			true  : 必要
//			false : 不要
//
//	EXCEPTIONS

bool
PageManageFile2::needTruncate(const Trans::Transaction&	Trans_,
							  bool&						Modified_,
							  PageID&					LastManagePageID_,
							  PageID&					LastUsedPageID_)
{
	Modified_ = false;

	// マウントされていなければトランケート不要
	if (this->m_VersionFile->isMounted(Trans_) == false) return false;

	// 本メソッド内で更新（トランケート）してしまうかどうか
	bool	modify = false;

	try {

		const Version::Page::Memory&	fileHeaderPage =
			Version::Page::fix(Trans_,
							   *this->m_VersionFile,
							   FileHeader::VersionPageID,
							   Buffer::Page::FixMode::ReadOnly,
							   Buffer::ReplacementPriority::Middle);

		const FileHeader::Item_Type1*	fileHeader =
			static_cast<const FileHeader::Item_Type1*>(
				static_cast<const VersionPage&>(fileHeaderPage).operator const void*());
				//fileHeaderPage.operator const void*());

		if (fileHeader->m_UnusePageNum == 0) {

			// 未使用の物理ページがなければトランケート不要
			return false;

		} else if (fileHeader->m_UsedPageNum == 0) {

			if (fileHeader->m_UnusePageNum == 1) {

				// 使用中物理ページ：なし
				// 未使用物理ページ：1 ページ

				// 物理ファイル生成直後の状態なのでトランケート不要
				return false;

			} else {

				// 使用中物理ページ：なし
				// 未使用物理ページ：2 ページ以上

				// この場合だけこのメソッド内で更新（トランケート）を行う
				modify = true;
			}
		}

		if (modify == false) {
			PageID	lastManagePageID =
				fileHeader->m_UsedPageNum + fileHeader->m_UnusePageNum - 1;
			PageID	lastUsedPageID = this->getLastPageID(Trans_, lastManagePageID);

			if (lastManagePageID == lastUsedPageID) {

				// 管理している最後の物理ページが使用中ならばトランケート不要
				return false;
			}

			LastManagePageID_ = lastManagePageID;
			LastUsedPageID_ = lastUsedPageID;
		}

	} catch (Exception::Object&) {

		_SYDNEY_RETHROW;

#ifndef NO_CATCH_ALL
	} catch (...) {
		_SYDNEY_THROW0(Exception::FileManipulateError);
#endif
	}

	if (modify) {

		// このメソッドで更新（トランケート）を行う

		try {

			// 物理ファイルをクリアした上でトランケートを行う
			this->clear(Trans_,
						false); // 生成しなおさない
			this->m_VersionFile->truncate(
				Trans_,
				this->convertToVersionPageID(0) + 1);

			Modified_ = true;

		} catch (Exception::Object&) {

			_SYDNEY_RETHROW;

#ifndef NO_CATCH_ALL
		} catch (...) {
			_SYDNEY_THROW0(Exception::FileManipulateError);
#endif
		}
	}

	// このメソッド内で更新（トランケート）をしていないのであれば
	// 呼び出し側で必要
	return modify == false;
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::getLastPageID --
//		最後の使用中の物理ページの識別子を返す
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	const PhysicalFile::PageID	LastManagePageID_
//		[IN]		物理ファイル内で管理している最後の物理ページの識別子
//
//	RETURN
//	PhysicalFile::PageID
//		最後の使用中の物理ページの識別子
//
//	EXCEPTIONS

PageID
PageManageFile2::getLastPageID(const Trans::Transaction&	Trans_,
							   const PageID					LastManagePageID_)
{
	PageID	lastPageID = ConstValue::UndefinedPageID;

	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	PageID	pageID = LastManagePageID_;

	// 各リーフの最初のビットで管理している物理ページをチェックするように
	// 物理ページ識別子を更新する
	pageID -= (pageID % pagePerLeaf);

	for (;
		 lastPageID == ConstValue::UndefinedPageID;
		 pageID -= pagePerLeaf) {

		Version::Page::ID	leafVPID =
			this->m_Leaf.getVersionPageID(pageID);
		const Version::Page::Memory&	leafPage =
			Version::Page::fix(Trans_,
							   *this->m_VersionFile,
							   leafVPID,
							   Buffer::Page::FixMode::ReadOnly,
							   Buffer::ReplacementPriority::Middle);

		int	pageIndex =
			this->m_Leaf.searchUsedPageBehind(
				static_cast<const VersionPage&>(leafPage).operator const void*());
				//leafPage.operator const void*());
		if (pageIndex >= 0) {

			// 使用中の物理ページが見つかった
			lastPageID = pageID + pageIndex;

		} else if (pageID == 0) {

			// 先頭のリーフまで検索したが見つからなかった
			break;
		}
	}

	return lastPageID;
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::getPageDataSize --
//		物理ページデータサイズを返す
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Memory::Size		VersionPageSize_
//		[IN]		バージョンページサイズ [byte]
//	const PhysicalFile::AreaNum	DummyAreaNum_
//		[IN]		ダミーの物理エリア数
//					※ public 版の getPageDataSize() と
//					　 インタフェースを分けるために必要。
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページデータサイズ [byte]
//
//	EXCEPTIONS

// static
PageSize
PageManageFile2::getPageDataSize(const Os::Memory::Size	VersionPageSize_,
								 const AreaNum			DummyAreaNum_)
{
	// バージョンページに格納可能な内容のサイズを得て、返す
	Os::Memory::Size	verifyVersionPageSize =
		Version::File::verifyPageSize(VersionPageSize_);
	return Version::Page::getContentSize(verifyVersionPageSize);
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::getManageTableNum --
//		物理ページ以前に存在する管理表数を返す
//
//	NOTES
//	物理ファイル先頭から引数 PageID_ で示される物理ページまでに存在する
//	ノードとリーフの総数を返す。
//	※ 物理ファイルヘッダと同居しているノードも含む。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	PhysicalFile::PageNum
//		管理表数（ノードとリーフの総数）
//
//	EXCEPTIONS

PageNum
PageManageFile2::getManageTableNum(const PageID	PageID_) const
{
	return
		this->m_Node.getCount(PageID_) + this->m_Leaf.getCount(PageID_);
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::getManageTableVersionPageID --
//		管理表のバージョンページ識別子を返す
//
//	NOTES
//	※ PageManageFile2 ではここでの管理表とはリーフのことである。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	Version::Page::ID
//		リーフのバージョンページ識別子
//
//	EXCEPTIONS

Version::Page::ID
PageManageFile2::getManageTableVersionPageID(const PageID	PageID_) const
{
	return this->m_Leaf.getVersionPageID(PageID_);
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::updateTree -- 木構造を更新する
//
//	NOTES
//	引数 PageID_ で示される物理ページの使用状況を更新する。
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

void
PageManageFile2::updateTree(const Trans::Transaction&	Trans_,
							void*						FileHeader_,
							void*						Leaf_,
							const PageID				PageID_,
							const bool					IsUse_,
							const bool					AppendPage_,
							const bool					Discardable_)
{
	this->m_Leaf.update(Leaf_, PageID_, IsUse_, AppendPage_);

	// リーフが未使用物理ページを管理しているか？
	PageNum	unusePageNum = this->m_Leaf.getUnusePageNum(Leaf_);
	bool	bitON = (unusePageNum > 0);

	// ノードの更新が必要か？
	Version::Page::ID	nodeVPID = this->m_Node.getVersionPageID(PageID_);
	const void*	constNode;
	if (nodeVPID == FileHeader::VersionPageID && FileHeader_ != 0) {
		constNode = FileHeader_;
	} else {
		PagePointer	nodePage =
			fixVersionPage(Trans_,
						   nodeVPID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);
		constNode = static_cast<const VersionPage&>(*nodePage).operator const void*();
		//constNode = nodePage->operator const void*();
	}
	if (this->m_Node.getBit(constNode, PageID_) != bitON) {

		void*	node;
		if (nodeVPID == FileHeader::VersionPageID && FileHeader_ != 0) {
			node = FileHeader_;
		} else {
			Buffer::Page::FixMode::Value	fixMode =
				Discardable_ ?
				File::DiscardableWriteFixMode : Buffer::Page::FixMode::Write;

			PagePointer	nodePage =
				fixVersionPage(Trans_,
							   nodeVPID,
							   fixMode,
							   Buffer::ReplacementPriority::Middle);
			node = nodePage->operator void*();
		}

		// ノード更新
		this->m_Node.update(node, PageID_, bitON);
	}
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::clearNode -- ノードの全ビット OFF
//
//	NOTES
//
//	ARGUMENTS
//	void*	Node_
//		ノード
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::clearNode(void*	Node_) const
{
	this->m_Node.clear(Node_);
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::updateNode -- ノードのビット ON/OFF
//
//	NOTES
//	引数 PageID_ で示される物理ページを管理しているリーフに対応する
//	ノードのビットの ON/OFF を行う。
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

void
PageManageFile2::updateNode(const Trans::Transaction&	Trans_,
							void*						FileHeader_,
							const PageID				PageID_,
							const bool					BitON_,
							const bool					Discardable_)
{
	Version::Page::ID	nodeVPID = this->m_Node.getVersionPageID(PageID_);
	void*	node;
	if (nodeVPID == FileHeader::VersionPageID) {
		node = FileHeader_;
	} else {
		Buffer::Page::FixMode::Value	fixMode =
			Discardable_ ?
			File::DiscardableWriteFixMode : Buffer::Page::FixMode::Write;
		PagePointer	nodePage =
			fixVersionPage(Trans_,
						   nodeVPID,
						   fixMode,
						   Buffer::ReplacementPriority::Middle);
		node = nodePage->operator void*();
	}

	this->m_Node.update(node, PageID_, BitON_);
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::clearTree -- ノードとリーフをクリアする
//
//	NOTES
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

void
PageManageFile2::clearTree(const Trans::Transaction&	Trans_,
							 const PageNum				TotalPageNum_)
{
	PageID	lastPageID = TotalPageNum_ - 1;

	// ノード（物理ファイルヘッダと同居のものは除く）をクリア
	Version::Page::ID	lastNodeVPID =
		this->m_Node.getVersionPageID(lastPageID);
	Version::Page::ID	nodeVPID =
		this->m_Node.getVersionPageID(this->m_Tree.getPageNum());
	PageNum	versionPagePerTree = this->m_Tree.getVersionPageNum();
	for (; nodeVPID <= lastNodeVPID; nodeVPID += versionPagePerTree) {

		Version::Page::Memory	nodePage =
			Version::Page::fix(Trans_,
							   *this->m_VersionFile,
							   nodeVPID,
							   Buffer::Page::FixMode::Allocate,
							   Buffer::ReplacementPriority::Low);
		nodePage.unfix(true);
	}

	// リーフをクリア
	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	for (PageID pageID = 0; pageID <= lastPageID; pageID += pagePerLeaf) {

		Version::Page::ID	leafVPID =
			this->m_Leaf.getVersionPageID(pageID);
		Version::Page::Memory	leafPage =
			Version::Page::fix(Trans_,
							   *this->m_VersionFile,
							   leafVPID,
							   Buffer::Page::FixMode::Allocate,
							   Buffer::ReplacementPriority::Low);
		if (leafVPID == 1) {

			// 先頭のリーフは
			// ヘッダの未使用物理ページ数をインクリメントする
			this->m_Leaf.updateUnusePageNum(leafPage.operator void*(), 1);
		}
		leafPage.unfix(true);
	}
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::prepareAppendPage --
//		物理ページ追加のための準備を行う
//
//	NOTES
//	現在の物理ファイル末尾にひとつの物理ページを追加するための準備を行う。
//
//	ARGUMENTS
//	const Trans::Transaction&			Trans_
//		[IN]		トランザクション記述子
//	void*								FileHeader_
//		[IN]		物理ファイルヘッダ
//	const PhysicalFile::PageID			LastPageID_
//		[IN]		物理ファイルが管理している最終物理ページの識別子
//	const Buffer::Page::FixMode::Value	AllocateFixMode_
//		[IN]		バージョンページ確保時の fix モード
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::prepareAppendPage(
	const Trans::Transaction&			Trans_,
	void*								FileHeader_,
	const PageID						LastPageID_,
	const Buffer::Page::FixMode::Value	AllocateFixMode_)
{
	// 追加する物理ページ識別子を求める
	PageID	pageID = LastPageID_ + 1;

	// 追加する物理ページの直前のバージョンページがリーフかもしれない
	Version::Page::ID	leafVPID;
	if (this->m_Leaf.getVersionPageID(LastPageID_) !=
		(leafVPID = this->m_Leaf.getVersionPageID(pageID))) {

		// 追加する物理ページの直前のバージョンページがリーフ

		// ノードの追加も必要かもしれない
		Version::Page::ID	nodeVPID;
		if (this->m_Node.getVersionPageID(LastPageID_) !=
			(nodeVPID = this->m_Node.getVersionPageID(pageID))) {

			// ノードの追加も必要

			PagePointer	nodePage =
				fixVersionPage(Trans_,
							   nodeVPID,
							   AllocateFixMode_,
							   Buffer::ReplacementPriority::Middle);

			// ノードの更新は必要なし
		}

		// リーフ確保
		PagePointer	leafPage =
			fixVersionPage(Trans_,
						   leafVPID,
						   AllocateFixMode_,
						   Buffer::ReplacementPriority::Middle);
	}
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::prepareAppendPage --
//		物理ページ追加のための準備を行う
//
//	NOTES
//	現在の物理ファイル末尾に複数の物理ページを追加するための準備を行う。
//
//	ARGUMENTS
//	const Trans::Transaction&			Trans_
//		[IN]		トランザクション記述子
//	void*								FileHeader_
//		[IN]		物理ファイルヘッダ
//	const PhysicalFile::PageID			LastPageID_
//		[IN]		物理ファイルが管理している最終物理ページの識別子
//	const PhysicalFile::PageID			PageID_
//		[IN]		追加する物理ページの識別子
//	const Buffer::Page::FixMode::Value	AllocateFixMode_
//		[IN]		バージョンページ確保時の fix モード
//	const Buffer::Page::FixMode::Value	WriteFixMode_
//		[IN]		バージョンページ更新時の fix モード
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::prepareAppendPage(
	const Trans::Transaction&			Trans_,
	void*								FileHeader_,
	const PageID						LastPageID_,
	const PageID						PageID_,
	const Buffer::Page::FixMode::Value	AllocateFixMode_,
	const Buffer::Page::FixMode::Value	WriteFixMode_)
{
	// まずバージョンページの使い道は意識せず単に確保してしまう。
	Version::Page::ID	startVPID =
		this->convertToVersionPageID(LastPageID_) + 1;
	Version::Page::ID	stopVPID =
		this->convertToVersionPageID(PageID_) - 1;
	try {

		for (Version::Page::ID vpid = startVPID; vpid <= stopVPID; vpid++) {
			Version::Page::fix(Trans_,
							   *(this->m_VersionFile),
							   vpid,
							   AllocateFixMode_,
							   Buffer::ReplacementPriority::Low);
									// 管理表は稀なのでとりあえず Low で
		}

	} catch (Exception::Object&) {

		this->m_VersionFile->truncate(Trans_, startVPID);
		_SYDNEY_RETHROW;

#ifndef NO_CATCH_ALL
	} catch (...) {

		this->m_VersionFile->truncate(Trans_, startVPID);
		throw Exception::FileManipulateError(moduleName,
											 srcFile,
											 __LINE__);
#endif
	}


	void*	leaf = 0;
	Version::Page::ID	fixedLeafVPID =
		this->m_Leaf.getVersionPageID(LastPageID_ + 1);

	bool	discardable =
		((WriteFixMode_ & Buffer::Page::FixMode::Discardable) != 0);

	FileHeader::Item_Type1*	fileHeader =
		static_cast<FileHeader::Item_Type1*>(FileHeader_);

	for (PageID pageID = LastPageID_ + 1; pageID < PageID_; pageID++) {

		Version::Page::ID	leafVPID =
			this->m_Leaf.getVersionPageID(pageID);
		if (leaf == 0 || fixedLeafVPID != leafVPID) {

			PagePointer	leafPage =
				fixVersionPage(Trans_,
							   leafVPID,
							   WriteFixMode_,
							   Buffer::ReplacementPriority::Middle);
			leaf = leafPage->operator void*();
			fixedLeafVPID = leafVPID;
		}
		this->updateTree(Trans_,
						 FileHeader_,
						 leaf,
						 pageID,
						 false,	// unuse
						 true,		// append mode
						 discardable);

		(fileHeader->m_UnusePageNum)++;
	}

	Version::Page::ID	leafVPID;
	if (fixedLeafVPID !=
		(leafVPID = this->m_Leaf.getVersionPageID(PageID_))) {
		PagePointer	leafPage =
			fixVersionPage(Trans_,
						   leafVPID,
						   WriteFixMode_,
						   Buffer::ReplacementPriority::Middle);
		//leaf = leafPage->operator void*();
	}
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::createTopPage -- 先頭物理ページを確保する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	void*						FileHeader_
//		[IN]		物理ファイルヘッダ（先頭ノードが同居）
//	void*						TopLeaf_
//		[IN]		先頭リーフ
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::createTopPage(const Trans::Transaction&	Trans_,
							   void*						FileHeader_,
							   void*						TopLeaf_)
{
	// 先頭物理ページを確保
	PagePointer	page = fixVersionPage(Trans_,
									  this->convertToVersionPageID(0),
									  Buffer::Page::FixMode::Allocate,
									  Buffer::ReplacementPriority::Low);

	// 管理表を更新
	this->m_Leaf.updateUnusePageNum(TopLeaf_, 1);
	this->m_Node.update(FileHeader_, 0, true);

	// 物理ファイルヘッダを更新
	FileHeader::Item_Type1*	fileHeader =
		static_cast<FileHeader::Item_Type1*>(FileHeader_);
	(fileHeader->m_UnusePageNum)++;
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::convertToVersionPageID --
//		物理ページ識別子からバージョンページ識別子へ変換する
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	Version::Page::ID
//		バージョンページ識別子
//
//	EXCEPTIONS

Version::Page::ID
PageManageFile2::convertToVersionPageID(const PageID	PageID_) const
{
	return
		PageID_ +
		this->m_Node.getCount(PageID_) +
		this->m_Leaf.getCount(PageID_);
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::isUsedPage --
//		使用中の物理ページかどうかをチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const void*					Leaf_
//		[IN]		リーフ
//	const PhysicalFile::PageID	PageID_
//		[IN]		使用中かどうかチェックする物理ページの識別子
//
//	RETURN
//	bool
//		使用中の物理ページかどうか
//			true  : 引数 PageID_ が使用中の物理ページの識別子である
//			false : 引数 PageID_ が未使用の物理ページの識別子である
//
//	EXCEPTIONS

bool
PageManageFile2::isUsedPage(const void*		Leaf_,
							const PageID	PageID_) const
{
	return this->m_Leaf.getBit(Leaf_, PageID_);
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::searchNextAssignPage --
//		次に割り当てる物理ページを検索する
//
//	NOTES
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

PageID
PageManageFile2::searchNextAssignPage(const Trans::Transaction&	Trans_,
									  const void*				FileHeader_,
									  const PageNum				ManagePageNum_)
{
	PageID	lastPageID = ManagePageNum_ - 1;
	PageID	assignPageID = ConstValue::UndefinedPageID;

	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	PageNum	remainLeafNum = this->m_Leaf.getCount(lastPageID);

	PageNum	pagePerTree = this->m_Tree.getPageNum();
	PageNum	leafPerNode = this->m_Node.getManageLeafMax();

	// 木構造単位で検索

	for (PageID pageID = 0; pageID <= lastPageID; pageID += pagePerTree) {

		Version::Page::ID	nodeVPID =
			this->m_Node.getVersionPageID(pageID);
		const void*	node;
		if (nodeVPID == FileHeader::VersionPageID) {
			node = FileHeader_;
		} else {
			PagePointer	nodePage =
				fixVersionPage(Trans_,
							   nodeVPID,
							   Buffer::Page::FixMode::ReadOnly,
							   Buffer::ReplacementPriority::Middle);
			node = static_cast<const VersionPage&>(*nodePage).operator const void*();
			//node = nodePage->operator const void*();
		}

		// ノードから未使用物理ページを管理しているリーフを検索する
		int	leafIndex =
			this->m_Node.searchLeaf(
				node,
				(remainLeafNum > leafPerNode) ? 
					leafPerNode : remainLeafNum);

		if (leafIndex >= 0) {

			// 未使用物理ページを管理しているリーフが見つかった

			Version::Page::ID	leafVPID =
				nodeVPID + 1 + ((pagePerLeaf + 1) * leafIndex);
			PagePointer	leaf =
				fixVersionPage(Trans_,
							   leafVPID,
							   Buffer::Page::FixMode::ReadOnly,
							   Buffer::ReplacementPriority::Middle);

			// 未使用物理ページを検索する
			int	pageIndex =
				//this->m_Leaf.searchUnusePage(leaf->operator const void*());
				this->m_Leaf.searchUnusePage(static_cast<const VersionPage&>(*leaf).operator const void*());

			; _SYDNEY_ASSERT(pageIndex >= 0);

			// 物理ページ識別子を求める
			assignPageID = pageID + (pagePerLeaf * leafIndex) + pageIndex;
			break;
		}

		if (remainLeafNum > leafPerNode) {
			remainLeafNum -= leafPerNode;
		} else {
			remainLeafNum = 0;
		}
	}

	if (assignPageID == ConstValue::UndefinedPageID) {
		assignPageID = lastPageID + 1;
	}

	return assignPageID;
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::checkPhysicalFile --
//		物理ページ管理機能つき物理ファイル PageManageFile2 の整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&		Trans_
//		[IN]		トランザクション記述子
//	Admin::Verification::Progress&	Progress_
//		[IN/OUT]	整合性検査の途中経過
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::checkPhysicalFile(const Trans::Transaction&		Trans_,
								   Admin::Verification::Progress&	Progress_)
{
	AutoUnfix	unfix(this);
	unfix.success();

	// 1. 管理物理ページ総数一致検査
	if (this->checkManagePageNumInFile(Trans_, Progress_) == false) return;

	// 2. 使用中物理ページ総数一致検査
	if (this->checkUsedPageNumInFile(Trans_, Progress_) == false) return;

	// 3. ノードビットマップ検査
	if (this->checkNodeBitmap(Trans_, Progress_) == false) return;

	// ※ リーフのビットマップは利用者から通知された使用中の物理ページの
	// 　 チェックの際に済んでいるのでここでは不要
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::checkManagePageNumInFile --
//		管理物理ページ総数一致検査
//
//	NOTES
//	物理ファイルヘッダに記録されている
//		A. 使用中の物理ページ数
//		B. 未使用の物理ページ数
//	と、すべてのリーフのヘッダに記録されている
//		C. 使用中の物理ページ数
//		D. 未使用の物理ページ数
//	の総数が一致しているかどうかを確認する。
//	( A + B ) != ( C の総数 + D の総数 ) で不整合。
//
//	ARGUMENTS
//	const Trans::Transaction&		Trans_
//		[IN]		トランザクション記述子
//	Admin::Verification::Progress&	Progress_
//		[IN/OUT]	整合性検査の途中経過
//
//	RETURN
//	bool
//		Progress_.isGood() の戻り値
//
//	EXCEPTIONS

bool
PageManageFile2::checkManagePageNumInFile(
	const Trans::Transaction&		Trans_,
	Admin::Verification::Progress&	Progress_)
{
	PageNum	managePageNum = 0;

	PageID	pageID = 0;
	Version::Page::ID	leafVPID = this->m_Leaf.getVersionPageID(pageID);
	Version::Page::ID	lastLeafVPID =
		this->m_Leaf.getVersionPageID(this->m_LastManagePageID);
	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	while (leafVPID <= lastLeafVPID) {

		PagePointer	leafPage =
			fixVersionPage(Trans_,
						   leafVPID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Low);
		const Leaf::Header*	leafHeader =
			static_cast<const Leaf::Header*>(
				static_cast<const VersionPage&>(*leafPage).operator const void*());
				//leafPage->operator const void*());

		managePageNum +=
			(leafHeader->m_UsedPageNum + leafHeader->m_UnusePageNum);

		pageID += pagePerLeaf;
		leafVPID = this->m_Leaf.getVersionPageID(pageID);
	}

	if (this->m_ManagePageNum != managePageNum) {

		// 不整合

		_SYDNEY_VERIFY_INCONSISTENT(
			Progress_,
			this->m_FilePath,
			Message::DiscordManagePageNum(this->m_ManagePageNum,
										  managePageNum));
	}

	return Progress_.isGood();
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::checkUsedPageNumInFile --
//		使用中物理ページ総数一致検査
//
//	NOTES
//	物理ファイルヘッダに記録されている
//		A. 使用中の物理ページ数
//	と、すべてのリーフのヘッダに記録されている
//		B. 使用中の物理ページ数
//	の総数が一致しているかどうかを確認する。
//	A != B の総数 で不整合。
//
//	ARGUMENTS
//	const Trans::Transaction&		Trans_
//		[IN]		トランザクション記述子
//	Admin::Verification::Progress&	Progress_
//		[IN/OUT]	整合性検査の途中経過
//
//	RETURN
//	bool
//		Progress_.isGood() の戻り値
//
//	EXCEPTIONS

bool
PageManageFile2::checkUsedPageNumInFile(
	const Trans::Transaction&		Trans_,
	Admin::Verification::Progress&	Progress_)
{
	PageNum	usedPageNum = 0;

	PageID	pageID = 0;
	Version::Page::ID	leafVPID = this->m_Leaf.getVersionPageID(pageID);
	Version::Page::ID	lastLeafVPID =
		this->m_Leaf.getVersionPageID(this->m_LastManagePageID);
	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	while (leafVPID <= lastLeafVPID) {

		PagePointer	leafPage =
			fixVersionPage(Trans_,
						   leafVPID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Low);
		const Leaf::Header*	leafHeader =
			static_cast<const Leaf::Header*>(
				static_cast<const VersionPage&>(*leafPage).operator const void*());
				//leafPage->operator const void*());

		usedPageNum += leafHeader->m_UsedPageNum;

		pageID += pagePerLeaf;
		leafVPID = this->m_Leaf.getVersionPageID(pageID);
	}

	PagePointer	fileHeaderPage =
		fixVersionPage(Trans_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Low);
	const FileHeader::Item_Type1*	fileHeader =
		static_cast<const FileHeader::Item_Type1*>(
			static_cast<const VersionPage&>(*fileHeaderPage).operator const void*());
			//fileHeaderPage->operator const void*());

	if (fileHeader->m_UsedPageNum != usedPageNum) {

		// 不整合

		_SYDNEY_VERIFY_INCONSISTENT(
			Progress_,
			this->m_FilePath,
			Message::DiscordUsePageNum(fileHeader->m_UsedPageNum,
									   usedPageNum));
	}

	return Progress_.isGood();
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::checkNodeBitmap --
//		ノードビットマップ検査
//
//	NOTES
//	リーフが未使用の物理ページを管理している場合には
//	対応するノードのビットが ON であり、未使用の物理ページを
//	管理していない場合には対応するノードのビットが OFF であることを
//	確認する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Trans_
//		[IN]		トランザクション記述子
//	Admin::Verification::Progress&	Progress_
//		[IN/OUT]	整合性検査の途中経過
//
//	RETURN
//	bool
//		Progress_.isGood() の戻り値
//
//	EXCEPTIONS

bool
PageManageFile2::checkNodeBitmap(const Trans::Transaction&		Trans_,
								 Admin::Verification::Progress&	Progress_)
{
	Version::Page::ID	nodeVPID = this->m_Node.getVersionPageID(0);
	Version::Page::ID	lastNodeVPID =
		this->m_Node.getVersionPageID(this->m_LastManagePageID);
	PageNum	leafPerNode = this->m_Node.getManageLeafMax();
	PageNum	versionPagePerTree = this->m_Tree.getVersionPageNum();

	Version::Page::ID	leafVPID = this->m_Leaf.getVersionPageID(0);
	Version::Page::ID	lastLeafVPID =
		this->m_Leaf.getVersionPageID(this->m_LastManagePageID);
	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();

	PageID	pageID = 0;

	while (nodeVPID <= lastNodeVPID) {

		PagePointer	nodePage =
			fixVersionPage(Trans_,
						   nodeVPID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Low);
		const void*	node =
			static_cast<const void*>(static_cast<const VersionPage&>(*nodePage).operator const void*());
			//static_cast<const void*>(nodePage->operator const void*());

		for (PageNum leafIndex = 0;
			 leafIndex < leafPerNode && leafVPID <= lastLeafVPID;
			 leafIndex++) {

			PagePointer	leafPage =
				fixVersionPage(Trans_,
							   leafVPID,
							   Buffer::Page::FixMode::ReadOnly,
							   Buffer::ReplacementPriority::Low);
			const Leaf::Header*	leafHeader =
				static_cast<const Leaf::Header*>(
					static_cast<const VersionPage&>(*leafPage).operator const void*());
					//leafPage->operator const void*());

			bool	manageUnusePageLeaf =
				this->m_Node.getBit(node, pageID);
			if (manageUnusePageLeaf != (leafHeader->m_UnusePageNum > 0)) {

				// 不整合

				_SYDNEY_VERIFY_INCONSISTENT(
					Progress_,
					this->m_FilePath,
					Message::DiscordUnusePageNumInTable(
						manageUnusePageLeaf ? 1 : 0,
						leafHeader->m_UnusePageNum));

				return Progress_.isGood();
			}

			pageID += pagePerLeaf;
			leafVPID = this->m_Leaf.getVersionPageID(pageID);
		}

		nodeVPID += versionPagePerTree;
	}

	return Progress_.isGood();
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::verifyAllTable --
//		整合性検査のために管理表を fix する
//
//	NOTES
//	すべてのノードとリーフを fix する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Trans_
//		[IN]		トランザクション記述子
//	Admin::Verification::Progress&	Progress_
//		[IN/OUT]	整合性検査の途中経過
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::verifyAllTable(const Trans::Transaction&		Trans_,
								Admin::Verification::Progress&	Progress_)
{
	; _SYDNEY_ASSERT(Progress_.isGood());

	// ノード

	Version::Page::ID	nodeVPID = this->m_Node.getVersionPageID(0);
	Version::Page::ID	lastNodeVPID =
		this->m_Node.getVersionPageID(this->m_LastManagePageID);
	PageNum	versionPagePerTree = this->m_Tree.getVersionPageNum();

	while (nodeVPID <= lastNodeVPID && Progress_.isGood()) {

		try {

			// ノードを fix
			Version::Page::Memory	node =
				File::fixVersionPage(Trans_,
									 this,
									 nodeVPID,
									 Buffer::Page::FixMode::ReadOnly,
									 Progress_);
			node.unfix(false);

#ifdef NO_CATCH_ALL
		} catch (Exception::Object&) {
#else
		} catch (...) {
#endif
			// ノードを fix できなかった

			_SYDNEY_VERIFY_ABORTED(Progress_,
								   this->m_FilePath,
								   Message::CanNotFixPageTable());

			_SYDNEY_RETHROW;
		}

		nodeVPID += versionPagePerTree;
	}

	// リーフ

	PageID				pageID = 0;
	Version::Page::ID	leafVPID = this->m_Leaf.getVersionPageID(pageID);
	Version::Page::ID	lastLeafVPID =
		this->m_Leaf.getVersionPageID(this->m_LastManagePageID);
	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	while (leafVPID <= lastLeafVPID && Progress_.isGood()) {

		try {

			// リーフを fix
			Version::Page::Memory	leaf =
				File::fixVersionPage(Trans_,
									 this,
									 leafVPID,
									 Buffer::Page::FixMode::ReadOnly,
									 Progress_);
			leaf.unfix(false);

#ifdef NO_CATCH_ALL
		} catch (Exception::Object&) {
#else
		} catch (...) {
#endif
			// リーフを fix できなかった

			_SYDNEY_VERIFY_ABORTED(Progress_,
								   this->m_FilePath,
								   Message::CanNotFixPageTable());

			_SYDNEY_RETHROW;
		}

		pageID += pagePerLeaf;
		leafVPID = this->m_Leaf.getVersionPageID(pageID);
	}
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::correspondUsePage --
//		利用者と自身の物理ページの使用状況が一致するかどうかをチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&		Trans_
//		[IN]		トランザクション記述子
//	Admin::Verification::Progress&	Progress_
//		[IN/OUT]	整合性検査の途中経過
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::correspondUsePage(const Trans::Transaction&		Trans_,
								   Admin::Verification::Progress&	Progress_)
{
	bool	correct =
		((this->m_Treatment & Admin::Verification::Treatment::Correct) != 0);
	Version::Page::ID	leafVPID = this->m_Leaf.getVersionPageID(0);
	Version::Page::ID	lastLeafVPID =
		this->m_Leaf.getVersionPageID(this->m_LastManagePageID);

	PageID	pageID = 0;

	while (leafVPID <= lastLeafVPID) {

		AutoUnfix	unfix(this);

		Admin::Verification::Progress
			tableProgress(Progress_.getConnection());

		Buffer::Page::FixMode::Value	fixMode =
			(this->isCorrect() ?
			 File::DiscardableWriteFixMode : Buffer::Page::FixMode::ReadOnly);

		PagePointer	leafPage =
			fixVersionPage(Trans_,
						   leafVPID,
						   fixMode,
						   Buffer::ReplacementPriority::Low);

		void*	leaf = 0;
		if (fixMode == Buffer::Page::FixMode::ReadOnly) {
			const Version::Page::Memory&	leafMemory = *leafPage;
			leaf = const_cast<void*>(static_cast<const Version::Page::Memory&>(leafMemory).operator const void*());
			//leaf = const_cast<void*>(leafMemory.operator const void*());
		} else {
			leaf = leafPage->operator void*();
		}

		while (this->m_Leaf.getVersionPageID(pageID) == leafVPID) {

			bool	used = this->m_Leaf.getBit(leaf, pageID);

			if (used != this->m_PageIDs[pageID]) {

				// 不整合

				Admin::Verification::Progress
					pageProgress(Progress_.getConnection());

				if (used) {

					// 物理ファイルマネージャ：使用中と認識
					// 利用者　　　　　　　　：未使用と認識

					// 修復可能

					if (correct) {

						// 修復
						this->correctUsePage(Trans_,
											 pageID,
											 leaf,
											 pageProgress);
					} else {

						_SYDNEY_VERIFY_INCONSISTENT(
							pageProgress,
							this->m_FilePath,
							Message::DiscordPageUseSituation2(pageID));
					}

				} else {

					// 物理ファイルマネージャ：未使用と認識
					// 利用者　　　　　　　　：使用中と認識

					// 修復不可能

					_SYDNEY_VERIFY_INCONSISTENT(
						pageProgress,
						this->m_FilePath,
						Message::DiscordPageUseSituation3(pageID));
				}

				tableProgress += pageProgress;

				if (tableProgress.isGood() == false) break;
			}

			pageID++;

		} // end while (this->m_Leaf.getVersionPageID(pageID) == leafVPID)

		Admin::Verification::Status::Value	tableProgressStatus =
			tableProgress.getStatus();
		if (tableProgressStatus == Admin::Verification::Status::Corrected)
			unfix.success();

		if (tableProgressStatus != Admin::Verification::Status::Consistent)
			Progress_ += tableProgress;

		if (Progress_.isGood() == false) break;

		leafVPID = this->m_Leaf.getVersionPageID(pageID);

	} // end while (leafVPID <= lastLeafVPID)
}

//	FUNCTION private
//	PhysicalFile::PageManageFile2::correctUsePage --
//		物理ページの使用状況を修復する
//
//	NOTES
//	物理ファイルマネージャでは使用中と認識していた物理ページを
//	未使用状態に修復する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Trans_
//		[IN]		トランザクション記述子
//	const PhysicalFile::PageID		PageID_
//		[IN]		使用状況を確認する物理ページの識別子
//	void*							Leaf_
//		[IN]		リーフ
//	Admin::Verification::Progress&	Progress_
//		[IN/OUT]	整合性検査の途中経過
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::correctUsePage(const Trans::Transaction&		Trans_,
								const PageID					PageID_,
								void*							Leaf_,
								Admin::Verification::Progress&	Progress_)
{
	bool	corrected = false;

	try {

		PagePointer	fileHeaderPage =
			fixVersionPage(Trans_,
						   FileHeader::VersionPageID,
						   File::DiscardableWriteFixMode,
						   Buffer::ReplacementPriority::Low);

		FileHeader::Item_Type1*	fileHeader =
			static_cast<FileHeader::Item_Type1*>(
				fileHeaderPage->operator void*());

		this->updateTree(Trans_,
						 fileHeader,
						 Leaf_,
						 PageID_,
						 false,	// unuse
						 false,	// not append page
						 true);	// discardable

		(fileHeader->m_UsedPageNum)--;
		(fileHeader->m_UnusePageNum)++;

		corrected = true;

#ifdef NO_CATCH_ALL
	} catch (Exception::Object&) {
#else
	} catch (...) {
#endif
		// 修復できず

		_SYDNEY_VERIFY_INCONSISTENT(
			Progress_,
			this->m_FilePath,
			Message::CanNotCorrectPageUseSituation(PageID_));
	}

	if (corrected) {

		// 修復できた

		_SYDNEY_VERIFY_CORRECTED(
			Progress_,
			this->m_FilePath,
			Message::CorrectedPageUseSituation(PageID_));
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFile2::Tree クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Tree::Tree -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

PageManageFile2::Tree::Tree()
	: m_PageNum(0),
	  m_VersionPageNum(0)
{
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Tree::~Tree -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

PageManageFile2::Tree::~Tree()
{
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Tree::setPageNum --
//		ひとつの木構造あたりのページ数を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::PageNum	LeafPerNode_
//		[IN]		ノードで管理可能なリーフ数
//	const PhysicalFile::PageNum	PagePerLeaf_
//		[IN]		リーフで管理可能な物理ページ数
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::Tree::setPageNum(const PageNum	LeafPerNode_,
								  const PageNum	PagePerLeaf_)
{
	this->m_PageNum = LeafPerNode_ * PagePerLeaf_;
	this->m_VersionPageNum = 1 + LeafPerNode_ + this->m_PageNum;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFile2::Node クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Node::Node -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::PageManageFile2::Tree&	Tree_
//		[IN]		木構造オブジェクト
//	const PhysicalFile::PageManageFile2::Leaf&	Leaf_
//		[IN]		リーフオブジェクト
//
//	RETURN
//
//	EXCEPTIONS

PageManageFile2::Node::Node(const Tree&	Tree_,
							const Leaf&	Leaf_)
	: m_BitmapOffset(0),
	  m_BitmapSize(0),
	  m_Tree(Tree_),
	  m_Leaf(Leaf_),
	  m_ManageLeafMax(0)
{
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Node::~Node -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

PageManageFile2::Node::~Node()
{
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Node::setBitmapSize --
//		ビットマップサイズを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Memory::Size	VersionPageDataSize_
//		[IN]		バージョンページデータサイズ [byte]
//	const Os::Memory::Size	FileHeaderSize_
//		[IN]		物理ファイルヘッダサイズ [byte]
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::Node::setBitmapSize(
	const Os::Memory::Size	VersionPageDataSize_,
	const Os::Memory::Size	FileHeaderSize_)
{
	this->m_BitmapOffset = FileHeaderSize_;
	this->m_BitmapSize = VersionPageDataSize_ - FileHeaderSize_;
	this->m_ManageLeafMax = this->m_BitmapSize * 8;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Node::update -- ビット ON/OFF
//
//	NOTES
//	ノードのビットマップの各ビットについて…
//		ビット ON :
//			ビットに対応するリーフで未使用物理ページを管理している
//		ビット OFF :
//			ビットに対応するリーフで未使用物理ページを管理していない
//
//	ARGUMENTS
//	void*						Node_
//		[IN]		ノード
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//	const bool					BitON_
//		[IN]		ビットを ON とするか OFF とするか
//						true  : ビット ON
//						false : ビット OFF
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::Node::update(void*			Node_,
							  const PageID	PageID_,
							  const bool	BitON_) const
{
	unsigned char*	bitmap8 =
		static_cast<unsigned char*>(Node_) + this->m_BitmapOffset;

	// ノード内でのリーフのインデックスを求める
	PageNum	leafNum = this->m_Leaf.getCount(PageID_);
	int	leafIndex = (leafNum - 1) % this->m_ManageLeafMax;

	bitmap8 += (leafIndex >> 3);
	//          ~~~~~~~~~~~~~~ == leafIndex / 8

	unsigned char	mask = 1 << (leafIndex % 8);

	if (BitON_) {
		*bitmap8 |= mask;
	} else {
		*bitmap8 &= ~mask;
	}
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Node::clear -- 全ビット OFF
//
//	NOTES
//
//	ARGUMENTS
//	void*	Node_
//		[IN]		ノード
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::Node::clear(void*	Node_) const
{
	unsigned char*	bitmap8 =
		static_cast<unsigned char*>(Node_) + this->m_BitmapOffset;
	unsigned int*	bitmap32 = syd_reinterpret_cast<unsigned int*>(bitmap8);

	for (PageNum i = 0; i < this->m_ManageLeafMax; i += 32, bitmap32++) {
		*bitmap32 = 0;
	}

	PageNum	remainLeafNum = this->m_ManageLeafMax & 0x1F;
	int	remainBytes = remainLeafNum >> 3;
	if (remainBytes > 0) {
		bitmap8 = syd_reinterpret_cast<unsigned char*>(bitmap32);
		for (int i = 0; i < remainBytes; i++) {
			*bitmap8 = 0;
		}
	}
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Node::clear -- ビット OFF
//
//	NOTES
//	引数 PageID_ から LastPageID_ までの物理ページを管理しているリーフに
//	対応するノードの各ビットを OFF にする。
//	ただし、更新するノードは引数 Node_ が指すものだけである。
//	また、引数 PageID_ が示す物理ページを管理しているリーフが
//	未使用物理ページを管理しているのであれば、そのリーフに対応する
//	ビットは OFF としない。
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	void*						Node_
//		[IN]		引数 PageID_ で示される物理ページを
//					管理しているリーフに対応するビットが含まれるノード
//	const PhysicalFile::PageID	PageID_
//		[IN]		開始物理ページ識別子
//	const PhysicalFile::PageID	LastPageID_
//		[IN]		終了物理ページ識別子
//	const PhysicalFile::PageNum	LeafUnusePageNum_
//		[IN]		引数 PageID_ で示される物理ページを管理している
//					リーフのヘッダに記録されている未使用物理ページ数
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::Node::clear(
	const Trans::Transaction&	Trans_,
	void*						Node_,
	const PageID				PageID_,
	const PageID				LastPageID_,
	const PageNum				LeafUnusePageNum_) const
{
	PageNum	pagePerLeaf = this->m_Leaf.getManagePageMax();
	Version::Page::ID	nodeVPID = this->getVersionPageID(PageID_);
	Version::Page::ID	lastNodeVPID = this->getVersionPageID(LastPageID_);
	if (nodeVPID == lastNodeVPID) {

		// ノードのビットマップの中間部分、または、
		// ぴったり最後のビットまでをクリア

		for (PageID	pageID = PageID_;
			 pageID <= LastPageID_;
			 pageID += pagePerLeaf) {

			// 引数 PageID_ で示される物理ページを管理しているリーフが
			// 未使用物理ページを管理しているのであれば、
			// そのリーフに対応するビットは OFF としない
			if (pageID == PageID_ && LeafUnusePageNum_ > 0) continue;

			this->update(Node_, pageID, false);
			//                            ~~~~~ bit OFF
		}

	} else {

		// ノードのビットマップの特定ビット以降すべてをクリア

		for (PageID pageID = PageID_;
			 nodeVPID == this->getVersionPageID(pageID);
			 pageID += pagePerLeaf) {

			// 同上
			if (pageID == PageID_ && LeafUnusePageNum_ > 0) continue;

			this->update(Node_, pageID, false);
			//                            ~~~~~ bit OFF
		}
	}
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Node::getBit -- ビット参照
//
//	NOTES
//
//	ARGUMENTS
//	const void*					Node_
//		[IN]		ノード
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	bool
//		true  : ビット ON
//		false : ビット OFF
//
//	EXCEPTIONS

bool
PageManageFile2::Node::getBit(const void*	Node_,
							  const PageID	PageID_) const
{
	const unsigned char*	bitmap8 =
		static_cast<const unsigned char*>(Node_) + this->m_BitmapOffset;

	// ノード内でのリーフのインデックスを求める
	PageNum	leafNum = this->m_Leaf.getCount(PageID_);
	int	leafIndex = (leafNum - 1) % this->m_ManageLeafMax;

	bitmap8 += (leafIndex >> 3);

	unsigned char	mask = 1 << (leafIndex % 8);

	return ((*bitmap8 & mask) != 0);
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Node::searchLeaf --
//		未使用物理ページを管理しているリーフを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const void*					Node_
//		[IN]		ノード
//	const PhysicalFile::PageNum	ManageLeafNum_
//		[IN]		引数 Node_ が指すノードが管理しているリーフ数
//
//	RETURN
//	int
//		未使用物理ページを管理しているリーフのインデックス
//		（引数 Node_ が指すノード内でのインデックス）
//
//	EXCEPTIONS

int
PageManageFile2::Node::searchLeaf(const void*	Node_,
								  const PageNum	ManageLeafNum_) const
{
	int	leafIndex = -1;

	const unsigned char*	bitmap8 =
		static_cast<const unsigned char*>(Node_) + this->m_BitmapOffset;

	// まずはビットマップを 32 ビットずつ舐める
	if (ManageLeafNum_ >= 32) {

		const unsigned int*	bitmap32 =
			syd_reinterpret_cast<const unsigned int*>(bitmap8);
		PageNum	manageLeafNum = ManageLeafNum_ & 0xFFFFFFE0;
		for (PageNum i = 0; i < manageLeafNum; i += 32, bitmap32++) {

			if (*bitmap32 != 0) {

				bitmap8 = syd_reinterpret_cast<const unsigned char*>(bitmap32);
				for (int j = 0; j < 4; j++, bitmap8++) {

					if (*bitmap8 != 0) {

						unsigned char	mask = 1;
						for (int k = 0; k < 8; k++, mask <<= 1) {

							if ((*bitmap8 & mask) != 0) {

								// ビット ON →
								// 未使用物理ページを管理しているリーフ

								leafIndex = i + (j << 3) + k;
								//               ~~~~~~ == j * 8
								break;
							}
						}

						; _SYDNEY_ASSERT(leafIndex >= 0);
						break;
					}

				} // end for j

				; _SYDNEY_ASSERT(leafIndex >= 0);
				break;
			}

		} // end for i

		if (leafIndex < 0) {
			bitmap8 = syd_reinterpret_cast<const unsigned char*>(bitmap32);
		}
	}

	// 32 ビットずつ舐めて見つからなかったら残りの 1 〜 3 バイトを舐める
	if (leafIndex < 0) {

		PageNum	remainLeafNum = ManageLeafNum_ & 0x1F;
		int	remainBytes = remainLeafNum >> 3;
		if ((remainLeafNum & 0x1F) != 0) remainBytes++;
		for (int i = 0; i < remainBytes; i++, bitmap8++) {

			if (*bitmap8 != 0) {

				unsigned char	mask = 1;
				for (int j = 0; j < 8; j++, mask <<= 1) {

					if ((*bitmap8 & mask) != 0) {

						// ビット ON →
						// 未使用物理ページを管理しているリーフ

						leafIndex =
							(ManageLeafNum_ & 0xFFFFFFE0) + (i << 3) + j;
						//   ~~~~~~~~~~~~~~~~~~~~~~~~~~~     ~~~~~~ == i * 8
						//   == ManageLeafNum_ / 32 * 32
						break;
					}
				}

				; _SYDNEY_ASSERT(leafIndex >= 0);
				break;
			}
		}
	}

	return leafIndex;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Node::getCount --
//		物理ページ以前に存在するノード数を返す
//
//	NOTES
//	物理ファイル先頭から、引数 PageID_ で示される物理ページまでの間に存在する
//	ノード数を返す。
//	物理ファイルヘッダと同居しているノードも含む。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	PhysicalFile::PageNum
//		物理ページ以前に存在するノード数
//
//	EXCEPTIONS

PageNum
PageManageFile2::Node::getCount(const PageID	PageID_) const
{
	return PageID_ / this->m_Tree.getPageNum() + 1;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Node::getVersionPageID --
//		ノードのバージョンページ識別子を返す
//
//	NOTES
//	引数 PageID_ で示される物理ページを（間接的に）管理しているノードの
//	バージョンページ識別子を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	Version::Page::ID
//		ノードのバージョンページ識別子
//
//	EXCEPTIONS

Version::Page::ID
PageManageFile2::Node::getVersionPageID(const PageID	PageID_) const
{
	PageNum	nodeNum = this->getCount(PageID_);
	return this->m_Tree.getVersionPageNum() * (nodeNum - 1);
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFile2::Leaf クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::Leaf -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::PageManageFile2::Tree&	Tree_
//		[IN]		木構造オブジェクト
//	const PhysicalFile::PageManageFile2::Node&	Node_
//		[IN]		ノードオブジェクト
//
//	RETURN
//
//	EXCEPTIONS

PageManageFile2::Leaf::Leaf(const Tree&	Tree_,
							const Node&	Node_)
	: m_BitmapOffset(0),
	  m_BitmapSize(0),
	  m_Tree(Tree_),
	  m_Node(Node_),
	  m_ManagePageMax(0)
{
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::~Leaf -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

PageManageFile2::Leaf::~Leaf()
{
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::setBitmapSize --
//		ビットマップサイズを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Memory::Size	VersionPageDataSize_
//		[IN]		バージョンページデータサイズ [byte]
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::Leaf::setBitmapSize(
	const Os::Memory::Size	VersionPageDataSize_)
{
	this->m_BitmapOffset = sizeof(Header);
	this->m_BitmapSize = VersionPageDataSize_ - this->m_BitmapOffset;
	this->m_ManagePageMax = this->m_BitmapSize * 8;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::update -- ビット ON/OFF
//
//	NOTES
//	リーフのビットマップの各ビットについて…
//		ビット ON :
//			ビットに対応する物理ページは使用中
//		ビット OFF :
//			ビットに対応する物理ページは未使用
//
//	※ リーフではヘッダの更新も行う。
//
//	ARGUMENTS
//	void*						Leaf_
//		[IN]		リーフ
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//	const bool					BitON_
//		[IN]		ビットを ON とするか OFF とするか
//						true  : ビット ON
//						false : ビット OFF
//	const bool					AppendPage_
//		[IN]		物理ページ追加に伴う管理表の更新かどうか
//						true  : 物理ページ追加に伴う管理表の更新
//						false : 物理ページの使用状況の変更に伴う管理表の更新
//
//	RETURN
//
//	EXCEPTIONS

void
PageManageFile2::Leaf::update(void*			Leaf_,
							  const PageID	PageID_,
							  const bool	BitON_,
							  const bool	AppendPage_) const
{
	unsigned char*	bitmap8 =
		static_cast<unsigned char*>(Leaf_) + this->m_BitmapOffset;

	// リーフ内での物理ページのインデックスを求める
	int	pageIndex = PageID_ % this->m_ManagePageMax;

	bitmap8 += (pageIndex >> 3);
	//          ~~~~~~~~~~~~~~ == pageIndex / 8

	unsigned char	mask = 1 << (pageIndex % 8);

	if (BitON_) {
		*bitmap8 |= mask;
	} else {
		*bitmap8 &= ~mask;
	}

	// ヘッダの更新

	Header*	header = static_cast<Header*>(Leaf_);
	if (BitON_) {
		(header->m_UsedPageNum)++;
		if (AppendPage_ == false) {
			; _SYDNEY_ASSERT(header->m_UnusePageNum > 0);
			(header->m_UnusePageNum)--;
		}
	} else {
		if (AppendPage_ == false) {
			; _SYDNEY_ASSERT(header->m_UsedPageNum > 0);
			(header->m_UsedPageNum)--;
		}
		(header->m_UnusePageNum)++;
	}
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::getBit -- ビット参照
//
//	NOTES
//
//	ARGUMENTS
//	const void*					Leaf_
//		[IN]		リーフ
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	bool
//		true  : ビット ON
//		false : ビット OFF
//
//	EXCEPTIONS

bool
PageManageFile2::Leaf::getBit(const void*	Leaf_,
							  const PageID	PageID_) const
{
	const unsigned char*	bitmap8 =
		static_cast<const unsigned char*>(Leaf_) + this->m_BitmapOffset;

	// リーフ内での物理ページのインデックスを求める
	int	pageIndex = PageID_ % this->m_ManagePageMax;

	bitmap8 += (pageIndex >> 3);

	unsigned char	mask = 1 << (pageIndex % 8);

	return ((*bitmap8 & mask) != 0);
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::searchUsedPage --
//		リーフから使用中の物理ページを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const void*	Leaf_
//		[IN]		リーフ
//	const int	StartIndex_
//		[IN]		検索開始ビットインデックス
//
//	RETURN
//	int
//		使用中の物理ページのインデックス
//		（引数 Leaf_ が指すリーフ内でのインデックス）
//
//	EXCEPTIONS

int
PageManageFile2::Leaf::searchUsedPage(
	const void*	Leaf_,
	const int	StartIndex_/* = -1 */) const
{
	const Header*	header = static_cast<const Header*>(Leaf_);
	if (header->m_UsedPageNum == 0) return -1;

	int	usedPageIndex = -1;

	PageNum	managePageNum = header->m_UsedPageNum + header->m_UnusePageNum;

	const unsigned char*	bitmap8 =
		static_cast<const unsigned char*>(Leaf_) + this->m_BitmapOffset;

	// まずはビットマップを 32 ビットずつ舐める
	if (managePageNum >= 32) {

		const unsigned int*	bitmap32 =
			syd_reinterpret_cast<const unsigned int*>(bitmap8);
		PageNum	managePageNum32 = managePageNum & 0xFFFFFFE0;
		for (PageNum i = 0; i < managePageNum32; i += 32, bitmap32++) {

			if (StartIndex_ >= 0 && static_cast<int>(i + 32) < StartIndex_) {
				continue;
			}

			if (*bitmap32 != 0) {

				bitmap8 = syd_reinterpret_cast<const unsigned char*>(bitmap32);
				for (int j = 0; j < 4; j++, bitmap8++) {

					if (*bitmap8 != 0) {

						unsigned char	mask = 1;
						for (int k = 0; k < 8; k++, mask <<= 1) {

							if ((*bitmap8 & mask) != 0) {

								// ビット ON → 使用中の物理ページ

								usedPageIndex = i + (j << 3) + k;
								if (usedPageIndex > StartIndex_) break;
								usedPageIndex = -1;
							}
						}

						if (usedPageIndex >= 0) break;
					}

				} // end for j

				if (usedPageIndex >= 0) break;
			}

		} // end for i

		if (usedPageIndex < 0) {
			bitmap8 = syd_reinterpret_cast<const unsigned char*>(bitmap32);
		}
	}

	// 32 ビットずつ舐めて見つからなかったら残りの 1 〜 3 バイトを舐める
	if (usedPageIndex < 0) {

		PageNum	remainPageNum = managePageNum & 0x1F;
		//		                ~~~~~~~~~~~~~~~~~~~~ == managePageNum % 32
		int	remainBytes = remainPageNum >> 3;
		//	              ~~~~~~~~~~~~~~~~~~ == remainPageNum / 8
		if ((remainPageNum & 0x1F) != 0) remainBytes++;
		for (int i = 0; i < remainBytes; i++, bitmap8++) {

			if (*bitmap8 != 0) {

				unsigned char	mask = 1;
				for (int j = 0; j < 8; j++, mask <<= 1) {

					if ((*bitmap8 & mask) != 0) {

						// ビット ON → 使用中物理ページ

						usedPageIndex =
							(managePageNum & 0xFFFFFFE0) + (i << 3) + j;
						if (usedPageIndex > StartIndex_) break;
						usedPageIndex = -1;
					}
				}

				if (usedPageIndex >= 0) break;
			}
		}
	}

	return usedPageIndex;
}

int
PageManageFile2::Leaf::searchUsedPageBehind(
	const void*	Leaf_,
	const int	StartIndex_/* = -1 */) const
{
	const Header*	header = static_cast<const Header*>(Leaf_);
	if (header->m_UsedPageNum == 0 || StartIndex_ == 0) return -1;

	int	usedPageIndex = -1;

	PageNum	managePageNum = header->m_UsedPageNum + header->m_UnusePageNum;

	int	startIndex = StartIndex_;
	if (startIndex >= static_cast<int>(managePageNum))
		startIndex = managePageNum - 1;

	const unsigned char*	bitmap8 =
		static_cast<const unsigned char*>(Leaf_) +
		this->m_BitmapOffset +
		((managePageNum - 1) >> 3);

	// ビットマップ最後に32 ビットで割り切れない余りの 1 〜 3 バイトがあれば
	// そこを最初に舐める
	PageNum	remainPageNum = managePageNum & 0x1F;
	//		                ~~~~~~~~~~~~~~~~~~~~ == managePageNum % 32
	int	remainBytes = remainPageNum >> 3;
	//	              ~~~~~~~~~~~~~~~~~~ == remainPageNum / 8
	if ((remainPageNum & 0x1F) != 0) remainBytes++;
	for (int i = remainBytes - 1; i >= 0; i--, bitmap8--) {

		if (*bitmap8 != 0) {

			unsigned char	mask = 0x80;
			for (int j = 7; j >= 0; j--, mask >>= 1) {

				if ((*bitmap8 & mask) != 0) {

					// ビット ON → 使用中物理ページ

					usedPageIndex =
						(managePageNum & 0xFFFFFFE0) + (i << 3) + j;
					if (startIndex < 0 || usedPageIndex < startIndex) break;
					usedPageIndex = -1;
				}
			}

			if (usedPageIndex >= 0) break;
		}
	}

	if (usedPageIndex < 0 && managePageNum >= 32) {

		bitmap8++;
		const unsigned int*	bitmap32 =
			syd_reinterpret_cast<const unsigned int*>(bitmap8);
		bitmap32--;
		for (PageNum i = managePageNum - remainPageNum;
			 ;
			 i -= 32, bitmap32--) {

			if (*bitmap32 != 0) {

				bitmap8 = syd_reinterpret_cast<const unsigned char*>(bitmap32 + 1);
				bitmap8--;
				for (int j = 3; j >= 0; j--, bitmap8--) {

					if (*bitmap8 != 0) {

						unsigned char	mask = 0x80;
						for (int k = 7; k >= 0; k--, mask >>= 1) {

							if ((*bitmap8 & mask) != 0) {

								// ビット ON → 使用中の物理ページ

								usedPageIndex = (i - 32) + (j << 3) + k;
								if (startIndex < 0 ||
									usedPageIndex < startIndex) break;
								usedPageIndex = -1;
							}
						}

						if (usedPageIndex >= 0) break;
					}

				} // end for j

				if (usedPageIndex >= 0) break;
			}

			if (i == 0) break;

		} // end for i
	}

	return usedPageIndex;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::searchUnusePage --
//		リーフから未使用物理ページを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const void*	Leaf_
//		[IN]		リーフ
//
//	RETURN
//	int
//		未使用物理ページのインデックス
//		（引数 Leaf_ が指すリーフ内でのインデックス）
//
//	EXCEPTIONS

int
PageManageFile2::Leaf::searchUnusePage(const void*	Leaf_) const
{
	const Header*	header = static_cast<const Header*>(Leaf_);
	if (header->m_UnusePageNum == 0) return -1;

	int	unusePageIndex = -1;

	PageNum	managePageNum = header->m_UsedPageNum + header->m_UnusePageNum;

	const unsigned char*	bitmap8 =
		static_cast<const unsigned char*>(Leaf_) + this->m_BitmapOffset;

	// まずはビットマップを 32 ビットずつ舐める
	if (managePageNum >= 32) {

		const unsigned int*	bitmap32 =
			syd_reinterpret_cast<const unsigned int*>(bitmap8);
		PageNum	managePageNum32 = managePageNum & 0xFFFFFFE0;
		for (PageNum i = 0; i < managePageNum32; i += 32, bitmap32++) {

			if (*bitmap32 != 0xFFFFFFFF) {

				bitmap8 = syd_reinterpret_cast<const unsigned char*>(bitmap32);
				for (int j = 0; j < 4; j++, bitmap8++) {

					if (*bitmap8 != 0xFF) {

						unsigned char	mask = 1;
						for (int k = 0; k < 8; k++, mask <<= 1) {

							if ((*bitmap8 & mask) == 0) {

								// ビット OFF → 未使用物理ページ

								unusePageIndex = i + (j << 3) + k;
								//                    ~~~~~~ == j * 8
								break;
							}
						}

						; _SYDNEY_ASSERT(unusePageIndex >= 0);
						break;
					}

				} // end for j

				; _SYDNEY_ASSERT(unusePageIndex >= 0);
				break;
			}

		} // end for i

		if (unusePageIndex < 0) {
			bitmap8 = syd_reinterpret_cast<const unsigned char*>(bitmap32);
		}
	}

	// 32 ビットずつ舐めて見つからなかったら残りの 1 〜 3 バイトを舐める
	if (unusePageIndex < 0) {

		PageNum	remainPageNum = managePageNum & 0x1F;
		int	remainBytes = remainPageNum >> 3;
		if ((remainPageNum & 0x1F) != 0) remainBytes++;
		for (int i = 0; i < remainBytes; i++, bitmap8++) {

			if (*bitmap8 != 0xFF) {

				unsigned char	mask = 1;
				for (int j = 0; j < 8; j++, mask <<= 1) {

					if ((*bitmap8 & mask) == 0) {

						// ビット OFF → 未使用物理ページ

						unusePageIndex =
							(managePageNum & 0xFFFFFFE0) + (i << 3) + j;
						//   ~~~~~~~~~~~~~~~~~~~~~~~~~~     ~~~~~~ == i * 8
						//   == managePageNum / 32 * 32
						break;
					}
				}

				; _SYDNEY_ASSERT(unusePageIndex >= 0);
				break;
			}
		}
	}

	; _SYDNEY_ASSERT(unusePageIndex >= 0);

	return unusePageIndex;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::getCount --
//		物理ページ以前に存在するリーフ数を返す
//
//	NOTES
//	物理ファイル先頭から、引数 PageID_ で示される物理ページまでの間に存在する
//	リーフ数を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	PhysicalFile::PageNum
//		物理ページ以前に存在するリーフ数
//
//	EXCEPTIONS

PageNum
PageManageFile2::Leaf::getCount(const PageID	PageID_) const
{
	return PageID_ / m_ManagePageMax + 1;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::getVersionPageID --
//		リーフのバージョンページ識別子を返す
//
//	NOTES
//	引数 PageID_ で示される物理ページを管理しているリーフの
//	バージョンページ識別子を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		[IN]		物理ページ識別子
//
//	RETURN
//	Version::Page::ID
//		リーフのバージョンページ識別子
//
//	EXCEPTIONS

Version::Page::ID
PageManageFile2::Leaf::getVersionPageID(const PageID	PageID_) const
{
	// 物理ページ以前のノード数
	PageNum	nodeNum = this->m_Node.getCount(PageID_);
	// 物理ページ以前のリーフ数
	PageNum	leafNum = this->getCount(PageID_);

	return
		(this->m_ManagePageMax * (leafNum - 1)) +
		leafNum +
		(nodeNum - 1);
}

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
