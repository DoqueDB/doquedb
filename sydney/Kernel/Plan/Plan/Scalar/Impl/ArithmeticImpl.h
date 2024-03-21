// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/ArithmeticImpl.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_ARITHMETICIMPL_H
#define __SYDNEY_PLAN_SCALAR_ARITHMETICIMPL_H

#include "Plan/Scalar/Arithmetic.h"
#include "Plan/Scalar/Impl/FunctionImpl.h"

#include "Common/Assert.h"
#include "Exception/NotCompatible.h"
#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN


namespace ArithmeticImpl
{
	////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Scalar::ArithmeticImpl::Base -- base class for arithmetic operations
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class Base
		: public FunctionImpl::Base<Handle_>
	{
	public:
		typedef FunctionImpl::Base<Handle_> Super;
		typedef Base<Handle_> This;

		// constructor
		Base(Tree::Node::Type eType_,
			 const STRING& cstrName_,
			 typename Handle_::Argument cArgument_)
			: Super(eType_, cstrName_, cArgument_)
		{}
		Base(Tree::Node::Type eType_,
				const DataType& cDataType_,
				const STRING& cstrName_,
				typename Handle_::Argument cArgument_)
			: Super(eType_, cDataType_, cstrName_, cArgument_)
		{}
		// destructor
		virtual ~Base() {}

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_)
		{
			switch(getType()) {
			case Tree::Node::Add:
			case Tree::Node::Subtract:
			case Tree::Node::Multiply:
			case Tree::Node::Divide:				
				cExec_.append("(");
				getOperandi(0)->setParameter(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cExec_,
											 cArgument_);
				cExec_.append(_getOperatorName(getType()));
				getOperandi(1)->setParameter(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cExec_,
											 cArgument_);
				cExec_.append(")");
				break;
			case Tree::Node::Absolute:
			case Tree::Node::Modulus:
			case Tree::Node::Negative:									
				Super::setParameter(cEnvironment_,
									cProgram_,
									pIterator_,
									cExec_,
									cArgument_);
				break;
			default:
				_SYDNEY_THROW0(Exception::NotSupported);				
			}
		}

		using Super::getType;
		using Super::getOperandi;
		using Super::setDataType;
		using Super::isAll;
		using Super::mapOperand;


	protected:
	////////////////////////
	// Scalar::Function::
		virtual void createDataType(Opt::Environment& cEnvironment_)
		{
			// set compatible type of all operands
			DataType cResult;
			if (!isAll(boost::bind(&This::getCompatibleType,
								   this,
								   _1,
								   boost::ref(cResult)))) {
				// if any operand has incompatible type, creating fails
				const char srcFile[] = __FILE__;
				const char moduleName[] = "Plan::Scalar";

				_SYDNEY_THROW0(Exception::NotCompatible);
			}
			setDataType(cResult);

			mapOperand(boost::bind(&This::castOperand,
								   this,
								   boost::ref(cEnvironment_),
								   _1));
		}


	private:
	};

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::ArithmeticImpl::Monadic -- function class for arithmetic operations
	//
	// NOTES
	typedef Base< Tree::Monadic<Arithmetic, Interface::IScalar> > Monadic;

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::ArithmeticImpl::Dyadic -- function class for arithmetic operations
	//
	// NOTES
	class Dyadic
		: public ArithmeticImpl::Base< Tree::Dyadic<Arithmetic, Interface::IScalar> >
	{
	public:
		typedef Dyadic This;
		typedef ArithmeticImpl::Base< Tree::Dyadic<Arithmetic, Interface::IScalar> > Super;

		// constructor
		Dyadic(Tree::Node::Type eType_,
			   const STRING& cstrName_,
			   Super::Argument cArgument_)
			: Super(eType_, cstrName_, cArgument_)
		{}
		Dyadic(Tree::Node::Type eType_,
			   const DataType& cDataType_,
			   const STRING& cstrName_,
			   Super::Argument cArgument_)
			: Super(eType_, cDataType_, cstrName_, cArgument_)
		{}

	protected:

	////////////////////////
	// Scalar::Function::
		virtual void createDataType(Opt::Environment& cEnvironment_)
		{
			if (getType() == Tree::Node::Modulus) {
				// use second operand's type
				setDataType(getOperand1()->getDataType());
			} else {
				Super::createDataType(cEnvironment_);
			}
		}
	private:
	};
	
	

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::ArithmeticImpl::Nadic -- function class for arithmetic operations
	//
	// NOTES
	typedef Base< Tree::Nadic<Arithmetic, Interface::IScalar> > Nadic;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_ARITHMETICIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
