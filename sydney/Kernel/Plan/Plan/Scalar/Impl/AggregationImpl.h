// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/AggregationImpl.h --
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

#ifndef __SYDNEY_PLAN_SCALAR_AGGREGATIONIMPL_H
#define __SYDNEY_PLAN_SCALAR_AGGREGATIONIMPL_H

#include "Plan/Scalar/Aggregation.h"
#include "Plan/Scalar/Impl/FunctionImpl.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace AggregationImpl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::AggregationImpl::Monadic -- implementation class for aggregation operations
	//
	// NOTES
	class Monadic
		: public FunctionImpl::Base< Tree::Monadic<Scalar::Aggregation, Interface::IScalar> >
	{
	public:
		typedef FunctionImpl::Base< Tree::Monadic<Scalar::Aggregation, Interface::IScalar> > Super;
		typedef Monadic This;

		// constructor
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
		Monadic()
			: Super(),
			  m_pConverted(0),
			  m_pConvertedForDist(0)
		{}
#endif
		Monadic(Tree::Node::Type eType_,
				const STRING& cstrName_,
				Interface::IScalar* pOperand_)
			: Super(eType_, cstrName_, pOperand_),
			  m_pConverted(0),
			  m_pConvertedForDist(0)
		{}
		Monadic(Tree::Node::Type eType_,
				const DataType& cDataType_,
				const STRING& cstrName_,
				Interface::IScalar* pOperand_)
			: Super(eType_, cDataType_, cstrName_, pOperand_),
			  m_pConverted(0),
			  m_pConvertedForDist(0)			  
		{}
		// destructor
		virtual ~Monadic() {}

	////////////////////////
	// Interface::IScalar::
		virtual Interface::IScalar* convertFunction(Opt::Environment& cEnvironment_,
													Interface::IRelation* pRelation_,
													Interface::IScalar* pFunction_,
													Schema::Field::Function::Value eFunction_);

		
		virtual void clearConvert(Opt::Environment& cEnvironment_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Interface::ICandidate* pCandidate_,
						   Scalar::DelayArgument& cArgument_);

	protected:
	////////////////////////
	// Scalar::Function::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
		// generate main
		int generateFunction(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_,
							 int iDataID_);

	////////////////////////
	// Scalar::Function::
		virtual void createDataType(Opt::Environment& cEnvironment_);

	private:
		// is use all operand values
		bool isCountAll(Opt::Environment& cEnvironment_);
		
		Scalar::Field* getBitSetField(Opt::Environment& cEnvironment_,
									  Candidate::AdoptArgument& cArgument_);

		Interface::IScalar* m_pConverted;
		Interface::IScalar* m_pConvertedForDist;
	};

	class MonadicDistribution
		: public Monadic
	{
	public:
		typedef Monadic Super;
		typedef MonadicDistribution This;

		// constructor
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
		MonadicDistribution()
			: Super()
		{}
#endif
		MonadicDistribution(Tree::Node::Type eType_,
				const STRING& cstrName_,
				Interface::IScalar* pOperand_)
			: Super(eType_, cstrName_, pOperand_)
		{}
		MonadicDistribution(Tree::Node::Type eType_,
				const DataType& cDataType_,
				const STRING& cstrName_,
				Interface::IScalar* pOperand_)
			: Super(eType_, cDataType_, cstrName_, pOperand_)
		{}
		// destructor
		virtual ~MonadicDistribution() {}

	protected:
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	};
	
		
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::AggregationImpl::MonadicWithMonadicOption --
	//					implementation class for aggregation operations
	//
	// NOTES
	class MonadicWithMonadicOption
		: public Tree::MonadicOption<Monadic, Interface::IScalar>
	{
	public:
		typedef Tree::MonadicOption<Monadic, Interface::IScalar> Super;
		typedef MonadicWithMonadicOption This;

		// constructor
		MonadicWithMonadicOption(Tree::Node::Type eType_,
								 const STRING& cstrName_,
								 Interface::IScalar* pOperand_,
								 Interface::IScalar* pOption_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(pOption_)
		{
			setOperand(pOperand_);
			setArgument(eType_, cstrName_);
		}
#else
			: Super(eType_, cstrName_, pOperand_, pOption_)
		{}
#endif
		MonadicWithMonadicOption(Tree::Node::Type eType_,
								 const DataType& cDataType_,
								 const STRING& cstrName_,
								 Interface::IScalar* pOperand_,
								 Interface::IScalar* pOption_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(pOption_)
		{
			setOperand(pOperand_);
			setArgument(eType_, cDataType_, cstrName_);
		}
#else
			: Super(eType_, cDataType_, cstrName_, pOperand_, pOption_)
		{}
#endif

		// destructor
		~MonadicWithMonadicOption() {}

	////////////////////////
	// Interface::IScalar::
		virtual Interface::IScalar* convertFunction(Opt::Environment& cEnvironment_,
													Interface::IRelation* pRelation_,
													Interface::IScalar* pFunction_,
													Schema::Field::Function::Value eFunction_)
		{return 0;}

	protected:
	////////////////////////
	// Scalar::Function::
	private:
	};
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_AGGREGATIONIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
