// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operation/UndoLog.cpp --
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

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Analysis/Operation/UndoLog.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"

#include "Exception/Unexpected.h"

#include "Opt/Environment.h"
#include "Opt/UndoLog.h"

#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Table.h"

#include "Schema/File.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_OPERATION_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Operation::Expression::Impl::UndoLogImpl --
	//		implementation classes for undoLog
	//
	// NOTES
	class UndoLogImpl
		: public Operation::UndoLog
	{
	public:
		typedef UndoLogImpl This;
		typedef Operation::UndoLog Super;

		// constructor
		UndoLogImpl()
			: Super()
		{}

		// destructor
		~UndoLogImpl() {}

	////////////////////////
	//Operation::UndoLog::
		virtual Plan::Interface::IRelation*
					getRelation(Opt::Environment& cEnvironment_,
								const Common::DataArrayData* pUndoLog_) const;
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
	const Impl::UndoLogImpl _analyzer;

} // namespace

//////////////////////////////////////////
// Operation::Impl::UndoLogImpl
//////////////////////////////////////////

// FUNCTION public
//	Operation::Impl::UndoLogImpl::getRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Common::DataArrayData* pUndoLog_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::UndoLogImpl::
getRelation(Opt::Environment& cEnvironment_,
			const Common::DataArrayData* pUndoLog_) const
{
	; _SYDNEY_ASSERT(pUndoLog_->getCount() > 1);

	// get schema table of target
	// [NOTES]
	//	Currently, all the undolog concern to same table
	//	-> checking first element is enough

	const Common::DataArrayData* pElement =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 pUndoLog_->getElement(1).get());
	if (pElement == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Schema::Object::ID::Value iFileID =
		pElement->getElement(Opt::UndoLog::Format::FileID)->getUnsignedInt();
	Schema::File* pSchemaFile = Schema::File::get(iFileID,
												  cEnvironment_.getDatabase(),
												  cEnvironment_.getTransaction());
	if (pSchemaFile == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Schema::Table* pSchemaTable =
		pSchemaFile->getTable(cEnvironment_.getTransaction());
	if (pSchemaTable == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Plan::Interface::IRelation* pRelation =
		Plan::Relation::Table::Undo::create(cEnvironment_,
											pSchemaTable,
											pUndoLog_);
	pRelation->setRowInfo(cEnvironment_,
						  Plan::Relation::RowInfo::create(cEnvironment_));
	return pRelation;
}

////////////////////////////////////////
// Operation::UndoLog
////////////////////////////////////////

// FUNCTION public
//	Operation::UndoLog::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	const UndoLog*
//
// EXCEPTIONS

const UndoLog*
UndoLog::
create()
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_OPERATION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
