// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/Exists.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Iterator";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Iterator/Exists.h"
#include "Execution/Iterator/Dyadic.h"
#include "Execution/Iterator/Class.h"
#include "Execution/Iterator/OperandElement.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/Unexpected.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

namespace
{
	struct _Type
	{
		enum Value {
			Exists = 0,
			NotExists,
			ValueNum
		};
	};

	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName[] = {
		"exists join",
		"not exists join",
		0
	};
}

namespace Impl
{
	// CLASS local
	//	Execution::Iterator::Impl::ExistsImpl -- implementation class of Exists
	//
	// NOTES
	class ExistsImpl
		: public Dyadic<Iterator::Exists>
	{
	public:
		typedef ExistsImpl This;
		typedef Dyadic<Iterator::Exists> Super;

		ExistsImpl()
			: Super()
		{}
		virtual ~ExistsImpl()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);
	//	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////////
	// Iterator::Base
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);

	private:
	};

	// CLASS local
	//	Execution::Iterator::Impl::NotExistsImpl -- implementation class of Exists
	//
	// NOTES
	class NotExistsImpl
		: public ExistsImpl
	{
	public:
		typedef NotExistsImpl This;
		typedef ExistsImpl Super;

		NotExistsImpl()
			: Super()
		{}
		~NotExistsImpl()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);
	//	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
	//	virtual void finish(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////////
	// Iterator::Base
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);

	private:
	};
} // namespace Impl

/////////////////////////////////////////////
// Execution::Iterator::Impl::ExistsImpl

// FUNCTION public
//	Iterator::Impl::ExistsImpl::next -- go to next tuple
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::ExistsImpl::
next(Interface::IProgram& cProgram_)
{
	if (hasNext() == true) {
		while (getOperand0().next(cProgram_) == true) {
			getOperand1().reset(cProgram_);
			if (getOperand1().next(cProgram_) == true) {
				return true;
			}
		}
	}
	return setHasNext(setHasData(false));
}

// FUNCTION public
//	Iterator::Impl::ExistsImpl::reset -- 
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
Impl::ExistsImpl::
reset(Interface::IProgram& cProgram_)
{
	getOperand0().reset(cProgram_);
	resetBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::ExistsImpl::getClassID -- 
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
Impl::ExistsImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Exists);
}

// FUNCTION protected
//	Iterator::Impl::ExistsImpl::explainThis -- 
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
Impl::ExistsImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::Exists]);
}

/////////////////////////////////////////////
// Execution::Iterator::Impl::NotExistsImpl

// FUNCTION public
//	Iterator::Impl::NotExistsImpl::next -- go to next tuple
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::NotExistsImpl::
next(Interface::IProgram& cProgram_)
{
	if (hasNext()) {
		while (getOperand0().next(cProgram_) == true) {
			getOperand1().reset(cProgram_);
			if (getOperand1().next(cProgram_) == false) {
				return true;
			}
		}
	}
	return setHasNext(setHasData(false));
}

// FUNCTION public
//	Iterator::Impl::NotExistsImpl::getClassID -- 
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
Impl::NotExistsImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::NotExists);
}

// FUNCTION protected
//	Iterator::Impl::NotExistsImpl::explainThis -- 
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
Impl::NotExistsImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::NotExists]);
}

//////////////////////////////
// Iterator::Exists::Not::

// FUNCTION public
//	Iterator::Exists::Not::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Exists*
//
// EXCEPTIONS

//static
Exists*
Exists::Not::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::NotExistsImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

//////////////////////////////
// Iterator::Exists::

// FUNCTION public
//	Iterator::Exists::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Exists*
//
// EXCEPTIONS

//static
Exists*
Exists::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::ExistsImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::Exists::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Exists*
//
// EXCEPTIONS

//static
Exists*
Exists::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Exists:
		{
			return new Impl::ExistsImpl;
		}
	case Class::Category::NotExists:
		{
			return new Impl::NotExistsImpl;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
