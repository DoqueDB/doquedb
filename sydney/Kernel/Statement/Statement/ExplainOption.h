// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExplainOption.h --
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

#ifndef __SYDNEY_STATEMENT_EXPLAINOPTION_H
#define __SYDNEY_STATEMENT_EXPLAINOPTION_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Hint;

//
//	CLASS
//	Statement::ExplainOption --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION ExplainOption
	: public Statement::Object
{
public:
	// Value held by this class
	typedef unsigned int Value;
	enum {
		None		= 0,
		Execute		= 1,
	};

	//constructor
	ExplainOption()
		: Object(ObjectType::ExplainOption)
	{}
	//constructors
	ExplainOption(Value iOption_, Hint* pHint_);
	ExplainOption(const ExplainOption& cOption_);

	//destructor
	virtual ~ExplainOption();

	// accessor
	Value getOption() const {return m_iOption;}
	Hint* getHint() const;

	void setOption(Value iOption_) {m_iOption = iOption_;}
	void setHint(Hint* pHint_);
    
	//copy
	Object* copy() const;

	// get SQL statement
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
private:
	// assignment operator is not used
	ExplainOption& operator=(const ExplainOption& cOther_);

	Value m_iOption;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_EXPLAINOPTION_H

//
//	Copyright (c) 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
