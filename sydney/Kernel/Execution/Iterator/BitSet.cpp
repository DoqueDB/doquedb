// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/BitSet.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Iterator/BitSet.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Iterator/Class.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Lock/Mode.h"

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
	const char* const _pszOperatorName = "bitset scan";
}

namespace Impl
{
	// CLASS local
	//	Iterator::Impl::BitSetImpl -- bitSet iterator
	//
	// NOTES

	class BitSetImpl
		: public Iterator::BitSet
	{
	public:
		typedef Iterator::BitSet Super;
		typedef BitSetImpl This;

		// constructor
		BitSetImpl()
			: Super(),
			  m_cBitSet(),
			  m_cRowID(),
			  m_cIterator(),
			  m_bOpened(false)
		{}
		BitSetImpl(int iBitSetID_)
			: Super(),
			  m_cBitSet(iBitSetID_),
			  m_cRowID(),
			  m_cIterator(),
			  m_bOpened(false)
		{}
		// destructor
		~BitSetImpl() {}

	///////////////////////////
	//Interface::BitSet::

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

		virtual void addOutData(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);

	private:
		void open(Interface::IProgram& cProgram_);

		// variable holder for bitset data
		Action::BitSetHolder m_cBitSet;

		// variable holder for result data
		Action::RowIDHolder m_cRowID;

		// iterator over the bitset object
		Common::BitSet::ConstIterator m_cIterator;

		// runtime flag
		bool m_bOpened;
	};
}

/////////////////////////////////////////////
// Execution::Iterator::Impl::BitSetImpl

// FUNCTION public
//	Iterator::Impl::BitSetImpl::initialize -- initialize
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
Impl::BitSetImpl::
initialize(Interface::IProgram& cProgram_)
{
	// obtain result data instance from variable
	if (m_cRowID.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cRowID.initialize(cProgram_);
		m_cBitSet.initialize(cProgram_);
	}
}

// FUNCTION public
//	Iterator::Impl::BitSetImpl::terminate -- terminate
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
Impl::BitSetImpl::
terminate(Interface::IProgram& cProgram_)
{
	// clear objects
	terminateBase(cProgram_);
	m_cRowID.terminate(cProgram_);
	m_cBitSet.terminate(cProgram_);
	m_bOpened = false;
}

// FUNCTION public
//	Iterator::Impl::BitSetImpl::next -- go to next tuple
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
Impl::BitSetImpl::
next(Interface::IProgram& cProgram_)
{
	// BitSet object is created when next is called first.
	open(cProgram_);
	if (hasNext()) {
		if (m_cIterator != m_cBitSet->end()) {
			// assign current value to the result data
			m_cRowID->setValue(*m_cIterator);
			// go to next entry
			++m_cIterator;
			setHasData(true);
		} else {
			setHasNext(setHasData(false));
		}

#ifndef NO_TRACE
		if (hasData()
			&& _OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "BitSet "
				<< " next = "
				<< m_cRowID->toString() << ModEndl;
		}
#endif
	}
	return hasNext();
}

// FUNCTION public
//	Iterator::Impl::BitSetImpl::reset -- 
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
Impl::BitSetImpl::
reset(Interface::IProgram& cProgram_)
{
	if (m_cBitSet.isInitialized()) {
		open(cProgram_);
		if (m_cBitSet->any()) {
			m_cIterator = m_cBitSet->begin();
			setHasNext(setHasData(true));
		} else {
			setHasNext(setHasData(false));
		}
		resetAction(cProgram_);
	}
}

// FUNCTION public
//	Iterator::Impl::BitSetImpl::finish -- 
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
Impl::BitSetImpl::
finish(Interface::IProgram& cProgram_)
{
	m_bOpened = false;
	finishBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::BitSetImpl::getOutData -- 
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
Impl::BitSetImpl::
getOutData(Interface::IProgram& cProgram_)
{
	return m_cRowID.getDataID();
}

// FUNCTION public
//	Iterator::Impl::BitSetImpl::getClassID -- 
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
Impl::BitSetImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::BitSet);
}

// FUNCTION public
//	Iterator::Impl::BitSetImpl::serialize -- 
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
Impl::BitSetImpl::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cBitSet.serialize(archiver_);
	m_cRowID.serialize(archiver_);
}

// FUNCTION protected
//	Iterator::Impl::BitSetImpl::explainThis -- 
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
Impl::BitSetImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" on ");
		m_cBitSet.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cRowID.explain(cProgram_, cExplain_);
	}
}

// FUNCTION protected
//	Iterator::Impl::BitSetImpl::addOutData -- 
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
Impl::BitSetImpl::
addOutData(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	; _SYDNEY_ASSERT(m_cRowID.isValid() == false);

	// variable for result data is set through addAction(OutData)
	// (this switch-case can be implemented in Base class....)
	m_cRowID.setDataID(cAction_.getInstanceID());
}

// FUNCTION private
//	Iterator::BitSetImpl::open -- 
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
Impl::BitSetImpl::
open(Interface::IProgram& cProgram_)
{
	if (!m_bOpened) {
		if (m_cBitSet->any()) {
			m_cIterator = m_cBitSet->begin();
			setHasNext(setHasData(true));
		} else {
			setHasNext(setHasData(false));
		}
		m_bOpened = true;
	}
}

////////////////////////////////
// Execution::Iterator::BitSet

// FUNCTION public
//	Iterator::BitSet::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iBitSetID_
//	
// RETURN
//	BitSet*
//
// EXCEPTIONS

//static
BitSet*
BitSet::
create(Interface::IProgram& cProgram_,
	   int iBitSetID_)
{
	AUTOPOINTER<This> pResult = new Impl::BitSetImpl(iBitSetID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::BitSet::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	BitSet*
//
// EXCEPTIONS

//static
BitSet*
BitSet::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::BitSet:
		{
			return new Impl::BitSetImpl;
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
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
