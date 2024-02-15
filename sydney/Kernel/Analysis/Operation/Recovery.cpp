// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operation/Recovery.cpp --
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
const char moduleName[] = "Analysis::Operation";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Analysis/Operation/Recovery.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/UnsignedIntegerArrayData.h"

#include "Exception/LogItemCorrupted.h"
#include "Exception/NotSupported.h"

#include "Opt/Argument.h"
#include "Opt/Environment.h"
#include "Opt/LogData.h"

#include "Plan/Interface/IFile.h"
#include "Plan/Predicate/Comparison.h"
#include "Plan/Relation/Projection.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Selection.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Generator.h"
#include "Plan/Scalar/Value.h"

#include "Schema/Column.h"
#include "Schema/Database.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_OPERATION_BEGIN

namespace RecoveryImpl
{
	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Operation::Expression::RecoveryImpl::Base --
	//		base class of implementation classes for recovery
	//
	// NOTES
	class Base
		: public Operation::Recovery
	{
	public:
		typedef Base This;
		typedef Operation::Recovery Super;

		// destructor
		virtual ~Base() {}

	////////////////////////
	//Operation::Recovery::
		virtual Plan::Interface::IRelation*
					getRelation(Opt::Environment& cEnvironment_,
								const Opt::LogData* pLogData_,
								bool bIsRollback_) const;
	protected:
		// constructor
		Base()
			: Super()
		{}

		// get schema table object
		Schema::Table* getSchemaTable(Opt::Environment& cEnvironment_,
									  const Opt::LogData* pLogData_) const;
		// get schema column objects
		VECTOR<Schema::Column*> getSchemaColumns(Opt::Environment& cEnvironment_,
												 const Schema::Table* pSchemaTable_,
												 const Opt::LogData* pLogData_) const;
		// get rowid value
		const Common::Data* getRowID(Opt::Environment& cEnvironment_,
									 const Opt::LogData* pLogData_) const;
		// get data1
		const Common::DataArrayData* getData1(Opt::Environment& cEnvironment_,
											  const Opt::LogData* pLogData_) const;
		// get data2
		const Common::DataArrayData* getData2(Opt::Environment& cEnvironment_,
											  const Opt::LogData* pLogData_) const;

		// get retrieve table for delete/update
		Plan::Relation::Table* getRetrieveTable(Opt::Environment& cEnvironment_,
												Schema::Table* pSchemaTable_) const;

		// get input relation filtering by rowid
		Plan::Interface::IRelation*
					getFilteredInput(Opt::Environment& cEnvironment_,
									 Plan::Relation::Table* pTable_,
									 const Common::Data* pRowID_) const;
	private:
		// getRelation main implementation
		virtual Plan::Interface::IRelation*
					getRelationMain(Opt::Environment& cEnvironment_,
									const Opt::LogData* pLogData_,
									bool bIsRollback_) const = 0;
	};

	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Operation::Expression::RecoveryImpl::Insert --
	//		implementation classes for recovery by insert
	//
	// NOTES
	class Insert
		: public Base
	{
	public:
		typedef Insert This;
		typedef Base Super;

		// constructor
		Insert()
			: Super()
		{}
		// destructor
		~Insert() {}

	////////////////////////
	//Operation::Recovery::
	//	virtual Plan::Interface::IRelation*
	//				getRelation(Opt::Environment& cEnvironment_,
	//							const Opt::LogData* pLogData_,
	//							bool bIsRollback_) const;
	protected:
	private:
	///////////////////////////
	// RecoveryImpl::Base::
		virtual Plan::Interface::IRelation*
					getRelationMain(Opt::Environment& cEnvironment_,
									const Opt::LogData* pLogData_,
									bool bIsRollback_) const;
	};

	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Operation::Expression::RecoveryImpl::Delete --
	//		implementation classes for recovery by delete
	//
	// NOTES
	class Delete
		: public Base
	{
	public:
		typedef Delete This;
		typedef Base Super;

		// constructor
		Delete()
			: Super()
		{}
		// destructor
		~Delete() {}

	////////////////////////
	//Operation::Recovery::
	//	virtual Plan::Interface::IRelation*
	//				getRelation(Opt::Environment& cEnvironment_,
	//							const Opt::LogData* pLogData_,
	//							bool bIsRollback_) const;
	protected:
	private:
	///////////////////////////
	// RecoveryImpl::Base::
		virtual Plan::Interface::IRelation*
					getRelationMain(Opt::Environment& cEnvironment_,
									const Opt::LogData* pLogData_,
									bool bIsRollback_) const;
	};

	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Operation::Expression::RecoveryImpl::Update --
	//		implementation classes for recovery by update
	//
	// NOTES
	class Update
		: public Base
	{
	public:
		typedef Update This;
		typedef Base Super;

		// constructor
		Update()
			: Super()
		{}
		// destructor
		~Update() {}

	////////////////////////
	//Operation::Recovery::
	//	virtual Plan::Interface::IRelation*
	//				getRelation(Opt::Environment& cEnvironment_,
	//							const Opt::LogData* pLogData_,
	//							bool bIsRollback_) const;
	protected:
	private:
	///////////////////////////
	// RecoveryImpl::Base::
		virtual Plan::Interface::IRelation*
					getRelationMain(Opt::Environment& cEnvironment_,
									const Opt::LogData* pLogData_,
									bool bIsRollback_) const;
	};
}

namespace
{
	// VARIABLE local
	//	_analyzerXXX -- instances
	//
	// NOTES
	const RecoveryImpl::Insert _analyzerInsert;
	const RecoveryImpl::Delete _analyzerDelete;
	const RecoveryImpl::Update _analyzerUpdate;

} // namespace

//////////////////////////////////////////
// Operation::RecoveryImpl::Base
//////////////////////////////////////////

// FUNCTION public
//	Operation::RecoveryImpl::Base::getRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Opt::LogData* pLogData_
//	bool bIsRollback_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
RecoveryImpl::Base::
getRelation(Opt::Environment& cEnvironment_,
			const Opt::LogData* pLogData_,
			bool bIsRollback_) const
{
	Plan::Interface::IRelation* pRelation = getRelationMain(cEnvironment_,
															pLogData_,
															bIsRollback_);
	if (pRelation == 0) {
		return 0;
	}
	Plan::Relation::RowInfo* pEmptyRow = Plan::Relation::RowInfo::create(cEnvironment_);
	return Plan::Relation::Projection::create(cEnvironment_,
											  pEmptyRow,
											  pRelation);
}

// FUNCTION protected
//	Operation::RecoveryImpl::Base::getSchemaTable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Opt::LogData* pLogData_
//	
// RETURN
//	Schema::Table*
//
// EXCEPTIONS

Schema::Table*
RecoveryImpl::Base::
getSchemaTable(Opt::Environment& cEnvironment_,
			   const Opt::LogData* pLogData_) const
{
	// get tableID from log data
	Schema::Object::ID::Value iTableID =
		pLogData_->getData().getElement(Opt::LogData::Format::TableID)
		->getUnsignedInt();
	return cEnvironment_.getDatabase()->getTable(iTableID, cEnvironment_.getTransaction());
}

// FUNCTION protected
//	Operation::RecoveryImpl::Base::getSchemaColumns -- get schema column objects
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Schema::Table* pSchemaTable_
//	const Opt::LogData* pLogData_
//	
// RETURN
//	VECTOR<Schema::Column*>
//
// EXCEPTIONS

VECTOR<Schema::Column*>
RecoveryImpl::Base::
getSchemaColumns(Opt::Environment& cEnvironment_,
				 const Schema::Table* pSchemaTable_,
				 const Opt::LogData* pLogData_) const
{
	VECTOR<Schema::Column*> vecResult;
	const Common::UnsignedIntegerArrayData* pArray =
		_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerArrayData*,
							 pLogData_->getData().getElement(Opt::LogData::Format::ColumnID).get());
	if (pArray == 0) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	Trans::Transaction& cTrans = cEnvironment_.getTransaction();

	Opt::MapContainer(pArray->getValue(), vecResult,
					  boost::bind(&Schema::Table::getColumnByID,
								  pSchemaTable_,
								  _1,
								  boost::ref(cTrans)));
	return vecResult;
}

// FUNCTION protected
//	Operation::RecoveryImpl::Base::getRowID -- get rowid value
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Opt::LogData* pLogData_
//	
// RETURN
//	const Common::Data*
//
// EXCEPTIONS

const Common::Data*
RecoveryImpl::Base::
getRowID(Opt::Environment& cEnvironment_,
		 const Opt::LogData* pLogData_) const
{
	return pLogData_->getData().getElement(Opt::LogData::Format::RowID).get();
}

// FUNCTION protected
//	Operation::RecoveryImpl::Base::getData1 -- get data1
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Opt::LogData* pLogData_
//	
// RETURN
//	const Common::DataArrayData*
//
// EXCEPTIONS

const Common::DataArrayData*
RecoveryImpl::Base::
getData1(Opt::Environment& cEnvironment_,
		 const Opt::LogData* pLogData_) const
{
	const Common::DataArrayData* pResult =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 pLogData_->getData().getElement(Opt::LogData::Format::Data1).get());
	if (pResult == 0) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return pResult;
}

// FUNCTION protected
//	Operation::RecoveryImpl::Base::getData2 -- get data2
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Opt::LogData* pLogData_
//	
// RETURN
//	const Common::DataArrayData*
//
// EXCEPTIONS

const Common::DataArrayData*
RecoveryImpl::Base::
getData2(Opt::Environment& cEnvironment_,
		 const Opt::LogData* pLogData_) const
{
	const Common::DataArrayData* pResult =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 pLogData_->getData().getElement(Opt::LogData::Format::Data2).get());
	if (pResult == 0) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return pResult;
}

// FUNCTION protected
//	Operation::RecoveryImpl::Base::getRetrieveTable -- get retrieve table for delete/update
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	Plan::Relation::Table*
//
// EXCEPTIONS

Plan::Relation::Table*
RecoveryImpl::Base::
getRetrieveTable(Opt::Environment& cEnvironment_,
				 Schema::Table* pSchemaTable_) const
{
	// create Interface from schema table
	Plan::Relation::Table* pResult =
		Plan::Relation::Table::Retrieve::create(cEnvironment_,
												pSchemaTable_);
	pResult->setCorrelationName(cEnvironment_,
								STRING(),
								VECTOR<STRING>());

	return pResult;
}

// FUNCTION protected
//	Operation::RecoveryImpl::Base::getFilteredInput -- get input relation filtering by rowid
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pTable_
//	const Common::Data* pRowID_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

Plan::Interface::IRelation*
RecoveryImpl::Base::
getFilteredInput(Opt::Environment& cEnvironment_,
				 Plan::Relation::Table* pTable_,
				 const Common::Data* pRowID_) const
{
	// create search filter
	Plan::Interface::IScalar* pColumn = pTable_->getScalar(cEnvironment_, 0); // ROWID
	Plan::Interface::IScalar* pValue = Plan::Scalar::Value::create(cEnvironment_,
																	   pRowID_);
	Plan::Interface::IPredicate* pCondition =
		Plan::Predicate::Comparison::create(cEnvironment_,
											Plan::Tree::Node::Equals,
											MAKEPAIR(pColumn, pValue),
											false /* not check comparability */);

	Plan::Interface::IRelation* pSelection =
		Plan::Relation::Selection::create(cEnvironment_,
										  pCondition,
										  pTable_);
	// empty result
	Plan::Relation::RowInfo* pEmptyRow = Plan::Relation::RowInfo::create(cEnvironment_);
	return Plan::Relation::Projection::create(cEnvironment_,
											  pEmptyRow,
											  pSelection);
}

//////////////////////////////////////////
// Operation::RecoveryImpl::Insert
//////////////////////////////////////////

// FUNCTION public
//	Operation::RecoveryImpl::Insert::getRelationMain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Opt::LogData* pLogData_
//	bool bIsRollback_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
RecoveryImpl::Insert::
getRelationMain(Opt::Environment& cEnvironment_,
				const Opt::LogData* pLogData_,
				bool bIsRollback_) const
{
	Schema::Table* pSchemaTable = getSchemaTable(cEnvironment_,
												 pLogData_);
	if (pSchemaTable == 0) {
		return 0;
	}

	const VECTOR<Schema::Column*>& vecSchemaColumn = getSchemaColumns(cEnvironment_,
																	  pSchemaTable,
																	  pLogData_);
	const Common::Data* pRowID = getRowID(cEnvironment_,
										  pLogData_);
	const Common::DataArrayData* pData1 = getData1(cEnvironment_,
												   pLogData_);

	int n = pData1->getCount();
	if (n != vecSchemaColumn.GETSIZE()) {
		// number of columns and values don't match
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	// create insert relation
	Plan::Relation::Table* pResult =
		Plan::Relation::Table::Insert::create(cEnvironment_,
											  pSchemaTable->getName(),
											  0 /* no input */);

	// create correspondence between column and log data
	// rowid
	Plan::Interface::IScalar* pValue = Plan::Scalar::Value::create(cEnvironment_,
																   pRowID);
	Schema::Column* pSchemaRowID = pSchemaTable->getTupleID(cEnvironment_.getTransaction());
	pResult->addInput(cEnvironment_,
					  0,
					  Plan::Scalar::Generator::RowID::Recovery::create(cEnvironment_,
																	   pSchemaRowID,
																	   pValue));
	VECTOR<Schema::Column*>::CONSTITERATOR columnIterator = vecSchemaColumn.begin();
	for (int i = 0; i < n; ++i, ++columnIterator) {

		if ((*columnIterator)->getPosition() == 0) continue; // skip rowid

		Plan::Interface::IScalar* pValue =
			Plan::Scalar::Value::create(cEnvironment_,
										pData1->getElement(i));
		if (bIsRollback_ == false
			&& (*columnIterator)->isIdentity()) {
			// need recovery
			pValue = Plan::Scalar::Generator::Identity::Recovery::create(cEnvironment_,
																		 *columnIterator,
																		 pValue);
		}
		pResult->addInput(cEnvironment_,
						  (*columnIterator)->getPosition(),
						  pValue);
	}
	return pResult;
}

//////////////////////////////////////////
// Operation::RecoveryImpl::Delete
//////////////////////////////////////////

// FUNCTION public
//	Operation::RecoveryImpl::Delete::getRelationMain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Opt::LogData* pLogData_
//	bool bIsRollback_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
RecoveryImpl::Delete::
getRelationMain(Opt::Environment& cEnvironment_,
				const Opt::LogData* pLogData_,
				bool bIsRollback_) const
{
	Schema::Table* pSchemaTable = getSchemaTable(cEnvironment_,
												 pLogData_);
	if (pSchemaTable == 0) {
		return 0;
	}

	const Common::Data* pRowID = getRowID(cEnvironment_,
										  pLogData_);

	Plan::Relation::Table* pTable = 0;
	Plan::Interface::IRelation* pInput = 0;
	{
		Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushNameScope();

		pTable = getRetrieveTable(cEnvironment_,
								  pSchemaTable);
		pInput = getFilteredInput(cEnvironment_,
								  pTable,
								  pRowID);
	}
	return Plan::Relation::Table::Delete::create(cEnvironment_,
												 pTable,
												 pInput);
}

//////////////////////////////////////////
// Operation::RecoveryImpl::Update
//////////////////////////////////////////

// FUNCTION public
//	Operation::RecoveryImpl::Update::getRelationMain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Opt::LogData* pLogData_
//	bool bIsRollback_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
RecoveryImpl::Update::
getRelationMain(Opt::Environment& cEnvironment_,
				const Opt::LogData* pLogData_,
				bool bIsRollback_) const
{
	Schema::Table* pSchemaTable = getSchemaTable(cEnvironment_,
												 pLogData_);
	if (pSchemaTable == 0) {
		return 0;
	}

	const VECTOR<Schema::Column*>& vecSchemaColumn = getSchemaColumns(cEnvironment_,
																	  pSchemaTable,
																	  pLogData_);
	const Common::Data* pRowID = getRowID(cEnvironment_,
										  pLogData_);
	const Common::DataArrayData* pData =
		bIsRollback_ ? getData1(cEnvironment_,
								pLogData_)
		: getData2(cEnvironment_,
				   pLogData_);

	int n = pData->getCount();
	if (n != vecSchemaColumn.GETSIZE()) {
		// number of columns and values don't match
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	Plan::Interface::IRelation* pInput = 0;
	Plan::Relation::Table* pTable = 0;
	{
		Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushNameScope();

		pTable = getRetrieveTable(cEnvironment_,
								  pSchemaTable);
		pInput = getFilteredInput(cEnvironment_,
								  pTable,
								  pRowID);
	}
	Plan::Relation::Table* pResult =
		Plan::Relation::Table::Update::create(cEnvironment_,
											  pTable,
											  pInput);

	VECTOR<Schema::Column*>::CONSTITERATOR columnIterator = vecSchemaColumn.begin();
	for (int i = 0; i < n; ++i, ++columnIterator) {

		Plan::Relation::Table::Position iPosition = (*columnIterator)->getPosition();

		if (iPosition == 0) continue; // skip rowid

		Plan::Interface::IScalar* pValue =
			Plan::Scalar::Value::create(cEnvironment_,
										pData->getElement(i));

		if ((*columnIterator)->isIdentity()) {
			// check for duplicate element of identity column
			if (columnIterator + 1 != vecSchemaColumn.end()
				&& *(columnIterator + 1) == *columnIterator) {
				if (bIsRollback_ == false) {
					// duplicate identity log found -> need assign to identity generator
					pValue = Plan::Scalar::Generator::Identity::Recovery::create(cEnvironment_,
																				 *columnIterator,
																				 pValue);
				}
				// anyway, duplicate log is skipped
				++i;
				++columnIterator;
			}
		}

		pResult->addInput(cEnvironment_,
						  iPosition,
						  pValue);
	}
	return pResult;
}

////////////////////////////////////////
// Operation::Recovery
////////////////////////////////////////

// FUNCTION public
//	Operation::Recovery::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	int iType_
//	bool bIsRollback_
//	
// RETURN
//	const Recovery*
//
// EXCEPTIONS

const Recovery*
Recovery::
create(int iType_,
	   bool bIsRollback_)
{
	switch (iType_) {
	case Opt::LogData::Type::Insert:
		{
			if (bIsRollback_) return &_analyzerDelete; else return &_analyzerInsert;
		}
	case Opt::LogData::Type::Delete:
	case Opt::LogData::Type::Delete_Undo:
		{
			if (bIsRollback_) return &_analyzerInsert; else return &_analyzerDelete;
		}
	case Opt::LogData::Type::Update:
	case Opt::LogData::Type::Update_Undo:
		{
			return &_analyzerUpdate;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::LogItemCorrupted);
		}
	}
}

_SYDNEY_ANALYSIS_OPERATION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
