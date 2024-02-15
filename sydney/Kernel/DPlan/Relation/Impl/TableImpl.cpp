// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Impl/TableImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Relation";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DPlan/Relation/Impl/TableImpl.h"
#include "DPlan/Candidate/Table.h"
#include "DPlan/Scalar/Field.h"
#include "DPlan/Scalar/UpdateField.h"

#include "Common/Assert.h"
#include "Common/DataInstance.h"
#include "Communication/Protocol.h"

#include "Exception/InvalidUpdateColumn.h"
#include "Exception/NotSupported.h"
#include "Exception/NullabilityViolation.h"
#include "Exception/ReadOnlyPartition.h"
#include "Exception/Unexpected.h"

#include "Opt/Environment.h"
#include "Opt/NameMap.h"

#include "Plan/Relation/Argument.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Scalar/Generator.h"
#include "Plan/Scalar/Invoke.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Sql/Table.h"

#include "Schema/Column.h"
#include "Schema/Default.h"
#include "Schema/Index.h"
#include "Schema/Partition.h"
#include "Schema/Table.h"


_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

////////////////////////////////////////////////
//	Plan::Relation::TableImpl::Base
// FUNCTION public
//	Relation::TableImpl::Base::registerToEnvironment -- register to environment
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
TableImpl::Base::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	Super::registerToEnvironment(cEnvironment_);
}





// FUNCTION public
//	Relation::TableImpl::Base::createField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	Plan::Scalar::Field*
//
// EXCEPTIONS

//virtual
Plan::Scalar::Field*
TableImpl::Base::
createField(Opt::Environment& cEnvironment_,
			Schema::Column* pSchemaColumn_)
{
	return DPlan::Scalar::Field::create(cEnvironment_,
										pSchemaColumn_,
										this);
}

// FUNCTION public
//	Relation::TableImpl::Base::getEstimateFile -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Plan::Interface::IFile*
//
// EXCEPTIONS

//virtual
Plan::Interface::IFile*
TableImpl::Base::
getEstimateFile()
{
	return 0;
}

////////////////////////////////////////////////
//	DPlan::Relation::TableImpl::Retrieve


// FUNCTION public
//	Relation::TableImpl::Retrieve::createAccessPlan -- create access dPlan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Plan::Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Plan::Interface::ICandidate*
TableImpl::Retrieve::
createAccessPlan(Opt::Environment& cEnvironment_,
				 Plan::AccessPlan::Source& cPlanSource_)
{
	Candidate::Table* pResult = 0;
	Schema::Partition* pRule = getSchemaTable()->getPartition(cEnvironment_.getTransaction());
	if (pRule) {
		// distributed -> obtain from all servers
		pResult = Candidate::Table::Distribute::Retrieve::create(cEnvironment_,
																 this);
	} else {
		// replicated -> obtain from any one server
		pResult = Candidate::Table::Replicate::Retrieve::create(cEnvironment_,
																this);
	}


	return pResult->createPlan(cEnvironment_,
						cPlanSource_,
						m_cFieldSet);

}



// FUNCTION public
//	Relation::TableImpl::Retrieve::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Plan::Relation::InquiryArgument& cArgument_
//	
// RETURN
//	Plan::Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation::InquiryResult
TableImpl::Retrieve::
inquiry(Opt::Environment& cEnvironment_,
		const Plan::Relation::InquiryArgument& cArgument_)
{
	InquiryResult iResult = 0;
	if (cArgument_.m_iTarget & Plan::Relation::InquiryArgument::Target::Distributed) {
		Schema::Partition* pRule =
			getSchemaTable()->getPartition(cEnvironment_.getTransaction());
		if (pRule) {
			iResult |= Plan::Relation::InquiryArgument::Target::Distributed;
		}
	}
	return iResult;
}

// FUNCTION public
//	Relation::TableImpl::Retrieve::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
//	
// RETURN
//	Plan::Interface::ISqlNode*
//
// EXCEPTIONS

//virtual
Plan::Sql::Query*
TableImpl::Retrieve::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Table* pTable =
		Plan::Sql::Table::create(cEnvironment_,
								 getSchemaTable()->getName(),
								 getCorrelationName(cEnvironment_));
	
	Plan::Sql::Query* pResult =
		Plan::Sql::Query::create(cEnvironment_,
								 Plan::Sql::Query::SELECT,
								 pTable,
								 getSchemaTable()->getPartition(cEnvironment_.getTransaction()) != 0);
	
	return pResult;
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
addRetrieved(Plan::Scalar::Field* pField_)
{
	m_cFieldSet.add(pField_);
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
	Plan::Interface::IScalar* pScalar = getScalar(cEnvironment_, iPosition_);
	; _SYDNEY_ASSERT(pScalar->isField());
	m_cFieldSet.add(pScalar->getField());
}

/////////////////////////////////////
// Relation::TableImpl::Put

// FUNCTION public
//	Relation::TableImpl::Put::addInput -- 
//
// NOTES
//	
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
		 Plan::Interface::IScalar* pSource_)
{
	Super::addInput(cEnvironment_,
					iPosition_,
					pSource_);
	
	Plan::Interface::IScalar* pInput = getLocalInput(cEnvironment_,
													 iPosition_,
													 pSource_);
	if (pInput && m_pLocalTable) {
		m_pLocalTable->addInput(cEnvironment_,
								iPosition_,
								pInput);
	}
}

// FUNCTION public
//	Relation::TableImpl::Put::getLocalInput -- 
//
// NOTES
//	UniqueなカラムとNullを許容しないカラムは分散マネージャ上のファイルにも記録する。
//	Nullを許容しないカラムは、カラの値を分散マネージャ上に記録する.
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::RowElement* pElement_
//	Interface::IScalar* pSource_
//	
// RETURN
//	Interface::IScalar* pScalar
//
// EXCEPTIONS

//virtual
Plan::Interface::IScalar*
TableImpl::Put::
getLocalInput(Opt::Environment& cEnvironment_,
			  Position iPosition_,
			  Plan::Interface::IScalar* pSource_)
{
	if (m_cUniqueColumn.none()) {
		const ModVector<Schema::Index*>& vecIndex =
			getSchemaTable()->getIndex(cEnvironment_.getTransaction());
		
		ModVector<Schema::Index*>::ConstIterator ite =vecIndex.begin();
		ModVector<Schema::Index*>::ConstIterator end =vecIndex.end();
		for (; ite != end; ++ite) {
			if ((*ite)->isUnique()) {
				ModVector<Schema::Column*> vecColumn =
					(*ite)->getColumn(cEnvironment_.getTransaction());
				Opt::ForEach(vecColumn,
							 boost::bind(&Common::BitSet::set,
										 boost::ref(m_cUniqueColumn),
										 boost::bind(&Schema::Column::getPosition,
													 _1),
										 true));
			}
		}
	}

	Plan::Interface::IScalar* pResult = 0;
	if (m_cUniqueColumn.test(iPosition_)) {
		if (m_pLocalTable == 0) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		pResult = pSource_;
	} else if (!getSchemaTable()->
			   getColumnByPosition(iPosition_,
								   cEnvironment_.getTransaction())->isNullable()) {
		Schema::Column* pColumn = getSchemaTable()->
			getColumnByPosition(iPosition_, cEnvironment_.getTransaction());
		Common::Data::Pointer pData = Common::DataInstance::create(pColumn->getType());
		pResult = Plan::Scalar::Value::create(cEnvironment_, pData);
	}
	return pResult;
}


// FUNCTION public
//	Relation::TableImpl::Put::registerToEnvironment -- register to environment
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
TableImpl::Put::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	Super::registerToEnvironment(cEnvironment_);
}


// FUNCTION public
//	Relation::TableImpl::Put::createField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	Plan::Scalar::Field*
//
// EXCEPTIONS

//virtual
Plan::Scalar::Field*
TableImpl::Put::
createField(Opt::Environment& cEnvironment_,
			Schema::Column* pSchemaColumn_)
{
	return DPlan::Scalar::Field::create(cEnvironment_,
										pSchemaColumn_,										
										this);
}


// FUNCTION protected
//	Relation::TableImpl::Put::getDefault -- get iscalar denoting default/generated value for the i-th column
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Plan::Interface::IScalar* pInput_
//	bool bForInsert_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
TableImpl::Put::
getDefault(Opt::Environment& cEnvironment_,
		   Schema::Column* pSchemaColumn_,
		   Plan::Interface::IScalar* pInput_,
		   bool bForInsert_)
{
	if (pSchemaColumn_->isTupleID() == false) {
		const Schema::Default& cDefault = pSchemaColumn_->getDefault();
		if (cDefault.isNull() == false) {
			if (cDefault.isIdentity()) {
				if (pInput_ == 0 && (bForInsert_ || cDefault.isUseOnUpdate())) {
					// create identity generator
					return Plan::Scalar::Generator::Identity::create(cEnvironment_,
																	 pSchemaColumn_);
				}
			} else if (cDefault.isFunction()) {
				if (bForInsert_ || cDefault.isUseOnUpdate()) {
					// create function generator
					return Plan::Scalar::Generator::Function::create(cEnvironment_,
																	 pSchemaColumn_);
				}
			} else if (cDefault.isConstant()) {
				return Plan::Scalar::Value::create(cEnvironment_,
												   Plan::Scalar::Value::DataPointer(cDefault.getConstant()),
												   Plan::Scalar::DataType(*pSchemaColumn_));
			}
		}
	}
	return 0;
}

// FUNCTION protected
//	Relation::TableImpl::Put::createInputData -- create input scalar data when data is specified
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Plan::Interface::IScalar* pInput_
//	Plan::Interface::IScalar* pDefault_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
TableImpl::Put::
createInputData(Opt::Environment& cEnvironment_,
				Schema::Column* pSchemaColumn_,
				Plan::Interface::IScalar* pInput_,
				Plan::Interface::IScalar* pDefault_)
{
	if (isExplicitValueAllowed(cEnvironment_, pSchemaColumn_) == false) {
		_SYDNEY_THROW1(Exception::InvalidUpdateColumn,
					   pSchemaColumn_->getName());
	}
	Plan::Interface::IScalar* pResult = pInput_;

	Plan::Scalar::DataType cColumnType(*pSchemaColumn_);
	if (pSchemaColumn_->isIdentity()) {
		// insert identity node to verify data
		pResult = Plan::Scalar::Generator::Identity::create(cEnvironment_,
															pSchemaColumn_,
															pResult);
	}
	return pResult;
}

// FUNCTION protected
//	Relation::TableImpl::Put::createCheck -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Partition* pRule_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
TableImpl::Put::
createCheck(Opt::Environment& cEnvironment_,
			Schema::Partition* pRule_,
			VECTOR<Plan::Scalar::Field*>& cTarget_)
{
	const ModVector<Schema::Object::ID::Value>& vecColumn = pRule_->getTarget().m_vecColumn;
	ModVector<Schema::Object::ID::Value>::ConstIterator ite = vecColumn.begin();
	
	// convert columnid into scalar
	VECTOR<Plan::Interface::IScalar*> vecOperand;
	for (; ite != vecColumn.end(); ++ite) {
		int pos = getSchemaTable()->
			getColumnByID((*ite), cEnvironment_.getTransaction())->getPosition();
		_SYDNEY_ASSERT(pos < cTarget_.GETSIZE() + 1); /* skip row id */
		vecOperand.PUSHBACK(cTarget_[pos-1]);
	}

	// check function invoker
	const Schema::ObjectName& cFunctionName = pRule_->getTarget().m_cFunctionName;
	return Plan::Scalar::Invoke::create(cEnvironment_,
										cFunctionName,
										vecOperand,
										cFunctionName);
}

/////////////////////////////////////
// Relation::TableImpl::Insert

// FUNCTION public
//	Relation::TableImpl::Insert::createAccessPlan -- create access dPlan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Plan::Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Plan::Interface::ICandidate*
TableImpl::Insert::
createAccessPlan(Opt::Environment& cEnvironment_,
				 Plan::AccessPlan::Source& cPlanSource_)
{
	Plan::Interface::ICandidate* pOperandCandidate = 0;
	if (getOperand()) {
		// mark operand result row info as retrieved
		Plan::Relation::RowInfo* pRowInfo = getOperand()->getRowInfo(cEnvironment_);
		if (pRowInfo) {
			pRowInfo->retrieveAll(cEnvironment_);
		}
		// create access plan
		pOperandCandidate = getOperand()->createAccessPlan(cEnvironment_, cPlanSource_);
	}



	VECTOR<Plan::Scalar::Field*> cTarget;
	createTargetField(cEnvironment_,
					  cTarget);

	Candidate::Table* pTableCandidate = 0;
	Schema::Partition* pRule = getSchemaTable()->getPartition(cEnvironment_.getTransaction());
	if (pRule) {
		if (pRule->getCategory() == Schema::Partition::Category::ReadOnly) {
			// insertion is not allowed for a table with read only partition
			_SYDNEY_THROW1(Exception::ReadOnlyPartition, getSchemaTable()->getName());
		}

		// create scalar to get partition rule check result
		Plan::Interface::IScalar* pCheck = createCheck(cEnvironment_,
													   pRule,
													   cTarget);

		// distributed -> insert according to rule
		pTableCandidate =
			Candidate::Table::Distribute::Insert::create(cEnvironment_,
														 this,
														 pOperandCandidate,
														 pCheck,
														 m_bRelocateUpdate);
	} else {
		//  replicated -> insert same thing to all servers
		pTableCandidate =
			Candidate::Table::Replicate::Insert::create(cEnvironment_,
														this,
														pOperandCandidate);
	}

	Plan::Utility::FieldSet cTargetSet(cTarget);
	return pTableCandidate->createPlan(cEnvironment_,
									   cPlanSource_,
									   cTargetSet);
}


// FUNCTION public
//	Relation::TableImpl::Insert::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
//	
// RETURN
//	Plan::Interface::ISqlNode*
//
// EXCEPTIONS

//virtual
Plan::Sql::Query*
TableImpl::Insert::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Table* pTable =
		Plan::Sql::Table::create(cEnvironment_,
								 getSchemaTable()->getName(),
								 getCorrelationName(cEnvironment_));	
	Plan::Sql::Query* pResult =
		Plan::Sql::Query::create(cEnvironment_,
								 Plan::Sql::Query::INSERT,
								 pTable,
								 getSchemaTable()->getPartition(cEnvironment_.getTransaction()) != 0);
	
	return pResult;
}

// FUNCTION public
//	Relation::TableImpl::Insert::getGeneratedColumn -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Plan::Relation::RowInfo*
//
// EXCEPTIONS

//virtual
Plan::Relation::RowInfo*
TableImpl::Insert::
getGeneratedColumn(Opt::Environment& cEnvironment_)
{
	return Plan::Relation::RowInfo::create(cEnvironment_);
}


// FUNCTION private
//	Relation::TableImpl::Insert::createTargetField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Plan::Scalar::Field*> cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Insert::
createTargetField(Opt::Environment& cEnvironment_,
				  VECTOR<Plan::Scalar::Field*>& cTarget_)
{
	int nMax = getMaxPosition(cEnvironment_);
	cTarget_.reserve(nMax);

	for (int i = 0; i < nMax; i++) {
		// check specified value for i-th column
		Plan::Interface::IScalar* pInput = getInput(cEnvironment_, i);

		Schema::Column* pSchemaColumn =
			getSchemaTable()->getColumnByPosition(Schema::Column::Position(i),
												  cEnvironment_.getTransaction());
		// field object to be added
		Plan::Interface::IScalar* pDefault = getDefault(cEnvironment_,
														pSchemaColumn,
														pInput,
														true /* for insert */);
		if (pInput) {
			pInput = createInputData(cEnvironment_,
									 pSchemaColumn,
									 pInput,
									 pDefault);
		} else {
			if (pDefault == 0) {
				if (pSchemaColumn->isNullable() == false
					&& !pSchemaColumn->isTupleID()) {
					// not null column
					_SYDNEY_THROW1(Exception::NullabilityViolation,
								   pSchemaColumn->getName());
				}
				// skip this column
				continue;
			}
			pInput = pDefault;
		}
		if (i==0) continue; // rowid はパラメータチェックのみ

		// add update column to target
		Plan::Scalar::Field* pColumn = getScalar(cEnvironment_, i)->getField();
		; _SYDNEY_ASSERT(pColumn);

		// add to target column
		Plan::Scalar::Field* pUpdateField =
			DPlan::Scalar::UpdateField::create(cEnvironment_,
											   pColumn,
											   pInput);
		cTarget_.PUSHBACK(pUpdateField);
	}
}



/////////////////////////////////////
// Relation::TableImpl::Update

// FUNCTION public
//	Relation::TableImpl::Update::createAccessPlan -- create access dPlan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Plan::Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Plan::Interface::ICandidate*
TableImpl::Update::
createAccessPlan(Opt::Environment& cEnvironment_,
				 Plan::AccessPlan::Source& cPlanSource_)
{
	Plan::Interface::ICandidate* pOperandCandidate = 0;
	if (getOperand()) {
		// mark operand result row info as retrieved
		Plan::Relation::RowInfo* pRowInfo = getOperand()->getRowInfo(cEnvironment_);
		if (pRowInfo) {
			pRowInfo->retrieveAll(cEnvironment_);
		}
		// create access plan
		pOperandCandidate = getOperand()->createAccessPlan(cEnvironment_, cPlanSource_);
	}

	VECTOR<Plan::Scalar::Field*> cTarget;
	createTargetField(cEnvironment_,
					  cTarget);
	
	Candidate::Table* pTableCandidate =
		Candidate::Table::Distribute::Update::create(cEnvironment_,
													 this,
													 pOperandCandidate);

	Plan::Utility::FieldSet cTargetSet(cTarget);
	return pTableCandidate->createPlan(cEnvironment_,
									   cPlanSource_,
									   cTargetSet);
}


// FUNCTION public
//	Relation::TableImpl::Update::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
//	
// RETURN
//	Plan::Interface::ISqlNode*
//
// EXCEPTIONS

//virtual
Plan::Sql::Query*
TableImpl::Update::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pOpeQuery = m_pRetrieve->generateSQL(cEnvironment_);
	Plan::Sql::Query* pResult =
		Plan::Sql::Query::create(cEnvironment_,
								 Plan::Sql::Query::UPDATE,
								 pOpeQuery,
								 getSchemaTable()->getPartition(cEnvironment_.getTransaction()) != 0);

	
	return pResult;
}



// FUNCTION public
//	Relation::TableImpl::Update::getGeneratedColumn -- 
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

Plan::Relation::RowInfo*
TableImpl::Update::
getGeneratedColumn(Opt::Environment& cEnvironment_)
{
	int iProtocolVersion = cEnvironment_.getProtocolVersion();

	// create generated columns using specified protocol version
	Plan::Relation::RowInfo* pResult = Plan::Relation::RowInfo::create(cEnvironment_);

	// use identity, current_timestamp
	if (Plan::Relation::RowElement* pRowElement = getIdentity(cEnvironment_)) {
		pResult->PUSHBACK(Plan::Relation::RowInfo::Element(STRING(), pRowElement));
	} else {
		// use null constant instead
		pResult->PUSHBACK(Plan::Relation::RowInfo::Element(
							  "<identity>",
							  Plan::Relation::RowElement::create(cEnvironment_,
																 0,
																 0,
																 Plan::Scalar::Value::create(cEnvironment_, Plan::Scalar::DataType::getIntegerType(), "identity"))));
	}
	if (Plan::Relation::RowElement* pRowElement = getCurrentTimestamp(cEnvironment_)) {
		pResult->PUSHBACK(Plan::Relation::RowInfo::Element(STRING(), pRowElement));
	} else {
		// use null constant instead
		pResult->PUSHBACK(Plan::Relation::RowInfo::Element(
							  "<current_timestamp>",
							  Plan::Relation::RowElement::create(cEnvironment_,
																 0,
																 0,
																 Plan::Scalar::Value::create(cEnvironment_, Plan::Scalar::DataType::getDateType(), "current_timestamp"))));

		// add rowid
		pResult->PUSHBACK(
			Plan::Relation::RowInfo::Element(STRING(),
											 Plan::Relation::RowElement::create(cEnvironment_, this, 0)));
	}
	return pResult;
}


// FUNCTION private
//	Relation::TableImpl::Update::createTargetField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Plan::Scalar::Field*> cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Update::
createTargetField(Opt::Environment& cEnvironment_,
				  VECTOR<Plan::Scalar::Field*>& cTarget_)
{
	int nMax = getMaxPosition(cEnvironment_);
	cTarget_.reserve(nMax);


	for (int i = 0; i < nMax; i++) {
		// check specified value for i-th column
		Plan::Interface::IScalar* pInput = getInput(cEnvironment_, i);

		Schema::Column* pSchemaColumn =
			getSchemaTable()->getColumnByPosition(Schema::Column::Position(i),
												  cEnvironment_.getTransaction());
		// field object to be added
		Plan::Interface::IScalar* pDefault = getDefault(cEnvironment_,
														pSchemaColumn,
														pInput,
														false /* for insert */);
		if (pInput) {
			pInput = createInputData(cEnvironment_,
									 pSchemaColumn,
									 pInput,
									 pDefault);
		} else {
			if (pSchemaColumn->getDefault().isUseOnUpdate()) {
				pInput = pDefault;
			} else {
				continue;
			}
		}
		if (i==0) continue; // rowid はパラメータチェックのみ

		Plan::Interface::IScalar* pOriginal = m_pTable->createField(cEnvironment_,
																	pSchemaColumn);
		
		// add update column to target
		Plan::Scalar::Field* pColumn = getScalar(cEnvironment_, i)->getField();
		; _SYDNEY_ASSERT(pColumn);

		// add to target column
		Plan::Scalar::Field* pUpdateField =
			DPlan::Scalar::UpdateField::create(cEnvironment_,
											   pColumn,
											   MAKEPAIR(pInput, pOriginal));
		cTarget_.PUSHBACK(pUpdateField);
	}
}




/////////////////////////////////////
// Relation::TableImpl::

// FUNCTION public
//	Relation::TableImpl::Delete::createAccessPlan -- create access dPlan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Plan::Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Plan::Interface::ICandidate*
TableImpl::Delete::
createAccessPlan(Opt::Environment& cEnvironment_,
				 Plan::AccessPlan::Source& cPlanSource_)
{
	
	Candidate::Table* pResult = Candidate::Table::Distribute::Delete::create(cEnvironment_,
																			 this);
	return pResult->createPlan(cEnvironment_,
							   cPlanSource_,
							   m_cFieldSet);
}

// FUNCTION public
//	Relation::TableImpl::Delete::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Plan::Relation::InquiryArgument& cArgument_
//	
// RETURN
//	Plan::Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation::InquiryResult
TableImpl::Delete::
inquiry(Opt::Environment& cEnvironment_,
		const Plan::Relation::InquiryArgument& cArgument_)
{
	InquiryResult iResult = 0;
	if (cArgument_.m_iTarget & Plan::Relation::InquiryArgument::Target::Distributed) {
		Schema::Partition* pRule =
			getSchemaTable()->getPartition(cEnvironment_.getTransaction());
		if (pRule) {
			iResult |= Plan::Relation::InquiryArgument::Target::Distributed;
		}
	}
	return iResult;
}


// FUNCTION public
//	Relation::TableImpl::Delete::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
//	
// RETURN
//	Plan::Interface::ISqlNode*
//
// EXCEPTIONS

//virtual
Plan::Sql::Query*
TableImpl::Delete::
generateSQL(Opt::Environment& cEnvironment_)
{

	Plan::Sql::Query* pOpeQuery = m_pRetrieve->generateSQL(cEnvironment_);
	Plan::Sql::Query* pResult =
		Plan::Sql::Query::create(cEnvironment_,
								 Plan::Sql::Query::DEL,
								 pOpeQuery,
								 getSchemaTable()->getPartition(cEnvironment_.getTransaction()) != 0);
	m_cFieldSet.foreachElement(boost::bind(&Plan::Interface::IScalar::retrieveFromCascade,
										   _1,
										   boost::ref(cEnvironment_),
										   pResult));
	
	return pResult;
}

// FUNCTION public
//	Relation::TableImpl::Delete::getGeneratedColumn -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Plan::Relation::RowInfo*
//
// EXCEPTIONS

//virtual

Plan::Relation::RowInfo*
TableImpl::Delete::
getGeneratedColumn(Opt::Environment& cEnvironment_)
{
	Plan::Relation::RowInfo* pResult = Plan::Relation::RowInfo::create(cEnvironment_);
	pResult->PUSHBACK(
		Plan::Relation::RowInfo::Element(STRING(),
										 Plan::Relation::RowElement::create(cEnvironment_, this, 0)));
	return pResult;
}


// FUNCTION public
//	Relation::TableImpl::Delete::addRetrieved -- 
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
TableImpl::Delete::
addRetrieved(Plan::Scalar::Field* pField_)
{
	m_cFieldSet.add(pField_);
}


// FUNCTION private
//	Relation::TableImpl::Delete::setRetrieved -- set retrieved flag
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
TableImpl::Delete::
setRetrieved(Opt::Environment& cEnvironment_,
			 Position iPosition_)
{

}





_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
