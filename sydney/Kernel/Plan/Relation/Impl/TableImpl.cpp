// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Impl/TableImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Relation/Impl/TableImpl.h"

#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Argument.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Table.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Scalar/Generator.h"
#include "Plan/Scalar/Operation.h"
#include "Plan/Scalar/UpdateField.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Sql/Table.h"
#include "Plan/Utility/ObjectSet.h"
#include "Plan/Utility/Trace.h"

#include "Common/Assert.h"

#include "Communication/Protocol.h"

#include "Exception/DuplicateUpdateColumn.h"
#include "Exception/InvalidUpdateColumn.h"
#include "Exception/NotSupported.h"
#include "Exception/NullabilityViolation.h"
#include "Exception/TableNotFound.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"

#include "Schema/Column.h"
#include "Schema/Database.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Index.h"
#include "Schema/Key.h"
#include "Schema/Table.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace
{
	struct _CreateTargetFieldArgument
	{
		const Utility::UIntSet& m_cLogRequiredPosition;
		bool m_bAddTarget;
		bool m_bAddLogged;
		bool m_bDoubleLog;

		Scalar::Field* m_pColumn;
		Interface::IScalar* m_pInput;
		Interface::IScalar* m_pOriginal;

		_CreateTargetFieldArgument(const Utility::UIntSet& cLogRequiredPosition_)
			: m_cLogRequiredPosition(cLogRequiredPosition_),
			  m_bAddTarget(false),
			  m_bAddLogged(false),
			  m_bDoubleLog(false),
			  m_pColumn(0),
			  m_pInput(0),
			  m_pOriginal(0)
		{}
	};
}

////////////////////////////////////////////////
//	Plan::Relation::TableImpl::Base

// FUNCTION public
//	Relation::TableImpl::Base::addField -- register field object
//
// NOTES
//
// ARGUMENTS
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Base::
addField(Scalar::Field* pField_)
{
	if (pField_->isFunction() == false) {
		Interface::IScalar* pOption = pField_->getOption();
		Schema::Object* pSchemaObject = 0;

		if (pField_->isColumn()) {
			pSchemaObject = pField_->getSchemaColumn();
		} else {
			pSchemaObject = pField_->getSchemaField();
		}
		m_mapSchemaField[pSchemaObject] = pField_;
		if (pOption) {
			m_mapColumnOption[pOption->getType()][pSchemaObject] = pField_;
		}

		; _SYDNEY_ASSERT(getField(pSchemaObject) != 0);
	}
}

// FUNCTION public
//	Relation::TableImpl::Base::getField -- get registered field object
//
// NOTES
//
// ARGUMENTS
//	Schema::Object* pSchemaObject_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
TableImpl::Base::
getField(Schema::Object* pSchemaObject_)
{
	Scalar::Field* pResult = m_mapSchemaField[pSchemaObject_];
	; _SYDNEY_ASSERT(pResult == 0
					 || (pResult && pResult->isColumn() && pResult->getSchemaColumn() == pSchemaObject_)
					 || (pResult && pResult->isColumn() == false && pResult->getSchemaField() == pSchemaObject_));
	return pResult;
}

// FUNCTION public
//	Relation::TableImpl::Base::getField -- get field with column and option
//
// NOTES
//
// ARGUMENTS
//	Schema::Object* pSchemaObject_
//	Interface::IScalar* pOption_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
TableImpl::Base::
getField(Schema::Object* pSchemaObject_,
		 Interface::IScalar* pOption_)
{
	; _SYDNEY_ASSERT(pSchemaObject_);
	; _SYDNEY_ASSERT(pOption_);

	return m_mapColumnOption[pOption_->getType()][pSchemaObject_];
}

// FUNCTION public
//	Relation::TableImpl::Base::addField -- register field corresponding to a column
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pColumn_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Base::
addField(Opt::Environment& cEnvironment_,
		 Scalar::Field* pColumn_,
		 Interface::IFile* pFile_,
		 Scalar::Field* pField_)
{
	; _SYDNEY_ASSERT(pFile_ == pField_->getFile());
	m_mapColumnField[pFile_][pColumn_] = pField_;
	m_mapColumnField[pFile_][pField_] = pField_;
}

// FUNCTION public
//	Relation::TableImpl::Base::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
TableImpl::Base::
getField(Interface::IFile* pFile_,
		 Scalar::Field* pColumn_)
{
	; _SYDNEY_ASSERT(pColumn_ != 0);
	return m_mapColumnField[pFile_][pColumn_];
}

// FUNCTION public
//	Relation::TableImpl::Base::addRetrieved -- 
//
// NOTES
//
// ARGUMENTS
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Base::
addRetrieved(Scalar::Field* pField_)
{
	; // do nothing
}

// FUNCTION public
//	Relation::TableImpl::Base::require -- 
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
TableImpl::Base::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	; // do nothing
}

// FUNCTION public
//	Relation::TableImpl::Base::getUsedTable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Base::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	cResult_.add(this);
}




// FUNCTION protected
//	Relation::TableImpl::Base::hasField -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Object* pSchemaObject_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Base::
hasField(Schema::Object* pSchemaObject_)
{
	return m_mapSchemaField.find(pSchemaObject_) != m_mapSchemaField.end();
}

////////////////////////////////////////////////
//	Plan::Relation::TableImpl::Retrieve

// FUNCTION public
//	Relation::TableImpl::Retrieve::setEstimateFile -- set estimating file
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	
// RETURN
//	Table::AutoReset
//
// EXCEPTIONS

//virtual
Table::AutoReset
TableImpl::Retrieve::
setEstimateFile(Interface::IFile* pFile_)
{
	m_pEstimateFile = pFile_;

	return AutoReset(this, &Table::resetEstimateFile);
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::getEstimateFile -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IFile*
//
// EXCEPTIONS

//virtual
Interface::IFile*
TableImpl::Retrieve::
getEstimateFile()
{
	return m_pEstimateFile;
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::resetEstimateFile -- 
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

//virtual
void
TableImpl::Retrieve::
resetEstimateFile()
{
	m_pEstimateFile = 0;	
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::setEstimatePredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	Table::AutoReset
//
// EXCEPTIONS

//virtual
Table::AutoReset
TableImpl::Retrieve::
setEstimatePredicate(Interface::IPredicate* pPredicate_)
{
	m_pEstimatePredicate = pPredicate_;
	return AutoReset(this, &Table::resetEstimatePredicate);
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::getEstimatePredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
TableImpl::Retrieve::
getEstimatePredicate()
{
	return m_pEstimatePredicate;
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::resetEstimatePredicate -- 
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

//virtual
void
TableImpl::Retrieve::
resetEstimatePredicate()
{
	m_pEstimatePredicate = 0;
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::isEstimating -- estimating
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
TableImpl::Retrieve::
isEstimating()
{
	return m_pEstimateFile != 0;
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::getField -- get field corresponding to a column
//
// NOTES
//
// ARGUMENTS
//	Scalar::Field* pColumn_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
TableImpl::Retrieve::
getField(Scalar::Field* pColumn_)
{
	; _SYDNEY_ASSERT(pColumn_ != 0);
	; _SYDNEY_ASSERT(isEstimating());

	return getField(m_pEstimateFile, pColumn_);
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::addRetrieved -- 
//
// NOTES
//
// ARGUMENTS
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Retrieve::
addRetrieved(Scalar::Field* pField_)
{
	m_cRetrievedColumn.add(pField_);
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::setEstimateCount -- 
//
// NOTES
//
// ARGUMENTS
//	const AccessPlan::Cost::Value& cValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Retrieve::
setEstimateCount(const AccessPlan::Cost::Value& cValue_)
{
	m_cEstimateCount = cValue_;
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::getEstimateCount -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

//virtual
AccessPlan::Cost::Value
TableImpl::Retrieve::
getEstimateCount()
{
	return m_cEstimateCount;
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::setAdoptCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Candidate::Table* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Retrieve::
setAdoptCandidate(Candidate::Table* pCandidate_)
{
	m_pAdoptCandidate = pCandidate_;
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::getAdoptCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Candidate::Table*
//
// EXCEPTIONS

//virtual
Candidate::Table*
TableImpl::Retrieve::
getAdoptCandidate()
{
	return m_pAdoptCandidate;
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::createAccessPlan -- create access plan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
TableImpl::Retrieve::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Process)) {
		_OPT_OPTIMIZATION_MESSAGE << "Table("
								  << getCorrelationName(cEnvironment_)
								  << ") create access plan with predicate:"
								  << Utility::Trace::toString(cEnvironment_, cPlanSource_.getPredicate())
								  << ModEndl;
	}
#endif
	// when new access plan is created, adopt information is reset
	setAdoptCandidate(0);

	// create new candidate object
	Candidate::Table* pResult = 0;
	if (cPlanSource_.isSimple()) {
		pResult = Candidate::Table::Simple::create(cEnvironment_,
												   this);
	} else {
		pResult = Candidate::Table::Retrieve::create(cEnvironment_,
													 this);
		pResult->createPlan(cEnvironment_,
							cPlanSource_,
							m_cRetrievedColumn);
	}
	return pResult;
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::inquiry -- inquiry about relation's attributes
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::IRelation::InquiryResult
TableImpl::Retrieve::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	InquiryResult iResult = 0;
	if (cArgument_.m_iTarget & InquiryArgument::Target::Distinct) {
		// check whether passed elements are distinct or not
		; _SYDNEY_ASSERT(cArgument_.m_pKey);
		if (checkDistinct(cEnvironment_,
						  const_cast<Utility::RowElementSet&>(*cArgument_.m_pKey))) {
			iResult |= InquiryArgument::Target::Distinct;
		}
	}
	return iResult;
}



// FUNCTION private
//	Relation::TableImpl::Retrieve::setRetrieved -- set retrieved flag
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

//virtual
void
TableImpl::Retrieve::
setRetrieved(Opt::Environment& cEnvironment_,
			 Position iPosition_)
{
	Interface::IScalar* pScalar = getScalar(cEnvironment_, iPosition_);
	; _SYDNEY_ASSERT(pScalar->isField());
	addRetrieved(pScalar->getField());
}

// FUNCTION private
//	Relation::TableImpl::Retrieve::checkDistinct -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::RowElementSet& cKey_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
checkDistinct(Opt::Environment& cEnvironment_,
			  Utility::RowElementSet& cKey_)
{
	Trans::Transaction& cTrans = cEnvironment_.getTransaction();
	Utility::RowElementSet::Iterator found =
		Opt::Find(cKey_.begin(),
				  cKey_.end(),
				  boost::bind(&Scalar::Field::checkIsUnique,
							  boost::ref(cEnvironment_),
							  boost::bind(&Relation::RowElement::getScalar,
										  _1,
										  boost::ref(cEnvironment_))));
	if (found != cKey_.end()) {
		// any unique column is found
		Relation::RowElement* pElement = *found;
		cKey_.clear();
		cKey_.add(pElement);

		return true;
	}
	// don't change cKey
	return false;
}

/////////////////////////////////////
// Relation::TableImpl::Refer

// FUNCTION public
//	Relation::TableImpl::Refer::createAccessPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	virtual Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
TableImpl::Refer::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	// create new candidate object
	Candidate::Table* pResult = Candidate::Table::Refer::create(cEnvironment_,
																this,
																m_pTargetTable);
	pResult->createPlan(cEnvironment_,
						cPlanSource_,
						getRetrievedColumn());

	return pResult;
}

// FUNCTION public
//	Relation::TableImpl::Refer::getCorrelationName -- 
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
TableImpl::Refer::
getCorrelationName(Opt::Environment& cEnvironment_)
{
	return getSchemaTable()->getName();
}

/////////////////////////////////////
// Relation::TableImpl::Put

// FUNCTION public
//	Relation::TableImpl::Put::setAdoptCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Candidate::Table* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Put::
setAdoptCandidate(Candidate::Table* pCandidate_)
{
	m_pAdoptCandidate = pCandidate_;
}

// FUNCTION public
//	Relation::TableImpl::Put::getAdoptCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Candidate::Table*
//
// EXCEPTIONS

//virtual
Candidate::Table*
TableImpl::Put::
getAdoptCandidate()
{
	return m_pAdoptCandidate;
}

// FUNCTION public
//	Relation::TableImpl::Put::addInput -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::RowElement* pElement_
//	Interface::IScalar* pSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Put::
addInput(Opt::Environment& cEnvironment_,
		 Position iPosition_,
		 Interface::IScalar* pSource_)
{
	// set correspondence between column and input data
	ModPair<InputMap::ITERATOR, ModBoolean> cResult =
		m_mapInput.insert(iPosition_, pSource_);
	if (cResult.second == ModFalse) {
		// same column was specified to target
		_SYDNEY_THROW1(Exception::DuplicateUpdateColumn,
					   getScalarName(cEnvironment_, iPosition_));
	}
}

// FUNCTION public
//	Relation::TableImpl::Put::getInput -- 
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

//virtual
Interface::IScalar*
TableImpl::Put::
getInput(Opt::Environment& cEnvironment_,
		 Position iPosition_)
{
	InputMap::ITERATOR found = m_mapInput.find(iPosition_);
	if (found != m_mapInput.end()) {
		return (*found).second;
	}
	return 0;
}

// FUNCTION public
//	Relation::TableImpl::Put::getGeneratedColumn -- 
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

//virtual
RowInfo*
TableImpl::Put::
getGeneratedColumn(Opt::Environment& cEnvironment_)
{
	int iProtocolVersion = cEnvironment_.getProtocolVersion();

	// create generated columns using specified protocol version
	RowInfo* pResult = RowInfo::create(cEnvironment_);
	if (iProtocolVersion >= Communication::Protocol::Version2) { // after v15
		if (isDelete() == false
			&& iProtocolVersion >= Communication::Protocol::Version4) { // jdbc: execute supported
			// use identity, current_timestamp
			if (RowElement* pRowElement = getIdentity(cEnvironment_)) {
				pResult->PUSHBACK(RowInfo::Element(STRING(), pRowElement));
			} else {
				// use null constant instead
				pResult->PUSHBACK(RowInfo::Element(
								   "<identity>",
								   RowElement::create(cEnvironment_,
													  0,
													  0,
													  Scalar::Value::Null::create(cEnvironment_))));
			}
			if (RowElement* pRowElement = getCurrentTimestamp(cEnvironment_)) {
				pResult->PUSHBACK(RowInfo::Element(STRING(), pRowElement));
			} else {
				// use null constant instead
				pResult->PUSHBACK(RowInfo::Element(
								   "<current_timestamp>",
								   RowElement::create(cEnvironment_,
													  0,
													  0,
													  Scalar::Value::Null::create(cEnvironment_))));
			}
		}
		// add rowid
		pResult->PUSHBACK(RowInfo::Element(STRING(), getRowID(cEnvironment_)));
	}
	return pResult;
}

// FUNCTION public
//	Relation::TableImpl::Put::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::IRelation::InquiryResult
TableImpl::Put::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	return 0;
}

// FUNCTION protected
//	Relation::TableImpl::Put::isExplicitValueAllowed -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Put::
isExplicitValueAllowed(Opt::Environment& cEnvironment_,
					   Schema::Column* pSchemaColumn_)
{
	if (cEnvironment_.isRecovery() == false) {
		if (pSchemaColumn_->isTupleID()) {
			// ROWID can not be specified value
			return false;
		} else if (pSchemaColumn_->isIdentity()
				   && pSchemaColumn_->getDefault().isUseAlways()) {
			// generator with ALWAYS can not be specified value
			return false;
		}
	}
	return true;
}

// FUNCTION protected
//	Relation::TableImpl::Put::isConsiderDefault -- check whether input data can be default value
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pInput_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Put::
isConsiderDefault(Opt::Environment& cEnvironment_,
				  Interface::IScalar* pInput_)
{
	if (pInput_ == 0) {
		return true;
	}
	switch (pInput_->getType()) {
	case Tree::Node::Variable:
	case Tree::Node::ConstantValue:
		{
			Common::Data::Pointer pData = pInput_->preCalculate(cEnvironment_);
			if (pData.get() == 0
				|| pData->isDefault()) {
				// data can be DEFAULT
				return true;
			}
			break;
		}
	default:
		{
			break;
		}
	}
	return false;
}

// FUNCTION protected
//	Relation::TableImpl::Put::isValueGenerated -- check whether a column value is generated
//
// NOTES
//
// ARGUMENTS
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Put::
isValueGenerated(Schema::Column* pSchemaColumn_)
{
	return pSchemaColumn_->getDefault().isConstant() == false;
}

// FUNCTION protected
//	Relation::TableImpl::Put::getDefault -- get iscalar denoting default/generated value for the i-th column
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Position iPosition_
//	Interface::IScalar* pInput_
//	bool bForInsert_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
TableImpl::Put::
getDefault(Opt::Environment& cEnvironment_,
		   Schema::Column* pSchemaColumn_,
		   Interface::IScalar* pInput_,
		   bool bForInsert_)
{
	if (pSchemaColumn_->isTupleID()) {
		if (bForInsert_) {
			// create rowid generator
			return Scalar::Generator::RowID::create(cEnvironment_,
													pSchemaColumn_);
		} else {
			;
		}
	} else {
		const Schema::Default& cDefault = pSchemaColumn_->getDefault();
		if (cDefault.isNull() == false) {
			if (cDefault.isIdentity()) {
				if (pInput_ == 0 && (bForInsert_ || cDefault.isUseOnUpdate())) {
					// create identity generator
					return Scalar::Generator::Identity::create(cEnvironment_,
															   pSchemaColumn_);
				}
			} else if (cDefault.isFunction()) {
				if (bForInsert_ || cDefault.isUseOnUpdate()) {
					// create function generator
					return Scalar::Generator::Function::create(cEnvironment_,
															   pSchemaColumn_);
				}
			} else if (cDefault.isConstant()) {
				return Scalar::Value::create(cEnvironment_,
											 Scalar::Value::DataPointer(cDefault.getConstant()),
											 Scalar::DataType(*pSchemaColumn_));
			} else {
				;
			}
		}
	}
	return 0;
}

// FUNCTION protected
//	Relation::TableImpl::Put::createInputData -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Interface::IScalar* pInput_
//	Interface::IScalar* pDefault_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
TableImpl::Put::
createInputData(Opt::Environment& cEnvironment_,
				Schema::Column* pSchemaColumn_,
				Interface::IScalar* pInput_,
				Interface::IScalar* pDefault_)
{
	if (isExplicitValueAllowed(cEnvironment_, pSchemaColumn_) == false) {
		_SYDNEY_THROW1(Exception::InvalidUpdateColumn,
					   pSchemaColumn_->getName());
	}
	Interface::IScalar* pResult = pInput_;

	Scalar::DataType cColumnType(*pSchemaColumn_);
	if (cEnvironment_.isRecovery() == false
		|| cColumnType.isLobType() == false
		|| pResult->getDataType().isObjectIDType() == false) {

		if (!Scalar::DataType::isAssignable(pResult->getDataType(),
											cColumnType)) {
			// if source data is not assignable to column type, create cast
			pResult = pResult->createCast(cEnvironment_,
										  cColumnType,
										  false /* not for comparison */);
		}
	}
	if (pSchemaColumn_->isIdentity()) {
		// insert identity node to verify data
		pResult = Scalar::Generator::Identity::create(cEnvironment_,
													  pSchemaColumn_,
													  pResult);
	} else {
		// use input data
		if (isConsiderDefault(cEnvironment_, pInput_)) {
			// if input data is DefaultData, use default instead
			if (pDefault_ == 0) {
				// use null constant
				pDefault_ = Scalar::Value::Null::create(cEnvironment_);
			}
			pResult = Scalar::Function::create(cEnvironment_,
											   Tree::Node::CoalesceDefault,
											   MAKEPAIR(pResult,
														pDefault_),
											   cColumnType,
											   STRING(pInput_->getName()));
		}
	}
	return pResult;
}

// FUNCTION protected
//	Relation::TableImpl::Put::hasInput -- has input data?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Schema::Column* pSchemaColumn_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Put::
hasInput(Opt::Environment& cEnvironment_,
		 const Schema::Column* pSchemaColumn_)
{
	return getInput(cEnvironment_, pSchemaColumn_->getPosition()) != 0;
}

// FUNCTION protected
//	Relation::TableImpl::Put::addLogColumn -- add position of logged column
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const ModVector<Schema::Column*>& vecSchemaColumn_
//	Utility::UIntSet& cLogRequiredPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
addLogColumn(Opt::Environment& cEnvironment_,
			 const ModVector<Schema::Column*>& vecSchemaColumn_,
			 Utility::UIntSet& cLogRequiredPosition_)
{
	if (Opt::IsAny(vecSchemaColumn_,
				   boost::bind(&This::hasInput,
							   this,
							   boost::ref(cEnvironment_),
							   _1))) {
		// at least one column is target of updating, all required columns should be logged
		FOREACH(vecSchemaColumn_,
				boost::bind(&Utility::UIntSet::addObject,
							&cLogRequiredPosition_,
							boost::bind(&Schema::Column::getPosition,
										_1)));
	}
}

// FUNCTION protected
//	Relation::TableImpl::Put::addReferenceByForeignKey -- add reference to other table according to constraints
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Candidate::Table*>& vecReference_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
addReferenceByForeignKey(Opt::Environment& cEnvironment_,
						 VECTOR<Candidate::Table*>& vecReference_)
{
	ModVector<Schema::Constraint*> vecConstraint =
		getSchemaTable()->getConstraint(Schema::Constraint::Category::ForeignKey,
										cEnvironment_.getTransaction());
	ReferMap cMap;
	Opt::ForEach(vecConstraint,
				 boost::bind(&This::addReference,
							 this,
							 boost::ref(cEnvironment_),
							 _1,
							 boost::ref(cMap)));
	getReference(cMap, vecReference_);
}

// FUNCTION protected
//	Relation::TableImpl::Put::addReferenceByReferedKey -- add reference to other table according to constraints
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pOperandCandidate_
//	Relation::Table* pRetrieve_
//	VECTOR<Candidate::Table*>& vecReference_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
addReferenceByReferedKey(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pOperandCandidate_,
						 Relation::Table* pRetrieve_,
						 VECTOR<Candidate::Table*>& vecReference_)
{
	ModVector<Schema::Constraint*> vecConstraint =
		getSchemaTable()->getConstraint(Schema::Constraint::Category::ReferedKey,
										cEnvironment_.getTransaction());
	ReferMap cMap;
	Opt::ForEach(vecConstraint,
				 boost::bind(&This::addReference,
							 this,
							 boost::ref(cEnvironment_),
							 _1,
							 boost::ref(cMap)));
	getReference(cMap, vecReference_);

	// key of refered constraint should be required
	Opt::ForEach(vecConstraint,
				 boost::bind(&This::requireReferedKey,
							 this,
							 boost::ref(cEnvironment_),
							 _1,
							 pOperandCandidate_,
							 pRetrieve_));
}

// FUNCTION private
//	Relation::TableImpl::Put::getColumnType -- get column type of i-th field
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Position iPosition_
//	
// RETURN
//	Scalar::DataType
//
// EXCEPTIONS

//virtual
Scalar::DataType
TableImpl::Put::
getColumnType(Opt::Environment& cEnvironment_,
			  Position iPosition_)
{
	return Scalar::DataType(*(getSchemaTable()->getColumnByPosition(
												   iPosition_,
												   cEnvironment_.getTransaction())));
}

// FUNCTION private
//	Relation::TableImpl::Put::getRowID -- get rowelement for put result of rowid
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Relation::RowElement*
//
// EXCEPTIONS

Relation::RowElement*
TableImpl::Put::
getRowID(Opt::Environment& cEnvironment_)
{
	return RowElement::create(cEnvironment_, this, 0);
}

// FUNCTION private
//	Relation::TableImpl::Put::getIdentity -- get rowelement for put result of identity
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Relation::RowElement*
//
// EXCEPTIONS

Relation::RowElement*
TableImpl::Put::
getIdentity(Opt::Environment& cEnvironment_)
{
	const Schema::Column* pIdentityColumn =
		getSchemaTable()->getIdentity(cEnvironment_.getTransaction());
	if (pIdentityColumn
		&& isAddToPutResult(cEnvironment_, pIdentityColumn)) {
		return RowElement::create(cEnvironment_,
								  this,
								  pIdentityColumn->getPosition());
	}
	return 0;;
}

// FUNCTION private
//	Relation::TableImpl::Put::getCurrentTimestamp -- get rowelement for put result of current_timestamp
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Relation::RowElement*
//
// EXCEPTIONS

Relation::RowElement*
TableImpl::Put::
getCurrentTimestamp(Opt::Environment& cEnvironment_)
{
	const ModVector<Schema::Column*>& vecSchemaColumn =
		getSchemaTable()->getColumn(cEnvironment_.getTransaction());
	ModVector<Schema::Column*>::ConstIterator iterator = vecSchemaColumn.begin();
	const ModVector<Schema::Column*>::ConstIterator last = vecSchemaColumn.end();
	for (; iterator != last; ++iterator) {
		if ((*iterator)->isCurrentTimestamp()
			&& isAddToPutResult(cEnvironment_, *iterator)) {
			// timestamp column is found
			break;
		}
	}
	if (iterator != last) {
		return RowElement::create(cEnvironment_,
								  this,
								  (*iterator)->getPosition());
	}
	return 0;
}

// FUNCTION private
//	Relation::TableImpl::Put::isAddToPutResult -- check whether a column should be added to put result
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Schema::Column* pSchemaColumn_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Put::
isAddToPutResult(Opt::Environment& cEnvironment_,
				 const Schema::Column* pSchemaColumn_)
{
	return isInsert()
		|| pSchemaColumn_->getDefault().isUseOnUpdate()
		|| hasInput(cEnvironment_, pSchemaColumn_);
}

// FUNCTION private
//	Relation::TableImpl::Put::addReference -- create refer relation corresponding to a constraint
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Constraint* pSchemaConstraint_
//	TableImpl::Put::ReferMap& cMap_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
addReference(Opt::Environment& cEnvironment_,
			 Schema::Constraint* pSchemaConstraint_,
			 ReferMap& cMap_)
{
	// add reference only when any key is assigned
	Trans::Transaction& cTrans = cEnvironment_.getTransaction();
	const ModVector<Schema::ObjectID::Value>& vecKeyID = pSchemaConstraint_->getColumnID();
	// check whether any key column is target
	if (Opt::IsAny(vecKeyID,
				   boost::bind(&This::hasField,
							   this,
							   boost::bind(&Schema::Table::getColumnByID,
										   getSchemaTable(),
										   _1,
										   boost::ref(cTrans))))) {
		// add refered table object
		Schema::Object::ID::Value iTableID = pSchemaConstraint_->getReferedTableID();
		; _SYDNEY_ASSERT(iTableID != Schema::Object::ID::Invalid);

		Schema::Table* pReferedSchemaTable =
			cEnvironment_.getDatabase()->getTable(iTableID, cTrans);

		Candidate::Table* pReferedCandidate = 0;
		ReferMap::ITERATOR found = cMap_.find(pReferedSchemaTable);
		if (found != cMap_.end()) {
			pReferedCandidate = (*found).second;

		} else {
			AccessPlan::Source cPlanSource;
			pReferedCandidate =
				_SYDNEY_DYNAMIC_CAST(Candidate::Table*,
									 Relation::Table::Refer::create(
												cEnvironment_,
												pReferedSchemaTable,
												this)
									 ->createAccessPlan(cEnvironment_,
														cPlanSource));
			cMap_.insert(pReferedSchemaTable,
						 pReferedCandidate);
		}
		pReferedCandidate->addConstraint(cEnvironment_,
										 pSchemaConstraint_);
	}
}

// FUNCTION private
//	Relation::TableImpl::Put::getReference -- get refer relations corresponding to constraints
//
// NOTES
//
// ARGUMENTS
//	const ReferMap& cMap_
//	VECTOR<Candidate::Table*>& vecReference_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
getReference(const ReferMap& cMap_,
			 VECTOR<Candidate::Table*>& vecReference_)
{
	Opt::ForEachValue(cMap_,
					  boost::bind(&VECTOR<Candidate::Table*>::PUSHBACK,
								  &vecReference_,
								  _1));
}

// FUNCTION private
//	Relation::TableImpl::Put::requireReferedKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Constraint* pSchemaConstraint_
//	Interface::ICandidate* pOperandCandidate_
//	Relation::Table* pRetrieve_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
requireReferedKey(Opt::Environment& cEnvironment_,
				  Schema::Constraint* pSchemaConstraint_,
				  Interface::ICandidate* pOperandCandidate_,
				  Relation::Table* pRetrieve_)
{
	Trans::Transaction& cTrans = cEnvironment_.getTransaction();
	const ModVector<Schema::ObjectID::Value>& vecKeyID = pSchemaConstraint_->getColumnID();
	Opt::ForEach(vecKeyID,
				 boost::bind(&Interface::IScalar::require,
							 boost::bind(&Relation::Table::getScalar,
										 pRetrieve_,
										 boost::ref(cEnvironment_),
										 boost::bind(&Schema::Column::getPosition,
													 boost::bind(&Schema::Table::getColumnByID,
																 getSchemaTable(),
																 _1,
																 boost::ref(cTrans)))),
							 boost::ref(cEnvironment_),
							 pOperandCandidate_));
}

// FUNCTION private
//	Relation::TableImpl::Put::setRetrieved -- 
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

//virtual
void
TableImpl::Put::
setRetrieved(Opt::Environment& cEnvironment_,
			 Position iPosition_)
{
	Interface::IScalar* pScalar = getScalar(cEnvironment_, iPosition_);
	; _SYDNEY_ASSERT(pScalar->isField());
	addRetrieved(pScalar->getField());
}

/////////////////////////////////////
// Relation::TableImpl::Insert

// FUNCTION public
//	Relation::TableImpl::Insert::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pColumn_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Insert::
addField(Opt::Environment& cEnvironment_,
		 Scalar::Field* pColumn_,
		 Interface::IFile* pFile_,
		 Scalar::Field* pField_)
{
	Scalar::Field* pField = pField_;
	if (pColumn_->isUpdate() == true
		&& pField_->isUpdate() == false
		&& pField_->getSchemaField()
		&& pField_->getSchemaField()->getSourceID() == Schema::Object::ID::Invalid) {
		// if source field does not exist, input data is set here
		Interface::IScalar* pInput = pColumn_->getUpdate()->getInput();
		pField = Scalar::UpdateField::create(cEnvironment_,
											 pField_,
											 pInput);
	}
	Super::addField(cEnvironment_,
					pColumn_,
					pFile_,
					pField);
}

// FUNCTION public
//	Relation::TableImpl::Insert::createAccessPlan -- create access plan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
TableImpl::Insert::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	Interface::ICandidate* pOperandCandidate = 0;
	if (getOperand()) {
		// mark operand result row info as retrieved
		Relation::RowInfo* pRowInfo = getOperand()->getRowInfo(cEnvironment_);
		if (pRowInfo) {
			pRowInfo->retrieveAll(cEnvironment_);
		}
		// create access plan
		pOperandCandidate = getOperand()->createAccessPlan(cEnvironment_, cPlanSource_);
	}

	Utility::FieldSet cTarget;
	VECTOR<Scalar::Field*> vecLogged;

	createTargetField(cEnvironment_,
					  cTarget,
					  vecLogged);

	VECTOR<Candidate::Table*> vecReference;

	if (cEnvironment_.isRecovery() == false) {
		// create object for refering other tables
		// for insert, foreign key constraint is checked
		addReferenceByForeignKey(cEnvironment_,
								 vecReference);
	}

	// create candidate object
	Candidate::Table* pResult =
		Candidate::Table::Insert::create(cEnvironment_,
										 this,
										 vecLogged,
										 vecReference,
										 pOperandCandidate);
	pResult->createPlan(cEnvironment_,
						cPlanSource_,
						cTarget);
	return pResult;
}

// FUNCTION private
//	Relation::TableImpl::Insert::createTargetField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::FieldSet& cTarget_
//	VECTOR<Scalar::Field*>& vecLogged_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Insert::
createTargetField(Opt::Environment& cEnvironment_,
				  Utility::FieldSet& cTarget_,
				  VECTOR<Scalar::Field*>& vecLogged_)
{
	int nMax = getMaxPosition(cEnvironment_);
	cTarget_.reserve(nMax);
	vecLogged_.reserve(nMax);

	bool bNeedLog = isNeedLog(cEnvironment_);

	for (int i = 0; i < nMax; i++) {
		// check specified value for i-th column
		Interface::IScalar* pInput = getInput(cEnvironment_, i);

		Schema::Column* pSchemaColumn =
			getSchemaTable()->getColumnByPosition(Schema::Column::Position(i),
												  cEnvironment_.getTransaction());
		// field object to be added
		Scalar::Field* pColumn = 0;

		bool bAddLogged = false;
		if (cEnvironment_.isRecovery()) {
			if (pInput == 0) {
				// skip this column
				continue;
			}
			if (cEnvironment_.isRollback()
				&& pSchemaColumn->isLob()) {
				// use undo
				// -> set objectid field instead
				Schema::File* pSchemaLobFile =
					pSchemaColumn->getField(cEnvironment_.getTransaction())
					->getFile(cEnvironment_.getTransaction());
				Schema::Field* pSchemaObjectID =
					pSchemaLobFile->getObjectID(cEnvironment_.getTransaction());

				pColumn = Scalar::Field::create(cEnvironment_,
												pSchemaObjectID,
												Interface::IFile::create(cEnvironment_,
																		 pSchemaLobFile),
												this);
			}
		} else {
			Interface::IScalar* pDefault = getDefault(cEnvironment_,
													  pSchemaColumn,
													  pInput,
													  true /* for insert */);
			if (pInput) {
				pInput = createInputData(cEnvironment_,
										 pSchemaColumn,
										 pInput,
										 pDefault);
				bAddLogged = bNeedLog;

			} else {
				if (pDefault == 0) {
					if (pSchemaColumn->isNullable() == false) {
						// not null column
						_SYDNEY_THROW1(Exception::NullabilityViolation,
									   pSchemaColumn->getName());
					}
					// skip this column
					continue;
				}
				pInput = pDefault;

				bAddLogged = bNeedLog && (pSchemaColumn->isTupleID() == false);
			}
		}

		// add update column to target
		if (pColumn == 0) {
			pColumn = getScalar(cEnvironment_, i)->getField();
		}
		; _SYDNEY_ASSERT(pColumn);

		// add to target column
		Scalar::Field* pUpdateField = Scalar::UpdateField::create(cEnvironment_,
																  pColumn,
																  pInput);
		cTarget_.add(pUpdateField);
		if (bAddLogged) {
			vecLogged_.PUSHBACK(pUpdateField);
		}
	}
}

/////////////////////////////////////
// Relation::TableImpl::Delete

// FUNCTION public
//	Relation::TableImpl::Delete::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pColumn_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Delete::
addField(Opt::Environment& cEnvironment_,
		 Scalar::Field* pColumn_,
		 Interface::IFile* pFile_,
		 Scalar::Field* pField_)
{
	Scalar::Field* pField = pField_;
	if (pColumn_->isUpdate() == true
		&& pField_->isUpdate() == false) {
		// if source field does not exist, input data is set here
		Interface::IScalar* pInput = 0;
		Interface::IScalar* pOriginal = pColumn_->getUpdate()->getOriginal();
		pField = Scalar::UpdateField::create(cEnvironment_,
											 pField_,
											 MAKEPAIR(pInput, pOriginal));
	}
	Super::addField(cEnvironment_,
					pColumn_,
					pFile_,
					pField);
}

// FUNCTION public
//	Relation::TableImpl::Delete::createAccessPlan -- create access plan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
TableImpl::Delete::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	// create access plan of input
	; _SYDNEY_ASSERT(getOperand());
	Interface::ICandidate* pOperandCandidate =
		getOperand()->createAccessPlan(cEnvironment_, cPlanSource_);

	Utility::FieldSet cTarget;
	VECTOR<Scalar::Field*> vecLogged;

	createTargetField(cEnvironment_,
					  pOperandCandidate,
					  cTarget,
					  vecLogged);

	VECTOR<Candidate::Table*> vecReference;

	if (cEnvironment_.isRecovery() == false) {
		// create object for refering other tables
		// for delete, refered key constraints are checked
		addReferenceByReferedKey(cEnvironment_,
								 pOperandCandidate,
								 m_pRetrieve,
								 vecReference);
	}

	// create candidate object
	Candidate::Table* pResult =
		Candidate::Table::Delete::create(cEnvironment_,
										 this,
										 m_pRetrieve,
										 vecLogged,
										 vecReference,
										 pOperandCandidate);
	pResult->createPlan(cEnvironment_,
						cPlanSource_,
						cTarget);
	return pResult;
}

// FUNCTION private
//	Relation::TableImpl::Delete::createTargetField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pOperandCandidate_
//	Utility::FieldSet& cTarget_
//	VECTOR<Scalar::Field*>& vecLogged_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Delete::
createTargetField(Opt::Environment& cEnvironment_,
				  Interface::ICandidate* pOperandCandidate_,
				  Utility::FieldSet& cTarget_,
				  VECTOR<Scalar::Field*>& vecLogged_)
{
	int nMax = getMaxPosition(cEnvironment_);
	cTarget_.reserve(nMax);
	vecLogged_.reserve(nMax);

	bool bNeedLog = isNeedLog(cEnvironment_);

	// all columns except for rowid are retrieved for log
	for (int i = 0; i < nMax; i++) {
		// get original column
		Interface::IScalar* pInput = 0;
		Interface::IScalar* pOriginal = m_pRetrieve->getScalar(cEnvironment_, i);

		if (cEnvironment_.isRollback()
			|| (pOriginal->getField() == 0
				|| pOriginal->getField()->getSchemaColumn() == 0
				|| pOriginal->getField()->getSchemaColumn()->isLob() == false)) {
			pOriginal->retrieve(cEnvironment_,
								pOperandCandidate_);
		}

		// add update column to target
		Scalar::Field* pColumn = getScalar(cEnvironment_, i)->getField();
		; _SYDNEY_ASSERT(pColumn);

		// add to target column
		Scalar::Field* pUpdateField = Scalar::UpdateField::create(cEnvironment_,
																  pColumn,
																  MAKEPAIR(pInput, pOriginal));
		cTarget_.add(pUpdateField);
		if (bNeedLog && i != 0) {
			// rowid is logged separatedly
			vecLogged_.PUSHBACK(pUpdateField);
		}
	}
}

// FUNCTION private
//	Relation::TableImpl::Delete::getRowID -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Relation::RowElement*
//
// EXCEPTIONS

//virtual
Relation::RowElement*
TableImpl::Delete::
getRowID(Opt::Environment& cEnvironment_)
{
	return RowElement::create(cEnvironment_, m_pRetrieve, 0);
}

/////////////////////////////////////
// Relation::TableImpl::Update

// FUNCTION public
//	Relation::TableImpl::Update::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pColumn_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Update::
addField(Opt::Environment& cEnvironment_,
		 Scalar::Field* pColumn_,
		 Interface::IFile* pFile_,
		 Scalar::Field* pField_)
{
	Scalar::Field* pField = pField_;
	if (pColumn_->isUpdate() == true
		&& pField_->isUpdate() == false) {
		// if source field does not exist, input data is set here
		Interface::IScalar* pInput = pColumn_->getUpdate()->getInput();
		Interface::IScalar* pOriginal = pColumn_->getUpdate()->getOriginal();
		pField = Scalar::UpdateField::create(cEnvironment_,
											 pField_,
											 MAKEPAIR(pInput, pOriginal));
	}
	Super::addField(cEnvironment_,
					pColumn_,
					pFile_,
					pField);
}

// FUNCTION public
//	Relation::TableImpl::Update::createAccessPlan -- create access plan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
TableImpl::Update::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	// create access plan of input
	; _SYDNEY_ASSERT(getOperand());

	// mark operand result row info as retrieved
	Plan::Relation::RowInfo* pRowInfo = getOperand()->getRowInfo(cEnvironment_);
	if (pRowInfo) {
		pRowInfo->retrieveAll(cEnvironment_);
	}

	Interface::ICandidate* pOperandCandidate =
		getOperand()->createAccessPlan(cEnvironment_, cPlanSource_);

	Utility::FieldSet cTarget;
	VECTOR<Scalar::Field*> vecLogged;

	// for update, multiple index's key should be logged in any case
	Utility::UIntSet cLogRequiredPosition;

	createTargetField(cEnvironment_,
					  pOperandCandidate,
					  cTarget,
					  vecLogged,
					  cLogRequiredPosition);

	VECTOR<Candidate::Table*> vecReference0;
	VECTOR<Candidate::Table*> vecReference1;

	if (cEnvironment_.isRecovery() == false) {
		// create object for refering other tables
		// for update, both foreign key and refered key constraints are checked
		addReferenceByReferedKey(cEnvironment_,
								 pOperandCandidate,
								 m_pRetrieve,
								 vecReference0);
		addReferenceByForeignKey(cEnvironment_,
								 vecReference1);
	}

	// create candidate object
	Candidate::Table* pResult =
		Candidate::Table::Update::create(cEnvironment_,
										 this,
										 m_pRetrieve,
										 vecLogged,
										 vecReference0,
										 vecReference1,
										 cLogRequiredPosition,
										 pOperandCandidate);
	pResult->createPlan(cEnvironment_,
						cPlanSource_,
						cTarget);
	return pResult;
}

// FUNCTION private
//	Relation::TableImpl::Update::createTargetField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pOperandCandidate_
//	Utility::FieldSet& cTarget_
//	VECTOR<Scalar::Field*>& vecLogged_
//	Utility::UIntSet& cLogRequiredPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Update::
createTargetField(Opt::Environment& cEnvironment_,
				  Interface::ICandidate* pOperandCandidate_,
				  Utility::FieldSet& cTarget_,
				  VECTOR<Scalar::Field*>& vecLogged_,
				  Utility::UIntSet& cLogRequiredPosition_)
{
	int nMax = getMaxPosition(cEnvironment_);
	cTarget_.reserve(nMax);
	vecLogged_.reserve(nMax);

	bool bNeedLog = isNeedLog(cEnvironment_);

	if (bNeedLog) {
		setLogColumn(cEnvironment_, cLogRequiredPosition_);
	}

	for (int i = 0; i < nMax; i++) {
		// check specified value for i-th column
		Schema::Column* pSchemaColumn =
			getSchemaTable()->getColumnByPosition(Schema::Column::Position(i),
												  cEnvironment_.getTransaction());

		_CreateTargetFieldArgument cArgument(cLogRequiredPosition_);
		cArgument.m_pInput = getInput(cEnvironment_, i);

		if (cEnvironment_.isRecovery()) {
			if (false ==
				createTargetFieldForRecovery(cEnvironment_,
											 pSchemaColumn,
											 cArgument)) {
				// skip this column
				continue;
			}
		} else {
			if (false ==
				createTargetFieldForNormal(cEnvironment_,
										   pSchemaColumn,
										   cArgument)) {
				// skip this column
				continue;
			}
		}

		createTargetFieldForBoth(cEnvironment_,
								 pSchemaColumn,
								 pOperandCandidate_,
								 cArgument);
		; _SYDNEY_ASSERT(cArgument.m_pColumn);

		// add to target column
		Scalar::Field* pUpdateField = Scalar::UpdateField::create(cEnvironment_,
																  cArgument.m_pColumn,
																  MAKEPAIR(cArgument.m_pInput,
																		   cArgument.m_pOriginal));
		if (cArgument.m_bAddTarget) {
			cTarget_.add(pUpdateField);
		}
		if (cArgument.m_bAddLogged) {
			vecLogged_.PUSHBACK(pUpdateField);
			if (cArgument.m_bDoubleLog) {
				// put to log doubly to denote it needs identity::assign in recovery
				vecLogged_.PUSHBACK(pUpdateField);
			}
		}
	}
}

// FUNCTION private
//	Relation::TableImpl::Update::setLogColumn -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::UIntSet& cLogRequiredPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Update::
setLogColumn(Opt::Environment& cEnvironment_,
			 Utility::UIntSet& cLogRequiredPosition_)
{
	// when index has more than two keys and any of those keys has input data,
	// whole keys should be logged.

	Trans::Transaction& cTrans = cEnvironment_.getTransaction();
	const ModVector<Schema::Index*>& vecSchemaIndex = getSchemaTable()->getIndex(cTrans);
	ModVector<Schema::Index*>::ConstIterator iterator = vecSchemaIndex.begin();
	const ModVector<Schema::Index*>::ConstIterator last = vecSchemaIndex.end();
	for (; iterator != last; ++iterator) {
		ModVector<Schema::Column*> vecSchemaColumn = (*iterator)->getColumn(cTrans);
		if (vecSchemaColumn.getSize() > 1) {
			addLogColumn(cEnvironment_,
						 vecSchemaColumn,
						 cLogRequiredPosition_);
		}
	}
}

// FUNCTION private
//	Relation::TableImpl::Update::createTargetFieldForRecovery -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	_CreateTargetFieldArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Update::
createTargetFieldForRecovery(Opt::Environment& cEnvironment_,
							 Schema::Column* pSchemaColumn_,
							 _CreateTargetFieldArgument& cArgument_)
{
	if (cArgument_.m_pInput == 0) {
		// skip this column
		return false;
	}

	// just add the column to target
	cArgument_.m_bAddTarget = true;

	; _SYDNEY_ASSERT(cArgument_.m_pInput);

	if (cEnvironment_.isRecovery() && pSchemaColumn_->isLob()) {

		Schema::File* pSchemaLobFile =
			pSchemaColumn_->getField(cEnvironment_.getTransaction())
			->getFile(cEnvironment_.getTransaction());
		Interface::IFile* pFile = Interface::IFile::create(cEnvironment_,
														   pSchemaLobFile);
		if (cArgument_.m_pInput->getDataType().isObjectIDType()) {
			// use undo
			// -> set objectid field instead
			Schema::Field* pSchemaObjectID =
				pSchemaLobFile->getObjectID(cEnvironment_.getTransaction());

			cArgument_.m_pColumn = Scalar::Field::create(cEnvironment_,
														 pSchemaObjectID,
														 pFile,
														 this);
			cArgument_.m_pOriginal = Scalar::Field::create(cEnvironment_,
														   pSchemaObjectID,
														   pFile,
														   m_pRetrieve);
		} else if (cArgument_.m_pInput->getDataType().getDataType() == Common::DataType::Array) {
			// use operation
			// -> convert array into operation
			Common::Data::Pointer pArray = cArgument_.m_pInput->preCalculate(cEnvironment_);
			Scalar::Field* pOperand = Scalar::Field::create(cEnvironment_,
															pSchemaColumn_,
															m_pRetrieve);
			cArgument_.m_pInput = Scalar::Operation::LogData::create(cEnvironment_,
																	 pOperand,
																	 pArray);
		}
	}
	return true;
}

// FUNCTION private
//	Relation::TableImpl::Update::createTargetFieldForNormal -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	_CreateTargetFieldArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Update::
createTargetFieldForNormal(Opt::Environment& cEnvironment_,
						   Schema::Column* pSchemaColumn_,
						   _CreateTargetFieldArgument& cArgument_)
{
	// Special operation for generator value recovery.
	cArgument_.m_bDoubleLog =
		(pSchemaColumn_->getDefault().isIdentity()
		 && ((cArgument_.m_pInput == 0
			  && pSchemaColumn_->getDefault().isUseOnUpdate())
			 || Schema::Identity::isGetMax(*pSchemaColumn_,
										   cEnvironment_.getTransaction())));

	Interface::IScalar* pDefault = getDefault(cEnvironment_,
											  pSchemaColumn_,
											  cArgument_.m_pInput,
											  false /* not for insert */);
	bool bNeedLog = isNeedLog(cEnvironment_);
	if (cArgument_.m_pInput) {
		cArgument_.m_pInput = createInputData(cEnvironment_,
											  pSchemaColumn_,
											  cArgument_.m_pInput,
											  pDefault);
		cArgument_.m_bAddTarget = true;
		cArgument_.m_bAddLogged = bNeedLog;

	} else {
		if (pSchemaColumn_->getDefault().isUseOnUpdate()) {
			// when input is not set and column has 'use-on-update' option,
			// use default value
			cArgument_.m_pInput = pDefault;
			cArgument_.m_bAddTarget = true;
			cArgument_.m_bAddLogged = bNeedLog;

		} else if (cArgument_.m_cLogRequiredPosition.isContaining(pSchemaColumn_->getPosition())) {
			// use column value for log
			cArgument_.m_bAddLogged = bNeedLog;

		} else {
			// skip this column
			return false;
		}
	}
	return true;
}

// FUNCTION private
//	Relation::TableImpl::Update::createTargetFieldForBoth -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Interface::ICandidate* pOperandCandidate_
//	_CreateTargetFieldArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Update::
createTargetFieldForBoth(Opt::Environment& cEnvironment_,
						 Schema::Column* pSchemaColumn_,
						 Interface::ICandidate* pOperandCandidate_,
						 _CreateTargetFieldArgument& cArgument_)
{
	if (cArgument_.m_pOriginal == 0) {
		// get original column
		cArgument_.m_pOriginal = m_pRetrieve->createField(cEnvironment_,
														  pSchemaColumn_);
	}
	if (cArgument_.m_bAddLogged && cArgument_.m_pInput == 0) {
		cArgument_.m_pInput = cArgument_.m_pOriginal;
	}

	if (cEnvironment_.isRecovery()
		|| pSchemaColumn_->isLob() == false) {
		// retrieve original value
		cArgument_.m_pOriginal->retrieve(cEnvironment_,
										 pOperandCandidate_);
	}

	// add update column to target
	if (cArgument_.m_pColumn == 0) {
		cArgument_.m_pColumn = getScalar(cEnvironment_, pSchemaColumn_->getPosition())->getField();
	}
}

// FUNCTION private
//	Relation::TableImpl::Update::getRowID -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Relation::RowElement*
//
// EXCEPTIONS

//virtual
Relation::RowElement*
TableImpl::Update::
getRowID(Opt::Environment& cEnvironment_)
{
	return RowElement::create(cEnvironment_, m_pRetrieve, 0);
}

/////////////////////////////////////
// Relation::TableImpl::Import

// FUNCTION public
//	Relation::TableImpl::Import::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pColumn_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Import::
addField(Opt::Environment& cEnvironment_,
		 Scalar::Field* pColumn_,
		 Interface::IFile* pFile_,
		 Scalar::Field* pField_)
{
	Scalar::Field* pField = pField_;
	if (pColumn_->isUpdate() == true
		&& pField_->isUpdate() == false
		&& pField_->getSchemaField()
		&& pField_->getSchemaField()->getSourceID() == Schema::Object::ID::Invalid) {
		// if source field does not exist, input data is set here
		Interface::IScalar* pInput = pColumn_->getUpdate()->getInput();
		pField = Scalar::UpdateField::create(cEnvironment_,
											 pField_,
											 pInput);
	}
	Super::addField(cEnvironment_,
					pColumn_,
					pFile_,
					pField);
}

// FUNCTION public
//	Relation::TableImpl::Import::createAccessPlan -- create access plan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
TableImpl::Import::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	; _SYDNEY_ASSERT(getOperand());

	// create access plan
	Interface::ICandidate* pOperandCandidate =
		getOperand()->createAccessPlan(cEnvironment_, cPlanSource_);

	int nMax = getMaxPosition(cEnvironment_);

	Utility::FieldSet cTarget;
	cTarget.reserve(nMax);

	for (int i = 0; i < nMax; i++) {
		// get target column
		Scalar::Field* pTargetField = getScalar(cEnvironment_, i)->getField();
		; _SYDNEY_ASSERT(pTargetField);
		; _SYDNEY_ASSERT(pTargetField->getSchemaField());
		// check specified value for i-th column
		Interface::IScalar* pInput = getInput(cEnvironment_, i);
		if (pInput == 0) {
			pInput = createInput(cEnvironment_,
								 pTargetField);
		}
		; _SYDNEY_ASSERT(pInput);

		// add to target column
		Scalar::Field* pUpdateField = Scalar::UpdateField::create(cEnvironment_,
																  pTargetField,
																  pInput);
		cTarget.add(pUpdateField);
	}

	VECTOR<Candidate::Table*> vecReference;
	if (m_cArgument.m_bCheckConstraint
		&& cEnvironment_.isRecovery() == false) {
		// create object for refering other tables
		// for import, foreign key constraint is checked
		addReferenceByForeignKey(cEnvironment_,
								 vecReference);
	}

	// create candidate object
	Candidate::Table* pResult =
		Candidate::Table::Import::create(cEnvironment_,
										 this,
										 vecReference,
										 pOperandCandidate);
	pResult->createPlan(cEnvironment_,
						cPlanSource_,
						cTarget);
	return pResult;
}

// FUNCTION private
//	Relation::TableImpl::Import::createInput -- create scalar node providing input value
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pTargetField_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
TableImpl::Import::
createInput(Opt::Environment& cEnvironment_,
			Scalar::Field* pTargetField_)
{
	Schema::Field* pSchemaField = pTargetField_->getSchemaField();
	; _SYDNEY_ASSERT(pSchemaField);

	Interface::IScalar* pResult = 0;

	if (pSchemaField->isObjectID()) {
		// objectid uses itself as input
		pResult = pTargetField_;

	} else {
		// search other field using source
		if (Schema::Field* pSource = pSchemaField->getSource(cEnvironment_.getTransaction())) {
			while (pSource->getSourceID() != Schema::Object::ID::Invalid) {
				pSource = pSource->getSource(cEnvironment_.getTransaction());
			}
			Scalar::Field* pOtherField = getField(pSource);
			if (pOtherField) {
				if (pOtherField->isUpdate()) {
					pResult = pOtherField->getUpdate()->getInput();
				}
			} else {
				pOtherField = createField(cEnvironment_,
										  pSource);
			}
			if (pResult == 0) {
				pResult = createInput(cEnvironment_,
									  pOtherField);
			}
		}
		if (pResult == 0) {
			// get default value using schema::column definition
			Schema::Column* pSchemaColumn = pSchemaField->getRelatedColumn(cEnvironment_.getTransaction());

			if (pSchemaColumn) {
				; _SYDNEY_ASSERT(pSchemaColumn->isTupleID() == false);
				pResult = getDefault(cEnvironment_,
									 pSchemaColumn,
									 pResult, /* == 0 */
									 true /* for insert */);
			}
			if (pResult == 0) {
				pResult = Scalar::Value::Null::create(cEnvironment_);
			}
		}
	}
	return pResult;
}

// FUNCTION private
//	Relation::TableImpl::Import::createField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Field* pSchemaField_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

Scalar::Field*
TableImpl::Import::
createField(Opt::Environment& cEnvironment_,
			Schema::Field* pSchemaField_)
{
	Schema::File* pSchemaFile = pSchemaField_->getFile(cEnvironment_.getTransaction());
	Interface::IFile* pFile = Interface::IFile::create(cEnvironment_,
													   pSchemaFile);

	return Scalar::Field::create(cEnvironment_,
								 pSchemaField_,
								 pFile,
								 this);
}

// FUNCTION private
//	Relation::TableImpl::Import::createRowInfo -- 
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

//virtual
RowInfo*
TableImpl::Import::
createRowInfo(Opt::Environment& cEnvironment_)
{
	// create result
	return RowInfo::create(cEnvironment_, this, 0, m_cArgument.m_vecTargetField.getSize());
}

// FUNCTION private
//	Relation::TableImpl::Import::createKeyInfo -- 
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

//virtual
RowInfo*
TableImpl::Import::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	// empty row
	return RowInfo::create(cEnvironment_);
}

// FUNCTION private
//	Relation::TableImpl::Import::setDegree -- 
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
TableImpl::Import::
setDegree(Opt::Environment& cEnvironment_)
{
	return m_cArgument.m_vecTargetField.getSize();
}

// FUNCTION private
//	Relation::TableImpl::Import::setMaxPosition -- 
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
TableImpl::Import::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return getDegree(cEnvironment_);
}

// FUNCTION private
//	Relation::TableImpl::Import::createScalarName -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<STRING>& vecName_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Import::
createScalarName(Opt::Environment& cEnvironment_,
				 VECTOR<STRING>& vecName_,
				 Position iPosition_)
{
	if (iPosition_ >= static_cast<Position>(m_cArgument.m_vecTargetField.getSize())) {
		return;
	}

	Opt::ExpandContainer(vecName_, m_cArgument.m_vecTargetField.getSize());
	; _SYDNEY_ASSERT(vecName_.GETSIZE() >= m_cArgument.m_vecTargetField.getSize());

	vecName_[iPosition_] = m_cArgument.m_vecTargetField[iPosition_]->getName();

}

// FUNCTION private
//	Relation::TableImpl::Import::createScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Interface::IScalar*>& vecScalar_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Import::
createScalar(Opt::Environment& cEnvironment_,
			 VECTOR<Interface::IScalar*>& vecScalar_,
			 Position iPosition_)
{
	if (iPosition_ >= static_cast<Position>(m_cArgument.m_vecTargetField.getSize())) {
		return;
	}

	Opt::ExpandContainer(vecScalar_, m_cArgument.m_vecTargetField.getSize());
	; _SYDNEY_ASSERT(vecScalar_.GETSIZE() >= m_cArgument.m_vecTargetField.getSize());

	vecScalar_[iPosition_] = createField(cEnvironment_,
										 m_cArgument.m_vecTargetField[iPosition_]);
}

// FUNCTION private
//	Relation::TableImpl::Import::createScalarType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Super::Type>& vecType_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Import::
createScalarType(Opt::Environment& cEnvironment_,
				 VECTOR<Super::Type>& vecType_,
				 Position iPosition_)
{
	// set all the columns by same type
	if (iPosition_ >= static_cast<Position>(m_cArgument.m_vecTargetField.getSize())) {
		return;
	}

	Opt::ExpandContainer(vecType_, m_cArgument.m_vecTargetField.getSize(), Tree::Node::Field);
	; _SYDNEY_ASSERT(vecType_.GETSIZE() >= m_cArgument.m_vecTargetField.getSize());
}

// FUNCTION private
//	Relation::TableImpl::Import::getColumnType -- get column type of i-th field
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Position iPosition_
//	
// RETURN
//	Scalar::DataType
//
// EXCEPTIONS

//virtual
Scalar::DataType
TableImpl::Import::
getColumnType(Opt::Environment& cEnvironment_,
			  Position iPosition_)
{
	if (iPosition_ >= static_cast<Position>(m_cArgument.m_vecTargetField.getSize())) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return Scalar::DataType(*(m_cArgument.m_vecTargetField[iPosition_]));
}

/////////////////////////////////////
// Relation::TableImpl::Undo

// FUNCTION public
//	Relation::TableImpl::Undo::createAccessPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
TableImpl::Undo::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	return Candidate::Table::Undo::create(cEnvironment_,
										  this,
										  m_pUndoLog);
}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
