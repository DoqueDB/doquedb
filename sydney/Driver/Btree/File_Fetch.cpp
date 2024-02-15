// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_Fetch.cpp --
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#include "Exception/BadArgument.h"
#include "Common/Assert.h"
#include "Common/CompressedBinaryData.h"

#include "LogicalFile/ObjectID.h"

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
//	Btree::File::fetchByKey --
//		Fetch検索条件でオブジェクトを検索する
//
//	NOTES
//	Fetch検索条件でオブジェクトを検索し、
//	該当するオブジェクトが存在すれば
//	そのオブジェクトのオブジェクトIDを返す。
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
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		Fetch検索条件と一致するオブジェクトのオブジェクトID
//		一致するオブジェクトがファイル内に存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::fetchByKey(const ModUInt32			TreeDepth_,
				 const PhysicalFile::PageID	RootNodePageID_,
				 PageVector&				AttachNodePages_,
				 ValueFile*					ValueFile_,
				 PageVector&				AttachValuePages_)
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->fetchBySimpleKey(TreeDepth_,
									  RootNodePageID_,
									  AttachNodePages_,
									  ValueFile_,
									  AttachValuePages_);
	}

	; _SYDNEY_ASSERT(this->m_FetchOptionData.get() != 0);

	; _SYDNEY_ASSERT(this->m_Searched == false);

	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getType() == Common::DataType::Array);
	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getElementType() == Common::DataType::Data);

	const Common::DataArrayData*	fetchCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 this->m_FetchOptionData.get());

	; _SYDNEY_ASSERT(fetchCondition != 0);

	// Fetch検索のためのヒントに、検索条件を設定する
	this->m_FetchHint.setCondition(fetchCondition);

	//
	// まずは、
	//     “先頭から連続しているFetch対象キーフィールドが
	//     　Fetch検索条件と一致するキーオブジェクト”
	// を検索する。
	//
	// 例えば、下図の“↑”がFetch対象キーフィールドだとすると、
	//     “key1とkey2とkey3がFetch検索条件と一致するキーオブジェクト”
	// を検索する。（key5のチェックは後回しにする。）
	// 
	//	┌───┬───┬───┬───┬───┬───┬───┐
	//	│ key1 │ key2 │ key3 │ key4 │ key5 │ val1 │ val2 │
	//	└───┴───┴───┴───┴───┴───┴───┘
	//	    ↑      ↑      ↑              ↑
	//

	// 先頭から連続しているFetch対象キーフィールドが
	// Fetch検索条件と一致するキーオブジェクトが
	// 記録されている可能性があるリーフページを検索する。
	// ※ ここで得られる物理ページ記述子は、
	// 　 “キーオブジェクト”が記録されている
	// 　 物理ページの記述子だとは限らない。
	// 　 “キー情報”が記録されている物理ページの記述子である。
	PhysicalFile::Page*	leafPage =
		this->searchLeafPageForFetch(TreeDepth_,
									 RootNodePageID_,
									 AttachNodePages_,
									 fetchCondition);

	if (leafPage == 0)
	{
		// 先頭から連続しているFetch対象キーフィールドが
		// Fetch検索条件と一致するキーオブジェクトが
		// 記録されている可能性のあるリーフページすら
		// 存在しなかった…

		return FileCommon::ObjectID::Undefined;
	}

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	bool	match = false;

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	// 先頭から連続しているFetch対象キーフィールドが
	// Fetch検索条件に最も近い値のキーオブジェクトへ辿る
	// キー情報のインデックスを取得する。
	// そのキーオブジェクトがFetch検索条件と一致しているかどうかは、
	// File::getKeyInformationIndexForFetchが設定する
	// ローカル変数matchを参照すればわかる。
	int	keyInfoIndex =
		this->getKeyInformationIndexForFetch(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			fetchCondition,
			true, // リーフページ
			match);

	if (match == false)
	{
		// 先頭から連続しているFetch対象キーフィールドが
		// Fetch検索条件と一致する
		// キーオブジェクトは存在しなかった…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	//
	// ここまでたどり着いたということは、
	// 先頭から連続しているFetch対象キーフィールドが
	// Fetch検索条件と一致するキーオブジェクトが
	// 見つかったということ！
	//

	// キーフィールドがFetch検索条件と一致する
	// キーオブジェクトへ辿るキー情報から
	// バリューオブジェクトのオブジェクトIDを読み込む。

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true); // リーフページ

	ModUInt64 valueObjectID = keyInfo.readValueObjectID();

	if (this->m_FetchHint.m_ExistSeparateKey)
	{
		// 先頭キーフィールドから連続している
		// Fetch対象キーフィールドから
		// 離れたFetch対象キーフィールドがある…

		//
		// 例えば、下図の“↑”がFetch対象キーフィールドだとすると、
		//     “key1とkey2がFetch検索条件と一致するキーオブジェクト”
		// は、既に見つかっているので、
		// このifブロック内で、
		//     “key4とkey5がFetch検索条件と一致するキーオブジェクト”
		// を検索する。
		// 
		//	┌───┬───┬───┬───┬───┬───┬───┐
		//	│ key1 │ key2 │ key3 │ key4 │ key5 │ val1 │ val2 │
		//	└───┴───┴───┴───┴───┴───┴───┘
		//	    ↑      ↑              ↑      ↑
		//

		// キーフィールドがFetch検索条件と一致する
		// キーオブジェクトへ辿るキー情報から
		// キーオブジェクトのオブジェクトIDを読み込む。

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		// キーオブジェクトが記録されているノード（物理）ページの
		// 物理ページ記述子を生成（アタッチ）する。

		PhysicalFile::Page*		keyObjectPage = 0;

		bool	attached =
			File::attachObjectPage(this->m_pTransaction,
								   keyObjectID,
								   leafPage,
								   keyObjectPage,
								   this->m_FixMode,
								   this->m_CatchMemoryExhaust,
								   AttachNodePages_);

		if (compareToFetchCondition(
				keyObjectPage, AttachNodePages_,
				Common::ObjectIDData::getLatterValue(keyObjectID),
				fetchCondition,
				FetchHint::CompareType::OnlySeparateKey,
				true) // キーオブジェクト
			!= 0)
		{
			// 先頭キーフィールドから連続している
			// Fetch対象キーフィールドから
			// 離れたFetch対象キーフィールドが
			// Fetch検索条件と一致しなかった…

			// しかし、まだ該当するオブジェクトが
			// 存在するかもしれないので、
			// 次以降のキーオブジェクトを参照してみる。
			valueObjectID =
				this->getNextObjectIDFetchByKey(leafPage,
												keyInfo,
												AttachNodePages_,
												ValueFile_,
												AttachValuePages_,
												fetchCondition);

			if (valueObjectID == FileCommon::ObjectID::Undefined)
			{
				// 次以降にも離れたFetch対象キーフィールドが
				// Fetch検索条件と一致しなかった…

				// しかし、まだ該当するオブジェクトが
				// 前に存在するかもしれないので、前以降の
				// キーオブジェクトを参照してみる。

				if (leafPage == 0 || leafPage->getID() != leafPageID)
				{
					checkMemoryExhaust(leafPage);

					leafPage = File::attachPage(this->m_pTransaction,
												this->m_pPhysicalFile,
												leafPageID,
												this->m_FixMode,
												this->m_CatchMemoryExhaust,
												AttachNodePages_);

					keyInfo.resetPhysicalPage(leafPage);
				}

				keyInfo.setStartOffsetByIndex(keyInfoIndex);

				valueObjectID =
					this->getPrevObjectIDFetchByKey(leafPage,
													keyInfo,
													AttachNodePages_,
													ValueFile_,
													AttachValuePages_,
													fetchCondition);
			}
		}

		if (attached)
		{
			checkMemoryExhaust(keyObjectPage);
		}

		if (valueObjectID == FileCommon::ObjectID::Undefined)
		{
			checkMemoryExhaust(leafPage);

			return valueObjectID;
		}
	}

	//
	// ここまでたどり着いたということは、
	// Fetch対象キーフィールドがFetch検索条件と一致する
	// キーオブジェクトが見つかったということ！
	//

	; _SYDNEY_ASSERT(valueObjectID != FileCommon::ObjectID::Undefined);

	if (this->m_FetchHint.m_OnlyKey == false)
	{
		// バリューフィールドも
		// Fetch対象フィールドとなっている…

		//
		// 例えば、下図の“↑”がFetch対象フィールドだとすると、
		//     “key1とkey2とkey4がFetch検索条件と一致するオブジェクト”
		// は、既に見つかっているので、
		// このifブロック内で、
		//     “val1がFetch検索条件と一致するオブジェクト”
		// を検索する。
		// 
		//	┌───┬───┬───┬───┬───┬───┬───┐
		//	│ key1 │ key2 │ key3 │ key4 │ key5 │ val1 │ val2 │
		//	└───┴───┴───┴───┴───┴───┴───┘
		//	    ↑      ↑              ↑              ↑
		//

		// バリューはバリューページにしか記録されていないので、
		// バリューオブジェクトが記録されているバリューページを
		// アタッチする

		PhysicalFile::Page*	valuePage = File::attachPage(
			m_pTransaction, ValueFile_->m_PhysicalFile,
			Common::ObjectIDData::getFormerValue(valueObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

		; _SYDNEY_ASSERT(valuePage != 0);

		if (this->compareToFetchCondition(
				valuePage,
				AttachValuePages_,
				Common::ObjectIDData::getLatterValue(valueObjectID),
				fetchCondition,
				FetchHint::CompareType::OnlyValue,
				false) // バリューオブジェクト
			!= 0)
		{
			// Fetch対象バリューフィールドが
			// Fetch検索条件と一致しなかった…

			ModUInt64	saveValueObjectID = valueObjectID;

			// しかし、まだ該当するオブジェクトが
			// 存在するかもしれないので、
			// 次以降のバリューオブジェクトを参照してみる。
			valueObjectID =
				this->getNextObjectIDFetchByKey(valuePage,
												AttachNodePages_,
												ValueFile_,
												AttachValuePages_,
												valueObjectID,
												fetchCondition);

			if (valueObjectID == FileCommon::ObjectID::Undefined)
			{
				// 次以降にもバリューが一致するオブジェクトが
				// 存在しなかった…

				// しかし、まだ該当するオブジェクトが
				// 前に存在するかもしれないので、前以降の
				// バリューオブジェクトを参照してみる。
				valueObjectID =
					this->getPrevObjectIDFetchByKey(valuePage,
													AttachNodePages_,
													ValueFile_,
													AttachValuePages_,
													saveValueObjectID,
													fetchCondition);
			}
		}

		if (this->m_CatchMemoryExhaust)
		{
			ValueFile_->m_PhysicalFile->detachPage(
				valuePage,
				PhysicalFile::Page::UnfixMode::NotDirty,
				false); // 本当にデタッチ（アンフィックス）してしまう
		}
	}

	checkMemoryExhaust(leafPage);

	return valueObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchLeafPageForFetch --
//		Fetch検索条件と一致するキーオブジェクトが記録されている
//		可能性があるリーフページを検索する
//
//	NOTES
//	先頭から連続しているFetch対象キーフィールドが
//	Fetch検索条件と一致するキーオブジェクトを
//	記録している可能性があるリーフページを検索する。
//	ただし、返すのは実際にキーオブジェクトが記録されている
//	物理ページの記述子ではなく、そのキーオブジェクトへ辿ることができる
//	キー情報が記録されている物理ページの記述子である。
//
//	ARGUMENTS
//	const ModUInt32					TreeDepth_
//		現在の木の深さ
//	const PhysicalFile::PageID		RootNodePageID_
//		ルートノードページの物理ページ識別子
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//
//	RETURN
//	PhysicalFile::Page*
//		先頭から連続しているFetch対象キーフィールドが
//		Fetch検索条件と一致するキーオブジェクトへ辿ることができる
//		キー情報が記録されている物理ページの記述子。
//		該当するキーオブジェクトが存在しない場合には、
//		0(ヌルポインタ）を返す。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::searchLeafPageForFetch(
	const ModUInt32					TreeDepth_,
	const PhysicalFile::PageID		RootNodePageID_,
	PageVector&						AttachNodePages_,
	const Common::DataArrayData*	Condition_) const
{
	if (TreeDepth_ == 1)
	{
		// 木の深さ（高さ）が1段の木構造…

		// つまり、ルートノードページがリーフページ

		// では、ルートノードページに
		// 先頭から連続しているFetch対象キーフィールドが
		// Fetch検索条件と一致するキーオブジェクトが記録されている
		// 可能性があるかを調べ、その可能性があるならば、
		// ルートノードページの
		// 物理ページ記述子を返せばよい。

		return this->containTargetKeyObjectForFetch(RootNodePageID_,
													AttachNodePages_,
													Condition_,
													true); // リーフページ
	}

	//
	// 以下は、木の深さ（高さ）が2段以上の木構造の場合の処理
	//

	// リーフページまで該当するノードページを辿っていく…

	PhysicalFile::PageID	nodePageID = RootNodePageID_;

	for (ModUInt32 depth = 1; depth < TreeDepth_; depth++)
	{
		nodePageID = this->searchChildNodePageForFetch(nodePageID,
													   AttachNodePages_,
													   Condition_);

		if (nodePageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			// Fetch検索条件と一致するオブジェクトが
			// 記録されている可能性がなくなってしまった…

			return 0;
		}
	}

	// ※ 物理ページのデタッチは、呼び出し側任せ。

	return File::attachPage(this->m_pTransaction,
							this->m_pPhysicalFile,
							nodePageID,
							this->m_FixMode,
							this->m_CatchMemoryExhaust,
							AttachNodePages_);
}

//
//	FUNCTION private
//	Btree::File::searchChildNodePageForFetch --
//		Fetch検索条件と一致するキーオブジェクトが記録されている
//		可能性がある子ノードページを検索する
//
//	NOTES
//	先頭から連続しているFetch対象キーフィールドが
//	Fetch検索条件と一致するキーオブジェクトを
//	記録している可能性があるノードページを
//	引数ParentNodePageID_が示す親ノードページの
//	子ノードページの中から検索する。
//	ただし、返すのは実際にキーオブジェクトが記録されている
//	物理ページではなく、そのキーオブジェクトへ辿ることができる
//	キー情報が記録されている物理ページの識別子である。
//	リーフページを含むノードページのキーテーブルが
//	1物理ページに収まる場合に呼び出される。
//
//	ARGUMENTS
//	const PhysicalFile::PageID		ParentNodePageID_
//		親ノードページの物理ページ識別子
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//
//	RETURN
//	PhysicalFile::PageID
//		先頭から連続しているFetch対象キーフィールドが
//		Fetch検索条件と一致するキーオブジェクトへ辿ることができる
//		キー情報が記録されている子ノードページの物理ページ識別子。
//		該当するキーオブジェクトが存在しない場合には、
//		PhysicalFile::ConstValue::UndefinedPageIDを返す。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::PageID
File::searchChildNodePageForFetch(
	const PhysicalFile::PageID		ParentNodePageID_,
	PageVector&						AttachNodePages_,
	const Common::DataArrayData*	Condition_) const
{
	// まずは、親ノードページに
	// 先頭から連続しているFetch対象キーフィールドが
	// Fetch検索条件と一致するキーオブジェクトが記録されている
	// 可能性があるかどうかを調べ、その可能性がある場合にのみ、
	// 子ノードページを検索する。

	PhysicalFile::Page*	parentNodePage =
		this->containTargetKeyObjectForFetch(ParentNodePageID_,
											 AttachNodePages_,
											 Condition_,
											 false); // リーフページ
											         // ではない

	if (parentNodePage == 0)
	{
		// 親ノードページに
		// 先頭から連続しているFetch対象キーフィールドが
		// Fetch検索条件と一致するキーオブジェクトが記録されている
		// 可能性がない…

		return PhysicalFile::ConstValue::UndefinedPageID;
	}

	//
	// 親ノードページに可能性があるのならば、
	// いずれかの子ノードページも可能性があるということになる。
	//
	// 子ノードページの検索開始！
	//

	bool	match = false; // dummy

	NodePageHeader	parentNodePageHeader(this->m_pTransaction,
										 parentNodePage,
										 false); // リーフページではない

	// 先頭から連続しているFetch対象キーフィールドが
	// Fetch検索条件に最も近い値のキーオブジェクトへ辿る
	// キー情報のインデックスを取得する。
	// ここでは、完全にFetch検索条件と一致している必要はないので、
	// ローカル変数matchは、ダミーである。
	int	keyInfoIndex =
		this->getKeyInformationIndexForFetch(
			parentNodePage,
			parentNodePageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			Condition_,
			false, // リーフページではない
			match);

	// 先頭から連続しているFetch対象キーフィールドが
	// Fetch検索条件に最も近い値のキーオブジェクトへ辿る
	// キー情報が見つかったので、
	// そのキー情報に記録されている
	// 「子ノードページの物理ページ識別子」
	// を読み込む。

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false); // リーフページではない

	PhysicalFile::PageID	childNodePageID =
		keyInfo.readChildNodePageID();

	; _SYDNEY_ASSERT(
		childNodePageID != PhysicalFile::ConstValue::UndefinedPageID);

	checkMemoryExhaust(parentNodePage);

	return childNodePageID;
}

//
//	FUNCTION private
//	Btree::File::getKeyInformationIndexForFetch --
//		Fetch検索条件に最も近い値を持つキーオブジェクトへ辿る
//		キー情報のインデックスを返す
//
//	NOTES
//	引数KeyInfoPage_が示すノードページ内で、
//	Fetch検索条件に最も近い値を持つキーオブジェクトを検索し、
//	そのキーオブジェクトへ辿ることができるキー情報のインデックスを
//	返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*				KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32					UseKeyInfoNum_
//		引数KeyInfoPage_が示すノードページ内の使用中のキー情報数
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const bool						IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	bool&							Match_
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
File::getKeyInformationIndexForFetch(
	PhysicalFile::Page*				KeyInfoPage_,
	const ModUInt32					UseKeyInfoNum_,
	PageVector&						AttachNodePages_,
	const Common::DataArrayData*	Condition_,
	const bool						IsLeafPage_,
	bool&							Match_) const
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

		int	compareResult = this->compareToFetchCondition(KeyInfoPage_,
														  AttachNodePages_,
														  keyObjectID,
														  Condition_);

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

			//
			// v4.0からの
			// Btree::File::expunge
			// Btree::File::update
			// は、
			// 『オブジェクトの値によりオブジェクトを削除／更新』
			// という仕様となった。
			// これらの処理の場合、“オブジェクトをfetch”することとなり、
			// 本関数が呼ばれる。
			// このとき、Fetch検索条件と一致するオブジェクトが
			// 複数存在する場合でも、“最初に見つかったオブジェクト”を
			// 削除／更新すればよいという仕様なので、
			// 以下のif文で分岐する。
			//

			if (this->m_pOpenParameter->m_iOpenMode
				!= FileCommon::OpenMode::Update)
			{
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

					if (this->compareToFetchCondition(KeyInfoPage_,
													  AttachNodePages_,
													  keyObjectID,
													  Condition_)
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
			}

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
//	Btree::File::containTargetKeyObjectForFetch --
//		Fetch検索条件と一致するキーオブジェクトが
//		ノードページ内に存在する可能性があるかどうかを知らせる
//
//	NOTES
//	引数NodePageID_で示されるノードページ内に、
//	キーフィールドへのFetch検索条件と一致するキーオブジェクトが
//	存在する可能性があるかどうかを知らせる。
//
//	ARGUMENTS
//	const PhysicalFile::PageID		NodePageID_
//		調べるノードページの物理ページ識別子
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const bool						IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	PhysicalFile::Page*
//		キーフィールドへのFetch検索条件と一致するキーオブジェクトが
//		ノードページ内に存在する可能性があれば、そのノードページの
//		物理ページ記述子、存在する可能性がなければ、0（ヌルポインタ）。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::containTargetKeyObjectForFetch(
	const PhysicalFile::PageID		NodePageID_,
	PageVector&						AttachNodePages_,
	const Common::DataArrayData*	Condition_,
	const bool						IsLeafPage_) const
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

		if (this->compareToFetchCondition(nodePage,
										  AttachNodePages_,
										  lastKeyObjectID,
										  Condition_)
			> 0)
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

//
//	FUNCTION private
//	Btree::File::compareToFetchCondition --
//		Fetch検索条件とオブジェクトを比較する
//
//	NOTES
//	引数Condition_で示されるFetch検索条件と、
//	引数ValueObjectID_が指し示すオブジェクトを比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const int						ConditionStartIndex_
//		Fetch検索条件の開始インデックス
//		（Condition_の先頭にオブジェクトIDフィールドが
//		　含まれている場合に、1を設定する。）
//	const ModUInt64					ValueObjectID_
//		検索対象となるオブジェクトのID
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*				ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const PhysicalFile::PageID		LeafPageID_ = UndefinedPageID
//		リーフページの物理ページ識別子
//	const ModUInt32					KeyInfoIndex_ = ModUInt32Max
//		キー情報のインデックス
//
//	RETURN
//	bool
//		Fetch検索条件とオブジェクトが等しいかどうか
//			0            : Fetch検索条件とオブジェクトが等しい
//			0x01/*bit0*/ : Fetch検索条件とオブジェクトが Key で異なる（Valueは不明）
//			0x02/*bit1*/ : Fetch検索条件とオブジェクトが Value で異なる（Keyは一致）
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToFetchCondition(
	const Common::DataArrayData*	Condition_,
	const int						ConditionStartIndex_,
	const ModUInt64					ValueObjectID_,
	PageVector&						AttachNodePages_,
	ValueFile*						ValueFile_,
	PageVector&						AttachValuePages_,
	const PhysicalFile::PageID		LeafPageID_,  // = UndefinedPageID
	const ModUInt32					KeyInfoIndex_ // = ModUInt32Max
	) const
{
	; _SYDNEY_ASSERT(Condition_ != 0);
	; _SYDNEY_ASSERT(ValueObjectID_ != FileCommon::ObjectID::Undefined);

	PhysicalFile::PageID	leafPageID = LeafPageID_;
	ModUInt32				keyInfoIndex = KeyInfoIndex_;

	if (leafPageID == PhysicalFile::ConstValue::UndefinedPageID ||
		keyInfoIndex == ModUInt32Max)
	{
		ValueFile_->readLeafInfo(ValueObjectID_,
								 AttachValuePages_,
								 this->m_CatchMemoryExhaust,
								 leafPageID,
								 keyInfoIndex);
	}

	//
	// まずは、キーオブジェクトが等しいかどうかをチェックする。
	//

	PhysicalFile::Page*	leafPage =
		this->attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 leafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	bool	compareResult = false;

	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		compareResult =
			(this->compareToFetchCondition(nullBitmapTop) == 0);
	}
	else
	{
		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		compareResult =
			(this->compareToFetchCondition(leafPage,
										   AttachNodePages_,
										   keyObjectID,
										   Condition_,
										   ConditionStartIndex_)
			 == 0);
	}

	checkMemoryExhaust(leafPage);

	if (!compareResult) return 0x01;//bit0 ※キーオブジェクトが異なっていた

	// キーオブジェクトは等しかった…
	{
		//
		// では、バリューオブジェクトが等しいかどうかをチェックする。
		//

		PhysicalFile::Page*	valuePage =	File::attachPage(
			m_pTransaction, ValueFile_->m_PhysicalFile,
			Common::ObjectIDData::getFormerValue(ValueObjectID_),
			m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

		compareResult =
			(this->compareToFetchCondition(
				valuePage, AttachValuePages_,
				Common::ObjectIDData::getLatterValue(ValueObjectID_),
				Condition_,
				FetchHint::CompareType::OnlyValue,
				false, // バリューオブジェクト
				ConditionStartIndex_)
			 == 0);

		if (this->m_CatchMemoryExhaust)
		{
			ValueFile_->m_PhysicalFile->detachPage(
				valuePage,
				PhysicalFile::Page::UnfixMode::NotDirty,
				false); // 本当にデタッチ（アンフィックス）してしまう
		}
	}

	return (compareResult)? 0 : 0x02;//bit1
}

//
//	FUNCTION private
//	Btree::File::compareToFetchCondition --
//		Fetch検索条件とキーオブジェクトを比較する
//
//	NOTES
//	引数Condition_で示されるFetch検索条件と、
//	引数KeyObjectID_が指し示すキーオブジェクト
//	（キーフィールドの値）を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*				KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	ModUInt64						KeyObjectID_
//		検索対象となるキーオブジェクトのID
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const int						ConditionStartIndex_ = 0
//		Fetch検索条件の開始インデックス
//		（Condition_の先頭にオブジェクトIDフィールドが
//		　含まれている場合に、1を設定する。）
//
//	RETURN
//	int
//		< 0 : Fetch検索条件の方がキー値順で前方
//		= 0 : Fetch検索条件とキーフィールドの値が等しい
//		> 0 : Fetch検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToFetchCondition(
	PhysicalFile::Page*				KeyInfoPage_,
	PageVector&						AttachNodePages_,
	ModUInt64						KeyObjectID_,
	const Common::DataArrayData*	Condition_,
	const int						ConditionStartIndex_ // = 0
	) const
{
	PhysicalFile::Page*		keyObjectPage = 0;

	bool	attached = File::attachObjectPage(this->m_pTransaction,
											  KeyObjectID_,
											  KeyInfoPage_,
											  keyObjectPage,
											  this->m_FixMode,
											  this->m_CatchMemoryExhaust,
											  AttachNodePages_);

	FetchHint::CompareType::Value	compareType =
		this->m_FetchHint.m_ExistSeparateKey ?
			FetchHint::CompareType::OnlyLinkKey :
			FetchHint::CompareType::OnlyKey;

	int	compareResult = compareToFetchCondition(
		keyObjectPage, AttachNodePages_,
		Common::ObjectIDData::getLatterValue(KeyObjectID_),
		Condition_,
		compareType,
		true, // キーオブジェクト
		ConditionStartIndex_);

	if (attached)
	{
		checkMemoryExhaust(keyObjectPage);
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareToFetchCondition --
//		Fetch検索条件とキーオブジェクト／バリューオブジェクトを比較する
//
//	NOTES
//	引数Condition_で示されるFetch検索条件と、
//	キーオブジェクト／バリューオブジェクト
//	（フィールドの値）を比較する。
//
//	ARGUMENTS
//	PhysicalFile::Page*							ObjectPage_
//		キーオブジェクト／バリューオブジェクトが記録されている
//		物理ページの記述子
//	Btree::PageVector&							AttachPages_
//		ページ記述子ベクターへの参照
//		(アタッチした物理ページの記述子をつむ）
//	const PhysicalFile::AreaID					ObjectAreaID_
//		キーオブジェクト／バリューオブジェクトが記録されている
//		物理エリアの識別子
//	const Common::DataArrayData*				Condition_
//		Fetch検索条件へのポインタ
//	const Btree::FetchHint::CompareType::Value	FetchCompareType_
//		Fetch検索対象
//	const bool									IsKeyObject_
//		検索対象がキーオブジェクトかバリューオブジェクトか
//			true  : キーオブジェクト
//			false : バリューオブジェクト
//	const int									ConditionStartIndex_ = 0
//		Fetch検索条件の開始インデックス
//		（Condition_の先頭にオブジェクトIDフィールドが
//		　含まれている場合に、1を設定する。）
//
//	RETURN
//	int
//		< 0 : Fetch検索条件の方がキー値順で前方
//		= 0 : Fetch検索条件とキーフィールド／バリューフィールドの値が等しい
//		> 0 : Fetch検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToFetchCondition(
	PhysicalFile::Page*					ObjectPage_,
	PageVector&							AttachPages_,
	const PhysicalFile::AreaID			ObjectAreaID_,
	const Common::DataArrayData*		Condition_,
	const FetchHint::CompareType::Value	FetchCompareType_,
	const bool							IsKeyObject_,
	const int							ConditionStartIndex_ // = 0
	) const
{
	; _SYDNEY_ASSERT(ObjectPage_ != 0);

	int	compareResult = 0;

	for (int i = 0; i < this->m_FetchHint.m_FieldNumber; i++)
	{
		int	fieldIndex = *(this->m_FetchHint.m_FieldIndexArray + i);

		if (FetchCompareType_ == FetchHint::CompareType::OnlySeparateKey &&
			fieldIndex <= this->m_FetchHint.m_LastLinkKeyFieldIndex)
		{
			continue;
		}

		if (FetchCompareType_ == FetchHint::CompareType::OnlyValue &&
			fieldIndex <= this->m_cFileParameter.m_KeyNum)
		{
			continue;
		}

		if (FetchCompareType_ == FetchHint::CompareType::OnlyLinkKey &&
			fieldIndex > this->m_FetchHint.m_LastLinkKeyFieldIndex)
		{
			break;
		}

		if ((FetchCompareType_ == FetchHint::CompareType::OnlyKey ||
			 FetchCompareType_ == FetchHint::CompareType::OnlySeparateKey) &&
			 fieldIndex > this->m_cFileParameter.m_KeyNum)
		{
			break;
		}

		//
		// Btree::File::updateから
		// “更新前後でオブジェクトの内容が変わるか？”
		// をチェックするためにFetch処理が行われる。
		// このとき、配列フィールドが含まれていると
		// Fetch処理を正しく行うことができない。
		// 更新時のコストを抑えるために、
		// 配列フィールドが含まれている場合には、
		// “オブジェクトの内容が変わる”ものとして
		// 比較を中断する。
		//
		// 配列フィールドはFetch検索対象にできないので
		// 現在のB+木ファイルの使われ方のみを考慮すれば
		// これでよいはず。
		//
		// しかし、オブジェクト削除時もオブジェクト全体を
		// Fetch検索条件として、削除対象のオブジェクトを
		// Fetchしている。
		// いずれかのバリューフィールドが配列フィールドの場合、
		// ここだけでは対応しきれておらず、
		// 配列フィールドの各要素を比較するようにしなくてはいけない。
		// いずれかのバリューフィールドがBinary型のフィールドの場合も
		// 同様でBinary型のバリューフィールドがFetch検索条件と
		// 等しいかどうかもチェックしなければいけない。
		//

		//if (*(this->m_FetchHint.m_FieldTypeArray + i) ==
		//	Common::DataType::Array)
		//{
		//	return 1;
		//}

		//
		// 【 上のコメントの正式対応 】
		// FetchモードでのFile::get()（“オブジェクトをFetch”）では、
		// 配列フィールドと、Binary型のフィールド
		// （共にキーフィールドにはできない）は、Fetch検索対象に
		// できない。
		// しかし、“オブジェクト全体をFetch検索条件として削除／更新”
		// するときには許可する。
		//

		if (*(this->m_FetchHint.m_IsNullArray + i))
		{
			if (this->getFieldPointer(ObjectPage_,
									  ObjectAreaID_,
									  fieldIndex,
									  IsKeyObject_)
				!= 0)
			{
				return -1 * *(this->m_FetchHint.m_MultiNumberArray + i);
			}
		}
		else if (*(this->m_FetchHint.m_FieldTypeArray + i) ==
				 Common::DataType::Array)
		{
			// 配列フィールド…

			//
			// 配列フィールドをキーフィールドにはできない
			//

			; _SYDNEY_ASSERT(IsKeyObject_ == false);

			char*	objectIDReadPos =
				static_cast<char*>(
					this->getFieldPointer(ObjectPage_,
										  ObjectAreaID_,
										  fieldIndex,
										  false)); // バリューオブジェクト

			if (objectIDReadPos == 0)
			{
				// フィールド値としてヌル値が記録されていた…

				return 1 * *(this->m_FetchHint.m_MultiNumberArray + i);
			}

			ModUInt64	objectID;
			File::readObjectID(objectIDReadPos, objectID);

			compareResult =
				this->compareToArrayFetchCondition(fieldIndex,
												   i,
												   ObjectPage_,
												   AttachPages_,
												   objectID,
												   Condition_,
												   ConditionStartIndex_);

			if (compareResult != 0)
			{
				break;
			}
		}
		else if (*(this->m_FetchHint.m_IsFixedArray + i))
		{
			// 固定長フィールド…

			void*	fieldValue =
				this->getFieldPointer(ObjectPage_,
									  ObjectAreaID_,
									  fieldIndex,
									  IsKeyObject_);

			if (fieldValue == 0)
			{
				// フィールド値としてヌル値が記録されていた…

				return 1 * *(this->m_FetchHint.m_MultiNumberArray + i);
			}

			compareResult =
				this->m_FetchHint.compareToFixedCondition(fieldValue, i);

			if (compareResult != 0)
			{
				break;
			}
		}
		else
		{
			//; _SYDNEY_ASSERT(
			//	*(this->m_FetchHint.m_FieldTypeArray + i) ==
			//	Common::DataType::String);

			if (*(this->m_FetchHint.m_FieldTypeArray + i) ==
				Common::DataType::String)
			{
				// String型のフィールド…

				compareResult =
					this->compareToStringFetchCondition(fieldIndex,
														i,
														ObjectPage_,
														AttachPages_,
														ObjectAreaID_,
														Condition_,
														ConditionStartIndex_,
														IsKeyObject_);
			}
			else
			{
				// Binary型のフィールド（のはず）…

				; _SYDNEY_ASSERT(
					*(this->m_FetchHint.m_FieldTypeArray + i) ==
					Common::DataType::Binary);

				//
				// Binary型のフィールドをキーフィールドにはできない
				//

				; _SYDNEY_ASSERT(IsKeyObject_ == false);

				compareResult =
					this->compareToBinaryFetchCondition(fieldIndex,
														i,
														ObjectPage_,
														AttachPages_,
														ObjectAreaID_,
														Condition_,
														ConditionStartIndex_);
			}

			if (compareResult != 0)
			{
				break;
			}
		}
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareToStringFetchCondition --
//		Fetch検索条件と記録されているString型のフィールド値を比較する
//
//	NOTES
//	Fetch検索条件と記録されているString型のフィールド値を比較する。
//
//	ARGUMENTS
//	const int						FieldIndex_
//		String型フィールドのインデックス
//	const int						HintArrayIndex_
//		Fetchヒント配列インデックス
//	PhysicalFile::Page*				ObjectPage_
//		代表オブジェクトが記録されている物理ページの記述子
//	Btree::PageVector&				AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//	PhysicalFile::AreaID			ObjectAreaID_
//		代表オブジェクトが記録されている物理エリアの識別子
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const int						ConditionStartIndex_
//		Fetch検索条件の開始インデックス
//		（Condition_の先頭にオブジェクトIDフィールドが
//		　含まれている場合に、1を設定する。）
//	const bool						IsKeyObject_
//		比較対象のフィールドがキーフィールド、バリューフィールドのいずれか
//			true  : キーフィールド
//			false : バリューフィールド
//
//	RETURN
//	int
//		比較結果
//			> 0 : Fetch検索条件の方がキー値順で前方
//			= 0 : Fetch検索条件と記録されているフィールド値が等しい
//			< 0 : Fetch検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToStringFetchCondition(
	const int						FieldIndex_,
	const int						HintArrayIndex_,
	PhysicalFile::Page*				ObjectPage_,
	PageVector&						AttachPages_,
	PhysicalFile::AreaID			ObjectAreaID_,
	const Common::DataArrayData*	Condition_,
	const int						ConditionStartIndex_,
	const bool						IsKeyObject_) const
{
	int	compareResult = 0;

	if (*(this->m_cFileParameter.m_FieldOutsideArray + FieldIndex_))
	{
		// 外置きフィールドオブジェクト…

		// 外置きフィールドオブジェクトのオブジェクトIDへのポインタを得る
		char*	objectIDReadPos =
			static_cast<char*>(
				this->getFieldPointer(ObjectPage_,
									  ObjectAreaID_,
									  FieldIndex_,
									  IsKeyObject_));

		if (objectIDReadPos == 0)
		{
			// フィールド値としてヌル値が記録されている…

			compareResult =
				1 *
				*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_);
		}
		else
		{
			ModUInt64	objectID;
			File::readObjectID(objectIDReadPos, objectID);

			compareResult =
				this->compareToStringFetchCondition(ObjectPage_,
													AttachPages_,
													objectID,
													Condition_,
													ConditionStartIndex_,
													HintArrayIndex_);
		}
	}
	else
	{
		// 外置きではないフィールド…

		File::InsideVarFieldLen*	fieldLenReadPos =
			static_cast<File::InsideVarFieldLen*>(
				this->getFieldPointer(ObjectPage_,
									  ObjectAreaID_,
									  FieldIndex_,
									  IsKeyObject_));

		if (fieldLenReadPos == 0)
		{
			// フィールド値としてヌル値が記録されている…

			compareResult =
				1 *
				*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_);
		}
		else
		{
			compareResult =
				this->compareToStringFetchCondition(fieldLenReadPos,
													Condition_,
													ConditionStartIndex_,
													HintArrayIndex_);
		}
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareToStringFetchCondition --
//		記録されている外置きではないString型のフィールド値と
//		Fetch検索条件を比較する
//
//	NOTES
//	記録されている外置きではないString型のフィールド値と
//	Fetch検索条件を比較する。
//
//	ARGUMENTS
//	const Btree::File::InsideVarFieldLen*	FieldLengthPos_
//		フィールド長が記録されている物理エリア内の領域へのポインタ
//	const Common::DataArrayData*			Condition_
//		Fetch検索条件へのポインタ
//	const int								ConditionStartIndex_
//		Fetch検索条件の開始インデックス
//		（Condition_の先頭にオブジェクトIDフィールドが
//		　含まれている場合に、1を設定する。）
//	const int								HintArrayIndex_
//		Fetchヒント配列インデックス
//
//	RETURN
//	int
//		< 0 : Fetch検索条件の方がキー値順で前方
//		= 0 : Fetch検索条件とキーフィールド／バリューフィールドの値が等しい
//		> 0 : Fetch検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToStringFetchCondition(
	const File::InsideVarFieldLen*	FieldLengthPos_,
	const Common::DataArrayData*	Condition_,
	const int						ConditionStartIndex_,
	const int						HintArrayIndex_) const
{
	; _SYDNEY_ASSERT(FieldLengthPos_ != 0);
	; _SYDNEY_ASSERT(Condition_ != 0);

	const Common::Data*	fieldCondition =
		Condition_->getElement(ConditionStartIndex_ + HintArrayIndex_).get();

	; _SYDNEY_ASSERT(fieldCondition != 0);

	; _SYDNEY_ASSERT(
		fieldCondition->getType() == Common::DataType::String);

	const Common::StringData*	stringFieldCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::StringData*,
							 fieldCondition);

	; _SYDNEY_ASSERT(stringFieldCondition != 0);

	const ModUnicodeString&	unicodeCondition =
		stringFieldCondition->getValue();

	ModSize	numChar = *FieldLengthPos_ / sizeof(ModUnicodeChar);

	const ModUnicodeChar*	fieldValue =
		syd_reinterpret_cast<const ModUnicodeChar*>(FieldLengthPos_ + 1);

	ModUnicodeChar	forZeroByte = 0;

	if (numChar == 0)
	{
		fieldValue = &forZeroByte;
	}

	//
	// 外置きでないのなら、短い文字列なので、
	// ModUnicodeStringをいちいちコンストラクトするよりも
	// 違う領域にコピーして、比較した方が早いはず。
	// 領域は64文字分もあれば十分。
	//

	/* これはやめたい。
	ModUnicodeString	unicodeStringFieldValue(fieldValue, numChar);

	SydInfoMessage
		<< "COMPARE RESULT = "
		<< unicodeCondition.compare(unicodeStringFieldValue) *
			*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_)
		<< ModEndl;

	 return
		unicodeCondition.compare(unicodeStringFieldValue) *
		*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_);
	*/

	/* エラーが起きる！？ :?*/
	; _SYDNEY_ASSERT(numChar < 64);

	ModUnicodeChar	fieldBuffer[64 + 1];

	ModUnicodeCharTrait::copy(fieldBuffer, fieldValue, numChar);

	fieldBuffer[numChar] = 0;

	/*SydInfoMessage
		<< "COMPARE RESULT = "
		<< unicodeCondition.compare(fieldBuffer, numChar) *
			*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_)
		<< ModEndl;*/

	return
		unicodeCondition.compare(fieldBuffer) *
		*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_);
	/**/
}

//
//	FUNCTION private
//	Btree::File::compareToStringFetchCondition --
//		外置きフィールドオブジェクトに記録されているString型の
//		フィールド値とFetch検索条件を比較する
//
//	NOTES
//	外置きフィールドオブジェクトに記録されているString型の
//	フィールド値とFetch検索条件を比較する。
//
//	ARGUMENTS
//	PhysicalFile::Page*				ObjectIDPage_
//		代表オブジェクトが記録されている物理ページの記述子
//		（フィールドオブジェクトのオブジェクトIDが
//		　代表オブジェクトに記録されている。）
//	Btree::PageVector&				AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページの記述子をつむ）
//	const ModUInt64					ObjectID_
//		フィールドオブジェクトのオブジェクトID
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const int						ConditionStartIndex_
//		Fetch検索条件の開始インデックス
//		（Condition_の先頭にオブジェクトIDフィールドが
//		　含まれている場合に、1を設定する。）
//	const int						HintArrayIndex_
//		Fetchヒント配列インデックス
//
//	RETURN
//	int
//		< 0 : Fetch検索条件の方がキー値順で前方
//		= 0 : Fetch検索条件とキーフィールド／バリューフィールドの値が等しい
//		> 0 : Fetch検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToStringFetchCondition(
	PhysicalFile::Page*				ObjectIDPage_,
	PageVector&						AttachPages_,
	const ModUInt64					ObjectID_,
	const Common::DataArrayData*	Condition_,
	const int						ConditionStartIndex_,
	const int						HintArrayIndex_) const
{
	; _SYDNEY_ASSERT(Condition_ != 0);

	const Common::Data*	fieldCondition =
		Condition_->getElement(ConditionStartIndex_ + HintArrayIndex_).get();

	; _SYDNEY_ASSERT(fieldCondition != 0);

	; _SYDNEY_ASSERT(fieldCondition->getType() == Common::DataType::String);

	const Common::StringData*	stringFieldCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::StringData*, fieldCondition);

	; _SYDNEY_ASSERT(stringFieldCondition != 0);

	const ModUnicodeString&	unicodeCondition =
		stringFieldCondition->getValue();

	int	compareResult = 0;

	PhysicalFile::Page*		objectPage = 0;

	bool	attached = File::attachObjectPage(this->m_pTransaction,
											  ObjectID_,
											  ObjectIDPage_,
											  objectPage,
											  this->m_FixMode,
											  this->m_CatchMemoryExhaust,
											  AttachPages_);

	const PhysicalFile::AreaID objectAreaID =
		Common::ObjectIDData::getLatterValue(ObjectID_);

	const void*	objectAreaTop = getConstAreaTop(objectPage, objectAreaID);

	const File::ObjectType*	objectType =
		static_cast<const File::ObjectType*>(objectAreaTop);

	// check object type
	if (*objectType == File::NormalObjectType)
	{
		Os::Memory::Size	fieldLen =
			objectPage->getAreaSize(objectAreaID) -
			File::ObjectTypeArchiveSize;

		// length to number of char
		ModSize	numChar = fieldLen / sizeof(ModUnicodeChar);

		const ModUnicodeChar*	fieldValue =
			syd_reinterpret_cast<const ModUnicodeChar*>(objectType + 1);

		ModUnicodeChar	forZeroByte = 0;

		if (numChar == 0)
		{
			fieldValue = &forZeroByte;
		}

		ModUnicodeString	unicodeStringFieldValue(fieldValue, numChar);

		compareResult = unicodeCondition.compare(unicodeStringFieldValue);
	}
	else if (*objectType == File::CompressedObjectType ||
			 *objectType == File::DivideCompressedObjectType)
	{
		Common::Data::Pointer	stringFieldValue;

		File::readOutsideVariableField(this->m_pTransaction,
									   ObjectIDPage_,
									   ObjectID_,
									   Common::DataType::String,
									   stringFieldValue,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachPages_);

		compareResult =
			stringFieldCondition->compareTo(stringFieldValue.get());
	}
	else
	{
		; _SYDNEY_ASSERT(*objectType == File::DivideObjectType);

		ModUnicodeString	unicodeStringFieldValue;
		File::readOutsideStringField(this->m_pTransaction,
									 ObjectIDPage_,
									 ObjectID_,
									 unicodeStringFieldValue,
									 this->m_FixMode,
									 this->m_CatchMemoryExhaust,
									 AttachPages_);

		compareResult = unicodeCondition.compare(unicodeStringFieldValue);
	}

	if (this->m_CatchMemoryExhaust && attached)
	{
		PhysicalFile::File*	physicalFile = objectPage->getFile();

		physicalFile->detachPage(
			objectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまう
	}

	 return
		compareResult *
		*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_);
}

//
//	FUNCTION private
//	Btree::File::compareToBinaryFetchCondition --
//		Fetch検索条件と記録されているBinary型のフィールド値を比較する
//
//	NOTES
//	Fetch検索条件と記録されているBinary型のフィールド値を比較する。
//
//	ARGUMENTS
//	const int						FieldIndex_
//		フィールドインデックス
//	const int						HintArrayIndex_
//		Fetchヒント配列インデックス
//	PhysicalFile::Page*				ObjectPage_
//		代表オブジェクトが記録されているバリューページの物理ページ記述子
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	PhysicalFile::AreaID			ObjectAreaID_
//		代表オブジェクトが記録されている物理エリアの識別子
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const int						ConditionStartIndex_
//		Fetch検索条件の開始インデックス
//		（Condition_の先頭にオブジェクトIDフィールドが
//		　含まれている場合に、1を設定する。）
//
//	RETURN
//	int
//		< 0 : Fetch検索条件の方がキー値順で前方
//		= 0 : Fetch検索条件とキーフィールド／バリューフィールドの値が等しい
//		> 0 : Fetch検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToBinaryFetchCondition(
	const int						FieldIndex_,
	const int						HintArrayIndex_,
	PhysicalFile::Page*				ObjectPage_,
	PageVector&						AttachValuePages_,
	PhysicalFile::AreaID			ObjectAreaID_,
	const Common::DataArrayData*	Condition_,
	const int						ConditionStartIndex_) const
{
	int	compareResult = 0;

	if (*(this->m_cFileParameter.m_FieldOutsideArray + FieldIndex_))
	{
		// 外置きフィールドオブジェクト…

		// 外置きフィールドオブジェクトのオブジェクトIDへのポインタを得る
		char*	objectIDReadPos =
			static_cast<char*>(
				this->getFieldPointer(ObjectPage_,
									  ObjectAreaID_,
									  FieldIndex_,
									  false)); // バリューフィールド
									           // ※ Binary型のフィールドを
									           // 　 キーフィールドには
									           // 　 できない。

		if (objectIDReadPos == 0)
		{
			compareResult =
				1 *
				*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_);
		}
		else
		{
			ModUInt64	objectID;
			File::readObjectID(objectIDReadPos, objectID);

			compareResult =
				this->compareToBinaryFetchCondition(ObjectPage_,
													AttachValuePages_,
													objectID,
													Condition_,
													ConditionStartIndex_,
													HintArrayIndex_);
		}
	}
	else
	{
		// 外置きではないフィールド…

		File::InsideVarFieldLen*	fieldLenReadPos =
			static_cast<File::InsideVarFieldLen*>(
				this->getFieldPointer(ObjectPage_,
									  ObjectAreaID_,
									  FieldIndex_,
									  false)); // バリューフィールド

		if (fieldLenReadPos == 0)
		{
			// フィールド値としてヌル値が記録されている…

			compareResult =
				1 *
				*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_);
		}
		else
		{
			compareResult =
				this->compareToBinaryFetchCondition(fieldLenReadPos,
													Condition_,
													ConditionStartIndex_,
													HintArrayIndex_);
		}
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareToBinaryFetchCondition --
//		記録されている外置きではないBinary型のフィールド値と
//		Fetch検索条件を比較する
//
//	NOTES
//	記録されている外置きではないBinary型のフィールド値と
//	Fetch検索条件を比較する。
//
//	ARGUMENTS
//	const Btree::File::InsideVarFieldLen*	FieldLengthPos_
//		フィールド長が記録されている物理エリア内の領域へのポインタ
//	const Common::DataArrayData*			Condition_
//		Fetch検索条件へのポインタ
//	const int								ConditionStartIndex_
//		Fetch検索条件の開始インデックス
//		（Condition_の先頭にオブジェクトIDフィールドが
//		　含まれている場合に、1を設定する。）
//	const int								HintArrayIndex_
//		Fetchヒント配列インデックス
//
//	RETURN
//	int
//		< 0 : Fetch検索条件の方がキー値順で前方
//		= 0 : Fetch検索条件とキーフィールド／バリューフィールドの値が等しい
//		> 0 : Fetch検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToBinaryFetchCondition(
	const InsideVarFieldLen*		FieldLengthPos_,
	const Common::DataArrayData*	Condition_,
	const int						ConditionStartIndex_,
	const int						HintArrayIndex_) const
{
	; _SYDNEY_ASSERT(FieldLengthPos_ != 0);
	; _SYDNEY_ASSERT(Condition_ != 0);

	const Common::Data*	fieldCondition =
		Condition_->getElement(ConditionStartIndex_ + HintArrayIndex_).get();

	; _SYDNEY_ASSERT(fieldCondition != 0);

	; _SYDNEY_ASSERT(
		fieldCondition->getType() == Common::DataType::Binary);

	const Common::BinaryData*	binaryFieldCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*,
							 fieldCondition);

	; _SYDNEY_ASSERT(binaryFieldCondition != 0);

	const void*	fieldValue = FieldLengthPos_ + 1;

	Common::BinaryData	dstBinaryData(fieldValue, *FieldLengthPos_);

	return
		binaryFieldCondition->compareTo(&dstBinaryData) *
		*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_);
}

//
//	FUNCTION private
//	Btree::File::compareToBinaryFetchCondition --
//		外置きフィールドオブジェクトに記録されているBinary型の
//		フィールド値とFetch検索条件を比較する
//
//	NOTES
//	外置きフィールドオブジェクトに記録されているBinary型の
//	フィールド値とFetch検索条件を比較する。
//
//	ARGUMENTS
//	PhysicalFile::Page*				ObjectIDPage_,
//		代表オブジェクトが記録されているバリューページの物理ページ記述子
//		（フィールドオブジェクトのオブジェクトIDが
//		　代表オブジェクトに記録されている。）
//	Btree::PageVector&				AttachValuePages_,
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const ModUInt64					ObjectID_,
//		フィールドオブジェクトのオブジェクトID
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const int						ConditionStartIndex_
//		Fetch検索条件の開始インデックス
//		（Condition_の先頭にオブジェクトIDフィールドが
//		　含まれている場合に、1を設定する。）
//	const int						HintArrayIndex_
//		Fetchヒント配列インデックス
//
//	RETURN
//	int
//		< 0 : Fetch検索条件の方がキー値順で前方
//		= 0 : Fetch検索条件とキーフィールド／バリューフィールドの値が等しい
//		> 0 : Fetch検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToBinaryFetchCondition(
	PhysicalFile::Page*				ObjectIDPage_,
	PageVector&						AttachValuePages_,
	const ModUInt64					ObjectID_,
	const Common::DataArrayData*	Condition_,
	const int						ConditionStartIndex_,
	const int						HintArrayIndex_) const
{
	; _SYDNEY_ASSERT(Condition_ != 0);

	const Common::Data*	fieldCondition =
		Condition_->getElement(ConditionStartIndex_ + HintArrayIndex_).get();

	; _SYDNEY_ASSERT(fieldCondition != 0);

	; _SYDNEY_ASSERT(fieldCondition->getType() == Common::DataType::Binary);

	const Common::BinaryData*	binaryFieldCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, fieldCondition);

	; _SYDNEY_ASSERT(binaryFieldCondition != 0);

	int	compareResult = 0;

	PhysicalFile::Page*	objectPage = 0;

	bool	attached = File::attachObjectPage(this->m_pTransaction,
											  ObjectID_,
											  ObjectIDPage_,
											  objectPage,
											  this->m_FixMode,
											  this->m_CatchMemoryExhaust,
											  AttachValuePages_);

	const PhysicalFile::AreaID objectAreaID =
		Common::ObjectIDData::getLatterValue(ObjectID_);

	const void*	objectAreaTop = getConstAreaTop(objectPage, objectAreaID);

	const File::ObjectType*	objectType =
		static_cast<const File::ObjectType*>(objectAreaTop);

	if (*objectType == File::NormalObjectType)
	{
		Os::Memory::Size	fieldLen =
			objectPage->getAreaSize(objectAreaID) -
			File::ObjectTypeArchiveSize;

		const void*	fieldValue = objectType + 1;

		Common::BinaryData	dstBinaryData(fieldValue, fieldLen);

		compareResult = binaryFieldCondition->compareTo(&dstBinaryData);
	}
	else if (*objectType == File::CompressedObjectType ||
			 *objectType == File::DivideCompressedObjectType)
	{
		Common::Data::Pointer	dstData;

		File::readOutsideVariableField(this->m_pTransaction,
									   ObjectIDPage_,
									   ObjectID_,
									   Common::DataType::Binary,
									   dstData,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachValuePages_);

		compareResult = binaryFieldCondition->compareTo(dstData.get());
	}
	else
	{
		; _SYDNEY_ASSERT(*objectType == File::DivideObjectType);

		Os::Memory::Size	fieldLen =
			File::getOutsideBinaryFieldLength(this->m_pTransaction,
											  ObjectIDPage_,
											  ObjectID_,
											  this->m_FixMode,
											  this->m_CatchMemoryExhaust,
											  AttachValuePages_);

		void*	binaryBuffer = ModDefaultManager::allocate(fieldLen);

		File::readOutsideBinaryField(this->m_pTransaction,
									 ObjectIDPage_,
									 ObjectID_,
									 static_cast<char*>(binaryBuffer),
									 0, // バッファのインデックス
									 this->m_FixMode,
									 this->m_CatchMemoryExhaust,
									 AttachValuePages_);

		Common::BinaryData	dstBinaryData(binaryBuffer, fieldLen);

		compareResult = binaryFieldCondition->compareTo(&dstBinaryData);
	}

	if (this->m_CatchMemoryExhaust && attached)
	{
		PhysicalFile::File*	physicalFile = objectPage->getFile();

		physicalFile->detachPage(
			objectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）して、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	return
		compareResult *
		*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_);
}

//
//	FUNCTION private
//	Btree::File::compareToArrayFetchCondition --
//		Fetch検索条件と記録されている配列フィールドの値を比較する
//
//	NOTES
//	Fetch検索条件と記録されている配列フィールドの値を比較する。
//
//	ARGUMENTS
//	const int						FieldIndex_
//		フィールドインデックス
//	const int						HintArrayIndex_
//		Fetchヒント配列インデックス
//	PhysicalFile::Page*				DirectObjectPage_
//		代表オブジェクトが記録されているバリューページの物理ページ記述子
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const ModUInt64					ObjectID_
//		配列フィールドオブジェクトのオブジェクトID
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const int						ConditionStartIndex_
//		Fetch検索条件の開始インデックス
//		（Condition_の先頭にオブジェクトIDフィールドが
//		　含まれている場合に、1を設定する。）
//
//	RETURN
//	int
//		< 0 : Fetch検索条件の方がキー値順で前方
//		= 0 : Fetch検索条件とキーフィールド／バリューフィールドの値が等しい
//		> 0 : Fetch検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToArrayFetchCondition(
	const int						FieldIndex_,
	const int						HintArrayIndex_,
	PhysicalFile::Page*				DirectObjectPage_,
	PageVector&						AttachValuePages_,
	const ModUInt64					ObjectID_,
	const Common::DataArrayData*	Condition_,
	const int						ConditionStartIndex_) const
{
	; _SYDNEY_ASSERT(Condition_ != 0);

	const Common::Data*	fieldCondition =
		Condition_->getElement(ConditionStartIndex_ + HintArrayIndex_).get();

	; _SYDNEY_ASSERT(fieldCondition != 0);

	; _SYDNEY_ASSERT(fieldCondition->getType() == Common::DataType::Array);

	const Common::DataArrayData*	arrayFieldCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, fieldCondition);

	; _SYDNEY_ASSERT(arrayFieldCondition != 0);

	Common::DataType::Type	elementDataType =
		*(this->m_cFileParameter.m_ElementTypeArray + FieldIndex_);

	Common::DataArrayData	dstArrayField;

	this->m_ValueFile->readArrayField(DirectObjectPage_,
									  ObjectID_,
									  elementDataType,
									  dstArrayField,
									  this->m_CatchMemoryExhaust,
									  AttachValuePages_);

	return
		arrayFieldCondition->compareTo(&dstArrayField) *
		*(this->m_FetchHint.m_MultiNumberArray + HintArrayIndex_);
}

//
//	FUNCTION private
//	Btree::File::fetchByKeyRev --
//		Fetch検索条件でオブジェクトを検索する
//
//	NOTES
//	Fetch検索条件でオブジェクトを検索し、
//	該当するオブジェクトが存在すれば
//	そのオブジェクトのオブジェクトIDを返す。
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
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		Fetch検索条件と一致するオブジェクトのオブジェクトID
//		一致するオブジェクトがファイル内に存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::fetchByKeyRev(const ModUInt32				TreeDepth_,
					const PhysicalFile::PageID	RootNodePageID_,
					PageVector&					AttachNodePages_,
					ValueFile*					ValueFile_,
					PageVector&					AttachValuePages_)
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->fetchBySimpleKeyRev(TreeDepth_,
										 RootNodePageID_,
										 AttachNodePages_,
										 ValueFile_,
										 AttachValuePages_);
	}

	; _SYDNEY_ASSERT(this->m_Searched == false);

	; _SYDNEY_ASSERT(this->m_FetchOptionData.get() != 0);

	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getType() == Common::DataType::Array);
	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getElementType() == Common::DataType::Data);

	const Common::DataArrayData*	fetchCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 this->m_FetchOptionData.get());

	; _SYDNEY_ASSERT(fetchCondition != 0);

	this->m_FetchHint.setCondition(fetchCondition);

	PhysicalFile::Page*	leafPage =
		this->searchLeafPageForFetch(TreeDepth_,
									 RootNodePageID_,
									 AttachNodePages_,
									 fetchCondition);

	if (leafPage == 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	bool	match = false;

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	int	keyInfoIndex =
		this->getKeyInformationIndexForFetch(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			fetchCondition,
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

	const ModUInt32 useKeyInfoNum =
		leafPageHeader.readUseKeyInformationNumber();

	if (static_cast<ModUInt32>(keyInfoIndex) == useKeyInfoNum - 1)
	{
		// 次のリーフページにも検索条件に一致するオブジェクトが
		// 存在する可能性がある…

		PhysicalFile::Page*	leafPageForNext = leafPage;

		PhysicalFile::PageID	leafPageIDForNext =
			leafPageForNext->getID();

		KeyInformation	keyInfoForNext(this->m_pTransaction,
									   leafPage,
									   keyInfoIndex,
									   true); // リーフページ

		bool	exist = false;

		while (this->assignNextKeyInformation(leafPageForNext,
											  AttachNodePages_,
											  leafPageHeader,
											  keyInfoForNext))
		{
			if (leafPageForNext->getID() != leafPageIDForNext)
			{
				leafPageIDForNext = leafPageForNext->getID();

				leafPageHeader.resetPhysicalPage(leafPageForNext);
			}

			ModUInt64	keyObjectID = keyInfoForNext.readKeyObjectID();

			if (this->compareToFetchCondition(leafPageForNext,
											  AttachNodePages_,
											  keyObjectID,
											  fetchCondition)
				!= 0)
			{
				break;
			}

			if (exist == false)
			{
				exist = true;
			}
		}

		if (exist)
		{
			bool	status =
				this->assignPrevKeyInformation(leafPageForNext,
											   AttachNodePages_,
											   leafPageHeader,
											   keyInfoForNext);

			; _SYDNEY_ASSERT(status);

			if (leafPage != leafPageForNext)
			{
				leafPage = leafPageForNext;

				leafPageHeader.resetPhysicalPage(leafPage);

				keyInfo.resetPhysicalPage(leafPage);
			}

			keyInfo.setStartOffsetByIndex(keyInfoForNext.getIndex());
		}
		else if (leafPageForNext == 0)
		{
			leafPage = File::attachPage(this->m_pTransaction,
										this->m_pPhysicalFile,
										leafPageIDForNext,
										this->m_FixMode,
										this->m_CatchMemoryExhaust,
										AttachNodePages_);

			keyInfo.resetPhysicalPage(leafPage);

			keyInfo.setStartOffsetByIndex(keyInfoIndex);
		}
	}

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	keyInfoIndex = keyInfo.getIndex();

	ModUInt64	valueObjectID = keyInfo.readValueObjectID();

	if (this->m_FetchHint.m_ExistSeparateKey)
	{
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

		if (compareToFetchCondition(
				keyObjectPage, AttachNodePages_,
				Common::ObjectIDData::getLatterValue(keyObjectID),
				fetchCondition,
				FetchHint::CompareType::OnlySeparateKey,
				true) // キーオブジェクト
			!= 0)
		{
			valueObjectID =
				this->getPrevObjectIDFetchByKey(leafPage,
												keyInfo,
												AttachNodePages_,
												ValueFile_,
												AttachValuePages_,
												fetchCondition);

			if (valueObjectID == FileCommon::ObjectID::Undefined)
			{
				if (leafPage == 0 || leafPage->getID() != leafPageID)
				{
					checkMemoryExhaust(leafPage);

					leafPage = File::attachPage(this->m_pTransaction,
												this->m_pPhysicalFile,
												leafPageID,
												this->m_FixMode,
												this->m_CatchMemoryExhaust,
												AttachNodePages_);

					keyInfo.resetPhysicalPage(leafPage);
				}

				keyInfo.setStartOffsetByIndex(keyInfoIndex);

				valueObjectID =
					this->getNextObjectIDFetchByKey(leafPage,
													keyInfo,
													AttachNodePages_,
													ValueFile_,
													AttachValuePages_,
													fetchCondition);
			}
		}

		if (attached)
		{
			checkMemoryExhaust(keyObjectPage);
		}

		if (valueObjectID == FileCommon::ObjectID::Undefined)
		{
			checkMemoryExhaust(leafPage);

			return valueObjectID;
		}
	}

	; _SYDNEY_ASSERT(valueObjectID != FileCommon::ObjectID::Undefined);

	if (this->m_FetchHint.m_OnlyKey == false)
	{
		PhysicalFile::Page*	valuePage =	File::attachPage(
			m_pTransaction, ValueFile_->m_PhysicalFile,
			Common::ObjectIDData::getFormerValue(valueObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

		; _SYDNEY_ASSERT(valuePage != 0);

		if (this->compareToFetchCondition(
				valuePage,
				AttachValuePages_,
				Common::ObjectIDData::getLatterValue(valueObjectID),
				fetchCondition,
				FetchHint::CompareType::OnlyValue,
				false) // バリューオブジェクト
			!= 0)
		{
			ModUInt64	saveValueObjectID = valueObjectID;
			valueObjectID =
				this->getPrevObjectIDFetchByKey(valuePage,
												AttachNodePages_,
												ValueFile_,
												AttachValuePages_,
												valueObjectID,
												fetchCondition);

			if (valueObjectID == FileCommon::ObjectID::Undefined)
			{
				valueObjectID =
					this->getNextObjectIDFetchByKey(valuePage,
													AttachNodePages_,
													ValueFile_,
													AttachValuePages_,
													saveValueObjectID,
													fetchCondition);
			}
		}

		if (this->m_CatchMemoryExhaust)
		{
			ValueFile_->m_PhysicalFile->detachPage(
				valuePage,
				PhysicalFile::Page::UnfixMode::NotDirty,
				false); // 本当にデタッチ（アンフィックス）してしまう
		}
	}

	checkMemoryExhaust(leafPage);

	return valueObjectID;
}

#ifdef OBSOLETE // 将来に対する予約
//
//	FUNCTION private
//	Btree::File::convertToObjectID --
//		引数からオブジェクトIDを取り出し、返す
//
//	NOTES
//	引数CommonData_からバリューオブジェクトのオブジェクトIDを取り出し、
//	返す。
//
//	ARGUMENTS
//	const Common::Data*	CommonData_
//		オブジェクトIDを値として持つオブジェクトへのポインタ
//
//	RETURN
//	ModUInt64
//		バリューオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//	[YET!]
//
// static
ModUInt64
File::convertToObjectID(const Common::Data* CommonData_)
{
	const LogicalFile::ObjectID* valueObjectIDData = 0;

	Common::DataType::Type	dataType = CommonData_->getType();

	if (dataType == Common::DataType::Array)
	{
		// 配列内の先頭の要素でオブジェクトIDが指定されている…

		const Common::DataArrayData*	arrayData =
			_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
								 CommonData_);

		; _SYDNEY_ASSERT(arrayData->getCount() > 0);

		const Common::Data*	firstElement =
			(arrayData->getElement(0)).get();

		; _SYDNEY_ASSERT(
			firstElement->getType() == LogicalFile::ObjectID().getType());

		valueObjectIDData =
			_SYDNEY_DYNAMIC_CAST(const LogicalFile::ObjectID*,
								 firstElement);
	}
	else if (dataType == LogicalFile::ObjectID().getType())
	{
		// オブジェクトIDが直接指定された…

		valueObjectIDData =
			_SYDNEY_DYNAMIC_CAST(const LogicalFile::ObjectID*,
								 CommonData_);
	}
	else
	{
		// 対応していない形でオブジェクトIDが記録されているらしい…

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	const ModUInt64 valueObjectID = valueObjectIDData.getValue();

	if (valueObjectID == FileCommon::ObjectID::Undefined)
	{
		// オブジェクトIDの値がおかしい…

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	return valueObjectID;
}
#endif

//
//	FUNCTION private
//	File::getNextObjectIDFetchByKey --
//		Fetch検索条件と一致する次のオブジェクトのIDを返す
//
//	NOTES
//	Fetch検索条件と一致する次のオブジェクトのを検索し、
//	該当するオブジェクトが存在すれば、そのオブジェクトのIDを返す。
//	“次のオブジェクト”とは、
//	前回の、Fetch検索条件と一致するオブジェクトを基準として、
//	キー値順に後方のオブジェクトのことを指す。
//	オープン時に、オブジェクトの挿入ソート順で
//	オブジェクトを取得するように設定されている場合に、
//	呼び出される。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*	ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&	AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		Fetch検索条件と一致するオブジェクトのID
//		一致するオブジェクトがファイル内に存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDFetchByKey(PageVector&	AttachNodePages_,
								ValueFile*	ValueFile_,
								PageVector&	AttachValuePages_) const
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

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getType() == Common::DataType::Array);
	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getElementType() == Common::DataType::Data);

	const Common::DataArrayData*	fetchCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 this->m_FetchOptionData.get());

	; _SYDNEY_ASSERT(fetchCondition != 0);

	ModUInt64	valueObjectID =
		this->getNextObjectIDFetchByKey(leafPage,
										keyInfo,
										AttachNodePages_,
										ValueFile_,
										AttachValuePages_,
										fetchCondition);

	checkMemoryExhaust(leafPage);

	return valueObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDFetchByKey --
//		Fetch検索条件と一致する次のオブジェクトのオブジェクトIDを返す
//
//	NOTES
//	Fetch検索条件と一致する次のオブジェクトを検索し、
//	該当するオブジェクトが存在すれば
//	そのオブジェクトのオブジェクトIDを返す。
//	“次のオブジェクト”とは、
//	前回の、Fetch検索条件と一致するオブジェクトを基準として、
//	キー値順に後方のオブジェクトのことを指す。
//	オープン時に、オブジェクトの挿入ソート順で
//	オブジェクトを取得するように設定されている場合に、
//	呼び出される。
//
//	ARGUMENTS
//	PhysicalFile::Page*				BeforeValuePage_
//		前回の、Fetch検索条件と一致するオブジェクトが記録されている
//		オブジェクトが記録されているバリューページの物理ページ記述子
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*				ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const ModUInt64					BeforeValueObjectID_
//		前回の、Fetch検索条件と一致するオブジェクトのオブジェクトID
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//
//	RETURN
//	ModUInt64
//		Fetch検索条件と一致するオブジェクトのオブジェクトID
//		一致するオブジェクトがファイル内に存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDFetchByKey(
	PhysicalFile::Page*				BeforeValuePage_,
	PageVector&						AttachNodePages_,
	ValueFile*						ValueFile_,
	PageVector&						AttachValuePages_,
	const ModUInt64					BeforeValueObjectID_,
	const Common::DataArrayData*	Condition_) const
{
	PhysicalFile::PageID	leafPageID =
		PhysicalFile::ConstValue::UndefinedPageID;

	ModUInt32	keyInfoIndex = ModUInt32Max;

	const void*	objectAreaTop = getConstAreaTop(
		BeforeValuePage_,
		Common::ObjectIDData::getLatterValue(BeforeValueObjectID_));

	ValueFile_->readLeafInfo(objectAreaTop,
							 leafPageID,
							 keyInfoIndex);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 leafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true); // リーフページ

	ModUInt64	valueObjectID =
		this->getNextObjectIDFetchByKey(leafPage,
										keyInfo,
										AttachNodePages_,
										ValueFile_,
										AttachValuePages_,
										Condition_);

	checkMemoryExhaust(leafPage);

	return valueObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDFetchByKey --
//		Fetch検索条件と一致する次のオブジェクトのオブジェクトIDを返す
//
//	NOTES
//	Fetch検索条件と一致する次のオブジェクトのを検索し、
//	該当するオブジェクトが存在すれば
//	そのオブジェクトのオブジェクトIDを返す。
//	“次のオブジェクト”とは、
//	前回の、Fetch検索条件と一致するオブジェクトを基準として、
//	キー値順に後方のオブジェクトのことを指す。
//	オープン時に、オブジェクトの挿入ソート順で
//	オブジェクトを取得するように設定されている場合に、
//	呼び出される。
//
//	ARGUMENTS
//	PhysicalFile::Page*&			LeafPage_
//		前回の、Fetch検索条件と一致するオブジェクトへ辿ることができる
//		キー情報が記録されているリーフページの物理ページ記述子
//	Btree::KeyInformation&			KeyInfo_
//		前回の、Fetch検索条件と一致するオブジェクトへ辿ることができる
//		キー情報への参照
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*				ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//
//	RETURN
//	ModUInt64
//		Fetch検索条件と一致するオブジェクトのオブジェクトID
//		一致するオブジェクトがファイル内に存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDFetchByKey(
	PhysicalFile::Page*&			LeafPage_,
	KeyInformation&					KeyInfo_,
	PageVector&						AttachNodePages_,
	ValueFile*						ValueFile_,
	PageVector&						AttachValuePages_,
	const Common::DataArrayData*	Condition_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->getNextObjectIDFetchBySimpleKey(LeafPage_,
													 KeyInfo_,
													 AttachNodePages_,
													 ValueFile_,
													 AttachValuePages_,
													 Condition_);
	}

	PhysicalFile::PageID	leafPageID = LeafPage_->getID();

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   LeafPage_,
								   true); // リーフページ

	PhysicalFile::Page*	valuePage = 0;

	ModUInt64	valueObjectID = FileCommon::ObjectID::Undefined;

	while (this->assignNextKeyInformation(LeafPage_,
										  AttachNodePages_,
										  leafPageHeader,
										  KeyInfo_))
	{
		if (LeafPage_->getID() != leafPageID)
		{
			leafPageID = LeafPage_->getID();
		}

		ModUInt64	keyObjectID = KeyInfo_.readKeyObjectID();

		if (this->compareToFetchCondition(LeafPage_,
										  AttachNodePages_,
										  keyObjectID,
										  Condition_)
			!= 0)
		{
			break;
		}

		if (this->m_FetchHint.m_ExistSeparateKey)
		{
			PhysicalFile::Page*		keyObjectPage = 0;

			bool	attached =
				File::attachObjectPage(this->m_pTransaction,
									   keyObjectID,
									   LeafPage_,
									   keyObjectPage,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachNodePages_);

			if (compareToFetchCondition(
					keyObjectPage, AttachNodePages_,
					Common::ObjectIDData::getLatterValue(keyObjectID),
					Condition_,
					FetchHint::CompareType::OnlySeparateKey,
					true) // キーオブジェクト
				!= 0)
			{
				continue;
			}

			if (attached)
			{
				checkMemoryExhaust(keyObjectPage);
			}
		}

		ModUInt64	srcValueObjectID = KeyInfo_.readValueObjectID();

		if (this->m_FetchHint.m_OnlyKey)
		{
			valueObjectID = srcValueObjectID;

			break;
		}
		
		valuePage =	File::attachPage(
			m_pTransaction,	ValueFile_->m_PhysicalFile,
			Common::ObjectIDData::getFormerValue(srcValueObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

		if (this->compareToFetchCondition(
			valuePage,
			AttachValuePages_,
			Common::ObjectIDData::getLatterValue(srcValueObjectID),
			Condition_,
			FetchHint::CompareType::OnlyValue,
			false) // バリューオブジェクト
			== 0)
		{
			valueObjectID = srcValueObjectID;

			break;
		}

		if (this->m_CatchMemoryExhaust)
		{
			ValueFile_->m_PhysicalFile->detachPage(valuePage,PhysicalFile::Page::UnfixMode::NotDirty,false); // 本当にデタッチ（アンフィックス）してしまう

			valuePage = 0;
		}
	}

	checkMemoryExhaust(LeafPage_); // 本当にデタッチ（アンフィックス）してしまう

	if (this->m_CatchMemoryExhaust)	// MemoryExhaust ならデタッチ（アンフィックス）されている
	{
		LeafPage_ = 0;
	}

	if (this->m_CatchMemoryExhaust && valuePage != 0)
	{
		ValueFile_->m_PhysicalFile->detachPage(valuePage,PhysicalFile::Page::UnfixMode::NotDirty,false); // 本当にデタッチ（アンフィックス）してしまう
	}

	return valueObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDFetchByKey --
//		Fetch検索条件と一致する前のオブジェクトのIDを返す
//
//	NOTES
//	Fetch検索条件と一致する前のオブジェクトを検索し、
//	該当するオブジェクトが存在すれば、そのオブジェクトのIDを返す。
//	“前のオブジェクト”とは、
//	前回の、Fetch検索条件と一致するオブジェクトを基準として、
//	キー値順に前方のオブジェクトのことを指す。
//	オープン時に、オブジェクトの挿入ソート順の逆順で
//	オブジェクトを取得するように設定されている場合に、
//	呼び出される。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*	ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&	AttachValuePages_
//		バリューページベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		Fetch検索条件と一致するオブジェクトのID。
//		一致するオブジェクトがファイル内に存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDFetchByKey(PageVector&	AttachNodePages_,
								ValueFile*	ValueFile_,
								PageVector&	AttachValuePages_) const
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

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getType() == Common::DataType::Array);
	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getElementType() == Common::DataType::Data);

	const Common::DataArrayData*	fetchCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 this->m_FetchOptionData.get());

	; _SYDNEY_ASSERT(fetchCondition != 0);

	ModUInt64	valueObjectID =
		this->getPrevObjectIDFetchByKey(leafPage,
										keyInfo,
										AttachNodePages_,
										ValueFile_,
										AttachValuePages_,
										fetchCondition);

	checkMemoryExhaust(leafPage);

	return valueObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDFetchByKey --
//		Fetch検索条件と一致する前のオブジェクトのオブジェクトIDを返す
//
//	NOTES
//	Fetch検索条件と一致する前のオブジェクトを検索し、
//	該当するオブジェクトが存在すれば
//	そのオブジェクトのオブジェクトIDを返す。
//	“前のオブジェクト”とは、
//	前回の、Fetch検索条件と一致するオブジェクトを基準として、
//	キー値順に前方のオブジェクトのことを指す。
//	オープン時に、オブジェクトの挿入ソート順の逆順で
//	オブジェクトを取得するように設定されている場合に、
//	呼び出される。
//
//	ARGUMENTS
//	PhysicalFile::Page*				BeforeValuePage_
//		前回の、Fetch検索条件と一致する
//		バリューオブジェクトが記録されている
//		バリューページの物理ページ記述子
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*				ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const ModUInt64					BeforeValueObjectID_
//		前回の、Fetch検索条件と一致する
//		バリューオブジェクトのオブジェクトID
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//
//	RETURN
//	ModUInt64
//		Fetch検索条件と一致するオブジェクトのオブジェクトID
//		一致するオブジェクトがファイル内に存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDFetchByKey(
	PhysicalFile::Page*				BeforeValuePage_,
	PageVector&						AttachNodePages_,
	ValueFile*						ValueFile_,
	PageVector&						AttachValuePages_,
	const ModUInt64					BeforeValueObjectID_,
	const Common::DataArrayData*	Condition_) const
{
	PhysicalFile::PageID	leafPageID =
		PhysicalFile::ConstValue::UndefinedPageID;

	ModUInt32	keyInfoIndex = ModUInt32Max;

	const void*	objectAreaTop = getConstAreaTop(
		BeforeValuePage_,
		Common::ObjectIDData::getLatterValue(BeforeValueObjectID_));

	ValueFile_->readLeafInfo(objectAreaTop,
							 leafPageID,
							 keyInfoIndex);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 leafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true); // リーフページ

	ModUInt64	valueObjectID =
		this->getPrevObjectIDFetchByKey(leafPage,
										keyInfo,
										AttachNodePages_,
										ValueFile_,
										AttachValuePages_,
										Condition_);

	checkMemoryExhaust(leafPage);

	return valueObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDFetchByKey --
//		Fetch検索条件と一致する前のオブジェクトのオブジェクトIDを返す
//
//	NOTES
//	Fetch検索条件と一致する前のオブジェクトを検索し、
//	該当するオブジェクトが存在すれば
//	そのオブジェクトのオブジェクトIDを返す。
//	“前のオブジェクト”とは、
//	前回の、Fetch検索条件と一致するオブジェクトを基準として、
//	キー値順に前方のオブジェクトのことを指す。
//	オープン時に、オブジェクトの挿入ソート順の逆順で
//	オブジェクトを取得するように設定されている場合に、
//	呼び出される。
//
//	ARGUMENTS
//	PhysicalFile::Page*&			LeafPage_
//		前回の、Fetch検索条件と一致するオブジェクトへ辿ることができる
//		キー情報が記録されているリーフページの物理ページ記述子
//	Btree::KeyInformation&			KeyInfo_
//		前回の、Fetch検索条件と一致するオブジェクトへ辿ることができる
//		キー情報への参照
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//	Btree::ValueFile*				ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//
//	RETURN
//	ModUInt64
//		Fetch検索条件と一致するオブジェクトのオブジェクトID
//		一致するオブジェクトがファイル内に存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDFetchByKey(
	PhysicalFile::Page*&			LeafPage_,
	KeyInformation&					KeyInfo_,
	PageVector&						AttachNodePages_,
	ValueFile*						ValueFile_,
	PageVector&						AttachValuePages_,
	const Common::DataArrayData*	Condition_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->getPrevObjectIDFetchBySimpleKey(LeafPage_,
													 KeyInfo_,
													 AttachNodePages_,
													 ValueFile_,
													 AttachValuePages_,
													 Condition_);
	}

	PhysicalFile::PageID	leafPageID = LeafPage_->getID();

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   LeafPage_,
								   true); // リーフページ

	PhysicalFile::Page*	valuePage = 0;

	ModUInt64	valueObjectID = FileCommon::ObjectID::Undefined;

	while (this->assignPrevKeyInformation(LeafPage_,
										  AttachNodePages_,
										  leafPageHeader,
										  KeyInfo_))
	{
		if (LeafPage_->getID() != leafPageID)
		{
			leafPageID = LeafPage_->getID();
		}

		ModUInt64	keyObjectID = KeyInfo_.readKeyObjectID();

		if (this->compareToFetchCondition(LeafPage_,
										  AttachNodePages_,
										  keyObjectID,
										  Condition_)
			!= 0)
		{
			break;
		}

		if (this->m_FetchHint.m_ExistSeparateKey)
		{
			PhysicalFile::Page*		keyObjectPage = 0;

			bool	attached =
				File::attachObjectPage(this->m_pTransaction,
									   keyObjectID,
									   LeafPage_,
									   keyObjectPage,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachNodePages_);

			if (compareToFetchCondition(
					keyObjectPage, AttachNodePages_,
					Common::ObjectIDData::getLatterValue(keyObjectID),
					Condition_,
					FetchHint::CompareType::OnlySeparateKey,
					true) // キーオブジェクト
				!= 0)
			{
				continue;
			}

			if (attached)
			{
				checkMemoryExhaust(keyObjectPage);
			}
		}

		ModUInt64	srcValueObjectID = KeyInfo_.readValueObjectID();

		if (this->m_FetchHint.m_OnlyKey)
		{
			valueObjectID = srcValueObjectID;

			break;
		}
		
		valuePage =	File::attachPage(
			m_pTransaction, ValueFile_->m_PhysicalFile,
			Common::ObjectIDData::getFormerValue(srcValueObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

		if (this->compareToFetchCondition(
			valuePage,
			AttachValuePages_,
			Common::ObjectIDData::getLatterValue(srcValueObjectID),
			Condition_,
			FetchHint::CompareType::OnlyValue,
			false) // バリューオブジェクト
			== 0)
		{
			valueObjectID = srcValueObjectID;

			break;
		}

		if (this->m_CatchMemoryExhaust)
		{
			ValueFile_->m_PhysicalFile->detachPage(valuePage,PhysicalFile::Page::UnfixMode::NotDirty,false); // 本当にデタッチ（アンフィックス）してしまう

			valuePage = 0;
		}
	}

	checkMemoryExhaust(LeafPage_);

	if (this->m_CatchMemoryExhaust && valuePage != 0)
	{
		ValueFile_->m_PhysicalFile->detachPage(valuePage,PhysicalFile::Page::UnfixMode::NotDirty,false); // 本当にデタッチ（アンフィックス）してしまう
	}

	if (this->m_CatchMemoryExhaust)
	{
		LeafPage_ = 0;
	}

	return valueObjectID;
}

#ifdef OBSOLETE // 将来に対する予約
//
//	FUNCTION private
//	Btree::File::fetchByOID -- オブジェクトIDでオブジェクトを検索する
//
//	NOTES
//	オブジェクトIDでオブジェクトを検索する。
//	clustered indexでB+木をレコードファイルのように使用するという、
//	つまり、あるBtreeファイルのバリュー値として、異なるBtreeファイルの
//	オブジェクトIDを挿入するような、そのような形態をとったときに使用される。
//
//	ARGUMENTS
//	Btree::ValueFile*	ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&	AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		Fetch検索条件と一致するオブジェクトIDのオブジェクトが
//		存在すればそのオブジェクトID。
//		存在しない場合には、FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::fetchByOID(ValueFile*		ValueFile_,
				 PageVector&	AttachValuePages_)
{
	; _SYDNEY_ASSERT(this->m_FetchOptionData.get() != 0);

	; _SYDNEY_ASSERT(this->m_Searched == false);

	ModUInt64	valueObjectID = FileCommon::ObjectID::Undefined;

	try
	{
		valueObjectID =
			this->convertToObjectID(this->m_FetchOptionData.get());
	}
	catch (Exception::BadArgument&)
	{
		// オブジェクトIDでFetchする場合、
		// 代表オブジェクトではなかったり、
		// オブジェクトが存在しないときには、
		// Ｂ＋木ファイルは例外を投げるのではなく、
		// ヌルポインタを返す。

		return FileCommon::ObjectID::Undefined;
	}

	PhysicalFile::Page*	valuePage =	File::attachPage(this->m_pTransaction,
		ValueFile_->m_PhysicalFile,
		Common::ObjectIDData::getFormerValue(valueObjectID),
		m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

	if (!isDirectObject(
			valuePage, Common::ObjectIDData::getLatterValue(valueObjectID)))
	{
		// 上のコメントの通りである。

		valueObjectID = FileCommon::ObjectID::Undefined;
	}

	if (this->m_CatchMemoryExhaust)
	{
		ValueFile_->m_PhysicalFile->detachPage(
			valuePage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまう
	}

	return valueObjectID;
}
#endif

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
