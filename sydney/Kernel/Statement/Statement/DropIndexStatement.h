// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DropIndexStatement.h --
// 
// Copyright (c) 2000, 2002, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DROPINDEXSTATEMENT_H
#define __SYDNEY_STATEMENT_DROPINDEXSTATEMENT_H

#include "Common/Common.h"
#include "Statement/Object.h"


_SYDNEY_BEGIN

namespace Statement
{

class Identifier;

//
//	CLASS
//	Statement::DropIndexStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION DropIndexStatement : public Statement::Object
{
public:
	//constructor
	DropIndexStatement()
		: Object(ObjectType::DropIndexStatement)
	{}
	//コンストラクタ
	DropIndexStatement(Identifier* pcIdxName_, bool bIfExists_);
	//コンストラクタ(2)
	explicit DropIndexStatement(const DropIndexStatement& cOther_);
	//デストラクタ
	virtual ~DropIndexStatement();

	//
	//	アクセサ
	//

	//Index 名アクセサ
	Identifier* getIndexName() const;
	void setIndexName(Identifier* pcIdxName_);

	bool isIfExists() const;

	//自身をコピーする
	Object* copy() const;

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
protected:
	//代入オペレータは使用しない
	DropIndexStatement& operator=(const DropIndexStatement& cOther_);

	bool m_bIfExists;
};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_DROPINDEXSTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
