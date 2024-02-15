// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Field.cpp --
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
const char moduleName[] = "DPlan::Scalar";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DPlan/Scalar/Field.h"
#include "DPlan/Scalar/Impl/FieldImpl.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_SCALAR_BEGIN

// FUNCTION public
//	Scalar::Field::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Plan::Relation::Table* pTable_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//static
Plan::Scalar::Field*
Field::
create(Opt::Environment& cEnvironment_,
	   Schema::Column* pSchemaColumn_,
	   Plan::Relation::Table* pTable_)
{
	AUTOPOINTER<FieldImpl::Base> pResult =
		new FieldImpl::SchemaColumn(pSchemaColumn_, pTable_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}




// FUNCTION public
//	Scalar::Field::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Plan::Relation::Table* pTable_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//static
Plan::Scalar::Field*
Field::
create(Opt::Environment& cEnvironment_,
	   Plan::Relation::Table* pTable_,
	   Plan::Interface::IScalar* pFunction_)
{
	AUTOPOINTER<FieldImpl::Base> pResult =
		new FieldImpl::FunctionField(pTable_, pFunction_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
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
Plan::Scalar::Field*
Field::
create(Opt::Environment& cEnvironment_,
	   Schema::Column* pSchemaColumn_,
	   Plan::Relation::Table* pTable_,
	   Plan::Scalar::Field* pColumn_,
	   Plan::Interface::IScalar* pOption_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  pSchemaColumn_,
					  pTable_);
	}
	
	AUTOPOINTER<FieldImpl::OptionColumn> pResult =
		new FieldImpl::OptionColumn(pSchemaColumn_, pTable_, pColumn_, pOption_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}



_SYDNEY_DPLAN_SCALAR_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
