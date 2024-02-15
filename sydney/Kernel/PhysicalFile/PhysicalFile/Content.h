// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Content.h --
//		物理ページのバッファリング内容関連のクラス定義、関数宣言
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

#ifndef __SYDNEY_PHYSICALFILE_CONTENT_H
#define __SYDNEY_PHYSICALFILE_CONTENT_H

#include "PhysicalFile/Module.h"
#include "PhysicalFile/Types.h"

#include "Common/Common.h"
#include "Version/Page.h"

_SYDNEY_BEGIN
_SYDNEY_PHYSICALFILE_BEGIN

//	CLASS
//	PhysicalFile::Content --
//		物理ページのバッファリング内容クラス
//
//	NOTES

class SYD_PHYSICALFILE_FUNCTION Content : public Common::Object
{
	friend class Page;

public:
	//
	// メンバ関数
	//

	// コンストラクタ
	Content();

	// デストラクタ
	virtual ~Content();

	// void*へのキャスト演算子
	operator void*() const;

	// char*へのキャスト演算子
	operator char*() const;

	// const void*へのキャスト演算子
	operator const void*() const;

	// const char*へのキャスト演算子
	operator const char*() const;

	// バッファリング内容のサイズを返す
	Os::Memory::Size getSize() const;


private:

	//
	// メンバ関数
	//

	// コンストラクタ
	Content(Version::Page::Memory*			Memory_,
			Type							PhysicalFileType_,
			Buffer::Page::FixMode::Value	FixMode_,
			PageSize						PageSize_,
			PageSize						PageDataSize_);

	// 再利用
	void reset(Version::Page::Memory*		Memory_,
			   Type							PhysicalFileType_,
			   Buffer::Page::FixMode::Value	FixMode_,
			   PageSize						PageSize_,
			   PageSize						PageDataSize_)
	{
		m_Memory = Memory_;
		m_PhysicalFileType = PhysicalFileType_;
		m_FixMode = FixMode_;
		m_PageSize = PageSize_;
		m_PageDataSize = PageDataSize_;
	}

	//
	// データメンバ
	//

	// 物理ファイルタイプ
	Type							m_PhysicalFileType;

	// バージョンページのバッファリング内容への参照
	Version::Page::Memory*			m_Memory;

	// フィックスモード
	Buffer::Page::FixMode::Value	m_FixMode;

	// 管理するバージョンページサイズ [byte]
	PageSize						m_PageSize;

	// 管理するバージョンページデータサイズ [byte]
	PageSize						m_PageDataSize;
	
}; // end of class PhysicalFile::Content

_SYDNEY_PHYSICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_CONTENT_H

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
