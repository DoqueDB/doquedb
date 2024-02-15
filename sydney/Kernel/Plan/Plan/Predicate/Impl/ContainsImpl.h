// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/ContainsImpl.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_CONTAINSIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_CONTAINSIMPL_H

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/Contains.h"

#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Nadic.h"
#include "Plan/Tree/Option.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::ContainsImpl -- containsImpl predicate
	//
	// NOTES
	class ContainsImpl
		: public BaseWithOption< Tree::Dyadic<Predicate::Contains, Interface::IScalar>,
								 Tree::Nadic<Tree::Option, Interface::IScalar> >
	{
	public:
		typedef BaseWithOption< Tree::Dyadic<Predicate::Contains, Interface::IScalar>,
								Tree::Nadic<Tree::Option, Interface::IScalar> > Super;
		typedef ContainsImpl This;

		ContainsImpl(const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
					 const VECTOR<Interface::IScalar*>& vecOption_)
			: Super(cOperand_, vecOption_),
			  m_pExpand(0),
			  m_pRankFrom(0),
			  m_pKwicOption(0)
		{}
		~ContainsImpl() {}

	////////////////////////////
	// Predicate::Contains::
		virtual void setExpand(Interface::IRelation* pQuery_)
		{m_pExpand = pQuery_;}
		virtual void setRankFrom(Interface::IRelation* pQuery_)
		{m_pRankFrom = pQuery_;}

		virtual void createKwicOption(Opt::Environment& cEnvironment_,
									  Interface::IScalar* pKwicSize_);
		virtual File::KwicOption* getKwicOption() {return m_pKwicOption;}

	///////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual void adoptIndex(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Action::FileAccess* pFileAccess_,
								Candidate::File* pFile_,
								Candidate::AdoptArgument& cArgument_);

	///////////////////////////
	// Interface::IScalar::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;		

	//////////////////////////////////////////
	// Tree::Node::Super::
		virtual ModSize getOptionSize() const;
		virtual const Tree::Node::Super* getOptionAt(ModInt32 iPosition_) const;

	protected:
	private:
		// find language column of operand
		bool findLanguageColumn(Opt::Environment& cEnvironment_,
								Candidate::Table* pTable_,
								Interface::IFile* pFile_);

	/////////////////////////////////////////
	// Predicate::Contains::
		virtual void addToEnvironment(Opt::Environment& cEnvironment_);

		Interface::IRelation* m_pExpand;
		Interface::IRelation* m_pRankFrom;
		File::KwicOption* m_pKwicOption;
	};

} // namespace Impl

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_CONTAINSIMPL_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
