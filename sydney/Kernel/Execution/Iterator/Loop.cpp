// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/Loop.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Execution/Iterator/Loop.h"
#include "Execution/Iterator/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/Unexpected.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

namespace
{
	struct _Type
	{
		enum {
			ForEver,
			Once,
			ValueNum
		};
	};

	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName[] = {
		"loop",
		"loop once"
	};
}

namespace LoopImpl
{
	// CLASS local
	//	Execution::Iterator::LoopImpl::ForEver -- implementation class of Loop
	//
	// NOTES
	class ForEver
		: public Iterator::Loop
	{
	public:
		typedef ForEver This;
		typedef Iterator::Loop Super;

		ForEver()
			: Super()
		{}
		virtual ~ForEver()
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
		virtual int getOutData(Interface::IProgram& cProgram_);

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
		virtual void addOutData(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);
	private:
		Action::DataHolder m_cData;
	};

	// CLASS local
	//	Execution::Iterator::LoopImpl::Once -- implementation class of Loop
	//
	// NOTES
	class Once
		: public ForEver
	{
	public:
		typedef Once This;
		typedef ForEver Super;

		Once()
			: Super()
		{}
		~Once()
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
	//////////////////////////////
	// Iterator::Base::
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
	private:
	};
}

/////////////////////////////////////////////
// Execution::Iterator::LoopImpl::ForEver

// FUNCTION public
//	Iterator::LoopImpl::ForEver::next -- go to next tuple
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
LoopImpl::ForEver::
next(Interface::IProgram& cProgram_)
{
	return setHasData(hasNext());
}

// FUNCTION public
//	Iterator::LoopImpl::ForEver::getOutData -- 
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
LoopImpl::ForEver::
getOutData(Interface::IProgram& cProgram_)
{
	return m_cData.getDataID();
}

// FUNCTION public
//	Iterator::LoopImpl::ForEver::getClassID -- 
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
LoopImpl::ForEver::
getClassID() const
{
	return Class::getClassID(Class::Category::LoopForEver);
}

// FUNCTION public
//	Iterator::LoopImpl::ForEver::serialize -- 
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
LoopImpl::ForEver::
serialize(ModArchive& archiver_)
{
	// serialize super classes
	serializeBase(archiver_);
}

// FUNCTION protected
//	Iterator::LoopImpl::ForEver::explainThis -- 
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
LoopImpl::ForEver::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::ForEver]);
}

// FUNCTION protected
//	Iterator::LoopImpl::ForEver::addOutData -- 
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
LoopImpl::ForEver::
addOutData(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	m_cData.setDataID(cAction_.getInstanceID());
}

/////////////////////////////////////////////
// Execution::Iterator::LoopImpl::Once

// FUNCTION public
//	Iterator::LoopImpl::Once::next -- go to next tuple
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
LoopImpl::Once::
next(Interface::IProgram& cProgram_)
{
	if (Super::next(cProgram_)) {
		setWasLast(cProgram_);
		return true;
	}
	return false;
}

// FUNCTION public
//	Iterator::LoopImpl::Once::getClassID -- 
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
LoopImpl::Once::
getClassID() const
{
	return Class::getClassID(Class::Category::LoopOnce);
}

// FUNCTION protected
//	Iterator::LoopImpl::Once::explainThis -- 
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
LoopImpl::Once::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::Once]);
}

//////////////////////////////
// Iterator::Loop::ForEver::

// FUNCTION public
//	Iterator::Loop::ForEver::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Loop*
//
// EXCEPTIONS

//static
Loop*
Loop::ForEver::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new LoopImpl::ForEver;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

//////////////////////////////
// Iterator::Loop::Once::

// FUNCTION public
//	Iterator::Loop::Once::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Loop*
//
// EXCEPTIONS

//static
Loop*
Loop::Once::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new LoopImpl::Once;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

//////////////////////////////
// Iterator::Loop::

// FUNCTION public
//	Iterator::Loop::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Loop*
//
// EXCEPTIONS

//static
Loop*
Loop::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::LoopForEver:
		{
			return new LoopImpl::ForEver;
		}
	case Class::Category::LoopOnce:
		{
			return new LoopImpl::Once;
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
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
