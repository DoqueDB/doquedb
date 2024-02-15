// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Item.cpp -- コマンドの要素を表すクラス
// 
// Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "SydTest";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SydTest/Item.h"

_SYDNEY_USING

using namespace SydTest;

//
//	FUNCTION public
//	SydTest::Item::Item -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Item::Type eType_
//		要素のタイプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Item::Item(Item::Type eType_)
: m_pNext(0), m_eType(eType_)
{
}

//
//	FUNCTION public
//	SydTest::Item::~Item -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Item::~Item()
{
	 this->release();
}

//
//	FUNCTION public
//	SydTest::Item::setNext -- 次の要素をセットする
//
//	NOTES
//	現在の要素の次に来る要素へのポインタをセットする
//
//	ARGUMENTS
//	Item* pNext_
//		次のコマンド要素を表すオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Item::setNext(Item* pNext_)
{
	m_pNext = pNext_;
}

//
//	FUNCTION public
//	SydTest::Item::getNext -- 次の要素を得る
//
//	NOTES
//	現在の要素の次に来る要素のオブジェクトを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Item*
//		次の要素を表すオブジェクトへのポインタ
//		次の要素がない場合はヌルポインタが返る
//
//	EXCEPTIONS
//	なし
//
Item*
Item::getNext() const
{
	return m_pNext;
}

//
//	FUNCTION public
//	SydTest::Item::getType -- 要素のタイプを得る
//
//	NOTES
//	コマンド要素のタイプ(コマンド 文字列 整数 パラメータ)を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Item::Type
//	コマンドのタイプ
//
//	EXCEPTIONS
//	なし
//

Item::Type
Item::getType() const
{
	return m_eType;
}

//
//	FUNCTION public
//	SydTest::Item::release -- オブジェクトの破棄
//
//	NOTES
//	オブジェクトを破棄する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//  なし
//
//	EXCEPTIONS
//	なし
//

void
Item::release()
{
	if (this->m_pNext != 0) {
		this->m_pNext->release();
		delete this->m_pNext, this->m_pNext=0;
	}
//	delete this;
}

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
