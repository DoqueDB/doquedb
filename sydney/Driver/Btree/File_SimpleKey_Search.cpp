// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_SimpleKey_Search.cpp -- Ｂ＋木ファイルクラスの実現ファイル（検索関連）
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
//	Btree::File::searchBySingleEqualsSimpleKey --
//		先頭キーフィールドの値が検索条件と一致するオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件と一致するオブジェクトを検索し、
//	ファイル内に該当オブジェクトが存在する場合には、
//	そのオブジェクトのIDを返す。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
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
//		先頭キーフィールドの値が検索条件と一致するオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleEqualsSimpleKey(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	// 先頭キーフィールドが検索条件と一致するオブジェクトが
	// 記録されている可能性があるリーフページを検索する。
	// ※ ここで得られる物理ページ記述子は、
	// 　 “キーオブジェクト”が記録されている
	// 　 物理ページの記述子だとは限らない。
	// 　 “キー情報”が記録されている物理ページの記述子である。
	PhysicalFile::Page*	leafPage =
		this->searchSimpleLeafPageForSingleCondition(TreeDepth_,
													 RootNodePageID_,
													 AttachNodePages_);

	if (leafPage == 0)
	{
		// 先頭キーフィールドが検索条件と一致するオブジェクトが
		// 記録されている可能性があるリーフページすら
		// 存在しなかった…

		return FileCommon::ObjectID::Undefined;
	}

	bool	match = false;

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	// 先頭キーフィールドが検索条件に最も近い値の
	// キーオブジェクトへ辿るキー情報のインデックスを取得する。
	// そのキーオブジェクトが検索条件と一致しているかどうかは、
	// File::getSimpleKeyInformationIndexForSingleConditionが設定する
	// ローカル変数matchを参照すればわかる。
	int	keyInfoIndex =
		this->getSimpleKeyInformationIndexForSingleCondition(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
			true, // リーフページ
			match);

	if (match == false)
	{
		// 先頭キーフィールドが検索条件と一致する
		// キーオブジェクトは存在しなかった…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	//
	// ここまでたどり着いたということは、
	// 先頭キーフィールドが検索条件と一致する
	// キーオブジェクトが見つかったということ！
	//

	// 先頭キーフィールドが検索条件と一致する
	// キーオブジェクトへ辿るキー情報から
	// バリューオブジェクトのオブジェクトIDを読み込む。

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleLessThanSimpleKey --
//		先頭キーフィールドの値が検索条件以下（または未満）の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件以下（または未満）の
//	オブジェクトを検索し、ファイル内に該当オブジェクトが存在する場合には、
//	そのオブジェクトのIDを返す。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&							FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//	const bool										ContainEquals_
//		比較演算子がLessThanとLessThanEqualsのどちらか
//			true  : LessThanEquals
//			false : LessThan
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//			※ File::searchBySpanConditionSimpleKey()のための引数である。
//	PhysicalFile::PageID*							KeyInfoLeafPageID_ = 0
//		検索条件と一致するキーフィールドの値が書き込まれているキー情報が
//		記録されているリーフページの物理ページ識別子へのポインタ
//		※ File::searchBySpanConditionSimpleKey()のための引数であり、
//		　 本関数がポインタが指している領域に物理ページ識別子を設定する。
//	ModUInt32*										KeyInfoIndex_ = 0
//		同、キー情報のインデックスへのポインタ
//		※ 同上。
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以下（または未満）のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleLessThanSimpleKey(
	FileInformation&						FileInfo_,
	PageVector&								AttachNodePages_,
	const bool								ContainEquals_,
	const SearchHint::CompareTarget::Value	Target_,            // = Start
	PhysicalFile::PageID*					KeyInfoLeafPageID_, // = 0
	ModUInt32*								KeyInfoIndex_       // = 0
	) const
{
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		// ということは、
		// 「 key1 < 100 」のような検索条件で、
		// 一番最初に返すオブジェクトは、
		// “キー値順に先頭オブジェクト”となる。

		// でも、“キー値順に先頭オブジェクト”の
		// 先頭キーフィールドの値に、例えば、100という値が
		// 記録されていたとして、
		// 検索条件が「 key1 < 50 」のような場合もある。
		// このような場合には、
		// 無条件で“キー値順に先頭オブジェクト”を
		// 返すわけにもいかない。
		// しかもSearch+Fetchの可能性もあるわけだし…。

		// なので、“キー値順に先頭オブジェクト”と
		// 検索条件との比較が必要となる。

		// File::searchBySingleLessThanSimpleKeyは、
		// File::searchBySpanConditionSimpleKeyからも呼ばれる。
		// しかし、それは、
		// “先頭キーフィールドのソート順が降順”の場合のみである。
		// したがって、引数KeyInfoLeafPageID_などは
		// この場合、0であるはず。
		; _SYDNEY_ASSERT(KeyInfoLeafPageID_ == 0 &&
						 KeyInfoIndex_ == 0);

		// しかも、“検索開始条件”と比較するはず。
		; _SYDNEY_ASSERT(Target_ == SearchHint::CompareTarget::Start);

		// 先頭リーフページの先頭キー情報を参照する。

		PhysicalFile::PageID	topLeafPageID =
			FileInfo_.readTopLeafPageID();

		leafPage = File::attachPage(this->m_pTransaction,
									this->m_pPhysicalFile,
									topLeafPageID,
									this->m_FixMode,
									this->m_CatchMemoryExhaust,
									AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								0,    // 先頭のキー情報
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(nullBitmapTop, Target_);

		if (compareResult < 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		resultObjectID =
			this->getTopObjectID(FileInfo_,
								 AttachNodePages_,
								 false); // バリューオブジェクトの
								         // オブジェクトIDを取得する
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		leafPage =
			this->searchSimpleLeafPageForSingleCondition(treeDepth,
														 rootNodePageID,
														 AttachNodePages_,
														 Target_);

		if (leafPage == 0)
		{
			return resultObjectID;
		}

		PhysicalFile::PageID	leafPageID = leafPage->getID();

		bool	match = false; // dummy

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		int	keyInfoIndex =
			this->getSimpleKeyInformationIndexForSingleCondition(
				leafPage,
				leafPageHeader.readUseKeyInformationNumber(),
				true, // リーフページ
				match,
				Target_);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								keyInfoIndex,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		resultObjectID = keyInfo.readValueObjectID();

		if (this->compareToTopSearchCondition(nullBitmapTop, Target_)
			< 0)
		{
			// 検索条件の方が
			// 記録されているキーオブジェクトよりも
			// キー値順で前だった…

			// つまり、検索条件の方が
			// 記録されているキーオブジェクトよりも
			// 大きかった…

			bool	lp = true;
			do
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// File::searchBySpanConditionから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある。

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();
				}

				ModUInt64	saveObjectID = resultObjectID;

				if (this->assignPrevKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					resultObjectID = saveObjectID;

					break;
				}

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				int	compareResult =
					this->compareToTopSearchCondition(nullBitmapTop,
													  Target_);

				if (compareResult > 0 ||
					(ContainEquals_ == false && compareResult == 0))
				{
					lp = false;
				}
				else
				{
					resultObjectID = keyInfo.readValueObjectID();
				}
			}
			while (lp);
		}
		else
		{
			if (ContainEquals_ == false)
			{
				bool	lp = true;
				do
				{
					if (this->assignNextKeyInformation(leafPage,
													   AttachNodePages_,
													   leafPageHeader,
													   keyInfo)
						== false)
					{
						// なかった…

						checkMemoryExhaust(leafPage);

						return FileCommon::ObjectID::Undefined;
					}

					nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

					; _SYDNEY_ASSERT(nullBitmapTop != 0);

					if (this->compareToTopSearchCondition(nullBitmapTop,
														  Target_)
						< 0)
					{
						resultObjectID = keyInfo.readValueObjectID();

						lp = false;

						if (KeyInfoLeafPageID_ != 0)
						{
							// File::searchBySpanConditionから呼ばれた…

							// ならば、
							// リーフページのキーオブジェクトに関する情報を
							// 設定する必要がある。

							; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

							*KeyInfoLeafPageID_ = leafPage->getID();

							*KeyInfoIndex_ = keyInfo.getIndex();
						}
					}
				}
				while (lp);
			}
			else
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// File::searchBySpanConditionから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある。

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();
				}
			}
		}
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleGreaterThanSimpleKey --
//		先頭キーフィールドの値が検索条件以上（または超）の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件以上（または超）の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&		FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool					ContainEquals_
//		比較演算子がGreaterThanとGreaterThanEqualsのどちらか
//			true  : GreaterThanEquals
//			false : GreaterThan
//	PhysicalFile::PageID*		KeyInfoLeafPageID_ = 0
//		検索条件と一致するキーフィールドの値が書き込まれているキー情報が
//		記録されているリーフページの物理ページ識別子へのポインタ
//		※ File::searchBySpanConditionSimpleKey()のための引数であり、
//		　 本関数がポインタが指している領域に物理ページ識別子を設定する。
//	ModUInt32*					KeyInfoIndex_ = 0
//		同、キー情報のインデックスへのポインタ
//		※ 同上。
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以上（または超）のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleGreaterThanSimpleKey(
	FileInformation&		FileInfo_,
	PageVector&				AttachNodePages_,
	const bool				ContainEquals_,
	PhysicalFile::PageID*	KeyInfoLeafPageID_, // = 0
	ModUInt32*				KeyInfoIndex_       // = 0
	) const
{
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		leafPage =
			this->searchSimpleLeafPageForSingleCondition(treeDepth,
														 rootNodePageID,
														 AttachNodePages_);

		if (leafPage == 0)
		{
			return resultObjectID;
		}

		PhysicalFile::PageID	leafPageID = leafPage->getID();

		bool	match = false; // dummy

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		int	keyInfoIndex =
			this->getSimpleKeyInformationIndexForSingleCondition(
				leafPage,
				leafPageHeader.readUseKeyInformationNumber(),
				true, // リーフページ
				match);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								keyInfoIndex,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		resultObjectID = keyInfo.readValueObjectID();

		if (this->compareToTopSearchCondition(nullBitmapTop) < 0)
		{
			// 検索条件の方が
			// 記録されているキーオブジェクトよりも
			// キー値順で前だった…

			// つまり、検索条件の方が
			// 記録されているキーオブジェクトよりも
			// 小さかった…

			bool	lp = true;
			do
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// File::searchBySpanConditionから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();
				}

				ModUInt64	saveObjectID = resultObjectID;

				if (this->assignPrevKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					resultObjectID = saveObjectID;

					break;
				}

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				int	compareResult =
					this->compareToTopSearchCondition(nullBitmapTop);

				if (compareResult > 0 ||
					(ContainEquals_ == false && compareResult == 0))
				{
					lp = false;
				}
				else
				{
					resultObjectID = keyInfo.readValueObjectID();
				}
			}
			while (lp);
		}
		else
		{
			// 検索条件の方が
			// 記録されているキーオブジェクトよりも
			// 大きかった…
			// または、
			// 検索条件と
			// 記録されているキーオブジェクトが
			// 等しかった…

			if (ContainEquals_ == false)
			{
				bool	lp = true;
				do
				{
					if (this->assignNextKeyInformation(leafPage,
													   AttachNodePages_,
													   leafPageHeader,
													   keyInfo)
						== false)
					{
						// なかった…

						checkMemoryExhaust(leafPage);

						return FileCommon::ObjectID::Undefined;
					}

					nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

					; _SYDNEY_ASSERT(nullBitmapTop != 0);

					if (this->compareToTopSearchCondition(nullBitmapTop)
						< 0)
					{
						resultObjectID = keyInfo.readValueObjectID();

						lp = false;

						if (KeyInfoLeafPageID_ != 0)
						{
							// File::searchBySpanConditionから呼ばれた…

							// ならば、
							// リーフページのキーオブジェクトに関する情報を
							// 設定する必要がある

							; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

							*KeyInfoLeafPageID_ = leafPage->getID();

							*KeyInfoIndex_ = keyInfo.getIndex();
						}
					}
				}
				while (lp);
			}
			else
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// File::searchBySpanConditionから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある。

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();
				}
			}
		}
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		// ということは、
		// 「 key1 > 100 」のような検索条件で、
		// 一番最初に返すオブジェクトは、
		// “キー値順に先頭オブジェクト”となる。

		// でも、“キー値順に先頭オブジェクト”の
		// 先頭キーフィールドの値に、例えば、50という値が
		// 記録されていたとして、
		// 検索条件が「 key1 > 100 」のような場合もある。
		// このような場合には、
		// 無条件で“キー値順に先頭オブジェクト”を
		// 返すわけにもいかない。
		// しかもSearch+Fetchの可能性もあるわけだし…。

		// なので、“キー値順に先頭オブジェクト”と
		// 検索条件との比較が必要となる。

		// File::searchBySingleGreaterThanは、
		// File::searchBySpanConditionからも呼ばれる。
		// しかし、それは、
		// “先頭キーフィールドのソート順が昇順”の場合のみである。
		// したがって、引数KeyInfoLeafPageID_などは
		// この場合、0であるはず。
		; _SYDNEY_ASSERT(KeyInfoLeafPageID_ == 0 &&
						 KeyInfoIndex_ == 0);

		PhysicalFile::PageID	topLeafPageID =
			FileInfo_.readTopLeafPageID();

		leafPage = File::attachPage(this->m_pTransaction,
									this->m_pPhysicalFile,
									topLeafPageID,
									this->m_FixMode,
									this->m_CatchMemoryExhaust,
									AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								0,    // 先頭のキー情報
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult = this->compareToTopSearchCondition(nullBitmapTop);

		if (compareResult < 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		resultObjectID = keyInfo.readValueObjectID();
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleEqualsToNullSimpleKey --
//		先頭キーフィールドの値がヌル値のオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値がヌル値のオブジェクトを検索する。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値がヌル値のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleEqualsToNullSimpleKey(
	FileInformation&	FileInfo_,
	PageVector&			AttachNodePages_) const
{
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		// ということは、
		// “キー値順に先頭オブジェクト”が
		// ヌル値かな？とチェックして、
		// ヌル値ならばそのオブジェクトを返せばよいし、
		// そうでなければ検索条件と一致するオブジェクトは
		// 存在しないこととなる。

		PhysicalFile::PageID	topLeafPageID =
			FileInfo_.readTopLeafPageID();

		leafPage = File::attachPage(this->m_pTransaction,
									this->m_pPhysicalFile,
									topLeafPageID,
									this->m_FixMode,
									this->m_CatchMemoryExhaust,
									AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								0,    // 先頭のキー情報
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (NullBitmap::isNull(nullBitmapTop,
							   this->m_cFileParameter.m_KeyNum,
							   0)
			== false)
		{
			// 先頭キーフィールドにヌル値が
			// 記録されているオブジェクトは
			// 存在しない…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		// 先頭オブジェクトの先頭キーフィールドに
		// ヌル値が記録されているので、ここに来た。

		resultObjectID = keyInfo.readValueObjectID();
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		// ということは、
		// “キー値順に最終オブジェクト”が
		// ヌル値かな？とチェックして、
		// ヌル値ならば最終オブジェクトから順に
		// キー値順に先頭オブジェクト方向にオブジェクトを
		// スキャンしていき、最も先頭オブジェクトに近い
		// 先頭キーフィールドにヌル値が設定されているオブジェクトを
		// 見つけ出し、そのオブジェクトを返せばよい。

		PhysicalFile::PageID	lastLeafPageID =
			FileInfo_.readLastLeafPageID();

		leafPage = File::attachPage(this->m_pTransaction,
									this->m_pPhysicalFile,
									lastLeafPageID,
									this->m_FixMode,
									this->m_CatchMemoryExhaust,
									AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		NodePageHeader	leafPageHeader(this->m_pTransaction,
										   leafPage,
										   true); // リーフページ

		ModUInt32	useKeyInfoNum =
			leafPageHeader.readUseKeyInformationNumber();

		; _SYDNEY_ASSERT(useKeyInfoNum > 0);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								useKeyInfoNum - 1,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (NullBitmap::isNull(nullBitmapTop,
							   this->m_cFileParameter.m_KeyNum,
							   0)
			== false)
		{
			// 先頭キーフィールドにヌル値が
			// 設定されているオブジェクトは
			// 存在しない…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		resultObjectID = keyInfo.readValueObjectID();

		bool	lp = true;
		do
		{
			ModUInt64	saveObjectID = resultObjectID;

			if (this->assignPrevKeyInformation(leafPage,
											   AttachNodePages_,
											   leafPageHeader,
											   keyInfo)
				== false)
			{
				resultObjectID = saveObjectID;

				break;
			}

			nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

			; _SYDNEY_ASSERT(nullBitmapTop != 0);

			if (NullBitmap::isNull(nullBitmapTop,
								   this->m_cFileParameter.m_KeyNum,
								   0)
				== false)
			{
				lp = false;
			}
			else
			{
				resultObjectID = keyInfo.readValueObjectID();
			}
		}
		while (lp);
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySpanConditionSimpleKey --
//		先頭キーフィールドの値が検索条件で指定された範囲内の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件で指定された範囲内の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件で指定された範囲内のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySpanConditionSimpleKey(
	FileInformation&	FileInfo_,
	PageVector&			AttachNodePages_) const
{
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		//
		// まずは、検索開始条件と一致するオブジェクトを検索する。
		//

		bool	containEquals =
			(*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals);

		PhysicalFile::PageID	keyInfoLeafPageID =
			PhysicalFile::ConstValue::UndefinedPageID;

		ModUInt32	keyInfoIndex = ModUInt32Max;

		resultObjectID =
			this->searchBySingleGreaterThanSimpleKey(FileInfo_,
													 AttachNodePages_,
													 containEquals,
													 &keyInfoLeafPageID,
													 &keyInfoIndex);

		if (resultObjectID == FileCommon::ObjectID::Undefined)
		{
			// 検索開始条件と一致するオブジェクトが
			// ファイル内に存在しなかった…

			return resultObjectID;
		}

		; _SYDNEY_ASSERT(
			keyInfoLeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

		; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

		//
		// 検索終了条件とも一致するかチェックする。
		//

		PhysicalFile::Page*	keyInfoLeafPage =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 keyInfoLeafPageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		KeyInformation	keyInfo(this->m_pTransaction,
								keyInfoLeafPage,
								keyInfoIndex,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(
				nullBitmapTop,
				SearchHint::CompareTarget::Stop);

		checkMemoryExhaust(keyInfoLeafPage);

		if (compareResult < 0)
		{
			// 検索終了条件とは一致しなかった…

			// ということは、ファイル内に
			// 指定範囲内のオブジェクトは存在しない。

			return FileCommon::ObjectID::Undefined;
		}

		if (compareResult == 0 &&
			this->m_SearchHint.m_StopOperator ==
			LogicalFile::TreeNodeInterface::LessThan)
		{
			// 同上

			return FileCommon::ObjectID::Undefined;
		}
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		//
		// まずは、検索終了条件と一致するオブジェクトを検索する
		//

		bool	containEquals =
			(this->m_SearchHint.m_StopOperator ==
			 LogicalFile::TreeNodeInterface::LessThanEquals);

		PhysicalFile::PageID	keyInfoLeafPageID =
			PhysicalFile::ConstValue::UndefinedPageID;

		ModUInt32	keyInfoIndex = ModUInt32Max;

		resultObjectID =
			this->searchBySingleLessThanSimpleKey(
				FileInfo_,
				AttachNodePages_,
				containEquals,
				SearchHint::CompareTarget::Stop,
				&keyInfoLeafPageID,
				&keyInfoIndex);

		if (resultObjectID == FileCommon::ObjectID::Undefined)
		{
			// 検索終了条件と一致するオブジェクトが
			// ファイル内に存在しなかった…

			return resultObjectID;
		}

		; _SYDNEY_ASSERT(
			keyInfoLeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

		; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

		//
		// 検索開始条件とも一致するかチェックする
		//

		PhysicalFile::Page*	keyInfoLeafPage =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 keyInfoLeafPageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		KeyInformation	keyInfo(this->m_pTransaction,
								keyInfoLeafPage,
								keyInfoIndex,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(
				nullBitmapTop,
				SearchHint::CompareTarget::Start);

		checkMemoryExhaust(keyInfoLeafPage);

		if (compareResult < 0)
		{
			// 検索開始条件とは一致しなかった…

			// ということは、ファイル内に
			// 指定範囲内のオブジェクトは存在しない。

			return FileCommon::ObjectID::Undefined;
		}

		if (compareResult == 0 &&
			*this->m_SearchHint.m_StartOperatorArray ==
			LogicalFile::TreeNodeInterface::GreaterThan)
		{
			// 同上

			return FileCommon::ObjectID::Undefined;
		}
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchByMultiConditionSimpleKey --
//		複数キーフィールドの値が検索条件で指定された複合条件と一致する
//		オブジェクトを検索する
//
//	NOTES
//	複数キーフィールドの値が検索条件で指定された複合条件と一致する
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
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
//		いずれの検索対象キーフィールドの値も、
//		そのキーフィールドに対する検索条件と一致するオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchByMultiConditionSimpleKey(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	leafPage =
		this->searchSimpleLeafPageForMultiCondition(TreeDepth_,
													RootNodePageID_,
													AttachNodePages_);

	if (leafPage == 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	bool	match = false;

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	int	keyInfoIndex =
		this->getSimpleKeyInformationIndexForMultiCondition(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
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

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	if (this->m_SearchHint.m_ExistSeparateKey ||
		this->m_SearchHint.m_LastStartOperatorIsEquals == false)
	{
		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToLastCondition(nullBitmapTop) == false)
		{
			resultObjectID = FileCommon::ObjectID::Undefined;

			NodePageHeader	leafPageHeader(this->m_pTransaction,
										   leafPage,
										   true); // リーフページ

			while (this->assignNextKeyInformation(leafPage,
												  AttachNodePages_,
												  leafPageHeader,
												  keyInfo))
			{
				if (leafPage->getID() != leafPageID)
				{
					leafPageID = leafPage->getID();

					leafPageHeader.resetPhysicalPage(leafPage);
				}

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				if (this->compareToMultiSearchCondition(nullBitmapTop) != 0)
				{
					break;
				}

				if (this->compareToLastCondition(nullBitmapTop))
				{
					resultObjectID = keyInfo.readValueObjectID();

					break;
				}
			}
		}
	}

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleEqualsRevSimpleKey --
//		先頭キーフィールドの値が検索条件と一致するオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件と一致するオブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
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
//		先頭キーフィールドの値が検索条件と一致するオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleEqualsRevSimpleKey(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	leafPage =
		this->searchSimpleLeafPageForSingleCondition(TreeDepth_,
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

	int	keyInfoIndex =
		this->getSimpleKeyInformationIndexForSingleCondition(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
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

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	bool	lp = true;
	do
	{
		ModUInt64	saveObjectID = resultObjectID;

		if (this->assignNextKeyInformation(leafPage,
										   AttachNodePages_,
										   leafPageHeader,
										   keyInfo)
			== false)
		{
			resultObjectID = saveObjectID;

			break;
		}

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToTopSearchCondition(nullBitmapTop) != 0)
		{
			lp = false;
		}
		else
		{
			resultObjectID = keyInfo.readValueObjectID();
		}
	}
	while (lp);

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleLessThanRevSimpleKey --
//		先頭キーフィールドの値が検索条件以下（または未満）の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件以下（または未満）の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&							FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//	Btree::ValueFile*								ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&								AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const bool										ContainEquals_
//		比較演算子がLessThanとLessThanEqualsのどちらか
//			true  : LessThanEquals
//			false : LessThan
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//	PhysicalFile::PageID*							KeyInfoLeafPageID_ = 0
//		検索条件と一致するキーフィールドの値が書き込まれているキー情報が
//		記録されているリーフページの物理ページ識別子へのポインタ
//		※ File::searchBySpanConditionSimpleKey()のための引数であり、
//		　 本関数がポインタが指している領域に物理ページ識別子を設定する。
//	ModUInt32*										KeyInfoIndex_ = 0
//		同、キー情報のインデックスへのポインタ
//		※ 同上。
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以下（または未満）のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleLessThanRevSimpleKey(
	FileInformation&						FileInfo_,
	PageVector&								AttachNodePages_,
	ValueFile*								ValueFile_,
	PageVector&								AttachValuePages_,
	const bool								ContainEquals_,
	const SearchHint::CompareTarget::Value	Target_,            // = Start
	PhysicalFile::PageID*					KeyInfoLeafPageID_, // = 0
	ModUInt32*								KeyInfoIndex_       // = 0
	) const
{
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		leafPage =
			this->searchSimpleLeafPageForSingleCondition(treeDepth,
														 rootNodePageID,
														 AttachNodePages_,
														 Target_);

		if (leafPage == 0)
		{
			// もしかしたら、挿入されているオブジェクトすべてが
			// 検索条件に一致するのかもしれない。
			// 例えば、先頭キーフィールドの最大値が100で、
			// 検索条件が「 key < 200 」のような場合、
			// File::searchSimpleLeafPageForSingleConditionは、
			// 0を返すので、ここで、“キー値順での最終オブジェクト”と
			// 検索条件を比較してみる。

			PhysicalFile::PageID	lastLeafPageID =
				FileInfo_.readLastLeafPageID();

			leafPage = File::attachPage(this->m_pTransaction,
										this->m_pPhysicalFile,
										lastLeafPageID,
										this->m_FixMode,
										this->m_CatchMemoryExhaust,
										AttachNodePages_);

			; _SYDNEY_ASSERT(leafPage != 0);

			NodePageHeader	leafPageHeader(this->m_pTransaction,
										   leafPage,
										   true); // リーフページ

			ModUInt32	useKeyInfoNum =
				leafPageHeader.readUseKeyInformationNumber();

			; _SYDNEY_ASSERT(useKeyInfoNum > 0);

			KeyInformation	keyInfo(this->m_pTransaction,
									leafPage,
									useKeyInfoNum - 1,
									true, // リーフページ
									this->m_cFileParameter.m_KeyNum,
									this->m_cFileParameter.m_KeySize);

			const NullBitmap::Value*	nullBitmapTop =
				keyInfo.assignConstKeyNullBitmap();

			; _SYDNEY_ASSERT(nullBitmapTop != 0);

			if (this->compareToTopSearchCondition(nullBitmapTop,
												  Target_)
				> 0)
			{
				// すべてのオブジェクトが検索条件と一致する…

				resultObjectID = keyInfo.readValueObjectID();
			}

			if (KeyInfoLeafPageID_ != 0)
			{
				// File::searchBySpanConditionRevから呼ばれた…

				// ならば、
				// リーフページのキーオブジェクトに関する情報を
				// 設定する必要がある。

				; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

				ValueFile_->readLeafInfo(resultObjectID,
										 AttachValuePages_,
										 this->m_CatchMemoryExhaust,
										 *KeyInfoLeafPageID_,
										 *KeyInfoIndex_);
			}

			checkMemoryExhaust(leafPage);

			return resultObjectID;
		}

		PhysicalFile::PageID	leafPageID = leafPage->getID();

		bool	match = false; // dummy

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		int	keyInfoIndex =
			this->getSimpleKeyInformationIndexForSingleCondition(
				leafPage,
				leafPageHeader.readUseKeyInformationNumber(),
				true, // リーフページ
				match,
				Target_);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								keyInfoIndex,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		resultObjectID = keyInfo.readValueObjectID();

		// LessThanEqualsでmatchでも、
		// もっと先に検索条件に一致するオブジェクトが
		// 存在するかもしれない。
		int	compareResult =
			this->compareToTopSearchCondition(nullBitmapTop,
											  Target_);

		if ((ContainEquals_ == false && compareResult <= 0) ||
			(ContainEquals_ && compareResult < 0))
		{
			// 検索条件の方が
			// 記録されているキーオブジェクトよりも
			// 小さかった…
			// または、
			// 検索条件と
			// 記録されているキーオブジェクトが
			// 等しかった…

			bool	lp = true;
			do
			{
				if (this->assignPrevKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					// なかった…

					checkMemoryExhaust(leafPage);

					return FileCommon::ObjectID::Undefined;
				}

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				compareResult =
					this->compareToTopSearchCondition(nullBitmapTop,
													  Target_);

				if (compareResult > 0 ||
					(ContainEquals_ && compareResult == 0))
				{
					resultObjectID = keyInfo.readValueObjectID();

					lp = false;

					if (KeyInfoLeafPageID_ != 0)
					{
						// File::searchBySpanConditionRevから呼ばれた…

						// ならば、
						// リーフページのキーオブジェクトに関する情報を
						// 設定する必要がある。

						; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

						*KeyInfoLeafPageID_ = leafPage->getID();

						*KeyInfoIndex_ = keyInfo.getIndex();
					}
				}
			}
			while (lp);
		}
		else
		{
			// 検索条件の方が
			// 記録されているキーオブジェクトよりも
			// 大きかった…

			bool	lp = true;
			do
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// File::searchBySpanConditionRevから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある。

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();
				}

				ModUInt64	saveObjectID = resultObjectID;

				if (this->assignNextKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					resultObjectID = saveObjectID;

					break;
				}

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				compareResult =
					this->compareToTopSearchCondition(nullBitmapTop,
													  Target_);

				if (compareResult < 0 ||
					(ContainEquals_ == false && compareResult == 0))
				{
					lp = false;
				}
				else
				{
					resultObjectID = keyInfo.readValueObjectID();
				}
			}
			while (lp);
		}
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		// ということは、
		// 「 key1 < 100 」のような検索条件で、
		// 一番最初に返すオブジェクトは、
		// “キー値順に最終オブジェクト”となる。

		// でも、“キー値順に最終オブジェクト”の
		// 先頭キーフィールドの値に、例えば、100という値が
		// 記録されていたとして、
		// 検索条件が「 key1 < 50 」のような場合もある。
		// このような場合には、
		// 無条件で“キー値順に最終オブジェクト”を
		// 返すわけにもいかない。
		// しかもSearch+Fetchの可能性もあるわけだし…。

		// なので、“キー値順に最終オブジェクト”と
		// 検索条件との比較が必要となる。

		// File::searchBySingleLessThanは、
		// File::searchBySpanConditionからも呼ばれる。
		// しかし、それは、
		// “先頭キーフィールドのソート順が昇順”の場合のみである。
		// したがって、引数KeyInfoLeafPageID_などは
		// この場合、0であるはず。
		; _SYDNEY_ASSERT(KeyInfoLeafPageID_ == 0 &&
						 KeyInfoIndex_ == 0);

		// しかも、“検索開始条件”と比較するはず。
		; _SYDNEY_ASSERT(Target_ == SearchHint::CompareTarget::Start);

		PhysicalFile::PageID	lastLeafPageID =
			FileInfo_.readLastLeafPageID();

		leafPage = File::attachPage(this->m_pTransaction,
									this->m_pPhysicalFile,
									lastLeafPageID,
									this->m_FixMode,
									this->m_CatchMemoryExhaust,
									AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		ModUInt32	useKeyInfoNum =
			leafPageHeader.readUseKeyInformationNumber();

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								useKeyInfoNum - 1,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(nullBitmapTop);

		if (compareResult > 0 ||
			ContainEquals_ == false && compareResult == 0)
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		resultObjectID = keyInfo.readValueObjectID();
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleGreaterThanRevSimpleKey --
//		先頭キーフィールドの値が検索条件以上（または超）の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件以上（または超）の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const bool				ContainEquals_
//		比較演算子がGreaterThanとGreaterThanEqualsのどちらか
//			true  : GreaterThanEquals
//			false : GreaterThan
//	PhysicalFile::PageID*	KeyInfoLeafPageID_ = 0
//		検索条件と一致するキーフィールドの値が書き込まれているキー情報が
//		記録されているリーフページの物理ページ識別子へのポインタ
//		※ File::searchBySpanConditionSimpleKey()のための引数であり、
//		　 本関数がポインタが指している領域に物理ページ識別子を設定する。
//	ModUInt32*				KeyInfoIndex_ = 0
//		同、キー情報のインデックスへのポインタ
//		※ 同上。
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以上（または超）のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleGreaterThanRevSimpleKey(
	FileInformation&		FileInfo_,
	PageVector&				AttachNodePages_,
	ValueFile*				ValueFile_,
	PageVector&				AttachValuePages_,
	const bool				ContainEquals_,
	PhysicalFile::PageID*	KeyInfoLeafPageID_, // = 0
	ModUInt32*				KeyInfoIndex_       // = 0
	) const
{
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		// File::searchBySingleGreaterThanRevSimpleKeyは、
		// File::searchBySpanConditionRevSimpleKeyからも呼ばれる。
		// しかし、それは、
		// “先頭キーフィールドのソート順が降順”の場合のみである。
		// したがって、引数KeyInfoLeafPageID_などは
		// この場合、0であるはず。
		; _SYDNEY_ASSERT(KeyInfoLeafPageID_ == 0 && KeyInfoIndex_ == 0);

		PhysicalFile::PageID	lastLeafPageID =
			FileInfo_.readLastLeafPageID();

		leafPage = File::attachPage(this->m_pTransaction,
									this->m_pPhysicalFile,
									lastLeafPageID,
									this->m_FixMode,
									this->m_CatchMemoryExhaust,
									AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		ModUInt32	useKeyInfoNum =
			leafPageHeader.readUseKeyInformationNumber();

		; _SYDNEY_ASSERT(useKeyInfoNum > 0);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								useKeyInfoNum - 1,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(nullBitmapTop);

		if (compareResult > 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			// なかった…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		resultObjectID = keyInfo.readValueObjectID();
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		leafPage =
			this->searchSimpleLeafPageForSingleCondition(treeDepth,
														 rootNodePageID,
														 AttachNodePages_);

		if (leafPage == 0)
		{
			// もしかしたら、挿入されているオブジェクトすべてが
			// 検索条件に一致するのかもしれない。
			// 例えば、先頭キーフィールドの最小値が300で、
			// 検索条件が「 key > 200 」のような場合、
			// File::searchSimpleLeafPageForSingleConditionは、
			// 0を返すので、ここで、“キー値順での最終オブジェクト”と
			// 検索条件を比較してみる。

			PhysicalFile::PageID	lastLeafPageID =
				FileInfo_.readLastLeafPageID();

			leafPage = File::attachPage(this->m_pTransaction,
										this->m_pPhysicalFile,
										lastLeafPageID,
										this->m_FixMode,
										this->m_CatchMemoryExhaust,
										AttachNodePages_);

			; _SYDNEY_ASSERT(leafPage != 0);

			NodePageHeader	leafPageHeader(this->m_pTransaction,
										   leafPage,
										   true); // リーフページ

			ModUInt32	useKeyInfoNum =
				leafPageHeader.readUseKeyInformationNumber();

			; _SYDNEY_ASSERT(useKeyInfoNum > 0);

			KeyInformation	keyInfo(this->m_pTransaction,
									leafPage,
									useKeyInfoNum - 1,
									true, // リーフページ
									this->m_cFileParameter.m_KeyNum,
									this->m_cFileParameter.m_KeySize);

			const NullBitmap::Value*	nullBitmapTop =
				keyInfo.assignConstKeyNullBitmap();

			; _SYDNEY_ASSERT(nullBitmapTop != 0);

			if (this->compareToTopSearchCondition(nullBitmapTop) > 0)
			{
				// すべてのオブジェクトが検索条件と一致する…

				resultObjectID = keyInfo.readValueObjectID();
			}

			if (KeyInfoLeafPageID_ != 0)
			{
				// File::searchBySpanConditionRevSimpleKeyから呼ばれた…

				// ならば、
				// リーフページのキーオブジェクトに関する情報を
				// 設定する必要がある。

				; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

				ValueFile_->readLeafInfo(resultObjectID,
										 AttachValuePages_,
										 this->m_CatchMemoryExhaust,
										 *KeyInfoLeafPageID_,
										 *KeyInfoIndex_);
			}

			checkMemoryExhaust(leafPage);

			return resultObjectID;
		}

		PhysicalFile::PageID	leafPageID = leafPage->getID();

		bool	match = false; // dummy

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		int	keyInfoIndex =
			this->getSimpleKeyInformationIndexForSingleCondition(
				leafPage,
				leafPageHeader.readUseKeyInformationNumber(),
				true, // リーフページ
				match);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								keyInfoIndex,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		resultObjectID = keyInfo.readValueObjectID();

		int	compareResult =
			this->compareToTopSearchCondition(nullBitmapTop);

		if ((ContainEquals_ == false && compareResult <= 0) ||
			(ContainEquals_ && compareResult < 0))
		{
			bool	lp = true;
			do
			{
				if (this->assignPrevKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					// なかった…

					checkMemoryExhaust(leafPage);

					return FileCommon::ObjectID::Undefined;
				}

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				compareResult =
					this->compareToTopSearchCondition(nullBitmapTop);

				if (compareResult > 0 ||
					(ContainEquals_ && compareResult == 0))
				{
					resultObjectID = keyInfo.readValueObjectID();

					lp = false;

					if (KeyInfoLeafPageID_ != 0)
					{
						// searchBySpanConditionから呼ばれた…

						// ならば、
						// リーフページのキーオブジェクトに関する情報を
						// 設定する必要がある。

						; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

						*KeyInfoLeafPageID_ = leafPage->getID();

						*KeyInfoIndex_ = keyInfo.getIndex();
					}
				}
			}
			while (lp);
		}
		else
		{
			bool	lp = true;
			do
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// searchBySpanConditionから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある。

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();
				}

				ModUInt64	saveObjectID = resultObjectID;

				if (this->assignNextKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					resultObjectID = saveObjectID;

					break;
				}

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				compareResult =
					this->compareToTopSearchCondition(nullBitmapTop);

				if (compareResult < 0 ||
					(ContainEquals_ == false && compareResult == 0))
				{
					lp = false;
				}
				else
				{
					resultObjectID = keyInfo.readValueObjectID();
				}
			}
			while (lp);
		}
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleEqualsToNullRevSimpleKey --
//		先頭キーフィールドの値がヌル値のオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値がヌル値のオブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値がヌル値のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleEqualsToNullRevSimpleKey(
	FileInformation&	FileInfo_,
	PageVector&			AttachNodePages_) const
{
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		// ということは、
		// ”キー値順に先頭オブジェクト”が
		// ヌル値かな？とチェックして、
		// ヌル値ならば、先頭から順にヌル値かどうかを
		// チェックしていって、ヌル値ではないオブジェクトの
		// 手前のオブジェクトを返せばよい。
		// ヌル値でないのなら、検索条件と一致するオブジェクトは
		// 存在しないこととなる。

		PhysicalFile::PageID	topLeafPageID =
			FileInfo_.readTopLeafPageID();

		leafPage = File::attachPage(this->m_pTransaction,
									this->m_pPhysicalFile,
									topLeafPageID,
									this->m_FixMode,
									this->m_CatchMemoryExhaust,
									AttachNodePages_);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								0,    // 先頭のキー情報
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (NullBitmap::isNull(nullBitmapTop,
							   this->m_cFileParameter.m_KeyNum,
							   0)
			== false)
		{
			// 先頭キーフィールドにヌル値が
			// 記録されているオブジェクトは
			// 存在しない…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		// 先頭オブジェクトの先頭キーフィールドに
		// ヌル値が記録されているので、ここに来た。

		resultObjectID = keyInfo.readValueObjectID();

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		bool	lp = true;
		do
		{
			ModUInt64	saveObjectID = resultObjectID;

			if (this->assignNextKeyInformation(leafPage,
											   AttachNodePages_,
											   leafPageHeader,
											   keyInfo)
				== false)
			{
				resultObjectID = saveObjectID;

				break;
			}

			nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

			; _SYDNEY_ASSERT(nullBitmapTop != 0);

			if (NullBitmap::isNull(nullBitmapTop,
								   this->m_cFileParameter.m_KeyNum,
								   0)
				== false)
			{
				lp = false;
			}
			else
			{
				resultObjectID = keyInfo.readValueObjectID();
			}
		}
		while (lp);
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		// ということは、
		// “キー値順に最終オブジェクト”が
		// ヌル値かな？とチェックして、
		// ヌル値ならばそのオブジェクトを返せばよいし、
		// そうでなければ検索条件と一致するオブジェクトは
		// 存在しないこととなる。

		PhysicalFile::PageID	lastLeafPageID =
			FileInfo_.readLastLeafPageID();

		leafPage = File::attachPage(this->m_pTransaction,
									this->m_pPhysicalFile,
									lastLeafPageID,
									this->m_FixMode,
									this->m_CatchMemoryExhaust,
									AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		ModUInt32	useKeyInfoNum =
			leafPageHeader.readUseKeyInformationNumber();

		; _SYDNEY_ASSERT(useKeyInfoNum > 0);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								useKeyInfoNum - 1,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (NullBitmap::isNull(nullBitmapTop,
							   this->m_cFileParameter.m_KeyNum,
							   0)
			== false)
		{
			// 先頭キーフィールドにヌル値が
			// 記録されているオブジェクトは
			// 存在しない…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		// 最終オブジェクトの先頭キーフィールドに
		// ヌル値が記録されているので、ここに来た。

		resultObjectID = keyInfo.readValueObjectID();
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySpanConditionRevSimpleKey --
//		先頭キーフィールドの値が検索条件で指定された範囲内の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件で指定された範囲内の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
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
//	ModUInt64
//		先頭キーフィールドの値が検索条件で指定された範囲内のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySpanConditionRevSimpleKey(
	FileInformation&	FileInfo_,
	PageVector&			AttachNodePages_,
	ValueFile*			ValueFile_,
	PageVector&			AttachValuePages_) const
{
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		//
		// まずは、検索終了条件と一致するオブジェクトを検索する。
		//

		bool	containEquals =
			(this->m_SearchHint.m_StopOperator ==
			 LogicalFile::TreeNodeInterface::LessThanEquals);

		PhysicalFile::PageID	keyInfoLeafPageID =
			PhysicalFile::ConstValue::UndefinedPageID;

		ModUInt32	keyInfoIndex = ModUInt32Max;

		resultObjectID =
			this->searchBySingleLessThanRevSimpleKey(
				FileInfo_,
				AttachNodePages_,
				ValueFile_,
				AttachValuePages_,
				containEquals,
				SearchHint::CompareTarget::Stop,
				&keyInfoLeafPageID,
				&keyInfoIndex);

		if (resultObjectID == FileCommon::ObjectID::Undefined)
		{
			// 検索終了条件と一致するオブジェクトが
			// ファイル内に存在しなかった…

			return resultObjectID;
		}

		; _SYDNEY_ASSERT(
			keyInfoLeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

		; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

		//
		// 検索開始条件とも一致するかチェックする。
		//

		PhysicalFile::Page*	keyInfoLeafPage =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 keyInfoLeafPageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		KeyInformation	keyInfo(this->m_pTransaction,
								keyInfoLeafPage,
								keyInfoIndex,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(
				nullBitmapTop,
				SearchHint::CompareTarget::Start);

		checkMemoryExhaust(keyInfoLeafPage);

		if (compareResult > 0)
		{
			// 検索開始条件とは一致しなかった…

			// ということは、ファイル内に
			// 指定範囲内のオブジェクトは存在しない。

			return FileCommon::ObjectID::Undefined;
		}

		if (compareResult == 0 &&
			*this->m_SearchHint.m_StartOperatorArray ==
			LogicalFile::TreeNodeInterface::GreaterThan)
		{
			// 同上

			return FileCommon::ObjectID::Undefined;
		}
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		//
		// まずは、検索開始条件と一致するオブジェクトを検索する
		//

		bool	containEquals =
			(*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals);

		PhysicalFile::PageID	keyInfoLeafPageID =
			PhysicalFile::ConstValue::UndefinedPageID;

		ModUInt32	keyInfoIndex = ModUInt32Max;

		resultObjectID =
			this->searchBySingleGreaterThanRevSimpleKey(
				FileInfo_,
				AttachNodePages_,
				ValueFile_,
				AttachValuePages_,
				containEquals,
				&keyInfoLeafPageID,
				&keyInfoIndex);

		if (resultObjectID == FileCommon::ObjectID::Undefined)
		{
			// 検索開始条件と一致するオブジェクトが
			// ファイル内に存在しなかった…

			return resultObjectID;
		}

		; _SYDNEY_ASSERT(
			keyInfoLeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

		; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

		//
		// 検索終了条件とも一致するかチェックする。
		//

		PhysicalFile::Page*	keyInfoLeafPage =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 keyInfoLeafPageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		KeyInformation	keyInfo(this->m_pTransaction,
								keyInfoLeafPage,
								keyInfoIndex,
								true, // リーフページ
								this->m_cFileParameter.m_KeyNum,
								this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(
				nullBitmapTop,
				SearchHint::CompareTarget::Stop);

		checkMemoryExhaust(keyInfoLeafPage);

		if (compareResult > 0)
		{
			// 検索終了条件とは一致しなかった…

			// ということは、ファイル内に
			// 指定範囲内のオブジェクトは存在しない。

			return FileCommon::ObjectID::Undefined;
		}

		if (compareResult == 0 &&
			this->m_SearchHint.m_StopOperator ==
			LogicalFile::TreeNodeInterface::LessThan)
		{
			// 同上

			return FileCommon::ObjectID::Undefined;
		}
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchByMultiConditionRevSimpleKey --
//		複数キーフィールドの値が検索条件で指定された複合条件と一致する
//		オブジェクトを検索する
//
//	NOTES
//	複数キーフィールドの値が検索条件で指定された複合条件と一致する
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
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
//		いずれの検索対象キーフィールドの値も、
//		そのキーフィールドに対する検索条件と一致するオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchByMultiConditionRevSimpleKey(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	leafPage =
		this->searchSimpleLeafPageForMultiCondition(TreeDepth_,
													RootNodePageID_,
													AttachNodePages_);

	if (leafPage == 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	bool	match = false;

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	int	keyInfoIndex =
		this->getSimpleKeyInformationIndexForMultiCondition(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
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

	bool	isEOF = true;

	while (this->assignNextKeyInformation(leafPage,
										  AttachNodePages_,
										  leafPageHeader,
										  keyInfo))
	{
		if (leafPage->getID() != leafPageID)
		{
			leafPageID = leafPage->getID();

			leafPageHeader.resetPhysicalPage(leafPage);
		}

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToMultiSearchCondition(nullBitmapTop) != 0)
		{
			this->assignPrevKeyInformation(leafPage,
										   AttachNodePages_,
										   leafPageHeader,
										   keyInfo);

			if (leafPage->getID() != leafPageID)
			{
				leafPageID = leafPage->getID();

				leafPageHeader.resetPhysicalPage(leafPage);
			}

			isEOF = false;

			break;
		}
	}

	if (isEOF)
	{
		leafPage = File::attachPage(this->m_pTransaction,
									this->m_pPhysicalFile,
									leafPageID,
									this->m_FixMode,
									this->m_CatchMemoryExhaust,
									AttachNodePages_);

		leafPageHeader.resetPhysicalPage(leafPage);
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	if (this->m_SearchHint.m_ExistSeparateKey ||
		this->m_SearchHint.m_LastStartOperatorIsEquals == false)
	{
		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (this->compareToLastCondition(nullBitmapTop) == false)
		{
			resultObjectID = FileCommon::ObjectID::Undefined;

			while (this->assignPrevKeyInformation(leafPage,
												  AttachNodePages_,
												  leafPageHeader,
												  keyInfo))
			{
				if (leafPage->getID() != leafPageID)
				{
					leafPageID = leafPage->getID();

					leafPageHeader.resetPhysicalPage(leafPage);
				}

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				if (this->compareToMultiSearchCondition(nullBitmapTop)
					!= 0)
				{
					break;
				}

				if (this->compareToLastCondition(nullBitmapTop))
				{
					resultObjectID = keyInfo.readValueObjectID();

					break;
				}
			}
		}
	}

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySingleEqualsSimpleKey --
//		先頭キーフィールドの値が検索条件と一致する次のオブジェクトのIDを返す
//
//	NOTES
//	先頭キーフィールドの値が検索条件と一致する、
//	キー値順で（ファイル終端方向に）次のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件と一致するオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySingleEqualsSimpleKey(
	PageVector&	AttachNodePages_) const
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

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	const NullBitmap::Value*	nullBitmapTop =
		keyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	// なぜ、File::compareToTopSearchConditionの第1引数を
	// this->m_LeafPageIDにしていないのかというと、
	// ちょっと前に呼び出したFile::assignNextKeyInformationが
	// leafPageを変えているかもしれないから。

	if (this->compareToTopSearchCondition(nullBitmapTop) != 0)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySingleLessThanSimpleKey --
//		先頭キーフィールドの値が検索条件以下（または未満）の
//		次のオブジェクトのIDを返す
//
//	NOTES
//	先頭キーフィールドの値が検索条件以下（または未満）の、
//	キー値順で（ファイル終端方向に）次のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//	const bool			ContainEquals_
//		比較演算子がLessThanとLessThanEqualsのどちらか
//			true  : LessThanEquals
//			false : LessThan
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以下（または未満）のオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySingleLessThanSimpleKey(
	PageVector&	AttachNodePages_,
	const bool	ContainEquals_) const
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

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	// 先頭キーフィールドのソート順が降順で、
	// そのソート順にオブジェクトを返していくのであれば、
	// 検索条件との比較は不要である。
	// ずーっと、キー値順での最終オブジェクトまで、
	// 検索条件を満たしているはずなので。

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(nullBitmapTop);

		if (compareResult < 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySingleGreaterThanSimpleKey --
//		先頭キーフィールドの値が検索条件以上（または超）の
//		次のオブジェクトのIDを返す
//
//	NOTES
//	先頭キーフィールドの値が検索条件以上（または超）の、
//	キー値順で（ファイル終端方向に）次のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//	const bool			ContainEquals_
//		比較演算子がGreaterThanとGreaterThanEqualsのどちらか
//			true  : GreaterThanEquals
//			false : GreaterThan
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以上（または超）のオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySingleGreaterThanSimpleKey(
	PageVector&	AttachNodePages_,
	const bool	ContainEquals_) const
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

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	// 先頭キーフィールドのソート順が昇順で、
	// そのソート順にオブジェクトを返していくのであれば、
	// 検索条件との比較は不要である。
	// ずーっと、キー値順での最終オブジェクトまで、
	// 検索条件を満たしているはずなので。

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Descending)
	{
		// 先頭キーフィールドのソート順は降順…

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(nullBitmapTop);

		if (compareResult < 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySingleEqualsToNullSimpleKey --
//		先頭キーフィールドの値がヌル値の次のオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値がヌル値の、
//	キー値順で（ファイル終端方向に）次のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値がヌル値のオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySingleEqualsToNullSimpleKey(
	PageVector&	AttachNodePages_) const
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

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	// 先頭キーフィールドのソート順が降順で、
	// そのソート順にオブジェクトを返していくのであれば、
	// 検索条件との比較は不要である。
	// ずーっと、キー値順での最終オブジェクトまで、
	// 先頭キーフィールドにヌル値が記録されているはずなので。

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (NullBitmap::isNull(nullBitmapTop,
							   this->m_cFileParameter.m_KeyNum,
							   0)
			== false)
		{
			// 先頭キーフィールドがヌル値ではなかった…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySpanConditionSimpleKey --
//		先頭キーフィールドの値が検索条件で指定された範囲内の
//		次のオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件で指定された範囲内の、
//	キー値順で（ファイル終端方向に）次のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件で指定された範囲内のオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySpanConditionSimpleKey(
	PageVector&	AttachNodePages_) const
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

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	const NullBitmap::Value*	nullBitmapTop =
		keyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	SearchHint::CompareTarget::Value	compareTarget =
		SearchHint::CompareTarget::Undefined;

	bool	containEquals = false;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		compareTarget = SearchHint::CompareTarget::Stop;

		containEquals =
			(this->m_SearchHint.m_StopOperator ==
			 LogicalFile::TreeNodeInterface::LessThanEquals);
	}
	else
	{
		compareTarget = SearchHint::CompareTarget::Start;

		containEquals =
			(*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals);
	}

	int	compareResult = this->compareToTopSearchCondition(nullBitmapTop,
														  compareTarget);

	checkMemoryExhaust(leafPage);

	if (compareResult < 0)
	{
		// 検索条件と一致しなかった…

		// ということは、ファイル内に
		// 指定範囲内のオブジェクトはもう存在しない。

		return FileCommon::ObjectID::Undefined;
	}

	if (compareResult == 0 && containEquals == false)
	{
		// 同上

		return FileCommon::ObjectID::Undefined;
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDByMultiConditionSimpleKey --
//		複数キーフィールドの値が検索条件で指定された複合条件と一致する
//		次のオブジェクトのIDを返す
//
//	NOTES
//	複数キーフィールドの値が検索条件で指定された複合条件と一致する、
//	キー値順で（ファイル終端方向に）次のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		いずれの検索対象キーフィールドの値も、
//		そのキーフィールドに対する検索条件と一致するオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDByMultiConditionSimpleKey(
	PageVector&	AttachNodePages_) const
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
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	const NullBitmap::Value*	nullBitmapTop =
		keyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	if (leafPage->getID() != leafPageID)
	{
		leafPageID = leafPage->getID();

		leafPageHeader.resetPhysicalPage(leafPage);
	}

	if (this->compareToMultiSearchCondition(nullBitmapTop) != 0)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	if (this->m_SearchHint.m_ExistSeparateKey ||
		this->m_SearchHint.m_LastStartOperatorIsEquals == false)
	{
		if (this->compareToLastCondition(nullBitmapTop) == false)
		{
			resultObjectID = FileCommon::ObjectID::Undefined;

			while (this->assignNextKeyInformation(leafPage,
												  AttachNodePages_,
												  leafPageHeader,
												  keyInfo))
			{
				if (leafPage->getID() != leafPageID)
				{
					leafPageID = leafPage->getID();

					leafPageHeader.resetPhysicalPage(leafPage);
				}

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				if (this->compareToMultiSearchCondition(nullBitmapTop)
					!= 0)
				{
					break;
				}

				if (this->compareToLastCondition(nullBitmapTop))
				{
					resultObjectID = keyInfo.readValueObjectID();

					break;
				}
			}
		}
	}

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySingleEqualsSimpleKey --
//		先頭キーフィールドの値が検索条件と一致する前のオブジェクトのIDを返す
//
//	NOTES
//	先頭キーフィールドの値が検索条件と一致する、
//	キー値順で（ファイル先頭方向に）前のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件と一致するオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySingleEqualsSimpleKey(
	PageVector&	AttachNodePages_) const
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

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	const NullBitmap::Value*	nullBitmapTop =
		keyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	if (this->compareToTopSearchCondition(nullBitmapTop) != 0)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySingleLessThanSimpleKey --
//		先頭キーフィールドの値が検索条件以下（または未満）の
//		前のオブジェクトのIDを返す
//
//	NOTES
//	先頭キーフィールドの値が検索条件以下（または未満）の、
//	キー値順で（ファイル先頭方向に）前のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//	const bool			ContainEquals_
//		比較演算子がLessThanとLessThanEqualsのどちらか
//			true  : LessThanEquals
//			false : LessThan
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以下（または未満）のオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySingleLessThanSimpleKey(
	PageVector&	AttachNodePages_,
	const bool	ContainEquals_) const
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

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Descending)
	{
		// 先頭キーフィールドのソート順は降順…

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(nullBitmapTop);

		if (compareResult > 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySingleGreaterThanSimpleKey --
//		先頭キーフィールドの値が検索条件以上（または超）の
//		前のオブジェクトのIDを返す
//
//	NOTES
//	先頭キーフィールドの値が検索条件以上（または超）の、
//	キー値順で（ファイル先頭方向に）前のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//	const bool			ContainEquals_
//		比較演算子がGreaterThanとGreaterThanEqualsのどちらか
//			true  : GreaterThanEquals
//			false : GreaterThan
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以上（または超）のオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySingleGreaterThanSimpleKey(
	PageVector&	AttachNodePages_,
	const bool	ContainEquals_) const
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

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// 前回返したオブジェクトが
		// “キー値順での先頭オブジェクト”であった…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult =
			this->compareToTopSearchCondition(nullBitmapTop);

		if (compareResult > 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySingleEqualsToNullSimpleKey --
//		先頭キーフィールドの値がヌル値の前のオブジェクトを検索する
//
//	NOTES
//		先頭キーフィールドの値がヌル値の、
//	キー値順で（ファイル先頭方向に）前のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ)
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値がヌル値のオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySingleEqualsToNullSimpleKey(
	PageVector&	AttachNodePages_) const
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

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Descending)
	{
		// 先頭キーフィールドのソート順は降順…

		const NullBitmap::Value*	nullBitmapTop =
			keyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		if (NullBitmap::isNull(nullBitmapTop,
							   this->m_cFileParameter.m_KeyNum,
							   0)
			== false)
		{
			// 先頭キーフィールドがヌル値ではなかった…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySpanConditionSimpleKey --
//		先頭キーフィールドの値が検索条件で指定された範囲内の
//		前のオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件で指定された範囲内の、
//	キー値順で（ファイル先頭方向に）前のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件で指定された範囲内のオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySpanConditionSimpleKey(
	PageVector&	AttachNodePages_) const
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

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	const NullBitmap::Value*	nullBitmapTop =
		keyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	SearchHint::CompareTarget::Value	compareTarget =
		SearchHint::CompareTarget::Undefined;

	bool	containEquals = false;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		compareTarget = SearchHint::CompareTarget::Start;

		containEquals =
			(*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals);
	}
	else
	{
		compareTarget = SearchHint::CompareTarget::Stop;

		containEquals =
			(this->m_SearchHint.m_StopOperator ==
			 LogicalFile::TreeNodeInterface::LessThanEquals);
	}

	int	compareResult = this->compareToTopSearchCondition(nullBitmapTop,
														  compareTarget);

	checkMemoryExhaust(leafPage);

	if (compareResult > 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	if (compareResult == 0 && containEquals == false)
	{
		return FileCommon::ObjectID::Undefined;
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDByMultiConditionSimpleKey --
//		複数キーフィールドの値が検索条件で指定された複合条件と一致する
//		前のオブジェクトのIDを返す
//
//	NOTES
//	複数キーフィールドの値が検索条件で指定された複合条件と一致する、
//	キー値順で（ファイル先頭方向に）前のオブジェクトのIDを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		いずれの検索対象キーフィールドの値も、
//		そのキーフィールドに対する検索条件と一致するオブジェクトのID
//		もうファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDByMultiConditionSimpleKey(
	PageVector&	AttachNodePages_) const
{
	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

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
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	const NullBitmap::Value*	nullBitmapTop =
		keyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	if (leafPage->getID() != leafPageID)
	{
		leafPageID = leafPage->getID();

		leafPageHeader.resetPhysicalPage(leafPage);
	}

	if (this->compareToMultiSearchCondition(nullBitmapTop) != 0)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	if (this->m_SearchHint.m_ExistSeparateKey ||
		this->m_SearchHint.m_LastStartOperatorIsEquals == false)
	{
		if (this->compareToLastCondition(nullBitmapTop) == false)
		{
			resultObjectID = FileCommon::ObjectID::Undefined;

			while (this->assignPrevKeyInformation(leafPage,
												  AttachNodePages_,
												  leafPageHeader,
												  keyInfo))
			{
				if (leafPage->getID() != leafPageID)
				{
					leafPageID = leafPage->getID();

					leafPageHeader.resetPhysicalPage(leafPage);
				}

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				if (this->compareToMultiSearchCondition(nullBitmapTop) != 0)
				{
					break;
				}

				if (this->compareToLastCondition(nullBitmapTop))
				{
					resultObjectID = keyInfo.readValueObjectID();

					break;
				}
			}
		}
	}

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::compareToLastCondition --
//		最終検索対象キーフィールドの値と検索条件を比較する
//
//	NOTES
//	最終検索対象キーフィールドの値と、そのキーフィールドに対する
//	検索条件を比較する。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*	NullBitmapTop_
//		ヌルビットマップ先頭へのポインタ
//
//	RETURN
//	bool
//		比較結果
//			true  : 最終検索対象キーフィールドの値が検索条件と一致する
//			false : 最終検索対象キーフィールドの値が検索条件と一致しない
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareToLastCondition(
	const NullBitmap::Value*	NullBitmapTop_) const
{
	bool	match = false;

	LogicalFile::TreeNodeInterface::Type	lastCompOpe =
		*(this->m_SearchHint.m_StartOperatorArray +
		  (this->m_SearchHint.m_FieldNumber - 1));

	if (lastCompOpe == LogicalFile::TreeNodeInterface::Equals)
	{
		// 比較演算子がEquals…

		match = this->compareToLastEquals(NullBitmapTop_);
	}
	else if (lastCompOpe == LogicalFile::TreeNodeInterface::LessThan)
	{
		// 比較演算子がLessThan…

		match = this->compareToLastLessThan(NullBitmapTop_,
											false); // Equals含まず
	}
	else if (lastCompOpe == LogicalFile::TreeNodeInterface::LessThanEquals)
	{
		// 比較演算子がLessThanEquals…

		match = this->compareToLastLessThan(NullBitmapTop_,
											true); // Equals含む
	}
	else if (lastCompOpe == LogicalFile::TreeNodeInterface::GreaterThan)
	{
		// 比較演算子がGreaterThan…

		//
		// ※ 範囲指定を含む。
		//

		match = this->compareToLastGreaterThan(NullBitmapTop_,
											   false); // Equals含まず
	}
	else if (lastCompOpe == LogicalFile::TreeNodeInterface::GreaterThanEquals)
	{
		// 比較演算子がGreaterThanEquals…

		match = this->compareToLastGreaterThan(NullBitmapTop_,
											   true); // Equals含む
	}
	else
	{
		// 比較演算子がEqualsToNull…
		// （のはず。）

		; _SYDNEY_ASSERT(
			lastCompOpe == LogicalFile::TreeNodeInterface::EqualsToNull);

		match = this->compareToLastEqualsToNull(NullBitmapTop_);
	}

	return match;
}

//
//	FUNCTION private
//	Btree::File::compareToLastEquals --
//		最終検索対象キーフィールドの値と検索条件が一致するかどうかを調べる
//
//	NOTES
//	最終検索対象キーフィールドの値と、そのキーフィールドに対する
//	検索条件が一致するかどうかを調べる。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*	NullBitmapTop_
//		ヌルビットマップ先頭へのポインタ
//
//	RETURN
//	bool
//		最終検索対象キーフィールドの値と検索条件が一致するかどうか
//			true  : 一致する
//			false : 一致しない
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareToLastEquals(const NullBitmap::Value*	NullBitmapTop_) const
{
	return (this->compareToLastSearchCondition(NullBitmapTop_) == 0);
}

//
//	FUNCTION private
//	Btree::File::compareToLastLessThan --
//		最終検索対象キーフィールドの値が検索条件以下（または未満）かどうかを
//		調べる
//
//	NOTES
//	最終検索対象キーフィールドの値が、そのキーフィールドに対する
//	検索条件以下（または未満）かどうかを調べる。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*	NullBitmapTop_
//		ヌルビットマップ先頭へのポインタ
//	const bool						ContainEquals_
//		比較演算子がLessThanとLessThanEqualsのどちらか
//			true  : LessThanEquals
//			false : LessThan
//
//	RETURN
//	bool
//		最終検索対象キーフィールドの値が検索条件以下（または未満）かどうか
//			true  : 検索条件以下（または未満）
//			false : 検索条件超（または以上）
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareToLastLessThan(
	const NullBitmap::Value*	NullBitmapTop_,
	const bool					ContainEquals_) const
{
	int	compareResult = this->compareToLastSearchCondition(NullBitmapTop_);

	int	arrayIndex = this->m_SearchHint.m_FieldNumber - 1;

	int	fieldIndex = *(this->m_SearchHint.m_FieldIndexArray + arrayIndex);

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + fieldIndex) ==
		FileParameter::SortOrder::Ascending)
	{
		return ContainEquals_ ? (compareResult >= 0) : (compareResult > 0);
	}
	else
	{
		return ContainEquals_ ? (compareResult <= 0) : (compareResult < 0);
	}
}

//
//	FUNCTION private
//	Btree::File::compareToLastGreaterThan --
//		最終検索対象キーフィールドの値が検索条件以上（または超）かどうかを
//		調べる
//
//	NOTES
//	最終検索対象キーフィールドの値が、そのキーフィールドに対する
//	検索条件以上（または超）かどうかを調べる。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*	NullBitmapTop_
//		ヌルビットマップ先頭へのポインタ
//	const bool						ContainEquals_
//		比較演算子がGreaterThanとGreatenEqualsのどちらか
//			true  : GreaterThanEquals
//			false : GreaterThan
//
//	RETURN
//	bool
//		最終検索対象キーフィールドの値が検索条件以上（または超）かどうか
//			true  : 検索条件以上（または超）
//			false : 検索条件未満（または以下）
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareToLastGreaterThan(
	const NullBitmap::Value*	NullBitmapTop_,
	const bool					ContainEquals_) const
{
	int	compareResult = this->compareToLastSearchCondition(NullBitmapTop_);

	bool	match = false;

	int	arrayIndex = this->m_SearchHint.m_FieldNumber - 1;

	int	fieldIndex = *(this->m_SearchHint.m_FieldIndexArray + arrayIndex);

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + fieldIndex) ==
		FileParameter::SortOrder::Ascending)
	{
		match = ContainEquals_ ? (compareResult <= 0) : (compareResult < 0);
	}
	else
	{
		match = ContainEquals_ ? (compareResult >= 0) : (compareResult > 0);
	}

	if (match == false)
	{
		return false;
	}

	if (this->m_SearchHint.m_StopOperator !=
		LogicalFile::TreeNodeInterface::Undefined)
	{
		// 範囲指定…

		; _SYDNEY_ASSERT(this->m_SearchHint.m_StopOperator ==
						 LogicalFile::TreeNodeInterface::LessThan ||
						 this->m_SearchHint.m_StopOperator ==
						 LogicalFile::TreeNodeInterface::LessThanEquals);

		bool	containEquals =
			(this->m_SearchHint.m_StopOperator ==
			 LogicalFile::TreeNodeInterface::LessThanEquals);

		compareResult =
			this->compareToLastSearchCondition(
				NullBitmapTop_,
				SearchHint::CompareTarget::Stop);

		if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray +
			  fieldIndex) ==
			FileParameter::SortOrder::Ascending)
		{
			match =
				containEquals ? (compareResult >= 0) : (compareResult > 0);
		}
		else
		{
			match =
				containEquals ? (compareResult <= 0) : (compareResult < 0);
		}
	}

	return match;
}

//
//	FUNCTION private
//	Btree::File::compareToLastEqualsToNull --
//		最終検索対象キーフィールドの値がヌル値かどうかを調べる
//
//	NOTES
//	最終検索対象キーフィールドの値がヌル値かどうかを調べる。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*	NullBitmapTop_
//		ヌルビットマップ先頭へのポインタ
//
//	RETURN
//	bool
//		最終検索対象キーフィールドの値がヌル値かどうか
//			true  : ヌル値
//			false : ヌル値ではない
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareToLastEqualsToNull(
	const NullBitmap::Value*	NullBitmapTop_) const
{
	int	keyIndex =
		*(this->m_SearchHint.m_FieldIndexArray +
		  (this->m_SearchHint.m_FieldNumber - 1));

	return NullBitmap::isNull(NullBitmapTop_,
							  this->m_cFileParameter.m_KeyNum,
							  keyIndex - 1);
}

//
//	FUNCTION private
//	Btree::File::compareToLastSearchCondition --
//		最終検索対象キーフィールドの値と検索条件を比較する
//
//	NOTES
//	最終検索対象キーフィールドの値と、そのキーフィールドに対する
//	検索条件を比較する。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*					NullBitmapTop_
//		ヌルビットマップ先頭へのポインタ
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToLastSearchCondition(
	const NullBitmap::Value*				NullBitmapTop_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	return
		this->compareToMultiSearchCondition(
			NullBitmapTop_,
			SearchHint::CompareType::OnlyLastKey,
			Target_);
}

//
//	FUNCTION private
//	Btree::File::searchSimpleLeafPageForMultiCondition --
//		複合条件と一致するキーフィールドの値が記録されている可能性のある
//		リーフページを検索する
//
//	NOTES
//	複数キーフィールドの値が検索条件で指定された複合条件と一致する
//	キーフィールドの値が書かれているキー情報が記録されている可能性のある
//	リーフページを検索する。
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
//		複合条件と一致するキーフィールドの値が記録されている可能性のある
//		リーフページの物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::searchSimpleLeafPageForMultiCondition(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	if (TreeDepth_ == 1)
	{
		// 木の深さ（高さ）が1段の木構造…

		// つまり、ルートノードページがリーフページ

		// では、ルートノードページに検索条件と一致する
		// オブジェクトが記録されている可能性があるかを調べ、
		// その可能性があるならば、ルートノードページの
		// 物理ページ記述子を返せばよい。

		return
			this->containTargetSimpleKeyForMultiCondition(
				RootNodePageID_,
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
			this->searchSimpleChildNodePageForMultiCondition(
				nodePageID,
				AttachNodePages_);

		if (nodePageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 検索条件と一致するオブジェクトが
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
//	Btree::File::searchSimpleChildNodePageForMultiCondition --
//		複合条件と一致するキーフィールドの値が記録されている可能性のある
//		子ノードページを検索する
//
//	NOTES
//	複数キーフィールドの値が検索条件で指定された複合条件と一致する
//	キーフィールドの値が書かれているキー情報が記録されている可能性のある
//	子ノードページを検索する。
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
//		複合条件と一致するキーフィールドの値が記録されている可能性のある
//		子ノードページの物理ページ識別子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::PageID
File::searchSimpleChildNodePageForMultiCondition(
	const PhysicalFile::PageID	ParentNodePageID_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	parentNodePage =
		this->containTargetSimpleKeyForMultiCondition(
			ParentNodePageID_,
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
		this->getSimpleKeyInformationIndexForMultiCondition(
			parentNodePage,
			parentNodePageHeader.readUseKeyInformationNumber(),
			false, // リーフページではない
			match);

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false, // リーフページではない
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	PhysicalFile::PageID	childNodePageID = keyInfo.readChildNodePageID();

	; _SYDNEY_ASSERT(
		childNodePageID != PhysicalFile::ConstValue::UndefinedPageID);

	checkMemoryExhaust(parentNodePage);

	return childNodePageID;
}

//
//	FUNCTION private
//	Btree::File::getSimpleKeyInformationIndexForMultiCondition --
//		複合条件に最も近いキーフィールドの値が書かれているキー情報の
//		インデックスを返す
//
//	NOTES
//	引数KeyInfoPage_が示すノードページ内で、
//	検索条件で指定された複合条件に最も近いキーフィールドの値を検索し、
//	そのキーフィールドの値が書かれているキー情報のインデックスを返す。
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
//		そのキー情報に書かれているキーフィールドの値が、
//		検索条件で指定された複合条件と完全に一致しているかどうか
//			true  : 完全に一致している
//			false : 完全に一致していない
//
//	RETURN
//	int
//		キー情報のインデックス
//
//	EXCEPTIONS
//	[YET!]
//
int
File::getSimpleKeyInformationIndexForMultiCondition(
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

		int	compareResult =
			this->compareToMultiSearchCondition(nullBitmapTop);

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

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				if (this->compareToMultiSearchCondition(nullBitmapTop)
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
//	Btree::File::containTargetSimpleKeyForMultiCondition --
//		複合条件と一致するキーフィールドの値が書かれているキー情報が
//		ノードページ内に存在する可能性があるかどうかを知らせる
//
//	NOTES
//	引数NodePageID_で示されるノードページ内に、
//	検索条件で指定された複合条件と一致する
//	キーフィールドの値が書かれているキー情報が
//	存在する可能性があるかどうかを知らせる。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	NodePageID_
//		調べるノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターのへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool					IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	PhysicalFile::Page*
//		複合条件と一致するキーフィールドの値が書かれているキー情報が
//		ノードページ内に存在する可能性があれば、そのノードページの
//		物理ページ記述子、存在する可能性がなければ、0（ヌルポインタ）。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::containTargetSimpleKeyForMultiCondition(
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

		if (this->compareToMultiSearchCondition(nullBitmapTop) > 0)
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
//	Btree::File::compareToMultiSearchCondition --
//		複合条件とキーフィールドの値を比較する
//
//	NOTES
//	検索条件で指定される複合条件とキー情報に記録されている
//	キーフィールドの値を比較し、比較結果を返す。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*					NullBitmapTop_
//		ヌルビットマップ先頭へのポインタ
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToMultiSearchCondition(
	const NullBitmap::Value*				NullBitmapTop_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	SearchHint::CompareType::Value	compareType =
		SearchHint::CompareType::Undefined;

	if (this->m_SearchHint.m_ExistSeparateKey)
	{
		// 先頭キーフィールドから連続している
		// 検索対象キーフィールドから
		// 離れた検索対象キーフィールドがある…

		compareType = SearchHint::CompareType::OnlyLinkKey;
	}
	else
	{
		// 先頭キーフィールドから連続している
		// 検索対象キーフィールドから
		// 離れた検索対象キーフィールドがない…

		if (this->m_SearchHint.m_LastStartOperatorIsEquals)
		{
			// 最終建託対象キーフィールドへの検索条件がEquals…

			compareType = SearchHint::CompareType::All;
		}
		else
		{
			// 最終建託対象キーフィールドへの検索条件がEquals以外…

			compareType = SearchHint::CompareType::OnlyNotLastKey;
		}
	}

	return this->compareToMultiSearchCondition(NullBitmapTop_,
											   compareType,
											   Target_);
}

//
//	FUNCTION private
//	Btree::File::compareToMultiSearchCondition --
//		複合条件とキーフィールドの値を比較する
//
//	NOTES
//	検索条件で指定される複合条件とキーフィールドの値を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*					NullBitmapTop_
//		ヌルビットマップ先頭へのポインタ
//	const Btree::SearchHint::CompareType::Value		CompareType_
//		比較対象
//			All             : 全ての検索対象キーフィールドを比較
//			OnlyLinkKey     : 連続した検索対象キーフィールドのみ比較
//			OnlySeparateKey : 離れた検索対象キーフィールドのみ比較
//			OnlyLastKey     : 最終検索対象キーフィールドのみ比較
//			OnlyNotLastKey  : 最終検索対象キーフィールド以外を比較
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToMultiSearchCondition(
	const NullBitmap::Value*				NullBitmapTop_,
	const SearchHint::CompareType::Value	CompareType_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	int	compareResult = 0;

	NullBitmap	nullBitmap(NullBitmapTop_,
						   this->m_cFileParameter.m_KeyNum,
						   NullBitmap::Access::ReadOnly);

	bool	existNull = nullBitmap.existNull();

	const void*	topKey = nullBitmap.getConstTail();

	for (int i = 0; i < this->m_SearchHint.m_FieldNumber; i++)
	{
		int	fieldIndex = *(this->m_SearchHint.m_FieldIndexArray + i);

		if (CompareType_ == SearchHint::CompareType::OnlyLinkKey &&
			fieldIndex > this->m_SearchHint.m_LastLinkKeyFieldIndex)
		{
			break;
		}

		if (CompareType_ == SearchHint::CompareType::OnlySeparateKey &&
			fieldIndex <= this->m_SearchHint.m_LastLinkKeyFieldIndex)
		{
			continue;
		}

		if (CompareType_ == SearchHint::CompareType::OnlyLastKey &&
			fieldIndex < this->m_SearchHint.m_LastKeyFieldIndex)
		{
			continue;
		}

		if (CompareType_ == SearchHint::CompareType::OnlyNotLastKey &&
			fieldIndex == this->m_SearchHint.m_LastKeyFieldIndex)
		{
			break;
		}

		; _SYDNEY_ASSERT(*(this->m_SearchHint.m_IsFixedArray + i));

		if (existNull && nullBitmap.isNull(fieldIndex - 1))
		{
			return 1 * *(this->m_SearchHint.m_MultiNumberArray + i);
		}

		const void*	key = this->assignKey(topKey, fieldIndex);

		if (Target_ == SearchHint::CompareTarget::Start)
		{
			compareResult =
				this->m_SearchHint.compareToFixedStartCondition(key, i);
		}
		else
		{
			compareResult =
				this->m_SearchHint.compareToFixedStopCondition(key);
		}

		if (compareResult != 0)
		{
			break;
		}
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::assignKey --
//		キー情報内に記録されているキーフィールドの値を指すポインタを返す
//
//	NOTES
//	キー情報内に記録されているキーフィールドの値を指すポインタを返す。
//
//	ARGUMENTS
//	const void*	TopKey_
//		先頭のキーフィールドの値が記録されている領域へのポインタ
//		（これもキー情報内を指している。）
//	const int	KeyIndex_
//		キーフィールドのインデックス
//
//	RETURN
//	const void*
//		キーフィールドの値を指すポインタ
//
//	EXCEPTIONS
//	なし
//
const void*
File::assignKey(const void*	TopKey_,
				const int	KeyIndex_) const
{
	; _SYDNEY_ASSERT(
		KeyIndex_ > 0 &&
		KeyIndex_ < this->m_cFileParameter.m_TopValueFieldIndex);

	; _SYDNEY_ASSERT(TopKey_ != 0);

	Os::Memory::Size	moveSize = 0;

	for (int i = 1; i < KeyIndex_; i++)
	{
		; _SYDNEY_ASSERT(
			*(this->m_cFileParameter.m_IsFixedFieldArray + i));

		Common::DataType::Type	keyDataType =
			*(this->m_cFileParameter.m_FieldTypeArray + i);

		moveSize +=
			FileCommon::DataManager::getFixedCommonDataArchiveSize(
				keyDataType);
	}

	return static_cast<const char*>(TopKey_) + moveSize;
}

//
//	FUNCTION private
//	Btree::File::searchSimpleLeafPageForSingleCondition --
//		検索条件と一致する先頭キーフィールドの値が記録されている可能性のある
//		リーフページを検索する
//
//	NOTES
//	検索条件と一致する先頭キーフィールドの値が書かれているキー情報が
//	記録されている可能性のあるリーフページを検索する。
//
//	ARGUMENTS
//	const ModUInt32									TreeDepth_
//		現在の木の深さ
//	const PhysicalFile::PageID						RootNodePageID_
//		ルートノードページの物理ページ識別子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	PhysicalFile::Page*
//		検索条件と一致する先頭キーフィールドの値が記録されている可能性のある
//		リーフページの物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::searchSimpleLeafPageForSingleCondition(
	const ModUInt32							TreeDepth_,
	const PhysicalFile::PageID				RootNodePageID_,
	PageVector&								AttachNodePages_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	if (TreeDepth_ == 1)
	{
		return
			this->containTargetSimpleKeyForSingleCondition(
				RootNodePageID_,
				AttachNodePages_,
				true, // リーフページ
				Target_);
	}

	PhysicalFile::PageID	nodePageID = RootNodePageID_;

	for (ModUInt32	depth = 1; depth < TreeDepth_; depth++)
	{
		nodePageID =
			this->searchSimpleChildNodePageForSingleCondition(
				nodePageID,
				AttachNodePages_,
				Target_);

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
//	Btree::File::searchSimpleChildNodePageForSingleCondition --
//		検索条件と一致する先頭キーフィールドの値が記録されている可能性のある
//		子ノードページを検索する
//
//	NOTES
//	検索条件と一致する先頭キーフィールドの値が書かれているキー情報が
//	記録されている可能性のある子ノードページを検索する。
//
//	ARGUMENTS
//	const PhysicalFile::PageID						ParentNodePageID_
//		親ノードページの物理ページ識別子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	PhysicalFile::PageID
//		検索条件と一致する先頭キーフィールドの値が記録されている可能性のある
//		子ノードページの物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::PageID
File::searchSimpleChildNodePageForSingleCondition(
	const PhysicalFile::PageID				ParentNodePageID_,
	PageVector&								AttachNodePages_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	PhysicalFile::Page*	parentNodePage =
		this->containTargetSimpleKeyForSingleCondition(
			ParentNodePageID_,
			AttachNodePages_,
			false, // リーフページではない
			Target_);

	if (parentNodePage == 0)
	{
		return PhysicalFile::ConstValue::UndefinedPageID;
	}

	bool	match = false; // dummy

	NodePageHeader	parentNodePageHeader(this->m_pTransaction,
										 parentNodePage,
										 false); // リーフページではない

	int	keyInfoIndex =
		this->getSimpleKeyInformationIndexForSingleCondition(
			parentNodePage,
			parentNodePageHeader.readUseKeyInformationNumber(),
			false, // リーフページではない
			match,
			Target_);

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false, // リーフページではない
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	PhysicalFile::PageID	childNodePageID = keyInfo.readChildNodePageID();

	; _SYDNEY_ASSERT(
		childNodePageID != PhysicalFile::ConstValue::UndefinedPageID);

	checkMemoryExhaust(parentNodePage);

	return childNodePageID;
}

//
//	FUNCTION private
//	Btree::File::getSimpleKeyInformationIndexForSingleCondition --
//		検索条件に最も近い先頭キーフィールドの値が書かれているキー情報の
//		インデックスを返す
//
//	NOTES
//	引数KeyInfoPage_が示すノードページ内で、
//	検索条件に最も近い先頭キーフィールドの値を検索し、
//	そのキーフィールドの値が書かれているキー情報のインデックスを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*								KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32									UseKeyInfoNum_
//		引数KeyInfoPage_が示すノードページ内の使用中のキー情報数
//	const bool										IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	bool&											Match_
//		そのキー情報に書かれている先頭キーフィールドの値が、
//		検索条件と完全に一致しているかどうか
//			true  : 完全に一致している
//			false : 完全に一致していない
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		キー情報のインデックス
//
//	EXCEPTIONS
//	[YET!]
//
int
File::getSimpleKeyInformationIndexForSingleCondition(
	PhysicalFile::Page*						KeyInfoPage_,
	const ModUInt32							UseKeyInfoNum_,
	const bool								IsLeafPage_,
	bool&									Match_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
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

		int	compareResult =
			this->compareToTopSearchCondition(nullBitmapTop,
											  Target_);

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

				nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

				; _SYDNEY_ASSERT(nullBitmapTop != 0);

				if (this->compareToTopSearchCondition(nullBitmapTop,
													  Target_)
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
//	Btree::File::containTargetSimpleKeyForSingleCondition -- 
//		検索条件と一致する先頭キーフィールドの値が書かれているキー情報が
//		ノードページ内に存在する可能性があるかどうかを知らせる
//
//	NOTES
//	引数NodePageID_で示されるノードページ内に、
//	検索条件と一致する先頭キーフィールドの値が書かれているキー情報が
//	存在する可能性があるかどうかを知らせる。
//
//	ARGUMENTS
//	const PhysicalFile::PageID						NodePageID_
//		調べるノードページの物理ページ識別子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターのへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool										IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	PhysicalFile::Page*
//		検索条件と一致するキーフィールドの値が書かれているキー情報が
//		ノードページ内に存在する可能性があれば、そのノードページの
//		物理ページ記述子、存在する可能性がなければ、0（ヌルポインタ）。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::containTargetSimpleKeyForSingleCondition(
	const PhysicalFile::PageID				NodePageID_,
	PageVector&								AttachNodePages_,
	const bool								IsLeafPage_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
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

		if (this->compareToTopSearchCondition(nullBitmapTop,
											  Target_)
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
//	Btree::File::compareToTopSearchCondition --
//		検索条件と先頭キーフィールドの値を比較する
//
//	NOTES
//	検索条件と先頭キーフィールドの値を比較し、比較結果を返す。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*					NullBitmapTop_
//		ヌルビットマップ先頭へのポインタ
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToTopSearchCondition(
	const NullBitmap::Value*				NullBitmapTop_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	// m_SearchHint.m_FieldTypeArray[0]を取る
	Common::DataType::Type	fieldType =
		*this->m_SearchHint.m_FieldTypeArray;

	; _SYDNEY_ASSERT(*this->m_SearchHint.m_IsFixedArray);

	NullBitmap	nullBitmap(NullBitmapTop_,
						   this->m_cFileParameter.m_KeyNum,
						   NullBitmap::Access::ReadOnly);

	if (nullBitmap.isNull(0))
	{
		return 1 * *this->m_SearchHint.m_MultiNumberArray;
	}

	const void*	firstKey = nullBitmap.getConstTail();

	if (Target_ == SearchHint::CompareTarget::Start)
	{
		return
			this->m_SearchHint.compareToFixedStartCondition(firstKey, 0);
	}
	else
	{
		return
			this->m_SearchHint.compareToFixedStopCondition(firstKey);
	}
}

//
//	FUNCTION private
//	Btree::File::searchAndFetchSimpleKey --
//		Search + Fetchでオブジェクトを検索する
//
//	NOTES
//	Search + Fetchでのオブジェクト検索時、
//	検索条件と一致するオブジェクトが見つかったのちに、
//	Fetch検索条件とも一致するオブジェクトを検索する。
//	つまり、検索条件と一致するオブジェクトの検索は、
//	読み出し側で行い、その後のFetch検索条件と一致するオブジェクトの
//	検索を本関数が行う。
//
//	ARGUMENTS
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Common::DataArrayData*&	ResultObject_
//		検索条件ともFetch検索条件とも一致する
//		オブジェクトを指すポインタ
//	const bool				FirstGet_
//		初回のFile::get()での呼び出しかどうか
//			true  : 初回のFile::get()
//			false : 2回目以降のFile::get()
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::searchAndFetchSimpleKey(PageVector&				AttachNodePages_,
							  ValueFile*				ValueFile_,
							  PageVector&				AttachValuePages_,
							  Common::DataArrayData*	ResultObject_,
							  const bool				FirstGet_)
{
	// 検索条件と一致するオブジェクトは
	// 既に見つかっているはずなので、
	// 適切なオブジェクトIDが設定されているはず。
	; _SYDNEY_ASSERT(this->m_ullObjectID != FileCommon::ObjectID::Undefined);

	//
	// バリューオブジェクトが記録されているバリューページをアタッチする
	//

	PhysicalFile::PageID valuePageID =
		Common::ObjectIDData::getFormerValue(m_ullObjectID);
	PhysicalFile::AreaID valueObjectAreaID =
		Common::ObjectIDData::getLatterValue(m_ullObjectID);

	PhysicalFile::Page*	valuePage =
		File::attachPage(this->m_pTransaction,
						 ValueFile_->m_PhysicalFile,
						 valuePageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachValuePages_);

	//
	// Fetch検索条件を設定する
	//

	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getType() == Common::DataType::Array);
	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getElementType() == Common::DataType::Data);

	const Common::DataArrayData*	fetchCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 this->m_FetchOptionData.get());

	; _SYDNEY_ASSERT(fetchCondition != 0);

	if (FirstGet_)
	{
		// 初回のgetで呼び出された…

		// では、Fetchのためのヒントを設定する。
		this->m_FetchHint.setCondition(fetchCondition);
	}

	//
	// まずは、キーフィールドがFetch検索条件と一致しているかを
	// 確認する。
	//

	// バリューオブジェクトから、
	// リーフページの物理ページ識別子と
	// キー情報のインデックスを読み込む。

	const void*	objectAreaTop = File::getConstAreaTop(valuePage,
													  valueObjectAreaID);

	ValueFile_->readLeafInfo(objectAreaTop,
							 this->m_LeafPageID,
							 this->m_KeyInfoIndex);

	// キー情報が記録されているリーフページをアタッチする
	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	// キー情報から、キーオブジェクトのオブジェクトIDを読み込む
	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	const NullBitmap::Value*	nullBitmapTop =
		keyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	bool	exist = true;

	if (this->compareToFetchCondition(nullBitmapTop) != 0)
	{
		// キーフィールドがFetch検索条件と一致しなかった…

		//
		// しょうがないので、検索条件ともFetch検索条件とも
		// 一致するキーオブジェクトを
		// “次、次…”（または“前、前…”）と
		// 検索していく…
		// （下はそのためのループ）
		//

		bool	lp = true;
		do
		{
			if (this->m_pOpenParameter->m_bSortReverse)
			{
				// オブジェクトの挿入ソート順とは逆順に
				// オブジェクトをget…

				// “前、前…”
				this->m_ullObjectID =
					this->getPrevObjectIDSearch(AttachNodePages_);
			}
			else
			{
				// オブジェクトの挿入ソート順にオブジェクトをget…

				// “次、次…”
				this->m_ullObjectID =
					this->getNextObjectIDSearch(AttachNodePages_);
			}

			if (this->m_ullObjectID == FileCommon::ObjectID::Undefined)
			{
				// 検索条件にすら一致するオブジェクトがもうない…

				exist = false;

				break;
			}

			valuePageID = Common::ObjectIDData::getFormerValue(m_ullObjectID);
			valueObjectAreaID =
				Common::ObjectIDData::getLatterValue(m_ullObjectID);

			if (valuePage->getID() != valuePageID)
			{
				// 今回のオブジェクトは、
				// 以前アタッチしていたバリューページとは、
				// 別のバリューページに記録されている…

				// バリューページのアタッチし直し

				if (this->m_CatchMemoryExhaust)
				{
					ValueFile_->m_PhysicalFile->detachPage(
						valuePage,
						PhysicalFile::Page::UnfixMode::NotDirty,
						false); // 本当にデタッチ（アンフィックス）
						        // してしまい、
						        // 物理ファイルマネージャが
						        // キャッシュしないようにする。
				}

				valuePage = File::attachPage(this->m_pTransaction,
											 ValueFile_->m_PhysicalFile,
											 valuePageID,
											 this->m_FixMode,
											 this->m_CatchMemoryExhaust,
											 AttachValuePages_);
			}

			// バリューオブジェクトから、
			// リーフページの物理ページ識別子と
			// キー情報のインデックスを読み込む。

			const void*	objectAreaTop =
				File::getConstAreaTop(valuePage,
									  valueObjectAreaID);

			ValueFile_->readLeafInfo(objectAreaTop,
									 this->m_LeafPageID,
									 this->m_KeyInfoIndex);

			if (leafPage->getID() != this->m_LeafPageID)
			{
				// キー情報が記録されているリーフページのアタッチし直し

				checkMemoryExhaust(leafPage);

				leafPage = File::attachPage(this->m_pTransaction,
											this->m_pPhysicalFile,
											this->m_LeafPageID,
											this->m_FixMode,
											this->m_CatchMemoryExhaust,
											AttachNodePages_);

				keyInfo.resetPhysicalPage(leafPage);
			}

			keyInfo.setStartOffsetByIndex(this->m_KeyInfoIndex);

			nullBitmapTop = keyInfo.assignConstKeyNullBitmap();

			; _SYDNEY_ASSERT(nullBitmapTop != 0);

			if (this->compareToFetchCondition(nullBitmapTop) == 0)
			{
				// 検索条件ともFetch検索条件とも
				// 一致するキーオブジェクトが見つかった…

				break;
			}
		}
		while (lp);
	}

	if (exist == false)
	{
		// 検索条件ともFetch検索条件とも一致するオブジェクトは
		// 存在しなかった…

		this->m_LeafPageID = PhysicalFile::ConstValue::UndefinedPageID;
		this->m_KeyInfoIndex = ModUInt32Max;

		if (this->m_CatchMemoryExhaust)
		{
			ValueFile_->m_PhysicalFile->detachPage(valuePage,PhysicalFile::Page::UnfixMode::NotDirty,false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}
		checkMemoryExhaust(leafPage);

		return false;
	}

	checkMemoryExhaust(leafPage);

	//
	// キーフィールドがFetch検索条件と一致していたので、
	// 次にバリューフィールドがFetch検索条件と一致しているかを
	// 確認する。
	//

	if (this->m_FetchHint.m_OnlyKey == false)
	{
		// バリューフィールドもFetch対象フィールドである…

		if (this->compareToFetchCondition(valuePage,
										  AttachValuePages_,
										  valueObjectAreaID,
										  fetchCondition,
										  FetchHint::CompareType::OnlyValue,
										  false) // バリューオブジェクト
			!= 0)
		{
			// バリューフィールドがFetch検索条件と一致しなかった…

			//
			// しょうがないので、検索条件ともFetch検索条件とも
			// 一致するキーオブジェクトを
			// “次、次…”（または“前、前…”）と
			// 検索していく…
			// （下はそのためのループ）
			//

			bool	lp = true;
			do
			{
				if (this->m_pOpenParameter->m_bSortReverse)
				{
					// オブジェクトの挿入ソート順とは逆順に
					// オブジェクトをget…

					// “前、前…”
					this->m_ullObjectID =
						this->getPrevObjectIDSearch(AttachNodePages_);
				}
				else
				{
					// オブジェクトの挿入ソート順にオブジェクトをget…

					// “次、次…”
					this->m_ullObjectID =
						this->getNextObjectIDSearch(AttachNodePages_);
				}

				if (this->m_ullObjectID == FileCommon::ObjectID::Undefined)
				{
					// 検索条件ともFetch検索条件とも一致するオブジェクトは
					// 存在しなかった…

					this->m_LeafPageID =
						PhysicalFile::ConstValue::UndefinedPageID;
					this->m_KeyInfoIndex = ModUInt32Max;

					if (this->m_CatchMemoryExhaust)
					{
						ValueFile_->m_PhysicalFile->detachPage(
							valuePage,
							PhysicalFile::Page::UnfixMode::NotDirty,
							false); // 本当にデタッチ（アンフィックス）
							        // してしまい、
							        // 物理ファイルマネージャが
							        // キャッシュしないようにする。
					}

					return false;
				}

				valuePageID =
					Common::ObjectIDData::getFormerValue(m_ullObjectID);
				valueObjectAreaID =
					Common::ObjectIDData::getLatterValue(m_ullObjectID);

				if (valuePage->getID() != valuePageID)
				{
					// 今回のオブジェクトは、
					// 以前アタッチしていたバリューページとは、
					// 別のバリューページに記録されている…

					// バリューページのアタッチし直し

					if (this->m_CatchMemoryExhaust)
					{
						ValueFile_->m_PhysicalFile->detachPage(
							valuePage,
							PhysicalFile::Page::UnfixMode::NotDirty,
							false); // 本当にデタッチ（アンフィックス）
									// してしまい、
							        // 物理ファイルマネージャが
							        // キャッシュしないようにする。
					}

					valuePage = File::attachPage(this->m_pTransaction,
												 ValueFile_->m_PhysicalFile,
												 valuePageID,
												 this->m_FixMode,
												 this->m_CatchMemoryExhaust,
												 AttachValuePages_);
				}

				// バリューオブジェクトから、
				// リーフページの物理ページ識別子と
				// キー情報のインデックスを読み込む。

				const void*	objectAreaTop =
					File::getConstAreaTop(valuePage,
										  valueObjectAreaID);

				ValueFile_->readLeafInfo(objectAreaTop,
										 this->m_LeafPageID,
										 this->m_KeyInfoIndex);

				if (this->compareToFetchCondition(
						valuePage,
						AttachValuePages_,
						valueObjectAreaID,
						fetchCondition,
						FetchHint::CompareType::OnlyValue,
						false) // バリューオブジェクト
					== 0)
				{
					// 検索条件ともFetch検索条件とも
					// 一致するバリューオブジェクトが見つかった…

					break;
				}
			}
			while (lp);
		}
	}

	if (this->m_CatchMemoryExhaust)
	{
		ValueFile_->m_PhysicalFile->detachPage(
			valuePage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	// 検索条件ともFetch検索条件とも完全に一致したオブジェクトが
	// 見つかったので、オブジェクト全体をファイルから読み込む。
	return this->getObject(this->m_ullObjectID,
						   ValueFile_,
						   ResultObject_,
						   true, // プロジェクションフィールドのみ読み込む
						   AttachNodePages_,
						   AttachValuePages_);
}

//
//	Copyright (c) 2001, 2002, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
