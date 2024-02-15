// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IRow.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Plan/Interface/IRow.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Scalar/Argument.h"

#include "Opt/Explain.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_INTERFACE_BEGIN

////////////////////////////////////
//	Plan::Interface::IRow::

// FUNCTION public
//	Interface::IRow::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Super::Argument cArgument_
//	
// RETURN
//	IRow*
//
// EXCEPTIONS

IRow*
IRow::
create(Opt::Environment& cEnvironment_,
	   Super::Argument cArgument_)
{
	AUTOPOINTER<This> pResult = new This(cArgument_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Interface::IRow::explain -- explain
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

// explain
void
IRow::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.put('(');
	joinOperand(boost::bind(&Interface::IScalar::explain,
							_1,
							pEnvironment_,
							boost::ref(cExplain_)),
				boost::bind(&Opt::Explain::putChar,
							boost::ref(cExplain_),
							','));
	cExplain_.put(')');
}

// FUNCTION public
//	Interface::IRow::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IRow::
getUsedTable(Utility::RelationSet& cResult_)
{
	foreachOperand(boost::bind(&Operand::getUsedTable,
							   _1,
							   boost::ref(cResult_)));
}

// FUNCTION public
//	Interface::IRow::getUsedField -- check used scalars
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IRow::
getUsedField(Utility::FieldSet& cResult_)
{
	foreachOperand(boost::bind(&Operand::getUsedField,
							   _1,
							   boost::ref(cResult_)));
}

// FUNCTION public
//	Interface::IRow::getUnknownKey -- check unknown keys
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IRow::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	foreachOperand(boost::bind(&Operand::getUnknownKey,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cResult_)));
}

// FUNCTION public
//	Interface::IRow::hasParameter -- has unassigned parameters?
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

bool
IRow::
hasParameter()
{
	return isAny(boost::bind(&Operand::hasParameter,
							 _1));
}

// FUNCTION public
//	Interface::IRow::require -- set refered scalars as required
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

void
IRow::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	foreachOperand(boost::bind(&Operand::require,
							   _1,
							   boost::ref(cEnvironment_),
							   pCandidate_));
}

// FUNCTION public
//	Interface::IRow::retrieve -- set refered scalars as retrieved
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
IRow::
retrieve(Opt::Environment& cEnvironment_)
{
	foreachOperand(boost::bind(&Operand::retrieve,
							   _1,
							   boost::ref(cEnvironment_)));
}

// FUNCTION public
//	Interface::IRow::retrieve -- set refered scalars as retrieved
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

void
IRow::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	foreachOperand(boost::bind(&Operand::retrieve,
							   _1,
							   boost::ref(cEnvironment_),
							   pCandidate_));
}

// FUNCTION public
//	Interface::IRow::use -- set refered scalars as used
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

void
IRow::
use(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	foreachOperand(boost::bind(&Operand::use,
							   _1,
							   boost::ref(cEnvironment_),
							   pCandidate_));
}

// FUNCTION public
//	Interface::IRow::delay -- set refered scalars as delayable
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
IRow::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	bool bResult = false;

	Iterator iterator = begin();
	const Iterator last = end();
	for (; iterator != last; ++iterator) {
		Interface::IScalar* pScalar = *iterator;
		if (pScalar->delay(cEnvironment_, pCandidate_, cArgument_)) {
			bResult = true;
		}
	}
	return bResult;
}


// FUNCTION public
//	Interface::IRow::toSQLStatement
//
// NOTES
//
// ARGUMENTS
//	
// RETURN
//	STRING
//
// EXCEPTIONS
STRING
IRow::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}


_SYDNEY_PLAN_INTERFACE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
