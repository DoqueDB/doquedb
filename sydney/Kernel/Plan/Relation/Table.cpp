// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Table.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#include "Plan/Relation/Table.h"
#include "Plan/Relation/Impl/TableImpl.h"

#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Candidate/Table.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/PrepareFailed.h"
#include "Exception/ReadOnlyDatabase.h"
#include "Exception/SystemTable.h"
#include "Exception/TableNotFound.h"
#include "Exception/Unexpected.h"

#include "Opt/Environment.h"
#include "Opt/NameMap.h"

#include "Schema/Column.h"
#include "Schema/Database.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Table.h"

#include "Trans/Transaction.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

///////////////////////////////////////////
//	Plan::Relation::Table::Retrieve

// FUNCTION public
//	Relation::Table::Retrieve::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrTableName_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Retrieve::
create(Opt::Environment& cEnvironment_, const STRING& cstrTableName_)
{
	Schema::Table* pSchemaTable =
		getSchemaTable(cEnvironment_, cstrTableName_);

	return create(cEnvironment_, pSchemaTable);
}

// FUNCTION public
//	Relation::Table::Retrieve::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Retrieve::
create(Opt::Environment& cEnvironment_,
	   Schema::Table* pSchemaTable_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Retrieve(pSchemaTable_);

	pResult->registerToEnvironment(cEnvironment_);

	This* pThis = pResult.release();
	return pThis;
}

///////////////////////////////////////////
//	Plan::Relation::Table::Refer

// FUNCTION public
//	Relation::Table::Refer::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Table* pSchemaTable_
//	Relation::Table* pTargetTable_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Refer::
create(Opt::Environment& cEnvironment_,
	   Schema::Table* pSchemaTable_,
	   Relation::Table* pTargetTable_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Refer(pSchemaTable_, pTargetTable_);
	pResult->registerToEnvironment(cEnvironment_);

	return pResult.release();
}

///////////////////////////////////////////
//	Plan::Relation::Table::Insert

// FUNCTION public
//	Relation::Table::Insert::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrTableName_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Insert::
create(Opt::Environment& cEnvironment_,
	   const STRING& cstrTableName_,
	   Interface::IRelation* pOperand_)
{
	Schema::Table* pSchemaTable =
		getSchemaTable(cEnvironment_, cstrTableName_);

	checkUpdatability(cEnvironment_, pSchemaTable);

	AUTOPOINTER<This> pResult = new TableImpl::Insert(pSchemaTable,
													  pOperand_);
	pResult->registerToEnvironment(cEnvironment_);

	This* pThis = pResult.release();

	pThis->setCorrelationName(cEnvironment_,
							  STRING(),
							  VECTOR<STRING>());

	// add table as insert target
	cEnvironment_.setInsertTable(pSchemaTable);

	return pThis;
}

///////////////////////////////////////////
//	Plan::Relation::Table::Delete

// FUNCTION public
//	Relation::Table::Delete::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pRetrieve_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Delete::
create(Opt::Environment& cEnvironment_,
	   Relation::Table* pRetrieve_,
	   Interface::IRelation* pOperand_)
{
	checkUpdatability(cEnvironment_, pRetrieve_->getSchemaTable());

	AUTOPOINTER<This> pResult = new TableImpl::Delete(pRetrieve_,
													  pOperand_);
	pResult->registerToEnvironment(cEnvironment_);

	This* pThis = pResult.release();

	pThis->setCorrelationName(cEnvironment_,
							  STRING(),
							  VECTOR<STRING>());

	// add table as update target
	cEnvironment_.setUpdateTable(pRetrieve_->getSchemaTable());

	return pThis;
}

///////////////////////////////////////////
//	Plan::Relation::Table::Update

// FUNCTION public
//	Relation::Table::Update::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pRetrieve_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Update::
create(Opt::Environment& cEnvironment_,
	   Relation::Table* pRetrieve_,
	   Interface::IRelation* pOperand_)
{
	checkUpdatability(cEnvironment_, pRetrieve_->getSchemaTable());

	AUTOPOINTER<This> pResult = new TableImpl::Update(pRetrieve_,
													  pOperand_);
	pResult->registerToEnvironment(cEnvironment_);

	This* pThis = pResult.release();

	pThis->setCorrelationName(cEnvironment_,
							  STRING(),
							  VECTOR<STRING>());

	// add table as update target
	cEnvironment_.setUpdateTable(pRetrieve_->getSchemaTable());

	return pThis;
}

///////////////////////////////////////////
//	Plan::Relation::Table::Import

// FUNCTION public
//	Relation::Table::Import::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Opt::ImportArgument& cArgument_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Import::
create(Opt::Environment& cEnvironment_,
	   const Opt::ImportArgument& cArgument_,
	   Interface::IRelation* pOperand_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Import(cArgument_,
													  pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

///////////////////////////////////////////
//	Plan::Relation::Table::Undo

// FUNCTION public
//	Relation::Table::Undo::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Table* pSchemaTable_
//	const Common::DataArrayData* pUndoLog_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Undo::
create(Opt::Environment& cEnvironment_,
	   Schema::Table* pSchemaTable_,
	   const Common::DataArrayData* pUndoLog_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Undo(pSchemaTable_,
													pUndoLog_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

////////////////////////////////////
//	Plan::Relation::Table

// FUNCTION public
//	Relation::Table::getSchemaTable -- get schema table object from name
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrTableName_
//	
// RETURN
//	Schema::Table*
//
// EXCEPTIONS

//static
Schema::Table*
Table::
getSchemaTable(Opt::Environment& cEnvironment_,
			   const STRING& cstrTableName_)
{
	// Obtain schema table object from name
	Schema::Table* pSchemaTable =
		cEnvironment_.getDatabase()->getTable(cstrTableName_,
											  cEnvironment_.getTransaction());
	if (!pSchemaTable) {
		// table not fonud
		if (cEnvironment_.isPrepare()
			&& Schema::Table::isToBeTemporary(cstrTableName_)) {
			_SYDNEY_THROW0(Exception::PrepareFailed);
		} else {
			_SYDNEY_THROW2(Exception::TableNotFound,
						   cstrTableName_, cEnvironment_.getDatabase()->getName());
		}
	}
	return pSchemaTable;
}

// FUNCTION public
//	Relation::Table::createField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Field* pSchemaField_
//	Interface::IFile* pFile_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

Scalar::Field*
Table::
createField(Opt::Environment& cEnvironment_,
			Schema::Field* pSchemaField_,
			Interface::IFile* pFile_,
			Scalar::Field* pColumn_)
{
	return Scalar::Field::create(cEnvironment_,
								 pSchemaField_,
								 pFile_,
								 this,
								 pColumn_);
}

// FUNCTION public
//	Relation::Table::createField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
Table::
createField(Opt::Environment& cEnvironment_,
			Schema::Column* pSchemaColumn_)
{
	return Scalar::Field::create(cEnvironment_,
								 pSchemaColumn_,
								 this);
}

// FUNCTION public
//	Relation::Table::setEstimateFile -- 
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
Table::
setEstimateFile(Interface::IFile* pFile_)
{
	return AutoReset(this, &Table::resetEstimateFile);
}

// FUNCTION public
//	Relation::Table::getEstimateFile -- 
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
Table::
getEstimateFile()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Relation::Table::resetEstimateFile -- 
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
Table::
resetEstimateFile()
{
	; // do nothing
}

// FUNCTION public
//	Relation::Table::setEstimatePredicate -- 
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
Table::
setEstimatePredicate(Interface::IPredicate* pPredicate_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Relation::Table::getEstimatePredicate -- 
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
Table::
getEstimatePredicate()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Relation::Table::resetEstimatePredicate -- 
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
Table::
resetEstimatePredicate()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Relation::Table::isEstimating -- 
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
Table::
isEstimating()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Relation::Table::getField -- 
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
Table::
getField(Scalar::Field* pColumn_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Relation::Table::setEstimateCount -- set estimate count cache
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
Table::
setEstimateCount(const AccessPlan::Cost::Value& cValue_)
{
	; // donothing
}

// FUNCTION public
//	Relation::Table::getEstimateCount -- get estimate count cache
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
Table::
getEstimateCount()
{
	return AccessPlan::Cost::Value();
}

// FUNCTION public
//	Relation::Table::setAdoptCandidate -- 
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
Table::
setAdoptCandidate(Candidate::Table* pCandidate_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Relation::Table::getAdoptCandidate -- 
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
Table::
getAdoptCandidate()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Relation::Table::isNeedLog -- table operation is needed to log?
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

bool
Table::
isNeedLog(Opt::Environment& cEnvironment_)
{
	// Even if transaction don't need log,
	// when database is recovery full mode, logical log is needed for roll forward recovery.
	return cEnvironment_.isRecovery() == false
		&& (cEnvironment_.getTransaction().isNecessaryLog() == true
			|| cEnvironment_.getDatabase()->isRecoveryFull() == true)
		&& getSchemaTable()->isTemporary() == false;
}

// FUNCTION public
//	Relation::Table::addInput -- add column <-> input data correspondence
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Position iPosition_
//	Interface::IScalar* pSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Table::
addInput(Opt::Environment& cEnvironment_,
		 Position iPosition_,
		 Interface::IScalar* pSource_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Relation::Table::getInput -- get input data from column
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
Table::
getInput(Opt::Environment& cEnvironment_,
		 Position iPosition_)
{
	return 0;
}

// FUNCTION public
//	Relation::Table::getGeneratedColumn -- get generate columns for result to client
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
Table::
getGeneratedColumn(Opt::Environment& cEnvironment_)
{
	return 0;
}

// FUNCTION public
//	Relation::Table::setCorrelationName -- 
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
Table::
setCorrelationName(Opt::Environment& cEnvironment_,
				   const STRING& cstrTableName_,
				   const VECTOR<STRING>& vecColumnName_)
{
	// create normal columns by setCorrelationName
	if (cstrTableName_.getLength() == 0) {
		Super::setCorrelationName(cEnvironment_,
								  m_pSchemaTable->getName(),
								  VECTOR<STRING>());
	} else {
		Super::setCorrelationName(cEnvironment_,
								  cstrTableName_,
								  vecColumnName_);
	}
	// add special columns
	Opt::NameMap* pNameMap = cEnvironment_.getNameMap();
	Opt::NameMap::NameScalarMap& cMap = pNameMap->getMap(this);
	// add rowid
	Schema::Column* pRowID = m_pSchemaTable->getTupleID(cEnvironment_.getTransaction());
	; _SYDNEY_ASSERT(pRowID);
	cMap[pRowID->getName()] = RowElement::create(cEnvironment_, this, 0);
}

// FUNCTION protected
//	Relation::Table::Table -- constructor
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

Table::
Table()
	: Super(Tree::Node::Table),
	  m_pSchemaTable(0)
{}

// FUNCTION protected
//	Relation::Table::Table -- constructor
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* cSchemaTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Table::
Table(Schema::Table* pSchemaTable_)
	: Super(Tree::Node::Table),
	  m_pSchemaTable(pSchemaTable_)
{}

// FUNCTION protected
//	Relation::Table::setSchemaTable -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
setSchemaTable(Schema::Table* pSchemaTable_)
{
	m_pSchemaTable = pSchemaTable_;
}

// FUNCTION protected
//	Relation::Table::checkUpdatability -- check updatability
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Table::
checkUpdatability(Opt::Environment& cEnvironment_,
				  Schema::Table* pSchemaTable_)
{
	if (pSchemaTable_->isSystem()) {
		// system table can't be updated
		_SYDNEY_THROW1(Exception::SystemTable,
					   pSchemaTable_->getName());
	}
	Schema::Database* pSchemaDatabase = pSchemaTable_->getDatabase(cEnvironment_.getTransaction());
	if (pSchemaDatabase->isReadOnly()) {
		_SYDNEY_THROW1(Exception::ReadOnlyDatabase, pSchemaDatabase->getName());
	}
}

// FUNCTION protected
//	Relation::Table::registerToEnvironment -- register to environment
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
Table::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	// add as table
	cEnvironment_.addTable(this);
	if (cEnvironment_.checkStatus(Opt::Environment::Status::Subquery)) {
		// add as table refered in subquery
		cEnvironment_.addSubqueryTable(getSchemaTable());
	}

	// use super class
	Super::registerToEnvironment(cEnvironment_);
}

// FUNCTION private
//	Relation::Table::createRowInfo -- set result row spec
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
Table::
createRowInfo(Opt::Environment& cEnvironment_)
{
	// obtain columns
	const ModVector<Schema::Column*>& vecColumn = m_pSchemaTable->getColumn(cEnvironment_.getTransaction());

	// create result (start from 1 to ignore rowid(0))
	return RowInfo::create(cEnvironment_, this, 1, vecColumn.getSize());
}

// FUNCTION private
//	Relation::Table::createKeyInfo -- 
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
Table::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	// rowid
	return RowInfo::create(cEnvironment_, this, 0, 1);
}

// FUNCTION pivate
//	Relation::Table::setDegree -- set degree of the relation
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
Table::
setDegree(Opt::Environment& cEnvironment_)
{
	// degree is the number of columns except for rowid
	return m_pSchemaTable->getColumn(cEnvironment_.getTransaction()).getSize() - 1;
}

// FUNCTION public
//	Relation::Table::setMaxPosition -- set max position of the relation
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
Table::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return getDegree(cEnvironment_) + 1;
}

// FUNCTION private
//	Relation::Table::createScalarName -- set scalar name
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
Table::
createScalarName(Opt::Environment& cEnvironment_,
				 VECTOR<STRING>& vecName_,
				 Position iPosition_)
{
	// obtain columns
	const ModVector<Schema::Column*>& vecColumn = m_pSchemaTable->getColumn(cEnvironment_.getTransaction());
	if (iPosition_ >= static_cast<Position>(vecColumn.getSize())) {
		return;
	}

	Opt::ExpandContainer(vecName_, vecColumn.getSize());
	; _SYDNEY_ASSERT(vecName_.GETSIZE() >= vecColumn.getSize());

	vecName_[iPosition_] = vecColumn[iPosition_]->getName();
}

// FUNCTION private
//	Relation::Table::createScalar -- set scalar interface
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
Table::
createScalar(Opt::Environment& cEnvironment_,
			 VECTOR<Interface::IScalar*>& vecScalar_,
			 Position iPosition_)
{
	// obtain columns
	const ModVector<Schema::Column*>& vecColumn = m_pSchemaTable->getColumn(cEnvironment_.getTransaction());
	if (iPosition_ >= static_cast<Position>(vecColumn.getSize())) {
		return;
	}

	Opt::ExpandContainer(vecScalar_, vecColumn.getSize());
	; _SYDNEY_ASSERT(vecScalar_.GETSIZE() >= vecColumn.getSize());

	vecScalar_[iPosition_] = createField(cEnvironment_,
										 vecColumn[iPosition_]);
}

// FUNCTION private
//	Relation::Table::createScalarType -- set scalar node type
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Interface::Type>& vecType_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Table::
createScalarType(Opt::Environment& cEnvironment_,
				 VECTOR<Tree::Node::Type>& vecType_,
				 Position iPosition_)
{
	// set all the columns by same type
	const ModVector<Schema::Column*>& vecColumn = m_pSchemaTable->getColumn(cEnvironment_.getTransaction());
	if (iPosition_ >= static_cast<Position>(vecColumn.getSize())) {
		return;
	}

	Opt::ExpandContainer(vecType_, vecColumn.getSize(), Tree::Node::Field);
	; _SYDNEY_ASSERT(vecType_.GETSIZE() >= vecColumn.getSize());
}

// FUNCTION private
//	Relation::Table::addAggregation -- add aggregation
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

//virtual
int
Table::
addAggregation(Opt::Environment& cEnvironment_,
			   Interface::IScalar* pScalar_,
			   Interface::IScalar* pOperand_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
