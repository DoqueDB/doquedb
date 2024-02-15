// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IRelation.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Interface";
}

#include "SyDefault.h"

#include "Plan/Interface/IRelation.h"

#include "Plan/Interface/IPredicate.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Selection.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Environment.h"
#include "Opt/NameMap.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_INTERFACE_USING

// FUNCTION public
//	Interface::IRelation::erase -- destructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	This* pThis_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
IRelation::
erase(Opt::Environment& cEnvironment_,
	  This* pThis_)
{
	if (pThis_) {
		pThis_->eraseFromEnvironment(cEnvironment_);
		delete pThis_;
	}
}

// FUNCTION public
//	Interface::IRelation::getRowInfo -- result row spec
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Relation::RowInfo*
//
// EXCEPTIONS

Relation::RowInfo*
IRelation::
getRowInfo(Opt::Environment& cEnvironment_)
{
	if (!m_pRowInfo) {
		m_pRowInfo = createRowInfo(cEnvironment_);
	}
	return m_pRowInfo;
}

// FUNCTION public
//	Interface::IRelation::getKeyInfo -- key row spec
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Relation::RowInfo*
//
// EXCEPTIONS

Relation::RowInfo*
IRelation::
getKeyInfo(Opt::Environment& cEnvironment_)
{
	if (!m_pKeyInfo) {
		m_pKeyInfo = createKeyInfo(cEnvironment_);
	}
	return m_pKeyInfo;
}

// FUNCTION public
//	Interface::IRelation::setRowInfo -- set result row spec
//
// NOTES
//	This method should be called by such relations that returns 0 for createrowinfo
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::RowInfo* pRowInfo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IRelation::
setRowInfo(Opt::Environment& cEnvironment_,
		   Relation::RowInfo* pRowInfo_)
{
	; _SYDNEY_ASSERT(!m_pRowInfo);
	m_pRowInfo = pRowInfo_;
}

// FUNCTION public
//	Interface::IRelation::getDegree -- degree of the relation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	IRelation::Size
//
// EXCEPTIONS

IRelation::Size
IRelation::
getDegree(Opt::Environment& cEnvironment_)
{
	if (m_iDegree < 0) {
		m_iDegree = setDegree(cEnvironment_);
	}
	; _SYDNEY_ASSERT(m_iDegree >= 0);
	return m_iDegree;
}

// FUNCTION public
//	Interface::IRelation::getMaxPosition -- max of position
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	IRelation::Position
//
// EXCEPTIONS

IRelation::Position
IRelation::
getMaxPosition(Opt::Environment& cEnvironment_)
{
	if (m_iMaxPosition < 0) {
		m_iMaxPosition = setMaxPosition(cEnvironment_);
	}
	; _SYDNEY_ASSERT(m_iMaxPosition >= 0);
	return m_iMaxPosition;
}

// FUNCTION public
//	Interface::IRelation::getCardinality -- cardinality of the relation if available
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	IRelation::Size
//
// EXCEPTIONS

//virtual
IRelation::Size
IRelation::
getCardinality(Opt::Environment& cEnvironment_)
{
	// default: can't return cardinality
	return 0;
}

// FUNCTION public
//	Interface::IRelation::getRow -- get i-th row if available
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	int iPosition_
//	VECTOR<Interface::IScalar*>& vecResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IRelation::
getRow(Opt::Environment& cEnvironment_,
	   int iPosition_,
	   VECTOR<Interface::IScalar*>& vecResult_)
{
	// default: can't return row
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Interface::IRelation::getCorrelationName -- get relation name
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	const STRING&
//
// EXCEPTIONS

//virtual
const STRING&
IRelation::
getCorrelationName(Opt::Environment& cEnvironment_)
{
	// default: use environment registration
	return cEnvironment_.getCorrelationName(this);
}

// FUNCTION public
//	Interface::IRelation::setCorrelationName -- replace name->scalar map with correlation name spec
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrTableName_
//	const VECTOR<STRING>& vecColumnName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
IRelation::
setCorrelationName(Opt::Environment& cEnvironment_,
				   const STRING& cstrTableName_,
				   const VECTOR<STRING>& vecColumnName_)
{
	if (cstrTableName_.getLength() == 0
		&& vecColumnName_.ISEMPTY() == false) {
		// by default, table name should be specified
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// obtain current row spec
	Relation::RowInfo* pRowInfo = getRowInfo(cEnvironment_);

	// reset namemap entry by specified name
	Opt::NameMap* pNameMap = cEnvironment_.getNameMap();
	Opt::NameMap::NameScalarMap& cMap = pNameMap->add(cstrTableName_, this);
	cMap.erase(cMap.begin(), cMap.end());

	Size n = vecColumnName_.GETSIZE();
	if (n) {
		; _SYDNEY_ASSERT(n == pRowInfo->GETSIZE());
		for (Position i = 0; i < n; ++i) {
			cMap[vecColumnName_[i]] = (*pRowInfo)[i].second;
			cEnvironment_.setAliasName((*pRowInfo)[i].second, vecColumnName_[i]);
		}
	} else {
		// use default name
		n = pRowInfo->getSize();
		for (Position i = 0; i < n; ++i) {
			cMap.insert(pRowInfo->getScalarName(cEnvironment_, i), (*pRowInfo)[i].second);
		}
		// add key row too
		Relation::RowInfo* pKeyInfo = getKeyInfo(cEnvironment_);
		n = pKeyInfo->getSize();
		for (Position i = 0; i < n; ++i) {
			cMap.insert(pKeyInfo->getScalarName(cEnvironment_, i), (*pKeyInfo)[i].second);
		}
	}
}

// FUNCTION public
//	Interface::IRelation::getScalarName -- get scalar name by position
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Position iPosition_
//	
// RETURN
//	const STRING&
//
// EXCEPTIONS

const STRING&
IRelation::
getScalarName(Opt::Environment& cEnvironment_,
			  Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < getMaxPosition(cEnvironment_));

	if (m_vecName.GETSIZE() <= iPosition_
		|| m_vecName[iPosition_].getLength() == 0) {
		createScalarName(cEnvironment_, m_vecName, iPosition_);
	}
	; _SYDNEY_ASSERT(m_vecName.GETSIZE() > iPosition_
					 && m_vecName[iPosition_].getLength() != 0);

	return m_vecName[iPosition_];
}

// FUNCTION public
//	Interface::IRelation::getScalar -- get scalar interface by position
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Position iPosition_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
IRelation::
getScalar(Opt::Environment& cEnvironment_,
		  Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < getMaxPosition(cEnvironment_));

	if (m_vecScalar.GETSIZE() <= iPosition_
		|| m_vecScalar[iPosition_] == 0) {
		createScalar(cEnvironment_, m_vecScalar, iPosition_);
	}

	; _SYDNEY_ASSERT(m_vecScalar.GETSIZE() > iPosition_
					 && m_vecScalar[iPosition_] != 0);

	return m_vecScalar[iPosition_];
}

// FUNCTION public
//	Interface::Interface::getNodeType -- get node type of each scalar by position
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Position iPosition_
//	
// RETURN
//	IRelation::Type
//
// EXCEPTIONS

IRelation::Type
IRelation::
getNodeType(Opt::Environment& cEnvironment_,
			Position iPosition_)
{
	if (m_vecType.GETSIZE() <= iPosition_
		|| m_vecType[iPosition_] == IRelation::Undefined) {
		createScalarType(cEnvironment_, m_vecType, iPosition_);
	}
	; _SYDNEY_ASSERT(m_vecType.GETSIZE() > iPosition_
					 && m_vecType[iPosition_] != IRelation::Undefined);

	return m_vecType[iPosition_];
}

// FUNCTION public
//	Interface::IRelation::retrieve -- set scalar as retrieve
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IRelation::
retrieve(Opt::Environment& cEnvironment_,
		 Position iPosition_)
{
	setRetrieved(cEnvironment_, iPosition_);
}

// FUNCTION public
//	Interface::IRelation::aggregate -- set scalar as aggregation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pScalar_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
IRelation::
aggregate(Opt::Environment& cEnvironment_,
		  Interface::IScalar* pScalar_,
		  Interface::IScalar* pOperand_)
{
	return addAggregation(cEnvironment_, pScalar_, pOperand_);
}

// FUNCTION public
//	Interface::IRelation::estimateUnwind -- estimate relation count in unwind result
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
IRelation::
estimateUnwind(Opt::Environment& cEnvironment_)
{
	// default: not added in unwind
	return 1;
}

// FUNCTION public
//	Interface::IRelation::estimateUnion -- estimate relation count in union rewrite
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
IRelation::
estimateUnion(Opt::Environment& cEnvironment_)
{
	// default: not added in rewrite
	return 1;
}

// FUNCTION public
//	Interface::IRelation::unwind -- unwind subquery
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	PAIR<Interface::IRelation*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Interface::IRelation*, Interface::IPredicate*>
IRelation::
unwind(Opt::Environment& cEnvironment_)
{
	return MAKEPAIR(this, static_cast<Interface::IPredicate*>(0));
}

// FUNCTION public
//	Interface::IRelation::rewrite -- rewrite by predicate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	PAIR<Interface::IRelation*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Interface::IRelation*, Interface::IPredicate*>
IRelation::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IPredicate* pPredicate_,
		Predicate::RewriteArgument& cArgument_)
{
	return pPredicate_->rewrite(cEnvironment_,
								this,
								cArgument_);
}


// FUNCTION public
//	Interface::IRelation::IRelation -- createSql
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	SqlQuery
//
// EXCEPTIONS
//
// VIRTUAL
Sql::Query*
IRelation::generateSQL(Opt::Environment& cEnvironment_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}


// FUNCTION protected
//	Interface::IRelation::IRelation -- costructor
//
// NOTES
//
// ARGUMENTS
//	IRelation::Type eType_
//	NameInfo* pNameInfo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

IRelation::
IRelation(IRelation::Type eType_)
	: Super(eType_),
	  m_pRowInfo(0),
	  m_pKeyInfo(0),
	  m_iDegree(-1),
	  m_iMaxPosition(-1),
	  m_cstrCorrelationName(),
	  m_cstrOutputVariableName(),
	  m_bOutputVariable(false),
	  m_vecName(),
	  m_vecScalar(),
	  m_vecType(),
	  m_bGrouping(false)
{}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
