// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoItem.h -- オートロック項目関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOCK_AUTOITEM_H
#define	__SYDNEY_LOCK_AUTOITEM_H

#include "Lock/Module.h"
#include "Lock/Item.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

//	CLASS
//	Lock::AutoItem -- オートロック項目記述子を表すクラス
//
//	NOTES

class AutoItem : public ModAutoPointer<Item>
{
public:
	// コンストラクター
	AutoItem(Item* item);
	// デストラクター
	~AutoItem();

	// ロック項目記述子を破棄する
	virtual void			free();
};

//	FUNCTION public
//	Lock::AutoItem::AutoItem --
//		オートロック項目記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Item*			item
//			オートロック項目記述子が保持する
//			ロック項目記述子を格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoItem::AutoItem(Item* item)
	: ModAutoPointer<Item>(item)
{}

//	FUNCTION public
//	Lock::AutoItem::~AutoItem --
//		オートロック項目記述子を表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoItem::~AutoItem()
{
	free();
}

//	FUNCTION public
//	Lock::AutoItem::free -- 保持するロック項目記述子を破棄する
//
//	NOTES
//		オートロック項目記述子の破棄時などに呼び出される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
AutoItem::free()
{
	if (isOwner())
		if (Item* item = release())
			Item::detach(item);
}

_SYDNEY_LOCK_END
_SYDNEY_END

#endif	// __SYDNEY_LOCK_AUTOITEM_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
