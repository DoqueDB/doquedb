// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Base.h --
// 
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ANALYSIS_PREDICATE_BASE_H
#define __SYDNEY_ANALYSIS_PREDICATE_BASE_H

#include "Analysis/Predicate/Module.h"
#include "Analysis/Value/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

////////////////////////////////////////////////////////////////////////////
// CLASS
//	Predicate::Base -- base class of value expression analyzer
//
// NOTES
class Base
	: public Value::ValueExpression
{
public:
	typedef Base This;
	typedef Value::ValueExpression Super;

	// destructor
	virtual ~Base() {}

///////////////////////////
// Interface::IAnalyzer
	virtual Plan::Interface::IRelation*
					getFilter(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;


	virtual Plan::Interface::IRelation*
	getDistributeFilter(Opt::Environment& cEnvironment_,
						Plan::Interface::IRelation* pRelation_,
						Statement::Object* pStatement_) const;
	
protected:
	// constructor
	Base() {}
private:
};

_SYDNEY_ANALYSIS_PREDICATE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

#endif // __SYDNEY_ANALYSIS_PREDICATE_BASE_H

//
//	Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
