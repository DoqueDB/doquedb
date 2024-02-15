// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operation/Import.cpp --
// 
// Copyright (c) 2010, 2011, 2015, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Analysis/Operation/Import.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Argument.h"
#include "Opt/Environment.h"

#include "Plan/Interface/IFile.h"
#include "Plan/Order/Key.h"
#include "Plan/Order/Specification.h"
#include "Plan/Relation/Sort.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Value.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_OPERATION_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Operation::Expression::Impl::ImportImpl --
	//		implementation classes for import statement analyzer
	//
	// NOTES
	class ImportImpl
		: public Operation::Import
	{
	public:
		typedef ImportImpl This;
		typedef Operation::Import Super;

		// constructor
		ImportImpl()
			: Super()
		{}
		// destructor
		virtual ~ImportImpl() {}

	////////////////////////
	//Operation::Import::
		virtual Plan::Interface::IRelation*
					getRelation(Opt::Environment& cEnvironment_,
								const Opt::ImportArgument& cArgument_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	_analyzer -- instance
	//
	// NOTES
	const Impl::ImportImpl _analyzer;

} // namespace

//////////////////////////////////////////
// Operation::Impl::ImportImpl
//////////////////////////////////////////

// FUNCTION public
//	Operation::Impl::ImportImpl::getRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Opt::ImportArgument& cArgument_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::ImportImpl::
getRelation(Opt::Environment& cEnvironment_,
			const Opt::ImportArgument& cArgument_) const
{
	Plan::Relation::Table* pSourceTable = 0;
	{
		Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushNameScope();
		pSourceTable = Plan::Relation::Table::Retrieve::create(cEnvironment_,
														 cArgument_.m_pTargetTable);
		pSourceTable->setCorrelationName(cEnvironment_,
										 STRING(),
										 VECTOR<STRING>());
	}

	// create source field
	VECTOR<Plan::Interface::IScalar*> vecSource;

	ModVector<Schema::Field*>::ConstIterator iterator = cArgument_.m_vecSourceField.begin();
	const ModVector<Schema::Field*>::ConstIterator last = cArgument_.m_vecSourceField.end();
	for (; iterator != last; ++iterator) {
		Plan::Interface::IScalar* pSource = 0;
		Schema::Field* pSourceField = *iterator;
		if (pSourceField == 0) {
			// no source value
			;
		} else {
			if (Schema::Column* pSchemaColumn =
				pSourceField->getRelatedColumn(cEnvironment_.getTransaction())) {

				pSource = Plan::Scalar::Field::create(cEnvironment_,
													  pSchemaColumn,
													  pSourceTable);
			} else {
				Schema::File* pSchemaFile = pSourceField->getFile(cEnvironment_.getTransaction());
				; _SYDNEY_ASSERT(pSchemaFile);
				Plan::Interface::IFile* pFile = Plan::Interface::IFile::create(cEnvironment_,
																			   pSchemaFile);
				// create field node
				pSource = Plan::Scalar::Field::create(cEnvironment_,
													  pSourceField,
													  pFile,
													  pSourceTable);
			}
			pSource->retrieve(cEnvironment_);
		}
		vecSource.PUSHBACK(pSource);
	}
	Plan::Interface::IRelation* pInput = pSourceTable;
	if (cArgument_.m_bRowIDOrder) {
		// add sorting part
		Plan::Interface::IScalar* pRowID = pSourceTable->getScalar(cEnvironment_, 0);
		; _SYDNEY_ASSERT(pRowID);
		Plan::Order::Key* pKey = Plan::Order::Key::create(cEnvironment_,
														  pRowID,
														  Plan::Order::Direction::Ascending);
		Plan::Order::Specification* pOrder = Plan::Order::Specification::create(cEnvironment_,
																				pKey);
		pInput = Plan::Relation::Sort::create(cEnvironment_,
											  pOrder,
											  pInput);
	}

	// create result relation
	Plan::Relation::Table* pResult =
		Plan::Relation::Table::Import::create(cEnvironment_,
											  cArgument_,
											  pInput);
	// add input
	int n = vecSource.GETSIZE();
	for (int i = 0; i < n; ++i) {
		if (vecSource[i]) {
			pResult->addInput(cEnvironment_,
							  i,
							  vecSource[i]);
		}
	}

	return pResult;
}

////////////////////////////////////////
// Operation::Import
////////////////////////////////////////

// FUNCTION public
//	Operation::Import::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Opt::ImportArgument& cArgument_
//	
// RETURN
//	const Import*
//
// EXCEPTIONS

const Import*
Import::
create(const Opt::ImportArgument& cArgument_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_OPERATION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
