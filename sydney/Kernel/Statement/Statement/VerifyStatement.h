// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VerifyStatement.h --
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_VERIFYSTATEMENT_H
#define __SYDNEY_STATEMENT_VERIFYSTATEMENT_H

#include "Common/Common.h"
#include "Statement/Object.h"
#include "Statement/VerifyOptionList.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Identifier;
class IntegerValue;

//
//	CLASS
//	Statement::VerifyStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION VerifyStatement : public Statement::Object
{
public:
	//constructor
	VerifyStatement()
		: Object(ObjectType::VerifyStatement)
	{}
    // コンストラクタ
    VerifyStatement(const IntegerValue* pcSchemaType_, 
					const Identifier* pName_,
                    const VerifyOptionList* pcList_);
    // デストラクタ
    virtual ~VerifyStatement();
    
    // 整合性検査種別を表す識別子
    struct SchemaType {
        enum {
            NoneType,
            Database,
            Table,	
            Index
        };
    };

    // オプションを表す識別子
    struct Option {
        enum {
            NoneType,
            Correct,
            Continue,
            Cascade,
            Verbose,
            Data
        };
    };

    // 検査対象のスキーマ種別を取得する
    int getSchemaType() const;

    // 名前を取得する
    const ModUnicodeString& getName() const;

    // 各フラグの取得
    bool isCorrect() const;
    bool isContinue() const;
    bool isCascade() const;
    bool isVerbose() const;
    bool isValue() const;

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
    // スキーマ種別を設定する
    void setSchemaType(const IntegerValue* pcType_);
    
    // 名前を設定する
    void setName(const Identifier* pcName_);

    // オプションを設定する
    void setOptionList(const VerifyOptionList* pcList_);

private:
    //代入オペレータは使用しない
    VerifyStatement operator = (const VerifyStatement& object_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_VERIFYSTATEMENT_H

//
//	Copyright (c) 2001, 2002, 2004, 2005, 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
