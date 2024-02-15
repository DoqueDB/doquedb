// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/ValueImpl.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Impl/ValueImpl.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"
#include "Common/DefaultData.h"
#include "Common/DoubleData.h"
#include "Common/NullData.h"
#include "Common/UnicodeString.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Exception/Unexpected.h"
#include "Exception/VariableNotFound.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"

#include "Server/Session.h"

#include "ModUnicodeCharTrait.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_SCALAR_USING

namespace
{
	// CONST
	//	$$$::_emptyString --
	//
	// NOTES

	STRING _emptyString;

	// FUNCTION local
	//	$$$::_cutString -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeChar* p_
	//	const ModUnicodeChar* last_
	//	ModSize uMaxLength_
	//	ModSize uTrailerLength_
	//	
	// RETURN
	//	const ModUnicodeChar*
	//
	// EXCEPTIONS

	const ModUnicodeChar*
	_cutString(const ModUnicodeChar* p_,
			   const ModUnicodeChar* last_,
			   ModSize uMaxLength_,
			   ModSize uTrailerLength_)
	{
		ModSize n = static_cast<ModSize>(last_ - p_);
		if (n * 2 <= uMaxLength_) {
			// never become over max length
			return last_;
		}
		if (n <= uMaxLength_) {
			const ModUnicodeChar* p = p_;
			for (; p != last_; ++p) {
				if (!ModUnicodeCharTrait::isAscii(*p)) ++n;
				else if (*p == Common::UnicodeChar::usQuate) ++n;
			}
			if (n <= uMaxLength_) {
				// width is less than max
				return last_;
			}
		}
		// search for cutting position
		ModSize l = 0;
		uMaxLength_ -= uTrailerLength_;
		const ModUnicodeChar* p = p_;
		for (; p != last_; ++p) {
			ModSize d = 1;
			if ((!ModUnicodeCharTrait::isAscii(*p)) || (*p == Common::UnicodeChar::usQuate))
				++d;

			if (l + d > uMaxLength_) break;

			l += d;
		}
		return p;
	}

	// FUNCTION local
	//	$$$::_escapeString -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeString& cstrValue_
	//	
	// RETURN
	//	ModUnicodeString
	//
	// EXCEPTIONS
	ModUnicodeString
	_escapeString(const ModUnicodeString& cstrValue_)
	{
		const ModSize uMaxLength = 30;
		const ModUnicodeString cstrTrailer("...");
		const ModSize uTrailerLength = 3;

		ModUnicodeString cstrResult;
		cstrResult.reallocate(uMaxLength);

		ModSize n = 0;
		const ModUnicodeChar* p = cstrValue_;
		const ModUnicodeChar* pTail = cstrValue_.getTail();

		// get end position to fit width less than maxlength
		const ModUnicodeChar* pCut = _cutString(p, pTail, uMaxLength, uTrailerLength);

		// find quote to escape
		const ModUnicodeChar* pQuote =
			ModUnicodeCharTrait::find(p, Common::UnicodeChar::usQuate, static_cast<ModSize>(pCut - p));
		if (pQuote) {
			do {
				cstrResult.append(Common::UnicodeChar::usQuate);
				cstrResult.append(p, static_cast<ModSize>(pQuote - p + 1));
				p = pQuote + 1;
				pQuote = (p >= pCut) ? 0
					: ModUnicodeCharTrait::find(p, Common::UnicodeChar::usQuate, static_cast<ModSize>(pCut - p));
			} while (pQuote);

			if (p < pCut) {
				cstrResult.append(p, static_cast<ModSize>(pCut - p));
			}
		} else {
			// not need to escape
			cstrResult.append(Common::UnicodeChar::usQuate);
			cstrResult.append(p, static_cast<ModSize>(pCut - p));
		}
		if (pCut < pTail) {
			cstrResult.append(cstrTrailer);
		}
		cstrResult.append(Common::UnicodeChar::usQuate);

		return cstrResult;
	}

	struct _Type
	{
		enum Operator {
			Symmetric = 0,
			Asymmetric,
			Upper,
			Lower,
			AverageLength,
			Df,
			ScoreFunction,
			Extractor,
			ClusteredLimit,
			ClusteredCombiner,
			ScoreCombiner,
			Calculator,
			Category,
			Scale,
			Language,
			Default
		};
	};
	
	const char* const _pszOperatorName[] = 
	{
		" symmetric",
		" asymmetric",
		" upper ",
		" lower ",
		" average length ",
		" df ",
		" score function ",
		" extractor",
		" clustered limit ",
		" clustered combiner ",		
		" score combiner ",
		" calculator ",
		" category ",
		" scale ",
		" language ",
		""
	};
	
	const char* _getOperatorName(Tree::Node::Type eType_)
	{
		switch (eType_)
		{
		case Plan::Tree::Node::Upper:					return _pszOperatorName[_Type::Upper];
		case Plan::Tree::Node::Lower:					return _pszOperatorName[_Type::Lower];			
		case Plan::Tree::Node::AverageLength:			return _pszOperatorName[_Type::AverageLength];
		case Plan::Tree::Node::Df:						return _pszOperatorName[_Type::Df];
		case Plan::Tree::Node::ScoreFunction:			return _pszOperatorName[_Type::ScoreFunction];
		case Plan::Tree::Node::Extractor:				return _pszOperatorName[_Type::Extractor];
		case Plan::Tree::Node::ClusteredLimit:			return _pszOperatorName[_Type::ClusteredLimit];
		case Plan::Tree::Node::ClusteredCombiner:		return _pszOperatorName[_Type::ClusteredCombiner]; 
		case Plan::Tree::Node::ScoreCombiner:			return _pszOperatorName[_Type::ScoreCombiner];
		case Plan::Tree::Node::Calculator:				return _pszOperatorName[_Type::Calculator];
		case Plan::Tree::Node::Category:				return _pszOperatorName[_Type::Category];
		case Plan::Tree::Node::Scale:					return _pszOperatorName[_Type::Scale];
		case Plan::Tree::Node::Language:				return _pszOperatorName[_Type::Language];
		default:										return _pszOperatorName[_Type::Default];
		}
	}
} // namespace

////////////////////////////////////
// ValueImpl::Base::
////////////////////////////////////

// FUNCTION public
//	Scalar::ValueImpl::Base::explain -- 
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
ValueImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	if (getData()) {
		if (getDataType().isCharacterStringType()
			|| getDataType().isLanguageType()) {
			cExplain_.put(_escapeString(getData()->toString()));
		} else {
			cExplain_.put(getData()->toString());
		}
	} else if (getName().getLength()) {
		cExplain_.put(getName());
	} else {
		cExplain_.put("<?>");
	}
}

// FUNCTION public
//	Scalar::ValueImpl::Base::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	ValueImpl::Base::Check::Value
//
// EXCEPTIONS

//virtual
ValueImpl::Base::Check::Value
ValueImpl::Base::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	return Check::Constant;
}

// FUNCTION public
//	Scalar::ValueImpl::Base::isRefering -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ValueImpl::Base::
isRefering(Interface::IRelation* pRelation_)
{
	return pRelation_ == 0;
}

// FUNCTION public
//	Scalar::ValueImpl::Base::getUsedTable -- check used tables
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

//virtual
void
ValueImpl::Base::
getUsedTable(Utility::RelationSet& cResult_)
{
	;
}

// FUNCTION public
//	Scalar::ValueImpl::Base::getUsedField -- 
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

//virtual
void
ValueImpl::Base::
getUsedField(Utility::FieldSet& cResult_)
{
	;
}

// FUNCTION public
//	Scalar::ValueImpl::Base::getUnknownKey -- 
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

//virtual
void
ValueImpl::Base::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	if (getDataType().isNoType()
		|| getDataType().getDataType() == Common::DataType::Null) {
		cResult_.m_cKey.add(this);
	}
}

// FUNCTION public
//	Scalar::ValueImpl::Base::require -- extract refered scalar
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
ValueImpl::Base::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	// value is always unnecessary to read value from a relation
	;
}

// FUNCTION public
//	Scalar::ValueImpl::Base::retrieve -- 
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

//virtual
void
ValueImpl::Base::
retrieve(Opt::Environment& cEnvironment_)
{
	// value is always unnecessary to read value from a relation
	;
}

// FUNCTION public
//	Scalar::ValueImpl::Base::retrieve -- extract refered scalar
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
ValueImpl::Base::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	// value is always unnecessary to read value from a relation
	;
}

// FUNCTION public
//	Scalar::ValueImpl::Base::delay -- extract refered scalar
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

//virtual
bool
ValueImpl::Base::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	// value is always unnecessary to read value from a relation
	// -> can be regarded as delayable
	return true;
}

// FUNCTION public
//	Scalar::ValueImpl::Base::setMetaData -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Common::ColumnMetaData& cMetaData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ValueImpl::Base::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	cMetaData_.setNotSearchable();
	cMetaData_.setReadOnly();
}

// FUNCTION public
//	Scalar::ValueImpl::Constant::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ValueImpl::Base::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	if (getType() == Plan::Tree::Node::Symmetric) {
		if (getData()->getInt() == 1) {
			cExec_.append(_pszOperatorName[_Type::Symmetric]);
		} else {
			cExec_.append(_pszOperatorName[_Type::Asymmetric]);
		}
	} else {
		cExec_.append(_getOperatorName(getType()));
		if (getData()) {
			if (getData()->getType() == Common::DataType::String ||
				getData()->getType() == Common::DataType::Date ||
				getData()->getType() == Common::DataType::DateTime ||
				getData()->getType() == Common::DataType::Language) {
				cExec_.append(" '");
				cExec_.append(getData()->getString());
				cExec_.append("' ");
			} else {
				cExec_.append(getData()->getString());
			}
		} else {
			cExec_.append(getValue());
		}
	}
}



////////////////////////////////////
// ValueImpl::Constant::
////////////////////////////////////

// FUNCTION public
//	Scalar::ValueImpl::Constant::getName -- get scalar name
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const STRING&
//
// EXCEPTIONS

//virtual
const STRING&
ValueImpl::Constant::
getName()
{
	return _emptyString;
}


// FUNCTION public
//	Scalar::ValueImpl::Constant::setExpectedType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const DataType& cToType_
//
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
ValueImpl::Constant::
setExpectedType(Opt::Environment& cEnvironment_,
				const DataType& cType_)
{

	if (m_pData.get() &&
		m_pData->isNull() &&
		! cType_.isNoType()) {
		DataPointer pData = cType_.createData();
		pData->setNull();
		m_pData = pData;
		return this;
	} else { 
		return Super::setExpectedType(cEnvironment_,
									  cType_);
	}
}
// FUNCTION public
//	Scalar::ValueImpl::Base::createCast -- 
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
ValueImpl::Constant::
createCast(Opt::Environment& cEnvironment_,
		   const DataType& cToType_,
		   bool bForComparison_,
		   Tree::Node::Type eType_ /* = Tree::Node::Undefined */)
{
	if (isNeedCast(cToType_, bForComparison_, eType_) == false) {
		// no need to cast
		return this;
	}

	DataPointer pTarget = cToType_.createData();
	if (getData()->isDefault()) {
		return Value::Default::create(cEnvironment_);

	} else if (getData()->isNull()
		|| cToType_.cast(getDataType(),
						 getData(),
						 pTarget,
						 bForComparison_,
						 bForComparison_ || eType_ == Tree::Node::Fetch) == false) {
		// if cast is failed for comparison, set result null
		return Value::Null::create(cEnvironment_);
	}
	return Super::create(cEnvironment_, pTarget, cToType_);
}

// FUNCTION public
//	Scalar::ValueImpl::Constant::preCalculate -- 
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
ValueImpl::Constant::
preCalculate(Opt::Environment& cEnvironment_)
{
	return m_pData;
}


// FUNCTION public
//	Scalar::ValueImpl::Constant::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//
//	
// RETURN
//	STRING
//
// EXCEPTIONS

//virtual
STRING
ValueImpl::Constant::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	if (m_cstrSQL.getLength() > 0) {
		return m_cstrSQL;
	} else {
		if (m_pData->getType() == Common::DataType::String ||
			m_pData->getType() == Common::DataType::Date ||
			m_pData->getType() == Common::DataType::DateTime ||
			m_pData->getType() == Common::DataType::Language) {
			ModUnicodeOstrStream cStream;
			cStream << " '" << m_pData->getString() << "' ";
			return cStream.getString();
		} else {
			return m_pData->getString();
		}
	}

}


// FUNCTION public
//	Scalar::ValueImpl::Constant::getValue -- get string representing contents of the node
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
ValueImpl::Constant::
getValue() const
{
	; _SYDNEY_ASSERT(m_pData.get());
	return m_pData->toString();
}

// FUNCTION public
//	Scalar::ValueImpl::Constant::getData -- get data object held by this node
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::Data*
//
// EXCEPTIONS

//virtual
const Common::Data*
ValueImpl::Constant::
getData() const
{
	return m_pData.get();
}

// FUNCTION protected
//	Scalar::ValueImpl::Constant::generateData -- 
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

//virtual
int
ValueImpl::Constant::
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	; _SYDNEY_ASSERT(m_pData.get());
	return cProgram_.addVariable(m_pData);
}

////////////////////////////////////
// ValueImpl::Variable::
////////////////////////////////////

// FUNCTION public
//	Scalar::ValueImpl::Asterisk::setParameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual

void
ValueImpl::Asterisk::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	cExec_.append("*");
}



////////////////////////////////////
// ValueImpl::Variable::
////////////////////////////////////

// FUNCTION public
//	Scalar::ValueImpl::Variable::setData -- set data (for variable)
//
// NOTES
//
// ARGUMENTS
//	const DataPointer& pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ValueImpl::Variable::
setData(const DataPointer& pData_)
{
	if (getDataType().isNoType()) {
		m_pData = pData_;
		setDataType(DataType(pData_->getType()));
	} else {
		if (m_pData.get() == 0) {
			m_pData = getDataType().createData();
		}
		getDataType().cast(DataType(pData_->getType()),
						   pData_,
						   m_pData);
	}
}

// FUNCTION public
//	Scalar::ValueImpl::Variable::getName -- get scalar name
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const STRING&
//
// EXCEPTIONS

//virtual
const STRING&
ValueImpl::Variable::
getName()
{
	return m_cstrName;
}

// FUNCTION public
//	Scalar::ValueImpl::Variable::setExpectedType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const DataType& cType_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
ValueImpl::Variable::
setExpectedType(Opt::Environment& cEnvironment_,
				const DataType& cType_)
{
	if (getDataType().isNoType()) {
		setDataType(cType_);
		return this;
	} else {
		return Super::setExpectedType(cEnvironment_, cType_);
	}
}

// FUNCTION public
//	Scalar::ValueImpl::Variable::preCalculate -- 
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
ValueImpl::Variable::
preCalculate(Opt::Environment& cEnvironment_)
{
	return m_pData;
}


// FUNCTION public
//	Scalar::ValueImpl::Variable::getValue -- get string representing contents of the node
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
ValueImpl::Variable::
getValue() const
{
	if (m_pData.get()) {
		return m_pData->toString();
	}
	return m_cstrName;
}

// FUNCTION public
//	Scalar::ValueImpl::Variable::getData -- get data object held by this node
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::Data*
//
// EXCEPTIONS

//virtual
const Common::Data*
ValueImpl::Variable::
getData() const
{
	return m_pData.get();
}

// FUNCTION protected
//	Scalar::ValueImpl::Variable::generateData -- 
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

//virtual
int
ValueImpl::Variable::
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	// add new variable
	if (m_pData.get()) {
		return cProgram_.addVariable(m_pData);
	} else {
		return cProgram_.addVariable(getDataType().createData());
	}
}

////////////////////////////////////
// ValueImpl::PlaceHolder::
////////////////////////////////////

// FUNCTION public
//	Scalar::ValueImpl::PlaceHolder::hasParameter -- 
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
ValueImpl::PlaceHolder::
hasParameter()
{
	return true;
}


// FUNCTION public
//	Predicate::Impl::DyadicComparison::setParameter -- 
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
ValueImpl::PlaceHolder::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	
	int iDataID = cProgram_.getPlaceHolder(m_iNumber);
	if (iDataID < 0) {
		Candidate::AdoptArgument cAdopArg;
		iDataID = generate(cEnvironment_,
						   cProgram_,
						   pIterator_,
						   cAdopArg);		
	}
	cExec_.append(_getOperatorName(getType()));
	cExec_.appendPlaceHolder(iDataID);
}


// FUNCTION protected
//	Scalar::ValueImpl::PlaceHolder::generateData -- 
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

//virtual
int
ValueImpl::PlaceHolder::
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	int iResult = cProgram_.getPlaceHolder(m_iNumber);
	if (iResult < 0) {
		iResult = Super::generateData(cEnvironment_, cProgram_, pIterator_, cArgument_);
		cProgram_.setPlaceHolder(m_iNumber, iResult);
	}
	return iResult;
}

////////////////////////////////////
// ValueImpl::BulkVariable::
////////////////////////////////////

// FUNCTION public
//	Scalar::ValueImpl::BulkVariable::createCast -- 
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
ValueImpl::BulkVariable::
createCast(Opt::Environment& cEnvironment_,
		   const DataType& cToType_,
		   bool bForComparison_,
		   Tree::Node::Type eType_ /* = Tree::Node::Undefined */)
{
	if (bForComparison_ == false
		&& getDataType().isNoType()) {

		// set datatype
		setDataType(cToType_);
		return this;

	} else {
		return Super::createCast(cEnvironment_,
								 cToType_,
								 bForComparison_,
								 eType_);
	}
}

////////////////////////////////////
// ValueImpl::RelationVariable::
////////////////////////////////////

// FUNCTION public
//	Scalar::ValueImpl::RelationVariable::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	ValueImpl::RelationVariable::Check::Value
//
// EXCEPTIONS

//virtual
ValueImpl::RelationVariable::Check::Value
ValueImpl::RelationVariable::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	if (cArgument_.m_pCandidate
		&& cArgument_.m_pCandidate->isReferingRelation(m_pRelation)) {
		return Check::Referred;
	} else if (cArgument_.m_vecPrecedingCandidate.ISEMPTY() == false) {
		VECTOR<Interface::ICandidate*>::CONSTITERATOR found =
			Opt::Find(cArgument_.m_vecPrecedingCandidate.begin(),
					  cArgument_.m_vecPrecedingCandidate.end(),
					  boost::bind(&Interface::ICandidate::isReferingRelation,
								  _1,
								  m_pRelation));
		if (found != cArgument_.m_vecPrecedingCandidate.end()) {
			return Check::Preceding;
		}
	}
	return Check::NotYet;
}

// FUNCTION public
//	Scalar::ValueImpl::RelationVariable::isRefering -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ValueImpl::RelationVariable::
isRefering(Interface::IRelation* pRelation_)
{
	return pRelation_ == m_pRelation;
}

// FUNCTION public
//	Scalar::ValueImpl::RelationVariable::getUsedTable -- 
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

//virtual
void
ValueImpl::RelationVariable::
getUsedTable(Utility::RelationSet& cResult_)
{
	cResult_.add(m_pRelation);
}

// FUNCTION public
//	Scalar::ValueImpl::RelationVariable::require -- 
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
ValueImpl::RelationVariable::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	m_pRelation->require(cEnvironment_,
						 pCandidate_);
}

// FUNCTION public
//	Scalar::ValueImpl::RelationVariable::retrieve -- 
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
ValueImpl::RelationVariable::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	require(cEnvironment_,
			pCandidate_);
}

// FUNCTION public
//	Scalar::ValueImpl::RelationVariable::delay -- 
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

//virtual
bool
ValueImpl::RelationVariable::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	require(cEnvironment_,
			pCandidate_);
	return false;
}


// FUNCTION public
//	Scalar::ValueImpl::SessionVariable::setParameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
ValueImpl::SessionVariable::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	Server::Session* pSession =
		Server::Session::getSession(cEnvironment_.getTransaction().getSessionID());
	if (pSession == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Server::Session::BitSetVariable* pSessionVariable = pSession->getBitSetVariable(getName());
	if (pSessionVariable == 0) {
		_SYDNEY_THROW1(Exception::VariableNotFound, getName());
	}
	Common::BitSet::ConstIterator ite =  pSessionVariable->getValue().begin();
	Common::BitSet::ConstIterator end =  pSessionVariable->getValue().end();
	ModUnicodeOstrStream cStream;
	char c = ' ';
	for (; ite != end; ++ite, c = ',') {
		cStream << c << (*ite);
	}
	cExec_.append(cStream.getString());
}
//
// Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
