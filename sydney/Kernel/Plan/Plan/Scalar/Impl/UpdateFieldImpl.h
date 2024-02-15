// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/UpdateFieldImpl.h --
// 
// Copyright (c) 2015, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_UPDATEFIELDIMPL_H
#define __SYDNEY_PLAN_SCALAR_UPDATEFIELDIMPL_H

#include "Plan/Scalar/UpdateField.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace UpdateFieldImpl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::UpdateFieldImpl::Base -- base class for implementation classes
	//
	// NOTES
	class Base
		: public Scalar::UpdateField
	{
	public:
		typedef Scalar::UpdateField Super;
		typedef Base This;

		// constructor
		explicit Base(Field* pField_)
			: Super(pField_),
			  m_pChecked(0)
		{}

		// destructor
		virtual ~Base() {}

	/////////////////////////////////////
	// UpdateField::
	//	virtual Interface::IScalar* getInput() = 0;
	//	virtual Interface::IScalar* getOriginal() = 0;

	////////////////////////////////////
	// Field::
		using Super::getField;
		virtual Field* getField(Interface::IFile* pFile_);
		virtual Field* checkPut(Opt::Environment& cEnvironment_);

	/////////////////////////////////////
	// Interface::IScalar
	//	virtual int generate(Opt::Environment& cEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Execution::Interface::IIterator* pIterator_,
	//						 Candidate::AdoptArgument& cArgument_);

	protected:
		// generate operand for updating operations
		int generateOperand(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_,
							Interface::IScalar* pOperand_);
	private:
		Field* m_pChecked;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::UpdateFieldImpl::Insert -- implementation class for insert updateField
	//
	// NOTES
	class Insert
		: public Base
	{
	public:
		typedef Base Super;
		typedef Insert This;
		typedef Interface::IScalar* Argument;

		// constructor
		Insert(Field* pField_,
			   Argument pInput_)
			: Super(pField_),
			  m_pInput(pInput_)
		{}

		// destructor
		virtual ~Insert() {}

	/////////////////////////////////////
	// UpdateField::
		virtual Interface::IScalar* getInput() {return m_pInput;}
		virtual Interface::IScalar* getOriginal() {return 0;}

	/////////////////////////////////////
	// Interface::IScalar
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

	protected:
	private:
		Interface::IScalar* m_pInput;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::UpdateFieldImpl::Delete -- implementation class for delete updateField
	//
	// NOTES
	class Delete
		: public Base
	{
	public:
		typedef Base Super;
		typedef Delete This;
		typedef Interface::IScalar* Argument;

		// constructor
		Delete(Field* pField_,
			   Argument pOriginal_)
			: Super(pField_),
			  m_pOriginal(pOriginal_)
		{}

		// destructor
		virtual ~Delete() {}

	/////////////////////////////////////
	// UpdateField::
		virtual Interface::IScalar* getInput() {return 0;}
		virtual Interface::IScalar* getOriginal() {return m_pOriginal;}

	/////////////////////////////////////
	// Interface::IScalar
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

	protected:
	private:
		Interface::IScalar* m_pOriginal;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::UpdateFieldImpl::Update -- implementation class for update updateField
	//
	// NOTES
	class Update
		: public Base
	{
	public:
		typedef Base Super;
		typedef Update This;
		typedef const PAIR<Interface::IScalar*, Interface::IScalar*>& Argument;

		// constructor
		Update(Field* pField_,
			   Argument cInput_)
			: Super(pField_),
			  m_pInput(cInput_.first),
			  m_pOriginal(cInput_.second)
		{}

		// destructor
		virtual ~Update() {}

	/////////////////////////////////////
	// UpdateField::
		virtual Interface::IScalar* getInput() {return m_pInput;}
		virtual Interface::IScalar* getOriginal() {return m_pOriginal;}

	/////////////////////////////////////
	// Interface::IScalar
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

	protected:
	private:
		Interface::IScalar* m_pInput;
		Interface::IScalar* m_pOriginal;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::UpdateFieldImpl::UpdateByOperation -- implementation class for update updateField
	//
	// NOTES
	class UpdateByOperation
		: public Update
	{
	public:
		typedef Update Super;
		typedef UpdateByOperation This;

		// constructor
		UpdateByOperation(Field* pField_,
						  Super::Argument cInput_)
			: Super(pField_, cInput_),
			  m_pOperation(0),
			  m_pRevert(0)
		{}

		// destructor
		~UpdateByOperation() {} // no subclasses

		// set operation nodes
		This* setOperation(Opt::Environment& cEnvironment_);

	/////////////////////////////////////
	// UpdateField::
		virtual Interface::IScalar* getInput();
		virtual Interface::IScalar* getOriginal();

	/////////////////////////////////////
	// Interface::IScalar
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

		virtual bool isOperation();

	protected:
	private:
		Interface::IScalar* m_pOperation;
		Interface::IScalar* m_pRevert;
	};
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_UPDATEFIELDIMPL_H

//
//	Copyright (c) 2015, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
