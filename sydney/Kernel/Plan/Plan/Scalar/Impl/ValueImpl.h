// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/ValueImpl.h --
// 
// Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_VALUEIMPL_H
#define __SYDNEY_PLAN_SCALAR_VALUEIMPL_H

#include "Plan/Scalar/Value.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace ValueImpl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::ValueImpl::Base -- base class for implementation classes
	//
	// NOTES
	class Base
		: public Scalar::Value
	{
	public:
		typedef Scalar::Value Super;
		typedef Base This;

		// constructor
		explicit Base(Type eType_)
			: Super(eType_)
		{}
		Base(Type eType_, const DataType& cType_)
			: Super(eType_, cType_)
		{}

		// destructor
		virtual ~Base() {}

	/////////////////////////////////////
	// Interface::IScalar
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
	//	virtual const STRING& getName();
	//	virtual Interface::IScalar* setExpectedType(Opt::Environment& cEnvironment_,
	//												const DataType& cType_);
	//	virtual Interface::IScalar* createCast(Opt::Environment& cEnvironment_,
	//										   const DataType& cToType_,
	//										   bool bForComparison_,
	//										   Tree::Node::Type eType_ = Tree::Node::Undefined);
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
	//	virtual Common::Data::Pointer preCalculate(Opt::Environment& cEnvironment_);
		virtual void setMetaData(Opt::Environment& cEnvironment_,
								 Common::ColumnMetaData& cMetaData_);
	//	virtual int generate(Opt::Environment& cEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Execution::Interface::IIterator* pIterator_,
	//						 Candidate::AdoptArgument& cArgument_);

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_);		
		

	/////////////////////////////////////
	// Node::
	//	virtual ModUnicodeString getValue() const;
	//	virtual const Common::Data* getData() const;
	protected:
	private:
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::ValueImpl::Constant -- implementation class for constant value
	//
	// NOTES
	class Constant
		: public Base
	{
	public:
		typedef Base Super;
		typedef Constant This;

		// constructor
		explicit Constant(const DataPointer& pData_)
			: Super(Tree::Node::ConstantValue),
			  m_pData(pData_),
			  m_cstrSQL()
		{
			Common::SQLData cType;
			if (!pData_->isNull()
				&& !pData_->isDefault()
				&& pData_->getSQLType(cType)) {
				setDataType(DataType(cType));
			} else {
				setDataType(DataType(pData_->getType()));
			}
		}

		explicit Constant(const DataPointer& pData_,
						  const STRING& cstrSQL_)
			: Super(Tree::Node::ConstantValue),
			  m_pData(pData_),
			  m_cstrSQL(cstrSQL_)
		{
			Common::SQLData cType;
			if (!pData_->isNull()
				&& !pData_->isDefault()
				&& pData_->getSQLType(cType)) {
				setDataType(DataType(cType));
			} else {
				setDataType(DataType(pData_->getType()));
			}
		}		

		// destructor
		~Constant() {} // no subclasses

	/////////////////////////////////////
	// Interface::IScalar
		virtual const STRING& getName();
		virtual Interface::IScalar* setExpectedType(Opt::Environment& cEnvironment_,
													const DataType& cType_);

		virtual Interface::IScalar* createCast(Opt::Environment& cEnvironment_,
											   const DataType& cToType_,
											   bool bForComparison_,
											   Tree::Node::Type eType_ = Tree::Node::Undefined);

		virtual Common::Data::Pointer preCalculate(Opt::Environment& cEnvironment_);

		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

			

	/////////////////////////////////////
	// Node::
		virtual ModUnicodeString getValue() const;
		virtual const Common::Data* getData() const;

	protected:
	/////////////////////////////////////
	// Interface::IScalar
		virtual int generateData(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);
	private:
		DataPointer m_pData;
		STRING m_cstrSQL;
	};


	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::ValueImpl::Asterisk -- implementation class for asterisk
	//
	// NOTES	
	class Asterisk
		:public Constant
	{
	public:
		typedef Constant Super;
		typedef Asterisk This;

		// Constant
		Asterisk()
			: Super(new Common::StringData(ModUnicodeString("*")))
		{}

		virtual ~Asterisk()
		{}

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::ValueImpl::Variable -- implementation class for variable
	//
	// NOTES
	class Variable
		: public Base
	{
	public:
		typedef Base Super;
		typedef Variable This;

		// constructor
		Variable(const STRING& cstrName_)
			: Super(Tree::Node::Variable),
			  m_pData(),
			  m_cstrName(cstrName_)
		{}
		Variable(const STRING& cstrName_,
				 const DataType& cDataType_)
			: Super(Tree::Node::Variable, cDataType_),
			  m_pData(),
			  m_cstrName(cstrName_)
		{}
		// destructor
		virtual ~Variable() {}

	//////////////////////////
	// Scalar::Value::
		virtual void setData(const DataPointer& pData_);

	/////////////////////////////////////
	// Interface::IScalar
		virtual const STRING& getName();
		virtual Interface::IScalar* setExpectedType(Opt::Environment& cEnvironment_,
													const DataType& cType_);
	//	virtual Interface::IScalar* createCast(Opt::Environment& cEnvironment_,
	//										   const DataType& cToType_,
	//										   bool bForComparison_,
	//										   Tree::Node::Type eType_ = Tree::Node::Undefined);
		virtual Common::Data::Pointer preCalculate(Opt::Environment& cEnvironment_);

	///////////////////////////
	// Interface::ISqlNode::			
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const
		{ return const_cast<Variable*>(this)->getName();}			

		
	/////////////////////////////////////
	// Node::
		virtual ModUnicodeString getValue() const;
		virtual const Common::Data* getData() const;

	protected:
	/////////////////////////////////////
	// Interface::IScalar
		virtual int generateData(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);
	private:
		DataPointer m_pData;
		STRING m_cstrName;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::ValueImpl::PlaceHolder -- implementation class for place holder
	//
	// NOTES
	class PlaceHolder
		: public Variable
	{
	public:
		typedef Variable Super;
		typedef PlaceHolder This;

		// constructor
		PlaceHolder(const STRING& cstrName_,
					int iNumber_)
			: Super(cstrName_),
			  m_iNumber(iNumber_)
		{}

		// destructor
		~PlaceHolder() {} // no subclasses

	/////////////////////////////////////
	// Interface::IScalar
		virtual bool hasParameter();

	/////////////////////////////////////
	// Interface::ISqlNode		
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_);

	protected:
	/////////////////////////////////////
	// Interface::IScalar
		virtual int generateData(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);
	private:
		int m_iNumber;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::ValueImpl::BulkVariable -- implementation class for variable
	//
	// NOTES
	class BulkVariable
		: public Variable
	{
	public:
		typedef Variable Super;
		typedef BulkVariable This;

		// constructor
		BulkVariable(const STRING& cstrName_)
			: Super(cstrName_)
		{}
		BulkVariable(const STRING& cstrName_,
					 const DataType& cDataType_)
			: Super(cstrName_, cDataType_)
		{}
		// destructor
		~BulkVariable() {}

	//////////////////////////
	// Scalar::Value::

	/////////////////////////////////////
	// Interface::IScalar
	//	virtual const STRING& getName();
	//	virtual Interface::IScalar* setExpectedType(Opt::Environment& cEnvironment_,
	//												const DataType& cType_);
		virtual Interface::IScalar* createCast(Opt::Environment& cEnvironment_,
											   const DataType& cToType_,
											   bool bForComparison_,
											   Tree::Node::Type eType_ = Tree::Node::Undefined);
	private:
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::ValueImpl::RelationVariable -- implementation class for variable
	//
	// NOTES
	class RelationVariable
		: public Variable
	{
	public:
		typedef Variable Super;
		typedef RelationVariable This;

		// constructor
		RelationVariable(Interface::IRelation* pRelation_,
						 const STRING& cstrName_)
			: Super(cstrName_),
			  m_pRelation(pRelation_)
		{}
		RelationVariable(Interface::IRelation* pRelation_,
						 const STRING& cstrName_,
						 const DataType& cDataType_)
			: Super(cstrName_, cDataType_),
			  m_pRelation(pRelation_)
		{}
		// destructor
		~RelationVariable() {}

	//////////////////////////
	// Scalar::Value::

	/////////////////////////////////////
	// Interface::IScalar
		virtual Check::Value check(Opt::Environment& cEnvironment_,
								   const CheckArgument& cArgument_);
		virtual bool isRefering(Interface::IRelation* pRelation_);
		virtual void getUsedTable(Utility::RelationSet& cResult_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_);
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Interface::ICandidate* pCandidate_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Interface::ICandidate* pCandidate_,
						   Scalar::DelayArgument& cArgument_);

	private:
		Interface::IRelation* m_pRelation;
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::ValueImpl::SessionVariable -- implementation class for variable
	//
	// NOTES
	class SessionVariable
		: public Variable
	{
	public:
		typedef Variable Super;
		typedef SessionVariable This;

		// constructor
		SessionVariable(const STRING& cstrName_)
			: Super(cstrName_)
		{}
		SessionVariable(const STRING& cstrName_,
						const DataType& cDataType_)
			: Super(cstrName_, cDataType_)
		{}
		// destructor
		~SessionVariable() {}

	//////////////////////////
	// Scalar::Value::

	/////////////////////////////////////
	// Interface::IScalar
	//	virtual const STRING& getName();
	//	virtual Interface::IScalar* setExpectedType(Opt::Environment& cEnvironment_,
	//												const DataType& cType_);
	//	virtual Interface::IScalar* createCast(Opt::Environment& cEnvironment_,
	//										   const DataType& cToType_,
	//										   bool bForComparison_,
	//										   Tree::Node::Type eType_ = Tree::Node::Undefined);

	/////////////////////////////////////
	// Interface::ISqlNode
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);					
	private:
	};
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_VALUEIMPL_H

//
//	Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
