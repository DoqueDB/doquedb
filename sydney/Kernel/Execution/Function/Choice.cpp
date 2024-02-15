// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Choice.cpp --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#include "Execution/Function/Choice.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/DataOperation.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	// CONST
	//	_pszFunctionName -- function name for explain
	//
	// NOTES
	const char* const _pszFunctionName = "get max";
}

namespace ChoiceImpl
{
	// CLASS local
	//	Execution::Function::ChoiceImpl::GetMax -- implementation class of Choice
	//
	// NOTES
	class GetMax
		: public Function::Choice
	{
	public:
		typedef GetMax This;
		typedef Function::Choice Super;

		GetMax()
			: Super(),
			  m_vecInData(),
			  m_cOutData()
		{}
		GetMax(const VECTOR<int>& vecOperandID_,
			   int iOutDataID_)
			: Super(),
			  m_vecInData(),
			  m_cOutData(iOutDataID_)
		{
			setData(vecOperandID_);
		}
		~GetMax()
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
		void calculate();
		void setData(const VECTOR<int>& vecInDataID_);

		VECTOR<Action::DataHolder> m_vecInData;
		Action::DataHolder m_cOutData;
	};
}

/////////////////////////////////////////////////
// Execution::Function::ChoiceImpl::GetMax

// FUNCTION public
//	Function::ChoiceImpl::GetMax::explain -- 
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
ChoiceImpl::GetMax::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszFunctionName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put("(");
		Opt::Join(m_vecInData,
				  boost::bind(&Action::DataHolder::explain,
							  _1,
							  boost::ref(cProgram_),
							  boost::ref(cExplain_)),
				  boost::bind(&Opt::Explain::putChar,
							  boost::ref(cExplain_),
							  ','));
		cExplain_.put(")");
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::ChoiceImpl::GetMax::initialize -- 
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
ChoiceImpl::GetMax::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cOutData.isInitialized() == false) {
		Opt::ForEach(m_vecInData,
					 boost::bind(&Action::DataHolder::initialize,
								 _1,
								 boost::ref(cProgram_)));
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::ChoiceImpl::GetMax::terminate -- 
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
ChoiceImpl::GetMax::
terminate(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecInData,
				 boost::bind(&Action::DataHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::ChoiceImpl::GetMax::execute -- 
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
ChoiceImpl::GetMax::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		calculate();
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::ChoiceImpl::GetMax::finish -- 
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
ChoiceImpl::GetMax::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::ChoiceImpl::GetMax::reset -- 
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
ChoiceImpl::GetMax::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::ChoiceImpl::GetMax::getClassID -- 
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
ChoiceImpl::GetMax::
getClassID() const
{
	return Class::getClassID(Class::Category::GetMax);
}

// FUNCTION public
//	Function::ChoiceImpl::GetMax::serialize -- 
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
ChoiceImpl::GetMax::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	Utility::SerializeObject(archiver_, m_vecInData);
	m_cOutData.serialize(archiver_);
}

// FUNCTION private
//	Function::ChoiceImpl::GetMax::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ChoiceImpl::GetMax::
calculate()
{
	VECTOR<Action::DataHolder>::ITERATOR found = m_vecInData.begin();
	VECTOR<Action::DataHolder>::ITERATOR iterator = m_vecInData.begin() + 1;
	const VECTOR<Action::DataHolder>::ITERATOR last = m_vecInData.end();
	for (; iterator != last; ++iterator) {
		if ((*found)->compareTo((*iterator).getData()) < 0) {
			found = iterator;
		}
	}
	getOutData()->assign((*found).getData());
}

// FUNCTION private
//	Function::ChoiceImpl::GetMax::setData -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecInDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ChoiceImpl::GetMax::
setData(const VECTOR<int>& vecInDataID_)
{
	int n = vecInDataID_.GETSIZE();
	m_vecInData.assign(n, Action::DataHolder());
	for (int i = 0; i < n; ++i) {
		m_vecInData[i].setDataID(vecInDataID_[i]);
	}
}

//////////////////////////////
// Function::Choice::

// FUNCTION public
//	Function::Choice::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	const VECTOR<int>& vecInDataID_
//	int iOutDataID_
//	
// RETURN
//	Choice*
//
// EXCEPTIONS

//static
Choice*
Choice::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   const VECTOR<int>& vecInDataID_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult;
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::GetMax:
		{
			pResult = new ChoiceImpl::GetMax(vecInDataID_,
											 iOutDataID_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Choice::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Choice*
//
// EXCEPTIONS

//static
Choice*
Choice::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::GetMax:
		{
			return new ChoiceImpl::GetMax;
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
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
