// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_Verify.cpp -- Ｂ＋木ファイルクラスの実現ファイル（整合性検査関連）
// 
// Copyright (c) 2001, 2002, 2004, 2006, 2023 Ricoh Company, Ltd.
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

#include "Exception/MemoryExhaust.h"
#include "Exception/NotSupported.h"

#include "Common/Assert.h"

#include "FileCommon/FileOption.h"

#include "Btree/TreeFile.h"
#include "Btree/FileInformation.h"
#include "Btree/ValueFile.h"
#include "Btree/NodePageHeader.h"
#include "Btree/KeyInformation.h"
#include "Btree/UseInfo.h"
#include "Btree/Message_IllegalFileVersion.h"
#include "Btree/Message_DiscordKeyNum.h"
#include "Btree/Message_ExistTopObject.h"
#include "Btree/Message_ExistLastObject.h"
#include "Btree/Message_DiscordRootNode.h"
#include "Btree/Message_IllegalTreeDepth.h"
#include "Btree/Message_DiscordDelegateKey.h"
#include "Btree/Message_IllegalKeyInfoIndex.h"
#include "Btree/Message_KeyInfoIndexNotEqualZero.h"
#include "Btree/Message_DiscordNextLeaf.h"
#include "Btree/Message_NotUnique.h"
#include "Btree/Message_IllegalNextNodePageID.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::File::setUseInfo --
//		ファイル内で使用しているすべての物理ページと物理エリアを登録する
//
//	NOTES
//	ファイル内で使用しているすべての物理ページと物理エリアを登録する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	Btree::FileInformation&		FileInfo_
//		ファイル管理情報への参照
//	Btree::UseInfo&				TreeFileUseInfo_
//		ツリーファイルの登録情報への参照
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	Btree::UseInfo&				ValueFileUseInfo_
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
File::setUseInfo(const Trans::Transaction&		Transaction_,
				 PhysicalFile::File*			TreePhysicalFile_,
				 FileInformation&				FileInfo_,
				 UseInfo&						TreeFileUseInfo_,
				 ValueFile*						ValueFile_,
				 UseInfo&						ValueFileUseInfo_,
				 Admin::Verification::Progress&	Progress_) const
{
	TreeFileUseInfo_.append(TreeFile::HeaderPageID,
							FileInformation::AreaID);

	ModUInt32	treeDepth = FileInfo_.readTreeDepth();

	PhysicalFile::PageID	rootNodePageID = FileInfo_.readRootNodePageID();

	this->setNodePageUseInfo(Transaction_,
							 TreePhysicalFile_,
							 treeDepth,
							 rootNodePageID,
							 TreeFileUseInfo_,
							 ValueFile_,
							 ValueFileUseInfo_,
							 Progress_);

	//
	// ※ バリューファイルは、生成時に
	// 　 先頭バリューページを必ず生成し、
	// 　 先頭バリューページが破棄されることはない。
	//

	ValueFileUseInfo_.append(0, PhysicalFile::ConstValue::UndefinedAreaID);
}

//
//	FUNCTION private
//	Btree::File::setNodePageUseInfo --
//		ツリーファイル内で使用しているすべてのノードページと
//		ノードページ内で使用しているすべての物理エリアを登録する
//
//	NOTES
//	キーオブジェクトにキーフィールドの値を記録するタイプの
//	ツリーファイル内で使用しているすべてのノードページと
//	ノードページ内で使用しているすべての物理エリアを登録する。
//	バリューファイル内での登録も、この関数経由で行う。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	const ModUInt32				DepthToLeaf_
//		対象となるノードページからリーフページまでの木の深さ
//		（リーフページが対象の場合に、= 1）
//	const PhysicalFile::PageID	NodePageID_
//		対象となるノードページの物理ページ識別子
//	Btree::UseInfo&				TreeFileUseInfo_
//		ツリーファイルの登録情報への参照
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	Btree::UseInfo&				ValueFileUseInfo_
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
File::setNodePageUseInfo(
	const Trans::Transaction&		Transaction_,
	PhysicalFile::File*				TreePhysicalFile_,
	const ModUInt32					DepthToLeaf_,
	const PhysicalFile::PageID		NodePageID_,
	UseInfo&						TreeFileUseInfo_,
	ValueFile*						ValueFile_,
	UseInfo&						ValueFileUseInfo_,
	Admin::Verification::Progress&	Progress_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		this->setSimpleNodePageUseInfo(Transaction_,
									   TreePhysicalFile_,
									   DepthToLeaf_,
									   NodePageID_,
									   TreeFileUseInfo_,
									   ValueFile_,
									   ValueFileUseInfo_,
									   Progress_);

		return;
	}

	; _SYDNEY_ASSERT(
		NodePageID_ != PhysicalFile::ConstValue::UndefinedPageID);

	bool	isLeafPage = (DepthToLeaf_ == 1);

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
									isLeafPage);

			for (ModUInt32 i = 0; i < useKeyInfoNum; i++)
			{
				try
				{
					keyInfo.setStartOffsetByIndex(i);

					ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

					//
					// キーオブジェクトのために使用している
					// ノードページと物理エリアを登録する
					//

					this->setKeyUseInfo(Transaction_,
										TreePhysicalFile_,
										keyObjectID,
										TreeFileUseInfo_,
										Progress_);

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

						this->setNodePageUseInfo(Transaction_,
												 TreePhysicalFile_,
												 DepthToLeaf_ - 1,
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

			} // end for i
		}

		PhysicalFile::PageID	nextNodePageID =
			nodePageHeader.readNextPhysicalPageID();

		TreePhysicalFile_->detachPage(
			nodePage,
			PhysicalFile::Page::UnfixMode::NotDirty);

		if (nextNodePageID == nodePageID)
		{
			ModUnicodeString	areaPath;
			this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

			_SYDNEY_VERIFY_INCONSISTENT(
				Progress_,
				areaPath,
				Message::IllegalNextNodePageID(nodePageID, nextNodePageID));

			break;
		}

		nodePageID = nextNodePageID;

	} // end while nodePageID != UndefinedPageID
}

//
//	FUNCTION private
//	Btree::File::setKeyUseInfo --
//		キーオブジェクトのために使用している
//		ノードページと物理エリアを登録する
//
//	NOTES
//	キーオブジェクトのために使用している
//	ノードページと物理エリアを登録する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*				TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	const ModUInt64					KeyObjectID_
//		代表キーオブジェクトのオブジェクトID
//	Btree::UseInfo&					TreeFileUseInfo_
//		ツリーファイルの登録情報への参照
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
File::setKeyUseInfo(const Trans::Transaction&		Transaction_,
					PhysicalFile::File*				TreePhysicalFile_,
					const ModUInt64					KeyObjectID_,
					UseInfo&						TreeFileUseInfo_,
					Admin::Verification::Progress&	Progress_) const
{
	const PhysicalFile::PageID keyObjectPageID =
		Common::ObjectIDData::getFormerValue(KeyObjectID_);
	const PhysicalFile::AreaID keyObjectAreaID =
		Common::ObjectIDData::getLatterValue(KeyObjectID_);

	TreeFileUseInfo_.append(keyObjectPageID, keyObjectAreaID);

	if (this->m_cFileParameter.m_ExistOutsideFieldInKey)
	{
		// キーフィールドのいずれかが外置きフィールドオブジェクト…

		PhysicalFile::Page*	keyObjectPage =
			TreePhysicalFile_->verifyPage(
				Transaction_,
				keyObjectPageID,
				Buffer::Page::FixMode::ReadOnly,
				Progress_);

		if (Progress_.isGood() == false)
		{
			return;
		}

		; _SYDNEY_ASSERT(keyObjectPage != 0);

		const void*	keyObjectAreaTop =
			File::getConstAreaTop(keyObjectPage,
								  keyObjectAreaID);

		const File::ObjectType*	objectType =
			static_cast<const File::ObjectType*>(keyObjectAreaTop);

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
							keyObjectPage,
							keyObjectAreaID,
							i,
							true)); // キーオブジェクト

				//
				// フィールドの値としてヌル値が記録されている場合、
				// Btree::File::getFieldPointerは
				// "0"（ヌルポインタ）を返す。
				// この場合、外置きフィールドオブジェクトは存在しない。
				//

				if (objectIDReadPos != 0)
				{
					// フィールドの値が、ヌル値ではない…

					ModUInt64	objectID;
					File::readObjectID(objectIDReadPos, objectID);

					try
					{
						File::setOutsideVariableFieldUseInfo(
							Transaction_,
							TreePhysicalFile_,
							keyObjectPage,
							objectID,
							TreeFileUseInfo_,
							Progress_);
					}
					catch (...)
					{
						TreePhysicalFile_->detachPage(
							keyObjectPage,
							PhysicalFile::Page::UnfixMode::NotDirty);

						_SYDNEY_RETHROW;
					}
				}
			}

		} // end for i

		TreePhysicalFile_->detachPage(
			keyObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty);
	}
}

//
//	FUNCTION private
//	Btree::File::setOutsideVariableFieldUseInfo --
//		外置き可変長フィールドオブジェクトのために使用している
//		物理ページと物理エリアを登録する
//
//	NOTES
//	外置き可変長フィールドオブジェクトのために使用している
//	物理ページと物理エリアを登録する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*				PhysicalFile_
//		物理ファイル記述子
//	PhysicalFile::Page*				DirectObjectPage_
//		代表オブジェクトが記録されている物理ページの記述子
//	const ModUInt64					FieldObjectID_
//		外置き可変長フィールドオブジェクトのオブジェクトID
//	Btree::UseInfo&					UseInfo_
//		登録情報への参照
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::setOutsideVariableFieldUseInfo(
	const Trans::Transaction&		Transaction_,
	PhysicalFile::File*				PhysicalFile_,
	PhysicalFile::Page*				DirectObjectPage_,
	const ModUInt64					FieldObjectID_,
	UseInfo&						UseInfo_,
	Admin::Verification::Progress&	Progress_)
{
	const PhysicalFile::PageID fieldObjectPageID =
		Common::ObjectIDData::getFormerValue(FieldObjectID_);
	const PhysicalFile::AreaID fieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(FieldObjectID_);

	UseInfo_.append(fieldObjectPageID, fieldObjectAreaID);

	bool	attached = false;

	PhysicalFile::Page*	fieldObjectPage = 0;

	if (fieldObjectPageID != DirectObjectPage_->getID())
	{
		attached = true;

		fieldObjectPage =
			PhysicalFile_->verifyPage(Transaction_,
									  fieldObjectPageID,
									  Buffer::Page::FixMode::ReadOnly,
									  Progress_);

		if (Progress_.isGood() == false)
		{
			return;
		}
	}
	else
	{
		fieldObjectPage = DirectObjectPage_;
	}

	; _SYDNEY_ASSERT(fieldObjectPage != 0);

	const File::ObjectType*	objectTypeReadPos =
		static_cast<const File::ObjectType*>(
			File::getConstAreaTop(fieldObjectPage, fieldObjectAreaID));

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (*objectTypeReadPos != File::NormalObjectType &&
		*objectTypeReadPos != File::CompressedObjectType)
	{
		; _SYDNEY_ASSERT(
			*objectTypeReadPos == File::DivideObjectType ||
			*objectTypeReadPos == File::DivideCompressedObjectType);

		const char*	nextLinkObjectIDReadPos =
			syd_reinterpret_cast<const char*>(objectTypeReadPos + 1);

		File::readObjectID(nextLinkObjectIDReadPos, nextLinkObjectID);
	}

	if (attached)
	{
		PhysicalFile_->detachPage(fieldObjectPage,
								  PhysicalFile::Page::UnfixMode::NotDirty);
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// リンクオブジェクト…

		// 再帰呼び出し
		File::setOutsideVariableFieldUseInfo(Transaction_,
											 PhysicalFile_,
											 DirectObjectPage_,
											 nextLinkObjectID,
											 UseInfo_,
											 Progress_);
	}
}

//
//	FUNCTION private
//	Btree::File::notifyUsePageAndArea --
//		物理ファイルマネージャに、Ｂ＋木ファイル内で使用している
//		すべての物理ページと物理エリアを通知する
//
//	NOTES
//	物理ファイルマネージャに、Ｂ＋木ファイル内で使用している
//	すべての物理ページと物理エリアを通知する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*				PhysicalFile_
//		物理ファイル記述子
//	Btree::UseInfo&					UseInfo_
//		登録情報への参照
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
File::notifyUsePageAndArea(
	const Trans::Transaction&		Transaction_,
	PhysicalFile::File*				PhysicalFile_,
	UseInfo&						UseInfo_,
	Admin::Verification::Progress&	Progress_) const
{
	ModSize	areaIDArraySize =
		sizeof(PhysicalFile::AreaID) * UseInfo_.m_AreaIDsMaxCount;
	PhysicalFile::AreaID*	areaIDArray =
		static_cast<PhysicalFile::AreaID*>(
			ModDefaultManager::allocate(areaIDArraySize));

	for (PhysicalFile::PageID pageID = 0;
		 pageID <= UseInfo_.m_LastPageID;
		 pageID++)
	{
		const UseInfo::AreaIDs&	areaIDs = UseInfo_.getAreaIDs(pageID);

		if (areaIDs.isEmpty() == ModFalse)
		{
			// 使用中の物理ページ…

			UseInfo::AreaIDs::ConstIterator	areaID = areaIDs.begin();

			PhysicalFile::AreaNum	areaNum = 0;
			PhysicalFile::AreaID*	areaIDArrayPointer = 0;

			if (*areaID != PhysicalFile::ConstValue::UndefinedAreaID)
			{
				UseInfo::AreaIDs::ConstIterator	areaIDsEnd = areaIDs.end();

				int	arrayIndex = 0;

				while (areaID != areaIDsEnd)
				{
					if (*areaID != PhysicalFile::ConstValue::UndefinedAreaID)
					{
						*(areaIDArray + arrayIndex) = *areaID;

						arrayIndex++;
					}

					areaID++;
				}

				areaIDArrayPointer = areaIDArray;

				areaNum = arrayIndex;
			}

			PhysicalFile_->notifyUsePage(Transaction_,
										 Progress_,
										 pageID,
										 areaNum,
										 areaIDArrayPointer);

			if (Progress_.isGood() == false)
			{
				// 検査中断…

				break;
			}
		}
	}

	ModDefaultManager::free(areaIDArray, areaIDArraySize);
}

//
//	FUNCTION private
//	Btree::File::checkBtreeFile --
//		Ｂ＋木ファイル内の整合性検査を行う
//
//	NOTES
//	Ｂ＋木ファイル内の整合性検査を行う。
//
//	ARGUMENTS
//	Btree::FileInformation&			FileInfo_
//		ファイル管理情報への参照
//	Btree::ValueFile*				ValueFile_
//		バリューファイル記述子
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
File::checkBtreeFile(FileInformation&				FileInfo_,
					 ValueFile*						ValueFile_,
					 Admin::Verification::Progress&	Progress_)
{
	ModUInt64	objectNum = FileInfo_.readObjectNum();

	//
	// ① ファイルバージョンの確認
	//

	this->checkFileVersion(FileInfo_, Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// ② リーフページのキー総数検査
	//

	this->checkLeafKeyNum(FileInfo_, objectNum, Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// ③ キー値順での先頭オブジェクトの確認
	//

	this->checkTopObject(FileInfo_, objectNum, Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// ④ キー値順での最終オブジェクトの確認
	//

	this->checkLastObject(FileInfo_, ValueFile_, objectNum, Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// ⑤ ルートノードページの一貫性検査
	//

	this->checkRootNode(FileInfo_, objectNum, Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// ⑥ 子ノードページの代表キー検査
	//

	this->checkDelegateKey(FileInfo_, objectNum, Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// ⑦ リーフページのリンク検査
	//

	this->checkLeafLink(FileInfo_, Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// ⑧ オブジェクトのユニーク性検査
	//

	this->checkObjectUnique(FileInfo_, ValueFile_, objectNum, Progress_);
}

//
//	FUNCTION private
//	Btree::File::checkFileVersion -- ファイルバージョンの確認
//
//	NOTES
//	ファイル管理情報に記録されている「ファイルバージョン」が
//	認識可能なバージョンかどうかを確認する。
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
File::checkFileVersion(FileInformation&					FileInfo_,
					   Admin::Verification::Progress&	Progress_)
{
	FileVersion::Value	fileVersion = FileInfo_.readFileVersion();

	if (fileVersion < FileVersion::Version1 ||
		fileVersion > FileVersion::CurrentVersion)
	{
		ModUnicodeString	areaPath;
		this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

		_SYDNEY_VERIFY_INCONSISTENT(
			Progress_,
			areaPath,
			Message::IllegalFileVersion(static_cast<int>(fileVersion)));
	}
}

//
//	FUNCTION private
//	Btree::File::checkLeafKeyNumber -- リーフページのキー総数検査
//
//	NOTES
//	ファイル管理情報に記録されている「オブジェクト数」と、
//	実際にリーフページ内に記録されているキーの総数が等しいかを検査する。
//
//	ARGUMENTS
//	Btree::FileInformation&			FileInfo_
//		ファイル管理情報への参照
//	const ModUInt64					ObjectNum_
//		挿入されているオブジェクト数
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
File::checkLeafKeyNum(FileInformation&					FileInfo_,
					  const ModUInt64					ObjectNum_,
					  Admin::Verification::Progress&	Progress_)
{
	//
	// オブジェクトが挿入されていなければ、検査不要。
	//
	if (ObjectNum_ == 0)
	{
		return;
	}

	ModUInt64	realObjectNum = 0;

	PhysicalFile::PageID	leafPageID = FileInfo_.readTopLeafPageID();

	; _SYDNEY_ASSERT(
		leafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	do
	{
		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   this->m_pPhysicalFile,
									   leafPageID,
									   Buffer::Page::FixMode::ReadOnly,
									   true,   // リーフページ
									   false); // 物理ページ記述子を
										       // キャッシュしない

		// 次のリーフページの物理ページ識別子を読み込んでおく
		leafPageID = leafPageHeader.readNextLeafPageID();

		ModUInt32	keyInfoNum =
			leafPageHeader.readUseKeyInformationNumber();

		realObjectNum += keyInfoNum;
	}
	while (leafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	if (ObjectNum_ != realObjectNum)
	{
		ModUnicodeString	areaPath;
		this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

		_SYDNEY_VERIFY_INCONSISTENT(
			Progress_,
			areaPath,
			Message::DiscordKeyNum(static_cast<int>(ObjectNum_),
								   static_cast<int>(realObjectNum)));
	}
}

//
//	FUNCTION private
//	Btree::File::checkTopObject -- 先頭オブジェクトの確認
//
//	NOTES
//	ファイル管理情報に記録されている「先頭リーフの物理ページID」で
//	示すリーフ内の先頭キーよりもキー値順で前方に
//	キーが存在しないことを確認する。
//
//	ARGUMENTS
//	Btree::FileInformation&			FileInfo_
//		ファイル管理情報への参照
//	const ModUInt64					ObjectNum_
//		挿入されているオブジェクト数
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
File::checkTopObject(FileInformation&				FileInfo_,
					 const ModUInt64				ObjectNum_,
					 Admin::Verification::Progress&	Progress_)
{
	//
	// オブジェクトが挿入されていなければ、検査不要。
	//

	if (ObjectNum_ == 0)
	{
		return;
	}

	PageVector	attachNodePages;

	this->m_ullObjectID = this->getTopObjectID(FileInfo_, attachNodePages);

	this->m_LeafPageID = FileInfo_.readTopLeafPageID();
	this->m_KeyInfoIndex = 0;

	if (this->getPrevObjectIDScan(attachNodePages) !=
		FileCommon::ObjectID::Undefined)
	{
		ModUnicodeString	areaPath;
		this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

		_SYDNEY_VERIFY_INCONSISTENT(Progress_,
									areaPath,
									Message::ExistTopObject());
	}

	this->m_SavePage = false;

	File::detachPageAll(this->m_pPhysicalFile,attachNodePages,PhysicalFile::Page::UnfixMode::NotDirty,false);
}

//
//	FUNCTION private
//	Btree::File::checkLastObject -- 最終オブジェクトの確認
//
//	NOTES
//	ファイル管理情報に記録されている「最終リーフの物理ページID」で
//	示すリーフ内の最終キーよりもキー値順で後方に
//	キーが存在しないことを確認する。
//
//	ARGUMENTS
//	Btree::FileInformation&			FileInfo_
//		ファイル管理情報への参照
//	Btree::ValueFile*				ValueFile_
//		バリューファイル記述子
//	const ModUInt64					ObjectNum_
//		挿入されているオブジェクト数
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
File::checkLastObject(FileInformation&					FileInfo_,
					  ValueFile*						ValueFile_,
					  const ModUInt64					ObjectNum_,
					  Admin::Verification::Progress&	Progress_)
{
	//
	// オブジェクトが挿入されていなければ、検査不要。
	//

	if (ObjectNum_ == 0)
	{
		return;
	}

	PageVector	attachNodePages;

	this->m_ullObjectID = this->getLastObjectID(FileInfo_, attachNodePages);

	ValueFile_->readLeafInfo(this->m_ullObjectID,
							 this->m_LeafPageID,
							 this->m_KeyInfoIndex);

	if (this->getNextObjectIDScan(attachNodePages) !=
		FileCommon::ObjectID::Undefined)
	{
		ModUnicodeString	areaPath;
		this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

		_SYDNEY_VERIFY_INCONSISTENT(
			Progress_,
			areaPath,
			Message::ExistLastObject());
	}

	this->m_SavePage = false;

	File::detachPageAll(this->m_pPhysicalFile,attachNodePages,PhysicalFile::Page::UnfixMode::NotDirty,false);
}

//
//	FUNCTION private
//	Btree::File::checkRootNode -- ルートノードページの一貫性検査
//
//	NOTES
//	各リーフページについて、上位ノードページを辿っていき、
//	すべてのリーフページが同一ルートノードページまで辿るかどうかを
//	検査する。
//
//	ARGUMENTS
//	Btree::FileInformation&			FileInfo_
//		ファイル管理情報への参照
//	const ModUInt64					ObjectNum_
//		挿入されているオブジェクト数
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
File::checkRootNode(FileInformation&				FileInfo_,
					const ModUInt64					ObjectNum_,
					Admin::Verification::Progress&	Progress_)
{
	ModUInt32	treeDepth = FileInfo_.readTreeDepth();

	PhysicalFile::PageID	rootNodePageID = FileInfo_.readRootNodePageID();

	if (ObjectNum_ == 0)
	{
		// オブジェクトが挿入されていない…

		//
		// オブジェクトが挿入されていなければ、
		// リーフページがルートノードページであることのみを
		// チェックする。
		// また、木の深さもチェックする。
		//

		PhysicalFile::PageID	topLeafPageID =
			FileInfo_.readTopLeafPageID();

		if (rootNodePageID != topLeafPageID)
		{
			ModUnicodeString	areaPath;
			this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

			_SYDNEY_VERIFY_INCONSISTENT(
				Progress_,
				areaPath,
				Message::DiscordRootNode(topLeafPageID,
										 rootNodePageID,
										 topLeafPageID));
		}
		else if (treeDepth > 1)
		{
			ModUnicodeString	areaPath;
			this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

			_SYDNEY_VERIFY_INCONSISTENT(
				Progress_,
				areaPath,
				Message::IllegalTreeDepth(treeDepth));
		}
	}
	else
	{
		// オブジェクトが挿入されている…

		PhysicalFile::PageID	leafPageID =
			FileInfo_.readTopLeafPageID();

		; _SYDNEY_ASSERT(
			leafPageID != PhysicalFile::ConstValue::UndefinedPageID);

		do
		{
			NodePageHeader	leafPageHeader(this->m_pTransaction,
										   this->m_pPhysicalFile,
										   leafPageID,
										   Buffer::Page::FixMode::ReadOnly,
										   true,   // リーフページ
										   false); // 物理ページ記述子を
										           // キャッシュしない

			PhysicalFile::PageID	currentLeafPageID = leafPageID;

			PhysicalFile::PageID	nodePageID = leafPageID;

			// 次のリーフページの物理ページ識別子を読み込んでおく
			leafPageID = leafPageHeader.readNextLeafPageID();

			bool	isLeafPage = true;

			for (ModUInt32 depth = treeDepth; depth > 1; depth--)
			{
				NodePageHeader
					nodePageHeader(this->m_pTransaction,
								   this->m_pPhysicalFile,
								   nodePageID,
								   Buffer::Page::FixMode::ReadOnly,
								   isLeafPage,
								   false); // 物理ページ記述子を
								           // キャッシュしない

				nodePageID = nodePageHeader.readParentNodePageID();

				if (nodePageID == PhysicalFile::ConstValue::UndefinedPageID)
				{
					break;
				}

				isLeafPage = false;
			}

			if (nodePageID != rootNodePageID)
			{
				ModUnicodeString	areaPath;
				this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

				_SYDNEY_VERIFY_INCONSISTENT(
					Progress_,
					areaPath,
					Message::DiscordRootNode(currentLeafPageID,
											 rootNodePageID,
											 nodePageID));

				break;
			}
		}
		while (leafPageID != PhysicalFile::ConstValue::UndefinedPageID);
	}
}

//
//	FUNCTION private
//	Btree::File::checkDelegateKey -- 子ノードページの代表キー検査
//
//	NOTES
//	ツリーファイル内の、子ノードページの代表キーに不正がないかを検査する。
//
//	検査方法：
//		ルートノードページ以外のノードページの代表キーと一致するキーが、
//		その親ノードページに記録されているかを検査する。
//		（オブジェクトの挿入ソート順が昇順の場合には、
//		　代表キーは子ノードページ内でキー値が最大のキーとなり、
//		　降順の場合には最小のキーとなる。）
//
//	ARGUMENTS
//	Btree::FileInformation&			FileInfo_
//		ファイル管理情報への参照
//	const modUInt64					ObjectNum_
//		挿入されているオブジェクト数
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
File::checkDelegateKey(FileInformation&					FileInfo_,
					   const ModUInt64					ObjectNum_,
					   Admin::Verification::Progress&	Progress_)
{
	//
	// オブジェクトが挿入されていなければ、検査不要。
	//

	if (ObjectNum_ == 0)
	{
		return;
	}

	ModUInt32	treeDepth = FileInfo_.readTreeDepth();

	PhysicalFile::PageID	rootNodePageID =
		FileInfo_.readRootNodePageID();

	PageVector	attachNodePages;

	while (true)
	{

		try
		{
			if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
			{
				this->checkDelegateSimpleKey(treeDepth,
											 rootNodePageID,
											 attachNodePages,
											 Progress_);
			}
			else
			{
				this->checkDelegateKey(treeDepth,
									   rootNodePageID,
									   attachNodePages,
									   Progress_);
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
				this->m_CatchMemoryExhaust = true;

				File::detachPageAll(this->m_pPhysicalFile,attachNodePages,PhysicalFile::Page::UnfixMode::NotDirty,false);
			}
		}
	}

	File::detachPageAll(this->m_pPhysicalFile,attachNodePages,PhysicalFile::Page::UnfixMode::NotDirty,false);
}

//
//	FUNCTION private
//	Btree::File::checkDelegateKey -- 子ノードページの代表キー検査
//		
//
//	NOTES
//	キーオブジェクトにキーフィールドの値が記録されているタイプの
//	ツリーファイル内の、子ノードページの代表キーに不正がないかを検査する。
//
//	ARGUMENTS
//	const ModUInt32					NodeDepth_
//		ルートノードページからの段数（ルートノードページ = 1）
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
File::checkDelegateKey(const ModUInt32					NodeDepth_,
					   const PhysicalFile::PageID		NodePageID_,
					   PageVector&						AttachNodePages_,
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
							0,      // とりあえず先頭のキー情報
							false); // リーフページではない

	for (ModUInt32 i = 0; i < useKeyInfoNum; i++)
	{
		keyInfo.setStartOffsetByIndex(i);

		ModUInt64	parentNodeKeyObjectID = keyInfo.readKeyObjectID();

		PhysicalFile::PageID	childNodePageID =
			keyInfo.readChildNodePageID();

		PhysicalFile::Page*	childNodePage =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 childNodePageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		ModUInt64	childNodeKeyObjectID =
			this->getLastObjectIDInNode(childNodePage,
										childNodeIsLeaf,
										false); // キーオブジェクトの
											    // オブジェクトIDを得る

		checkMemoryExhaust(childNodePage);

		if (this->compareKeyObject(parentNodeKeyObjectID,
								   childNodeKeyObjectID,
								   AttachNodePages_)
			!= 0)
		{
			ModUnicodeString	areaPath;
			this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

			_SYDNEY_VERIFY_INCONSISTENT(
				Progress_,
				areaPath,
				Message::DiscordDelegateKey(
					Common::ObjectIDData::getFormerValue(
						parentNodeKeyObjectID),
					Common::ObjectIDData::getLatterValue(
						parentNodeKeyObjectID),
					Common::ObjectIDData::getFormerValue(
						childNodeKeyObjectID),
					Common::ObjectIDData::getLatterValue(
						childNodeKeyObjectID)));

			break;
		}

		this->checkDelegateKey(NodeDepth_ - 1,
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
//	Btree::File::checkLeafLink -- リーフページのリンク検査
//
//	NOTES
//	キーオブジェクトにキーフィールドの値が記録されているタイプの
//	ツリーファイル内の、リーフページ同士のリンクに不正がないかを検査する。
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
File::checkLeafLink(FileInformation&				FileInfo_,
					Admin::Verification::Progress&	Progress_)
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		this->checkSimpleLeafLink(FileInfo_, Progress_);
		
		return;
	}

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

				this->searchKeyInformation(
					leafPage,
					parentNodePage,
					keyInfoIndex,
					attachNodePages,
					true); // 子ノードページはリーフページ

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
											false); // リーフページではない

					if (keyInfo.readChildNodePageID() != nextLeafPageID)
					{
						ModUnicodeString	areaPath;
						this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

						_SYDNEY_VERIFY_INCONSISTENT(
							Progress_,
							areaPath,
							Message::DiscordNextLeaf(keyInfo.readChildNodePageID(),
													 nextLeafPageID));

						checkMemoryExhaust(leafPage,parentNodePage,nextLeafPage); // 本当にデタッチ（アンフィックス）

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

					this->searchKeyInformation(nextLeafPage,
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

			} // end while true

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

	} // end while true

	File::detachPageAll(this->m_pPhysicalFile,attachNodePages,PhysicalFile::Page::UnfixMode::NotDirty,false);
}

//
//	FUNCTION private
//	Btree::File::checkObjectUnique -- オブジェクトのユニーク性検査
//
//	NOTES
//	オブジェクトのユニーク性が指定されているファイルについて、
//	重複オブジェクトが存在しないことを確認する。
//	オブジェクトのユニーク性は、
//	ファイルIDで『キーフィールドの値でユニーク』または
//	『オブジェクト（キー＋バリュー）の値』でユニークと指定される。
//
//	ARGUMENTS
//	Btree::FileInformation&			FileInfo_
//		ファイル管理情報への参照
//	Btree::ValueFile*				ValueFile_
//		バリューファイル記述子
//	const ModUInt64					ObjectNum_
//		挿入されているオブジェクト数
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
File::checkObjectUnique(FileInformation&				FileInfo_,
						ValueFile*						ValueFile_,
						const ModUInt64					ObjectNum_,
						Admin::Verification::Progress&	Progress_)
{
	//
	// 以下の条件の場合には、検査不要。
	//     ① オブジェクトのユニーク指定がされていない
	//     ② オブジェクトが挿入されていない
	//

	if (this->m_cFileParameter.m_UniqueType ==
		FileParameter::UniqueType::NotUnique ||
		ObjectNum_ == 0)
	{
		return;
	}

	PageVector	attachNodePages;
	PageVector	attachValuePages;

	try
	{
		ModUInt64	srcObjectID = this->getTopObjectID(FileInfo_,
													   attachNodePages);
		while (true)
		{
			ValueFile_->readLeafInfo(srcObjectID,
									 this->m_LeafPageID,
									 this->m_KeyInfoIndex);

			ModUInt64	dstObjectID = this->getNextObjectIDScan(attachNodePages);

			if (dstObjectID == FileCommon::ObjectID::Undefined)
			{
				break;
			}

			PhysicalFile::PageID	srcLeafPageID = this->m_LeafPageID;
			ModUInt32				srcKeyInfoIndex = this->m_KeyInfoIndex;

			PhysicalFile::PageID	dstLeafPageID =
				PhysicalFile::ConstValue::UndefinedPageID;
			ModUInt32				dstKeyInfoIndex = ModUInt32Max;

			ValueFile_->readLeafInfo(dstObjectID,
									 dstLeafPageID,
									 dstKeyInfoIndex);

			bool	isUnique = true;

			if (this->compareKeyObjectForVerify(srcLeafPageID,
												srcKeyInfoIndex,
												dstLeafPageID,
												dstKeyInfoIndex,
												attachNodePages))
			{
				// キーオブジェクトが等しい…

				if (this->m_cFileParameter.m_UniqueType ==
					FileParameter::UniqueType::Object)
				{
					// オブジェクト値でユニーク…

					if (this->compareObjectForVerify(
							ValueFile_->m_PhysicalFile,
							srcObjectID,
							dstObjectID,
							false, // バリューオブジェクト
							attachValuePages))
					{
						// バリューオブジェクトも等しい…

						//
						// オブジェクトがユニークではなくなっている。
						//

						isUnique = false;
					}
				}
				else
				{
					// キー値でユニーク…

					//
					// オブジェクトがユニークではなくなっている。
					//

					isUnique = false;
				}

				if (isUnique == false)
				{
					ModUnicodeString	areaPath;
					this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

					_SYDNEY_VERIFY_INCONSISTENT(
						Progress_,
						areaPath,
						Message::NotUnique(
							Common::ObjectIDData::getFormerValue(srcObjectID),
							Common::ObjectIDData::getLatterValue(srcObjectID),
							Common::ObjectIDData::getFormerValue(dstObjectID),
							Common::ObjectIDData::getLatterValue(dstObjectID)));

					break;
				}
			}

			srcObjectID = dstObjectID;

		} // end while true
	}
	catch (...)
	{
		File::detachPageAll(this->m_pPhysicalFile,attachNodePages,ValueFile_->m_PhysicalFile,attachValuePages,PhysicalFile::Page::UnfixMode::NotDirty,false);

		_SYDNEY_RETHROW;
	}

	File::detachPageAll(this->m_pPhysicalFile,attachNodePages,ValueFile_->m_PhysicalFile,attachValuePages,PhysicalFile::Page::UnfixMode::NotDirty,false);
}

//
//	FUNCTION private
//	Btree::File::compareKeyObjectForVerify --
//		キーフィールドの値を比較する
//
//	NOTES
//	整合性検査のために、キーオブジェクトに記録されている
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
File::compareKeyObjectForVerify(
	const PhysicalFile::PageID	SrcLeafPageID_,
	const ModUInt32				SrcKeyInfoIndex_,
	const PhysicalFile::PageID	DstLeafPageID_,
	const ModUInt32				DstKeyInfoIndex_,
	PageVector&					AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->compareSimpleKeyForVerify(SrcLeafPageID_,
											   SrcKeyInfoIndex_,
											   DstLeafPageID_,
											   DstKeyInfoIndex_,
											   AttachNodePages_);
	}

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
							   true); // リーフページ

	ModUInt64	srcKeyObjectID = srcKeyInfo.readKeyObjectID();

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
							   true); // リーフページ

	ModUInt64	dstKeyObjectID = dstKeyInfo.readKeyObjectID();

	bool	compareResult =
		this->compareObjectForVerify(this->m_pPhysicalFile,
									 srcKeyObjectID,
									 dstKeyObjectID,
									 true, // キーオブジェクト
									 AttachNodePages_);

	checkMemoryExhaust(srcLeafPage);
	checkMemoryExhaust(dstLeafPage);

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareObjectForVerify --
//		オブジェクトの値を比較する
//
//	NOTES
//	キーオブジェクトまたはバリューオブジェクトの値を比較する。
//	オブジェクトのユニーク性検査のためのメソッドなので、
//	いずれかのオブジェクトがヌル値のフィールドを含んでいる場合には、
//	falseを返す。
//
//	ARGUMENTS
//	PhysicalFile::File*	PhysicalFile_
//		物理ページ記述子
//	const ModUInt64		SrcObjectID_
//		比較元代表キーオブジェクト／バリューオブジェクトのオブジェクトID
//	const ModUInt64		DstObjectID_
//		比較先代表キーオブジェクト／バリューオブジェクトのオブジェクトID
//	const bool			IsKeyObject_
//		オブジェクトがキーオブジェクトかバリューオブジェクトか
//			true  : キーオブジェクト
//			false : バリューオブジェクト
//	Btree::PageVector&	AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページの記述子をつむ）
//
//	RETURN
//	bool
//		比較結果
//			true  : 2つのオブジェクトの値が等しい
//			false : 2つのオブジェクトの値が異なる
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareObjectForVerify(PhysicalFile::File*	PhysicalFile_,
							 const ModUInt64		SrcObjectID_,
							 const ModUInt64		DstObjectID_,
							 const bool				IsKeyObject_,
							 PageVector&			AttachPages_) const
{
	PhysicalFile::Page*	srcObjectPage = File::attachPage(
		m_pTransaction, PhysicalFile_,
		Common::ObjectIDData::getFormerValue(SrcObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachPages_);

	PhysicalFile::Page*	dstObjectPage = File::attachPage(
		m_pTransaction, PhysicalFile_,
		Common::ObjectIDData::getFormerValue(DstObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachPages_);

	bool compareResult = this->compareObjectForVerify(
		srcObjectPage,
		Common::ObjectIDData::getLatterValue(SrcObjectID_),
		dstObjectPage,
		Common::ObjectIDData::getLatterValue(DstObjectID_),
		IsKeyObject_, AttachPages_);

	if (this->m_CatchMemoryExhaust)
	{
		PhysicalFile_->detachPage(
			srcObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
				    // 物理ファイルマネージャがキャッシュしないようにする。	

		PhysicalFile_->detachPage(
			dstObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
				    // 物理ファイルマネージャがキャッシュしないようにする。	
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareObjectForVerify --
//		オブジェクトの値を比較する
//
//	NOTES
//	キーオブジェクトまたはバリューオブジェクトの値を比較する。
//	オブジェクトのユニーク性検査のためのメソッドなので、
//	いずれかのオブジェクトがヌル値のフィールドを含んでいる場合には、
//	falseを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*			SrcObjectPage_
//		比較元代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理ページの記述子
//	const PhysicalFile::AreaID	SrcObjectAreaID_
//		比較元代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理エリアの識別子
//	PhysicalFile::Page*			DstObjectPage_
//		比較先代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理ページの記述子
//	const PhysicalFile::AreaID	DstObjectAreaID_
//		比較先代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理エリアの識別子
//	const bool					IsKeyObject_
//		オブジェクトがキーオブジェクト化バリューオブジェクトか
//			true  : キーオブジェクト
//			false : バリューオブジェクト
//	Btree::PageVector&			AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページの記述子をつむ）
//
//	RETURN
//	bool
//		比較結果
//			true  : 2つのオブジェクトの値が等しい
//			false : 2つのオブジェクトの値が異なる
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareObjectForVerify(PhysicalFile::Page*		SrcObjectPage_,
							 const PhysicalFile::AreaID	SrcObjectAreaID_,
							 PhysicalFile::Page*		DstObjectPage_,
							 const PhysicalFile::AreaID	DstObjectAreaID_,
							 const bool					IsKeyObject_,
							 PageVector&				AttachPages_) const
{
	; _SYDNEY_ASSERT(
		SrcObjectAreaID_ != PhysicalFile::ConstValue::UndefinedAreaID);
	; _SYDNEY_ASSERT(
		DstObjectAreaID_ != PhysicalFile::ConstValue::UndefinedAreaID);

	int	startIndex =
		IsKeyObject_ ? 1 : this->m_cFileParameter.m_TopValueFieldIndex;

	int	stopIndex =
		IsKeyObject_ ? this->m_cFileParameter.m_TopValueFieldIndex :
					   this->m_cFileParameter.m_FieldNum;

	bool	compareResult = true;

	for (int i = startIndex; i < stopIndex; i++)
	{
		if (*(this->m_cFileParameter.m_IsArrayFieldArray + i))
		{
			// 配列フィールド…

			; _SYDNEY_ASSERT(IsKeyObject_ == false);

			compareResult =
				this->compareArrayFieldForVerify(i,
												 SrcObjectPage_,
												 SrcObjectAreaID_,
												 DstObjectPage_,
												 DstObjectAreaID_,
												 AttachPages_);
		}
		else if (*(this->m_cFileParameter.m_IsFixedFieldArray + i))
		{
			// 固定長フィールド

			compareResult =
				this->compareFixedFieldForVerify(i,
												 SrcObjectPage_,
												 SrcObjectAreaID_,
												 DstObjectPage_,
												 DstObjectAreaID_,
												 IsKeyObject_);
		}
		else
		{
			// 可変長フィールド…

			compareResult =
				this->compareVariableFieldForVerify(i,
													SrcObjectPage_,
													SrcObjectAreaID_,
													DstObjectPage_,
													DstObjectAreaID_,
													IsKeyObject_,
													AttachPages_);
		}

		if (compareResult == false)
		{
			// フィールドが異なる…

			break;
		}
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareFixedFieldForVerify --
//		固定長フィールドの値を比較する
//
//	NOTES
//	固定長フィールドの値を比較する。
//	オブジェクトのユニーク性検査のためのメソッドなので、
//	いずれかのオブジェクトがヌル値の場合には、falseを返す。
//
//	ARGUMENTS
//	const int					FieldIndex_
//		比較対象固定長フィールドのインデックス
//	PhysicalFile::Page*			SrcObjectPage_
//		比較元代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理ページの記述子
//	const PhysicalFile::AreaID	SrcObjectAreaID_
//		比較元代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理エリアの識別子
//	PhysicalFile::Page*			DstObjectPage_
//		比較先代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理ページの記述子
//	const PhysicalFile::AreaID	DstObjectAreaID_
//		比較先代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理エリアの識別子
//	const bool					IsKeyObject_
//		キーオブジェクト、バリューオブジェクトいずれのフィールドか
//			true  : キーオブジェクトのフィールド
//			false : バリューオブジェクトのフィールド
//
//	RETURN
//	bool
//		比較結果
//			true  : 2つの固定長フィールドの値が等しい
//			false : 2つの固定長フィールドの値が異なる
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareFixedFieldForVerify(
	const int					FieldIndex_,
	PhysicalFile::Page*			SrcObjectPage_,
	const PhysicalFile::AreaID	SrcObjectAreaID_,
	PhysicalFile::Page*			DstObjectPage_,
	const PhysicalFile::AreaID	DstObjectAreaID_,
	const bool					IsKeyObject_) const
{
	void*	srcValue = this->getFieldPointer(SrcObjectPage_,
											 SrcObjectAreaID_,
											 FieldIndex_,
											 IsKeyObject_);

	if (srcValue == 0)
	{
		//
		// ヌル値のフィールドならば、そのオブジェクトはユニーク。
		// （この関数的には、“異なる”ということになる。）
		//

		return false;
	}

	void*	dstValue = this->getFieldPointer(DstObjectPage_,
											 DstObjectAreaID_,
											 FieldIndex_,
											 IsKeyObject_);

	if (dstValue == 0)
	{
		//
		// 同上
		//

		return false;
	}

	int	compareResult = 0;

	Common::DataType::Type	fieldType =
		*(this->m_cFileParameter.m_FieldTypeArray + FieldIndex_);

	if (fieldType == Common::DataType::Integer)
	{
		compareResult = this->compareIntegerField(srcValue, dstValue);
	}
	else if (fieldType == Common::DataType::UnsignedInteger)
	{

		compareResult = this->compareUnsignedIntegerField(srcValue,
														  dstValue);
	}
	else if (fieldType == Common::DataType::Integer64)
	{
		compareResult = this->compareInteger64Field(srcValue, dstValue);
	}
	else if (fieldType == Common::DataType::UnsignedInteger64)
	{
		compareResult = this->compareUnsignedInteger64Field(srcValue,
															dstValue);
	}
	else if (fieldType == Common::DataType::Float)
	{
		compareResult = this->compareFloatField(srcValue, dstValue);
	}
	else if (fieldType == Common::DataType::Double)
	{
		compareResult = this->compareDoubleField(srcValue, dstValue);
	}
	else if (fieldType == Common::DataType::Date)
	{
		compareResult = this->compareDateField(srcValue, dstValue);
	}
	else if (fieldType == Common::DataType::DateTime)
	{
		compareResult = this->compareTimeField(srcValue, dstValue);
	}
	else if (fieldType == Common::DataType::ObjectID)
	{
		compareResult = this->compareObjectIDField(srcValue, dstValue);
	}
	else
	{
		throw Exception::NotSupported(moduleName, srcFile, __LINE__);
	}

	return compareResult == 0;
}

//
//	FUNCTION private
//	Btree::File::compareArrayFieldForVerify --
//		配列フィールドの値を比較する
//
//	NOTES
//	配列フィールドの値を比較する。
//	オブジェクトのユニーク性検査のためのメソッドなので、
//	いずれかのオブジェクトがヌル値の場合には、falseを返す。
//
//	ARGUMENTS
//	const int					FieldIndex_
//		比較対象配列フィールドのインデックス
//	PhysicalFile::Page*			SrcObjectPage_
//		比較元代表バリューオブジェクトが記録されている物理ページの記述子
//	const PhysicalFile::AreaID	SrcObjectAreaID_
//		比較元代表バリューオブジェクトが記録されている物理エリアの識別子
//	PhysicalFile::Page*			DstObjectPage_
//		比較先代表バリューオブジェクトが記録されている物理ページの記述子
//	const PhysicalFile::AreaID	DstObjectAreaID_
//		比較先代表バリューオブジェクトが記録されている物理エリアの識別子
//	Btree::PageVector&			AttachValuePages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページの記述子をつむ）
//
//	RETURN
//	bool
//		比較結果
//			true  : 2つの配列フィールドの値が等しい
//			false : 2つの配列フィールドの値が異なる
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareArrayFieldForVerify(
	const int					FieldIndex_,
	PhysicalFile::Page*			SrcObjectPage_,
	const PhysicalFile::AreaID	SrcObjectAreaID_,
	PhysicalFile::Page*			DstObjectPage_,
	const PhysicalFile::AreaID	DstObjectAreaID_,
	PageVector&					AttachValuePages_) const
{
	char*	srcFieldObjectIDReadPos =
		static_cast<char*>(
			this->getFieldPointer(SrcObjectPage_,
								  SrcObjectAreaID_,
								  FieldIndex_,
								  false)); // バリューオブジェクト

	if (srcFieldObjectIDReadPos == 0)
	{
		//
		// ヌル値のフィールドならば、そのオブジェクトはユニーク。
		// （この関数的には、“異なる”ということになる。）
		//

		return false;
	}

	char*	dstFieldObjectIDReadPos =
		static_cast<char*>(
			this->getFieldPointer(DstObjectPage_,
								  DstObjectAreaID_,
								  FieldIndex_,
								  false)); // バリューオブジェクト

	if (dstFieldObjectIDReadPos == 0)
	{
		//
		// 同上
		//

		return false;
	}

	ModUInt64	srcFieldObjectID;
	File::readObjectID(srcFieldObjectIDReadPos, srcFieldObjectID);

	ModUInt64	dstFieldObjectID;
	File::readObjectID(dstFieldObjectIDReadPos, dstFieldObjectID);

	Common::DataType::Type	elementDataType =
		*(this->m_cFileParameter.m_ElementTypeArray + FieldIndex_);

	return this->compareArrayFieldForVerify(elementDataType,
											SrcObjectPage_,
											srcFieldObjectID,
											DstObjectPage_,
											dstFieldObjectID,
											AttachValuePages_);
}

//
//	FUNCTION private
//	Btree::File::hasNullElement -- ヌル値の要素が含まれているか調べる
//		
//
//	NOTES
//	引数ArrayField_にヌル値の要素が含まれているか調べる。
//
//	ARGUMENTS
//	const Common::DataArrayData&	ArrayField_
//		配列フィールドオブジェクトへの参照
//
//	RETURN
//	bool
//		ヌル値の要素が含まれているかどうか
//			true  : ヌル値の要素が含まれている
//			false : ヌル値の要素が含まれていない
//
//	EXCEPTIONS
//	[YET!]
//
// static
bool
File::hasNullElement(const Common::DataArrayData&	ArrayField_)
{
	int	elementNum = ArrayField_.getCount();

	for (int i = 0; i < elementNum; i++)
	{
		const Common::Data*	element = ArrayField_.getElement(i).get();

		if (element->isNull())
		{
			return true;
		}
	}

	return false;
}

//
//	FUNCTION private
//	Btree::File::compareArrayFieldForVerify --
//		配列フィールドの値を比較する
//
//	NOTES
//	配列フィールドの値を比較する。
//	オブジェクトのユニーク性検査のためのメソッドなので、
//	いずれかのオブジェクトがヌル値の場合には、falseを返す。
//
//	ARGUMENTS
//	const Common::DataType::Type	ElementDataType_
//		要素のデータ型
//	PhysicalFile::Page*				SrcDirectObjectPage_
//		比較元代表バリューオブジェクトが記録されている物理ページ
//		先頭へのポインタ
//	const ModUInt64					SrcFieldObjectID_
//		比較元配列フィールドオブジェクトのID
//	PhysicalFile::Page*				DstDirectObjectPage_
//		比較先代表バリューオブジェクトが記録されている物理ページ
//		先頭へのポインタ
//	const ModUInt64					DstFieldObjectID_
//		比較先配列フィールドオブジェクトのID
//	Btree::PageVector&				AttachValuePages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページの記述子をつむ）
//
//	RETURN
//	bool
//		比較結果
//			true  : 2つの配列フィールドの値が等しい
//			false : 2つの配列フィールドの値が異なる
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareArrayFieldForVerify(
	const Common::DataType::Type	ElementDataType_,
	PhysicalFile::Page*				SrcDirectObjectPage_,
	const ModUInt64					SrcFieldObjectID_,
	PhysicalFile::Page*				DstDirectObjectPage_,
	const ModUInt64					DstFieldObjectID_,
	PageVector&						AttachValuePages_) const
{
	Common::DataArrayData	srcArrayField;

	this->m_ValueFile->readArrayField(SrcDirectObjectPage_,
									  SrcFieldObjectID_,
									  ElementDataType_,
									  srcArrayField,
									  this->m_CatchMemoryExhaust,
									  AttachValuePages_);

	if (this->hasNullElement(srcArrayField))
	{
		//
		// いずれかの要素にヌル値が記録されているのならば、
		// そのオブジェクトはユニーク。
		// （この関数的には、“異なる”ということになる。）
		//

		return false;
	}

	int	srcElementNum = srcArrayField.getCount();

	Common::DataArrayData	dstArrayField;

	this->m_ValueFile->readArrayField(DstDirectObjectPage_,
									  DstFieldObjectID_,
									  ElementDataType_,
									  dstArrayField,
									  this->m_CatchMemoryExhaust,
									  AttachValuePages_);

	if (this->hasNullElement(dstArrayField))
	{
		//
		// 同上
		//

		return false;
	}

	int	dstElementNum = dstArrayField.getCount();

	if (srcElementNum != dstElementNum)
	{
		//
		// 同上
		//

		return false;
	}

	for (int i = 0; i < srcElementNum; i++)
	{
		Common::Data*	srcElement = srcArrayField.getElement(i).get();
		Common::Data*	dstElement = dstArrayField.getElement(i).get();

		if (srcElement->compareTo(dstElement) != 0)
		{
			// Common::Data::compareTo() の返り値
			//		0	自分自身と与えたデータは等しい
			//		-1	自分自身のほうが小さい
			//		1	与えたデータのほうが小さい
			return false;
		}
	}

	return true;
}

//
//	FUNCTION private
//	Btree::File::compareVariableFieldForVerify --
//		可変長フィールドの値を比較する
//
//	NOTES
//	可変長フィールドの値を比較する。
//	オブジェクトのユニーク性検査のためのメソッドなので、
//	いずれかのオブジェクトがヌル値の場合には、falseを返す。
//
//	ARGUMENTS
//	const int					FieldIndex_
//		比較対象可変長フィールドのインデックス
//	PhysicalFile::Page*			SrcObjectPage_
//		比較元代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理ページの記述子
//	const PhysicalFile::AreaID	SrcObjectAreaID_
//		比較元代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理エリアの識別子
//	PhysicalFile::Page*			DstObjectPage_
//		比較先代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理ページの記述子
//	const PhysicalFile::AreaID	DstObjectAreaID_
//		比較先代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理エリアの識別子
//	const bool					IsKeyObject_
//		キーオブジェクト、バリューオブジェクトいずれのフィールドか
//			true  : キーオブジェクトのフィールド
//			false : バリューオブジェクトのフィールド
//	Btree::PageVector&			AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページの記述子をつむ）
//
//	RETURN
//	bool
//		比較結果
//			true  : 2つの可変長フィールドの値が等しい
//			false : 2つの可変長フィールドの値が異なる
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareVariableFieldForVerify(
	const int					FieldIndex_,
	PhysicalFile::Page*			SrcObjectPage_,
	const PhysicalFile::AreaID	SrcObjectAreaID_,
	PhysicalFile::Page*			DstObjectPage_,
	const PhysicalFile::AreaID	DstObjectAreaID_,
	const bool					IsKeyObject_,
	PageVector&					AttachPages_) const
{
	void*	srcField = this->getFieldPointer(SrcObjectPage_,
											 SrcObjectAreaID_,
											 FieldIndex_,
											 IsKeyObject_);

	if (srcField == 0)
	{
		//
		// ヌル値のフィールドならば、そのオブジェクトはユニーク。
		// （この関数的には、“異なる”ということになる。）
		//

		return false;
	}

	void*	dstField = this->getFieldPointer(DstObjectPage_,
											 DstObjectAreaID_,
											 FieldIndex_,
											 IsKeyObject_);

	if (dstField == 0)
	{
		//
		// 同上
		//

		return false;
	}

	int	compareResult = 0;

	if (*(this->m_cFileParameter.m_FieldOutsideArray + FieldIndex_))
	{
		// 外置き可変長フィールド…

		ModUInt64	srcFieldObjectID;
		File::readObjectID(static_cast<char*>(srcField),
						   srcFieldObjectID);

		ModUInt64	dstFieldObjectID;
		File::readObjectID(static_cast<char*>(dstField),
						   dstFieldObjectID);

		return
			this->compareOutsideVariableFieldForVerify(
				FieldIndex_,
				srcFieldObjectID,
				SrcObjectPage_,
				dstFieldObjectID,
				DstObjectPage_,
				AttachPages_);
	}
	else
	{
		// 外置き可変長フィールドではない…

		Common::DataType::Type	fieldType =
			*(this->m_cFileParameter.m_FieldTypeArray + FieldIndex_);

		File::InsideVarFieldLen*	srcFieldLen =
			static_cast<File::InsideVarFieldLen*>(srcField);

		File::InsideVarFieldLen*	dstFieldLen =
			static_cast<File::InsideVarFieldLen*>(dstField);

		return this->compareVariableFieldForVerify(fieldType,
												   srcFieldLen + 1,
												   *srcFieldLen,
												   dstFieldLen + 1,
												   *dstFieldLen);
	}
}

//
//	FUNCTION private
//	Btree::File::compareVariableFieldForVerify --
//		可変長フィールドの値を比較する
//
//	NOTES
//	可変長フィールドの値を比較する。
//	オブジェクトのユニーク性検査のためのメソッドなので、
//	いずれかのオブジェクトがヌル値の場合には、falseを返す。
//
//	ARGUMENTS
//	const Common::DataType::Type	FieldType_
//		フィールドのデータ型
//	const void*						SrcField_
//		比較元可変長フィールドの値が記録されている領域先頭へのポインタ
//	const Os::Memory::Size			SrcFieldLen_
//		比較元可変長フィールド長 [byte]
//	const void*						DstField_
//		比較先可変長フィールドの値が記録されている領域先頭へのポインタ
//	const Os::Memory::Size			DstFieldLen_
//		比較元可変長フィールド長 [byte]
//
//	RETURN
//	bool
//		比較結果
//			true  : 2つの可変長フィールドの値が等しい
//			false : 2つの可変長フィールドの値が異なる
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareVariableFieldForVerify(
	const Common::DataType::Type	FieldType_,
	const void*						SrcField_,
	const Os::Memory::Size			SrcFieldLen_,
	const void*						DstField_,
	const Os::Memory::Size			DstFieldLen_) const
{
	if (FieldType_ == Common::DataType::String)
	{
		Common::StringData	srcField;
		this->readStringField(SrcField_, SrcFieldLen_, srcField);

		Common::StringData	dstField;
		this->readStringField(DstField_, DstFieldLen_, dstField);

		return srcField.compareTo(&dstField) == 0;
	}
	else if (FieldType_ == Common::DataType::Binary)
	{
		Common::BinaryData	srcField(SrcField_, SrcFieldLen_);

		Common::BinaryData	dstField(DstField_, DstFieldLen_);

		return srcField.compareTo(&dstField) == 0;
	}
	else
	{
		throw Exception::NotSupported(moduleName, srcFile, __LINE__);
	}
}

//
//	FUNCTION private
//	Btree::File::readStringField -- 文字列フィールドの値を読み込む
//
//	NOTES
//	文字列フィールドの値を読み込む。
//
//	ARGUMENTS
//	const void*				Field_
//		文字列フィールドの値が記録されている領域先頭へのポインタ
//	const Os::Memroy::Size	FieldLen_
//		文字列フィールド長 [byte]
//	Common::StringData&		StringField_
//		読み込み先の文字列フィールドオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::readStringField(
	const void*				Field_,
	const Os::Memory::Size	FieldLen_,
	Common::StringData&		StringField_) const
{
	ModSize	numChar = FieldLen_ / sizeof(ModUnicodeChar);

	ModUnicodeChar	forZeroByte = 0;

	const ModUnicodeChar*	field = 0;

	if (numChar == 0)
	{
		field = &forZeroByte;
	}
	else
	{
		field = static_cast<const ModUnicodeChar*>(Field_);
	}

	ModUnicodeString	stringField(field, numChar);

	StringField_.setValue(stringField);
}

//
//	FUNCTION private
//	Btree::File::compareOutsideVariableFieldForVerify --
//		外置き可変長フィールドの値を比較する
//
//	NOTES
//	外置き可変長フィールドの値を比較する。
//	オブジェクトのユニーク性検査のためのメソッドなので、
//	いずれかのオブジェクトがヌル値の場合には、falseを返す。
//
//	ARGUMENTS
//	const int					FieldIndex_
//		比較対象可変長フィールドのインデックス
//	const ModUInt64				SrcFieldObjectID_
//		比較元可変長フィールドオブジェクトのID
//	PhysicalFile::Page*			SrcDirectObjectPage_
//		比較元代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理ページの記述子
//	const ModUInt64				DstFieldObjectID_
//		比較先可変長フィールドオブジェクトのID
//	PhysicalFile::Page*			DstDirectObjectPage_
//		比較先代表キーオブジェクト／バリューオブジェクトが記録されている
//		物理ページの記述子
//	Btree::PageVector&			AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページの記述子をつむ）
//
//	RETURN
//	bool
//		比較結果
//			true  : 2つの可変長フィールドの値が等しい
//			false : 2つの可変長フィールドの値が異なる
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareOutsideVariableFieldForVerify(
	const int					FieldIndex_,
	const ModUInt64				SrcFieldObjectID_,
	PhysicalFile::Page*			SrcDirectObjectPage_,
	const ModUInt64				DstFieldObjectID_,
	PhysicalFile::Page*			DstDirectObjectPage_,
	PageVector&					AttachPages_) const
{
	PhysicalFile::Page*		srcFieldObjectPage = 0;

	bool	srcAttached =
		File::attachObjectPage(this->m_pTransaction,
							   SrcFieldObjectID_,
							   SrcDirectObjectPage_,
							   srcFieldObjectPage,
							   this->m_FixMode,
							   this->m_CatchMemoryExhaust,
							   AttachPages_);

	const PhysicalFile::AreaID srcFieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(SrcFieldObjectID_);

	const void*	srcFieldObjectAreaTop =
		File::getConstAreaTop(srcFieldObjectPage,
							  srcFieldObjectAreaID);

	PhysicalFile::Page*		dstFieldObjectPage = 0;

	bool	dstAttached =
		File::attachObjectPage(this->m_pTransaction,
							   DstFieldObjectID_,
							   DstDirectObjectPage_,
							   dstFieldObjectPage,
							   this->m_FixMode,
							   this->m_CatchMemoryExhaust,
							   AttachPages_);

	const PhysicalFile::AreaID dstFieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(DstFieldObjectID_);

	const void*	dstFieldObjectAreaTop =
		File::getConstAreaTop(dstFieldObjectPage,
							  dstFieldObjectAreaID);

	const File::ObjectType*	srcObjectType =
		static_cast<const File::ObjectType*>(srcFieldObjectAreaTop);

	const File::ObjectType*	dstObjectType =
		static_cast<const File::ObjectType*>(dstFieldObjectAreaTop);

	Common::DataType::Type	fieldType =
		*(this->m_cFileParameter.m_FieldTypeArray + FieldIndex_);

	// check object type

	if (*srcObjectType != File::NormalObjectType ||
		*dstObjectType != File::NormalObjectType)
	{
		Common::Data::Pointer	srcField;
		File::readOutsideVariableField(this->m_pTransaction,
									   SrcDirectObjectPage_,
									   SrcFieldObjectID_,
									   fieldType,
									   srcField,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachPages_);

		Common::Data::Pointer	dstField;
		File::readOutsideVariableField(this->m_pTransaction,
									   DstDirectObjectPage_,
									   DstFieldObjectID_,
									   fieldType,
									   dstField,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachPages_);

		return
			srcField.get()->compareTo(dstField.get()) == 0;
	}

	Os::Memory::Size	srcFieldLen =
		srcFieldObjectPage->getAreaSize(srcFieldObjectAreaID) -
		File::ObjectTypeArchiveSize;

	Os::Memory::Size	dstFieldLen =
		dstFieldObjectPage->getAreaSize(dstFieldObjectAreaID) -
		File::ObjectTypeArchiveSize;

	return this->compareVariableFieldForVerify(fieldType,
											   srcObjectType + 1,
											   srcFieldLen,
											   dstObjectType + 1,
											   dstFieldLen);
}

//
//	Copyright (c) 2001, 2002, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
