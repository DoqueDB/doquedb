// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Base.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Scalar";
}

#include "SyDefault.h"

#include "Plan/Scalar/Base.h"
#include "Plan/Scalar/Cast.h"

#include "Plan/Sql/Query.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_SCALAR_USING

//////////////////////////////
//	Plan::Scalar::Base

// FUNCTION public
//	Scalar::Base::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(getName());
}

// FUNCTION public
//	Scalar::Base::setExpectedType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Scalar::DataType& cType_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
Base::
setExpectedType(Opt::Environment& cEnvironment_,
				const Scalar::DataType& cType_)
{
	// default: create cast node
	return createCast(cEnvironment_,
					  cType_,
					  false /* not for comparison */)
		->setNodeType(getType());
}

// FUNCTION public
//	Scalar::Base::createCast -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const DataType& cToType_
//	bool bForComparison_
//	Tree::Node::Type eType_ = Tree::Node::Undefined
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
Base::
createCast(Opt::Environment& cEnvironment_,
		   const DataType& cToType_,
		   bool bForComparison_,
		   Tree::Node::Type eType_ /* = Tree::Node::Undefined */)
{
	if (isNeedCast(cToType_,
				   bForComparison_,
				   eType_) == false) {
		return this;
	}

	Interface::IScalar* pResult = getCast(cEnvironment_,
										  cToType_,
										  bForComparison_);
	if (pResult == 0) {
		pResult =  Scalar::Cast::create(cEnvironment_,
										this,
										cToType_,
										bForComparison_,
										bForComparison_);
		setCast(cEnvironment_,
				cToType_,
				bForComparison_,
				pResult);
	}
	return pResult;
}

// FUNCTION public
//	Scalar::Base::isKnownNull -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
isKnownNull(Opt::Environment& cEnvironment_)
{
	return cEnvironment_.isKnownNull(this);
}

// FUNCTION public
//	Scalar::Base::isKnownNotNull -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
isKnownNotNull(Opt::Environment& cEnvironment_)
{
	return cEnvironment_.isKnownNull(this);
}

// FUNCTION public
//	Scalar::Base::hasParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
hasParameter()
{
	return false;
}

// FUNCTION public
//	Scalar::Base::isArbitraryElement -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
isArbitraryElement()
{
	return false;
}

// FUNCTION public
//	Scalar::Base::isField -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
isField()
{
	return false;
}

// FUNCTION public
//	Scalar::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
Base::
getField()
{
	return 0;
}

// FUNCTION public
//	Scalar::Base::hasField -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
hasField(Interface::IFile* pFile_)
{
	return false;
}

// FUNCTION public
//	Scalar::Base::isOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
isOperation()
{
	return false;
}

// FUNCTION public
//	Scalar::Base::isEquivalent -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IScalar* pScalar_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
isEquivalent(Interface::IScalar* pScalar_)
{
	return pScalar_ == this;
}

// FUNCTION public
//	Scalar::Base::addOption -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOption_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
Base::
addOption(Opt::Environment& cEnvironment_,
		  Interface::IScalar* pOption_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Scalar::Base::preCalculate -- 
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

//virtual
Common::Data::Pointer
Base::
preCalculate(Opt::Environment& cEnvironment_)
{
	// by default, scalar value cannot be calculated in optimization phase
	return Common::Data::Pointer();
}

// FUNCTION public
//	Scalar::Base::retrieveFromCascade
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Sql::Query* pQuery_
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
Base::
retrieveFromCascade(Opt::Environment& cEnvironment_,
					Sql::Query* pQuery_)
{
	pQuery_->addProjectionColumn(this);
}


// FUNCTION public
//	Scalar::Base::use -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
use(Opt::Environment& cEnvironment_,
	Interface::ICandidate* pCandidate_)
{
	// by default, use is equivalent to require
	require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Scalar::Base::isSubquery -- 
//
// NOTES
//
// ARGUMENTS
//	
//	
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
isSubquery() const
{
	return false;
}

// FUNCTION public
//	Scalar::Base::equalsOperand -- 
//
// NOTES
//
// ARGUMENTS
//	IScalar& arg 
//	
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
equalsOperand(const IScalar* pArg) const
{
	if (pArg->getType() != getType() ||
		pArg->getOperandSize() != getOperandSize()) {
		return false;
	}
	for (unsigned int i = 0; i < pArg->getOperandSize(); i++) {
		const LogicalFile::TreeNodeInterface* pArgOperand = pArg->getOperandAt(i);
		unsigned int j = 0;
		for (; j < getOperandSize(); j++) {
			if (getOperandAt(j) == pArgOperand) break;
		}
		if (j == getOperandSize()) return false;
	}
	return true;
}

// FUNCTION protected
//	Scalar::Base::isNeedCast -- 
//
// NOTES
//
// ARGUMENTS
//	const DataType& cToType_
//	bool bForComparison_
//	Tree::Node::Type eType_ = Tree::Node::Undefined
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Base::
isNeedCast(const DataType& cToType_,
		   bool bForComparison_,
		   Tree::Node::Type eType_ /* = Tree::Node::Undefined */)
{
	return ((bForComparison_ == false
			 && DataType::isAssignable(getDataType(), cToType_, eType_) == false)
			||
			(bForComparison_ == true
			 && ((eType_ == Tree::Node::Fetch
				  && getDataType().getDataType() != cToType_.getDataType())
				 ||
				 (eType_ != Tree::Node::Fetch
				  && DataType::isComparable(getDataType(), cToType_) == false))));
}

// FUNCTION private
//	Scalar::Base::getCast -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const DataType& cToType_
//	bool bForComparison_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
Base::
getCast(Opt::Environment& cEnvironment_,
		const DataType& cToType_,
		bool bForComparison_)
{
	CastMap& cMap = m_mapCast[bForComparison_?1:0];
	CastMap::Iterator found = cMap.find(cToType_);
	if (found != cMap.end()) {
		return (*found).second;
	}
	return 0;
}

// FUNCTION private
//	Scalar::Base::setCast -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const DataType& cToType_
//	bool bForComparison_
//	Interface::IScalar* pResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
setCast(Opt::Environment& cEnvironment_,
		const DataType& cToType_,
		bool bForComparison_,
		Interface::IScalar* pResult_)
{
	m_mapCast[bForComparison_?1:0][cToType_] = pResult_;
}

//
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
