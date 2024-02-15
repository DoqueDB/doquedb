// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/FileGetProperty.cpp --
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
const char moduleName[] = "Execution::Operator";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Operator/FileGetProperty.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"

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
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "get property";
}

namespace Impl
{
	// CLASS local
	//	Execution::Operator::Impl::FileGetPropertyImpl -- implementation class of FileGetProperty
	//
	// NOTES
	class FileGetPropertyImpl
		: public Operator::FileGetProperty
	{
	public:
		typedef FileGetPropertyImpl This;
		typedef Operator::FileGetProperty Super;

		FileGetPropertyImpl()
			: Super(),
			  m_cFileAccess(),
			  m_cKey(),
			  m_cValue()
		{}
		FileGetPropertyImpl(int iFileAccessID_,
							int iPropertyKeyID_,
							int iPropertyValueID_)
			: Super(),
			  m_cFileAccess(iFileAccessID_),
			  m_cKey(iPropertyKeyID_),
			  m_cValue(iPropertyValueID_)
		{}
		~FileGetPropertyImpl() {}

	///////////////////////////
	// Operator::FileGetProperty::

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
		// file access utility class
		Action::FileAccessHolder m_cFileAccess;

		Action::ArrayDataHolder m_cKey;
		Action::ArrayDataHolder m_cValue;
	};
}

///////////////////////////////////////////////
// Execution::Operator::Impl::FileGetPropertyImpl

// FUNCTION public
//	Operator::Impl::FileGetPropertyImpl::explain -- 
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
Impl::FileGetPropertyImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperatorName);

	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" to (");
		m_cKey.explain(cProgram_, cExplain_);
		cExplain_.put(", ");
		m_cValue.explain(cProgram_, cExplain_);
		cExplain_.put(")");
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Operator::Impl::FileGetPropertyImpl::initialize -- 
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
Impl::FileGetPropertyImpl::
initialize(Interface::IProgram& cProgram_)
{
	// convert ID to object
	if (m_cFileAccess.isInitialized() == false) {
		m_cFileAccess.initialize(cProgram_);
		m_cKey.initialize(cProgram_);
		m_cValue.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::Impl::FileGetPropertyImpl::terminate -- 
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
Impl::FileGetPropertyImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cFileAccess.terminate(cProgram_);
	m_cKey.terminate(cProgram_);
	m_cValue.terminate(cProgram_);
}

// FUNCTION public
//	Operator::Impl::FileGetPropertyImpl::execute -- 
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
Impl::FileGetPropertyImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		m_cFileAccess->getProperty(cProgram_,
								   m_cKey.get(),
								   m_cValue.get());
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "File [" << m_cFileAccess->getName()
				<< "] getProperty"
				<< ModEndl;
			}
#endif
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::Impl::FileGetPropertyImpl::finish -- 
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
Impl::FileGetPropertyImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::Impl::FileGetPropertyImpl::reset -- 
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
Impl::FileGetPropertyImpl::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::Impl::FileGetPropertyImpl::getClassID -- 
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
Impl::FileGetPropertyImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::FileGetProperty);
}

// FUNCTION public
//	Operator::Impl::FileGetPropertyImpl::serialize -- 
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
Impl::FileGetPropertyImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cFileAccess.serialize(archiver_);
	m_cKey.serialize(archiver_);
	m_cValue.serialize(archiver_);
}

//////////////////////////////
// Operator::FileGetProperty::

// FUNCTION public
//	Operator::FileGetProperty::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iGetPropertyKeyID_
//	
// RETURN
//	FileGetProperty*
//
// EXCEPTIONS

//static
FileGetProperty*
FileGetProperty::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iPropertyKeyID_,
	   int iPropertyValueID_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pIterator_,
				  pFileAccess->getID(),
				  iPropertyKeyID_,
				  iPropertyValueID_);
}

// FUNCTION public
//	Operator::FileGetProperty::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iGetPropertyKeyID_
//	
// RETURN
//	FileGetProperty*
//
// EXCEPTIONS

//static
FileGetProperty*
FileGetProperty::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iPropertyKeyID_,
	   int iPropertyValueID_)
{
	AUTOPOINTER<This> pResult = new Impl::FileGetPropertyImpl(iFileAccessID_,
															  iPropertyKeyID_,
															  iPropertyValueID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::FileGetProperty::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	FileGetProperty*
//
// EXCEPTIONS

//static
FileGetProperty*
FileGetProperty::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::FileGetProperty);
	return new Impl::FileGetPropertyImpl;
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
