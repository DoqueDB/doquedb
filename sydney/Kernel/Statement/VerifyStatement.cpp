// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VerifyStatement.cpp --
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2010, 2023 Ricoh Company, Ltd.
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
#include "Statement/VerifyStatement.h"
#include "Statement/Identifier.h"
#include "Statement/VerifyOptionList.h"
#include "Statement/IntegerValue.h"

#include "Common/Assert.h"
#if 0
#include "Analysis/VerifyStatement.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバの m_vecpElements 内でのindex
	enum {
        f_Type,			// オプション識別子
        f_Name,			// スキーマ名
        f_Option,		// オプションフラグ
        f__end_index
	};

    //
    // ENUM
    //		$$::Mask
    // MEMO
    //		オプションを取得する時のマスク
    // 
    namespace Mask
    {
        enum {
            Correct		= 1,
            Continue	= 2,
            Cascade		= 4,
            Verbose		= 8,
            Data		= 16
        };
    }
}

//
//	FUNCTION public
//	Statement::VerifyStatement::VerifyStatement -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ(2)
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
VerifyStatement::VerifyStatement(const IntegerValue* pcSchemaType_, const Identifier* pName_,
                                 const VerifyOptionList* pcList_)
	: Object(ObjectType::VerifyStatement, f__end_index, Object::System)
{
    setSchemaType(pcSchemaType_);
    setName(pName_);
	setOptionList(pcList_);
}

//
//	FUNCTION public
//	Statement::VerifyStatement::~VerifyStatement -- デストラクタ
//
//	NOTES
//	デストラクタ
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
VerifyStatement::~VerifyStatement()
{
}

//	FUNCTION public
//		Statement::VerifyStatement::getSchemaType
//
//	NOTES
//		検査対象のスキーマ種別を取得する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//		スキーマ種別
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送

int
VerifyStatement::getSchemaType() const
{
    ; _SYDNEY_ASSERT(m_vecpElements[f_Type]);

	const IntegerValue* pInt =
		_SYDNEY_DYNAMIC_CAST(const IntegerValue*, m_vecpElements[f_Type]);
    ; _SYDNEY_ASSERT(pInt);
    
    return pInt->getValue();
}

//	FUNCTION public
//		Statement::VerifyStatement::getName
//
//	NOTES
//		名前を取得する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeString&
//			スキーマ名
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送

const ModUnicodeString&
VerifyStatement::getName() const
{
    ; _SYDNEY_ASSERT(m_vecpElements[f_Name]);

    const Identifier* pIdent =
		_SYDNEY_DYNAMIC_CAST(const Identifier*, m_vecpElements[f_Name]);
    ; _SYDNEY_ASSERT(pIdent);
    ; _SYDNEY_ASSERT(pIdent->getIdentifier());

    return *(pIdent->getIdentifier());
}

//	FUNCTION public
//		Statement::VerifyStatement::isCorrect -- CORRECT フラグの取得
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool	true  : オプションが選択されている
//				false : オプションが選択されていない
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送

bool
VerifyStatement::isCorrect() const
{
    ; _SYDNEY_ASSERT(m_vecpElements[f_Option]);

    const IntegerValue* pInt =
		_SYDNEY_DYNAMIC_CAST(const IntegerValue*, m_vecpElements[f_Option]);
    ; _SYDNEY_ASSERT(pInt);

    return pInt->getValue() & Mask::Correct;
}

//	FUNCTION public
//		Statement::VerifyStatement::isContinue -- CONTINUE オプションの取得
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool	true  : オプションが選択されている
//				false : オプションが選択されていない
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送

bool
VerifyStatement::isContinue() const
{
    ; _SYDNEY_ASSERT(m_vecpElements[f_Option]);

	const IntegerValue* pInt =
		_SYDNEY_DYNAMIC_CAST(const IntegerValue*, m_vecpElements[f_Option]);
    ; _SYDNEY_ASSERT(pInt);

    return pInt->getValue() & Mask::Continue;
}

//	FUNCTION public
//		Statement::VerifyStatement::isCascade -- CASCADE オプションの取得
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool	true  : オプションが選択されている
//				false : オプションが選択されていない
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送

bool
VerifyStatement::isCascade() const
{
    ; _SYDNEY_ASSERT(m_vecpElements[f_Option]);

    const IntegerValue* pInt =
		_SYDNEY_DYNAMIC_CAST(const IntegerValue*, m_vecpElements[f_Option]);
    ; _SYDNEY_ASSERT(pInt);

    return pInt->getValue() & Mask::Cascade;
}

//	FUNCTION public
//		Statement::VerifyStatement::isVerbose -- VERBOSE オプションの取得
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool	true  : オプションが選択されている
//				false : オプションが選択されていない
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送

bool
VerifyStatement::isVerbose() const
{
    ; _SYDNEY_ASSERT(m_vecpElements[f_Option]);

    const IntegerValue* pInt =
		_SYDNEY_DYNAMIC_CAST(const IntegerValue*, m_vecpElements[f_Option]);
    ; _SYDNEY_ASSERT(pInt);

    return pInt->getValue() & Mask::Verbose;
}

bool
VerifyStatement::
isValue() const
{
    ; _SYDNEY_ASSERT(m_vecpElements[f_Option]);

    const IntegerValue* pInt =
		_SYDNEY_DYNAMIC_CAST(const IntegerValue*, m_vecpElements[f_Option]);
    ; _SYDNEY_ASSERT(pInt);

    return pInt->getValue() & Mask::Data;
}

//
//	FUNCTION public
//		Statement::VerifyStatement::setSchemaType -- スキーマ種別を設定する
//
//	NOTES
//
//	ARGUMENTS
//		IntegerValue* pType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
VerifyStatement::setSchemaType(const IntegerValue* pcType_)
{
    ; _SYDNEY_ASSERT(pcType_);
    
    m_vecpElements[f_Type] = const_cast<IntegerValue*>(pcType_);
}

//
//	FUNCTION public
//		Statement::VerifyStatement::
//
//	NOTES
//
//	ARGUMENTS
//		const VerifyOptionList* pcList_
//			オプション配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
VerifyStatement::setName(const Identifier* pcName_)
{
	;_SYDNEY_ASSERT (pcName_);

	m_vecpElements[f_Name] = const_cast<Identifier*>(pcName_);
}

//
//	FUNCTION public
//		Statement::VerifyStatement::setOptionList -- オプションを設定する
//
//	NOTES
//
//	ARGUMENTS
//		const VerifyOptionList* pcList_
//			オプション配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
VerifyStatement::setOptionList(const VerifyOptionList* pcList_)
{
    int iTmp = 0;

    // 設定されていれば値を作る
    if ( pcList_ )
    {
        // マスク値の作成
        int iMax = pcList_->getSize();
        for ( int i = 0; i < iMax; i++ )
        {
            switch( pcList_->getValue(i) )
            {
            case Option::Correct:		iTmp |= Mask::Correct;		break;
            case Option::Continue:	    iTmp |= Mask::Continue;		break;
            case Option::Cascade:		iTmp |= Mask::Cascade;      break;
            case Option::Verbose:		iTmp |= Mask::Verbose;      break;
            case Option::Data:			iTmp |= Mask::Data;	      break;
            default:					; _SYDNEY_ASSERT(false);
            }
        }
    }

    // 値設定
    if ( !m_vecpElements[f_Option] )
    {
        m_vecpElements[f_Option] = new IntegerValue();
    }

	IntegerValue* pInt = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_Option]);
	; _SYDNEY_ASSERT(pInt);

    pInt->setValue(iTmp);
}

//
//	FUNCTION public
//	Statement::VerifyStatement::copy -- 自身をコピーする
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
VerifyStatement::copy() const
{
	return new VerifyStatement(*this);
}

#if 0
namespace
{
	Analysis::VerifyStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
VerifyStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2001, 2002, 2004, 2005, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
