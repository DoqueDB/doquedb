// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/OperationImpl.h --
// 
// Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_OPERATIONIMPL_H
#define __SYDNEY_PLAN_SCALAR_OPERATIONIMPL_H

#include "Plan/Scalar/Operation.h"
#include "Plan/Scalar/Impl/FunctionImpl.h"

#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace OperationImpl
{
	////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Scalar::OperationImpl::Base -- implementation class for operation operations
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

	/////////////////////////
	// Interface::IScalar::
		virtual bool checkOperation(Opt::Environment& cEnvironment_,
									Interface::IScalar* pOperand_)
		{
			return true;
		}
		virtual PAIR<Interface::IScalar*, Interface::IScalar*>
					createOperation(Opt::Environment& cEnvironment_,
									Interface::IScalar* pOperand)
		{
			return MAKEPAIR((Interface::IScalar*)this, pOperand);
		}

	protected:
	/////////////////////////
	// Interface::IScalar::
		virtual int generateData(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_)
		{
			// operations don't provide data ahead
			return -1;
		}
	private:
	};

	////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Scalar::OperationImpl::BaseWithOption -- implementation class for operation operations
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class BaseWithOption
		: public FunctionImpl::BaseWithOption<Handle_>
	{
	public:
		typedef FunctionImpl::BaseWithOption<Handle_> Super;
		typedef BaseWithOption<Handle_> This;

		BaseWithOption(Tree::Node::Type eType_,
					   const STRING& cstrName_,
					   typename Handle_::Argument cArgument1_,
					   typename Handle_::OptionArgument cArgument2_)
			: Super(eType_, cstrName_, cArgument1_, cArgument2_)
		{}
		BaseWithOption(Tree::Node::Type eType_,
					   const DataType& cDataType_,
					   const STRING& cstrName_,
					   typename Handle_::Argument cArgument1_,
					   typename Handle_::OptionArgument cArgument2_)
			: Super(eType_, cDataType_, cstrName_, cArgument1_, cArgument2_)
		{}

		// destructor
		virtual ~BaseWithOption() {}

	protected:
	/////////////////////////
	// Interface::IScalar::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_)
		{
			if (cArgument_.m_bOperation) {
				// generate operation
				generateOperation(cEnvironment_,
								  cProgram_,
								  pIterator_,
								  cArgument_);
				return -1;
			}

			if (cArgument_.m_bSkipCheck) {
				// generate for skip check -> use only option
				return generateSkipCheck(cEnvironment_,
										 cProgram_,
										 pIterator_,
										 cArgument_);
			}

			// generate data array data for log data to express the operation
			return generateLogData(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cArgument_);
		}
	private:
		// generate skip check key
		virtual int generateSkipCheck(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_) = 0;
		// generate log data
		virtual int generateLogData(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_) = 0;
		// generate operation action
		virtual void generateOperation(Opt::Environment& cEnvironment_,
									   Execution::Interface::IProgram& cProgram_,
									   Execution::Interface::IIterator* pIterator_,
									   Candidate::AdoptArgument& cArgument_) = 0;
	};

	//////////////////////////////////////////////////
	// CLASS public
	//	Plan::Scalar::OperationImpl::Append
	//
	// NOTES
	class Append
		: public OperationImpl::BaseWithOption< Tree::MonadicOption<
									OperationImpl::Base< Tree::Monadic<Operation, Interface::IScalar> >,
									Interface::IScalar > >
	{
	public:
		typedef Append This;
		typedef OperationImpl::BaseWithOption< Tree::MonadicOption<
									OperationImpl::Base< Tree::Monadic<Operation, Interface::IScalar> >,
									Interface::IScalar > > Super;
		static const Tree::Node::Type _eType = Tree::Node::Append;

		Append(const STRING& cstrName_,
			   Super::Argument cArgument_,
			   Super::OptionArgument cOptionArgument_)
			: Super(_eType, cstrName_, cArgument_, cOptionArgument_)
		{}
		Append(const DataType& cDataType_,
			   const STRING& cstrName_,
			   Super::Argument cArgument_,
			   Super::OptionArgument cOptionArgument_)
			: Super(_eType, cDataType_, cstrName_, cArgument_, cOptionArgument_)
		{}
	protected:
	//	virtual int generateThis(Opt::Environment& cEnvironment_,
	//							 Execution::Interface::IProgram& cProgram_,
	//							 Execution::Interface::IIterator* pIterator_,
	//							 Candidate::AdoptArgument& cArgument_,
	//							 int iDataID_);
	private:
		virtual int generateSkipCheck(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);
		virtual int generateLogData(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_);
		virtual void generateOperation(Opt::Environment& cEnvironment_,
									   Execution::Interface::IProgram& cProgram_,
									   Execution::Interface::IIterator* pIterator_,
									   Candidate::AdoptArgument& cArgument_);
	};

	//////////////////////////////////////////////////
	// CLASS public
	//	Plan::Scalar::OperationImpl::Truncate
	//
	// NOTES
	class Truncate
		: public OperationImpl::BaseWithOption< Tree::MonadicOption<
									 OperationImpl::Base< Tree::Monadic<Operation, Interface::IScalar> >,
										   Interface::IScalar > >
	{
	public:
		typedef Truncate This;
		typedef OperationImpl::BaseWithOption< Tree::MonadicOption<
								   OperationImpl::Base< Tree::Monadic<Operation, Interface::IScalar> >,
									   Interface::IScalar > > Super;
		static const Tree::Node::Type _eType = Tree::Node::Truncate;

		Truncate(const STRING& cstrName_,
				 Super::Argument cArgument_,
				 Super::OptionArgument cOptionArgument_)
			: Super(_eType, cstrName_, cArgument_, cOptionArgument_)
		{}
		Truncate(const DataType& cDataType_,
				 const STRING& cstrName_,
				 Super::Argument cArgument_,
				 Super::OptionArgument cOptionArgument_)
			: Super(_eType, cDataType_, cstrName_, cArgument_, cOptionArgument_)
		{}

	protected:
	private:
		virtual int generateSkipCheck(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);
		virtual int generateLogData(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_);
		virtual void generateOperation(Opt::Environment& cEnvironment_,
									   Execution::Interface::IProgram& cProgram_,
									   Execution::Interface::IIterator* pIterator_,
									   Candidate::AdoptArgument& cArgument_);
	};

	//////////////////////////////////////////////////
	// CLASS public
	//	Plan::Scalar::OperationImpl::Replace
	//
	// NOTES
	class Replace
		: public OperationImpl::BaseWithOption< Tree::NadicOption<
									OperationImpl::Base< Tree::Monadic<Operation, Interface::IScalar> >,
									Interface::IScalar > >
	{
	public:
		typedef Replace This;
		typedef OperationImpl::BaseWithOption< Tree::NadicOption<
									OperationImpl::Base< Tree::Monadic<Operation, Interface::IScalar> >,
									Interface::IScalar > > Super;
		static const Tree::Node::Type _eType = Tree::Node::Replace;

		Replace(const STRING& cstrName_,
				Super::Argument cArgument_,
				Super::OptionArgument cOptionArgument_)
			: Super(_eType, cstrName_, cArgument_, cOptionArgument_)
		{}
		Replace(const DataType& cDataType_,
				const STRING& cstrName_,
				Super::Argument cArgument_,
				Super::OptionArgument cOptionArgument_)
			: Super(_eType, cDataType_, cstrName_, cArgument_, cOptionArgument_)
		{}
	protected:
	//	virtual int generateThis(Opt::Environment& cEnvironment_,
	//							 Execution::Interface::IProgram& cProgram_,
	//							 Execution::Interface::IIterator* pIterator_,
	//							 Candidate::AdoptArgument& cArgument_,
	//							 int iDataID_);
	private:
		virtual int generateSkipCheck(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);
		virtual int generateLogData(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_);
		virtual void generateOperation(Opt::Environment& cEnvironment_,
									   Execution::Interface::IProgram& cProgram_,
									   Execution::Interface::IIterator* pIterator_,
									   Candidate::AdoptArgument& cArgument_);
	};

} // namespace OperationImpl

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_OPERATIONIMPL_H

//
//	Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
