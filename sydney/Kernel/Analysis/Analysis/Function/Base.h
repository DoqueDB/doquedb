// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Base.h --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ANALYSIS_FUNCTION_BASE_H
#define __SYDNEY_ANALYSIS_FUNCTION_BASE_H

#include "Analysis/Value/ValueExpression.h"
#include "Analysis/Function/Module.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

////////////////////////////////////////////////////////////////////////////
// CLASS
//	Function::Base -- base class of value expression analyzer for functions
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
// Interface::IAnalyzer::
	// generate Scalar from Statement::Object
	virtual Plan::Interface::IScalar*
				getScalar(Opt::Environment& cEnvironment_,
						  Plan::Interface::IRelation* pRelation_,
						  Statement::Object* pStatement_) const;
	// generate RowElement from Statement::Object
	virtual Plan::Relation::RowElement*
				getRowElement(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	// get element numbers added by following methods
	virtual int getDegree(Opt::Environment& cEnvironment_,
						  Statement::Object* pStatement_) const;	
	// generate Plan::Tree::Node from Statement::Object
	virtual void addScalar(Opt::Environment& cEnvironment_,
						   Plan::Interface::IRelation* pRelation_,
						   Statement::Object* pStatement_,
						   VECTOR<Plan::Interface::IScalar*>& vecScalar_) const;
protected:
	// constructor
	Base() : Super() {}

private:
	// create Scalar instance
	virtual Plan::Interface::IScalar*
				createScalar(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Statement::Object* pStatement_) const = 0;

};

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

#endif // __SYDNEY_ANALYSIS_FUNCTION_BASE_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
