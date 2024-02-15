// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/CastImpl.h --
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

#ifndef __SYDNEY_PLAN_SCALAR_CASTIMPL_H
#define __SYDNEY_PLAN_SCALAR_CASTIMPL_H

#include "Plan/Scalar/Impl/FunctionImpl.h"
#include "Plan/Scalar/Cast.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace CastImpl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::CastImpl::Monadic -- implementation class for cast operations
	//
	// NOTES
	class Monadic
		: public FunctionImpl::Base< Tree::Monadic<Scalar::Cast, Interface::IScalar> >
	{
	public:
		typedef FunctionImpl::Base< Tree::Monadic<Scalar::Cast, Interface::IScalar> > Super;
		typedef Monadic This;

		// constructor
		Monadic(const DataType& cDataType_,
				Interface::IScalar* pOperand_,
				bool bForComparison_,
				bool bNoThrow_)
			: Super(Tree::Node::Cast, cDataType_, STRING(), pOperand_),
			  m_bForComparison(bForComparison_),
			  m_bNoThrow(bNoThrow_)
		{}

		// destructor
		~Monadic() {}

	/////////////////////////
	// Interface::IScalar::


	/////////////////////////
	// Interface::ISqlNode::
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);
		
		virtual void retrieveFromCascade(Opt::Environment& cEnvironment_,
										 Plan::Sql::Query* pQuery_);		
	protected:
		// generate
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	private:
		bool m_bForComparison;
		bool m_bNoThrow;
	};

} // namespace CastImpl

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_CASTIMPL_H

//
//	Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
