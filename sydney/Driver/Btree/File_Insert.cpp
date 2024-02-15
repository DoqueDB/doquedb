// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_Insert.cpp -- Ｂ＋木ファイルクラスの実現ファイル（挿入関連）
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
#include "Btree/FileInformation.h"
#include "Btree/KeyInformation.h"
#include "Btree/NodePageHeader.h"
#include "Btree/ValueFile.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/BadArgument.h"
#include "Exception/FileNotOpen.h"
#include "Exception/IllegalFileAccess.h"
#include "Exception/UniquenessViolation.h"
#include "Exception/NotSupported.h"
#include "LogicalFile/ObjectID.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::File::insertCheck -- オブジェクト挿入前のチェックを行う
//
//	NOTES
//	オブジェクトを挿入するために
//	ファイルや挿入オブジェクトに不正がないかどうかを
//	チェックする。
//
//	ARGUMENTS
//	Common::DataArrayData*	Object_
//		挿入オブジェクトへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::insertCheck(Common::DataArrayData*	Object_) const
{
	//
	// ファイルのチェック
	// （挿入のための準備ができているか？）
	//

	if (this->m_pPhysicalFile == 0 ||
		this->m_ValueFile == 0 ||
		this->m_pOpenParameter->m_bEstimate)
	{
		// ファイルがオープンされていない、
		// もしくは、見積もりのためにオープンされている…

		SydErrorMessage << "file not open." << ModEndl;

		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	if (this->m_pOpenParameter->m_iOpenMode != FileCommon::OpenMode::Update)
	{
		// 更新モードでオープンされていない…

		SydErrorMessage << "illegal open mode." << ModEndl;

		throw Exception::IllegalFileAccess(moduleName, srcFile, __LINE__);
	}

	//
	// オブジェクトのチェック
	// （利用者から適切なオブジェクトが渡されたか？）
	//

	if (Object_ == 0)
	{
		// 挿入するオブジェクトが設定されていない…

		SydErrorMessage << "object not found." << ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	//
	// フィールドのチェック
	// （各フィールドが適切か？）
	//

	//
	// フィールドのチェック① − まずはデータ型のチェック
	//

	Common::Data*	objectIDField = Object_->getElement(0).get();

	// ObjectIDの位置にはNullDataをセットするようにした

	if (objectIDField->getType() != LogicalFile::ObjectID().getType()
		&& !objectIDField->isNull())
	{
		// オブジェクトIDフィールドがきちんと用意されていない…

		SydErrorMessage << "illegal object ID field type." << ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	int	i;

	for (i = 1; i < this->m_cFileParameter.m_FieldNum; i++)
	{
		Common::Data*	field = Object_->getElement(i).get();

		Common::DataType::Type	fieldType = field->getType();

		if (fieldType != *(this->m_cFileParameter.m_FieldTypeArray + i) &&
			!field->isNull())
		{
			// フィールドが適切ではない…

			SydErrorMessage << "illegal field type." << ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}
	}

	//
	// フィールドのチェック② − 次に可変長フィールドのチェック
	//

	if (this->m_cFileParameter.m_ExistVariableFieldInKey ||
		this->m_cFileParameter.m_ExistVariableFieldInValue)
	{
		int	startIndex, stopIndex;

		if (this->m_cFileParameter.m_ExistVariableFieldInKey &&
			this->m_cFileParameter.m_ExistVariableFieldInValue)
		{
			startIndex = 1;
			stopIndex = this->m_cFileParameter.m_FieldNum - 1;
		}
		else if (this->m_cFileParameter.m_ExistVariableFieldInKey)
		{
			startIndex = 1;
			stopIndex = this->m_cFileParameter.m_TopValueFieldIndex - 1;
		}
		else
		{
			startIndex = this->m_cFileParameter.m_TopValueFieldIndex;
			stopIndex = this->m_cFileParameter.m_FieldNum - 1;
		}

		for (i = startIndex; i <= stopIndex; i++)
		{
			if (*(this->m_cFileParameter.m_IsFixedFieldArray + i) == false &&
				*(this->m_cFileParameter.m_IsArrayFieldArray + i) == false &&
				*(this->m_cFileParameter.m_FieldMaxLengthArray + i) !=
				File::UnlimitedFieldLen)
			{
				Common::Data*	variableField = Object_->getElement(i).get();

				if (variableField->isNull())
				{
					continue;
				}

				const void*			value;
				Os::Memory::Size	valueLen;
				FileCommon::DataManager::getCommonDataBuffer(*variableField,
															 value,
															 valueLen);

				if (valueLen >
					*(this->m_cFileParameter.m_FieldMaxLengthArray + i))
				{
					// フィールド長超過…

					SydErrorMessage
						<< "variable field length overflow."
						<< ModEndl;

					throw Exception::BadArgument(moduleName,
												 srcFile,
												 __LINE__);
				}
			}
		}
	}

	//
	// フィールドのチェック③ − 次に配列フィールドのチェック
	//

	if (this->m_cFileParameter.m_ExistArrayFieldInValue)
	{
		//
		// ※ 配列フィールドをキーフィールドにはできない。
		//

		for (i = this->m_cFileParameter.m_TopValueFieldIndex;
			 i < this->m_cFileParameter.m_FieldNum;
			 i++)
		{
			if (*(this->m_cFileParameter.m_IsArrayFieldArray + i))
			{
				Common::Data*	field = Object_->getElement(i).get();

				Common::DataType::Type	fieldType = field->getType();

				if (field->isNull())
				{
					continue;
				}

				if (fieldType != Common::DataType::Array)
				{
					// （Common::IntegerArrayDataなどの）
					// Common::DataArrayData以外の
					// 配列データがフィールド値として
					// 設定されている…！？

					SydErrorMessage
						<< "illegal array field type." << ModEndl;

					throw Exception::BadArgument(moduleName,
												 srcFile,
												 __LINE__);
				}

				Common::DataArrayData*	arrayField =
					_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, field);

				; _SYDNEY_ASSERT(arrayField != 0);

				int	elementNum = arrayField->getCount();

				if (*(this->m_cFileParameter.m_ElementMaxNumArray + i) !=
					static_cast<int>(File::UnlimitedFieldLen))
				{
					if (elementNum >
						*(this->m_cFileParameter.m_ElementMaxNumArray + i))
					{
						// 要素数超過…

						SydErrorMessage
							<< "array field number of elements overflow."
							<< ModEndl;

						throw Exception::BadArgument(moduleName,
													 srcFile,
													 __LINE__);
					}
				}

				for (int j = 0; j < elementNum; j++)
				{
					Common::Data*	element =
						arrayField->getElement(j).get();

					Common::DataType::Type	elementType = element->getType();

					if (elementType !=
						*(this->m_cFileParameter.m_ElementTypeArray + i) &&
						!element->isNull())
					{
						// 要素型が不一致…

						SydErrorMessage
							<< "illegal element type."
							<< ModEndl;

						throw Exception::BadArgument(moduleName,
													 srcFile,
													 __LINE__);
					}

					if (!element->isNull() &&
						*(this->m_cFileParameter.m_IsFixedElementArray + i)
						== false &&
						*(this->m_cFileParameter.m_ElementMaxLengthArray + i)
						!= File::UnlimitedFieldLen)
					{
						const void*			elementValue;
						Os::Memory::Size	elementValueLen;
						FileCommon::DataManager::getCommonDataBuffer(
							*element,
							elementValue,
							elementValueLen);

						if (elementValueLen >
							*(this->m_cFileParameter.m_ElementMaxLengthArray
							  + i))
						{
							// 要素長超過…

							SydErrorMessage
								<< "variable element length overflow."
								<< ModEndl;

							throw Exception::BadArgument(moduleName,
														 srcFile,
														 __LINE__);
						}
					}

				} // end for j

			} // end if

		} // end for i
	}
}

//
//	FUNCTION private
//	Btree::File::insertKey --
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
//	UniquenessViolation
//		ファイル内に記録されているオブジェクトのユニーク性が失われてしまう
//	[YET!]
//
void
File::insertKey(FileInformation&		FileInfo_,
				ValueFile*				ValueFile_,
				Common::DataArrayData*	Object_,
				PageVector&				AttachNodePages_,
				PageIDVector&			AllocateNodePageIDs_,
				PageVector&				AttachValuePages_,
				const bool				DoUniqueCheck_ // = false
				) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		// キー値をキー情報内に記録する…

		this->insertSimpleKey(FileInfo_,
							  ValueFile_,
							  Object_,
							  AttachNodePages_,
							  AllocateNodePageIDs_,
							  AttachValuePages_,
							  DoUniqueCheck_);
	}
	else
	{
		// キー値をキーオブジェクトに記録する…
		// （キー値をキー情報の外に記録する…）

		//
		// ファイル管理情報から、
		// 「現在の木の深さ」と「ルートノードページの物理ページ識別子」を
		// 読み込む。
		//

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		// キーフィールドを挿入するリーフページを検索する
		PhysicalFile::Page*	leafPage =
			this->searchLeafPageForInsert(treeDepth,
										  rootNodePageID,
										  Object_,
										  AttachNodePages_);

		// リーフページが見つからないはずがない
		; _SYDNEY_ASSERT(leafPage != 0);

		if (DoUniqueCheck_)
		{
			// ユニークチェックを行う…

			this->uniqueCheck(FileInfo_,
							  ValueFile_,
							  Object_,
							  leafPage->getID(),
							  AttachNodePages_,
							  AttachValuePages_);
		}

		// リーフページにキーフィールドを挿入する
		this->insertKeyObject(
			FileInfo_,
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
}

//
//	FUNCTION private
//	Btree::File::uniqueCheck -- オブジェクト挿入前にユニークチェックを行う
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
File::uniqueCheck(FileInformation&				FileInfo_,
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
							true); // リーフページ

	if (useKeyInfoNum > 0)
	{
		bool	match = false;

		int	keyInfoIndex =
			this->getKeyInformationIndexForInsert(
				leafPage,
				useKeyInfoNum,
				AttachNodePages_,
				Object_,
				true,  // リーフページ
				true); // 子ノードを検索するわけではないが、
					   // File::getKeyInformationIndexForInsert内の
					   // アサートで引っ掛かってしまうので、trueにしてある。
					   // ここで、trueを設定してもfalseを設定しても
					   // 影響ないので。

		keyInfo.setStartOffsetByIndex(keyInfoIndex);

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		if (this->compareToFetchCondition(leafPage,
										  AttachNodePages_,
										  keyObjectID,
										  Object_,
										  1) // for object ID field
			== 0)
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

				if (this->compareToFetchCondition(
					valuePage,
					AttachValuePages_,
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
							        // してしまい、
							        // 物理ファイルマネージャが
							        // キャッシュしないようにする。
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

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		if (this->compareToFetchCondition(leafPage,
										  AttachNodePages_,
										  keyObjectID,
										  Object_,
										  1) // for object ID field
			!= 0)

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
						        // してしまい、
						        // 物理ファイルマネージャが
						        // キャッシュしないようにする。
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

		if (this->compareToFetchCondition(
			valuePage,
			AttachValuePages_,
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
					        // 物理ファイルマネージャが
					        // キャッシュしないようにする。
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

			valuePage = 0;
		}
	}

	checkMemoryExhaust(leafPage);
}

//
//	FUNCTION private
//	Btree::File::insertKeyObject --
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
//		（ルートノードページが1）
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
File::insertKeyObject(
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

	// ノードページに分割が必要な場合だけ、分割する
	bool	split = this->splitNodePage(FileInfo_,
										TopNodePage_,
										AttachNodePages_,
										AllocateNodePageIDs_,
										newNodePage,
										IsLeafPage_,
										ValueFile_,
										AttachValuePages_,
										Object_);

	PhysicalFile::Page*	topNodePage = 0;

	// キーを挿入する topNodePage を決定する。
	if (split)
	{
		// キーフィールドを挿入するノードページが分割された…

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
									IsLeafPage_);

			ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

			; _SYDNEY_ASSERT(
				keyObjectID != FileCommon::ObjectID::Undefined &&
				keyObjectID != 0);

			int	compareResult =
				this->compareToFetchCondition(TopNodePage_,
											  AttachNodePages_,
											  keyObjectID,
											  Object_,
											  1); // Object_には、
												  // オブジェクトIDも
												  // ついているので。

			if (compareResult < 0)
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
	// キーを挿入する topNodePage が決定された

	KeyInfoNodePageID_ = topNodePage->getID();

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   topNodePage,
								   IsLeafPage_);

	ModUInt32	useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	KeyInfoIndex_ =
		this->getKeyInformationIndexForInsert(
			topNodePage,
			useKeyInfoNum,
			AttachNodePages_,
			Object_,
			IsLeafPage_,
			false, // 子ノードを探すためではない
			PrevKeyInfoChildNodePage_);

	this->shiftKeyInfoForInsert(topNodePage,
								useKeyInfoNum,
								KeyInfoIndex_,
								IsLeafPage_,
								ValueFile_,
								AttachValuePages_);

	ModUInt64	keyObjectID = writeKeyObject(topNodePage,
											 Object_,
											 AttachNodePages_,
											 AllocateNodePageIDs_,
											 IsLeafPage_);

	KeyInformation	keyInfo(this->m_pTransaction,
							topNodePage,
							KeyInfoIndex_,
							IsLeafPage_);

	keyInfo.writeKeyObjectID(keyObjectID);

	nodePageHeader.incUseKeyInformationNumber();

	if (split)
	{
		this->resetParentNodePage(FileInfo_,
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
			this->resetParentNodeKey(topNodePage,
									 Object_,
									 NodeDepth_,
									 AttachNodePages_,
									 AllocateNodePageIDs_,
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
//	Btree::File::resetParentNodeKey --
//		親ノードページのキーオブジェクトを更新する
//
//	NOTES
//	親ノードページに記録されている、子ノードページの代表キーオブジェクトを
//	更新する。
//
//	ARGUMENTS
//	PhysicalFile::Page*		ChildNodePage_
//		子ノードページの物理ページ記述子
//	Common::DataArrayData*	Object_
//		更新後のオブジェクトへのポインタ
//	const ModUInt32			ParentNodeDepth_
//		親ノードページの階層（ルートノードページが1）
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
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
File::resetParentNodeKey(PhysicalFile::Page*	ChildNodePage_,
						 Common::DataArrayData*	Object_,
						 const ModUInt32		ParentNodeDepth_,
						 PageVector&			AttachNodePages_,
						 PageIDVector&			AllocateNodePageIDs_,
						 const bool				IsLeafPage_) const
{
	if (ParentNodeDepth_ == 1)
	{
		return;
	}

	PhysicalFile::Page*	parentNodePage = 0;
	ModUInt32			keyInfoIndex = ModUInt32Max;

	this->searchKeyInformation(ChildNodePage_,
							   parentNodePage,
							   keyInfoIndex,
							   AttachNodePages_,
							   IsLeafPage_);

	; _SYDNEY_ASSERT(parentNodePage != 0);
	; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false); // リーフページではない

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	this->rewriteKeyObject(keyObjectID,
						   Object_,
						   AttachNodePages_,
						   AllocateNodePageIDs_,
						   false); // リーフページではない

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   parentNodePage,
								   false); // リーフページではない

	if (keyInfoIndex == nodePageHeader.readUseKeyInformationNumber() - 1)
	{
		this->resetParentNodeKey(parentNodePage,
								 Object_,
								 ParentNodeDepth_ - 1,
								 AttachNodePages_,
								 AllocateNodePageIDs_,
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
//	Btree::File::rewriteKeyObject -- キーオブジェクトを更新する
//
//	NOTES
//	キーオブジェクトを更新する。
//
//	ARGUMENTS
//	const ModUInt64			KeyObjectID_
//		更新するキーオブジェクトのオブジェクトID
//	Common::DataArrayData*	Object_
//		更新後のオブジェクトへのポインタ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理p−ゑ時識別子をつむ）
//	const bool				IsLeafPage_
//		更新するキーオブジェクトが記録されているノードページが
//		リーフページかどうか
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
File::rewriteKeyObject(const ModUInt64			KeyObjectID_,
					   Common::DataArrayData*	Object_,
					   PageVector&				AttachNodePages_,
					   PageIDVector&			AllocateNodePageIDs_,
					   const bool				IsLeafPage_) const
{
	PhysicalFile::Page*	objectPage = File::attachPage(
		m_pTransaction, m_pPhysicalFile,
		Common::ObjectIDData::getFormerValue(KeyObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

	void* objectAreaTop = File::getAreaTop(
		objectPage, Common::ObjectIDData::getLatterValue(KeyObjectID_));

	this->rewriteKeyObject(objectAreaTop,
						   Object_,
						   objectPage,
						   AttachNodePages_,
						   AllocateNodePageIDs_,
						   IsLeafPage_);

	if (this->m_CatchMemoryExhaust)
	{
		this->m_pPhysicalFile->detachPage(
			objectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}
}

//
//	FUNCTION private
//	Btree::File::rewriteKeyObject -- キーオブジェクトを更新する
//
//	NOTES
//	キーオブジェクトを更新する。
//
//	ARGUMENTS
//	void*					KeyObjectAreaTop_
//		キーオブジェクトが記録されている物理エリア先頭へのポインタ
//	Common::DataArrayData*	Object_
//		更新後のオブジェクトへのポインタ
//	PhysicalFile::Page*		KeyObjectPage_
//		キーオブジェクトが記録されている物理ページの記述子
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
//		更新するキーオブジェクトが記録されているノードページが
//		リーフページかどうか
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
File::rewriteKeyObject(void*					KeyObjectAreaTop_,
					   Common::DataArrayData*	Object_,
					   PhysicalFile::Page*		KeyObjectPage_,
					   PageVector&				AttachNodePages_,
					   PageIDVector&			AllocateNodePageIDs_,
					   const bool				IsLeafPage_) const
{
	File::ObjectType*	objectType =
		static_cast<File::ObjectType*>(KeyObjectAreaTop_);

	; _SYDNEY_ASSERT((*objectType & File::NormalObjectType) != 0);

	NullBitmap::Value*	nullBitmapTop =
		syd_reinterpret_cast<NullBitmap::Value*>(objectType + 1);

	NullBitmap	nullBitmap(nullBitmapTop,
						   this->m_cFileParameter.m_KeyNum,
						   NullBitmap::Access::ReadWrite);

	bool	existNull = nullBitmap.existNull();

	char*	keyWritePos = static_cast<char*>(nullBitmap.getTail());

	for (int i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
	{
		Common::Data*	key = Object_->getElement(i).get();

		Common::DataType::Type	keyDataType = key->getType();

		if (key->isNull())
		{
			if (*(this->m_cFileParameter.m_FieldOutsideArray + i) &&
				(existNull == false || nullBitmap.isNull(i - 1) == false))
			{
				ModUInt64	objectID;
				File::readObjectID(keyWritePos, objectID);

				; _SYDNEY_ASSERT(
					objectID != FileCommon::ObjectID::Undefined);

				this->freeOutsideVariableKeyArea(objectID, AttachNodePages_);
			}

			keyWritePos = this->writeNullKey(nullBitmapTop,
											 keyWritePos,
											 i);
		}
		else if (*(this->m_cFileParameter.m_IsFixedFieldArray + i))
		{
			// 固定長フィールド…

			nullBitmap.off(i - 1);

			keyWritePos = File::writeFixedField(keyWritePos, key);
		}
		else
		{
			// 可変長フィールド…

			nullBitmap.off(i - 1);

			// キーフィールドにできる可変長フィールドは、
			// 文字列フィールドだけである。
			; _SYDNEY_ASSERT(
				*(this->m_cFileParameter.m_FieldTypeArray + i) ==
				Common::DataType::String);

			if (*(this->m_cFileParameter.m_FieldOutsideArray + i))
			{
				// 外置き文字列フィールド…

				if (existNull == false || nullBitmap.isNull(i - 1) == false)
				{
					ModUInt64	objectID;
					File::readObjectID(keyWritePos, objectID);

					; _SYDNEY_ASSERT(
						objectID != FileCommon::ObjectID::Undefined);

					this->freeOutsideVariableKeyArea(objectID,
													 AttachNodePages_);
				}

				ModUInt64	newObjectID =
					this->writeOutsideVariableKey(KeyObjectPage_,
												  key,
												  AttachNodePages_,
												  AllocateNodePageIDs_,
												  IsLeafPage_);

				keyWritePos = File::writeObjectID(keyWritePos, newObjectID);
			}
			else
			{
				// 外置きではない文字列フィールド…

				Os::Memory::Size	maxLen =
					*(this->m_cFileParameter.m_FieldMaxLengthArray + i);

				keyWritePos = File::writeInsideVariableField(keyWritePos,
													   key,
													   maxLen);
			}
		}
	
	} // end for i
}

//
//	FUNCTION private
//	Btree::File::resetParentNodePage -- 親ノードページを更新する
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
File::resetParentNodePage(FileInformation&		FileInfo_,
						  ValueFile*			ValueFile_,
						  const ModUInt32		ChildNodeDepth_,
						  PhysicalFile::Page*	ChildNodePage1_,
						  PhysicalFile::Page*	ChildNodePage2_,
						  PageVector&			AttachNodePages_,
						  PageIDVector&			AllocateNodePageIDs_,
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
			ModUInt64	childNodeLastKeyObjectID =
				this->getLastObjectIDInNode(childNodePages[i],
											ChildNodeIsLeafPage_,
											false); // キーオブジェクトを取得

			PhysicalFile::PageID	keyInfoPageID =
				PhysicalFile::ConstValue::UndefinedPageID;
			ModUInt32				keyInfoIndex = 0;

			this->insertKeyObject(FileInfo_,
								  ValueFile_,
								  ChildNodeDepth_,
								  newRootNodePage,
								  childNodeLastKeyObjectID,
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
									false); // リーフページではない

			keyInfo.writeChildNodePageID((childNodePages[i])->getID());

			if (this->m_CatchMemoryExhaust)
			{
				this->m_pPhysicalFile->detachPage(
					keyInfoPage,
					PhysicalFile::Page::UnfixMode::Dirty,
					false); // 本当にデタッチ（アンフィックス）してしまい、
			                // 物理ファイルマネージャが
					        // キャッシュしないようにする。
			}

			NodePageHeader	childNodePageHeader(this->m_pTransaction,
												childNodePages[i],
												ChildNodeIsLeafPage_);

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
		this->rewriteParentNodeKey(ChildNodePage1_,
								   ChildNodeIsLeafPage_,
								   0,
								   AttachNodePages_,
								   AllocateNodePageIDs_);

		NodePageHeader	childNodePageHeader(this->m_pTransaction,
											ChildNodePage2_,
											ChildNodeIsLeafPage_);

		ModUInt32	useKeyInfoNum =
			childNodePageHeader.readUseKeyInformationNumber();

		; _SYDNEY_ASSERT(useKeyInfoNum > 0);

		KeyInformation	childNodeKeyInfo(this->m_pTransaction,
										 ChildNodePage2_,
										 useKeyInfoNum - 1,
										 ChildNodeIsLeafPage_);

		ModUInt64	keyObjectID = childNodeKeyInfo.readKeyObjectID();

		PhysicalFile::Page*	parentNodePage = 0;
		ModUInt32			keyInfoIndex = ModUInt32Max;

		this->searchKeyInformation(ChildNodePage1_,
								   parentNodePage,
								   keyInfoIndex,
								   AttachNodePages_,
								   ChildNodeIsLeafPage_);

		; _SYDNEY_ASSERT(parentNodePage != 0);

		PhysicalFile::PageID	parentNodePageID =
			PhysicalFile::ConstValue::UndefinedPageID;
		this->insertKeyObject(FileInfo_,
							  ValueFile_,
							  ChildNodeDepth_ - 1,
							  parentNodePage,
							  keyObjectID,
							  AttachNodePages_,
							  AllocateNodePageIDs_,
							  AttachValuePages_,
							  parentNodePageID,
							  keyInfoIndex,
							  false, // リーフページではない
							  ChildNodePage1_);

		; _SYDNEY_ASSERT(parentNodePageID !=
						 PhysicalFile::ConstValue::UndefinedPageID);
		; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

		if (this->m_CatchMemoryExhaust)
		{
			this->m_pPhysicalFile->detachPage(
				parentNodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
			            // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}

		parentNodePage = File::attachPage(this->m_pTransaction,
										  this->m_pPhysicalFile,
										  parentNodePageID,
										  this->m_FixMode,
										  this->m_CatchMemoryExhaust,
										  AttachNodePages_);

		KeyInformation	parentNodeKeyInfo(this->m_pTransaction,
										  parentNodePage,
										  keyInfoIndex,
										  false); // リーフページではない

		parentNodeKeyInfo.writeChildNodePageID(ChildNodePage2_->getID());

		if (this->m_CatchMemoryExhaust)
		{
			this->m_pPhysicalFile->detachPage(
				parentNodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}

		childNodePageHeader.writeParentNodePageID(parentNodePageID);
	}
}

//
//	FUNCTION private
//	Btree::File::rewriteParentNodeKey --
//		親ノードページのキーオブジェクトを更新する
//
//	NOTES
//	親ノードページのキーオブジェクトを更新する。
//
//	ARGUMENTS
//	PhysicalFile::Page*		ChildNodePage_
//		子ノードページの物理ページ記述子
//	const bool				ChildNodeIsLeafPage_
//		子ノードページがリーフページかどうか
//			true  : 子ノードページがリーフページ
//			false : 子ノードページがリーフページ以外のノードページ
//	const ModUInt32			ParentNodeDepth_
//		親ノードページの階層（ルートノードページが1）
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_,
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::rewriteParentNodeKey(PhysicalFile::Page*	ChildNodePage_,
						   const bool			ChildNodeIsLeafPage_,
						   const ModUInt32		ParentNodeDepth_,
						   PageVector&			AttachNodePages_,
						   PageIDVector&		AllocateNodePageIDs_) const
{
	ModUInt64	childKeyObjectID =
		this->getLastObjectIDInNode(ChildNodePage_,
									ChildNodeIsLeafPage_,
									false); // キーオブジェクトの
									        // オブジェクトIDを取得

	PhysicalFile::Page*	parentNodePage = 0;
	ModUInt32			keyInfoIndex = ModUInt32Max;

	this->searchKeyInformation(ChildNodePage_,
							   parentNodePage,
							   keyInfoIndex,
							   AttachNodePages_,
							   ChildNodeIsLeafPage_);

	; _SYDNEY_ASSERT(parentNodePage != 0);

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false); // リーフページではない

	ModUInt64	parentKeyObjectID = keyInfo.readKeyObjectID();

	this->rewriteKeyObject(childKeyObjectID,
						   parentKeyObjectID,
						   AttachNodePages_,
						   AllocateNodePageIDs_,
						   false); // リーフページではない

	if (ParentNodeDepth_ > 1)
	{
		NodePageHeader	nodePageHeader(this->m_pTransaction,
									   parentNodePage,
									   false); // リーフページではない

		if (keyInfoIndex == nodePageHeader.readUseKeyInformationNumber() - 1)
		{
			this->rewriteParentNodeKey(parentNodePage,
									   false, // リーフページではない
									   ParentNodeDepth_ - 1,
									   AttachNodePages_,
									   AllocateNodePageIDs_);
		}
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
//	Btree::File::rewriteKeyObject -- キーオブジェクトを更新する
//
//	NOTES
//	キーオブジェクトを更新する。
//
//	ARGUMENTS
//	const ModUInt64			SrcKeyObjectID_
//		更新後のキー値が記録されているキーオブジェクトのオブジェクトID
//	const ModUInt64			DstKeyObjectID_
//		更新対象となるキーオブジェクトのオブジェクトID
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				DstNodeIsLeafPage_
//		引数DstKeyObjectID_で示される更新対象のキーオブジェクトが
//		記録されているノードページがリーフページかどうか
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
File::rewriteKeyObject(const ModUInt64	SrcKeyObjectID_,
					   const ModUInt64	DstKeyObjectID_,
					   PageVector&		AttachNodePages_,
					   PageIDVector&	AllocateNodePageIDs_,
					   const bool		DstNodeIsLeafPage_) const
{
	PhysicalFile::Page*	srcKeyObjectPage = File::attachPage(
		m_pTransaction, m_pPhysicalFile,
		Common::ObjectIDData::getFormerValue(SrcKeyObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

	const void*	srcKeyObjectAreaTop = File::getConstAreaTop(
		srcKeyObjectPage,
		Common::ObjectIDData::getLatterValue(SrcKeyObjectID_));

	PhysicalFile::Page*	dstKeyObjectPage = File::attachPage(
		m_pTransaction, m_pPhysicalFile,
		Common::ObjectIDData::getFormerValue(DstKeyObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

	void* dstKeyObjectAreaTop = File::getAreaTop(
		dstKeyObjectPage,
		Common::ObjectIDData::getLatterValue(DstKeyObjectID_));

	this->rewriteKeyObject(srcKeyObjectAreaTop,
						   dstKeyObjectAreaTop,
						   dstKeyObjectPage,
						   AttachNodePages_,
						   AllocateNodePageIDs_,
						   DstNodeIsLeafPage_);

	if (this->m_CatchMemoryExhaust)
	{
		this->m_pPhysicalFile->detachPage(
			srcKeyObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。

		this->m_pPhysicalFile->detachPage(
			dstKeyObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}
}

//
//	FUNCTION private
//	Btree::File::rewriteKeyObject -- キーオブジェクトを更新する
//
//	NOTES
//	キーオブジェクトを更新する。
//
//	ARGUMENTS
//	const void*				SrcKeyObjectAreaTop_
//		更新後のキー値が記録されているキーオブジェクトが
//		記録されている物理エリア先頭へのポインタ
//	void*					DstKeyObjectAreaTop_
//		更新対象となるキーオブジェクトが記録されている
//		物理エリア先頭へのポインタ
//	PhysicalFile::Page*		DstKeyObjectPage_
//		更新対象となるキーオブジェクトが記録されているノードページの
//		物理ページ記述子
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				DstNodeIsLeafPage_
//		更新対象となるキーオブジェクトが記録されているノードページが
//		リーフページかどうか
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
File::rewriteKeyObject(const void*			SrcKeyObjectAreaTop_,
					   void*				DstKeyObjectAreaTop_,
					   PhysicalFile::Page*	DstKeyObjectPage_,
					   PageVector&			AttachNodePages_,
					   PageIDVector&		AllocateNodePageIDs_,
					   const bool			DstNodeIsLeafPage_) const
{
	const File::ObjectType*	srcObjectType =
		static_cast<const File::ObjectType*>(SrcKeyObjectAreaTop_);

	File::ObjectType*	dstObjectType =
		static_cast<File::ObjectType*>(DstKeyObjectAreaTop_);

	// オブジェクトタイプは同じはず。
	; _SYDNEY_ASSERT(*srcObjectType == *dstObjectType);

	const NullBitmap::Value*	srcNullBitmapTop =
		syd_reinterpret_cast<const NullBitmap::Value*>(srcObjectType + 1);

	NullBitmap	srcNullBitmap(srcNullBitmapTop,
							  this->m_cFileParameter.m_KeyNum,
							  NullBitmap::Access::ReadOnly);

	NullBitmap::Value*	dstNullBitmapTop =
		syd_reinterpret_cast<NullBitmap::Value*>(dstObjectType + 1);

	NullBitmap	dstNullBitmap(dstNullBitmapTop,
							  this->m_cFileParameter.m_KeyNum,
							  NullBitmap::Access::ReadWrite);

	const char*	srcKeyTop =
		static_cast<const char*>(srcNullBitmap.getConstTail());

	char*	dstKeyTop = static_cast<char*>(dstNullBitmap.getTail());

	for (int i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
	{
		if (*(this->m_cFileParameter.m_IsFixedFieldArray + i))
		{
			this->writeFixedField(i, srcKeyTop, dstKeyTop);
		}
		else
		{
			; _SYDNEY_ASSERT(
				*(this->m_cFileParameter.m_FieldTypeArray + i) ==
				Common::DataType::String);

			this->rewriteStringKey(i,
								   srcNullBitmap,
								   dstNullBitmap,
								   srcKeyTop,
								   dstKeyTop,
								   DstKeyObjectPage_,
								   AttachNodePages_,
								   AllocateNodePageIDs_,
								   DstNodeIsLeafPage_);
		}
	}

	NullBitmap::Size	nullBitmapSize = srcNullBitmap.getSize();

	ModOsDriver::Memory::copy(dstNullBitmapTop,
							  srcNullBitmapTop,
							  nullBitmapSize);
}

//
//	FUNCTION private
//	Btree::File::rewriteStringKey -- 文字列キーフィールドの値を更新する
//
//	NOTES
//	文字列キーフィールドの値を更新する。
//
//	ARGUMENTS
//	const int					KeyFieldIndex_
//		キーフィールドインデックス
//	const Btree::NullBitmap&	SrcNullBitmap_
//		更新する代表キーオブジェクトのヌルビットマップへの参照
//	Btree::NullBitmap&			DstNullBitmap_
//		更新後の代表キーオブジェクトのヌルビットマップへの参照
//	const char*&				SrcKeyTop_
//		更新する代表キーオブジェクト内のキーフィールド値が
//		記録されている領域へのポインタ
//	char*&						DstKeyTop_
//		更新後の代表キーオブジェクト内のキーフィールド値が
//		記録されている領域へのポインタ
//	PhysicalFile::Page*			DstKeyObjectPage_
//		更新後の代表キーオブジェクトが記録されている
//		ノードページの物理ページ記述子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&		AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool					DstNodeIsLeafPage_
//		更新後のキーオブジェクトが記録されているノードページが
//		リーフページかどうか
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
File::rewriteStringKey(const int			KeyFieldIndex_,
					   const NullBitmap&	SrcNullBitmap_,
					   NullBitmap&			DstNullBitmap_,
					   const char*&			SrcKeyTop_,
					   char*&				DstKeyTop_,
					   PhysicalFile::Page*	DstKeyObjectPage_,
					   PageVector&			AttachNodePages_,
					   PageIDVector&		AllocateNodePageIDs_,
					   const bool			DstNodeIsLeafPage_) const
{
	if (*(this->m_cFileParameter.m_FieldOutsideArray + KeyFieldIndex_))
	{
		// 外置き文字列キーフィールド…

		ModUInt64	srcObjectID;
		SrcKeyTop_ = File::readObjectID(SrcKeyTop_, srcObjectID);

		ModUInt64	dstObjectID;

		if (DstNullBitmap_.isNull(KeyFieldIndex_ - 1) == false)
		{
			File::readObjectID(DstKeyTop_, dstObjectID);

			this->freeOutsideVariableKeyArea(dstObjectID, AttachNodePages_);
		}

		if (SrcNullBitmap_.isNull(KeyFieldIndex_ - 1) == false)
		{
			DstNullBitmap_.off(KeyFieldIndex_ - 1);

			dstObjectID = this->copyVariableKeyField(DstKeyObjectPage_,
													 srcObjectID,
													 AttachNodePages_,
													 AllocateNodePageIDs_,
													 DstNodeIsLeafPage_,
													 false); // copy
		}
		else
		{
			DstNullBitmap_.on(KeyFieldIndex_ - 1);

			dstObjectID = FileCommon::ObjectID::Undefined;
		}

		DstKeyTop_ = File::writeObjectID(DstKeyTop_, dstObjectID);
	}
	else
	{
		// 外置きではない文字列キーフィールド…

		Os::Memory::Size	fieldLen =
			File::InsideVarFieldLenArchiveSize +
			*(this->m_cFileParameter.m_FieldMaxLengthArray + KeyFieldIndex_);

		ModOsDriver::Memory::copy(DstKeyTop_, SrcKeyTop_, fieldLen);

		SrcKeyTop_ += fieldLen;
		DstKeyTop_ += fieldLen;
	}
}

//
//	FUNCTION private
//	Btree::File::insertKeyObject --
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
//		（ルートノードページが1）
//	PhysicalFile::Page*		TopNodePage_
//		ノードページの物理ページ記述子
//	const ModUInt64			ObjectID_
//		挿入するキーフィールドの値が記録されているキーオブジェクトのID
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
File::insertKeyObject(
	FileInformation&		FileInfo_,
	ValueFile*				ValueFile_,
	const ModUInt32			NodeDepth_,
	PhysicalFile::Page*		TopNodePage_,
	const ModUInt64			ObjectID_,
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

	bool	split = this->splitNodePage(FileInfo_,
										TopNodePage_,
										AttachNodePages_,
										AllocateNodePageIDs_,
										newNodePage,
										IsLeafPage_,
										ValueFile_,
										AttachValuePages_);

	PhysicalFile::Page*	topNodePage = 0;

	// キーを挿入する topNodePage を決定する。
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
									IsLeafPage_);

			ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

			; _SYDNEY_ASSERT(
				keyObjectID != FileCommon::ObjectID::Undefined &&
				keyObjectID != 0);

			int	compareResult = this->compareKeyObject(ObjectID_,
													   keyObjectID,
													   AttachNodePages_);

			if (compareResult < 0)
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
	// キーを挿入する topNodePage が決定された。

	KeyInfoNodePageID_ = topNodePage->getID();

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   topNodePage,
								   IsLeafPage_);

	ModUInt32	useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	KeyInfoIndex_ =
		this->getKeyInformationIndexForInsert(
			topNodePage,
			useKeyInfoNum,
			AttachNodePages_,
			ObjectID_,
			IsLeafPage_,
			false, // 子ノードを探すためではない
			PrevKeyInfoChildNodePage_);

	this->shiftKeyInfoForInsert(topNodePage,
								useKeyInfoNum,
								KeyInfoIndex_,
								IsLeafPage_,
								ValueFile_,
								AttachValuePages_);

	ModUInt64	keyObjectID = this->writeKeyObject(topNodePage,
												   ObjectID_,
												   AttachNodePages_,
												   AllocateNodePageIDs_,
												   IsLeafPage_);

	KeyInformation	keyInfo(this->m_pTransaction,
							topNodePage,
							KeyInfoIndex_,
							IsLeafPage_);

	keyInfo.writeKeyObjectID(keyObjectID);

	nodePageHeader.incUseKeyInformationNumber();

	if (split)
	{
		this->resetParentNodePage(FileInfo_,
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
			this->resetParentNodeKey(topNodePage,
									 ObjectID_,
									 NodeDepth_,
									 AttachNodePages_,
									 AllocateNodePageIDs_,
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
//	Btree::File::writeKeyObject --
//		ノードページにキーオブジェクトを書き込む
//
//	NOTES
//	異なる領域に記録されているキーオブジェクトを
//	ノードページに書き込む。
//
//	ARGUMENTS
//	PhysicalFile::Page*		TopNodePage_
//		ノードページの物理ページ記述子
//	const ModUInt64			KeyObjectID_
//		書き込むキーオブジェクトのオブジェクトID
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_,
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	ModUInt64
//		キーオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::writeKeyObject(PhysicalFile::Page*	TopNodePage_,
					 const ModUInt64		KeyObjectID_,
					 PageVector&			AttachNodePages_,
					 PageIDVector&			AllocateNodePageIDs_,
					 const bool				IsLeafPage_) const
{
	PhysicalFile::Page*	srcKeyObjectPage = File::attachPage(
		m_pTransaction, m_pPhysicalFile,
		Common::ObjectIDData::getFormerValue(KeyObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

	const void*	srcKeyObjectAreaTop = File::getConstAreaTop(
		srcKeyObjectPage,
		Common::ObjectIDData::getLatterValue(KeyObjectID_));

	Os::Memory::Size	directObjectSize =
		this->m_cFileParameter.m_DirectKeyObjectSize;

	PhysicalFile::Page*	dstKeyObjectPage =
		this->searchInsertNodePage(TopNodePage_,
								   AttachNodePages_,
								   AllocateNodePageIDs_,
								   directObjectSize,
								   IsLeafPage_);

	PhysicalFile::PageID	dstKeyObjectPageID =
		dstKeyObjectPage->getID();

	PhysicalFile::AreaID	dstKeyObjectAreaID =
		dstKeyObjectPage->allocateArea(*this->m_pTransaction,
									   directObjectSize);

	void*	dstKeyObjectAreaTop = File::getAreaTop(dstKeyObjectPage,
												   dstKeyObjectAreaID);

	this->writeKeyObject(srcKeyObjectAreaTop,
						 dstKeyObjectAreaTop,
						 dstKeyObjectPage,
						 AttachNodePages_,
						 AllocateNodePageIDs_,
						 IsLeafPage_);

	if (this->m_CatchMemoryExhaust)
	{
		this->m_pPhysicalFile->detachPage(
			srcKeyObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。

		if (dstKeyObjectPage != TopNodePage_)
		{
			this->m_pPhysicalFile->detachPage(
				dstKeyObjectPage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}
	}

	return File::makeObjectID(dstKeyObjectPageID, dstKeyObjectAreaID);
}

//
//	FUNCTION private
//	Btree::File::writeKeyObject --
//		ノードページにキーオブジェクトを書き込む
//
//	NOTES
//	異なる領域に記録されているキーオブジェクトを
//	ノードページに書き込む。
//
//	ARGUMENTS
//	const void*				SrcKeyObjectAreaTop_
//		書き込むキーフィールドの値が記録されているキーオブジェクトの
//		物理エリア先頭へのポインタ
//	void*					DstKeyObjectAreaTop_
//		書き込み先のキーオブジェクトの物理エリア先頭へのポインタ
//	PhysicalFile::Page*		DstKeyObjectPage_
//		書き込み先のキーオブジェクトの物理エリアが存在する
//		ノードページの物理ページ記述子
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				DstNodeIsLeafPage_
//		書き込み先のノードページがリーフページかどうか
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
File::writeKeyObject(const void*			SrcKeyObjectAreaTop_,
					 void*					DstKeyObjectAreaTop_,
					 PhysicalFile::Page*	DstKeyObjectPage_,
					 PageVector&			AttachNodePages_,
					 PageIDVector&			AllocateNodePageIDs_,
					 const bool				DstNodeIsLeafPage_) const
{
	const File::ObjectType*	srcObjectType =
		static_cast<const File::ObjectType*>(SrcKeyObjectAreaTop_);

	File::ObjectType*	dstObjectType =
		static_cast<File::ObjectType*>(DstKeyObjectAreaTop_);

	*dstObjectType = *srcObjectType;

	const NullBitmap::Value*	srcNullBitmapTop =
		syd_reinterpret_cast<const NullBitmap::Value*>(srcObjectType + 1);

	NullBitmap	srcNullBitmap(srcNullBitmapTop,
							  this->m_cFileParameter.m_KeyNum,
							  NullBitmap::Access::ReadOnly);

	bool	existNull = srcNullBitmap.existNull();

	NullBitmap::Value*	dstNullBitmapTop =
		syd_reinterpret_cast<NullBitmap::Value*>(dstObjectType + 1);

	NullBitmap	dstNullBitmap(dstNullBitmapTop,
							  this->m_cFileParameter.m_KeyNum,
							  NullBitmap::Access::ReadWrite);

	NullBitmap::Size	nullBitmapSize =
		NullBitmap::getSize(this->m_cFileParameter.m_KeyNum);

	ModOsDriver::Memory::copy(dstNullBitmapTop,
							  srcNullBitmapTop,
							  nullBitmapSize);

	const char*	srcKeyTop =
		static_cast<const char*>(srcNullBitmap.getConstTail());

	char*	dstKeyTop = static_cast<char*>(dstNullBitmap.getTail());

	for (int i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
	{
		if (existNull && srcNullBitmap.isNull(i - 1))
		{
			srcKeyTop += this->m_cFileParameter.getFieldArchiveSize(i);
			dstKeyTop += this->m_cFileParameter.getFieldArchiveSize(i);
		}
		else if (*(this->m_cFileParameter.m_IsFixedFieldArray + i))
		{
			this->writeFixedField(i, srcKeyTop, dstKeyTop);
		}
		else
		{
			; _SYDNEY_ASSERT(
				*(this->m_cFileParameter.m_FieldTypeArray + i) ==
				Common::DataType::String);

			this->writeStringKey(i,
								 srcKeyTop,
								 dstKeyTop,
								 DstKeyObjectPage_,
								 AttachNodePages_,
								 AllocateNodePageIDs_,
								 DstNodeIsLeafPage_);
		}
	}
}

//
//	FUNCTION private
//	Btree::File::writeFixedField -- 固定長フィールドの値を書き込む
//
//	NOTES
//	固定長フィールドの値を書き込む。
//
//	ARGUMENTS
//	const int		FieldIndex_
//		フィールドインデックス
//	const char*&	SrcFieldTop_
//		書き込む固定長フィールドが記録されている領域へのポインタ
//	char*&			DstFieldTop_
//		書き込み先の領域へのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::writeFixedField(const int		FieldIndex_,
					  const char*&	SrcFieldTop_,
					  char*&		DstFieldTop_) const
{
	Common::DataType::Type	fieldDataType =
		*(this->m_cFileParameter.m_FieldTypeArray + FieldIndex_);

	Os::Memory::Size	fieldSize =
		FileCommon::DataManager::getFixedCommonDataArchiveSize(
			fieldDataType);

	ModOsDriver::Memory::copy(DstFieldTop_, SrcFieldTop_, fieldSize);

	SrcFieldTop_ += fieldSize;
	DstFieldTop_ += fieldSize;
}

//
//	FUNCTION private
//	Btree::File::writeStringKey -- 文字列型のキーフィールドの値を書き込む
//
//	NOTES
//	異なる領域に記録されている文字列型のキーフィールドの値を
//	ノードページに書き込む。
//
//	ARGUMENTS
//	const int				FieldIndex_
//		フィールドインデックス
//	const char*&			SrcKeyTop_
//		書き込むキーフィールドの値が記録されている領域（物理エリア内）への
//		ポインタ
//	char*&					DstKeyTop_
//		書き込み先の領域（物理エリア内）へのポインタ
//	PhysicalFile::Page*		DstKeyObjectPage_
//		書き込み先のキーオブジェクトの物理エリアが存在する
//		ノードページの物理ページ記述子
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				DstNodeIsLeafPage_
//		書き込み先のノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::writeStringKey(const int				FieldIndex_,
					 const char*&			SrcKeyTop_,
					 char*&					DstKeyTop_,
					 PhysicalFile::Page*	DstKeyObjectPage_,
					 PageVector&			AttachNodePages_,
					 PageIDVector&			AllocateNodePageIDs_,
					 const bool				DstNodeIsLeafPage_) const
{
	if (*(this->m_cFileParameter.m_FieldOutsideArray + FieldIndex_))
	{
		// 外置き文字列キーフィールド…

		// SrcKeyTop_とDstKeyTop_は、外置きオブジェクトのIDを指している。

		ModUInt64	srcFieldObjectID;
		SrcKeyTop_ = File::readObjectID(SrcKeyTop_, srcFieldObjectID);

		; _SYDNEY_ASSERT(
			srcFieldObjectID != FileCommon::ObjectID::Undefined);

		ModUInt64	dstFieldObjectID =
			this->copyVariableKeyField(DstKeyObjectPage_,
									   srcFieldObjectID,
									   AttachNodePages_,
									   AllocateNodePageIDs_,
									   DstNodeIsLeafPage_,
									   false); // copy (not move)

		DstKeyTop_ = File::writeObjectID(DstKeyTop_, dstFieldObjectID);
	}
	else
	{
		// 外置きではない文字列フィールド…

		// SrcKeyTop_とDstKeyTop_は、フィールド長を指している。

		Os::Memory::Size	fieldLen =
			File::InsideVarFieldLenArchiveSize +
			*(this->m_cFileParameter.m_FieldMaxLengthArray + FieldIndex_);

		ModOsDriver::Memory::copy(DstKeyTop_, SrcKeyTop_, fieldLen);

		SrcKeyTop_ += fieldLen;
		DstKeyTop_ += fieldLen;
	}
}

//
//	FUNCTION private
//	Btree::File::resetParentNodeKey --
//		親ノードページのキーオブジェクトを更新する
//
//	NOTES
//	親ノードページに記録されている、子ノードページの代表キーオブジェクトを
//	更新する。
//
//	ARGUMENTS
//	PhysicalFile::Page*		ChildNodePage_
//		子ノードページの物理ページ記述子
//	const ModUInt64			ObjectID_
//		更新後のキーフィールドの値が記録されている
//		子ノードページのキーオブジェクトのID
//	const ModUInt32			ParentNodeDepth_
//		親ノードページの階層（ルートノードページが1）
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
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
File::resetParentNodeKey(PhysicalFile::Page*	ChildNodePage_,
						 const ModUInt64		ObjectID_,
						 const ModUInt32		ParentNodeDepth_,
						 PageVector&			AttachNodePages_,
						 PageIDVector&			AllocateNodePageIDs_,
						 const bool				IsLeafPage_) const
{
	if (ParentNodeDepth_ == 1)
	{
		return;
	}

	PhysicalFile::Page*	parentNodePage = 0;
	ModUInt32			keyInfoIndex = ModUInt32Max;

	this->searchKeyInformation(ChildNodePage_,
							   parentNodePage,
							   keyInfoIndex,
							   AttachNodePages_,
							   IsLeafPage_);

	; _SYDNEY_ASSERT(parentNodePage != 0);
	; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false); // リーフページではない

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	this->rewriteKeyObject(ObjectID_,
						   keyObjectID,
						   AttachNodePages_,
						   AllocateNodePageIDs_,
						   false); // リーフページではない

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   parentNodePage,
								   false); // リーフページではない

	if (keyInfoIndex == nodePageHeader.readUseKeyInformationNumber() - 1)
	{
		this->resetParentNodeKey(parentNodePage,
								 ObjectID_,
								 ParentNodeDepth_ - 1,
								 AttachNodePages_,
								 AllocateNodePageIDs_,
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
//	Btree::File::getKeyInformationIndexForInsert --
//		挿入するオブジェクトのキーフィールドの値に最も近い値が
//		記録されているキーオブジェクトへ辿るキー情報のインデックスを返す
//		
//	NOTES
//	引数KeyInfoPage_が示すノードページ内で、
//	挿入するオブジェクトのキーフィールドの値に最も近い
//	値が記録されているキーオブジェクトを検索し、
//	そのキーオブジェクトへ辿るキー情報のインデックスを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32		UseKeyInfoNum_
//		引数KeyInfoPage_が示すノードページ内の使用中のキー情報数
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const ModUInt64		ObjectID_
//		挿入するキーフィールドの値が記録されているキーオブジェクトのID
//	const bool			IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	const bool			SearchChildNodePage_
//		子ノードページを検索するために呼び出されたかどうか
//			true  : 子ノードページを検索するために呼び出された
//			false : 引数KeyInfoPage_が示すノードページの
//			        どこにキー情報を追加するかを特定するために
//			        呼び出された
//	PhysicalFile::Page*	PrevKeyInfoChildNodePage_ = 0
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
File::getKeyInformationIndexForInsert(
	PhysicalFile::Page*	KeyInfoPage_,
	const ModUInt32		UseKeyInfoNum_,
	PageVector&			AttachNodePages_,
	const ModUInt64		ObjectID_,
	const bool			IsLeafPage_,
	const bool			SearchChildNodePage_,
	PhysicalFile::Page*	PrevKeyInfoChildNodePage_ // = 0
	) const
{
	if (PrevKeyInfoChildNodePage_ != 0)
	{
		PhysicalFile::Page*	parentNodePage = 0;
		ModUInt32			prevKeyInfoIndex = ModUInt32Max;

		this->searchKeyInformation(PrevKeyInfoChildNodePage_,
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
							IsLeafPage_);

	int	midKeyInfoIndex = 0;
	int	firstKeyInfoIndex = 0;
	int	lastKeyInfoIndex = UseKeyInfoNum_ - 1;

	while (firstKeyInfoIndex <= lastKeyInfoIndex)
	{
		midKeyInfoIndex = (firstKeyInfoIndex + lastKeyInfoIndex) >> 1;

		keyInfo.setStartOffsetByIndex(midKeyInfoIndex);

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		int	compareResult = this->compareKeyObject(ObjectID_,
												   keyObjectID,
												   AttachNodePages_);

		if (compareResult < 0)
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
//	Btree::File::searchKeyInformation --
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
//	なし
//
void
File::searchKeyInformation(PhysicalFile::Page*	ChildNodePage_,
						   PhysicalFile::Page*&	ParentNodePage_,
						   ModUInt32&			KeyInfoIndex_,
						   PageVector&			AttachNodePages_,
						   const bool			ChildNodeIsLeafPage_) const
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
							false); // リーフページではない

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
//	FUNCTION private
//	Btree::File::compareKeyObject -- 2つのキーオブジェクトを比較する
//
//	NOTES
//	ファイル内に記録されている2つのキーオブジェクトを比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const ModUInt64		SrcKeyObjectID_
//		比較元キーオブジェクトのID
//	const ModUInt64		DstKeyObjectID_
//		比較先キーオブジェクトのID
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	int
//		比較結果
//			< 0 : 引数SrcKeyObjectID_が示す比較元キーオブジェクトの方が
//			      キー値順で前方
//			= 0 : 2つのキーオブジェクトに記録されているキー値が等しい
//			> 0 : 引数SrcKeyObjectID_が示す比較元キーオブジェクトの方が
//			      キー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareKeyObject(const ModUInt64	SrcKeyObjectID_,
					   const ModUInt64	DstKeyObjectID_,
					   PageVector&		AttachNodePages_) const
{
	// src

	PhysicalFile::Page*	srcKeyObjectPage = File::attachPage(
		m_pTransaction, m_pPhysicalFile,
		Common::ObjectIDData::getFormerValue(SrcKeyObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

	const PhysicalFile::AreaID srcKeyObjectAreaID =
		Common::ObjectIDData::getLatterValue(SrcKeyObjectID_);

	// dst

	PhysicalFile::Page*	dstKeyObjectPage = File::attachPage(
		m_pTransaction, m_pPhysicalFile,
		Common::ObjectIDData::getFormerValue(DstKeyObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

	const PhysicalFile::AreaID dstKeyObjectAreaID =
		Common::ObjectIDData::getLatterValue(DstKeyObjectID_);

	int	compareResult = 0;

	for (int i = 1;
		 i < this->m_cFileParameter.m_TopValueFieldIndex &&
		 compareResult == 0;
		 i++)
	{
		if (*(this->m_cFileParameter.m_IsFixedFieldArray + i))
		{
			compareResult = this->compareFixedField(i,
													srcKeyObjectPage,
													srcKeyObjectAreaID,
													dstKeyObjectPage,
													dstKeyObjectAreaID);
		}
		else
		{
			compareResult = this->compareStringField(i,
													 srcKeyObjectPage,
													 srcKeyObjectAreaID,
													 dstKeyObjectPage,
													 dstKeyObjectAreaID,
													 AttachNodePages_);
		}
	}

	checkMemoryExhaust(srcKeyObjectPage);
	checkMemoryExhaust(dstKeyObjectPage);

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareFixedField -- 2つの固定長フィールドの値を比較する
//
//	NOTES
//	ファイル内に記録されている2つの固定長フィールドの値を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const int					FieldIndex_
//		フィールドインデックス
//	PhysicalFile::Page*			SrcKeyObjectPage_
//		比較元の固定長フィールドの値が記録されている物理ページの記述子
//	const PhysicalFile::AreaID	SrcKeyObjectAreaID_
//		比較元の固定長フィールドの値が記録されている物理エリアの識別子
//	PhysicalFile::Page*			DstKeyObjectPage_
//		比較先の固定長フィールドの値が記録されている物理ページの記述子
//	const PhysicalFile::AreaID	DstKeyObjectAreaID_
//		比較先の固定長フィールドの値が記録されている物理エリアの識別子
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元のフィールド値の方がキー値順で前方
//			= 0 : 2つのフィールドの値が等しい
//			> 0 : 比較元のフィールド値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし
//
int
File::compareFixedField(
	const int					FieldIndex_,
	PhysicalFile::Page*			SrcKeyObjectPage_,
	const PhysicalFile::AreaID	SrcKeyObjectAreaID_,
	PhysicalFile::Page*			DstKeyObjectPage_,
	const PhysicalFile::AreaID	DstKeyObjectAreaID_) const
{
	void*	srcValue = this->getFieldPointer(SrcKeyObjectPage_,
											 SrcKeyObjectAreaID_,
											 FieldIndex_,
											 true); // キーオブジェクト

	void*	dstValue = this->getFieldPointer(DstKeyObjectPage_,
											 DstKeyObjectAreaID_,
											 FieldIndex_,
											 true); // キーオブジェクト

	if (srcValue == 0 || dstValue == 0)
	{
		if (srcValue == 0 && dstValue == 0)
		{
			return 0;
		}

		int	compareResult = (srcValue == 0) ? -1 : 1;

		return
			compareResult *
			*(this->m_cFileParameter.m_MultiNumberArray + FieldIndex_);
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

	return
		compareResult *
		*(this->m_cFileParameter.m_MultiNumberArray + FieldIndex_);
}

//	FUNCTION private
//	Btree::File::compareIntegerField --
//		2つの32ビット符号付き整数値を比較する
//
//	NOTES
//	ファイル内に記録されている2つの32ビット符号付き整数値を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const void*	SrcValue_
//		比較元の値が記録されている領域へのポインタ
//	const void*	DstValue_
//		比較先の値が記録されている領域へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元の値の方がキー値順で前方
//			= 0 : 2つの値が等しい
//			> 0 : 比較元の値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし

// static
int
File::compareIntegerField(const void* SrcValue_,
						  const void* DstValue_)
{
	Common::IntegerData src;
	src.setDumpedValue(static_cast<const char*>(SrcValue_));
	Common::IntegerData dst;
	dst.setDumpedValue(static_cast<const char*>(DstValue_));

	return src.compareTo(&dst);
}

//	FUNCTION private
//	Btree::File::compareUnsignedIntegerField --
//		2つの32ビット符号なし整数値を比較する
//
//	NOTES
//	ファイル内に記録されている2つの32ビット符号なし整数値を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const void*	SrcValue_
//		比較元の値が記録されている領域へのポインタ
//	const void*	DstValue_
//		比較先の値が記録されている領域へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元の値の方がキー値順で前方
//			= 0 : 2つの値が等しい
//			> 0 : 比較元の値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし

// static
int
File::compareUnsignedIntegerField(const void* SrcValue_,
								  const void* DstValue_)
{
	Common::UnsignedIntegerData src;
	src.setDumpedValue(static_cast<const char*>(SrcValue_));
	Common::UnsignedIntegerData dst;
	dst.setDumpedValue(static_cast<const char*>(DstValue_));

	return src.compareTo(&dst);
}

//	FUNCTION private
//	Btree::File::compareInteger64Field --
//		2つの64ビット符号付き整数値を比較する
//
//	NOTES
//	ファイル内に記録されている2つの64ビット符号付き整数値を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const void*	SrcValue_
//		比較元の値が記録されている領域へのポインタ
//	const void*	DstValue_
//		比較先の値が記録されている領域へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元の値の方がキー値順で前方
//			= 0 : 2つの値が等しい
//			> 0 : 比較元の値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし

// static
int
File::compareInteger64Field(const void*	SrcValue_,
							const void*	DstValue_)
{
	Common::Integer64Data src;
	src.setDumpedValue(static_cast<const char*>(SrcValue_));
	Common::Integer64Data dst;
	dst.setDumpedValue(static_cast<const char*>(DstValue_));

	return src.compareTo(&dst);
}

//	FUNCTION private
//	Btree::File::compareUnsignedInteger64Field --
//		2つの64ビット符号なし整数値を比較する
//
//	NOTES
//	ファイル内に記録されている2つの64ビット符号なし整数値を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const void*	SrcValue_
//		比較元の値が記録されている領域へのポインタ
//	const void*	DstValue_
//		比較先の値が記録されている領域へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元の値の方がキー値順で前方
//			= 0 : 2つの値が等しい
//			> 0 : 比較元の値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし

// static
int
File::compareUnsignedInteger64Field(const void*	SrcValue_,
									const void*	DstValue_)
{
	Common::UnsignedInteger64Data src;
	src.setDumpedValue(static_cast<const char*>(SrcValue_));
	Common::UnsignedInteger64Data dst;
	dst.setDumpedValue(static_cast<const char*>(DstValue_));

	return src.compareTo(&dst);
}

//	FUNCTION private
//	Btree::File::compareFloatField -- 2つの32ビット実数値を比較する
//
//	NOTES
//	ファイル内に記録されている2つの32ビット実数値を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const void*	SrcValue_
//		比較元の値が記録されている領域へのポインタ
//	const void*	DstValue_
//		比較先の値が記録されている領域へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元の値の方がキー値順で前方
//			= 0 : 2つの値が等しい
//			> 0 : 比較元の値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし

// static
int
File::compareFloatField(const void*	SrcValue_,
						const void*	DstValue_)
{
	Common::FloatData src;
	src.setDumpedValue(static_cast<const char*>(SrcValue_));
	Common::FloatData dst;
	dst.setDumpedValue(static_cast<const char*>(DstValue_));

	return src.compareTo(&dst);
}

//	FUNCTION private
//	Btree::File::compareDoubleField -- 2つの64ビット実数値を比較する
//
//	NOTES
//	ファイル内に記録されている2つの64ビット実数値を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const void*	SrcValue_
//		比較元の値が記録されている領域へのポインタ
//	const void*	DstValue_
//		比較先の値が記録されている領域へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元の値の方がキー値順で前方
//			= 0 : 2つの値が等しい
//			> 0 : 比較元の値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし

int
File::compareDoubleField(const void* SrcValue_,
						 const void* DstValue_)
{
	Common::DoubleData src;
	src.setDumpedValue(static_cast<const char*>(SrcValue_));
	Common::DoubleData dst;
	dst.setDumpedValue(static_cast<const char*>(DstValue_));

	return src.compareTo(&dst);
}

//	FUNCTION private
//	Btree::File::compareDateField -- 2つの日付を比較する
//
//	NOTES
//	ファイル内に記録されている2つの日付を比較し、比較結果を返す。
//
//	ARGUMENTS
//	const void*	SrcValue_
//		比較元の値が記録されている領域へのポインタ
//	const void*	DstValue_
//		比較先の値が記録されている領域へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元の値の方がキー値順で前方
//			= 0 : 2つの値が等しい
//			> 0 : 比較元の値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし

// static
int
File::compareDateField(const void* SrcValue_,
					   const void* DstValue_)
{
	Common::DateData src;
	src.setDumpedValue(static_cast<const char*>(SrcValue_));
	Common::DateData dst;
	dst.setDumpedValue(static_cast<const char*>(DstValue_));

	return src.compareTo(&dst);
}

//	FUNCTION private
//	Btree::File::compareTimeField -- 2つの時間を比較する
//
//	NOTES
//	ファイル内に記録されている2つの時間を比較し、比較結果を返す。
//
//	ARGUMENTS
//	const void*	SrcValue_
//		比較元の値が記録されている領域へのポインタ
//	const void*	DstValue_
//		比較先の値が記録されている領域へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元の値の方がキー値順で前方
//			= 0 : 2つの値が等しい
//			> 0 : 比較元の値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし

// static
int
File::compareTimeField(const void* SrcValue_,
					   const void* DstValue_)
{
	Common::DateTimeData src;
	src.setDumpedValue(static_cast<const char*>(SrcValue_));
	Common::DateTimeData dst;
	dst.setDumpedValue(static_cast<const char*>(DstValue_));

	return src.compareTo(&dst);
}

//	FUNCTION private
//	Btree::File::compareObjectIDField -- 2つのオブジェクトIDを比較する
//
//	NOTES
//	ファイル内に記録されている2つのオブジェクトIDを比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const void*	SrcValue_
//		比較元のオブジェクトIDが記録されている領域へのポインタ
//	const void*	DstValue_
//		比較先のオブジェクトIDが記録されている領域へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元の値の方がキー値順で前方
//			= 0 : 2つの値が等しい
//			> 0 : 比較元の値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし

// static
int
File::compareObjectIDField(const void* SrcValue_,
						   const void* DstValue_)
{
	Common::ObjectIDData src;
	src.setDumpedValue(static_cast<const char*>(SrcValue_));
	Common::ObjectIDData dst;
	dst.setDumpedValue(static_cast<const char*>(DstValue_));

	return src.compareTo(&dst);
}

//
//	FUNCTION private
//	Btree::File::compareStringField -- 2つの文字列フィールドを比較する
//
//	NOTES
//	ファイル内に記録されている2つの文字列フィールドを比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const int					FieldIndex_
//		フィールドインデックス
//	PhysicalFile::Page*			SrcKeyObjectPage_
//		比較元の文字列フィールドの値が記録されている物理ページの記述子
//	const PhysicalFile::AreaID	SrcKeyObjectAreaID_
//		比較元の文字列フィールドの値が記録されている物理エリアの識別子
//	PhysicalFile::Page*			DstKeyObjectPage_
//		比較先の文字列フィールドの値が記録されている物理ページの記述子
//	const PhysicalFile::AreaID	DstKeyObjectAreaID_
//		比較先の文字列フィールドの値が記録されている物理エリアの識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元のフィールド値の方がキー値順で前方
//			= 0 : 2つのフィールドの値が等しい
//			> 0 : 比較元のフィールド値の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareStringField(
	const int					FieldIndex_,
	PhysicalFile::Page*			SrcKeyObjectPage_,
	const PhysicalFile::AreaID	SrcKeyObjectAreaID_,
	PhysicalFile::Page*			DstKeyObjectPage_,
	const PhysicalFile::AreaID	DstKeyObjectAreaID_,
	PageVector&					AttachNodePages_) const
{
	void*	srcValue = this->getFieldPointer(SrcKeyObjectPage_,
											 SrcKeyObjectAreaID_,
											 FieldIndex_,
											 true); // キーオブジェクト

	void*	dstValue = this->getFieldPointer(DstKeyObjectPage_,
											 DstKeyObjectAreaID_,
											 FieldIndex_,
											 true); // キーオブジェクト

	if (srcValue == 0 || dstValue == 0)
	{
		if (srcValue == 0 && dstValue == 0)
		{
			return 0;
		}

		int	compareResult = (srcValue == 0) ? -1 : 1;

		return
			compareResult *
			*(this->m_cFileParameter.m_MultiNumberArray + FieldIndex_);
	}

	if (*(this->m_cFileParameter.m_FieldOutsideArray + FieldIndex_))
	{
		// 外置き…

		ModUInt64	srcFieldObjectID;
		File::readObjectID(static_cast<char*>(srcValue), srcFieldObjectID);

		ModUInt64	dstFieldObjectID;
		File::readObjectID(static_cast<char*>(dstValue), dstFieldObjectID);

		return this->compareOutsideStringField(FieldIndex_,
											   srcFieldObjectID,
											   SrcKeyObjectPage_,
											   dstFieldObjectID,
											   DstKeyObjectPage_,
											   AttachNodePages_);
	}

	InsideVarFieldLen*	srcValueLen =
		static_cast<InsideVarFieldLen*>(srcValue);
	InsideVarFieldLen*	dstValueLen =
		static_cast<InsideVarFieldLen*>(dstValue);

	int	compareResult = this->compareStringField(srcValueLen + 1,
												 *srcValueLen,
												 dstValueLen + 1,
												 *dstValueLen);

	return
		compareResult *
		*(this->m_cFileParameter.m_MultiNumberArray + FieldIndex_);
}

//
//	FUNCTION private
//	Btree::File::compareOutsideStringField --
//		2つの外置き文字列フィールドを比較する
//
//	NOTES
//	ファイル内に記録されている2つの外置き文字列フィールドを比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const int					FieldIndex_
//		フィールドインデックス
//	const ModUInt64				SrcFieldObjectID_
//		比較元外置きオブジェクトのID
//	PhysicalFile::Page*			SrcDirectObjectPage_
//		比較元外置きオブジェクトへ辿る代表オブジェクトが
//		記録されている物理ページの記述子
//	const ModUInt64				DstFieldObjectID_
//		比較先外置きオブジェクトのID
//	PhysicalFile::Page*			DstDirectObjectPage_
//		比較先外置きオブジェクトへ辿る代表オブジェクトが
//		記録されている物理ページの記述子
//	Btree::PageVector&			AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元のフィールド値の方がキー値順で前方
//			= 0 : 2つのフィールドの値が等しい
//			> 0 : 比較元のフィールド値の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareOutsideStringField(
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
		getConstAreaTop(srcFieldObjectPage, srcFieldObjectAreaID);

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
		getConstAreaTop(dstFieldObjectPage, dstFieldObjectAreaID);

	const File::ObjectType*	srcObjectType =
		static_cast<const File::ObjectType*>(srcFieldObjectAreaTop);

	const File::ObjectType*	dstObjectType =
		static_cast<const File::ObjectType*>(dstFieldObjectAreaTop);

	int	compareResult = 0;

	// check object type

	if (*srcObjectType != File::NormalObjectType ||
		*dstObjectType != File::NormalObjectType)
	{
		// 分割されているか、圧縮されている…

		Common::Data::Pointer	srcStringField;

		File::readOutsideVariableField(this->m_pTransaction,
									   SrcDirectObjectPage_,
									   SrcFieldObjectID_,
									   Common::DataType::String,
									   srcStringField,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachPages_);

		Common::Data::Pointer	dstStringField;

		File::readOutsideVariableField(this->m_pTransaction,
									   DstDirectObjectPage_,
									   DstFieldObjectID_,
									   Common::DataType::String,
									   dstStringField,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachPages_);

		compareResult =
			srcStringField.get()->compareTo(dstStringField.get());
	}
	else
	{
		Os::Memory::Size	srcFieldLen =
			srcFieldObjectPage->getAreaSize(srcFieldObjectAreaID) -
			File::ObjectTypeArchiveSize;

		Os::Memory::Size	dstFieldLen =
			dstFieldObjectPage->getAreaSize(dstFieldObjectAreaID) -
			File::ObjectTypeArchiveSize;

		compareResult = this->compareStringField(srcObjectType + 1,
												 srcFieldLen,
												 dstObjectType + 1,
												 dstFieldLen);
	}

	if (this->m_CatchMemoryExhaust && srcAttached)
	{
		srcFieldObjectPage->getFile()->detachPage(
			srcFieldObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	if (this->m_CatchMemoryExhaust && dstAttached)
	{
		dstFieldObjectPage->getFile()->detachPage(
			dstFieldObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	return
		compareResult *
		*(this->m_cFileParameter.m_MultiNumberArray + FieldIndex_);
}

//
//	FUNCTION private
//	Btree::File::compareStringField -- 2つの文字列フィールドを比較する
//
//	NOTES
//	ファイル内に記録されている2つの文字列フィールドを比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const void*				SrcField_
//		比較元文字列フィールドの値が記録されている領域へのポインタ
//	const Os::Memory::Size	SrcFieldLen_
//		比較元文字列フィールド長 [byte]
//	const void*				DstField_
//		比較先文字列フィールドの値が記録されている領域へのポインタ
//	const Os::Memory::Size	DstFieldLen_
//		比較先文字列フィールド長 [byte]
//
//	RETURN
//	int
//		比較結果
//			< 0 : 比較元のフィールド値の方がキー値順で前方
//			= 0 : 2つのフィールドの値が等しい
//			> 0 : 比較元のフィールド値の方がキー値順で後方
//
//	EXCEPTIONS
//	なし
//
int
File::compareStringField(const void*			SrcField_,
						 const Os::Memory::Size	SrcFieldLen_,
						 const void*			DstField_,
						 const Os::Memory::Size	DstFieldLen_) const
{
	ModSize	srcNumChar = SrcFieldLen_ / sizeof(ModUnicodeChar);
	ModSize	dstNumChar = DstFieldLen_ / sizeof(ModUnicodeChar);

	const ModUnicodeChar*	srcField =
		static_cast<const ModUnicodeChar*>(SrcField_);

	const ModUnicodeChar*	dstField =
		static_cast<const ModUnicodeChar*>(DstField_);

	ModUnicodeChar	forZeroByte = 0;
	const ModUnicodeChar*	field = 0;

	field = (srcNumChar > 0) ? srcField : &forZeroByte;

	ModUnicodeString	srcString(field, srcNumChar);

	field = (dstNumChar > 0) ? dstField : &forZeroByte;

	ModUnicodeString	dstString(field, dstNumChar);

	return srcString.compare(dstString);
}

//
//	FUNCTION private
//	Btree::File::getLastObjectIDInNode --
//		ノード内の最終キー情報に記録されているオブジェクトIDを返す
//
//	NOTES
//	ノード内の最終キー情報に記録されている
//	キーオブジェクト、バリューオブジェクトいずれかのオブジェクトIDを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*	NodePage_
//		ノードページの物理ページ記述子
//	const bool			IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	const bool			GetValueObjectID_
//		キーオブジェクト、バリューオブジェクト
//		いずれのオブジェクトIDを返すか
//			true  : バリューオブジェクトのIDを返す
//			false : キーオブジェクトのIDを返す
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//
//	EXCEPTIONS
//	なし
//
ModUInt64
File::getLastObjectIDInNode(PhysicalFile::Page*	NodePage_,
							const bool			IsLeafPage_,
							const bool			GetValueObjectID_) const
{
	; _SYDNEY_ASSERT(NodePage_ != 0);

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   NodePage_,
								   IsLeafPage_);

	ModUInt32	useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	; _SYDNEY_ASSERT(useKeyInfoNum > 0);

	KeyInformation	keyInfo(this->m_pTransaction,
							NodePage_,
							useKeyInfoNum - 1,
							IsLeafPage_);

	ModUInt64	objectID = FileCommon::ObjectID::Undefined;

	objectID =
		GetValueObjectID_ ?
			keyInfo.readValueObjectID() : keyInfo.readKeyObjectID();

	; _SYDNEY_ASSERT(objectID != FileCommon::ObjectID::Undefined &&
					 objectID != 0);

	return objectID;
}

//
//	FUNCTION private
//	Btree::File::writeKeyObject --
//		ノードページにキーオブジェクトを書き込む
//
//	NOTES
//	ノードページにキーオブジェクトを書き込む
//
//	ARGUMENTS
//	PhysicalFile::Page*		TopNodePage_
//		ノードページの物理ページ記述子
//	Common::DataArrayData*	Object_
//		書き込むオブジェクトへのポインタ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	ModUInt64
//		書き込んだキーオブジェクトのID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::writeKeyObject(PhysicalFile::Page*	TopNodePage_,
					 Common::DataArrayData*	Object_,
					 PageVector&			AttachNodePages_,
					 PageIDVector&			AllocateNodePageIDs_,
					 const bool				IsLeafPage_) const
{
	Os::Memory::Size	directObjectSize =
		this->m_cFileParameter.m_DirectKeyObjectSize;

	PhysicalFile::Page*	directObjectPage =
		this->searchInsertNodePage(TopNodePage_,
								   AttachNodePages_,
								   AllocateNodePageIDs_,
								   directObjectSize,
								   IsLeafPage_);

	PhysicalFile::PageID	directObjectPageID = directObjectPage->getID();

	PhysicalFile::AreaID	directObjectAreaID =
		directObjectPage->allocateArea(*this->m_pTransaction,
									   directObjectSize);

	char*	directObjectAreaTop =
		static_cast<char*>(File::getAreaTop(directObjectPage,
											directObjectAreaID));

	ModVector<ModUInt64>	variableFieldObjectIDs;

	int	i;

	if (this->m_cFileParameter.m_ExistVariableFieldInKey &&
		this->m_cFileParameter.m_ExistOutsideFieldInKey)
	{
		// 外置きの可変長キーフィールドが存在する…

		for (i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
		{
			if (*(this->m_cFileParameter.m_IsFixedFieldArray + i) ==
				false &&
				*(this->m_cFileParameter.m_FieldOutsideArray + i))
			{
				// 外置き可変長キーフィールド…

				const Common::Data*	variableField =
					Object_->getElement(i).get();

				; _SYDNEY_ASSERT(variableField != 0);

				if (!variableField->isNull())
				{
					// フィールド値がヌル値ではない…

					//
					// では、外置き可変長フィールドオブジェクトを
					// 書き込んでオブジェクトIDを取得する
					//

					ModUInt64	variableFieldObjectID =
						this->writeOutsideVariableKey(directObjectPage,	//searchInsertNodePage() で見つけたページ
													  variableField,
													  AttachNodePages_,
													  AllocateNodePageIDs_,
													  IsLeafPage_);

					variableFieldObjectIDs.pushBack(variableFieldObjectID);
				}
			}
		}
	}

	//
	// 配列はキーフィールドにできないので、
	// ここでは、考慮不要。
	//

	ModVector<ModUInt64>::Iterator	variableFieldObjectIDIterator =
		variableFieldObjectIDs.begin();
	ModVector<ModUInt64>::Iterator	variableFieldObjectIDsEnd =
		variableFieldObjectIDs.end();

	File::ObjectType*	objectType =
		syd_reinterpret_cast<File::ObjectType*>(directObjectAreaTop);

	*objectType = File::NormalObjectType;

	NullBitmap::Value*	nullBitmapTop =
		syd_reinterpret_cast<NullBitmap::Value*>(objectType + 1);

	NullBitmap::clear(nullBitmapTop,
					  this->m_cFileParameter.m_KeyNum);

	char*	keyWritePos =
		static_cast<char*>(
			NullBitmap::getTail(nullBitmapTop,
								this->m_cFileParameter.m_KeyNum));

	for (i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
	{
		const Common::Data*	keyField = Object_->getElement(i).get();

		if (keyField->isNull())
		{
			// キー値としてヌル値が設定されていた…

			keyWritePos = this->writeNullKey(nullBitmapTop,
											 keyWritePos,
											 i);
		}
		else if (*(this->m_cFileParameter.m_IsFixedFieldArray + i))
		{
			// 固定長フィールド…

			keyWritePos = File::writeFixedField(keyWritePos, keyField);
		}
		else
		{
			// 可変長フィールド…

			// キーフィールドにできる可変長フィールドは
			// String型のフィールドだけである。
			; _SYDNEY_ASSERT(
				*(this->m_cFileParameter.m_FieldTypeArray + i) ==
				Common::DataType::String);

			if (*(this->m_cFileParameter.m_FieldOutsideArray + i))
			{
				// 外置き可変長フィールド…

				; _SYDNEY_ASSERT(variableFieldObjectIDIterator !=
								 variableFieldObjectIDsEnd);

				keyWritePos =
					File::writeObjectID(keyWritePos,
										*variableFieldObjectIDIterator);

				variableFieldObjectIDIterator++;
			}
			else
			{
				// 外置きではない可変長フィールド…

				Os::Memory::Size	maxLen =
					*(this->m_cFileParameter.m_FieldMaxLengthArray + i);

				keyWritePos = File::writeInsideVariableField(keyWritePos,
															 keyField,
															 maxLen);
			}
		}
	}

	; _SYDNEY_ASSERT(variableFieldObjectIDIterator ==
					 variableFieldObjectIDsEnd);

#ifdef DEBUG

	Os::Memory::Size	areaSize =
		directObjectPage->getAreaSize(directObjectAreaID);

	Os::Memory::Size	writeSize =
		static_cast<Os::Memory::Size>(keyWritePos - directObjectAreaTop);

	; _SYDNEY_ASSERT(writeSize == areaSize);

#endif

	if (this->m_CatchMemoryExhaust && (directObjectPage != TopNodePage_))
	{
		this->m_pPhysicalFile->detachPage(
			directObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	return File::makeObjectID(directObjectPageID, directObjectAreaID);
}

//
//	FUNCTION private
//	Btree::File::writeNullKey -- キー値としてヌル値を書き込む
//
//	NOTES
//	キーフィールドの値としてヌル値を書き込む。
//
//	ARGUMENTS
//	Btree::NullBitmap::Value*	NullBitmapTop_
//		ヌルビットマップ先頭へのポインタ
//	char*						KeyTop_
//		キーフィールドの値を書き込む領域へのポインタ
//	const int					KeyFieldIndex_
//		キーフィールドのインデックス
//
//	RETURN
//	char*
//		次のキーフィールドの値を書き込む領域へのポインタ
//
//	EXCEPTIONS
//	なし
//
char*
File::writeNullKey(NullBitmap::Value*	NullBitmapTop_,
				   char*				KeyTop_,
				   const int			KeyFieldIndex_) const
{
	; _SYDNEY_ASSERT(NullBitmapTop_ != 0);
	; _SYDNEY_ASSERT(KeyTop_ != 0);
	; _SYDNEY_ASSERT(
		KeyFieldIndex_ > 0 &&
		KeyFieldIndex_ < this->m_cFileParameter.m_TopValueFieldIndex);

	//
	// ヌルビットマップのビットをONする。
	//

	NullBitmap::on(NullBitmapTop_,
				   this->m_cFileParameter.m_KeyNum,
				   KeyFieldIndex_ - 1);

	//
	// キー値の書き込みサイズ分をスキップする。
	//

	char*	nextKeyTop = 0;

	if (*(this->m_cFileParameter.m_IsFixedFieldArray + KeyFieldIndex_))
	{
		// 固定長フィールド…

		//
		// キー値の書き込みサイズ分をスキップする。
		//

		Common::DataType::Type	keyDataType =
			*(this->m_cFileParameter.m_FieldTypeArray + KeyFieldIndex_);

		Os::Memory::Size	keySize =
			FileCommon::DataManager::getFixedCommonDataArchiveSize(
				keyDataType);

		nextKeyTop = KeyTop_ + keySize;
	}
	else
	{
		// 可変長フィールド…

		if (*(this->m_cFileParameter.m_FieldOutsideArray + KeyFieldIndex_))
		{
			// 外置き可変長フィールド…

			//
			// 外置きオブジェクトのオブジェクトIDの書き込みサイズ分を
			// スキップする。
			//

			nextKeyTop = KeyTop_ + File::ObjectIDArchiveSize;
		}
		else
		{
			// 外置きではない可変長フィールド…

			//
			// キー値のサイズの書き込みサイズ分と
			// フィールド最大長分、スキップする。
			//

			//
			// 外置きではない可変長フィールドは、
			// 代表オブジェクト内で、
			// 下図のような物理構成となっている。
			//
			//     ┌───────────┐
			//     │　　フィールド長　　　│ 1[byte]
			//     ├───────────┤
			//     │　　フィールド値　　　│ *
			//     └───────────┘
			//

			//
			// ※ 外置きではない可変長フィールドでは、
			// 　 フィールド値を記録する領域として、
			// 　 フィールド最大長の分だけ確保してある。
			//

			nextKeyTop =
				KeyTop_ +
				File::InsideVarFieldLenArchiveSize +
				*(this->m_cFileParameter.m_FieldMaxLengthArray +
				  KeyFieldIndex_);
		}
	}

	; _SYDNEY_ASSERT(nextKeyTop != 0);

	return nextKeyTop;
}

//	FUNCTION private
//	Btree::File::writeObjectID --
//		外置きフィールドオブジェクトのIDを書き込む
//
//	NOTES
//	代表オブジェクトに、外置きフィールドオブジェクトのIDを書き込む。
//
//	ARGUMENTS
//	char*			WritePos_
//		外置きフィールドオブジェクトのIDを書き込む領域へのポインタ
//	const ModUInt64	ObjectID_
//		書き込む外置きフィールドオブジェクトのID
//
//	RETURN
//	char*
//		次のフィールドの値を書き込む領域へのポインタ
//
//	EXCEPTIONS
//	なし

// static
char*
File::writeObjectID(char*			WritePos_,
					const ModUInt64	ObjectID_)
{
	return WritePos_ + Common::ObjectIDData(ObjectID_).dumpValue(WritePos_);
}

//	FUNCTION private
//	Btree::File::readObjectID --
//		外置きフィールドオブジェクトのIDを読み込む
//
//	NOTES
//	代表オブジェクトに記録されている、外置きフィールドオブジェクトの
//	IDを読み込む。
//
//	ARGUMENTS
//	const char*	ReadPos_
//		外置きフィールドオブジェクトのIDが書き込まれている領域へのポインタ
//	ModUInt64&	ObjectID_
//		外置きフィールドオブジェクトを読み込む先
//
//	RETURN
//	const char*
//		次のフィールドの値が書き込まれている領域へのポインタ
//
//	EXCEPTIONS
//	なし

// static
const char*
File::readObjectID(const char*	ReadPos_,
				   ModUInt64&	ObjectID_)
{
	Common::ObjectIDData data;
	ReadPos_ += data.setDumpedValue(ReadPos_);
	ObjectID_ = data.getValue();
	return ReadPos_;
}

//
//	FUNCTION private
//	Btree::File::writeFixedField -- 固定長フィールドの値を書き込む
//
//	NOTES
//	固定長フィールドの値を書き込む。
//
//	ARGUMENTS
//	char*				FieldTop_
//		書き込み先の領域へのポインタ
//	const Common::Data*	Field_
//		書き込む値が記録されている固定長フィールドデータ
//
//	RETURN
//	char*
//		次のフィールドの値が書き込まれている領域へのポインタ
//
//	EXCEPTIONS
//	[YET!]
//
// static
char*
File::writeFixedField(char*					FieldTop_,
					  const Common::Data*	Field_)
{
	FileCommon::DataManager::writeFixedCommonData(*Field_, FieldTop_);

	return
		FieldTop_ +
		FileCommon::DataManager::getFixedCommonDataArchiveSize(*Field_);
}

//
//	FUNCTION private
//	Btree::File::shiftKeyInfoForInsert --
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
File::shiftKeyInfoForInsert(PhysicalFile::Page*	KeyInfoPage_,
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
							   IsLeafPage_);

	KeyInformation	dstKeyInfo(this->m_pTransaction,
							   KeyInfoPage_,
							   srcKeyInfoIndex + 1,
							   IsLeafPage_);

	PhysicalFile::PageID	keyInfoPageID = KeyInfoPage_->getID();

	PhysicalFile::Page*	valueObjectPage = 0;

	do
	{
		srcKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex);

		dstKeyInfo.setStartOffsetByIndex(srcKeyInfoIndex + 1);

		dstKeyInfo.writeKeyObjectID(srcKeyInfo.readKeyObjectID());

		if (IsLeafPage_)
		{
			ModUInt64	srcValueObjectID = srcKeyInfo.readValueObjectID();
			
			dstKeyInfo.writeValueObjectID(srcValueObjectID);

			ValueFile_->update(srcValueObjectID,
							   keyInfoPageID,
							   srcKeyInfoIndex + 1,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);
		}
		else
		{
			dstKeyInfo.writeChildNodePageID(
				srcKeyInfo.readChildNodePageID());
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
//	Btree::File::isSplitCondition -- 分割条件の検査
//
//	NOTES
//	ノードページが分割する条件に達したか検査する。
//
//	分割条件
//	旧：キーテーブル内のキーの数が次数を超えた場合
//      ※ “物理的に一杯かどうか”は問題にしない。
//
//	新：キーテーブル内のキーの数が２以上で、←重要！！
//		かつ、ノードの合計サイズが物理ページを越えた場合に分割
//      ※ 次数の条件に加え、“物理的に一杯かどうか”も問題にする。
//      ∵削除時のマージ結果複数ページ使用していることがある。
//
//      削除時のマージとの兼ね合い：
//      キーテーブル内のキー数が小さいまま分割するので、
//      削除した途端マージの条件が成立する可能性がある。
//      最悪削除の度ごとにマージすることになり遅くなる。
//      →この問題は現時点では、既知の問題として無視する。
//        ∵ヘッダ情報にはキー数しか記録されていない。
//          マージ条件に合計サイズを考慮させると、
//          ファイルフォーマットの変更か、マージ先ノードの
//          キーオブジェクトの走査が必要になる。
//
//	ARGUMENTS
//	const PhysicalFile::Page& KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//
//	const NodePageHeader& pageheader_
//		ノードページのヘッダ
//
//	KeyInformation& keyInfo_
//		キー情報
//
//	Common::DataArrayData*		Object_ = 0;
//		挿入するオブジェクトへのポインタ
//
//	RETURN
//	bool
//		ノードの分割条件に達したか
//			true  : 条件に達した
//			false : 条件に達しなかった
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::isSplitCondition( const PhysicalFile::Page& KeyInfoPage_
                          ,const NodePageHeader& pageheader_
                          ,KeyInformation& keyInfo_
                          ,const Common::DataArrayData* Object_ // = 0
                          ) const
{
	ModUInt32 keyNumInNode = pageheader_.readUseKeyInformationNumber();

	// ノードページ一杯にキーオブジェクトが記録されるので、ノードページを分割する。
	// 旧：
	//   ※ “物理的に一杯かどうか”ではない。
	//   　 次数で一杯になったのである。
	// 新：
	//   ※ “次数で一杯かどうか”に加え、
	//   　 物理的に一杯になったの場合も含む。
	//

	if (keyNumInNode < 2)
	{
		// 分割可能なノード数に達していない
		return false;
	}
	else if (this->m_cFileParameter.m_KeyPerNode <= keyNumInNode)
	{
		// 既にキーテーブルが一杯
		return true;
	}
	else if (! (this->m_cFileParameter.m_ExistVariableFieldInKey && this->m_cFileParameter.m_ExistOutsideFieldInKey) )
	{
		// 外置きの可変長キーフィールドは存在しない → 何もしない
		// ∵次数の決定時に 1ページに収まる様にページサイズを調整している。
		return false;
	}
	else
	{
		// 外置きの可変長キーフィールドが存在する…

		// 挿入するキーオブジェクトが存在しない → すでにページに記録済み（判定の対象外とする。）
		if (Object_)
		{
			// 外置き可変長キーの合計サイズについて
			Os::Memory::Size fieldLen = 0;
			for (int i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; ++i)
			{
				// 外置き可変長キーフィールド
				if ( *(this->m_cFileParameter.m_IsFixedFieldArray + i) == false
				  && *(this->m_cFileParameter.m_FieldOutsideArray + i))
				{
					const Common::Data*	const field = Object_->getElement(i).get();
					; _SYDNEY_ASSERT(field);

					if (!field->isNull())
					{
						const void*			buf;//dummy
						Os::Memory::Size	len = 0;
						FileCommon::DataManager::getCommonDataBuffer(*field, buf, len);

						fieldLen += len;// 外置き可変長キーフィールド長の合計
					}
				}
			}
			if (pageheader_.getFreeAreaSize() <= fieldLen )
			{
				// 外置き可変長キーの合計サイズがページのFreeAreaのサイズを超えていた。
				return true;
			}
		}

		// 構成するページ数の合計について
		// ※既に複数ページで構成されていることは、削除時のマージの際に起こりえる。

		// キーテーブルを走査して、
		// すべての代表キーオブジェクトがキーテーブルのあるページに現れるかどうか、
		// すべての外置き可変長代表キーオブジェクトが同じページにあるかどうか、
		// すべての外置き可変長代表キーオブジェクトのタイプがDivideでないかどうかを確認。

		const PhysicalFile::PageID  PAGEID = KeyInfoPage_.getID();
		for (ModUInt32 i = 0; i < keyNumInNode; ++i)
		{
			keyInfo_.setStartOffsetByIndex(i);

			const ModUInt64 OID = keyInfo_.readKeyObjectID();

			if (Common::ObjectIDData::getFormerValue(OID) != PAGEID) {
				// 代表キーオブジェクトが異なるページにある。
				return true;
				//break;
			}

			// 代表キーオブジェクトは同じページにある。
			for (int i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; ++i)
			{
				if (*(this->m_cFileParameter.m_FieldOutsideArray + i)) //外置き可変長キーフィールドについて
				{
					const char* const OIDPOS = static_cast<char*>(
						getFieldPointer(
							KeyInfoPage_,
							Common::ObjectIDData::getLatterValue(OID),
							i, true)); // キーオブジェクト
					if (!OIDPOS) continue;// OIDPOS が空

					// 外置き可変長キーフィールドオブジェクトが存在する。
					// ∵ キーフィールドに配列フィールドは存在しない。
					ModUInt64	objectID;
					File::readObjectID(OIDPOS, objectID);

					if (Common::ObjectIDData::getFormerValue(objectID) != PAGEID ) {
						// 外置き可変長キーフィールドオブジェクトが異なるページにある。
						return true;
						//break;
					}

					// 外置き可変長キーフィールドオブジェクトが同じページにある。
					const File::ObjectType*	OBJECTTYPE =
						static_cast<const File::ObjectType*>(
							File::getConstAreaTop(
								KeyInfoPage_,
							Common::ObjectIDData::getLatterValue(objectID)));

					if ( *OBJECTTYPE & (File::DivideObjectType | File::DivideArrayObjectType | File::DivideCompressedObjectType) ) {
						// 外置き可変長キーフィールドオブジェクトが複数ページで構成されている。
						return true;
						//break;
					}
				}
			}
		}
	}
	// まだ、ツリーノードには余裕がある。
	return false;
}


//
//	FUNCTION private
//	Btree::File::splitNodePage -- ノードを分割する
//
//	NOTES
//	必要ならば、ノードの分割を行う。
//
//	分割条件が変更になったが、分割処理自体は変わらない。
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
//	Common::DataArrayData*		Object_ = 0
//		挿入するオブジェクトへのポインタ
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
File::splitNodePage(FileInformation&		FileInfo_,
					PhysicalFile::Page*		KeyInfoPage_,
					PageVector&				AttachNodePages_,
					PageIDVector&			AllocateNodePageIDs_,
					PhysicalFile::Page*&	NewKeyInfoPage_,
					const bool				IsLeafPage_,
					ValueFile*				ValueFile_,
					PageVector&				AttachValuePages_,
					const Common::DataArrayData* Object_ // = 0
					) const
{
	_SYDNEY_ASSERT(KeyInfoPage_);
	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   KeyInfoPage_,
								   IsLeafPage_);

	ModUInt32	keyNumInNode =
		nodePageHeader.readUseKeyInformationNumber();

	KeyInformation	kInfo(this->m_pTransaction, KeyInfoPage_, 0/*先頭から*/, IsLeafPage_);
	// ノードページが分割する条件に達したか
	if (! isSplitCondition(*KeyInfoPage_, nodePageHeader, kInfo, Object_)) return false;

	// 以下、ノードページが分割する条件に達した時の処理

	//	分割条件が変更になったが、分割処理自体は変わらない。

	; _SYDNEY_ASSERT(
		keyNumInNode <= this->m_cFileParameter.m_KeyPerNode);

	ModVector<ModUInt64>	keyObjectIDs;     // for node/leaf
	PageIDVector			childNodePageIDs; // for node
	ModVector<ModUInt64>	dataObjectIDs;    // for leaf

	keyObjectIDs.reserve(keyNumInNode);

	if (IsLeafPage_)
	{
		dataObjectIDs.reserve(keyNumInNode);
	}
	else
	{
		childNodePageIDs.reserve(keyNumInNode);
	}

	ModUInt32	startKeyInfoIndex =
		static_cast<ModUInt32>(
			keyNumInNode / 100.0 *
			this->m_cFileParameter.m_NodeKeyDivideRate);

	if (startKeyInfoIndex == 0)
	{
		startKeyInfoIndex = 1;// ∵0番はかならず元ページに残す
	}
	else if (startKeyInfoIndex >= keyNumInNode)
	{
		startKeyInfoIndex = keyNumInNode - 1;
	}


	this->setKeyTableToVector(KeyInfoPage_,
							  keyNumInNode,
							  startKeyInfoIndex,
							  keyObjectIDs,
							  childNodePageIDs,
							  dataObjectIDs,
							  IsLeafPage_);

	NewKeyInfoPage_ = this->createNodePage(AttachNodePages_,
										   AllocateNodePageIDs_,
										   IsLeafPage_);

	NodePageHeader	newNodePageHeader(this->m_pTransaction,
									  NewKeyInfoPage_,
									  IsLeafPage_);

	ModUInt32	moveKeyNum = keyObjectIDs.getSize();

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

	int	keyInfoIndex = 0;
	KeyInformation	keyInfo(this->m_pTransaction,
							NewKeyInfoPage_,
							keyInfoIndex,
							IsLeafPage_);

	PhysicalFile::Page*	valueObjectPage = 0;

	PageIDVector::Iterator			childNodePageIDIterator = childNodePageIDs.begin();
	ModVector<ModUInt64>::Iterator	dataObjectIDIterator = dataObjectIDs.begin();

	ModVector<ModUInt64>::Iterator	keyObjectIDEnd = keyObjectIDs.end();
	for ( ModVector<ModUInt64>::Iterator itr = keyObjectIDs.begin(); itr != keyObjectIDEnd; ++itr )
	{
		ModUInt64	keyObjectID = this->moveKey(NewKeyInfoPage_,
												*itr,
												AttachNodePages_,
												AllocateNodePageIDs_,
												IsLeafPage_);

		keyInfo.setStartOffsetByIndex(keyInfoIndex);

		keyInfo.writeKeyObjectID(keyObjectID);

		if (IsLeafPage_)
		{
			keyInfo.writeValueObjectID(*dataObjectIDIterator);

			ValueFile_->update(*dataObjectIDIterator,
							   NewKeyInfoPage_->getID(),
							   keyInfoIndex,
							   this->m_CatchMemoryExhaust,
							   valueObjectPage,
							   AttachValuePages_);

			++dataObjectIDIterator;
		}
		else
		{
			keyInfo.writeChildNodePageID(*childNodePageIDIterator);

			this->resetNodePageHeaderParentNodePageID(
				NewKeyInfoPage_->getID(),
				*childNodePageIDIterator,
				AttachNodePages_,
				IsLeafPage_);

			++childNodePageIDIterator;
		}

		++keyInfoIndex;
	}

	this->compactionNodePage(KeyInfoPage_, AttachNodePages_, IsLeafPage_);

	return true;
}

//
//	FUNCTION private
//	Btree::File::setKeyTableToVector -- キーテーブルをベクターにつむ
//
//	NOTES
//	引数KeyInfoPage_で指定されたノードページ内に記録されている
//	キーテーブル内の情報を、引数KeyObjectIDs_などのベクターに
//	コピーする。
//
//	ARGUMENTS
//	PhysicalFile::Page*		KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32			KeyNumInNode_
//		ノードページ内のキー情報数
//	const ModUInt32			StartKeyInfoIndex_
//		ベクターにコピーを開始するキー情報のインデックス
//	ModVector<ModUInt64>&	KeyObjectIDs_
//		キーオブジェクトのIDをつむベクターへの参照
//	Btree::PageIDVector&	ChildNodePageIDs_
//		子ノードページの物理ページ識別子をつむベクターへの参照
//	ModVector<ModUInt64>&	ValueObjectIDs_
//		バリューオブジェクトのIDをつむベクターへの参照
//	const bool				IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページではないノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::setKeyTableToVector(PhysicalFile::Page*	KeyInfoPage_,
						  const ModUInt32		KeyNumInNode_,
						  const ModUInt32		StartKeyInfoIndex_,
						  ModVector<ModUInt64>&	KeyObjectIDs_,
						  PageIDVector&			ChildNodePageIDs_,
						  ModVector<ModUInt64>&	ValueObjectIDs_,
						  const bool			IsLeafPage_) const
{
	; _SYDNEY_ASSERT(KeyInfoPage_ != 0);
	; _SYDNEY_ASSERT(StartKeyInfoIndex_ < KeyNumInNode_);

	KeyInformation	keyInfo(this->m_pTransaction,
							KeyInfoPage_,
							StartKeyInfoIndex_,
							IsLeafPage_);
								
	for (ModUInt32 i = StartKeyInfoIndex_; i < KeyNumInNode_; i++)
	{
		keyInfo.setStartOffsetByIndex(i);

		KeyObjectIDs_.pushBack(keyInfo.readKeyObjectID());

		if (IsLeafPage_)
		{
			ValueObjectIDs_.pushBack(keyInfo.readValueObjectID());
		}
		else
		{
			ChildNodePageIDs_.pushBack(keyInfo.readChildNodePageID());
		}
	}
}

//
//	FUNCTION private
//	Btree::File::createNodePage -- ノードページを生成する
//
//	NOTES
//	ノードページを生成する。
//
//	ARGUMENTS
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
//		生成するノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	PhysicalFile::Page*
//		生成したノードページの物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::createNodePage(PageVector&	AttachNodePages_,
					 PageIDVector&	AllocateNodePageIDs_,
					 const bool		IsLeafPage_) const
{
	PhysicalFile::PageID	topNodePageID =
		this->m_pPhysicalFile->allocatePage(*this->m_pTransaction);

	AllocateNodePageIDs_.pushBack(topNodePageID);

	PhysicalFile::Page*	topNodePage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 topNodePageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	Os::Memory::Size	nodePageHeaderSize =
		NodePageHeader::getArchiveSize(IsLeafPage_);

	PhysicalFile::AreaID	nodePageHeaderAreaID =
		topNodePage->allocateArea(*this->m_pTransaction,
								  nodePageHeaderSize);

	; _SYDNEY_ASSERT(nodePageHeaderAreaID == NodePageHeader::AreaID);

	NodePageHeader	topNodePageHeader(this->m_pTransaction,
									  topNodePage,
									  IsLeafPage_);

	topNodePageHeader.write(PhysicalFile::ConstValue::UndefinedPageID,
							PhysicalFile::ConstValue::UndefinedPageID,
							PhysicalFile::ConstValue::UndefinedPageID,
							this->m_cFileParameter.m_KeyPerNode,
							0,
							PhysicalFile::ConstValue::UndefinedPageID,
							PhysicalFile::ConstValue::UndefinedPageID);

	PhysicalFile::AreaID	keyTableAreaID =
		topNodePage->allocateArea(
			*this->m_pTransaction,
			(KeyInformation::getSize(IsLeafPage_,
									 this->m_cFileParameter.m_KeyPosType,
									 this->m_cFileParameter.m_KeyNum) +
			 this->m_cFileParameter.m_KeySize) *
			this->m_cFileParameter.m_KeyPerNode);

	; _SYDNEY_ASSERT(keyTableAreaID == KeyInformation::KeyTableAreaID);

	Os::Memory::Size	freeAreaSize =
		topNodePage->getFreeAreaSize(*this->m_pTransaction,
									 1);

	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyObject)
	{
		// キー値をキーオブジェクトに記録する…
		// （キー値をキー情報の外に記録する…）

		Os::Memory::Size	objectSize =
			this->m_cFileParameter.m_DirectKeyObjectSize;

		if (freeAreaSize < objectSize)
		{
			PhysicalFile::PageID	lastNodePageID =
				this->m_pPhysicalFile->allocatePage(*this->m_pTransaction);

			AllocateNodePageIDs_.pushBack(lastNodePageID);

			PhysicalFile::Page*	lastNodePage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 lastNodePageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			; _SYDNEY_ASSERT(lastNodePage != 0);

			nodePageHeaderAreaID =
				lastNodePage->allocateArea(*this->m_pTransaction,
										   nodePageHeaderSize);

			; _SYDNEY_ASSERT(nodePageHeaderAreaID == NodePageHeader::AreaID);

			NodePageHeader	lastNodePageHeader(this->m_pTransaction,
											   lastNodePage,
											   IsLeafPage_);

			lastNodePageHeader.write(
				PhysicalFile::ConstValue::UndefinedPageID,
				topNodePageID,
				PhysicalFile::ConstValue::UndefinedPageID,
				0,
				0,
				PhysicalFile::ConstValue::UndefinedPageID,
				PhysicalFile::ConstValue::UndefinedPageID);

			if (this->m_CatchMemoryExhaust)
			{
				this->m_pPhysicalFile->detachPage(
					lastNodePage,
					PhysicalFile::Page::UnfixMode::Dirty,
					false); // 本当にデタッチ（アンフィックス）してしまい、
					        // 物理ファイルマネージャが
					        // キャッシュしないようにする。
			}

			topNodePageHeader.writeNextPhysicalPageID(lastNodePageID);
		}
	}

	return topNodePage;
}

//
//	FUNCTION private
//	Btree::File::resetLeafPageLink --
//		リーフページ同士の双方向リンクをはりかえる
//
//	NOTES
//	リーフページ同士の双方向リンクをはりかえる
//
//	ARGUMENTS
//	PhysicalFile::Page*	KeyInfoPage1_
//		リーフページの物理ページ記述子
//	PhysicalFile::Page*	KeyInfoPage2_
//		リーフページの物理ページ記述子
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
File::resetLeafPageLink(PhysicalFile::Page*	KeyInfoPage1_,
						PhysicalFile::Page*	KeyInfoPage2_,
						PageVector&			AttachNodePages_) const
{
	NodePageHeader	leafPageHeader1(this->m_pTransaction,
									KeyInfoPage1_,
									true); // リーフページ

	NodePageHeader	leafPageHeader2(this->m_pTransaction,
									KeyInfoPage2_,
									true); // リーフページ

	PhysicalFile::PageID	keyInfoPageID1 = KeyInfoPage1_->getID();
	PhysicalFile::PageID	keyInfoPageID2 = KeyInfoPage2_->getID();

	PhysicalFile::PageID	oldNextLeafPageID =
		leafPageHeader1.readNextLeafPageID();

	if (oldNextLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		PhysicalFile::Page*	keyInfoPage3 =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 oldNextLeafPageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		NodePageHeader	leafPageHeader3(this->m_pTransaction,
										keyInfoPage3,
										true); // リーフページ
		; _SYDNEY_ASSERT(
			leafPageHeader3.readPrevLeafPageID() == keyInfoPageID1);

		leafPageHeader1.writeNextLeafPageID(keyInfoPageID2);
		leafPageHeader2.writePrevLeafPageID(keyInfoPageID1);
		leafPageHeader2.writeNextLeafPageID(oldNextLeafPageID);
		leafPageHeader3.writePrevLeafPageID(keyInfoPageID2);

		if (this->m_CatchMemoryExhaust)
		{
			this->m_pPhysicalFile->detachPage(
				keyInfoPage3,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}
	}
	else
	{
		leafPageHeader1.writeNextLeafPageID(keyInfoPageID2);
		leafPageHeader2.writePrevLeafPageID(keyInfoPageID1);
	}
}

//
//	FUNCTION private
//	Btree::File::copyParentNodePageID --
//		親ノードページの物理ページ識別子をコピーする
//
//	NOTES
//	引数KeyInfoPage1_で示されるノードページのヘッダに記録されている
//	親ノードページの物理ページ識別子を読み込み、
//	引数KeyInfoPage2_で示されるノードページのヘッダに書き込む。
//
//	ARGUMENTS
//	PhysicalFile::Page*	KeyInfoPage1_
//		ノードページの物理ページ記述子
//	PhysicalFile::Page*	KeyInfoPage2_
//		ノードページの物理ページ記述子
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool			IsLeafPage_
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
File::copyParentNodePageID(PhysicalFile::Page*	KeyInfoPage1_,
						   PhysicalFile::Page*	KeyInfoPage2_,
						   PageVector&			AttachNodePages_,
						   const bool			IsLeafPage_) const
{
	; _SYDNEY_ASSERT(KeyInfoPage1_ != 0 && KeyInfoPage2_ != 0);

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   KeyInfoPage1_,
								   IsLeafPage_);

	PhysicalFile::PageID	parentNodePageID =
		nodePageHeader.readParentNodePageID();

	if (parentNodePageID == PhysicalFile::ConstValue::UndefinedPageID)
	{
		return;
	}

	PhysicalFile::Page*	nodePage = KeyInfoPage2_;

	do
	{
		nodePageHeader.resetPhysicalPage(nodePage);

		nodePageHeader.writeParentNodePageID(parentNodePageID);

		PhysicalFile::PageID	nextPhysicalPageID =
			nodePageHeader.readNextPhysicalPageID();

		if (this->m_CatchMemoryExhaust && nodePage != KeyInfoPage2_)
		{
			this->m_pPhysicalFile->detachPage(
				nodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}

		if (nextPhysicalPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			nodePage = 0;
		}
		else
		{
			nodePage = File::attachPage(this->m_pTransaction,
										this->m_pPhysicalFile,
										nextPhysicalPageID,
										this->m_FixMode,
										this->m_CatchMemoryExhaust,
										AttachNodePages_);
		}
	}
	while (nodePage != 0);
}

//
//	FUNCTION private
//	Btree::File::moveKey -- キーオブジェクトを移動する
//
//	NOTES
//	キーオブジェクトを移動する。
//
//	ARGUMENTS
//	PhysicalFile::Page*		DstTopNodePage_
//		キーオブジェクト移動先ノードページの物理ページ記述子
//	const ModUInt64			SrcDirectKeyObjectID_
//		移動するキーオブジェクトのID
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
//		移動先ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	ModUInt64
//		移動後のキーオブジェクトのID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::moveKey(PhysicalFile::Page*	DstTopNodePage_,
			  const ModUInt64		SrcDirectKeyObjectID_,
			  PageVector&			AttachNodePages_,
			  PageIDVector&			AllocateNodePageIDs_,
			  const bool			IsLeafPage_) const
{
	PhysicalFile::Page*	srcDirectKeyObjectPage = File::attachPage(
		m_pTransaction, m_pPhysicalFile,
		Common::ObjectIDData::getFormerValue(SrcDirectKeyObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);
						 
	const PhysicalFile::AreaID srcDirectKeyObjectAreaID =
		Common::ObjectIDData::getLatterValue(SrcDirectKeyObjectID_);

	const char*	srcDirectKeyObjectAreaTop =
		static_cast<const char*>(
			File::getConstAreaTop(srcDirectKeyObjectPage,
								  srcDirectKeyObjectAreaID));

	Os::Memory::Size	objectSize =
		this->m_cFileParameter.m_DirectKeyObjectSize;

	PhysicalFile::Page*	dstDirectKeyObjectPage =
		this->searchInsertNodePage(DstTopNodePage_,
								   AttachNodePages_,
								   AllocateNodePageIDs_,
								   objectSize,
								   IsLeafPage_);

	PhysicalFile::PageID	dstDirectKeyObjectPageID =
		dstDirectKeyObjectPage->getID();

	PhysicalFile::AreaID	dstDirectKeyObjectAreaID =
		dstDirectKeyObjectPage->allocateArea(*this->m_pTransaction,
											 objectSize);

	char*	dstDirectKeyObjectAreaTop =
		static_cast<char*>(File::getAreaTop(dstDirectKeyObjectPage,
											dstDirectKeyObjectAreaID));

	ModVector<ModUInt64>	variableFieldObjectIDs;

	int	i;

	if (this->m_cFileParameter.m_ExistVariableFieldInKey &&
		this->m_cFileParameter.m_ExistOutsideFieldInKey)
	{
		for (i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
		{
			if (*(this->m_cFileParameter.m_FieldOutsideArray + i) == false)
			{
				continue;
			}

			ModUInt64	dstVariableFieldObjectID =
				FileCommon::ObjectID::Undefined;

			char*	srcVariableFieldObjectIDReadPos =
				static_cast<char*>(
					this->getFieldPointer(srcDirectKeyObjectPage,
										  srcDirectKeyObjectAreaID,
										  i,
										  true)); // キーオブジェクト

			if (srcVariableFieldObjectIDReadPos != 0)
			{
				ModUInt64	srcVariableFieldObjectID;
				File::readObjectID(srcVariableFieldObjectIDReadPos,
								   srcVariableFieldObjectID);

				//
				// ※ コピー元の物理エリアは、
				// 　 File::copyVaribleKeyField内で
				// 　 解放してくれる。
				//

				dstVariableFieldObjectID =
					this->copyVariableKeyField(DstTopNodePage_,
											   srcVariableFieldObjectID,
											   AttachNodePages_,
											   AllocateNodePageIDs_,
											   IsLeafPage_,
											   true); // move
			}

			variableFieldObjectIDs.pushBack(dstVariableFieldObjectID);
		}
	}

	//
	// 配列はキーにできないので、考慮不要。
	//

	const File::ObjectType*	srcObjectType =
		syd_reinterpret_cast<const File::ObjectType*>(
			srcDirectKeyObjectAreaTop);

	File::ObjectType*	dstObjectType =
		syd_reinterpret_cast<File::ObjectType*>(dstDirectKeyObjectAreaTop);

	*dstObjectType = *srcObjectType;

	const NullBitmap::Value*	srcNullBitmapTop =
		syd_reinterpret_cast<const NullBitmap::Value*>(srcObjectType + 1);

	NullBitmap::Value*	dstNullBitmapTop =
		syd_reinterpret_cast<NullBitmap::Value*>(dstObjectType + 1);

	NullBitmap::Size	nullBitmapSize =
		NullBitmap::getSize(this->m_cFileParameter.m_KeyNum);

	ModOsDriver::Memory::copy(dstNullBitmapTop,
							  srcNullBitmapTop,
							  nullBitmapSize);

	const char*	srcKeyTop =
		static_cast<const char*>(
			NullBitmap::getConstTail(srcNullBitmapTop,
									 this->m_cFileParameter.m_KeyNum));

	char*	dstKeyTop =
		static_cast<char*>(
			NullBitmap::getTail(dstNullBitmapTop,
								this->m_cFileParameter.m_KeyNum));

	ModVector<ModUInt64>::Iterator	variableFieldObjectIDIterator =
		variableFieldObjectIDs.begin();
	ModVector<ModUInt64>::Iterator	variableFieldObjectIDsEnd =
		variableFieldObjectIDs.end();

	for (i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
	{
		if (*(this->m_cFileParameter.m_IsFixedFieldArray + i))
		{
			// fixed

			Common::DataType::Type	fieldDataType =
				*(this->m_cFileParameter.m_FieldTypeArray + i);

			Os::Memory::Size	fieldSize =
				FileCommon::DataManager::getFixedCommonDataArchiveSize(
					fieldDataType);

			ModOsDriver::Memory::copy(dstKeyTop, srcKeyTop, fieldSize);

			srcKeyTop += fieldSize;

			dstKeyTop += fieldSize;
		}
		else
		{
			// variable

			; _SYDNEY_ASSERT(
				*(this->m_cFileParameter.m_FieldTypeArray + i) ==
				Common::DataType::String);

			if (*(this->m_cFileParameter.m_FieldOutsideArray + i))
			{
				// outside

				; _SYDNEY_ASSERT(variableFieldObjectIDIterator !=
								 variableFieldObjectIDsEnd);

				File::writeObjectID(dstKeyTop,
									*variableFieldObjectIDIterator++);

				srcKeyTop += File::ObjectIDArchiveSize;

				dstKeyTop += File::ObjectIDArchiveSize;
			}
			else
			{
				// inside

				const File::InsideVarFieldLen*	srcKeyLen =
					syd_reinterpret_cast<const File::InsideVarFieldLen*>(
						srcKeyTop);

				File::InsideVarFieldLen*	dstKeyLen =
					syd_reinterpret_cast<File::InsideVarFieldLen*>(
						dstKeyTop);

				*dstKeyLen = *srcKeyLen;

				srcKeyTop =
					syd_reinterpret_cast<const char*>(srcKeyLen + 1);

				dstKeyTop = syd_reinterpret_cast<char*>(dstKeyLen + 1);

				if (*srcKeyLen > 0)
				{
					ModOsDriver::Memory::copy(dstKeyTop,
											  srcKeyTop,
											  *srcKeyLen);
				}

				srcKeyTop +=
					*(this->m_cFileParameter.m_FieldMaxLengthArray + i);

				dstKeyTop +=
					*(this->m_cFileParameter.m_FieldMaxLengthArray + i);
			}
		}

	} // end for i

	; _SYDNEY_ASSERT(variableFieldObjectIDIterator ==
					 variableFieldObjectIDsEnd);

#ifdef DEBUG

	Os::Memory::Size	areaSize =
		dstDirectKeyObjectPage->getAreaSize(dstDirectKeyObjectAreaID);

	Os::Memory::Size	writeSize =
		syd_reinterpret_cast<char*>(dstKeyTop) - dstDirectKeyObjectAreaTop;

	; _SYDNEY_ASSERT(writeSize == areaSize);

#endif

	srcDirectKeyObjectPage->freeArea(*this->m_pTransaction,
									 srcDirectKeyObjectAreaID);

	if (this->m_CatchMemoryExhaust)
	{
		this->m_pPhysicalFile->detachPage(
			srcDirectKeyObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャが
			        // キャッシュしないようにする。

		if (dstDirectKeyObjectPage != DstTopNodePage_)
		{
			this->m_pPhysicalFile->detachPage(
				dstDirectKeyObjectPage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}
	}

	return File::makeObjectID(dstDirectKeyObjectPageID,
							  dstDirectKeyObjectAreaID);
}

//
//	FUNCTION private
//	Btree::File::searchInsertNodePage --
//		キーオブジェクトを記録する物理ページを検索する
//
//	NOTES
//	ノードページを構成する物理ページの中から、
//	キーオブジェクト（外置きオブジェクトを含む）を記録するための
//	物理ページを検索する。
//	該当する物理ページが存在しない場合には、
//	新たに物理ページを確保し、ノードページに加える。
//
//	ARGUMENTS
//	PhysicalFile::Page*		TopNodePage_
//		ノードページを構成する物理ページのうちの先頭の物理ページの記述子
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const Os::Memory::Size	ObjectSize_
//		オブジェクトサイズ [byte]
//	const bool				IsLeafPage_
//		ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	PhysicalFile::Page*
//		キーオブジェクト記録先物理ページの記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::searchInsertNodePage(PhysicalFile::Page*		TopNodePage_,
						   PageVector&				AttachNodePages_,
						   PageIDVector&			AllocateNodePageIDs_,
						   const Os::Memory::Size	ObjectSize_,
						   const bool				IsLeafPage_) const
{
	; _SYDNEY_ASSERT(TopNodePage_ != 0);

	PhysicalFile::Page*	nodePage = TopNodePage_;

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   nodePage,
								   IsLeafPage_);

	while (true)
	{
		nodePageHeader.resetPhysicalPage(nodePage);

		Os::Memory::Size	freeAreaSize = nodePageHeader.getFreeAreaSize();

		if (nodePageHeader.getFreeAreaSize() > ObjectSize_)
		{
			break;
		}

		PhysicalFile::PageID	nextPhysicalPageID =
			nodePageHeader.readNextPhysicalPageID();

		if (nextPhysicalPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			PhysicalFile::PageID	newNodePageID =
				this->m_pPhysicalFile->allocatePage(*this->m_pTransaction);

			AllocateNodePageIDs_.pushBack(newNodePageID);

			PhysicalFile::Page*	newNodePage =
				File::attachPage(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 newNodePageID,
								 this->m_FixMode,
								 this->m_CatchMemoryExhaust,
								 AttachNodePages_);

			PhysicalFile::AreaID	newNodePageHeaderAreaID =
				newNodePage->allocateArea(
					*this->m_pTransaction,
					NodePageHeader::getArchiveSize(IsLeafPage_));

			NodePageHeader	newNodePageHeader(this->m_pTransaction,
											  newNodePage,
											  IsLeafPage_);

			PhysicalFile::PageID	prevLeafPageID =
				PhysicalFile::ConstValue::UndefinedPageID;
			PhysicalFile::PageID	nextLeafPageID =
				PhysicalFile::ConstValue::UndefinedPageID;

			if (IsLeafPage_)
			{
				prevLeafPageID = nodePageHeader.readPrevLeafPageID();
				nextLeafPageID = nodePageHeader.readNextLeafPageID();
			}

			newNodePageHeader.write(nodePageHeader.readParentNodePageID(),
									nodePage->getID(),
									PhysicalFile::ConstValue::UndefinedPageID,
									0,
									0,
									prevLeafPageID,
									nextLeafPageID);

			nodePageHeader.writeNextPhysicalPageID(newNodePageID);

			if (this->m_CatchMemoryExhaust && nodePage != TopNodePage_)
			{
				this->m_pPhysicalFile->detachPage(
					nodePage,
					PhysicalFile::Page::UnfixMode::Dirty,
					false); // 本当にデタッチ（アンフィックス）してしまい、
					        // 物理ファイルマネージャが
					        // キャッシュしないようにする。
			}

			nodePage = newNodePage;

			break;
		}
		else
		{
			if (nodePage != TopNodePage_)
			{
				checkMemoryExhaust(nodePage);
			}

			nodePage = File::attachPage(this->m_pTransaction,
										this->m_pPhysicalFile,
										nextPhysicalPageID,
										this->m_FixMode,
										this->m_CatchMemoryExhaust,
										AttachNodePages_);
		}
	}

	return nodePage;
}

//
//	FUNCTION private
//	Btree::File::resetNodePageHeaderParentNodePageID --
//		ノードページヘッダの親ノードページの物理ページ識別子を更新する
//
//	NOTES
//	ノードページヘッダ内に記録されている
//	「親ノードページの物理ページ識別子」を更新する。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ParentNodePageID_
//		更新後の「親ノードページの物理ページ識別子」
//	const PhysicalFile::PageID	TopChildNodePageID_
//		子ノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool					ChildNodeIsLeafPage_
//		子ノードページがリーフページかどうか
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
File::resetNodePageHeaderParentNodePageID(
	const PhysicalFile::PageID	ParentNodePageID_,
	const PhysicalFile::PageID	TopChildNodePageID_,
	PageVector&					AttachNodePages_,
	const bool					ChildNodeIsLeafPage_) const
{
	; _SYDNEY_ASSERT(
		ParentNodePageID_ != PhysicalFile::ConstValue::UndefinedPageID &&
		TopChildNodePageID_ != PhysicalFile::ConstValue::UndefinedPageID);

	PhysicalFile::Page*	childNodePage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 TopChildNodePageID_,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	NodePageHeader	childNodePageHeader(this->m_pTransaction,
										childNodePage,
										ChildNodeIsLeafPage_);

	do
	{
		childNodePageHeader.resetPhysicalPage(childNodePage);

		childNodePageHeader.writeParentNodePageID(ParentNodePageID_);

		PhysicalFile::PageID	nextPhysicalPageID =
			childNodePageHeader.readNextPhysicalPageID();

		if (this->m_CatchMemoryExhaust)
		{
			this->m_pPhysicalFile->detachPage(
				childNodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}

		if (nextPhysicalPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			childNodePage = 0;
		}
		else
		{
			childNodePage = File::attachPage(this->m_pTransaction,
											 this->m_pPhysicalFile,
											 nextPhysicalPageID,
											 this->m_FixMode,
											 this->m_CatchMemoryExhaust,
											 AttachNodePages_);
		}
	}
	while (childNodePage != 0);
}

//
//	FUNCTION private
//	Btree::File::compactionNodePage --
//		ノードページ内の物理エリアを再配置する
//
//	NOTES
//	ノードページ内の物理エリアを再配置する。
//
//	ARGUMENTS
//	PhysicalFile::Page*	TopNodePage_
//		ノードページの物理ページ記述子
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool			IsLeafPage_
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
File::compactionNodePage(PhysicalFile::Page*	TopNodePage_,
						 PageVector&			AttachNodePages_,
						 const bool				IsLeafPage_) const
{
	; _SYDNEY_ASSERT(TopNodePage_ != 0);

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   TopNodePage_,
								   IsLeafPage_);

	PhysicalFile::Page*	nodePage = TopNodePage_;

	do
	{
		nodePage->compaction(*this->m_pTransaction);

		nodePageHeader.resetPhysicalPage(nodePage);

		PhysicalFile::PageID	nextPhysicalPageID =
			nodePageHeader.readNextPhysicalPageID();

		if (this->m_CatchMemoryExhaust && nodePage != TopNodePage_)
		{
			this->m_pPhysicalFile->detachPage(
				nodePage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}

		if (nextPhysicalPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			nodePage = 0;
		}
		else
		{
			nodePage = File::attachPage(this->m_pTransaction,
										this->m_pPhysicalFile,
										nextPhysicalPageID,
										this->m_FixMode,
										this->m_CatchMemoryExhaust,
										AttachNodePages_);
		}
	}
	while (nodePage != 0);
}

//
//	FUNCTION private
//	Btree::File::searchLeafPageForInsert --
//		キー値を記録するためのリーフページを検索する
//
//	NOTES
//	キー値を記録するためのリーフページを検索する。
//	ただし、返すのは実際にキーオブジェクトを記録する
//	物理ページの記述子ではなく、そのキーオブジェクトへ辿るための
//	キー情報を記録するための物理ページの記述子である。
//
//	ARGUMENTS
//	const ModUInt32				TreeDepth_
//		現在の木の深さ
//	const PhysicalFile::PageID	RootNodePageID_
//		ルートノードページの物理ページ識別子
//	Common::DataArrayData*		Object_
//		挿入するオブジェクトへのポインタ
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
File::searchLeafPageForInsert(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	Common::DataArrayData*		Object_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::PageID	nodePageID = RootNodePageID_;

	for (ModUInt32 i = 1; i < TreeDepth_; i++)
	{
		nodePageID = this->searchChildNodePageForInsert(nodePageID,
														Object_,
														AttachNodePages_);
	}

	//
	// 木の一番深いノードページまでたどり着いたので
	// そのノードページはリーフページである。
	//

	return File::attachPage(this->m_pTransaction,
							this->m_pPhysicalFile,
							nodePageID,
							this->m_FixMode,
							this->m_CatchMemoryExhaust,
							AttachNodePages_);
}

//
//	FUNCTION private 
//	Btree::File::searchChildNodePageForInsert --
//		キー値を記録するための子ノードページを検索する
//
//	NOTES
//	キー値を記録するための子ノードページを検索する。
//	ただし、返すのは実際にキーオブジェクトを記録する
//	物理ページの識別子ではなく、そのキーオブジェクトへ辿るための
//	キー情報を記録するための物理ページの識別子である。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ParentNodePageID_
//		親ノードページの物理ページ識別子
//	Common::DataArrayData*		Object_
//		挿入するオブジェクトへのポインタ
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	PhysicalFile::PageID
//		子ノードページの物理ページ識別子
//
//	EXCEPTIONS
//	
//
PhysicalFile::PageID
File::searchChildNodePageForInsert(
	const PhysicalFile::PageID	ParentNodePageID_,
	Common::DataArrayData*		Object_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	parentNodePage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 ParentNodePageID_,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	//
	// キーテーブルが1物理ページに収まる場合に、
	// 本関数が呼び出されるのだから、
	// 引数ParentNodePageID_が示すノード（物理）ページ内に
	// 記録されているキーテーブル内のキー情報を参照すればよい。
	//

	NodePageHeader	parentNodePageHeader(this->m_pTransaction,
										 parentNodePage,
										 false); // リーフページではない

	int	keyInfoIndex =
		this->getKeyInformationIndexForInsert(
			parentNodePage,
			parentNodePageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			Object_,
			false, // リーフページではない
			true); // 子ノードページを探す

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false); // リーフページではない

	PhysicalFile::PageID	childNodePageID =
		keyInfo.readChildNodePageID();

	checkMemoryExhaust(parentNodePage);

	return childNodePageID;
}

//
//	FUNCTION private
//	Btree::File::getKeyInformationIndexForInsert --
//		挿入するオブジェクトのキーフィールドの値に最も近い
//		値が記録されているキーオブジェクトへ辿るキー情報の
//		インデックスを返す
//
//	NOTES
//	引数KeyInfoPage_が示すノードページ内で、
//	挿入するオブジェクトのキーフィールドの値に最も近い
//	値が記録されているキーオブジェクトを検索し、
//	そのキーオブジェクトへ辿るキー情報のインデックスを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*		KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32			UseKeyInfoNum_
//		引数KeyInfoPage_が示すノードページ内の使用中のキー情報数
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Common::DataArrayData*	Object_
//		挿入オブジェクトへのポインタ
//	const bool				IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	const bool				SearchChildNodePage_
//		子ノードページを検索するために呼び出されたかどうか
//			true  : 子ノードページを検索するために呼び出された
//			false : 引数KeyInfoPage_が示すノードページの
//			        どこにキー情報を追加するかを特定するために
//			        呼び出された
//	PhysicalFile::Page*		PrevKeyInfoChildNodePage_ = 0
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
File::getKeyInformationIndexForInsert(
	PhysicalFile::Page*		KeyInfoPage_,
	const ModUInt32			UseKeyInfoNum_,
	PageVector&				AttachNodePages_,
	Common::DataArrayData*	Object_,
	const bool				IsLeafPage_,
	const bool				SearchChildNodePage_,
	PhysicalFile::Page*		PrevKeyInfoChildNodePage_ // = 0
	) const
{
	if (PrevKeyInfoChildNodePage_ != 0)
	{
		PhysicalFile::Page*	parentNodePage = 0;
		ModUInt32			prevKeyInfoIndex = ModUInt32Max;

		this->searchKeyInformation(PrevKeyInfoChildNodePage_,
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
							IsLeafPage_);

	int	midKeyInfoIndex = 0;
	int	firstKeyInfoIndex = 0;
	int	lastKeyInfoIndex = UseKeyInfoNum_ - 1;

	while (firstKeyInfoIndex <= lastKeyInfoIndex)
	{
		midKeyInfoIndex = (firstKeyInfoIndex + lastKeyInfoIndex) >> 1;

		keyInfo.setStartOffsetByIndex(midKeyInfoIndex);

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		int	compareResult =
			this->compareToFetchCondition(KeyInfoPage_,
										  AttachNodePages_,
										  keyObjectID,
										  Object_,
										  1); // Object_には、
										      // オブジェクトIDも
										      // ついているので。

		if (compareResult < 0)
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
//	Btree::File::setObjectID --
//		リーフページのキー情報にオブジェクトIDを書き込む
//
//	NOTES
//	リーフページのキー情報に、
//	バリューオブジェクトのオブジェクトIDを書き込む。
//
//	ARGUMENTS
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
File::setObjectID(PageVector&	AttachNodePages_) const
{
	; _SYDNEY_ASSERT(
		this->m_ullObjectID != FileCommon::ObjectID::Undefined);

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(
		this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true,  // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	keyInfo.writeValueObjectID(this->m_ullObjectID);

	if (this->m_CatchMemoryExhaust)
	{
		this->m_pPhysicalFile->detachPage(
			leafPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャが
			        // キャッシュしないようにする。
	}
}

//
//	FUNCTION private
//	Btree::File::setObjectID --
//		オブジェクトの先頭フィールドにオブジェクトIDを設定する
//
//	NOTES
//	オブジェクトの先頭フィールドに
//	バリューオブジェクトのオブジェクトIDを設定する。
//	※ 先頭フィールドにLogicalFile::ObjectIDをnewするのではなく、
//	　 オブジェクトIDをsetValueするだけである。
//
//	ARGUMENTS
//	Common::DataArrayData*	Object_
//		オブジェクトへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
File::setObjectID(Common::DataArrayData*	Object_) const
{
	; _SYDNEY_ASSERT(
		this->m_ullObjectID != FileCommon::ObjectID::Undefined);

	Common::Data*	firstField = Object_->getElement(0).get();
	LogicalFile::ObjectID*	objectIDField;

	if (firstField->isNull()) {
		// NullDataが入っていたらObjectIdにする
		objectIDField = new LogicalFile::ObjectID();
		Object_->setElement(0, objectIDField);

	} else {
		; _SYDNEY_ASSERT(
			 firstField->getType() == LogicalFile::ObjectID().getType());

		objectIDField = _SYDNEY_DYNAMIC_CAST(LogicalFile::ObjectID*, firstField);
		; _SYDNEY_ASSERT(objectIDField != 0);
	}

	objectIDField->setValue(m_ullObjectID);
}

//
//	Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
