// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/CheckUnknown.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Predicate/CheckUnknown.h"
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
	const char* const _pszExplainName = "check unknown";
}
namespace CheckUnknownImpl
{
	// CLASS local
	//	Execution::Predicate::CheckUnknownImpl::Base -- base class of implementation class of CheckUnknown
	//
	// NOTES
	class Base
		: public CheckUnknown
	{
	public:
		typedef Base This;
		typedef CheckUnknown Super;

		// destructor
		virtual ~Base() {}

	///////////////////////////
	// Predicate::CheckUnknown::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);

		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
	//	int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	virtual void serialize(ModArchive& archiver_);

	protected:
		// constructor
		Base()
			: Super(),
			  m_bArray(false)
		{}
		Base(bool bArray_)
			: Super(),
			  m_bArray(bArray_)
		{}

		void serializeBase(ModArchive& archiver_);
		bool checkIsUnknown(const Common::Data* pData_);

	private:
		virtual void explainData(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_) = 0;
		bool m_bArray;
	};

	// CLASS local
	//	Execution::Predicate::CheckUnknownImpl::Nadic -- implementation class of CheckUnknown
	//
	// NOTES
	class Nadic
		: public Base
	{
	public:
		typedef Nadic This;
		typedef Base Super;

		// constructor
		Nadic()
			: Super(),
			  m_vecData()
		{}
		Nadic(const VECTOR<int>& vecID_,
			  bool bArray_);

		// destructor
		~Nadic() {}

	///////////////////////////
	// Predicate::CheckUnknown::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////////
	// CheckUnknownImpl::Base::
		virtual void explainData(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
	///////////////////////////////
	// Predicate::Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		VECTOR<Action::DataHolder> m_vecData;
	};

	// CLASS local
	//	Execution::Predicate::CheckUnknownImpl::Monadic -- implementation class of CheckUnknown
	//
	// NOTES
	class Monadic
		: public Base
	{
	public:
		typedef Monadic This;
		typedef Base Super;

		// constructor
		Monadic()
			: Super(),
			  m_cData()
		{}
		Monadic(int iID_,
				bool bArray_)
			: Super(bArray_),
			  m_cData(iID_)
		{}

		// destructor
		~Monadic() {}

	///////////////////////////
	// Predicate::CheckUnknown::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////////
	// CheckUnknownImpl::Base::
		virtual void explainData(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
	///////////////////////////////
	// Predicate::Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		Action::DataHolder m_cData;
	};
} // namespace CheckUnknownImpl

///////////////////////////////////////
// Predicate::CheckUnknownImpl::Base

// FUNCTION public
//	Predicate::CheckUnknownImpl::Base::explain -- 
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
CheckUnknownImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushIndent();
	cExplain_.put(_pszExplainName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		explainData(cProgram_, cExplain_);
	}
	cExplain_.popIndent(true /* force new line */);
}

// FUNCTION public
//	Predicate::CheckUnknownImpl::Base::finish -- 
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
CheckUnknownImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::CheckUnknownImpl::Base::reset -- 
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
CheckUnknownImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION protected
//	Predicate::CheckUnknownImpl::Base::serializeBase -- 
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

void
CheckUnknownImpl::Base::
serializeBase(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_bArray);
}

// FUNCTION protected
//	Predicate::CheckUnknownImpl::Base::checkIsUnknown -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
CheckUnknownImpl::Base::
checkIsUnknown(const Common::Data* pData_)
{
	if (m_bArray == false
		|| pData_->getType() != Common::DataType::Array) {
		return pData_->isNull();
	}

	if (pData_->isNull()) {
		return true;
	}
	const Common::DataArrayData* pArray = _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
															   pData_);
	// empty array become null too
	return (pArray && pArray->getCount() == 0);
}

///////////////////////////////////////
// Predicate::CheckUnknownImpl::Nadic

// FUNCTION public
//	Predicate::CheckUnknownImpl::Nadic::Nadic -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecID_
//	bool bArray_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

CheckUnknownImpl::Nadic::
Nadic(const VECTOR<int>& vecID_,
	  bool bArray_)
	: Super(bArray_),
	  m_vecData()
{
	m_vecData.reserve(vecID_.GETSIZE());
	VECTOR<int>::CONSTITERATOR iterator = vecID_.begin();
	const VECTOR<int>::CONSTITERATOR last = vecID_.end();
	for (; iterator != last; ++iterator) {
		m_vecData.PUSHBACK(Action::DataHolder(*iterator));
	}
}

// FUNCTION public
//	Predicate::CheckUnknownImpl::Nadic::initialize -- 
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
CheckUnknownImpl::Nadic::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	Opt::ForEach(m_vecData,
				 boost::bind(&Action::DataHolder::initialize,
							 _1,
							 boost::ref(cProgram_)));
}

// FUNCTION public
//	Predicate::CheckUnknownImpl::Nadic::terminate -- 
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
CheckUnknownImpl::Nadic::
terminate(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecData,
				 boost::bind(&Action::DataHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	terminateBase(cProgram_);
}

// FUNCTION public
//	Predicate::CheckUnknownImpl::Nadic::getClassID -- 
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
CheckUnknownImpl::Nadic::
getClassID() const
{
	return Class::getClassID(Class::Category::CheckUnknownNadic);
}

// FUNCTION public
//	Predicate::CheckUnknownImpl::Nadic::serialize -- for serialize
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
CheckUnknownImpl::Nadic::
serialize(ModArchive& cArchive_)
{
	serializeBase(cArchive_);
	Utility::SerializeObject(cArchive_, m_vecData);
}

// FUNCTION private
//	Predicate::CheckUnknownImpl::Nadic::explainData -- 
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
CheckUnknownImpl::Nadic::
explainData(Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put("(");
	Opt::Join(m_vecData,
			  boost::bind(&Action::DataHolder::explain,
						  _1,
						  boost::ref(cProgram_),
						  boost::ref(cExplain_)),
			  boost::bind(&Opt::Explain::putChar,
						  &cExplain_,
						  ','));
	cExplain_.put(")");
}

// FUNCTION private
//	Predicate::CheckUnknownImpl::Nadic::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	CheckUnknown::Boolean::Value
//
// EXCEPTIONS

//virtual
CheckUnknown::Boolean::Value
CheckUnknownImpl::Nadic::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return Opt::IsAny(m_vecData,
					  boost::bind(&This::checkIsUnknown,
								  this,
								  boost::bind(&Action::DataHolder::getData,
											  _1)))
		? Boolean::Unknown
		: Boolean::False;
}

///////////////////////////////////////
// Predicate::CheckUnknownImpl::Monadic

// FUNCTION public
//	Predicate::CheckUnknownImpl::Monadic::initialize -- 
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
CheckUnknownImpl::Monadic::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::CheckUnknownImpl::Monadic::terminate -- 
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
CheckUnknownImpl::Monadic::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized()) {
		m_cData.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::CheckUnknownImpl::Monadic::getClassID -- 
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
CheckUnknownImpl::Monadic::
getClassID() const
{
	return Class::getClassID(Class::Category::CheckUnknownMonadic);
}

// FUNCTION public
//	Predicate::CheckUnknownImpl::Monadic::serialize -- for serialize
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
CheckUnknownImpl::Monadic::
serialize(ModArchive& cArchive_)
{
	serializeBase(cArchive_);
	m_cData.serialize(cArchive_);
}

// FUNCTION private
//	Predicate::CheckUnknownImpl::Monadic::explainData -- 
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
CheckUnknownImpl::Monadic::
explainData(Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	m_cData.explain(cProgram_, cExplain_);
}

// FUNCTION public
//	Predicate::CheckUnknownImpl::Monadic::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	CheckUnknown::Boolean::Value
//
// EXCEPTIONS

//virtual
CheckUnknown::Boolean::Value
CheckUnknownImpl::Monadic::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return checkIsUnknown(m_cData.getData()) ? Boolean::Unknown : Boolean::False;
}

//////////////////////////////
// Predicate::CheckUnknown::

// FUNCTION public
//	Predicate::CheckUnknown::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iID_
//	bool bArray_
//	
// RETURN
//	CheckUnknown*
//
// EXCEPTIONS

//static
CheckUnknown*
CheckUnknown::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iID_,
	   bool bArray_)
{
	AUTOPOINTER<This> pResult = new CheckUnknownImpl::Monadic(iID_, bArray_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::CheckUnknown::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecID_
//	bool bArray_
//	
// RETURN
//	CheckUnknown*
//
// EXCEPTIONS

//static
CheckUnknown*
CheckUnknown::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecID_,
	   bool bArray_)
{
	AUTOPOINTER<This> pResult = new CheckUnknownImpl::Nadic(vecID_, bArray_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::CheckUnknown::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	CheckUnknown*
//
// EXCEPTIONS

//static
CheckUnknown*
CheckUnknown::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::CheckUnknownNadic:
		{
			return new CheckUnknownImpl::Nadic;
		}
	case Class::Category::CheckUnknownMonadic:
		{
			return new CheckUnknownImpl::Monadic;
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
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
