// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/QualifiedJoin.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Analysis/Query/QualifiedJoin.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Exception/CommonColumnNotFound.h"
#include "Exception/DuplicateCommonColumn.h"
#include "Exception/NotSupported.h"

#include "Opt/Algorithm.h"
#include "Opt/Environment.h"
#include "Opt/NameMap.h"

#include "Plan/Predicate/Combinator.h"
#include "Plan/Predicate/Comparison.h"
#include "Plan/Relation/Join.h"
#include "Plan/Relation/Projection.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/RowElement.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Utility/ObjectSet.h"

#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/JoinType.h"
#include "Statement/QualifiedJoin.h"
#include "Statement/Type.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace
{
	// jointype -> treenode type
	Plan::Tree::Node::Type _cNodeTypeTable[] =
	{
		Plan::Interface::IRelation::SimpleJoin,		// Inner
		Plan::Interface::IRelation::LeftOuterJoin,	// Left
		Plan::Interface::IRelation::RightOuterJoin, // Right
		Plan::Interface::IRelation::FullOuterJoin,	// Full
		Plan::Interface::IRelation::Exists			// Exists
	};

	/////////////////////////////
	// CLASS local
	//	$$$::_CommonColumnMap -- common column creator
	//
	// NOTES
	class _CommonColumnMap
	{
	public:
		typedef _CommonColumnMap This;

		// type of vector holding common column pair
		typedef PAIR<STRING,
					 PAIR<Plan::Relation::RowElement*,
						  Plan::Relation::RowElement*> > VectorElement;
		typedef VECTOR<VectorElement> Vector;
		// type of map holding name->position in vector relationship
		typedef PAIR<int, Plan::Relation::RowElement*> NameMapValue;
		typedef MAP<STRING, NameMapValue, Opt::CaseInsensitiveComparator> NameMap;

		// constructor
		_CommonColumnMap()
			: m_mapName(),
			  m_vecElement()
		{}
		// destructor
		~_CommonColumnMap() {}

		// add required name from column name list
		bool addName(const STRING& cstrName_);
		// add name map from rowinfo elements
		bool addName(Opt::Environment& cEnvironment_,
					 Plan::Relation::RowInfo::Element& cElement_);
		// check one column for name
		bool checkColumnFirst(Opt::Environment& cEnvironment_,
							  Plan::Relation::RowInfo::Element& cElement_);
		bool checkColumnSecond(Opt::Environment& cEnvironment_,
							   Plan::Relation::RowInfo::Element& cElement_);

		Vector& getVector() {return m_vecElement;}

		// check error status
		void checkError()
		{
			Opt::ForEach(m_mapName,
						 boost::bind(&This::checkMapValue,
									 this,
									 _1));
		}
		// ignore error status
		void ignoreError();

	protected:
	private:
		struct Value {
			enum {
				Initial = -1,
				Invalid = -2
			};
		};

		const STRING& getColumnName(Opt::Environment& cEnvironment_,
									Plan::Relation::RowInfo::Element& cElement_);
		int getPosition(const NameMapValue& cValue_);
		void setPosition(NameMapValue& cValue_,
						 int iPosition_);
		Plan::Relation::RowElement* getFirstElement(int iPosition_);
		Plan::Relation::RowElement* getSecondElement(int iPosition_);
		void setFirstElement(int iPosition_,
							 Plan::Relation::RowElement* pElement_);
		void setSecondElement(int iPosition_,
							  Plan::Relation::RowElement* pElement_);
		void checkMapValue(const PAIR<STRING, NameMapValue>& cEntry_);

		NameMap m_mapName;
		Vector m_vecElement;
	};
}

namespace QualifiedJoinImpl
{
	/////////////////////////////////////////////
	// CLASS
	//	Query::Impl::QualifiedJoinImpl::Base -- base class for qualified join analyzer
	//
	// NOTES
	class Base
		: public Query::QualifiedJoin
	{
	public:
		typedef Base This;
		typedef Query::QualifiedJoin Super;

		// constructor
		Base() : Super() {}
		// destructor
		virtual ~Base() {}

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
		// create join relation
		virtual Plan::Interface::IRelation*
				createJoin(Opt::Environment& cEnvironment_,
						   Plan::Tree::Node::Type eNodeType_,
						   Plan::Interface::IRelation* pOperand0_,
						   Plan::Interface::IRelation* pOperand1_,
						   Statement::Object* pStatement_) const = 0;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::QualifiedJoinImpl::Condition -- QualifiedJoin by condition
	//
	// NOTES
	class Condition
		: public Base
	{
	public:
		typedef Condition This;
		typedef Base Super;

		// constructor
		Condition() : Super() {}
		// destructor
		~Condition() {}

	protected:
	private:
	//////////////////////////////
	// Base::
		virtual Plan::Interface::IRelation*
				createJoin(Opt::Environment& cEnvironment_,
						   Plan::Tree::Node::Type eNodeType_,
						   Plan::Interface::IRelation* pOperand0_,
						   Plan::Interface::IRelation* pOperand1_,
						   Statement::Object* pStatement_) const;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::QualifiedJoinImpl::ColumnList -- QualifiedJoin by column list
	//
	// NOTES
	class ColumnList
		: public Base
	{
	public:
		typedef ColumnList This;
		typedef Base Super;

		// constructor
		ColumnList() : Super() {}
		// destructor
		virtual ~ColumnList() {}

		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;				

	protected:
		void addCommonColumn(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pJoin_,
							 Plan::Relation::RowInfo* pRowInfo_,
							 VECTOR<Plan::Interface::IPredicate*>& vecCondition_,
							 const PAIR<Plan::Relation::RowElement*,
							 			Plan::Relation::RowElement*>& cCommonElement_,
							 Plan::Utility::RowElementSet& cCommonColumn0_,
							 Plan::Utility::RowElementSet& cCommonColumn1_,
							 const STRING& cstrName_) const;
	private:
		// create result row from a operand
		void addRowInfo(Opt::Environment& cEnvironment_,
						Plan::Interface::IRelation* pOperand_,
						const Plan::Utility::RowElementSet& cCommonColumn_,
						Plan::Relation::RowInfo* pRowInfo_) const;
		// add created rowelements to namemap
		void addNameMap(Opt::Environment& cEnvironment_,
						Plan::Interface::IRelation* pJoin_,
						Plan::Relation::RowInfo* pRowInfo_) const;

		// create common columns
		virtual void createCommonColumn(Opt::Environment& cEnvironment_,
										Plan::Interface::IRelation* pOperand0_,
										Plan::Interface::IRelation* pOperand1_,
										Plan::Interface::IRelation* pJoin_,
										Plan::Relation::RowInfo* pRowInfo_,
										VECTOR<Plan::Interface::IPredicate*>& vecCondition_,
										Plan::Utility::RowElementSet& cCommonColumn0_,
										Plan::Utility::RowElementSet& cCommonColumn1_,
										Statement::ColumnNameList* pCNL_) const;
	//////////////////////////////
	// Base::
		virtual Plan::Interface::IRelation*
				createJoin(Opt::Environment& cEnvironment_,
						   Plan::Tree::Node::Type eNodeType_,
						   Plan::Interface::IRelation* pOperand0_,
						   Plan::Interface::IRelation* pOperand1_,
						   Statement::Object* pStatement_) const;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::QualifiedJoinImpl::Natural -- QualifiedJoin for natural join
	//
	// NOTES
	class Natural
		: public ColumnList
	{
	public:
		typedef Natural This;
		typedef ColumnList Super;

		// constructor
		Natural() : Super() {}
		// destructor
		~Natural() {}


	protected:
	private:
	//////////////////////////////////////
	// QualifiedJoinImpl::ColumnList::
		virtual void createCommonColumn(Opt::Environment& cEnvironment_,
										Plan::Interface::IRelation* pOperand0_,
										Plan::Interface::IRelation* pOperand1_,
										Plan::Interface::IRelation* pJoin_,
										Plan::Relation::RowInfo* pRowInfo_,
										VECTOR<Plan::Interface::IPredicate*>& vecCondition_,
										Plan::Utility::RowElementSet& cCommonColumn0_,
										Plan::Utility::RowElementSet& cCommonColumn1_,
										Statement::ColumnNameList* pCNL_) const;
	//////////////////////////////
	// Base::
	//	virtual Plan::Interface::IRelation*
	//			createJoin(Opt::Environment& cEnvironment_,
	//					   Plan::Tree::Node::Type eNodeType_,
	//					   Plan::Interface::IRelation* pOperand0_,
	//					   Plan::Interface::IRelation* pOperand1_,
	//					   Statement::Object* pStatement_) const;
	};
}

namespace
{
	// VARIABLE local
	//	_analyzerXXX -- instance
	//
	// NOTES
	const QualifiedJoinImpl::Condition _analyzerCondition;
	const QualifiedJoinImpl::ColumnList _analyzerColumn;
	const QualifiedJoinImpl::Natural _analyzerNatural;

} //namespace

///////////////////////////////////
// $$$::_CommonColumnMap
///////////////////////////////////

// FUNCTION public
//	$$$::_CommonColumnMap::addName -- add required name from column name list
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrName_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
_CommonColumnMap::
addName(const STRING& cstrName_)
{
	NameMap::ITERATOR found = m_mapName.find(cstrName_);
	if (found == m_mapName.end()) {
		// set initial value
		m_mapName[cstrName_] = MAKEPAIR(static_cast<int>(Value::Initial),
										static_cast<Plan::Relation::RowElement*>(0));
		return true;
	}
	// same name has been already set
	return false;
}

// FUNCTION public
//	$$$::_CommonColumnMap::addName -- add name map from rowinfo elements
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::RowInfo::Element& cElement_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
_CommonColumnMap::
addName(Opt::Environment& cEnvironment_,
		Plan::Relation::RowInfo::Element& cElement_)
{
	const STRING& cstrName = getColumnName(cEnvironment_, cElement_);
	NameMap::ITERATOR found = m_mapName.find(cstrName);
	if (found == m_mapName.end()) {
		// set initial value
		m_mapName[cstrName] = MAKEPAIR(static_cast<int>(Value::Initial),
									   cElement_.second);
		return true;
	}
	// same name has been already set
	// -> this name can't be used as common column
	setPosition((*found).second, Value::Invalid);
	return false;
}

// FUNCTION public
//	$$$::_CommonColumnMap::checkColumnFirst -- check one column for name
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::RowInfo::Element& cElement_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
_CommonColumnMap::
checkColumnFirst(Opt::Environment& cEnvironment_,
				 Plan::Relation::RowInfo::Element& cElement_)
{
	const STRING& cstrName = getColumnName(cEnvironment_, cElement_);
	NameMap::ITERATOR found = m_mapName.find(cstrName);
	if (found != m_mapName.end()) {
		// specified name found
		NameMapValue& cNameMapValue = (*found).second;
		int iPosition = cNameMapValue.first;
		Plan::Relation::RowElement* pPresetElement = cNameMapValue.second;
		if (iPosition != Value::Invalid) {
			if (iPosition == Value::Initial) {
				// add new common column
				setPosition(cNameMapValue, iPosition = m_vecElement.GETSIZE());
				m_vecElement.PUSHBACK(MAKEPAIR(cstrName,
											   MAKEPAIR(static_cast<Plan::Relation::RowElement*>(0),
														pPresetElement)));
			}
			if (getFirstElement(iPosition) != 0) {
				// another element has been found
				setFirstElement(iPosition, 0);
				setPosition(cNameMapValue, Value::Invalid);
			} else {
				setFirstElement(iPosition, cElement_.second);
				return true;
			}
		}
		return false;
	}
	// when no matching name for the column, result is true
	return true;
}

// FUNCTION public
//	$$$::_CommonColumnMap::checkColumnSecond -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::RowInfo::Element& cElement_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
_CommonColumnMap::
checkColumnSecond(Opt::Environment& cEnvironment_,
				  Plan::Relation::RowInfo::Element& cElement_)
{
	const STRING& cstrName = getColumnName(cEnvironment_, cElement_);
	NameMap::ITERATOR found = m_mapName.find(cstrName);
	if (found != m_mapName.end()) {
		// specified name found
		NameMapValue& cNameMapValue = (*found).second;
		int iPosition = cNameMapValue.first;
		Plan::Relation::RowElement* pPresetElement = cNameMapValue.second;
		if (iPosition != Value::Invalid) {
			if (iPosition == Value::Initial) {
				// initial status is not allowed
				;
			} else if (getSecondElement(iPosition) != 0) {
				// another element has been found
				setPosition(cNameMapValue, Value::Invalid);
			} else {
				setSecondElement(iPosition, cElement_.second);
				return true;
			}
		}
		return false;
	}
	return true;
}

// FUNCTION public
//	$$$::_CommonColumnMap::ignoreError -- ignore error status
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_CommonColumnMap::
ignoreError()
{
	Vector::ITERATOR iterator = m_vecElement.begin();
	while (iterator != m_vecElement.end()) {
		if ((*iterator).second.first == 0
			|| (*iterator).second.second == 0) {
			iterator = m_vecElement.erase(iterator);
		} else {
			++iterator;
		}
	}
}

// FUNCTION private
//	$$$::_CommonColumnMap::getColumnName -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::RowInfo::Element& cElement_
//	
// RETURN
//	const STRING&
//
// EXCEPTIONS

const STRING&
_CommonColumnMap::
getColumnName(Opt::Environment& cEnvironment_,
			  Plan::Relation::RowInfo::Element& cElement_)
{
	return cEnvironment_.getAliasName(cElement_.second);
}

// FUNCTION private
//	$$$::_CommonColumnMap::getPosition -- 
//
// NOTES
//
// ARGUMENTS
//	const NameMapValue& cValue_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
_CommonColumnMap::
getPosition(const NameMapValue& cValue_)
{
	return cValue_.first;
}

// FUNCTION private
//	$$$::_CommonColumnMap::setPosition -- 
//
// NOTES
//
// ARGUMENTS
//	NameMapValue& cValue_
//	int iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_CommonColumnMap::
setPosition(NameMapValue& cValue_,
			int iPosition_)
{
	cValue_.first = iPosition_;
}

// FUNCTION private
//	$$$::_CommonColumnMap::getFirstElement -- 
//
// NOTES
//
// ARGUMENTS
//	int iPosition_
//	
// RETURN
//	Plan::Relation::RowElement*
//
// EXCEPTIONS

Plan::Relation::RowElement*
_CommonColumnMap::
getFirstElement(int iPosition_)
{
	return m_vecElement[iPosition_].second.first;
}

// FUNCTION private
//	$$$::_CommonColumnMap::getSecondElement -- 
//
// NOTES
//
// ARGUMENTS
//	int iPosition_
//	
// RETURN
//	Plan::Relation::RowElement*
//
// EXCEPTIONS

Plan::Relation::RowElement*
_CommonColumnMap::
getSecondElement(int iPosition_)
{
	return m_vecElement[iPosition_].second.second;
}

// FUNCTION private
//	$$$::_CommonColumnMap::setFirstElement -- 
//
// NOTES
//
// ARGUMENTS
//	int iPosition_
//	Plan::Relation::RowElement* pElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_CommonColumnMap::
setFirstElement(int iPosition_,
				Plan::Relation::RowElement* pElement_)
{
	m_vecElement[iPosition_].second.first = pElement_;
}

// FUNCTION private
//	$$$::_CommonColumnMap::setSecondElement -- 
//
// NOTES
//
// ARGUMENTS
//	int iPosition_
//	Plan::Relation::RowElement* pElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_CommonColumnMap::
setSecondElement(int iPosition_,
				 Plan::Relation::RowElement* pElement_)
{
	m_vecElement[iPosition_].second.second = pElement_;
}

// FUNCTION private
//	$$$::_CommonColumnMap::checkMapValue -- 
//
// NOTES
//
// ARGUMENTS
//	const PAIR<STRING
//	NameMapValue>& cEntry_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_CommonColumnMap::
checkMapValue(const PAIR<STRING, NameMapValue>& cEntry_)
{
	switch (getPosition(cEntry_.second)) {
	case Value::Initial:
		{
			_SYDNEY_THROW1(Exception::CommonColumnNotFound,
						   cEntry_.first);
		}
	case Value::Invalid:
		{
			_SYDNEY_THROW1(Exception::DuplicateCommonColumn,
						   cEntry_.first);
		}
	default:
		{
			if (getFirstElement(getPosition(cEntry_.second)) == 0
				|| getSecondElement(getPosition(cEntry_.second)) == 0) {
				_SYDNEY_THROW1(Exception::CommonColumnNotFound,
							   cEntry_.first);
			}
			break;
		}
	}
}

///////////////////////////////////
// Query::QualifiedJoinImpl::Base
///////////////////////////////////

// FUNCTION public
//	Query::QualifiedJoinImpl::Base::getRelation -- generate Plan::Tree::Node from Statement::Object
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
QualifiedJoinImpl::Base::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::QualifiedJoin* pQJ =
		_SYDNEY_DYNAMIC_CAST(Statement::QualifiedJoin*, pStatement_);

	// Join operand
	Statement::Object* pLeft = pQJ->getLeft();
	Statement::Object* pRight = pQJ->getRight();
	// Join condition
	Statement::Object* pSpec = pQJ->getJoinSpec();

	Plan::Interface::IRelation* pLeftRelation = 0;
	Plan::Interface::IRelation* pRightRelation = 0;

	{
		// create new namescope if current scope is not joined table
		Opt::Environment::AutoPop cAutoPop =
			cEnvironment_.pushNameScope(Opt::Environment::Scope::JoinedTable);

		// create relations from operand
		pLeftRelation = pLeft->getAnalyzer2()->getRelation(cEnvironment_, pLeft);
		pRightRelation = pRight->getAnalyzer2()->getRelation(cEnvironment_, pRight);
	}

	Opt::Environment::AutoPop cAutoPop2(0,0);
	if (pQJ->getJoinType() != Statement::JoinType::Inner
		&& pQJ->getJoinType() != Statement::JoinType::Exists) {
		Opt::Environment::AutoPop cAutoPopTmp =
			cEnvironment_.pushStatus(Opt::Environment::Status::NoTopPredicate);
		cAutoPop2 = cAutoPopTmp; // to avoid compile error in GCC
	}

	// add join condition
	return createJoin(cEnvironment_,
					  _cNodeTypeTable[pQJ->getJoinType()],
					  pLeftRelation,
					  pRightRelation,
					  pSpec);
}

// FUNCTION public
//	Query::QualifiedJoinImpl::Base::getRelation -- generate Plan::Tree::Node from Statement::Object
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
QualifiedJoinImpl::Base::
getDistributeRelation(Opt::Environment& cEnvironment_,
					  Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_,
					   pStatement_);
}

// FUNCTION public
//	Query::QualifiedJoinImpl::Base::getDegree -- 
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
QualifiedJoinImpl::Base::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::QualifiedJoin* pQJ =
		_SYDNEY_DYNAMIC_CAST(Statement::QualifiedJoin*, pStatement_);

	// Join operand
	Statement::Object* pLeft = pQJ->getLeft();
	Statement::Object* pRight = pQJ->getRight();

	return pLeft->getAnalyzer2()->getDegree(cEnvironment_, pLeft)
		+ pRight->getAnalyzer2()->getDegree(cEnvironment_, pRight);
}

///////////////////////////////////////
// Query::QualifiedJoinImpl::Condition
///////////////////////////////////////

// FUNCTION local
//	Query::QualifiedJoinImpl::Condition::createJoin -- create join relation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Tree::Node::Type eNodeType_
//	Plan::Interface::IRelation* pOperand0_
//	Plan::Interface::IRelation* pOperand1_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
QualifiedJoinImpl::Condition::
createJoin(Opt::Environment& cEnvironment_,
		   Plan::Tree::Node::Type eNodeType_,
		   Plan::Interface::IRelation* pOperand0_,
		   Plan::Interface::IRelation* pOperand1_,
		   Statement::Object* pStatement_) const
{
	; _SYDNEY_ASSERT(pStatement_);
	Plan::Relation::Join* pJoin =
		Plan::Relation::Join::create(cEnvironment_,
									 eNodeType_,
									 0,
									 MAKEPAIR(pOperand0_, pOperand1_));

	Plan::Interface::IPredicate* pJoinCondition =
		pStatement_->getAnalyzer2()->getPredicate(cEnvironment_, pJoin, pStatement_);

	pJoin->setJoinPredicate(pJoinCondition);
	return pJoin;
}

/////////////////////////////////////////
// Query::QualifiedJoinImpl::ColumnList
/////////////////////////////////////////

// FUNCTION protected
//	Query::QualifiedJoinImpl::ColumnList::addCommonColumn -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pJoin_
//	Plan::Relation::RowInfo* pRowInfo_
//	VECTOR<Plan::Interface::IPredicate*>& vecCondition_
//	const PAIR<Plan::Relation::RowElement*, Plan::Relation::RowElement*>& cCommonElement_
//	Plan::Utility::RowElementSet& cCommonColumn0_
//	Plan::Utility::RowElementSet& cCommonColumn1_
//	const STRING& cstrName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
QualifiedJoinImpl::ColumnList::
addCommonColumn(Opt::Environment& cEnvironment_,
				Plan::Interface::IRelation* pJoin_,
				Plan::Relation::RowInfo* pRowInfo_,
				VECTOR<Plan::Interface::IPredicate*>& vecCondition_,
				const PAIR<Plan::Relation::RowElement*, Plan::Relation::RowElement*>& cCommonElement_,
				Plan::Utility::RowElementSet& cCommonColumn0_,
				Plan::Utility::RowElementSet& cCommonColumn1_,
				const STRING& cstrName_) const
{
	; _SYDNEY_ASSERT(cCommonElement_.first);
	; _SYDNEY_ASSERT(cCommonElement_.second);

	cCommonColumn0_.add(cCommonElement_.first);
	cCommonColumn1_.add(cCommonElement_.second);

	Plan::Interface::IScalar* pColumn0 = cCommonElement_.first->getScalar(cEnvironment_);
	Plan::Interface::IScalar* pColumn1 = cCommonElement_.second->getScalar(cEnvironment_);

	if (cEnvironment_.checkStatus(Opt::Environment::Status::NoTopPredicate) == false
		&& pJoin_->getType() != Plan::Tree::Node::RightOuterJoin
		&& pJoin_->getType() != Plan::Tree::Node::LeftOuterJoin) {
		cEnvironment_.addKnownNotNull(pColumn0);
		cEnvironment_.addKnownNotNull(pColumn1);
	}

	// check comparability here because result is used in rowinfo too
	Plan::Predicate::Comparison::checkComparability(cEnvironment_,
													Plan::Tree::Node::Equals,
													&pColumn0,
													&pColumn1);

	// create join condition
	vecCondition_.PUSHBACK(Plan::Predicate::Comparison::create(
								  cEnvironment_,
								  Plan::Tree::Node::Equals,
								  MAKEPAIR(pColumn0, pColumn1),
								  false /* no more check for comparability */));

	// create row element of common column
	Plan::Relation::RowElement* pCommonColumn = 0;
	switch (pJoin_->getType()) {
	case Plan::Tree::Node::SimpleJoin:
	case Plan::Tree::Node::Exists:
		{
			// inner join, exists -> use left
			pCommonColumn = cCommonElement_.first;
			break;
		}
	case Plan::Tree::Node::RightOuterJoin:
		{
			SWAP(pColumn0, pColumn1);
			// cont.
		}
	case Plan::Tree::Node::LeftOuterJoin:
	default:
		{
			// left, right, full outer join
			// [NOTE] full outer join is not supported for now
			pCommonColumn = Plan::Relation::RowElement::create(
										   cEnvironment_,
										   0,
										   0,
										   Plan::Scalar::Function::create(
												  cEnvironment_,
												  Plan::Tree::Node::Coalesce,
												  MAKEPAIR(pColumn0, pColumn1),
												  cstrName_));
			break;
		}
	}

	// add to result row
	pRowInfo_->PUSHBACK(MAKEPAIR(cstrName_,
								 pCommonColumn));
}

// FUNCTION private
//	Query::QualifiedJoinImpl::ColumnList::addRowInfo -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pOperand_
//	const Plan::Utility::RowElementSet& cCommonColumn_
//	Plan::Relation::RowInfo* pRowInfo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
QualifiedJoinImpl::ColumnList::
addRowInfo(Opt::Environment& cEnvironment_,
		   Plan::Interface::IRelation* pOperand_,
		   const Plan::Utility::RowElementSet& cCommonColumn_,
		   Plan::Relation::RowInfo* pRowInfo_) const
{
	Plan::Relation::RowInfo* pOperandRowInfo = pOperand_->getRowInfo(cEnvironment_);
	Plan::Relation::RowInfo::Iterator iterator = pOperandRowInfo->begin();
	Plan::Relation::RowInfo::Iterator last = pOperandRowInfo->end();
	while (iterator != last) {
		if (cCommonColumn_.isContaining((*iterator).second)) {
			// remove from result row of operand
			iterator = pOperandRowInfo->erase(iterator);
			last = pOperandRowInfo->end();
		} else {
			// add to join relation's result row
			pRowInfo_->PUSHBACK(*iterator);
			++iterator;
		}
	}
}

// FUNCTION public
//	Query::QualifiedJoinImpl::Base::getRelation -- generate Plan::Tree::Node from Statement::Object
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
QualifiedJoinImpl::ColumnList::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	if (cEnvironment_.hasCascade()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	return Super::getRelation(cEnvironment_,
							  pStatement_);
}

// FUNCTION private
//	Query::QualifiedJoinImpl::ColumnList::addNameMap -- add created rowelements to namemap
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pJoin_
//	Plan::Relation::RowInfo* pRowInfo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
QualifiedJoinImpl::ColumnList::
addNameMap(Opt::Environment& cEnvironment_,
		   Plan::Interface::IRelation* pJoin_,
		   Plan::Relation::RowInfo* pRowInfo_) const
{
	// get current namemap
	Opt::NameMap* pNameMap = cEnvironment_.getNameMap();
	// namemap for join
	Opt::NameMap::NameScalarMap& cScalarMap = pNameMap->getMap(pJoin_);
	// add all rowelements to namemap
	pRowInfo_->foreachEntry(boost::bind(&Opt::NameMap::addElement,
										pNameMap,
										boost::ref(cScalarMap),
										_1, _2));
}

// FUNCTION private
//	Query::QualifiedJoinImpl::ColumnList::createCommonColumn -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pOperand0_
//	Plan::Interface::IRelation* pOperand1_
//	Plan::Interface::IRelation* pJoin_
//	Plan::Relation::RowInfo* pRowInfo_
//	VECTOR<Plan::Interface::IPredicate*>& vecCondition_
//	Plan::Utility::RowElementSet& cCommonColumn0_
//	Plan::Utility::RowElementSet& cCommonColumn1_
//	Statement::ColumnNameList* pCNL_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
QualifiedJoinImpl::ColumnList::
createCommonColumn(Opt::Environment& cEnvironment_,
				   Plan::Interface::IRelation* pOperand0_,
				   Plan::Interface::IRelation* pOperand1_,
				   Plan::Interface::IRelation* pJoin_,
				   Plan::Relation::RowInfo* pRowInfo_,
				   VECTOR<Plan::Interface::IPredicate*>& vecCondition_,
				   Plan::Utility::RowElementSet& cCommonColumn0_,
				   Plan::Utility::RowElementSet& cCommonColumn1_,
				   Statement::ColumnNameList* pCNL_) const
{
	if (int nCNL = pCNL_->getCount()) {
		// get operand0's rowinfo
		Plan::Relation::RowInfo* pOperandRowInfo0 = pOperand0_->getRowInfo(cEnvironment_);
		// get operand1's rowinfo
		Plan::Relation::RowInfo* pOperandRowInfo1 = pOperand1_->getRowInfo(cEnvironment_);

		// create map for specified name
		_CommonColumnMap cCommonColumn;
		for (int i = 0; i < nCNL; ++i) {
			Statement::ColumnName* pName = pCNL_->getColumnNameAt(i);
			; _SYDNEY_ASSERT(pName->getIdentifierString());
			STRING cstrName(*pName->getIdentifierString());

			cCommonColumn.addName(cstrName);
		}

		// check columns of operand0 and operand1
		// [NOTE]
		//	use IsAll so that loop can be skipped if any element cause error
		Opt::IsAll(*pOperandRowInfo0,
				   boost::bind(&_CommonColumnMap::checkColumnFirst,
							   boost::ref(cCommonColumn),
							   boost::ref(cEnvironment_),
							   _1))
			&& Opt::IsAll(*pOperandRowInfo1,
						  boost::bind(&_CommonColumnMap::checkColumnSecond,
									  boost::ref(cCommonColumn),
									  boost::ref(cEnvironment_),
									  _1));

		// check for any error
		// [NOTE]
		//	error is checked for case of unfound column specification
		cCommonColumn.checkError();

		// create common column
		Opt::ForEachMap(cCommonColumn.getVector(),
						boost::bind(&This::addCommonColumn,
									this,
									boost::ref(cEnvironment_),
									pJoin_,
									pRowInfo_,
									boost::ref(vecCondition_),
									_2,
									boost::ref(cCommonColumn0_),
									boost::ref(cCommonColumn1_),
									_1));
	}
}

// FUNCTION private
//	Query::QualifiedJoinImpl::ColumnList::createJoin -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Tree::Node::Type eNodeType_
//	Plan::Interface::IRelation* pOperand0_
//	Plan::Interface::IRelation* pOperand1_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
QualifiedJoinImpl::ColumnList::
createJoin(Opt::Environment& cEnvironment_,
		   Plan::Tree::Node::Type eNodeType_,
		   Plan::Interface::IRelation* pOperand0_,
		   Plan::Interface::IRelation* pOperand1_,
		   Statement::Object* pStatement_) const
{
	// create result relation
	Plan::Relation::Join* pJoin =
		Plan::Relation::Join::create(cEnvironment_,
									 eNodeType_,
									 0,
									 MAKEPAIR(pOperand0_, pOperand1_));

	VECTOR<Plan::Interface::IPredicate*> vecCondition;
	Plan::Utility::RowElementSet cCommonColumn0;
	Plan::Utility::RowElementSet cCommonColumn1;

	// result row of join
	Plan::Relation::RowInfo* pRowInfo = Plan::Relation::RowInfo::create(cEnvironment_);

	Statement::ColumnNameList* pCNL = _SYDNEY_DYNAMIC_CAST(Statement::ColumnNameList*, pStatement_);
	; _SYDNEY_ASSERT(pStatement_ == 0 || pCNL);

	// create common columns
	createCommonColumn(cEnvironment_,
					   pOperand0_,
					   pOperand1_,
					   pJoin,
					   pRowInfo,
					   vecCondition,
					   cCommonColumn0,
					   cCommonColumn1,
					   pCNL);

	// create name map using common column
	addNameMap(cEnvironment_,
			   pJoin,
			   pRowInfo);

	// add left row
	addRowInfo(cEnvironment_,
			   pOperand0_,
			   cCommonColumn0,
			   pRowInfo);
	// add right row
	addRowInfo(cEnvironment_,
			   pOperand1_,
			   cCommonColumn1,
			   pRowInfo);

	// set join column to the result
	pJoin->setJoinPredicate(Plan::Predicate::Combinator::create(cEnvironment_,
																Plan::Tree::Node::And,
																vecCondition));
	// set rowinfo
	pJoin->setRowInfo(cEnvironment_,
					  pRowInfo);

	return pJoin;
}

/////////////////////////////////////////
// Query::QualifiedJoinImpl::Natural
/////////////////////////////////////////

// FUNCTION private
//	Query::QualifiedJoinImpl::Natural::createCommonColumn -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pOperand0_
//	Plan::Interface::IRelation* pOperand1_
//	Plan::Interface::IRelation* pJoin_
//	Plan::Relation::RowInfo* pRowInfo_
//	VECTOR<Plan::Interface::IPredicate*>& vecCondition_
//	Plan::Utility::RowElementSet& cCommonColumn0_
//	Plan::Utility::RowElementSet& cCommonColumn1_
//	Statement::ColumnNameList* pCNL_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
QualifiedJoinImpl::Natural::
createCommonColumn(Opt::Environment& cEnvironment_,
				   Plan::Interface::IRelation* pOperand0_,
				   Plan::Interface::IRelation* pOperand1_,
				   Plan::Interface::IRelation* pJoin_,
				   Plan::Relation::RowInfo* pRowInfo_,
				   VECTOR<Plan::Interface::IPredicate*>& vecCondition_,
				   Plan::Utility::RowElementSet& cCommonColumn0_,
				   Plan::Utility::RowElementSet& cCommonColumn1_,
				   Statement::ColumnNameList* pCNL_) const
{
	// get operand0's rowinfo
	Plan::Relation::RowInfo* pOperandRowInfo0 = pOperand0_->getRowInfo(cEnvironment_);
	// get operand1's rowinfo
	Plan::Relation::RowInfo* pOperandRowInfo1 = pOperand1_->getRowInfo(cEnvironment_);

	// create map for operand1's column name
	_CommonColumnMap cCommonColumn;
	Opt::ForEach(*pOperandRowInfo1,
				 boost::bind(&_CommonColumnMap::addName,
							 boost::ref(cCommonColumn),
							 boost::ref(cEnvironment_),
							 _1));

	// check name of operand0's columns
	Opt::ForEach(*pOperandRowInfo0,
				 boost::bind(&_CommonColumnMap::checkColumnFirst,
							 boost::ref(cCommonColumn),
							 boost::ref(cEnvironment_),
							 _1));

	// ignore illegal pairs
	cCommonColumn.ignoreError();

	// create common column
	Opt::ForEachMap(cCommonColumn.getVector(),
					boost::bind(&This::addCommonColumn,
								this,
								boost::ref(cEnvironment_),
								pJoin_,
								pRowInfo_,
								boost::ref(vecCondition_),
								_2,
								boost::ref(cCommonColumn0_),
								boost::ref(cCommonColumn1_),
								_1));
}

//////////////////////////////////
// Query::QualifiedJoin
//////////////////////////////////

// FUNCTION public
//	Query::QualifiedJoin::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::QualifiedJoin* pStatement_
//	
// RETURN
//	const QualifiedJoin*
//
// EXCEPTIONS

//static
const QualifiedJoin*
QualifiedJoin::
create(const Statement::QualifiedJoin* pStatement_)
{
	if (pStatement_->getJoinSpec() == 0) {
		return &_analyzerNatural;
	}
	if (pStatement_->getJoinSpec()->getType() == Statement::ObjectType::ColumnNameList) {
		return &_analyzerColumn;
	}
	return &_analyzerCondition;
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
