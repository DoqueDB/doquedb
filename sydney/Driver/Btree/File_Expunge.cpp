// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_Expunge.cpp -- Ｂ＋木ファイルクラスの実現ファイル（削除関連）
// 
// Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Btree/File.h"

#include "Exception/FileNotOpen.h"
#include "Exception/IllegalFileAccess.h"
#include "Exception/BadArgument.h"

#include "Common/Assert.h"

#include "Btree/FileInformation.h"
#include "Btree/ValueFile.h"
#include "Btree/NodePageHeader.h"
#include "Btree/KeyInformation.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::File::expungeCheck --
//		オブジェクトを削除するための環境が整っているかをチェックする
//
//	NOTES
//	オブジェクトを削除するための環境が整っているかをチェックする。
//
//	ARGUMENTS
//	const Common::DataArrayData*	SearchCondition_
//		削除対象オブジェクトへのポインタ（検索条件）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		オープンされていない
//	IllegalFileAccess
//		オブジェクト削除のためにオープンされていない
//	BadArgument
//		検索条件が不正
//
void
File::expungeCheck(const Common::DataArrayData*	SearchCondition_) const
{
	if (this->m_pPhysicalFile == 0 ||
		this->m_pOpenParameter->m_bEstimate)
	{
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	if (this->m_pOpenParameter->m_iOpenMode !=
		FileCommon::OpenMode::Update)
	{
		throw Exception::IllegalFileAccess(moduleName, srcFile, __LINE__);
	}

	if (SearchCondition_== 0)
	{
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
}

//
//	FUNCTION private
//	Btree::File::deleteLeafPageKey --
//		リーフページのキーオブジェクトを削除する
//
//	NOTES
//	リーフページのキーオブジェクトを削除する。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageIDVector&	FreeNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（フリーしたノードページの物理ページ識別子をつむ）
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::deleteLeafPageKey(FileInformation&	FileInfo_,
						PageVector&			AttachNodePages_,
						PageIDVector&		AllocateNodePageIDs_,
						PageIDVector&		FreeNodePageIDs_,
						ValueFile*			ValueFile_,
						PageVector&			AttachValuePages_) const
{
	ModUInt32	currentTreeDepth = FileInfo_.readTreeDepth();

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	this->deleteKey(FileInfo_,
					leafPage,
					this->m_KeyInfoIndex,
					currentTreeDepth,
					AttachNodePages_,
					AllocateNodePageIDs_,
					FreeNodePageIDs_,
					ValueFile_,
					AttachValuePages_,
					true); // リーフページ

	while (true)
	{
		bool	rootIsLeaf = (currentTreeDepth == 1);

		if (rootIsLeaf)
		{
			break;
		}

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		PhysicalFile::Page*	rootNodePage =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 rootNodePageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		NodePageHeader	rootNodePageHeader(this->m_pTransaction,
										   rootNodePage,
										   rootIsLeaf);

		ModUInt32	useKeyInfoNum =
			rootNodePageHeader.readUseKeyInformationNumber();

		KeyInformation	keyInfo(this->m_pTransaction,
								rootNodePage,
								0,
								rootIsLeaf,
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		PhysicalFile::PageID	childNodePageID =
			keyInfo.readChildNodePageID();

		if (useKeyInfoNum > 1)
		{
			checkMemoryExhaust(rootNodePage);

			break;
		}

		PhysicalFile::PageID	newRootNodePageID = childNodePageID;

		this->freeNodePage(rootNodePage,
						   AttachNodePages_,
						   FreeNodePageIDs_,
						   rootIsLeaf);

		currentTreeDepth--;

		FileInfo_.writeTreeDepth(currentTreeDepth);
		FileInfo_.writeRootNodePageID(newRootNodePageID);
	}

	if (this->m_CatchMemoryExhaust && leafPage != 0)
	{
		this->m_pPhysicalFile->detachPage(
			leafPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまう
	}
}

//
//	FUNCTION private
//	Btree::File::deleteKey -- キーオブジェクトを削除する
//
//	NOTES
//	キーオブジェクトを削除する。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	PhysicalFile::Page*&	NodePage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32			KeyInfoIndex_
//		キー情報インデックス
//	const ModUInt32			NodeDepth_
//		ルートノードページから、
//		削除するキーオブジェクトが記録されている
//		ノードページまでの、段数（ルートノードページ = 1）
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageIDVector&	FreeNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（フリーしたノードページの物理ページ識別子をつむ）
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const bool				IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::deleteKey(FileInformation&		FileInfo_,
				PhysicalFile::Page*&	NodePage_,
				const ModUInt32			KeyInfoIndex_,
				const ModUInt32			NodeDepth_,
				PageVector&				AttachNodePages_,
				PageIDVector&			AllocateNodePageIDs_,
				PageIDVector&			FreeNodePageIDs_,
				ValueFile*				ValueFile_,
				PageVector&				AttachValuePages_,
				const bool				IsLeafPage_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		// キー値をキー情報内に記録するタイプのファイル…

		this->deleteSimpleKey(FileInfo_,
							  NodePage_,
							  KeyInfoIndex_,
							  NodeDepth_,
							  AttachNodePages_,
							  FreeNodePageIDs_,
							  ValueFile_,
							  AttachValuePages_,
							  IsLeafPage_);
	}
	else
	{
		// キー値をキーオブジェクトに記録するタイプのファイル…

		this->deleteKeyObject(FileInfo_,
							  NodePage_,
							  KeyInfoIndex_,
							  NodeDepth_,
							  AttachNodePages_,
							  AllocateNodePageIDs_,
							  FreeNodePageIDs_,
							  ValueFile_,
							  AttachValuePages_,
							  IsLeafPage_);
	}
}

//
//	FUNCTION private
//	Btree::File::isPossibleMerge -- マージ可能なノードページかを調べる
//
//	NOTES
//	マージ可能なノードページかを調べる。
//
//	ARGUMENTS
//	PhysicalFile::Page*	NodePage_
//		マージ先のノードページの物理ページ記述子
//	const bool			IsLeafPage_
//		マージ対象のノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	bool
//		マージ可能かどうか
//			true  : マージ可能
//			false : マージ不可能
//
//	EXCEPTIONS
//	なし
//
bool
File::isPossibleMerge(PhysicalFile::Page*	NodePage_,
					  const bool			IsLeafPage_) const
{
	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   NodePage_,
								   IsLeafPage_);

	ModUInt32	useKeyNum = nodePageHeader.readUseKeyInformationNumber();

	return
		(useKeyNum + this->m_cFileParameter.m_NodeMergeCheckThreshold)
		< this->m_cFileParameter.m_NodeMergeExecuteThreshold;
}

//
//	FUNCTION private
//	Btree::File::isSameParentNodePage --
//		親ノードページが同じかどうかを調べる
//
//	NOTES
//	引数で指定される2つの子ノードページの親ノードページが
//	同じかどうかを調べる。
//
//	ARGUMENTS
//	NodePageHeader&	SrcChildNodePageHeader_
//		子ノードページヘッダへの参照
//	NodePageHeader&	DstChildNodePageHeader_
//		子ノードページヘッダへの参照
//
//	RETURN
//	bool
//		親ノードページが同じかどうか
//			true  : 同じ
//			false : 異なる
//
//	EXCEPTIONS
//	なし
//
bool
File::isSameParentNodePage(NodePageHeader&	SrcChildNodePageHeader_,
						   NodePageHeader&	DstChildNodePageHeader_) const
{
	return
		SrcChildNodePageHeader_.readParentNodePageID() ==
		DstChildNodePageHeader_.readParentNodePageID();
}

//
//	FUNCTION private
//	Btree::File::searchMergeNodePage -- マージ先のノードページを検索する
//
//	NOTES
//	マージ先のノードページを検索し、
//	該当するノードページへ辿ることができれば、そのノードページの
//	物理ページ記述子を返す。
//	該当するノードページへ辿ることができない場合や、
//	マージ先のノードページへ辿ることはできても、
//	マージすることにより、そのノードページの使用率が
//	50％以上になってしまう場合などには0（ヌルポインタ）を返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*		SrcNodePage_
//		マージするノードページの物理ページ記述子
//	Btree::NodePageHeader&	SrcNodePageHeader_
//		マージするノードページのヘッダへの参照
//	const bool				IsLeafPage_
//		マージするノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	bool&					IsPrev_
//		戻り値が“前のノードページ”の物理ページ記述子かどうか
//			true  : 戻り値が前のノードページ
//			false : 戻り値が後ろのノードページ
//
//	RETURN
//	PhysicalFile::Page*
//		マージ先のノードページの物理ページ記述子
//		該当するノードページへ辿ることができない場合などは
//		0（ヌルポインタ）。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::searchMergeNodePage(PhysicalFile::Page*	SrcNodePage_,
						  NodePageHeader&		SrcNodePageHeader_,
						  const bool			IsLeafPage_,
						  PageVector&			AttachNodePages_,
						  bool&					IsPrev_) const
{
	if (IsLeafPage_)
	{
		// マージ対象のノードページがリーフページ…

		// では、前後のリーフページを参照し、
		// マージ可能かどうかを確認する。

		PhysicalFile::PageID	prevLeafPageID =
			SrcNodePageHeader_.readPrevLeafPageID();

		if (prevLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			PhysicalFile::Page*	prevLeafPage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 prevLeafPageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			NodePageHeader	prevLeafPageHeader(this->m_pTransaction,
											   prevLeafPage,
											   true); // リーフページ

			if (this->isPossibleMerge(prevLeafPage,
									  true) // リーフページ
				&&
				this->isSameParentNodePage(SrcNodePageHeader_,
										   prevLeafPageHeader))
			{
				// 前のリーフページとマージ可能…

				IsPrev_ = true;

				return prevLeafPage;
			}

			checkMemoryExhaust(prevLeafPage);
		}

		PhysicalFile::PageID	nextLeafPageID =
			SrcNodePageHeader_.readNextLeafPageID();

		if (nextLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			PhysicalFile::Page*	nextLeafPage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 nextLeafPageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			NodePageHeader	nextLeafPageHeader(this->m_pTransaction,
											   nextLeafPage,
											   true); // リーフページ

			if (this->isPossibleMerge(nextLeafPage,
									  true) // リーフページ
				&&
				this->isSameParentNodePage(SrcNodePageHeader_,
										   nextLeafPageHeader))
			{
				// 後ろのリーフページとマージ可能…

				IsPrev_ = false;

				return nextLeafPage;
			}

			checkMemoryExhaust(nextLeafPage);
		}
	}
	else
	{
		// マージ対象のノードページがリーフページではない…

		// では、親ノードページから前後のノードページへ辿り、
		// 前後のノードページがマージ可能かどうかを確認する。

		PhysicalFile::Page*	parentNodePage = 0;
		ModUInt32			parentNodeKeyInfoIndex = ModUInt32Max;

		int					keyNum = 0;
		Os::Memory::Size	keySize = 0;

		if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
		{
			this->searchKeyInformationSimpleKey(SrcNodePage_,
												parentNodePage,
												parentNodeKeyInfoIndex,
												AttachNodePages_,
												IsLeafPage_);

			keyNum = this->m_cFileParameter.m_KeyNum;
			keySize = this->m_cFileParameter.m_KeySize;
		}
		else
		{
			this->searchKeyInformation(SrcNodePage_,
									   parentNodePage,
									   parentNodeKeyInfoIndex,
									   AttachNodePages_,
									   IsLeafPage_);
		}

		if (parentNodeKeyInfoIndex > 0)
		{
			KeyInformation	parentPrevKeyInfo(this->m_pTransaction,
											  parentNodePage,
											  parentNodeKeyInfoIndex - 1,
											  false, // リーフページではない
											  keyNum,
											  keySize);

			PhysicalFile::PageID	prevNodePageID =
				parentPrevKeyInfo.readChildNodePageID();

			PhysicalFile::Page*	prevNodePage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 prevNodePageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			if (this->isPossibleMerge(prevNodePage,
									  false)) // リーフページではない
			{
				// 前のノードページとマージ可能…

				IsPrev_ = true;

				return prevNodePage;
			}

			checkMemoryExhaust(prevNodePage);
		}

		NodePageHeader	parentNodePageHeader(this->m_pTransaction,
											 parentNodePage,
											 false); // リーフページではない

		ModUInt32	parentNodeUseKeyNum =
			parentNodePageHeader.readUseKeyInformationNumber();

		if (parentNodeKeyInfoIndex < parentNodeUseKeyNum - 1)
		{
			KeyInformation	parentNextKeyInfo(this->m_pTransaction,
											  parentNodePage,
											  parentNodeKeyInfoIndex + 1,
											  false, // リーフページではない
											  keyNum,
											  keySize);

			PhysicalFile::PageID	nextNodePageID =
				parentNextKeyInfo.readChildNodePageID();

			PhysicalFile::Page*	nextNodePage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 nextNodePageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			if (this->isPossibleMerge(nextNodePage,
									  false)) // リーフページではない
			{
				// 後ろのノードページとマージ可能…

				IsPrev_ = false;

				return nextNodePage;
			}

			checkMemoryExhaust(nextNodePage);
		}
	}

	return 0;
}

//
//	FUNCTION private
//	Btree::File::deleteKeyObject --
//		キーオブジェクトを削除する
//
//	NOTES
//	キーオブジェクトを削除する。
//	キーテーブルが1物理ページに収まる場合に呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	PhysicalFile::Page*&		NodePage_
//		キー情報が記録されているノードページの物理ページ識別子
//	const ModUInt32			KeyInfoIndex_
//		キー情報インデックス
//	const ModUInt32			NodeDepth_
//		削除するキーオブジェクトが記録されているノードページの階層
//		（ルートノードページから、削除するキーオブジェクトが
//		　記録されているノードページまでの、段数。ルートノードページが1。）
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageIDVector&	FreeNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（フリーしたノードページの物理ページ識別子をつむ）
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const bool				IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::deleteKeyObject(FileInformation&		FileInfo_,
					  PhysicalFile::Page*&	NodePage_,
					  const ModUInt32		KeyInfoIndex_,
					  const ModUInt32		NodeDepth_,
					  PageVector&			AttachNodePages_,
					  PageIDVector&			AllocateNodePageIDs_,
					  PageIDVector&			FreeNodePageIDs_,
					  ValueFile*			ValueFile_,
					  PageVector&			AttachValuePages_,
					  const bool			IsLeafPage_) const
{
	KeyInformation	keyInfo(this->m_pTransaction,
							NodePage_,
							KeyInfoIndex_,
							IsLeafPage_);

	ModUInt64	directKeyObjectID = keyInfo.readKeyObjectID();

	PhysicalFile::PageID	nodePageID = NodePage_->getID();

	PhysicalFile::Page*		directKeyObjectPage = 0;

	bool	attached =
		File::attachObjectPage(this->m_pTransaction,
							   directKeyObjectID,
							   NodePage_,
							   directKeyObjectPage,
							   this->m_FixMode,
							   this->m_CatchMemoryExhaust,
							   AttachNodePages_);

	freeKeyObjectArea(directKeyObjectPage,
					  Common::ObjectIDData::getLatterValue(directKeyObjectID),
					  AttachNodePages_,
					  FreeNodePageIDs_);

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   NodePage_,
								   IsLeafPage_);

	ModUInt32	useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	ModUInt32	lastUseKeyInfoIndex = useKeyInfoNum - 1;

	bool	doFreePage = false;

	if (KeyInfoIndex_ == lastUseKeyInfoIndex)
	{
		// ノード内の最後のキーを削除した…

		if (useKeyInfoNum == 1)
		{
			// しかも、ノード内の唯一存在していたキーを削除した…

			doFreePage = true;

			if (NodeDepth_ > 1)
			{
				// ルートノードページ以外のキーを削除した…

				PhysicalFile::Page*	parentNodePage = 0;
				ModUInt32			parentNodeKeyInfoIndex = ModUInt32Max;

				this->searchKeyInformation(NodePage_,
										   parentNodePage,
										   parentNodeKeyInfoIndex,
										   AttachNodePages_,
										   IsLeafPage_);

				this->deleteKeyObject(FileInfo_,
									  parentNodePage,
									  parentNodeKeyInfoIndex,
									  NodeDepth_ - 1,
									  AttachNodePages_,
									  AllocateNodePageIDs_,
									  FreeNodePageIDs_,
									  ValueFile_,
									  AttachValuePages_,
									  false); // リーフページではない
			}
			else
			{
				// ※ ルートノードページは解放しない

				doFreePage = false;
			}

			if (IsLeafPage_)
			{
				// リーフページのキーを削除した…

				// リーフページのリンクの繋ぎ変えをする

				PhysicalFile::PageID	prevLeafPageID =
					nodePageHeader.readPrevLeafPageID();
				PhysicalFile::PageID	nextLeafPageID =
					nodePageHeader.readNextLeafPageID();

				if (prevLeafPageID !=
					PhysicalFile::ConstValue::UndefinedPageID)
				{
					PhysicalFile::Page*	prevLeafPage =
						File::attachPage(this->m_pTransaction,
										 this->m_pPhysicalFile,
										 prevLeafPageID,
										 this->m_FixMode,
										 this->m_CatchMemoryExhaust,
										 AttachNodePages_);

					NodePageHeader	prevLeafPageHeader(this->m_pTransaction,
													   prevLeafPage,
													   IsLeafPage_);

					prevLeafPageHeader.writeNextLeafPageID(nextLeafPageID);

					if (this->m_CatchMemoryExhaust)
					{
						this->m_pPhysicalFile->detachPage(
							prevLeafPage,
							PhysicalFile::Page::UnfixMode::Dirty,
							false); // 本当にデタッチ（アンフィックス）
							        // してしまう
					}
				}
				
				if (nextLeafPageID !=
					PhysicalFile::ConstValue::UndefinedPageID)
				{
					PhysicalFile::Page*	nextLeafPage =
						File::attachPage(this->m_pTransaction,
										 this->m_pPhysicalFile,
										 nextLeafPageID,
										 this->m_FixMode,
										 this->m_CatchMemoryExhaust,
										 AttachNodePages_);

					NodePageHeader	nextLeafPageHeader(this->m_pTransaction,
													   nextLeafPage,
													   IsLeafPage_);

					nextLeafPageHeader.writePrevLeafPageID(prevLeafPageID);

					if (this->m_CatchMemoryExhaust)
					{
						this->m_pPhysicalFile->detachPage(
							nextLeafPage,
							PhysicalFile::Page::UnfixMode::Dirty,
							false); // 本当にデタッチ（アンフィックス）
							        // してしまう
					}
				}

				// ファイル管理情報を更新する

				PhysicalFile::PageID	topLeafPageID =
					FileInfo_.readTopLeafPageID();
				PhysicalFile::PageID	lastLeafPageID =
					FileInfo_.readLastLeafPageID();

				if (topLeafPageID == lastLeafPageID)
				{
					doFreePage = false;

					nodePageHeader.decUseKeyInformationNumber();
				}
				else
				{
					if (nodePageID == topLeafPageID)
					{
						FileInfo_.writeTopLeafPageID(nextLeafPageID);
					}

					if (nodePageID == lastLeafPageID)
					{
						FileInfo_.writeLastLeafPageID(prevLeafPageID);
					}
				}
			}

			if (doFreePage)
			{
				this->freeNodePage(NodePage_,
								   AttachNodePages_,
								   FreeNodePageIDs_,
								   IsLeafPage_);
			}
			else
			{
				this->compactionNodePage(NodePage_,
										 AttachNodePages_,
										 IsLeafPage_);
			}
		}
		else
		{
			// ノード内に複数キーが存在しており、
			// その中での最後のキーを削除した…

			nodePageHeader.decUseKeyInformationNumber();

			if (NodeDepth_ > 1)
			{
				this->rewriteParentNodeKey(NodePage_,
										   IsLeafPage_,
										   NodeDepth_ - 1,
										   AttachNodePages_,
										   AllocateNodePageIDs_);
			}

			this->compactionNodePage(NodePage_,
									 AttachNodePages_,
									 IsLeafPage_);
		}
	}
	else
	{
		this->shiftKeyInfoForExpunge(NodePage_,
									 useKeyInfoNum,
									 KeyInfoIndex_,
									 IsLeafPage_,
									 ValueFile_,
									 AttachValuePages_);

		nodePageHeader.decUseKeyInformationNumber();

		this->compactionNodePage(NodePage_,
								 AttachNodePages_,
								 IsLeafPage_);
	}

	if (doFreePage == false &&
		useKeyInfoNum > 1 &&
		useKeyInfoNum - 1 ==
		this->m_cFileParameter.m_NodeMergeCheckThreshold)
	{
		// ノードマージを実行するかチェックする条件が整った…

		// 可能ならばノードページをマージする
		this->mergeNodePage(FileInfo_,
							NodeDepth_,
							NodePage_,
							nodePageHeader,
							IsLeafPage_,
							AttachNodePages_,
							AllocateNodePageIDs_,
							FreeNodePageIDs_,
							ValueFile_,
							AttachValuePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::mergeNodePage -- ノードページをマージする
//
//	NOTES
//	引数SrcNodePage_で示されるノードページを
//	マージすることができるかどうかを確認し、
//	可能ならばマージを行う。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	const ModUInt32			NodeDepth_
//		マージするノードページの階層（ルートノードページが1）
//	PhysicalFile::Page*&	SrcNodePage_
//		マージするノードページの物理ページ記述子
//	Btree::NodePageHeader&	SrcNodePageHeader_
//		マージするノードページのヘッダへの参照
//	const bool				IsLeafPage_
//		マージするノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageIDVector&	FreeNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（フリーしたノードページの物理ページ識別子をつむ）
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::mergeNodePage(FileInformation&		FileInfo_,
					const ModUInt32			NodeDepth_,
					PhysicalFile::Page*&	SrcNodePage_,
					NodePageHeader&			SrcNodePageHeader_,
					const bool				IsLeafPage_,
					PageVector&				AttachNodePages_,
					PageIDVector&			AllocateNodePageIDs_,
					PageIDVector&			FreeNodePageIDs_,
					ValueFile*				ValueFile_,
					PageVector&				AttachValuePages_) const
{
	if (NodeDepth_ == 1)
	{
		// ルートノードページ…

		// ルートノードページはマージできない。

		return;
	}

	// マージ先のノードページを検索する
	bool	partnerIsPrev = false;
	PhysicalFile::Page*	dstNodePage =
		this->searchMergeNodePage(SrcNodePage_,
								  SrcNodePageHeader_,
								  IsLeafPage_,
								  AttachNodePages_,
								  partnerIsPrev);

	if (dstNodePage == 0)
	{
		// マージ先のノードページが見つからなかった…

		// では、マージはできません。

		return;
	}

	ModUInt32	srcNodeUseKeyNum =
		SrcNodePageHeader_.readUseKeyInformationNumber();

	NodePageHeader	dstNodePageHeader(this->m_pTransaction,
									  dstNodePage,
									  IsLeafPage_);

	ModUInt32	dstNodeUseKeyNum =
		dstNodePageHeader.readUseKeyInformationNumber();

	; _SYDNEY_ASSERT(
		srcNodeUseKeyNum + dstNodeUseKeyNum <=
			this->m_cFileParameter.m_KeyPerNode);

	//
	// マージ対象の2つのノードページ内の使用中のキー情報数によって、
	// どちらからどちらにマージするかを決める。
	// （キー情報数が少ない方のノードページを、
	// 　多い方のノードページにくっつける。）
	//

	bool	freeIsSrc = false;

	if ((partnerIsPrev && dstNodeUseKeyNum < srcNodeUseKeyNum) ||
		(partnerIsPrev == false && dstNodeUseKeyNum >= srcNodeUseKeyNum))
	{
		this->mergeWithNextNodePage(FileInfo_,
									NodeDepth_,
									partnerIsPrev ?
										dstNodePage : SrcNodePage_,
									partnerIsPrev ?
										SrcNodePage_ : dstNodePage,
									IsLeafPage_,
									AttachNodePages_,
									AllocateNodePageIDs_,
									FreeNodePageIDs_,
									ValueFile_,
									AttachValuePages_);

		freeIsSrc = (partnerIsPrev == false);
	}
	else
	{
		this->mergeWithPrevNodePage(FileInfo_,
									NodeDepth_,
									partnerIsPrev ?
										SrcNodePage_ : dstNodePage,
									partnerIsPrev ?
										dstNodePage : SrcNodePage_,
									IsLeafPage_,
									AttachNodePages_,
									AllocateNodePageIDs_,
									FreeNodePageIDs_,
									ValueFile_,
									AttachValuePages_);

		freeIsSrc = partnerIsPrev;
	}

	PhysicalFile::Page*	freeNodePage =
		freeIsSrc ? SrcNodePage_ : dstNodePage;

	//
	// リーフページを解放するならば、
	// ファイル管理情報を更新する必要があるかもしれない。
	//

	if (IsLeafPage_)
	{
		PhysicalFile::PageID	freeNodePageID = freeNodePage->getID();

		NodePageHeader	freeNodePageHeader(this->m_pTransaction,
										   freeNodePage,
										   true); // リーフページ

		if (freeNodePageID == FileInfo_.readTopLeafPageID())
		{
			FileInfo_.writeTopLeafPageID(
				freeNodePageHeader.readNextLeafPageID());
		}
		else if (freeNodePageID == FileInfo_.readLastLeafPageID())
		{
			FileInfo_.writeLastLeafPageID(
				freeNodePageHeader.readPrevLeafPageID());
		}
	}

	if (this->m_CatchMemoryExhaust && dstNodePage != 0 && freeIsSrc)
	{
		this->m_pPhysicalFile->detachPage(
			dstNodePage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	//
	// マージしたノードページ（マージ先ではなくて）を解放する
	//

	this->freeNodePage(freeNodePage,
					   AttachNodePages_,
					   FreeNodePageIDs_,
					   IsLeafPage_);

	if (freeIsSrc)
	{
		SrcNodePage_ = 0;
	}
}

//
//	FUNCTION private
//	Btree::File::mergeWithPrevNodePage -- 前のノードページとマージする
//
//	NOTES
//	前のノードページとマージする。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	const ModUInt32			NodeDepth_
//		マージするノードページの階層（ルートノードページが1）
//	PhysicalFile::Page*		SrcNodePage_
//		マージするノードページの物理ページ記述子
//	PhysicalFile::Page*		DstNodePage_
//		マージ先ノードページの物理ページ記述子
//	const bool				IsLeafPage_
//		マージ対象のノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageIDVector&	FreeNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（フリーしたノードページの物理ページ識別子をつむ）
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::mergeWithPrevNodePage(FileInformation&	FileInfo_,
							const ModUInt32		NodeDepth_,
							PhysicalFile::Page*	SrcNodePage_,
							PhysicalFile::Page*	DstNodePage_,
							const bool			IsLeafPage_,
							PageVector&			AttachNodePages_,
							PageIDVector&		AllocateNodePageIDs_,
							PageIDVector&		FreeNodePageIDs_,
							ValueFile*			ValueFile_,
							PageVector&			AttachValuePages_) const
{
	//
	// 前のノードページとマージするための準備をする。
	//

	PhysicalFile::PageID	dstNodePageID = DstNodePage_->getID();

	NodePageHeader	srcNodePageHeader(this->m_pTransaction,
									  SrcNodePage_,
									  IsLeafPage_);

	ModUInt32	srcNodeUseKeyNum =
		srcNodePageHeader.readUseKeyInformationNumber();

	NodePageHeader	dstNodePageHeader(this->m_pTransaction,
									  DstNodePage_,
									  IsLeafPage_);

	ModUInt32	dstNodeUseKeyNum =
		dstNodePageHeader.readUseKeyInformationNumber();

	PhysicalFile::Page*	parentNodePage = 0;
	ModUInt32			srcParentNodeKeyInfoIndex = ModUInt32Max;

	this->searchKeyInformation(SrcNodePage_,
							   parentNodePage,
							   srcParentNodeKeyInfoIndex,
							   AttachNodePages_,
							   IsLeafPage_);

	; _SYDNEY_ASSERT(parentNodePage != 0);
	; _SYDNEY_ASSERT(srcParentNodeKeyInfoIndex > 0);

	//
	// マージするノードページのキーテーブルとキーオブジェクトを
	// マージ先のノードページに移動（コピー）する。
	//

	KeyInformation	srcKeyInfo(this->m_pTransaction,
							   SrcNodePage_,
							   0, // キー情報のインデックス
							   IsLeafPage_);

	KeyInformation	dstKeyInfo(this->m_pTransaction,
							   DstNodePage_,
							   dstNodeUseKeyNum,
							   IsLeafPage_);

	PhysicalFile::Page*	valueObjectPage = 0;

	bool	childNodeIsLeaf =
		((NodeDepth_ + 1) == FileInfo_.readTreeDepth());

	for (ModUInt32 i = 0; i < srcNodeUseKeyNum; i++)
	{
		srcKeyInfo.setStartOffsetByIndex(i);

		dstKeyInfo.setStartOffsetByIndex(dstNodeUseKeyNum + i);

		// キーオブジェクトを移動する

		dstKeyInfo.writeKeyObjectID(
			this->moveKey(DstNodePage_,
						  srcKeyInfo.readKeyObjectID(),
						  AttachNodePages_,
						  AllocateNodePageIDs_,
						  IsLeafPage_));

		if (IsLeafPage_)
		{
			// マージ対象のノードページがリーフページ…

			ModUInt64	srcValueObjectID = srcKeyInfo.readValueObjectID();

			dstKeyInfo.writeValueObjectID(srcValueObjectID);

			// バリューオブジェクトに記録されている
			// リーフページの情報を更新する
			ValueFile_->update(srcValueObjectID,
							   dstNodePageID,
							   dstNodeUseKeyNum + i,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
		}
		else
		{
			// マージ対象のノードページがリーフページではない…

			PhysicalFile::PageID	childNodePageID =
				srcKeyInfo.readChildNodePageID();

			dstKeyInfo.writeChildNodePageID(childNodePageID);

			// 子ノードページヘッダに記録されている
			// 「親ノードページの物理ページ識別子」を更新する

			PhysicalFile::Page*	childNodePage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 childNodePageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			NodePageHeader	childNodePageHeader(this->m_pTransaction,
												childNodePage,
												childNodeIsLeaf);

			childNodePageHeader.writeParentNodePageID(dstNodePageID);

			if (this->m_CatchMemoryExhaust)
			{
				this->m_pPhysicalFile->detachPage(
					childNodePage,
					PhysicalFile::Page::UnfixMode::Dirty,
					false); // 本当にデタッチ（アンフィックス）してしまい、
					        // 物理ファイルマネージャが
					        // キャッシュしないようにする。
			}
		}
	}

	//
	// マージ先のノードページヘッダを更新する。
	//

	// ノードページ内の使用中のキー情報数を更新する
	dstNodePageHeader.writeUseKeyInformationNumber(
		srcNodeUseKeyNum + dstNodeUseKeyNum);

	if (IsLeafPage_)
	{
		// マージ対象のノードページがリーフページ…

		// リーフページ同士の双方向リンクをはりかえる

		PhysicalFile::PageID	nextLeafPageID =
			srcNodePageHeader.readNextLeafPageID();

		dstNodePageHeader.writeNextLeafPageID(nextLeafPageID);

		if (nextLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			PhysicalFile::Page*	nextLeafPage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 nextLeafPageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			NodePageHeader	nextLeafPageHeader(this->m_pTransaction,
											   nextLeafPage,
											   true); // リーフページ

			nextLeafPageHeader.writePrevLeafPageID(dstNodePageID);

			if (this->m_CatchMemoryExhaust)
			{
				this->m_pPhysicalFile->detachPage(
					nextLeafPage,
					PhysicalFile::Page::UnfixMode::Dirty,
					false); // 本当にデタッチ（アンフィックス）してしまい、
							// 物理ファイルマネージャが
							// キャッシュしないようにする。
			}
		}
	}

	//
	// 親ノードページに記録されている、マージ先の
	// ノードページの代表キーを更新する。
	//

	KeyInformation	srcParentKeyInfo(this->m_pTransaction,
									 parentNodePage,
									 srcParentNodeKeyInfoIndex,
									 false); // リーフページではない

	KeyInformation	dstParentKeyInfo(this->m_pTransaction,
									 parentNodePage,
									 srcParentNodeKeyInfoIndex - 1,
									 false); // リーフページではない

	// キーオブジェクトのIDを入れ替える
	// こうすることによって、マージ元のノードページの代表キーの
	// 削除が簡単になる。

	ModUInt64	srcKeyObjectID = srcParentKeyInfo.readKeyObjectID();

	ModUInt64	dstKeyObjectID = dstParentKeyInfo.readKeyObjectID();

	srcParentKeyInfo.writeKeyObjectID(dstKeyObjectID);

	dstParentKeyInfo.writeKeyObjectID(srcKeyObjectID);

	//
	// 親ノードページに記録されている、マージ元の
	// ノードページの代表キーを削除する。
	//

	this->deleteKeyObject(FileInfo_,
						  parentNodePage,
						  srcParentNodeKeyInfoIndex,
						  NodeDepth_ - 1,
						  AttachNodePages_,
						  AllocateNodePageIDs_,
						  FreeNodePageIDs_,
						  ValueFile_,
						  AttachValuePages_,
						  false); // リーフページではない

	if (this->m_CatchMemoryExhaust && parentNodePage != 0)
	{
		this->m_pPhysicalFile->detachPage(
			parentNodePage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}
}

//
//	FUNCTION private
//	Btree::File::mergeWithNextNodePage -- 後ろのノードページとマージする
//
//	NOTES
//	後ろのノードページとマージする。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	const ModUInt32			NodeDepth_
//		マージするノードページの階層（ルートノードページが1）
//	PhysicalFile::Page*		SrcNodePage_
//		マージするノードページの物理ページ記述子
//	PhysicalFile::Page*		DstNodePage_
//		マージ先ノードページの物理ページ記述子
//	const bool				IsLeafPage_
//		マージ対象のノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageIDVector&	FreeNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（フリーしたノードページの物理ページ識別子をつむ）
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::mergeWithNextNodePage(FileInformation&	FileInfo_,
							const ModUInt32		NodeDepth_,
							PhysicalFile::Page*	SrcNodePage_,
							PhysicalFile::Page*	DstNodePage_,
							const bool			IsLeafPage_,
							PageVector&			AttachNodePages_,
							PageIDVector&		AllocateNodePageIDs_,
							PageIDVector&		FreeNodePageIDs_,
							ValueFile*			ValueFile_,
							PageVector&			AttachValuePages_) const
{
	//
	// 後ろのノードページとマージするための準備をする。
	//

	PhysicalFile::PageID	dstNodePageID = DstNodePage_->getID();

	NodePageHeader	srcNodePageHeader(this->m_pTransaction,
									  SrcNodePage_,
									  IsLeafPage_);

	ModUInt32	srcNodeUseKeyNum =
		srcNodePageHeader.readUseKeyInformationNumber();

	NodePageHeader	dstNodePageHeader(this->m_pTransaction,
									  DstNodePage_,
									  IsLeafPage_);

	ModUInt32	dstNodeUseKeyNum =
		dstNodePageHeader.readUseKeyInformationNumber();

	PhysicalFile::Page*	parentNodePage = 0;
	ModUInt32			srcParentNodeKeyInfoIndex = ModUInt32Max;

	this->searchKeyInformation(SrcNodePage_,
							   parentNodePage,
							   srcParentNodeKeyInfoIndex,
							   AttachNodePages_,
							   IsLeafPage_);

	; _SYDNEY_ASSERT(parentNodePage != 0);

#ifdef DEBUG

	NodePageHeader	parentNodePageHeader(this->m_pTransaction,
										 parentNodePage,
										 IsLeafPage_);

	; _SYDNEY_ASSERT(
		srcParentNodeKeyInfoIndex <
			parentNodePageHeader.readUseKeyInformationNumber() - 1);
										 
#endif

	//
	// マージ先のキーテーブルをシフトする。
	//

	ModUInt32	srcKeyInfoIndex = dstNodeUseKeyNum - 1;
	ModUInt32	dstKeyInfoIndex = srcKeyInfoIndex + srcNodeUseKeyNum;

	KeyInformation	srcKeyInfo(this->m_pTransaction,
							   DstNodePage_,
							   srcKeyInfoIndex,
							   IsLeafPage_);

	KeyInformation	dstKeyInfo(this->m_pTransaction,
							   DstNodePage_,
							   dstKeyInfoIndex,
							   IsLeafPage_);

	PhysicalFile::Page*	valueObjectPage = 0;

	bool	childNodeIsLeaf =
		((NodeDepth_ + 1) == FileInfo_.readTreeDepth());

	while (true)
	{
		srcKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex);

		dstKeyInfo.setStartOffsetByIndex(dstKeyInfoIndex);

		dstKeyInfo.writeKeyObjectID(srcKeyInfo.readKeyObjectID());

		if (IsLeafPage_)
		{
			// マージ対象のノードページがリーフページ…

			ModUInt64	valueObjectID = srcKeyInfo.readValueObjectID();

			dstKeyInfo.writeValueObjectID(valueObjectID);

			ValueFile_->update(valueObjectID,
							   dstNodePageID,
							   dstKeyInfoIndex,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
		}
		else
		{
			// マージ対象のノードページがリーフページではない…

			dstKeyInfo.writeChildNodePageID(
				srcKeyInfo.readChildNodePageID());
		}

		if (srcKeyInfoIndex == 0)
		{
			break;
		}

		srcKeyInfoIndex--;
		dstKeyInfoIndex--;
	}

	//
	// マージするノードページのキーテーブルを
	// マージ先のノードページに移動（コピー）する。
	//

	srcKeyInfo.resetPhysicalPage(SrcNodePage_);

	for (ModUInt32 i = 0; i < srcNodeUseKeyNum; i++)
	{
		srcKeyInfo.setStartOffsetByIndex(i);

		dstKeyInfo.setStartOffsetByIndex(i);

		// キーオブジェクトを移動する

		dstKeyInfo.writeKeyObjectID(
			this->moveKey(DstNodePage_,
						  srcKeyInfo.readKeyObjectID(),
						  AttachNodePages_,
						  AllocateNodePageIDs_,
						  IsLeafPage_));

		if (IsLeafPage_)
		{
			// マージ対象のノードページがリーフページ…

			ModUInt64	srcValueObjectID = srcKeyInfo.readValueObjectID();

			dstKeyInfo.writeValueObjectID(srcValueObjectID);

			ValueFile_->update(srcValueObjectID,
							   dstNodePageID,
							   i,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
		}
		else
		{
			// マージ対象のノードページがリーフページではない…

			PhysicalFile::PageID	childNodePageID =
				srcKeyInfo.readChildNodePageID();

			dstKeyInfo.writeChildNodePageID(childNodePageID);

			// 子ノードページヘッダに記録されている
			// 「親ノードページの物理ページ識別子」を更新する

			PhysicalFile::Page*	childNodePage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 childNodePageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			NodePageHeader	childNodePageHeader(this->m_pTransaction,
												childNodePage,
												childNodeIsLeaf);

			childNodePageHeader.writeParentNodePageID(dstNodePageID);

			if (this->m_CatchMemoryExhaust)
			{
				this->m_pPhysicalFile->detachPage(
					childNodePage,
					PhysicalFile::Page::UnfixMode::Dirty,
					false); // 本当にデタッチ（アンフィックス）してしまい、
					        // 物理ファイルマネージャが
					        // キャッシュしないようにする。
			}
		}
	}

	//
	// マージ先のノードページのヘッダを更新する。
	//

	// ノードページ内の使用中のキー情報数を更新する
	dstNodePageHeader.writeUseKeyInformationNumber(
		srcNodeUseKeyNum + dstNodeUseKeyNum);

	if (IsLeafPage_)
	{
		// マージ対象のノードページがリーフページ…

		// リーフページ同士の双方向リンクをはりかえる

		PhysicalFile::PageID	prevLeafPageID =
			srcNodePageHeader.readPrevLeafPageID();

		dstNodePageHeader.writePrevLeafPageID(prevLeafPageID);

		if (prevLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			PhysicalFile::Page*	prevLeafPage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 prevLeafPageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			NodePageHeader	prevLeafPageHeader(this->m_pTransaction,
											   prevLeafPage,
											   true); // リーフページ

			prevLeafPageHeader.writeNextLeafPageID(dstNodePageID);

			if (this->m_CatchMemoryExhaust)
			{
				this->m_pPhysicalFile->detachPage(
					prevLeafPage,
					PhysicalFile::Page::UnfixMode::Dirty,
					false); // 本当にデタッチ（アンフィックス）してしまい、
							// 物理ファイルマネージャが
							// キャッシュしないようにする。
			}
		}
	}

	//
	// 親ノードページに記録されている、マージ元の
	// ノードページの代表キーを削除する。
	//

	this->deleteKeyObject(FileInfo_,
						  parentNodePage,
						  srcParentNodeKeyInfoIndex,
						  NodeDepth_ - 1,
						  AttachNodePages_,
						  AllocateNodePageIDs_,
						  FreeNodePageIDs_,
						  ValueFile_,
						  AttachValuePages_,
						  false); // リーフページではない

	if (this->m_CatchMemoryExhaust && parentNodePage != 0)
	{
		this->m_pPhysicalFile->detachPage(
			parentNodePage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}
}

//
//	FUNCTION private
//	Btree::File::shiftKeyInfoForExpunge --
//		削除のためにキーテーブル内のキー情報をシフトする
//
//	NOTES
//	削除するキーオブジェクトへ辿ることができるキー情報を削除するために、
//	キーテーブル内での後ろのキー情報を前詰する。
//
//	ARGUMENTS
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32		UseKeyInfoNum_
//		ノードページ内の使用中のキー情報数
//	const ModUInt32		KeyInfoIndex_
//		キー情報インデックス
//	const bool			IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	Btree::ValueFile*	ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&	AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::shiftKeyInfoForExpunge(PhysicalFile::Page*	KeyInfoPage_,
							 const ModUInt32		UseKeyInfoNum_,
							 const ModUInt32		KeyInfoIndex_,
							 const bool				IsLeafPage_,
							 ValueFile*				ValueFile_,
							 PageVector&			AttachValuePages_) const
{
	; _SYDNEY_ASSERT(UseKeyInfoNum_ > 1);

	ModUInt32	lastKeyInfoIndex = UseKeyInfoNum_ - 1;

	; _SYDNEY_ASSERT(KeyInfoIndex_ <= lastKeyInfoIndex);

	if (KeyInfoIndex_ == lastKeyInfoIndex)
	{
		return;
	}

	ModUInt32	srcKeyInfoIndex = KeyInfoIndex_ + 1;

	KeyInformation	srcKeyInfo(this->m_pTransaction,
							   KeyInfoPage_,
							   srcKeyInfoIndex,
							   IsLeafPage_);

	KeyInformation	dstKeyInfo(this->m_pTransaction,
							   KeyInfoPage_,
							   srcKeyInfoIndex - 1,
							   IsLeafPage_);

	PhysicalFile::PageID	keyInfoPageID = KeyInfoPage_->getID();

	PhysicalFile::Page*	valueObjectPage = 0;

	do
	{
		srcKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex);

		dstKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex - 1);

		dstKeyInfo.writeKeyObjectID(srcKeyInfo.readKeyObjectID());

		if (IsLeafPage_)
		{
			ModUInt64	srcValueObjectID = srcKeyInfo.readValueObjectID();

			dstKeyInfo.writeValueObjectID(srcValueObjectID);

			ValueFile_->update(srcValueObjectID,
							   keyInfoPageID,
							   srcKeyInfoIndex - 1,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
		}
		else
		{
			dstKeyInfo.writeChildNodePageID(
				srcKeyInfo.readChildNodePageID());
		}

		srcKeyInfoIndex++;
	}
	while (srcKeyInfoIndex <= lastKeyInfoIndex);
}

//
//	FUNCTION private
//	Btree::File::freeNodePage -- ノードページを解放する
//
//	NOTES
//	ノードページを解放する。
//	解放するノードページが複数の物理ページで構成されている場合には、
//	ノードページを構成しているすべての物理ページを解放する。
//
//	ARGUMENTS
//	PhysicalFile::Page*&	TopNodePage_
//		ノードページの物理ページ記述子
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//	Btree::PageIDVector&	FreeNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（フリーしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::freeNodePage(PhysicalFile::Page*&	TopNodePage_,
				   PageVector&			AttachNodePages_,
				   PageIDVector&		FreeNodePageIDs_,
				   const bool			IsLeafPage_) const
{
	PhysicalFile::Page*		nodePage = TopNodePage_;
	PhysicalFile::PageID	nodePageID = nodePage->getID();

	while (nodePageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		; _SYDNEY_ASSERT(nodePage != 0);

		NodePageHeader	nodePageHeader(this->m_pTransaction,
									   nodePage,
									   IsLeafPage_);

		PhysicalFile::PageID	nextPhysicalPageID =
			nodePageHeader.readNextPhysicalPageID();

		PhysicalFile::AreaID	freeAreaID = NodePageHeader::AreaID;

		while (freeAreaID != PhysicalFile::ConstValue::UndefinedAreaID)
		{
			nodePage->freeArea(*this->m_pTransaction, freeAreaID);

			freeAreaID = nodePage->getNextAreaID(*this->m_pTransaction,
												 freeAreaID);
		}

		this->detachPage(AttachNodePages_,
						 nodePage,
						 PhysicalFile::Page::UnfixMode::Dirty,
						 false);

		FreeNodePageIDs_.pushBack(nodePageID);

		this->m_pPhysicalFile->freePage(*this->m_pTransaction,
										nodePageID);

		nodePageID = nextPhysicalPageID;

		if (nodePageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			nodePage = File::attachPage(this->m_pTransaction,
										this->m_pPhysicalFile,
										nodePageID,
										this->m_FixMode,
										this->m_CatchMemoryExhaust,
										AttachNodePages_);
		}
	}

	TopNodePage_ = 0;
}

//
//	FUNCTION private
//	Btree::File::freeKeyObjectArea --
//		キーオブジェクトが記録されている物理エリアを解放する
//
//	NOTES
//	キーオブジェクトが記録されている物理エリアを解放する。
//
//	ARGUMENTS
//	PhysicalFile::Page*			DirectKeyObjectPage_
//		代表キーオブジェクトが記録されているノードページの物理ページ記述子
//	const PhysicalFile::AreaID	DirectKeyObjectAreaID_
//		代表キーオブジェクトが記録されている物理エリアの識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&		FreeNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（フリーしたノードページの物理ページ識別子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::freeKeyObjectArea(
	PhysicalFile::Page*			DirectKeyObjectPage_,
	const PhysicalFile::AreaID	DirectKeyObjectAreaID_,
	PageVector&					AttachNodePages_,
	PageIDVector&				FreeNodePageIDs_) const
{
	if (this->m_cFileParameter.m_ExistOutsideFieldInKey)
	{
		const File::ObjectType*	objectType =
			static_cast<const File::ObjectType*>(
				File::getConstAreaTop(DirectKeyObjectPage_,
									  DirectKeyObjectAreaID_));

		; _SYDNEY_ASSERT((*objectType & File::NormalObjectType) != 0);

		for (int i = 1;
			 i < this->m_cFileParameter.m_TopValueFieldIndex;
			 i++)
		{
			if (*(this->m_cFileParameter.m_FieldOutsideArray + i))
			{
				char*	objectIDReadPos =
					static_cast<char*>(
						this->getFieldPointer(
							DirectKeyObjectPage_,
							DirectKeyObjectAreaID_,
							i,
							true)); // キーオブジェクト

				if (objectIDReadPos == 0)
				{
					// フィールドの値として、
					// ヌルが記録されている。
					// この場合、外置きオブジェクトの物理エリアは
					// 存在しない。

					continue;
				}

				ModUInt64	objectID;
				File::readObjectID(objectIDReadPos, objectID);

				// ※ キーフィールドに配列フィールドは存在しない。
				// 　 キーフィールドで外置きなのは、
				// 　 可変長フィールドオブジェクトのみである。

				File::freeVariableFieldObjectArea(
					this->m_pTransaction,
					objectID,
					DirectKeyObjectPage_,
					this->m_FixMode,
					this->m_CatchMemoryExhaust,
					AttachNodePages_,
					FreeNodePageIDs_);

			} // end if

		} // end for i

	} // end if

	DirectKeyObjectPage_->freeArea(*this->m_pTransaction,
								   DirectKeyObjectAreaID_);
}

//
//	Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
