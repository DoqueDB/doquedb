// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-o
// vi:set ts=4 sw=4:
//
// Scalar/Impl/FieldImpl.cpp --
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
const char moduleName[] = "Plan::Scalar::Impl";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Impl/FieldImpl.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Scalar/CheckedField.h"
#include "Plan/Scalar/Value.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Table.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IPredicate.h"

#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"
#include "Common/DoubleData.h"
#include "Common/SQLData.h"

#include "Exception/InvalidFullTextUsage.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/Argument.h"
#include "Execution/Action/Locator.h"
#include "Execution/Function/Factory.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Operator/Locator.h"
#include "Execution/Operator/SystemColumn.h"

#include "Opt/Environment.h"

#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace
{
	// function field specifications
	const struct {
		bool m_bRowIDAvailable;
		bool m_bNotNull;
	} _FunctionFieldSpec[] =
	{
	   //rowid	notnull
		{false,	false},	// Undefined
		{ true,	 true},	// Score
		{ true,	 true},	// Section
		{false,	 true},	// Word
		{false,	 true},	// WordDf
		{false,	 true},	// WordScale
		{false,	false}, // AverageLength
		{false,	false},	// AverageCharLength
		{false,	false},	// AverageWordCount
		{ true,	 true},	// Tf
		{false,	 true},	// Count
		{ true,	 true},	// ClusterId
		{ true,	 true},	// ClusterWord
		{ true,	 true},	// Kwic
		{ true,	 true},	// Existence
		{false,	false},	// MinKey
		{false,	false},	// MaxKey
		{ true,	 true},	// NeighborId
		{ true,	 true},	// NeighborDistance
	};
}

////////////////////////////////////
//	Scalar::FieldImpl::Base

// FUNCTION public
//	Scalar::FieldImpl::Base::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
FieldImpl::Base::
getField(Interface::IFile* pFile_)
{
	return m_pTable->getField(pFile_, this);
}

// FUNCTION public
//	Scalar::FieldImpl::Base::isDelayed -- 
//
// NOTES
//
// ARGUMENTS
//	Candidate::Table* pCandidate_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FieldImpl::Base::
isDelayed(Candidate::Table* pCandidate_)
{
	return pCandidate_->isDelayed(this);
}

// FUNCTION public
//	Scalar::FieldImpl::Base::getDelayedKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
FieldImpl::Base::
getDelayedKey(Opt::Environment& cEnvironment_)
{
	if (isDelayable()) {
		// rowid field is key for delayed retrieval
		Scalar::Field* pRowID = m_pTable->getScalar(cEnvironment_, 0)->getField();
		return pRowID;

	} else {
		return 0;
	}
}

// FUNCTION public
//	Scalar::FieldImpl::Base::checkRetrieve -- 
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
Field*
FieldImpl::Base::
checkRetrieve(Opt::Environment& cEnvironment_)
{
	if (m_pChecked == 0) {
		Utility::FileSet cFileSet;
		createRetrieveFile(cEnvironment_,
						   cFileSet);
		m_pChecked = CheckedField::create(cEnvironment_,
										  this,
										  cFileSet);
	}
	return m_pChecked;
}

// FUNCTION public
//	Scalar::FieldImpl::Base::checkPut -- 
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
Field*
FieldImpl::Base::
checkPut(Opt::Environment& cEnvironment_)
{
	// checkPut is called only for UpdateField
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Scalar::FieldImpl::Base::createRetrieveFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::FileSet& cFileSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FieldImpl::Base::
createRetrieveFile(Opt::Environment& cEnvironment_,
				   Utility::FileSet& cFileSet_)
{
	(void) Field::getRetrieveFile(cEnvironment_,
								  GetFileArgument(this,
												  0,
												  cFileSet_));
}

// FUNCTION public
//	Scalar::FieldImpl::Base::explain -- 
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
FieldImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	if (pEnvironment_) {
		cExplain_.put(m_pTable->getCorrelationName(*pEnvironment_)).put(".").put(getName());
	} else {
		cExplain_.put(getName());
	}
}

// FUNCTION public
//	Scalar::FieldImpl::Base::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	FieldImpl::Base::Check::Value
//
// EXCEPTIONS

//virtual
FieldImpl::Base::Check::Value
FieldImpl::Base::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	if (cArgument_.m_pCandidate
		&& cArgument_.m_pCandidate->isReferingRelation(m_pTable)) {
		return Check::Referred;
	} else if (cArgument_.m_vecPrecedingCandidate.ISEMPTY() == false) {
		VECTOR<Interface::ICandidate*>::CONSTITERATOR found =
			Opt::Find(cArgument_.m_vecPrecedingCandidate.begin(),
					  cArgument_.m_vecPrecedingCandidate.end(),
					  boost::bind(&Interface::ICandidate::isReferingRelation,
								  _1,
								  m_pTable));
		if (found != cArgument_.m_vecPrecedingCandidate.end()) {
			(*found)->require(cEnvironment_,
							  this);
			return Check::Preceding;
		}
	}
	return Check::NotYet;
}

// FUNCTION public
//	Scalar::FieldImpl::Base::isRefering -- 
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
FieldImpl::Base::
isRefering(Interface::IRelation* pRelation_)
{
	return pRelation_ == m_pTable;
}

// FUNCTION public
//	Scalar::FieldImpl::Base::isKnownNull -- 
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
FieldImpl::Base::
isKnownNull(Opt::Environment& cEnvironment_)
{
	return cEnvironment_.isKnownNull(this);
}

// FUNCTION public
//	Scalar::FieldImpl::Base::isKnownNotNull -- 
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
FieldImpl::Base::
isKnownNotNull(Opt::Environment& cEnvironment_)
{
	return cEnvironment_.isKnownNotNull(this);
}

// FUNCTION public
//	Scalar::FieldImpl::Base::hasField -- 
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
FieldImpl::Base::
hasField(Interface::IFile* pFile_)
{
	return m_pTable->getField(pFile_, this) != 0;
}

// FUNCTION public
//	Scalar::FieldImpl::Base::require -- extract refered scalar
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
FieldImpl::Base::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	if (pCandidate_) {
		pCandidate_->require(cEnvironment_,
							 this);
	} else {
		retrieve(cEnvironment_);
	}
}

// FUNCTION public
//	Scalar::FieldImpl::Base::retrieve -- 
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
FieldImpl::Base::
retrieve(Opt::Environment& cEnvironment_)
{
	getTable()->addRetrieved(this);
}

// FUNCTION public
//	Scalar::FieldImpl::Base::retrieve -- extract refered scalar
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
FieldImpl::Base::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	pCandidate_->retrieve(cEnvironment_,
						  this);
}


// FUNCTION public
//	Scalar::FieldImpl::Base::use -- 
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
FieldImpl::Base::
use(Opt::Environment& cEnvironment_,
	Interface::ICandidate* pCandidate_)
{
	pCandidate_->use(cEnvironment_,
					 this);
}

// FUNCTION public
//	Scalar::FieldImpl::Base::delay -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_,
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FieldImpl::Base::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	if (isDelayable()) {
		return pCandidate_->delay(cEnvironment_,
								  this,
								  cArgument_);
	}
	return false;
}

// FUNCTION public
//	setMetaData -- 
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
FieldImpl::Base::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	cMetaData_.setTableName(getTable()->getSchemaTable()->getName());
	cMetaData_.setTableAliasName(getTable()->getCorrelationName(cEnvironment_));
	cMetaData_.setDatabaseName(cEnvironment_.getDatabase()->getName());
}

// FUNCTION protected
//	Scalar::FieldImpl::Base::generateThis -- generate for this node
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
FieldImpl::Base::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	; _SYDNEY_ASSERT(isLob() || iDataID_ >= 0);

	if (Candidate::Table* pCandidate = getTable()->getAdoptCandidate()) {
		// clear candidate temporary
		getTable()->setAdoptCandidate(0);
		Candidate::AdoptArgument cMyArgument(cArgument_);
		cMyArgument.m_pTable = pCandidate;

		// generate fetching action if needed
		if (pCandidate->addFileFetch(cEnvironment_,
									 cProgram_,
									 pIterator_,
									 this,
									 cMyArgument) == false) {
			; _SYDNEY_ASSERT(cArgument_.m_pTable);
		}
		// set candidate again
		getTable()->setAdoptCandidate(pCandidate);
	} else {
		; _SYDNEY_ASSERT(cArgument_.m_pTable);
	}

	return iDataID_;
}

//////////////////////////////////////////////////
//	Scalar::FieldImpl::SchemaColumn

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::SchemaColumn -- constructor
//
// NOTES
//
// ARGUMENTS
//	Schema::Column* pSchemaColumn_
//	Relation::Table* pTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

FieldImpl::SchemaColumn::
SchemaColumn(Schema::Column* pSchemaColumn_,
			 Relation::Table* pTable_)
	: Super(DataType(*pSchemaColumn_),
			pTable_,
			getDelayable(pSchemaColumn_, pTable_),
			pSchemaColumn_->isTupleID()),
	  m_pSchemaColumn(pSchemaColumn_)
{}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::getSchemaField -- get corresponding schema field object
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Schema::Field*
//
// EXCEPTIONS

//virtual
Schema::Field*
FieldImpl::SchemaColumn::
getSchemaField()
{
	if (getTable()->isEstimating()) {
		; _SYDNEY_ASSERT(getTable()->getField(this));
		return getTable()->getField(this)->getSchemaField();
	}
	return 0;
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::isKnownNotNull -- 
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
isKnownNotNull(Opt::Environment& cEnvironment_)
{
	return m_pSchemaColumn->isNullable() == false
		|| Super::isKnownNotNull(cEnvironment_);
}

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
Field*
FieldImpl::SchemaColumn::
createFetchKey(Opt::Environment& cEnvironment_)
{
	Trans::Transaction& cTrans = cEnvironment_.getTransaction();
	Schema::Field* pSchemaField = getSchemaColumn()->getField(cTrans);
	ModVector<Schema::Field*> vecFetchKey =
		pSchemaField->getFile(cTrans)->getFetchKey(cTrans);

	if (vecFetchKey.getSize() == 1) {
		Schema::Field* pKeyField = *(vecFetchKey.begin());
		if (pKeyField->getRelatedColumn(cTrans) == getSchemaColumn()) {
			// fetch key for getting key itself -> no use
			return 0;
		}
		Interface::IFile* pFile =
			Interface::IFile::create(cEnvironment_,
									 pSchemaField->getFile(cTrans));
		Field* pField = Field::create(cEnvironment_,
									  pSchemaField,
									  pFile,
									  getTable(),
									  this);

		return Field::create(cEnvironment_,
							 pKeyField,
							 pFile,
							 getTable());
	}
	return 0;

}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FieldImpl::SchemaColumn::
addField(Interface::IFile* pFile_,
		 Utility::FieldSet& cFieldSet_)
{
	Field* pField = getField(pFile_);
	if (pField) {
		pField->addField(pFile_,
						 cFieldSet_);
	}
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::getName -- get scalar name
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
//	Scalar::FieldImpl::SchemaColumn::setMetaData -- set column meta data
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
Interface::IScalar*
FieldImpl::SchemaColumn::
addOption(Opt::Environment& cEnvironment_,
		  Interface::IScalar* pOption_)
{
	if (pOption_ == 0
		|| (pOption_->getType() != Tree::Node::All
			&& pOption_->getType() != Tree::Node::Expand)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	return Field::create(cEnvironment_,
						 m_pSchemaColumn,
						 getTable(),
						 this,
						 pOption_);
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::convertFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	Interface::IScalar* pFunction_
//	Schema::Field::Function::Value eFunction_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
FieldImpl::SchemaColumn::
convertFunction(Opt::Environment& cEnvironment_,
				Interface::IRelation* pRelation_,
				Interface::IScalar* pFunction_,
				Schema::Field::Function::Value eFunction_)
{
	Interface::IScalar* pResult = 0;

	if (eFunction_ != Schema::Field::Function::Undefined) {
		// get key field corresponding to the column
		ModVector<Schema::Field*> vecIndexField;
		Trans::Transaction& cTrans = cEnvironment_.getTransaction();
		if (getSchemaColumn()->getIndexField(cTrans, vecIndexField)) {
			VECTOR<Schema::Field*> vecFunctionField;
			Opt::FilterContainer(vecIndexField, vecFunctionField,
								 boost::bind(&Schema::Field::hasFunction,
											 _1,
											 eFunction_,
											 boost::ref(cTrans)));
			if (vecFunctionField.ISEMPTY() == false) {
				Utility::FileSet cFileSet;
				Opt::ForEach(vecFunctionField,
							 boost::bind(&FunctionField::createFunctionFile,
										 boost::ref(cEnvironment_),
										 _1,
										 boost::ref(cFileSet),
										 getTable(),
										 this));

				// create function field
				pResult = Field::create(cEnvironment_,
										eFunction_,
										cFileSet,
										getTable(),
										pFunction_,
										this);
			}
		}
	}
	return pResult;
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::getPosition -- 
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
int
FieldImpl::SchemaColumn::
getPosition(Interface::IRelation* pRelation_)
{
	if (pRelation_ == getTable()) {
		return getSchemaColumn()->getPosition();
	}
	return -1;
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaColumn::getValue -- get string representing contents of the node
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
	This* pThis = const_cast<This*>(this);
	if (pThis->getTable()->isEstimating()) {
		; _SYDNEY_ASSERT(pThis->getTable()->getField(pThis));
		return pThis->getTable()->getField(pThis)->getValue();
	}
	return m_pSchemaColumn->getName();
}

// FUNCTION protected
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
setArgument(const DataType& cDataType_,
			Schema::Column* pSchemaColumn_,
			Relation::Table* pTable_)
{
	Super::setArgument(cDataType_,
					   pTable_,
					   !pSchemaColumn_->isFixed(),
					   pSchemaColumn_->isTupleID());
	m_pSchemaColumn = pSchemaColumn_;
}

// FUNCTION private
//	Scalar::FieldImpl::SchemaColumn::getDelayable -- get delayability
//
// NOTES
//
// ARGUMENTS
//	Schema::Column* pSchemaColumn_
//	Relation::Table* pTable_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
FieldImpl::SchemaColumn::
getDelayable(Schema::Column* pSchemaColumn_,
			 Relation::Table* pTable_)
{
	return pSchemaColumn_->isTupleID() == false
		&& (pTable_ == 0
			|| (pTable_->getSchemaTable()->isTemporary() == false
				&& pTable_->getSchemaTable()->isSystem() == false));
}

//////////////////////////////////////////////////
//	Scalar::FieldImpl::RowID

// FUNCTION public
//	Scalar::FieldImpl::RowID::getDelayedKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
FieldImpl::RowID::
getDelayedKey(Opt::Environment& cEnvironment_)
{
	return 0;
}


//////////////////////////////////////////////////
//	Scalar::FieldImpl::LobColumn

// FUNCTION public
//	Scalar::FieldImpl::LobColumn::generate -- 
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
FieldImpl::LobColumn::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// get variable ID corresponding to the field
	int iDataID = getNodeVariable(pIterator_,
								  cArgument_);
	if (iDataID >= 0
		&& false == isUseLocator(cEnvironment_,
								 cArgument_)) {
		// already generated and value is needed
		// -> return dataID here
		return iDataID;
	}
		
	// generate using default implementation
	iDataID = Super::generate(cEnvironment_,
							  cProgram_,
							  pIterator_,
							  cArgument_);
	// get locator ID
	int iLocatorID = getLocator(cEnvironment_,
								pIterator_,
								cArgument_);
	if (iLocatorID >= 0) {
		// use locator
		if (isUseLocator(cEnvironment_,
						 cArgument_)) {
			// return locator ID instead
			return iLocatorID;
		}

		// Lob value is required -> generate getall function
		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::Locator::Get::create(
												 cProgram_,
												 pIterator_,
												 iLocatorID,
												 iDataID),
								   cArgument_.m_eTarget);
	}
	return iDataID;
}

// FUNCTION protected
//	Scalar::FieldImpl::LobColumn::generateData -- 
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
FieldImpl::LobColumn::
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	if (isUseLocator(cEnvironment_,
					 cArgument_)) {
		return -1;
	} else {
		return Super::generateData(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cArgument_);
	}
}

// FUNCTION private
//	Scalar::FieldImpl::LobColumn::isUseLocator -- use locator?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FieldImpl::LobColumn::
isUseLocator(Opt::Environment& cEnvironment_,
			 Candidate::AdoptArgument& cArgument_)
{
	return (cArgument_.m_bLocator
			|| (cArgument_.m_pTable
				&& cArgument_.m_pTable->getTable() == getTable()));
}

// FUNCTION private
//	Scalar::FieldImpl::LobColumn::getLocator -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
FieldImpl::LobColumn::
getLocator(Opt::Environment& cEnvironment_,
		   Execution::Interface::IIterator* pIterator_,
		   Candidate::AdoptArgument& cArgument_)
{
	Candidate::Table* pCandidate = getTable()->getAdoptCandidate();
	if (pCandidate == 0) {
		pCandidate = cArgument_.m_pTable;
	}
	; _SYDNEY_ASSERT(pCandidate);

	Field* pField = pCandidate->getUsedField(cEnvironment_,
											 this);
	if (pField) {
		return pField->getLocator(cEnvironment_, pIterator_);
	}
	return -1;
}

//////////////////////////////////////////////////
//	Scalar::FieldImpl::SchemaField

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::SchemaField -- constructor
//
// NOTES
//
// ARGUMENTS
//	Schema::Field* pSchemaField_
//	Interface::IFile* pFile_
//	Relation::Table* pTable_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

FieldImpl::SchemaField::
SchemaField(Schema::Field* pSchemaField_,
			Interface::IFile* pFile_,
			Relation::Table* pTable_,
			Scalar::Field* pColumn_)
	: Super(pColumn_ ? pColumn_->getDataType() : DataType(*pSchemaField_),
			pTable_,
			getDelayable(pSchemaField_, pTable_, pColumn_),
			pColumn_ && pColumn_->isRowID()),
	  m_pFile(pFile_),
	  m_pSchemaField(pSchemaField_),
	  m_pColumn(pColumn_),
	  m_cstrValue()
{}


// FUNCTION public
//	Scalar::FieldImpl::SchemaField::SchemaField -- constructor
//
// NOTES
//
// ARGUMENTS
//	Schema::Field* pSchemaField_
//	Interface::IFile* pFile_
//	Relation::Table* pTable_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

FieldImpl::SchemaField::
SchemaField(Schema::Field* pSchemaField_,
			Interface::IFile* pFile_,
			Relation::Table* pTable_,
			Scalar::Field* pColumn_,
			const DataType& cDataType_)
	: Super(cDataType_,
			pTable_,
			getDelayable(pSchemaField_, pTable_, pColumn_),
			pColumn_ && pColumn_->isRowID()),
	  m_pFile(pFile_),
	  m_pSchemaField(pSchemaField_),
	  m_pColumn(pColumn_),
	  m_cstrValue()
{}


// FUNCTION public
//	Scalar::FieldImpl::SchemaField::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
FieldImpl::SchemaField::
getField(Interface::IFile* pFile_)
{
	return pFile_ == m_pFile ? this : Super::getField(pFile_);
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::isDelayed -- 
//
// NOTES
//
// ARGUMENTS
//	Candidate::Table* pCandidate_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FieldImpl::SchemaField::
isDelayed(Candidate::Table* pCandidate_)
{
	if (m_pColumn) {
		return m_pColumn->isDelayed(pCandidate_);
	} else {
		return Super::isDelayed(pCandidate_);
	}
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::isPutKey -- 
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
FieldImpl::SchemaField::
isPutKey()
{
	if (getFile()->getSchemaFile()->getIndexID() != Schema::Object::ID::Invalid) {
		// for index files, key and data is put key
		return getSchemaField()->getCategory() == Schema::Field::Category::Key
			|| getSchemaField()->getCategory() == Schema::Field::Category::Data;
	} else {
		// for other files, key or objectid is put key
		return getSchemaField()->getCategory() == Schema::Field::Category::Key
			|| getSchemaField()->getCategory() == Schema::Field::Category::ObjectID;
	}
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::isRowIDAvailable -- 
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
FieldImpl::SchemaField::
isRowIDAvailable(Opt::Environment& cEnvironment_)
{
	switch (m_pSchemaField->getFunction()) {
	case Schema::Field::Function::Word:
	case Schema::Field::Function::WordDf:
	case Schema::Field::Function::WordScale:
		return false;
	default:
		return true;
	}
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::isKnownNull -- 
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
FieldImpl::SchemaField::
isKnownNull(Opt::Environment& cEnvironment_)
{
	return m_pColumn ? m_pColumn->isKnownNull(cEnvironment_)
		: cEnvironment_.isKnownNull(this);
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::isKnownNotNull -- 
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
FieldImpl::SchemaField::
isKnownNotNull(Opt::Environment& cEnvironment_)
{
	Schema::Field* pSource = m_pSchemaField;
	while (pSource->getSourceID() != Schema::Object::ID::Invalid) {
		pSource = pSource->getSource(cEnvironment_.getTransaction());
	}
	return (pSource->isObjectID() == false
			&& pSource->isNullable(cEnvironment_.getTransaction()) == false)
		|| (m_pColumn && m_pColumn->isKnownNotNull(cEnvironment_))
		|| Super::isKnownNotNull(cEnvironment_);
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::isUnique -- 
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
FieldImpl::SchemaField::
isUnique(Opt::Environment& cEnvironment_)
{
	return m_pColumn ? m_pColumn->isUnique(cEnvironment_)
		: isObjectID();
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::isObjectID -- 
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
FieldImpl::SchemaField::
isObjectID()
{
	return m_pSchemaField->isObjectID();
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::createFetchKey -- 
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
Field*
FieldImpl::SchemaField::
createFetchKey(Opt::Environment& cEnvironment_)
{
	Trans::Transaction& cTrans = cEnvironment_.getTransaction();
	ModVector<Schema::Field*> vecFetchKey =
		m_pSchemaField->getFile(cTrans)->getFetchKey(cTrans);

	if (vecFetchKey.getSize() == 1) {
		Schema::Field* pKeyField = *(vecFetchKey.begin());
		if (pKeyField == m_pSchemaField) {
			// fetch key for getting key itself -> no use
			return 0;
		}
		return Field::create(cEnvironment_,
							 pKeyField,
							 getFile(),
							 getTable());
	}
	return 0;
}


// FUNCTION public
//	Scalar::FieldImpl::SchemaField::isExpandElement -- 
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
FieldImpl::SchemaField::
isExpandElement()
{
	if (m_pColumn) {
		return m_pColumn->isExpandElement();
	} else {
		return Super::isExpandElement();
	}
}



// FUNCTION public
//	Scalar::FieldImpl::SchemaField::addLocator -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IIterator* pIterator_
//	int iLocatorID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FieldImpl::SchemaField::
addLocator(Opt::Environment& cEnvironment_,
		   Execution::Interface::IIterator* pIterator_,
		   int iLocatorID_)
{
	if (m_pColumn) {
		m_pColumn->addLocator(cEnvironment_,
							  pIterator_,
							  iLocatorID_);
	} else {
		Super::addLocator(cEnvironment_,
						  pIterator_,
						  iLocatorID_);
	}
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::getLocator -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
FieldImpl::SchemaField::
getLocator(Opt::Environment& cEnvironment_,
		   Execution::Interface::IIterator* pIterator_)
{
	if (m_pColumn) {
		return m_pColumn->getLocator(cEnvironment_,
									 pIterator_);
	} else {
		return Super::getLocator(cEnvironment_,
								 pIterator_);
	}
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::getName -- get scalar name
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
FieldImpl::SchemaField::
getName()
{
	if (m_pColumn) {
		return m_pColumn->getName();
	} else {
		return m_pSchemaField->getName();
	}
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::delay -- 
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
FieldImpl::SchemaField::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	if (m_pColumn) {
		return m_pColumn->delay(cEnvironment_,
								pCandidate_,
								cArgument_);
	} else {
		return Super::delay(cEnvironment_,
							pCandidate_,
							cArgument_);
	}
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::setMetaData -- set column meta data
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
FieldImpl::SchemaField::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	Super::setMetaData(cEnvironment_, cMetaData_);

	// if field is specified directly, it means function column.
	cMetaData_.setNotSearchable();
	if (!getSchemaField()->isPutable()) {
		cMetaData_.setReadOnly();
	}
	if (cEnvironment_.isKnownNotNull(this)) {
		cMetaData_.setNotNullable();
	}
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::generate -- 
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
FieldImpl::SchemaField::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	if (m_pColumn) {
		return m_pColumn->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
	} else {
		return Super::generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
	}
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::addOption -- 
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
FieldImpl::SchemaField::
addOption(Opt::Environment& cEnvironment_,
		  Interface::IScalar* pOption_)
{
	if (pOption_ == 0
		|| pOption_->getType() != Tree::Node::All) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	return Field::create(cEnvironment_,
						 this,
						 m_pColumn,
						 pOption_);
}

// FUNCTION public
//	Scalar::FieldImpl::SchemaField::getValue -- get string representing contents of the node
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
FieldImpl::SchemaField::
getValue() const
{
	if (m_cstrValue.getLength() == 0) {
		OSTRSTREAM cStream;
		cStream << m_pSchemaField->getPosition();
		const_cast<This&>(*this).m_cstrValue = cStream.getString();
	}
	return m_cstrValue;
}




// FUNCTION protected
//	Scalar::FieldImpl::SchemaField::setArgument -- 
//
// NOTES
//
// ARGUMENTS
//	const DataType& cDataType_
//	Schema::Field* pSchemaField_
//	Interface::IFile* pFile_
//	Relation::Table* pTable_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FieldImpl::SchemaField::
setArgument(const DataType& cDataType_,
			Schema::Field* pSchemaField_,
			Interface::IFile* pFile_,
			Relation::Table* pTable_,
			Scalar::Field* pColumn_)
{
	Super::setArgument(cDataType_,
					   pTable_,
					   !pSchemaField_->isFunction() && !pSchemaField_->isFixed(),
					   pColumn_ && pColumn_->isRowID());
	m_pFile = pFile_;
	m_pSchemaField = pSchemaField_;
	m_pColumn = pColumn_;
}


// FUNCTION private
//	Scalar::FieldImpl::SchemaField::getDelayable -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Field* pSchemaField_
//	Relation::Table* pTable_
//	Scalar::Field* pColumn_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
FieldImpl::SchemaField::
getDelayable(Schema::Field* pSchemaField_,
			 Relation::Table* pTable_,
			 Scalar::Field* pColumn_)
{
	return pSchemaField_->isFunction() == false
		&& (pColumn_ == 0 || pColumn_->isDelayable())
		&& (pTable_ == 0
			|| (pTable_->getSchemaTable()->isTemporary() == false
				&& pTable_->getSchemaTable()->isSystem() == false));
}

//////////////////////////////////////////
//	Scalar::FieldImpl::BitSetField

// FUNCTION public
//	Scalar::FieldImpl::BitSetField::BitSetField -- constructor
//
// NOTES
//
// ARGUMENTS
//	Schema::Field* pSchemaField_
//	Interface::IFile* pFile_
//	Relation::Table* pTable_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

FieldImpl::BitSetField::
BitSetField(Schema::Field* pSchemaField_,
			Interface::IFile* pFile_,
			Relation::Table* pTable_,
			Scalar::Field* pColumn_)
	: Super(pSchemaField_,
			pFile_,
			pTable_,
			pColumn_,
			DataType(Common::DataType::BitSet))
{}


// FUNCTION public
//	Scalar::FieldImpl::BitSetField::generate -- 
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
FieldImpl::BitSetField::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{

	if (cArgument_.m_bForceRowIDSet) {
		return Super::generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
	} else {
		return Super::Super::generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
	}
}



//////////////////////////////////////////
//	Scalar::FieldImpl::SystemColumn

// FUNCTION protected
//	Scalar::FieldImpl::SystemColumn::generateThis -- 
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
FieldImpl::SystemColumn::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	; _SYDNEY_ASSERT(m_pRowID);

	int iRowIDID = m_pRowID->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);

	// add function
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::SystemColumn::create(
									 cProgram_,
									 pIterator_,
									 getSchemaColumn(),
									 iRowIDID,
									 iDataID_),
							   cArgument_.m_eTarget);
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
FunctionField(Schema::Field::Function::Value eFunction_,
			  const Utility::FileSet& cFileSet_,
			  Relation::Table* pTable_,
			  Interface::IScalar* pFunction_,
			  Interface::IScalar* pOperand_)
	: Super(pFunction_->getDataType(),
			pTable_,
			false,
			false),
	  m_eFunction(eFunction_),
	  m_cFileSet(cFileSet_),
	  m_pFunction(pFunction_),
	  m_pOperand(pOperand_)
{}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::createFunctionFile -- create FileSet for function field
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Field* pSchemaField_
//	Utility::FileSet& cFileSet_
//	Relation::Table* pTable_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
FieldImpl::FunctionField::
createFunctionFile(Opt::Environment& cEnvironment_,
				   Schema::Field* pSchemaField_,
				   Utility::FileSet& cFileSet_,
				   Relation::Table* pTable_,
				   Interface::IScalar* pOperand_)
{
	Schema::File* pSchemaFile =
		pSchemaField_->getFile(cEnvironment_.getTransaction());
	Interface::IFile* pFile =
		Interface::IFile::create(cEnvironment_,
								 pSchemaFile);
	cFileSet_.add(pFile);

	; _SYDNEY_ASSERT(pOperand_->isField());
	// create corresponding field in the file
	(void)Field::create(cEnvironment_,
						pSchemaField_,
						pFile,
						pTable_,
						pOperand_->getField());
}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
FieldImpl::FunctionField::
getField(Interface::IFile* pFile_)
{
	if (Interface::IPredicate* pPredicate = getTable()->getEstimatePredicate()) {
		if (checkAvailability(pPredicate) == false) {
			return 0;
		}
	}
	return m_cFileSet.isContaining(pFile_) ? this : Super::getField(pFile_);
}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::isRowIDAvailable -- 
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
	return _FunctionFieldSpec[m_eFunction].m_bRowIDAvailable;
}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::hasAlternativeValue -- 
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
hasAlternativeValue(Opt::Environment& cEnvironment_)
{
	// corresponding contains predicate exists, alternative value is available
	return (m_eFunction == Schema::Field::Function::Score)
		&& (cEnvironment_.getContains(m_pOperand).getSize() > 0);
}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::getAlternativeValue -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
FieldImpl::FunctionField::
getAlternativeValue(Opt::Environment& cEnvironment_)
{
	if (hasAlternativeValue(cEnvironment_)) {
		// alternative value is 0.
		return Scalar::Value::create(cEnvironment_,
									 new Common::DoubleData(0.0));
	}
	return 0;
}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::createFetchKey -- 
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
Field*
FieldImpl::FunctionField::
createFetchKey(Opt::Environment& cEnvironment_)
{
	// never fetched
	return 0;
}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::getFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
FieldImpl::FunctionField::
getFunction()
{
	return m_pFunction;
}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::checkAvailability -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FieldImpl::FunctionField::
checkAvailability(Interface::IPredicate* pPredicate_)
{
	switch (m_pFunction->getType()) {
	case Tree::Node::Tf:
	case Tree::Node::ClusterID:
	case Tree::Node::FeatureValue:
	case Tree::Node::Score:
	case Tree::Node::Section:
	case Tree::Node::Word:
	case Tree::Node::WordDf:
	case Tree::Node::WordScale:
		{
			// used field in predicate should be same as this
			Utility::FieldSet cPredicateField;
			Utility::FieldSet cFunctionField;
			pPredicate_->getUsedField(cPredicateField);
			m_pFunction->getUsedField(cFunctionField);

			return cPredicateField == cFunctionField;
		}
	default:
		{
			return true;
		}
	}

}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::createRetrieveFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::FileSet& cFileSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FieldImpl::FunctionField::
createRetrieveFile(Opt::Environment& cEnvironment_,
				   Utility::FileSet& cFileSet_)
{
	cFileSet_.merge(m_cFileSet);
}

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
//	Scalar::FieldImpl::FunctionField::hasField -- 
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
FieldImpl::FunctionField::
hasField(Interface::IFile* pFile_)
{
	return m_cFileSet.isContaining(pFile_);
}

// FUNCTION public
//	Scalar::FieldImpl::FunctionField::setMetaData -- 
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
FieldImpl::FunctionField::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	Base::setMetaData(cEnvironment_,
					  cMetaData_);

	cMetaData_.setNotSearchable();
	cMetaData_.setReadOnly();
	if (_FunctionFieldSpec[m_eFunction].m_bNotNull) {
		cMetaData_.setNotNullable();
	}
}

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
Interface::IScalar*
FieldImpl::FunctionField::
addOption(Opt::Environment& cEnvironment_,
		  Interface::IScalar* pOption_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
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
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION protected
//	Scalar::FieldImpl::FunctionField::generateData -- 
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
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	int iResult = Field::Super::generateData(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cArgument_);

	if (Interface::IScalar* pAlternative = getAlternativeValue(cEnvironment_)) {
		int iResetData = pAlternative->generate(cEnvironment_,
												cProgram_,
												pIterator_,
												cArgument_);
		// add as used data with reset data
		pIterator_->useVariable(iResult, iResetData);
	} else {
		pIterator_->useVariable(iResult);
	}
	return iResult;
}

//////////////////////////////////////////
//	Scalar::FieldImpl::GetMax

// FUNCTION public
//	Scalar::FieldImpl::GetMax::GetMax -- constructor
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<Scalar::Field*>& vecOperand_
//	Relation::Table* pTable_
//	Interface::IScalar* pNullData_
//	const STRING& cstrName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

// constructor
FieldImpl::GetMax::
GetMax(const VECTOR<Scalar::Field*>& vecOperand_,
	   Relation::Table* pTable_,
	   Interface::IScalar* pNullData_,
	   const STRING& cstrName_)
  : Super(vecOperand_),
	m_pNullData(pNullData_),
	m_cstrName(cstrName_)
{
	setArgument(vecOperand_[0]->getDataType(),
				pTable_,
				false, // not delayable
				false);// not rowid
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
FieldImpl::GetMax::
getField(Interface::IFile* pFile_)
{
	Iterator iterator = begin();
	const Iterator last = end();
	for (; iterator != last; ++iterator) {
		if (Scalar::Field* pField = (*iterator)->getField(pFile_)) {
			return pField;
		}
	}
	return 0;
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::isRowIDAvailable -- 
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
FieldImpl::GetMax::
isRowIDAvailable(Opt::Environment& cEnvironment_)
{
	return isAll(boost::bind(&Scalar::Field::isRowIDAvailable,
							 _1,
							 boost::ref(cEnvironment_)));
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::hasAlternativeValue -- 
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
FieldImpl::GetMax::
hasAlternativeValue(Opt::Environment& cEnvironment_)
{
	return isAll(boost::bind(&Scalar::Field::hasAlternativeValue,
							 _1,
							 boost::ref(cEnvironment_)));
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::getAlternativeValue -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
FieldImpl::GetMax::
getAlternativeValue(Opt::Environment& cEnvironment_)
{
	// use first operand
	return getOperandi(0)->getAlternativeValue(cEnvironment_);
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::createFetchKey -- 
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
Field*
FieldImpl::GetMax::
createFetchKey(Opt::Environment& cEnvironment_)
{
	// never fetched
	return 0;
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::getFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
FieldImpl::GetMax::
getFunction()
{
	Interface::IFile* pFile = getTable()->getEstimateFile();
	if (pFile) {
		if (getField(pFile)) {
			return getField(pFile)->getOriginal();
		}
		return m_pNullData;
	}
	return getOperandi(0);
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::checkAvailability -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FieldImpl::GetMax::
checkAvailability(Interface::IPredicate* pPredicate_)
{
	return isAny(boost::bind(&Scalar::Field::checkAvailability,
							 _1,
							 pPredicate_));
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::createRetrieveFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::FileSet& cFileSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FieldImpl::GetMax::
createRetrieveFile(Opt::Environment& cEnvironment_,
				   Utility::FileSet& cFileSet_)
{
	foreachOperand(boost::bind(&Scalar::Field::createRetrieveFile,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cFileSet_)));
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::getName -- 
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
FieldImpl::GetMax::
getName()
{
	return m_cstrName;
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::hasField -- 
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
FieldImpl::GetMax::
hasField(Interface::IFile* pFile_)
{
	return isAny(boost::bind(&Scalar::Field::hasField,
							 _1,
							 pFile_));
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::setMetaData -- 
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
FieldImpl::GetMax::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	// use first operand
	getOperandi(0)->setMetaData(cEnvironment_,
								cMetaData_);
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::addOption -- 
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
FieldImpl::GetMax::
addOption(Opt::Environment& cEnvironment_,
		  Interface::IScalar* pOption_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Scalar::FieldImpl::GetMax::getValue -- 
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
FieldImpl::GetMax::
getValue() const
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION protected
//	Scalar::FieldImpl::GetMax::generateData -- 
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
FieldImpl::GetMax::
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	return Field::Super::generateData(cEnvironment_,
									  cProgram_,
									  pIterator_,
									  cArgument_);
}

// FUNCTION protected
//	Scalar::FieldImpl::GetMax::generateThis -- 
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
FieldImpl::GetMax::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	// generate operand
	MapResult<int> vecOperandID;
	mapOperand(vecOperandID,
			   boost::bind(&Scalar::Field::generate,
						   _1,
						   boost::ref(cEnvironment_),
						   boost::ref(cProgram_),
						   pIterator_,
						   boost::ref(cArgument_)));
	// add function
	pIterator_->addCalculation(cProgram_,
							   Execution::Function::Factory::create(
									   cProgram_,
									   pIterator_,
									   Tree::Node::GetMax,
									   vecOperandID,
									   iDataID_),
							   cArgument_.m_eTarget);
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
			 Relation::Table* pTable_,
			 Scalar::Field* pColumn_,
			 Interface::IScalar* pOption_)
	: Super(pColumn_, pOption_)
{
	setArgument((isArbitraryElement()||isExpandElement())
				? DataType::getElementType(DataType(*pSchemaColumn_))
				: DataType(*pSchemaColumn_),
				pSchemaColumn_,
				pTable_);
}

//////////////////////////////////////////
//	Scalar::FieldImpl::OptionField

// FUNCTION public
//	Scalar::FieldImpl::OptionField::OptionField -- constructor
//
// NOTES
//
// ARGUMENTS
//	Schema::Field* pSchemaField_
//	Interface::IFile* pFile_
//	Relation::Table* pTable_
//	Scalar::Field* pColumn_
//	Interface::IScalar* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

FieldImpl::OptionField::
OptionField(Schema::Field* pSchemaField_,
			Interface::IFile* pFile_,
			Relation::Table* pTable_,
			Scalar::Field* pColumn_,
			Interface::IScalar* pOption_)
	: Super(pColumn_, pOption_)
{
	setArgument(pColumn_->getDataType(),
				pSchemaField_,
				pFile_,
				pTable_,
				pColumn_);
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
