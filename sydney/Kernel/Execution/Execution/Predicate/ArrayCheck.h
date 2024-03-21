// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Predicate/ArrayCheck.h --
// 
// Copyright (c) 2011, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_PREDICATE_ARRAYCHECK_H
#define __SYDNEY_EXECUTION_PREDICATE_ARRAYCHECK_H

#include "Execution/Predicate/Base.h"
#include "Execution/Action/DataHolder.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

namespace ArrayCheck
{

////////////////////////////////////////////////////////////////////////////////
// TEMPLATE CLASS
//	Execution::Predicate::ArrayCheck::Base -- base class of arraycheck classes
//
// TEMPLATE ARGUMENTS
//	class Base_
//
// NOTES
//	This class is not constructed directly

template <class Base_>
class Base
	: public Base_
{
public:
	typedef Base_ Super;
	typedef Base<Base_> This;

	// destructor
	virtual ~Base() {}

/////////////////////////////
// Interface::IAction::
	virtual void initialize(Interface::IProgram& cProgram_)
	{
		if (m_cArray.isInitialized() == false) {
			Super::initialize(cProgram_);
			m_cArray.initialize(cProgram_);
		}
	}
	virtual void terminate(Interface::IProgram& cProgram_)
	{
		if (m_cArray.isInitialized() == true) {
			Super::terminate(cProgram_);
			m_cArray.terminate(cProgram_);
		}
	}

///////////////////////////////
// ModSerializer
	virtual void serialize(ModArchive& archiver_)
	{
		Super::serialize(archiver_);
		m_cArray.serialize(archiver_);
	}

protected:
	// constructor
	Base()
		: Super(),
		  m_cArray()
	{}
	template <class A_>
	Base(A_ a_)
		: Super(a_),
		  m_cArray()
	{}
	template <class A1_, class A2_>
	Base(A1_ a1_, A2_ a2_)
		: Super(a1_, a2_),
		  m_cArray()
	{}
	template <class A1_, class A2_, class A3_>
	Base(A1_ a1_, A2_ a2_, A3_ a3_)
		: Super(a1_, a2_, a3_),
		  m_cArray()
	{}

	// set array data
	void setArray(int iDataID_)
	{
		m_cArray.setDataID(iDataID_);
	}
	// get data
	Action::ArrayDataHolder& getArray()
	{
		return m_cArray;
	}
	// check whether result can be known as unknown
	virtual bool isUnknown() = 0;

private:
	Action::ArrayDataHolder m_cArray;
};

////////////////////////////////////////////////////////////////////////////////
// TEMPLATE CLASS
//	Execution::Predicate::ArrayCheck::AnyElement -- predicate class for checking array elements
//
// TEMPLATE ARGUMENTS
//	class Base_
//
// NOTES
//	This class is not constructed directly

template <class Base_>
class AnyElement
	: public Base<Base_>
{
public:
	typedef Base<Base_> Super;
	typedef AnyElement This;
	typedef Interface::IPredicate::Boolean Boolean;

	// destructor
	virtual ~AnyElement() {}

	using Super::isUnknown;
	using Super::getArray;
	using Super::checkByData;

protected:
	// constructor
	AnyElement() : Super() {}
	template <class A_>
	AnyElement(A_ a_)
		: Super(a_)
	{}
	template <class A1_, class A2_>
	AnyElement(A1_ a1_, A2_ a2_)
		: Super(a1_, a2_)
	{}
	template <class A1_, class A2_, class A3_>
	AnyElement(A1_ a1_, A2_ a2_, A3_ a3_)
		: Super(a1_, a2_, a3_)
	{}

private:
///////////////////////////
// Predicate::Base::
	virtual Boolean::Value
					evaluate(Interface::IProgram& cProgram_,
							 Action::ActionList& cActionList_)
	{
		if (isUnknown()) {
			return Boolean::Unknown;
		}
		// if any element of array goes true, result is true,
		// otherwise, result is false even when all elements went into unknown
		const Common::DataArrayData* pArray = getArray().getData();
		int n = pArray->getCount();
		for (int i = 0; i < n; ++i) {
			if (Boolean::True == checkByData(cProgram_,
											 pArray->getElement(i).get()))
				return Boolean::True;
		}
		return Boolean::False;
	}
};

////////////////////////////////////////////////////////////////////////////////
// TEMPLATE CLASS
//	Execution::Predicate::ArrayCheck::AllElement -- predicate class for checking array elements
//
// TEMPLATE ARGUMENTS
//	class Base_
//
// NOTES
//	This class is not constructed directly

template <class Base_>
class AllElement
	: public Base<Base_>
{
public:
	typedef Base<Base_> Super;
	typedef AllElement This;
	typedef Interface::IPredicate::Boolean Boolean;

	// destructor
	virtual ~AllElement() {}

	using Super::isUnknown;
	using Super::getArray;
	using Super::checkByData;

protected:
	// constructor
	AllElement() : Super() {}
	template <class A_>
	AllElement(A_ a_)
		: Super(a_)
	{}
	template <class A1_, class A2_>
	AllElement(A1_ a1_, A2_ a2_)
		: Super(a1_, a2_)
	{}
	template <class A1_, class A2_, class A3_>
	AllElement(A1_ a1_, A2_ a2_, A3_ a3_)
		: Super(a1_, a2_, a3_)
	{}

private:
///////////////////////////
// Predicate::Base::
	virtual Boolean::Value
					evaluate(Interface::IProgram& cProgram_,
							 Action::ActionList& cActionList_)
	{
		if (isUnknown()) {
			return Boolean::Unknown;
		}
		// if all elements of array goes true, result is true,
		// otherwise, result is false even when all elements went into unknown
		const Common::DataArrayData* pArray = getArray().getData();
		int n = pArray->getCount();
		for (int i = 0; i < n; ++i) {
			if (Boolean::True != checkByData(cProgram_,
											 pArray->getElement(i).get())) {
				return Boolean::False;
			}
		}
		return Boolean::True;
	}
};

} // namespace ArrayCheck

_SYDNEY_EXECUTION_PREDICATE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_PREDICATE_ARRAYCHECK_H

//
//	Copyright (c) 2011, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
