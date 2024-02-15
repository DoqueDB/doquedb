// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/FileBitSet.cpp --
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

#include "Execution/Operator/FileBitSet.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
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
	struct _Type
	{
		enum Value {
			Intersect,
			Union,
			Difference,
			ValueNum
		};
	};
	const char* const _pszExplainName[] =
	{
		"intersect(bitset) ",
		"union(bitset) ",
		"difference(bitset) ",
	};
}

namespace FileBitSetImpl
{
	// CLASS local
	//	Execution::Operator::FileBitSetImpl::Base -- implementation class of FileBitSet
	//
	// NOTES
	class Base
		: public Operator::FileBitSet
	{
	public:
		typedef Base This;
		typedef Operator::FileBitSet Super;

		virtual ~Base() {}

	///////////////////////////
	// Operator::FileBitSet::

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
	//	int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		// constructorr
		Base()
			: Super(),
			  m_cFileAccess(),
			  m_cBitSet(),
			  m_cMyBitSet(),
			  m_cData(),
			  m_cExplainArgument(),
			  m_cLocker(),
			  m_bResult(true)
		{}
		Base(int iFileAccessID_,
			 int iBitSetID_,
			 int iMyBitSetID_,
			 const Opt::ExplainFileArgument& cArgument_)
			: Super(),
			  m_cFileAccess(iFileAccessID_),
			  m_cBitSet(iBitSetID_),
			  m_cMyBitSet(iMyBitSetID_),
			  m_cData(),
			  m_cExplainArgument(cArgument_),
			  m_cLocker(),
			  m_bResult(true)
		{}

		// accessor
		Action::LockerHolder& getLocker() {return m_cLocker;}

	private:
		// explain operator
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

		// check whether combine is necessary
		virtual bool needCombine(Common::BitSet& cResult_) = 0;
		// combine my bitset
		virtual bool combine(Common::BitSet& cResult_,
							 const Common::BitSet& cMyResult_) = 0;
		// combine with empty bitset
		virtual bool combineEmpty(Common::BitSet& cResult_) = 0;

		// file access utility class
		Action::FileAccessHolder m_cFileAccess;

		// bitset object
		Action::BitSetHolder m_cBitSet;
		// bitset data obtained from logical file
		Action::BitSetHolder m_cMyBitSet;

		// data used for reading from logical file (not pooled in IProgram)
		Common::DataArrayData m_cData;

		// Predicate object for explain
		Opt::ExplainFileArgument m_cExplainArgument;

		// locker
		Action::LockerHolder m_cLocker;

		// result of execution
		bool m_bResult;
	};

	// CLASS local
	//	Execution::Operator::FileBitSetImpl::Intersect -- implementation class of FileBitSet
	//
	// NOTES
	class Intersect
		: public Base
	{
	public:
		typedef Intersect This;
		typedef Base Super;

		// constructor
		Intersect()
			: Super()
		{}
		Intersect(int iFileAccessID_,
				  int iBitSetID_,
				  int iMyBitSetID_,
				  const Opt::ExplainFileArgument& cArgument_)
			: Super(iFileAccessID_, iBitSetID_, iMyBitSetID_, cArgument_)
		{}
		// destructor
		~Intersect() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual bool needCombine(Common::BitSet& cResult_);
		virtual bool combine(Common::BitSet& cResult_,
							 const Common::BitSet& cMyResult_);
		virtual bool combineEmpty(Common::BitSet& cResult_);
	};

	// CLASS local
	//	Execution::Operator::FileBitSetImpl::Union -- implementation class of FileBitSet
	//
	// NOTES
	class Union
		: public Base
	{
	public:
		typedef Union This;
		typedef Base Super;

		// constructor
		Union()
			: Super()
		{}
		Union(int iFileAccessID_,
			  int iBitSetID_,
			  int iMyBitSetID_,
			  const Opt::ExplainFileArgument& cArgument_)
			: Super(iFileAccessID_, iBitSetID_, iMyBitSetID_, cArgument_)
		{}
		// destructor
		~Union() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual bool needCombine(Common::BitSet& cResult_);
		virtual bool combine(Common::BitSet& cResult_,
							 const Common::BitSet& cMyResult_);
		virtual bool combineEmpty(Common::BitSet& cResult_);
	};

	// CLASS local
	//	Execution::Operator::FileBitSetImpl::Difference -- implementation class of FileBitSet
	//
	// NOTES
	class Difference
		: public Base
	{
	public:
		typedef Difference This;
		typedef Base Super;

		// constructor
		Difference()
			: Super()
		{}
		Difference(int iFileAccessID_,
				   int iBitSetID_,
				   int iMyBitSetID_,
				   const Opt::ExplainFileArgument& cArgument_)
			: Super(iFileAccessID_, iBitSetID_, iMyBitSetID_, cArgument_)
		{}
		// destructor
		~Difference() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual bool needCombine(Common::BitSet& cResult_);
		virtual bool combine(Common::BitSet& cResult_,
							 const Common::BitSet& cMyResult_);
		virtual bool combineEmpty(Common::BitSet& cResult_);
	};
}

///////////////////////////////////////////////
// Execution::Operator::FileBitSetImpl::Base

// FUNCTION public
//	Operator::FileBitSetImpl::Base::explain -- 
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
FileBitSetImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	explainOperator(cProgram_, cExplain_);

	cExplain_.pushIndent();
	m_cFileAccess.explain(pEnvironment_, cProgram_, cExplain_, m_cExplainArgument);
	m_cFileAccess.explainStartUp(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popIndent(true /* force new line */);
}

// FUNCTION public
//	Operator::FileBitSetImpl::Base::initialize -- 
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
FileBitSetImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cBitSet.isInitialized() == false) {
		m_cFileAccess.initialize(cProgram_);
		m_cLocker.setID(m_cFileAccess->getLocker());
		// prepare result bitset
		m_cBitSet.initialize(cProgram_);
		m_cMyBitSet.initialize(cProgram_);
		// prepare my bitset
		m_cData.setElement(0, static_cast<const Common::Data*>(m_cMyBitSet.getData()));
		; _SYDNEY_ASSERT(m_cBitSet.isInitialized());
	}
}

// FUNCTION public
//	Operator::FileBitSetImpl::Base::terminate -- 
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
FileBitSetImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	m_cFileAccess.terminate(cProgram_);
	m_cLocker.setID(-1);
	m_cBitSet.terminate(cProgram_);
	m_cMyBitSet.terminate(cProgram_);
}

// FUNCTION public
//	Operator::FileBitSetImpl::Base::execute -- 
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
FileBitSetImpl::Base::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (m_bResult = needCombine(*m_cBitSet)) {
			// bitset has at least one bit
			// -> read from logical file as bitset
			m_cFileAccess->open(cProgram_);
			if (m_cFileAccess->getData(cProgram_, &m_cData)) {
				m_bResult = combine(*m_cBitSet, *m_cMyBitSet);
			} else {
				m_bResult = combineEmpty(*m_cBitSet);
			}
		}
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "File [" << m_cFileAccess->getName()
				<< "] getData = "
				<< Opt::Trace::toString(m_cData)
				<< ModEndl;
			_OPT_EXECUTION_MESSAGE
				<< "BitSet bitset = "
				<< Opt::Trace::toString(*m_cBitSet)
				<< ModEndl;
		}
#endif
		done();
	}
	return m_bResult ? Action::Status::Success : Action::Status::False;
}

// FUNCTION public
//	Operator::FileBitSetImpl::Base::finish -- 
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
FileBitSetImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileBitSetImpl::Base::reset -- 
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
FileBitSetImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::FileBitSetImpl::Base::serialize -- 
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
FileBitSetImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cFileAccess.serialize(archiver_);
	m_cBitSet.serialize(archiver_);
	m_cMyBitSet.serialize(archiver_);
	m_cExplainArgument.serialize(archiver_);
}

////////////////////////////////////////////////////
// Execution::Operator::FileBitSetImpl::Intersect

// FUNCTION public
//	Operator::FileBitSetImpl::Intersect::getClassID -- 
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
FileBitSetImpl::Intersect::
getClassID() const
{
	return Class::getClassID(Class::Category::FileBitSetIntersect);
}

// FUNCTION private
//	Operator::FileBitSetImpl::Intersect::explainOperator -- 
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
FileBitSetImpl::Intersect::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Intersect]);
}

// FUNCTION private
//	Operator::FileBitSetImpl::Intersect::needCombine -- check whether combine is necessary
//
// NOTES
//
// ARGUMENTS
//	Common::BitSet& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileBitSetImpl::Intersect::
needCombine(Common::BitSet& cResult_)
{
	return cResult_.any();
}

// FUNCTION private
//	Operator::FileBitSetImpl::Intersect::combine -- combine my bitset
//
// NOTES
//
// ARGUMENTS
//	Common::BitSet& cResult_
//	const Common::BitSet& cMyResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileBitSetImpl::Intersect::
combine(Common::BitSet& cResult_,
		const Common::BitSet& cMyResult_)
{
	if (getLocker()) {
		Common::BitSet cAbandon(cResult_);
		cAbandon ^= cMyResult_;
		getLocker()->unlock(cAbandon);
	}
	cResult_ &= cMyResult_;
	if (getLocker()) {
		// result should be unlock once because locked twice
		getLocker()->unlock(cResult_);
	}
	return cResult_.any();
}

// FUNCTION private
//	Operator::FileBitSetImpl::Intersect::combineEmpty -- combine with empty bitset
//
// NOTES
//
// ARGUMENTS
//	Common::BitSet& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileBitSetImpl::Intersect::
combineEmpty(Common::BitSet& cResult_)
{
	if (getLocker()) {
		getLocker()->unlock(cResult_);
	}
	cResult_.reset();
	return false;
}

////////////////////////////////////////////////////
// Execution::Operator::FileBitSetImpl::Union

// FUNCTION public
//	Operator::FileBitSetImpl::Union::getClassID -- 
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
FileBitSetImpl::Union::
getClassID() const
{
	return Class::getClassID(Class::Category::FileBitSetUnion);
}

// FUNCTION private
//	Operator::FileBitSetImpl::Union::explainOperator -- 
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
FileBitSetImpl::Union::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Union]);
}

// FUNCTION private
//	Operator::FileBitSetImpl::Union::needCombine -- check whether combine is necessary
//
// NOTES
//
// ARGUMENTS
//	Common::BitSet& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileBitSetImpl::Union::
needCombine(Common::BitSet& cResult_)
{
	return true;
}

// FUNCTION private
//	Operator::FileBitSetImpl::Union::combine -- combine my bitset
//
// NOTES
//
// ARGUMENTS
//	Common::BitSet& cResult_
//	const Common::BitSet& cMyResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileBitSetImpl::Union::
combine(Common::BitSet& cResult_,
		const Common::BitSet& cMyResult_)
{
	if (getLocker()) {
		Common::BitSet cAbandon(cResult_);
		cAbandon &= cMyResult_; // duplicate tuples should be unlocked once
		getLocker()->unlock(cAbandon);
	}
	cResult_ |= cMyResult_;
	return true;
}

// FUNCTION private
//	Operator::FileBitSetImpl::Union::combineEmpty -- combine with empty bitset
//
// NOTES
//
// ARGUMENTS
//	Common::BitSet& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileBitSetImpl::Union::
combineEmpty(Common::BitSet& cResult_)
{
	// do nothing
	return true;
}

////////////////////////////////////////////////////
// Execution::Operator::FileBitSetImpl::Difference

// FUNCTION public
//	Operator::FileBitSetImpl::Difference::getClassID -- 
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
FileBitSetImpl::Difference::
getClassID() const
{
	return Class::getClassID(Class::Category::FileBitSetDifference);
}

// FUNCTION private
//	Operator::FileBitSetImpl::Difference::explainOperator -- 
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
FileBitSetImpl::Difference::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Difference]);
}

// FUNCTION private
//	Operator::FileBitSetImpl::Difference::needCombine -- check whether combine is necessary
//
// NOTES
//
// ARGUMENTS
//	Common::BitSet& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileBitSetImpl::Difference::
needCombine(Common::BitSet& cResult_)
{
	return cResult_.any();
}

// FUNCTION private
//	Operator::FileBitSetImpl::Difference::combine -- combine my bitset
//
// NOTES
//
// ARGUMENTS
//	Common::BitSet& cResult_
//	const Common::BitSet& cMyResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileBitSetImpl::Difference::
combine(Common::BitSet& cResult_,
		const Common::BitSet& cMyResult_)
{
	if (getLocker()) {
		Common::BitSet cAbandon(cResult_);
		cAbandon &= cMyResult_;
		getLocker()->unlock(cAbandon); // duplicate part should be unlocked twice
		getLocker()->unlock(cMyResult_);
	}
	cResult_ -= cMyResult_;
	return cResult_.any();
}

// FUNCTION private
//	Operator::FileBitSetImpl::Difference::combineEmpty -- combine with empty bitset
//
// NOTES
//
// ARGUMENTS
//	Common::BitSet& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileBitSetImpl::Difference::
combineEmpty(Common::BitSet& cResult_)
{
	// do nothing
	return true;
}

///////////////////////////////////////
// Operator::FileBitSet::Intersect

// FUNCTION public
//	Operator::FileBitSet::Intersect::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iBitSetID_
//	int iMyBitSetID_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	FileBitSet*
//
// EXCEPTIONS

//static
FileBitSet*
FileBitSet::Intersect::
create(Interface::IProgram& cProgram_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iBitSetID_,
	   int iMyBitSetID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pFileAccess->getID(),
				  iBitSetID_,
				  iMyBitSetID_,
				  cArgument_);
}

// FUNCTION public
//	Operator::FileBitSet::Intersect::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iBitSetID_
//	int iMyBitSetID_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	FileBitSet*
//
// EXCEPTIONS

//static
FileBitSet*
FileBitSet::Intersect::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iBitSetID_,
	   int iMyBitSetID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	AUTOPOINTER<This> pResult = 
		new FileBitSetImpl::Intersect(iFileAccessID_,
									  iBitSetID_,
									  iMyBitSetID_,
									  cArgument_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

///////////////////////////////////////
// Operator::FileBitSet::Union

// FUNCTION public
//	Operator::FileBitSet::Union::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iBitSetID_
//	int iMyBitSetID_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	FileBitSet*
//
// EXCEPTIONS

//static
FileBitSet*
FileBitSet::Union::
create(Interface::IProgram& cProgram_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iBitSetID_,
	   int iMyBitSetID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pFileAccess->getID(),
				  iBitSetID_,
				  iMyBitSetID_,
				  cArgument_);
}

// FUNCTION public
//	Operator::FileBitSet::Union::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iBitSetID_
//	int iMyBitSetID_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	FileBitSet*
//
// EXCEPTIONS

//static
FileBitSet*
FileBitSet::Union::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iBitSetID_,
	   int iMyBitSetID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	AUTOPOINTER<This> pResult = 
		new FileBitSetImpl::Union(iFileAccessID_,
								  iBitSetID_,
								  iMyBitSetID_,
								  cArgument_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

///////////////////////////////////////
// Operator::FileBitSet::Difference

// FUNCTION public
//	Operator::FileBitSet::Difference::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	int iBitSetID_
//	int iMyBitSetID_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	FileBitSet*
//
// EXCEPTIONS

//static
FileBitSet*
FileBitSet::Difference::
create(Interface::IProgram& cProgram_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   int iBitSetID_,
	   int iMyBitSetID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	Action::FileAccess* pFileAccess =
		Action::FileAccess::create(cProgram_,
								   pSchemaTable_,
								   pSchemaFile_,
								   cOpenOption_);
	return create(cProgram_,
				  pFileAccess->getID(),
				  iBitSetID_,
				  iMyBitSetID_,
				  cArgument_);
}

// FUNCTION public
//	Operator::FileBitSet::Difference::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iFileAccessID_
//	int iBitSetID_
//	int iMyBitSetID_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	FileBitSet*
//
// EXCEPTIONS

//static
FileBitSet*
FileBitSet::Difference::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iFileAccessID_,
	   int iBitSetID_,
	   int iMyBitSetID_,
	   const Opt::ExplainFileArgument& cArgument_)
{
	AUTOPOINTER<This> pResult = 
		new FileBitSetImpl::Difference(iFileAccessID_,
									   iBitSetID_,
									   iMyBitSetID_,
									   cArgument_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::FileBitSet::

// FUNCTION public
//	Operator::FileBitSet::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	FileBitSet*
//
// EXCEPTIONS

//static
FileBitSet*
FileBitSet::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::FileBitSetIntersect:
		{
			return new FileBitSetImpl::Intersect;
		}
	case Class::Category::FileBitSetUnion:
		{
			return new FileBitSetImpl::Union;
		}
	case Class::Category::FileBitSetDifference:
		{
			return new FileBitSetImpl::Difference;
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
