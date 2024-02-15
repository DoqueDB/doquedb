// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/Base.h --
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_BASE_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_BASE_H

#include "boost/bind.hpp"

#include "Plan/Interface/IPredicate.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace Impl
{
	/////////////////////////////////////////////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Predicate::Impl::Base -- base class of Predicate interface implement
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	//	This class will not created directly
	template <class Handle_>
	class Base
		: public Handle_
	{
	public:
		typedef Handle_ Super;
		typedef Base<Handle_> This;
		typedef typename Super::Operand Operand;

		// destructor
		virtual ~Base() {}

	///////////////////////////////
	// Interface::IPredicate::
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_)
		{
			// extract scalar in the relation used in this predicate
			foreachOperand(boost::bind(&Operand::require,
									   _1,
									   boost::ref(cEnvironment_),
									   pCandidate_));
		}
		virtual void retrieve(Opt::Environment& cEnvironment_)
		{
			// extract scalar in the relation used in this predicate
			foreachOperand(boost::bind(&Operand::retrieve,
									   _1,
									   boost::ref(cEnvironment_)));
		}
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Interface::ICandidate* pCandidate_)
		{
			// extract scalar in the relation used in this predicate
			foreachOperand(boost::bind(&Operand::retrieve,
									   _1,
									   boost::ref(cEnvironment_),
									   pCandidate_));
		}
		virtual void use(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
		{
			// extract scalar in the relation used in this predicate
			foreachOperand(boost::bind(&Operand::use,
									   _1,
									   boost::ref(cEnvironment_),
									   pCandidate_));
		}
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Interface::ICandidate* pCandidate_,
						   Scalar::DelayArgument& cArgument_)
		{
			return isAll(boost::bind(&Operand::delay,
									 _1,
									 boost::ref(cEnvironment_),
									 pCandidate_,
									 boost::ref(cArgument_)));
		}
	///////////////////////////
	// Interface::IScalar::
		virtual void getUsedTable(Utility::RelationSet& cResult_)
		{
			foreachOperand(boost::bind(&Operand::getUsedTable,
									   _1,
									   boost::ref(cResult_)));
		}
		virtual void getUsedField(Utility::FieldSet& cResult_)
		{
			foreachOperand(boost::bind(&Operand::getUsedField,
									   _1,
									   boost::ref(cResult_)));
		}
		virtual void getUnknownKey(Opt::Environment& cEnvironment_,
								   Predicate::CheckUnknownArgument& cResult_)
		{
			foreachOperand(boost::bind(&Operand::getUnknownKey,
									   _1,
									   boost::ref(cEnvironment_),
									   boost::ref(cResult_)));
		}

		virtual bool hasParameter()
		{
			return isAny(boost::bind(&Operand::hasParameter,
									 _1));
		}

	protected:
		// constructor
		Base(typename Handle_::Argument cArgument_)
			: Super(cArgument_)
		{}
		template <class A_>
		Base(A_ a_, typename Handle_::Argument cArgument_)
			: Super(a_, cArgument_)
		{}
		template <class A1_, class A2_>
		Base(A1_ a1_, A2_ a2_, typename Handle_::Argument cArgument_)
			: Super(a1_, a2_, cArgument_)
		{}

	private:
	};
	/////////////////////////////////////////////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Predicate::Impl::BaseWithOption -- base class of Predicate interface implement
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	//	This class will not created directly
	template <class Handle1_, class Handle2_>
	class BaseWithOption
		: public Base<Handle1_>
	{
	public:
		typedef Base<Handle1_> Super;
		typedef BaseWithOption<Handle1_, Handle2_> This;
		typedef typename Super::Operand Operand;
		typedef typename Handle2_::Operand Option;

		// destructor
		virtual ~BaseWithOption() {}

	///////////////////////////////
	// Interface::IPredicate::
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_)
		{
			// extract scalar in the relation used in this predicate
			Super::require(cEnvironment_,
						   pCandidate_);
			foreachOption(boost::bind(&Option::require,
									  _1,
									  boost::ref(cEnvironment_),
									  pCandidate_));
		}
		virtual void retrieve(Opt::Environment& cEnvironment_)
		{
			// extract scalar in the relation used in this predicate
			Super::retrieve(cEnvironment_);
			foreachOption(boost::bind(&Option::retrieve,
									  _1,
									  boost::ref(cEnvironment_)));
		}
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Interface::ICandidate* pCandidate_)
		{
			// extract scalar in the relation used in this predicate
			Super::retrieve(cEnvironment_,
							pCandidate_);
			foreachOption(boost::bind(&Option::retrieve,
									  _1,
									  boost::ref(cEnvironment_),
									  pCandidate_));
		}
		virtual void use(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
		{
			// extract scalar in the relation used in this predicate
			Super::use(cEnvironment_,
					   pCandidate_);
			foreachOption(boost::bind(&Option::use,
									  _1,
									  boost::ref(cEnvironment_),
									  pCandidate_));
		}
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Interface::ICandidate* pCandidate_,
						   Scalar::DelayArgument& cArgument_)
		{
			return Super::delay(cEnvironment_,
								pCandidate_,
								cArgument_)
				&& isAllOption(boost::bind(&Option::delay,
										   _1,
										   boost::ref(cEnvironment_),
										   pCandidate_,
										   boost::ref(cArgument_)));
		}
	///////////////////////////
	// Interface::IScalar::
		virtual void getUsedTable(Utility::RelationSet& cResult_)
		{
			Super::getUsedTable(cResult_);
			foreachOption(boost::bind(&Option::getUsedTable,
									  _1,
									  boost::ref(cResult_)));
		}
		virtual void getUsedField(Utility::FieldSet& cResult_)
		{
			Super::getUsedField(cResult_);
			foreachOption(boost::bind(&Option::getUsedField,
									  _1,
									  boost::ref(cResult_)));
		}
		virtual void getUnknownKey(Opt::Environment& cEnvironment_,
								   Predicate::CheckUnknownArgument& cResult_)
		{
			Super::getUnknownKey(cEnvironment_,
								 cResult_);
			foreachOption(boost::bind(&Option::getUnknownKey,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cResult_)));
		}

		virtual bool hasParameter()
		{
			return Super::hasParameter()
				|| isAnyOption(boost::bind(&Option::hasParameter,
										   _1));
		}

	//////////////////////////////////////////
	// Tree::Node::Super::
		virtual ModSize getOptionSize() const
		{return m_cHandle2.getOperandSize();}
		virtual const Tree::Node::Super* getOptionAt(ModInt32 iPosition_) const
		{return m_cHandle2.getOperandAt(iPosition_);}

		Option* getOptioni(int iPos_)
		{return m_cHandle2.getOperandi(iPos_);}

		typename Handle2_::Argument getOptionArgument()
		{return m_cHandle2.getArgument();}

	protected:
		// constructor
		BaseWithOption(typename Handle1_::Argument cArgument1_,
					   typename Handle2_::Argument cArgument2_)
			: Super(cArgument1_),
			  m_cHandle2(cArgument2_)
		{}
		template <class A_>
		BaseWithOption(A_ a_,
					   typename Handle1_::Argument cArgument1_,
					   typename Handle2_::Argument cArgument2_)
			: Super(a_, cArgument1_),
			  m_cHandle2(cArgument2_)
		{}
		template <class A1_, class A2_>
		BaseWithOption(A1_ a1_, A2_ a2_,
					   typename Handle1_::Argument cArgument1_,
					   typename Handle2_::Argument cArgument2_)
			: Super(a1_, a2_, cArgument1_),
			  m_cHandle2(cArgument2_)
		{}

		// scan over options
		template <class Function_>
		Function_ foreachOption(Function_ function_)
		{
			return m_cHandle2.foreachOperand(function_);
		}
		template <class Function_>
		bool isAllOption(Function_ function_)
		{
			return m_cHandle2.isAll(function_);
		}
		template <class Function_>
		bool isAnyOption(Function_ function_)
		{
			return m_cHandle2.isAny(function_);
		}
	private:
		Handle2_ m_cHandle2;
	};

} // namespace Impl

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_BASE_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
