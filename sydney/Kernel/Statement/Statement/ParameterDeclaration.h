// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ParameterDeclaration -- ParameterDeclaration
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_PARAMETERDECLARATION_H
#define __SYDNEY_STATEMENT_PARAMETERDECLARATION_H

#include "Statement/Object.h"
#include "Common/SQLData.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Identifier;
	
//
//	CLASS
//		ParameterDeclaration -- ParameterDeclaration
//
//	NOTES
//		ParameterDeclaration
//
class SYD_STATEMENT_FUNCTION ParameterDeclaration : public Statement::Object
{
public:
	//constructor
	ParameterDeclaration()
		: Object(ObjectType::ParameterDeclaration)
	{}
	// コンストラクタ (2)
	ParameterDeclaration(Identifier* pName_, const Common::SQLData& cType_);

	// アクセサ
	// ParameterNameを得る
	Identifier* getParameterName() const;
	// ParameterNameを設定する
	void setParameterName(Identifier* pName_);

	// DataType を得る
	const Common::SQLData& getParameterType() const;
	// DataType を設定する
	void setParameterType(const Common::SQLData& cType_);

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	//自身をコピーする
	Object* copy() const;

	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
private:
	// 代入オペレーターは使わない
	ParameterDeclaration& operator=(const ParameterDeclaration& cOther_);

	// メンバ変数
	Common::SQLData m_cType;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_PARAMETERDECLARATION_H

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
