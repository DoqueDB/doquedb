// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DropAreaStatement.h --
// 
// Copyright (c) 2000, 2002, 2006, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DROPAREASTATEMENT_H
#define __SYDNEY_STATEMENT_DROPAREASTATEMENT_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{

class Identifier;

//
//	CLASS
//	Statement::DropAreaStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION DropAreaStatement  : public Statement::Object
{
public:
	//constructor
	DropAreaStatement()
		: Object(ObjectType::DropAreaStatement)
	{}
	// コンストラクタ
	DropAreaStatement( Identifier* pName_, bool bIfExists_ );
	// コンストラクタ (2)
	explicit DropAreaStatement( const DropAreaStatement& cOther_ );
	//デストラクタ
	virtual ~DropAreaStatement();

	// アクセサ
	// Name を得る
	Identifier* getName() const;
	// Name を設定する
	void setName(Identifier* pName_);
	// Name を ModString で得る
	const ModUnicodeString* getNameString() const;

	// if exists?
	bool isIfExists() const;

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
private:
	// 代入オペレーターは使わない
	DropAreaStatement& operator=(const DropAreaStatement& cOther_);

	bool m_bIfExists;
};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_DROPAREASTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2006, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
