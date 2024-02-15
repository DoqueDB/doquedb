// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirPage.h -- ビットマップページへのディレクトリ構造ページ
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_DIRPAGE_H
#define __SYDNEY_BITMAP_DIRPAGE_H

#include "Bitmap/Module.h"
#include "Bitmap/NodePage.h"
#include "Bitmap/BitmapPage.h"

#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class BitmapFile;
class LongBitmapIterator;

//
//	CLASS
//	Bitmap::DirPage -- B木部分のページ
//
//	NOTES
//
class DirPage : public NodePage
{
	friend class LongBitmapIterator;
	
public:
	//
	//	STRUCT
	//	Bitmap::DirPage::Header
	//
	struct Header
	{
		int	m_iStepCount;	// 段数
	};
	
	// コンストラクタ
	DirPage(BitmapFile& cFile_);
	// デストラクタ
	virtual ~DirPage();

	// 終端か
	bool isEnd() { return m_bEndOfData; }
	// 次に移動する
	void next();
	// 指定位置に移動する
	void seek(ModSize offset_, bool bUpdate_ = false);
	// 現在位置のデータを得る
	PhysicalFile::PageID get();
	// 現在位置のデータを書く
	void set(PhysicalFile::PageID uiPageID_);

	// 物理ページを設定する
	void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_);

	// 初期化
	void initialize();

	// 物理ページをdetachする
	void detach();

	// ステップ数を得る
	int getStepCount() { return m_pHeader->m_iStepCount; }
	// ステップ数を設定する
	void setStepCount(int iStepCount_)
	{
		dirty();
		m_pHeader->m_iStepCount = iStepCount_;
	}

	// 格納エントリ数を得る
	static ModSize getMaxCount(Os::Memory::Size uiPageSize_);
	
private:
	// ページの内容を読み込む
	void load();
	// 物理ページが変更されたので読み直す
	void reload();

	BitmapFile& m_cFile;

	// ページヘッダー
	Header* m_pHeader;
	
	PhysicalFile::PageID* m_pBegin;
	PhysicalFile::PageID* m_pEnd;

	// 現在位置
	PhysicalFile::PageID* m_pCurrent;

	// 下位のDIRページ
	DirPage* m_pDirPage;

	// 終了か
	bool m_bEndOfData;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_DIRPAGE_H

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
