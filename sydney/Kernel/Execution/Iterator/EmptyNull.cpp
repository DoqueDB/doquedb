// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/EmptyNull.cpp --
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
const char moduleName[] = "Execution::Iterator";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Iterator/EmptyNull.h"
#include "Execution/Iterator/Class.h"
#include "Execution/Iterator/Monadic.h"

#include "Execution/Action/Collection.h"
#include "Execution/Interface/ICollection.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Exception/NotSupported.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/Functional.h"

#include "Opt/Explain.h"
#include "Opt/Sort.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "if empty set null ";
}

namespace Impl
{
	// CLASS local
	//	Execution::Iterator::Impl::EmptyNullImpl -- implementation class of EmptyNull
	//
	// NOTES
	class EmptyNullImpl
		: public Monadic<Iterator::EmptyNull>
	{
	public:
		typedef EmptyNullImpl This;
		typedef Monadic<Iterator::EmptyNull> Super;

		struct Status
		{
			enum Value
			{
				Empty,
				HasNext,
				NoNext,
				SetNull,
				EndOfData,
				ValueNum
			};
		};

		EmptyNullImpl()
			: Super(),
			  m_eStatus(Status::Empty),
			  m_cData()
		{}
		~EmptyNullImpl()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
	//	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
		virtual bool isEndOfData() {return m_eStatus == Status::EndOfData;}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	//////////////////////////////
	// Iterator::Base::
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
		virtual void addInput(Interface::IProgram& cProgram_,
							  const Action::Argument& cAction_);
	private:
		Status::Value m_eStatus;

		// data is always array because it's created by row::generate in plan (joinimpl)
		Action::ArrayDataHolder m_cData;
	};
}

/////////////////////////////////////////////
// Execution::Iterator::Impl::EmptyNullImpl

// FUNCTION public
//	Iterator::Impl::EmptyNullImpl::initialize -- 
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
Impl::EmptyNullImpl::
initialize(Interface::IProgram& cProgram_)
{
	// initialize super classes
	initializeBase(cProgram_);
	initializeOperand(cProgram_);

	m_cData.initialize(cProgram_);
	m_eStatus = Status::Empty;
}

// FUNCTION public
//	Iterator::Impl::EmptyNullImpl::terminate -- 
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
Impl::EmptyNullImpl::
terminate(Interface::IProgram& cProgram_)
{
	// terminate super classes
	terminateBase(cProgram_);
	terminateOperand(cProgram_);

	m_cData.terminate(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::EmptyNullImpl::next -- go to next tuple
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
Impl::EmptyNullImpl::
next(Interface::IProgram& cProgram_)
{
	switch (m_eStatus) {
	case Status::Empty:
		{
			if (getOperand().next(cProgram_)) {
				m_eStatus = Status::HasNext;
			} else {
				// no data -> set data null
				Utility::DataType::setNullElements(m_cData.get());
				m_eStatus = Status::SetNull;
			}
			return true;
		}
	case Status::HasNext:
		{
			if (getOperand().next(cProgram_) == false) {
				m_eStatus = Status::EndOfData;
				return false;
			}
			return true;
		}
	case Status::NoNext:
	case Status::SetNull:
		{
			m_eStatus = Status::EndOfData;
			return false;
		}
	case Status::EndOfData:
	default:
		{
			return false;
		}
	}
}

// FUNCTION public
//	Iterator::Impl::EmptyNullImpl::reset -- 
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
Impl::EmptyNullImpl::
reset(Interface::IProgram& cProgram_)
{
	resetOperand(cProgram_);
	resetBase(cProgram_);
	m_eStatus = Status::Empty;
}

// FUNCTION public
//	Iterator::Impl::EmptyNullImpl::finish -- 
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
Impl::EmptyNullImpl::
finish(Interface::IProgram& cProgram_)
{
	finishOperand(cProgram_);
	finishBase(cProgram_);
	m_eStatus = Status::Empty;
}

// FUNCTION public
//	Iterator::Impl::EmptyNullImpl::setWasLast -- 
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
Impl::EmptyNullImpl::
setWasLast(Interface::IProgram& cProgram_)
{
	m_eStatus = Status::NoNext;
}

// FUNCTION public
//	Iterator::Impl::EmptyNullImpl::getClassID -- 
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
Impl::EmptyNullImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::EmptyNull);
}

// FUNCTION public
//	Iterator::Impl::EmptyNullImpl::serialize -- 
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
Impl::EmptyNullImpl::
serialize(ModArchive& archiver_)
{
	// serialize super classes
	serializeBase(archiver_);
	serializeOperand(archiver_);
	m_cData.serialize(archiver_);
}

// FUNCTION protected
//	Iterator::Impl::EmptyNullImpl::explainThis -- 
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
Impl::EmptyNullImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
	m_cData.explain(cProgram_, cExplain_);
}

// FUNCTION protected
//	Iterator::Impl::EmptyNullImpl::addInput -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EmptyNullImpl::
addInput(Interface::IProgram& cProgram_,
		 const Action::Argument& cAction_)
{
	Super::addInput(cProgram_,
					cAction_);

	m_cData.setDataID(cAction_.getArgumentID());
}

//////////////////////////////
// Iterator::EmptyNull::

// FUNCTION public
//	Iterator::EmptyNull::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	EmptyNull*
//
// EXCEPTIONS

//static
EmptyNull*
EmptyNull::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::EmptyNullImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::EmptyNull::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	EmptyNull*
//
// EXCEPTIONS

//static
EmptyNull*
EmptyNull::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::EmptyNull);
	return new Impl::EmptyNullImpl;
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
