// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Comparison.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2016, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Predicate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Predicate/Comparison.h"
#include "Execution/Predicate/ArrayCheck.h"
#include "Execution/Predicate/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

namespace
{
	const char* const _pszExplainName = "check ";
	const char* const _pszOperatorName[] = {
		"=",
		"<=",
		">=",
		"<",
		">",
		"!=",
		"is null",
		"is not null",
		"distinct",
		"<undefined>",
	};
}
namespace ComparisonImpl
{
	// CLASS local
	//	Execution::Predicate::ComparisonImpl::Dyadic -- implementation class of Comparison
	//
	// NOTES
	class Dyadic
		: public Predicate::Comparison
	{
	public:
		typedef Dyadic This;
		typedef Predicate::Comparison Super;

		// constructor
		Dyadic()
			: Super(),
			  m_eType(Type::Undefined),
			  m_cData0(),
			  m_cData1()
		{}
		Dyadic(Type::Value eType_)
			: Super(),
			  m_eType(eType_),
			  m_cData0(),
			  m_cData1()
		{}
		Dyadic(Type::Value eType_, int iDataID0_, int iDataID1_)
			: Super(),
			  m_eType(eType_),
			  m_cData0(iDataID0_),
			  m_cData1(iDataID1_)
		{}

		// destructor
		virtual ~Dyadic() {}

	///////////////////////////
	// Predicate::Comparison::

	/////////////////////////////
	// Interface::IPredicate::
		virtual Boolean::Value checkByData(Interface::IProgram& cProgram_,
										   const Common::Data* pData_);

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
		const Common::Data* getData0() {return m_cData0.getData();}
		const Common::Data* getData1() {return m_cData1.getData();}

	private:
	///////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		// type
		Type::Value m_eType;

		// operands
		Action::DataHolder m_cData0;
		Action::DataHolder m_cData1;
	};

	// CLASS local
	//	Execution::Predicate::ComparisonImpl::Monadic -- implementation class of Comparison
	//
	// NOTES
	class Monadic
		: public Predicate::Comparison
	{
	public:
		typedef Monadic This;
		typedef Predicate::Comparison Super;

		// constructor
		Monadic()
			: Super(),
			  m_eType(Type::Undefined),
			  m_cData()
		{}
		Monadic(Type::Value eType_)
			: Super(),
			  m_eType(eType_),
			  m_cData()
		{}
		Monadic(Type::Value eType_, int iDataID_)
			: Super(),
			  m_eType(eType_),
			  m_cData(iDataID_)
		{}

		// destructor
		virtual ~Monadic() {}

	///////////////////////////
	// Predicate::Comparison::

	/////////////////////////////
	// Interface::IPredicate::
		virtual Boolean::Value checkByData(Interface::IProgram& cProgram_,
										   const Common::Data* pData_);

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
		const Common::Data* getData() {return m_cData.getData();}

	private:
	///////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		// type
		Type::Value m_eType;
		// operand
		Action::DataHolder m_cData;
	};

	// CLASS local
	//	Execution::Predicate::ComparisonImpl::Distinct -- implementation class of Comparison
	//
	// NOTES
	class Distinct
		: public Dyadic
	{
	public:
		typedef Distinct This;
		typedef Dyadic Super;

		// constructor
		Distinct()
			: Super()
		{}
		Distinct(Type::Value eType_)
			: Super(eType_)
		{}
		Distinct(Type::Value eType_, int iDataID0_, int iDataID1_)
			: Super(eType_, iDataID0_, iDataID1_)
		{}

		// destructor
		~Distinct() {}

	///////////////////////////
	// Predicate::Comparison::

	/////////////////////////////
	// Interface::IPredicate::
		virtual Boolean::Value checkByData(Interface::IProgram& cProgram_,
										   const Common::Data* pData_);

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	};

	// CLASS local
	//	Execution::Predicate::ComparisonImpl::DyadicRow -- implementation class of Comparison
	//
	// NOTES
	class DyadicRow
		: public Predicate::Comparison
	{
	public:
		typedef DyadicRow This;
		typedef Predicate::Comparison Super;

		// constructor
		DyadicRow()
			: Super(),
			  m_eType(Type::Undefined),
			  m_vecData0(),
			  m_vecData1()
		{}
		DyadicRow(Type::Value eType_)
			: Super(),
			  m_eType(eType_),
			  m_vecData0(),
			  m_vecData1()
		{}
		DyadicRow(Type::Value eType_,
				  const VECTOR<int>& vecDataID0_,
				  const VECTOR<int>& vecDataID1_)
			: Super(),
			  m_eType(eType_),
			  m_vecData0(),
			  m_vecData1()
		{
			setData(m_vecData0, vecDataID0_);
			setData(m_vecData1, vecDataID1_);
		}

		// destructor
		~DyadicRow() {}

	///////////////////////////
	// Predicate::Comparison::

	/////////////////////////////
	// Interface::IPredicate::
	//	virtual Boolean::Value checkByData(Interface::IProgram& cProgram_,
	//									   const Common::Data* pData_);

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
		void setData(VECTOR<Action::DataHolder>& vecData_,
					 const VECTOR<int>& vecID_);
		Boolean::Value compare(Interface::IProgram& cProgram_,
							   Type::Value eType_,
							   const Action::DataHolder& cData0_,
							   const Action::DataHolder& cData1_);

	///////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		// type
		Type::Value m_eType;

		// operands
		VECTOR<Action::DataHolder> m_vecData0;
		VECTOR<Action::DataHolder> m_vecData1;
	};

	// CLASS local
	//	Execution::Predicate::ComparisonImpl::DyadicAnyElement -- implementation class of Comparison
	//
	// NOTES
	class DyadicAnyElement
		: public ArrayCheck::AnyElement<Dyadic>
	{
	public:
		typedef DyadicAnyElement This;
		typedef ArrayCheck::AnyElement<Dyadic> Super;

		// constructor
		DyadicAnyElement()
			: Super()
		{}
		DyadicAnyElement(Type::Value eType_)
			: Super(eType_)
		{}
		DyadicAnyElement(Type::Value eType_, int iDataID0_, int iDataID1_)
			: Super(eType_, iDataID0_, iDataID1_)
		{
			setArray(iDataID0_);
		}

		// destructor
		~DyadicAnyElement() {}

	///////////////////////////
	// Predicate::Comparison::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	////////////////////////////
	// ArrayCheck::Base::
		virtual bool isUnknown();
	};

	// CLASS local
	//	Execution::Predicate::ComparisonImpl::MonadicAnyElement -- implementation class of Comparison
	//
	// NOTES
	class MonadicAnyElement
		: public ArrayCheck::AnyElement<Monadic>
	{
	public:
		typedef MonadicAnyElement This;
		typedef ArrayCheck::AnyElement<Monadic> Super;

		// constructor
		MonadicAnyElement()
			: Super(),
			  m_bCascade(false)
		{}
		MonadicAnyElement(Type::Value eType_)
			: Super(eType_),
			  m_bCascade(false)
		{}
		MonadicAnyElement(Type::Value eType_, int iDataID_, bool bCascade_)
			: Super(eType_, iDataID_),
			  m_bCascade(bCascade_)
		{
			setArray(iDataID_);
		}

		// destructor
		~MonadicAnyElement() {}

	///////////////////////////
	// Predicate::Comparison::

	/////////////////////////////
	// Interface::IPredicate::
		virtual Boolean::Value checkByData(Interface::IProgram& cProgram_,
										   const Common::Data* pData_);

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	////////////////////////////
	// ArrayCheck::Base::
		virtual bool isUnknown();

		bool m_bCascade;
	};

	// CLASS local
	//	Execution::Predicate::ComparisonImpl::DyadicAllElement -- implementation class of Comparison
	//
	// NOTES
	class DyadicAllElement
		: public ArrayCheck::AllElement<Dyadic>
	{
	public:
		typedef DyadicAllElement This;
		typedef ArrayCheck::AllElement<Dyadic> Super;

		// constructor
		DyadicAllElement()
			: Super()
		{}
		DyadicAllElement(Type::Value eType_)
			: Super(eType_)
		{}
		DyadicAllElement(Type::Value eType_, int iDataID0_, int iDataID1_)
			: Super(eType_, iDataID0_, iDataID1_)
		{
			setArray(iDataID0_);
		}

		// destructor
		~DyadicAllElement() {}

	///////////////////////////
	// Predicate::Comparison::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	////////////////////////////
	// ArrayCheck::Base::
		virtual bool isUnknown();
	};

	// CLASS local
	//	Execution::Predicate::ComparisonImpl::MonadicAllElement -- implementation class of Comparison
	//
	// NOTES
	class MonadicAllElement
		: public ArrayCheck::AllElement<Monadic>
	{
	public:
		typedef MonadicAllElement This;
		typedef ArrayCheck::AllElement<Monadic> Super;

		// constructor
		MonadicAllElement()
			: Super(),
			  m_bCascade(false)
		{}
		MonadicAllElement(Type::Value eType_)
			: Super(eType_),
			  m_bCascade(false)
		{}
		MonadicAllElement(Type::Value eType_, int iDataID_, bool bCascade_)
			: Super(eType_, iDataID_),
			  m_bCascade(bCascade_)
		{
			setArray(iDataID_);
		}

		// destructor
		~MonadicAllElement() {}

	///////////////////////////
	// Predicate::Comparison::

	/////////////////////////////
	// Interface::IPredicate::
		virtual Boolean::Value checkByData(Interface::IProgram& cProgram_,
										   const Common::Data* pData_);

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
	////////////////////////////
	// ArrayCheck::Base::
		virtual bool isUnknown();

		bool m_bCascade;
	};
} // namespace ComparisonImpl

namespace
{
	const Comparison::Boolean::Value _CompareTable[][Comparison::Type::ValueNum] =
	{
#define F Comparison::Boolean::False
#define T Comparison::Boolean::True
#define U Comparison::Boolean::Unknown
	  // eq le ge lt gt ne nl nn dt ??
		{F, T, F, T, F, T, F, T, T, U},	// compare == -1 (notnull for monadic)
		{T, T, T, F, F, F, T, F, F, U},	// compare == 0 (null for monadic)
		{F, F, T, F, T, T, U, U, T, U},	// compare == 1
#undef F
#undef T
#undef U
	};

} // namespace $$$

///////////////////////////////////////
// Predicate::ComparisonImpl::Dyadic

// FUNCTION public
//	Predicate::ComparisonImpl::Dyadic::checkByData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	Comparison::Boolean::Value
//
// EXCEPTIONS

//virtual
Comparison::Boolean::Value
ComparisonImpl::Dyadic::
checkByData(Interface::IProgram& cProgram_,
			const Common::Data* pData_)
{
	if (getData1()->isNull() || pData_->isNull()) {
		return Boolean::Unknown;
	}

	int iResult = pData_->compareTo(getData1());
	; _SYDNEY_ASSERT(iResult >= -1 && iResult <= 1);

	return _CompareTable[iResult+1][m_eType];
}

// FUNCTION public
//	Predicate::ComparisonImpl::Dyadic::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Dyadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszExplainName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cData0.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	cExplain_.put(_pszOperatorName[m_eType]);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cData1.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::ComparisonImpl::Dyadic::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Dyadic::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData0.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData0.initialize(cProgram_);
		m_cData1.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::ComparisonImpl::Dyadic::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Dyadic::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData0.isInitialized()) {
		m_cData0.terminate(cProgram_);
		m_cData1.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::ComparisonImpl::Dyadic::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Dyadic::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::ComparisonImpl::Dyadic::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Dyadic::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::ComparisonImpl::Dyadic::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
ComparisonImpl::Dyadic::
getClassID() const
{
	return Class::getClassID(Class::Category::ComparisonDyadic);
}

// FUNCTION public
//	Predicate::ComparisonImpl::Dyadic::serialize -- for serialize
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Dyadic::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	int iType = m_eType;
	cArchive_(iType);
	if (cArchive_.isLoad()) {
		m_eType = static_cast<Type::Value>(iType);
	}
	m_cData0.serialize(cArchive_);
	m_cData1.serialize(cArchive_);
}

// FUNCTION private
//	Predicate::ComparisonImpl::Dyadic::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Comparison::Boolean::Value
//
// EXCEPTIONS

//virtual
Comparison::Boolean::Value
ComparisonImpl::Dyadic::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return checkByData(cProgram_,
					   getData0());
}

///////////////////////////////////////
// Predicate::ComparisonImpl::Monadic

// FUNCTION public
//	Predicate::ComparisonImpl::Monadic::checkByData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	Comparison::Boolean::Value
//
// EXCEPTIONS

//virtual
Comparison::Boolean::Value
ComparisonImpl::Monadic::
checkByData(Interface::IProgram& cProgram_,
			const Common::Data* pData_)
{
	return _CompareTable[pData_->isNull() ? 1 : 0][m_eType];
}

// FUNCTION public
//	Predicate::ComparisonImpl::Monadic::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Monadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszExplainName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cData.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	cExplain_.put(_pszOperatorName[m_eType]);
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::ComparisonImpl::Monadic::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Monadic::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::ComparisonImpl::Monadic::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Monadic::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized()) {
		terminateBase(cProgram_);
		m_cData.terminate(cProgram_);
	}
}

// FUNCTION public
//	Predicate::ComparisonImpl::Monadic::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Monadic::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::ComparisonImpl::Monadic::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Monadic::
reset(Interface::IProgram& cProgram_)
{
}

// FUNCTION public
//	Predicate::ComparisonImpl::Monadic::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
ComparisonImpl::Monadic::
getClassID() const
{
	return Class::getClassID(Class::Category::ComparisonMonadic);
}

// FUNCTION public
//	Predicate::ComparisonImpl::Monadic::serialize -- for serialize
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::Monadic::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	int iType = m_eType;
	cArchive_(iType);
	if (cArchive_.isLoad()) {
		m_eType = static_cast<Type::Value>(iType);
	}
	m_cData.serialize(cArchive_);
}

// FUNCTION private
//	Predicate::ComparisonImpl::Monadic::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Comparison::Boolean::Value
//
// EXCEPTIONS

//virtual
Comparison::Boolean::Value
ComparisonImpl::Monadic::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return checkByData(cProgram_,
					   getData());
}

/////////////////////////////////////////////////
// Predicate::ComparisonImpl::Distinct

// FUNCTION public
//	Predicate::ComparisonImpl::Distinct::checkByData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	Comparison::Boolean::Value
//
// EXCEPTIONS

//virtual
Comparison::Boolean::Value
ComparisonImpl::Distinct::
checkByData(Interface::IProgram& cProgram_,
			const Common::Data* pData_)
{
	return pData_->distinct(getData1())
		? Boolean::True : Boolean::False;
}

// FUNCTION public
//	Predicate::ComparisonImpl::Distinct::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
ComparisonImpl::Distinct::
getClassID() const
{
	return Class::getClassID(Class::Category::ComparisonDistinct);
}

/////////////////////////////////////////////////
// Predicate::ComparisonImpl::DyadicRow::

// FUNCTION public
//	Predicate::ComparisonImpl::DyadicRow::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::DyadicRow::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszExplainName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.pushIndent();
		cExplain_.put("(");
		Opt::Join(m_vecData0,
				  boost::bind(&Action::DataHolder::explain,
							  _1,
							  boost::ref(cProgram_),
							  boost::ref(cExplain_)),
				  boost::bind(&Opt::Explain::putChar,
							  boost::ref(cExplain_),
							  ','));
		cExplain_.put(") ");
		cExplain_.popIndent();
	}
	cExplain_.put(_pszOperatorName[m_eType]);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.pushIndent();
		cExplain_.put(" (");
		Opt::Join(m_vecData1,
				  boost::bind(&Action::DataHolder::explain,
							  _1,
							  boost::ref(cProgram_),
							  boost::ref(cExplain_)),
				  boost::bind(&Opt::Explain::putChar,
							  boost::ref(cExplain_),
							  ','));
		cExplain_.put(") ");
		cExplain_.popIndent();
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::ComparisonImpl::DyadicRow::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::DyadicRow::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	Opt::ForEach(m_vecData0,
				 boost::bind(&Action::DataHolder::initialize,
							 _1,
							 boost::ref(cProgram_)));
	Opt::ForEach(m_vecData1,
				 boost::bind(&Action::DataHolder::initialize,
							 _1,
							 boost::ref(cProgram_)));
}

// FUNCTION public
//	Predicate::ComparisonImpl::DyadicRow::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::DyadicRow::
terminate(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecData0,
				 boost::bind(&Action::DataHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	Opt::ForEach(m_vecData1,
				 boost::bind(&Action::DataHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	terminateBase(cProgram_);
}

// FUNCTION public
//	Predicate::ComparisonImpl::DyadicRow::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::DyadicRow::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::ComparisonImpl::DyadicRow::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::DyadicRow::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::ComparisonImpl::DyadicRow::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
ComparisonImpl::DyadicRow::
getClassID() const
{
	return Class::getClassID(Class::Category::ComparisonDyadicRow);
}

// FUNCTION public
//	Predicate::ComparisonImpl::DyadicRow::serialize -- for serialize
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::DyadicRow::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	int iType = m_eType;
	cArchive_(iType);
	if (cArchive_.isLoad()) {
		m_eType = static_cast<Type::Value>(iType);
	}
	Utility::SerializeObject(cArchive_, m_vecData0);
	Utility::SerializeObject(cArchive_, m_vecData1);
}

// FUNCTION private
//	Predicate::ComparisonImpl::DyadicRow::setData -- 
//
// NOTES
//
// ARGUMENTS
//	VECTOR<Action::DataHolder>& vecData_
//	const VECTOR<int>& vecDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ComparisonImpl::DyadicRow::
setData(VECTOR<Action::DataHolder>& vecData_,
		const VECTOR<int>& vecDataID_)
{
	int n = vecDataID_.GETSIZE();
	vecData_.assign(n, Action::DataHolder());
	for (int i = 0; i < n; ++i) {
		vecData_[i].setDataID(vecDataID_[i]);
	}
}

// FUNCTION private
//	Predicate::ComparisonImpl::DyadicRow::compare -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::DataHolder& cData0_
//	const Action::DataHolder& cData1_
//	
// RETURN
//	Comparison::Boolean::Value
//
// EXCEPTIONS

Comparison::Boolean::Value
ComparisonImpl::DyadicRow::
compare(Interface::IProgram& cProgram_,
		Type::Value eType_,
		const Action::DataHolder& cData0_,
		const Action::DataHolder& cData1_)
{
	if (cData0_->isNull() || cData1_->isNull()) {
		return Boolean::Unknown;
	}

	int iResult = cData0_->compareTo(cData1_.getData());
	; _SYDNEY_ASSERT(iResult >= -1 && iResult <= 1);

	return _CompareTable[iResult+1][eType_];
}

// FUNCTION private
//	Predicate::ComparisonImpl::DyadicRow::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Comparison::Boolean::Value
//
// EXCEPTIONS

//virtual
Comparison::Boolean::Value
ComparisonImpl::DyadicRow::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	VECTOR<Action::DataHolder>::CONSTITERATOR iterator0 = m_vecData0.begin();
	const VECTOR<Action::DataHolder>::CONSTITERATOR last0 = m_vecData0.end();
	VECTOR<Action::DataHolder>::CONSTITERATOR iterator1 = m_vecData1.begin();

	; _SYDNEY_ASSERT(m_vecData0.GETSIZE() == m_vecData1.GETSIZE());

	// skip while two elements are equal
	while (iterator0 != last0
		   && compare(cProgram_,
					  Type::Equals,
					  *iterator0,
					  *iterator1) == Boolean::True) {
		++iterator0;
		++iterator1;
	}
	if (iterator0 != last0) {
		return compare(cProgram_,
					   m_eType,
					   *iterator0,
					   *iterator1);
	}
	return _CompareTable[1 /* == */][m_eType];
}

/////////////////////////////////////////////////
// Predicate::ComparisonImpl::DyadicAnyElement

// FUNCTION public
//	Predicate::ComparisonImpl::DyadicAnyElement::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
ComparisonImpl::DyadicAnyElement::
getClassID() const
{
	return Class::getClassID(Class::Category::ComparisonDyadicAnyElement);
}

// FUNCTION private
//	Predicate::ComparisonImpl::DyadicAnyElement::isUnknown -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
ComparisonImpl::DyadicAnyElement::
isUnknown()
{
	return getData0()->isNull() || getData1()->isNull();
}

/////////////////////////////////////////////////
// Predicate::ComparisonImpl::MonadicAnyElement

// FUNCTION public
//	Predicate::ComparisonImpl::MonadicAnyElement::checkByData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	Comparison::Boolean::Value
//
// EXCEPTIONS

//virtual
Comparison::Boolean::Value
ComparisonImpl::MonadicAnyElement::
checkByData(Interface::IProgram& cProgram_,
			const Common::Data* pData_)
{
	if (m_bCascade == false
		|| pData_->isNull()
		|| pData_->getType() != Common::DataType::Array) {
		return Super::checkByData(cProgram_, pData_);
	} else {
		// check element
		; _SYDNEY_ASSERT(pData_->getElementType() == Common::DataType::Data);
		const Common::DataArrayData* pArrayData =
			_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_);
		int n = pArrayData->getCount();
		for (int i = 0; i < n; ++i) {
			if (Boolean::True == checkByData(cProgram_,
											 pArrayData->getElement(i).get())) {
				return Boolean::True;
			}
		}
		return Boolean::False;
	}
}

// FUNCTION public
//	Predicate::ComparisonImpl::MonadicAnyElement::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
ComparisonImpl::MonadicAnyElement::
getClassID() const
{
	return Class::getClassID(Class::Category::ComparisonMonadicAnyElement);
}

// FUNCTION private
//	Predicate::ComparisonImpl::MonadicAnyElement::isUnknown -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
ComparisonImpl::MonadicAnyElement::
isUnknown()
{
	// monadic predicate become null when array is empty or null
	return getData()->isNull() || getArray()->getCount() == 0;
}

/////////////////////////////////////////////////
// Predicate::ComparisonImpl::DyadicAllElement

// FUNCTION public
//	Predicate::ComparisonImpl::DyadicAllElement::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
ComparisonImpl::DyadicAllElement::
getClassID() const
{
	return Class::getClassID(Class::Category::ComparisonDyadicAllElement);
}

// FUNCTION private
//	Predicate::ComparisonImpl::DyadicAllElement::isUnknown -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
ComparisonImpl::DyadicAllElement::
isUnknown()
{
	return getData0()->isNull() || getData1()->isNull();
}

/////////////////////////////////////////////////
// Predicate::ComparisonImpl::MonadicAllElement

// FUNCTION public
//	Predicate::ComparisonImpl::MonadicAllElement::checkByData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	Comparison::Boolean::Value
//
// EXCEPTIONS

//virtual
Comparison::Boolean::Value
ComparisonImpl::MonadicAllElement::
checkByData(Interface::IProgram& cProgram_,
			const Common::Data* pData_)
{
	if (m_bCascade == false
		|| pData_->isNull()
		|| pData_->getType() != Common::DataType::Array) {
		return Super::checkByData(cProgram_, pData_);
	} else {
		// check element
		; _SYDNEY_ASSERT(pData_->getElementType() == Common::DataType::Data);
		const Common::DataArrayData* pArrayData =
			_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_);
		int n = pArrayData->getCount();
		for (int i = 0; i < n; ++i) {
			if (Boolean::True != checkByData(cProgram_,
											 pArrayData->getElement(i).get())) {
				return Boolean::False;
			}
		}
		return Boolean::True;
	}
}

// FUNCTION public
//	Predicate::ComparisonImpl::MonadicAllElement::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
ComparisonImpl::MonadicAllElement::
getClassID() const
{
	return Class::getClassID(Class::Category::ComparisonMonadicAllElement);
}

// FUNCTION public
//	Predicate::ComparisonImpl::MonadicAllElement::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ComparisonImpl::MonadicAllElement::
serialize(ModArchive& archiver_)
{
	Super::serialize(archiver_);
	archiver_(m_bCascade);
}

// FUNCTION private
//	Predicate::ComparisonImpl::MonadicAllElement::isUnknown -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
ComparisonImpl::MonadicAllElement::
isUnknown()
{
	return getData()->isNull();
}

///////////////////////////////////////////
// Predicate::Comparison::Row::

// FUNCTION public
//	Predicate::Comparison::Row::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecData0_
//	const VECTOR<int>& vecData1_
//	Type::Value eType_
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::Row::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecData0_,
	   const VECTOR<int>& vecData1_,
	   Type::Value eType_)
{
	if (vecData0_.GETSIZE() <= 1
		|| vecData0_.GETSIZE() != vecData1_.GETSIZE()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	AUTOPOINTER<This> pResult;
	switch (eType_) {
	case Type::Equals:
	case Type::LessThanEquals:
	case Type::GreaterThanEquals:
	case Type::LessThan:
	case Type::GreaterThan:
	case Type::NotEquals:
		{
			pResult = new ComparisonImpl::DyadicRow(eType_, vecData0_, vecData1_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

///////////////////////////////////////////
// Predicate::Comparison::AnyElement::

// FUNCTION public
//	Predicate::Comparison::AnyElement::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	Type::Value eType_
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::AnyElement::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   Type::Value eType_)
{
	AUTOPOINTER<This> pResult;
	switch (eType_) {
	case Type::Equals:
	case Type::LessThanEquals:
	case Type::GreaterThanEquals:
	case Type::LessThan:
	case Type::GreaterThan:
	case Type::NotEquals:
		{
			pResult = new ComparisonImpl::DyadicAnyElement(eType_, iDataID0_, iDataID1_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Comparison::AnyElement::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecData0_
//	int iData1_
//	Type::Value eType_
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::AnyElement::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecData0_,
	   int iData1_,
	   Type::Value eType_)
{
	if (vecData0_.GETSIZE() == 1) {
		return Comparison::create(cProgram_,
								  pIterator_,
								  vecData0_[0],
								  iData1_,
								  eType_);
	}
	return AnyElement::create(cProgram_,
							  pIterator_,
							  cProgram_.addVariable(vecData0_),
							  iData1_,
							  eType_);
}

// FUNCTION public
//	Predicate::Comparison::AnyElement::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	Type::Value eType_
//	bool bCascade_ = false
//	bool bCheckIsArray_ = false
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::AnyElement::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_,
	   Type::Value eType_,
	   bool bCascade_ /* = false */,
	   bool bCheckIsArray_ /* = false */)
{
	if (bCheckIsArray_ && cProgram_.isArray(iDataID_) == false) {
		// normal comparison
		return Comparison::create(cProgram_,
								  pIterator_,
								  iDataID_,
								  eType_);
	}

	AUTOPOINTER<This> pResult;
	switch (eType_) {
	case Type::IsNull:
	case Type::IsNotNull:
		{
			pResult = new ComparisonImpl::MonadicAnyElement(eType_, iDataID_, bCascade_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Comparison::AnyElement::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecDataID_
//	Type::Value eType_
//	bool bCascade_ = false
//	bool bCheckIsArray_ = false
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::AnyElement::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecDataID_,
	   Type::Value eType_,
	   bool bCascade_ /* = false */,
	   bool bCheckIsArray_ /* = false */)
{
	if (vecDataID_.GETSIZE() == 1) {
		return AnyElement::create(cProgram_,
								  pIterator_,
								  vecDataID_[0],
								  eType_,
								  bCascade_,
								  bCheckIsArray_);
	}
	return AnyElement::create(cProgram_,
							  pIterator_,
							  cProgram_.addVariable(vecDataID_),
							  eType_,
							  bCascade_);
}

///////////////////////////////////////////
// Predicate::Comparison::AllElement::

// FUNCTION public
//	Predicate::Comparison::AllElement::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	Type::Value eType_
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::AllElement::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   Type::Value eType_)
{
	AUTOPOINTER<This> pResult;
	switch (eType_) {
	case Type::Equals:
	case Type::LessThanEquals:
	case Type::GreaterThanEquals:
	case Type::LessThan:
	case Type::GreaterThan:
	case Type::NotEquals:
		{
			pResult = new ComparisonImpl::DyadicAllElement(eType_, iDataID0_, iDataID1_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Comparison::AllElement::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecData0_
//	int iData1_
//	Type::Value eType_
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::AllElement::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecData0_,
	   int iData1_,
	   Type::Value eType_)
{
	return AllElement::create(cProgram_,
							  pIterator_,
							  cProgram_.addVariable(vecData0_),
							  iData1_,
							  eType_);
}

// FUNCTION public
//	Predicate::Comparison::AllElement::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	Type::Value eType_
//	bool bCascade_ = false
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::AllElement::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_,
	   Type::Value eType_,
	   bool bCascade_ /* = false */)
{
	AUTOPOINTER<This> pResult;
	switch (eType_) {
	case Type::IsNull:
	case Type::IsNotNull:
		{
			pResult = new ComparisonImpl::MonadicAllElement(eType_, iDataID_, bCascade_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Comparison::AllElement::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecDataID_
//	Type::Value eType_
//	bool bCascade_ = false
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::AllElement::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecDataID_,
	   Type::Value eType_,
	   bool bCascade_ /* = false */)
{
	return AllElement::create(cProgram_,
							  pIterator_,
							  cProgram_.addVariable(vecDataID_),
							  eType_,
							  bCascade_);
}

//////////////////////////////
// Predicate::Comparison::

// FUNCTION public
//	Predicate::Comparison::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	Type::Value eType_
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   Type::Value eType_)
{
	AUTOPOINTER<This> pResult;
	switch (eType_) {
	case Type::Equals:
	case Type::LessThanEquals:
	case Type::GreaterThanEquals:
	case Type::LessThan:
	case Type::GreaterThan:
	case Type::NotEquals:
		{
			pResult = new ComparisonImpl::Dyadic(eType_, iDataID0_, iDataID1_);
			break;
		}
	case Type::IsDistinct:
		{
			pResult = new ComparisonImpl::Distinct(eType_, iDataID0_, iDataID1_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Comparison::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	Type::Value eType_
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_,
	   Type::Value eType_)
{
	AUTOPOINTER<This> pResult;
	switch (eType_) {
	case Type::IsNull:
	case Type::IsNotNull:
		{
			pResult = new ComparisonImpl::Monadic(eType_, iDataID_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Comparison::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::ComparisonDyadic:
		{
			return new ComparisonImpl::Dyadic;
		}
	case Class::Category::ComparisonMonadic:
		{
			return new ComparisonImpl::Monadic;
		}
	case Class::Category::ComparisonDistinct:
		{
			return new ComparisonImpl::Distinct;
		}
	case Class::Category::ComparisonDyadicAnyElement:
		{
			return new ComparisonImpl::DyadicAnyElement;
		}
	case Class::Category::ComparisonMonadicAnyElement:
		{
			return new ComparisonImpl::MonadicAnyElement;
		}
	case Class::Category::ComparisonDyadicAllElement:
		{
			return new ComparisonImpl::DyadicAllElement;
		}
	case Class::Category::ComparisonMonadicAllElement:
		{
			return new ComparisonImpl::MonadicAllElement;
		}
	case Class::Category::ComparisonDyadicRow:
		{
			return new ComparisonImpl::DyadicRow;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_PREDICATE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
