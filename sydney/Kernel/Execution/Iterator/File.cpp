// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/File.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2015, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Iterator/File.h"
#include "Execution/Iterator/Class.h"
#include "Execution/Action/Argument.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Operator/FileOperation.h"
#include "Execution/Operator/Iterate.h"
#include "Execution/Operator/Locker.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

#include "Plan/File/Parameter.h" // for explain

#include "Schema/File.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

namespace
{
	struct _Explain
	{
		enum {
			Sequencial = 0,
			Fetch,
			Search,
			Simple
		};
	};
	const char* const _pszExplainName[] =
	{
		"sequential scan",				// no fetch, no predicate
		"index fetch",					// fetch
		"index scan",					// predicate
		"header fetch",					// simple
	};
}

namespace Impl
{
	// CLASS local
	//	Iterator::Impl::SingleFileImpl -- file iterator for one file
	//
	// NOTES

	class SingleFileImpl
		: public Iterator::File
	{
	public:
		typedef Iterator::File Super;
		typedef SingleFileImpl This;

		// constructor
		SingleFileImpl()
			: Super(),
			  m_cFileAccess(),
			  m_cExplainArgument(),
			  m_cData(),
			  m_cKey(),
			  m_bFetch(false),
			  m_bIsFetched(false),
			  m_cAggregation()
		{}
		SingleFileImpl(int iFileAccessID_,
					   const Opt::ExplainFileArgument& cArgument_)
			: Super(),
			  m_cFileAccess(iFileAccessID_),
			  m_cExplainArgument(cArgument_),
			  m_cData(),
			  m_cKey(),
			  m_bFetch(false),
			  m_bIsFetched(false),
			  m_cAggregation()
		{}
		// destructor
		virtual ~SingleFileImpl() {}

	///////////////////////////
	// Iterator::File::

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();
		virtual int getLocker(Interface::IProgram& cProgram_);
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
		virtual void addInput(Interface::IProgram& cProgram_,
							  const Action::Argument& cAction_);

		virtual void addAggregation(Interface::IProgram& cProgram_,
									const Action::Argument& cAction_);

	private:
		void explainMethod(Opt::Explain& cExplain_);

		// file access utility class
		Action::FileAccessHolder m_cFileAccess;
		// argument for explain
		Opt::ExplainFileArgument m_cExplainArgument;

		// result data
		Action::ArrayDataHolder m_cData;
		// key data
		Action::ArrayDataHolder m_cKey;

		Action::ActionList m_cAggregation;

		bool m_bFetch;					// key is valid?
		bool m_bIsFetched;				// has fetched?
	};
}

/////////////////////////////////////////////
// Execution::Iterator::Impl::SingleFileImpl

// FUNCTION public
//	Iterator::Impl::SingleFileImpl::initialize -- initialize
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
Impl::SingleFileImpl::
initialize(Interface::IProgram& cProgram_)
{
	// convert variable ID to object to which result data is assigned
	if (m_cData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cFileAccess.initialize(cProgram_);
		m_cData.initialize(cProgram_);
		m_cKey.initialize(cProgram_);
		m_bFetch = m_cKey.isInitialized();
		m_cAggregation.initialize(cProgram_);
	}
}

// FUNCTION public
//	Iterator::Impl::SingleFileImpl::terminate -- terminate
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
Impl::SingleFileImpl::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cFileAccess.terminate(cProgram_);
	m_cData.terminate(cProgram_);
	m_cKey.terminate(cProgram_);
	m_cAggregation.terminate(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::SingleFileImpl::finish -- 
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
Impl::SingleFileImpl::
finish(Interface::IProgram& cProgram_)
{
	if (m_cFileAccess.isInitialized()) {
		m_cFileAccess->close();
	}
	m_bIsFetched = false;
	finishBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::SingleFileImpl::next -- go to next tuple
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
Impl::SingleFileImpl::
next(Interface::IProgram& cProgram_)
{
	// open file when next is called at first.
	if (hasNext() == false || m_cFileAccess->open(cProgram_) == false) {
		setHasData(false);
	} else {
		if (m_bFetch && !m_bIsFetched) {
#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
				_OPT_EXECUTION_MESSAGE
					<< "File [" << m_cFileAccess->getName()
					<< "] fetch by "
					<< Opt::Trace::toString(*m_cKey)
					<< ModEndl;
			}
#endif
			if (Opt::IsAny(m_cKey->getValue(),
						   boost::bind(&Common::Data::isNull,
									   boost::bind(&Common::Data::Pointer::get,
												   _1))) == false) {
				// set fetch key when next is called at first after reset.
				m_cFileAccess->fetch(m_cKey.getData());

			} else {
				setHasData(false);
			}
			m_bIsFetched = true;
		}

		// read one tuple from logical file and assign to the result data 
		setHasData(hasData() && m_cFileAccess->getData(cProgram_, m_cData.get()));

		// Fileから集約関数は、count(*)をbitsetから行う場合のみ
		if (m_cAggregation.getSize() == 1) {
			m_cAggregation.accumulate(cProgram_);
			m_cAggregation.execute(cProgram_);
		}

#ifndef NO_TRACE
		if (hasData()
			&& _OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "File [" << m_cFileAccess->getName()
				<< "] getData = "
				<< Opt::Trace::toString(*m_cData)
				<< ModEndl;
		}
#endif
	}
	return setHasNext(hasData());
}

// FUNCTION public
//	Iterator::Impl::SingleFileImpl::reset --
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
Impl::SingleFileImpl::
reset(Interface::IProgram& cProgram_)
{
	if (m_cFileAccess.isInitialized()
		&& m_cFileAccess->isOpened()) {
		m_cFileAccess->reset();
		m_bIsFetched = false;


#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "File [" << m_cFileAccess->getName()
				<< "] reset"
				<< ModEndl;
		}
#endif
	}
	resetBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::SingleFileImpl::getLocker -- 
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
Impl::SingleFileImpl::
getLocker(Interface::IProgram& cProgram_)
{
	return cProgram_.getFileAccess(m_cFileAccess.getID())->getLocker();
}

// FUNCTION public
//	Iterator::Impl::SingleFileImpl::getOutData -- 
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
Impl::SingleFileImpl::
getOutData(Interface::IProgram& cProgram_)
{
	return m_cData.getDataID();
}

// FUNCTION public
//	Iterator::Impl::SingleFileImpl::getClassID -- 
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
Impl::SingleFileImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::SingleFile);
}

// FUNCTION public
//	Iterator::Impl::SingleFileImpl::serialize -- 
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
Impl::SingleFileImpl::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cFileAccess.serialize(archiver_);
	m_cData.serialize(archiver_);
	m_cKey.serialize(archiver_);
	m_cExplainArgument.serialize(archiver_);
	m_cAggregation.serialize(archiver_);
}

// FUNCTION protected
//	Iterator::Impl::SingleFileImpl::explainThis -- 
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
Impl::SingleFileImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	explainMethod(cExplain_);
	cExplain_.put(" on ");
	cExplain_.popNoNewLine();

	cExplain_.pushIndent();
	m_cFileAccess.explain(pEnvironment_, cProgram_, cExplain_, m_cExplainArgument);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		if (m_cKey.isValid()) {
			cExplain_.newLine();
			cExplain_.pushNoNewLine();
			cExplain_.put("by ");
			m_cKey.explain(cProgram_, cExplain_);
			cExplain_.popNoNewLine();
		}
		cExplain_.pushNoNewLine();
		cExplain_.put(" to ");
		m_cData.explain(cProgram_, cExplain_);
		cExplain_.popNoNewLine();
		if (m_cAggregation.getSize() > 0) {
			cExplain_.pushIndent().newLine(true);
			cExplain_.put("aggregation:").newLine();
			m_cAggregation.explain(pEnvironment_, cProgram_, cExplain_);
			cExplain_.popIndent();
		}
	}
	m_cFileAccess.explainStartUp(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popIndent(true /* force new line */);
}


// FUNCTION protected
//	Iterator::Impl::SingleFileImpl::addAggregation
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
Impl::SingleFileImpl::
addAggregation(Interface::IProgram& cProgram_,
			   const Action::Argument& cAction_)
{
	int iActionID = cAction_.getInstanceID();
	m_cAggregation.addID(iActionID);
}


// FUNCTION protected
//	Iterator::Impl::SingleFileImpl::addOutData -- 
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
Impl::SingleFileImpl::
addOutData(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	; _SYDNEY_ASSERT(m_cData.isValid() == false);

	// variable for result data is set through addAction(Projection)
	// (this switch-case can be implemented in Base class....)
	m_cData.setDataID(cAction_.getInstanceID());
}

// FUNCTION protected
//	Iterator::Impl::SingleFileImpl::addInput -- 
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
Impl::SingleFileImpl::
addInput(Interface::IProgram& cProgram_,
		 const Action::Argument& cAction_)
{
	; _SYDNEY_ASSERT(m_cKey.isValid() == false);

	// variable for key used by fetch
	m_cKey.setDataID(cAction_.getInstanceID());
}

// FUNCTION private
//	Iterator::Impl::SingleFileImpl::explainMethod -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::SingleFileImpl::
explainMethod(Opt::Explain& cExplain_)
{
	if (m_cKey.isValid()) {
		cExplain_.put(_pszExplainName[_Explain::Fetch]);
	} else if (m_cExplainArgument.m_bIsSearch) {
		cExplain_.put(_pszExplainName[_Explain::Search]);
	} else if (m_cExplainArgument.m_bIsSimple) {
		cExplain_.put(_pszExplainName[_Explain::Simple]);
	} else {
		cExplain_.put(_pszExplainName[_Explain::Sequencial]);
	}
}

////////////////////////////////
// Execution::Iterator::File

// FUNCTION public
//	Iterator::File::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	File*
//
// EXCEPTIONS

//static
File*
File::
create(Interface::IProgram& cProgram_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pFileAccess->getID(),
				  cArgument_);
}

// FUNCTION public
//	Iterator::File::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iFileAccessID_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	File*
//
// EXCEPTIONS

//static
File*
File::
create(Interface::IProgram& cProgram_,
	   int iFileAccessID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	AUTOPOINTER<File> pFile = new Impl::SingleFileImpl(iFileAccessID_,
													   cArgument_);
	pFile->registerToProgram(cProgram_);
	return pFile.release();
}

// FUNCTION public
//	Iterator::File::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	File*
//
// EXCEPTIONS

//static
File*
File::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::SingleFile);
	return new Impl::SingleFileImpl;
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
