// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Generator.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_GENERATOR_H
#define __SYDNEY_PLAN_SCALAR_GENERATOR_H

#include "Plan/Scalar/Base.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Column;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::Generator -- Interface for generator
//
//	NOTES
//	This class is not created directly.
//	Implementation class of this interface is described in cpp file.
class Generator
	: public Base
{
public:
	typedef Base Super;
	typedef Generator This;

	struct RowID
	{
		// constructor of rowid generator
		static This* create(Opt::Environment& cEnvironment_,
							Schema::Column* pSchemaColumn_);
		struct Recovery
		{
			static This* create(Opt::Environment& cEnvironment_,
								Schema::Column* pSchemaColumn_,
								Interface::IScalar* pInput_);
		};
	};
	struct Identity
	{
		// constructor of identity generator
		static This* create(Opt::Environment& cEnvironment_,
							Schema::Column* pSchemaColumn_);
		static This* create(Opt::Environment& cEnvironment_,
							Schema::Column* pSchemaColumn_,
							Interface::IScalar* pInput_);
		struct Recovery
		{
			static This* create(Opt::Environment& cEnvironment_,
								Schema::Column* pSchemaColumn_,
								Interface::IScalar* pInput_);
		};
	};
	struct Function
	{
		// constructor of function generator
		static This* create(Opt::Environment& cEnvironment_,
							Schema::Column* pSchemaColumn_);
	};
	// destructor
	virtual ~Generator() {}

/////////////////////////
// Interface::IScalar::
	virtual const STRING& getName() {return m_cstrName;}
	virtual Check::Value check(Opt::Environment& cEnvironment_,
							   const CheckArgument& cArgument_);
	virtual bool isRefering(Interface::IRelation* pRelation_);
	virtual void getUsedTable(Utility::RelationSet& cResult_);
	virtual void getUsedField(Utility::FieldSet& cResult_);
	virtual void getUnknownKey(Opt::Environment& cEnvironment_,
							   Predicate::CheckUnknownArgument& cResult_);
	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_);
	virtual void retrieve(Opt::Environment& cEnvironment_);
	virtual void retrieve(Opt::Environment& cEnvironment_,
						  Interface::ICandidate* pCandidate_);
	virtual bool delay(Opt::Environment& cEnvironment_,
					   Interface::ICandidate* pCandidate_,
					   Scalar::DelayArgument& cArgument_);
	virtual void setMetaData(Opt::Environment& cEnvironment_,
							 Common::ColumnMetaData& cMetaData_);
//	virtual int generate(Opt::Environment& cEnvironment_,
//						 Execution::Interface::IProgram& cProgram_,
//						 Execution::Interface::IIterator* pIterator_,
//						 Candidate::AdoptArgument& cArgument_);

	///////////////////////////
	// Interface::ISqlNode::		
	virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
								  const Plan::Sql::QueryArgument& cArgument_) const
	{return m_cstrName;}

protected:
	// constructor
	Generator(const DataType& cDataType_)
		: Super(Tree::Node::Variable, cDataType_),
		  m_cstrName()
	{}

	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);

private:
	STRING m_cstrName;
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_GENERATOR_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
