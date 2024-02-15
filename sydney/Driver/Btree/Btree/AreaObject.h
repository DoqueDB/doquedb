// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaObject.h --
//		エリアオブジェクトの基本クラスのヘッダーファイル
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

#ifndef __SYDNEY_BTREE_AREAOBJECT_H
#define __SYDNEY_BTREE_AREAOBJECT_H

#include "Btree/Module.h"

#include "Common/Object.h"
#include "FileCommon/DataManager.h"
#include "PhysicalFile/Types.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

namespace PhysicalFile
{
class File;
class Page;
}

_SYDNEY_BTREE_BEGIN

//
//	CLASS
//	Btree::AreaObject --
//		エリアオブジェクトの基本クラス
//
//	NOTES
//	物理エリアに記録するオブジェクトの基本クラス。
//
class AreaObject : public Common::Object
{
	friend class File;

public:

	// コンストラクタ
	AreaObject(
		const Trans::Transaction*					pTransaction_,
		PhysicalFile::File*							pFile_,
		const PhysicalFile::PageID					uiPageID_,
		const PhysicalFile::AreaID					uiAreaID_,
		const Buffer::Page::FixMode::Value			eFixMode_,
		const Buffer::ReplacementPriority::Value	ReplacementPriority_,
		const bool									SavePage_);

	// コンストラクタ
	AreaObject(
		const Trans::Transaction*	pTransaction_,
		PhysicalFile::Page*			pPage_,
		const PhysicalFile::AreaID	uiAreaID_);

	// デストラクタ
	virtual ~AreaObject();

	// エリアオブジェクトが記録されている物理ページの
	// 空き領域サイズを返す [byte]
	Os::Memory::Size getFreeAreaSize() const;

#ifdef OBSOLETE
	// エリアオブジェクトが記録されている物理ページの
	// 物理ページ識別子を返す
	PhysicalFile::PageID getPageID() const;
#endif //OBSOLETE

	// アタッチした物理ページをアタッチ前の状態にするように設定する
	void setRecoverPage();

protected:

	// 物理ページを再設定する
	void resetPhysicalPage(PhysicalFile::Page*	NewPhysicalPage_);

	// 物理エリア先頭へのポインタを返す
	const char* getConstAreaTop() const;

	//
	// データメンバ
	//

	// トランザクション記述子
	const Trans::Transaction*			m_pTransaction;

	// 物理ファイル記述子
	PhysicalFile::File*					m_pFile;

	// 物理ページ記述子
	PhysicalFile::Page*					m_pPage;

	// 物理エリア識別子
	PhysicalFile::AreaID				m_uiAreaID;

	// 物理エリアの先頭へのポインタ
	char*								m_AreaTop;

	// 物理エリアの先頭へのポインタ
	const char*							m_ConstAreaTop;

	// フィックスモード
	const Buffer::Page::FixMode::Value	m_eFixMode;

	// 自身で物理ページをアタッチしたか
	bool								m_bAttachPageBySelf;

	// 物理ページをキャッシュするかどうか
	bool								m_SavePage;

	// 物理ページをアタッチ前の状態に戻すかどうか
	bool								m_RecoverPage;

}; // end of class Btree::AreaObject

_SYDNEY_BTREE_END
_SYDNEY_END

#endif // __SYDNEY_BTREE_AREAOBJECT_H

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
