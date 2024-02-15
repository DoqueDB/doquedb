// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/Filter.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Execution/Iterator/Filter.h"
#include "Execution/Iterator/Class.h"
#include "Execution/Iterator/Monadic.h"
#include "Execution/Iterator/Nadic.h"

#include "Execution/Action/Collection.h"
#include "Execution/Interface/ICollection.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Exception/NotSupported.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/Functional.h"

#include "Opt/Explain.h"
#include "Opt/Sort.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	struct _Type
	{
		enum {
			Filter,
			Distribute,
			ValueNum
		};
	};

	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName[] = {
		"filter by ",
		"distribution filter "
	};
}

namespace Impl
{
	// CLASS local
	//	Execution::Iterator::Impl::FilterImpl -- implementation class of Filter
	//
	// NOTES
	class FilterImpl
		: public Monadic<Iterator::Filter>
	{
	public:
		typedef FilterImpl This;
		typedef Monadic<Iterator::Filter> Super;

		FilterImpl()
			: Super(),
			  m_cAggregation(),
			  m_cCollection()
		{}
		FilterImpl(int iCollectionID_)
			: Super(),
			  m_cAggregation(),
			  m_cCollection()
		{
			m_cCollection.setCollectionID(iCollectionID_);
		}
		~FilterImpl()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
	//	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
		virtual void finish(Interface::IProgram& cProgram_);
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

		virtual void addAggregation(Interface::IProgram& cProgram_,
									const Action::Argument& cAction_);
		virtual void addOutData(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);
		virtual void addInData(Interface::IProgram& cProgram_,
							   const Action::Argument& cAction_);
		virtual void addInput(Interface::IProgram& cProgram_,
							  const Action::Argument& cAction_);
	private:
		Action::ActionList m_cAggregation;
		Action::Collection m_cCollection;
	};

	class DistributionFilterImpl
		: public Nadic<Iterator::Filter>
	{
	public:
		typedef DistributionFilterImpl This;
		typedef Nadic<Iterator::Filter> Super;

		DistributionFilterImpl()
			: Super(),
			  m_cAggregation()
		{}

		~DistributionFilterImpl()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
	//	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
		virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();
	//	virtual int getOutData(Interface::IProgram& cProgram_);

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

		virtual void addAggregation(Interface::IProgram& cProgram_,
									const Action::Argument& cAction_);
		
		virtual void addInData(Interface::IProgram& cProgram_,
							   const Action::Argument& cAction_);		

	private:
		Action::ActionList m_cAggregation;
	};
}

/////////////////////////////////////////////
// Execution::Iterator::Impl::FilterImpl

// FUNCTION public
//	Iterator::Impl::FilterImpl::initialize -- 
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
Impl::FilterImpl::
initialize(Interface::IProgram& cProgram_)
{
	// initialize super classes
	initializeBase(cProgram_);
	initializeOperand(cProgram_);
	m_cCollection.initialize(cProgram_);
	m_cCollection.prepareGetInterface();
	m_cCollection.preparePutInterface();

	m_cAggregation.initialize(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::FilterImpl::terminate -- 
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
Impl::FilterImpl::
terminate(Interface::IProgram& cProgram_)
{
	// terminate super classes
	terminateBase(cProgram_);
	terminateOperand(cProgram_);
	m_cAggregation.terminate(cProgram_);
	m_cCollection.terminate(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::FilterImpl::next -- go to next tuple
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
Impl::FilterImpl::
next(Interface::IProgram& cProgram_)
{
	bool bResult = hasNext();
	if (hasNext()) {
		if (m_cAggregation.getSize() == 0) {
			// no aggregation

			if (m_cCollection.isGetNextOperand()) {
				// put operand data while collection.put returns true
				while (getOperand().next(cProgram_)) {
					if (m_cCollection.put(cProgram_) == false) {
						m_cCollection.shift(cProgram_);
					} else {
						break;
					}
				}
			}
			// get filtered data
			setHasNext(bResult = m_cCollection.get(cProgram_));

		} else if (m_cCollection.isEmptyGrouping()) {
			// with aggregation (no grouping keys)
			while (getOperand().next(cProgram_)) {
				m_cAggregation.accumulate(cProgram_);
			}
			m_cAggregation.execute(cProgram_);
			bResult = true;
			setHasNext(false);

		} else {
			// with aggregation (grouping)
			while (getOperand().next(cProgram_)) {
				if (m_cCollection.put(cProgram_) == false) {
					// put result false means more tuples should be put
					if (m_cCollection.get(cProgram_) == true) {
						m_cAggregation.accumulate(cProgram_);
					}
				} else {
					break;
				}
			}
			// get filtered data
			if (setHasNext(m_cCollection.get(cProgram_))) {
				m_cAggregation.accumulate(cProgram_);
			}
			if (hasNext()
				|| (m_cCollection.isEmptyGrouping() && m_cCollection.isEmpty())) {
				m_cAggregation.execute(cProgram_);
				bResult = true;
			} else {
				bResult = false;
			}
		}
	}
	return setHasData(bResult);
}

// FUNCTION public
//	Iterator::Impl::FilterImpl::reset -- 
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
Impl::FilterImpl::
reset(Interface::IProgram& cProgram_)
{
	if (m_cCollection.isInitialized()) {
		resetOperand(cProgram_);
		m_cCollection.clear();
		resetBase(cProgram_);
	}
}

// FUNCTION public
//	Iterator::Impl::FilterImpl::finish -- 
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
Impl::FilterImpl::
finish(Interface::IProgram& cProgram_)
{
	finishOperand(cProgram_);
	m_cCollection.clear();
	finishBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::FilterImpl::getOutData -- 
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
Impl::FilterImpl::
getOutData(Interface::IProgram& cProgram_)
{
	return m_cCollection.getGetDataID();
}

// FUNCTION public
//	Iterator::Impl::FilterImpl::getClassID -- 
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
Impl::FilterImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Filter);
}

// FUNCTION public
//	Iterator::Impl::FilterImpl::serialize -- 
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
Impl::FilterImpl::
serialize(ModArchive& archiver_)
{
	// serialize super classes
	serializeBase(archiver_);
	serializeOperand(archiver_);
	m_cCollection.serialize(archiver_);
	m_cAggregation.serialize(archiver_);
}

// FUNCTION protected
//	Iterator::Impl::FilterImpl::explainThis -- 
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
Impl::FilterImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::Filter]);
	m_cCollection.explain(pEnvironment_, cProgram_, cExplain_);
	m_cCollection.explainPutData(cProgram_, cExplain_);
	m_cCollection.explainGetData(cProgram_, cExplain_);
	if (m_cAggregation.getSize() > 0) {
		cExplain_.pushIndent().newLine(true);
		cExplain_.put("aggregation:").newLine();
		m_cAggregation.explain(pEnvironment_, cProgram_, cExplain_);
		cExplain_.popIndent();
	}
}

// FUNCTION protected
//	Iterator::Impl::FilterImpl::addAggregation -- 
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
Impl::FilterImpl::
addAggregation(Interface::IProgram& cProgram_,
			   const Action::Argument& cAction_)
{
	int iActionID = cAction_.getInstanceID();
	m_cAggregation.addID(iActionID);
}

// FUNCTION protected
//	Iterator::Impl::FilterImpl::addOutData -- 
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
Impl::FilterImpl::
addOutData(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	; _SYDNEY_ASSERT(m_cCollection.isGetDataValid() == false);

	// set collection get data
	m_cCollection.setGetDataID(cAction_.getInstanceID());
}

// FUNCTION protected
//	Iterator::Impl::FilterImpl::addInData -- 
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
Impl::FilterImpl::
addInData(Interface::IProgram& cProgram_,
		  const Action::Argument& cAction_)
{
	; _SYDNEY_ASSERT(m_cCollection.isPutDataValid() == false);
	m_cCollection.setPutDataID(cAction_.getInstanceID());
}

// FUNCTION protected
//	Iterator::Impl::FilterImpl::addInput -- 
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
Impl::FilterImpl::
addInput(Interface::IProgram& cProgram_,
		 const Action::Argument& cAction_)
{
	Super::addInput(cProgram_,
					cAction_);

	; _SYDNEY_ASSERT(m_cCollection.isPutDataValid() == false);
	m_cCollection.setPutDataID(cAction_.getArgumentID());
}



/////////////////////////////////////////////
// Execution::Iterator::Impl::DistributionFilterImpl

// FUNCTION public
//	Iterator::Impl::DistributionFilterImpl::initialize -- 
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
Impl::DistributionFilterImpl::
initialize(Interface::IProgram& cProgram_)
{
	// initialize super classes
	initializeBase(cProgram_);
	initializeOperand(cProgram_);
	m_cAggregation.initialize(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::DistributionFilterImpl::terminate -- 
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
Impl::DistributionFilterImpl::
terminate(Interface::IProgram& cProgram_)
{
	// terminate super classes
	terminateBase(cProgram_);
	terminateOperand(cProgram_);
	m_cAggregation.terminate(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::DistributionFilterImpl::next -- go to next tuple
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
Impl::DistributionFilterImpl::
next(Interface::IProgram& cProgram_)
{
	for (int i = 0; i < getOperandSize(); ++i) {
		if (getElement(i).next(cProgram_)) {
			setResult(i);
			m_cAggregation.accumulate(cProgram_);
		} else {
			// 各オペランドの行数は同じはず
			if (i != 0)	_SYDNEY_THROW0(Exception::Unexpected);
			return setHasData(false);
		}
	}
	
	m_cAggregation.execute(cProgram_);
	return setHasData(true);
}

// FUNCTION public
//	Iterator::Impl::DistributionFilterImpl::reset -- 
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
Impl::DistributionFilterImpl::
reset(Interface::IProgram& cProgram_)
{
	resetOperand(cProgram_);
	resetBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::DistributionFilterImpl::finish -- 
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
Impl::DistributionFilterImpl::
finish(Interface::IProgram& cProgram_)
{
	finishOperand(cProgram_);
	finishBase(cProgram_);
}


// FUNCTION public
//	Iterator::Impl::DistributionFilterImpl::getClassID -- 
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
Impl::DistributionFilterImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::DistributionFilter);
}

// FUNCTION public
//	Iterator::Impl::DistributionFilterImpl::serialize -- 
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
Impl::DistributionFilterImpl::
serialize(ModArchive& archiver_)
{
	// serialize super classes
	serializeBase(archiver_);
	serializeOperand(archiver_);
	m_cAggregation.serialize(archiver_);
}

// FUNCTION protected
//	Iterator::Impl::DistributionFilterImpl::explainThis -- 
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
Impl::DistributionFilterImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::Distribute]);
	if (m_cAggregation.getSize() > 0) {
		cExplain_.pushIndent().newLine(true);
		cExplain_.put("aggregation:").newLine();
		m_cAggregation.explain(pEnvironment_, cProgram_, cExplain_);
		cExplain_.popIndent();
	}
}

// FUNCTION protected
//	Iterator::Impl::DistributionFilterImpl::addAggregation -- 
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
Impl::DistributionFilterImpl::
addAggregation(Interface::IProgram& cProgram_,
			   const Action::Argument& cAction_)
{
	int iActionID = cAction_.getInstanceID();
	m_cAggregation.addID(iActionID);
}



// FUNCTION protected
//	Iterator::Impl::DistributionFilterImpl::addInData -- 
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
Impl::DistributionFilterImpl::
addInData(Interface::IProgram& cProgram_,
		  const Action::Argument& cAction_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}




//////////////////////////////
// Iterator::Filter::

// FUNCTION public
//	Iterator::Filter::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iCollectionID_
//	
// RETURN
//	Filter*
//
// EXCEPTIONS

//static
Filter*
Filter::
create(Interface::IProgram& cProgram_,
	   int iCollectionID_)
{
	AUTOPOINTER<This> pResult = new Impl::FilterImpl(iCollectionID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::Filter::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iCollectionID_
//	
// RETURN
//	Filter*
//
// EXCEPTIONS

//static
Filter*
Filter::Distribute::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::DistributionFilterImpl();
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::Filter::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Filter*
//
// EXCEPTIONS

//static
Filter*
Filter::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::Filter:
		return new Impl::FilterImpl;
	case Class::Category::DistributionFilter:
		return new Impl::DistributionFilterImpl;
	default:
		_SYDNEY_THROW0(Exception::Unexpected);
	}
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
