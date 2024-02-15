// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/CascadeInput.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Execution/Iterator/CascadeInput.h"
#include "Execution/Iterator/Class.h"
#include "Execution/Iterator/Nadic.h"
#include "Execution/Iterator/OperandElement.h"
#include "Execution/Interface/ICollection.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Exception/NotSupported.h"

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
	const char* const _pszOperatorName = "cascade input";
}

namespace Impl
{
	// CLASS local
	//	Execution::Iterator::Impl::CascadeInputImpl -- implementation class of CascadeInput
	//
	// NOTES
	class CascadeInputImpl
		: public Nadic<Iterator::CascadeInput>
	{
	public:
		typedef CascadeInputImpl This;
		typedef Nadic<Iterator::CascadeInput> Super;

		CascadeInputImpl()
			: Super(),
			  m_iTarget(0)
		{}
		~CascadeInputImpl()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);
	//	virtual void startUp(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual bool doAction(Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////////
	// Iterator::Base::
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);

	private:
		int m_iTarget;
	};
} // namespace Impl

/////////////////////////////////////////////
// Execution::Iterator::Impl::CascadeInputImpl

// FUNCTION public
//	Iterator::Impl::CascadeInputImpl::next -- go to next tuple
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
Impl::CascadeInputImpl::
next(Interface::IProgram& cProgram_)
{
	if (m_iTarget >= getOperandSize()) {
		return setHasData(false);
	}

	while (getElement(m_iTarget).next(cProgram_) == false) {
		if (++m_iTarget >= getOperandSize()) {
			return setHasData(false);
		}
	}
	setResult(m_iTarget);
	return true;
}

// FUNCTION public
//	Iterator::Impl::CascadeInputImpl::reset -- 
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
Impl::CascadeInputImpl::
reset(Interface::IProgram& cProgram_)
{
	if (m_iTarget >= getOperandSize()) {
		m_iTarget = getOperandSize() - 1;
	}
	for (int i = m_iTarget; i >= 0; --i) {
		getElement(i).reset(cProgram_);
	}
	m_iTarget = 0;
	resetBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::CascadeInputImpl::finish -- 
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
Impl::CascadeInputImpl::
finish(Interface::IProgram& cProgram_)
{
	finishOperand(cProgram_);
	finishBase(cProgram_);
	m_iTarget = 0;
}

// FUNCTION public
//	Iterator::Impl::CascadeInputImpl::setWasLast -- 
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
Impl::CascadeInputImpl::
setWasLast(Interface::IProgram& cProgram_)
{
	m_iTarget = getOperandSize();
}

// FUNCTION public
//	Iterator::Impl::CascadeInputImpl::getClassID -- 
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
Impl::CascadeInputImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::CascadeInput);
}

// FUNCTION protected
//	Iterator::Impl::CascadeInputImpl::explainThis -- 
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
Impl::CascadeInputImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
}

//////////////////////////////
// Iterator::CascadeInput::

// FUNCTION public
//	Iterator::CascadeInput::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	CascadeInput*
//
// EXCEPTIONS

//static
CascadeInput*
CascadeInput::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::CascadeInputImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::CascadeInput::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	CascadeInput*
//
// EXCEPTIONS

//static
CascadeInput*
CascadeInput::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::CascadeInput);
	return new Impl::CascadeInputImpl;
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
