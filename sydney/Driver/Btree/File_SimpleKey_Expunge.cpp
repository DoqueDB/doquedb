// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_SimpleKey_Expunge.cpp --
//		Ｂ＋木ファイルクラスの実現ファイル（削除関連）
//		※ キーフィールドの値が、キーオブジェクトではなく
//		　 キー情報に記録されているタイプのファイル用のメソッド群
// 
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
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
#include "Btree/File.h"

#include "Exception/NotSupported.h"

#include "Common/Assert.h"

#include "Btree/FileInformation.h"
#include "Btree/ValueFile.h"
#include "Btree/NodePageHeader.h"
#include "Btree/KeyInformation.h"

_SYDNEY_USING

using namespace Btree;

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::File::deleteSimpleKey -- キー情報を削除する
//
//	NOTES
//	キー情報を削除する。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	PhysicalFile::Page*&	NodePage_
//		削除するキー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32			KeyInfoIndex_
//		削除するキー情報のインデックス
//	const ModUInt32			NodeDepth_
//		削除するキー情報が記録されているノードページの階層
//		（ルートノードページから、削除するキー情報が記録されている
//		　ノードページまでの段数。ルートノードページが1。）
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
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
File::deleteSimpleKey(FileInformation&		FileInfo_,
					  PhysicalFile::Page*&	NodePage_,
					  const ModUInt32		KeyInfoIndex_,
					  const ModUInt32		NodeDepth_,
					  PageVector&			AttachNodePages_,
					  PageIDVector&			FreeNodePageIDs_,
					  ValueFile*			ValueFile_,
					  PageVector&			AttachValuePages_,
					  const bool			IsLeafPage_) const
{
	PhysicalFile::PageID	nodePageID = NodePage_->getID();

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   NodePage_,
								   IsLeafPage_);

	ModUInt32	useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	ModUInt32	lastUseKeyInfoIndex = useKeyInfoNum - 1;

	bool	doFreePage = false;

	if (KeyInfoIndex_ == lastUseKeyInfoIndex)
	{
		// ノード内の最後のキーを削除…

		if (useKeyInfoNum == 1)
		{
			// しかも、ノード内の唯一存在していたキーを削除…

			doFreePage = true;

			if (NodeDepth_ > 1)
			{
				// ルートノードページ以外のキーを削除…

				PhysicalFile::Page*	parentNodePage = 0;
				ModUInt32			parentNodeKeyInfoIndex = ModUInt32Max;

				this->searchKeyInformationSimpleKey(NodePage_,
													parentNodePage,
													parentNodeKeyInfoIndex,
													AttachNodePages_,
													IsLeafPage_);

				this->deleteSimpleKey(FileInfo_,
									  parentNodePage,
									  parentNodeKeyInfoIndex,
									  NodeDepth_ - 1,
									  AttachNodePages_,
									  FreeNodePageIDs_,
									  ValueFile_,
									  AttachValuePages_,
									  false); // リーフページではない

				if (this->m_CatchMemoryExhaust && parentNodePage != 0)
				{
					this->m_pPhysicalFile->detachPage(
						parentNodePage,
						PhysicalFile::Page::UnfixMode::Dirty,
						false); // 本当にデタッチ（アンフィックス）
						        // してしまい、
						        // 物理ファイルマネージャが
						        // キャッシュしないようにする。
				}
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
		}
		else
		{
			// ノード内に複数キーが存在しており、
			// その中での最後のキーを削除した…

			nodePageHeader.decUseKeyInformationNumber();

			if (NodeDepth_ > 1)
			{
				this->rewriteParentNodeSimpleKey(
					NodePage_,
					IsLeafPage_,
					NodeDepth_ - 1,
					AttachNodePages_);
			}
		}
	}
	else
	{
		// ノード内の最後ではないキーを削除…

		this->shiftKeyInfoForSimpleKeyExpunge(NodePage_,
											  useKeyInfoNum,
											  KeyInfoIndex_,
											  IsLeafPage_,
											  ValueFile_,
											  AttachValuePages_);

		nodePageHeader.decUseKeyInformationNumber();
	}

	if (doFreePage == false &&
		useKeyInfoNum > 1 &&
		useKeyInfoNum - 1 ==
		this->m_cFileParameter.m_NodeMergeCheckThreshold)
	{
		// ノードマージを実行するかチェックする条件が整った…

		// 可能ならばノードページをマージする
		this->mergeSimpleNodePage(FileInfo_,
								  NodeDepth_,
								  NodePage_,
								  nodePageHeader,
								  IsLeafPage_,
								  AttachNodePages_,
								  FreeNodePageIDs_,
								  ValueFile_,
								  AttachValuePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::mergeSimpleNodePage -- ノードページをマージする
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
File::mergeSimpleNodePage(FileInformation&		FileInfo_,
						  const ModUInt32		NodeDepth_,
						  PhysicalFile::Page*&	SrcNodePage_,
						  NodePageHeader&		SrcNodePageHeader_,
						  const bool			IsLeafPage_,
						  PageVector&			AttachNodePages_,
						  PageIDVector&			FreeNodePageIDs_,
						  ValueFile*			ValueFile_,
						  PageVector&			AttachValuePages_) const
{
	if (NodeDepth_ == 1)
	{
		// ルートノードページ…

		// ルートノードページはマージできない。

		return;
	}

	// マージする相手のノードページを検索する
	bool	partnerIsPrev = false;
	PhysicalFile::Page*	dstNodePage =
		this->searchMergeNodePage(SrcNodePage_,
								  SrcNodePageHeader_,
								  IsLeafPage_,
								  AttachNodePages_,
								  partnerIsPrev);

	if (dstNodePage == 0)
	{
		// マージ相手が見つからなかった…

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
		this->mergeWithNextSimpleNodePage(FileInfo_,
										  NodeDepth_,
										  partnerIsPrev ?
											  dstNodePage : SrcNodePage_,
										  partnerIsPrev ?
											  SrcNodePage_ : dstNodePage,
										  IsLeafPage_,
										  AttachNodePages_,
										  FreeNodePageIDs_,
										  ValueFile_,
										  AttachValuePages_);

		freeIsSrc = (partnerIsPrev == false);
	}
	else
	{
		this->mergeWithPrevSimpleNodePage(FileInfo_,
										  NodeDepth_,
										  partnerIsPrev ?
											  SrcNodePage_ : dstNodePage,
										  partnerIsPrev ?
											  dstNodePage : SrcNodePage_,
										  IsLeafPage_,
										  AttachNodePages_,
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
//	Btree::File::mergeWithPrevSimpleNodePage --
//		前のノードページとマージする
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
File::mergeWithPrevSimpleNodePage(
	FileInformation&	FileInfo_,
	const ModUInt32		NodeDepth_,
	PhysicalFile::Page*	SrcNodePage_,
	PhysicalFile::Page*	DstNodePage_,
	const bool			IsLeafPage_,
	PageVector&			AttachNodePages_,
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

	this->searchKeyInformationSimpleKey(SrcNodePage_,
										parentNodePage,
										srcParentNodeKeyInfoIndex,
										AttachNodePages_,
										IsLeafPage_);

	; _SYDNEY_ASSERT(parentNodePage != 0);
	; _SYDNEY_ASSERT(srcParentNodeKeyInfoIndex > 0);

#ifdef DEBUG

	//
	// 子ノードページ同士の前後関係をチェック
	//

	PhysicalFile::Page*	dstParentNodePage = 0;
	ModUInt32			dstParentNodeKeyInfoIndex = ModUInt32Max;

	this->searchKeyInformationSimpleKey(DstNodePage_,
										dstParentNodePage,
										dstParentNodeKeyInfoIndex,
										AttachNodePages_,
										IsLeafPage_);

	; _SYDNEY_ASSERT(
		dstParentNodePage->getID() == parentNodePage->getID());
	; _SYDNEY_ASSERT(
		dstParentNodeKeyInfoIndex == srcParentNodeKeyInfoIndex - 1);

	checkMemoryExhaust(dstParentNodePage);

#endif

	//
	// マージするノードページのキーテーブルを
	// マージ先のノードページに移動（コピー）する。
	//

	KeyInformation	srcKeyInfo(this->m_pTransaction,
							   SrcNodePage_,
							   0, // キー情報のインデックス
							   IsLeafPage_,
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	KeyInformation	dstKeyInfo(this->m_pTransaction,
							   DstNodePage_,
							   dstNodeUseKeyNum,
							   IsLeafPage_,
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	PhysicalFile::Page*	valueObjectPage = 0;

	bool	childNodeIsLeaf =
		((NodeDepth_ + 1) == FileInfo_.readTreeDepth());

	for (ModUInt32 i = 0; i < srcNodeUseKeyNum; i++)
	{
		srcKeyInfo.setStartOffsetByIndex(i);

		dstKeyInfo.setStartOffsetByIndex(dstNodeUseKeyNum + i);

		dstKeyInfo.copy(srcKeyInfo);

		if (IsLeafPage_)
		{
			// マージ対象のノードページがリーフページ…

			// バリューオブジェクトに記録されている
			// リーフページの情報を更新する
			ValueFile_->update(dstKeyInfo.readValueObjectID(),
							   dstNodePageID,
							   dstNodeUseKeyNum + i,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
		}
		else
		{
			// マージ対象のノードページがリーフページではない…

			// 子ノードページヘッダに記録されている
			// 「親ノードページの物理ページ識別子」を更新する

			PhysicalFile::PageID	childNodePageID =
				dstKeyInfo.readChildNodePageID();

			PhysicalFile::Page*	childNodePage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 dstKeyInfo.readChildNodePageID(),
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
									 false, // リーフページではない
									 this->m_cFileParameter.m_KeyNum,
									 this->m_cFileParameter.m_KeySize);

	KeyInformation	dstParentKeyInfo(this->m_pTransaction,
									 parentNodePage,
									 srcParentNodeKeyInfoIndex - 1,
									 false, // リーフページではない
									 this->m_cFileParameter.m_KeyNum,
									 this->m_cFileParameter.m_KeySize);

	dstParentKeyInfo.copy(srcParentKeyInfo);

	dstParentKeyInfo.writeChildNodePageID(dstNodePageID);

	//
	// 親ノードページに記録されている、マージ元の
	// ノードページの代表キーを削除する。
	//

	this->deleteSimpleKey(FileInfo_,
						  parentNodePage,
						  srcParentNodeKeyInfoIndex,
						  NodeDepth_ - 1,
						  AttachNodePages_,
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
//	Btree::File::mergeWithNextSimpleNodePage --
//		後ろのノードページとマージする
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
File::mergeWithNextSimpleNodePage(
	FileInformation&	FileInfo_,
	const ModUInt32		NodeDepth_,
	PhysicalFile::Page*	SrcNodePage_,
	PhysicalFile::Page*	DstNodePage_,
	const bool			IsLeafPage_,
	PageVector&			AttachNodePages_,
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

	this->searchKeyInformationSimpleKey(SrcNodePage_,
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
							   IsLeafPage_,
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	KeyInformation	dstKeyInfo(this->m_pTransaction,
							   DstNodePage_,
							   dstKeyInfoIndex,
							   IsLeafPage_,
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	PhysicalFile::Page*	valueObjectPage = 0;

	bool	childNodeIsLeaf =
		((NodeDepth_ + 1) == FileInfo_.readTreeDepth());

	while (true)
	{
		srcKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex);

		dstKeyInfo.setStartOffsetByIndex(dstKeyInfoIndex);

		dstKeyInfo.copy(srcKeyInfo);

		if (IsLeafPage_)
		{
			// マージ対象のノードページがリーフページ…

			// バリューオブジェクトに記録されている
			// リーフページの情報を更新する
			ValueFile_->update(dstKeyInfo.readValueObjectID(),
							   dstNodePageID,
							   dstKeyInfoIndex,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
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

		dstKeyInfo.copy(srcKeyInfo);

		if (IsLeafPage_)
		{
			// マージ対象のノードページがリーフページ…

			// バリューオブジェクトに記録されている
			// リーフページの情報を更新する
			ValueFile_->update(dstKeyInfo.readValueObjectID(),
							   dstNodePageID,
							   i,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
		}
		else
		{
			// マージ対象のノードページがリーフページではない…

			// 子ノードページヘッダに記録されている
			// 「親ノードページの物理ページ識別子」を更新する

			PhysicalFile::PageID	childNodePageID =
				dstKeyInfo.readChildNodePageID();

			PhysicalFile::Page*	childNodePage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 dstKeyInfo.readChildNodePageID(),
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
		// マージしたノードページがリーフページ…

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

	this->deleteSimpleKey(FileInfo_,
						  parentNodePage,
						  srcParentNodeKeyInfoIndex,
						  NodeDepth_ - 1,
						  AttachNodePages_,
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
//	Btree::File::shiftKeyInfoForSimpleKeyExpunge --
//		削除のためにキーテーブル内のキー情報をシフトする
//
//	NOTES
//	削除のためにキーテーブル内のキー情報をシフトする。
//
//	ARGUMENTS
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32		UseKeyInfoNum_
//		使用中のキー情報数
//	const ModUInt32		KeyInfoIndex_
//		削除するキー情報のインデックス
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
File::shiftKeyInfoForSimpleKeyExpunge(
	PhysicalFile::Page*	KeyInfoPage_,
	const ModUInt32		UseKeyInfoNum_,
	const ModUInt32		KeyInfoIndex_,
	const bool			IsLeafPage_,
	ValueFile*			ValueFile_,
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
							   IsLeafPage_,
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	KeyInformation	dstKeyInfo(this->m_pTransaction,
							   KeyInfoPage_,
							   srcKeyInfoIndex - 1,
							   IsLeafPage_,
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	PhysicalFile::PageID	keyInfoPageID = KeyInfoPage_->getID();

	PhysicalFile::Page*	valueObjectPage = 0;

	do
	{
		srcKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex);

		dstKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex - 1);

		dstKeyInfo.copy(srcKeyInfo);

		if (IsLeafPage_)
		{
			ModUInt64	dstDataObjectID = dstKeyInfo.readValueObjectID();

			ValueFile_->update(dstDataObjectID,
							   keyInfoPageID,
							   srcKeyInfoIndex - 1,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
		}

		srcKeyInfoIndex++;
	}
	while (srcKeyInfoIndex <= lastKeyInfoIndex);
}

//
//	Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
