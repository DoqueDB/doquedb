// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NonManagePage.h --
//		管理機能なし物理ファイルの物理ページ関連クラス定義、関数宣言
// 
// Copyright (c) 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_NONMANAGEPAGE_H
#define __SYDNEY_PHYSICALFILE_NONMANAGEPAGE_H

#include "PhysicalFile/NonManageFile.h"
#include "PhysicalFile/Page.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{

//
//	CLASS
//	PhysicalFile::NonManagePage --
//		管理機能なし物理ファイルの物理ページ記述子クラス
//
//	NOTES
//
class NonManagePage : public Page
{
	friend class File;
	friend class NonManageFile;

public:

	// 物理ページデータサイズを返す
	PageSize getPageDataSize(const AreaNum	AreaNum_ = 1) const
		{ return m_File->getPageDataSize(AreaNum_); };
	
	// 物理ファイル記述子を返す
	File* getFile() const { return m_File; };

private:

	//
	// メンバ関数
	//

	// コンストラクタ
	NonManagePage(
		const Trans::Transaction&					Transaction_,
		NonManageFile*								File_,
		const PageID								PageID_,
		const Buffer::Page::FixMode::Value			FixMode_,
		const Buffer::ReplacementPriority::Value	ReplacementPriority_ =
									Buffer::ReplacementPriority::Middle);

	// コンストラクタ
	NonManagePage(const Trans::Transaction&				Transaction_,
				  NonManageFile*						File_,
				  const PageID							PageID_,
				  const Buffer::Page::FixMode::Value	FixMode_,
				  Admin::Verification::Progress&		Progress_);

	// デストラクタ
	virtual ~NonManagePage();

	// メモリー再利用
	void reset(
		const Trans::Transaction&			Transaction_,
		PageID								PageID_,
		Buffer::Page::FixMode::Value		FixMode_,
		Buffer::ReplacementPriority::Value	ReplacementPriority_ =
									Buffer::ReplacementPriority::Middle);
	
#ifdef DEBUG
	// 利用者に公開する領域のサイズを返す
	PageSize getUserAreaSize() const;
#endif

	//
	// データメンバ
	//

	// 物理ファイル記述子
	NonManageFile*				m_File;

	// 管理機能なし物理ファイルの物理ページ識別子
	static PageID	ID;

}; // end of class PhysicalFile::NonManagePage

} // end of namespace PhysicalFile

_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_NONMANAGEPAGE_H

//
//	Copyright (c) 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
