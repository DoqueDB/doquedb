// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/FieldImpl.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Scalar::Impl";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DPlan/Scalar/Impl/FieldImpl.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"

#include "Plan/Scalar/Aggregation.h"
#include "Plan/Sql/Argument.h"
#include "Plan/Tree/Node.h"

#include "Schema/Column.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_SCALAR_BEGIN

////////////////////////////////////////
// Scalar::FieldImpl::Base::

// FUNCTION public
//	Scalar::FieldImpl::Base::registerToEnvironment -- register to environment
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
FieldImpl::Base::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	Super::registerToEnvironment(cEnvironment_);
}

////////////////////////////////////////
// Scalar::FieldImpl::SchemaColumn::

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::SchemaColumn -- constructor
//
// NOTES
//
// ARGUMENTS
//	Schema::Column* pSchemaColumn_
//	Plan::Relation::Table* pTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

FieldImpl::SchemaColumn::
SchemaColumn(Schema::Column* pSchemaColumn_,
			 Plan::Relation::Table* pTable_)
	: Super(Plan::Scalar::DataType(*pSchemaColumn_),
			pTable_,
			false /* not delayable */,
			pSchemaColumn_->isTupleID()),
	  m_pSchemaColumn(pSchemaColumn_)

{}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::isUnique -- 
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
FieldImpl::SchemaColumn::
isUnique(Opt::Environment& cEnvironment_)
{
	return m_pSchemaColumn->isUnique(cEnvironment_.getTransaction());
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::isRowIDAvailable -- 
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
FieldImpl::SchemaColumn::
isRowIDAvailable(Opt::Environment& cEnvironment_)
{
	return true;
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::createFetchKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//virtual
Plan::Scalar::Field*
FieldImpl::SchemaColumn::
createFetchKey(Opt::Environment& cEnvironment_)
{
	return 0;
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IFile* pFile_
//	Plan::Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FieldImpl::SchemaColumn::
addField(Plan::Interface::IFile* pFile_,
		 Plan::Utility::FieldSet& cFieldSet_)
{
	; // do nothing
}



// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::getName -- 
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
FieldImpl::SchemaColumn::
getName()
{
	return m_pSchemaColumn->getName();
}



// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::setMetaData -- 
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
FieldImpl::SchemaColumn::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	Super::setMetaData(cEnvironment_, cMetaData_);

	// set metadata from schema column
	getSchemaColumn()->setMetaData(cEnvironment_.getTransaction(), cMetaData_);
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::addOption -- 
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
Plan::Interface::IScalar*
FieldImpl::SchemaColumn::
addOption(Opt::Environment& cEnvironment_,
		  Plan::Interface::IScalar* pOption_)
{
	if (pOption_ == 0
		|| (pOption_->getType() != Plan::Tree::Node::All
			&& pOption_->getType() != Plan::Tree::Node::Expand)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	return DPlan::Scalar::Field::create(cEnvironment_,
										m_pSchemaColumn,
										getTable(),
										this,
										pOption_);
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::getValue -- 
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
FieldImpl::SchemaColumn::
getValue() const
{
	return m_pSchemaColumn->getName();
}


// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
STRING
FieldImpl::SchemaColumn::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	Plan::Relation::Table* pTable = const_cast<This*>(this)->getTable();
	if (pTable &&
		cArgument_.m_bNeedRelationForColumn) {

		if (pTable->getCorrelationName(cEnvironment_).getLength() > 0) {
			cStream << pTable->getCorrelationName(cEnvironment_);
		}  else {
			cStream << pTable->getSchemaTable()->getName();
		}
		cStream << ".";
	}

	cStream << const_cast<This*>(this)->getName();
	return cStream.getString();
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::setParameter -- 
//
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
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
FieldImpl::SchemaColumn::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	
	if (cArgument_.m_bNeedRelationForColumn) 
		if (getCorrelationName().getLength() > 0) {
			cExec_.append(getCorrelationName());
			cExec_.append(".");
		} else 	if (getTable()) {
			if (getTable()->getCorrelationName(cEnvironment_).getLength() > 0) {
				cExec_.append(getTable()->getCorrelationName(cEnvironment_));
			}  else {
				cExec_.append(getTable()->getSchemaTable()->getName());
			}
			cExec_.append(".");
		}

	
	cExec_.append(getName());

}


// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::SchemaColumn -- 
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

FieldImpl::SchemaColumn::
SchemaColumn()
	: Super(),
	  m_pSchemaColumn(0)
{}


// FUNCTION protected
//	Scalar::FieldImpl::SchemaColumn::setArgument -- 
//
// NOTES
//
// ARGUMENTS
//	const DataType& cDataType_
//	Schema::Column* pSchemaColumn_
//	Relation::Table* pTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FieldImpl::SchemaColumn::
setArgument(const Plan::Scalar::DataType& cDataType_,
			Schema::Column* pSchemaColumn_,
			Plan::Relation::Table* pTable_)
{
	Super::setArgument(cDataType_,
					   pTable_,
					   !pSchemaColumn_->isFixed(),
					   pSchemaColumn_->isTupleID());
	m_pSchemaColumn = pSchemaColumn_;
}

// FUNCTION protected
//	Scalar::FieldImpl::SchemaColumn::generateThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Plan::Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
FieldImpl::SchemaColumn::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Plan::Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	return iDataID_;
}


//////////////////////////////////////////
//	Scalar::FieldImpl::FunctionField

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::FunctionField -- constructor
//
// NOTES
//
// ARGUMENTS
//	Schema::Field::Function::Value eFunction_
//	const Utility::FileSet& cFileSet_,
//	Relation::Table* pTable_
//	Interface::IScalar* pFunction_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

FieldImpl::FunctionField::
FunctionField(Plan::Relation::Table* pTable_,
			  Plan::Interface::IScalar* pFunction_)
	: Super(pFunction_->getDataType(),
			pTable_,
			false,
			false),
	  m_pFunction(pFunction_),
	  m_pConvertedForDist(0)
{}





// FUNCTION public
//	Scalar::FieldImpl::FunctionField::addOption -- 
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
Plan::Interface::IScalar*
FieldImpl::FunctionField::
addOption(Opt::Environment& cEnvironment_,
		  Plan::Interface::IScalar* pOption_)
{
	return Super::addOption(cEnvironment_, pOption_);
}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::getValue -- 
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
FieldImpl::FunctionField::
getValue() const
{
	// never called
	_SYDNEY_THROW0(Exception::NotSupported);
}


// FUNCTION public
//	Scalar::FieldImpl::FunctionField::FunctionField -- 
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

FieldImpl::FunctionField::
FunctionField()
	: Super(),
	  m_pFunction(0),
	  m_pConvertedForDist(0)
{}




// FUNCTION public
//	Scalar::FieldImpl::FunctionField::getName -- 
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
FieldImpl::FunctionField::
getName()
{
	return m_pFunction->getName();
}


// FUNCTION public
//	Scalar::FieldImpl::FunctionField::getID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
FieldImpl::FunctionField::
getID() const
{
	if (m_pConvertedForDist)
		return m_pConvertedForDist->getID();
	else 
		return Super::getID();
}




// FUNCTION public
//	Scalar::FieldImp::FunctionField::generate -- 
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
FieldImpl::FunctionField::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Plan::Candidate::AdoptArgument& cArgument_)
{
	if (m_pConvertedForDist) {
		Plan::Interface::IScalar* pTemp = m_pConvertedForDist;
		m_pConvertedForDist = 0;
		int iDataID = pTemp->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
		m_pConvertedForDist = pTemp;
		return  iDataID;
	}
	
	return Super::generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
}


// FUNCTION public
//	Scalar::FieldImpl:FunctionField::isRowIDAvailable -- 
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
FieldImpl::FunctionField::
isRowIDAvailable(Opt::Environment& cEnvironment_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}





// FUNCTION protected
//	Scalar::FieldImpl::SchemaColumn::generateThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Plan::Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
FieldImpl::FunctionField::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Plan::Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	return iDataID_;
}


//////////////////////////////////////////
//	Scalar::FieldImpl::OptionColumn

// FUNCTION public
//	Scalar::FieldImpl::OptionColumn::OptionColumn -- constructor
//
// NOTES
//
// ARGUMENTS
//	Schema::Column* pSchemaColumn_
//	Relation::Table* pTable_
//	Scalar::Field* pColumn_
//	Interface::IScalar* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

FieldImpl::OptionColumn::
OptionColumn(Schema::Column* pSchemaColumn_,
			 Plan::Relation::Table* pTable_,
			 Plan::Scalar::Field* pColumn_,
			 Plan::Interface::IScalar* pOption_)
	: Super(pColumn_, pOption_)
{
	setArgument((isArbitraryElement()||isExpandElement())
				? Plan::Scalar::DataType::getElementType(Plan::Scalar::DataType(*pSchemaColumn_))
				: Plan::Scalar::DataType(*pSchemaColumn_),
				pSchemaColumn_,
				pTable_);
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::setParameter -- 
//
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
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
FieldImpl::OptionColumn::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	if (isExpandElement() &&
		cArgument_.m_bProjectionColumn) {
		cExec_.append("grouping_element()");
		return;
	}
	
	Super::setParameter(cEnvironment_,
						cProgram_,
						pIterator_,
						cExec_,
						cArgument_);
	
	if (isExpandElement()||isArbitraryElement())
		cExec_.append("[]");
	
}




_SYDNEY_DPLAN_SCALAR_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
