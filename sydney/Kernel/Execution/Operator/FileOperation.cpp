// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/FileOperation.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Operator/FileOperation.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/FileAccess.h"
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
	struct _Explain
	{
		enum Value
		{
			Insert = 0,
			Update,
			Expunge,
			UndoUpdate,
			UndoExpunge,
			ValueNum
		};
	};

	// CON ST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName[] = {
		"insert",
		"update",
		"expunge",
		"undo update",
		"undo expunge",
		0
	};
}

namespace FileOperationImpl
{
	// CLASS local
	//	Execution::Operator::FileOperationImpl::Base -- base class of implementation classes
	//
	// NOTES
	class Base
		: public Operator::FileOperation
	{
	public:
		typedef Base This;
		typedef Operator::FileOperation Super;

		// destructor
		virtual ~Base() {}

	///////////////////////////
	// Operator::FileOperation::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);

	protected:
		// constructor
		Base()
			: Super(),
			  m_cFileAccess()
		{}
		Base(int iFileAccessID_)
			: Super(),
			  m_cFileAccess(iFileAccessID_)
		{}

		void initializeBase(Interface::IProgram& cProgram_);
		void terminateBase(Interface::IProgram& cProgram_);
		void serializeBase(ModArchive& archiver_);

	/////////////////////////////
	// Interface::IAction::
		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);

	private:
		virtual const char* getOperatorName() = 0;
		virtual void doOperation(Interface::IProgram& cProgram_,
								 Action::FileAccessHolder& cFileAccess_) = 0;
		virtual Opt::Explain& explainThis(Opt::Environment* pEnvironment_,
										  Interface::IProgram& cProgram_,
										  Opt::Explain& cExplain_) = 0;
										  
		Action::FileAccessHolder m_cFileAccess;
	};

	// CLASS local
	//	Execution::Operator::FileOperationImpl::Insert -- implementation class of FileOperation
	//
	// NOTES
	class Insert
		: public Base
	{
	public:
		typedef Insert This;
		typedef Base Super;

		Insert()
			: Super(),
			  m_cData()
		{}
		Insert(int iFileAccessID_,
				   int iDataID_)
			: Super(iFileAccessID_),
			  m_cData(iDataID_)
		{}
		virtual ~Insert() {}

	///////////////////////////
	// Operator::FileOperation::

	/////////////////////////////
	// Interface::IAction::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	//////////////////////////////
	// FileOperationImpl::Base::
		// chaneged to protected because subclass use this
		virtual void doOperation(Interface::IProgram& cProgram_,
								 Action::FileAccessHolder& cFileAccess_);
	private:
	//////////////////////////////
	// FileOperationImpl::Base::
		virtual const char* getOperatorName();
		virtual Opt::Explain& explainThis(Opt::Environment* pEnvironment_,
										  Interface::IProgram& cProgram_,
										  Opt::Explain& cExplain_);

		// variable of insert data
		Action::ArrayDataHolder m_cData;
	};

	// CLASS local
	//	Execution::Operator::FileOperationImpl::Update -- implementation class of FileOperation
	//
	// NOTES
	class Update
		: public Base
	{
	public:
		typedef Update This;
		typedef Base Super;

		Update()
			: Super(),
			  m_cKey(),
			  m_cData()
		{}
		Update(int iFileAccessID_,
			   int iKeyID_,
			   int iDataID_)
			: Super(iFileAccessID_),
			  m_cKey(iKeyID_),
			  m_cData(iDataID_)
		{}
		virtual ~Update() {}

	///////////////////////////
	// Operator::FileOperation::

	/////////////////////////////
	// Interface::IAction::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	//////////////////////////////
	// FileOperationImpl::Base::
		// chaneged to protected because subclass use this
		virtual void doOperation(Interface::IProgram& cProgram_,
								 Action::FileAccessHolder& cFileAccess_);
	private:
	//////////////////////////////
	// FileOperationImpl::Base::
		virtual const char* getOperatorName();
		virtual Opt::Explain& explainThis(Opt::Environment* pEnvironment_,
										  Interface::IProgram& cProgram_,
										  Opt::Explain& cExplain_);

		// variable of update key and data
		Action::ArrayDataHolder m_cKey;
		Action::ArrayDataHolder m_cData;
	};

	// CLASS local
	//	Execution::Operator::FileOperationImpl::Expunge -- implementation class of FileOperation
	//
	// NOTES
	class Expunge
		: public Base
	{
	public:
		typedef Expunge This;
		typedef Base Super;

		Expunge()
			: Super(),
			  m_cKey()
		{}
		Expunge(int iFileAccessID_,
				int iKeyID_)
			: Super(iFileAccessID_),
			  m_cKey(iKeyID_)
		{}
		virtual ~Expunge() {}

	///////////////////////////
	// Operator::FileOperation::

	/////////////////////////////
	// Interface::IAction::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	//////////////////////////////
	// FileOperationImpl::Base::
		// chaneged to protected because subclass use this
		virtual void doOperation(Interface::IProgram& cProgram_,
								 Action::FileAccessHolder& cFileAccess_);
	private:
	//////////////////////////////
	// FileOperationImpl::Base::
		virtual const char* getOperatorName();
		virtual Opt::Explain& explainThis(Opt::Environment* pEnvironment_,
										  Interface::IProgram& cProgram_,
										  Opt::Explain& cExplain_);

		// variable of expunge key
		Action::ArrayDataHolder m_cKey;
	};

	// CLASS local
	//	Execution::Operator::FileOperationImpl::UndoUpdate -- implementation class of FileOperation
	//
	// NOTES
	class UndoUpdate
		: public Base
	{
	public:
		typedef UndoUpdate This;
		typedef Base Super;

		UndoUpdate()
			: Super(),
			  m_cKey()
		{}
		UndoUpdate(int iFileAccessID_,
				   int iKeyID_)
			: Super(iFileAccessID_),
			  m_cKey(iKeyID_)
		{}
		virtual ~UndoUpdate() {}

	///////////////////////////
	// Operator::FileOperation::

	/////////////////////////////
	// Interface::IAction::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	//////////////////////////////
	// FileOperationImpl::Base::
		// chaneged to protected because subclass use this
		virtual void doOperation(Interface::IProgram& cProgram_,
								 Action::FileAccessHolder& cFileAccess_);
	private:
	//////////////////////////////
	// FileOperationImpl::Base::
		virtual const char* getOperatorName();
		virtual Opt::Explain& explainThis(Opt::Environment* pEnvironment_,
										  Interface::IProgram& cProgram_,
										  Opt::Explain& cExplain_);

		// variable of undoUpdate key
		Action::ArrayDataHolder m_cKey;
	};

	// CLASS local
	//	Execution::Operator::FileOperationImpl::UndoExpunge -- implementation class of FileOperation
	//
	// NOTES
	class UndoExpunge
		: public Base
	{
	public:
		typedef UndoExpunge This;
		typedef Base Super;

		UndoExpunge()
			: Super(),
			  m_cKey()
		{}
		UndoExpunge(int iFileAccessID_,
				int iKeyID_)
			: Super(iFileAccessID_),
			  m_cKey(iKeyID_)
		{}
		virtual ~UndoExpunge() {}

	///////////////////////////
	// Operator::FileOperation::

	/////////////////////////////
	// Interface::IAction::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	//////////////////////////////
	// FileOperationImpl::Base::
		// chaneged to protected because subclass use this
		virtual void doOperation(Interface::IProgram& cProgram_,
								 Action::FileAccessHolder& cFileAccess_);
	private:
	//////////////////////////////
	// FileOperationImpl::Base::
		virtual const char* getOperatorName();
		virtual Opt::Explain& explainThis(Opt::Environment* pEnvironment_,
										  Interface::IProgram& cProgram_,
										  Opt::Explain& cExplain_);

		// variable of undoExpunge key
		Action::ArrayDataHolder m_cKey;
	};
}

///////////////////////////////////////////////////
// Execution::Operator::FileOperationImpl::Base

// FUNCTION public
//	Operator::FileOperationImpl::Base::explain -- 
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
FileOperationImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();

	cExplain_.put(getOperatorName()).put(" ");
	m_cFileAccess.explain(pEnvironment_, cProgram_, cExplain_, Opt::ExplainFileArgument());
	explainThis(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Operator::FileOperationImpl::Base::execute -- 
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
FileOperationImpl::Base::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		bool bHasData = false;

		m_cFileAccess->open(cProgram_);

		doOperation(cProgram_, m_cFileAccess);

		done();
	}
	return Action::Status::Success;
}

// FUNCTION protected
//	Operator::FileOperationImpl::Base::initializeBase -- 
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
FileOperationImpl::Base::
initializeBase(Interface::IProgram& cProgram_)
{
	m_cFileAccess.initialize(cProgram_);
}

// FUNCTION protected
//	Operator::FileOperationImpl::Base::terminateBase -- 
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
FileOperationImpl::Base::
terminateBase(Interface::IProgram& cProgram_)
{
	m_cFileAccess.terminate(cProgram_);
}

// FUNCTION protected
//	Operator::FileOperationImpl::Base::serializeBase -- 
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
FileOperationImpl::Base::
serializeBase(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cFileAccess.serialize(archiver_);
}

//////////////////////////////////////////////////////
// Execution::Operator::FileOperationImpl::Insert

// FUNCTION public
//	Operator::FileOperationImpl::Insert::initialize -- 
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
FileOperationImpl::Insert::
initialize(Interface::IProgram& cProgram_)
{
	// convert ID to object
	if (m_cData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::FileOperationImpl::Insert::terminate -- 
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
FileOperationImpl::Insert::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::FileOperationImpl::Insert::finish -- 
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
FileOperationImpl::Insert::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileOperationImpl::Insert::reset -- 
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
FileOperationImpl::Insert::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileOperationImpl::Insert::getClassID -- 
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
FileOperationImpl::Insert::
getClassID() const
{
	return Class::getClassID(Class::Category::FileInsert);
}

// FUNCTION public
//	Operator::FileOperationImpl::Insert::serialize -- 
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
FileOperationImpl::Insert::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cData.serialize(archiver_);
}

// FUNCTION protected
//	Operator::FileOperationImpl::Insert::doOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::FileAccessHolder& cFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileOperationImpl::Insert::
doOperation(Interface::IProgram& cProgram_,
			Action::FileAccessHolder& cFileAccess_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "File [" << cFileAccess_->getName() << "] "
			<< getOperatorName() << " "
			<< Opt::Trace::toString(*m_cData)
			<< ModEndl;
	}
#endif

	cFileAccess_->insert(cProgram_,
						 const_cast<Common::DataArrayData*>(m_cData.getData()));
}

// FUNCTION private
//	Operator::FileOperationImpl::Insert::getOperatorName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
FileOperationImpl::Insert::
getOperatorName()
{
	return _pszOperatorName[_Explain::Insert];
}

// FUNCTION private
//	Operator::FileiOperationImpl::Insert::explainThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Opt::Explain&
//
// EXCEPTIONS

//virtual
Opt::Explain&
FileOperationImpl::Insert::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cData.explain(cProgram_, cExplain_);
	}
	return cExplain_;
}

//////////////////////////////////////////////////////
// Execution::Operator::FileOperationImpl::Update

// FUNCTION public
//	Operator::FileOperationImpl::Update::initialize -- 
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
FileOperationImpl::Update::
initialize(Interface::IProgram& cProgram_)
{
	// convert ID to object
	if (m_cData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cKey.initialize(cProgram_);
		m_cData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::FileOperationImpl::Update::terminate -- 
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
FileOperationImpl::Update::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cKey.terminate(cProgram_);
	m_cData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::FileOperationImpl::Update::finish -- 
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
FileOperationImpl::Update::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileOperationImpl::Update::reset -- 
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
FileOperationImpl::Update::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileOperationImpl::Update::getClassID -- 
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
FileOperationImpl::Update::
getClassID() const
{
	return Class::getClassID(Class::Category::FileUpdate);
}

// FUNCTION public
//	Operator::FileOperationImpl::Update::serialize -- 
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
FileOperationImpl::Update::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cKey.serialize(archiver_);
	m_cData.serialize(archiver_);
}

// FUNCTION protected
//	Operator::FileOperationImpl::Update::doOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::FileAccessHolder& cFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileOperationImpl::Update::
doOperation(Interface::IProgram& cProgram_,
			Action::FileAccessHolder& cFileAccess_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "File [" << cFileAccess_->getName() << "] "
			<< getOperatorName() << " "
			<< "(" << Opt::Trace::toString(*m_cKey) << ")"
			<< " => " << Opt::Trace::toString(*m_cData)
			<< ModEndl;
	}
#endif

	cFileAccess_->update(cProgram_,
						 m_cKey.getData(),
						 const_cast<Common::DataArrayData*>(m_cData.getData()));
}

// FUNCTION private
//	Operator::FileOperationImpl::Update::getOperatorName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
FileOperationImpl::Update::
getOperatorName()
{
	return _pszOperatorName[_Explain::Update];
}

// FUNCTION private
//	Operator::FileiOperationImpl::Update::explainThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Opt::Explain&
//
// EXCEPTIONS

//virtual
Opt::Explain&
FileOperationImpl::Update::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cKey.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cData.explain(cProgram_, cExplain_);
	}
	return cExplain_;
}

//////////////////////////////////////////////////////
// Execution::Operator::FileOperationImpl::Expunge

// FUNCTION public
//	Operator::FileOperationImpl::Expunge::initialize -- 
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
FileOperationImpl::Expunge::
initialize(Interface::IProgram& cProgram_)
{
	// convert ID to object
	if (m_cKey.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cKey.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::FileOperationImpl::Expunge::terminate -- 
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
FileOperationImpl::Expunge::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cKey.terminate(cProgram_);
}

// FUNCTION public
//	Operator::FileOperationImpl::Expunge::finish -- 
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
FileOperationImpl::Expunge::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileOperationImpl::Expunge::reset -- 
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
FileOperationImpl::Expunge::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileOperationImpl::Expunge::getClassID -- 
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
FileOperationImpl::Expunge::
getClassID() const
{
	return Class::getClassID(Class::Category::FileExpunge);
}

// FUNCTION public
//	Operator::FileOperationImpl::Expunge::serialize -- 
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
FileOperationImpl::Expunge::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cKey.serialize(archiver_);
}

// FUNCTION protected
//	Operator::FileOperationImpl::Expunge::doOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::FileAccessHolder& cFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileOperationImpl::Expunge::
doOperation(Interface::IProgram& cProgram_,
			Action::FileAccessHolder& cFileAccess_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "File [" << cFileAccess_->getName() << "] "
			<< getOperatorName() << " "
			<< "(" << Opt::Trace::toString(*m_cKey) << ")"
			<< ModEndl;
	}
#endif

	cFileAccess_->expunge(cProgram_, m_cKey.getData());
}

// FUNCTION private
//	Operator::FileOperationImpl::Expunge::getOperatorName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
FileOperationImpl::Expunge::
getOperatorName()
{
	return _pszOperatorName[_Explain::Expunge];
}

// FUNCTION private
//	Operator::FileiOperationImpl::Expunge::explainThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Opt::Explain&
//
// EXCEPTIONS

//virtual
Opt::Explain&
FileOperationImpl::Expunge::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cKey.explain(cProgram_, cExplain_);
	}
	return cExplain_;
}

//////////////////////////////////////////////////////
// Execution::Operator::FileOperationImpl::UndoUpdate

// FUNCTION public
//	Operator::FileOperationImpl::UndoUpdate::initialize -- 
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
FileOperationImpl::UndoUpdate::
initialize(Interface::IProgram& cProgram_)
{
	// convert ID to object
	if (m_cKey.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cKey.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::FileOperationImpl::UndoUpdate::terminate -- 
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
FileOperationImpl::UndoUpdate::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cKey.terminate(cProgram_);
}

// FUNCTION public
//	Operator::FileOperationImpl::UndoUpdate::finish -- 
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
FileOperationImpl::UndoUpdate::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileOperationImpl::UndoUpdate::reset -- 
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
FileOperationImpl::UndoUpdate::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileOperationImpl::UndoUpdate::getClassID -- 
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
FileOperationImpl::UndoUpdate::
getClassID() const
{
	return Class::getClassID(Class::Category::FileUndoUpdate);
}

// FUNCTION public
//	Operator::FileOperationImpl::UndoUpdate::serialize -- 
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
FileOperationImpl::UndoUpdate::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cKey.serialize(archiver_);
}

// FUNCTION protected
//	Operator::FileOperationImpl::UndoUpdate::doOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::FileAccessHolder& cFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileOperationImpl::UndoUpdate::
doOperation(Interface::IProgram& cProgram_,
			Action::FileAccessHolder& cFileAccess_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "File [" << cFileAccess_->getName() << "] "
			<< getOperatorName() << " "
			<< "(" << Opt::Trace::toString(*m_cKey) << ")"
			<< ModEndl;
	}
#endif

	cFileAccess_->undoUpdate(cProgram_, m_cKey.getData());
}

// FUNCTION private
//	Operator::FileOperationImpl::UndoUpdate::getOperatorName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
FileOperationImpl::UndoUpdate::
getOperatorName()
{
	return _pszOperatorName[_Explain::UndoUpdate];
}

// FUNCTION private
//	Operator::FileiOperationImpl::UndoUpdate::explainThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Opt::Explain&
//
// EXCEPTIONS

//virtual
Opt::Explain&
FileOperationImpl::UndoUpdate::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cKey.explain(cProgram_, cExplain_);
	}
	return cExplain_;
}

//////////////////////////////////////////////////////
// Execution::Operator::FileOperationImpl::UndoExpunge

// FUNCTION public
//	Operator::FileOperationImpl::UndoExpunge::initialize -- 
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
FileOperationImpl::UndoExpunge::
initialize(Interface::IProgram& cProgram_)
{
	// convert ID to object
	if (m_cKey.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cKey.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::FileOperationImpl::UndoExpunge::terminate -- 
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
FileOperationImpl::UndoExpunge::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cKey.terminate(cProgram_);
}

// FUNCTION public
//	Operator::FileOperationImpl::UndoExpunge::finish -- 
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
FileOperationImpl::UndoExpunge::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileOperationImpl::UndoExpunge::reset -- 
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
FileOperationImpl::UndoExpunge::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileOperationImpl::UndoExpunge::getClassID -- 
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
FileOperationImpl::UndoExpunge::
getClassID() const
{
	return Class::getClassID(Class::Category::FileUndoExpunge);
}

// FUNCTION public
//	Operator::FileOperationImpl::UndoExpunge::serialize -- 
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
FileOperationImpl::UndoExpunge::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cKey.serialize(archiver_);
}

// FUNCTION protected
//	Operator::FileOperationImpl::UndoExpunge::doOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::FileAccessHolder& cFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileOperationImpl::UndoExpunge::
doOperation(Interface::IProgram& cProgram_,
			Action::FileAccessHolder& cFileAccess_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "File [" << cFileAccess_->getName() << "] "
			<< getOperatorName() << " "
			<< "(" << Opt::Trace::toString(*m_cKey) << ")"
			<< ModEndl;
	}
#endif

	cFileAccess_->undoExpunge(cProgram_, m_cKey.getData());
}

// FUNCTION private
//	Operator::FileOperationImpl::UndoExpunge::getOperatorName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
FileOperationImpl::UndoExpunge::
getOperatorName()
{
	return _pszOperatorName[_Explain::UndoExpunge];
}

// FUNCTION private
//	Operator::FileiOperationImpl::UndoExpunge::explainThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Opt::Explain&
//
// EXCEPTIONS

//virtual
Opt::Explain&
FileOperationImpl::UndoExpunge::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cKey.explain(cProgram_, cExplain_);
	}
	return cExplain_;
}

/////////////////////////////////////
// Operator::FileOperation::Insert

// FUNCTION public
//	Operator::FileOperation::Insert::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iDataID_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

//static
FileOperation*
FileOperation::Insert::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iDataID_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pIterator_,
				  pFileAccess->getID(),
				  iDataID_);
}

// FUNCTION public
//	Operator::FileOperation::Insert::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iDataID_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

FileOperation*
FileOperation::Insert::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult;
	pResult = new FileOperationImpl::Insert(iFileAccessID_,
											iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Operator::FileOperation::Update

// FUNCTION public
//	Operator::FileOperation::Update::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iKeyID_
//	int iDataID_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

//static
FileOperation*
FileOperation::Update::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iKeyID_,
	   int iDataID_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pIterator_,
				  pFileAccess->getID(),
				  iKeyID_,
				  iDataID_);
}

// FUNCTION public
//	Operator::FileOperation::Update::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iKeyID_
//	int iDataID_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

//static
FileOperation*
FileOperation::Update::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iKeyID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult;
	pResult = new FileOperationImpl::Update(iFileAccessID_,
											iKeyID_,
											iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Operator::FileOperation::Expunge

// FUNCTION public
//	Operator::FileOperation::Expunge::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iKeyID_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

//static
FileOperation*
FileOperation::Expunge::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iKeyID_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pIterator_,
				  pFileAccess->getID(),
				  iKeyID_);
}

// FUNCTION public
//	Operator::FileOperation::Expunge::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iKeyID_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

//static
FileOperation*
FileOperation::Expunge::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iKeyID_)
{
	AUTOPOINTER<This> pResult;
	pResult = new FileOperationImpl::Expunge(iFileAccessID_,
											iKeyID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Operator::FileOperation::UndoUpdate

// FUNCTION public
//	Operator::FileOperation::UndoUpdate::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iKeyID_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

//static
FileOperation*
FileOperation::UndoUpdate::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iKeyID_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pIterator_,
				  pFileAccess->getID(),
				  iKeyID_);
}

// FUNCTION public
//	Operator::FileOperation::UndoUpdate::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iKeyID_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

//static
FileOperation*
FileOperation::UndoUpdate::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iKeyID_)
{
	AUTOPOINTER<This> pResult;
	pResult = new FileOperationImpl::UndoUpdate(iFileAccessID_,
												iKeyID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Operator::FileOperation::UndoExpunge

// FUNCTION public
//	Operator::FileOperation::UndoExpunge::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iKeyID_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

//static
FileOperation*
FileOperation::UndoExpunge::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iKeyID_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pIterator_,
				  pFileAccess->getID(),
				  iKeyID_);
}

// FUNCTION public
//	Operator::FileOperation::UndoExpunge::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iKeyID_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

//static
FileOperation*
FileOperation::UndoExpunge::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iKeyID_)
{
	AUTOPOINTER<This> pResult;
	pResult = new FileOperationImpl::UndoExpunge(iFileAccessID_,
												 iKeyID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::FileOperation::

// FUNCTION public
//	Operator::FileOperation::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	FileOperation*
//
// EXCEPTIONS

//static
FileOperation*
FileOperation::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::FileInsert:
		{
			return new FileOperationImpl::Insert;
		}
	case Class::Category::FileUpdate:
		{
			return new FileOperationImpl::Update;
		}
	case Class::Category::FileExpunge:
		{
			return new FileOperationImpl::Expunge;
		}
	case Class::Category::FileUndoUpdate:
		{
			return new FileOperationImpl::UndoUpdate;
		}
	case Class::Category::FileUndoExpunge:
		{
			return new FileOperationImpl::UndoExpunge;
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
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
