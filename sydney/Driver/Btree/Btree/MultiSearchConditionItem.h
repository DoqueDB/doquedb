// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiSearchConditionItem.h --
//		Ｂ＋木索引による複合検索条件解析データクラスのヘッダーファイル
// 
// Copyright (c) 2000, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_MULTISEARCHCONDITIONITEM_H
#define __SYDNEY_BTREE_MULTISEARCHCONDITIONITEM_H

#include "ModString.h"

#include "Btree/Module.h"
#include "Common/Object.h"

_SYDNEY_BEGIN

namespace Btree
{

//
//	CLASS
//	Btree::MultiSearchConditionItem --
//		Ｂ＋木索引による複合検索条件解析データクラス
//
//	NOTES
//	Ｂ＋木索引による複合検索条件解析データクラス。
//
class MultiSearchConditionItem : public Common::Object
{
public:

	// コンストラクタ
	MultiSearchConditionItem();

	// デストラクタ
	~MultiSearchConditionItem();

	//
	// データメンバ
	//

	// 検索範囲開始条件文字列
	ModUnicodeString	m_cstrStart;

	// 検索範囲開始条件の比較演算子文字列
	int			m_iStartOpe;

	// 検索範囲終了条件文字列
	ModUnicodeString	m_cstrStop;

	// 検索範囲終了条件の比較演算子文字列
	int			m_iStopOpe;

private:

};

} // end of namespace Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_MULTISEARCHCONDITIONITEM_H

//
//	Copyright (c) 2000, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
