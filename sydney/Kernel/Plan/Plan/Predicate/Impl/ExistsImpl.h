// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/ExistsImpl.h --
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_EXISTSIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_EXISTSIMPL_H

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/Exists.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::ExistsImpl -- existsImpl predicate
	//
	// NOTES
	class ExistsImpl
		: public Predicate::Exists
	{
	public:
		typedef Predicate::Exists Super;
		typedef ExistsImpl This;

		ExistsImpl(Interface::IRelation* pOperand_,
				   const Utility::RelationSet& cOuterRelation_,
				   bool bUnderExists_,
				   bool bIsNot_ = false)
			: Super(),
			  m_pOperand(pOperand_),
			  m_cOuterRelation(cOuterRelation_),
			  m_bUnderExists(bUnderExists_)
		{
			if (bIsNot_) {
				setNodeType(Tree::Node::NotExists);
			}
		}
		~ExistsImpl() {}

	////////////////////////////
	// Predicate::Exists::

	///////////////////////////////
	// Interface::IPredicate::
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void retrieve(Opt::Environment& cEnvironment_);
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Interface::ICandidate* pCandidate_);
		virtual void use(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_);
	//	virtual bool delay(Opt::Environment& cEnvironment_,
	//					   Interface::ICandidate* pCandidate_,
	//					   Scalar::DelayArgument& cArgument_);
		virtual Interface::IPredicate* convertNot(Opt::Environment& cEnvironment_);
		virtual int estimateRewrite(Opt::Environment& cEnvironment_);
		virtual Interface::IPredicate::RewriteResult
						rewrite(Opt::Environment& cEnvironment_,
								Interface::IRelation* pRelation_,
								Predicate::RewriteArgument& cArgument_);
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual bool hasSubquery();
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cCost_);

	///////////////////////////
	// Interface::IScalar::
		virtual void getUsedTable(Utility::RelationSet& cResult_);
		virtual void getUsedField(Utility::FieldSet& cResult_);
		virtual void getUnknownKey(Opt::Environment& cEnvironment_,
								   Predicate::CheckUnknownArgument& cResult_);
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

	///////////////////////////
	// Interface::IScalar::		
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);		


	//////////////////////////////////////////
	// Tree::Node::Super::

	protected:
	private:
		void checkOuterRelation(Interface::IRelation* pRelation_,
								Opt::Environment& cEnvironment_,
								const CheckArgument& cArgument_,
								Check::Value* pValue_);

		Interface::IRelation* m_pOperand;
		Utility::RelationSet m_cOuterRelation;
		bool m_bUnderExists;
	};

} // namespace Impl

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_EXISTSIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
