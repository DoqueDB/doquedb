// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Locator.cpp -- 
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "LogicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "LogicalFile/Locator.h"

#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_LOGICALFILE_USING

//
//	FUNCTION public
//	LogicalFile::Locator::Locator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Locator::Locator(const Trans::Transaction& cTransaction_)
{
	// トランザクション記述子の参照カウンタを1増やす
	m_pTransaction = Trans::Transaction::attach(cTransaction_);
}

//
//	FUNCTION public
//	LogicalFile::Locator::~Locator -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Locator::~Locator()
{
	// トランザクション記述子の参照カウンタを1減らす
	Trans::Transaction::detach(m_pTransaction);
}

//
//	FUNCTION public
//	LogicalFile::Locator::isInvalid -- 無効なLocatorかどうかをチェックする
//
//	NOTES
//	この基底クラスではトランザクションが実行中かどうかをチェックする
//	さらに無効になる場合がサブクラスで存在する場合には、
//	この関数を上書きする必要がある
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		無効な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Locator::isInvalid() const
{
	return (m_pTransaction->isInProgress() == false) ? true : false;
}

//
//	FUNCTION protected
//	LogicalFile::Locator::getTransaction -- トランザクション記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Trans::Transaction&
//		トランザクション記述子への参照
//
//	EXCEPTIONS
//
const Trans::Transaction&
Locator::getTransaction() const
{
	return *m_pTransaction;
}

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
