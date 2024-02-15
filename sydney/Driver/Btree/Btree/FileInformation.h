// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileInformation.h -- Ｂ＋木ファイル管理情報クラスのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2023, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_FILEINFORMATION_H
#define __SYDNEY_BTREE_FILEINFORMATION_H

#include "Btree/AreaObject.h"
#include "Btree/Version.h"

_SYDNEY_BEGIN

namespace Btree
{

//
//	CLASS
//	Btree::FileInformation -- Ｂ＋木ファイル管理情報クラス
//
//	NOTES
//	Ｂ＋木ファイル管理情報クラス。
//	Ｂ＋木ファイル管理情報は、ファイルの先頭物理ページである
//	ヘッダページ内に記録され、下図のような物理構成である。
//
//		┌──────────────────────────┐
//		│　　　　　　ファイルバージョン (32bit)　　　　　　　│
//		├──────────────────────────┤
//		│　　　　　最終更新時刻の年・月・日 (32bit)　　　　　│
//		├──────────────────────────┤
//		│　　　最終更新時刻の時・分・秒・ミリ秒 (32bit)　　　│
//		├──────────────────────────┤
//		│　　　　　　　現在の木の深さ (32bit)　　　　　　　　│
//		├──────────────────────────┤
//		│　　ルートノードページの物理ページ識別子 (32bit)　　│
//		├──────────────────────────┤
//		│　　先頭リーフページの物理ページ識別子 (32bit)　　　│
//		├──────────────────────────┤
//		│　　最終リーフページの物理ページ識別子 (32bit)　　　│
//		├──────────────────────────┤
//		│　　　　挿入されているオブジェクト数 (64bit)　　　　│
//		└──────────────────────────┘
//
class FileInformation : public Btree::AreaObject
{
public:

	// コンストラクタ
	FileInformation(
		const Trans::Transaction*			Transaction_,
		PhysicalFile::File*					File_,
		const Buffer::Page::FixMode::Value	FixMode_,
		const bool							SavePage_);

	// コンストラクタ
	FileInformation(const Trans::Transaction*	Transaction_,
					PhysicalFile::Page*			HeaderPage_);

	// デストラクタ
	~FileInformation();

	// ファイル管理情報を初期化する
	static void initialize(const Trans::Transaction*	Transaction_,
						   PhysicalFile::Page*			HeaderPage_);

	//
	// 書き込みメソッド
	//

	// 「ファイルバージョン」を書き込む
	void writeFileVersion(const FileVersion::Value	FileVersion_);

	// 「最終更新時刻」に現在の時刻を書き込む
	void writeModificationTime();

	// 「現在の木の深さ」を書き込む
	void writeTreeDepth(const ModUInt32	TreeDepth_);

	// 「現在の木の深さ」をインクリメントする
	void incTreeDepth();

#ifdef OBSOLETE
	// 「現在の木の深さ」をデクリメントする
	void decTreeDepth();
#endif //OBSOLETE

	// 「ルートノードページの物理ページ識別子」を書き込む
	void writeRootNodePageID(const PhysicalFile::PageID	RootNodePageID_);

	// 「先頭リーフページの物理ページ識別子」を書き込む
	void writeTopLeafPageID(const PhysicalFile::PageID	TopLeafPageID_);

	// 「最終リーフページの物理ページ識別子」を書き込む
	void writeLastLeafPageID(const PhysicalFile::PageID	LastLeafPageID_);

	// 「挿入されているオブジェクト数」を書き込む
	void writeObjectNum(const ModUInt64	ObjectNum_);

	// 「挿入されているオブジェクト数」をインクリメントする
	void incObjectNum();

	// 「挿入されているオブジェクト数」をデクリメントする
	void decObjectNum();

	//
	// 読み込みメソッド
	//

	// 「ファイルバージョン」を読み込む
	FileVersion::Value readFileVersion() const;

#ifdef OBSOLETE
	// 「最終更新時刻」を読み込む
	void readModificationTime(int&	Year_,
							  int&	Month_,
							  int&	Day_,
							  int&	Hour_,
							  int&	Minute_,
							  int&	Second_,
							  int&	MilliSecond_) const;
#endif //OBSOLETE

	// 「現在の木の深さ」を読み込む
	ModUInt32 readTreeDepth() const;

	// 「ルートノードページの物理ページ識別子」を読み込む
	PhysicalFile::PageID readRootNodePageID() const;

	// 「先頭リーフページの物理ページ識別子」を読み込む
	PhysicalFile::PageID readTopLeafPageID() const;

	// 「最終リーフページの物理ページ識別子」を読み込む
	PhysicalFile::PageID readLastLeafPageID() const;

	// 「挿入されているオブジェクト数」を読み込む
	ModUInt64 readObjectNum() const;

	//
	// 静的データメンバ
	//

	// ファイル管理情報が記録されている物理エリアの識別子
	static const PhysicalFile::AreaID	AreaID; // = 0

	// ファイル管理情報の記録サイズ
	static const Os::Memory::Size		ArchiveSize;

private:

	//
	// 静的データメンバ
	//

	// 「ファイルバージョン」のファイル管理情報エリア内での位置 [byte]
	static const Os::Memory::Offset		FileVersionOffset;

	// 「最終更新時刻」のファイル管理情報エリア内での位置 [byte]
	static const Os::Memory::Offset		ModificationTimeOffset;

	// 「現在の木の深さ」のファイル管理情報エリア内での位置 [byte]
	static const Os::Memory::Offset		TreeDepthOffset;

	// 「ルートノードページの物理ページ識別子」の
	// ファイル管理情報エリア内での位置 [byte]
	static const Os::Memory::Offset		RootNodePageIDOffset;

	// 「先頭リーフページの物理ページ識別子」の
	// ファイル管理情報エリア内での位置 [byte]
	static const Os::Memory::Offset		TopLeafPageIDOffset;

	// 「最終リーフページの物理ページ識別子」の
	// ファイル管理情報エリア内での位置 [byte]
	static const Os::Memory::Offset		LastLeafPageIDOffset;

	// 「挿入されているオブジェクト数」の
	// ファイル管理情報エリア内での位置 [byte]
	static const Os::Memory::Offset		ObjectNumOffset;

}; // end of class Btree::FileInformation

} // end of namespace Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_FILEINFORMATION_H

//
//	Copyright (c) 2000, 2001, 2023, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
