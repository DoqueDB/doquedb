// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PageManagePage.h --
//		物理ページ管理機能付き物理ファイルの
//		物理ページ関連クラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2004, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_PAGEMANAGEPAGE_H
#define __SYDNEY_PHYSICALFILE_PAGEMANAGEPAGE_H

#include "PhysicalFile/Page.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{

//
//	CLASS
//	PhysicalFile::PageManagePage --
//		物理ページ管理機能付き物理ファイルの物理ページ記述子クラス
//
//	NOTES
//	物理ページ管理機能付き物理ファイルの物理ページ記述子クラス。
//
class PageManagePage : public Page
{
	friend class File;

public:

	//
	// メンバ関数
	//

	// コンストラクタ
	PageManagePage(
		const Trans::Transaction&					Transaction_,
		File*										File_,
		const PageID								PageID_,
		const Buffer::Page::FixMode::Value			FixMode_,
		const Buffer::ReplacementPriority::Value	ReplacementPriority_ =
									Buffer::ReplacementPriority::Middle);

	// コンストラクタ
	PageManagePage(const Trans::Transaction&			Transaction_,
				   File*								File_,
				   const PageID							PageID_,
				   const Buffer::Page::FixMode::Value	FixMode_,
				   Admin::Verification::Progress&		Progress_);

	// 物理ページデータサイズを返す
	PageSize getPageDataSize(const AreaNum	AreaNum_ = 1) const;

	// 物理ファイル記述子を返す
	File* getFile() const { return m_File; };

private:

	// デストラクタ
	virtual ~PageManagePage();

	// メモリー再利用
	void reset(
		const Trans::Transaction&			Transaction_,
		PageID								PageID_,
		Buffer::Page::FixMode::Value		FixMode_,
		Buffer::ReplacementPriority::Value	ReplacementPriority_ =
									Buffer::ReplacementPriority::Middle);

	// 利用者に公開する領域のサイズを返す
#ifdef DEBUG
	PageSize getUserAreaSize() const;
#endif

	//
	// データメンバ
	//

	// 物理ファイル記述子
	// PageManagePageはPageManageFileとPageManageFile2から使われるので、
	// 型を指定できない。
	File*				m_File;

}; // end of class PhysicalFile::PageManagePage

} // end of namespace PhysicalFile

_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_PAGEMANAGEPAGE_H

//
//	Copyright (c) 2000, 2001, 2004, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
