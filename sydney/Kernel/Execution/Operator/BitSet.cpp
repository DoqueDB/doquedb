// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/BitSet.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Execution/Operator/BitSet.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/Argument.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/Locker.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Explain.h"

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
			Collection,
			ValueNum
		};
	};
	const char* const _pszExplainName = "bitset ";
	const char* const _pszOperatorName[] =
	{
		"intersect",
		"union",
		"difference",
		"collection",
	};
}

namespace BitSetImpl
{
	// CLASS local
	//	Execution::Operator::BitSetImpl::Base -- implementation class of BitSet
	//
	// NOTES
	class Base
		: public Operator::BitSet
	{
	public:
		typedef Base This;
		typedef Operator::BitSet Super;

		virtual ~Base() {}

	///////////////////////////
	// Operator::BitSet::

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
			  m_cBitSet(),
			  m_cMyBitSet(),
			  m_bResult(true)
		{}
		Base(int iBitSetID_,
			 int iMyBitSetID_)
			: Super(),
			  m_cBitSet(iBitSetID_),
			  m_cMyBitSet(iMyBitSetID_),
			  m_bResult(true)
		{}

		// combine my bitset
		virtual bool combine(Common::BitSet& cResult_,
							 const Common::BitSet& cMyResult_) = 0;

	private:
		// explain operator
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

		// bitset object
		Action::BitSetHolder m_cBitSet;
		// bitset data obtained from logical file
		Action::BitSetHolder m_cMyBitSet;

		// result of execution
		bool m_bResult;
	};

	// CLASS local
	//	Execution::Operator::BitSetImpl::Intersect -- implementation class of BitSet
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
		Intersect(int iBitSetID_,
				  int iMyBitSetID_)
			: Super(iBitSetID_, iMyBitSetID_)
		{}
		// destructor
		virtual ~Intersect() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	///////////////////////////////
	// Base
		virtual bool combine(Common::BitSet& cResult_,
							 const Common::BitSet& cMyResult_);
	private:
	///////////////////////////////
	// Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Operator::BitSetImpl::Union -- implementation class of BitSet
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
		Union(int iBitSetID_,
			  int iMyBitSetID_)
			: Super(iBitSetID_, iMyBitSetID_)
		{}
		// destructor
		virtual ~Union() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	///////////////////////////////
	// Base
		virtual bool combine(Common::BitSet& cResult_,
							 const Common::BitSet& cMyResult_);
	private:
	///////////////////////////////
	// Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Operator::BitSetImpl::Difference -- implementation class of BitSet
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
		Difference(int iBitSetID_,
				   int iMyBitSetID_)
			: Super(iBitSetID_, iMyBitSetID_)
		{}
		// destructor
		virtual ~Difference() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	///////////////////////////////
	// Base
		virtual bool combine(Common::BitSet& cResult_,
							 const Common::BitSet& cMyResult_);
	private:
	///////////////////////////////
	// Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// TEMPLATE CLASS
	//	Execution::Operator::BitSetImpl::UnlockMixin --
	//
	// TEMPLATE ARGUMENTS
	//	class Base_
	//
	// NOTES
	template <class Base_>
	class UnlockMixin
		: public Base_
	{
	public:
		typedef UnlockMixin<Base_> This;
		typedef Base_ Super;

		// constructor
		UnlockMixin()
			: Super(),
			  m_cLocker()
		{}
		UnlockMixin(int iBitSetID_,
					int iMyBitSetID_,
					int iLockerID_)
			: Super(iBitSetID_, iMyBitSetID_),
			  m_cLocker(iLockerID_)
		{}

		// destructor
		virtual ~UnlockMixin()
		{}

	/////////////////////////////
	// Interface::IAction::
		virtual void initialize(Interface::IProgram& cProgram_)
		{
			if (m_cLocker.isInitialized() == false) {
				Super::initialize(cProgram_);
				m_cLocker.initialize(cProgram_);
			}
		}
		virtual void terminate(Interface::IProgram& cProgram_)
		{
			if (m_cLocker.isInitialized()) {
				Super::terminate(cProgram_);
				m_cLocker.terminate(cProgram_);
			}
		}

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_)
		{
			Super::serialize(archiver_);
			m_cLocker.serialize(archiver_);
		}

	protected:
		void unlockAll(const Common::BitSet& cBitSet_)
		{
			m_cLocker->unlock(cBitSet_);
		}
	private:
		virtual void unlock(const Common::BitSet& cBitSet0_,
							const Common::BitSet& cBitSet1_) = 0;

	///////////////////////////////
	// Base
		virtual bool combine(Common::BitSet& cResult_,
							 const Common::BitSet& cMyResult_)
		{
			if (m_cLocker->isNeedLock()) {
				unlock(cResult_, cMyResult_);
			}
			return Super::combine(cResult_, cMyResult_);
		}

		Action::LockerHolder m_cLocker;
	};

	// CLASS
	//	Execution::Operator::BitSetImpl::IntersectUnlock --
	//
	// NOTES
	class IntersectUnlock
		: public UnlockMixin<Intersect>
	{
	public:
		typedef IntersectUnlock This;
		typedef UnlockMixin<Intersect> Super;

		// constructor
		IntersectUnlock()
			: Super()
		{}
		IntersectUnlock(int iBitSetID_,
						int iMyBitSetID_,
						int iLockerID_)
			: Super(iBitSetID_, iMyBitSetID_, iLockerID_)
		{}

		// destructor
		~IntersectUnlock() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////
	// UnlockMixin::
		virtual void unlock(const Common::BitSet& cBitSet0_,
							const Common::BitSet& cBitSet1_);
	};

	// CLASS
	//	Execution::Operator::BitSetImpl::UnionUnlock --
	//
	// NOTES
	class UnionUnlock
		: public UnlockMixin<Union>
	{
	public:
		typedef UnionUnlock This;
		typedef UnlockMixin<Union> Super;

		// constructor
		UnionUnlock()
			: Super()
		{}
		UnionUnlock(int iBitSetID_,
					int iMyBitSetID_,
					int iLockerID_)
			: Super(iBitSetID_, iMyBitSetID_, iLockerID_)
		{}

		// destructor
		~UnionUnlock() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////
	// UnlockMixin::
		virtual void unlock(const Common::BitSet& cBitSet0_,
							const Common::BitSet& cBitSet1_);
	};

	// CLASS
	//	Execution::Operator::BitSetImpl::DifferenceUnlock --
	//
	// NOTES
	class DifferenceUnlock
		: public UnlockMixin<Difference>
	{
	public:
		typedef DifferenceUnlock This;
		typedef UnlockMixin<Difference> Super;

		// constructor
		DifferenceUnlock()
			: Super()
		{}
		DifferenceUnlock(int iBitSetID_,
						 int iMyBitSetID_,
						 int iLockerID_)
			: Super(iBitSetID_, iMyBitSetID_, iLockerID_)
		{}

		// destructor
		~DifferenceUnlock() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////
	// UnlockMixin::
		virtual void unlock(const Common::BitSet& cBitSet0_,
							const Common::BitSet& cBitSet1_);
	};

	// CLASS local
	//	Execution::Operator::BitSetImpl::Collection -- implementation class of BitSet
	//
	// NOTES
	class Collection
		: public Operator::BitSet
	{
	public:
		typedef Collection This;
		typedef Operator::BitSet Super;

		// constructorr
		Collection()
			: Super(),
			  m_cData(),
			  m_cBitSet(),
			  m_pRowID(0)
		{}
		Collection(int iDataID_,
				   int iBitSetID_)
			: Super(),
			  m_cData(iDataID_),
			  m_cBitSet(iBitSetID_),
			  m_pRowID(0)
		{}

		virtual ~Collection() {}

	///////////////////////////
	// Operator::BitSet::

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
		// data obtained from subquery
		Action::ArrayDataHolder m_cData;
		// bitset object
		Action::BitSetHolder m_cBitSet;

		const Common::UnsignedIntegerData* m_pRowID;
	};
}

///////////////////////////////////////////////
// Execution::Operator::BitSetImpl::Base

// FUNCTION public
//	Operator::BitSetImpl::Base::explain -- 
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
BitSetImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName);

	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cBitSet.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}

	explainOperator(cProgram_, cExplain_);

	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cMyBitSet.explain(cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Operator::BitSetImpl::Base::initialize -- 
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
BitSetImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cBitSet.isInitialized() == false) {
		// prepare result bitset
		m_cBitSet.initialize(cProgram_);
		// prepare my bitset
		m_cMyBitSet.initialize(cProgram_);
		; _SYDNEY_ASSERT(m_cBitSet.isInitialized());
	}
}

// FUNCTION public
//	Operator::BitSetImpl::Base::terminate -- 
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
BitSetImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	m_cBitSet.terminate(cProgram_);
	m_cMyBitSet.terminate(cProgram_);
}

// FUNCTION public
//	Operator::BitSetImpl::Base::execute -- 
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
BitSetImpl::Base::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		m_bResult = combine(*m_cBitSet, *m_cMyBitSet);
		done();
	}
	return m_bResult ? Action::Status::Success : Action::Status::False;
}

// FUNCTION public
//	Operator::BitSetImpl::Base::finish -- 
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
BitSetImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::BitSetImpl::Base::reset -- 
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
BitSetImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::BitSetImpl::Base::serialize -- 
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
BitSetImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cBitSet.serialize(archiver_);
	m_cMyBitSet.serialize(archiver_);
}

////////////////////////////////////////////////////
// Execution::Operator::BitSetImpl::Intersect

// FUNCTION public
//	Operator::BitSetImpl::Intersect::getClassID -- 
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
BitSetImpl::Intersect::
getClassID() const
{
	return Class::getClassID(Class::Category::BitSetIntersect);
}

// FUNCTION private
//	Operator::BitSetImpl::Intersect::explainOperator -- 
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
BitSetImpl::Intersect::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::Intersect]);
}

// FUNCTION private
//	Operator::BitSetImpl::Intersect::combine -- combine my bitset
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
BitSetImpl::Intersect::
combine(Common::BitSet& cResult_,
		const Common::BitSet& cMyResult_)
{
	cResult_ &= cMyResult_;
	return cResult_.any();
}

////////////////////////////////////////////////////
// Execution::Operator::BitSetImpl::Union

// FUNCTION public
//	Operator::BitSetImpl::Union::getClassID -- 
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
BitSetImpl::Union::
getClassID() const
{
	return Class::getClassID(Class::Category::BitSetUnion);
}

// FUNCTION private
//	Operator::BitSetImpl::Union::explainOperator -- 
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
BitSetImpl::Union::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::Union]);
}

// FUNCTION private
//	Operator::BitSetImpl::Union::combine -- combine my bitset
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
BitSetImpl::Union::
combine(Common::BitSet& cResult_,
		const Common::BitSet& cMyResult_)
{
	cResult_ |= cMyResult_;
	return true;
}

////////////////////////////////////////////////////
// Execution::Operator::BitSetImpl::Difference

// FUNCTION public
//	Operator::BitSetImpl::Difference::getClassID -- 
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
BitSetImpl::Difference::
getClassID() const
{
	return Class::getClassID(Class::Category::BitSetDifference);
}

// FUNCTION private
//	Operator::BitSetImpl::Difference::explainOperator -- 
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
BitSetImpl::Difference::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::Difference]);
}

// FUNCTION private
//	Operator::BitSetImpl::Difference::combine -- combine my bitset
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
BitSetImpl::Difference::
combine(Common::BitSet& cResult_,
		const Common::BitSet& cMyResult_)
{
	cResult_ -= cMyResult_;
	return cResult_.any();
}

////////////////////////////////////////////////////////
// Execution::Operator::BitSetImpl::IntersectUnlock

// FUNCTION public
//	Operator::BitSetImpl::IntersectUnlock::getClassID -- 
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
BitSetImpl::IntersectUnlock::
getClassID() const
{
	return Class::getClassID(Class::Category::BitSetIntersectUnlock);
}

// FUNCTION private
//	Operator::BitSetImpl::IntersectUnlock::unlock -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::BitSet& cBitSet0_
//	const Common::BitSet& cBitSet1_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
BitSetImpl::IntersectUnlock::
unlock(const Common::BitSet& cBitSet0_,
	   const Common::BitSet& cBitSet1_)
{
	// unlock union
	Common::BitSet cBitSet(cBitSet0_);
	cBitSet |= cBitSet1_;

	unlockAll(cBitSet);
}

////////////////////////////////////////////////////////
// Execution::Operator::BitSetImpl::UnionUnlock

// FUNCTION public
//	Operator::BitSetImpl::UnionUnlock::getClassID -- 
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
BitSetImpl::UnionUnlock::
getClassID() const
{
	return Class::getClassID(Class::Category::BitSetUnionUnlock);
}

// FUNCTION private
//	Operator::BitSetImpl::UnionUnlock::unlock -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::BitSet& cBitSet0_
//	const Common::BitSet& cBitSet1_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
BitSetImpl::UnionUnlock::
unlock(const Common::BitSet& cBitSet0_,
	   const Common::BitSet& cBitSet1_)
{
	// unlock intersect
	Common::BitSet cBitSet(cBitSet0_);
	cBitSet &= cBitSet1_;

	unlockAll(cBitSet);
}

////////////////////////////////////////////////////////
// Execution::Operator::BitSetImpl::DifferenceUnlock

// FUNCTION public
//	Operator::BitSetImpl::DifferenceUnlock::getClassID -- 
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
BitSetImpl::DifferenceUnlock::
getClassID() const
{
	return Class::getClassID(Class::Category::BitSetDifferenceUnlock);
}

// FUNCTION private
//	Operator::BitSetImpl::DifferenceUnlock::unlock -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::BitSet& cBitSet0_
//	const Common::BitSet& cBitSet1_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
BitSetImpl::DifferenceUnlock::
unlock(const Common::BitSet& cBitSet0_,
	   const Common::BitSet& cBitSet1_)
{
	// unlock intersect and differed operand
	Common::BitSet cBitSet(cBitSet0_);
	cBitSet &= cBitSet1_;

	unlockAll(cBitSet);
	unlockAll(cBitSet1_);
}

///////////////////////////////////////////////
// Execution::Operator::BitSetImpl::Collection

// FUNCTION public
//	Operator::BitSetImpl::Collection::explain -- 
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
BitSetImpl::Collection::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName);

	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cBitSet.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}

	cExplain_.put(_pszOperatorName[_Type::Collection]);

	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" from ");
		m_cData.explain(cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Operator::BitSetImpl::Collection::initialize -- 
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
BitSetImpl::Collection::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cBitSet.isInitialized() == false) {
		// prepare result bitset
		m_cBitSet.initialize(cProgram_);
		// prepare data
		m_cData.initialize(cProgram_);
		; _SYDNEY_ASSERT(m_cBitSet.isInitialized());
		; _SYDNEY_ASSERT(m_cData.isInitialized());
		; _SYDNEY_ASSERT(m_cData->getCount() == 1);
		; _SYDNEY_ASSERT(m_cData->getElement(0)->getType() == Common::DataType::UnsignedInteger);

		m_pRowID = _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData*,
										m_cData->getElement(0).get());
		; _SYDNEY_ASSERT(m_pRowID);
	}
}

// FUNCTION public
//	Operator::BitSetImpl::Collection::terminate -- 
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
BitSetImpl::Collection::
terminate(Interface::IProgram& cProgram_)
{
	m_cBitSet.terminate(cProgram_);
	m_cData.terminate(cProgram_);
	m_pRowID = 0;
}

// FUNCTION public
//	Operator::BitSetImpl::Collection::execute -- 
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
BitSetImpl::Collection::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		; _SYDNEY_ASSERT(!m_pRowID->isNull());
		m_cBitSet->set(m_pRowID->getValue());
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::BitSetImpl::Collection::finish -- 
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
BitSetImpl::Collection::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::BitSetImpl::Collection::reset -- 
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
BitSetImpl::Collection::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::BitSetImpl::Collection::getClassID -- 
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
BitSetImpl::Collection::
getClassID() const
{
	return Class::getClassID(Class::Category::BitSetCollection);
}

// FUNCTION public
//	Operator::BitSetImpl::Collection::serialize -- 
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
BitSetImpl::Collection::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cData.serialize(archiver_);
	m_cBitSet.serialize(archiver_);
}

///////////////////////////////////////
// Operator::BitSet::Intersect

// FUNCTION public
//	Operator::BitSet::Intersect::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iBitSetID_
//	int iMyBitSetID_
//	
// RETURN
//	BitSet*
//
// EXCEPTIONS

//static
BitSet*
BitSet::Intersect::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iBitSetID_,
	   int iMyBitSetID_)
{
	AUTOPOINTER<This> pResult = 
		new BitSetImpl::Intersect(iBitSetID_,
								  iMyBitSetID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::BitSet::Intersect::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iBitSetID_
//	int iMyBitSetID_
//	int iLockerID_
//	
// RETURN
//	BitSet*
//
// EXCEPTIONS

//static
BitSet*
BitSet::Intersect::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iBitSetID_,
	   int iMyBitSetID_,
	   int iLockerID_)
{
	if (iLockerID_ < 0) {
		return create(cProgram_, pIterator_, iBitSetID_, iMyBitSetID_);
	}
	AUTOPOINTER<This> pResult = 
		new BitSetImpl::IntersectUnlock(iBitSetID_,
										iMyBitSetID_,
										iLockerID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

///////////////////////////////////////
// Operator::BitSet::Union

// FUNCTION public
//	Operator::BitSet::Union::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iBitSetID_
//	int iMyBitSetID_
//	
// RETURN
//	BitSet*
//
// EXCEPTIONS

//static
BitSet*
BitSet::Union::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iBitSetID_,
	   int iMyBitSetID_)
{
	AUTOPOINTER<This> pResult = 
		new BitSetImpl::Union(iBitSetID_,
							  iMyBitSetID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::BitSet::Union::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iBitSetID_
//	int iMyBitSetID_
//	int iLockerID_
//	
// RETURN
//	BitSet*
//
// EXCEPTIONS

//static
BitSet*
BitSet::Union::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iBitSetID_,
	   int iMyBitSetID_,
	   int iLockerID_)
{
	if (iLockerID_ < 0) {
		return create(cProgram_, pIterator_, iBitSetID_, iMyBitSetID_);
	}
	AUTOPOINTER<This> pResult = 
		new BitSetImpl::UnionUnlock(iBitSetID_,
									iMyBitSetID_,
									iLockerID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

///////////////////////////////////////
// Operator::BitSet::Difference

// FUNCTION public
//	Operator::BitSet::Difference::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iBitSetID_
//	int iMyBitSetID_
//	
// RETURN
//	BitSet*
//
// EXCEPTIONS

//static
BitSet*
BitSet::Difference::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iBitSetID_,
	   int iMyBitSetID_)
{
	AUTOPOINTER<This> pResult = 
		new BitSetImpl::Difference(iBitSetID_,
								   iMyBitSetID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::BitSet::Difference::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iBitSetID_
//	int iMyBitSetID_
//	int iLockerID_
//	
// RETURN
//	BitSet*
//
// EXCEPTIONS

//static
BitSet*
BitSet::Difference::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iBitSetID_,
	   int iMyBitSetID_,
	   int iLockerID_)
{
	if (iLockerID_ < 0) {
		return create(cProgram_, pIterator_, iBitSetID_, iMyBitSetID_);
	}
	AUTOPOINTER<This> pResult = 
		new BitSetImpl::DifferenceUnlock(iBitSetID_,
										 iMyBitSetID_,
										 iLockerID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

///////////////////////////////////////
// Operator::BitSet::Collection

// FUNCTION public
//	Operator::BitSet::Collection::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	int iBitSetID_
//	
// RETURN
//	BitSet*
//
// EXCEPTIONS

//static
BitSet*
BitSet::Collection::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_,
	   int iBitSetID_)
{
	AUTOPOINTER<This> pResult = 
		new BitSetImpl::Collection(iDataID_,
								   iBitSetID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::BitSet::

// FUNCTION public
//	Operator::BitSet::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	BitSet*
//
// EXCEPTIONS

//static
BitSet*
BitSet::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::BitSetIntersect:
		{
			return new BitSetImpl::Intersect;
		}
	case Class::Category::BitSetIntersectUnlock:
		{
			return new BitSetImpl::IntersectUnlock;
		}
	case Class::Category::BitSetUnion:
		{
			return new BitSetImpl::Union;
		}
	case Class::Category::BitSetUnionUnlock:
		{
			return new BitSetImpl::UnionUnlock;
		}
	case Class::Category::BitSetDifference:
		{
			return new BitSetImpl::Difference;
		}
	case Class::Category::BitSetDifferenceUnlock:
		{
			return new BitSetImpl::DifferenceUnlock;
		}
	case Class::Category::BitSetCollection:
		{
			return new BitSetImpl::Collection;
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
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
