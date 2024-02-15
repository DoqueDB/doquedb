// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/Expand.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DExecution::Iterator";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DExecution/Iterator/Expand.h"
#include "DExecution/Iterator/Class.h"
#include "DExecution/Action/ServerAccess.h"
#include "DExecution/Action/Fulltext.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/Message.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Action/Collection.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/ForwordingIterator.h"
#include "Execution/Iterator/Filter.h"
#include "Execution/Iterator/Input.h"
#include "Execution/Iterator/Monadic.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

#include "Os/Timer.h"

#include "Schema/Cascade.h"
#include "Schema/Database.h"

_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_ITERATOR_BEGIN

namespace
{
	const char* const _pszExplainName = " expand ";
}

namespace Impl
{
	// CLASS local
	//	Iterator::ExpandImpl::Expand -- 
	//	関連文書の特徴語を抽出する。
	//	
	//
	// NOTES

	class ExpandImpl
		: public Execution::Iterator::ForwordingIterator<Iterator::Expand>
	{
	public:
		typedef Execution::Iterator::ForwordingIterator<Iterator::Expand> Super;
		
		typedef ExpandImpl This;

		// constructor
		ExpandImpl()
			: Super(),
			  m_cFulltext(),
			  m_cDocNumData(),
			  m_cAvgLengthData()
		{}
		
		ExpandImpl(Execution::Interface::IIterator* pIterator_,
				   int iFulltextID_,
				   int iDocNumDataID_,
				   int iAvgLengthID_)
			: Super(pIterator_),
			  m_cFulltext(iFulltextID_),
			  m_cDocNumData(iDocNumDataID_),
			  m_cAvgLengthData(iAvgLengthID_),
			  m_cTimer()
		{}
		

		// destructor
		virtual ~ExpandImpl() {}

	///////////////////////////
	// Iterator::Expand::

	///////////////////////////
	//Execution::Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Execution::Interface::IProgram& cProgram_);
		virtual void terminate(Execution::Interface::IProgram& cProgram_);
		virtual Execution::Action::Status::Value startUp(Execution::Interface::IProgram& cProgram_);
		virtual void finish(Execution::Interface::IProgram& cProgram_);
	//	virtual bool next(Execution::Interface::IProgram& cProgram_);
	//	virtual void reset(Execution::Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Execution::Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Execution::Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Execution::Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Execution::Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:


	//////////////////////////////
	// Execution::Iterator::Base::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);



		// virtual void addInput(Execution::Interface::IProgram& cProgram_,
		//					  const Execution::Action::Argument& cAction_);		


		
	private:
		Os::Timer m_cTimer;
		Action::FulltextHolder m_cFulltext;
		// 総文書数
		Execution::Action::UnsignedIntegerDataHolder m_cDocNumData;
		Execution::Action::UnsignedIntegerDataHolder m_cAvgLengthData;

	};


}

/////////////////////////////////////////////
// DExecution::Iterator::Impl::ExpandImpl

// FUNCTION public
//	Iterator::Impl::ExpandImpl::initialize -- initialize
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExpandImpl::
initialize(Execution::Interface::IProgram& cProgram_)
{
	Super::initialize(cProgram_);
	if (!m_cFulltext.isInitialized()) {
		m_cFulltext.initialize(cProgram_);
		m_cDocNumData.initialize(cProgram_);
		m_cAvgLengthData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Iterator::Impl::ExpandImpl::terminate -- terminate
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExpandImpl::
terminate(Execution::Interface::IProgram& cProgram_)
{
	Super::terminate(cProgram_);
	if (m_cFulltext.isInitialized()) {
		m_cFulltext.terminate(cProgram_);
		m_cDocNumData.terminate(cProgram_);
		m_cAvgLengthData.terminate(cProgram_);
	}
}

// FUNCTION public
//	Iterator::Impl::ExpandImpl::startUp -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Execution::Action::Status::Value
//
// EXCEPTIONS

//virtual
Execution::Action::Status::Value
Impl::ExpandImpl::
startUp(Execution::Interface::IProgram& cProgram_)
{
#ifndef NO_TRACE 
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Time)) {
		m_cTimer.reset();
		m_cTimer.start();
	}
#endif
	
	Execution::Action::Status::Value eResult = Super::startUp(cProgram_);
	m_cFulltext->setCollectionSize(m_cDocNumData->getValue());
	m_cFulltext->setAvgLength(m_cAvgLengthData->getValue());
	
#ifndef NO_TRACE	
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Time)) {
		m_cTimer.end();
		_OPT_EXECUTION_MESSAGE
				<< "Fulltext Time(calculate avelage char length): "
				<< m_cTimer.get() << " ms"
				<< ModEndl;
		m_cTimer.reset();
		m_cTimer.start();
	}
#endif
	
	return eResult;
}

// FUNCTION public
//	Iterator::Impl::ExpandImpl::finish --
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExpandImpl::
finish(Execution::Interface::IProgram& cProgram_)
{
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Time)) {
		m_cTimer.end();
		_OPT_EXECUTION_MESSAGE
				<< "Fulltext Time(couting word DF): "
				<< m_cTimer.get() << " ms"
				<< ModEndl;
		m_cTimer.reset();
	}
	Super::finish(cProgram_);
}


// FUNCTION public
//	Iterator::Impl::ExpandImpl::getClassID -- 
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
Impl::ExpandImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Expand);
}

// FUNCTION public
//	Iterator::Impl::ExpandImpl::serialize -- 
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
Impl::ExpandImpl::
serialize(ModArchive& archiver_)
{
	Super::serialize(archiver_);
	m_cFulltext.serialize(archiver_);
	m_cDocNumData.serialize(archiver_);
	m_cAvgLengthData.serialize(archiver_);
}



// FUNCTION protected
//	Iterator::Impl::ExpandImpl::explainThis -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExpandImpl::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName);
	cExplain_.put(" from ");
	Super::explain(pEnvironment_,
				   cProgram_,
				   cExplain_);
	m_cFulltext.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.pushIndent().newLine(true);
	cExplain_.put(" using count  {");
	m_cDocNumData.explain(cProgram_,
						  cExplain_);
	
	cExplain_.put("} using avg length  {");
	m_cAvgLengthData.explain(cProgram_,
							 cExplain_);
	cExplain_.put("}");
	cExplain_.popIndent();
}




////////////////////////////////////////////
// DExecution::Iterator::Expand

// FUNCTION public
//	Iterator::Expand::Execute::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Schema::Cascade* pSchemaCascade_
//	const STRING& cstrSQL_
//	
// RETURN
//	Execution::Interface::IIterator* 
//
// EXCEPTIONS

//static
Expand*
Expand::
create(Execution::Interface::IProgram& cProgram_,
	   int iCollectionID_,
	   int iFulltextID_,
	   int iDocNumDataID_,
	   int iAvgLengthID_,
	   bool bSimple_)
	   
{

	Execution::Interface::IIterator* pIterator;
	if (bSimple_)
		pIterator =	Execution::Iterator::Input::create(cProgram_, iCollectionID_);
	else
		pIterator = Execution::Iterator::Filter::create(cProgram_, iCollectionID_);
	
	AUTOPOINTER<This> pExpand = new Impl::ExpandImpl(pIterator,
													 iFulltextID_,	
													 iDocNumDataID_,
													 iAvgLengthID_);
	
	pExpand->registerToProgram(cProgram_);
	return pExpand.release();
}

//////////////////////////////////////
// DExecution::Iterator::Expand

// FUNCTION public
//	Iterator::Expand::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Expand*
//
// EXCEPTIONS

//static
Expand*
Expand::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Expand:
		{
			return new Impl::ExpandImpl;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_DEXECUTION_ITERATOR_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
