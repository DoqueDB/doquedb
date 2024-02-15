// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/KwicImpl.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Scalar::Impl";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Impl/KwicImpl.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Function.h"

#include "Plan/File/KwicOption.h"
#include "Plan/Predicate/Contains.h"
#include "Plan/Relation/Table.h"

#include "Common/Assert.h"

#include "Exception/KwicWithoutContains.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/Argument.h"
#include "Execution/Function/Kwic.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Environment.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace
{
	//////////////////
	// CLASS local
	//	$$$::_SetOption -- set option to Kwic executor
	//
	// NOTES
	class _SetOption
	{
	public:
		_SetOption(Opt::Environment& cEnvironment_,
				   Execution::Interface::IProgram& cProgram_,
				   Execution::Interface::IIterator* pIterator_,
				   Candidate::AdoptArgument& cArgument_,
				   Execution::Function::Kwic* pKwic_)
			: m_cEnvironment(cEnvironment_),
			  m_cProgram(cProgram_),
			  m_pIterator(pIterator_),
			  m_cArgument(cArgument_),
			  m_pKwic(pKwic_)
		{}

		void operator()(Interface::IScalar* pOption_);

	protected:
	private:
		Opt::Environment& m_cEnvironment;
		Execution::Interface::IProgram& m_cProgram;
		Execution::Interface::IIterator* m_pIterator;
		Candidate::AdoptArgument& m_cArgument;
		Execution::Function::Kwic* m_pKwic;
	};
} // namespace

/////////////////////////
//	$$$::_SetOption::

// FUNCTION public
//	$$$::_SetOption::operator() -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IScalar* pOption_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_SetOption::
operator()(Interface::IScalar* pOption_)
{
	int iValueID = pOption_->generate(m_cEnvironment,
									  m_cProgram,
									  m_pIterator,
									  m_cArgument);
	switch (pOption_->getType()) {
	case Tree::Node::KwicSize:
		{
			m_pKwic->setSize(iValueID);
			break;
		}
	case Tree::Node::KwicStartTag:
		{
			m_pKwic->setStartTag(iValueID);
			break;
		}
	case Tree::Node::KwicEndTag:
		{
			m_pKwic->setEndTag(iValueID);
			break;
		}
	case Tree::Node::KwicEscape:
		{
			m_pKwic->setEscape(iValueID);
			break;
		}
	case Tree::Node::KwicEllipsis:
		{
			m_pKwic->setEllipsis(iValueID);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

///////////////////////////////////
//	Scalar::Impl::KwicImpl::

// FUNCTION public
//	Scalar::Impl::KwicImpl::generateThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::KwicImpl::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	int iSourceID = getOperand0()->generate(cEnvironment_,
											cProgram_,
											pIterator_,
											cArgument_);
	int iPositionID = getOperand1()->generate(cEnvironment_,
											  cProgram_,
											  pIterator_,
											  cArgument_);
	Execution::Function::Kwic* pKwic = Execution::Function::Kwic::create(cProgram_,
																		 pIterator_,
																		 iSourceID,
																		 iPositionID,
																		 iDataID_);
	const VECTOR<Predicate::Contains*>& vecContains =
		cEnvironment_.getContainsByAnyOperand(getOperand0());
	VECTOR<Predicate::Contains*>::CONSTITERATOR iterator = vecContains.begin();
	const VECTOR<Predicate::Contains*>::CONSTITERATOR last = vecContains.end();

	if (iterator == last) {
		// no contains -> can't execute
		_SYDNEY_THROW0(Exception::KwicWithoutContains);
	}
	// first contains predicate
	generateKwic(cEnvironment_,
				 cProgram_,
				 pIterator_,
				 cArgument_,
				 *iterator,
				 pKwic,
				 false /* not secondary */);
	++iterator;
	Opt::ForEach(iterator, last,
				 boost::bind(&This::generateKwic,
							 this,
							 boost::ref(cEnvironment_),
							 boost::ref(cProgram_),
							 pIterator_,
							 boost::ref(cArgument_),
							 _1,
							 pKwic,
							 true /* secondary */));
	return iDataID_;
}

// FUNCTION private
//	Scalar::Impl::KwicImpl::generateKwic -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Predicate::Contains* pContains_
//	Execution::Function::Kwic* pKwic_
//	bool bSecondary_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::KwicImpl::
generateKwic(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 Predicate::Contains* pContains_,
			 Execution::Function::Kwic* pKwic_,
			 bool bSecondary_)
{
	// set option from Kwic option of file
	File::KwicOption* pKwicOption = pContains_->getKwicOption();
	; _SYDNEY_ASSERT(pKwicOption);

	const PAIR<int, int>& cPropertyID = pKwicOption->generateProperty(cProgram_);
	pKwic_->setPropertyKey(cPropertyID.first);
	pKwic_->setPropertyValue(cPropertyID.second);
	if (pKwicOption->getLanguage()) {
		int iLanguageID = pKwicOption->getLanguage()->generate(cEnvironment_,
															   cProgram_,
															   pIterator_,
															   cArgument_);
		pKwic_->setLanguage(iLanguageID);
	}

	if (bSecondary_ == false) {
		// set field ID for the operand
		Interface::IFile* pFile = pKwicOption->getFile();
		if (pFile == 0
			|| getOperand0()->isField() == false) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		{
			Relation::Table* pTable = getOperand0()->getField()->getTable();
			Relation::Table::AutoReset cAuto = pTable->setEstimateFile(pFile);
			Schema::Field* pSchemaField = getOperand0()->getField()->getSchemaField();
			if (pSchemaField == 0) {
				_SYDNEY_THROW0(Exception::Unexpected);
			}
			pKwic_->setFieldID(pSchemaField->getPosition());
		}

		// set option from Kwic parameter
		foreachOption(_SetOption(cEnvironment_,
								 cProgram_,
								 pIterator_,
								 cArgument_,
								 pKwic_));

		pIterator_->addCalculation(cProgram_,
								   pKwic_,
								   cArgument_.m_eTarget);
	}
}

// FUNCTION private
//	Scalar::Impl::KwicImpl::createDataType -- 
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
Impl::KwicImpl::
createDataType(Opt::Environment& cEnvironment_)
{
	if (getOperand0()->getDataType().isCharacterStringType() == false) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	DataType cDataType(getOperand0()->getDataType());
	cDataType.setMaxCardinality(0);
	cDataType.setFlag(DataType::Flag::Variable);
	setDataType(cDataType);
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
