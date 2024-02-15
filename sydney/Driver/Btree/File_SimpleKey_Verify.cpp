// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_SimpleKey_Verify.cpp --
//		Ｂ＋木ファイルクラスの実現ファイル（整合性検査関連）
// 
// Copyright (c) 2001, 2002, 2006, 2023 Ricoh Company, Ltd.
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

#include "Exception/MemoryExhaust.h"

#include "Common/Assert.h"

#include "FileCommon/FileOption.h"

#include "Btree/TreeFile.h"
#include "Btree/FileInformation.h"
#include "Btree/ValueFile.h"
#include "Btree/NodePageHeader.h"
#include "Btree/KeyInformation.h"
#include "Btree/UseInfo.h"
#include "Btree/Message_DiscordDelegateKey.h"
#include "Btree/Message_IllegalKeyInfoIndex.h"
#include "Btree/Message_KeyInfoIndexNotEqualZero.h"
#include "Btree/Message_DiscordNextLeaf.h"

_SYDNEY_USING

using namespace Btree;

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::File::setSimpleNodePageUseInfo --
//		ツリーファイル内で使用しているすべてのノードページと
//		ノードページ内で使用しているすべての物理エリアを登録する
//
//	NOTES
//	キー情報にキーフィールドの値を記録するタイプの
//	ツリーファイル内で使用しているすべてのノードページと
//	ノードページ内で使用しているすべての物理エリアを登録する。
//	バリューファイル内での登録も、この関数経由で行う。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子
//	PhysicalFile::File*				TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	const ModUInt32					NodeDepth_
//		ノードの階層
//	const PhysicalFile::PageID		NodePageID_
//		ノードページの物理ページ識別子
//	Btree::UseInfo&					TreeFileUseInfo_
//		ツリーファイルの登録情報への参照
//	Btree::ValueFile*				ValueFile_
//		バリューファイル記述子
//	Btree::UseInfo&					ValueFileUseInfo_
//		バリューファイルの登録情報への参照
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::setSimpleNodePageUseInfo(
	const Trans::Transaction&		Transaction_,
	PhysicalFile::File*				TreePhysicalFile_,
	const ModUInt32					NodeDepth_,
	const PhysicalFile::PageID		NodePageID_,
	UseInfo&						TreeFileUseInfo_,
	ValueFile*						ValueFile_,
	UseInfo&						ValueFileUseInfo_,
	Admin::Verification::Progress&	Progress_) const
{
	; _SYDNEY_ASSERT(
		NodePageID_ != PhysicalFile::ConstValue::UndefinedPageID);

	bool	isLeafPage = (NodeDepth_ == 1);

	PhysicalFile::PageID	nodePageID = NodePageID_;

	while (nodePageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		TreeFileUseInfo_.append(nodePageID, NodePageHeader::AreaID);

		PhysicalFile::Page*	nodePage =
			TreePhysicalFile_->verifyPage(Transaction_,
										  nodePageID,
										  Buffer::Page::FixMode::ReadOnly,
										  Progress_);

		if (Progress_.isGood() == false)
		{
			return;
		}

		; _SYDNEY_ASSERT(nodePage != 0);

		NodePageHeader	nodePageHeader(&Transaction_,
									   nodePage,
									   isLeafPage);

		ModUInt32	keyInfoNum = nodePageHeader.readKeyInformationNumber();

		if (keyInfoNum > 0)
		{
			TreeFileUseInfo_.append(nodePageID,
									KeyInformation::KeyTableAreaID);

			ModUInt32	useKeyInfoNum =
				nodePageHeader.readUseKeyInformationNumber();

			KeyInformation	keyInfo(&Transaction_,
									nodePage,
									0, // とりあえず先頭のキー情報
									isLeafPage,
									this->m_cFileParameter.m_KeyNum,
									this->m_cFileParameter.m_KeySize);

			for (ModUInt32 i = 0; i < useKeyInfoNum; i++)
			{
				try
				{
					keyInfo.setStartOffsetByIndex(i);

					if (isLeafPage)
					{
						// リーフページ…

						//
						// バリューオブジェクトのために使用している
						// バリューページと物理エリアを登録する
						//

						ModUInt64	valueObjectID =
							keyInfo.readValueObjectID();

						ValueFile_->setUseInfo(valueObjectID,
											   ValueFileUseInfo_,
											   Progress_);
					}
					else
					{
						// リーフページではない…

						//
						// 子ノードページ内で使用している
						// すべての物理エリアを登録する
						//

						PhysicalFile::PageID	childNodePageID =
							keyInfo.readChildNodePageID();

						this->setSimpleNodePageUseInfo(Transaction_,
													   TreePhysicalFile_,
													   NodeDepth_ - 1,
													   childNodePageID,
													   TreeFileUseInfo_,
													   ValueFile_,
													   ValueFileUseInfo_,
													   Progress_);
					}

					if (Progress_.isGood() == false)
					{
						TreePhysicalFile_->detachPage(
							nodePage,
							PhysicalFile::Page::UnfixMode::NotDirty);

						return;
					}
				}
				catch (...)
				{
					TreePhysicalFile_->detachPage(
						nodePage,
						PhysicalFile::Page::UnfixMode::NotDirty);

					_SYDNEY_RETHROW;
				}
			}
		}

		nodePageID = nodePageHeader.readNextPhysicalPageID();

		TreePhysicalFile_->detachPage(
			nodePage,
			PhysicalFile::Page::UnfixMode::NotDirty);

	} // end while nodePageID != UndefinedPageID
}

//
//	FUNCTION private
//	Btree::File::checkDelegateSimpleKey --
//		子ノードページの代表キー検査
//
//	NOTES
//	キー情報にキーフィールドの値が記録されているタイプの
//	ツリーファイル内の、子ノードページの代表キーに不正がないかを検査する。
//
//	検査方法：
//		ルートノードページ以外のノードページの代表キーと一致するキーが、
//		その親ノードページに記録されているかを検査する。
//		（オブジェクトの挿入ソート順が昇順の場合には、
//		　代表キーは子ノードページ内でキー値が最大のキーとなり、
//		　降順の場合には、最小のキーとなる。）
//
//	ARGUMENTS
//	const ModUInt32					NodeDepth_
//		ノードの階層
//	const PhysicalFile::PageID		NodePageID_
//		ノードページの物理ページ識別子
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::checkDelegateSimpleKey(const ModUInt32				NodeDepth_,
							 const PhysicalFile::PageID		NodePageID_,
							 PageVector&					AttachNodePages_,
							 Admin::Verification::Progress&	Progress_)
{
	; _SYDNEY_ASSERT(
		NodePageID_ != PhysicalFile::ConstValue::UndefinedPageID);

	//
	// リーフページには、子ノードページが存在しないので、
	// 何もしない。
	//

	if (NodeDepth_ == 1)
	{
		return;
	}

	bool	childNodeIsLeaf = (NodeDepth_ == 2);

	PhysicalFile::PageID	nodePageID = NodePageID_;

	PhysicalFile::Page*	nodePage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 nodePageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   nodePage,
								   false); // リーフページではない

	ModUInt32	useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	; _SYDNEY_ASSERT(useKeyInfoNum > 0);

	KeyInformation	keyInfo(this->m_pTransaction,
							nodePage,
							0,     // とりあえず先頭のキー情報
							false, // リーフページではない
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	for (ModUInt32 i = 0; i < useKeyInfoNum; i++)
	{
		keyInfo.setStartOffsetByIndex(i);

		const NullBitmap::Value*	parentNodeNullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(parentNodeNullBitmapTop != 0);

		PhysicalFile::PageID	childNodePageID =
			keyInfo.readChildNodePageID();

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

		ModUInt32	childNodeUseKeyInfoNum =
			childNodePageHeader.readUseKeyInformationNumber();

		; _SYDNEY_ASSERT(childNodeUseKeyInfoNum > 0);

		KeyInformation	childNodeKeyInfo(this->m_pTransaction,
										 childNodePage,
										 childNodeUseKeyInfoNum - 1,
										 childNodeIsLeaf,
										 this->m_cFileParameter.m_KeyNum,
										 this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	childNodeNullBitmapTop =
			childNodeKeyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(childNodeNullBitmapTop != 0);

		if (this->compareSimpleKey(parentNodeNullBitmapTop,
								   childNodeNullBitmapTop)
			!= 0)
		{
			//
			// ※ ここでは、キーオブジェクトが記録されている
			// 　 物理ページの識別子ではなく、キー情報が記録されている
			// 　 物理ページの識別子を、
			// 　 キーオブジェクトが記録されている
			// 　 物理エリアの識別子ではなく、
			// 　 キー情報のインデックスを渡す。
			//
			ModUnicodeString	areaPath;
			this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

			_SYDNEY_VERIFY_INCONSISTENT(
				Progress_,
				areaPath,
				Message::DiscordDelegateKey(nodePageID,
											i,
											childNodePageID,
											childNodeUseKeyInfoNum - 1));

			break;
		}

		checkMemoryExhaust(childNodePage);

		this->checkDelegateSimpleKey(NodeDepth_ - 1,
									 childNodePageID,
									 AttachNodePages_,
									 Progress_);

		if (Progress_.isGood() == false)
		{
			break;
		}

	} // end for i

	checkMemoryExhaust(nodePage);
}

//
//	FUNCTION private
//	Btree::File::checkSimpleLeafLink -- リーフページのリンク検査
//
//	NOTES
//	キー情報にキーフィールドの値が記録されているタイプの
//	ツリーファイル内の、リーフページのリンクに不正がないかを検査する。
//
//	検査方法：
//		リーフページの1段上位階層のノードページの
//		各キー情報に記録されている「子ノードページの物理ページ識別子」と
//		リーフページ同士のリンクが一致しているかどうかを
//		すべてのリーフページについて調べる。
//
//	ARGUMENTS
//	Btree::FileInformation&			FileInfo_
//		ファイル管理情報への参照
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::checkSimpleLeafLink(FileInformation&					FileInfo_,
						  Admin::Verification::Progress&	Progress_)
{
	//
	// 木が1段ならば、検査のしようがない。
	//

	if (FileInfo_.readTreeDepth() == 1)
	{
		// 木が1段…

		return;
	}

	PhysicalFile::PageID	leafPageID = FileInfo_.readTopLeafPageID();

	PhysicalFile::PageID	nextLeafPageID =
		PhysicalFile::ConstValue::UndefinedPageID;

	PageVector	attachNodePages;

	while (true)
	{
		try
		{
			int	cnt=0;
			while (true)
			{
				PhysicalFile::Page*	leafPage =
					File::attachPage(this->m_pTransaction,
									 this->m_pPhysicalFile,
									 leafPageID,
									 this->m_FixMode,
									 this->m_CatchMemoryExhaust,
									 attachNodePages);

				NodePageHeader	leafPageHeader(this->m_pTransaction,
											   leafPage,
											   true); // リーフページ

				nextLeafPageID = leafPageHeader.readNextLeafPageID();

				if (nextLeafPageID ==
					PhysicalFile::ConstValue::UndefinedPageID)
				{
					checkMemoryExhaust(leafPage);

					break;
				}

				PhysicalFile::PageID	parentNodePageID1 =
					leafPageHeader.readParentNodePageID();

				PhysicalFile::Page*	parentNodePage = 0;
				ModUInt32	keyInfoIndex = ModUInt32Max;

				this->searchKeyInformationSimpleKey(
					leafPage,
					parentNodePage,
					keyInfoIndex,
					attachNodePages,
					true); // 子ノードページはリーフページ

				; _SYDNEY_ASSERT(parentNodePage != 0);

				NodePageHeader	parentNodePageHeader(this->m_pTransaction,
													 parentNodePage,
													 false); // リーフページ
															 // ではない

				ModUInt32	useKeyInfoNum =
					parentNodePageHeader.readUseKeyInformationNumber();

				; _SYDNEY_ASSERT(useKeyInfoNum > 0);

				PhysicalFile::Page*	nextLeafPage =
					File::attachPage(this->m_pTransaction,
									 this->m_pPhysicalFile,
									 nextLeafPageID,
									 this->m_FixMode,
									 this->m_CatchMemoryExhaust,
									 attachNodePages);

				NodePageHeader	nextLeafPageHeader(this->m_pTransaction,
												   nextLeafPage,
												   true); // リーフページ

				PhysicalFile::PageID	parentNodePageID2 =
					nextLeafPageHeader.readParentNodePageID();

				if (parentNodePageID1 == parentNodePageID2)
				{
					// 親ノードページが同一…

					if (keyInfoIndex >= useKeyInfoNum)
					{
						ModUnicodeString	areaPath;
						this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

						_SYDNEY_VERIFY_INCONSISTENT(
							Progress_,
							areaPath,
							Message::IllegalKeyInfoIndex(keyInfoIndex,
														 useKeyInfoNum));

						checkMemoryExhaust(leafPage,parentNodePage,nextLeafPage);

						break;
					}

					KeyInformation	keyInfo(this->m_pTransaction,
											parentNodePage,
											keyInfoIndex + 1,
											false, // リーフページではない
											this->m_cFileParameter.m_KeyNum,
											this->m_cFileParameter.m_KeySize);

					if (keyInfo.readChildNodePageID() != nextLeafPageID)
					{
						ModUnicodeString	areaPath;
						this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

						_SYDNEY_VERIFY_INCONSISTENT(
							Progress_,
							areaPath,
							Message::DiscordNextLeaf(keyInfo.readChildNodePageID(),
													 nextLeafPageID));

						checkMemoryExhaust(leafPage,parentNodePage,nextLeafPage);

						break;
					}
				}
				else
				{
					// 親ノードページが異なる…

					//
					// リーフページ以外のノードページは、
					// 同一階層のノードページ同士のリンクは張っていない。
					// 従って、前のリーフページへ辿ることができる
					// 親ノードページ内のキー情報が、そのノードページ内での
					// 最終キー情報であり、
					// 次のリーフページへ辿ることができる
					// 親ノードページ内のキー情報が、そのノードページ内での
					// 先頭キー情報であることを確認する。
					//

					if (keyInfoIndex < useKeyInfoNum - 1)
					{
						ModUnicodeString	areaPath;
						this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

						_SYDNEY_VERIFY_INCONSISTENT(
							Progress_,
							areaPath,
							Message::IllegalKeyInfoIndex(keyInfoIndex,
														 useKeyInfoNum));

						checkMemoryExhaust(leafPage,parentNodePage,nextLeafPage);

						break;
					}

					checkMemoryExhaust(parentNodePage);

					this->searchKeyInformationSimpleKey(nextLeafPage,
														parentNodePage,
														keyInfoIndex,
														attachNodePages,
														true); // 子ノードページは
															   // リーフページ

					if (keyInfoIndex != 0)
					{
						ModUnicodeString	areaPath;
						this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

						_SYDNEY_VERIFY_INCONSISTENT(
							Progress_,
							areaPath,
							Message::KeyInfoIndexNotEqualZero(keyInfoIndex));

						checkMemoryExhaust(leafPage,parentNodePage,nextLeafPage);

						break;
					}
				}

				checkMemoryExhaust(leafPage,parentNodePage,nextLeafPage);

				leafPageID = nextLeafPageID;
				cnt++;
			}

			// MemoryExhaustが送出されることなく、
			// 目的のオブジェクトをファイルから読み込むことができたので
			// ループから抜ける。
			break;
		}
		catch (Exception::MemoryExhaust&)
		{
			// MemoryExhaustをキャッチ…

			if (this->m_CatchMemoryExhaust)
			{
				// 各物理ページをアタッチ後に不要になったら
				// 即デタッチしていたにもかかわらず
				// またまたMemoryExhaustが起こったのであれば
				// どうしようもない…

				_SYDNEY_RETHROW;
			}
			else
			{
				// MemoryExhaustをキャッチしたのは、
				// 今回が最初…

				// リトライの準備をする。

				this->m_CatchMemoryExhaust = true;

				File::detachPageAll(this->m_pPhysicalFile,attachNodePages,PhysicalFile::Page::UnfixMode::NotDirty,this->m_SavePage);
			}
		}

	} // end while (true)

	File::detachPageAll(this->m_pPhysicalFile,attachNodePages,PhysicalFile::Page::UnfixMode::NotDirty,this->m_SavePage);
}

//
//	FUNCTION private
//	Btree::File::compareSimpleKeyForVerify --
//		キーフィールドの値を比較する
//
//	NOTES
//	整合性検査のために、キー情報に記録されている
//	キーフィールドの値を比較する。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	SrcLeafPageID_
//		比較元キー情報が記録されているリーフページの物理ページ識別子
//	const ModUInt32				SrcKeyInfoIndex_
//		比較元キー情報のインデックス
//	const PhysicalFile::PageID	DstLeafPageID_
//		比較先キー情報が記録されているリーフページの物理ページ識別子
//	const ModUInt32				DstKeyInfoIndex_
//		比較先キー情報のインデックス
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	bool
//		比較元と比較先2つのキー情報に記録されている
//		キーフィールドの値が等しいかどうか
//			true  : 2つのキーフィールドの値が等しい
//			false : 2つのキーフィールドの値が異なる
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareSimpleKeyForVerify(
	const PhysicalFile::PageID	SrcLeafPageID_,
	const ModUInt32				SrcKeyInfoIndex_,
	const PhysicalFile::PageID	DstLeafPageID_,
	const ModUInt32				DstKeyInfoIndex_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	srcLeafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 SrcLeafPageID_,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	KeyInformation	srcKeyInfo(this->m_pTransaction,
							   srcLeafPage,
							   SrcKeyInfoIndex_,
							   true, // リーフページ
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	const NullBitmap::Value*	srcNullBitmapTop =
		srcKeyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(srcNullBitmapTop != 0);

	PhysicalFile::Page*	dstLeafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 DstLeafPageID_,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	KeyInformation	dstKeyInfo(this->m_pTransaction,
							   dstLeafPage,
							   DstKeyInfoIndex_,
							   true, // リーフページ
							   this->m_cFileParameter.m_KeyNum,
							   this->m_cFileParameter.m_KeySize);

	const NullBitmap::Value*	dstNullBitmapTop =
		dstKeyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(dstNullBitmapTop != 0);

	bool	compareResult =
		(this->compareSimpleKey(srcNullBitmapTop, dstNullBitmapTop) == 0);

	checkMemoryExhaust(srcLeafPage);
	checkMemoryExhaust(dstLeafPage);

	return compareResult;
}

//
//	Copyright (c) 2001, 2002, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
