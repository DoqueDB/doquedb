// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/ArrayImpl.h --
// 
// Copyright (c) 2010, 2011, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_ARRAYIMPL_H
#define __SYDNEY_PLAN_SCALAR_ARRAYIMPL_H

#include "Plan/Scalar/Impl/FunctionImpl.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace ArrayImpl
{
	namespace Constructor
	{
		////////////////////////////////////////
		// CLASS
		//	Plan::Scalar::ArrayImpl::Constructor::Base -- base class for array operations
		//
		// NOTES
		class Base
			: public Scalar::Function
		{
		public:
			typedef Scalar::Function Super;
			typedef Base This;

			// destructor
			virtual ~Base() {}

		/////////////////////////
		// Interface::IScalar::
			virtual int generate(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);
		protected:
			// constructor
			Base() : Super() {}
			Base(Tree::Node::Type eType_,
				 const STRING& cstrName_)
				: Super(eType_, cstrName_)
			{}
			Base(Tree::Node::Type eType_,
				 const DataType& cDataType_,
				 const STRING& cstrName_)
				: Super(eType_, cDataType_, cstrName_)
			{}

			// get compatible type for the operation
			bool getCompatibleType(Interface::IScalar* pOperand_,
								   DataType& cResult_);
			// generate element data
			int generateElement(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
								const DataType& cElementType_,
								Interface::IScalar* pElement_);
		private:
			// generate element data
			virtual void generateElements(Opt::Environment& cEnvironment_,
										  Execution::Interface::IProgram& cProgram_,
										  Execution::Interface::IIterator* pIterator_,
										  Candidate::AdoptArgument& cArgument_,
										  const DataType& cElementType_,
										  VECTOR<int>& vecResult_) = 0;
		};

		////////////////////////////////////////
		// TEMPLATE CLASS
		//	Plan::Scalar::ArrayImpl::Constructor::Generic -- function class for array operations
		//
		// TEMPLATE ARGUMENTS
		//	class Handle_
		//
		// NOTES
		template <class Handle_>
		class Generic
			: public FunctionImpl::Base<Handle_>
		{
		public:
			typedef FunctionImpl::Base<Handle_> Super;
			typedef Generic<Handle_> This;

			// constructor
			Generic(Tree::Node::Type eType_,
					const STRING& cstrName_,
					typename Handle_::Argument cArgument_)
				: Super(eType_, cstrName_, cArgument_)
			{}
			Generic(Tree::Node::Type eType_,
					const DataType& cDataType_,
					const STRING& cstrName_,
					typename Handle_::Argument cArgument_)
				: Super(eType_, cDataType_, cstrName_, cArgument_)
			{}
			// destructor
			virtual ~Generic() {}

			using Super::getSize;
			using Super::isAll;
			using Super::setDataType;
			using Super::foreachOperand;


		protected:

		protected:

		protected:
		private:
		////////////////////////
		// Scalar::Function::
			virtual void createDataType(Opt::Environment& cEnvironment_)
			{
				const char srcFile[] = __FILE__;
				const char moduleName[] = "Plan::Scalar";

				DataType cResult;
				if (!isAll(boost::bind(&This::getCompatibleType,
									   this,
									   _1,
									   boost::ref(cResult)))) {
					// if any operand has incompatible type, creating fails
					_SYDNEY_THROW0(Exception::NotSupported);
				}
				if (cResult.isCharacterStringType()) {
					// when element type is character string, set as unlimited
					cResult.setFlag(DataType::Flag::Unlimited);
					cResult.setLength(0);
				}
				setDataType(DataType::getArrayType(getSize(), cResult));
			}

		//////////////////////////////////////
		// ArrayImpl::Constructor::Base::
			virtual void generateElements(Opt::Environment& cEnvironment_,
										  Execution::Interface::IProgram& cProgram_,
										  Execution::Interface::IIterator* pIterator_,
										  Candidate::AdoptArgument& cArgument_,
										  const DataType& cElementType_,
										  VECTOR<int>& vecResult_)
			{
				foreachOperand(boost::bind(&VECTOR<int>::PUSHBACK,
										   &vecResult_,
										   boost::bind(&This::generateElement,
													   this,
													   boost::ref(cEnvironment_),
													   boost::ref(cProgram_),
													   pIterator_,
													   boost::ref(cArgument_),
													   boost::cref(cElementType_),
													   _1)));
			}
		};

		/////////////////////////////////////////////////////////////////////////////////////
		// TYPEDEF
		//	Plan::Scalar::ArrayImpl::Constructor::Monadic -- function class for array operations
		//
		// NOTES
		typedef Generic< Tree::Monadic<Base, Interface::IScalar> > Monadic;

		/////////////////////////////////////////////////////////////////////////////////////
		// TYPEDEF
		//	Plan::Scalar::ArrayImpl::Constructor::Dyadic -- function class for array operations
		//
		// NOTES
		typedef Generic< Tree::Dyadic<Base, Interface::IScalar> > Dyadic;

		/////////////////////////////////////////////////////////////////////////////////////
		// TYPEDEF
		//	Plan::Scalar::ArrayImpl::Constructor::Nadic -- function class for array operations
		//
		// NOTES
		typedef Generic< Tree::Nadic<Base, Interface::IScalar> > Nadic;

	} // namespace Constructor

	namespace Element
	{
		////////////////////////////////////////
		// CLASS
		//	Plan::Scalar::ArrayImpl::Element::Base -- function class for array operations
		//
		// NOTES
		class Base
			: public FunctionImpl::MonadicWithMonadicOption
		{
		public:
			typedef FunctionImpl::MonadicWithMonadicOption Super;
			typedef Base This;

			// constructor
			Base(Tree::Node::Type eType_,
				 const STRING& cstrName_,
				 Interface::IScalar* pOperand_,
				 Interface::IScalar* pOption_)
				: Super(eType_, cstrName_, pOperand_, pOption_)
			{}
			Base(Tree::Node::Type eType_,
				 const DataType& cDataType_,
				 const STRING& cstrName_,
				 Interface::IScalar* pOperand_,
				 Interface::IScalar* pOption_)
				: Super(eType_, cDataType_, cstrName_, pOperand_, pOption_)
			{}
			// destructor
			virtual ~Base() {}

		protected:
		private:
		////////////////////////
		// Scalar::Function::
			virtual void createDataType(Opt::Environment& cEnvironment_);
		};

		////////////////////////////////////////
		// CLASS
		//	Plan::Scalar::ArrayImpl::Element::Reference -- function class for array operations
		//
		// NOTES
		class Reference
			: public Base
		{
		public:
			typedef Base Super;
			typedef Reference This;

			// constructor
			Reference(Tree::Node::Type eType_,
					  const STRING& cstrName_,
					  Interface::IScalar* pOperand_,
					  Interface::IScalar* pOption_)
				: Super(eType_, cstrName_, pOperand_, pOption_)
			{}
			Reference(Tree::Node::Type eType_,
					  const DataType& cDataType_,
					  const STRING& cstrName_,
					  Interface::IScalar* pOperand_,
					  Interface::IScalar* pOption_)
				: Super(eType_, cDataType_, cstrName_, pOperand_, pOption_)
			{}
			// destructor
			virtual ~Reference() {}

		protected:
		////////////////////////
		// Scalar::Function::
			virtual int generateThis(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Candidate::AdoptArgument& cArgument_,
									 int iDataID_);
		private:
		};

		////////////////////////////////////////
		// CLASS
		//	Plan::Scalar::ArrayImpl::Element::Arbitrary -- function class for array operations
		//
		// NOTES
		class Arbitrary
			: public Base
		{
		public:
			typedef Base Super;
			typedef Arbitrary This;

			// constructor
			Arbitrary(Tree::Node::Type eType_,
					  const STRING& cstrName_,
					  Interface::IScalar* pOperand_,
					  Interface::IScalar* pOption_)
				: Super(eType_, cstrName_, pOperand_, pOption_)
			{}
			Arbitrary(Tree::Node::Type eType_,
					  const DataType& cDataType_,
					  const STRING& cstrName_,
					  Interface::IScalar* pOperand_,
					  Interface::IScalar* pOption_)
				: Super(eType_, cDataType_, cstrName_, pOperand_, pOption_)
			{}
			// destructor
			virtual ~Arbitrary() {}

		/////////////////////////
		// Interface::IScalar::
			virtual int generate(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);
			virtual bool isArbitraryElement();

		protected:
		private:
		};
	} // namespace ElementReference

} // namespace ArrayImpl

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_ARRAYIMPL_H

//
//	Copyright (c) 2010, 2011, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
