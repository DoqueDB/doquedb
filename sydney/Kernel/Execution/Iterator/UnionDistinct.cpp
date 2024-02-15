// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/UnionDistinct.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Execution/Iterator/UnionDistinct.h"
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
	const char* const _pszOperatorName = "union distinct";
}

namespace Impl
{
	// CLASS local
	//	Execution::Iterator::Impl::UnionOperandComparator --
	//
	// NOTES
	class UnionOperandComparator
	{
	public:
		UnionOperandComparator(const VECTOR<OperandElement>& vecOperand_)
			: m_vecOperand(vecOperand_)
		{}
		~UnionOperandComparator() {}

		bool operator()(int iPos1_, int iPos2_) const;

	protected:
	private:
		const VECTOR<OperandElement>& m_vecOperand;
	};

	// CLASS local
	//	Execution::Iterator::Impl::UnionDistinctImpl -- implementation class of UnionDistinct
	//
	// NOTES
	class UnionDistinctImpl
		: public Nadic<Iterator::UnionDistinct>
	{
	public:
		typedef UnionDistinctImpl This;
		typedef Nadic<Iterator::UnionDistinct> Super;

		UnionDistinctImpl()
			: Super(),
			  m_vecSortedOperand(),
			  m_cComparator(getOperand()),
			  m_pKey(0),
			  m_vecInputData(),
			  m_vecResultData(),
			  m_vecAlternativeData()
		{}
		~UnionDistinctImpl()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
	//	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
	//	virtual void finish(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);
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
	// Iterator::Nadic::
		virtual void explainOperand(Opt::Environment* pEnvironment_,
									Interface::IProgram& cProgram_,
									Opt::Explain& cExplain_);
	//////////////////////////////
	// Iterator::Base::
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
		virtual void addInData(Interface::IProgram& cProgram_,
							   const Action::Argument& cAction_);
	private:
		// set key data from i-th operand
		void setKey(int iPos_);
		// set result data from i-th operand
		void setResult(int iPos_);
		// set default data from i-th operand
		void setDefault(int iPos_);

		VECTOR<int> m_vecSortedOperand;
		UnionOperandComparator m_cComparator;

		Common::DataArrayData* m_pKey;
		VECTOR<Action::ArrayDataHolder> m_vecInputData;
		VECTOR<Action::ArrayDataHolder> m_vecResultData;
		VECTOR<Action::ArrayDataHolder> m_vecAlternativeData;
	};
} // namespace Impl

////////////////////////////////////////////////////////////
// Execution::Iterator::Impl::UnionOperandComparator

// FUNCTION public
//	Iterator::Impl::UnionOperandComparator::operator -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::UnionOperandComparator::
operator()(int iPos1_, int iPos2_) const
{
	const OperandElement& cElement1 = m_vecOperand[iPos1_];
	const OperandElement& cElement2 = m_vecOperand[iPos2_];

	if (cElement1.isHasData() != cElement2.isHasData()
		|| cElement1.isHasData() == false) {
		// if either of operand has no more data, set it to tail
		return cElement1.isHasData();
	}

	const Common::DataArrayData* p1 = cElement1.getData();
	const Common::DataArrayData* p2 = cElement2.getData();

	return p1->compareTo(p2) < 0;
}

/////////////////////////////////////////////
// Execution::Iterator::Impl::UnionDistinctImpl

// FUNCTION public
//	Iterator::Impl::UnionDistinctImpl::initialize -- 
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
Impl::UnionDistinctImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (getOperandSize() != m_vecInputData.GETSIZE()
		|| getOperandSize() != m_vecResultData.GETSIZE()
		|| getOperandSize() != m_vecAlternativeData.GETSIZE()) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	// initialize super classes
	initializeBase(cProgram_);
	initializeOperand(cProgram_);

	// first element of result should be key array
	m_pKey = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*,
								  getOutData()->getElement(0).get());
	if (m_pKey == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Opt::ForEach(m_vecInputData,
				 boost::bind(&Action::ArrayDataHolder::initialize,
							 _1,
							 boost::ref(cProgram_)));
	Opt::ForEach(m_vecResultData,
				 boost::bind(&Action::ArrayDataHolder::initialize,
							 _1,
							 boost::ref(cProgram_)));
	Opt::ForEach(m_vecAlternativeData,
				 boost::bind(&Action::ArrayDataHolder::initialize,
							 _1,
							 boost::ref(cProgram_)));
}

// FUNCTION public
//	Iterator::Impl::UnionDistinctImpl::terminate -- 
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
Impl::UnionDistinctImpl::
terminate(Interface::IProgram& cProgram_)
{
	// terminate super classes
	terminateOperand(cProgram_);
	terminateBase(cProgram_);

	Opt::ForEach(m_vecInputData,
				 boost::bind(&Action::ArrayDataHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	Opt::ForEach(m_vecResultData,
				 boost::bind(&Action::ArrayDataHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	Opt::ForEach(m_vecAlternativeData,
				 boost::bind(&Action::ArrayDataHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
}

// FUNCTION public
//	Iterator::Impl::UnionDistinctImpl::next -- go to next tuple
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
Impl::UnionDistinctImpl::
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
			 m_cComparator);
	}

	// use result of first element
	; _SYDNEY_ASSERT(getElement(m_vecSortedOperand[0]).isHasData());
	setKey(m_vecSortedOperand[0]);
	setResult(m_vecSortedOperand[0]);

	// set other operands results with same key
	const Common::DataArrayData* pKey = getElement(m_vecSortedOperand[0]).getData();
	int i = 1;
	while (i < n
		   && getElement(m_vecSortedOperand[i]).isHasData()
		   && pKey->equals(getElement(m_vecSortedOperand[i]).getData())) {
		setResult(m_vecSortedOperand[i]);
		++i;
	}
	int nSameKey = i;

	// set default value for operands results with different key
	for (; i < n; ++i) {
		setDefault(m_vecSortedOperand[i]);
	}

	// re-sort operands
	while (nSameKey > 0) {
		int iTop = m_vecSortedOperand[0];
		if (getElement(iTop).next(cProgram_)) {
			// move same keys forward
			if (nSameKey > 1)  {
				ModCopy(m_vecSortedOperand.begin() + 1,
						m_vecSortedOperand.begin() + nSameKey,
						m_vecSortedOperand.begin());
			}
			// insert top operand to appropriate position
			int i = nSameKey;
			for (; i < n; ++i) {
				if (m_cComparator(m_vecSortedOperand[i], iTop)) {
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
		--nSameKey;
	}
	if (getElement(m_vecSortedOperand[0]).isHasData() == false) {
		setHasNext(false);
	}

	return true;
}

// FUNCTION public
//	Iterator::Impl::UnionDistinctImpl::getClassID -- 
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
Impl::UnionDistinctImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::UnionDistinct);
}

// FUNCTION public
//	Iterator::Impl::UnionDistinctImpl::serialize -- 
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
Impl::UnionDistinctImpl::
serialize(ModArchive& archiver_)
{
	// serialize super classes
	serializeBase(archiver_);
	serializeOperand(archiver_);
	Utility::SerializeObject(archiver_, m_vecInputData);
	Utility::SerializeObject(archiver_, m_vecResultData);
	Utility::SerializeObject(archiver_, m_vecAlternativeData);
}

// FUNCTION protected
//	Iterator::Impl::UnionDistinctImpl::explainOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::UnionDistinctImpl::
explainOperand(Opt::Environment* pEnvironment_,
			   Interface::IProgram& cProgram_,
			   Opt::Explain& cExplain_)
{
	if (cExplain_.isOn(Opt::Explain::Option::Data) == false) {
		Super::explainOperand(pEnvironment_,
							  cProgram_,
							  cExplain_);
	} else {
		// put data for each operand
		if (getOperandSize() != m_vecInputData.GETSIZE()
			|| getOperandSize() != m_vecResultData.GETSIZE()) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		cExplain_.pushIndent();
		VECTOR<OperandElement>::ITERATOR iterator = getOperand().begin();
		const VECTOR<OperandElement>::ITERATOR last = getOperand().end();
		int i = 0;
		for (; iterator != last; ++iterator, ++i) {
			if (m_vecResultData[i].isValid()
				&& m_vecInputData[i].isValid()) {
				cExplain_.newLine(true /* force */).put("<- ");
				m_vecResultData[i].explain(cProgram_, cExplain_);
				cExplain_.put(" <- ");
				m_vecInputData[i].explain(cProgram_, cExplain_);
				cExplain_.pushIndent();
				(*iterator).explain(pEnvironment_, cProgram_, cExplain_);
				cExplain_.popIndent();
			} else {
				(*iterator).explain(pEnvironment_, cProgram_, cExplain_);
			}
		}
		cExplain_.popIndent();
	}
}

// FUNCTION protected
//	Iterator::Impl::UnionDistinctImpl::explainThis -- 
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
Impl::UnionDistinctImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
}

// FUNCTION protected
//	Iterator::Impl::UnionDistinctImpl::addInData -- 
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
Impl::UnionDistinctImpl::
addInData(Interface::IProgram& cProgram_,
		  const Action::Argument& cAction_)
{
	// argument might be -1
	m_vecInputData.PUSHBACK(Action::ArrayDataHolder(cAction_.getInstanceID()));
	m_vecResultData.PUSHBACK(Action::ArrayDataHolder(cAction_.getArgumentID()));
	m_vecAlternativeData.PUSHBACK(Action::ArrayDataHolder(cAction_.getOptionID()));
}

// FUNCTION private
//	Iterator::Impl::UnionDistinctImpl::setKey -- set key data from i-th operand
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
Impl::UnionDistinctImpl::
setKey(int iPos_)
{
	// operand's data should be key
	Utility::DataType::assignElements(m_pKey,
									  getElement(iPos_).getData());
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
Impl::UnionDistinctImpl::
setResult(int iPos_)
{
	Action::ArrayDataHolder& cInput = m_vecInputData[iPos_];
	Action::ArrayDataHolder& cResult = m_vecResultData[iPos_];

	if (cResult.isValid()) {
		Utility::DataType::assignElements(cResult.get(),
										  cInput.getData());
	}
}

// FUNCTION public
//	Iterator::Impl::UnionDistinctImpl::setDefault -- set default data of i-th operand
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
Impl::UnionDistinctImpl::
setDefault(int iPos_)
{
	Action::ArrayDataHolder& cResult = m_vecResultData[iPos_];
	Action::ArrayDataHolder& cAlternative = m_vecAlternativeData[iPos_];

	if (cResult.isValid()) {
		if (cAlternative.isValid()) {
			Utility::DataType::assignElements(cResult.get(),
											  cAlternative.getData());
		} else {
			Utility::DataType::setNullElements(cResult.get());
		}
	}
}

//////////////////////////////
// Iterator::UnionDistinct::

// FUNCTION public
//	Iterator::UnionDistinct::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	UnionDistinct*
//
// EXCEPTIONS

//static
UnionDistinct*
UnionDistinct::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::UnionDistinctImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::UnionDistinct::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	UnionDistinct*
//
// EXCEPTIONS

//static
UnionDistinct*
UnionDistinct::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::UnionDistinct);
	return new Impl::UnionDistinctImpl;
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
