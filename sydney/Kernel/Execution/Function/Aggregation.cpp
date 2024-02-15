// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Aggregation.cpp --
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
const char moduleName[] = "Execution::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Function/Aggregation.h"
#include "Execution/Function/Class.h"
#include "Execution/Action/Collection.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Collection/Distinct.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DoubleData.h"
#include "Common/WordData.h"
#include "Common/DataInstance.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/NotSupported.h"
#include "Exception/NumericValueOutOfRange.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	struct _Type
	{
		enum Value {
			Count,
			Sum,
			Avg,
			Max,
			Min,
			Distinct,
			Word,
			ValueNum
		};
	};
	const char* const _pszExplainName[] =
	{
		"count",
		"sum",
		"avg",
		"max",
		"min",
		"distinct",
		"word",
		0
	};
}

namespace AggregationImpl
{
	// CLASS local
	//	Execution::Function::AggregationImpl::Base -- base class of implementation classes of Aggregation
	//
	// NOTES
	class Base
		: public Function::Aggregation
	{
	public:
		typedef Base This;
		typedef Function::Aggregation Super;

		virtual ~Base() {}

	///////////////////////////
	// Function::Aggregation::

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
	//	virtual void accumulate(Interface::IProgram& cProgram_,
	//							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		Base()
			: Super(),
			  m_cOperandData(),
			  m_cData()
		{}
		Base(int iOperandID_,
			 int iDataID_)
			: Super(),
			  m_cOperandData(iOperandID_),
			  m_cData(iDataID_)
		{}

		// for explain
		virtual void explainFunction(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

		bool initializeBase(Interface::IProgram& cProgram_);
		bool terminateBase(Interface::IProgram& cProgram_);

		// accessor
		const Action::DataHolder& getOperandData() {return m_cOperandData;}
		Action::DataHolder& getData() {return m_cData;}

	private:
		Action::DataHolder m_cOperandData;
		Action::DataHolder m_cData;
	};

	// CLASS local
	//	Execution::Function::AggregationImpl::Calculator -- base class of implementation classes of Aggregation
	//
	// NOTES
	class Calculator
		: public Base
	{
	public:
		typedef Calculator This;
		typedef Base Super;

		virtual ~Calculator() {}

	///////////////////////////
	// Function::Aggregation::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
	//	virtual void accumulate(Interface::IProgram& cProgram_,
	//							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
		Calculator()
			: Super(),
			  m_pData()
		{}
		Calculator(int iOperandID_,
				   int iDataID_)
			: Super(iOperandID_, iDataID_),
			  m_pData()
		{}

		// accessor
		Common::Data* getTempData() {return m_pData.get();}
		void setTempData(const Common::Data::Pointer& pData_) {m_pData = pData_;}

	private:
		Common::Data::Pointer m_pData;
	};

	// CLASS local
	//	Execution::Function::AggregationImpl::Count -- implementation class of count
	//
	// NOTES
	class Count
		: public Base
	{
	public:
		typedef Count This;
		typedef Base Super;

		Count()
			: Super(),
			  m_uCount(0)
		{}
		Count(int iOperandID_,
			  int iDataID_)
			: Super(iOperandID_, iDataID_),
			  m_uCount(0)
		{}
		~Count() {}

	///////////////////////////
	// Function::Aggregation::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);
		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void accumulate(Interface::IProgram& cProgram_,
								Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual void explainFunction(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);

		SIZE m_uCount;
	};

	// CLASS local
	//	Execution::Function::AggregationImpl::Count -- implementation class of count
	//
	// NOTES
	class BitSetCount
		: public Base
	{
	public:
		typedef BitSetCount This;
		typedef Base Super;

		BitSetCount()
			: Super()
		{}
		
		BitSetCount(int iOperandID_,
			  int iDataID_)
			: Super(iOperandID_, iDataID_)
		{}
		
		~BitSetCount() {}

	///////////////////////////
	// Function::Aggregation::

	/////////////////////////////
	// Interface::IAction::
		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void reset(Interface::IProgram& cProgram_) {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual void explainFunction(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};


	
	// CLASS local
	//	Execution::Function::AggregationImpl::Sum -- implementation class of sum
	//
	// NOTES
	class Sum
		: public Calculator
	{
	public:
		typedef Sum This;
		typedef Calculator Super;

		Sum()
			: Super(),
			  m_bOverflow(false),
			  m_pBackup()
		{}
		Sum(int iOperandID_,
			int iDataID_)
			: Super(iOperandID_, iDataID_),
			  m_bOverflow(false),
			  m_pBackup()
		{}
		~Sum() {}

	///////////////////////////
	// Function::Aggregation::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void accumulate(Interface::IProgram& cProgram_,
								Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual void explainFunction(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);

		bool m_bOverflow;
		Common::Data::Pointer m_pBackup;
	};

	// CLASS local
	//	Execution::Function::AggregationImpl::Avg -- implementation class of avg
	//
	// NOTES
	class Avg
		: public Calculator
	{
	public:
		typedef Avg This;
		typedef Calculator Super;

		Avg()
			: Super(),
			  m_uCount(0),
			  m_bOverflow(false),
			  m_pBackup()
		{}
		Avg(int iOperandID_,
			int iDataID_)
			: Super(iOperandID_, iDataID_),
			  m_uCount(0),
			  m_bOverflow(false),
			  m_pBackup()
		{}
		~Avg() {}

	///////////////////////////
	// Function::Aggregation::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void accumulate(Interface::IProgram& cProgram_,
								Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual void explainFunction(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);

		SIZE m_uCount;
		bool m_bOverflow;
		Common::Data::Pointer m_pBackup;
	};

	// CLASS local
	//	Execution::Function::AggregationImpl::Max -- implementation class of max
	//
	// NOTES
	class Max
		: public Calculator
	{
	public:
		typedef Max This;
		typedef Calculator Super;

		Max()
			: Super()
		{}
		Max(int iOperandID_,
			int iDataID_)
			: Super(iOperandID_, iDataID_)
		{}
		~Max() {}

	///////////////////////////
	// Function::Aggregation::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);
		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void accumulate(Interface::IProgram& cProgram_,
								Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual void explainFunction(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Function::AggregationImpl::Min -- implementation class of min
	//
	// NOTES
	class Min
		: public Calculator
	{
	public:
		typedef Min This;
		typedef Calculator Super;

		Min()
			: Super()
		{}
		Min(int iOperandID_,
			int iDataID_)
			: Super(iOperandID_, iDataID_)
		{}
		~Min() {}

	///////////////////////////
	// Function::Aggregation::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);
		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void accumulate(Interface::IProgram& cProgram_,
								Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual void explainFunction(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Function::AggregationImpl::Distinct -- implementation class of distinct
	//
	// NOTES
	class Distinct
		: public Base
	{
	public:
		typedef Distinct This;
		typedef Base Super;

		Distinct()
			: Super(),
			  m_cCollection()
		{}
		Distinct(int iOperandID_,
				 int iDataID_,
				 int iCollectionID_)
			: Super(iOperandID_, iDataID_),
			  m_cCollection(iCollectionID_,
							iOperandID_)
		{}
		~Distinct() {}

	///////////////////////////
	// Function::Aggregation::

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
		virtual void accumulate(Interface::IProgram& cProgram_,
								Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////
	// Base::
		virtual void explainFunction(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);

	private:
		Action::Collection m_cCollection;
	};

	// CLASS local
	//	Execution::Function::AggregationImpl::Word -- implementation class of avg
	//
	// NOTES
	class Word
		: public Calculator
	{
	public:
		typedef Word This;
		typedef Calculator Super;

		Word()
			: Super(),
			  m_uCount(0),
			  m_pBackup()
		{}
		Word(int iOperandID_,
			int iDataID_)
			: Super(iOperandID_, iDataID_),
			  m_uCount(0),
			  m_pBackup()
		{}
		~Word() {}

	///////////////////////////
	// Function::Aggregation::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void accumulate(Interface::IProgram& cProgram_,
								Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual void explainFunction(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);

		SIZE m_uCount;
		bool m_bOverflow;
		Common::Data::Pointer m_pBackup;
	};
	
}

///////////////////////////////////////////////
// Execution::Function::AggregationImpl::Base

// FUNCTION public
//	Function::AggregationImpl::Base::explain -- 
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
AggregationImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	explainFunction(cProgram_, cExplain_);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put("(");
		m_cOperandData.explain(cProgram_, cExplain_);
		cExplain_.put(")");
		cExplain_.put(" to ");
		m_cData.explain(cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Function::AggregationImpl::Base::initialize -- 
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
AggregationImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
}

// FUNCTION public
//	Function::AggregationImpl::Base::terminate -- 
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
AggregationImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
}

// FUNCTION public
//	Function::AggregationImpl::Base::finish -- 
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
AggregationImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Function::AggregationImpl::Base::serialize -- 
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
AggregationImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cOperandData.serialize(archiver_);
	m_cData.serialize(archiver_);
}

// FUNCTION protected
//	Function::AggregationImpl::Base::initializeBase -- 
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

bool
AggregationImpl::Base::
initializeBase(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		m_cData.initialize(cProgram_);
		m_cOperandData.initialize(cProgram_);
		return true;
	}
	return false;
}

// FUNCTION protected
//	Function::AggregationImpl::Base::terminateBase -- 
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

bool
AggregationImpl::Base::
terminateBase(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized()) {
		m_cData.terminate(cProgram_);
		m_cOperandData.terminate(cProgram_);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////
// Execution::Function::AggregationImpl::Calculator

// FUNCTION public
//	Function::AggregationImpl::Calculator::initialize -- 
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
AggregationImpl::Calculator::
initialize(Interface::IProgram& cProgram_)
{
	if (initializeBase(cProgram_)) {
		m_pData = Common::DataInstance::create(getData()->getType());
		m_pData->setNull();
	}
}

// FUNCTION public
//	Function::AggregationImpl::Calculator::terminate -- 
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
AggregationImpl::Calculator::
terminate(Interface::IProgram& cProgram_)
{
	if (terminateBase(cProgram_)) {
		m_pData = Common::Data::Pointer();
	}
}

///////////////////////////////////////////////
// Execution::Function::AggregationImpl::Count

// FUNCTION public
//	Function::AggregationImpl::Count::execute -- 
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
AggregationImpl::Count::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		Common::UnsignedIntegerData cData(m_uCount);
		getData()->assign(&cData, true /* for assign */);
		m_uCount = 0;
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::AggregationImpl::Count::accumulate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
AggregationImpl::Count::
accumulate(Interface::IProgram& cProgram_,
		   Action::ActionList& cActionList_)
{
	if (getOperandData()->isNull() == false) ++m_uCount;
}

// FUNCTION public
//	Function::AggregationImpl::Count::reset -- 
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
AggregationImpl::Count::
reset(Interface::IProgram& cProgram_)
{
	m_uCount = 0;
}

// FUNCTION public
//	Function::AggregationImpl::Count::getClassID -- 
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
AggregationImpl::Count::
getClassID() const
{
	return Class::getClassID(Class::Category::Count);
}

// FUNCTION private
//	Function::AggregationImpl::Count::explainFunction -- 
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
AggregationImpl::Count::
explainFunction(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Count]);
}


///////////////////////////////////////////////
// Execution::Function::AggregationImpl::Count

// FUNCTION public
//	Function::AggregationImpl::Count::execute -- 
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
AggregationImpl::BitSetCount::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		const Common::Data* pData = getOperandData().getData();
		SIZE uCount = 0;
		if (!pData || pData->getType() != Common::DataType::BitSet) {
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		
		Common::UnsignedIntegerData cData(
			_SYDNEY_DYNAMIC_CAST(const Common::BitSet*, pData)->count());
		
		getData()->assign(&cData, true /* for assign */);
		done();
	}
	return Action::Status::Success;
}



// FUNCTION public
//	Function::AggregationImpl::BitSetCount::getClassID -- 
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
AggregationImpl::BitSetCount::
getClassID() const
{
	return Class::getClassID(Class::Category::BitSetCount);
}


// FUNCTION private
//	Function::AggregationImpl::BitSetCount::explainFunction -- 
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
AggregationImpl::BitSetCount::
explainFunction(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Count]);
}




///////////////////////////////////////////////
// Execution::Function::AggregationImpl::Sum

// FUNCTION public
//	Function::AggregationImpl::Sum::initialize -- 
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
AggregationImpl::Sum::
initialize(Interface::IProgram& cProgram_)
{
	Super::initialize(cProgram_);
	if (m_pBackup.get() == 0) {
		m_pBackup = Common::DataInstance::create(getData()->getType());
		m_pBackup->setNull();
	}
}

// FUNCTION public
//	Function::AggregationImpl::Sum::terminate -- 
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
AggregationImpl::Sum::
terminate(Interface::IProgram& cProgram_)
{
	Super::terminate(cProgram_);
	m_pBackup = Common::Data::Pointer();
}

// FUNCTION public
//	Function::AggregationImpl::Sum::execute -- 
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
AggregationImpl::Sum::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		bool bThrow = (Opt::Configuration::getOverflowNull().get() == false);
		getData()->assign(getTempData(), bThrow);
		getTempData()->setNull();
		m_bOverflow = false;
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::AggregationImpl::Sum::accumulate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
AggregationImpl::Sum::
accumulate(Interface::IProgram& cProgram_,
		   Action::ActionList& cActionList_)
{
	if (m_bOverflow == false
		&& getOperandData()->isNull() == false) {
		if (getTempData()->isNull() == true) {
			getTempData()->assign(getOperandData().getData(), true /* for assign */);
			m_pBackup->assign(getTempData(), true);
		} else {
		retry:
			if (getTempData()->operateWith(Common::DataOperation::Addition,
										   getOperandData().getData())
				== false) {
				// exceeds type value range
				// -> try to use double precision data
				Common::Data::Pointer pNewData =
					Utility::DataType::getDoublePrecision(m_pBackup.get());
				if (pNewData.get() == 0) {
					m_bOverflow = true;
					if (Opt::Configuration::getOverflowNull().get() == true) {
						// set null instead of error
						getTempData()->setNull();
					} else {
						_SYDNEY_THROW0(Exception::NumericValueOutOfRange);
					}
				} else {
					setTempData(pNewData);
					m_pBackup = pNewData->copy();
					goto retry;	
				}
			} else {
				m_pBackup->assign(getTempData(), true);
			}
		}
	}
}

// FUNCTION public
//	Function::AggregationImpl::Sum::reset -- 
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
AggregationImpl::Sum::
reset(Interface::IProgram& cProgram_)
{
	getTempData()->setNull();
	m_bOverflow = false;
}

// FUNCTION public
//	Function::AggregationImpl::Sum::getClassID -- 
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
AggregationImpl::Sum::
getClassID() const
{
	return Class::getClassID(Class::Category::Sum);
}

// FUNCTION private
//	Function::AggregationImpl::Sum::explainFunction -- 
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
AggregationImpl::Sum::
explainFunction(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Sum]);
}

///////////////////////////////////////////////
// Execution::Function::AggregationImpl::Avg

// FUNCTION public
//	Function::AggregationImpl::Avg::initialize -- 
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
AggregationImpl::Avg::
initialize(Interface::IProgram& cProgram_)
{
	Super::initialize(cProgram_);
	if (m_pBackup.get() == 0) {
		m_pBackup = Common::DataInstance::create(getData()->getType());
		m_pBackup->setNull();
	}
}

// FUNCTION public
//	Function::AggregationImpl::Avg::terminate -- 
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
AggregationImpl::Avg::
terminate(Interface::IProgram& cProgram_)
{
	Super::terminate(cProgram_);
	m_pBackup = Common::Data::Pointer();
}

// FUNCTION public
//	Function::AggregationImpl::Avg::execute -- 
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
AggregationImpl::Avg::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (m_uCount > 1) {
			Common::UnsignedIntegerData cCount(m_uCount);
			if (getTempData()->operateWith(Common::DataOperation::Division, &cCount) == false) {
				// reaches here only when data is null
				; _SYDNEY_ASSERT(getTempData()->isNull());
			}
		}
		bool bThrow = (Opt::Configuration::getOverflowNull().get() == false);
		getData()->assign(getTempData(), bThrow);
		getTempData()->setNull();
		m_uCount = 0;
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::AggregationImpl::Avg::accumulate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
AggregationImpl::Avg::
accumulate(Interface::IProgram& cProgram_,
		   Action::ActionList& cActionList_)
{
	if (m_bOverflow == false
		&& getOperandData()->isNull() == false) {
		++m_uCount;
		if (getTempData()->isNull() == true) {
			getTempData()->assign(getOperandData().getData(), true /* for assign */);
			m_pBackup->assign(getTempData(), true);
		} else {
		retry:
			if (getTempData()->operateWith(Common::DataOperation::Addition,
										   getOperandData().getData())
				== false) {
				// exceeds type value range
				// -> try to use double precision data
				Common::Data::Pointer pNewData =
					Utility::DataType::getDoublePrecision(m_pBackup.get());
				if (pNewData.get() == 0) {
					m_bOverflow = true;
					if (Opt::Configuration::getOverflowNull().get() == true) {
						// set null instead of error
						getTempData()->setNull();
					} else {
						_SYDNEY_THROW0(Exception::NumericValueOutOfRange);
					}
				} else {
					setTempData(pNewData);
					m_pBackup = pNewData->copy();
					goto retry;	
				}
			} else {
				m_pBackup->assign(getTempData(), true);
			}
		}
	}
}

// FUNCTION public
//	Function::AggregationImpl::Avg::reset -- 
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
AggregationImpl::Avg::
reset(Interface::IProgram& cProgram_)
{
	getTempData()->setNull();
	m_uCount = 0;
	m_bOverflow = false;
}

// FUNCTION public
//	Function::AggregationImpl::Avg::getClassID -- 
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
AggregationImpl::Avg::
getClassID() const
{
	return Class::getClassID(Class::Category::Avg);
}

// FUNCTION private
//	Function::AggregationImpl::Avg::explainFunction -- 
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
AggregationImpl::Avg::
explainFunction(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Avg]);
}

///////////////////////////////////////////////
// Execution::Function::AggregationImpl::Max

// FUNCTION public
//	Function::AggregationImpl::Max::execute -- 
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
AggregationImpl::Max::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		getData()->assign(getTempData(), true /* for assign */);
		getTempData()->setNull();
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::AggregationImpl::Max::accumulate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
AggregationImpl::Max::
accumulate(Interface::IProgram& cProgram_,
		   Action::ActionList& cActionList_)
{
	if (getOperandData()->isNull() == false) {
		if (getTempData()->isNull() == true
			|| getOperandData()->compareTo(getTempData()) > 0) {
			// first data or greater data
			getTempData()->assign(getOperandData().getData(), true /* for assign */);
		}
	}
}

// FUNCTION public
//	Function::AggregationImpl::Max::reset -- 
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
AggregationImpl::Max::
reset(Interface::IProgram& cProgram_)
{
	getTempData()->setNull();
}

// FUNCTION public
//	Function::AggregationImpl::Max::getClassID -- 
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
AggregationImpl::Max::
getClassID() const
{
	return Class::getClassID(Class::Category::Max);
}

// FUNCTION private
//	Function::AggregationImpl::Max::explainFunction -- 
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
AggregationImpl::Max::
explainFunction(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Max]);
}

///////////////////////////////////////////////
// Execution::Function::AggregationImpl::Min

// FUNCTION public
//	Function::AggregationImpl::Min::execute -- 
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
AggregationImpl::Min::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		getData()->assign(getTempData(), true /* for assign */);
		getTempData()->setNull();
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::AggregationImpl::Min::accumulate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
AggregationImpl::Min::
accumulate(Interface::IProgram& cProgram_,
		   Action::ActionList& cActionList_)
{
	if (getOperandData()->isNull() == false) {
		if (getTempData()->isNull() == true
			|| getOperandData()->compareTo(getTempData()) < 0) {
			// first data or smaller data
			getTempData()->assign(getOperandData().getData(), true /* for assign */);
		}
	}
}

// FUNCTION public
//	Function::AggregationImpl::Min::reset -- 
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
AggregationImpl::Min::
reset(Interface::IProgram& cProgram_)
{
	getTempData()->setNull();
}

// FUNCTION public
//	Function::AggregationImpl::Min::getClassID -- 
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
AggregationImpl::Min::
getClassID() const
{
	return Class::getClassID(Class::Category::Min);
}

// FUNCTION private
//	Function::AggregationImpl::Min::explainFunction -- 
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
AggregationImpl::Min::
explainFunction(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Min]);
}

///////////////////////////////////////////////
// Execution::Function::AggregationImpl::Distinct

// FUNCTION public
//	Function::AggregationImpl::Distinct::explain -- 
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
AggregationImpl::Distinct::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	explainFunction(cProgram_, cExplain_);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		getOperandData().explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		getData().explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::AggregationImpl::Distinct::initialize -- 
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
AggregationImpl::Distinct::
initialize(Interface::IProgram& cProgram_)
{
	if (initializeBase(cProgram_)) {
		m_cCollection.initialize(cProgram_);
		m_cCollection.preparePutInterface();
	}
}

// FUNCTION public
//	Function::AggregationImpl::Distinct::terminate -- 
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
AggregationImpl::Distinct::
terminate(Interface::IProgram& cProgram_)
{
	if (terminateBase(cProgram_)) {
		m_cCollection.terminate(cProgram_);
	}
}

// FUNCTION public
//	Function::AggregationImpl::Distinct::execute -- 
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
AggregationImpl::Distinct::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	// flush put interface
	m_cCollection.flush();
	return Action::Status::Success;
}

// FUNCTION public
//	Function::AggregationImpl::Distinct::accumulate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
AggregationImpl::Distinct::
accumulate(Interface::IProgram& cProgram_,
		   Action::ActionList& cActionList_)
{
	if (m_cCollection.put(cProgram_)) {
		getData()->assign(getOperandData().getData());
	} else {
		getData()->setNull();
	}
}

// FUNCTION public
//	Function::AggregationImpl::Distinct::reset -- 
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
AggregationImpl::Distinct::
reset(Interface::IProgram& cProgram_)
{
	// flush put interface
	m_cCollection.flush();
}

// FUNCTION public
//	Function::AggregationImpl::Distinct::getClassID -- 
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
AggregationImpl::Distinct::
getClassID() const
{
	return Class::getClassID(Class::Category::Distinct);
}

// FUNCTION public
//	Function::AggregationImpl::Distinct::serialize -- 
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
AggregationImpl::Distinct::
serialize(ModArchive& archiver_)
{
	Super::serialize(archiver_);
	m_cCollection.serialize(archiver_);
}

// FUNCTION protected
//	Function::AggregationImpl::Distinct::explainFunction -- 
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
AggregationImpl::Distinct::
explainFunction(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Distinct]);
}

///////////////////////////////////////////////
// Execution::Function::AggregationImpl::Word

// FUNCTION public
//	Function::AggregationImpl::Word::initialize -- 
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
AggregationImpl::Word::
initialize(Interface::IProgram& cProgram_)
{
	Super::initialize(cProgram_);
	if (m_pBackup.get() == 0) {
		m_pBackup = Common::DataInstance::create(getData()->getType());
		m_pBackup->setNull();
	}
}

// FUNCTION public
//	Function::AggregationImpl::Word::terminate -- 
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
AggregationImpl::Word::
terminate(Interface::IProgram& cProgram_)
{
	Super::terminate(cProgram_);
	m_pBackup = Common::Data::Pointer();
}

// FUNCTION public
//	Function::AggregationImpl::Word::execute -- 
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
AggregationImpl::Word::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (m_uCount > 1) {
			if (!getTempData()->isNull()
				&& getTempData()->getType() == Common::DataType::Word) {
				Common::WordData* pData = _SYDNEY_DYNAMIC_CAST(Common::WordData*, getTempData());
				Common::DoubleData dScale(pData->getScale());
				Common::UnsignedIntegerData iCount(m_uCount);
				if (dScale.operateWith(Common::DataOperation::Division, &iCount) == false) {
					; _SYDNEY_ASSERT(getTempData()->isNull());
				}
				pData->setScale(dScale.getValue());
			}
		}
		bool bThrow = (Opt::Configuration::getOverflowNull().get() == false);
		getData()->assign(getTempData(), bThrow);
		getTempData()->setNull();
		m_uCount = 0;
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::AggregationImpl::Word::accumulate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
AggregationImpl::Word::
accumulate(Interface::IProgram& cProgram_,
		   Action::ActionList& cActionList_)
{
	if (getOperandData()->isNull() == false) {
		++m_uCount;
		if (getTempData()->isNull() == true) {
			getTempData()->assign(getOperandData().getData(), true /* for assign */);
			m_pBackup->assign(getTempData(), true);
		} else {
			if (getTempData()->operateWith(Common::DataOperation::Addition,
										   getOperandData().getData())) {
				m_pBackup->assign(getTempData(), true);
			} else {
				_SYDNEY_THROW0(Exception::NumericValueOutOfRange);
			}
		}
	}
}

// FUNCTION public
//	Function::AggregationImpl::Word::reset -- 
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
AggregationImpl::Word::
reset(Interface::IProgram& cProgram_)
{
	getTempData()->setNull();
	m_uCount = 0;
}

// FUNCTION public
//	Function::AggregationImpl::Word::getClassID -- 
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
AggregationImpl::Word::
getClassID() const
{
	return Class::getClassID(Class::Category::Word);
}

// FUNCTION private
//	Function::AggregationImpl::Word::explainFunction -- 
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
AggregationImpl::Word::
explainFunction(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Word]);
}




/////////////////////////////////
// Function::Aggregation::Count

// FUNCTION public
//	Function::Aggregation::Count::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iOperandID_
//	int iDataID_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::Count::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iOperandID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new AggregationImpl::Count(iOperandID_, iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}



/////////////////////////////////
// Function::Aggregation::BitSetCount

// FUNCTION public
//	Function::Aggregation::BitSetCount::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iOperandID_
//	int iDataID_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::BitSetCount::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iOperandID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new AggregationImpl::BitSetCount(iOperandID_, iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}


/////////////////////////////////
// Function::Aggregation::Sum

// FUNCTION public
//	Function::Aggregation::Sum::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iOperandID_
//	int iDataID_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::Sum::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iOperandID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new AggregationImpl::Sum(iOperandID_, iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Function::Aggregation::Avg

// FUNCTION public
//	Function::Aggregation::Avg::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iOperandID_
//	int iDataID_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::Avg::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iOperandID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new AggregationImpl::Avg(iOperandID_, iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Function::Aggregation::Max

// FUNCTION public
//	Function::Aggregation::Max::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iOperandID_
//	int iDataID_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::Max::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iOperandID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new AggregationImpl::Max(iOperandID_, iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Function::Aggregation::Min

// FUNCTION public
//	Function::Aggregation::Min::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iOperandID_
//	int iDataID_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::Min::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iOperandID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = 
		new AggregationImpl::Min(iOperandID_, iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Function::Aggregation::Distinct

// FUNCTION public
//	Function::Aggregation::Distinct::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iOperandID_
//	int iDataID_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::Distinct::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iOperandID_,
	   int iDataID_)
{
	Interface::ICollection* pDistinct = Collection::Distinct::create(cProgram_);

	AUTOPOINTER<This> pResult =
		new AggregationImpl::Distinct(iOperandID_,
									  iDataID_,
									  pDistinct->getID());
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}


/////////////////////////////////
// Function::Aggregation::Word

// FUNCTION public
//	Function::Aggregation::Word::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iOperandID_
//	int iDataID_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::Word::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iOperandID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new AggregationImpl::Word(iOperandID_, iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}


//////////////////////////////
// Function::Aggregation::

// FUNCTION public
//	Function::Aggregation::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Count:
		{
			return new AggregationImpl::Count;
		}
	case Class::Category::Sum:
		{
			return new AggregationImpl::Sum;
		}
	case Class::Category::Avg:
		{
			return new AggregationImpl::Avg;
		}
	case Class::Category::Max:
		{
			return new AggregationImpl::Max;
		}
	case Class::Category::Min:
		{
			return new AggregationImpl::Min;
		}
	case Class::Category::Distinct:
		{
			return new AggregationImpl::Distinct;
		}
	case Class::Category::BitSetCount:
		{
			return new AggregationImpl::BitSetCount;
		}
	case Class::Category::Word:
		{
			return new AggregationImpl::Word;
		}
		
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
