// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/TableReferenceList.cpp --
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
const char moduleName[] = "Analysis::Query";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Query/TableReferenceList.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

#include "Plan/Relation/Join.h"


#include "Statement/TableReferenceList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace TableReferenceListImpl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::TableReferenceListImpl::Single -- table reference list analyzer
	//
	// NOTES
	class Single
		: public Query::TableReferenceList
	{
	public:
		typedef Single This;
		typedef Query::TableReferenceList Super;

		// constructor
		Single() : Super() {}
		// destructor
		~Single() {}

	/////////////////////////////
	//Interface::Analyzer::
		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
		virtual Plan::Interface::IRelation*
				getDistributeRelation(Opt::Environment& cEnvironment_,
									  Statement::Object* pStatement_) const;
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::TableReferenceListImpl::Double -- table reference list analyzer
	//
	// NOTES
	class Double
		: public Query::TableReferenceList
	{
	public:
		typedef Double This;
		typedef Query::TableReferenceList Super;

		// constructor
		Double() : Super() {}
		// destructor
		~Double() {}

	/////////////////////////////
	//Interface::Analyzer::
		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
		virtual Plan::Interface::IRelation*
				getDistributeRelation(Opt::Environment& cEnvironment_,
									  Statement::Object* pStatement_) const;
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::TableReferenceListImpl::Multi -- table reference list analyzer
	//
	// NOTES
	class Multi
		: public Query::TableReferenceList
	{
	public:
		typedef Multi This;
		typedef Query::TableReferenceList Super;

		// constructor
		Multi() : Super() {}
		// destructor
		~Multi() {}

	/////////////////////////////
	//Interface::Analyzer::
		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
		virtual Plan::Interface::IRelation*
				getDistributeRelation(Opt::Environment& cEnvironment_,
									  Statement::Object* pStatement_) const;
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	_analyzerXXX -- instance
	//
	// NOTES
	const TableReferenceListImpl::Single _analyzerSingle;
	const TableReferenceListImpl::Double _analyzerDouble;
	const TableReferenceListImpl::Multi _analyzerMulti;
}

////////////////////////////////////////////////
// Query::TableReferenceListImpl::Single
////////////////////////////////////////////////

// FUNCTION public
//	Query::TableReferenceListImpl::Single::getRelation --
//		generate Plan::Tree::Node from Statement::Object
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
TableReferenceListImpl::Single::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::TableReferenceList* pTRL =
		_SYDNEY_DYNAMIC_CAST(Statement::TableReferenceList*, pStatement_);
	; _SYDNEY_ASSERT(pTRL);
	; _SYDNEY_ASSERT(pTRL->getCount() == 1);

	// one table
	return pTRL->getAt(0)->getAnalyzer2()->getRelation(cEnvironment_,
													   pTRL->getAt(0));
}

// FUNCTION public
//	Query::TableReferenceListImpl::Single::getDistributeRelation --
//		generate Plan::Tree::Node from Statement::Object
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
TableReferenceListImpl::Single::
getDistributeRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

// FUNCTION public
//	Query::TableReferenceListImpl::Single::getDegree -- 
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
TableReferenceListImpl::Single::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::TableReferenceList* pTRL =
		_SYDNEY_DYNAMIC_CAST(Statement::TableReferenceList*, pStatement_);
	; _SYDNEY_ASSERT(pTRL);
	; _SYDNEY_ASSERT(pTRL->getCount() == 1);

	// one table
	return pTRL->getAt(0)->getAnalyzer2()->getDegree(cEnvironment_,
													 pTRL->getAt(0));
}

//////////////////////////////////////////////////
// Query::TableReferenceListImpl::Double
//////////////////////////////////////////////////

// FUNCTION public
//	Query::TableReferenceListImpl::Double::getRelation -- generate Plan::Tree::Node from Statement::Object
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
TableReferenceListImpl::Double::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::TableReferenceList* pTRL =
		_SYDNEY_DYNAMIC_CAST(Statement::TableReferenceList*, pStatement_);
	; _SYDNEY_ASSERT(pTRL);
	; _SYDNEY_ASSERT(pTRL->getCount() == 2);

	// two tables
	PAIR<Plan::Interface::IRelation*, Plan::Interface::IRelation*> cOperand;
	cOperand.first =
		pTRL->getAt(0)->getAnalyzer2()->getRelation(cEnvironment_,
													pTRL->getAt(0));
	cOperand.second =
		pTRL->getAt(1)->getAnalyzer2()->getRelation(cEnvironment_,
													pTRL->getAt(1));


	return Plan::Relation::Join::create(cEnvironment_,
										Plan::Interface::IRelation::SimpleJoin, 
										0, /* no join predicates */
										cOperand);
}

// FUNCTION public
//	Query::TableReferenceListImpl::Double::getDistributeRelation -- 
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
TableReferenceListImpl::Double::
getDistributeRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

// FUNCTION public
//	Query::TableReferenceListImpl::Double::getDegree -- 
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
TableReferenceListImpl::Double::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::TableReferenceList* pTRL =
		_SYDNEY_DYNAMIC_CAST(Statement::TableReferenceList*, pStatement_);
	; _SYDNEY_ASSERT(pTRL);
	; _SYDNEY_ASSERT(pTRL->getCount() == 2);

	// two tables
	return pTRL->getAt(0)->getAnalyzer2()->getDegree(cEnvironment_,
													 pTRL->getAt(0))
		+ pTRL->getAt(1)->getAnalyzer2()->getDegree(cEnvironment_,
													pTRL->getAt(1));
}

//////////////////////////////////////////////////
// Query::TableReferenceListImpl::Multi
//////////////////////////////////////////////////

// FUNCTION public
//	Query::TableReferenceListImpl::Multi::getRelation -- generate Plan::Tree::Node from Statement::Object
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
TableReferenceListImpl::Multi::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::TableReferenceList* pTRL =
		_SYDNEY_DYNAMIC_CAST(Statement::TableReferenceList*, pStatement_);
	; _SYDNEY_ASSERT(pTRL);

	int n = pTRL->getCount();
	; _SYDNEY_ASSERT(n);

	// more than two tables
	VECTOR<Plan::Interface::IRelation*> vecOperand;
	vecOperand.reserve(n);
	int i = 0;
	do {
		vecOperand.PUSHBACK(pTRL->getAt(i)->getAnalyzer2()->getRelation(
												cEnvironment_,
												pTRL->getAt(i)));
	} while (++i < n);

	return Plan::Relation::Join::create(cEnvironment_,
										Plan::Interface::IRelation::SimpleJoin,
										0, /* no join predicates */
										vecOperand);	
}

// FUNCTION public
//	Query::TableReferenceListImpl::Multi::getDistributeRelation -- 
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
TableReferenceListImpl::Multi::
getDistributeRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

// FUNCTION public
//	Query::TableReferenceListImpl::Multi::getDegree -- 
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
TableReferenceListImpl::Multi::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::TableReferenceList* pTRL =
		_SYDNEY_DYNAMIC_CAST(Statement::TableReferenceList*, pStatement_);
	; _SYDNEY_ASSERT(pTRL);

	int n = pTRL->getCount();
	; _SYDNEY_ASSERT(n);

	int iResult = 0;
	int i = 0;
	do {
		iResult += pTRL->getAt(i)->getAnalyzer2()->getDegree(cEnvironment_,
															 pTRL->getAt(i));
	} while (++i < n);
	return iResult;
}

///////////////////////////////
// Query::TableReferenceList
///////////////////////////////

// FUNCTION public
//	Query::TableReferenceList::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::TableReferenceList* pStatement_
//	
// RETURN
//	const TableReferenceList*
//
// EXCEPTIONS

//static
const TableReferenceList*
TableReferenceList::
create(const Statement::TableReferenceList* pStatement_)
{
	switch (pStatement_->getCount()) {
	case 1:
		{
			return &_analyzerSingle;
		}
	case 2:
		{
			return &_analyzerDouble;
		}
	default:
		{
			return &_analyzerMulti;
		}
	}
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
