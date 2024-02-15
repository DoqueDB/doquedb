// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SetTransactionStatement.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Statement";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/Type.h"
#include "Statement/SetTransactionStatement.h"
#include "Statement/TransactionModeList.h"
#include "Statement/TransactionMode.h"
#include "Statement/IntegerValue.h"
#include "Statement/TransactAccMode.h"
#include "Statement/IsolationLevel.h"

#include "Common/Assert.h"
#if 0
#include "Analysis/SetTransactionStatement.h"
#endif

_SYDNEY_USING
using namespace Statement;

namespace
{
	// メンバのm_vecpElements内でのindex
	enum
	{
		f_TransMode,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::SetTransactionStatement::SetTransactionStatement
//		-- コンストラクタ(1)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const TransactionMode* pcTrensMode_
//		トランザクションモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
SetTransactionStatement::SetTransactionStatement(
		const TransactionModeList* pcTransMode_)
	: Object(ObjectType::SetTransactionStatement,
			 f__end_index)
{
	setTransactMode(pcTransMode_);
}

//
//	FUNCTION public
//	Statement::SetTransactionStatement::~SetTransactionStatement
//		-- デストラクタ
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
SetTransactionStatement::~SetTransactionStatement()
{
}

//
//	FUNCTION public
//	Statement::SetTransactionStatement::setTransactMode
//		-- トランザクションモードリストアクセサ
//
//	NOTES
//	トランザクションモードリストアクセサ
//
//	ARGUMENTS
//	const TransactionModeList* pcTransMode_
//		トランザクションリストモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
SetTransactionStatement::setTransactMode(
		const TransactionModeList* pcTransModeList_)
{
	Object* pcTransMode = 0;
	IntegerValue* pcAccModeVal = 0;
	IntegerValue* pcIsoLevelVal = 0;
	bool bUsingSnapshot = false;

	int n = pcTransModeList_->getCount();

	for (int i = 0; i < n; ++i)
	{
		pcTransMode = pcTransModeList_->getAt(i);

		if ( pcTransMode && ObjectType::TransactAccMode == pcTransMode->getType() )
		{
			TransactAccMode* pcAccMode = _SYDNEY_DYNAMIC_CAST(TransactAccMode*, pcTransMode);
			; _SYDNEY_ASSERT(pcAccMode);

			pcAccModeVal =
				new IntegerValue(pcAccMode->getAccMode()->getValue());
		}
		if ( pcTransMode && ObjectType::IsolationLevel == pcTransMode->getType() )
		{
			IsolationLevel* pcIsoLevel = _SYDNEY_DYNAMIC_CAST(IsolationLevel*, pcTransMode);
			; _SYDNEY_ASSERT(pcIsoLevel);

			pcIsoLevelVal =
				new IntegerValue(pcIsoLevel->getIsoLevel()->getValue());
		}
		if ( pcTransMode && ObjectType::IntegerValue == pcTransMode->getType() )
		{
			IntegerValue* pcInteger = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pcTransMode);
			; _SYDNEY_ASSERT(pcInteger);
			
			bUsingSnapshot = (pcInteger->getValue() ? true : false);
		}
	}

	if ( pcAccModeVal == 0 )
	{
		pcAccModeVal = new IntegerValue(TransactionMode::AccUnknown);
	}
	if ( pcIsoLevelVal == 0 )
	{
		pcIsoLevelVal = new IntegerValue(TransactionMode::IsoUnknown);
	}

	//TransactMode クラス構築
	setTransactMode(new TransactionMode(pcAccModeVal, pcIsoLevelVal, bUsingSnapshot));

	return;
}


//
//	FUNCTION public
//	Statement::SetTransactionStatement::
//		-- TransactionMode へのアクセサ
//
//	NOTES
//	TransactionMode へのアクセサ
//
//	ARGUMENTS
//
//
//	RETURN
//
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
SetTransactionStatement::setTransactMode(
	const TransactionMode* pcTransMode_)
{
	m_vecpElements[f_TransMode] = const_cast<TransactionMode*>(pcTransMode_);
}

//
//	FUNCTION public
//	Statement::SetTransactionStatement::
//		-- TransactionMode へのアクセサ
//
//	NOTES
//	TransactionMode へのアクセサ
//
//	ARGUMENTS
//
//
//	RETURN
//
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
TransactionMode*
SetTransactionStatement::getTransactMode() const
{
	TransactionMode* pResult = 0;
	Object* pObj = m_vecpElements[f_TransMode];
	if ( pObj && ObjectType::TransactionMode == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(TransactionMode*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::SetTransactionStatement::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Statement::Object*
//
//	EXCEPTIONS
//	なし
//
Object*
SetTransactionStatement::copy() const
{
	return new SetTransactionStatement(*this);
}

#if 0
namespace
{
	Analysis::SetTransactionStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
SetTransactionStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
