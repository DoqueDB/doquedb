// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tree/Monadic.h --
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

#ifndef __SYDNEY_PLAN_TREE_MONADIC_H
#define __SYDNEY_PLAN_TREE_MONADIC_H

#include "Plan/Tree/Module.h"
#include "Plan/Tree/Declaration.h"
#include "Plan/Tree/Node.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_TREE_BEGIN

////////////////////////////////////
//	TEMPLATE CLASS
//	Plan::Tree::Monadic -- 
//
//	TEMPLATE ARGUMENTS
//	class Base_
//		Base class
//	class Operand_
//		Operand class
//
//	NOTES
template <class Base_, class Operand_>
class Monadic
	: public Base_
{
public:
	typedef Base_ Super;
	typedef Operand_ Operand;
	typedef Monadic<Base_, Operand_> This;
	typedef Operand* Argument;

	template <class V_>
	struct MapResult
	{
		V_ m_cValue;

		operator V_() {return m_cValue;}
	};
	typedef MapResult<int> MapIntResult;

	// FUNCTION public
	//	Plan::Tree::Monadic::Monadic -- constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	Operand* pOperand_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	Monadic(Argument pOperand_)
		: Base_(), m_pOperand(pOperand_)
	{}

	// FUNCTION public
	//	Plan::Tree::Monadic::Monadic -- copy constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Monadic& cOther_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	Monadic(const Monadic& cOther_)
		: Base_(), m_pOperand(cOther_.m_pOperand)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Monadic::Monadic -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A_ a_
	//	Operand* pOperand_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A_>
	Monadic(A_ a_, Argument pOperand_)
		: Base_(a_), m_pOperand(pOperand_)
	{}
	// TEMPLATE FUNCTION public
	//	Plan::Tree::Monadic::Monadic -- generic constructor
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
	//	Operand* pOperand_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_>
	Monadic(A1_ a1_, A2_ a2_, Argument pOperand_)
		: Base_(a1_, a2_), m_pOperand(pOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::Monadic -- constructor
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
	//	Operand* pOperand_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_>
	Monadic(A1_ a1_, A2_ a2_, A3_ a3_, Argument pOperand_)
		: Base_(a1_, a2_, a3_), m_pOperand(pOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::Monadic -- constructor
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
	//	Operand* pOperand_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_>
	Monadic(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, Argument pOperand_)
		: Base_(a1_, a2_, a3_, a4_), m_pOperand(pOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::Monadic -- constructor
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
	//	Operand* pOperand_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_, class A5_>
	Monadic(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, A5_ a5_, Argument pOperand_)
		: Base_(a1_, a2_, a3_, a4_, a5_), m_pOperand(pOperand_)
	{}

	// FUNCTION public
	//	Plan::Tree::Monadic::~Monadic -- destructor
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
	virtual ~Monadic() {}

	// accessor
	Operand* getOperand() {return m_pOperand;}
	const Operand* getOperand() const {return m_pOperand;}

	void setOperand(Operand* pOperand_) {m_pOperand = pOperand_;}

	int getSize() const {return 1;}
	bool isEmpty() const {return false;}
	Operand* getOperandi(int iPos_) const {return iPos_ == 0 ? m_pOperand : 0;}
	void setOperandi(int iPos_, Operand* pOperand_) {if (iPos_ == 0) m_pOperand = pOperand_;}

	Argument getArgument() {return m_pOperand;}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Monadic::foreachOperand -- scan over operands
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
		function_(m_pOperand);
		return function_;
	}

	template <class Function_>
	Function_ foreachOperand_i(Function_ function_)
	{
		function_(m_pOperand, 0);
		return function_;
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Monadic::isAll -- check isAll
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
		return function_(m_pOperand);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Monadic::isAny -- check isAny
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
		return function_(m_pOperand);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Monadic::mapOperand -- scan over operands
	//
	// TEMPLATE ARGUMENTS
	//	class V_
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	MapResult<V_&> cResult_
	//	Function_ function_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class V_, class Function_>
	void mapOperand(MapResult<V_>& cResult_, Function_ function_)
	{
		cResult_.m_cValue = function_(m_pOperand);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Monadic::mapOperand -- replace operands with function
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
		m_pOperand = function_(m_pOperand);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Monadic::joinOperand -- scan over operands
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
		function_(m_pOperand);
		return function_;
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Monadic::getMinOperand -- scan over operands
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
		Value_ cValue(function_(m_pOperand));
		if (cValue < cResult) cResult = cValue;

		return cResult;
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Monadic::getMaxOperand -- scan over operands
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
		Value_ cValue(function_(m_pOperand));
		if (cValue > cResult) cResult = cValue;

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
	virtual ModSize getOperandSize() const {return 1;}
	virtual const LogicalFile::TreeNodeInterface* getOperandAt(ModInt32 iPosition_) const
	{return iPosition_ == 0 ? m_pOperand : 0;}

protected:
private:
	Operand* m_pOperand;
};

////////////////////////////////////
//	TEMPLATE CLASS
//	Plan::Tree::MonadicOption -- 
//
//	TEMPLATE ARGUMENTS
//	class Base_
//		Base class
//	class Option_
//		Option class
//
//	NOTES
template <class Base_, class Option_>
class MonadicOption
	: public Base_
{
public:
	typedef Base_ Super;
	typedef Option_ Option;
	typedef MonadicOption<Base_, Option_> This;
	typedef Option* OptionArgument;

	template <class V_>
	struct OptionMapResult
	{
		V_ m_cValue;
		operator V_() {return m_cValue;}
	};
	typedef OptionMapResult<int> OptionMapIntResult;

	// FUNCTION public
	//	Plan::Tree::MonadicOption::MonadicOption -- constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	Option* pOption_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	MonadicOption(OptionArgument pOption_)
		: Base_(), m_pOption(pOption_)
	{}

	// FUNCTION public
	//	Plan::Tree::MonadicOption::MonadicOption -- copy constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	const MonadicOption& cOther_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	MonadicOption(const MonadicOption& cOther_)
		: Base_(), m_pOption(cOther_.m_pOption)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::MonadicOption::MonadicOption -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A_ a_
	//	Option* pOption_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A_>
	MonadicOption(A_ a_, OptionArgument pOption_)
		: Base_(a_), m_pOption(pOption_)
	{}
	// TEMPLATE FUNCTION public
	//	Plan::Tree::MonadicOption::MonadicOption -- generic constructor
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
	//	Option* pOption_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_>
	MonadicOption(A1_ a1_, A2_ a2_, OptionArgument pOption_)
		: Base_(a1_, a2_), m_pOption(pOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::MonadicOption -- constructor
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
	//	Option* pOption_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_>
	MonadicOption(A1_ a1_, A2_ a2_, A3_ a3_, OptionArgument pOption_)
		: Base_(a1_, a2_, a3_), m_pOption(pOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::MonadicOption -- constructor
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
	//	Option* pOption_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_>
	MonadicOption(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, OptionArgument pOption_)
		: Base_(a1_, a2_, a3_, a4_), m_pOption(pOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Tree::MonadicOption -- constructor
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
	//	Option* pOption_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class A1_, class A2_, class A3_, class A4_, class A5_>
	MonadicOption(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, A5_ a5_, OptionArgument pOption_)
		: Base_(a1_, a2_, a3_, a4_, a5_), m_pOption(pOption_)
	{}

	// FUNCTION public
	//	Plan::Tree::MonadicOption::~MonadicOption -- destructor
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
	virtual ~MonadicOption() {}

	// accessor
	Option* getOption() {return m_pOption;}
	const Option* getOption() const {return m_pOption;}

	void setOption(Option* pOption_) {m_pOption = pOption_;}

	Option* getOptioni(int iPos_) {return iPos_ == 0 ? m_pOption : 0;}

	OptionArgument getOptionArgument() {return m_pOption;}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::MonadicOption::foreachOption -- scan over options
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
		function_(m_pOption);
		return function_;
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::MonadicOption::isAllOption -- check isAllOption
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
		return function_(m_pOption);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::MonadicOption::isAnyOption -- check isAnyOption
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
		return function_(m_pOption);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::MonadicOption::mapOption -- scan over options
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
		cResult_.m_cValue = function_(m_pOption);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::MonadicOption::joinOption -- scan over options
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
		function_(m_pOption);
		return function_;
	}

//////////////////////////////////////////
// Node::Interface::
//	Type getType() const;
//	virtual ModUnicodeString getValue() const;
//
	virtual ModSize getOptionSize() const {return 1;}
	virtual const LogicalFile::TreeNodeInterface* getOptionAt(ModInt32 iPosition_) const
	{return iPosition_ == 0 ? m_pOption : 0;}
//
//	virtual ModSize getOperandSize() const;
//	virtual const LogicalFile::TreeNodeInterface* getOperandAt(ModInt32 iPosition_) const;

protected:
private:
	Option* m_pOption;
};

_SYDNEY_PLAN_TREE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_TREE_MONADIC_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
