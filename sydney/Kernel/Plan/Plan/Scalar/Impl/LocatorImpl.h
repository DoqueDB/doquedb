// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/LocatorImpl.h --
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

#ifndef __SYDNEY_PLAN_SCALAR_LOCATORIMPL_H
#define __SYDNEY_PLAN_SCALAR_LOCATORIMPL_H

#include "Plan/Scalar/Locator.h"
#include "Plan/Scalar/Impl/FunctionImpl.h"

#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace LocatorImpl
{
	////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Scalar::LocatorImpl::Base -- implementation class for locator operations
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

	protected:
	private:
	};

	////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Scalar::LocatorImpl::BaseWithOption -- implementation class for locator operations
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

	/////////////////////////
	// Interface::IScalar::
	protected:
	private:
	};

	//////////////////////////////////////////////////
	// CLASS public
	//	Plan::Scalar::LocatorImpl::CharLength
	//
	// NOTES
	class CharLength
		: public LocatorImpl::Base< Tree::Monadic<Locator, Interface::IScalar> >
	{
	public:
		typedef CharLength This;
		typedef LocatorImpl::Base< Tree::Monadic<Locator, Interface::IScalar> > Super;
		static const Tree::Node::Type _eType = Tree::Node::CharLength;

		CharLength(const STRING& cstrName_,
				   Super::Argument cArgument_)
			: Super(_eType, cstrName_, cArgument_)
		{}
		CharLength(const DataType& cDataType_,
				   const STRING& cstrName_,
				   Super::Argument cArgument_)
			: Super(_eType, cDataType_, cstrName_, cArgument_)
		{}
	protected:
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	private:
	};

	//////////////////////////////////////////////////
	// CLASS public
	//	Plan::Scalar::LocatorImpl::StringConcatenate
	//
	// NOTES
	class StringConcatenate
		: public LocatorImpl::Base< Tree::Dyadic<Locator, Interface::IScalar> >
	{
	public:
		typedef StringConcatenate This;
		typedef LocatorImpl::Base< Tree::Dyadic<Locator, Interface::IScalar> > Super;
		static const Tree::Node::Type _eType = Tree::Node::StringConcatenate;

		StringConcatenate(const STRING& cstrName_,
						  Super::Argument cArgument_)
			: Super(Tree::Node::StringConcatenate, cstrName_, cArgument_)
		{}
		StringConcatenate(const DataType& cDataType_,
						  const STRING& cstrName_,
						  Super::Argument cArgument_)
			: Super(Tree::Node::StringConcatenate, cDataType_, cstrName_, cArgument_)
		{}

	///////////////////////////////
	// Interface::IScalar*
		virtual bool checkOperation(Opt::Environment& cEnvironment_,
									Interface::IScalar* pOperand_);
		virtual PAIR<Interface::IScalar*, Interface::IScalar*>
					createOperation(Opt::Environment& cEnvironment_,
									Interface::IScalar* pOperand_);

	protected:
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	private:
	};

	//////////////////////////////////////////////////
	// CLASS public
	//	Plan::Scalar::LocatorImpl::SubString
	//
	// NOTES
	class SubString
		: public LocatorImpl::BaseWithOption< Tree::NadicOption<
									 LocatorImpl::Base< Tree::Monadic<Locator, Interface::IScalar> >,
										   Interface::IScalar > >
	{
	public:
		typedef SubString This;
		typedef LocatorImpl::BaseWithOption< Tree::NadicOption<
									LocatorImpl::Base< Tree::Monadic<Locator, Interface::IScalar> >,
										  Interface::IScalar > > Super;
		static const Tree::Node::Type _eType = Tree::Node::SubString;

		SubString(const STRING& cstrName_,
				  Super::Argument cArgument_,
				  Super::OptionArgument cOptionArgument_)
			: Super(Tree::Node::SubString, cstrName_, cArgument_, cOptionArgument_)
		{}
		SubString(const DataType& cDataType_,
				  const STRING& cstrName_,
				  Super::Argument cArgument_,
				  Super::OptionArgument cOptionArgument_)
			: Super(Tree::Node::SubString, cDataType_, cstrName_, cArgument_, cOptionArgument_)
		{}
	protected:
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	private:
	};

	//////////////////////////////////////////////////
	// CLASS public
	//	Plan::Scalar::LocatorImpl::Overlay
	//
	// NOTES
	class Overlay
		: public LocatorImpl::BaseWithOption< Tree::NadicOption<
									 LocatorImpl::Base< Tree::Dyadic<Locator, Interface::IScalar> >,
										   Interface::IScalar > >
	{
	public:
		typedef Overlay This;
		typedef LocatorImpl::BaseWithOption< Tree::NadicOption<
									LocatorImpl::Base< Tree::Dyadic<Locator, Interface::IScalar> >,
										  Interface::IScalar > > Super;
		static const Tree::Node::Type _eType = Tree::Node::Overlay;

		Overlay(const STRING& cstrName_,
				Super::Argument cArgument_,
				Super::OptionArgument cOptionArgument_)
			: Super(Tree::Node::Overlay, cstrName_, cArgument_, cOptionArgument_)
		{}
		Overlay(const DataType& cDataType_,
				const STRING& cstrName_,
				Super::Argument cArgument_,
				Super::OptionArgument cOptionArgument_)
			: Super(Tree::Node::Overlay, cDataType_, cstrName_, cArgument_, cOptionArgument_)
		{}

	///////////////////////////////
	// Interface::IScalar*
		virtual bool checkOperation(Opt::Environment& cEnvironment_,
									Interface::IScalar* pOperand_);
		virtual PAIR<Interface::IScalar*, Interface::IScalar*>
					createOperation(Opt::Environment& cEnvironment_,
									Interface::IScalar* pOperand_);

	protected:
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	private:
	};

} // namespace LocatorImpl

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_LOCATORIMPL_H

//
//	Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
