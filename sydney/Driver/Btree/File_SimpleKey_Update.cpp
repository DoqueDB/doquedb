// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_SimpleKey_Update.cpp --
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

#include "Common/Assert.h"

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
//	Btree::File::isNecessaryMoveSimpleKey --
//		リーフページのキーの位置が変更される更新処理かをチェックする
//
//	NOTES
//	更新処理により、リーフページに記録されているキーの位置が
//	変わるかどうかをチェックする。
//	“リーフページに記録されているキーの位置が変わる”とは…
//		キーテーブル内のキー情報は、キー値順にソートされている。
//		また、ファイル内に存在するすべてのリーフページは、
//		これもキー値順に参照可能なようにリンクされている。
//		更新処理により、キー情報の位置が、
//			① キーテーブル内で変わる。
//			② 他のリーフページに移動する。
//		①、②いずれかに該当することをこう表現してみました。
//
//	ARGUMENTS
//	const Common::DataArrayData*	AfterObject_
//		更新後のオブジェクトへのポインタ
//	BtreePageVector&				AttachNodePages_
//		ノードページ記述子ベクター
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
File::isNecessaryMoveSimpleKey(
	const Common::DataArrayData*	AfterObject_,
	PageVector&						AttachNodePages_) const
{
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
									true, // リーフページ
									this->m_cFileParameter.m_KeyNum,
									this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			prevKeyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult = this->compareToFetchCondition(nullBitmapTop);

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
									true, // リーフページ
									this->m_cFileParameter.m_KeyNum,
									this->m_cFileParameter.m_KeySize);

		const NullBitmap::Value*	nullBitmapTop =
			nextKeyInfo.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(nullBitmapTop != 0);

		int	compareResult = this->compareToFetchCondition(nullBitmapTop);

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
//	Btree::File::updateSimpleKey --
//		キー情報に記録されているキーフィールドの値を更新する
//
//	NOTES
//	キー情報に記録されているキーフィールドの値を更新する。
//
//	ARGUMENTS
//	const Common::DataArrayData*	Object_
//		更新後のオブジェクトへのポインタ
//	Btree::PageVector&				AttachNodePages_
//		ノードページ記述子ベクター
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::updateSimpleKey(const Common::DataArrayData*	Object_,
					  PageVector&					AttachNodePages_) const
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

	this->writeSimpleKey(leafPage,
						 this->m_KeyInfoIndex,
						 Object_,
						 true); // リーフページ

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
//	Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
