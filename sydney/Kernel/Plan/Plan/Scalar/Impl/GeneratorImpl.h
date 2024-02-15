// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/GeneratorImpl.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_GENERATORIMPL_H
#define __SYDNEY_PLAN_SCALAR_GENERATORIMPL_H

#include "Plan/Scalar/Generator.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Column;
	class Table;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace GeneratorImpl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::GeneratorImpl::RowID -- implementation class for generator operation
	//
	// NOTES
	class RowID
		: public Scalar::Generator
	{
	public:
		typedef Scalar::Generator Super;
		typedef RowID This;

		// constructor
		RowID(Schema::Table* pSchemaTable_,
			  const DataType& cDataType_)
			: Super(cDataType_),
			  m_pSchemaTable(pSchemaTable_)
		{}

		// destructor
		~RowID() {}

	protected:
	private:
	//////////////////////////////
	// Scalar::Generator::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);

		Schema::Table* m_pSchemaTable;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::GeneratorImpl::Identity -- implementation class for generator operation
	//
	// NOTES
	class Identity
		: public Scalar::Generator
	{
	public:
		typedef Scalar::Generator Super;
		typedef Identity This;

		// constructor
		Identity(Schema::Column* pSchemaColumn_,
				 const DataType& cDataType_,
				 Interface::IScalar* pInput_)
			: Super(cDataType_),
			  m_pSchemaColumn(pSchemaColumn_),
			  m_pInput(pInput_)
		{}

		// destructor
		~Identity() {}

	protected:
	private:
	//////////////////////////////
	// Scalar::Generator::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);

		Schema::Column* m_pSchemaColumn;
		Interface::IScalar* m_pInput;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::GeneratorImpl::Function -- implementation class for generator operation
	//
	// NOTES
	class Function
		: public Scalar::Generator
	{
	public:
		typedef Scalar::Generator Super;
		typedef Function This;

		// constructor
		Function(Schema::Column* pSchemaColumn_,
				 const DataType& cDataType_)
			: Super(cDataType_),
			  m_pSchemaColumn(pSchemaColumn_)
		{}

		// destructor
		~Function() {}

	protected:
	private:
	//////////////////////////////
	// Scalar::Generator::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);

		Schema::Column* m_pSchemaColumn;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::GeneratorImpl::RecoveryRowID -- implementation class for generator operation
	//
	// NOTES
	class RecoveryRowID
		: public Scalar::Generator
	{
	public:
		typedef Scalar::Generator Super;
		typedef RecoveryRowID This;

		// constructor
		RecoveryRowID(Schema::Table* pSchemaTable_,
					  const DataType& cDataType_,
					  Interface::IScalar* pInput_)
			: Super(cDataType_),
			  m_pSchemaTable(pSchemaTable_),
			  m_pInput(pInput_)
		{}

		// destructor
		~RecoveryRowID() {}

	protected:
	private:
	//////////////////////////////
	// Scalar::Generator::
		virtual int generateData(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);

		Schema::Table* m_pSchemaTable;
		Interface::IScalar* m_pInput;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::GeneratorImpl::RecoveryIdentity -- implementation class for generator operation
	//
	// NOTES
	class RecoveryIdentity
		: public Scalar::Generator
	{
	public:
		typedef Scalar::Generator Super;
		typedef RecoveryIdentity This;

		// constructor
		RecoveryIdentity(Schema::Column* pSchemaColumn_,
						 const DataType& cDataType_,
						 Interface::IScalar* pInput_)
			: Super(cDataType_),
			  m_pSchemaColumn(pSchemaColumn_),
			  m_pInput(pInput_)
		{}

		// destructor
		~RecoveryIdentity() {}

	protected:
	private:
	//////////////////////////////
	// Scalar::Generator::
		virtual int generateData(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);

		Schema::Column* m_pSchemaColumn;
		Interface::IScalar* m_pInput;
	};
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_GENERATORIMPL_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
