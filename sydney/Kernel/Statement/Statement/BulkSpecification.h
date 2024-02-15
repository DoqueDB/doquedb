// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::BulkSpecification -- BulkSpecification
// 
// Copyright (c) 2006, 2007, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_BULKSPECIFICATION_H
#define __SYDNEY_STATEMENT_BULKSPECIFICATION_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ValueExpression;

//
//	CLASS
//		BulkSpecification -- BulkSpecification
//
//	NOTES
//		BulkSpecification
//
class SYD_STATEMENT_FUNCTION BulkSpecification : public Statement::Object
{
public:
	//constructor
	BulkSpecification()
		: Object(ObjectType::BulkSpecification)
	{}
	// コンストラクタ
	BulkSpecification(ValueExpression* pInputData_,
					  ValueExpression* pWith_,
					  ValueExpression* pHint_);

	// アクセサ
	// InputData を得る
	ValueExpression* getInputData() const;
	// InputData を設定する
	void setInputData(ValueExpression* pInputData_);

	// With を得る
	ValueExpression* getWith() const;
	// With を設定する
	void setWith(ValueExpression* pWith_);

	// Hint を得る
	ValueExpression* getHint() const;
	// Hint を設定する
	void setHint(ValueExpression* pHint_);

	// 入力用かを示すフラグを得る
	bool isInput() const;
	// 入力用かを示すフラグを設定する
	void setInput(bool bInput_);

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
private:
	// 代入オペレーターは使わない
	BulkSpecification& operator=(const BulkSpecification& cOther_);

	// メンバ変数
	bool m_bInput;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_BULKSPECIFICATION_H

//
// Copyright (c) 2006, 2007, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
