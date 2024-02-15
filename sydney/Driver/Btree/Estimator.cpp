// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Estimator.cpp -- 見積りクラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
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
#include "Btree/Estimator.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "LogicalFile/Estimate.h"

#include "PhysicalFile/File.h"

#include "Btree/File.h"
#include "Btree/FileInformation.h"
#include "Btree/ValueFile.h"
#include "Btree/OpenParameter.h"
#include "Btree/NodePageHeader.h"
#include "Btree/KeyInformation.h"

_SYDNEY_USING

using namespace Btree;

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::Estimator::getSize -- Ｂ＋木ファイルサイズを返す
//
//	NOTES
//	Ｂ＋木ファイルサイズを返す。
//	Ｂ＋木ファイルは、
//		・ツリーファイル
//		・バリューファイル
//	の物理ファイルから構成されているので、
//	それらの物理ファイルのサイズの合計を返す。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	PhysicalFile::File*			ValuePhysicalFile_
//		バリューファイルの物理ファイル記述子
//
//	RETURN
//	ModUInt64
//		Ｂ＋木ファイルサイズ [byte]
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
Estimator::getSize(const Trans::Transaction*	Transaction_,
				   PhysicalFile::File*			TreePhysicalFile_,
				   PhysicalFile::File*			ValuePhysicalFile_)
{
	; _SYDNEY_ASSERT(Transaction_ != 0);
	; _SYDNEY_ASSERT(TreePhysicalFile_ != 0);
	; _SYDNEY_ASSERT(ValuePhysicalFile_ != 0);

	// 物理ファイルの実体である OS ファイルサイズを取得する
	return TreePhysicalFile_->getSize() + ValuePhysicalFile_->getSize();
}

//
//	FUNCTION public
//	Btree::Estimator::getCount --
//		Ｂ＋木ファイルに挿入されているオブジェクト数を返す
//
//	NOTES
//	Ｂ＋木ファイルに挿入されているオブジェクトの総数を返す。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_,
//		トランザクション記述子
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//
//	RETURN
//	ModInt64
//		オブジェクトの総数
//
//	EXCEPTIONS
//	[YET!]
//
// static
ModInt64
Estimator::getCount(const Trans::Transaction*	Transaction_,
					PhysicalFile::File*			TreePhysicalFile_)
{
	; _SYDNEY_ASSERT(Transaction_ != 0);
	; _SYDNEY_ASSERT(TreePhysicalFile_ != 0);

	//
	// 「挿入されているオブジェクト数」は、
	// ツリーファイルの先頭の物理ページであるヘッダページ内の
	// ファイル管理情報に記録されている。
	//

	FileInformation	fileInfo(Transaction_,
							 TreePhysicalFile_,
							 Buffer::Page::FixMode::ReadOnly,
							 false);

	return fileInfo.readObjectNum();
}

//
//	FUNCTION public
//	Btree::Estimator::getOverhead -- 検索時のオーバヘッドコストを返す
//
//	NOTES
//	検索時のオーバヘッドコストの概算を秒数で返す。
//	ここでいう検索とは、Ｂ＋木ファイルオープン後の初回オブジェクト取得時
//	（初回の関数File::get()時）に行なう検索である。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_,
//		トランザクション記述子
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	const Btree::OpenParameter&	OpenParam_
//		Ｂ＋木ファイルオープンパラメータオブジェクトへの参照
//
//	RETURN
//	double
//		検索時のオーバヘッドコスト [秒]
//
//	EXCEPTIONS
//	[YET!]
//
double
Estimator::getOverhead(const Trans::Transaction*	Transaction_,
					   PhysicalFile::File*			TreePhysicalFile_,
					   const Btree::FileParameter&	FileParam_,
					   const Btree::OpenParameter&	OpenParam_)
{
	; _SYDNEY_ASSERT(Transaction_ != 0);
	; _SYDNEY_ASSERT(TreePhysicalFile_ != 0);

	double	cost = 0.0;

	//
	// オーバーヘッドコストは
	//   ・オープンモード
	//   ・オブジェクト取得サブモード
	// により異なる。
	//

	if (OpenParam_.m_iOpenMode == FileCommon::OpenMode::Read)
	{
		// Readモード…

		//
		// ということは、ScanまたはFetchのいずれかである。
		//

		if (OpenParam_.m_iReadSubMode == OpenParameter::ScanRead)
		{
			//
			// Scan モードでオブジェクトを返す場合は、
			// 先頭オブジェクトのオブジェクトIDから
			// 直接、バリューオブジェクトを参照可能なので、
			// オーバーヘッドコストはかからないものとする。
			//

			cost = 0.0;
		}
		else
		{
			; _SYDNEY_ASSERT(
				OpenParam_.m_iReadSubMode == OpenParameter::FetchRead);

			//
			// オーバーヘッドコストは
			// SearchもFetchも同じ。
			//

			cost = Estimator::getOverheadAtSearch(Transaction_,
												  TreePhysicalFile_,
												  FileParam_,
												  OpenParam_);
		}
	}
	else if (OpenParam_.m_iOpenMode == FileCommon::OpenMode::Search)
	{
		// Searchモード…

		cost = Estimator::getOverheadAtSearch(Transaction_,
											  TreePhysicalFile_,
											  FileParam_,
											  OpenParam_);
	}

	//
	// ※ 更新系のオープンモードでオープンするのであれば
	// 　 オーバーヘッドコストはないものとする。
	//

	return cost;
}

//
//	FUNCTION public
//	Btree::Estimator::getProcessCost --
//		オブジェクトへアクセスする際のプロセスコストを返す
//
//	NOTES
//	オブジェクトへアクセスする際のプロセスコストを返す。
//	プロセスコストはオープンモードにより、異なる。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	const Btree::OpenParameter&	OpenParam_
//		Ｂ＋木ファイルオープンパラメータオブジェクトへの参照
//
//	RETURN
//	double
//		オブジェクトへアクセスする際のプロセスコスト [秒]
//
//	EXCEPTIONS
//	[YET!]
//
double
Estimator::getProcessCost(const Trans::Transaction*		Transaction_,
						  PhysicalFile::File*			TreePhysicalFile_,
						  ValueFile*					ValueFile_,
						  const Btree::FileParameter&	FileParam_,
						  const Btree::OpenParameter&	OpenParam_)
{
	; _SYDNEY_ASSERT(Transaction_ != 0);
	; _SYDNEY_ASSERT(TreePhysicalFile_ != 0);
	; _SYDNEY_ASSERT(ValueFile_ != 0);

	//
	// プロセスコストはオープンモードにより異なる
	//

	double	cost = 0.0;

	if (OpenParam_.m_iOpenMode == FileCommon::OpenMode::Read ||
		OpenParam_.m_iOpenMode == FileCommon::OpenMode::Search)
	{
		cost = Estimator::getProcessCostAtRead(Transaction_,
											   TreePhysicalFile_,
											   ValueFile_,
											   FileParam_,
											   OpenParam_);

	}

	//
	// ※ 更新系のオープンモードでオープンするのであれば
	// 　 プロセスコストはないものとする。
	//

	return cost;
}

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::Estimator::getOverheadAtSearch --
//		リーフページのキーオブジェクト検索のオーバヘッドコストを返す
//
//	NOTES
//	ルートノードページからリーフページのキーオブジェクトを
//	検索するまでにかかるオーバヘッドコストを返す。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	const Btree::OpenParameter&	OpenParam_
//		Ｂ＋木ファイルオープンパラメータオブジェクトへの参照
//
//	RETURN
//	double
//		リーフページのキーオブジェクト検索のオーバヘッドコスト [秒]
//
//	EXCEPTIONS
//	[YET!]
//
double
Estimator::getOverheadAtSearch(
	const Trans::Transaction*	Transaction_,
	PhysicalFile::File*			TreePhysicalFile_,
	const Btree::FileParameter&	FileParam_,
	const Btree::OpenParameter&	OpenParam_)
{
	//
	// ファイル管理情報から
	// 「挿入されているオブジェクト数」を読み込む
	//

	FileInformation	fileInfo(Transaction_,
							 TreePhysicalFile_,
							 Buffer::Page::FixMode::ReadOnly,
							 false);

	ModUInt64	objectNum = fileInfo.readObjectNum();

	if (objectNum == 0)
	{
		//
		// オブジェクトが挿入されていないのであれば
		// 検索のためのオーバヘッドはない
		//

		return 0.0;
	}

	// キーフィールドの物理サイズを取得する
	Os::Memory::Size	searchKeyArchiveSize =
		Estimator::getKeyFieldArchiveSize(Transaction_,
										  TreePhysicalFile_,
										  objectNum,
										  FileParam_);

	// ファイルパラメータから
	// 1ノードあたりのキーオブジェクト数を取得する
	ModUInt32	keyPerNode = FileParam_.m_KeyPerNode;

	//
	// 1 ノードあたりの平均読み込み回数を算出する
	//                             ~~~~

	ModUInt32	log2KeyPerNode = 0;
	for (; keyPerNode > 0; keyPerNode >>= 1)
	{
		log2KeyPerNode++;
	}
	int	readNumPerNode = log2KeyPerNode >> 1;
	
	// 1 ノードあたりの平均読み込みサイズを算出する
	//                             ~~~~~~
	Os::Memory::Size	readSizePerNode =
		readNumPerNode * searchKeyArchiveSize;

	// Ｂ＋木ファイル管理情報から現在の木の深さを読み込む
	ModUInt32	treeDepth = fileInfo.readTreeDepth();

	// システムパラメータ CostFileToMemory を取得する
	// ※ CostFileToMemory ... 1 秒間に物理ファイルからメモリ上に
	//                         転送できるバイト数。
	int	costFM = Estimator::getCostFileToMemory();

	// ルートノードページからリーフページのキーオブジェクトを
	// 検索するまでにかかるオーバーヘッドコスト [秒] を算出する
	return (readSizePerNode * treeDepth) / static_cast<double>(costFM);
}

//
//	FUNCTION private
//	Btree::Estimator::getKeyFieldArchiveSize --
//		キーフィールドの記録サイズを返す
//
//	NOTES
//	キーフィールドの記録サイズを返す。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	const modUInt64				ObjectNum_
//		挿入されているオブジェクト数
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	Os::Memory::Size
//		キーフィールドのアーカイブサイズ [byte]
//
//	EXCEPTIONS
//	[YET!]
//
Os::Memory::Size
Estimator::getKeyFieldArchiveSize(
	const Trans::Transaction*	Transaction_,
	PhysicalFile::File*			TreePhysicalFile_,
	const ModUInt64				ObjectNum_,
	const Btree::FileParameter&	FileParam_)
{
	Os::Memory::Size	fieldArchiveSize = 0;

	if (Estimator::existUnlimitedVariableField(FileParam_,
											   true) // キーオブジェクト
		== false)
	{
		// 無制限可変長キーフィールドが含まれていない…

		if (FileParam_.m_KeyPosType == KeyPosType::KeyInfo)
		{
			; _SYDNEY_ASSERT(FileParam_.m_KeySize > 0);

			fieldArchiveSize = FileParam_.m_KeySize;
		}
		else
		{
			fieldArchiveSize = FileParam_.m_DirectKeyObjectSize;

			fieldArchiveSize +=
				Estimator::getOutsideVariableFieldArchiveSize(
					FileParam_,
					true); // キーオブジェクト
		}

		return fieldArchiveSize;
	}

	//
	// 無制限可変長キーフィールドが含まれている
	// この場合、ファイルに含まれる全てのリーフページを参照し
	// キーオブジェクト 1 つあたりにどれだけの領域を
	// 使用しているかを求めて、
	// キーフィールドのアーカイブサイズとする
	//

	//
	// リーフページデータサイズを取得する
	//

	Os::Memory::Size	leafPageDataSize =
		TreePhysicalFile_->getPageDataSize() -
		Btree::NodePageHeader::getArchiveSize(true); // リーフページ

	//
	// 全リーフページ数と、
	// リーフページとして使用されている全物理ページ数を取得する
	//

	ModUInt32	leafPageNum = 0;
	ModUInt32	leafPhysicalPageNum = 0;
	Estimator::setLeafPageNumber(Transaction_,
								 TreePhysicalFile_,
								 leafPageNum,
								 leafPhysicalPageNum);

	// 全リーフページで使用しているデータサイズを求める
	Os::Memory::Size	leafPageTotalSize =
		leafPageDataSize * leafPhysicalPageNum;

	// 1 つのリーフページ内のキーテーブルのアーカイブサイズを求める
	Os::Memory::Size	leafPageKeyTableSize =
		Btree::KeyInformation::getSize(true) * // リーフページ
		FileParam_.m_KeyPerNode;

	// 全リーフページでのキーテーブルのアーカイブサイズを求める
	Os::Memory::Size	keyTableTotalSize =
		leafPageKeyTableSize * leafPageNum;

	; _SYDNEY_ASSERT(leafPageTotalSize > keyTableTotalSize);

	// キーテーブルの分を引く
	leafPageTotalSize -= keyTableTotalSize;

	// リーフページ全ての空き領域合計サイズを取得する
	Os::Memory::Size	leafPageFreeAreaSize =
		Estimator::getLeafPageFreeAreaSize(Transaction_,
										   TreePhysicalFile_);

	; _SYDNEY_ASSERT(leafPageTotalSize > leafPageFreeAreaSize);

	// 空き領域の分も引く
	leafPageTotalSize -= leafPageFreeAreaSize;

	// キーフィールドのアーカイブサイズを求める
	return static_cast<Os::Memory::Size>(leafPageTotalSize / ObjectNum_);
}

//
//	FUNCTION private
//	Btree::Estimator::existUnlimitedVariableField --
//		オブジェクトに無制限可変長フィールドが含まれているかを知らせる
//
//	NOTES
//	オブジェクトに無制限可変長フィールドが含まれているかを知らせる。
//
//	ARGUMENTS
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	bool						IsKeyObject_
//		キーオブジェクトかどうか
//			true  : キーフィールド
//			false : バリューフィールド
//
//	RETURN
//	bool
//		オブジェクトに無制限可変長フィールドが含まれるかどうか
//			true  : オブジェクトに無制限可変長フィールドが含まれる
//			false : オブジェクトに無制限可変長フィールドが含まれない
//
//	EXCEPTIONS
//	なし
//
bool
Estimator::existUnlimitedVariableField(
	const Btree::FileParameter&	FileParam_,
	bool						IsKeyObject_)
{
	int	startIndex, stopIndex;

	if (IsKeyObject_)
	{
		if (FileParam_.m_ExistVariableFieldInKey == false)
		{
			return false;
		}

		startIndex = 1;
		stopIndex = FileParam_.m_TopValueFieldIndex - 1;
	}
	else
	{
		if (FileParam_.m_ExistVariableFieldInValue == false)
		{
			return false;
		}

		startIndex = FileParam_.m_TopValueFieldIndex;
		stopIndex = FileParam_.m_FieldNum - 1;
	}

	for (int i = startIndex; i <= stopIndex; i++)
	{
		if (*(FileParam_.m_IsArrayFieldArray + i) == false &&
			*(FileParam_.m_IsFixedFieldArray + i) == false &&
			*(FileParam_.m_FieldMaxLengthArray + i) ==
			File::UnlimitedFieldLen)
		{
			return true;
		}
	}

	return false;
}

//
//	FUNCTION private
//	Btree::Estimator::setLeafPageNumber -- リーフページ数を設定する
//
//	NOTES
//	引数LeafPageNumber_にリーフページ数を、
//	引数PhysicalPageNumber_に
//	リーフページとして使用されている全物理ページ数を
//	設定する。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	ModUInt32&					LeafPageNum_
//		リーフページ数への参照
//		本関数が設定する。
//	ModUInt32&					PhysicalPageNum_
//		リーフページとして使用されている全物理ページ数への参照
//		本関数が設定する。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
Estimator::setLeafPageNumber(
	const Trans::Transaction*	Transaction_,
	PhysicalFile::File*			TreePhysicalFile_,
	ModUInt32&					LeafPageNum_,
    ModUInt32&					PhysicalPageNum_)
{
	LeafPageNum_ = PhysicalPageNum_ = 0;

	PhysicalFile::PageID	topLeafPageID;

	{
		FileInformation	fileInfo(Transaction_,
								 TreePhysicalFile_,
								 Buffer::Page::FixMode::ReadOnly,
								 false);

		topLeafPageID = fileInfo.readTopLeafPageID();
	}

	while (topLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		PhysicalFile::PageID	leafPageID = topLeafPageID;

		while (leafPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			NodePageHeader	leafPageHeader(Transaction_,
										   TreePhysicalFile_,
										   leafPageID,
										   Buffer::Page::FixMode::ReadOnly,
										   true,   // リーフページ
										   false); // 

			if (leafPageID == topLeafPageID)
			{
				topLeafPageID = leafPageHeader.readNextLeafPageID();
			}

			leafPageID = leafPageHeader.readNextPhysicalPageID();

			PhysicalPageNum_++;
		}

		LeafPageNum_++;
	}
}

//
//	FUNCTION private
//	Btree::Estimator::getLeafPageFreeAreaSize --
//		リーフページ全ての空き領域合計サイズを返す
//
//	NOTES
//	リーフページ全ての空き領域合計サイズを返す。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//
//	RETURN
//	Os::Memory::Size
//		リーフページ全ての空き領域合計サイズ [byte]
//
//	EXCEPTIONS
//	[YET!]
//
Os::Memory::Size
Estimator::getLeafPageFreeAreaSize(
	const Trans::Transaction*	Transaction_,
	PhysicalFile::File*			TreePhysicalFile_)
{
	Os::Memory::Size	leafPageFreeAreaSize = 0;

	//
	// ファイル管理情報から
	// 「先頭リーフページの物理ページ識別子」を読み込む
	//

	PhysicalFile::PageID	topLeafPageID;

	{
		FileInformation	fileInfo(Transaction_,
								 TreePhysicalFile_,
								 Buffer::Page::FixMode::ReadOnly,
								 false);

		topLeafPageID = fileInfo.readTopLeafPageID();
	}

	//
	// リーフページ同士の双方向リンクをたどって、
	// 全てのリーフページの空き領域合計サイズを加算していく
	//

	while (topLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		//
		// 物理ページ同士の双方向リンクをたどって、
		// リーフページを構成する全ての物理ページの
		// 空き領域合計サイズを加算していく
		//

		PhysicalFile::PageID	leafPageID = topLeafPageID;

		while (leafPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			// リーフページを構成する物理ページに記録されている
			// リーフページヘッダをアタッチする
			NodePageHeader	leafPageHeader(Transaction_,
										   TreePhysicalFile_,
										   leafPageID,
										   Buffer::Page::FixMode::ReadOnly,
										   true,   // リーフページ
										   false); // 

			if (leafPageID == topLeafPageID)
			{
				// リーフを構成する先頭の物理ページの場合、
				// 次のリーフページのＩＤを求めておく
				topLeafPageID = leafPageHeader.readNextLeafPageID();
			}

			// リーフページを構成する物理ページをアタッチする
			PhysicalFile::Page*	leafPage =
				TreePhysicalFile_->attachPage(
					*Transaction_,
					leafPageID,
					Buffer::Page::FixMode::ReadOnly);

			// 物理ページ内の空き領域合計サイズを取得する
			// allocateArea した後の空き領域を求める
			leafPageFreeAreaSize +=
				leafPage->getUnuseAreaSize(*Transaction_, 1);

			// リーフページを構成する物理ページをデタッチする
			TreePhysicalFile_->detachPage(
				leafPage,
				PhysicalFile::Page::UnfixMode::NotDirty,
				false);

			// リーフページを構成する物理ページの
			// リーフページヘッダから
			// 「次の物理ページの物理ページ識別子」を読み込む
			leafPageID = leafPageHeader.readNextPhysicalPageID();
		}

	} // end while topLeafPageID != UndefinedPageID

	return leafPageFreeAreaSize;
}

//
//	FUNCTION private
//	Btree::Estimator::getOutsideVariableFieldArchiveSize --
//		可変長フィールドの記録サイズを返す
//
//	NOTES
//	可変長フィールドの記録サイズを返す。
//	Ｂ＋木ファイルパラメータを参照し、
//	指定されている可変長フィールドの最大長により、
//	記録サイズを算出する。
//	したがって返される記録サイズは、
//	実際に挿入されている可変長フィールドデータの記録サイズとは
//	異なる。（等しい場合もあるが）
//
//	ARGUMENTS
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	bool						IsKeyObject_
//		キーオブジェクトかどうか
//			true  : キーオブジェクト
//			value : バリューオブジェクト
//
//	RETURN
//	Os::Memory::Size
//		可変長フィールドの記録サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
Os::Memory::Size
Estimator::getOutsideVariableFieldArchiveSize(
	const Btree::FileParameter&	FileParam_,
	bool						IsKeyObject_)
{
	int	startIndex, stopIndex;

	if (IsKeyObject_)
	{
		if (FileParam_.m_ExistVariableFieldInKey == false ||
			FileParam_.m_ExistOutsideFieldInKey == false)
		{
			return 0;
		}

		startIndex = 1;

		stopIndex = FileParam_.m_TopValueFieldIndex - 1;
	}
	else
	{
		if (FileParam_.m_ExistVariableFieldInValue == false ||
			FileParam_.m_ExistOutsideFieldInValue == false)
		{
			return 0;
		}

		startIndex = FileParam_.m_TopValueFieldIndex;

		stopIndex = FileParam_.m_FieldNum - 1;
	}

	Os::Memory::Size	varSize = 0;

	for (int i = startIndex; i <= stopIndex; i++)
	{
		if (*(FileParam_.m_FieldOutsideArray + i) &&
			*(FileParam_.m_IsArrayFieldArray + i) == false)
		{
			// 外置き可変長フィールド…

			if (*(FileParam_.m_FieldMaxLengthArray + i) !=
				File::UnlimitedFieldLen)
			{
				// 無制限長ではない…

				//
				// 外置き可変長フィールドは、
				// 下図のような物理構成となっている。
				//
				//     ┌───────────┐
				//     │　オブジェクトタイプ　│ 1[byte]
				//     ├───────────┤
				//     │　　フィールド値　　　│ *
				//     └───────────┘
				//

				// for object type
				varSize += File::ObjectTypeArchiveSize;

				//
				// 指定されている可変長フィールドの最大長により、
				// 記録サイズを算出する。
				// ※ “見積もり”なので、実際のフィールド長ではない。
				//

				// for field value
				varSize += *(FileParam_.m_FieldMaxLengthArray + i);
			}
		}
	}

	return varSize;
}

//
//	FUNCTION private
//	Btree::Estimator::getProcessCostAtRead --
//		オブジェクト取得の際のプロセスコストを返す
//
//	NOTES
//	オブジェクト取得の際のプロセスコストを返す。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			TreePhysicalFile_
//		ツリーファイルの物理ファイル記述子
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	const Btree::OpenParameter&	OpenParam_
//		Ｂ＋木ファイルオープンパラメータオブジェクトへの参照
//
//	RETURN
//	double
//		オブジェクト取得の際のプロセスコスト [秒]
//
//	EXCEPTIONS
//	[YET!]
//
double
Estimator::getProcessCostAtRead(
	const Trans::Transaction*	Transaction_,
	PhysicalFile::File*			TreePhysicalFile_,
	ValueFile*					ValueFile_,
	const Btree::FileParameter&	FileParam_,
	const Btree::OpenParameter&	OpenParam_)
{
	FileInformation	fileInfo(Transaction_,
							 TreePhysicalFile_,
							 Buffer::Page::FixMode::ReadOnly,
							 false);

	ModUInt64	objectNum = fileInfo.readObjectNum();

	if (objectNum == 0)
	{
		return 0.0;
	}

	double	cost = 0.0;

	if (OpenParam_.m_iReadSubMode == OpenParameter::FetchRead)
	{
		//
		// Fetchのプロセスコストはオーバーヘッドコストと同じ。
		// しかもオーバーヘッドコストは、
		// SearchもFetchも同じ。
		//

		cost = Estimator::getOverheadAtSearch(Transaction_,
											  TreePhysicalFile_,
											  FileParam_,
											  OpenParam_);
	}
	else
	{
		//
		// Search モードのプロセスコストも
		// 初回検索 (overhead) が終れば、
		// Scan モードのそれと同じ。
		//

		//
		// 前後のオブジェクトを検索（シーク）する時間は
		// ほとんど無視しても構わない程度のはず。
		//

		cost = Estimator::getValueObjectReadTime(Transaction_,
												 ValueFile_,
												 objectNum,
												 FileParam_);
	}

	return cost;
}

//
//	FUNCTION private
//	Btree::Estimator::getValueObjectReadTime --
//		バリューオブジェクト読み込み時間を返す
//
//	NOTES
//	バリューオブジェクト読み込み時間を返す。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			ValueFile_
//		バリューファイル記述子
//	const ModUInt64				ObjectNum_
//		挿入されているオブジェクト数
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	double
//		バリューオブジェクト読み込み時間 [秒]
//
//	EXCEPTIONS
//	[YET!]
//
double
Estimator::getValueObjectReadTime(
	const Trans::Transaction*	Transaction_,
	ValueFile*					ValueFile_,
	const ModUInt64				ObjectNum_,
	const Btree::FileParameter&	FileParam_)
{
	Os::Memory::Size	valueFieldArchiveSize =
		Estimator::getValueFieldArchiveSize(Transaction_,
											ValueFile_,
											ObjectNum_,
											FileParam_);

	int	costFM = Estimator::getCostFileToMemory();

	return static_cast<double>(valueFieldArchiveSize) / costFM;
}

//
//	FUNCTION private
//	Btree::Estimator::getValueFieldArchiveSize --
//		バリューフィールドの記録サイズを返す
//
//	NOTES
//	バリューフィールドの記録サイズを返す。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			ValueFile_
//		バリューファイル記述子
//	const ModUInt64				ObjectNum_
//		挿入されているオブジェクト数
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//
//	RETURN
//	Os::Memory::Size
//		バリューフィールドの記録サイズ [byte]
//
//	EXCEPTIONS
//	[YET!]
//
Os::Memory::Size
Estimator::getValueFieldArchiveSize(
	const Trans::Transaction*	Transaction_,
	ValueFile*					ValueFile_,
	const ModUInt64				ObjectNum_,
	const Btree::FileParameter&	FileParam_)
{
	Os::Memory::Size	fieldArchiveSize = 0;

	if (Estimator::existUnlimitedVariableField(FileParam_, false) == false)
	{
		// 無制限可変長フィールドが含まれていない…

		fieldArchiveSize = FileParam_.m_DirectValueObjectSize;

		fieldArchiveSize +=
			Estimator::getOutsideVariableFieldArchiveSize(
				FileParam_,
				false); // バリューオブジェクト

		return fieldArchiveSize;
	}

	//
	// 無制限可変長フィールドが含まれている
	//

	Os::Memory::Size	valuePageDataSize =
		ValueFile_->m_PhysicalFile->getPageDataSize();

	// バリューページ数を取得する
	PhysicalFile::PageNum	valuePageNumber =
		Estimator::getValuePageNum(Transaction_,
								   ValueFile_->m_PhysicalFile);

	Os::Memory::Size	valuePageTotalSize =
		valuePageNumber * valuePageDataSize;

	Os::Memory::Size	valuePageFreeAreaSize =
		Estimator::getValuePageFreeAreaSize(Transaction_,
											ValueFile_->m_PhysicalFile);

	; _SYDNEY_ASSERT(valuePageTotalSize > valuePageFreeAreaSize);

	valuePageTotalSize -= valuePageFreeAreaSize;

	return static_cast<Os::Memory::Size>(valuePageTotalSize / ObjectNum_);
}

//
//	FUNCTION private
//	Btree::Estimator::getValuePageFreeAreaSize --
//		全てのバリューページの空き領域合計サイズを返す
//
//	NOTES
//	全てのバリューページの空き領域合計サイズを返す。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			ValuePhysicalFile_
//		バリューファイルの物理ファイル記述子
//
//	RETURN
//	Os::Memory::Size
//		全てのバリューページの空き領域合計サイズ [byte]
//
//	EXCEPTIONS
//	[YET!]
//
Os::Memory::Size
Estimator::getValuePageFreeAreaSize(
	const Trans::Transaction*	Transaction_,
	PhysicalFile::File*			ValuePhysicalFile_)
{
	Os::Memory::Size	valuePageFreeAreaSize = 0;

	PhysicalFile::PageID	valuePageID = 0;

	while (valuePageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		PhysicalFile::Page*	valuePage =
			ValuePhysicalFile_->attachPage(*Transaction_,
										   valuePageID,
										   Buffer::Page::FixMode::ReadOnly);

		// allocateArea した後の空き領域を求める
		valuePageFreeAreaSize += valuePage->getUnuseAreaSize(*Transaction_,
															 1);

		valuePageID = ValuePhysicalFile_->getNextPageID(*Transaction_,
														valuePageID);

		ValuePhysicalFile_->detachPage(
			valuePage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false);
	}

	return valuePageFreeAreaSize;
}

//
//	FUNCTION private
//	Btree::Estimator::getValuePageNum -- バリューページ数を返す
//
//	NOTES
//	バリューページ数を返す。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			ValuePhysicalFile_
//		バリューファイルの物理ファイル記述子
//
//	RETURN
//	PhysicalFile::PageNum
//		バリューページ数
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageNum
Estimator::getValuePageNum(
	const Trans::Transaction*	Transaction_,
	PhysicalFile::File*			ValuePhysicalFile_)
{
	PhysicalFile::PageNum	valuePageNum = 0;

	PhysicalFile::PageID	valuePageID = 0;

	while (valuePageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		valuePageID = ValuePhysicalFile_->getNextPageID(*Transaction_,
														valuePageID);

		valuePageNum++;
	}

	return valuePageNum;
}

//
//	FUNCTION private
//	Btree::Estimator::getCostFileToMemory --
//		1 秒間にファイルからメモリに転送できるバイト数を返す
//
//	NOTES
//	1 秒間にファイルからメモリに転送できるバイト数を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		1 秒間にファイルからメモリに転送できるバイト数 [byte/sec]
//
//	EXCEPTIONS
//	なし
//
int
Estimator::getCostFileToMemory()
{
	return
		LogicalFile::Estimate::getTransferSpeed(
			LogicalFile::Estimate::File);
}

#ifdef OBSOLETE
//
//	FUNCTION private
//	Btree::Estimator::getCostMemoryToFile --
//		1 秒間にメモリからファイルに転送できるバイト数を返す
//
//	NOTES
//	1 秒間にメモリからファイルに転送できるバイト数を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		1 秒間にメモリからファイルに転送できるバイト数 [byte/sec]
//
//	EXCEPTIONS
//	なし
//
int
Estimator::getCostMemoryToFile()
{
	//
	// ファイルからメモリと同じ値を使用する
	//
	return
		LogicalFile::Estimate::getTransferSpeed(
			LogicalFile::Estimate::File);
}
#endif //OBSOLETE

#ifdef OBSOLETE
//
//	FUNCTION private
//	Btree::Estimator::getCostMemoryToMemory --
//		1 秒間にメモリからメモリに転送できるバイト数を返す
//
//	NOTES
//	1 秒間にメモリからメモリに転送できるバイト数を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		1 秒間にメモリからメモリに転送できるバイト数 [byte/sec]
//
//	EXCEPTIONS
//	なし
//
int
Estimator::getCostMemoryToMemory()
{
	return
		LogicalFile::Estimate::getTransferSpeed(
			LogicalFile::Estimate::Memory);
}
#endif //OBSOLETE

//
//	Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
