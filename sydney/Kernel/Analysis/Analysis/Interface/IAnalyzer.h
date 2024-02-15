// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IAnalyzer.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ANALYSIS_INTERFACE_IANALYZER_H
#define __SYDNEY_ANALYSIS_INTERFACE_IANALYZER_H

#include "Analysis/Interface/Module.h"

#include "Common/Object.h"

#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"

#include "Plan/Declaration.h"

_SYDNEY_BEGIN
namespace Statement
{
	class Object;
}

_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_INTERFACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
// CLASS
//	Analysis::Interface::IAnalyzer -- Base class for analyzing statement node
//
// NOTES
class IAnalyzer
	: public Common::Object
{
public:
	// constructor
	IAnalyzer() {}
	// destructor
	virtual ~IAnalyzer() {}

	// generate Plan::Tree::Node from Statement::Object
	virtual Plan::Interface::IRelation*
					getRelation(Opt::Environment& cEnvironment_,
								Statement::Object* pStatement_) const;
	// generate Plan::Tree::Node from Statement::Object
	virtual Plan::Interface::IRelation*
					getFilter(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	// generate Plan::Tree::Node from Statement::Object
	virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	// generate Plan::Tree::Node from Statement::Object
	virtual Plan::Relation::RowElement*
					getRowElement(Opt::Environment& cEnvironment_,
								  Plan::Interface::IRelation* pRelation_,
								  Statement::Object* pStatement_) const;
	// generate Plan::Tree::Node from Statement::Object
	virtual Plan::Interface::IPredicate*
					getPredicate(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	////////////////////////////////

	// get element numbers added by following methods
	virtual int getDegree(Opt::Environment& cEnvironment_,
						  Statement::Object* pStatement_) const;	

	// generate Plan::Tree::Node from Statement::Object
	virtual void addScalar(Opt::Environment& cEnvironment_,
						   Plan::Interface::IRelation* pRelation_,
						   Statement::Object* pStatement_,
						   VECTOR<Plan::Interface::IScalar*>& vecScalar_) const;
	// add RowInfo::Element from Statement::Object
	virtual void addColumns(Opt::Environment& cEnvironment_,
							Plan::Relation::RowInfo* pRowInfo_,
							Plan::Interface::IRelation* pRelation_,
							Statement::Object* pStatement_) const;

	//////////////////////////////////////

	// generate Plan::Tree::Node from Statement::Object
	virtual Plan::Interface::IRelation*
					getDistributeRelation(Opt::Environment& cEnvironment_,
										  Statement::Object* pStatement_) const;
	// generate Plan::Tree::Node from Statement::Object
	virtual Plan::Interface::IRelation*
					getDistributeFilter(Opt::Environment& cEnvironment_,
										Plan::Interface::IRelation* pRelation_,
										Statement::Object* pStatement_) const;

protected:
private:
};

_SYDNEY_ANALYSIS_INTERFACE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

#endif // __SYDNEY_ANALYSIS_INTERFACE_IANALYZER_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
