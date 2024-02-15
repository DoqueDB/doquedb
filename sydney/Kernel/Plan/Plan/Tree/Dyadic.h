// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tree/Dyadic.h --
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

#ifndef __SYDNEY_PLAN_TREE_DYADIC_H
#define __SYDNEY_PLAN_TREE_DYADIC_H

#include "Plan/Tree/Module.h"
#include "Plan/Tree/Declaration.h"
#include "Plan/Tree/Node.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_TREE_BEGIN

////////////////////////////////////
//	TEMPLATE CLASS
//	Plan::Tree::Dyadic -- 
//
//	TEMPLATE ARGUMENTS
//	class Base_
//		Base class
//	class Operand_
//		Operand class
//
//	NOTES
//		Base_ is expected to be a subclass of Tree::Node
template <class Base_, class Operand_>
class Dyadic
	: public Base_
{
public:
	typedef Base_ Super;
	typedef Operand_ Operand;
	typedef Dyadic<Base_, Operand_> This;
	typedef PAIR<Operand_*, Operand_*> Pair;
	typedef const Pair& Argument;

	template <class V_> struct MapResult : public PAIR<V_, V_> {};
	typedef MapResult<int> MapIntResult;

	// FUNCTION public
	//	Plan::Tree::Dyadic::Dyadic -- constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Pair& cOperand_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	Dyadic(Argument cOperand_)
		: Base_(), m_cOperand(cOperand_)
	{}

	// FUNCTION public
	//	Plan::Tree::Dyadic::Dyadic -- constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	Operand* pOperand0_
	//	Operand* pOperand1_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	Dyadic(Operand* pOperand0_, Operand* pOperand1_)
		: Base_(), m_cOperand(pOperand0_, pOperand1_)
	{}

	// FUNCTION public
	//	Plan::Tree::Dyadic::Dyadic -- copy constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Dyadic& cOther_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	Dyadic(const Dyadic& cOther_)
		: Base_(), m_cOperand(cOther_.m_cOperand)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::Dyadic -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A_ a_
	//	const Pair& cOperand_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A_>
	Dyadic(A_ a_, Argument cOperand_)
		: Base_(a_), m_cOperand(cOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::Dyadic -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	const Pair& cOperand_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_>
	Dyadic(A1_ a1_, A2_ a2_, Argument cOperand_)
		: Base_(a1_, a2_), m_cOperand(cOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::Dyadic -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	const Pair& cOperand_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_>
	Dyadic(A1_ a1_, A2_ a2_, A3_ a3_, Argument cOperand_)
		: Base_(a1_, a2_, a3_), m_cOperand(cOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::Dyadic -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	class A4_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	A4_ a4_
	//	const Pair& cOperand_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_>
	Dyadic(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, Argument cOperand_)
		: Base_(a1_, a2_, a3_, a4_), m_cOperand(cOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::Dyadic -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	class A4_
	//	class A5_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	A4_ a4_
	//	A5_ a5_
	//	const Pair& cOperand_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_, class A5_>
	Dyadic(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, A5_ a5_, Argument cOperand_)
		: Base_(a1_, a2_, a3_, a4_, a5_), m_cOperand(cOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::Dyadic -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A_ a_
	//	Operand* pOperand0_
	//	Operand* pOperand1_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A_>
	Dyadic(A_ a_, Operand* pOperand0_, Operand* pOperand1_)
		: Base_(a_), m_cOperand(pOperand0_, pOperand1_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::Dyadic -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	Operand* pOperand0_
	//	Operand* pOperand1_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_>
	Dyadic(A1_ a1_, A2_ a2_, Operand* pOperand0_, Operand* pOperand1_)
		: Base_(a1_, a2_), m_cOperand(pOperand0_, pOperand1_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::Dyadic -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	Operand* pOperand0_
	//	Operand* pOperand1_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_>
	Dyadic(A1_ a1_, A2_ a2_, A3_ a3_, Operand* pOperand0_, Operand* pOperand1_)
		: Base_(a1_, a2_, a3_), m_cOperand(pOperand0_, pOperand1_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::Dyadic -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	class A4_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	A4_ a4_
	//	Operand* pOperand0_
	//	Operand* pOperand1_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_>
	Dyadic(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, Operand* pOperand0_, Operand* pOperand1_)
		: Base_(a1_, a2_, a3_, a4_), m_cOperand(pOperand0_, pOperand1_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::Dyadic -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	class A4_
	//	class A5_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	A4_ a4_
	//	A5_ a5_
	//	Operand* pOperand0_
	//	Operand* pOperand1_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_, class A5_>
	Dyadic(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, A5_ a5_, Operand* pOperand0_, Operand* pOperand1_)
		: Base_(a1_, a2_, a3_, a4_, a5_), m_cOperand(pOperand0_, pOperand1_)
	{}

	// FUNCTION public
	//	Plan::Tree::Dyadic::~Dyadic -- destructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	Nothing
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	virtual ~Dyadic() {}

	// accessor
	Operand* getOperand0() {return m_cOperand.first;}
	Operand* getOperand1() {return m_cOperand.second;}
	const Operand* getOperand0() const {return m_cOperand.first;}
	const Operand* getOperand1() const {return m_cOperand.second;}

	void setOperand0(Operand* pOperand_) {m_cOperand.first = pOperand_;}
	void setOperand1(Operand* pOperand_) {m_cOperand.second = pOperand_;}

	int getSize() const {return 2;}
	bool isEmpty() const {return false;}
	Operand* getOperandi(int iPos_) const
	{return iPos_ == 0 ? m_cOperand.first
			: (iPos_ == 1 ? m_cOperand.second : 0);}
	void setOperandi(int iPos_, Operand* pOperand_)
	{
		if (iPos_ == 0) m_cOperand.first = pOperand_;
		else if (iPos_ == 1) m_cOperand.second = pOperand_;
	}

	Argument getArgument() {return m_cOperand;}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::foreachOperand -- scan over operands
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function
	//	
	// RETURN
	//	Function_
	//
	// EXCEPTIONS
	template <class Function_>
	Function_ foreachOperand(Function_ function_)
	{
		function_(m_cOperand.first);
		function_(m_cOperand.second);
		return function_;
	}

	template <class Function_>
	Function_ foreachOperand_i(Function_ function_)
	{
		function_(m_cOperand.first, 0);
		function_(m_cOperand.second, 1);
		return function_;
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::isAll -- check isAll
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS
	template <class Function_>
	bool isAll(Function_ function_)
	{
		return function_(m_cOperand.first)
			&& function_(m_cOperand.second);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::isAny -- check isAny
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS
	template <class Function_>
	bool isAny(Function_ function_)
	{
		return function_(m_cOperand.first)
			|| function_(m_cOperand.second);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::mapOperand -- scan over operands
	//
	// TEMPLATE ARGUMENTS
	//	class V_
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	MapResult<V_>& cResult_
	//	Function_ function_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class V_, class Function_>
	void mapOperand(MapResult<V_>& cResult_, Function_ function_)
	{
		cResult_.first = function_(m_cOperand.first);
		cResult_.second = function_(m_cOperand.second);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::mapOperand -- replace operands with function
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class Function_>
	void mapOperand(Function_ function_)
	{
		m_cOperand.first = function_(m_cOperand.first);
		m_cOperand.second = function_(m_cOperand.second);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::joinOperand -- scan over operands
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//	class Delimiter_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//	Delimiter_ delimiter_
	//	
	// RETURN
	//	Function_
	//
	// EXCEPTIONS
	template <class Function_, class Delimiter_>
	Function_ joinOperand(Function_ function_, Delimiter_ delimiter_)
	{
		function_(m_cOperand.first);
		delimiter_();
		function_(m_cOperand.second);
		return function_;
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::getMinOperand -- scan over operands
	//
	// TEMPLATE ARGUMENTS
	//	class Value_
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Value_ init_
	//	Function_ function_
	//	
	// RETURN
	//	Value_
	//
	// EXCEPTIONS
	template <class Value_, class Function_>
	Value_ getMinOperand(Value_ init_, Function_ function_)
	{
		Value_ cResult(init_);
		Value_ cValue0(function_(m_cOperand.first));
		if (cValue0 < cResult) cResult = cValue0;
		Value_ cValue1(function_(m_cOperand.second));
		if (cValue1 < cResult) cResult = cValue1;

		return cResult;
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Dyadic::getMaxOperand -- scan over operands
	//
	// TEMPLATE ARGUMENTS
	//	class Value_
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Value_ init_
	//	Function_ function_
	//	
	// RETURN
	//	Value_
	//
	// EXCEPTIONS
	template <class Value_, class Function_>
	Value_ getMaxOperand(Value_ init_, Function_ function_)
	{
		Value_ cResult(init_);
		Value_ cValue0(function_(m_cOperand.first));
		if (cValue0 > cResult) cResult = cValue0;
		Value_ cValue1(function_(m_cOperand.second));
		if (cValue1 > cResult) cResult = cValue1;

		return cResult;
	}

//////////////////////////////////////////
// Node::Interface::
//	Type getType() const;
//	virtual ModUnicodeString getValue() const;
//
//	virtual ModSize getOptionSize() const;
//	virtual const LogicalFile::TreeNodeInterface* getOptionAt(ModInt32 iPosition_) const;
//
	virtual ModSize getOperandSize() const {return 2;}
	virtual const LogicalFile::TreeNodeInterface* getOperandAt(ModInt32 iPosition_) const
	{return iPosition_ == 0 ? m_cOperand.first : (iPosition_ == 1 ? m_cOperand.second : 0);}

protected:
private:
	Pair m_cOperand;
};

////////////////////////////////////
//	TEMPLATE CLASS
//	Plan::Tree::DyadicOption -- 
//
//	TEMPLATE ARGUMENTS
//	class Base_
//		Base class
//	class Option_
//		Option class
//
//	NOTES
//		Base_ is expected to be a subclass of Tree::Node
template <class Base_, class Option_>
class DyadicOption
	: public Base_
{
public:
	typedef Base_ Super;
	typedef Option_ Option;
	typedef DyadicOption<Base_, Option_> This;
	typedef PAIR<Option_*, Option_*> Pair;
	typedef const Pair& OptionArgument;
	template <class V_> struct OptionMapResult : public PAIR<V_, V_> {};
	typedef OptionMapResult<int> OptionMapIntResult;

	// FUNCTION public
	//	Plan::Tree::DyadicOption::DyadicOption -- constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Pair& cOption_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	DyadicOption(OptionArgument cOption_)
		: Base_(), m_cOption(cOption_)
	{}

	// FUNCTION public
	//	Plan::Tree::DyadicOption::DyadicOption -- constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	Option* pOption0_
	//	Option* pOption1_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	DyadicOption(Option* pOption0_, Option* pOption1_)
		: Base_(), m_cOption(pOption0_, pOption1_)
	{}

	// FUNCTION public
	//	Plan::Tree::DyadicOption::DyadicOption -- copy constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	const DyadicOption& cOther_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	DyadicOption(const DyadicOption& cOther_)
		: Base_(), m_cOption(cOther_.m_cOption)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::DyadicOption::DyadicOption -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A_ a_
	//	const Pair& cOption_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A_>
	DyadicOption(A_ a_, OptionArgument cOption_)
		: Base_(a_), m_cOption(cOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::DyadicOption::DyadicOption -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	const Pair& cOption_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_>
	DyadicOption(A1_ a1_, A2_ a2_, OptionArgument cOption_)
		: Base_(a1_, a2_), m_cOption(cOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::DyadicOption -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	const Pair& cOption_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_>
	DyadicOption(A1_ a1_, A2_ a2_, A3_ a3_, OptionArgument cOption_)
		: Base_(a1_, a2_, a3_), m_cOption(cOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::DyadicOption -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	class A4_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	A4_ a4_
	//	const Pair& cOption_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_>
	DyadicOption(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, OptionArgument cOption_)
		: Base_(a1_, a2_, a3_, a4_), m_cOption(cOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::DyadicOption -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	class A4_
	//	class A5_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	A4_ a4_
	//	A5_ a5_
	//	const Pair& cOption_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_, class A5_>
	DyadicOption(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, A5_ a5_, OptionArgument cOption_)
		: Base_(a1_, a2_, a3_, a4_, a5_), m_cOption(cOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::DyadicOption::DyadicOption -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A_ a_
	//	Option* pOption0_
	//	Option* pOption1_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A_>
	DyadicOption(A_ a_, Option* pOption0_, Option* pOption1_)
		: Base_(a_), m_cOption(pOption0_, pOption1_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::DyadicOption::DyadicOption -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	Option* pOption0_
	//	Option* pOption1_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_>
	DyadicOption(A1_ a1_, A2_ a2_, Option* pOption0_, Option* pOption1_)
		: Base_(a1_, a2_), m_cOption(pOption0_, pOption1_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::DyadicOption -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	Option* pOption0_
	//	Option* pOption1_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_>
	DyadicOption(A1_ a1_, A2_ a2_, A3_ a3_, Option* pOption0_, Option* pOption1_)
		: Base_(a1_, a2_, a3_), m_cOption(pOption0_, pOption1_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::DyadicOption -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	class A4_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	A4_ a4_
	//	Option* pOption0_
	//	Option* pOption1_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_>
	DyadicOption(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, Option* pOption0_, Option* pOption1_)
		: Base_(a1_, a2_, a3_, a4_), m_cOption(pOption0_, pOption1_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::DyadicOption -- constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A1_
	//	class A2_
	//	class A3_
	//	class A4_
	//	class A5_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	A1_ a1_
	//	A2_ a2_
	//	A3_ a3_
	//	A4_ a4_
	//	A5_ a5_
	//	Option* pOption0_
	//	Option* pOption1_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_, class A5_>
	DyadicOption(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, A5_ a5_, Option* pOption0_, Option* pOption1_)
		: Base_(a1_, a2_, a3_, a4_, a5_), m_cOption(pOption0_, pOption1_)
	{}

	// FUNCTION public
	//	Plan::Tree::DyadicOption::~DyadicOption -- destructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	Nothing
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	virtual ~DyadicOption() {}

	// accessor
	Option* getOption0() {return m_cOption.first;}
	Option* getOption1() {return m_cOption.second;}
	const Option* getOption0() const {return m_cOption.first;}
	const Option* getOption1() const {return m_cOption.second;}

	void setOption0(Option* pOption_) {m_cOption.first = pOption_;}
	void setOption1(Option* pOption_) {m_cOption.second = pOption_;}

	Option* getOptioni(int iPos_)
	{return iPos_ == 0 ? m_cOption.first
			: (iPos_ == 1 ? m_cOption.second : 0);}

	OptionArgument getOptionArgument() {return m_cOption;}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::DyadicOption::foreachOption -- scan over options
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function
	//	
	// RETURN
	//	Function_
	//
	// EXCEPTIONS
	template <class Function_>
	Function_ foreachOption(Function_ function_)
	{
		function_(m_cOption.first);
		function_(m_cOption.second);
		return function_;
	}

	template <class Function_>
	Function_ foreachOption_i(Function_ function_)
	{
		function_(m_cOption.first, 0);
		function_(m_cOption.second, 1);
		return function_;
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::DyadicOption::isAllOption -- check isAllOption
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS
	template <class Function_>
	bool isAllOption(Function_ function_)
	{
		return function_(m_cOption.first)
			&& function_(m_cOption.second);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::DyadicOption::isAnyOption -- check isAnyOption
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS
	template <class Function_>
	bool isAnyOption(Function_ function_)
	{
		return function_(m_cOption.first)
			|| function_(m_cOption.second);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::DyadicOption::mapOption -- scan over options
	//
	// TEMPLATE ARGUMENTS
	//	class V_
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	OptionMapResult<V_>& cResult_
	//	Function_ function_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class V_, class Function_>
	void mapOption(OptionMapResult<V_>& cResult_, Function_ function_)
	{
		cResult_.first = function_(m_cOption.first);
		cResult_.second = function_(m_cOption.second);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::DyadicOption::joinOption -- scan over options
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//	class Delimiter_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//	Delimiter_ delimiter_
	//	
	// RETURN
	//	Function_
	//
	// EXCEPTIONS
	template <class Function_, class Delimiter_>
	Function_ joinOption(Function_ function_, Delimiter_ delimiter_)
	{
		function_(m_cOption.first);
		delimiter_();
		function_(m_cOption.second);
		return function_;
	}

//////////////////////////////////////////
// Node::Interface::
//	Type getType() const;
//	virtual ModUnicodeString getValue() const;
//
	virtual ModSize getOptionSize() const {return 2;}
	virtual const LogicalFile::TreeNodeInterface* getOptionAt(ModInt32 iPosition_) const
	{return iPosition_ == 0 ? m_cOption.first : (iPosition_ == 1 ? m_cOption.second : 0);}
//
//	virtual ModSize getOperandSize() const;
//	virtual const LogicalFile::TreeNodeInterface* getOperandAt(ModInt32 iPosition_) const;

protected:
private:
	Pair m_cOption;
};

_SYDNEY_PLAN_TREE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_TREE_DYADIC_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
