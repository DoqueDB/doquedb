// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Output.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Operator";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Execution/Operator/Output.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/Collection.h"
#include "Execution/Interface/ICollection.h"
#include "Execution/Interface/IProgram.h"

#include "Exception/Cancel.h"

#include "Common/Assert.h"
#include "Common/Data.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "output";
}

namespace Impl
{

	// CLASS local
	//	Execution::Operator::Impl::OutputImpl -- implementation class of Output
	//
	// NOTES
	class OutputImpl
		: public Operator::Output
	{
	public:
		typedef OutputImpl This;
		typedef Operator::Output Super;

		OutputImpl()
			: Super(),
			  m_cCollection()
		{}
		OutputImpl(int iCollectionID_,
				   int iDataID_)
			: Super(),
			  m_cCollection(iCollectionID_, iDataID_)
		{}
		~OutputImpl()
		{}

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		// collection to which result data is output
		Action::Collection m_cCollection;
	};

	
	// CLASS local
	//	Execution::Operator::Impl::ArrayOutputImpl -- implementation class of Output
	//
	// NOTES
	class ArrayOutputImpl
		: public Operator::Output
	{
	public:
		typedef ArrayOutputImpl This;
		typedef Operator::Output Super;

		ArrayOutputImpl()
			: Super(),
			  m_cInData(),
			  m_cOutData(),
			  m_vecData()
		{}
		
		ArrayOutputImpl(int iOutDataID_,
				   int iDataID_)
			: Super(),
			  m_cInData(iDataID_),
			  m_cOutData(iOutDataID_),
			  m_vecData()
		{}
		
		virtual ~ArrayOutputImpl()
		{ m_vecData.clear();}

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		Action::DataHolder m_cInData;
		Action::ArrayDataHolder m_cOutData;
		Common::LargeVector<Common::Data::Pointer> m_vecData;
		
	};
}

/////////////////////////////////////////////
// Execution::Operator::Impl::OutputImpl

// FUNCTION public
//	Operator::Impl::OutputImpl::explain -- 
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
Impl::OutputImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperatorName).put(" to ");
	cExplain_.popNoNewLine();
	cExplain_.pushIndent();
	m_cCollection.explain(pEnvironment_, cProgram_, cExplain_);
	m_cCollection.explainPutData(cProgram_, cExplain_);
	cExplain_.popIndent();
}

// FUNCTION public
//	Operator::Impl::OutputImpl::initialize -- 
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
Impl::OutputImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (!m_cCollection.isInitialized()) {
		m_cCollection.initialize(cProgram_);
		m_cCollection.preparePutInterface();
	}
}

// FUNCTION public
//	Operator::Impl::OutputImpl::terminate -- 
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
Impl::OutputImpl::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cCollection.isInitialized()) {
		m_cCollection.terminate(cProgram_);
	}
}

// FUNCTION public
//	Operator::Impl::OutputImpl::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
Impl::OutputImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		try {
			// write data to the collection
			m_cCollection.put(cProgram_);
		} catch (Exception::Cancel& e){
			return Action::Status::Break;
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::Impl::OutputImpl::finish -- 
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
Impl::OutputImpl::
finish(Interface::IProgram& cProgram_)
{
	// execute finish method(eg. sorting) if implemented
	m_cCollection.finish(cProgram_);
}

// FUNCTION public
//	Operator::Impl::OutputImpl::reset -- 
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
Impl::OutputImpl::
reset(Interface::IProgram& cProgram_)
{
	// flush output interface
	m_cCollection.flush();
}

// FUNCTION public
//	Operator::Impl::OutputImpl::getClassID -- 
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
Impl::OutputImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Output);
}

// FUNCTION public
//	Operator::Impl::OutputImpl::serialize -- 
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
Impl::OutputImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cCollection.serialize(archiver_);
}


/////////////////////////////////////////////
// Execution::Operator::Impl::ArrayOutputImpl

// FUNCTION public
//	Operator::Impl::ArrayOutputImpl::explain -- 
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
Impl::ArrayOutputImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	m_cInData.explain(cProgram_,
					  cExplain_);
	cExplain_.put(_pszOperatorName).put(" to ");
	cExplain_.popNoNewLine();
	cExplain_.pushIndent();
	m_cOutData.explain(cProgram_,
					   cExplain_);	
	cExplain_.popIndent();
}

// FUNCTION public
//	Operator::Impl::ArrayOutputImpl::initialize -- 
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
Impl::ArrayOutputImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (!m_cInData.isInitialized()) {
		m_cInData.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);		
	}
}

// FUNCTION public
//	Operator::Impl::ArrayOutputImpl::terminate -- 
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
Impl::ArrayOutputImpl::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cInData.isInitialized()) {
		m_cInData.terminate(cProgram_);
		m_cOutData.terminate(cProgram_);
	}
}

// FUNCTION public
//	Operator::Impl::ArrayOutputImpl::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
Impl::ArrayOutputImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		try {
			Common::Data::Pointer pData = m_cInData->copy();
			m_vecData.pushBack(pData);
			m_cOutData->pushBack(pData);
		} catch (Exception::Cancel& e){
			return Action::Status::Break;
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::Impl::ArrayOutputImpl::finish -- 
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
Impl::ArrayOutputImpl::
finish(Interface::IProgram& cProgram_)
{
	m_vecData.clear();
}

// FUNCTION public
//	Operator::Impl::ArrayOutputImpl::reset -- 
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
Impl::ArrayOutputImpl::
reset(Interface::IProgram& cProgram_)
{
	m_vecData.clear();
}

// FUNCTION public
//	Operator::Impl::ArrayOutputImpl::getClassID -- 
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
Impl::ArrayOutputImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::ArrayOutput);
}

// FUNCTION public
//	Operator::Impl::ArrayOutputImpl::serialize -- 
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
Impl::ArrayOutputImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cInData.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}


//////////////////////////////
// Operator::Output::

// FUNCTION public
//	Operator::Output::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iCollectionID_
//	int iDataID_
//	
// RETURN
//	Output*
//
// EXCEPTIONS

//static
Output*
Output::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iCollectionID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::OutputImpl(iCollectionID_,
													 iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::Output::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iCollectionID_
//	int iDataID_
//	
// RETURN
//	Output*
//
// EXCEPTIONS

//static
Output*
Output::Array::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iOutDataID_,
	   int iInDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::ArrayOutputImpl(iOutDataID_,
														  iInDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::Output::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Output*
//
// EXCEPTIONS

//static
Output*
Output::
getInstance(int iCategory_)
{

	switch (iCategory_) {
	case Class::Category::Output:
	{
		return new Impl::OutputImpl;
	}
	case Class::Category::ArrayOutput:
	{
		return new Impl::ArrayOutputImpl;
	}
	}
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
