// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_SimpleKey_Insert.cpp --
//		Ｂ＋木ファイルクラスの実現ファイル（挿入関連）
//		※ キーフィールドの値が、キーオブジェクトではなく
//		　 キー情報に記録されているタイプのファイル用のメソッド群
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
#include "Btree/File.h"

#include "Exception/UniquenessViolation.h"
#include "Exception/NotSupported.h"

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
//	Btree::File::insertSimpleKey --
//		ツリーファイルにキーフィールドを挿入する
//
//	NOTES
//	ツリーファイルにキーフィールドを挿入する。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Common::DataArrayData*	Object_
//		挿入オブジェクトへのポインタ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const bool				DoUniqueCheck_ = false
//		ユニークチェックを行うかどうか
//			true  : ユニークチェックを行う
//			false : ユニークチェックを行わない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::insertSimpleKey(
	FileInformation&		FileInfo_,
	ValueFile*				ValueFile_,
	Common::DataArrayData*	Object_,
	PageVector&				AttachNodePages_,
	PageIDVector&			AllocateNodePageIDs_,
	PageVector&				AttachValuePages_,
	const bool				DoUniqueCheck_ // = false
	) const
{
	ModUInt32	treeDepth = FileInfo_.readTreeDepth();

	PhysicalFile::PageID	rootNodePageID = FileInfo_.readRootNodePageID();

	PhysicalFile::Page*	leafPage =
		this->searchLeafPageForSimpleKeyInsert(treeDepth,
											   rootNodePageID,
											   AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	if (DoUniqueCheck_)
	{
		this->uniqueCheckSimpleKey(FileInfo_,
								   ValueFile_,
								   Object_,
								   leafPage->getID(),
								   AttachNodePages_,
								   AttachValuePages_);
	}

	this->insertSimpleKey(FileInfo_,
						  ValueFile_,
						  treeDepth,
						  leafPage,
						  Object_,
						  AttachNodePages_,
						  AllocateNodePageIDs_,
						  AttachValuePages_,
						  this->m_LeafPageID,
						  this->m_KeyInfoIndex,
						  true);  // リーフページ

	if (this->m_CatchMemoryExhaust)
	{
		this->m_pPhysicalFile->detachPage(
			leafPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）
				    // してしまい、
				    // 物理ファイルマネージャが
				    // キャッシュしないようにする。
	}
}

//
//	FUNCTION private
//	Btree::File::insertSimpleKey --
//		ノードページにキーフィールドを挿入する
//
//	NOTES
//	ノードページにキーフィールドを挿入する。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	const ModUInt32			NodeDepth_
//		キーフィールドを挿入するノードページの階層
//		（何段目のノードに挿入するのか）
//	PhysicalFile::Page*		TopNodePage_
//		ノードページの物理ページ記述子
//	Common::DataArrayData*	Object_
//		挿入するオブジェクトへのポインタ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	PhysicalFile::PageID&	KeyInfoNodePageID_
//		キー情報が記録されているノードページの物理ページ識別子
//	ModUInt32&				KeyInfoIndex_
//		キー情報のインデックス
//	const bool				IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	PhysicalFile::Page*		PrevKeyInfoChildNodePage_ = 0
//		直前のキー情報が記録されている子ノードページの物理ページ記述子
//		（このノードページの親ノードページから、
//		　このノードページに辿ることができるキー情報を探し出し、
//		　その直後のキー情報から辿る子ノードページに、
//		　キーフィールドを挿入する際に指定する。）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::insertSimpleKey(
	FileInformation&		FileInfo_,
	ValueFile*				ValueFile_,
	const ModUInt32			NodeDepth_,
	PhysicalFile::Page*		TopNodePage_,
	Common::DataArrayData*	Object_,
	PageVector&				AttachNodePages_,
	PageIDVector&			AllocateNodePageIDs_,
	PageVector&				AttachValuePages_,
	PhysicalFile::PageID&	KeyInfoNodePageID_,
	ModUInt32&				KeyInfoIndex_,
	const bool				IsLeafPage_,
	PhysicalFile::Page*		PrevKeyInfoChildNodePage_ // = 0
	) const
{
	PhysicalFile::Page*	newNodePage = 0;

	bool	split = this->splitSimpleKeyTable(FileInfo_,
											  TopNodePage_,
											  AttachNodePages_,
											  AllocateNodePageIDs_,
											  newNodePage,
											  IsLeafPage_,
											  ValueFile_,
											  AttachValuePages_);

	PhysicalFile::Page*	topNodePage = 0;

	if (split)
	{
		; _SYDNEY_ASSERT(newNodePage != 0);

		if (PrevKeyInfoChildNodePage_ != 0)
		{
			NodePageHeader	childNodePageHeader(this->m_pTransaction,
												PrevKeyInfoChildNodePage_,
												IsLeafPage_);

			PhysicalFile::PageID	parentNodePageID =
				childNodePageHeader.readParentNodePageID();

			; _SYDNEY_ASSERT(parentNodePageID !=
							 PhysicalFile::ConstValue::UndefinedPageID);

			if (parentNodePageID == TopNodePage_->getID())
			{
				topNodePage = TopNodePage_;
			}
			else if (parentNodePageID == newNodePage->getID())
			{
				topNodePage = newNodePage;
			}
			else
			{
				topNodePage = File::attachPage(this->m_pTransaction,
											   this->m_pPhysicalFile,
											   parentNodePageID,
											   this->m_FixMode,
											   this->m_CatchMemoryExhaust,
											   AttachNodePages_);
			}
		}
		else
		{
			NodePageHeader	nodePageHeader(this->m_pTransaction,
										   TopNodePage_,
										   IsLeafPage_);

			const ModUInt32 useKeyInfoNum =
				nodePageHeader.readUseKeyInformationNumber();

			KeyInformation	keyInfo(this->m_pTransaction,
									TopNodePage_,
									useKeyInfoNum - 1,
									IsLeafPage_,
									this->m_cFileParameter.m_KeyNum,
									this->m_cFileParameter.m_KeySize);

			const NullBitmap::Value*	nullBitmapTop =
				keyInfo.assignConstKeyNullBitmap();

			; _SYDNEY_ASSERT(nullBitmapTop != 0);

			if (this->compareToFetchCondition(nullBitmapTop) < 0)
			{
				topNodePage = TopNodePage_;
			}
			else
			{
				topNodePage = newNodePage;
			}
		}
	}
	else
	{
		topNodePage = TopNodePage_;
	}

	KeyInfoNodePageID_ = topNodePage->getID();

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   topNodePage,
								   IsLeafPage_);

	ModUInt32	useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	KeyInfoIndex_ =
		this->getKeyInformationIndexForSimpleKeyInsert(
			topNodePage,
			useKeyInfoNum,
			AttachNodePages_,
			IsLeafPage_,
			false, // 子ノードを探すためではない
			PrevKeyInfoChildNodePage_);

	this->shiftKeyInfoForSimpleKeyInsert(topNodePage,
										 useKeyInfoNum,
										 KeyInfoIndex_,
										 IsLeafPage_,
										 ValueFile_,
										 AttachValuePages_);

	this->writeSimpleKey(topNodePage,
						 KeyInfoIndex_,
						 Object_,
						 IsLeafPage_);

	nodePageHeader.incUseKeyInformationNumber();

	if (split)
	{
		this->resetParentSimpleNodePage(FileInfo_,
										ValueFile_,
										NodeDepth_,
										TopNodePage_,
										newNodePage,
										AttachNodePages_,
										AllocateNodePageIDs_,
										AttachValuePages_,
										IsLeafPage_);
	}
	else
	{
		ModUInt32	useKeyInfoNum =
			nodePageHeader.readUseKeyInformationNumber();

		if (KeyInfoIndex_ == useKeyInfoNum - 1)
		{
			this->resetParentNodeSimpleKey(topNodePage,
										   KeyInfoIndex_,
										   NodeDepth_,
										   AttachNodePages_,
										   IsLeafPage_);
		}
	}

	if (this->m_CatchMemoryExhaust)
	{
		if (newNodePage != 0 && newNodePage != topNodePage)
		{
			this->m_pPhysicalFile->detachPage(
				newNodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}

		if (topNodePage != TopNodePage_)
		{
			this->m_pPhysicalFile->detachPage(
				topNodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}
	}
}

//
//	FUNCTION private
//	Btree::File::resetParentNodeSimpleKey -- 親ノードページのキー値を更新する
//
//	NOTES
//	親ノードページのキー値を更新する。
//
//	ARGUMENTS
//	PhysicalFile::Page*	ChildNodePage_
//		子ノードページの物理ページ記述子
//	const ModUInt32		ChildNodeKeyInfoIndex_
//		子ノードページのキー情報のインデックス
//	const ModUInt32		ParentNodeDepth_
//		親ノードページの階層（ルートノードページが1）
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool			ChildNodeIsLeafPage_
//		子ノードページがリーフページかどうか
//			true  : 子ノードページがリーフページ
//			false : 子ノードページがリーフページ以外のノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::resetParentNodeSimpleKey(
	PhysicalFile::Page*	ChildNodePage_,
	const ModUInt32		ChildNodeKeyInfoIndex_,
	const ModUInt32		ParentNodeDepth_,
	PageVector&			AttachNodePages_,
	const bool			ChildNodeIsLeafPage_) const
{
	KeyInformation	childNodeKeyInfo(this->m_pTransaction,
									 ChildNodePage_,
									 ChildNodeKeyInfoIndex_,
									 ChildNodeIsLeafPage_,
									 this->m_cFileParameter.m_KeyNum,
									 this->m_cFileParameter.m_KeySize);

	const NullBitmap::Value*	nullBitmapTop =
		childNodeKeyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	this->resetParentNodeSimpleKey(ChildNodePage_,
								   nullBitmapTop,
								   ParentNodeDepth_,
								   AttachNodePages_,
								   ChildNodeIsLeafPage_);
}

//
//	FUNCTION private
//	Btree::File::resetParentSimpleNodePage -- 親ノードページを更新する
//
//	NOTES
//	分割が発生した子ノードページの親ノードページを更新する。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	const ModUInt32			ChildNodeDepth_
//		子ノードページの階層（ルートノードページが1）
//	PhysicalFile::Page*		ChildNodePage1_
//		子ノードページの物理ページ記述子
//	PhysicalFile::Page*		ChildNodePage2_
//		子ノードページの物理ページ記述子
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const bool				ChildNodeIsLeafPage_
//		子ノードページがリーフページかどうか
//			true  : 子ノードページがリーフページ
//			false : 子ノードページがリーフページ以外のノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::resetParentSimpleNodePage(
	FileInformation&	FileInfo_,
	ValueFile*			ValueFile_,
	const ModUInt32		ChildNodeDepth_,
	PhysicalFile::Page*	ChildNodePage1_,
	PhysicalFile::Page*	ChildNodePage2_,
	PageVector&			AttachNodePages_,
	PageIDVector&		AllocateNodePageIDs_,
	PageVector&			AttachValuePages_,
	const bool			ChildNodeIsLeafPage_) const
{
	if (ChildNodeDepth_ == 1)
	{
		// ルートノードページに分割が起こった

		PhysicalFile::Page*	newRootNodePage =
			this->createNodePage(AttachNodePages_,
								 AllocateNodePageIDs_,
								 false); // リーフページではない

		FileInfo_.incTreeDepth();

		FileInfo_.writeRootNodePageID(newRootNodePage->getID());

		PhysicalFile::Page*	childNodePages[2];

		childNodePages[0] = ChildNodePage1_;
		childNodePages[1] = ChildNodePage2_;

		for (int i = 0; i < 2; i++)
		{
			NodePageHeader	childNodePageHeader(this->m_pTransaction,
												childNodePages[i],
												ChildNodeIsLeafPage_);

			ModUInt32	childNodeUseKeyInfoNum =
				childNodePageHeader.readUseKeyInformationNumber();

			KeyInformation
				childNodeLastKeyInfo(this->m_pTransaction,
									 childNodePages[i],
									 childNodeUseKeyInfoNum - 1,
									 ChildNodeIsLeafPage_,
									 this->m_cFileParameter.m_KeyNum,
									 this->m_cFileParameter.m_KeySize);

			const NullBitmap::Value*	nullBitmapTop =
				childNodeLastKeyInfo.assignConstKeyNullBitmap();

			; _SYDNEY_ASSERT(nullBitmapTop != 0);

			PhysicalFile::PageID	keyInfoPageID =
				PhysicalFile::ConstValue::UndefinedPageID;
			ModUInt32				keyInfoIndex = ModUInt32Max;

			this->insertSimpleKey(FileInfo_,
								  ValueFile_,
								  ChildNodeDepth_,
								  newRootNodePage,
								  nullBitmapTop,
								  AttachNodePages_,
								  AllocateNodePageIDs_,
								  AttachValuePages_,
								  keyInfoPageID,
								  keyInfoIndex,
								  false); // リーフページではない

			; _SYDNEY_ASSERT(keyInfoPageID !=
							 PhysicalFile::ConstValue::UndefinedPageID);

			PhysicalFile::Page*	keyInfoPage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 keyInfoPageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			KeyInformation	keyInfo(this->m_pTransaction,
									keyInfoPage,
									keyInfoIndex,
									false, // リーフページではない
									this->m_cFileParameter.m_KeyNum,
									this->m_cFileParameter.m_KeySize);

			keyInfo.writeChildNodePageID((childNodePages[i])->getID());

			if (this->m_CatchMemoryExhaust)
			{
				this->m_pPhysicalFile->detachPage(
					keyInfoPage,
					PhysicalFile::Page::UnfixMode::Dirty,
					false); // 本当にデタッチ（アンフィックス）してしまう
			}

			childNodePageHeader.writeParentNodePageID(
				newRootNodePage->getID());
		}


		if (this->m_CatchMemoryExhaust)
		{
			this->m_pPhysicalFile->detachPage(
				newRootNodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}
	}
	else
	{
		this->rewriteParentNodeSimpleKey(ChildNodePage1_,
										 ChildNodeIsLeafPage_,
										 0,
										 AttachNodePages_);

		NodePageHeader	childNodePageHeader(this->m_pTransaction,
											ChildNodePage2_,
											ChildNodeIsLeafPage_);

		ModUInt32	useKeyInfoNum =
			childNodePageHeader.readUseKeyInformationNumber();

		; _SYDNEY_ASSERT(useKeyInfoNum > 0);

		KeyInformation	childNodeKeyInfo(this->m_pTransaction,
										 ChildNodePage2_,
										 useKeyInfoNum - 1,
										 ChildNodeIsLeafPage_,
										 this->m_cFileParameter.m_KeyNum,
										 this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			childNodeKeyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		PhysicalFile::Page*	parentNodePage = 0;
		ModUInt32			parentNodeKeyInfoIndex = ModUInt32Max;

		this->searchKeyInformationSimpleKey(ChildNodePage1_,
											parentNodePage,
											parentNodeKeyInfoIndex,
											AttachNodePages_,
											ChildNodeIsLeafPage_);

		; _SYDNEY_ASSERT(parentNodePage != 0);

		PhysicalFile::PageID	parentNodePageID =
			PhysicalFile::ConstValue::UndefinedPageID;

		this->insertSimpleKey(FileInfo_,
							  ValueFile_,
							  ChildNodeDepth_ - 1,
							  parentNodePage,
							  nullBitmapTop,
							  AttachNodePages_,
							  AllocateNodePageIDs_,
							  AttachValuePages_,
							  parentNodePageID,
							  parentNodeKeyInfoIndex,
							  false, // リーフページではない
							  ChildNodePage1_);

		; _SYDNEY_ASSERT(parentNodePageID !=
						 PhysicalFile::ConstValue::UndefinedPageID);
		; _SYDNEY_ASSERT(parentNodeKeyInfoIndex != ModUInt32Max);

		if (this->m_CatchMemoryExhaust)
		{
			this->m_pPhysicalFile->detachPage(
				parentNodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまう
		}

		parentNodePage = File::attachPage(this->m_pTransaction,
										  this->m_pPhysicalFile,
										  parentNodePageID,
										  this->m_FixMode,
										  this->m_CatchMemoryExhaust,
										  AttachNodePages_);

		KeyInformation	parentNodeKeyInfo(this->m_pTransaction,
										  parentNodePage,
										  parentNodeKeyInfoIndex,
										  false, // リーフページではない
										  this->m_cFileParameter.m_KeyNum,
										  this->m_cFileParameter.m_KeySize);

		parentNodeKeyInfo.writeChildNodePageID(ChildNodePage2_->getID());

		if (this->m_CatchMemoryExhaust)
		{
			this->m_pPhysicalFile->detachPage(
				parentNodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまう
		}

		childNodePageHeader.writeParentNodePageID(parentNodePageID);
	}
}

//
//	FUNCTION private
//	Btree::File::rewriteParentNodeSimpleKey -- 親ノードページのキー値を更新する
//
//	NOTES
//	親ノードページのキー値を更新する。
//
//	ARGUMENTS
//	PhysicalFile::Page*	ChildNodePage_
//		子ノードページの物理ページ記述子
//	const bool			ChildNodeIsLeafPage_
//		子ノードページがリーフページかどうか
//			true  : 子ノードページがリーフページ
//			false : 子ノードページがリーフページ以外のノードページ
//	const ModUInt32		ParentNodeDepth_
//		親ノードページの階層（ルートノードページが1）
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::rewriteParentNodeSimpleKey(
	PhysicalFile::Page*	ChildNodePage_,
	const bool			ChildNodeIsLeafPage_,
	const ModUInt32		ParentNodeDepth_,
	PageVector&			AttachNodePages_) const
{
	PhysicalFile::Page*	parentNodePage = 0;
	ModUInt32			parentNodeKeyInfoIndex = ModUInt32Max;

	this->searchKeyInformationSimpleKey(ChildNodePage_,
										parentNodePage,
										parentNodeKeyInfoIndex,
										AttachNodePages_,
										ChildNodeIsLeafPage_);

	; _SYDNEY_ASSERT(parentNodePage != 0);

	NodePageHeader	childNodePageHeader(this->m_pTransaction,
										ChildNodePage_,
										ChildNodeIsLeafPage_);

	ModUInt32	childNodeUseKeyInfoNum =
		childNodePageHeader.readUseKeyInformationNumber();

	KeyInformation	childNodeLastKeyInfo(this->m_pTransaction,
										 ChildNodePage_,
										 childNodeUseKeyInfoNum - 1,
										 ChildNodeIsLeafPage_,
										 this->m_cFileParameter.m_KeyNum,
										 this->m_cFileParameter.m_KeySize);

	const NullBitmap::Value*	nullBitmapTop =
		childNodeLastKeyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	this->writeSimpleKey(parentNodePage,
						 parentNodeKeyInfoIndex,
						 nullBitmapTop,
						 false); // リーフページではない

	if (ParentNodeDepth_ > 1)
	{
		NodePageHeader	nodePageHeader(this->m_pTransaction,
									   parentNodePage,
									   false); // リーフページではない

		if (parentNodeKeyInfoIndex ==
			nodePageHeader.readUseKeyInformationNumber() - 1)
		{
			this->rewriteParentNodeSimpleKey(
				parentNodePage,
				false, // リーフページではない
				ParentNodeDepth_ - 1,
				AttachNodePages_);
		}
	}

	if (this->m_CatchMemoryExhaust)
	{
		this->m_pPhysicalFile->detachPage(
			parentNodePage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまう
	}
}

//
//	FUNCTION private
//	Btree::File::insertSimpleKey --
//		ノードページにキーフィールドを挿入する
//
//	NOTES
//	ノードページにキーフィールドを挿入する。
//
//	ARGUMENTS
//	Btree::FileInformation&			FileInfo_
//		ファイル管理情報への参照
//	Btree::ValueFile*				ValueFile_
//		バリューファイル記述子
//	const ModUInt32					NodeDepth_
//		キーフィールドを挿入するノードページの階層
//		（何段目のノードに挿入するのか）
//		（ルートノードページが1）
//	PhysicalFile::Page*				TopNodePage_
//		ノードページの物理ページ記述子
//	const Btree::NullBitmap::Value*	SrcNullBitmapTop_
//		挿入するキー値が記録されているキー情報内の
//		ヌルビットマップ先頭へのポインタ
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&			AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	PhysicalFile::PageID&			KeyInfoNodePageID_
//		キー情報が記録されているノードページの物理ページ識別子
//	ModUInt32&						KeyInfoIndex_
//		キー情報のインデックス
//	const bool						IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	PhysicalFile::Page*				PrevKeyInfoChildNodePage_ = 0
//		直前のキー情報が記録されている子ノードページの物理ページ記述子
//		（このノードページの親ノードページから、
//		　このノードページに辿ることができるキー情報を探し出し、
//		　その直後のキー情報から辿る子ノードページに、
//		　キーフィールドを挿入する際に指定する。）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::insertSimpleKey(
	FileInformation&			FileInfo_,
	ValueFile*					ValueFile_,
	const ModUInt32				NodeDepth_,
	PhysicalFile::Page*			TopNodePage_,
	const NullBitmap::Value*	SrcNullBitmapTop_,
	PageVector&					AttachNodePages_,
	PageIDVector&				AllocateNodePageIDs_,
	PageVector&					AttachValuePages_,
	PhysicalFile::PageID&		KeyInfoNodePageID_,
	ModUInt32&					KeyInfoIndex_,
	const bool					IsLeafPage_,
	PhysicalFile::Page*			PrevKeyInfoChildNodePage_ // = 0
	) const
{
	PhysicalFile::Page*	newNodePage = 0;

	bool	split = this->splitSimpleKeyTable(FileInfo_,
											  TopNodePage_,
											  AttachNodePages_,
											  AllocateNodePageIDs_,
											  newNodePage,
											  IsLeafPage_,
											  ValueFile_,
											  AttachValuePages_);

	PhysicalFile::Page*	topNodePage = 0;

	if (split)
	{
		; _SYDNEY_ASSERT(newNodePage != 0);

		if (PrevKeyInfoChildNodePage_ != 0)
		{
			NodePageHeader	childNodePageHeader(this->m_pTransaction,
												PrevKeyInfoChildNodePage_,
												IsLeafPage_);

			PhysicalFile::PageID	parentNodePageID =
				childNodePageHeader.readParentNodePageID();

			; _SYDNEY_ASSERT(parentNodePageID !=
							 PhysicalFile::ConstValue::UndefinedPageID);

			if (parentNodePageID == TopNodePage_->getID())
			{
				topNodePage = TopNodePage_;
			}
			else if (parentNodePageID == newNodePage->getID())
			{
				topNodePage = newNodePage;
			}
			else
			{
				topNodePage = File::attachPage(this->m_pTransaction,
											   this->m_pPhysicalFile,
											   parentNodePageID,
											   this->m_FixMode,
											   this->m_CatchMemoryExhaust,
											   AttachNodePages_);
			}
		}
		else
		{
			NodePageHeader	nodePageHeader(this->m_pTransaction,
										   TopNodePage_,
										   IsLeafPage_);

			const ModUInt32 useKeyInfoNum =
				nodePageHeader.readUseKeyInformationNumber();

			KeyInformation	keyInfo(this->m_pTransaction,
									TopNodePage_,
									useKeyInfoNum - 1,
									IsLeafPage_,
									this->m_cFileParameter.m_KeyNum,
									this->m_cFileParameter.m_KeySize);

			const NullBitmap::Value*	dstNullBitmapTop =
				keyInfo.assignConstKeyNullBitmap();

			; _SYDNEY_ASSERT(dstNullBitmapTop != 0);

			if (this->compareSimpleKey(SrcNullBitmapTop_,
									   dstNullBitmapTop)
				< 0)
			{
				topNodePage = TopNodePage_;
			}
			else
			{
				topNodePage = newNodePage;
			}
		}
	}
	else
	{
		topNodePage = TopNodePage_;
	}

	KeyInfoNodePageID_ = topNodePage->getID();

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   topNodePage,
								   IsLeafPage_);

	ModUInt32	useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	KeyInfoIndex_ =
		this->getKeyInformationIndexForSimpleKeyInsert(
			topNodePage,
			useKeyInfoNum,
			AttachNodePages_,
			SrcNullBitmapTop_,
			IsLeafPage_,
			false, // 子ノードを探すためではない
			PrevKeyInfoChildNodePage_);

	this->shiftKeyInfoForSimpleKeyInsert(topNodePage,
										 useKeyInfoNum,
										 KeyInfoIndex_,
										 IsLeafPage_,
										 ValueFile_,
										 AttachValuePages_);

	this->writeSimpleKey(topNodePage,
						 KeyInfoIndex_,
						 SrcNullBitmapTop_,
						 IsLeafPage_);

	nodePageHeader.incUseKeyInformationNumber();

	if (split)
	{
		this->resetParentSimpleNodePage(FileInfo_,
										ValueFile_,
										NodeDepth_,
										TopNodePage_,
										newNodePage,
										AttachNodePages_,
										AllocateNodePageIDs_,
										AttachValuePages_,
										IsLeafPage_);
	}
	else
	{
		ModUInt32	useKeyInfoNum =
			nodePageHeader.readUseKeyInformationNumber();

		if (KeyInfoIndex_ == useKeyInfoNum - 1)
		{
			this->resetParentNodeSimpleKey(topNodePage,
										   SrcNullBitmapTop_,
										   NodeDepth_,
										   AttachNodePages_,
										   IsLeafPage_);
		}
	}

	if (this->m_CatchMemoryExhaust)
	{
		if (newNodePage != 0 && newNodePage != topNodePage)
		{
			this->m_pPhysicalFile->detachPage(
				newNodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}

		if (topNodePage != TopNodePage_)
		{
			this->m_pPhysicalFile->detachPage(
				topNodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}
	}
}

//
//	FUNCTION private
//	Btree::File::resetParentNodeSimpleKey -- 親ノードページのキー値を更新する
//
//	NOTES
//	親ノードページのキー値を更新する。
//
//	ARGUMENTS
//	PhysicalFile::Page*				ChildNodePage_
//		子ノードページの物理ページ記述子
//	const Btree::NullBitmap::Value*	SrcNullBitmapTop_
//		子ノードページ内の、キー値が記録されているキー情報内の
//		ヌルビットマップ先頭へのポインタ
//	const ModUInt32					NodeDepth_
//		親ノードページの階層（ルートノードページが1）
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool						IsLeafPage_
//		子ノードページがリーフページかどうか
//			true  : 子ノードページがリーフページ
//			false : 子ノードページがリーフページ以外のノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::resetParentNodeSimpleKey(
	PhysicalFile::Page*			ChildNodePage_,
	const NullBitmap::Value*	SrcNullBitmapTop_,
	const ModUInt32				NodeDepth_,
	PageVector&					AttachNodePages_,
	const bool					IsLeafPage_) const
{
	if (NodeDepth_ == 1)
	{
		return;
	}

	PhysicalFile::Page*	parentNodePage = 0;
	ModUInt32			keyInfoIndex = ModUInt32Max;

	this->searchKeyInformationSimpleKey(ChildNodePage_,
										parentNodePage,
										keyInfoIndex,
										AttachNodePages_,
										IsLeafPage_);

	; _SYDNEY_ASSERT(parentNodePage != 0);
	; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

	this->writeSimpleKey(parentNodePage,
						 keyInfoIndex,
						 SrcNullBitmapTop_,
						 false); // リーフページではない

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   parentNodePage,
								   false); // リーフページではない

	if (keyInfoIndex == nodePageHeader.readUseKeyInformationNumber() - 1)
	{
		this->resetParentNodeSimpleKey(
			parentNodePage,
			SrcNullBitmapTop_,
			NodeDepth_ - 1,
			AttachNodePages_,
			false); // リーフページではない
	}

	if (this->m_CatchMemoryExhaust)
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
//	Btree::File::writeSimpleKey -- ノードページにキー値を書き込む
//
//	NOTES
//	異なる領域に記録されているキー値をノードページに書き込む。
//
//	ARGUMENTS
//	PhysicalFile::Page*				NodePage_
//		ノードページの物理ページ記述子
//	const ModUInt32					KeyInfoIndex_
//		キー情報インデックス
//	const Btree::NullBitmap::Value*	SrcNullBitmapTop_
//		書き込むキー値が記録されているキー情報内の
//		ヌルビットマップ先頭へのポインタ
//	const bool						IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
File::writeSimpleKey(PhysicalFile::Page*		NodePage_,
					 const ModUInt32			KeyInfoIndex_,
					 const NullBitmap::Value*	SrcNullBitmapTop_,
					 const bool					IsLeafPage_) const
{
	KeyInformation	keyInfo(this->m_pTransaction,
							NodePage_,
							KeyInfoIndex_,
							IsLeafPage_,
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	NullBitmap::Value*	dstNullBitmapTop = keyInfo.assignKeyNullBitmap();

	; _SYDNEY_ASSERT(dstNullBitmapTop != 0);

	ModSize	keySize =
		NullBitmap::getSize(this->m_cFileParameter.m_KeyNum) +
		this->m_cFileParameter.m_KeySize;

	ModOsDriver::Memory::copy(dstNullBitmapTop, SrcNullBitmapTop_, keySize);
}

//
//	FUNCTION private
//	Btree::File::getKeyInformationIndexForSimpleKeyInsert --
//		異なる領域に記録されているキー値に最も近い値が
//		記録されているキー情報のインデックスを返す
//
//	NOTES
//	引数KeyInfoPage_が示すノードページ内で、
//	異なる領域に記録されているキー値に最も近い値が
//	記録されているキー情報を検索し、インデックスを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*				KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32					UseKeyInfoNum_
//		引数KeyInfoPage_が示すノードページ内の使用中のキー情報数
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const Btree::NullBitmap::Value*	SrcNullBitmapTop_
//		キー値が記録されているキー情報内の
//		ヌルビットマップ先頭へのポインタ
//	const bool						IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	const bool						SearchChildNodePage_
//		子ノードページを検索するために呼び出されたかどうか
//			true  : 子ノードページを検索するために呼び出された
//			false : 引数KeyInfoPage_が示すノードページの
//			        どこにキー情報を追加するかを特定するために
//			        呼び出された
//	PhysicalFile::Page*				PrevKeyInfoChildNodePage_ = 0
//		直前のキー情報が記録されている子ノードページの物理ページ記述子
//		（このノードページの親ノードページから、
//		　このノードページに辿ることができるキー情報を探し出し、
//		　その直後のキー情報から辿る子ノードページに、
//		　キーフィールドを挿入する際に指定する。）
//
//	RETURN
//	int
//		キー情報のインデックス
//
//	EXCEPTIONS
//	[YET!]
//
int
File::getKeyInformationIndexForSimpleKeyInsert(
	PhysicalFile::Page*			KeyInfoPage_,
	const ModUInt32				UseKeyInfoNum_,
	PageVector&					AttachNodePages_,
	const NullBitmap::Value*	SrcNullBitmapTop_,
	const bool					IsLeafPage_,
	const bool					SearchChildNodePage_,
	PhysicalFile::Page*			PrevKeyInfoChildNodePage_ // = 0
	) const
{
	if (PrevKeyInfoChildNodePage_ != 0)
	{
		PhysicalFile::Page*	parentNodePage = 0;
		ModUInt32			prevKeyInfoIndex = ModUInt32Max;

		this->searchKeyInformationSimpleKey(PrevKeyInfoChildNodePage_,
											parentNodePage,
											prevKeyInfoIndex,
											AttachNodePages_,
											IsLeafPage_);

		; _SYDNEY_ASSERT(parentNodePage != 0);
		; _SYDNEY_ASSERT(prevKeyInfoIndex != ModUInt32Max);

		checkMemoryExhaust(parentNodePage);

		return prevKeyInfoIndex + 1;
	}

	if (UseKeyInfoNum_ == 0)
	{
		// 使用中のキー情報が存在しなかった…

		// この場合には、キー情報のインデックスとして
		// 0を返す。
		return 0;
	}

	KeyInformation	keyInfo(this->m_pTransaction,
							KeyInfoPage_,
							0,
							IsLeafPage_,
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	int	midKeyInfoIndex = 0;
	int	firstKeyInfoIndex = 0;
	int	lastKeyInfoIndex = UseKeyInfoNum_ - 1;

	while (firstKeyInfoIndex <= lastKeyInfoIndex)
	{
		midKeyInfoIndex = (firstKeyInfoIndex + lastKeyInfoIndex) >> 1;

		keyInfo.setStartOffsetByIndex(midKeyInfoIndex);

		const NullBitmap::Value*	dstNullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(dstNullBitmapTop != 0);

		if (this->compareSimpleKey(SrcNullBitmapTop_,
								   dstNullBitmapTop)
			< 0)
		{
			lastKeyInfoIndex = midKeyInfoIndex - 1;
		}
		else
		{
			firstKeyInfoIndex = midKeyInfoIndex + 1;
		}
	}

	if (SearchChildNodePage_ &&
		static_cast<ModUInt32>(firstKeyInfoIndex) == UseKeyInfoNum_)
	{
		firstKeyInfoIndex--;
	}

	return firstKeyInfoIndex;
}

//
//	FUNCTION private
//	Btree::File::compareSimpleKey -- キー値を比較する
//
//	NOTES
//	キー値を比較する。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*	SrcNullBitmapTop_
//		比較元のキー値が記録されているキー情報内の
//		ヌルビットマップ先頭へのポインタ
//	const Btree::NullBitmap::Value*	DstNullBitmapTop_
//		比較先のキー値が記録されているキー情報内の
//		ヌルビットマップ先頭へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元のキーの方がキー値順で前方
//			= 0 : 2つのキー値が等しい
//			> 0 : 比較元のキーの方がキー値順で後方
//
//	EXCEPTIONS
//	なし
//
int
File::compareSimpleKey(const NullBitmap::Value*	SrcNullBitmapTop_,
					   const NullBitmap::Value*	DstNullBitmapTop_) const
{
	NullBitmap	srcNullBitmap(SrcNullBitmapTop_,
							  this->m_cFileParameter.m_KeyNum,
							  NullBitmap::Access::ReadOnly);

	bool	srcExistNull = srcNullBitmap.existNull();

	const char*	srcKey =
		static_cast<const char*>(srcNullBitmap.getConstTail());

	NullBitmap	dstNullBitmap(DstNullBitmapTop_,
							  this->m_cFileParameter.m_KeyNum,
							  NullBitmap::Access::ReadOnly);

	bool	dstExistNull = dstNullBitmap.existNull();

	const char*	dstKey =
		static_cast<const char*>(dstNullBitmap.getConstTail());

	int	compareResult = 0;

	for (int i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
	{
		bool	srcIsNull = srcExistNull && srcNullBitmap.isNull(i - 1);
		bool	dstIsNull = dstExistNull && dstNullBitmap.isNull(i - 1);

		if (srcIsNull || dstIsNull)
		{
			if (srcIsNull != dstIsNull)
			{
				compareResult = srcIsNull ? -1 : 1;

				return
					compareResult *
					*(this->m_cFileParameter.m_MultiNumberArray + i);
			}
		}
		else
		{
			Common::DataType::Type	keyDataType =
				*(this->m_cFileParameter.m_FieldTypeArray + i);

			; _SYDNEY_ASSERT(
				FileCommon::DataManager::isVariable(keyDataType) == false);

			if (keyDataType == Common::DataType::Integer)
			{
				compareResult = this->compareIntegerField(srcKey, dstKey);
			}
			else if (keyDataType == Common::DataType::UnsignedInteger)
			{
				compareResult = this->compareUnsignedIntegerField(srcKey,
																  dstKey);
			}
			else if (keyDataType == Common::DataType::Integer64)
			{
				compareResult = this->compareInteger64Field(srcKey, dstKey);
			}
			else if (keyDataType == Common::DataType::UnsignedInteger64)
			{
				compareResult = this->compareUnsignedInteger64Field(srcKey,
																	dstKey);
			}
			else if (keyDataType == Common::DataType::Float)
			{
				compareResult = this->compareFloatField(srcKey, dstKey);
			}
			else if (keyDataType == Common::DataType::Double)
			{
				compareResult = this->compareDoubleField(srcKey, dstKey);
			}
			else if (keyDataType == Common::DataType::Date)
			{
				compareResult = this->compareDateField(srcKey, dstKey);
			}
			else if (keyDataType == Common::DataType::DateTime)
			{
				compareResult = this->compareTimeField(srcKey, dstKey);
			}
			else if (keyDataType == Common::DataType::ObjectID)
			{
				compareResult = this->compareObjectIDField(srcKey, dstKey);
			}
			else
			{
				throw Exception::NotSupported(moduleName, srcFile, __LINE__);
			}

			if (compareResult != 0)
			{
				compareResult *=
					*(this->m_cFileParameter.m_MultiNumberArray + i);

				break;
			}

			Os::Memory::Size	keySize =
				FileCommon::DataManager::getFixedCommonDataArchiveSize(
					keyDataType);

			srcKey += keySize;
			dstKey += keySize;
		}
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::writeSimpleKey -- ノードページにキー値を書き込む
//
//	NOTES
//	ノードページにキー値を書き込む。
//
//	ARGUMENTS
//	PhysicalFile::Page*				NodePage_
//		ノードページの物理ページ記述子
//	const ModUInt32					KeyInfoIndex_
//		書き込み先のキー情報のインデックス
//	const Common::DataArrayData*	Object_
//		書き込むオブジェクトへのポインタ
//	const bool						IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
File::writeSimpleKey(PhysicalFile::Page*			NodePage_,
					 const ModUInt32				KeyInfoIndex_,
					 const Common::DataArrayData*	Object_,
					 const bool						IsLeafPage_) const
{
	KeyInformation	keyInfo(this->m_pTransaction,
							NodePage_,
							KeyInfoIndex_,
							IsLeafPage_,
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	NullBitmap::Value*	nullBitmapTop = keyInfo.assignKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	NullBitmap::clear(nullBitmapTop, this->m_cFileParameter.m_KeyNum);

	char*	key = static_cast<char*>(keyInfo.assignKeyTop());

	for (int i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
	{
		Common::DataType::Type	keyDataType =
			*(this->m_cFileParameter.m_FieldTypeArray + i);

		const Common::Data*	keyField = Object_->getElement(i).get();

		if (keyField->isNull())
		{
			NullBitmap::on(nullBitmapTop,
						   this->m_cFileParameter.m_KeyNum,
						   i - 1);
		}
		else
		{
			FileCommon::DataManager::writeFixedCommonData(*keyField,
														  key);
		}

		key +=
			FileCommon::DataManager::getFixedCommonDataArchiveSize(
				keyDataType);
	}
}

//
//	FUNCTION private
//	Btree::File::shiftKeyInfoForSimpleKeyInsert --
//		挿入のためにキーテーブル内のキー情報をシフトする
//
//	NOTES
//	挿入のためにキーテーブル内のキー情報をシフトする。
//
//	ARGUMENTS
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32		UseKeyInfoNum_
//		使用中のキー情報数
//	const ModUInt32		KeyInfoIndex_
//		挿入するキー情報のインデックス
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
File::shiftKeyInfoForSimpleKeyInsert(
	PhysicalFile::Page*	KeyInfoPage_,
	const ModUInt32		UseKeyInfoNum_,
	const ModUInt32		KeyInfoIndex_,
	const bool			IsLeafPage_,
	ValueFile*			ValueFile_,
	PageVector&			AttachValuePages_) const
{
	if (UseKeyInfoNum_ == 0 || KeyInfoIndex_ >= UseKeyInfoNum_)
	{
		return;
	}

	ModUInt32	srcKeyInfoIndex = UseKeyInfoNum_ - 1;

	KeyInformation	srcKeyInfo(this->m_pTransaction,
							   KeyInfoPage_,
							   srcKeyInfoIndex,
							   IsLeafPage_,
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	KeyInformation	dstKeyInfo(this->m_pTransaction,
							   KeyInfoPage_,
							   srcKeyInfoIndex + 1,
							   IsLeafPage_,
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	PhysicalFile::PageID	keyInfoPageID = KeyInfoPage_->getID();

	PhysicalFile::Page*	valueObjectPage = 0;

	do
	{
		srcKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex);

		dstKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex + 1);

		dstKeyInfo.copy(srcKeyInfo);

		if (IsLeafPage_)
		{
			ValueFile_->update(dstKeyInfo.readValueObjectID(),
							   keyInfoPageID,
							   srcKeyInfoIndex + 1,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
		}

		if (srcKeyInfoIndex == 0)
		{
			break;
		}

		srcKeyInfoIndex--;
	}
	while (srcKeyInfoIndex >= KeyInfoIndex_);
}

//
//	FUNCTION private
//	Btree::File::splitSimpleKeyTable -- ノードを分割する
//
//	NOTES
//	必要ならば、ノードの分割を行う。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	PhysicalFile::Page*		KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	PhysicalFile::Page*&	NewKeyInfoPage_
//		分割によって新たに確保したノードページの物理ページ記述子
//	const bool				IsLeafPage_
//		分割対象のノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	bool
//		ノードを分割したかどうか
//			true  : ノードを分割した
//			false : ノードを分割しなかった（する必要がなかった）
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::splitSimpleKeyTable(FileInformation&		FileInfo_,
						  PhysicalFile::Page*	KeyInfoPage_,
						  PageVector&			AttachNodePages_,
						  PageIDVector&			AllocateNodePageIDs_,
						  PhysicalFile::Page*&	NewKeyInfoPage_,
						  const bool			IsLeafPage_,
						  ValueFile*			ValueFile_,
						  PageVector&			AttachValuePages_) const
{
	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   KeyInfoPage_,
								   IsLeafPage_);

	ModUInt32	keyNumInNode =
		nodePageHeader.readUseKeyInformationNumber();

	if (keyNumInNode < this->m_cFileParameter.m_KeyPerNode)
	{
		// まだノードページは一杯ではない…

		// なので、分割は不要。

		return false;
	}

	//
	// ノードページ一杯にキーオブジェクトが記録されているので、
	// ノードページを分割する。
	// ※ “物理的に一杯かどうか”ではない。
	// 　 次数で一杯になったのである。
	//

	; _SYDNEY_ASSERT(
		keyNumInNode == this->m_cFileParameter.m_KeyPerNode);

	ModUInt32	moveKeyNum =
		keyNumInNode -
		static_cast<ModUInt32>(
			keyNumInNode / 100.0 *
			this->m_cFileParameter.m_NodeKeyDivideRate);

	if (moveKeyNum == 0)
	{
		moveKeyNum = 1;
	}
	else if (moveKeyNum >= keyNumInNode)
	{
		moveKeyNum = keyNumInNode - 1;
	}

	NewKeyInfoPage_ = this->createNodePage(AttachNodePages_,
										   AllocateNodePageIDs_,
										   IsLeafPage_);

	PhysicalFile::PageID	newKeyInfoPageID = NewKeyInfoPage_->getID();

	NodePageHeader	newNodePageHeader(this->m_pTransaction,
									  NewKeyInfoPage_,
									  IsLeafPage_);

	nodePageHeader.writeUseKeyInformationNumber(keyNumInNode - moveKeyNum);

	newNodePageHeader.writeUseKeyInformationNumber(moveKeyNum);

	if (IsLeafPage_)
	{
		if (KeyInfoPage_->getID() == FileInfo_.readLastLeafPageID())
		{
			FileInfo_.writeLastLeafPageID(NewKeyInfoPage_->getID());
		}

		this->resetLeafPageLink(KeyInfoPage_,
								NewKeyInfoPage_,
								AttachNodePages_);
	}

	this->copyParentNodePageID(KeyInfoPage_,
							   NewKeyInfoPage_,
							   AttachNodePages_,
							   IsLeafPage_);

	int	srcKeyInfoLastIndex = keyNumInNode - 1;
	int	srcKeyInfoIndex = keyNumInNode - moveKeyNum;
	KeyInformation	srcKeyInfo(this->m_pTransaction,
							   KeyInfoPage_,
							   srcKeyInfoIndex,
							   IsLeafPage_,
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	int	dstKeyInfoIndex = 0;
	KeyInformation	dstKeyInfo(this->m_pTransaction,
							   NewKeyInfoPage_,
							   dstKeyInfoIndex,
							   IsLeafPage_,
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	PhysicalFile::Page*	valueObjectPage = 0;

	while (srcKeyInfoIndex <= srcKeyInfoLastIndex)
	{
		srcKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex);
		dstKeyInfo.setStartOffsetByIndex(dstKeyInfoIndex);

		dstKeyInfo.copy(srcKeyInfo);

		if (IsLeafPage_)
		{
			ValueFile_->update(dstKeyInfo.readValueObjectID(),
							   newKeyInfoPageID,
							   dstKeyInfoIndex,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
		}
		else
		{
			this->resetNodePageHeaderParentNodePageID(
				newKeyInfoPageID,
				dstKeyInfo.readChildNodePageID(),
				AttachNodePages_,
				IsLeafPage_);
		}

		srcKeyInfoIndex++;
		dstKeyInfoIndex++;
	}

	return true;
}

//
//	FUNCTION private
//	Btree::File::uniqueCheckSimpleKey --
//		オブジェクト挿入前にユニークチェックを行う
//
//	NOTES
//	オブジェクト挿入前にユニークチェックを行う。
//	引数Object_で示すオブジェクトを挿入することにより
//	オブジェクトのユニーク性が失われてしまう場合には
//	例外UniquenessViolationを送出する。
//
//	ARGUMENTS
//	Btree::FileInformation&		FileInfo_
//		ファイル管理情報への参照
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	Common::DataArrayData*		Object_
//		挿入オブジェクトへのポインタ
//	const PhysicalFile::PageID	LeafPageID_
//		キーフィールドの値を書き込むリーフページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	UniquenessViolation
//		ファイル内に記録されているオブジェクトのユニーク性が失われてしまう
//	[YET!]
//
void
File::uniqueCheckSimpleKey(
	FileInformation&				FileInfo_,
	ValueFile*					ValueFile_,
	Common::DataArrayData*		Object_,
	const PhysicalFile::PageID	LeafPageID_,
	PageVector&					AttachNodePages_,
	PageVector&					AttachValuePages_) const
{
	//
	// ファイルが空ならば、絶対ユニーク。
	//

	if (FileInfo_.readObjectNum() == 0)
	{
		return;
	}

	; _SYDNEY_ASSERT(
		this->m_cFileParameter.m_UniqueType ==
		FileParameter::UniqueType::Key ||
		this->m_cFileParameter.m_UniqueType ==
		FileParameter::UniqueType::Object);

	; _SYDNEY_ASSERT(Object_ != 0);
	; _SYDNEY_ASSERT(
		LeafPageID_ != PhysicalFile::ConstValue::UndefinedPageID);

	PhysicalFile::Page*	valuePage = 0;

	PhysicalFile::PageID	leafPageID = LeafPageID_;

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 leafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	ModUInt32	useKeyInfoNum =
		leafPageHeader.readUseKeyInformationNumber();

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							0,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (useKeyInfoNum > 0)
	{
		bool	match = false;

		int	keyInfoIndex =
			this->getKeyInformationIndexForSimpleKeyInsert(
				leafPage,
				useKeyInfoNum,
				AttachNodePages_,
				true,  // リーフページ
				true); // 子ノードを検索するわけではないが、
					   // File::getKeyInformationIndexForSimpleKeyInsert内の
					   // アサートで引っ掛かってしまうので、trueにしてある。
					   // ここで、trueを設定してもfalseを設定しても
					   // 影響ないので。

		keyInfo.setStartOffsetByIndex(keyInfoIndex);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToFetchCondition(nullBitmapTop) == 0)
		{
			if (this->m_cFileParameter.m_UniqueType ==
				FileParameter::UniqueType::Key)
			{
				checkMemoryExhaust(leafPage);

#ifdef DEBUG

				SydErrorMessage << "Unique error." << ModEndl;

#endif

				throw Exception::UniquenessViolation(moduleName,
													srcFile, 
													__LINE__);
			}
			else
			{
				ModUInt64	valueObjectID = keyInfo.readValueObjectID();

				valuePage = File::attachPage(
					m_pTransaction, ValueFile_->m_PhysicalFile,
					Common::ObjectIDData::getFormerValue(valueObjectID),
					m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

				if (compareToFetchCondition(
						valuePage, AttachValuePages_,
						Common::ObjectIDData::getLatterValue(valueObjectID),
						Object_,
						FetchHint::CompareType::OnlyValue,
						false, // バリューオブジェクト
						1)     // for object ID field
					== 0)
				{
					checkMemoryExhaust(leafPage);
					if (this->m_CatchMemoryExhaust)
					{
						ValueFile_->m_PhysicalFile->detachPage(valuePage,PhysicalFile::Page::UnfixMode::NotDirty,false); // 本当にデタッチ（アンフィックス）
					}

#ifdef DEBUG

					SydErrorMessage << "Unique error." << ModEndl;

#endif

					throw Exception::UniquenessViolation(moduleName,
														srcFile,
														__LINE__);
				}
			}
		}
	}

	while (this->assignPrevKeyInformation(leafPage,
										  AttachNodePages_,
										  leafPageHeader,
										  keyInfo))
	{
		if (leafPageID != leafPage->getID())
		{
			leafPageID = leafPage->getID();
		}

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToFetchCondition(nullBitmapTop) != 0)
		{
			break;
		}

		if (this->m_cFileParameter.m_UniqueType ==
			FileParameter::UniqueType::Key)
		{
			checkMemoryExhaust(leafPage);
			if (this->m_CatchMemoryExhaust)
			{
				if (valuePage != 0)
				{
					ValueFile_->m_PhysicalFile->detachPage(valuePage,PhysicalFile::Page::UnfixMode::NotDirty,false); // 本当にデタッチ（アンフィックス）
				}
			}

#ifdef DEBUG

			SydErrorMessage << "Unique error." << ModEndl;

#endif

			throw Exception::UniquenessViolation(moduleName,
												srcFile,
												__LINE__);
		}

		ModUInt64	valueObjectID = keyInfo.readValueObjectID();

		valuePage = File::attachPage(
			m_pTransaction, ValueFile_->m_PhysicalFile,
			Common::ObjectIDData::getFormerValue(valueObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

		if (compareToFetchCondition(
				valuePage, AttachValuePages_,
				Common::ObjectIDData::getLatterValue(valueObjectID),
				Object_,
				FetchHint::CompareType::OnlyValue,
				false, // バリューオブジェクト
				1)     // for object ID field
			== 0)
		{
			checkMemoryExhaust(leafPage);
			if (this->m_CatchMemoryExhaust)
			{
				ValueFile_->m_PhysicalFile->detachPage(valuePage,PhysicalFile::Page::UnfixMode::NotDirty,false); // 本当にデタッチ（アンフィックス）してしまい、
			}

#ifdef DEBUG

			SydErrorMessage << "Unique error." << ModEndl;

#endif

			throw Exception::UniquenessViolation(moduleName,
												srcFile,
												__LINE__);
		}

		if (this->m_CatchMemoryExhaust)
		{
			ValueFile_->m_PhysicalFile->detachPage(
				valuePage,
				PhysicalFile::Page::UnfixMode::NotDirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}
	}

	checkMemoryExhaust(leafPage);
}

//
//	FUNCTION private
//	Btree::File::searchLeafPageForSimpleKeyInsert --
//		キー値を記録するためのリーフページを検索する
//
//	NOTES
//	キー値を記録するためのリーフページを検索する。
//
//	ARGUMENTS
//	const ModUInt32				TreeDepth_
//		現在の木の深さ
//	const PhysicalFile::PageID	RootNodePageID_
//		ルートノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	PhysicalFile::Page*
//		リーフページの物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::searchLeafPageForSimpleKeyInsert(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::PageID	nodePageID = RootNodePageID_;

	for (ModUInt32 i = 1; i < TreeDepth_; i++)
	{
		nodePageID =
			this->searchChildNodePageForSimpleKeyInsert(nodePageID,
														AttachNodePages_);
	}

	return File::attachPage(this->m_pTransaction,
							this->m_pPhysicalFile,
							nodePageID,
							this->m_FixMode,
							this->m_CatchMemoryExhaust,
							AttachNodePages_);
}

//
//	FUNCTION private
//	Btree::File::searchChildNodePageForSimpleKeyInsert --
//		キー値を記録するための子ノードページを検索する
//
//	NOTES
//	キー値を記録するための子ノードページを検索する。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ParentNodePageID_
//		親ノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	PhysicalFile::PageID
//		子ノードページの物理ページ識別子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::PageID
File::searchChildNodePageForSimpleKeyInsert(
	const PhysicalFile::PageID	ParentNodePageID_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	parentNodePage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 ParentNodePageID_,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	NodePageHeader	parentNodePageHeader(this->m_pTransaction,
										 parentNodePage,
										 false); // リーフページではない

	int	keyInfoIndex =
		this->getKeyInformationIndexForSimpleKeyInsert(
			parentNodePage,
			parentNodePageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			false, // リーフページではない
			true); // 子ノードページを探す

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false, // リーフページではない
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	PhysicalFile::PageID	childNodePageID =
		keyInfo.readChildNodePageID();

	checkMemoryExhaust(parentNodePage);

	return childNodePageID;
}

//
//	FUNCTION private
//	Btree::File::getKeyInformationIndexForSimpleKeyInsert --
//		挿入するオブジェクトのキーフィールドの値に最も近い
//		値が記録されているキー情報のインデックスを返す
//
//	NOTES
//	引数KeyInfoPage_が示すノードページ内で、
//	挿入するオブジェクトのキーフィールドの値に最も近い
//	値が記録されているキー情報のインデックスを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*			KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32				UseKeyInfoNum_
//		引数KeyInfoPage_が示すノードページ内の使用中のキー情報数
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool					IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	const bool					SearchChildNodePage_
//		子ノードページを検索するために呼び出されたかどうか
//			true  : 子ノードページを検索するために呼び出された
//			false : 引数KeyInfoPage_が示すノードページの
//			        どこにキー情報を追加するかを特定するために
//			        呼び出された
//	PhysicalFile::Page*			PrevKeyInfoChildNodePage_ = 0
//		直前のキー情報が記録されている子ノードページの物理ページ記述子
//		（このノードページの親ノードページから、
//		　このノードページに辿ることができるキー情報を探し出し、
//		　その直後のキー情報から辿る子ノードページに、
//		　キーフィールドを挿入する際に指定する。）
//
//	RETURN
//	int
//		キー情報のインデックス
//
//	EXCEPTIONS
//	[YET!]
//
int
File::getKeyInformationIndexForSimpleKeyInsert(
	PhysicalFile::Page*			KeyInfoPage_,
	const ModUInt32				UseKeyInfoNum_,
	PageVector&					AttachNodePages_,
	const bool					IsLeafPage_,
	const bool					SearchChildNodePage_,
	PhysicalFile::Page*			PrevKeyInfoChildNodePage_ // = 0
	) const
{
	if (PrevKeyInfoChildNodePage_ != 0)
	{
		PhysicalFile::Page*	parentNodePage = 0;
		ModUInt32			prevKeyInfoIndex = ModUInt32Max;

		this->searchKeyInformationSimpleKey(PrevKeyInfoChildNodePage_,
											parentNodePage,
											prevKeyInfoIndex,
											AttachNodePages_,
											IsLeafPage_);

		; _SYDNEY_ASSERT(parentNodePage != 0);
		; _SYDNEY_ASSERT(prevKeyInfoIndex != ModUInt32Max);

		checkMemoryExhaust(parentNodePage);

		return prevKeyInfoIndex + 1;
	}

	if (UseKeyInfoNum_ == 0)
	{
		// 使用中のキー情報が存在しなかった…

		// この場合には、キー情報のインデックスとして
		// 0を返す。
		return 0;
	}

	KeyInformation	keyInfo(this->m_pTransaction,
							KeyInfoPage_,
							0,
							IsLeafPage_,
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	int	midKeyInfoIndex = 0;
	int	firstKeyInfoIndex = 0;
	int	lastKeyInfoIndex = UseKeyInfoNum_ - 1;

	while (firstKeyInfoIndex <= lastKeyInfoIndex)
	{
		midKeyInfoIndex = (firstKeyInfoIndex + lastKeyInfoIndex) >> 1;

		keyInfo.setStartOffsetByIndex(midKeyInfoIndex);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToFetchCondition(nullBitmapTop) < 0)
		{
			lastKeyInfoIndex = midKeyInfoIndex - 1;
		}
		else
		{
			firstKeyInfoIndex = midKeyInfoIndex + 1;
		}
	}

	if (SearchChildNodePage_ &&
		static_cast<ModUInt32>(firstKeyInfoIndex) == UseKeyInfoNum_)
	{
		firstKeyInfoIndex--;
	}

	return firstKeyInfoIndex;
}

//
//	FUNCTION private
//	Btree::File::searchKeyInformationSimpleKey --
//		親ノードページとキー情報を検索する
//
//	NOTES
//	引数ChildNodePage_が示す子ノードページの親ノードページと、
//	親ノードページ内に存在するキー情報のうちで
//	子ノードページへ辿るキー情報のインデックスを検索し、
//	引数ParentNodePage_とKeyInfoIndex_にそれぞれ設定する。
//	ただし、親ノードページ内に該当するキー情報が存在しない場合には、
//	引数ParentNodePage_には0（ヌルポインタ）が、
//	引数KeyInfoIndex_にはModUInt32Maxがそれぞれ設定される。
//
//	ARGUMENTS
//	PhysicalFile::Page*		ChildNodePage_
//		子ノードページの物理ページ記述子
//	PhysicalFile::Page*&	ParentNodePage_
//		親ノードページの物理ページ記述子
//		（この関数内で設定する）
//	ModUInt32&				KeyInfoIndex_
//		親ノードページ内のキー情報のインデックス
//		（この関数内で設定する）
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool				ChildNodeIsLeafPage_
//		子ノードページがリーフページかどうか
//			true  : 子ノードページがリーフページ
//			false : 子ノードページがリーフページ以外のノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::searchKeyInformationSimpleKey(
	PhysicalFile::Page*		ChildNodePage_,
	PhysicalFile::Page*&	ParentNodePage_,
	ModUInt32&				KeyInfoIndex_,
	PageVector&				AttachNodePages_,
	const bool				ChildNodeIsLeafPage_) const
{
	NodePageHeader	childNodePageHeader(this->m_pTransaction,
										ChildNodePage_,
										ChildNodeIsLeafPage_);

	PhysicalFile::PageID	parentNodePageID =
		childNodePageHeader.readParentNodePageID();

	; _SYDNEY_ASSERT(
		parentNodePageID != PhysicalFile::ConstValue::UndefinedPageID);

	ParentNodePage_ = File::attachPage(this->m_pTransaction,
									   this->m_pPhysicalFile,
									   parentNodePageID,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachNodePages_);

	NodePageHeader	parentNodePageHeader(this->m_pTransaction,
										 ParentNodePage_,
										 false); // リーフページではない

	int	useKeyInfoNum =
		parentNodePageHeader.readUseKeyInformationNumber();

	; _SYDNEY_ASSERT(useKeyInfoNum > 0);

	KeyInformation	keyInfo(this->m_pTransaction,
							ParentNodePage_,
							0,
							false, // リーフページではない
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	PhysicalFile::PageID	childNodePageID = ChildNodePage_->getID();

	for (int i = 0; i < useKeyInfoNum; i++)
	{
		keyInfo.setStartOffsetByIndex(i);

		if (keyInfo.readChildNodePageID() == childNodePageID)
		{
			KeyInfoIndex_ = i;

			return;
		}
	}

	checkMemoryExhaust(ParentNodePage_);

	ParentNodePage_ = 0;
	KeyInfoIndex_ = ModUInt32Max;
}

//
//	Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
