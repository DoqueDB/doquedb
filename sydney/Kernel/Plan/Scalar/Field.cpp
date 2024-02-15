// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Field.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2016, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Impl/FieldImpl.h"

#include "Plan/Scalar/Argument.h"
#include "Plan/Scalar/CheckedField.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Relation/Table.h"

#include "Common/Assert.h"

#include "DPlan/Scalar/Field.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

#include "Execution/Action/Argument.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Environment.h"

#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace
{
	// TEMPLATE CLASS local
	//	$$$::_True -- function class for returning always true
	//
	// TEMPLATE ARGUMENT
	//	T_
	//
	// NOTES
	template<class T_>
	class _True
	{
	public:
		_True() {}
		~_True() {}
		bool operator()(T_*) {return true;}
	};

	class _AbleToScan
	{
	public:
		_AbleToScan(Opt::Environment& cEnvironment_,
					Relation::Table* pTable_)
			: m_cEnvironment(cEnvironment_),
			  m_pTable(pTable_)
		{}
		~_AbleToScan() {}

		bool operator()(Schema::File* pSchemaFile_)
		{
			Interface::IFile* pFile = Interface::IFile::create(m_cEnvironment,
															   pSchemaFile_);
			return pSchemaFile_->isAbleToScan(pFile->hasAllTuples(m_cEnvironment,
																  m_pTable));
		}
	private:
		Opt::Environment& m_cEnvironment;
		Relation::Table* m_pTable;
	};


	class _AbleToRetrieveArrayElement
	{
	public:
		_AbleToRetrieveArrayElement(Opt::Environment& cEnvironment_,
					Field* pField_)
			: m_cEnvironment(cEnvironment_),
			  m_pField(pField_)
		{}
		~_AbleToRetrieveArrayElement() {}

		bool operator()(Schema::File* pSchemaFile_)
		{
	
			Schema::Field* pSchemaField;
			if (m_pField->isColumn()) {
				pSchemaField =
					m_pField->getSchemaColumn()->getField(m_cEnvironment.getTransaction());
			} else {
				pSchemaField = m_pField->getSchemaField();
			}


			return pSchemaFile_->isGettable(m_cEnvironment.getTransaction(),
											pSchemaField,
											m_pField);
		}
	private:
		Opt::Environment& m_cEnvironment;
		Field* m_pField;
	};

	// STRUCT local
	//	$$$::_FetchableFieldFilter --FieldFilter for getFetchFile
	//
	// NOTES
	struct _FetchableFieldFilter
	{
		bool operator()(const Schema::Field* pField_) const
		{
			return pField_->isKey() || pField_->isObjectID();
		}
	};

	// TEMPLATE FUNCTION local
	//	$$$::_getIndexFile -- generic function for Field::getXXXFile
	//
	// TEMPLATE ARGUMENTS
	//	class FieldFilter_
	//	class FileFilter_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Opt::Environment& cEnvironment_
	//	const GetFileArgument& cArgument_
	//	FieldFilter_ fieldFilter_
	//	FileFilter_ fileFilter_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS

	template <class FieldFilter_, class FileFilter_>
	bool
	_getIndexFile(Opt::Environment& cEnvironment_,
				  const GetFileArgument& cArgument_,
				  FieldFilter_ fieldFilter_,
				  FileFilter_ fileFilter_)
	{
		if (cArgument_.m_pField->isField() == false) {
			return false;
		}

		bool bResult = false;

		Field* pField = cArgument_.m_pField->getField();
		; _SYDNEY_ASSERT(pField);

		if (pField->isFunction()
			|| pField->getTable()->getSchemaTable()->isVirtual()) {
			return false;
		}

		Schema::Field* pSchemaField = pField->getSchemaField();
		if (pSchemaField) {
			if (fieldFilter_(pSchemaField)) {
				Schema::File* pSchemaFile = pSchemaField->getFile(cEnvironment_.getTransaction());
				if (fileFilter_(pSchemaFile)) {
					Interface::IFile* pFile =
						Interface::IFile::create(cEnvironment_,
												 pSchemaFile);
					cArgument_.m_cFile.add(pFile);
					bResult = true;
				}
			}
		} else {
			; _SYDNEY_ASSERT(pField->isColumn());
			Schema::Column* pSchemaColumn = pField->getSchemaColumn();

			ModVector<Schema::Field*> vecKeyField;

			// [NOTE]
			// offline indexes are filtered out here
			if (pSchemaColumn->getIndexField(cEnvironment_.getTransaction(),
											 vecKeyField)) {
				for (ModVector<Schema::Field*>::Iterator iterator = vecKeyField.begin();
					 iterator != vecKeyField.end();
					 ++iterator) {
					Schema::File* pSchemaFile = (*iterator)->getFile(cEnvironment_.getTransaction());
					if (fileFilter_(pSchemaFile)) {
						Interface::IFile* pFile =
							Interface::IFile::create(cEnvironment_,
													 pSchemaFile);
						Field* pKeyField = Field::create(cEnvironment_,
														 *iterator,
														 pFile,
														 pField->getTable(),
														 pField);
						cArgument_.m_cFile.add(pFile);
						bResult = true;
					}
				}
			}
		}
		return bResult;
	}

	// TEMPLATE FUNCTION local
	//	$$$::_checkOneFile -- check one field for scanning or sortting
	//
	// TEMPLATE ARGUMENTS
	//	class FieldFilter_
	//	class FileFilter_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Opt::Environment& cEnvironment_
	//	Schema::File* pSchemaFile_
	//	Schema::Field* pSchemaField_
	//	Field* pColumn_
	//	Utility::FileSet& cFile_
	//	FieldFilter_ fieldFilter_
	//	FileFilter_ fileFilter_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS

	template <class FieldFilter_, class FileFilter_>
	bool
	_checkOneFile(Opt::Environment& cEnvironment_,
				  Schema::File* pSchemaFile_,
				  Schema::Field* pSchemaField_,
				  Field* pColumn_,
				  Utility::FileSet& cFile_,
				  FieldFilter_ fieldFilter_,
				  FileFilter_ fileFilter_)

	{
		if (fieldFilter_(pSchemaField_)
			&& fileFilter_(pSchemaFile_)) {
			Interface::IFile* pFile = Interface::IFile::create(cEnvironment_,
															   pSchemaFile_);
			Field* pField = Field::create(cEnvironment_,
										  pSchemaField_,
										  pFile,
										  pColumn_->getTable(),
										  pColumn_,
										  pColumn_->getOption());

			// add file to result
			cFile_.add(pFile);
			return true;
		}
		return false;
	}

	// TEMPLATE FUNCTION local
	//	$$$::_checkSourceDestination -- Field checker for getSortKey
	//
	// TEMPLATE ARGUMENTS
	//	class FieldFilter_
	//	class FileFilter_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Opt::Environment& cEnvironment_
	//	const GetFileArgument& cArgument_
	//	FieldFilter_ fieldFilter_
	//	FileFilter_ fileFilter_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS

	template <class FieldFilter_, class FileFilter_>
	bool
	_checkSourceDestination(Opt::Environment& cEnvironment_,
							const GetFileArgument& cArgument_,
							FieldFilter_ fieldFilter_,
							FileFilter_ fileFilter_)
	{
		bool bResult = false;

		; _SYDNEY_ASSERT(cArgument_.m_pField->isField());

		Field* pField = cArgument_.m_pField->getField();
		; _SYDNEY_ASSERT(pField);

		if (pField->isFunction()
			|| pField->getTable()->getSchemaTable()->isVirtual()) {
			return false;
		}

		Trans::Transaction& cTrans = cEnvironment_.getTransaction();

		Schema::Field* pSchemaField = 0;

		if (pField->isColumn()) {
			// ordinal column
			Schema::Column* pSchemaColumn = pField->getSchemaColumn();
			; _SYDNEY_ASSERT(pSchemaColumn);
			if (pSchemaColumn->getCategory() == Schema::Column::Category::Function) {
				// system column
				pSchemaColumn = pField->getTable()->getSchemaTable()->getTupleID(cTrans);
			}
			pSchemaField = pSchemaColumn->getField(cTrans);
		} else {
			pSchemaField = pField->getSchemaField();
			if (pSchemaField->getSourceID() != Schema::Object::ID::Invalid) {
				pSchemaField = pSchemaField->getSource(cTrans);
			}
		}

		Schema::File* pSchemaFile = pSchemaField->getFile(cTrans);

		if (pField->isFunction() == false) {
			// check by filters
			bResult = _checkOneFile(cEnvironment_,
									pSchemaFile,
									pSchemaField,
									pField,
									cArgument_.m_cFile,
									fieldFilter_,
									fileFilter_);
		}

		// check destinations
		const ModVector<Schema::Field*>& vecDest =
			pSchemaField->getDestination(cTrans);

		if (ModSize n = vecDest.getSize()) {
			ModSize i = 0;
			do {
				Schema::Field* pDest = vecDest[i];
				bResult = _checkOneFile(cEnvironment_,
										pDest->getFile(cTrans),
										pDest,
										pField,
										cArgument_.m_cFile,
										fieldFilter_,
										fileFilter_)
					|| bResult;
			} while (++i < n);
		}
		return bResult;
	}

} // namespace

////////////////////////////////////
//	Scalar::Field

// FUNCTION public
//	Scalar::Field::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Field* pSchemaField_
//	Interface::IFile* pFile_
//	Relation::Table* pTable_
//	Scalar::Field* pColumn_ = 0
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//static
Field*
Field::
create(Opt::Environment& cEnvironment_,
	   Schema::Field* pSchemaField_,
	   Interface::IFile* pFile_,
	   Relation::Table* pTable_,
	   Scalar::Field* pColumn_ /* = 0 */)
{
	if (This* pField = pTable_->getField(pSchemaField_)) {
		if (pColumn_) {
			// add column->field relationship to table object
			pTable_->addField(cEnvironment_,
							  pColumn_,
							  pFile_,
							  pField);
		}
		return pField;
	} else {
		if (pColumn_ == 0 && pSchemaField_->isFunction() == false) {
			if (Schema::Column* pSchemaColumn =
				pSchemaField_->getRelatedColumn(cEnvironment_.getTransaction())) {
				pColumn_ = Field::create(cEnvironment_,
										 pSchemaColumn,
										 pTable_);
			}
		}
		AUTOPOINTER<This> pResult;
		if (pSchemaField_->isTupleID(cEnvironment_.getTransaction())
			&& pFile_->getSchemaFile()->isAbleToBitSetSort()
			&& pTable_->isGrouping()) {
			pResult = new FieldImpl::BitSetField(pSchemaField_,
												 pFile_,
												 pTable_,
												 pColumn_);
		} else {
			pResult = new FieldImpl::SchemaField(pSchemaField_,
												 pFile_,
												 pTable_,
												 pColumn_);
		}
		pResult->registerToEnvironment(cEnvironment_);

		This* pThis = pResult.release();
		if (pColumn_) {
			// add column->field relationship to table object
			pTable_->addField(cEnvironment_,
							  pColumn_,
							  pFile_,
							  pThis);
		}	

		return pThis;

	}
}

// FUNCTION public
//	Scalar::Field::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Field* pSchemaColumn_
//	Interface::IFile* pFile_
//	Relation::Table* pTable_
//	Scalar::Field* pColumn_
//	Interface::IScalar* pOption_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//static
Field*
Field::
create(Opt::Environment& cEnvironment_,
	   Schema::Field* pSchemaField_,
	   Interface::IFile* pFile_,
	   Relation::Table* pTable_,
	   Scalar::Field* pColumn_ ,
	   Interface::IScalar* pOption_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  pSchemaField_,
					  pFile_,
					  pTable_,
					  pColumn_);
	}

	if (This* pField = pTable_->getField(pSchemaField_, pOption_)) {
		return pField;
	} else if (pColumn_
			   && pColumn_->isColumn()){
		AUTOPOINTER<FieldImpl::OptionField> pResult =
			new FieldImpl::OptionField(pSchemaField_,
									   pFile_,
									   pTable_,
									   pColumn_,
									   pOption_);
		pResult->registerToEnvironment(cEnvironment_);
		This* pThis = pResult.release();
		if (pColumn_) {
			// add column->field relationship to table object
			pTable_->addField(cEnvironment_,
							  pColumn_,
							  pFile_,
							  pThis);
		}
		return pThis;
	} else {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
}


// FUNCTION public
//	Scalar::Field::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Relation::Table* pTable_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//static
Field*
Field::
create(Opt::Environment& cEnvironment_,
	   Schema::Column* pSchemaColumn_,
	   Relation::Table* pTable_)
{
	if (This* pColumn = pTable_->getField(pSchemaColumn_)) {
		return pColumn;
	} else {
		AUTOPOINTER<This> pResult;
		if (pSchemaColumn_->getCategory() == Schema::Column::Category::Function) {
			// function column needs rowid as input
			Trans::Transaction& cTrans = cEnvironment_.getTransaction();
			Schema::Column* pRowIDColumn = pTable_->getSchemaTable()->getTupleID(cTrans);

			This* pRowID = create(cEnvironment_,
								  pRowIDColumn,
								  pTable_);

			pResult = new FieldImpl::SystemColumn(pSchemaColumn_, pRowID, pTable_);
		} else if (pSchemaColumn_->isTupleID()) {
			pResult = new FieldImpl::RowID(pSchemaColumn_, pTable_);
		} else if (pSchemaColumn_->isLob()) {
			pResult = new FieldImpl::LobColumn(pSchemaColumn_, pTable_);
		} else {
			pResult = new FieldImpl::SchemaColumn(pSchemaColumn_, pTable_);
		}
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}
}

// FUNCTION public
//	Scalar::Field::create -- by function field
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Field::Function::Value eFunction_
//	const Utility::FileSet& cFileSet_
//	Relation::Table* pTable_
//	Interface::IScalar* pFunction_
//	Interface::IScalar* pOperand_ /* = 0 */
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//static
Field*
Field::
create(Opt::Environment& cEnvironment_,
	   Schema::Field::Function::Value eFunction_,
	   const Utility::FileSet& cFileSet_,
	   Relation::Table* pTable_,
	   Interface::IScalar* pFunction_,
	   Interface::IScalar* pOperand_ /* = 0 */)
{
	// duplicate check should have been done in TableImpl
	AUTOPOINTER<This> pResult;
	if (cEnvironment_.hasCascade()) {
		pResult = DPlan::Scalar::Field::create(cEnvironment_,
											   pTable_,
											   pFunction_);
	} else {
		pResult = new FieldImpl::FunctionField(eFunction_,
											   cFileSet_,
											   pTable_,
											   pFunction_,
											   pOperand_);
		pResult->registerToEnvironment(cEnvironment_);		
	}

	return pResult.release();
}

// FUNCTION public
//	Scalar::Field::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	Scalar::Field* pColumn_
//	Interface::IScalar* pOption_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//static
Field*
Field::
create(Opt::Environment& cEnvironment_,
	   Scalar::Field* pField_,
	   Scalar::Field* pColumn_,
	   Interface::IScalar* pOption_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  pField_->getSchemaField(),
					  pField_->getFile(),
					  pField_->getTable(),
					  pColumn_);
	}

	if (This* pColumn = pField_->getTable()->getField(pField_->getSchemaField(), pOption_)) {
		return pColumn;
	} else {
		AUTOPOINTER<FieldImpl::OptionField> pResult =
			new FieldImpl::OptionField(pField_->getSchemaField(),
									   pField_->getFile(),
									   pField_->getTable(),
									   pColumn_,
									   pOption_);
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}
}

// FUNCTION public
//	Scalar::Field::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Relation::Table* pTable_
//	Scalar::Field* pColumn_
//	Interface::IScalar* pOption_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//static
Field*
Field::
create(Opt::Environment& cEnvironment_,
	   Schema::Column* pSchemaColumn_,
	   Relation::Table* pTable_,
	   Scalar::Field* pColumn_,
	   Interface::IScalar* pOption_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  pSchemaColumn_,
					  pTable_);
	}

	if (This* pColumn = pTable_->getField(pSchemaColumn_, pOption_)) {
		return pColumn;
	} else {
		AUTOPOINTER<FieldImpl::OptionColumn> pResult =
			new FieldImpl::OptionColumn(pSchemaColumn_, pTable_, pColumn_, pOption_);
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}
}

// FUNCTION public
//	Scalar::Field::create -- by a set of Field
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eType_
//	const VECTOR<Scalar::Field*>& vecOperand_
//	Relation::Table* pTable_
//	const STRING& cstrName_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//static
Field*
Field::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eType_,
	   const VECTOR<Scalar::Field*>& vecOperand_,
	   Relation::Table* pTable_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult;
	switch (eType_) {
	case Tree::Node::GetMax:
		{
			pResult = new FieldImpl::GetMax(vecOperand_,
											pTable_,
											Value::Null::create(cEnvironment_),
											cstrName_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Field::getFetchFile -- get fetchable files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const GetFileArgument& cArgument_
//	
// RETURN
//	bool	true if vecKey_ has at least one element
//
// EXCEPTIONS

//static
bool
Field::
getFetchFile(Opt::Environment& cEnvironment_,
			 const GetFileArgument& cArgument_)
{
	return _getIndexFile(cEnvironment_,
						 cArgument_,
						 _FetchableFieldFilter(),
						 boost::bind(&Schema::File::isAbleToFetch, _1));
}

// FUNCTION public
//	Scalar::Field::getSearchFile -- get key fields of searchable files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const GetFileArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Field::
getSearchFile(Opt::Environment& cEnvironment_,
			  const GetFileArgument& cArgument_)
{
	return _getIndexFile(cEnvironment_,
						 cArgument_,
						 boost::bind(&Schema::Field::isKey, _1),
						 boost::bind(&Schema::File::isAbleToSearch,
									 _1,
									 boost::cref(*cArgument_.m_pPredicate)));
}

// FUNCTION public
//	Scalar::Field::getScanFile -- get scannable files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const GetFileArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Field::
getScanFile(Opt::Environment& cEnvironment_,
			const GetFileArgument& cArgument_)
{
	; _SYDNEY_ASSERT(cArgument_.m_pField->isField());

	Field* pField = cArgument_.m_pField->getField();
	; _SYDNEY_ASSERT(pField);
	return _checkSourceDestination(cEnvironment_,
								   cArgument_,
								   boost::bind(&Schema::Field::isGetable, _1),
								   _AbleToScan(cEnvironment_,
											   pField->getTable()));
}

// FUNCTION public
//	Scalar::Field::getSortFile -- get key fields of sortable files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const GetFileArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Field::
getSortFile(Opt::Environment& cEnvironment_,
			const GetFileArgument& cArgument_)
{
	return _checkSourceDestination(cEnvironment_,
								   cArgument_,
								   boost::bind(&Schema::Field::isGetable, _1),
								   boost::bind(&Schema::File::isAbleToSort, _1));
}


// FUNCTION public
//	Scalar::Field::getGroupingFile -- get key fields of groupable files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const GetFileArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Field::
getGroupingFile(Opt::Environment& cEnvironment_,
				const GetFileArgument& cArgument_)
{
	return _checkSourceDestination(cEnvironment_,
								   cArgument_,
								   _True<Schema::Field>(),
								   boost::bind(&Schema::File::isAbleToBitSetSort, _1));
}

// FUNCTION public
//	Scalar::Field::getRetrieveFile -- get key fields of retrieving files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Order::GetFileArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Field::
getRetrieveFile(Opt::Environment& cEnvironment_,
				const GetFileArgument& cArgument_)
{
	; _SYDNEY_ASSERT(cArgument_.m_pField->isField());

	Field* pField = cArgument_.m_pField->getField();
	; _SYDNEY_ASSERT(pField);
	return _checkSourceDestination(cEnvironment_,
								   cArgument_,
								   boost::bind(&Schema::Field::isGetable, _1),
								   _AbleToRetrieveArrayElement(cEnvironment_, pField));
											   
}


// FUNCTION public
//	Scalar::Field::getPutFile -- get put files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const GetFileArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Field::
getPutFile(Opt::Environment& cEnvironment_,
		   const GetFileArgument& cArgument_)
{
	return _checkSourceDestination(cEnvironment_,
								   cArgument_,
								   boost::bind(&Schema::Field::isPutable, _1),
								   _True<Schema::File>());
}

// FUNCTION public
//	Scalar::Field::getCandidate -- find corresponding table candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pField_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Candidate::Table*
//
// EXCEPTIONS

//static
Candidate::Table*
Field::
getCandidate(Opt::Environment& cEnvironment_,
			 Interface::IScalar* pField_,
			 Interface::ICandidate* pCandidate_)
{
	; _SYDNEY_ASSERT(pField_->isField());

	Field* pField = pField_->getField();
	; _SYDNEY_ASSERT(pField);

	return pCandidate_->getCandidate(cEnvironment_,
									 pField->getTable());
}

// FUNCTION public
//	Scalar::Field::checkIsUnique -- check a field is unique?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pField_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Field::
checkIsUnique(Opt::Environment& cEnvironment_,
			  Interface::IScalar* pField_)
{
	; _SYDNEY_ASSERT(pField_->isField());

	Field* pField = pField_->getField();
	; _SYDNEY_ASSERT(pField);

	return pField->isKnownNotNull(cEnvironment_)
		&& pField->isUnique(cEnvironment_);
}

// FUNCTION public
//	Scalar::Field::generateAlternative -- 
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

int
Field::
generateAlternative(Opt::Environment& cEnvironment_,
					Execution::Interface::IProgram& cProgram_,
					Execution::Interface::IIterator* pIterator_,
					Candidate::AdoptArgument& cArgument_)
{
	if (Interface::IScalar* pAlternative = getAlternativeValue(cEnvironment_)) {
		return pAlternative->generate(cEnvironment_,
									  cProgram_,
									  pIterator_,
									  cArgument_);
	} else {
		return Value::Null::create(cEnvironment_)->generate(cEnvironment_,
															cProgram_,
															pIterator_,
															cArgument_);
	}
}

// FUNCTION public
//	Scalar::Field::addField -- add to field set
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
Field::
addField(Interface::IFile* pFile_,
		 Utility::FieldSet& cFieldSet_)
{
	cFieldSet_.add(this);
}

// FUNCTION public
//	Scalar::Field::addLocator -- add locator to environment
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
Field::
addLocator(Opt::Environment& cEnvironment_,
		   Execution::Interface::IIterator* pIterator_,
		   int iLocatorID_)
{
	cEnvironment_.addLocator(this,
							 pIterator_,
							 iLocatorID_);
}

// FUNCTION public
//	Scalar::Field::getLocator -- get locator from environment
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
Field::
getLocator(Opt::Environment& cEnvironment_,
		   Execution::Interface::IIterator* pIterator_)
{
	return cEnvironment_.getLocator(this,
									pIterator_);
}

// FUNCTION public
//	Scalar::Field::getUsedTable -- check used tables
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
Field::
getUsedTable(Utility::RelationSet& cResult_)
{
	cResult_.add(getTable());
}

// FUNCTION public
//	Scalar::Field::getUsedField -- 
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
Field::
getUsedField(Utility::FieldSet& cResult_)
{
	cResult_.add(this);
}

// FUNCTION public
//	Scalar::Field::getUnknownKey -- 
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
Field::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	if (!isKnownNotNull(cEnvironment_)) {
		cResult_.m_cKey.add(this);
	}
}

// FUNCTION protected
//	Scalar::Field::equals -- comparator for MAP
//
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Field::
equals(This& cOther_)
{
	Field* p0 = this;
	while (p0->isWrapper()) p0 = p0->getWrapper()->getWrappedField();
	Field* p1 = &cOther_;
	while (p1->isWrapper()) p1 = p1->getWrapper()->getWrappedField();

	return p0->getID() == p1->getID();
}

// FUNCTION protected
//	Scalar::Field::less -- 
//
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Field::
less(This& cOther_)
{
	Field* p0 = this;
	while (p0->isWrapper()) p0 = p0->getWrapper()->getWrappedField();
	Field* p1 = &cOther_;
	while (p1->isWrapper()) p1 = p1->getWrapper()->getWrappedField();

	return p0->getID() < p1->getID();
}

// FUNCTION protected
//	Scalar::Field::generateData -- generate data
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
Field::
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	int iResult = Super::generateData(cEnvironment_,
									  cProgram_,
									  pIterator_,
									  cArgument_);
	// add as used data
	pIterator_->useVariable(iResult);
	return iResult;
}

// FUNCTION protected
//	Scalar::Field::generateThis -- generate for this node
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
Field::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	// do nothing
	return iDataID_;
}

// FUNCTION private
//	Scalar::Field::registerToEnvironment -- register to environment
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
Field::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	// add to table object
	getTable()->addField(this);
	// add to environment
	cEnvironment_.addColumn(this);
	// use super class
	Super::registerToEnvironment(cEnvironment_);
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
