// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/CharJoin.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Function/CharJoin.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/StringData.h"

#include "Exception/NotCompatible.h"
#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	// CONST
	//	_pszOperator -- operator name for explain
	//
	// NOTES
	const char* const _pszOperator = "char_join";
}

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::CharJoinImpl -- base class of implementation class
	//
	// NOTES
	class CharJoinImpl
		: public Function::CharJoin
	{
	public:
		typedef CharJoinImpl This;
		typedef Function::CharJoin Super;

		CharJoinImpl()
			: Super(),
			  m_cSeparator(),
			  m_vecInData(),
			  m_cOutData()
		{}
		CharJoinImpl(const VECTOR<int>& vecInDataID_,
					 int iOptionID_,
					 int iOutDataID_)
			: Super(),
			  m_cSeparator(iOptionID_),
			  m_vecInData(),
			  m_cOutData(iOutDataID_)
		{
			setData(vecInDataID_);
		}
		virtual ~CharJoinImpl()
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
	private:
		void setData(const VECTOR<int>& vecInDataID_);
		void addElement(const Action::DataHolder& cElement_,
						OSTRSTREAM& cStream_);
		void addData(const Common::Data& cData_,
					 OSTRSTREAM& cStream_);
		void addSeparator(OSTRSTREAM& cStream_);

		Action::StringDataHolder m_cSeparator;
		VECTOR<Action::DataHolder> m_vecInData;
		Action::StringDataHolder m_cOutData;
	};
}

/////////////////////////////////////////////////
// Execution::Function::Impl::CharJoinImpl

// FUNCTION public
//	Function::Impl::CharJoinImpl::explain -- 
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
Impl::CharJoinImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperator);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put("(");
		m_cSeparator.explain(cProgram_, cExplain_);
		Opt::ForEach(m_vecInData,
					 boost::bind(&Action::DataHolder::explain,
								 _1,
								 boost::ref(cProgram_),
								 boost::bind(&Opt::Explain::putChar,
											 boost::ref(cExplain_),
											 ',')));
		cExplain_.put(")");
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::Impl::CharJoinImpl::initialize -- 
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
Impl::CharJoinImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cOutData.isInitialized() == false) {
		Opt::ForEach(m_vecInData,
					 boost::bind(&Action::DataHolder::initialize,
								 _1,
								 boost::ref(cProgram_)));
		m_cSeparator.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::Impl::CharJoinImpl::terminate -- 
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
Impl::CharJoinImpl::
terminate(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecInData,
				 boost::bind(&Action::DataHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	m_cSeparator.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::Impl::CharJoinImpl::execute -- 
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
Impl::CharJoinImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		OSTRSTREAM cStream;
		Opt::Join(m_vecInData,
				  boost::bind(&This::addElement,
							  this,
							  _1,
							  boost::ref(cStream)),
				  boost::bind(&This::addSeparator,
							  this,
							  boost::ref(cStream)));
		m_cOutData->setValue(ModUnicodeString(cStream.getString()));
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::Impl::CharJoinImpl::finish -- 
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
Impl::CharJoinImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::CharJoinImpl::reset -- 
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
Impl::CharJoinImpl::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::CharJoinImpl::getClassID -- 
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
Impl::CharJoinImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::CharJoin);
}

// FUNCTION public
//	Function::Impl::CharJoinImpl::serialize -- 
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
Impl::CharJoinImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	Utility::SerializeObject(archiver_, m_vecInData);
	m_cSeparator.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

// FUNCTION private
//	Function::Impl::CharJoinImpl::setData -- 
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
Impl::CharJoinImpl::
setData(const VECTOR<int>& vecInDataID_)
{
	int n = vecInDataID_.GETSIZE();
	m_vecInData.assign(n, Action::DataHolder());
	for (int i = 0; i < n; ++i) {
		m_vecInData[i].setDataID(vecInDataID_[i]);
	}
}

// FUNCTION private
//	Function::Impl::CharJoinImpl::addElement -- 
//
// NOTES
//
// ARGUMENTS
//	const Action::DataHolder& cElement_
//	OSTRSTREAM& cStream_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::CharJoinImpl::
addElement(const Action::DataHolder& cElement_,
		   OSTRSTREAM& cStream_)
{
	addData(*cElement_, cStream_);
}

// FUNCTION private
//	Function::Impl::CharJoinImpl::addData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	OSTRSTREAM& cStream_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::CharJoinImpl::
addData(const Common::Data& cData_,
		OSTRSTREAM& cStream_)
{
	switch (cData_.getType()) {
	case Common::DataType::String:
		{
			cStream_ << _SYDNEY_DYNAMIC_CAST(const Common::StringData&, cData_).getValue();
			break;
		}
	case Common::DataType::Array:
		{
			if (cData_.getElementType() == Common::DataType::Data) {
				const Common::DataArrayData& cArray =
					_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, cData_);

				int n = cArray.getCount();
				int i = 0;
				addData(*cArray.getElement(i), cStream_);
				for (++i; i < n; ++i) {
					addSeparator(cStream_);
					addData(*cArray.getElement(i), cStream_);
				}
				break;
			}
			; // thru.
		}
	default:
		{
			// unsupported type
			_SYDNEY_THROW0(Exception::NotCompatible);
		}
	}
}

// FUNCTION private
//	Function::Impl::CharJoinImpl::addSeparator -- 
//
// NOTES
//
// ARGUMENTS
//	OSTRSTREAM& cStream_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::CharJoinImpl::
addSeparator(OSTRSTREAM& cStream_)
{
	cStream_ << m_cSeparator->getValue();
}

/////////////////////////////////////
// Function::CharJoin::

// FUNCTION public
//	Function::CharJoin::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecInDataID_
//	int iOptionID_
//	int iOutDataID_
//	
// RETURN
//	CharJoin*
//
// EXCEPTIONS

//static
CharJoin*
CharJoin::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecInDataID_,
	   int iOptionID_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::CharJoinImpl(vecInDataID_,
													   iOptionID_,
													   iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Function::CharJoin::

// FUNCTION public
//	Function::CharJoin::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	CharJoin*
//
// EXCEPTIONS

//static
CharJoin*
CharJoin::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::CharJoin:
		{
			return new Impl::CharJoinImpl;
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
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
