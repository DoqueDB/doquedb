// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/MergeSort.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Execution/Iterator/MergeSort.h"
#include "Execution/Iterator/Class.h"
#include "Execution/Iterator/Nadic.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/ICollection.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/Serialize.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/Functional.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Sort.h"
#include "Opt/Trace.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "merge sort";
}

namespace Impl
{
	// CLASS local
	//	Execution::Iterator::Impl::MergeOperandComparator --
	//
	// NOTES
	class SortComparator
	{
	public:
		SortComparator()
			: m_iPosition(-1),
			  m_iDirection(-1)
		{}
		SortComparator(int iPosition_,
					   int iDirection_)
			: m_iPosition(iPosition_),
			  m_iDirection(iDirection_ == 0 ? 1 : -1)
		{}
		SortComparator(const SortComparator& cOther_)
			: m_iPosition(cOther_.m_iPosition),
			  m_iDirection(cOther_.m_iDirection)
		{}

		int compare(const Common::DataArrayData* p1_,
					const Common::DataArrayData* p2_)
		{
			return m_iDirection *
				p1_->getElement(m_iPosition)->compareTo(
									 p2_->getElement(m_iPosition).get());
		}


	protected:
	private:
		int m_iPosition;
		int m_iDirection;
		
	};

	// CLASS local
	//	Execution::Iterator::Impl::MergeSortImpl -- implementation class of MergeSort
	//
	// NOTES
	class MergeSortImpl
		: public Nadic<Iterator::MergeSort>
	{
	public:
		typedef MergeSortImpl This;
		typedef Nadic<Iterator::MergeSort> Super;

		MergeSortImpl()
			: Super(),
			  m_vecPosition(),
			  m_vecDirection(),
			  m_vecSortedOperand()
		{}
		~MergeSortImpl()
		{}

		bool compare(int iPos1_, int iPos2_);

	///////////////////////////
	//Execution::IIterator::MergeSort
		void virtual setSortParameter(const VECTOR<int>& vecPostion_, const VECTOR<int>& vecDirection_);
		
	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
	//	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
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

	private:
		void setResult(int iPos_);		
		VECTOR<int> m_vecPosition;
		VECTOR<int> m_vecDirection;
		VECTOR<int> m_vecSortedOperand;
		VECTOR<SortComparator> m_vecComparator;

	};
} // namespace Impl


/////////////////////////////////////////////
// Execution::Iterator::Impl::MergeSortImpl


// FUNCTION public
//	Iterator::Impl::MergeSortImpl::setSortParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Execution
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MergeSortImpl::
setSortParameter(const VECTOR<int>& vecPosition_, const VECTOR<int>& vecDirection_)
{
	m_vecPosition = vecPosition_;
	m_vecDirection = vecDirection_;
}
	
// FUNCTION public
//	Iterator::Impl::MergeSortImpl::initialize -- 
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
Impl::MergeSortImpl::
initialize(Interface::IProgram& cProgram_)
{
	// initialize super classes
	initializeBase(cProgram_);
	initializeOperand(cProgram_);
	if (getOutData() == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	
	if (m_vecComparator.ISEMPTY()) {
		for (unsigned int i = 0; i < m_vecPosition.GETSIZE(); i++) {
			int iKeyPosition = m_vecPosition[i];
			int iDirection = (m_vecDirection.GETSIZE() > i) ? m_vecDirection[i] : 0;
			m_vecComparator.PUSHBACK(SortComparator(iKeyPosition, iDirection));
		}
	}
}

// FUNCTION public
//	Iterator::Impl::MergeSortImpl::terminate -- 
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
Impl::MergeSortImpl::
terminate(Interface::IProgram& cProgram_)
{
	// terminate super classes
	terminateOperand(cProgram_);
	terminateBase(cProgram_);
	
	m_vecSortedOperand.clear();
}

// FUNCTION public
//	Iterator::Impl::MergeSortImpl::next -- go to next tuple
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
Impl::MergeSortImpl::
next(Interface::IProgram& cProgram_)
{
	if (!hasNext()) {
		return setHasData(false);
	}

	int n = getOperandSize();

	if (m_vecSortedOperand.ISEMPTY()) {
		// first call
		m_vecSortedOperand.reserve(n);
		bool bResult = false;
		for (int i = 0; i < n; ++i) {
			bResult = getElement(i).next(cProgram_) || bResult;
			m_vecSortedOperand.PUSHBACK(i);
		}
		if (bResult == false) {
			// all operands returns false at calling next 
			return setHasData(false);
		}
		
		// sort initialdata
		SORT(m_vecSortedOperand.begin(),
			 m_vecSortedOperand.end(),
			 boost::bind(&This::compare,
						 this,
						 _1,
						 _2));
	}

	
	// use result of first element
	; _SYDNEY_ASSERT(getElement(m_vecSortedOperand[0]).isHasData());
	setResult(m_vecSortedOperand[0]);
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Variable)) {
		_OPT_EXECUTION_MESSAGE
			<< "MergeSort: setResult from #" << m_vecSortedOperand[0]
			<< " = "
			<< Opt::Trace::toString(*getOutData())
			<< ModEndl;
	}
#endif

	int iTop = m_vecSortedOperand[0];
	if (getElement(iTop).next(cProgram_)) {
		int i = 1;
		for (; i < n; ++i) {
			if (compare(m_vecSortedOperand[i], iTop)) {
				// move i-th element forward
				m_vecSortedOperand[i - 1] = m_vecSortedOperand[i];
			} else {
				break;
			}
		}
		m_vecSortedOperand[i - 1] = iTop;
	} else {
		// set iTop to tail element
		ModCopy(m_vecSortedOperand.begin() + 1,
				m_vecSortedOperand.end(),
				m_vecSortedOperand.begin());
		m_vecSortedOperand[n - 1] = iTop;
	}

	if (getElement(m_vecSortedOperand[0]).isHasData() == false) {
		setHasNext(false);
	}
	
	return true;
}

	// FUNCTION public
//	Iterator::Impl::MergeSortImpl::reset -- 
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
Impl::MergeSortImpl::
reset(Interface::IProgram& cProgram_)
{
	if (!m_vecSortedOperand.ISEMPTY()) {
		resetOperand(cProgram_);
		m_vecSortedOperand.clear();
	}
	resetBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::MergeSortImpl::finish -- 
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
Impl::MergeSortImpl::
finish(Interface::IProgram& cProgram_)
{
	if (!m_vecSortedOperand.ISEMPTY()) {
		finishOperand(cProgram_);
		m_vecSortedOperand.clear();
	}
	finishBase(cProgram_);
}


// FUNCTION private
//	Iterator::Impl::UnionDistinctImpl::setResult -- set result data of i-th operand
//
// NOTES
//
// ARGUMENTS
//	int iPos_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::MergeSortImpl::
setResult(int iPos_)
{
	Common::Data::Pointer pCopy = getOperand()[iPos_].getData()->copy();
	Utility::DataType::assignElements(getOutData(), _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pCopy.get()));
}


// FUNCTION public
//	Iterator::Impl::MergeSortImpl::getClassID -- 
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
Impl::MergeSortImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::MergeSort);
}

// FUNCTION public
//	Iterator::Impl::MergeSortImpl::serialize -- 
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
Impl::MergeSortImpl::
serialize(ModArchive& archiver_)
{
	// serialize super classes
	serializeBase(archiver_);
	serializeOperand(archiver_);
	Utility::SerializeValue(archiver_, m_vecPosition);
	Utility::SerializeValue(archiver_, m_vecDirection);
	Utility::SerializeValue(archiver_, m_vecSortedOperand);
}



// FUNCTION protected
//	Iterator::Impl::MergeSortImpl::explainThis -- 
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
Impl::MergeSortImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
}

// FUNCTION private
//	Iterator::Impl::MergeSortImpl::compare -- compare
//
// NOTES
//
// ARGUMENTS
//	int iPos1_
//	int iPos2_
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::MergeSortImpl::
compare(int iPos1_, int iPos2_)
{
	const OperandElement& cElement1 = getOperand()[iPos1_];
	const OperandElement& cElement2 = getOperand()[iPos2_];

	if (cElement1.isHasData() != cElement2.isHasData()
		|| cElement1.isHasData() == false) {
		// if either of operand has no more data, set it to tail
		return cElement1.isHasData();
	}

	const Common::DataArrayData* p1 = cElement1.getData();
	const Common::DataArrayData* p2 = cElement2.getData();

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Variable)) {
		_OPT_EXECUTION_MESSAGE
			<< "MergeSort: compare "
			<< Opt::Trace::toString(*p1)
			<< " <=> "
			<< Opt::Trace::toString(*p2)
			<< ModEndl;
	}
#endif

	VECTOR<SortComparator>::ITERATOR iterator = m_vecComparator.begin();
	const VECTOR<SortComparator>::ITERATOR last = m_vecComparator.end();
	for (; iterator != last; ++iterator) {
		switch ((*iterator).compare(p1, p2)) {
		case 0:
			{
				continue;
			}
		case -1:
			{
				return true;
			}
		case 1:
			{
				return false;
			}
		default:
			{
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
	}
	return false;

}

//////////////////////////////
// Iterator::MergeSort::

// FUNCTION public
//	Iterator::MergeSort::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	MergeSort*
//
// EXCEPTIONS

//static
MergeSort*
MergeSort::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::MergeSortImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::MergeSort::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	MergeSort*
//
// EXCEPTIONS

//static
MergeSort*
MergeSort::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::MergeSort);
	return new Impl::MergeSortImpl;
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
