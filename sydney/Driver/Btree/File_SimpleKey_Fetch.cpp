// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_SimpleKey_Fetch.cpp --
//		Ｂ＋木ファイルクラスの実現ファイル（Fetch関連）
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
#include "SyDynamicCast.h"
#include "Btree/File.h"

#include "Exception/NotSupported.h"

#include "Common/Assert.h"

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
//	Btree::File::fetchBySimpleKey --
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
File::fetchBySimpleKey(const ModUInt32				TreeDepth_,
					   const PhysicalFile::PageID	RootNodePageID_,
					   PageVector&					AttachNodePages_,
					   ValueFile*					ValueFile_,
					   PageVector&					AttachValuePages_)
{
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

	PhysicalFile::Page*	leafPage =
		this->searchSimpleLeafPageForFetch(TreeDepth_,
										   RootNodePageID_,
										   AttachNodePages_);

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
	// File::getSimpleKeyInformationIndexForFetchが設定する
	// ローカル変数matchを参照すればわかる。
	int	keyInfoIndex =
		this->getSimpleKeyInformationIndexForFetch(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
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
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	ModUInt64	valueObjectID = keyInfo.readValueObjectID();

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

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToFetchCondition(
				nullBitmapTop,
				FetchHint::CompareType::OnlySeparateKey)
			!= 0)
		{
			// 先頭キーフィールドから連続している
			// Fetch対象キーフィールドから
			// 離れたFetch対象キーフィールドが
			// Fetch検索条件と一致しなかった…

			ModUInt64	saveValueObjectID = valueObjectID;

			// しかし、まだ該当するオブジェクトが
			// 存在するかもしれないので、
			// 次以降のキーオブジェクトを参照してみる。
			valueObjectID =
				this->getNextObjectIDFetchBySimpleKey(leafPage,
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
					this->getPrevObjectIDFetchBySimpleKey(leafPage,
														  keyInfo,
														  AttachNodePages_,
														  ValueFile_,
														  AttachValuePages_,
														  fetchCondition);
			}
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

		if (compareToFetchCondition(
				valuePage, AttachValuePages_,
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
				this->getNextObjectIDFetchBySimpleKey(valuePage,
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
					this->getPrevObjectIDFetchBySimpleKey(valuePage,
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
//	Btree::File::fetchBySimpleKeyRev --
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
File::fetchBySimpleKeyRev(const ModUInt32				TreeDepth_,
						  const PhysicalFile::PageID	RootNodePageID_,
						  PageVector&					AttachNodePages_,
						  ValueFile*					ValueFile_,
						  PageVector&					AttachValuePages_)
{
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

	this->m_FetchHint.setCondition(fetchCondition);

	PhysicalFile::Page*	leafPage =
		this->searchSimpleLeafPageForFetch(TreeDepth_,
										   RootNodePageID_,
										   AttachNodePages_);

	if (leafPage == 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	bool	match = false;

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	const ModUInt32 useKeyInfoNum =
		leafPageHeader.readUseKeyInformationNumber();

	int	keyInfoIndex =
		this->getSimpleKeyInformationIndexForFetch(
			leafPage,
			useKeyInfoNum,
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
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

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
									   true, // リーフページ
									   this->m_cFileParameter.m_KeyNum,
									   this->m_cFileParameter.m_KeySize);

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

			const NullBitmap::Value*	nullBitmapTop =
				keyInfoForNext.assignConstKeyNullBitmap();

			; _SYDNEY_ASSERT(nullBitmapTop != 0);

			if (this->compareToFetchCondition(nullBitmapTop) != 0)
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
		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToFetchCondition(
				nullBitmapTop,
				FetchHint::CompareType::OnlySeparateKey)
			!= 0)
		{
			valueObjectID =
				this->getPrevObjectIDFetchBySimpleKey(leafPage,
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
					this->getNextObjectIDFetchBySimpleKey(leafPage,
														  keyInfo,
														  AttachNodePages_,
														  ValueFile_,
														  AttachValuePages_,
														  fetchCondition);
			}
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
		PhysicalFile::Page*	valuePage = File::attachPage(
			m_pTransaction, ValueFile_->m_PhysicalFile,
			Common::ObjectIDData::getFormerValue(valueObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

		; _SYDNEY_ASSERT(valuePage != 0);

		if (compareToFetchCondition(
				valuePage, AttachValuePages_,
				Common::ObjectIDData::getLatterValue(valueObjectID),
				fetchCondition,
				FetchHint::CompareType::OnlyValue,
				false) // バリューオブジェクト
			!= 0)
		{
			ModUInt64	saveValueObjectID = valueObjectID;
			valueObjectID =
				this->getPrevObjectIDFetchBySimpleKey(valuePage,
													  AttachNodePages_,
													  ValueFile_,
													  AttachValuePages_,
													  valueObjectID,
													  fetchCondition);

			if (valueObjectID == FileCommon::ObjectID::Undefined)
			{
				valueObjectID =
					this->getNextObjectIDFetchBySimpleKey(valuePage,
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
//	Btree::File::searchSimpleLeafPageForFetch --
//		Fetch検索条件と一致するキーフィールドの値が記録されている
//		可能性があるリーフページを検索する
//
//	NOTES
//	先頭から連続しているFetch対象キーフィールドの値が
//	Fetch検索条件と一致するキーフィールドを記録している
//	可能性があるリーフページを検索する。
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
//		先頭から連続しているFetch対象キーフィールドが
//		Fetch検索条件と一致するキーフィールドの値が記録されている
//		リーフページの物理ページ記述子。
//		該当するキーフィールドが存在しない場合には、
//		0(ヌルポインタ）を返す。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::searchSimpleLeafPageForFetch(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
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

		return this->containTargetSimpleKeyForFetch(RootNodePageID_,
													AttachNodePages_,
													true); // リーフページ
	}

	//
	// 以下は、木の深さ（高さ）が2段以上の木構造の場合の処理
	//

	// リーフページまで該当するノードページを辿っていく…

	PhysicalFile::PageID	nodePageID = RootNodePageID_;

	for (ModUInt32 depth = 1; depth < TreeDepth_; depth++)
	{
		nodePageID =
			this->searchChildSimpleNodePageForFetch(nodePageID,
													AttachNodePages_);

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
//	Btree::File::searchChildSimpleNodePageForFetch --
//		Fetch検索条件と一致するキーフィールドの値が記録されている
//		可能性がある子ノードページを検索する
//
//	NOTES
//	先頭から連続しているFetch対象キーフィールドの値が
//	Fetch検索条件と一致するキーフィールドを記録している
//	可能性があるノードページを
//	引数ParentNodePageID_が示す親ノードページの
//	子ノードページの中から検索する。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ParentNodePageID_
//		親ノードページの物理ページ記述子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	PhysicalFile::PageID
//		先頭から連続しているFetch対象キーフィールドの値が
//		Fetch検索条件と一致するキーフィールドを記録している
//		可能性があるノードページの物理ページ識別子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::PageID
File::searchChildSimpleNodePageForFetch(
	const PhysicalFile::PageID	ParentNodePageID_,
	PageVector&					AttachNodePages_) const
{
	// まずは、親ノードページに
	// 先頭から連続しているFetch対象キーフィールドが
	// Fetch検索条件と一致するキーオブジェクトが記録されている
	// 可能性があるかどうかを調べ、その可能性がある場合にのみ、
	// 子ノードページを検索する。

	PhysicalFile::Page*	parentNodePage =
		this->containTargetSimpleKeyForFetch(ParentNodePageID_,
											 AttachNodePages_,
											 false); // リーフページではない

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
		this->getSimpleKeyInformationIndexForFetch(
			parentNodePage,
			parentNodePageHeader.readUseKeyInformationNumber(),
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
							false, // リーフページではない
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	PhysicalFile::PageID	childNodePageID =
		keyInfo.readChildNodePageID();

	; _SYDNEY_ASSERT(
		childNodePageID != PhysicalFile::ConstValue::UndefinedPageID);

	checkMemoryExhaust(parentNodePage);

	return childNodePageID;
}

//
//	FUNCTION private
//	Btree::File::getSimpleKeyInformationIndexForFetch --
//		Fetch検索条件に最も近いキーフィールドの値を記録している
//		キー情報のインデックスを返す
//
//	NOTES
//	Fetch検索条件に最も近いキーフィールドの値を記録している
//	キー情報のインデックスを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32		UseKeyInfoNum_
//		引数KeyInfoPage_が示すノードページ内の使用中のキー情報数
//	const bool			IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	bool&				Match_
//		そのキー情報に記録されている、
//		先頭から連続しているFetch対象キーフィールドが
//		Fetch検索条件と完全に一致しているかどうか
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
File::getSimpleKeyInformationIndexForFetch(
	PhysicalFile::Page*	KeyInfoPage_,
	const ModUInt32		UseKeyInfoNum_,
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
							IsLeafPage_,
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	Match_ = false;

	while (firstKeyInfoIndex <= lastKeyInfoIndex)
	{
		midKeyInfoIndex = (firstKeyInfoIndex + lastKeyInfoIndex) >> 1;

		keyInfo.setStartOffsetByIndex(midKeyInfoIndex);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult = this->compareToFetchCondition(nullBitmapTop);

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
					if (keyInfoIndex ==
						static_cast<int>(UseKeyInfoNum_) - 1)
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

					nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

					; _SYDNEY_ASSERT(nullBitmapTop != 0);

					if (this->compareToFetchCondition(nullBitmapTop) != 0)
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
//	Btree::File::containTargetSimpleKeyForFetch --
//		Fetch検索条件と一致するキーフィールドの値が記録されている
//		キー情報が、ノードページ内に存在する可能性があるかどうかを知らせる
//
//	NOTES
//	引数NodePageID_で示されるノードページ内に、
//	Fetch検索条件と一致するキーフィールドの値が記録されている
//	キー情報が存在する可能性があるかどうかを知らせる。
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
//		Fetch検索条件と一致するキーフィールドの値が記録されている
//		キー情報がノードページ内に存在する可能性があれば、
//		そのノードページの物理ページ記述子、
//		存在する可能性がなければ、0（ヌルポインタ）。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::containTargetSimpleKeyForFetch(
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
								IsLeafPage_,
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToFetchCondition(nullBitmapTop) > 0)
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
//		Fetch検索条件と記録されているキーフィールドの値を比較する
//
//	NOTES
//	Fetch検索条件と、キー情報内に記録されている
//	キーフィールドの値を比較し、比較結果を返す。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*	NullBitmapTop_
//		キー情報内のヌルビットマップ先頭へのポインタ
//		（キーフィールドの値はヌルビットマップに続く領域に
//		　記録されている。）
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
	const NullBitmap::Value*	NullBitmapTop_) const
{
	FetchHint::CompareType::Value	compareType =
		this->m_FetchHint.m_ExistSeparateKey ?
			FetchHint::CompareType::OnlyLinkKey :
			FetchHint::CompareType::OnlyKey;

	return this->compareToFetchCondition(NullBitmapTop_, compareType);
}

//
//	FUNCTION private
//	Btree::File::compareToFetchCondition --
//		Fetch検索条件と記録されているキーフィールドの値を比較する
//
//	NOTES
//	Fetch検索条件と、キー情報内に記録されている
//	キーフィールドの値を比較し、比較結果を返す。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*				NullBitmapTop_
//		キー情報内のヌルビットマップ先頭へのポインタ
//		（キーフィールドの値はヌルビットマップに続く領域に
//		　記録されている。）
//	const Btree::FetchHint::CompareType::Value	FetchCompareType_
//		Fetch比較対象
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
	const NullBitmap::Value*			NullBitmapTop_,
	const FetchHint::CompareType::Value	FetchCompareType_) const
{
	; _SYDNEY_ASSERT(NullBitmapTop_ != 0);
	; _SYDNEY_ASSERT(
		FetchCompareType_ != FetchHint::CompareType::OnlyValue);

	NullBitmap	nullBitmap(NullBitmapTop_,
						   this->m_cFileParameter.m_KeyNum,
						   NullBitmap::Access::ReadOnly);

	bool	existNull = nullBitmap.existNull();

	const void*	topKey = nullBitmap.getConstTail();

	int	compareResult = 0;

	for (int i = 0; i < this->m_FetchHint.m_FieldNumber; i++)
	{
		int	keyIndex = *(this->m_FetchHint.m_FieldIndexArray + i);

		if (FetchCompareType_ == FetchHint::CompareType::OnlySeparateKey &&
			keyIndex <= this->m_FetchHint.m_LastLinkKeyFieldIndex)
		{
			continue;
		}

		if (FetchCompareType_ == FetchHint::CompareType::OnlyLinkKey &&
			keyIndex > this->m_FetchHint.m_LastLinkKeyFieldIndex)
		{
			break;
		}

		if ((FetchCompareType_ == FetchHint::CompareType::OnlyKey ||
			 FetchCompareType_ == FetchHint::CompareType::OnlySeparateKey) &&
			 keyIndex > this->m_cFileParameter.m_KeyNum)
		{
			break;
		}

		bool	isNull =
			existNull ? nullBitmap.isNull(keyIndex - 1) : false;

		if (*(this->m_FetchHint.m_IsNullArray + i))
		{
			if (isNull == false)
			{
				return -1 * *(this->m_FetchHint.m_MultiNumberArray + i);
			}
		}
		else
		{
			; _SYDNEY_ASSERT(*(this->m_FetchHint.m_IsFixedArray + i));

			if (isNull)
			{
				return 1 * *(this->m_FetchHint.m_MultiNumberArray + i);
			}

			const void*	key = this->assignKey(topKey, keyIndex);

			compareResult =
				this->m_FetchHint.compareToFixedCondition(key, i);

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
//	Btree::File::getNextObjectIDFetchBySimpleKey --
//		Fetch検索条件と一致する次のオブジェクトのオブジェクトIDを返す
//
//	NOTES
//	Fetch検索条件と一致する次のオブジェクトを検索し、
//	該当するオブジェクトが存在すれば、
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
File::getNextObjectIDFetchBySimpleKey(
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

	const void*	objectAreaTop = File::getConstAreaTop(
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
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	ModUInt64	valueObjectID =
		this->getNextObjectIDFetchBySimpleKey(leafPage,
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
//	Btree::File::getNextObjectIDFetchBySimpleKey --
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
File::getNextObjectIDFetchBySimpleKey(
	PhysicalFile::Page*&			LeafPage_,
	KeyInformation&					KeyInfo_,
	PageVector&						AttachNodePages_,
	ValueFile*						ValueFile_,
	PageVector&						AttachValuePages_,
	const Common::DataArrayData*	Condition_) const
{
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

		const NullBitmap::Value*	nullBitmapTop =
			KeyInfo_.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToFetchCondition(nullBitmapTop) != 0)
		{
			break;
		}

		if (this->m_FetchHint.m_ExistSeparateKey)
		{
			if (this->compareToFetchCondition(
					nullBitmapTop,
					FetchHint::CompareType::OnlySeparateKey)
				!= 0)
			{
				continue;
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

		if (compareToFetchCondition(
				valuePage, AttachValuePages_,
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
//	Btree::File::getPrevObjectIDFetchBySimpleKey --
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
File::getPrevObjectIDFetchBySimpleKey(
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

	const void*	objectAreaTop = File::getConstAreaTop(
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
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	ModUInt64	valueObjectID =
		this->getPrevObjectIDFetchBySimpleKey(leafPage,
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
//	Btree::File::getPrevObjectIDFetchBySimpleKey --
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
File::getPrevObjectIDFetchBySimpleKey(
	PhysicalFile::Page*&			LeafPage_,
	KeyInformation&					KeyInfo_,
	PageVector&						AttachNodePages_,
	ValueFile*						ValueFile_,
	PageVector&						AttachValuePages_,
	const Common::DataArrayData*	Condition_) const
{
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

		const NullBitmap::Value*	nullBitmapTop =
			KeyInfo_.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToFetchCondition(nullBitmapTop) != 0)
		{
			break;
		}

		if (this->m_FetchHint.m_ExistSeparateKey)
		{
			if (this->compareToFetchCondition(
					nullBitmapTop,
					FetchHint::CompareType::OnlySeparateKey)
				!= 0)
			{
				continue;
			}
		}

		ModUInt64	srcValueObjectID = KeyInfo_.readValueObjectID();

		if (this->m_FetchHint.m_OnlyKey)
		{
			valueObjectID = srcValueObjectID;

			break;
		}
		
		valuePage = File::attachPage(
			m_pTransaction, ValueFile_->m_PhysicalFile,
			Common::ObjectIDData::getFormerValue(srcValueObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

		if (compareToFetchCondition(
				valuePage, AttachValuePages_,
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

	if (this->m_CatchMemoryExhaust)	// MemoryExhaust ならデタッチ（アンフィックス）されている
	{
		LeafPage_ = 0;
	}

	return valueObjectID;
}

//
//	Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
