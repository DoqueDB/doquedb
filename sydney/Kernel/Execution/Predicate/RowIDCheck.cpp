// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/RowIDCheck.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#include "Execution/Predicate/RowIDCheck.h"
#include "Execution/Predicate/Class.h"

#include "Execution/Action/Collection.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/IteratorHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

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
	const char* const _pszExplainName[_Type::ValueNum] = {
		"check rowID(bitset)",
		"check rowID(collection)"
	};
}

namespace RowIDCheckImpl
{
	// CLASS local
	//	Execution::Predicate::RowIDCheckImpl::Base -- base class of implementation class of RowIDCheck
	//
	// NOTES
	class Base
		: public Predicate::RowIDCheck
	{
	public:
		typedef Base This;
		typedef Predicate::RowIDCheck Super;

		// constructor
		Base()
			: Super(),
			  m_cIterator(),
			  m_cRowID(),
			  m_cPrevBitSet(),
			  m_bInitialized(false)
		{}
		Base(int iIteratorID_,
			 int iRowIDID_,
			 int iPrevBitSetID_)
			: Super(),
			  m_cIterator(iIteratorID_),
			  m_cRowID(iRowIDID_),
			  m_cPrevBitSet(iPrevBitSetID_),
			  m_bInitialized(false)
		{}

		// destructor
		virtual ~Base() {}

	///////////////////////////
	// Predicate::RowIDCheck::

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
		// accessor
		Interface::IIterator* getIterator() {return m_cIterator.getIterator();}
		const Common::UnsignedIntegerData* getRowID() {return m_cRowID.getData();}
		Common::BitSet* getPrevBitSet() {return m_cPrevBitSet.get();}

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

		// iterator to get checked target
		Action::IteratorHolder m_cIterator;

		// rowid data object to be checked
		Action::RowIDHolder m_cRowID;

		// previous bitset data object
		Action::BitSetHolder m_cPrevBitSet;

		// runtime flag
		bool m_bInitialized;
	};

	// CLASS local
	//	Execution::Predicate::RowIDCheckImpl::ByBitSet -- implementation class of RowIDCheck (bitset)
	//
	// NOTES
	//	iterator should provide bitset
	class ByBitSet
		: public Base
	{
	public:
		typedef ByBitSet This;
		typedef Base Super;

		ByBitSet()
			: Super(),
			  m_cBitSet()
		{}
		ByBitSet(int iIteratorID_,
				 int iRowIDID_,
				 int iBitSetID_,
				 int iPrevBitSetID_)
			: Super(iIteratorID_,
					iRowIDID_,
					iPrevBitSetID_),
			  m_cBitSet(iBitSetID_)
		{}
		~ByBitSet() {}

	/////////////////////////////
	// Base::

	///////////////////////////
	// Predicate::RowIDCheck::

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
		Action::BitSetHolder m_cBitSet;
	};

	// CLASS local
	//	Execution::Predicate::RowIDCheckImpl::ByCollection -- implementation class of RowIDCheck (collection)
	//
	// NOTES
	//	iterator should provide tuples including rowid value
	class ByCollection
		: public Base
	{
	public:
		typedef ByCollection This;
		typedef Base Super;

		ByCollection()
			: Super(),
			  m_cResult(),
			  m_cInRowID(),
			  m_cInData(),
			  m_cCollection(),
			  m_mapTuple()
		{}
		ByCollection(int iIteratorID_,
					 int iRowIDID_,
					 int iResultID_,
					 int iInRowIDID_,
					 int iInDataID_,
					 int iPrevBitSetID_,
					 int iCollectionID_)
			: Super(iIteratorID_,
					iRowIDID_,
					iPrevBitSetID_),
			  m_cResult(iResultID_),
			  m_cInRowID(iInRowIDID_),
			  m_cInData(iInDataID_),
			  m_cCollection(iCollectionID_, iInDataID_),
			  m_mapTuple()
		{}
		~ByCollection() {}

	/////////////////////////////
	// Base::

	///////////////////////////
	// Predicate::RowIDCheck::

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
					 int,
					 LESS<unsigned int> > TupleMap;

		Action::ArrayDataHolder m_cResult; // to output checked tuple
		Action::RowIDHolder m_cInRowID;	   // to obtain rowid from iterator
		Action::ArrayDataHolder m_cInData; // to obtain tuple from iterator
		Action::Collection m_cCollection;  // collection holder

		TupleMap m_mapTuple;
	};
}

///////////////////////////////////////////////////
// Execution::Predicate::RowIDCheckImpl::Base

// FUNCTION public
//	Predicate::RowIDCheckImpl::Base::explain -- 
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
RowIDCheckImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	explainOperator(cProgram_, cExplain_);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.pushNoNewLine();
		cExplain_.put(" by ");
		m_cRowID.explain(cProgram_, cExplain_);
		cExplain_.popNoNewLine();
	}
	cExplain_.pushIndent();
	m_cIterator.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popIndent(true /* new line */);
}

// FUNCTION public
//	Predicate::RowIDCheckImpl::Base::initialize -- 
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
RowIDCheckImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cRowID.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cIterator.initialize(cProgram_);
		m_cRowID.initialize(cProgram_);
		m_cPrevBitSet.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::RowIDCheckImpl::Base::terminate -- 
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
RowIDCheckImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	m_cIterator.terminate(cProgram_);
	m_cRowID.terminate(cProgram_);
	m_cPrevBitSet.terminate(cProgram_);
	terminateData(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Predicate::RowIDCheckImpl::Base::finish -- 
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
RowIDCheckImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::RowIDCheckImpl::Base::reset -- 
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
RowIDCheckImpl::Base::
reset(Interface::IProgram& cProgram_)
{
}

// FUNCTION public
//	Predicate::RowIDCheckImpl::Base::serializeBase -- for serialize
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
RowIDCheckImpl::Base::
serializeBase(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cIterator.serialize(cArchive_);
	m_cRowID.serialize(cArchive_);
	m_cPrevBitSet.serialize(cArchive_);
}

// FUNCTION private
//	Predicate::RowIDCheckImpl::Base::evaluate -- 
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
RowIDCheck::Boolean::Value
RowIDCheckImpl::Base::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	if (!m_bInitialized) {
		if (initializeData(cProgram_) == false) {
			// this predicate never become true
			m_bInitialized = true;
			return Boolean::NeverTrue;
		}
		m_bInitialized = true;
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Variable)) {
		_OPT_EXECUTION_MESSAGE
			<< "RowID check by "
			<< m_cRowID->toString() << ModEndl;
	}
#endif
	return checkRowID(cProgram_, m_cRowID->getValue());
}

///////////////////////////////////////
// Predicate::RowIDCheckImpl::ByBitSet

// FUNCTION public
//	Predicate::RowIDCheckImpl::ByBitSet::getClassID -- 
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
RowIDCheckImpl::ByBitSet::
getClassID() const
{
	return Class::getClassID(Class::Category::RowIDCheckByBitSet);
}

// FUNCTION public
//	Predicate::RowIDCheckImpl::ByBitSet::serialize -- 
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
RowIDCheckImpl::ByBitSet::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cBitSet.serialize(archiver_);
}

// FUNCTION private
//	Predicate::RowIDCheckImpl::ByBitSet::explainOperator -- 
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
RowIDCheckImpl::ByBitSet::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszExplainName[_Type::ByBitSet]);
	cExplain_.put(" with ");
	m_cBitSet.explain(cProgram_, cExplain_);
	cExplain_.popNoNewLine();
}

// FUNCTION private
//	Predicate::RowIDCheckImpl::ByBitSet::initializeData -- 
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
RowIDCheckImpl::ByBitSet::
initializeData(Interface::IProgram& cProgram_)
{
	m_cBitSet.initialize(cProgram_);
	// do iterator once
	bool bResult = false;
	cProgram_.initialize(getIterator());
	try {
		switch (cProgram_.startUp(getIterator())) {
		case Action::Status::Success:
		case Action::Status::False:
		case Action::Status::Continue:
			{
				if (cProgram_.next(getIterator())) {
					if (getPrevBitSet()) {
						*m_cBitSet &= *getPrevBitSet();
						*getPrevBitSet() = *m_cBitSet;
					}
					bResult = m_cBitSet->any();
				}
				break;
			}
		case Action::Status::Break:
			{
				break;
			}
		default:
			{
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
		cProgram_.finish(getIterator());
	} catch (...) {
		try {
			cProgram_.terminate(getIterator());
		} catch (...) {
			// ignore
		}
		_SYDNEY_RETHROW;
	}
	return bResult;
}

// FUNCTION private
//	Predicate::RowIDCheckImpl::ByBitSet::terminateData -- 
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
RowIDCheckImpl::ByBitSet::
terminateData(Interface::IProgram& cProgram_)
{
	m_cBitSet.terminate(cProgram_);
}

// FUNCTION private
//	Predicate::RowIDCheckImpl::ByBitSet::checkRowID -- 
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
RowIDCheckImpl::ByBitSet::
checkRowID(Interface::IProgram& cProgram_,
		   unsigned int iRowID_)
{
	return m_cBitSet->test(iRowID_) ? Boolean::True : Boolean::False;
}

///////////////////////////////////////////
// Predicate::RowIDCheckImpl::ByCollection

// FUNCTION public
//	Predicate::RowIDCheckImpl::ByCollection::getClassID -- 
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
RowIDCheckImpl::ByCollection::
getClassID() const
{
	return Class::getClassID(Class::Category::RowIDCheckByCollection);
}

// FUNCTION public
//	Predicate::RowIDCheckImpl::ByCollection::serialize -- 
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
RowIDCheckImpl::ByCollection::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cResult.serialize(archiver_);
	m_cInData.serialize(archiver_);
	m_cInRowID.serialize(archiver_);
	m_cCollection.serialize(archiver_);
}

// FUNCTION private
//	Predicate::RowIDCheckImpl::ByCollection::explainOperator -- 
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
RowIDCheckImpl::ByCollection::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::ByCollection]);
}

// FUNCTION private
//	Predicate::RowIDCheckImpl::ByCollection::initializeData -- 
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
RowIDCheckImpl::ByCollection::
initializeData(Interface::IProgram& cProgram_)
{
	// bitset holding all rowids
	Common::BitSet cBitSet;
	Common::BitSet* pPrevBitSet = getPrevBitSet();

	if (pPrevBitSet && pPrevBitSet->none()) return false;

	m_cResult.initialize(cProgram_);
	m_cInData.initialize(cProgram_);
	m_cInRowID.initialize(cProgram_);
	m_cCollection.initialize(cProgram_);
	m_cCollection.prepareGetInterface();
	m_cCollection.preparePutInterface();

	// iterate to obtain all data
	cProgram_.initialize(getIterator());
	try {
		if (cProgram_.startUp(getIterator()) != Action::Status::Break) {
			while (cProgram_.next(getIterator())) {
				// check previous bitset
				if (pPrevBitSet
					&& pPrevBitSet->test(m_cInRowID->getValue()) == false) {
					continue;
				}
				// put tuple to map
				m_cCollection.put(cProgram_);
				int iPosition = m_cCollection.getLastPosition();
				m_mapTuple.insert(m_cInRowID->getValue(),
								  iPosition);
				cBitSet.set(m_cInRowID->getValue());
			}
			if (pPrevBitSet) {
				*pPrevBitSet &= cBitSet;
			}
		}
		cProgram_.finish(getIterator());
	} catch (...) {
		try {
			cProgram_.terminate(getIterator());
		} catch (...) {
			// ignore
		}
		_SYDNEY_RETHROW;
	}

	return !m_mapTuple.ISEMPTY();
}

// FUNCTION private
//	Predicate::RowIDCheckImpl::ByCollection::terminateData -- 
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
RowIDCheckImpl::ByCollection::
terminateData(Interface::IProgram& cProgram_)
{
	m_cResult.terminate(cProgram_);
	m_cInData.terminate(cProgram_);
	m_cInRowID.terminate(cProgram_);
	m_mapTuple.clear();
}

// FUNCTION private
//	Predicate::RowIDCheckImpl::ByCollection::checkRowID -- 
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
RowIDCheckImpl::ByCollection::
checkRowID(Interface::IProgram& cProgram_,
		   unsigned int iRowID_)
{
	TupleMap::Iterator found = m_mapTuple.find(iRowID_);
	if (found != m_mapTuple.end()) {
		int iPosition = (*found).second;
		if (m_cCollection.get(cProgram_, iPosition)) {
			// assign data
			Utility::DataType::assignElements(m_cResult.get(),
											  _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
																   m_cCollection.getData()));
			return Boolean::True;
		}
	}
	return Boolean::False;
}

/////////////////////////////////////
// Predicate::RowIDCheck::ByBitSet

// FUNCTION public
//	Predicate::RowIDCheck::ByBitSet::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iIteratorID_
//	int iRowIDID_
//	int iBitSetID_
//	int iPrevBitSetID_
//	
// RETURN
//	RowIDCheck*
//
// EXCEPTIONS

//static
RowIDCheck*
RowIDCheck::ByBitSet::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIteratorID_,
	   int iRowIDID_,
	   int iBitSetID_,
	   int iPrevBitSetID_)
{
	AUTOPOINTER<This> pResult =
		new RowIDCheckImpl::ByBitSet(iIteratorID_,
									 iRowIDID_,
									 iBitSetID_,
									 iPrevBitSetID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Predicate::RowIDCheck::ByCollection

// FUNCTION public
//	Predicate::RowIDCheck::ByCollection::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iIteratorID_
//	const PAIR<int, int>& cRowIDID_
//	const PAIR<int, int>& cTupleID_
//	int iPrevBitSetID_
//	int iCollectionID_
//	
// RETURN
//	RowIDCheck*
//
// EXCEPTIONS

//static
RowIDCheck*
RowIDCheck::ByCollection::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIteratorID_,
	   const PAIR<int, int>& cRowIDID_,
	   const PAIR<int, int>& cTupleID_,
	   int iPrevBitSetID_,
	   int iCollectionID_)
{
	AUTOPOINTER<This> pResult =
		new RowIDCheckImpl::ByCollection(iIteratorID_,
										 cRowIDID_.first, // checked rowid
										 cTupleID_.first, // result data
										 cRowIDID_.second,// rowid used in getting data
										 cTupleID_.second,// tuple used in getting data
										 iPrevBitSetID_,
										 iCollectionID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Predicate::RowIDCheck::

// FUNCTION public
//	Predicate::RowIDCheck::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	RowIDCheck*
//
// EXCEPTIONS

//static
RowIDCheck*
RowIDCheck::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::RowIDCheckByBitSet:
		{
			return new RowIDCheckImpl::ByBitSet;
		}
	case Class::Category::RowIDCheckByCollection:
		{
			return new RowIDCheckImpl::ByCollection;
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
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
