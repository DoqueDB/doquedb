// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/RowInfo.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Relation";
}

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Plan/Relation/RowInfo.h"

#include "Plan/Interface/IRelation.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"
#include "Common/ResultSetMetaData.h"

#include "Execution/Interface/IProgram.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

namespace
{
	// CLASS local
	//	_ElementMap --
	//
	// NOTES
	class _ElementMap
		: public MAP<Interface::IRelation*,
					 Utility::RowElementSet,
					 Utility::RelationSet::Comparator>
	{
	public:
		Utility::RowElementSet& getElement(Interface::IRelation* pRelation_)
		{return (*this)[pRelation_];}
	};

	// CLASS local
	//	_DistinctChecker --
	//
	// NOTES
	class _DistinctChecker
	{
	public:
		_DistinctChecker(Opt::Environment& cEnvironment_,
						 Utility::RowElementSet& cResult_)
			: m_cEnvironment(cEnvironment_),
			  m_cResult(cResult_)
		{}

		void operator()(_ElementMap::ValueType& cEntry_)
		{
			if (cEntry_.first) {
				// get distinct key subset
				(void)Relation::Inquiry::isDistinct(m_cEnvironment,
													cEntry_.first,
													&cEntry_.second);
			}
			// add to result
			m_cResult.merge(cEntry_.second);
		}
	protected:
	private:
		Opt::Environment& m_cEnvironment;
		Utility::RowElementSet& m_cResult;
	};
}

///////////////////////////////
//	Plan::Relation::RowInfo

// FUNCTION public
//	Relation::RowInfo::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	RowInfo*
//
// EXCEPTIONS

//static
RowInfo*
RowInfo::
create(Opt::Environment& cEnvironment_)
{
	AUTOPOINTER<This> pResult = new This;
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Relation::RowInfo::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Vector& vecElement_
//	
// RETURN
//	RowInfo*
//
// EXCEPTIONS

//static
RowInfo*
RowInfo::
create(Opt::Environment& cEnvironment_,
	   const Vector& vecElement_)
{
	AUTOPOINTER<This> pResult = new This(vecElement_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Relation::RowInfo::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	const STRING& cCorrelationName_
//	Position iStart_
//	Size iSize_
//	
// RETURN
//	RowInfo*
//
// EXCEPTIONS

//static
RowInfo*
RowInfo::
create(Opt::Environment& cEnvironment_,
	   Interface::IRelation* pRelation_,
	   Position iStart_,
	   Position iEnd_)
{
	AUTOPOINTER<This> pResult = new This;
	if (iEnd_ > iStart_) {
		pResult->reserve(iEnd_ - iStart_);
		for (Position i = iStart_; i < iEnd_; ++i) {
			pResult->PUSHBACK(Element(STRING(),
									  RowElement::create(cEnvironment_, pRelation_, i)));
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Relation::RowInfo::append -- append contents
//
// NOTES
//
// ARGUMENTS
//	RowInfo* pRowInfo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
RowInfo::
append(RowInfo* pRowInfo_)
{
	if (pRowInfo_) {
		insert(end(), pRowInfo_->begin(), pRowInfo_->end());
	}
}

// FUNCTION public
//	Relation::RowInfo::getScalarName -- get scalar name in this row spec
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
RowInfo::
getScalarName(Opt::Environment& cEnvironment_,
			  Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < static_cast<Position>(GETSIZE()));

	return getScalarName(cEnvironment_,
						 (*this)[iPosition_]);
}

// FUNCTION public
//	Relation::RowInfo::getScalarName -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Element& cElement_
//	
// RETURN
//	const STRING&
//
// EXCEPTIONS

const STRING&
RowInfo::
getScalarName(Opt::Environment& cEnvironment_,
			  Element& cElement_)
{
	return (cElement_.first.getLength() == 0)
		? cElement_.second->getScalarName(cEnvironment_)
		: cElement_.first;
}

// FUNCTION public
//	Relation::RowInfo::getScalar -- get scalar interface in this row spec
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
RowInfo::
getScalar(Opt::Environment& cEnvironment_,
		  Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < static_cast<Position>(GETSIZE()));

	return (*this)[iPosition_].second->getScalar(cEnvironment_);
}

// FUNCTION public
//	Relation::RowInfo::getDistinctKey -- get distinct key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::RowElementSet& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
RowInfo::
getDistinctKey(Opt::Environment& cEnvironment_,
			   Utility::RowElementSet& cResult_)
{
	// Grouping elements by relation
	_ElementMap mapElement;
	foreachElement(boost::bind(&Utility::RowElementSet::addObject,
							   boost::bind(&_ElementMap::getElement,
										   boost::ref(mapElement),
										   boost::bind(&RowElement::getRelation,
													   _1)),
							   _1));
	Opt::ForEach(mapElement, _DistinctChecker(cEnvironment_, cResult_));

	return cResult_.isEmpty() == false;
}

// FUNCTION public
//	Relation::RowInfo::retrieve -- mark scalar as retrieve
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
RowInfo::
retrieve(Opt::Environment& cEnvironment_,
		 Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < static_cast<Position>(GETSIZE()));

	(*this)[iPosition_].second->retrieve(cEnvironment_);
}

// FUNCTION public
//	Relation::RowInfo::retrieveAll -- mark all scalars as retrieve
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
RowInfo::
retrieveAll(Opt::Environment& cEnvironment_)
{
	foreachElement(boost::bind(&RowElement::retrieve,
							   _1,
							   boost::ref(cEnvironment_)));
}

// FUNCTION public
//	Relation::RowInfo::createMetaData -- create result set metadata
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

Common::Data::Pointer
RowInfo::
createMetaData(Opt::Environment& cEnvironment_)
{
	AUTOPOINTER<Common::ResultSetMetaData> pResult = new Common::ResultSetMetaData;
	pResult->reserve(getSize());
	Iterator iterator = begin();
	const Iterator last = end();
	for (; iterator != last; ++iterator) {
		Common::ColumnMetaData cColumnMeta;
		(*iterator).second->setMetaData(cEnvironment_, cColumnMeta);
		if ((*iterator).first.getLength() != 0) {
			cColumnMeta.setColumnAliasName((*iterator).first);
		} else {
			cColumnMeta.setColumnAliasName(cColumnMeta.getColumnName());
		}
		pResult->pushBack(cColumnMeta);
	}
	return pResult.release();
}

// FUNCTION public
//	Relation::RowInfo::generate -- generate variable
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
RowInfo::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecID;
	vecID.reserve(getSize());

	mapElement(vecID,
			   boost::bind(&Interface::IScalar::generate,
						   boost::bind(&RowElement::getScalar,
									   _1,
									   boost::ref(cEnvironment_)),
						   boost::ref(cEnvironment_),
						   boost::ref(cProgram_),
						   pIterator_,
						   boost::ref(cArgument_)));

	if (vecID.getSize()) {
		return cProgram_.addVariable(vecID);
	} else {
		return -1;
	}
}


// FUNCTION public
//	Relation::RowInfo::generate -- generate variable
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
RowInfo::
generateFromType(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecID;
	vecID.reserve(getSize());

	mapElement(vecID,
			   boost::bind(&Interface::IScalar::generateFromType,
						   boost::bind(&RowElement::getScalar,
									   _1,
									   boost::ref(cEnvironment_)),
						   boost::ref(cEnvironment_),
						   boost::ref(cProgram_),
						   pIterator_,
						   boost::ref(cArgument_)));

	if (vecID.getSize()) {
		return cProgram_.addVariable(vecID);
	} else {
		return -1;
	}
}

// FUNCTION private
//	Relation::RowInfo::RowInfo -- costructor
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

RowInfo::
RowInfo()
	: Super()
{}

// FUNCTION private
//	Relation::RowInfo::RowInfo -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Vector& vecElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

RowInfo::
RowInfo(const Vector& vecElement_)
	: Super(vecElement_)
{}

// FUNCTION private
//	Relation::RowInfo::registerToEnvironment -- register to environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
RowInfo::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.addRowInfo(this);
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
