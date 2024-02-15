// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ItemReference -- ItemReference
// 
// Copyright (c) 1999, 2002, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_ITEMREFERENCE_H
#define __SYDNEY_STATEMENT_ITEMREFERENCE_H

#include "Statement/Object.h"

class ModUnicodeString;

_SYDNEY_BEGIN

namespace Statement
{
	class Identifier;
	
//
//	CLASS
//		ItemReference -- ItemReference
//
//	NOTES
//		ItemReference
//
class SYD_STATEMENT_FUNCTION ItemReference : public Statement::Object
{
public:
	//constructor
	ItemReference()
		: Object(ObjectType::ItemReference)
	{}
	// コンストラクタ (2)
	ItemReference(Identifier* pItemQualifier_, Identifier* pItemName_);

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// アクセサ
	// ItemQualifier を得る
	Identifier* getItemQualifier() const;
	// ItemQualifier を設定する
	void setItemQualifier(Identifier* pItemQualifier_);
	// ItemQualifier を ModString で得る
	const ModUnicodeString* getItemQualifierString() const;

	// ItemName を得る
	Identifier* getItemName() const;
	// ItemName を設定する
	void setItemName(Identifier* pItemName_);
	// ItemName を ModString で得る
	const ModUnicodeString* getItemNameString() const;

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	ItemReference& operator=(const ItemReference& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_ITEMREFERENCE_H

//
// Copyright (c) 1999, 2002, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
