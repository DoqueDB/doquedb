// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Function.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_FUNCTION_H
#define __SYDNEY_PLAN_SCALAR_FUNCTION_H

#include "Plan/Scalar/Base.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::Function -- Interface for function
//
//	NOTES
//	This class is not created directly.
//	Implementation class of this interface is described in cpp file.
class Function
	: public Base
{
public:
	typedef Base Super;
	typedef Function This;

	// constructor
	// no arguments
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const STRING& cstrName_);
	// N-arguments
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						const STRING& cstrName_);

	// 2-arguments
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						const STRING& cstrName_);
	// 1-argument
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const STRING& cstrName_);
	// N-arguments 1-option
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						Interface::IScalar* pOption_,
						const STRING& cstrName_);
	// N-arguments N-options
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_,
						const STRING& cstrName_);
	// 2-arguments 1-option
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						Interface::IScalar* pOption_,
						const STRING& cstrName_);
	// 2-arguments N-options
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_,
						const STRING& cstrName_);
	// 1-argument 1-option
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						Interface::IScalar* pOption_,
						const STRING& cstrName_);
	// 1-argument N-options
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_,
						const STRING& cstrName_);
	// no arguments (with data type)
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// N-arguments (with data type)
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// 2-arguments (with data type)
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// 1-argument (with data type)
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// N-arguments 1-option (with data type)
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						Interface::IScalar* pOption_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// N-arguments N-options (with data type)
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// 2-arguments 1-option (with data type)
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						Interface::IScalar* pOption_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// 2-arguments N-options (with data type)
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// 1-argument 1-option (with data type)
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						Interface::IScalar* pOption_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// 1-argument N-options (with data type)
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// destructor
	virtual ~Function() {}

	// get searchable files
	static bool getSearchFile(Opt::Environment& cEnvironment_,
							  const GetFileArgument& cArgument_);
	
	static Candidate::Table* getCandidate(Opt::Environment& cEnvironment_,
										  Interface::IScalar* pFunction_,
										  Interface::ICandidate* pCandidate_);


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

	virtual void retrieveFromCascade(Opt::Environment& cEnvironment_,
									 Plan::Sql::Query* pQuery_);

	
	///////////////////////////
	// Interface::ISqlNode::		
	virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
								  const Plan::Sql::QueryArgument& cArgument_) const
	{return m_cstrName;}

protected:
	// constructor
	Function()
		: Super(Tree::Node::Undefined)
	{}
	Function(Tree::Node::Type eOperator_,
			 const STRING& cstrName_)
		: Super(eOperator_),
		  m_cstrName(cstrName_)
	{}
	Function(Tree::Node::Type eOperator_,
			 const DataType& cDataType_,
			 const STRING& cstrName_)
		: Super(eOperator_, cDataType_),
		  m_cstrName(cstrName_)
	{}
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
	void setArgument(Tree::Node::Type eOperator_,
					 const STRING& cstrName_)
	{
		setType(eOperator_);
		m_cstrName = cstrName_;
	}
	void setArgument(Tree::Node::Type eOperator_,
					 const DataType& cDataType_,
					 const STRING& cstrName_)
	{
		setType(eOperator_);
		setDataType(cDataType_);
		m_cstrName = cstrName_;
	}
#endif

	// get compatible type for the operation
	bool getCompatibleType(Interface::IScalar* pOperand_,
						   DataType& cResult_);

	// create result type
	virtual void createDataType(Opt::Environment& cEnvironment_);

/////////////////////////
// Interface::IScalar::
	virtual int generateThis(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_,
							 int iDataID_);

private:
	STRING m_cstrName;
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_FUNCTION_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
