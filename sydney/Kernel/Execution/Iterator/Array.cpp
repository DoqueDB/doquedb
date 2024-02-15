// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/Array.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Iterator/Array.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Iterator/Class.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "array scan";
}

namespace Impl
{
	// CLASS local
	//	Iterator::Impl::ArrayImpl -- array iterator
	//
	// NOTES

	class ArrayImpl
		: public Iterator::Array
	{
	public:
		typedef Iterator::Array Super;
		typedef ArrayImpl This;

		// constructor
		ArrayImpl()
			: Super(),
			  m_cData(),
			  m_cOutData(),
			  m_pArrayData(0),
			  m_iCursor(-1)
		{}
		// destructor
		~ArrayImpl() {}

	///////////////////////////
	//Interface::Array::

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
		virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();
	//	virtual void addLocker(Interface::IProgram& cProgram_,
	//						   const Action::LockerArgument& cArgument_);
	//	virtual int getLocker(Interface::IProgram& cProgram_);
		virtual int getOutData(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////
	//Iterator::Base::
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);

		virtual void addInData(Interface::IProgram& cProgram_,
							   const Action::Argument& cAction_);
		virtual void addOutData(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);

	private:
		// variable holder for array data
		Action::DataHolder m_cData;
		Action::DataHolder m_cOutData;

		// used in run-time
		const Common::DataArrayData* m_pArrayData;
		int m_iCursor;
	};
}

/////////////////////////////////////////////
// Execution::Iterator::Impl::ArrayImpl

// FUNCTION public
//	Iterator::Impl::ArrayImpl::initialize -- initialize
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
Impl::ArrayImpl::
initialize(Interface::IProgram& cProgram_)
{
	// obtain result data instance from variable
	if (m_cData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Iterator::Impl::ArrayImpl::terminate -- terminate
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
Impl::ArrayImpl::
terminate(Interface::IProgram& cProgram_)
{
	// clear objects
	terminateBase(cProgram_);
	m_cOutData.terminate(cProgram_);
	m_cData.terminate(cProgram_);
	m_pArrayData = 0;
	m_iCursor = -1;
}

// FUNCTION public
//	Iterator::Impl::ArrayImpl::next -- go to next tuple
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
Impl::ArrayImpl::
next(Interface::IProgram& cProgram_)
{
	if (hasNext()) {
		if (m_iCursor++ < 0) {
			// get data at first scanning
			if (m_pArrayData != m_cData.getData()) {
				if (m_cData->getType() != Common::DataType::Array
					|| m_cData->getElementType() != Common::DataType::Data) {
					_SYDNEY_THROW0(Exception::Unexpected);
				}
				m_pArrayData = _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, m_cData.getData());
			}
			if (m_pArrayData->isNull()) {
				return setHasNext(setHasData(false));
			}
			m_iCursor = 0;
		}
		if (m_iCursor < m_pArrayData->getCount()) {
			m_cOutData->assign(m_pArrayData->getElement(m_iCursor).get());
			setHasData(true);
		} else {
			setHasNext(setHasData(false));
		}

#ifndef NO_TRACE
		if (hasData()
			&& _OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "Array "
				<< " next = "
				<< m_cOutData->toString() << ModEndl;
		}
#endif
	}
	return hasNext();
}

// FUNCTION public
//	Iterator::Impl::ArrayImpl::reset -- 
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
Impl::ArrayImpl::
reset(Interface::IProgram& cProgram_)
{
	resetBase(cProgram_);
	m_iCursor = -1;
}

// FUNCTION public
//	Iterator::Impl::ArrayImpl::finish -- 
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
Impl::ArrayImpl::
finish(Interface::IProgram& cProgram_)
{
	finishBase(cProgram_);
	m_iCursor = -1;
}

// FUNCTION public
//	Iterator::Impl::ArrayImpl::getOutData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::ArrayImpl::
getOutData(Interface::IProgram& cProgram_)
{
	return m_cOutData.getDataID();
}

// FUNCTION public
//	Iterator::Impl::ArrayImpl::getClassID -- 
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
Impl::ArrayImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Array);
}

// FUNCTION public
//	Iterator::Impl::ArrayImpl::serialize -- 
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
Impl::ArrayImpl::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cData.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

// FUNCTION protected
//	Iterator::Impl::ArrayImpl::explainThis -- 
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
Impl::ArrayImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" on ");
		m_cData.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
}

// FUNCTION protected
//	Iterator::Impl::ArrayImpl::addInData -- 
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
Impl::ArrayImpl::
addInData(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	; _SYDNEY_ASSERT(m_cData.isValid() == false);

	// variable for result data is set through addAction(InData)
	// (this switch-case can be implemented in Base class....)
	m_cData.setDataID(cAction_.getInstanceID());
}

// FUNCTION protected
//	Iterator::Impl::ArrayImpl::addOutData -- 
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
Impl::ArrayImpl::
addOutData(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	; _SYDNEY_ASSERT(m_cOutData.isValid() == false);

	// variable for result data is set through addAction(OutData)
	// (this switch-case can be implemented in Base class....)
	m_cOutData.setDataID(cAction_.getInstanceID());
}

////////////////////////////////
// Execution::Iterator::Array

// FUNCTION public
//	Iterator::Array::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Array*
//
// EXCEPTIONS

//static
Array*
Array::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::ArrayImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::Array::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Array*
//
// EXCEPTIONS

//static
Array*
Array::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Array:
		{
			return new Impl::ArrayImpl;
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
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
