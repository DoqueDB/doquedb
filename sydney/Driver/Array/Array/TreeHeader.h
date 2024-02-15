// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TreeHeader.h --
// 
// Copyright (c) 2007, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_TREEHEADER_H
#define __SYDNEY_ARRAY_TREEHEADER_H

#include "Array/Module.h"

#include "PhysicalFile/Types.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_ARRAY_BEGIN

//
//	CLASS
//	Array::TreeHeader --
//
//	NOTES
//	ARRAY does NOT manage the count where the muximum value is inserted.
//	Because the usage of the ARRAY is different from BTREE2,
//	the ARRAY's key of the new entry is NOT increased monotonically.
//
class TreeHeader
{
public:
	// コンストラクタ
	TreeHeader();
	// デストラクタ
	~TreeHeader();

	// Initialize.
	void initialize();

	// ルートページを得る
	PhysicalFile::PageID getRootPageID() const
		{ return m_uiRootPageID; }
	// ルートページを設定する
	void setRootPageID(PhysicalFile::PageID uiPageID_)
		{ m_uiRootPageID = uiPageID_; }

	// 一番左のリーフページを得る
	PhysicalFile::PageID getLeftLeafPageID() const
		{ return m_uiLeftLeafPageID; }
	// 一番左のリーフページを設定する
	void setLeftLeafPageID(PhysicalFile::PageID uiPageID_)
		{ m_uiLeftLeafPageID = uiPageID_; }

	// 一番右のリーフページを得る
	PhysicalFile::PageID getRightLeafPageID() const
		{ return m_uiRightLeafPageID; }
	// 一番右のリーフページを設定する
	void setRightLeafPageID(PhysicalFile::PageID uiPageID_)
		{ m_uiRightLeafPageID = uiPageID_; }

	// エントリ数を増やす(同時に総挿入数も更新される)
	void incrementCount();
	// エントリ数を減らす
	void decrementCount();
	// エントリ数を得る
	ModSize getCount() const { return m_uiCount; }

	// 木の段数を増やす
	void incrementStepCount() { ++m_uiStepCount; }
	// 木の段数を減らす
	void decrementStepCount();
	// 木の段数を得る
	ModSize getStepCount() const { return m_uiStepCount; }

	// バッファに書き込む
	void dump(ModUInt32*& p_) const;
	// バッファから読み込む
	void restore(const ModUInt32*& p_);
	// ダンプサイズを得る
	ModSize getDumpedByteSize() const { return static_cast<ModSize>(sizeof(*this)); }
	
private:
	// [NOTE] デストラクタに不要なvirtualがあったため、
	//  インスタンスに仮想関数テーブルへのポインタが含まれてしまっていた。
	//  そのため、HeaderPage::preFlush()でそのポインタが書かれてしまい、
	//  32bit版と64bit版とでポインタのバイト数が異なることから、
	//  互換性がなくなってしまっていた。
	//  virtualを削除することでポインタは書かれなくなるが、
	//  HeaderPage::setPhysicalPage()は、ポインタが書かれているとみなすので、
	//  そのままでは以前に作成された索引を読めなくなってしまう。
	//  32bit版で作成された索引をそのまま使うためにダミーのメンバーを追加し、
	//  64bit版で作成された索引は再作成してもらうことにする。
	char					_dummy2[4];				// BACKWARD COMPATIBILITY
	
	PhysicalFile::PageID	m_uiRootPageID;			// ルートページ
	PhysicalFile::PageID	m_uiLeftLeafPageID;		// 一番左のリーフページ
	PhysicalFile::PageID	m_uiRightLeafPageID;	// 一番右のリーフページ
	ModUInt32				m_uiCount;				// エントリ数
	ModUInt32				m_uiStepCount;			// 木の段数
	char					_dummy[4];				// BACKWARD COMPATIBILITY
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_HEADERPAGE_H

//
//	Copyright (c) 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
