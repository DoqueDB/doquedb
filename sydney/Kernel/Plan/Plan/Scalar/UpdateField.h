// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/UpdateField.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_UPDATEFIELD_H
#define __SYDNEY_PLAN_SCALAR_UPDATEFIELD_H

#include "Plan/Scalar/FieldWrapper.h"
#include "Plan/Interface/IScalar.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::UpdateField -- Interface for update field
//
//	NOTES
class UpdateField
	: public FieldWrapper
{
public:
	typedef FieldWrapper Super;
	typedef UpdateField This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Field* pField_,
						Interface::IScalar* pInput_);
	static This* create(Opt::Environment& cEnvironment_,
						Field* pField_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cInput_);
	// destructor
	virtual ~UpdateField() {}

	// accessor
	virtual Interface::IScalar* getInput() = 0;
	virtual Interface::IScalar* getOriginal() = 0;

////////////////////////////////////
// Field::
//	using Super::getField;
//	virtual Field* getField(Interface::IFile* pFile_);
//	virtual Field* checkPut(Opt::Environment& cEnvironment_);
	virtual bool isUpdate() {return true;}
	virtual UpdateField* getUpdate() {return this;}
	virtual void addField(Interface::IFile* pFile_,
						  Utility::FieldSet& cFieldSet_)
	{this->Field::addField(pFile_,
						   cFieldSet_);}

////////////////////////////////////
// Interface::IScalar::
//	virtual int generate(Opt::Environment& cEnvironment_,
//						 Execution::Interface::IProgram& cProgram_,
//						 Execution::Interface::IIterator* pIterator_,
//						 Candidate::AdoptArgument& cArgument_);
//
//	virtual void setParameter(Opt::Environment& cEnvironment_,
// 							  Execution::Interface::IProgram& cProgram_,
//	 						  Execution::Interface::IIterator* pIterator_,
// 							  DExecution::Action::StatementConstruction& cExec_,
// 							  const Plan::Sql::QueryArgument& cArgument_);

	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_)
	{
		// use Field's implementation
		Field::registerToEnvironment(cEnvironment_);
	}

protected:
	// constructor
	UpdateField(Field* pField_)
		: Super(pField_, pField_->getDataType())
	{}

private:
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_UPDATEFIELD_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
