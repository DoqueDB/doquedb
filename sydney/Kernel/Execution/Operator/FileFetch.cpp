// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/FileFetch.cpp --
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
const char moduleName[] = "Execution::Operator";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Operator/FileFetch.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Action/Locator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

#include "Schema/File.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	// ENUM
	//  _Type::Value -- subclass types
	//
	// NOTES

	struct _Type
	{
		enum Value
		{
			Normal = 0,
			GetLocator,
			ValueNum
		};
	};

	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName[_Type::ValueNum] =
		{
			"file fetch",
			"file getlocator"
		};
}

namespace FileFetchImpl
{
	// CLASS local
	//	Execution::Operator::FileFetchImpl::Base -- base class of implementation class of FileFetch
	//
	// NOTES
	class Base
		: public Operator::FileFetch
	{
	public:
		typedef Base This;
		typedef Operator::FileFetch Super;

		Base()
			: Super(),
			  m_cFileAccess(),
			  m_cFetchKey(),
			  m_cData(),
			  m_iFetchKeyID(-1)
		{}
		Base(int iFileAccessID_,
			   int iFetchKeyID_)
			: Super(),
			  m_cFileAccess(iFileAccessID_),
			  m_cFetchKey(),
			  m_cData(),
			  m_iFetchKeyID(iFetchKeyID_)
		{}
		virtual ~Base() {}

	///////////////////////////
	// Operator::FileFetch::
		virtual void setOutput(int iDataID_) {m_cData.setDataID(iDataID_);}

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
	//	int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
		void initializeBase(Interface::IProgram& cProgram_);
		void terminateBase(Interface::IProgram& cProgram_);
		void serializeBase(ModArchive& archiver_);

		// accessor
		Action::FileAccess* getFileAccess() {return m_cFileAccess.get();}
		Common::DataArrayData* getFetchKey() {return &m_cFetchKey;}
		Action::ArrayDataHolder& getData() {return m_cData;}

	private:
		virtual void explainThis(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_) = 0;
		virtual void explainData(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_) = 0;

		// file access utility class
		Action::FileAccessHolder m_cFileAccess;

		// fetch key data (first element is converted from m_iFetchKeyID)
		Common::DataArrayData m_cFetchKey;
		// variable of result data to which fetched tuple is assigned
		Action::ArrayDataHolder m_cData;

		// variable ID of data which is used as fetch key
		int m_iFetchKeyID;
	};

	// CLASS local
	//	Execution::Operator::FileFetchImpl::Normal -- implementation class of FileFetch
	//
	// NOTES
	class Normal
		: public Base
	{
	public:
		typedef Normal This;
		typedef Base Super;

		Normal()
			: Super()
		{}
		Normal(int iFileAccessID_,
			   int iFetchKeyID_)
			: Super(iFileAccessID_, iFetchKeyID_)
		{}
		~Normal() {}

	/////////////////////////////
	// Interface::IAction::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
	/////////////////////
	// Base::
		virtual void explainThis(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
		virtual void explainData(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Operator::FileFetchImpl::GetLocator -- implementation class of FileFetch
	//
	// NOTES
	class GetLocator
		: public Base
	{
	public:
		typedef GetLocator This;
		typedef Base Super;

		GetLocator()
			: Super(),
			  m_cLocator()
		{}
		GetLocator(int iFileAccessID_,
				   int iFetchKeyID_,
				   int iLocatorID_)
			: Super(iFileAccessID_, iFetchKeyID_),
			  m_cLocator(iLocatorID_)
		{}
		~GetLocator() {}

	/////////////////////////////
	// Interface::IAction::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
	/////////////////////
	// Base::
		virtual void explainThis(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
		virtual void explainData(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);

		Action::LocatorHolder m_cLocator;
	};
}

///////////////////////////////////////////////
// Execution::Operator::FileFetchImpl::Base

// FUNCTION public
//	Operator::FileFetchImpl::Base::explain -- 
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
FileFetchImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();

	explainThis(cProgram_, cExplain_);
	cExplain_.put(" on ");
	m_cFileAccess.explain(pEnvironment_, cProgram_, cExplain_, Opt::ExplainFileArgument());

	cExplain_.popNoNewLine();

	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.pushNoNewLine();
		cExplain_.pushIndent().newLine();
		cExplain_.put("by data:");
		cProgram_.explainVariable(cExplain_, m_iFetchKeyID);
		cExplain_.put(" to ");
		explainData(cProgram_, cExplain_);
		cExplain_.popIndent();
		cExplain_.popNoNewLine();
	}
	m_cFileAccess.explainStartUp(pEnvironment_, cProgram_, cExplain_);
}

// FUNCTION public
//	Operator::FileFetchImpl::Base::finish -- 
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
FileFetchImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileFetchImpl::Base::reset -- 
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
FileFetchImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION protected
//	Operator::FileFetchImpl::Base::initializeBase -- 
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
FileFetchImpl::Base::
initializeBase(Interface::IProgram& cProgram_)
{
	// convert ID to object
	if (m_cData.isInitialized() == false) {
		m_cFileAccess.initialize(cProgram_);
		m_cData.initialize(cProgram_);
		// fetch key is set to first element of dataarraydata
		// because Logical file assumes fetch key is dataarraydata
		// but FileFetch is used for single fetch-key files.
		m_cFetchKey.setElement(0, cProgram_.getVariable(m_iFetchKeyID));
	}
}

// FUNCTION protected
//	Operator::FileFetchImpl::Base::terminateBase -- 
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
FileFetchImpl::Base::
terminateBase(Interface::IProgram& cProgram_)
{
	m_cFileAccess.terminate(cProgram_);
	m_cData.terminate(cProgram_);
	m_cFetchKey.clear();
}

// FUNCTION protected
//	Operator::FileFetchImpl::Base::serializeBase -- 
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
FileFetchImpl::Base::
serializeBase(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cFileAccess.serialize(archiver_);
	archiver_(m_iFetchKeyID);
	m_cData.serialize(archiver_);
}

/////////////////////////////////////////////////
// Execution::Operator::FileFetchImpl::Normal

// FUNCTION public
//	Operator::FileFetchImpl::Normal::initialize -- 
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
FileFetchImpl::Normal::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
}

// FUNCTION public
//	Operator::FileFetchImpl::Normal::terminate -- 
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
FileFetchImpl::Normal::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
}

// FUNCTION public
//	Operator::FileFetchImpl::Normal::execute -- 
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
FileFetchImpl::Normal::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		bool bHasData = false;
		if (!getFetchKey()->getElement(0)->isNull()) {
			// fetch key is not null -> read from logical file
			getFileAccess()->open(cProgram_);
			getFileAccess()->fetch(getFetchKey());
#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
				_OPT_EXECUTION_MESSAGE
					<< "File [" << getFileAccess()->getName()
					<< "] fetch by "
					<< Opt::Trace::toString(*getFetchKey())
					<< ModEndl;
			}
#endif
			bHasData = getFileAccess()->getData(cProgram_, getData().get());
		}
		if (!bHasData) {
			// if fetch key is null or no tuple are read
			// -> set all the elements of result data as null
			int n = getData()->getCount();
			for (int i = 0; i < n; ++i) {
				getData()->getElement(i)->setNull();
			}
		}
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			if (bHasData) {
				_OPT_EXECUTION_MESSAGE
					<< "File [" << getFileAccess()->getName()
					<< "] getData = "
					<< Opt::Trace::toString(*getData())
					<< ModEndl;
			}
		}
#endif
		done();
	}
	return Action::Status::Success;
}

// 
// FUNCTION public
//	Operator::FileFetchImpl::Normal::getClassID -- 
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
FileFetchImpl::Normal::
getClassID() const
{
	return Class::getClassID(Class::Category::FileFetch);
}

// FUNCTION public
//	Operator::FileFetchImpl::Normal::serialize -- 
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
FileFetchImpl::Normal::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
}

// FUNCTION private
//	Operator::FileFetchImpl::Normal::explainThis -- 
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
FileFetchImpl::Normal::
explainThis(Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::Normal]);
}

// FUNCTION private
//	Operator::FileFetchImpl::Normal::explainData -- 
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
FileFetchImpl::Normal::
explainData(Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	getData().explain(cProgram_, cExplain_);
}

/////////////////////////////////////////////////
// Execution::Operator::FileFetchImpl::GetLocator

// FUNCTION public
//	Operator::FileFetchImpl::GetLocator::initialize -- 
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
FileFetchImpl::GetLocator::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cLocator.initialize(cProgram_);
}

// FUNCTION public
//	Operator::FileFetchImpl::GetLocator::terminate -- 
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
FileFetchImpl::GetLocator::
terminate(Interface::IProgram& cProgram_)
{
	m_cLocator.terminate(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Operator::FileFetchImpl::GetLocator::execute -- 
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
FileFetchImpl::GetLocator::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		bool bHasData = false;
		if (!getFetchKey()->getElement(0)->isNull()) {
			// fetch key is not null -> read from logical file
			getFileAccess()->open(cProgram_);
			bHasData = getFileAccess()->getLocator(cProgram_,
												   getFetchKey(),
												   m_cLocator.getLocator());
		}
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			if (bHasData) {
				_OPT_EXECUTION_MESSAGE
					<< "File [" << getFileAccess()->getName()
					<< "] getLocator by "
					<< Opt::Trace::toString(*getFetchKey())
					<< ModEndl;
			}
		}
#endif
		done();
	}
	return Action::Status::Success;
}

// 
// FUNCTION public
//	Operator::FileFetchImpl::GetLocator::getClassID -- 
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
FileFetchImpl::GetLocator::
getClassID() const
{
	return Class::getClassID(Class::Category::FileGetLocator);
}

// FUNCTION public
//	Operator::FileFetchImpl::GetLocator::serialize -- 
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
FileFetchImpl::GetLocator::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cLocator.serialize(archiver_);
}

// FUNCTION private
//	Operator::FileFetchImpl::GetLocator::explainThis -- 
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
FileFetchImpl::GetLocator::
explainThis(Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::GetLocator]);
}

// FUNCTION private
//	Operator::FileFetchImpl::GetLocator::explainData -- 
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
FileFetchImpl::GetLocator::
explainData(Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	m_cLocator.explain(cProgram_, cExplain_);
}

////////////////////////////////////////
// Operator::FileFetch::GetLocator::

// FUNCTION public
//	Operator::FileFetch::GetLocator::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iFetchKeyID_
//	int iLocatorID_
//	
// RETURN
//	FileFetch*
//
// EXCEPTIONS

//static
FileFetch*
FileFetch::GetLocator::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iFetchKeyID_,
	   int iLocatorID_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pIterator_,
				  pFileAccess->getID(),
				  iFetchKeyID_,
				  iLocatorID_);
}

// FUNCTION public
//	Operator::FileFetch::GetLocator::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iFetchKeyID_
//	int iLocatorID_
//	
// RETURN
//	FileFetch*
//
// EXCEPTIONS

//static
FileFetch*
FileFetch::GetLocator::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iFetchKeyID_,
	   int iLocatorID_)
{
	AUTOPOINTER<This> pResult = new FileFetchImpl::GetLocator(iFileAccessID_,
															  iFetchKeyID_,
															  iLocatorID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::FileFetch::

// FUNCTION public
//	Operator::FileFetch::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iFetchKeyID_
//	
// RETURN
//	FileFetch*
//
// EXCEPTIONS

//static
FileFetch*
FileFetch::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iFetchKeyID_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pIterator_,
				  pFileAccess->getID(),
				  iFetchKeyID_);
}

// FUNCTION public
//	Operator::FileFetch::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iFetchKeyID_
//	
// RETURN
//	FileFetch*
//
// EXCEPTIONS

//static
FileFetch*
FileFetch::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iFetchKeyID_)
{
	AUTOPOINTER<This> pResult = new FileFetchImpl::Normal(iFileAccessID_,
														  iFetchKeyID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::FileFetch::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	FileFetch*
//
// EXCEPTIONS

//static
FileFetch*
FileFetch::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::FileFetch:
		{
			return new FileFetchImpl::Normal;
		}
	case Class::Category::FileGetLocator:
		{
			return new FileFetchImpl::GetLocator;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);	
		}
	}
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
