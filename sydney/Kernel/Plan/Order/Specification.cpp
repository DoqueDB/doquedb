// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/Specification.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Order";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Order/Specification.h"
#include "Plan/Order/Argument.h"
#include "Plan/Order/CheckedKey.h"
#include "Plan/Order/CheckedSpecification.h"
#include "Plan/Order/GeneratedSpecification.h"
#include "Plan/Order/Key.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"

#include "Common/Assert.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Execution/Interface/IProgram.h"

#include "Exception/Unexpected.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

namespace
{
	const char* const _pszExplainName[] = {
		"order by ",
		"group by "
	};
}

namespace SpecificationImpl
{
	// TEMPLATE CLASS
	//	SpecificationImpl::Base
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_ 
	//
	// NOTES
	template <class Handle_>
	class Base
		: public Handle_
	{
	public:
		typedef Base<Handle_> This;
		typedef Handle_ Super;

		// destructor
		virtual ~Base() {}

	///////////////////////////
	// Order::Specification::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Opt::Explain& cExplain_)
		virtual Specification::Size getKeySize() {return getSize();}
		virtual Order::Key* getKey(Specification::Position iPos_) {return getOperandi(iPos_);}

		virtual void setPartitionKey(const VECTOR<Interface::IScalar*>& vecPartitionKey_)
		{m_vecPartitionKey = vecPartitionKey_;}
		virtual const VECTOR<Interface::IScalar*>& getPartitionKey()
		{return m_vecPartitionKey;}

		virtual bool isRefering(Interface::IRelation* pRelation_,
								Specification::Position iPosition_)
		{
			Order::Key* pKey = getKey(iPosition_);
			return pKey && pKey->getScalar()->isRefering(pRelation_);
		}

		virtual bool hasAlternativeValue(Opt::Environment& cEnvironment_)
		{
			if (getKeySize() > 0) {
				if (getKey(0)->getScalar()->isField()) {
					return getKey(0)->getScalar()->getField()->hasAlternativeValue(cEnvironment_);
				}
			}
			return false;
		}

		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_)
		{
			foreachOperand(boost::bind(&Key::require,
									   _1,
									   boost::ref(cEnvironment_),
									   pCandidate_));
		}

		virtual bool isBitSetSort() {return m_bBitSetSort;}
		virtual void setBitSetSort() {m_bBitSetSort = true;}

		virtual bool hasExpandElement()
		{
			Specification::Size num = getKeySize();
			for ( int i = 0; i < num; i++ ) {
				if (getKey(i)->getScalar()->isField()) {
					Scalar::Field* pField = getKey(i)->getScalar()->getField();
					if (pField->isExpandElement()) {
						return true;
					}
				}
			}
			return false;
		}

		virtual bool isWordGrouping()
		{
			if (getKeySize() == 1
				&& getKey(0)->getScalar()->isField()
				&& getKey(0)->getScalar()->getField()->isFunction()
				&& getKey(0)->getScalar()->getField()->
				getFunction()->getType() == Plan::Tree::Node::Word) {
				return true;
			} else {
				return false;
			}
		}

		virtual bool hasFunctionKey()
		{
			return isAny(boost::bind(&Key::isFunction,
									 _1));
		}

		virtual bool isAscending()
		{
			return getKeySize() > 0
				&& getKey(0)->getDirection() == Order::Direction::Ascending;
		}

		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const
		{
			OSTRSTREAM cStream;
			char cSEPARTE = ' ';
			for (int i = 0; i < getSize(); ++i) {
				cStream << cSEPARTE << getOperandi(i)->toSQLStatement(cEnvironment_, cArgument_);
				cSEPARTE = ',';
			}
			return cStream.getString();
		}

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_)
		{
			char cSEPARTE = ' ';
			for (int i = 0; i < getSize(); ++i) {
				cExec_.append(cSEPARTE);
				getOperandi(i)->setParameter(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cExec_,
											 cArgument_);
				cSEPARTE = ',';
			}
		}
			
	protected:
		// constructor
		Base(typename Handle_::Argument cArgument_)
			: Super(cArgument_),
			  m_bBitSetSort(false)
		{}

		int generateScalar(Opt::Environment& cEnvironment_,
						   Execution::Interface::IProgram& cProgram_,
						   Execution::Interface::IIterator* pIterator_,
						   Candidate::AdoptArgument& cArgument_,
						   Interface::IScalar* pScalar_,
						   const Candidate::RowDelayArgument& cDelayArgument_)
		{
			return cDelayArgument_.m_bGenerate
				? pScalar_->generate(cEnvironment_, cProgram_, pIterator_, cArgument_)
				: pScalar_->generateFromType(cEnvironment_, cProgram_, pIterator_, cArgument_);
		}

	private:
		VECTOR<Interface::IScalar*> m_vecPartitionKey;
		bool m_bBitSetSort;
	};

	

	
	// CLASS
	//	SpecificationImpl::Monadic
	//
	// NOTES

	class Monadic
		: public Base< Tree::Monadic<Specification, Order::Key> >
	{
	public:
		typedef Monadic This;
		typedef Base< Tree::Monadic<Specification, Order::Key> > Super;

		// constructor
		Monadic(Order::Key* pKey_)
			: Super(pKey_)
		{}
		// destructor
		~Monadic() {}

	///////////////////////////
	// Order::Specification::
		virtual GeneratedSpecification* generate(Opt::Environment& cEnvironment_,
												 Execution::Interface::IProgram& cProgram_,
												 Execution::Interface::IIterator* pIterator_,
												 Candidate::AdoptArgument& cArgument_,
												 Candidate::Row* pRow_,
												 const Candidate::RowDelayArgument& cDelayArgument_);
		virtual int generateKey(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_);
		virtual Specification* check(Opt::Environment& cEnvironment_,
									 const CheckArgument& cArgument_);
	protected:
	private:
	///////////////////////////
	// Order::Specification::
		virtual void explainOperand(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);

		bool isWordFieldType(Interface::IScalar* pScalar_);
	};

	// CLASS
	//	SpecificationImpl::MonadicWithMonadicOption
	//
	// NOTES
	class MonadicWithMonadicOption
		: public Tree::MonadicOption<Monadic, Interface::IScalar>
	{
	public:
		typedef MonadicWithMonadicOption This;
		typedef Tree::MonadicOption<Monadic, Interface::IScalar> Super;

		// constructor
		MonadicWithMonadicOption(Order::Key* pKey_, Interface::IScalar* pOption_)
			: Super(pKey_, pOption_)
		{}
		// destructor
		~MonadicWithMonadicOption() {}

		Interface::IScalar* getGroupByOption() {return getOption();}

		///////////////////////////
		// Order::Specification::
		virtual bool isGroupBy() {return true;}
		
	};

	// CLASS
	//	SpecificationImpl::Nadic
	//
	// NOTES

	class Nadic
		: public Base< Tree::Nadic<Specification, Order::Key> >
	{
	public:
		typedef Nadic This;
		typedef Base< Tree::Nadic<Specification, Order::Key> > Super;

		// constructor
		Nadic(const VECTOR<Order::Key*>& vecKey_)
			: Super(vecKey_)
		{}
		// destructor
		~Nadic() {}

	///////////////////////////
	// Order::Specification::
		virtual GeneratedSpecification* generate(Opt::Environment& cEnvironment_,
												 Execution::Interface::IProgram& cProgram_,
												 Execution::Interface::IIterator* pIterator_,
												 Candidate::AdoptArgument& cArgument_,
												 Candidate::Row* pRow_,
												 const Candidate::RowDelayArgument& cDelayArgument_);
		virtual int generateKey(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_);
		virtual Specification* check(Opt::Environment& cEnvironment_,
									 const CheckArgument& cArgument_);
	protected:
	private:
	///////////////////////////
	// Order::Specification::
		virtual void explainOperand(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);
	};
} // namespace

////////////////////////////////////
// SpecificationImpl::Base

////////////////////////////////////
// SpecificationImpl::Monadic

// FUNCTION public
//	Order::SpecificationImpl::Monadic::generate -- generate tuple data and position/direction vector
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::Row* pRow_
//	const Candidate::RowDelayArgument& cDelayArgument_
//	
// RETURN
//	GeneratedSpecification*
//
// EXCEPTIONS

//virtual
GeneratedSpecification*
SpecificationImpl::Monadic::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_,
		 Candidate::Row* pRow_,
		 const Candidate::RowDelayArgument& cDelayArgument_)
{
	VECTOR<int> vecID;
	VECTOR<Interface::IScalar*> vecScalar;
	VECTOR<int> vecPosition;
	VECTOR<int> vecDirection;
	VECTOR<int> vecWordPosition;

	vecID.reserve(pRow_->getSize() + 1);
	vecScalar.reserve(pRow_->getSize() + 1);

	bool bFound = false;
	Candidate::Row::Iterator iterator = pRow_->begin();
	const Candidate::Row::Iterator last = pRow_->end();
	for (; iterator != last; ++iterator) {
		Interface::IScalar* pScalar = *iterator;
		if (!bFound
			&& pScalar->isEquivalent(getOperand()->getScalar())) {
			// key is found in the row
			bFound = true;
			vecPosition.PUSHBACK(vecID.GETSIZE());
			vecDirection.PUSHBACK(getOperand()->getDirection() == Direction::Descending ? 1 : 0);
			vecWordPosition.PUSHBACK(0);
			vecID.PUSHBACK(generateScalar(cEnvironment_, cProgram_, pIterator_, cArgument_,
										  pScalar, cDelayArgument_));
			vecScalar.PUSHBACK(pScalar);

		} else if (!bFound
				   && isWordFieldType(pScalar)) {
			bFound = true;
			vecPosition.PUSHBACK(vecID.GETSIZE());
			vecDirection.PUSHBACK(getOperand()->getDirection() == Direction::Descending ? 1 : 0);
			vecWordPosition.PUSHBACK(getOperand()->getScalar()->getField()->getFunction()->getType());
			vecID.PUSHBACK(generateScalar(cEnvironment_, cProgram_, pIterator_, cArgument_,
										  pScalar, cDelayArgument_));
			vecScalar.PUSHBACK(pScalar);
		} else if (cDelayArgument_.m_cKey.isEmpty()
				   || (!cDelayArgument_.m_cKey.isContaining(pScalar)
					   && cDelayArgument_.m_cNotDelayed.isContaining(pScalar))) {
			// otherwise, scalar is included in result only when it is not delayed
			vecID.PUSHBACK(generateScalar(cEnvironment_, cProgram_, pIterator_, cArgument_,
										  pScalar, cDelayArgument_));
			vecScalar.PUSHBACK(pScalar);
		}
	}
	if (!cDelayArgument_.m_cKey.isEmpty()) {
		vecScalar.insert(vecScalar.end(),
						 cDelayArgument_.m_cKey.begin(),
						 cDelayArgument_.m_cKey.end());
		cDelayArgument_.m_cKey.mapElement(vecID,
										  boost::bind(&This::generateScalar,
													  this,
													  boost::ref(cEnvironment_),
													  boost::ref(cProgram_),
													  pIterator_,
													  boost::ref(cArgument_),
													  _1,
													  boost::cref(cDelayArgument_)));
	}

	if (!bFound) {
		// no key is included in the row
		vecPosition.PUSHBACK(vecID.GETSIZE());
		vecDirection.PUSHBACK(getOperand()->getDirection() == Direction::Descending ? 1 : 0);
		vecWordPosition.PUSHBACK(0);
		vecID.PUSHBACK(generateScalar(cEnvironment_, cProgram_, pIterator_, cArgument_,
									  getOperand()->getScalar(), cDelayArgument_));
		vecScalar.PUSHBACK(getOperand()->getScalar());
	}

	return GeneratedSpecification::create(cEnvironment_,
										  this,
										  vecID,
										  vecPosition,
										  vecDirection,
										  vecScalar,
										  vecWordPosition);
}

// FUNCTION public
//	Order::SpecificationImpl::Monadic::generateKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
bool
SpecificationImpl::Monadic::
isWordFieldType(Interface::IScalar* pScalar_)
{
	if (pScalar_->isField()
		&& pScalar_->getField()->isFunction()
		&& pScalar_->getField()->getFunction()->getType() == Tree::Node::Word) {
		
		if (getOperand()->getScalar()->isField()
			&& getOperand()->getScalar()->getField()->isFunction()) {
			Interface::IScalar* pFunction = getOperand()->getScalar()->getField()->getFunction();
			if (pFunction->getType() == Tree::Node::WordDf
				|| pFunction->getType()  == Tree::Node::WordScale) {
				return true;
			}
		}
	}
	return false;
}





// FUNCTION public
//	Order::SpecificationImpl::Monadic::generateKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
SpecificationImpl::Monadic::
generateKey(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecID;
	vecID.PUSHBACK(getOperand()->getScalar()->generate(cEnvironment_,
													   cProgram_,
													   pIterator_,
													   cArgument_));
	return cProgram_.addVariable(vecID);
}

// FUNCTION public
//	SpecificationImpl::Monadic::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Specification*
//
// EXCEPTIONS

//virtual
Specification*
SpecificationImpl::Monadic::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	Order::Key* pKey = getOperand();
	if (Interface::IScalar::Check::isOn(pKey->getScalar()->check(cEnvironment_,
																 cArgument_),
										Interface::IScalar::Check::Referred)) {
		; _SYDNEY_ASSERT(!pKey->isChecked());
		Order::Key* pChecked = pKey->check(cEnvironment_,
										   cArgument_);
		if (pChecked) {
			; _SYDNEY_ASSERT(pChecked->isChecked());
			return CheckedSpecification::create(cEnvironment_,
												this,
												pChecked,
												pChecked->getChecked()->getFile());
		}
	}
	// no relationships with pTable_
	// or no index files can be used
	return 0;
}

// FUNCTION private
//	Order::SpecificationImpl::Monadic::explainOperand -- 
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

//virtual
void
SpecificationImpl::Monadic::
explainOperand(Opt::Environment* pEnvironment_,
			   Opt::Explain& cExplain_)
{
	getOperand()->explain(pEnvironment_, cExplain_);
}




////////////////////////////////////
// SpecificationImpl::Nadic

// FUNCTION public
//	Order::SpecificationImpl::Nadic::generate -- generate tuple data and position/direction vector
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::Row* pRow_
//	const Candidate::RowDelayArgument& cDelayArgument_
//	
// RETURN
//	GeneratedSpecification*
//
// EXCEPTIONS

//virtual
GeneratedSpecification*
SpecificationImpl::Nadic::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_,
		 Candidate::Row* pRow_,
		 const Candidate::RowDelayArgument& cDelayArgument_)
{
	VECTOR<int> vecID;
	VECTOR<Interface::IScalar*> vecScalar;
	VECTOR<int> vecPosition;
	VECTOR<int> vecDirection;

	vecID.reserve(pRow_->getSize() + 1);
	vecScalar.reserve(pRow_->getSize() + 1);

	int n = getSize();
	VECTOR<bool> vecFound(n, false);
	vecPosition.assign(n, 0);
	vecDirection.assign(n, 0);

	Candidate::Row::Iterator iterator = pRow_->begin();
	const Candidate::Row::Iterator last = pRow_->end();
	for (; iterator != last; ++iterator) {
		Interface::IScalar* pScalar = *iterator;
		int iFound = -1;
		for (int i = 0; i < n; ++i) {
			if (getOperandi(i)->getScalar() == pScalar) {
				iFound = i;
				break;
			}
		}
		if (iFound >= 0) {
			vecFound[iFound] = true;
			vecPosition[iFound] = vecID.GETSIZE();
			vecDirection[iFound] = (getOperandi(iFound)->getDirection() == Direction::Descending ? 1 : 0);
			vecID.PUSHBACK(generateScalar(cEnvironment_, cProgram_, pIterator_, cArgument_,
										  pScalar, cDelayArgument_));
			vecScalar.PUSHBACK(pScalar);

		} else if (cDelayArgument_.m_cKey.isEmpty()
				   || (!cDelayArgument_.m_cKey.isContaining(pScalar)
					   && cDelayArgument_.m_cNotDelayed.isContaining(pScalar))) {
			vecID.PUSHBACK(generateScalar(cEnvironment_, cProgram_, pIterator_, cArgument_,
										  pScalar, cDelayArgument_));
			vecScalar.PUSHBACK(pScalar);
		}
	}
	if (!cDelayArgument_.m_cKey.isEmpty()) {
		vecScalar.insert(vecScalar.end(),
						 cDelayArgument_.m_cKey.begin(),
						 cDelayArgument_.m_cKey.end());
		cDelayArgument_.m_cKey.mapElement(vecID,
										  boost::bind(&This::generateScalar,
													  this,
													  boost::ref(cEnvironment_),
													  boost::ref(cProgram_),
													  pIterator_,
													  boost::ref(cArgument_),
													  _1,
													  boost::cref(cDelayArgument_)));
	}
	for (int i = 0; i < n; ++i) {
		if (!vecFound[i]) {
			// no key is included in the row
			vecPosition[i] = vecID.GETSIZE();
			vecDirection[i] =(getOperandi(i)->getDirection() == Direction::Descending ? 1 : 0);
			vecID.PUSHBACK(generateScalar(cEnvironment_, cProgram_, pIterator_, cArgument_,
										  getOperandi(i)->getScalar(), cDelayArgument_));
			vecScalar.PUSHBACK(getOperandi(i)->getScalar());
		}
	}

	return GeneratedSpecification::create(cEnvironment_,
										  this,
										  vecID,
										  vecPosition,
										  vecDirection,
										  vecScalar);
}

// FUNCTION public
//	Order::SpecificationImpl::Nadic::generateKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
SpecificationImpl::Nadic::
generateKey(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecID;
	Opt::MapContainer(begin(),
					  end(),
					  vecID,
					  boost::bind(&Interface::IScalar::generate,
								  boost::bind(&Order::Key::getScalar,
											  _1),
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator_,
								  boost::ref(cArgument_)));
	return cProgram_.addVariable(vecID);
}

// FUNCTION public
//	SpecificationImpl::Nadic::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Specification*
//
// EXCEPTIONS

//virtual
Specification*
SpecificationImpl::Nadic::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	VECTOR<Order::Key*> vecChecked;
	Position iEnd = getSize();
	Direction::Value eDirection = Direction::Unknown;
	Utility::FileSet cFile;
	for (Position i = 0; i < iEnd; ++i) {
		Order::Key* pKey = getOperandi(i);

		if (eDirection == Direction::Unknown) {
			eDirection = pKey->getDirection();
		} else if (pKey->getDirection() != Direction::Unknown
				   && pKey->getDirection() != eDirection) {
			// index can be used only when direction is same
			break;
		}
		if (Interface::IScalar::Check::isOn(pKey->getScalar()->check(cEnvironment_,
																	 cArgument_),
											Interface::IScalar::Check::Referred)) {
			; _SYDNEY_ASSERT(!pKey->isChecked());
			Order::Key* pChecked = pKey->check(cEnvironment_,
											   cArgument_);
			if (pChecked) {
				; _SYDNEY_ASSERT(pChecked->isChecked());
				Utility::FileSet cTmp = pChecked->getChecked()->getFile();
				if (i > 0) {
					cTmp.intersect(cFile);
				}
				if (cTmp.isEmpty()) {
					break;
				}
				cFile = cTmp;
				vecChecked.PUSHBACK(pChecked);
			} else {
				// no index files can be used
				break;
			}
		} else {
			// no relationships with target table
			break;
		}
	}
	if (vecChecked.ISEMPTY() == false
		&& vecChecked.GETSIZE() < getSize()
		&& cArgument_.m_bCheckPartial == false) {
		Opt::ForEach(vecChecked,
					 boost::bind(&Order::Key::erase,
								 boost::ref(cEnvironment_),
								 _1));
		vecChecked.clear();
	}
	if (vecChecked.ISEMPTY()) {
		// all keys have no relationships with target table
		// or no keys can use index file
		return 0;
	}
	return CheckedSpecification::create(cEnvironment_,
										this,
										vecChecked,
										cFile);
}

// FUNCTION private
//	Order::SpecificationImpl::Nadic::explainOperand -- 
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

//virtual
void
SpecificationImpl::Nadic::
explainOperand(Opt::Environment* pEnvironment_,
			   Opt::Explain& cExplain_)
{
	joinOperand(boost::bind(&Operand::explain,
							_1,
							pEnvironment_,
							boost::ref(cExplain_)),
				boost::bind(&Opt::Explain::putChar,
							&cExplain_,
							','));
}

//////////////////////////
// Order::Specification

// FUNCTION public
//	Order::Specification::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Specification::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszExplainName[isGroupBy()?1:0]);
	explainOperand(pEnvironment_, cExplain_);
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Order::Specification::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Key* pKey_
//	
// RETURN
//	Specification*
//
// EXCEPTIONS

//static
Specification*
Specification::
create(Opt::Environment& cEnvironment_,
	   Order::Key* pKey_)
{
	AUTOPOINTER<This> pResult;
	pResult = new SpecificationImpl::Monadic(pKey_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Order::Specification::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Order::Key*>& vecKey_
//	
// RETURN
//	Specification*
//
// EXCEPTIONS

//static
Specification*
Specification::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Order::Key*>& vecKey_ , bool bGroupBy_)
{
	AUTOPOINTER<This> pResult;
	if (vecKey_.GETSIZE() == 1) {
		if(bGroupBy_) {
			Interface::IScalar* pOption = Scalar::Value::create(cEnvironment_, "GROUPBY");
			pOption->setNodeType(TreeNodeInterface::GroupBy);
			pResult = new SpecificationImpl::
				MonadicWithMonadicOption(*vecKey_.begin(), pOption);
		} else {
			pResult = new SpecificationImpl::Monadic(*vecKey_.begin());
		}
	} else {
		pResult = new SpecificationImpl::Nadic(vecKey_);
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();

}

// FUNCTION public
//	Order::Specification::getCompatible -- get compatible specification
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Specification* pSpecification1_
//	Specification* pSpecification2_
//	
// RETURN
//	Specification*
//
// EXCEPTIONS

//static
Specification*
Specification::
getCompatible(Opt::Environment& cEnvironment_,
			  Specification* pSpecification1_,
			  Specification* pSpecification2_)
{
	if (pSpecification1_ == 0) {
		return pSpecification2_;
	} else if (pSpecification2_ == 0) {
		return pSpecification1_;
	}

	VECTOR<Order::Key*> vecKey;
	int n1 = pSpecification1_->getKeySize();
	int n2 = pSpecification2_->getKeySize();
	int n = MIN(n1, n2);
	int iUse = n1 == n2 ? 0 : (n2 < n1 ? 1 : 2);
										// 0: use either, 1: use 1_, 2: use 2_, 3: create new
	for (int i = 0; i < n; ++i) {
		Order::Key* pKey1 = pSpecification1_->getKey(i);
		Order::Key* pKey2 = pSpecification2_->getKey(i);
		switch (Order::Key::compare(pKey1, pKey2)) {
		default:
		case -1:
			{
				// incompatible
				return 0;
			}
		case 0:
			{
				if (iUse == 3) vecKey.PUSHBACK(pKey1);
				break;
			}
		case 1:
			{
				if (iUse == 0) {
					iUse = 1;
				} else if (iUse == 2) {
					for (int j = 0; j < i; ++j) {
						vecKey.PUSHBACK(pSpecification2_->getKey(j));
					}
					vecKey.PUSHBACK(pKey1);
					iUse = 3;
				} else if (iUse == 3) {
					vecKey.PUSHBACK(pKey1);
				}
				break;
			}
		case 2:
			{
				if (iUse == 0) {
					iUse = 2;
				} else if (iUse == 1) {
					for (int j = 0; j < i; ++j) {
						vecKey.PUSHBACK(pSpecification1_->getKey(j));
					}
					vecKey.PUSHBACK(pKey2);
					iUse = 3;
				} else if (iUse == 3) {
					vecKey.PUSHBACK(pKey2);
				}
				break;
			}
		}
	}
	if (n < n1) {
		if (iUse == 0 || iUse == 1) {
			iUse = 1;
		} else {
			; _SYDNEY_ASSERT(iUse == 3);
			for (int j = n; j < n1; ++j) {
				vecKey.PUSHBACK(pSpecification1_->getKey(j));
			}
			iUse = 3;
		}
	}
	if (n < n2) {
		if (iUse == 0 || iUse == 2) {
			iUse = 2;
		} else {
			; _SYDNEY_ASSERT(iUse == 3);
			for (int j = n; j < n2; ++j) {
				vecKey.PUSHBACK(pSpecification2_->getKey(j));
			}
			iUse = 3;
		}
	}

	// create return value
	if (iUse == 0 || iUse == 1) {
		return pSpecification1_;
	} else if (iUse == 2) {
		return pSpecification2_;
	} else {
		return create(cEnvironment_, vecKey, false);
	}
}

// FUNCTION public
//	Order::Specification::isCompatible -- check compatibility
//
// NOTES
//	check first argument can be used instead of second argument.
//	even when first argument is partially replacable, it results in true.
//
// ARGUMENTS
//	Specification* pSpecification1_
//	Specification* pSpecification2_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Specification::
isCompatible(Specification* pSpecification1_,
			 Specification* pSpecification2_)
{
	if (pSpecification1_ == 0 || pSpecification2_ == 0) return false;

	int n1 = pSpecification1_->getKeySize();
	int n2 = pSpecification2_->getKeySize();
	int n = MIN(n1, n2);

	for (int i = 0; i < n; ++i) {
		Order::Key* pKey1 = pSpecification1_->getKey(i);
		Order::Key* pKey2 = pSpecification2_->getKey(i);
		switch (Order::Key::compare(pKey1, pKey2)) {
		default:
		case -1:
			{
				// incompatible
				return false;
			}
		case 0:
			{
				break;
			}
		case 1:
			{
				// key1 can be used instead of key2
				break;
			}
		case 2:
			{
				// key2 can be used instead of key1 but key1 cannot for key2
				return false;
			}
		}
	}
	return n > 0;
}

// FUNCTION public
//	Order::Specification::hasSamePartitionKey -- has same partition key?
//
// NOTES
//
// ARGUMENTS
//	Specification* pSpecification1_
//	Specification* pSpecification2_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Specification::
hasSamePartitionKey(Specification* pSpecification1_,
					Specification* pSpecification2_)
{
	if (pSpecification1_ == 0 || pSpecification2_ == 0) return false;

	const VECTOR<Interface::IScalar*>& vecPartitionKey1 = pSpecification1_->getPartitionKey();
	const VECTOR<Interface::IScalar*>& vecPartitionKey2 = pSpecification2_->getPartitionKey();

	SIZE np1 = vecPartitionKey1.GETSIZE();
	SIZE np2 = vecPartitionKey2.GETSIZE();
	if (np1 != np2) return false;
	if (np1 > 0) {
		for (SIZE ip1 = 0; ip1 < np1; ++ip1) {
			if (vecPartitionKey1[ip1] != vecPartitionKey2[ip1]) {
				// partition key is not equivalent
				return false;
			}
		}
	}
	return true;
}

// FUNCTION private
//	Order::Specification::registerToEnvironment -- register to environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Specification::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.addObject(this);
}

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
