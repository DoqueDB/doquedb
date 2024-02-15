// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/SelectSubList.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Query";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Analysis/Query/SelectSubList.h"

#include "Exception/NotSupported.h"
#include "Exception/TableNotFound.h"

#include "Opt/Environment.h"

#include "Plan/Interface/IRelation.h"
#include "Plan/Relation/RowInfo.h"

#include "Schema/Database.h"

#include "Statement/DerivedColumn.h"
#include "Statement/Identifier.h"
#include "Statement/SelectSubList.h"
#include "Statement/SelectSubListList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace SelectSubListImpl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::SelectSubListImpl::Identifier --
	//		select sublist analyzer for identifier
	//
	// NOTES
	class Identifier
		: public Query::SelectSubList
	{
	public:
		typedef Identifier This;
		typedef Query::SelectSubList Super;

		// constructor
		Identifier() {}
		// destructor
		~Identifier() {}

	//////////////////////////////////
	// Analysis::Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
		virtual void addColumns(Opt::Environment& cEnvironment_,
								Plan::Relation::RowInfo* pRowInfo_,
								Plan::Interface::IRelation* pRelation_,
								Statement::Object* pStatement_) const;
	protected:
	private:
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::SelectSubListImpl::Column --
	//		select subList analyzer for derived column
	//
	// NOTES
	class Column
		: public Query::SelectSubList
	{
	public:
		typedef Column This;
		typedef Query::SelectSubList Super;

		// constructor
		Column() {}
		// destructor
		~Column() {}

		//////////////////////////////////
		// Analysis::Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
		virtual void addColumns(Opt::Environment& cEnvironment_,
								Plan::Relation::RowInfo* pRowInfo_,
								Plan::Interface::IRelation* pRelation_,
								Statement::Object* pStatement_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzerXXX -- instance
	//
	// NOTES

	const SelectSubListImpl::Identifier _analyzerIdentifier;
	const SelectSubListImpl::Column _analyzerColumn;

} // namespace

/////////////////////////////////////////////////////
// Query::SelectSubListImpl::Identifier
/////////////////////////////////////////////////////

// FUNCTION public
//	Query::SelectSubListImpl::Identifier::getDegree -- 
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
SelectSubListImpl::Identifier::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::SelectSubList* pSSL =
		_SYDNEY_DYNAMIC_CAST(Statement::SelectSubList*, pStatement_);
	; _SYDNEY_ASSERT(pSSL);

	Statement::Object* pElement = pSSL->getDerivedColumnOrIdentifier();
	; _SYDNEY_ASSERT(pElement->getType() == Statement::ObjectType::Identifier);
	Statement::Identifier* pId = _SYDNEY_DYNAMIC_CAST(Statement::Identifier*, pElement);
	; _SYDNEY_ASSERT(pId);
	const ModUnicodeString* pName = pId->getIdentifier();
	; _SYDNEY_ASSERT(pName);

	// search for relation denoted by the name
	Plan::Interface::IRelation* pRelation = cEnvironment_.getRelation(*pName);
	if (pRelation == 0) {
		_SYDNEY_THROW2(Exception::TableNotFound, *pName, cEnvironment_.getDatabase()->getName());
	}

	return pRelation->getDegree(cEnvironment_);
}

// FUNCTION public
//	Query::SelectSubListImpl::Identifier::addColumns -- add RowInfo::Element from Statement::Object
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
SelectSubListImpl::Identifier::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::SelectSubList* pSSL =
		_SYDNEY_DYNAMIC_CAST(Statement::SelectSubList*, pStatement_);
	; _SYDNEY_ASSERT(pSSL);

	Statement::Object* pElement = pSSL->getDerivedColumnOrIdentifier();
	; _SYDNEY_ASSERT(pElement->getType() == Statement::ObjectType::Identifier);
	Statement::Identifier* pId = _SYDNEY_DYNAMIC_CAST(Statement::Identifier*, pElement);
	; _SYDNEY_ASSERT(pId);
	const ModUnicodeString* pName = pId->getIdentifier();
	; _SYDNEY_ASSERT(pName);

	// search for relation denoted by the name
	Plan::Interface::IRelation* pRelation = cEnvironment_.getRelation(*pName);
	if (pRelation == 0) {
		_SYDNEY_THROW2(Exception::TableNotFound, *pName, cEnvironment_.getDatabase()->getName());
	}	

	// add result row of the relation to the return value
	Plan::Relation::RowInfo* pRowInfo = pRelation->getRowInfo(cEnvironment_);
	pRowInfo_->append(pRowInfo);
}

////////////////////////////////////////////
// Query::SelectSubListImpl::Column
////////////////////////////////////////////

// FUNCTION public
//	Query::SelectSubListImpl::Column::getDegree -- 
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
SelectSubListImpl::Column::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::SelectSubList* pSSL =
		_SYDNEY_DYNAMIC_CAST(Statement::SelectSubList*, pStatement_);
	; _SYDNEY_ASSERT(pSSL);

	Statement::Object* pElement = pSSL->getDerivedColumnOrIdentifier();
	; _SYDNEY_ASSERT(pElement->getType() == Statement::ObjectType::DerivedColumn);
	Statement::DerivedColumn* pDC = _SYDNEY_DYNAMIC_CAST(Statement::DerivedColumn*, pElement);
	; _SYDNEY_ASSERT(pDC);

	return pDC->getAnalyzer2()->getDegree(cEnvironment_,
										  pDC);
}

// FUNCTION public
//	Query::SelectSubListImpl::Column::addColumns -- add RowInfo::Element from Statement::Object
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
SelectSubListImpl::Column::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::SelectSubList* pSSL =
		_SYDNEY_DYNAMIC_CAST(Statement::SelectSubList*, pStatement_);
	; _SYDNEY_ASSERT(pSSL);

	Statement::Object* pElement = pSSL->getDerivedColumnOrIdentifier();
	; _SYDNEY_ASSERT(pElement->getType() == Statement::ObjectType::DerivedColumn);
	Statement::DerivedColumn* pDC = _SYDNEY_DYNAMIC_CAST(Statement::DerivedColumn*, pElement);
	; _SYDNEY_ASSERT(pDC);

	pDC->getAnalyzer2()->addColumns(cEnvironment_,
									pRowInfo_,
									pRelation_,
									pDC);
}

///////////////////////////////////////
// Query::SelectSubList
///////////////////////////////////////

// FUNCTION public
//	Query::SelectSubList::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::SelectSubList* pStatement_
//	
// RETURN
//	const SelectSubList*
//
// EXCEPTIONS

//static
const SelectSubList*
SelectSubList::
create(const Statement::SelectSubList* pStatement_)
{
	switch (pStatement_->getDerivedColumnOrIdentifier()->getType()) {
	case Statement::ObjectType::Identifier:
		{
			return &_analyzerIdentifier;
		}
	case Statement::ObjectType::DerivedColumn:
		{
			return &_analyzerColumn;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
