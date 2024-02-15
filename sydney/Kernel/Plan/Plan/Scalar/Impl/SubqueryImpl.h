// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/SubqueryImpl.h --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_SUBQUERYIMPL_H
#define __SYDNEY_PLAN_SCALAR_SUBQUERYIMPL_H

#include "Plan/Scalar/Subquery.h"

#include "Exception/NotSupported.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace Impl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::Impl::SubqueryImpl -- implementation classes of Scalar::Subquery
	//
	// NOTES
	class SubqueryImpl
		: public Scalar::Subquery
	{
	public:
		typedef Scalar::Subquery Super;
		typedef SubqueryImpl This;

		// constructor
		SubqueryImpl(Interface::IRelation* pSubRelation_,
					 const Utility::RelationSet& cOuterRelation_,
					 int iPosition_,
					 const STRING& cstrName_)
			: Super(),
			  m_pSubRelation(pSubRelation_),
			  m_cOuterRelation(cOuterRelation_),
			  m_iPosition(iPosition_),
			  m_cstrName(cstrName_)
		{}

		// destructor
		~SubqueryImpl() {}

	/////////////////////////////////////
	// Interface::IScalar
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual const STRING& getName();
	//	virtual Interface::IScalar* setExpectedType(Opt::Environment& cEnvironment_,
	//												const Scalar::DataType& cType_);
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

		virtual bool isSubquery() const {return true;}

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

		
			

	/////////////////////////////////////
	// Node::
	//	virtual ModUnicodeString getSubquery() const;
	//	virtual const Common::Data* getData() const;
	protected:
	////////////////////////
	// Interface::IScalar::
		virtual int generateData(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);
	private:
		void checkOuterRelation(Interface::IRelation* pRelation_,
								const CheckArgument& cArgument_,
								Check::Value* pValue_);

		Interface::IRelation* m_pSubRelation;
		Utility::RelationSet m_cOuterRelation;
		int m_iPosition;
		STRING m_cstrName;
	};
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_SUBQUERYIMPL_H

//
//	Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
