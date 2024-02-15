// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Types.h --
//		物理ファイルマネージャ用のデータ型などを定義するヘッダファイル
// 
// Copyright (c) 2000, 2001, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_TYPES_H
#define __SYDNEY_PHYSICALFILE_TYPES_H

#include "Common/Common.h"

#include "Os/File.h"

#include "Version/Page.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{

//
//	TYPEDEF public
//	PhysicalFile::FileSize --
//		物理ファイルサイズを表す型
//
//	NOTES
//	物理ファイルサイズを表す型。
//
typedef Os::File::Size	FileSize;

//
//	TYPEDEF public
//	PhysicalFile::PageID --
//		物理ページ識別子を表す型
//
//	NOTES
//	物理ページ識別子を表す型。
//
typedef Version::Page::ID	PageID;

//
//	TYPEDEF public
//	PhysicalFile::PageSize --
//		物理ページサイズを表す型
//
//	NOTES
//	物理ページサイズを表す型。
//
typedef Os::Memory::Size	PageSize;

//
//	TYPEDEF public
//	PhysicalFile::PageSize::ShortPageSize --
//		物理ページサイズを表す型
//
//	NOTES
//	物理ページサイズを表す型。
//
typedef unsigned short		ShortPageSize;

//
//	TYPEDEF public
//	PhysicalFile::PageOffset --
//		物理ページ内のオフセットを表す型
//
//	NOTES
//	物理ページ内のオフセットを表す型。
//
typedef int	PageOffset;

typedef short	ShortPageOffset;

//
//	TYPEDEF public
//	PhysicalFile::PageNum --
//		物理ページ数を表す型
//
//	NOTES
//	物理ページ数を表す型。
//
typedef unsigned int PageNum;

//
//	TYPEDEF public
//	PhysicalFile::ShortPageNum --
//		物理ページ数を表す型
//		『空き領域管理表ヘッダに記録されている情報の縮小』のために作成した。
//		2001/04/21 現在では AreaManagerFile::Header 内でしか使用していない 
//
//	NOTES
//	物理ページ数を表す型。
//
typedef unsigned short ShortPageNum;

//
//	TYPEDEF public
//	PhysicalFile::AreaID --
//		物理エリア識別子を表す型
//
//	NOTES
//	物理エリア識別子を表す型。
//
typedef PageID	AreaID;

//
//	TYPEDEF public
//	PhysicalFile::AreaSize --
//		物理エリアサイズを表す型
//
//	NOTES
//	物理エリアサイズを表す型。
//
typedef PageSize	AreaSize;

typedef ShortPageSize	ShortAreaSize;

//
//	TYPEDEF public
//	PhysicalFile::AreaOffset --
//		物理エリアオフセットを表す型
//
//	NOTES
//	物理エリアオフセットを表す型。
//
typedef PageOffset	AreaOffset;

//
//	TYPEDEF public
//	PhysicalFile::AreaNum --
//		物理エリア数を表す型
//
//	NOTES
//	物理エリア数を表す型。
//
typedef PageNum	AreaNum;

typedef ShortPageNum	ShortAreaNum;

//
//	ENUM public
//	PhysicalFile::Type --
//		物理ファイルタイプ
//
//	NOTES
//	現在、物理ファイルタイプには、以下の3種類がある。
//		1. 空き領域管理機能付き物理ファイル
//		2. 物理ページ管理機能付き物理ファイル
//		3. 管理機能なし物理ファイル
//
enum Type
{
	// 空き領域管理機能付き物理ファイル
	AreaManageType = 0,
	// 物理ページ管理機能付き物理ファイル
	PageManageType,
	// 管理機能なし物理ファイル
	NonManageType,
	// 物理エリア管理機能付き物理ファイル
	DirectAreaType,
	// 物理ページ管理機能付き物理ファイル
	PageManageType2,
	// 物理ファイルタイプ数
	FileTypeNum
};

namespace ConstValue
{
	// 物理ページ内の使用率上限のデフォルト値
	const unsigned int	DefaultPageUseRate = 100;

	// バージョンファイル最大サイズのデフォルト値
	const Os::File::Size	DefaultFileMaxSize = 0;

	// バージョンファイルエクステンションサイズのデフォルト値
	const Os::File::Size	DefaultFileExtensionSize = 0;

	// 無効な物理ページ識別子
	const PageID		UndefinedPageID = 0xFFFFFFFF;

	// 無効な物理エリア識別子
	const AreaID		UndefinedAreaID = 0xFFFFFFFF;

	// 無効な物理エリアオフセット（物理ページ内でのオフセット）
	const PageOffset	UndefinedAreaOffset = 0xFFFFFFFF;

	// 無効な物理ページ数
	const PageNum		UndefinedPageNum = 0xFFFFFFFF;

	// 無効な物理エリアサイズ
	const AreaSize		UndefinedAreaSize = 0xFFFFFFFF;


	// 2byte ページサイズの為の上位バイト抽出マスク値
	const PageSize		PageSizeUpperMask = 0xFFFF0000;

	// 2byte AreaIDの為の上位バイト抽出マスク値
	const AreaID		AreaIDUpperMask = 0xFFFF0000;
}

} // end of namespace PhysicalFile

_SYDNEY_END

#endif // __SYDNEY_PHYSICALFILE_TYPES_H

//
//	Copyright (c) 2000, 2001, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
