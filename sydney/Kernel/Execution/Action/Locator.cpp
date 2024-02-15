// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/Locator.cpp --
// 
// Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/Locator.h"
#include "Execution/Action/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/Locator.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

namespace Impl
{
	////////////////////////////////////
	// CLASS
	//	Action::Impl::LocatorImpl --
	//
	// NOTES
	class LocatorImpl
		: public Locator
	{
	public:
		typedef Locator Super;
		typedef LocatorImpl This;

		// constructor
		LocatorImpl()
			: Super(),
			  m_pLocator(0),
			  m_pLogicalFile(0)
		{}
		// destructor
		~LocatorImpl() {}

	/////////////////////
	// Action::Locator::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual bool getLocator(LogicalFile::AutoLogicalFile& cLogicalFile_,
								Common::DataArrayData* pKey_);
		virtual void length(Common::UnsignedIntegerData* pResult_);
		virtual void get(const Common::UnsignedIntegerData* pStart_,
						 const Common::UnsignedIntegerData* pLength_,
						 Common::Data* pResult_);
		virtual void append(const Common::Data* pAppendData_);
		virtual void truncate(const Common::UnsignedIntegerData* pTruncateLength_);
		virtual void replace(const Common::Data* pPlaceData_,
							 const Common::UnsignedIntegerData* pStart_,
							 const Common::UnsignedIntegerData* pLength_);
		virtual void clear();
		virtual bool isValid();

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	/////////////////////
	// ModSerializer::
		void serialize(ModArchive& archiver_);

	protected:
	private:
		AUTOPOINTER<LogicalFile::Locator> m_pLocator;
		LogicalFile::AutoLogicalFile* m_pLogicalFile;
	};

} // namespace Impl

///////////////////////////////////////////////
// Execution::Action::Impl::LocatorImpl

// FUNCTION public
//	Action::Impl::LocatorImpl::initialize -- initialize
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

void
Impl::LocatorImpl::
initialize(Interface::IProgram& cProgram_)
{
	// do nothing
}

// FUNCTION public
//	Action::Impl::LocatorImpl::terminate -- terminate
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

void
Impl::LocatorImpl::
terminate(Interface::IProgram& cProgram_)
{
	clear();
}

// FUNCTION public
//	Action::Impl::LocatorImpl::getLocator -- 
//
// NOTES
//
// ARGUMENTS
//	LogicalFile::AutoLogicalFile& cLogicalFile_
//	Common::DataArrayData* pKey_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::LocatorImpl::
getLocator(LogicalFile::AutoLogicalFile& cLogicalFile_,
		   Common::DataArrayData* pKey_)
{
	m_pLocator = cLogicalFile_.getLocator(pKey_);
	if (m_pLocator.get()) {
		m_pLogicalFile = &cLogicalFile_;
	}
	return m_pLocator.get() != 0;
}

// FUNCTION public
//	Action::Impl::LocatorImpl::length -- 
//
// NOTES
//
// ARGUMENTS
//	Common::UnsignedIntegerData* pResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::LocatorImpl::
length(Common::UnsignedIntegerData* pResult_)
{
	if (m_pLocator.get()
		&& m_pLocator->isInvalid() == false) {
		; _SYDNEY_ASSERT(m_pLogicalFile);
		LogicalFile::AutoLogicalFile::AutoUnlatch cAuto =
			m_pLogicalFile->latch();
		m_pLocator->length(pResult_);
	} else {
		pResult_->setNull();
	}
}

// FUNCTION public
//	Action::Impl::LocatorImpl::get -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::UnsignedIntegerData* pStart_
//	const Common::UnsignedIntegerData* pLength_
//	Common::Data* pResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::LocatorImpl::
get(const Common::UnsignedIntegerData* pStart_,
	const Common::UnsignedIntegerData* pLength_,
	Common::Data* pResult_)
{
	if (m_pLocator.get()
		&& m_pLocator->isInvalid() == false) {
		; _SYDNEY_ASSERT(m_pLogicalFile);
		LogicalFile::AutoLogicalFile::AutoUnlatch cAuto =
			m_pLogicalFile->latch();
		if (m_pLocator->get(pStart_, pLength_, pResult_)) {
			return;
		}
	}
	pResult_->setNull();
}

// FUNCTION public
//	Action::Impl::LocatorImpl::append -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pAppendData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::LocatorImpl::
append(const Common::Data* pAppendData_)
{
	if (m_pLocator.get()
		&& m_pLocator->isInvalid() == false) {
		; _SYDNEY_ASSERT(m_pLogicalFile);
		LogicalFile::AutoLogicalFile::AutoUnlatch cAuto =
			m_pLogicalFile->latch();
		m_pLocator->append(pAppendData_);
	}
}

// FUNCTION public
//	Action::Impl::LocatorImpl::truncate -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::UnsignedIntegerData* pTruncateLength_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::LocatorImpl::
truncate(const Common::UnsignedIntegerData* pTruncateLength_)
{
	if (m_pLocator.get()
		&& m_pLocator->isInvalid() == false) {
		; _SYDNEY_ASSERT(m_pLogicalFile);
		LogicalFile::AutoLogicalFile::AutoUnlatch cAuto =
			m_pLogicalFile->latch();
		m_pLocator->truncate(pTruncateLength_);
	}
}

// FUNCTION public
//	Action::Impl::LocatorImpl::replace -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pPlaceData_
//	const Common::UnsignedIntegerData* pStart_
//	const Common::UnsignedIntegerData* pLength_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::LocatorImpl::
replace(const Common::Data* pPlaceData_,
		const Common::UnsignedIntegerData* pStart_,
		const Common::UnsignedIntegerData* pLength_)
{
	if (pLength_) {
		// length is not supported for the locator
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	if (m_pLocator.get()
		&& m_pLocator->isInvalid() == false) {
		; _SYDNEY_ASSERT(m_pLogicalFile);
		LogicalFile::AutoLogicalFile::AutoUnlatch cAuto =
			m_pLogicalFile->latch();
		m_pLocator->replace(pStart_, pPlaceData_);
	}
}

// FUNCTION public
//	Action::Impl::LocatorImpl::clear -- clear locator
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

//virtual
void
Impl::LocatorImpl::
clear()
{
	m_pLocator = AUTOPOINTER<LogicalFile::Locator>();
	m_pLogicalFile = 0;
}

// FUNCTION public
//	Action::Impl::LocatorImpl::isValid -- is locator assigned?
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

//virtual
bool
Impl::LocatorImpl::
isValid()
{
	return m_pLocator.get() != 0;
}

// FUNCTION public
//	Action::Impl::LocatorImpl::getClassID -- 
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
Impl::LocatorImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Locator);
}

// FUNCTION public
//	Action::Impl::LocatorImpl::serialize -- serialize
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
Impl::LocatorImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
}

////////////////////////////////////
// Execution::Action::Locator

// FUNCTION public
//	Action::Locator::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pThis = new Impl::LocatorImpl;
	pThis->registerToProgram(cProgram_);
	return pThis.release();
}

// FUNCTION public
//	Action::Locator::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Locator:
		{
			return new Impl::LocatorImpl;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

// FUNCTION protected
//	Action::Locator::registerToProgram -- register to program
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

void
Locator::
registerToProgram(Interface::IProgram& cProgram_)
{
	// Instance ID is obtained by registerLocator method.
	setID(cProgram_.registerLocator(this));
}

//////////////////////////////////////////
// Execution::Action::LocatorHolder

// FUNCTION public
//	Action::LocatorHolder::initialize -- initialize Locator instance
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

void
LocatorHolder::
initialize(Interface::IProgram& cProgram_)
{
	if (isValid() && !isInitialized()) {
		m_pLocator = cProgram_.getLocator(m_iID);
		if (m_pLocator == 0) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		m_pLocator->initialize(cProgram_);
	}
}

// FUNCTION public
//	Action::LocatorHolder::terminate -- terminate Locator instance
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

void
LocatorHolder::
terminate(Interface::IProgram& cProgram_)
{
	if (m_pLocator) {
		m_pLocator->terminate(cProgram_);
		m_pLocator = 0;
	}
}

// FUNCTION public
//	Action::LocatorHolder::clear -- clear Locator instance
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

void
LocatorHolder::
clear()
{
	if (m_pLocator) {
		m_pLocator->clear();
	}
}

// FUNCTION public
//	Action::LocatorHolder::serialize -- serializer
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
LocatorHolder::
serialize(ModArchive& archiver_)
{
	archiver_(m_iID);
}

// FUNCTION public
//	Action::LocatorHolder::explain -- explain
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

void
LocatorHolder::
explain(Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("locator:#").put(m_iID);
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
