// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Coalesce.cpp --
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
const char moduleName[] = "Execution::Function";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Execution/Function/Coalesce.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/ActionHolder.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	struct _Type
	{
		enum Value
		{
			Coalesce,
			CoalesceDefault,
			ValueNum
		};
	};

	// CONST
	//	_pszFunctionName -- function name for explain
	//
	// NOTES
	const char* const _pszFunctionName[_Type::ValueNum] = {
		"coalesce",
		"case (is not default)"
	};
}

namespace CoalesceImpl
{
	// CLASS local
	//	Execution::Function::CoalesceImpl::Dyadic -- implementation class of Coalesce
	//
	// NOTES
	class Dyadic
		: public Function::Coalesce
	{
	public:
		typedef Dyadic This;
		typedef Function::Coalesce Super;

		Dyadic()
			: Super(),
			  m_cInData0(),
			  m_cInData1(),
			  m_cOutData()
		{}
		Dyadic(int iInDataID0_,
			   int iInDataID1_,
			   int iOutDataID_)
			: Super(),
			  m_cInData0(iInDataID0_),
			  m_cInData1(iInDataID1_),
			  m_cOutData(iOutDataID_)
		{}
		virtual ~Dyadic()
		{}

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
		// accessor
		const Common::Data* getInData0() {return m_cInData0.getData();}
		const Common::Data* getInData1() {return m_cInData1.getData();}
		Common::Data* getOutData() {return m_cOutData.get();}

	private:
		virtual void explainThis(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
		virtual bool checkData(const Common::Data& cData_);

		Action::DataHolder m_cInData0;
		Action::DataHolder m_cInData1;
		Action::DataHolder m_cOutData;
	};

	// CLASS local
	//	Execution::Function::CoalesceImpl::ByDefault -- implementation class of CoalesceDefault
	//
	// NOTES
	class ByDefault
		: public Dyadic
	{
	public:
		typedef ByDefault This;
		typedef Dyadic Super;

		ByDefault()
			: Super()
		{}
		ByDefault(int iInDataID0_,
				  int iInDataID1_,
				  int iOutDataID_)
			: Super(iInDataID0_,
					iInDataID1_,
					iOutDataID_)
		{}
		~ByDefault()
		{}

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	//////////////////////////
	// CoalesceImpl::
		virtual void explainThis(Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
		virtual bool checkData(const Common::Data& cData_);
	};

	// CLASS local
	//	Execution::Function::CoalesceImpl::Nadic -- implementation class of Coalesce
	//
	// NOTES
	class Nadic
		: public Function::Coalesce
	{
	public:
		typedef Nadic This;
		typedef Function::Coalesce Super;

		Nadic()
			: Super(),
			  m_vecAction(),
			  m_vecData(),
			  m_cOutData()
		{}
		Nadic(const VECTOR<int>& vecActionID_,
			  const VECTOR<int>& vecDataID_,
			  int iOutDataID_)
			: Super(),
			  m_vecAction(),
			  m_vecData(),
			  m_cOutData(iOutDataID_)
		{
			setAction(vecActionID_);
			setData(vecDataID_);
		}
		~Nadic()
		{}

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
		// accessor
		Common::Data* getOutData() {return m_cOutData.get();}

	private:
		void setAction(const VECTOR<int>& vecActionID_);
		void setData(const VECTOR<int>& vecDataID_);

		VECTOR<Action::ActionHolder> m_vecAction;
		VECTOR<Action::DataHolder> m_vecData;
		Action::DataHolder m_cOutData;
	};
}

/////////////////////////////////////////////////
// Execution::Function::CoalesceImpl::Dyadic

// FUNCTION public
//	Function::CoalesceImpl::Dyadic::explain -- 
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
CoalesceImpl::Dyadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	explainThis(cProgram_, cExplain_);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put("(");
		m_cInData0.explain(cProgram_, cExplain_);
		cExplain_.put(",");
		m_cInData1.explain(cProgram_, cExplain_);
		cExplain_.put(") to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::CoalesceImpl::Dyadic::initialize -- 
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
CoalesceImpl::Dyadic::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cInData0.isInitialized() == false) {
		m_cInData0.initialize(cProgram_);
		m_cInData1.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::CoalesceImpl::Dyadic::terminate -- 
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
CoalesceImpl::Dyadic::
terminate(Interface::IProgram& cProgram_)
{
	m_cInData0.terminate(cProgram_);
	m_cInData1.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::CoalesceImpl::Dyadic::execute -- 
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
CoalesceImpl::Dyadic::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (checkData(*m_cInData0)) {
			getOutData()->assign(getInData1());
		} else {
			getOutData()->assign(getInData0());
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::CoalesceImpl::Dyadic::finish -- 
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
CoalesceImpl::Dyadic::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::CoalesceImpl::Dyadic::reset -- 
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
CoalesceImpl::Dyadic::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::CoalesceImpl::Dyadic::getClassID -- 
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
CoalesceImpl::Dyadic::
getClassID() const
{
	return Class::getClassID(Class::Category::Coalesce);
}

// FUNCTION public
//	Function::CoalesceImpl::Dyadic::serialize -- 
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
CoalesceImpl::Dyadic::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cInData0.serialize(archiver_);
	m_cInData1.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

// FUNCTION private
//	Function::CoalesceImpl::Dyadic::explainThis -- 
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
CoalesceImpl::Dyadic::
explainThis(Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszFunctionName[_Type::Coalesce]);
}

// FUNCTION private
//	Function::CoalesceImpl::Dyadic::checkData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
CoalesceImpl::Dyadic::
checkData(const Common::Data& cData_)
{
	return cData_.isNull();
}

/////////////////////////////////////////////////
// Execution::Function::CoalesceImpl::ByDefault

// FUNCTION public
//	Function::CoalesceImpl::ByDefault::getClassID -- 
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
CoalesceImpl::ByDefault::
getClassID() const
{
	return Class::getClassID(Class::Category::CoalesceDefault);
}

// FUNCTION private
//	Function::CoalesceImpl::ByDefault::explainThis -- 
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
CoalesceImpl::ByDefault::
explainThis(Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszFunctionName[_Type::CoalesceDefault]);
}

// FUNCTION private
//	Function::CoalesceImpl::ByDefault::checkData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
CoalesceImpl::ByDefault::
checkData(const Common::Data& cData_)
{
	return cData_.isDefault();
}

/////////////////////////////////////////////////
// Execution::Function::CoalesceImpl::Nadic

// FUNCTION public
//	Function::CoalesceImpl::Nadic::explain -- 
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
CoalesceImpl::Nadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszFunctionName[_Type::Coalesce]);
	cExplain_.popNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.pushIndent();
		cExplain_.put("(");
		Opt::Join(m_vecAction,
				  boost::bind(&Action::ActionHolder::explain,
							  _1,
							  pEnvironment_,
							  boost::ref(cProgram_),
							  boost::ref(cExplain_)),
				  boost::bind(&Opt::Explain::putChar,
							  boost::ref(cExplain_),
							  ','));
		cExplain_.put(") to ");
		m_cOutData.explain(cProgram_, cExplain_);
		cExplain_.popIndent();
	}
}

// FUNCTION public
//	Function::CoalesceImpl::Nadic::initialize -- 
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
CoalesceImpl::Nadic::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cOutData.isInitialized() == false) {
		Opt::ForEach(m_vecAction,
					 boost::bind(&Action::ActionHolder::initialize,
								 _1,
								 boost::ref(cProgram_)));
		Opt::ForEach(m_vecData,
					 boost::bind(&Action::DataHolder::initialize,
								 _1,
								 boost::ref(cProgram_)));
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::CoalesceImpl::Nadic::terminate -- 
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
CoalesceImpl::Nadic::
terminate(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecAction,
				 boost::bind(&Action::ActionHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	Opt::ForEach(m_vecData,
				 boost::bind(&Action::DataHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::CoalesceImpl::Nadic::execute -- 
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
CoalesceImpl::Nadic::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		; _SYDNEY_ASSERT(m_vecAction.GETSIZE() == m_vecData.GETSIZE());

		VECTOR<Action::ActionHolder>::ITERATOR iteratorA = m_vecAction.begin();
		const VECTOR<Action::ActionHolder>::ITERATOR lastA = m_vecAction.end();
		VECTOR<Action::DataHolder>::ITERATOR iteratorD = m_vecData.begin();
		const VECTOR<Action::DataHolder>::ITERATOR lastD = m_vecData.end();

		for (; iteratorA != lastA; ++iteratorA, ++iteratorD) {
			; _SYDNEY_ASSERT(iteratorD != lastD);

			Action::Status::Value eResult = (*iteratorA)->execute(cProgram_, cActionList_);
			if (eResult != Action::Status::Success) return eResult;

			// check data
			if ((*iteratorD)->isNull() == false) {
				getOutData()->assign((*iteratorD).getData());
				break;
			}
		}
		if (iteratorA == lastA) {
			// use last element's result
			getOutData()->assign(m_vecData.GETBACK().getData());
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::CoalesceImpl::Nadic::finish -- 
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
CoalesceImpl::Nadic::
finish(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecAction,
				 boost::bind(&Interface::IAction::finish,
							 boost::bind(&Action::ActionHolder::getAction,
										 _1),
							 boost::ref(cProgram_)));
}

// FUNCTION public
//	Function::CoalesceImpl::Nadic::reset -- 
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
CoalesceImpl::Nadic::
reset(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecAction,
				 boost::bind(&Interface::IAction::reset,
							 boost::bind(&Action::ActionHolder::getAction,
										 _1),
							 boost::ref(cProgram_)));
}

// FUNCTION public
//	Function::CoalesceImpl::Nadic::getClassID -- 
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
CoalesceImpl::Nadic::
getClassID() const
{
	return Class::getClassID(Class::Category::CoalesceNadic);
}

// FUNCTION public
//	Function::CoalesceImpl::Nadic::serialize -- 
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
CoalesceImpl::Nadic::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	Utility::SerializeObject(archiver_, m_vecAction);
	Utility::SerializeObject(archiver_, m_vecData);
	m_cOutData.serialize(archiver_);
}

// FUNCTION private
//	Function::CoalesceImpl::Nadic::setAction -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecActionID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
CoalesceImpl::Nadic::
setAction(const VECTOR<int>& vecActionID_)
{
	int n = vecActionID_.GETSIZE();
	m_vecAction.assign(n, Action::ActionHolder());
	for (int i = 0; i < n; ++i) {
		m_vecAction[i].setActionID(vecActionID_[i]);
	}
}


// FUNCTION private
//	Function::CoalesceImpl::Nadic::setData -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
CoalesceImpl::Nadic::
setData(const VECTOR<int>& vecDataID_)
{
	int n = vecDataID_.GETSIZE();
	m_vecData.assign(n, Action::DataHolder());
	for (int i = 0; i < n; ++i) {
		m_vecData[i].setDataID(vecDataID_[i]);
	}
}

//////////////////////////////
// Function::Coalesce::

// FUNCTION public
//	Function::Coalesce::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iInDataID0_
//	int iInDataID1_
//	int iOutDataID_
//	
// RETURN
//	Coalesce*
//
// EXCEPTIONS

//static
Coalesce*
Coalesce::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID0_,
	   int iInDataID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new CoalesceImpl::Dyadic(iInDataID0_,
														 iInDataID1_,
														 iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Coalesce::Default::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iInDataID0_
//	int iInDataID1_
//	int iOutDataID_
//	
// RETURN
//	Coalesce*
//
// EXCEPTIONS

//static
Coalesce*
Coalesce::Default::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID0_,
	   int iInDataID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new CoalesceImpl::ByDefault(iInDataID0_,
															iInDataID1_,
															iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Coalesce::Nadic::create -- constructor -- N-adic coalese
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecActionID_
//	const VECTOR<int>& vecDataID_
//	int iOutDataID_
//	
// RETURN
//	Coalesce*
//
// EXCEPTIONS

//static
Coalesce*
Coalesce::Nadic::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecActionID_,
	   const VECTOR<int>& vecDataID_,
	   int iOutDataID_)
{
	if (vecActionID_.GETSIZE() == 0
		|| vecActionID_.GETSIZE() != vecDataID_.GETSIZE()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	AUTOPOINTER<This> pResult = new CoalesceImpl::Nadic(vecActionID_,
														vecDataID_,
														iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Coalesce::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Coalesce*
//
// EXCEPTIONS

//static
Coalesce*
Coalesce::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::Coalesce:
		{
			return new CoalesceImpl::Dyadic;
		}
	case Class::Category::CoalesceDefault:
		{
			return new CoalesceImpl::ByDefault;
		}
	case Class::Category::CoalesceNadic:
		{
			return new CoalesceImpl::Nadic;
		}
	default:
		{
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
