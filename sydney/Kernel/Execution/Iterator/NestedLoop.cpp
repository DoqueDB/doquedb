// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/NestedLoop.cpp --
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

#include "Execution/Iterator/NestedLoop.h"
#include "Execution/Iterator/Class.h"
#include "Execution/Iterator/Nadic.h"
#include "Execution/Iterator/OperandElement.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

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
	const char* const _pszOperatorName = "nested loop join";
}

namespace Impl
{
	// CLASS local
	//	Execution::Iterator::Impl::NestedLoopImpl -- implementation class of NestedLoop
	//
	// NOTES
	class NestedLoopImpl
		: public Nadic<Iterator::NestedLoop>
	{
	public:
		typedef NestedLoopImpl This;
		typedef Nadic<Iterator::NestedLoop> Super;

		NestedLoopImpl()
			: Super(),
			  m_iDepth(0)
		{}
		~NestedLoopImpl()
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
		virtual void finish(Interface::IProgram& cProgram_);
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
		int m_iDepth; // depth of current loop
	};
} // namespace Impl

/////////////////////////////////////////////
// Execution::Iterator::Impl::NestedLoopImpl

// FUNCTION public
//	Iterator::Impl::NestedLoopImpl::next -- go to next tuple
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
Impl::NestedLoopImpl::
next(Interface::IProgram& cProgram_)
{
	if (hasNext() == false) {
		return setHasData(false);
	}
	while (true) {
		while (getElement(m_iDepth).next(cProgram_) == true) {
			if (m_iDepth >= getOperandSize() - 1) {
				return true;
			}
			++m_iDepth;
		}
		if (m_iDepth > 0) {
			getElement(m_iDepth).reset(cProgram_);
			--m_iDepth;
			continue;
		}
		// all operands reached end
		break;
	}
	return setHasData(false);
}

// FUNCTION public
//	Iterator::Impl::NestedLoopImpl::reset -- 
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
Impl::NestedLoopImpl::
reset(Interface::IProgram& cProgram_)
{
	if (m_iDepth >= getOperandSize()) {
		m_iDepth = getOperandSize() - 1;
	}
	for (int i = m_iDepth; i >= 0; --i) {
		getElement(i).reset(cProgram_);
	}
	m_iDepth = 0;
	resetBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::NestedLoopImpl::finish -- 
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
Impl::NestedLoopImpl::
finish(Interface::IProgram& cProgram_)
{
	Super::finish(cProgram_);
	m_iDepth = 0;
}

// FUNCTION public
//	Iterator::Impl::NestedLoopImpl::getClassID -- 
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
Impl::NestedLoopImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::NestedLoop);
}

// FUNCTION protected
//	Iterator::Impl::NestedLoopImpl::explainThis -- 
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
Impl::NestedLoopImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
}

//////////////////////////////
// Iterator::NestedLoop::

// FUNCTION public
//	Iterator::NestedLoop::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	NestedLoop*
//
// EXCEPTIONS

//static
NestedLoop*
NestedLoop::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::NestedLoopImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::NestedLoop::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	NestedLoop*
//
// EXCEPTIONS

//static
NestedLoop*
NestedLoop::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::NestedLoop);
	return new Impl::NestedLoopImpl;
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
