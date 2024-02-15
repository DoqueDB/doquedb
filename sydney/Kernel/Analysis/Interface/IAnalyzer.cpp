// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Analyzer.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Analysis::Interface";
}

#include "SyDefault.h"
#include "Analysis/Interface/IAnalyzer.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_ANALYSIS_USING
_SYDNEY_ANALYSIS_INTERFACE_USING

// FUNCTION public
//	Interface::IAnalyzer::getRelation -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
IAnalyzer::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IAnalyzer::getFilter -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
IAnalyzer::
getFilter(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IAnalyzer::getScalar -- generate Plan::Interface::IScalar from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

//virtual
Plan::Interface::IScalar*
IAnalyzer::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IAnalyzer::getRowElement -- generate Plan::Relation::RowElement from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Relation::RowElement*
//
// EXCEPTIONS

//virtual
Plan::Relation::RowElement*
IAnalyzer::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IAnalyzer::getDegree -- get element numbers added by following methods
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
IAnalyzer::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IAnalyzer::addScalar -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	VECTOR<Plan::Interface::IScalar*>& vecScalar_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
IAnalyzer::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IAnalyzer::addColumns -- add RowInfo::Element from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::RowInfo* pRowInfo_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
IAnalyzer::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IAnalyzer::getPredicate -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
IAnalyzer::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IAnalyzer::getDistributeRelation -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
IAnalyzer::
getDistributeRelation(Opt::Environment& cEnvironment_,
					  Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IAnalyzer::getDistributeFilter -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
IAnalyzer::
getDistributeFilter(Opt::Environment& cEnvironment_,
					Plan::Interface::IRelation* pRelation_,
					Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
