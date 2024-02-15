// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_Update.cpp --
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

#include "LogicalFile/ObjectID.h"

#include "Btree/NodePageHeader.h"
#include "Btree/KeyInformation.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
// PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::File::updateCheck --
//		オブジェクトを更新するための環境が整っているかをチェックする
//
//	NOTES
//	オブジェクトを更新するための環境が整っているかをチェックする
//
//	ARGUMENTS
//	const Common::DataArrayData*	SearchCondition_
//		更新対象オブジェクトへのポインタ（検索条件）
//	Common::DataArrayData*			Object_
//		更新後のオブジェクトへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		ファイルがオープンされていない
//	IllegalFileAccess
//		オブジェクト更新のためにオープンされていない
//	BadArgument
//		検索条件または更新後のオブジェクトが不正
//
void
File::updateCheck(const Common::DataArrayData*	SearchCondition_,
				  Common::DataArrayData*		Object_) const
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

	if (SearchCondition_ == 0 || Object_ == 0)
	{
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
}

//
//	FUNCTION private
//	Btree::File::isKeyUpdate --
//		キーオブジェクトの更新を伴う更新処理かをチェックする
//
//	NOTES
//	オブジェクトの更新により、
//	キーオブジェクトの更新も必要かどうかをチェックする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		キーオブジェクトの更新が必要かどうか
//			true  : キーオブジェクトの更新が必要
//			false : キーオブジェクトの更新が不要
//
//	EXCEPTIONS
//	なし
//
bool
File::isKeyUpdate() const
{
	//
	// 特定のフィールドを選択していないのならば
	// オブジェクト（すべてのフィールド）の更新なので
	// キーフィールドも更新される。（絶対とはいえないが...）
	//

	if (this->m_pOpenParameter->m_bFieldSelect == false)
	{
		return true;
	}

	//
	// 選択された特定のフィールドにキーフィールドが含まれているか
	// チェックする
	//

	; _SYDNEY_ASSERT(this->m_pOpenParameter->m_TargetFieldNum > 0);

	for (int i = 0; i < this->m_pOpenParameter->m_TargetFieldNum; i++)
	{
		int	targetFieldIndex =
			*(this->m_pOpenParameter->m_TargetFieldIndexArray + i);

		if (targetFieldIndex < this->m_cFileParameter.m_TopValueFieldIndex)
		{
			return true;
		}
	}

	return false;
}

//
//	FUNCTION private
//	Btree::File::isNecessaryMoveKey --
//		リーフページのキーの位置が変更される更新処理かをチェックする
//
//	NOTES
//	更新処理により、リーフページに記録されているキーの位置が
//	変わるかどうかをチェックする。
//	“リーフページに記録されているキーの位置が変わる”とは…
//		キーテーブル内のキー情報は、キー値順にソートされている
//		（キー値順にキーオブジェクトを参照可能なようにソートされている）。
//		また、ファイル内に存在するすべてのリーフページは、
//		これもキー値順にキーオブジェクトを参照可能なように
//		リンクされている。
//		更新処理により、キー情報の位置が、
//			① キーテーブル内で変わる。
//			② 他のリーフページに移動する。
//		①、②いずれかに該当することをこう表現してみました。
//
//	ARGUMENTS
//	Common::DataArrayData*	AfterObject_
//		更新後のオブジェクトへのポインタ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	bool
//		リーフページのキーの位置が変わるかどうか
//			true  : キーの位置が変わる
//			false : キーの位置はそのままで、値だけが更新される
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::isNecessaryMoveKey(
	Common::DataArrayData*	AfterObject_,
	PageVector&				AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->isNecessaryMoveSimpleKey(AfterObject_, AttachNodePages_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);
	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	targetLeafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	NodePageHeader	targetLeafPageHeader(this->m_pTransaction,
										 targetLeafPage,
										 true); // リーフページ

	ModUInt32	useKeyInfoNum =
		targetLeafPageHeader.readUseKeyInformationNumber();

	if (useKeyInfoNum == 1)
	{
		// リーフページ内の使用中のキー情報が1つ…

		PhysicalFile::PageID	prevLeafPageID =
			targetLeafPageHeader.readPrevLeafPageID();

		PhysicalFile::PageID	nextLeafPageID =
			targetLeafPageHeader.readNextLeafPageID();

		checkMemoryExhaust(targetLeafPage);

		if (prevLeafPageID != PhysicalFile::ConstValue::UndefinedPageID ||
			nextLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 前後いずれか（または両方）にリーフページが存在する…

			//
			// ということは、親ノードが存在することになり、
			// リーフページ内にキー情報が1つしかないのであれば、
			// そのキーは親ノードにも記録されていて、
			// 親ノードのキーの書き換えも必要となるため、
			// 無条件でキーが動くと解釈する。
			//

			return true;
		}
		else
		{
			// 前後ともリーフページが存在しない…

			//
			// ということは、ファイル内にオブジェクトが1つしか存在ない。
			// なので、キーは前後に動きようがない。
			//

			return false;
		}
	}

	bool	attachedPrevLeafPage = false;

	PhysicalFile::Page*	prevLeafPage = 0;
	PhysicalFile::Page*	nextLeafPage = 0;

	ModUInt32	prevKeyInfoIndex = ModUInt32Max;
	ModUInt32	nextKeyInfoIndex = ModUInt32Max;

	// リーフページ内の使用中のキー情報が複数のはず。
	// （0は有り得ないはず。）
	; _SYDNEY_ASSERT(useKeyInfoNum > 1);

	ModUInt32	lastKeyInfoIndex = useKeyInfoNum - 1;

	if (this->m_KeyInfoIndex == 0)
	{
		// キー情報が、リーフページ内の先頭キー情報…

		PhysicalFile::PageID	prevLeafPageID =
			targetLeafPageHeader.readPrevLeafPageID();

		if (prevLeafPageID !=
			PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 前にリーフページが存在する…

			//
			// 前のリーフページをアタッチする
			//

			prevLeafPage = File::attachPage(this->m_pTransaction,
											this->m_pPhysicalFile,
											prevLeafPageID,
											this->m_FixMode,
											this->m_CatchMemoryExhaust,
											AttachNodePages_);
			attachedPrevLeafPage = true;

			NodePageHeader	prevLeafPageHeader(this->m_pTransaction,
											   prevLeafPage,
											   true); // リーフページ

			ModUInt32	prevUseKeyInfoNum =
				prevLeafPageHeader.readUseKeyInformationNumber();

			; _SYDNEY_ASSERT(prevUseKeyInfoNum > 0);

			prevKeyInfoIndex = prevUseKeyInfoNum - 1;
		}

		nextLeafPage = targetLeafPage;

		nextKeyInfoIndex = this->m_KeyInfoIndex + 1;
	}
	else if (this->m_KeyInfoIndex == lastKeyInfoIndex)
	{
		// キー情報が、リーフページ内の最終キー情報…

		if (targetLeafPageHeader.readParentNodePageID() !=
			PhysicalFile::ConstValue::UndefinedPageID)
		{
			// リーフページに親ノードページが存在する…

			//
			// ということは、そのキーは親ノードにも記録されていて
			// 親ノードのキーの書き換えも必要となるため、
			// 無条件でキーが動くと解釈する。
			//

			checkMemoryExhaust(targetLeafPage);

			return true;
		}

		prevLeafPage = targetLeafPage;

		prevKeyInfoIndex = this->m_KeyInfoIndex - 1;
	}
	else
	{
		// キー情報が、リーフページ内の
		// 先頭キー情報でも最終キー情報でもない…

		prevLeafPage = targetLeafPage;
		nextLeafPage = targetLeafPage;

		prevKeyInfoIndex = this->m_KeyInfoIndex - 1;
		nextKeyInfoIndex = this->m_KeyInfoIndex + 1;
	}

	// 必ずキー値順に前後いずれかのキーと比較するはず。
	; _SYDNEY_ASSERT(prevLeafPage != 0 || nextLeafPage != 0);
	; _SYDNEY_ASSERT(prevKeyInfoIndex != ModUInt32Max ||
					 nextKeyInfoIndex != ModUInt32Max);

	bool	necessary = false;

	if (prevLeafPage != 0)
	{
		// キー値順に前にキーがある…

		//
		// 直前のキーと比較する。
		//

		KeyInformation	prevKeyInfo(this->m_pTransaction,
									prevLeafPage,
									prevKeyInfoIndex,
									true); // リーフページ

		ModUInt64	prevKeyObjectID = prevKeyInfo.readKeyObjectID();

		int	compareResult =
			this->compareToFetchCondition(
				prevLeafPage,
				AttachNodePages_,
				prevKeyObjectID,
				AfterObject_,
				1); // AfterObject_に
					// オブジェクトIDフィールドがないので。

		necessary = (compareResult < 0);
	}

	if (necessary == false && nextLeafPage != 0)
	{
		// キー値順に後ろにキーがある…

		//
		// 直後のキーと比較する。
		//

		KeyInformation	nextKeyInfo(this->m_pTransaction,
									nextLeafPage,
									nextKeyInfoIndex,
									true); // リーフページ

		ModUInt64	nextKeyObjectID = nextKeyInfo.readKeyObjectID();

		int	compareResult =
			this->compareToFetchCondition(
				nextLeafPage,
				AttachNodePages_,
				nextKeyObjectID,
				AfterObject_,
				1); // AfterObject_に
				    // オブジェクトIDフィールドがないので。

		necessary = (compareResult > 0);
	}

	if (attachedPrevLeafPage)
	{
		; _SYDNEY_ASSERT((!this->m_CatchMemoryExhaust)||(prevLeafPage != targetLeafPage));

		checkMemoryExhaust(prevLeafPage);
	}
	checkMemoryExhaust(targetLeafPage);

	return necessary;
}

//
//	FUNCTION private
//	Btree::File::isValueUpdate --
//		バリューオブジェクトの更新を伴う更新処理かをチェックする
//
//	NOTES
//	バリューオブジェクトの更新を伴う更新処理かをチェックする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		バリューオブジェクトの更新を伴う更新処理かどうか
//			true  : バリューオブジェクトの更新を伴う
//			false : キーオブジェクトの更新のみである
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::isValueUpdate() const
{
	if (this->m_pOpenParameter->m_bFieldSelect == false)
	{
		return true;
	}

	; _SYDNEY_ASSERT(this->m_pOpenParameter->m_TargetFieldNum > 0);

	for (int i = 0; i < this->m_pOpenParameter->m_TargetFieldNum; i++)
	{
		int	targetFieldIndex =
			*(this->m_pOpenParameter->m_TargetFieldIndexArray + i);

		if (targetFieldIndex >= this->m_cFileParameter.m_TopValueFieldIndex)
		{
			return true;
		}
	}

	return false;
}

//
//	FUNCTION private
//	Btree::File::makeUpdateObject --
//		更新後のオブジェクトを生成する
//
//	NOTES
//	更新後のオブジェクトを生成する。
//
//	ARGUMENTS
//	Common::DataArrayData*	AfterObject_
//		更新後のオブジェクトへのポインタ
//		（更新対象フィールドのみ要素として格納されている）
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	Common::DataArrayData*
//		更新後のオブジェクトへのポインタ
//		（非更新対象フィールドも要素として格納されている）
//
//	EXCEPTIONS
//	[YET!]
//
Common::DataArrayData*
File::makeUpdateObject(Common::DataArrayData*	AfterObject_,
					   PageVector&				AttachNodePages_,
					   ValueFile*				ValueFile_,
					   PageVector&				AttachValuePages_) const
{
	; _SYDNEY_ASSERT(AfterObject_ != 0);

	if (this->m_pOpenParameter->m_bFieldSelect == false)
	{
		// オブジェクト全体を更新…

		// Common::Data::copyの仕様が変わったので修正
		// ついでにここではcopyではなく要素の転写をするようにした
		ModAutoPointer<Common::DataArrayData> updateObject =
			new Common::DataArrayData();
		int n = AfterObject_->getCount();
		updateObject->reserve(n + 1);

		updateObject->pushBack(new LogicalFile::ObjectID());
		for (int i = 0; i < n; ++i) {
			updateObject->pushBack(AfterObject_->getElement(i));
		}
		return updateObject.release();
	}

	Common::DataArrayData*	pObject = 0;
	this->getObject(this->m_ullObjectID,
					ValueFile_,
					pObject,//[out]
					false, // オブジェクト全体を読み込む
					AttachNodePages_,
					AttachValuePages_);

	; _SYDNEY_ASSERT(pObject != 0);

	ModAutoPointer<Common::DataArrayData> beforeObject = pObject;
	ModAutoPointer<Common::DataArrayData> updateObject = new Common::DataArrayData();

	updateObject->reserve(this->m_cFileParameter.m_FieldNum);

	updateObject->pushBack(new LogicalFile::ObjectID());

	int	afterObjectFieldIndex = 0;

	for (int i = 1; i < this->m_cFileParameter.m_FieldNum; i++)
	{
		Common::Data*	updateField = 0;

		if (File::isSelected(this->m_pOpenParameter, i))
		{
			updateField =
				AfterObject_->getElement(afterObjectFieldIndex++).get();
		}
		else
		{
			updateField =
				beforeObject->getElement(i).get();
		}

		updateObject->pushBack(updateField->copy());
	}

	return updateObject.release();
}

//
//	FUNCTION private
//	Btree::File::isSelected -- 選択フィールドかどうかをチェックする
//
//	NOTES
//	引数FieldIndex_で示されるフィールドが、
//	オブジェクト更新やプロジェクションで
//	選択されているフィールドかどうかをチェックする。
//
//	ARGUMENTS
//	const Btree::OpenParameter*	OpenParam_
//		オープンパラメータへのポインタ
//	const int					FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	bool
//		選択フィールドかどうか
//			true  : 選択フィールドである
//			false : 選択フィールドではない
//
//	EXCEPTIONS
//	なし
//
// static
bool
File::isSelected(const OpenParameter*	OpenParam_,
				 const int				FieldIndex_)
{
	if (OpenParam_->m_bFieldSelect == false)
	{
		return true;
	}

	; _SYDNEY_ASSERT(OpenParam_->m_TargetFieldNum > 0);

	for (int i = 0; i < OpenParam_->m_TargetFieldNum; i++)
	{
		if (*(OpenParam_->m_TargetFieldIndexArray + i) == FieldIndex_)
		{
			return true;
		}
	}

	return false;
}

//
//	FUNCTION private
//	Btree::File::updateKey -- キーオブジェクトを更新する
//
//	NOTES
//	キーオブジェクトを更新する。
//
//	ARGUMENTS
//	Common::DataArrayData*	Object_
//		更新後のオブジェクトへのポインタ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノード−ページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノード−ページの物理ページ識別子をつむ）
//	Btree::PageIDVector&	FreeNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（フリーしたノード−ページの物理ページ識別子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::updateKey(Common::DataArrayData*	Object_,
				PageVector&				AttachNodePages_,
				PageIDVector&			AllocateNodePageIDs_,
				PageIDVector&			FreeNodePageIDs_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		this->updateSimpleKey(Object_, AttachNodePages_);
	}
	else
	{
		this->updateKeyObject(Object_,
							  AttachNodePages_,
							  AllocateNodePageIDs_,
							  FreeNodePageIDs_);
	}
}

//
//	FUNCTION private
//	Btree::File::updateKeyObject --
//		キーオブジェクトを更新する
//
//	NOTES
//	オブジェクトタイプがノーマルオブジェクトの
//	キーオブジェクトを更新する。
//
//	ARGUMENTS
//	Common::DataArrayData*	Object_
//		更新後のオブジェクトへのポインタ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	Btree::PageIDVector&	FreeNodePageIDs_
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
File::updateKeyObject(Common::DataArrayData*	Object_,
					  PageVector&				AttachNodePages_,
					  PageIDVector&				AllocateNodePageIDs_,
					  PageIDVector&				FreeNodePageIDs_) const
{
	; _SYDNEY_ASSERT(this->m_LeafPageID !=
					 PhysicalFile::ConstValue::UndefinedPageID);
	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

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
							true); // リーフページ

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	PhysicalFile::Page*		keyObjectPage = 0;

	bool	attached =
		File::attachObjectPage(this->m_pTransaction,
							   keyObjectID,
							   leafPage,
							   keyObjectPage,
							   this->m_FixMode,
							   this->m_CatchMemoryExhaust,
							   AttachNodePages_);

	const PhysicalFile::AreaID keyObjectAreaID =
		Common::ObjectIDData::getLatterValue(keyObjectID);

	char*	keyObjectAreaTop =
		static_cast<char*>(File::getAreaTop(keyObjectPage,
											keyObjectAreaID));

	int	i;

	ModVector<ModUInt64>	variableFieldObjectIDs;

	if (this->m_cFileParameter.m_ExistOutsideFieldInKey)
	{
		for (i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
		{
			if (File::isSelected(this->m_pOpenParameter, i) &&
				*(this->m_cFileParameter.m_FieldOutsideArray + i))
			{
				char*	objectIDReadPos =
					static_cast<char*>(
						this->getFieldPointer(keyObjectPage,
											  keyObjectAreaID,
											  i,
											  true)); // キーオブジェクト

				if (objectIDReadPos != 0)
				{
					ModUInt64	objectID;
					File::readObjectID(objectIDReadPos, objectID);

					File::freeVariableFieldObjectArea(
						this->m_pTransaction,
						objectID,
						keyObjectPage,
						this->m_FixMode,
						this->m_CatchMemoryExhaust,
						AttachNodePages_,
						FreeNodePageIDs_);
				}

				Common::Data*	variableField =
					Object_->getElement(i).get();

				; _SYDNEY_ASSERT(variableField != 0);

				if (!variableField->isNull())
				{
					ModUInt64	variableFieldObjectID =
						this->writeOutsideVariableKey(leafPage,
													  variableField,
													  AttachNodePages_,
													  AllocateNodePageIDs_,
													  true); // リーフページ

					variableFieldObjectIDs.pushBack(
						variableFieldObjectID);
				}
			}
		}
	}

	ModVector<ModUInt64>::Iterator	variableFieldObjectIDIterator =
		variableFieldObjectIDs.begin();
	ModVector<ModUInt64>::Iterator	variableFieldObjectIDsEnd =
		variableFieldObjectIDs.end();

	File::ObjectType*	objectType =
		syd_reinterpret_cast<File::ObjectType*>(keyObjectAreaTop);

	; _SYDNEY_ASSERT(*objectType == File::NormalObjectType);

	NullBitmap::Value*	nullBitmapTop =
		syd_reinterpret_cast<NullBitmap::Value*>(objectType + 1);

	NullBitmap	nullBitmap(nullBitmapTop,
						   this->m_cFileParameter.m_KeyNum,
						   NullBitmap::Access::ReadWrite);

	char*	keyWritePos =
		static_cast<char*>(nullBitmap.getTail());

	for (i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
	{
		if (File::isSelected(this->m_pOpenParameter, i))
		{
			// 更新対象キーフィールド…

			Common::Data*	keyField = Object_->getElement(i).get();

			// まずは必ずヌルビットをOFFしておく。
			nullBitmap.off(i - 1);

			if (keyField->isNull())
			{
				// キー値としてヌル値が設定されていた…

				keyWritePos = this->writeNullKey(nullBitmapTop,
												 keyWritePos,
												 i);
			}
			else if (*(this->m_cFileParameter.m_FieldOutsideArray + i))
			{
				// 外置き可変長フィールド…

				; _SYDNEY_ASSERT(variableFieldObjectIDIterator !=
								 variableFieldObjectIDsEnd);

				keyWritePos =
					File::writeObjectID(keyWritePos,
										*variableFieldObjectIDIterator);

				variableFieldObjectIDIterator++;
			}
			else if (*(this->m_cFileParameter.m_IsFixedFieldArray + i))
			{
				// 固定長フィールド…

				keyWritePos = File::writeFixedField(keyWritePos, keyField);
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
		else
		{
			// 更新対象ではないキーフィールド…

			keyWritePos += this->m_cFileParameter.getFieldArchiveSize(i);
		}
	}

	; _SYDNEY_ASSERT(variableFieldObjectIDIterator ==
					 variableFieldObjectIDsEnd);

#ifdef DEBUG

	Os::Memory::Size	areaSize =
		keyObjectPage->getAreaSize(keyObjectAreaID);

	Os::Memory::Size	writeSize =
		syd_reinterpret_cast<char*>(keyWritePos) - keyObjectAreaTop;

	; _SYDNEY_ASSERT(writeSize == areaSize);

#endif

	if (this->m_CatchMemoryExhaust)
	{
		if (attached)
		{
			this->m_pPhysicalFile->detachPage(
				keyObjectPage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまう
		}

		this->m_pPhysicalFile->detachPage(
			leafPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまう
	}
}

//
//	Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
