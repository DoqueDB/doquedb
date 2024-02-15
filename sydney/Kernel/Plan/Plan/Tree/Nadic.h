// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tree/Nadic.h --
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

#ifndef __SYDNEY_PLAN_TREE_NADIC_H
#define __SYDNEY_PLAN_TREE_NADIC_H

#include "Plan/Tree/Module.h"
#include "Plan/Tree/Declaration.h"
#include "Plan/Tree/Node.h"
#include "Plan/Utility/Algorithm.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_TREE_BEGIN

////////////////////////////////////
//	TEMPLATE CLASS
//	Plan::Tree::Nadic -- 
//
//	TEMPLATE ARGUMENTS
//	class Base_
//		Base class
//	class Operand_
//		Operand class
//
//	NOTES
template <class Base_, class Operand_>
class Nadic
	: public Base_
{
public:
	typedef Base_ Super;
	typedef Operand_ Operand;
	typedef Nadic<Base_, Operand_> This;
	typedef VECTOR<Operand_*> Vector;
	typedef const Vector& Argument;
	typedef typename Vector::ITERATOR Iterator;
	typedef typename Vector::CONSTITERATOR ConstIterator;

	template <class V_> class MapResult : public VECTOR<V_> {};
	typedef MapResult<int> MapIntResult;

	// FUNCTION public
	//	Plan::Tree::Nadic::Nadic -- constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Vector& vecOperand_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	Nadic(Argument vecOperand_)
		: Base_(), m_vecOperand(vecOperand_)
	{}

	// FUNCTION public
	//	Plan::Tree::Nadic::Nadic -- constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	ConstIterator first_
	//	ConstIterator last_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	Nadic(ConstIterator first_, ConstIterator last_)
		: Base_(), m_vecOperand()
	{
		m_vecOperand.assign(first_, last_);
	}

	// FUNCTION public
	//	Plan::Tree::Nadic::Nadic -- copy constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Nadic& cOther_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	Nadic(const Nadic& cOther_)
		: Base_(cOther_), m_vecOperand(cOther_.m_vecOperand)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::Nadic -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A_ a_
	//	const Vector& vecOperand_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A_>
	Nadic(A_ a_, Argument vecOperand_)
		: Base_(a_), m_vecOperand(vecOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::Nadic -- generic constructor
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
	//	const Vector& vecOperand_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_>
	Nadic(A1_ a1_, A2_ a2_, Argument vecOperand_)
		: Base_(a1_, a2_), m_vecOperand(vecOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::Nadic -- generic constructor
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
	//	const Vector& vecOperand_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_, class A3_>
	Nadic(A1_ a1_, A2_ a2_, A3_ a3_, Argument vecOperand_)
		: Base_(a1_, a2_, a3_), m_vecOperand(vecOperand_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::Nadic -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A_ a_
	//	ConstIterator first_
	//	ConstIterator last_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A_>
	Nadic(A_ a_, ConstIterator first_, ConstIterator last_)
		: Base_(a_), m_vecOperand(first_, last_)
	{
		m_vecOperand.assign(first_, last_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::Nadic -- generic constructor
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
	//	ConstIterator first_
	//	ConstIterator last_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_>
	Nadic(A1_ a1_, A2_ a2_, ConstIterator first_, ConstIterator last_)
		: Base_(a1_, a2_), m_vecOperand(first_, last_)
	{
		m_vecOperand.assign(first_, last_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::Nadic -- generic constructor
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
	//	ConstIterator first_
	//	ConstIterator last_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_, class A3_>
	Nadic(A1_ a1_, A2_ a2_, A3_ a3_, ConstIterator first_, ConstIterator last_)
		: Base_(a1_, a2_, a3_), m_vecOperand(first_, last_)
	{
		m_vecOperand.assign(first_, last_);
	}

	// FUNCTION public
	//	Plan::Tree::Nadic::~Nadic -- destructor
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
	virtual ~Nadic() {}

	// accessor
	Iterator begin() {return m_vecOperand.begin();}
	Iterator end() {return m_vecOperand.end();}
	ConstIterator begin() const {return m_vecOperand.begin();}
	ConstIterator end() const {return m_vecOperand.end();}

	int getSize() const {return m_vecOperand.GETSIZE();}
	bool isEmpty() const {return m_vecOperand.ISEMPTY();}
	Operand* getOperandi(int iPos_) const
	{return iPos_ >= 0 && iPos_ < getSize() ? m_vecOperand[iPos_] : 0;}
	void setOperandi(int iPos_, Operand* pOperand_)
	{if (iPos_ >= 0 && iPos_ < getSize()) m_vecOperand[iPos_] = pOperand_;}

	Argument getArgument() {return m_vecOperand;}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::foreachOperand -- scan over operands
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
		return FOREACH(m_vecOperand, function_);
	}
	template <class Function_>
	Function_ foreachOperand_i(Function_ function_)
	{
		return FOREACH_i(m_vecOperand, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::isAll -- check isAll
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
		return Opt::IsAll(m_vecOperand, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::isAny -- check isAny
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
		return Opt::IsAny(m_vecOperand, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::mapOperand -- replace operands with function
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
		Opt::MapContainer(m_vecOperand, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::mapOperand -- scan over operands
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
		Opt::MapContainer(*this, cResult_, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::joinOperand -- scan over operands
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
		return Opt::Join(*this, function_, delimiter_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::getMinOperand -- scan over operands
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
		return Opt::GetMin(begin(), end(), init_, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::getMaxOperand -- scan over operands
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
		return Opt::GetMax(begin(), end(), init_, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::Nadic::findOperand -- scan over operands
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
	//	Iterator
	//
	// EXCEPTIONS
	template <class Function_>
	Iterator findOperand(Function_ function_)
	{
		return Opt::Find(begin(), end(), function_);
	}

//////////////////////////////////////////
// Node::Interface::
//	Type getType() const;
//	virtual ModUnicodeString getValue() const;
//
//	virtual ModSize getOptionSize() const;
//	virtual const LogicalFile::TreeNodeInterface* getOptionAt(ModInt32 iPosition_) const;
//
	virtual ModSize getOperandSize() const {return getSize();}
	virtual const LogicalFile::TreeNodeInterface* getOperandAt(ModInt32 iPosition_) const
	{return iPosition_ >= 0 && iPosition_ < static_cast<ModInt32>(getSize())
			? static_cast<const LogicalFile::TreeNodeInterface*>(m_vecOperand[iPosition_]) : 0;}

protected:
	void addOperand(Operand_* pOperand_)
	{
		m_vecOperand.PUSHBACK(pOperand_);
	}

private:
	Vector m_vecOperand;
};

////////////////////////////////////
//	TEMPLATE CLASS
//	Plan::Tree::NadicOption -- 
//
//	TEMPLATE ARGUMENTS
//	class Base_
//		Base class
//	class Option_
//		Option class
//
//	NOTES
template <class Base_, class Option_>
class NadicOption
	: public Base_
{
public:
	typedef Base_ Super;
	typedef Option_ Option;
	typedef NadicOption<Base_, Option_> This;
	typedef VECTOR<Option_*> OptionVector;
	typedef const OptionVector& OptionArgument;
	typedef typename OptionVector::ITERATOR OptionIterator;
	typedef typename OptionVector::CONSTITERATOR ConstOptionIterator;

	template <class V_> class OptionMapResult : public VECTOR<V_> {};
	typedef OptionMapResult<int> OptionMapIntResult;

	// FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	const OptionVector& vecOption_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	NadicOption(OptionArgument vecOption_)
		: Base_(), m_vecOption(vecOption_)
	{}

	// FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	ConstOptionIterator first_
	//	ConstOptionIterator last_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	NadicOption(ConstOptionIterator first_, ConstOptionIterator last_)
		: Base_(), m_vecOption()
	{
		m_vecOption.assign(first_, last_);
	}

	// FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- copy constructor
	//
	// NOTES
	//
	// ARGUMENTS
	//	const NadicOption& cOther_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	NadicOption(const NadicOption& cOther_)
		: Base_(cOther_), m_vecOption(cOther_.m_vecOption)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A_ a_
	//	const OptionVector& vecOption_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A_>
	NadicOption(A_ a_, OptionArgument vecOption_)
		: Base_(a_), m_vecOption(vecOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- generic constructor
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
	//	const OptionVector& vecOption_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_>
	NadicOption(A1_ a1_, A2_ a2_, OptionArgument vecOption_)
		: Base_(a1_, a2_), m_vecOption(vecOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- generic constructor
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
	//	const OptionVector& vecOption_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_, class A3_>
	NadicOption(A1_ a1_, A2_ a2_, A3_ a3_, OptionArgument vecOption_)
		: Base_(a1_, a2_, a3_), m_vecOption(vecOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- generic constructor
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
	//	const OptionVector& vecOption_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_, class A3_, class A4_>
	NadicOption(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, OptionArgument vecOption_)
		: Base_(a1_, a2_, a3_, a4_), m_vecOption(vecOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- generic constructor
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
	//	const OptionVector& vecOption_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_, class A3_, class A4_, class A5_>
	NadicOption(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, A5_ a5_, OptionArgument vecOption_)
		: Base_(a1_, a2_, a3_, a4_, a5_), m_vecOption(vecOption_)
	{}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- generic constructor
	//
	// TEMPLATE ARGUMENTS
	//	class A_
	//
	// NOTES
	//
	// ARGUMENTS
	//	A_ a_
	//	ConstOptionIterator first_
	//	ConstOptionIterator last_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A_>
	NadicOption(A_ a_, ConstOptionIterator first_, ConstOptionIterator last_)
		: Base_(a_), m_vecOption(first_, last_)
	{
		m_vecOption.assign(first_, last_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- generic constructor
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
	//	ConstOptionIterator first_
	//	ConstOptionIterator last_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_>
	NadicOption(A1_ a1_, A2_ a2_, ConstOptionIterator first_, ConstOptionIterator last_)
		: Base_(a1_, a2_), m_vecOption(first_, last_)
	{
		m_vecOption.assign(first_, last_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- generic constructor
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
	//	ConstOptionIterator first_
	//	ConstOptionIterator last_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_, class A3_>
	NadicOption(A1_ a1_, A2_ a2_, A3_ a3_, ConstOptionIterator first_, ConstOptionIterator last_)
		: Base_(a1_, a2_, a3_), m_vecOption(first_, last_)
	{
		m_vecOption.assign(first_, last_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- generic constructor
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
	//	ConstOptionIterator first_
	//	ConstOptionIterator last_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_, class A3_, class A4_>
	NadicOption(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, ConstOptionIterator first_, ConstOptionIterator last_)
		: Base_(a1_, a2_, a3_, a4_), m_vecOption(first_, last_)
	{
		m_vecOption.assign(first_, last_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::NadicOption -- generic constructor
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
	//	ConstOptionIterator first_
	//	ConstOptionIterator last_
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class A1_, class A2_, class A3_, class A4_, class A5_>
	NadicOption(A1_ a1_, A2_ a2_, A3_ a3_, A4_ a4_, A5_ a5_, ConstOptionIterator first_, ConstOptionIterator last_)
		: Base_(a1_, a2_, a3_, a4_, a5_), m_vecOption(first_, last_)
	{
		m_vecOption.assign(first_, last_);
	}

	// FUNCTION public
	//	Plan::Tree::NadicOption::~NadicOption -- destructor
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
	virtual ~NadicOption() {}

	// accessor
	Option* getOptioni(int iPos_)
	{return iPos_ >= 0 && iPos_ < static_cast<int>(m_vecOption.GETSIZE()) ? m_vecOption[iPos_] : 0;}

	OptionArgument getOptionArgument() {return m_vecOption;}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::foreachOption -- scan over options
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
		return FOREACH(m_vecOption, function_);
	}
	template <class Function_>
	Function_ foreachOption_i(Function_ function_)
	{
		return FOREACH_i(m_vecOption, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::isAllOption -- check isAllOption
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
		return Opt::IsAll(m_vecOption, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::isAnyOption -- check isAnyOption
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
		return Opt::IsAny(m_vecOption, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::mapOption -- scan over options
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
		Opt::MapContainer(m_vecOption, cResult_, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Tree::NadicOption::joinOption -- scan over options
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
		return Opt::Join(m_vecOption, function_, delimiter_);
	}

//////////////////////////////////////////
// Node::Interface::
//	Type getType() const;
//	virtual ModUnicodeString getValue() const;
//
	virtual ModSize getOptionSize() const {return m_vecOption.GETSIZE();}
	virtual const LogicalFile::TreeNodeInterface* getOptionAt(ModInt32 iPosition_) const
	{return iPosition_ >= 0 && iPosition_ < static_cast<ModInt32>(m_vecOption.GETSIZE())
			? static_cast<const LogicalFile::TreeNodeInterface*>(m_vecOption[iPosition_]) : 0;}
//
//	virtual ModSize getOperandSize() const;
//	virtual const LogicalFile::TreeNodeInterface* getOperandAt(ModInt32 iPosition_) const;

protected:
	void addOption(Option_* pOption_)
	{
		m_vecOption.PUSHBACK(pOption_);
	}

private:
	OptionVector m_vecOption;
};

_SYDNEY_PLAN_TREE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_TREE_NADIC_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
