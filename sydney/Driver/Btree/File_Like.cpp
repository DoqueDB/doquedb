// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_Like.cpp --
// 
// Copyright (c) 2001, 2002, 2004, 2009, 2023 Ricoh Company, Ltd.
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

#include "Exception/NotSupported.h"

#include "Common/Assert.h"

#include "Utility/CharTrait.h"

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
//	Btree::File::likeSearch -- like演算子による文字列検索を行う
//
//	NOTES
//	like演算子による文字列検索を行い、
//	該当するオブジェクトが存在すれば、
//	そのオブジェクトのIDを返す。
//	該当するオブジェクトが存在しなければ、
//	FileCommon::ObjectID::Undefinedを返す。
//	オープン時に、オブジェクトの挿入ソート順で
//	オブジェクトを取得するように設定されている場合に、
//	呼び出される。
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
//	ModUInt64
//		検索条件と一致するオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::likeSearch(const ModUInt32			TreeDepth_,
				 const PhysicalFile::PageID	RootNodePageID_,
				 PageVector&				AttachNodePages_) const
{
	PhysicalFile::Page*	leafPage =
		this->searchLeafPageForLike(TreeDepth_,
									RootNodePageID_,
									AttachNodePages_);

	if (leafPage == 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	bool	match = false;

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	int	keyInfoIndex =
		this->getKeyInformationIndexForLike(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			true, // リーフページ
			match);

	if (match == false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true); // リーフページ

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	ModUInt64	valueObjectID = keyInfo.readValueObjectID();

	PhysicalFile::Page*		keyObjectPage = 0;

	bool	attached =
		File::attachObjectPage(this->m_pTransaction,
							   keyObjectID,
							   leafPage,
							   keyObjectPage,
							   this->m_FixMode,
							   this->m_CatchMemoryExhaust,
							   AttachNodePages_);

	PhysicalFile::Page*	outsideObjectPage = 0;
	ModSize	numChar = 0;
	bool	isDivide;
	bool	isCompressed;

	ModUnicodeChar*	fieldValue = getTopStringFieldPointer(
		keyObjectPage, outsideObjectPage, AttachNodePages_,
		Common::ObjectIDData::getLatterValue(keyObjectID),
		numChar,
		isDivide,
		isCompressed);

	ModUnicodeChar	forEmpty = 0;
	if (numChar == 0)
	{
		fieldValue = &forEmpty;
	}

	; _SYDNEY_ASSERT(fieldValue != 0);

	const ModUnicodeChar*	patternHead =
		(const ModUnicodeChar*)this->m_SearchHint.m_PatternString;
	ModSize	patternNumChar =
		this->m_SearchHint.m_PatternString.getLength();

	bool	compareResult = false;

	if (isDivide || isCompressed)
	{
		// divide or compressed...

		char*	objectIDReadPos = static_cast<char*>(
			getFieldPointer(
				keyObjectPage,
				Common::ObjectIDData::getLatterValue(keyObjectID),
				1,      // field index
				true)); // key object

		ModUInt64	objectID;
		File::readObjectID(objectIDReadPos, objectID);

		ModUnicodeString	unicodeFieldValue;

		if (isDivide)
		{
			// divide...

			File::readOutsideStringField(this->m_pTransaction,
										 keyObjectPage,
										 objectID,
										 unicodeFieldValue,
										 this->m_FixMode,
										 this->m_CatchMemoryExhaust,
										 AttachNodePages_);
		}
		else
		{
			// compressed...

			Common::Data::Pointer	fieldValue;

			File::readOutsideVariableField(this->m_pTransaction,
										   keyObjectPage,
										   objectID,
										   Common::DataType::String,
										   fieldValue,
										   this->m_FixMode,
										   this->m_CatchMemoryExhaust,
										   AttachNodePages_);

			Common::StringData*	stringFieldValue =
				_SYDNEY_DYNAMIC_CAST(Common::StringData*, fieldValue.get());

			unicodeFieldValue = stringFieldValue->getString();
		}

		numChar = unicodeFieldValue.getLength();

		if (this->m_pOpenParameter->m_cSearchCondition.m_SetEscape)
		{
			compareResult =
				Utility::CharTrait::like(
					(const ModUnicodeChar*)unicodeFieldValue,
					numChar,
					patternHead,
					patternNumChar,
					this->m_pOpenParameter->m_cSearchCondition.m_Escape);
		}
		else
		{
			compareResult =
				Utility::CharTrait::like(
					(const ModUnicodeChar*)unicodeFieldValue,
					numChar,
					patternHead,
					patternNumChar);
		}
	}
	else
	{
		// normal...

		if (this->m_pOpenParameter->m_cSearchCondition.m_SetEscape)
		{
			compareResult =
				Utility::CharTrait::like(
					fieldValue,
					numChar,
					patternHead,
					patternNumChar,
					this->m_pOpenParameter->m_cSearchCondition.m_Escape);
		}
		else
		{
			compareResult = Utility::CharTrait::like(fieldValue,
														  numChar,
														  patternHead,
														  patternNumChar);
		}
	}

	if (compareResult == false)
	{
		checkMemoryExhaust(leafPage);
		if (attached)
		{
			checkMemoryExhaust(keyObjectPage);
		}
		if (this->m_CatchMemoryExhaust)	// MemoryExhaust ならデタッチ（アンフィックス）されている
		{
			leafPage = 0;
			keyObjectPage = 0;
		}

		PhysicalFile::PageID	saveLeafPageID = this->m_LeafPageID;
		ModUInt32				saveKeyInfoIndex = this->m_KeyInfoIndex;

		this->m_LeafPageID = leafPageID;
		this->m_KeyInfoIndex = keyInfoIndex;

		valueObjectID = this->getNextObjectIDByLike(AttachNodePages_);

		this->m_LeafPageID = saveLeafPageID;
		this->m_KeyInfoIndex = saveKeyInfoIndex;
	}

	checkMemoryExhaust(leafPage);

	if (attached)
	{
		checkMemoryExhaust(keyObjectPage);
	}

	return valueObjectID;
}

//
//	FUNCTION private
//	Btree::File::likeSearchRev -- like演算子による文字列検索を行う
//
//	NOTES
//	like演算子による文字列検索を行い、
//	該当するオブジェクトが存在すれば、
//	そのオブジェクトのIDを返す。
//	該当するオブジェクトが存在しなければ、
//	FileCommon::ObjectID::Undefinedを返す。
//	オープン時に、オブジェクトの挿入ソート順の逆順で
//	オブジェクトを取得するように設定されている場合に、
//	呼び出される。
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
//	ModUInt64
//		検索条件と一致するオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::likeSearchRev(const ModUInt32				TreeDepth_,
					const PhysicalFile::PageID	RootNodePageID_,
					PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	leafPage =
		this->searchLeafPageForLike(TreeDepth_,
									RootNodePageID_,
									AttachNodePages_);

	if (leafPage == 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	bool	match = false;

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	int	keyInfoIndex =
		this->getKeyInformationIndexForLike(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			true, // リーフページ
			match);

	if (match == false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true); // リーフページ

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	ModUInt64	valueObjectID = keyInfo.readValueObjectID();

	bool	lp = true;
	do
	{
		int	saveKeyInfoIndex = keyInfo.getIndex();

		if (this->assignNextKeyInformation(leafPage,
										   AttachNodePages_,
										   leafPageHeader,
										   keyInfo)
			== false)
		{
			if (leafPage == 0 || leafPageID != leafPage->getID())
			{
				checkMemoryExhaust(leafPage);

				leafPage = File::attachPage(this->m_pTransaction,
											this->m_pPhysicalFile,
											leafPageID,
											this->m_FixMode,
											this->m_CatchMemoryExhaust,
											AttachNodePages_);
			}

			break;
		}

		PhysicalFile::PageID	saveLeafPageID = leafPageID;

		if (leafPageID != leafPage->getID())
		{
			leafPageID = leafPage->getID();
		}

		ModUInt64	nextKeyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(
			nextKeyObjectID != FileCommon::ObjectID::Undefined &&
			nextKeyObjectID != 0);

		if (this->compareToLikeSearchCondition(leafPage,
											   AttachNodePages_,
											   nextKeyObjectID)
			!= 0)
		{
			lp = false;

			keyInfoIndex = saveKeyInfoIndex;

			if (saveLeafPageID != leafPageID)
			{
				checkMemoryExhaust(leafPage);

				leafPageID = saveLeafPageID;

				leafPage = File::attachPage(this->m_pTransaction,
											this->m_pPhysicalFile,
											leafPageID,
											this->m_FixMode,
											this->m_CatchMemoryExhaust,
											AttachNodePages_);
			}
		}
		else
		{
			keyObjectID = nextKeyObjectID;
			valueObjectID = keyInfo.readValueObjectID();
		}
	}
	while (lp);

	; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
					 keyObjectID != 0);

	; _SYDNEY_ASSERT(valueObjectID != FileCommon::ObjectID::Undefined);

	PhysicalFile::Page*		keyObjectPage = 0;

	bool	attached =
		File::attachObjectPage(this->m_pTransaction,
							   keyObjectID,
							   leafPage,
							   keyObjectPage,
							   this->m_FixMode,
							   this->m_CatchMemoryExhaust,
							   AttachNodePages_);

	PhysicalFile::Page*	outsideObjectPage = 0;
	ModSize	numChar = 0;
	bool	isDivide;
	bool	isCompressed;

	ModUnicodeChar*	fieldValue = getTopStringFieldPointer(
		keyObjectPage, outsideObjectPage, AttachNodePages_,
		Common::ObjectIDData::getLatterValue(keyObjectID),
		numChar,
		isDivide,
		isCompressed);

	ModUnicodeChar	forEmpty = 0;
	if (numChar == 0)
	{
		fieldValue = &forEmpty;
	}

	; _SYDNEY_ASSERT(fieldValue != 0);

	const ModUnicodeChar*	patternHead =
		(const ModUnicodeChar*)this->m_SearchHint.m_PatternString;
	ModSize	patternNumChar =
		this->m_SearchHint.m_PatternString.getLength();

	bool	compareResult = false;

	if (isDivide || isCompressed)
	{
		// divide or compressed...

		char*	objectIDReadPos = static_cast<char*>(
			getFieldPointer(
				keyObjectPage,
				Common::ObjectIDData::getLatterValue(keyObjectID),
				1,      // field index
				true)); // key object

		ModUInt64	objectID;
		File::readObjectID(objectIDReadPos, objectID);

		ModUnicodeString	unicodeFieldValue;

		if (isDivide)
		{
			// divide...

			File::readOutsideStringField(this->m_pTransaction,
										 keyObjectPage,
										 objectID,
										 unicodeFieldValue,
										 this->m_FixMode,
										 this->m_CatchMemoryExhaust,
										 AttachNodePages_);
		}
		else
		{
			// compressed...

			Common::Data::Pointer	fieldValue;

			File::readOutsideVariableField(this->m_pTransaction,
										   keyObjectPage,
										   objectID,
										   Common::DataType::String,
										   fieldValue,
										   this->m_FixMode,
										   this->m_CatchMemoryExhaust,
										   AttachNodePages_);

			Common::StringData*	stringFieldValue =
				_SYDNEY_DYNAMIC_CAST(Common::StringData*,
									 fieldValue.get());

			unicodeFieldValue = stringFieldValue->getString();
		}

		numChar = unicodeFieldValue.getLength();

		if (this->m_pOpenParameter->m_cSearchCondition.m_SetEscape)
		{
			compareResult =
				Utility::CharTrait::like(
					(const ModUnicodeChar*)unicodeFieldValue,
					numChar,
					patternHead,
					patternNumChar,
					this->m_pOpenParameter->m_cSearchCondition.m_Escape);
		}
		else
		{
			compareResult =
				Utility::CharTrait::like(
					(const ModUnicodeChar*)unicodeFieldValue,
					numChar,
					patternHead,
					patternNumChar);
		}
	}
	else
	{
		// normal...

		if (this->m_pOpenParameter->m_cSearchCondition.m_SetEscape)
		{
			compareResult =
				Utility::CharTrait::like(
					fieldValue,
					numChar,
					patternHead,
					patternNumChar,
					this->m_pOpenParameter->m_cSearchCondition.m_Escape);
		}
		else
		{
			compareResult = Utility::CharTrait::like(fieldValue,
														  numChar,
														  patternHead,
														  patternNumChar);
		}
	}

	if (compareResult == false)
	{
		checkMemoryExhaust(leafPage);
		if (attached)
		{
			checkMemoryExhaust(keyObjectPage);
		}
		if (this->m_CatchMemoryExhaust)	// MemoryExhaust ならデタッチ（アンフィックス）されている
		{
			leafPage = 0;
			keyObjectPage = 0;
		}

		PhysicalFile::PageID	saveLeafPageID = this->m_LeafPageID;
		ModUInt32				saveKeyInfoIndex = this->m_KeyInfoIndex;

		this->m_LeafPageID = leafPageID;
		this->m_KeyInfoIndex = keyInfoIndex;

		valueObjectID = this->getPrevObjectIDByLike(AttachNodePages_);

		this->m_LeafPageID = saveLeafPageID;
		this->m_KeyInfoIndex = saveKeyInfoIndex;
	}

	checkMemoryExhaust(leafPage);
	if (attached)
	{
		checkMemoryExhaust(keyObjectPage);
	}

	return valueObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchLeafPageForLike --
//		like演算子による検索条件と一致するキーオブジェクトが
//		記録されている可能性があるリーフページを検索する
//
//	NOTES
//	先頭のString型のキーフィールドがlike演算子による
//	検索条件と一致するキーオブジェクトを記録している可能性がある
//	リーフページを検索する。
//	ただし、返すのは実際にキーオブジェクトが記録されている
//	物理ページの記述子ではなく、そのキーオブジェクトへ辿ることができる
//	キー情報が記録されている物理ページの記述子である。
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
//		先頭のString型のキーフィールドがlike演算子による
//		検索条件と一致するキーオブジェクトへ辿ることができる
//		キー情報が記録されている物理ページの記述子。
//		該当するキーオブジェクトが存在しない場合には、
//		0（ヌルポインタ）を返す。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::searchLeafPageForLike(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	if (TreeDepth_ == 1)
	{
		return
			this->containTargetKeyObjectForLike(
				RootNodePageID_,
				AttachNodePages_,
				true); // リーフページ
	}

	PhysicalFile::PageID	nodePageID = RootNodePageID_;

	for (ModUInt32	depth = 1; depth < TreeDepth_; depth++)
	{
		nodePageID = this->searchChildNodePageForLike(nodePageID,
													  AttachNodePages_);

		if (nodePageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			return 0;
		}
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
//	Btree::File::containTargetKeyObjectForLike --
//		like演算子による検索条件と一致するキーオブジェクトが
//		ノードページ内に存在する可能性があるかどうかを知らせる
//
//	NOTES
//	引数NodePageID_で示されるノードページ内に、
//	like演算子による検索条件と一致するキーオブジェクトが
//	ノードページ内に存在する可能性があるかどうかを知らせる。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	NodePageID_
//		調べるノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool					IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	PhysicalFile::Page*
//		like演算子による検索条件と一致するキーオブジェクトが
//		ノードページ内に存在する可能性があれば、そのノードページの
//		物理ページ記述子、存在する可能性がなければ、0（ヌルポインタ）。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::containTargetKeyObjectForLike(
	const PhysicalFile::PageID	NodePageID_,
	PageVector&					AttachNodePages_,
	const bool					IsLeafPage_) const
{
	PhysicalFile::Page*	nodePage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 NodePageID_,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(nodePage != 0);

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   nodePage,
								   IsLeafPage_);

	const ModUInt32 useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	bool	exist = true;

	if (useKeyInfoNum > 0)
	{
		KeyInformation	keyInfo(this->m_pTransaction,
								nodePage,
								useKeyInfoNum - 1,
								IsLeafPage_);

		ModUInt64	lastKeyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(
			lastKeyObjectID != FileCommon::ObjectID::Undefined &&
			lastKeyObjectID != 0);

		int	compareResult =
			this->compareToLikeSearchCondition(nodePage,
											   AttachNodePages_,
											   lastKeyObjectID);

		if (compareResult > 0)
		{
			exist = false;
		}
	}
	else
	{
		exist = false;
	}

	if (exist == false)
	{
		checkMemoryExhaust(nodePage);

		nodePage = 0;
	}

	return nodePage;
}

//	FUNCTION private
//	Btree::File::compareToLikeSearchCondition --
//		like演算子による検索条件と先頭キーフィールドの値を比較する
//
//	NOTES
//	like演算子による検索条件と先頭のString型のキーフィールドの値を
//	比較し、比較結果を返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*			KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const ModUInt64				KeyObjectID_
//		キーオブジェクトのオブジェクトID
//
//	RETURN
//	int
//		< 0 : 検索条件の方がキー値順で前方
//		= 0 : 検索条件と先頭キーフィールドの値が等しい
//		> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]

int
File::compareToLikeSearchCondition(
	PhysicalFile::Page*			KeyInfoPage_,
	PageVector&					AttachNodePages_,
	const ModUInt64				KeyObjectID_) const
{
	PhysicalFile::Page*		keyObjectPage = 0;

	bool	attached =
		File::attachObjectPage(this->m_pTransaction,
							   KeyObjectID_,
							   KeyInfoPage_,
							   keyObjectPage,
							   this->m_FixMode,
							   this->m_CatchMemoryExhaust,
							   AttachNodePages_);

	const PhysicalFile::AreaID	keyObjectAreaID =
		Common::ObjectIDData::getLatterValue(KeyObjectID_);

	PhysicalFile::Page*	outsideObjectPage = 0;
	ModSize	numChar = 0;
	bool	isDivide;
	bool	isCompressed;

	ModUnicodeChar*	fieldValue =
		this->getTopStringFieldPointer(keyObjectPage,
									   outsideObjectPage,
									   AttachNodePages_,
									   keyObjectAreaID,
									   numChar,
									   isDivide,
									   isCompressed);

	ModUnicodeChar	forEmpty = 0;
	if (numChar == 0)
	{
		fieldValue = &forEmpty;
	}

	int	compareResult;

	if (fieldValue == 0)
		compareResult = 1;
	else {
		ModSize	conditionNumChar =
			this->m_SearchHint.m_LikeSearchCondition.getLength();

		if (isDivide) {

			// divide...

			char*	objectIDReadPos =
				static_cast<char*>(
					this->getFieldPointer(
						keyObjectPage,
						keyObjectAreaID,
						1,      // field index
						true)); // key object

			ModUInt64	objectID;
			File::readObjectID(objectIDReadPos, objectID);

			ModUnicodeString	unicodeFieldValue;

			File::readOutsideStringField(this->m_pTransaction,
										 keyObjectPage,
										 objectID,
										 unicodeFieldValue,
										 this->m_FixMode,
										 this->m_CatchMemoryExhaust,
										 AttachNodePages_);

			compareResult = m_SearchHint.m_LikeSearchCondition.compare(
				unicodeFieldValue, conditionNumChar);

		} else if (isCompressed) {

			char*	objectIDReadPos =
				static_cast<char*>(
					this->getFieldPointer(
						keyObjectPage,
						keyObjectAreaID,
						1,      // field index
						true)); // key object

			ModUInt64	objectID;
			File::readObjectID(objectIDReadPos, objectID);

			Common::Data::Pointer	fieldValue;

			File::readOutsideVariableField(this->m_pTransaction,
										   keyObjectPage,
										   objectID,
										   Common::DataType::String,
										   fieldValue,
										   this->m_FixMode,
										   this->m_CatchMemoryExhaust,
										   AttachNodePages_);

			Common::StringData*	stringFieldValue =
				_SYDNEY_DYNAMIC_CAST(Common::StringData*, fieldValue.get());

			compareResult =	m_SearchHint.m_LikeSearchCondition.compare(
				stringFieldValue->getString(), conditionNumChar);
		} else {

			// normal...

			ModUnicodeString	unicodeFieldValue(fieldValue, numChar);

			compareResult =	m_SearchHint.m_LikeSearchCondition.compare(
				unicodeFieldValue, conditionNumChar);
		}
	}

	if (attached)
		checkMemoryExhaust(keyObjectPage);

	return compareResult * *m_SearchHint.m_MultiNumberArray;
}

//
//	FUNCTION private
//	Btree::File::searchChildNodePageForLike --
//		like演算子による検索条件と一致するキーオブジェクトが
//		記録されている可能性がある子ノードページを検索する
//
//	NOTES
//	先頭のString型のキーフィールドがlike演算子による
//	検索条件と一致するキーオブジェクトを記録している可能性がある
//	ノードページを、引数ParentNodePageID_が示す親ノードページの
//	子ノードページの中から検索する。
//	ただし、返すのは実際にキーオブジェクトが記録されている
//	物理ページではなく、そのキーオブジェクトへ辿ることができる
//	キー情報が記録されている物理ページの識別子である。
//	リーフページを含むノードページのキーテーブルが
//	1物理ページに収まる場合に呼び出される。
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
//		先頭のString型のキーフィールドがlike演算子による
//		検索条件と一致するキーオブジェクトを記録している可能性がある
//		子ノードページの物理ページ識別子。
//		該当するキーオブジェクトが存在しない場合には、
//		PhysicalFile::ConstValue::UndefinedPageIDを返す。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::PageID
File::searchChildNodePageForLike(
	const PhysicalFile::PageID	ParentNodePageID_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	parentNodePage =
		this->containTargetKeyObjectForLike(ParentNodePageID_,
											AttachNodePages_,
											false); // リーフページではない

	if (parentNodePage == 0)
	{
		return PhysicalFile::ConstValue::UndefinedPageID;
	}

	bool	match = false; // dummy

	NodePageHeader	parentNodePageHeader(this->m_pTransaction,
										 parentNodePage,
										 false); // リーフページではない

	int	keyInfoIndex =
		this->getKeyInformationIndexForLike(
			parentNodePage,
			parentNodePageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			false, // リーフページではない
			match);

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false); // リーフページではない

	PhysicalFile::PageID	childNodePageID = keyInfo.readChildNodePageID();

	; _SYDNEY_ASSERT(
		childNodePageID != PhysicalFile::ConstValue::UndefinedPageID);

	checkMemoryExhaust(parentNodePage);

	return childNodePageID;
}

//
//	FUNCTION private
//	Btree::File::getKeyInformationIndexForLike --
//		like演算子による検索条件に最も近い値を持つキーオブジェクトへ辿る
//		キー情報のインデックスを返す
//
//	NOTES
//	引数KeyInfoPage_が示すノードページ内で、
//	like演算子による検索条件に最も近い値を持つキーオブジェクトを検索し、
//	そのキーオブジェクトへ辿ることができるキー情報のインデックスを
//	返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32		UseKeyInfoNum_
//		引数KeyInfoPage_が示すノードページ内の使用中のキー情報数
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//	const bool			IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	bool&				Match_
//		そのキー情報から辿ることができるキーオブジェクトに
//		記録されている、先頭から連続している
//		Fetch対象キーフィールドがFetch検索条件と
//		完全に一致しているかどうか
//			true  : 完全に一致している
//			false : 完全には一致していない
//
//	RETURN
//	int
//		キー情報のインデックス
//
//	EXCEPTIONS
//	[YET!]
//
int
File::getKeyInformationIndexForLike(PhysicalFile::Page*	KeyInfoPage_,
									const ModUInt32		UseKeyInfoNum_,
									PageVector&			AttachNodePages_,
									const bool			IsLeafPage_,
									bool&				Match_) const
{
	; _SYDNEY_ASSERT(UseKeyInfoNum_ > 0);

	int	midKeyInfoIndex = 0;
	int	firstKeyInfoIndex = 0;
	int	lastKeyInfoIndex = UseKeyInfoNum_ - 1;

	int	keyInfoIndex = -1;

	KeyInformation	keyInfo(this->m_pTransaction,
							KeyInfoPage_,
							0,
							IsLeafPage_);

	Match_ = false;

	while (firstKeyInfoIndex <= lastKeyInfoIndex)
	{
		midKeyInfoIndex = (firstKeyInfoIndex + lastKeyInfoIndex) >> 1;

		keyInfo.setStartOffsetByIndex(midKeyInfoIndex);

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		int	compareResult =
			this->compareToLikeSearchCondition(KeyInfoPage_,
											   AttachNodePages_,
											   keyObjectID);

		if (compareResult < 0)
		{
			lastKeyInfoIndex = midKeyInfoIndex - 1;
		}
		else if (compareResult > 0)
		{
			firstKeyInfoIndex = midKeyInfoIndex + 1;
		}
		else
		{
			Match_ = true;

			keyInfoIndex = midKeyInfoIndex;

			int	addNum;

			if (this->m_pOpenParameter->m_bSortReverse)
			{
				if (keyInfoIndex == static_cast<int>(UseKeyInfoNum_) - 1)
				{
					break;
				}

				addNum = 1;
			}
			else
			{
				if (keyInfoIndex == 0)
				{
					break;
				}

				addNum = -1;
			}

			while (true)
			{
				keyInfoIndex += addNum;

				keyInfo.setStartOffsetByIndex(keyInfoIndex);

				keyObjectID = keyInfo.readKeyObjectID();

				; _SYDNEY_ASSERT(
					keyObjectID != FileCommon::ObjectID::Undefined &&
					keyObjectID != 0);

				if (this->compareToLikeSearchCondition(KeyInfoPage_,
													   AttachNodePages_,
													   keyObjectID)
					!= 0)
				{
					keyInfoIndex -= addNum;

					break;
				}

				if (this->m_pOpenParameter->m_bSortReverse)
				{
					if (keyInfoIndex ==
						static_cast<int>(UseKeyInfoNum_ - 1))
					{
						break;
					}
				}
				else
				{
					if (keyInfoIndex == 0)
					{
						break;
					}
				}

			} // end while

			break;
		}
	}

	if (keyInfoIndex == -1)
	{
		keyInfoIndex = firstKeyInfoIndex;
	}

	return keyInfoIndex;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDByLike --
//		like演算子による検索条件と一致する次のオブジェクトのIDを返す
//
//	NOTES
//	like演算子による検索条件と一致する次のオブジェクトを検索し、
//	該当するオブジェクトが存在すれば、そのオブジェクトのIDを返す。
//	“次のオブジェクト”とは、
//	前回の、like演算子による検索条件と一致するオブジェクトを基準として、
//	キー値順に後方のオブジェクトのことを指す。
//	オープン時に、オブジェクトの挿入ソート順で
//	オブジェクトを取得するように設定されている場合に、
//	呼び出される。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		like演算子による検索条件と一致するオブジェクトのID。
//		一致するオブジェクトがファイル内に存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDByLike(PageVector&	AttachNodePages_) const
{
	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	ModUInt64	valueObjectID = FileCommon::ObjectID::Undefined;

	while (true)
	{
		if (this->assignNextKeyInformation(leafPage,
										   AttachNodePages_,
										   leafPageHeader,
										   keyInfo)
			== false)
		{
			checkMemoryExhaust(leafPage);

			break;
		}

		if (leafPageID != leafPage->getID())
		{
			leafPageID = leafPage->getID();
		}

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		if (this->compareToLikeSearchCondition(leafPage,
											   AttachNodePages_,
											   keyObjectID)
			!= 0)
		{
			checkMemoryExhaust(leafPage);

			break;
		}

		PhysicalFile::Page*		keyObjectPage = 0;

		bool	attached =
			File::attachObjectPage(this->m_pTransaction,
								   keyObjectID,
								   leafPage,
								   keyObjectPage,
								   this->m_FixMode,
								   this->m_CatchMemoryExhaust,
								   AttachNodePages_);

		PhysicalFile::Page*	outsideObjectPage = 0;
		ModSize	numChar = 0;
		bool	isDivide;
		bool	isCompressed;

		ModUnicodeChar*	fieldValue = getTopStringFieldPointer(
			keyObjectPage, outsideObjectPage, AttachNodePages_,
			Common::ObjectIDData::getLatterValue(keyObjectID),
			numChar,
			isDivide,
			isCompressed);

		ModUnicodeChar	forEmpty = 0;
		if (numChar == 0)
		{
			fieldValue = &forEmpty;
		}

		; _SYDNEY_ASSERT(fieldValue != 0);

		const ModUnicodeChar*	patternHead =
			(const ModUnicodeChar*)this->m_SearchHint.m_PatternString;
		ModSize	patternNumChar =
			this->m_SearchHint.m_PatternString.getLength();

		bool	compareResult = false;

		if (isDivide || isCompressed)
		{
			// divide or compressed...

			char* objectIDReadPos = static_cast<char*>(
				getFieldPointer(
					keyObjectPage,
					Common::ObjectIDData::getLatterValue(keyObjectID),
					1,      // field index
					true)); // key object

			ModUInt64	objectID;
			File::readObjectID(objectIDReadPos, objectID);

			ModUnicodeString	unicodeFieldValue;

			if (isDivide)
			{
				// divide...

				File::readOutsideStringField(this->m_pTransaction,
											 keyObjectPage,
											 objectID,
											 unicodeFieldValue,
											 this->m_FixMode,
											 this->m_CatchMemoryExhaust,
											 AttachNodePages_);
			}
			else
			{
				// compressed...

				Common::Data::Pointer	fieldValue;

				File::readOutsideVariableField(this->m_pTransaction,
											   keyObjectPage,
											   objectID,
											   Common::DataType::String,
											   fieldValue,
											   this->m_FixMode,
											   this->m_CatchMemoryExhaust,
											   AttachNodePages_);

				Common::StringData*	stringFieldValue =
					_SYDNEY_DYNAMIC_CAST(Common::StringData*,
										 fieldValue.get());

				unicodeFieldValue = stringFieldValue->getString();
			}

			numChar = unicodeFieldValue.getLength();

			if (this->m_pOpenParameter->m_cSearchCondition.m_SetEscape)
			{
				compareResult =
					Utility::CharTrait::like(
						(const ModUnicodeChar*)unicodeFieldValue,
						numChar,
						patternHead,
						patternNumChar,
						this->m_pOpenParameter->m_cSearchCondition.m_Escape);
			}
			else
			{
				compareResult =
					Utility::CharTrait::like(
						(const ModUnicodeChar*)unicodeFieldValue,
						numChar,
						patternHead,
						patternNumChar);
			}
		}
		else
		{
			// normal...

			if (this->m_pOpenParameter->m_cSearchCondition.m_SetEscape)
			{
				compareResult =
					Utility::CharTrait::like(
						fieldValue,
						numChar,
						patternHead,
						patternNumChar,
						this->m_pOpenParameter->m_cSearchCondition.m_Escape);
			}
			else
			{
				compareResult = Utility::CharTrait::like(fieldValue,
															  numChar,
															  patternHead,
															  patternNumChar);
			}
		}

		if (compareResult)
		{
			valueObjectID = keyInfo.readValueObjectID();

			break;
		}
	}

	return valueObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDByLike --
//		like演算子による検索条件と一致する前のオブジェクトのIDを返す
//
//	NOTES
//	like演算子による検索条件と一致する前のオブジェクトを検索し、
//	該当するオブジェクトが存在すれば、そのオブジェクトのIDを返す。
//	“前のオブジェクト”とは、
//	前回の、like演算子による検索条件と一致するオブジェクトを基準として、
//	キー値順に前方のオブジェクトのことを指す。
//	オープン時に、オブジェクトの挿入ソート順の逆順で
//	オブジェクトを取得するように設定されている場合に、
//	呼び出される。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		like演算子による検索条件と一致するオブジェクトのID
//		一致するオブジェクトがファイル内に存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDByLike(PageVector&	AttachNodePages_) const
{
	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	ModUInt64	valueObjectID = FileCommon::ObjectID::Undefined;

	while (true)
	{
		if (this->assignPrevKeyInformation(leafPage,
										   AttachNodePages_,
										   leafPageHeader,
										   keyInfo)
			== false)
		{
			checkMemoryExhaust(leafPage);

			break;
		}

		if (leafPageID != leafPage->getID())
		{
			leafPageID = leafPage->getID();
		}

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		if (this->compareToLikeSearchCondition(leafPage,
											   AttachNodePages_,
											   keyObjectID)
			!= 0)
		{
			checkMemoryExhaust(leafPage);

			break;
		}

		PhysicalFile::Page*		keyObjectPage = 0;

		bool	attached =
			File::attachObjectPage(this->m_pTransaction,
								   keyObjectID,
								   leafPage,
								   keyObjectPage,
								   this->m_FixMode,
								   this->m_CatchMemoryExhaust,
								   AttachNodePages_);

		PhysicalFile::Page*	outsideObjectPage = 0;
		ModSize	numChar = 0;
		bool	isDivide;
		bool	isCompressed;

		ModUnicodeChar*	fieldValue = getTopStringFieldPointer(
			keyObjectPage, outsideObjectPage, AttachNodePages_,
			Common::ObjectIDData::getLatterValue(keyObjectID),
			numChar,
			isDivide,
			isCompressed);

		ModUnicodeChar	forEmpty = 0;
		if (numChar == 0)
		{
			fieldValue = &forEmpty;
		}

		; _SYDNEY_ASSERT(fieldValue != 0);

		const ModUnicodeChar*	patternHead =
			(const ModUnicodeChar*)this->m_SearchHint.m_PatternString;
		ModSize	patternNumChar =
			this->m_SearchHint.m_PatternString.getLength();

		bool	compareResult = false;

		if (isDivide || isCompressed)
		{
			// divide or compressed...

			char* objectIDReadPos = static_cast<char*>(
				getFieldPointer(
					keyObjectPage,
					Common::ObjectIDData::getLatterValue(keyObjectID),
					1,      // field index
					true)); // key object

			ModUInt64	objectID;
			File::readObjectID(objectIDReadPos, objectID);

			ModUnicodeString	unicodeFieldValue;

			if (isDivide)
			{
				// divide...

				File::readOutsideStringField(this->m_pTransaction,
											 keyObjectPage,
											 objectID,
											 unicodeFieldValue,
											 this->m_FixMode,
											 this->m_CatchMemoryExhaust,
											 AttachNodePages_);
			}
			else
			{
				// compressed...

				Common::Data::Pointer	fieldValue;

				File::readOutsideVariableField(this->m_pTransaction,
											   keyObjectPage,
											   objectID,
											   Common::DataType::String,
											   fieldValue,
											   this->m_FixMode,
											   this->m_CatchMemoryExhaust,
											   AttachNodePages_);

				Common::StringData*	stringFieldValue =
					_SYDNEY_DYNAMIC_CAST(Common::StringData*,
										 fieldValue.get());

				unicodeFieldValue = stringFieldValue->getString();
			}

			numChar = unicodeFieldValue.getLength();

			if (this->m_pOpenParameter->m_cSearchCondition.m_SetEscape)
			{
				compareResult =
					Utility::CharTrait::like(
						(const ModUnicodeChar*)unicodeFieldValue,
						numChar,
						patternHead,
						patternNumChar,
						this->m_pOpenParameter->m_cSearchCondition.m_Escape);
			}
			else
			{
				compareResult =
					Utility::CharTrait::like(
						(const ModUnicodeChar*)unicodeFieldValue,
						numChar,
						patternHead,
						patternNumChar);
			}
		}
		else
		{
			// normal...

			if (this->m_pOpenParameter->m_cSearchCondition.m_SetEscape)
			{
				compareResult =
					Utility::CharTrait::like(
						fieldValue,
						numChar,
						patternHead,
						patternNumChar,
						this->m_pOpenParameter->m_cSearchCondition.m_Escape);
			}
			else
			{
				compareResult = Utility::CharTrait::like(fieldValue,
															  numChar,
															  patternHead,
															  patternNumChar);
			}
		}

		if (compareResult)
		{
			valueObjectID = keyInfo.readValueObjectID();

			break;
		}
	}

	return valueObjectID;
}

//
//	Copyright (c) 2001, 2002, 2004, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
