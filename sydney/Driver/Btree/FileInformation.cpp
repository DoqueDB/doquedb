// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileInformation.cpp -- Ｂ＋木ファイル管理情報クラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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

#include "Btree/FileInformation.h"
#include "Btree/TreeFile.h"

#include "Common/Assert.h"
#include "PhysicalFile/Page.h"

#include "ModTime.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PUBLIC CONST
//
//////////////////////////////////////////////////

//
//	CONST public
//	Btree::FileInformation::AreaID --
//		ファイル管理情報エリアの物理エリア識別子
//
//	NOTES
//	ファイル管理情報エリアの物理エリア識別子。
//
// static
const PhysicalFile::AreaID
FileInformation::AreaID = 0;

//
//	CONST public
//	Btree::FileInformation::ArchiveSize --
//		ファイル管理情報の記録サイズ
//
//	NOTES
//	ファイル管理情報の記録サイズ。[byte]
//
// static
const Os::Memory::Size
FileInformation::ArchiveSize =
	sizeof(unsigned int) +             // ファイルバージョン
	(sizeof(int) << 1) +               // 最終更新時刻
	sizeof(ModUInt32) +                // 現在の木の深さ
	sizeof(PhysicalFile::PageID) * 3 + // 物理ページ識別子3つ分
	sizeof(ModUInt64);                 // 挿入されているオブジェクト数

//////////////////////////////////////////////////
//
//	PRIVATE CONST
//
//////////////////////////////////////////////////

//
//	CONST private
//	Btree::FileInformation::FileVersionOffset --
//		「ファイルバージョン」のファイル管理情報内での位置
//
//	NOTES
//	「ファイルバージョン」のファイル管理情報内での位置。[byte]
//
// static
const Os::Memory::Offset
FileInformation::FileVersionOffset = 0;

//
//	CONST private
//	Btree::FileInformation::ModificationTimeOffset --
//		「最終更新時刻」のファイル管理情報エリア内での位置
//
//	NOTES
//	「最終更新時刻」のファイル管理情報エリア内での位置。[byte]
//
// static
const Os::Memory::Offset
FileInformation::ModificationTimeOffset =
	FileInformation::FileVersionOffset + sizeof(unsigned int);
	// FileInformation::Vers型の変数を ~~~~~~~~~~~~~~~~~~~~~~
	// unsigned intにして記録するので。

//
//	CONST private
//	Btree::FileInformation::TreeDepthOffset --
//		「現在の木の深さ」のファイル管理情報エリア内での位置
//
//	NOTES
//	「現在の木の深さ」のファイル管理情報エリア内での位置。[byte]
//
// static
const Os::Memory::Offset
FileInformation::TreeDepthOffset =
	FileInformation::ModificationTimeOffset + (sizeof(int) << 1);
	//                                                     ~~~~ 「掛ける２」

//
//	CONST private
//	Btree::FileInformation::RootNodePageIDOffset --
//		「ルートノードページの物理ページ識別子」の
//		ファイル管理情報エリア内での位置
//
//	NOTES
//	「ルートノードページの物理ページ識別子」の
//	ファイル管理情報エリア内での位置。 [byte]
//
// static
const Os::Memory::Offset
FileInformation::RootNodePageIDOffset =
	FileInformation::TreeDepthOffset + sizeof(ModUInt32);

//
//	CONST private
//	Btree::FileInformation::TopLeafPageIDOffset --
//		「先頭リーフページの物理ページ識別子」の
//		ファイル管理情報エリア内での位置
//
//	NOTES
//	「先頭リーフページの物理ページ識別子」の
//	ファイル管理情報エリア内での位置。[byte]
//
// static
const Os::Memory::Offset
FileInformation::TopLeafPageIDOffset =
	FileInformation::RootNodePageIDOffset + sizeof(PhysicalFile::PageID);

//
//	CONST private
//	Btree::FileInformation::LastLeafPageIDOffset --
//		「最終リーフページの物理ページ識別子」の
//		ファイル管理情報エリア内での位置
//
//	NOTES
//	「最終リーフページの物理ページ識別子」の
//	ファイル管理情報エリア内での位置。[byte]
//
// static
const Os::Memory::Offset
FileInformation::LastLeafPageIDOffset =
	FileInformation::TopLeafPageIDOffset + sizeof(PhysicalFile::PageID);

//
//	CONST private
//	Btree::FileInformation::ObjectNumOffset --
//		「挿入されているオブジェクト数」の
//		ファイル管理情報エリア内での位置
//
//	NOTES
//	「挿入されているオブジェクト数」の
//	ファイル管理情報エリア内での位置。[byte]
//		
// static
const Os::Memory::Offset
FileInformation::ObjectNumOffset =
	FileInformation::LastLeafPageIDOffset + sizeof(PhysicalFile::PageID);

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::FileInformation::FileInformation -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	親クラスであるBtree::AreaObjectのコンストラクタ内で
//	物理ページをアタッチする。
//
//	ARGUMENTS
//	const Trans::Transaction*			pTransaction_
//		トランザクション記述子
//	PhysicalFile::File*					pFile_
//		物理ファイル記述子
//	const Buffer::Page::FixMode::Value	eFixMode_
//		フィックスモード
//	const bool							SavePage_
//		物理ファイルマネージャでアタッチした物理ページを
//		キャッシュしておくかどうか
//			true  : キャッシュしておく
//			false : キャッシュせず、デタッチ時にアンフィックスする
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
FileInformation::FileInformation(
	const Trans::Transaction*			pTransaction_,
	PhysicalFile::File*					pFile_,
	const Buffer::Page::FixMode::Value	eFixMode_,
	const bool							SavePage_)
	: Btree::AreaObject(pTransaction_,
						pFile_,
						TreeFile::HeaderPageID,
						FileInformation::AreaID,
						eFixMode_,
						Buffer::ReplacementPriority::Middle,
						SavePage_)
{
}

//
//	FUNCTION public
//	Btree::FileInformation::FileInformation -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	親クラスであるBtree::AreaObjectのコンストラクタ内で
//	物理ページをアタッチしない。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::Page*			HeaderPage_
//		ファイル管理情報を記録するヘッダページの物理ページ記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
FileInformation::FileInformation(const Trans::Transaction*	Transaction_,
								 PhysicalFile::Page*		HeaderPage_)
	: Btree::AreaObject(Transaction_,
						HeaderPage_,
						FileInformation::AreaID)
{
}

//
//	FUNCTION public
//	Btree::FileInformation::~FileInformation -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
FileInformation::~FileInformation()
{
}

//
//	FUNCTION public
//	Btree::FileInformation::initialize --
//		ファイル管理情報を初期化する
//
//	NOTES
//	ファイル管理情報を初期化する。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::Page*			HeaderPage_
//		ファイル管理情報を記録するヘッダページの物理ページ記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
FileInformation::initialize(const Trans::Transaction*	Transaction_,
							PhysicalFile::Page*			HeaderPage_)
{
	; _SYDNEY_ASSERT(Transaction_ != 0);
	; _SYDNEY_ASSERT(HeaderPage_ != 0);

	// ファイル管理情報エリアを生成する
	PhysicalFile::AreaID	fileInfoAreaID =
		HeaderPage_->allocateArea(*Transaction_,
								  FileInformation::ArchiveSize);

	; _SYDNEY_ASSERT(fileInfoAreaID == FileInformation::AreaID);

	//
	// 初期状態のファイル管理情報を書き込む
	//

	FileInformation	fileInfo(Transaction_, HeaderPage_);

	fileInfo.writeFileVersion(FileVersion::CurrentVersion);

	fileInfo.writeModificationTime();

	fileInfo.writeTreeDepth(1);

	fileInfo.writeRootNodePageID(PhysicalFile::ConstValue::UndefinedPageID);

	fileInfo.writeTopLeafPageID(PhysicalFile::ConstValue::UndefinedPageID);

	fileInfo.writeLastLeafPageID(PhysicalFile::ConstValue::UndefinedPageID);

	fileInfo.writeObjectNum(0);
}

//
//	書き込みメソッド
//

//
//	FUNCTION public
//	Btree::FileInformation::writeFileVersion --
//		「ファイルバージョン」を書き込む
//
//	NOTES
//	「ファイルバージョン」を書き込む。
//
//	ARGUMENTS
//	const Btree::FileVersion::Value	FileVersion_
//		ファイルバージョン
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::writeFileVersion(const FileVersion::Value	FileVersion_)
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	unsigned int*	versionWritePos =
		syd_reinterpret_cast<unsigned int*>(
			this->m_AreaTop + FileInformation::FileVersionOffset);

	*versionWritePos = static_cast<unsigned int>(FileVersion_);
}

//	FUNCTION public
//	Btree::FileInformation::writeModificationTime --
//		「最終更新時刻」に現在の時刻を書き込む
//
//	NOTES
//	「最終更新時刻」に現在の時刻を書き込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
FileInformation::writeModificationTime()
{
	; _SYDNEY_ASSERT(m_AreaTop != 0);

	const ModTime t = ModTime::getCurrentTime();
	Common::DateTimeData(t.getYear(), t.getMonth(), t.getDay(),
						 t.getHour(), t.getMinute(), t.getSecond(),
						 t.getMilliSecond()).dumpValue(
							 m_AreaTop + FileInformation::ModificationTimeOffset);
}

//
//	FUNCTION public
//	Btree::FileInformation::writeTreeDepth --
//		「現在の木の深さ」を書き込む
//
//	NOTES
//	「現在の木の深さ」を書き込む。
//
//	ARGUMENTS
//	const ModUInt32	TreeDepth_
//		現在の木の深さ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::writeTreeDepth(const ModUInt32	TreeDepth_)
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	ModUInt32*	treeDepthWritePos =
		syd_reinterpret_cast<ModUInt32*>(
			this->m_AreaTop + FileInformation::TreeDepthOffset);

	*treeDepthWritePos = TreeDepth_;
}

//
//	FUNCTION public
//	Btree::FileInformation::incTreeDepth --
//		「現在の木の深さ」をインクリメントする
//
//	NOTES
//	「現在の木の深さ」をインクリメントする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::incTreeDepth()
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	ModUInt32*	treeDepthWritePos =
		syd_reinterpret_cast<ModUInt32*>(
			this->m_AreaTop + FileInformation::TreeDepthOffset);

	(*treeDepthWritePos)++;
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	Btree::FileInformation::decTreeDepth --
//		「現在の木の深さ」をデクリメントする
//
//	NOTES
//	「現在の木の深さ」をデクリメントする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::decTreeDepth()
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	ModUInt32*	treeDepthWritePos =
		syd_reinterpret_cast<ModUInt32*>(
			this->m_AreaTop + FileInformation::TreeDepthOffset);

	; _SYDNEY_ASSERT(*treeDepthWritePos > 1);

	(*treeDepthWritePos)--;
}
#endif //OBSOLETE

//
//	FUNCTION public
//	Btree::FileInformation::writeRootNodePageID --
//		「ルートノードページの物理ページ識別子」を書き込む
//
//	NOTES
//	「ルートノードページの物理ページ識別子」を書き込む。
//
//	ARGUMENTS
//	const ModPhysicalPageID	RootNodePageID_
//		ルートノードページの物理ページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::writeRootNodePageID(
	const PhysicalFile::PageID	RootNodePageID_)
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	PhysicalFile::PageID*	pageIDWritePos =
		syd_reinterpret_cast<PhysicalFile::PageID*>(
			this->m_AreaTop + FileInformation::RootNodePageIDOffset);

	*pageIDWritePos = RootNodePageID_;
}

//
//	FUNCTION public
//	Btree::FileInformation::writeTopLeafPageID --
//		「先頭リーフページの物理ページ識別子」を書き込む
//
//	NOTES
//	「先頭リーフページの物理ページ識別子」を書き込む。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	TopLeafPageID_
//		先頭リーフページの物理ページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::writeTopLeafPageID(
	const PhysicalFile::PageID	TopLeafPageID_)
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	PhysicalFile::PageID*	pageIDWritePos =
		syd_reinterpret_cast<PhysicalFile::PageID*>(
			this->m_AreaTop + FileInformation::TopLeafPageIDOffset);

	*pageIDWritePos = TopLeafPageID_;
}

//
//	FUNCTION public
//	Btree::FileInformation::writeLastLeafPageID --
//		「最終リーフページの物理ページ識別子」を書き込む
//
//	NOTES
//	「最終リーフページの物理ページ識別子」を書き込む。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	LastLeafPageID_
//		最終リーフページの物理ページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::writeLastLeafPageID(
	const PhysicalFile::PageID	LastLeafPageID_)
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	PhysicalFile::PageID*	pageIDWritePos =
		syd_reinterpret_cast<PhysicalFile::PageID*>(
			this->m_AreaTop + FileInformation::LastLeafPageIDOffset);

	*pageIDWritePos = LastLeafPageID_;
}

//
//	FUNCTION public
//	Btree::FileInformation::writeObjectNum --
//		「挿入されているオブジェクト数」を書き込む
//
//	NOTES
//	「挿入されているオブジェクト数」を書き込む。
//
//	ARGUMENTS
//	const ModUInt64	ObjectNum_
//		挿入されているオブジェクト数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::writeObjectNum(const ModUInt64	ObjectNum_)
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	ModUInt64*	objectNumWritePos =
		syd_reinterpret_cast<ModUInt64*>(
			this->m_AreaTop + FileInformation::ObjectNumOffset);

	*objectNumWritePos = ObjectNum_;
}

//
//	FUNCTION public
//	Btree::FileInformation::incObjectNum --
//		「挿入されているオブジェクト数」をインクリメントする
//
//	NOTES
//	「挿入されているオブジェクト数」をインクリメントする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::incObjectNum()
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	ModUInt64*	objectNumWritePos =
		syd_reinterpret_cast<ModUInt64*>(
			this->m_AreaTop + FileInformation::ObjectNumOffset);

	(*objectNumWritePos)++;
}

//
//	FUNCTION public
//	Btree::FileInformation::decObjectNum --
//		「挿入されているオブジェクト数」をデクリメントする
//
//	NOTES
//	「挿入されているオブジェクト数」をデクリメントする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::decObjectNum()
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	ModUInt64*	objectNumWritePos =
		syd_reinterpret_cast<ModUInt64*>(
			this->m_AreaTop + FileInformation::ObjectNumOffset);

	; _SYDNEY_ASSERT(*objectNumWritePos > 0);

	(*objectNumWritePos)--;
}

//
//	読み込みメソッド
//

//
//	FUNCTION public
//	Btree::FileInformation::readFileVersion --
//		「ファイルバージョン」を読み込む
//
//	NOTES
//	「ファイルバージョン」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Btree::FileVersion::Value
//		ファイルバージョン
//
//	EXCEPTIONS
//	なし
//
FileVersion::Value
FileInformation::readFileVersion() const
{
	const unsigned int*	versionReadPos =
		syd_reinterpret_cast<const unsigned int*>(
			this->getConstAreaTop() + FileInformation::FileVersionOffset);

	return static_cast<FileVersion::Value>(*versionReadPos);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Btree::FileInformation::readModificationTime --
//		「最終更新時刻」を読み込む
//
//	NOTES
//	「最終更新時刻」を読み込む。
//
//	ARGUMENTS
//	int&	Year_
//		年
//	int&	Month_
//		月
//	int&	Day_
//		日
//	int&	Hour_
//		時
//	int&	Minute_
//		分
//	int&	Second_
//		秒
//	int&	MilliSecond_
//		ミリ秒
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	なし

void
FileInformation::readModificationTime(int&	Year_,
									  int&	Month_,
									  int&	Day_,
									  int&	Hour_,
									  int&	Minute_,
									  int&	Second_,
									  int&	MilliSecond_) const
{
	Common::DateTimeData data;
	data.setDumpedValue(
		getConstAreaTop() + FileInformation::ModificationTimeOffset);

	Year_ = data.getYear();
	Month_ = data.getMonth();
	Day_ = data.getDate();
	Hour_ = data.getHour();
	Minute_ = data.getMinute();
	Second_ = data.getSecond();
	MilliSecond_ = data.getMillisecond();
}
#endif //OBSOLETE

//
//	FUNCTION public
//	Btree::FileInformation::readTreeDepth --
//		「現在の木の深さ」を読み込む
//
//	NOTES
//	「現在の木の深さ」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		現在の木の深さ
//
//	EXCEPTIONS
//	なし
//
ModUInt32
FileInformation::readTreeDepth() const
{
	const ModUInt32*	treeDepthReadPos =
		syd_reinterpret_cast<const ModUInt32*>(
			this->getConstAreaTop() + FileInformation::TreeDepthOffset);

	return *treeDepthReadPos;
}

//
//	FUNCTION public
//	Btree::FileInformation::readRootNodePageID --
//		「ルートノードページの物理ページ識別子」を読み込む
//
//	NOTES
//	「ルートノードページの物理ページ識別子」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		ルートノードページの物理ページ識別子
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageID
FileInformation::readRootNodePageID() const
{
	const PhysicalFile::PageID*	pageIDReadPos =
		syd_reinterpret_cast<const PhysicalFile::PageID*>(
			this->getConstAreaTop() +
			FileInformation::RootNodePageIDOffset);

	return *pageIDReadPos;
}

//
//	FUNCTION public
//	Btree::FileInformation::readTopLeafPageID --
//		「先頭リーフページの物理ページ識別子」を読み込む
//
//	NOTES
//	「先頭リーフページの物理ページ識別子」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		先頭リーフページの物理ページ識別子
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageID
FileInformation::readTopLeafPageID() const
{
	const PhysicalFile::PageID*	pageIDReadPos =
		syd_reinterpret_cast<const PhysicalFile::PageID*>(
			this->getConstAreaTop() +
			FileInformation::TopLeafPageIDOffset);

	return *pageIDReadPos;
}

//
//	FUNCTION public
//	Btree::FileInformation::readLastLeafPageID --
//		「最終リーフページの物理ページ識別子」を読み込む
//
//	NOTES
//	「最終リーフページの物理ページ識別子」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		最終リーフページの物理ページ識別子
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageID
FileInformation::readLastLeafPageID() const
{
	const PhysicalFile::PageID*	pageIDReadPos =
		syd_reinterpret_cast<const PhysicalFile::PageID*>(
			this->getConstAreaTop() +
			FileInformation::LastLeafPageIDOffset);

	return *pageIDReadPos;
}

//
//	FUNCTION public
//	Btree::FileInformation::readObjectNum --
//		「挿入されているオブジェクト数」を読み込む
//
//	NOTES
//	「挿入されているオブジェクト数」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		挿入されているオブジェクト数
//
//	EXCEPTIONS
//	なし
//
ModUInt64
FileInformation::readObjectNum() const
{
	const ModUInt64*	objectNumReadPos =
		syd_reinterpret_cast<const ModUInt64*>(
			this->getConstAreaTop() +
			FileInformation::ObjectNumOffset);

	return *objectNumReadPos;
}

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
