// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExplainStatement.h --
// 
// Copyright (c) 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_EXPLAINSTATEMENT_H
#define __SYDNEY_STATEMENT_EXPLAINSTATEMENT_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ExplainOption;

//
//	CLASS
//	Statement::ExplainStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION ExplainStatement
	: public Object
{
public:
	//constructor
	ExplainStatement()
		: Object(ObjectType::ExplainStatement)
	{}
	//constructors
	ExplainStatement(Object* pStatement_, ExplainOption* pOption_);

	//destructor
	virtual ~ExplainStatement();

	// accessor
	Object* getStatement() const;
	ExplainOption* getOption() const;

	void setStatement(Object* pStatement_);
	void setOption(ExplainOption* pOption_);
    
	//copy
	Object* copy() const;

	// get SQL statement
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

protected:
private:
	// assignment operator is not used
	ExplainStatement& operator=(const ExplainStatement& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_EXPLAINSTATEMENT_H

//
//	Copyright (c) 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
