// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/FileCheck.cpp --
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
const char moduleName[] = "Execution::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Predicate/FileCheck.h"
#include "Execution/Predicate/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/Cancel.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

#include "Schema/File.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

namespace
{
	struct _Type
	{
		enum Value {
			ByBitSet = 0,
			ByCollection,
			ValueNum
		};
	};
	const char* const _pszExplainName[] = {
		"check(bitset) ",
		"check(collection) ",
	};
}

namespace FileCheckImpl
{
	// CLASS local
	//	Execution::Predicate::FileCheckImpl::Base -- base class of implementation class of FileCheck
	//
	// NOTES
	class Base
		: public Predicate::FileCheck
	{
	public:
		typedef Base This;
		typedef Predicate::FileCheck Super;

		// constructor
		Base()
			: Super(),
			  m_cFileAccess(),
			  m_cRowID(),
			  m_cPrevBitSet(),
			  m_bInitialized(false),
			  m_cExplainArgument()
		{}
		Base(int iFileAccessID_,
			 int iRowIDID_,
			 int iPrevBitSetID_,
			 const Opt::ExplainFileArgument& cArgument_)
			: Super(),
			  m_cFileAccess(iFileAccessID_),
			  m_cRowID(iRowIDID_),
			  m_cPrevBitSet(iPrevBitSetID_),
			  m_bInitialized(false),
			  m_cExplainArgument(cArgument_)
		{}

		// destructor
		virtual ~Base() {}

	///////////////////////////
	// Predicate::FileCheck::

	/////////////////////////////
	// Interface::IPredicate::
	//	virtual Boolean::Value check(Interface::IProgram& cProgram_);

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	protected:
		// open file
		bool open(Interface::IProgram& cProgram_) {return m_cFileAccess->open(cProgram_);}

		// getdata
		bool getData(Interface::IProgram& cProgram_,
					 Common::DataArrayData* pData_)
		{return m_cFileAccess->getData(cProgram_, pData_);}

		// get filename
		ModUnicodeString getName() {return m_cFileAccess->getName();}

		// accessor
		Common::UnsignedIntegerData* getRowID() {return m_cRowID.getData();}
		Common::BitSet* getPrevBitSet() {return m_cPrevBitSet.getData();}

		// for serialize
		void serializeBase(ModArchive& cArchive_);

	private:
		// for explain
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;
		// create data
		virtual bool initializeData(Interface::IProgram& cProgram_) = 0;
		// drop data
		virtual void terminateData(Interface::IProgram& cProgram_) = 0;
		// check main
		virtual Boolean::Value checkRowID(Interface::IProgram& cProgram_,
										  unsigned int iRowID_) = 0;

	/////////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		// file access utility class
		Action::FileAccessHolder m_cFileAccess;

		// rowid data object converted from m_iRowIDID
		Action::RowIDHolder m_cRowID;

		// variable of bitset object which stores result of previously executed FileCheck if exists
		Action::BitSetHolder m_cPrevBitSet;

		// runtime flag
		bool m_bInitialized;

		// argument for explain
		Opt::ExplainFileArgument m_cExplainArgument;
	};

	// CLASS local
	//	Execution::Predicate::FileCheckImpl::ByBitSet -- implementation class of FileCheck (bitset)
	//
	// NOTES
	//	logical file result is obtained as bitset and checked for existence of rowid in the bitset
	class ByBitSet
		: public Base
	{
	public:
		typedef ByBitSet This;
		typedef Base Super;

		ByBitSet()
			: Super(),
			  m_cResult(),
			  m_pBitSet(0),
			  m_cBitSet(),
			  m_cData()
		{}
		ByBitSet(int iFileAccessID_,
				 int iRowIDID_,
				 int iResultID_,
				 int iPrevBitSetID_,
				 const Opt::ExplainFileArgument& cArgument_)
			: Super(iFileAccessID_,
					iRowIDID_,
					iPrevBitSetID_,
					cArgument_),
			  m_cResult(iResultID_),
			  m_pBitSet(0),
			  m_cBitSet(),
			  m_cData()
		{}
		~ByBitSet() {}

	/////////////////////////////
	// Base::

	///////////////////////////
	// Predicate::FileCheck::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
	/////////////////////////////
	// Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual bool initializeData(Interface::IProgram& cProgram_);
		virtual void terminateData(Interface::IProgram& cProgram_);
		virtual Boolean::Value checkRowID(Interface::IProgram& cProgram_,
										  unsigned int iRowID_);
		Action::BitSetHolder m_cResult;
		Common::BitSet* m_pBitSet;
		Common::BitSet m_cBitSet;
		Common::DataArrayData m_cData;
	};

	// CLASS local
	//	Execution::Predicate::FileCheckImpl::ByCollection -- implementation class of FileCheck (collection)
	//
	// NOTES
	//	logical file result is stored in (rowid->tuple) map.
	//	checked for existence of rowid in the map and related tuple is assigned to result data

	class ByCollection
		: public Base
	{
	public:
		typedef ByCollection This;
		typedef Base Super;

		ByCollection()
			: Super(),
			  m_cResult(),
			  m_mapTuple()
		{}
		ByCollection(int iFileAccessID_,
					 int iRowIDID_,
					 int iResultID_,
					 int iPrevBitSetID_,
					 const Opt::ExplainFileArgument& cArgument_)
			: Super(iFileAccessID_,
					iRowIDID_,
					iPrevBitSetID_,
					cArgument_),
			  m_cResult(iResultID_),
			  m_mapTuple()
		{}
		~ByCollection() {}

	/////////////////////////////
	// Base::

	///////////////////////////
	// Predicate::FileCheck::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
	/////////////////////////////
	// Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual bool initializeData(Interface::IProgram& cProgram_);
		virtual void terminateData(Interface::IProgram& cProgram_);
		virtual Boolean::Value checkRowID(Interface::IProgram& cProgram_,
										  unsigned int iRowID_);

		typedef MAP< unsigned int,
					 Common::Data::Pointer,
					 LESS<unsigned int> > TupleMap;

		Action::ArrayDataHolder m_cResult;
		TupleMap m_mapTuple;
	};
}

///////////////////////////////////////////////////
// Execution::Predicate::FileCheckImpl::Base

// FUNCTION public
//	Predicate::FileCheckImpl::Base::explain -- 
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
FileCheckImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	explainOperator(cProgram_, cExplain_);
	m_cFileAccess.explain(pEnvironment_, cProgram_, cExplain_, m_cExplainArgument);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.pushNoNewLine();
		cExplain_.put(" by ");
		m_cRowID.explain(cProgram_, cExplain_);
		cExplain_.popNoNewLine();
	}
	m_cFileAccess.explainStartUp(pEnvironment_, cProgram_, cExplain_);
}

// FUNCTION public
//	Predicate::FileCheckImpl::Base::initialize -- 
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
FileCheckImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cRowID.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cFileAccess.initialize(cProgram_);
		m_cRowID.initialize(cProgram_);
		if (m_cPrevBitSet.isValid()) {
			m_cPrevBitSet.initialize(cProgram_);
		}
	}
}

// FUNCTION public
//	Predicate::FileCheckImpl::Base::terminate -- 
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
FileCheckImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	m_cFileAccess.terminate(cProgram_);
	m_cRowID.terminate(cProgram_);
	m_cPrevBitSet.terminate(cProgram_);
	terminateData(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Predicate::FileCheckImpl::Base::finish -- 
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
FileCheckImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::FileCheckImpl::Base::reset -- 
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
FileCheckImpl::Base::
reset(Interface::IProgram& cProgram_)
{
}

// FUNCTION public
//	Predicate::FileCheckImpl::Base::serializeBase -- for serialize
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileCheckImpl::Base::
serializeBase(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cFileAccess.serialize(cArchive_);
	m_cRowID.serialize(cArchive_);
	m_cPrevBitSet.serialize(cArchive_);
	m_cExplainArgument.serialize(cArchive_);
}

// FUNCTION private
//	Predicate::FileCheckImpl::Base::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Boolean::Value
//
// EXCEPTIONS

//virtual
FileCheck::Boolean::Value
FileCheckImpl::Base::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	if (!m_bInitialized) {
		if (initializeData(cProgram_) == false) {
			// this file become never true
			m_bInitialized = true;
			return Boolean::NeverTrue;
		}
		m_bInitialized = true;
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "File [" << getName()
			<< "] check by "
			<< m_cRowID->toString() << ModEndl;
	}
#endif
	return checkRowID(cProgram_, m_cRowID->getValue());
}

///////////////////////////////////////
// Predicate::FileCheckImpl::ByBitSet

// FUNCTION public
//	Predicate::FileCheckImpl::ByBitSet::getClassID -- 
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
FileCheckImpl::ByBitSet::
getClassID() const
{
	return Class::getClassID(Class::Category::FileCheckByBitSet);
}

// FUNCTION public
//	Predicate::FileCheckImpl::ByBitSet::serialize -- 
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
FileCheckImpl::ByBitSet::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cResult.serialize(archiver_);
}

// FUNCTION private
//	Predicate::FileCheckImpl::ByBitSet::explainOperator -- 
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
FileCheckImpl::ByBitSet::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::ByBitSet]);
}

// FUNCTION private
//	Predicate::FileCheckImpl::ByBitSet::initializeData -- 
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
FileCheckImpl::ByBitSet::
initializeData(Interface::IProgram& cProgram_)
{
	; _SYDNEY_ASSERT(m_cResult.isInitialized() == false);

	// prepare bitset
	if (getPrevBitSet() == 0) {
		// first checking file -> use result as output
		m_cResult.initialize(cProgram_);
		; _SYDNEY_ASSERT(m_cResult.isInitialized());
		m_pBitSet = m_cResult.getData();
	} else {
		if (getPrevBitSet()->none()) return false;

		// use local bitset as output
		m_pBitSet = &m_cBitSet;
	}
	m_cData.setElement(0, static_cast<const Common::Data*>(m_pBitSet));

	// open file
	open(cProgram_);

	// get data as bitset
	if (getData(cProgram_, &m_cData)) {
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "File [" << getName()
				<< "] getData (bitset)"
				<< ModEndl;
		}
#endif
		if (getPrevBitSet()) {
			// take and with preceding bitset
			*getPrevBitSet() &= *m_pBitSet;
			*m_pBitSet = *getPrevBitSet();
		}
	}
	return m_pBitSet->any();
}

// FUNCTION private
//	Predicate::FileCheckImpl::ByBitSet::terminateData -- 
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
FileCheckImpl::ByBitSet::
terminateData(Interface::IProgram& cProgram_)
{
	m_cResult.terminate(cProgram_);
	m_pBitSet = 0;
}

// FUNCTION private
//	Predicate::FileCheckImpl::ByBitSet::checkRowID -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	unsigned int iRowID_
//	
// RETURN
//	Interface::IPredicate::Boolean::Value
//
// EXCEPTIONS

//virtual
Interface::IPredicate::Boolean::Value
FileCheckImpl::ByBitSet::
checkRowID(Interface::IProgram& cProgram_,
		   unsigned int iRowID_)
{
	return m_pBitSet->test(iRowID_) ? Boolean::True : Boolean::False;
}

///////////////////////////////////////////
// Predicate::FileCheckImpl::ByCollection

// FUNCTION public
//	Predicate::FileCheckImpl::ByCollection::getClassID -- 
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
FileCheckImpl::ByCollection::
getClassID() const
{
	return Class::getClassID(Class::Category::FileCheckByCollection);
}

// FUNCTION public
//	Predicate::FileCheckImpl::ByCollection::serialize -- 
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
FileCheckImpl::ByCollection::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cResult.serialize(archiver_);
}

// FUNCTION private
//	Predicate::FileCheckImpl::ByCollection::explainOperator -- 
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
FileCheckImpl::ByCollection::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::ByCollection]);
}

// FUNCTION private
//	Predicate::FileCheckImpl::ByCollection::initializeData -- 
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
FileCheckImpl::ByCollection::
initializeData(Interface::IProgram& cProgram_)
{
	// bitset holding all rowids
	Common::BitSet cBitSet;

	m_cResult.initialize(cProgram_);

	if (getPrevBitSet() && getPrevBitSet()->none()) return false;

	// open file
	open(cProgram_);

	// get data using result
	while (getData(cProgram_, m_cResult.get())) {
		// check cancel
		if (cProgram_.getTransaction()->isCanceledStatement()) {
			_SYDNEY_THROW0(Exception::Cancel);
		}
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "File [" << getName()
				<< "] getData = "
				<< Opt::Trace::toString(*m_cResult)
				<< ModEndl;
		}
#endif
		if (getPrevBitSet()
			&& getPrevBitSet()->test(getRowID()->getValue()) == false) {
			continue;
		}
		m_mapTuple.insert(getRowID()->getValue(),
						  m_cResult->copy());
		cBitSet.set(getRowID()->getValue());
	}
	if (getPrevBitSet()) {
		*getPrevBitSet() &= cBitSet;
	}
	return !m_mapTuple.ISEMPTY();
}

// FUNCTION private
//	Predicate::FileCheckImpl::ByCollection::terminateData -- 
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
FileCheckImpl::ByCollection::
terminateData(Interface::IProgram& cProgram_)
{
	m_cResult.terminate(cProgram_);
	m_mapTuple.clear();
}

// FUNCTION private
//	Predicate::FileCheckImpl::ByCollection::checkRowID -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	unsigned int iRowID_
//	
// RETURN
//	Interface::IPredicate::Boolean::Value
//
// EXCEPTIONS

//virtual
Interface::IPredicate::Boolean::Value
FileCheckImpl::ByCollection::
checkRowID(Interface::IProgram& cProgram_,
		   unsigned int iRowID_)
{
	TupleMap::Iterator found = m_mapTuple.find(iRowID_);
	if (found != m_mapTuple.end()) {
		// assign data
		m_cResult->assign((*found).second.get());
		return Boolean::True;
	}
	return Boolean::False;
}

/////////////////////////////////////
// Predicate::FileCheck::ByBitSet

// FUNCTION public
//	Predicate::FileCheck::ByBitSet::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iRowIDID_
//	int iResultID_
//	int iPrevBitSetID_
//	Type::Value eType_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	FileCheck*
//
// EXCEPTIONS

//static
FileCheck*
FileCheck::ByBitSet::
create(Interface::IProgram& cProgram_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iRowIDID_,
	   int iResultID_,
	   int iPrevBitSetID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pFileAccess->getID(),
				  iRowIDID_,
				  iResultID_,
				  iPrevBitSetID_,
				  cArgument_);
}

// FUNCTION public
//	Predicate::FileCheck::ByBitSet::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iRowIDID_
//	int iResultID_
//	int iPrevBitSetID_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	FileCheck*
//
// EXCEPTIONS

//static
FileCheck*
FileCheck::ByBitSet::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iRowIDID_,
	   int iResultID_,
	   int iPrevBitSetID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	AUTOPOINTER<This> pResult =
		new FileCheckImpl::ByBitSet(iFileAccessID_,
									iRowIDID_,
									iResultID_,
									iPrevBitSetID_,
									cArgument_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Predicate::FileCheck::ByCollection

// FUNCTION public
//	Predicate::FileCheck::ByCollection::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iRowIDID_
//	int iResultID_
//	int iPrevBitSetID_
//	Type::Value eType_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	FileCheck*
//
// EXCEPTIONS

//static
FileCheck*
FileCheck::ByCollection::
create(Interface::IProgram& cProgram_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iRowIDID_,
	   int iResultID_,
	   int iPrevBitSetID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pFileAccess->getID(),
				  iRowIDID_,
				  iResultID_,
				  iPrevBitSetID_,
				  cArgument_);
}

// FUNCTION public
//	Predicate::FileCheck::ByCollection::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iRowIDID_
//	int iResultID_
//	int iPrevBitSetID_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	FileCheck*
//
// EXCEPTIONS

//static
FileCheck*
FileCheck::ByCollection::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iRowIDID_,
	   int iResultID_,
	   int iPrevBitSetID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	AUTOPOINTER<This> pResult =
		new FileCheckImpl::ByCollection(iFileAccessID_,
										iRowIDID_,
										iResultID_,
										iPrevBitSetID_,
										cArgument_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Predicate::FileCheck::

// FUNCTION public
//	Predicate::FileCheck::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	FileCheck*
//
// EXCEPTIONS

//static
FileCheck*
FileCheck::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::FileCheckByBitSet:
		{
			return new FileCheckImpl::ByBitSet;
		}
	case Class::Category::FileCheckByCollection:
		{
			return new FileCheckImpl::ByCollection;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_PREDICATE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
