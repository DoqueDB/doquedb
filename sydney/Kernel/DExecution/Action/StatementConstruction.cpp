// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/StatementConstruction.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DExecution::Action";
}

#include "boost/bind.hpp"


#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DExecution/Action/Class.h"
#include "DExecution/Action/StatementConstruction.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/StringData.h"

#include "Exception/Unexpected.h"
#include "Exception/NotSupported.h"

#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"




_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_ACTION_BEGIN

///////////////////////////////////////
// DExecution::Action::StatementConstruction

// FUNCTION public
//	Action::StatementConstruction::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Schema::Cascade* pSchemaCascade_
//	const STRING& cstrSQL_
//	
// RETURN
//	StatementConstruction*
//
// EXCEPTIONS

//static
StatementConstruction*
StatementConstruction::
create(Execution::Interface::IProgram& cProgram_,
	   Execution::Interface::IIterator* pIterator_,
	   int iSqlID_,
	   bool bConcatinate_ /* false */)
{
	AUTOPOINTER<This> pResult = new StatementConstruction(iSqlID_,
														  bConcatinate_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}



// FUNCTION public
//	Action::StatementConstruction::explain -- explain serveraccess
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
StatementConstruction::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	VECTOR<STRING>::ConstIterator iteStatement = m_vecStatement.begin();
	VECTOR<int>::ConstIterator iteType =  m_vecType.begin();
	VECTOR<Execution::Action::DataHolder>::
		ConstIterator iteData = m_vecData.begin();
	VECTOR<Execution::Action::DataHolder>::
		ConstIterator itePlaceHolder = m_vecPlaceHolder.begin();
	
	for (; iteType != m_vecType.end(); ++iteType) {
		if (*iteType == STATEMENT) {
			_SYDNEY_ASSERT(iteStatement != m_vecStatement.end());
			cExplain_.put(*iteStatement);
			iteStatement++;
		} else if (*iteType == DATA_ID) {
			_SYDNEY_ASSERT(iteData != m_vecData.end());
			(*iteData).explain(cProgram_, cExplain_);
			iteData++;
		} else if (*iteType == PLACEHOLDER || *iteType == ARRAY_PLACEHOLDER) {
			_SYDNEY_ASSERT(itePlaceHolder != m_vecPlaceHolder.end());
			(*itePlaceHolder).explain(cProgram_, cExplain_);
			itePlaceHolder++;
		}
	}
	cExplain_.put(" -> ");
	m_cSqlData.explain(cProgram_, cExplain_);
	m_cPreActions.explain(pEnvironment_, cProgram_, cExplain_);
}

// FUNCTION public
//	Action::StatementConstruction::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
StatementConstruction::
initialize(Execution::Interface::IProgram& cProgram_)
{
	if (!m_cSqlData.isInitialized()) {
		m_cSqlData.initialize(cProgram_);
		
		VECTOR<Execution::Action::DataHolder>::Iterator ite = m_vecData.begin();
		for (; ite != m_vecData.end(); ++ite)
			(*ite).initialize(cProgram_);

		ite = m_vecPlaceHolder.begin();
		for (; ite != m_vecPlaceHolder.end(); ++ite)
			(*ite).initialize(cProgram_);		

		m_cPreActions.initialize(cProgram_);
	}
}

// FUNCTION public
//	Action::StatementConstruction::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
StatementConstruction::
terminate(Execution::Interface::IProgram& cProgram_)
{
	m_cSqlData.terminate(cProgram_);
	VECTOR<Execution::Action::DataHolder>::Iterator ite = m_vecData.begin();
	for (; ite != m_vecData.end(); ++ite)
		(*ite).terminate(cProgram_);

	ite = m_vecPlaceHolder.begin();
	for (; ite != m_vecPlaceHolder.end(); ++ite)
		(*ite).terminate(cProgram_);	

	m_cPreActions.terminate(cProgram_);
}

// FUNCTION public
//	Action::StatementConstruction::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::ActionList& cActionList_
//	
// RETURN
//	Execution::Action::Status::Value
//
// EXCEPTIONS

//virtual
Execution::Action::Status::Value
StatementConstruction::
execute(Execution::Interface::IProgram& cProgram_,
		Execution::Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		m_cPreActions.execute(cProgram_);
		toStatement(cProgram_);
		done();
	}
	return Execution::Action::Status::Success;
}


// FUNCTION public
//	Action::StatementConstruction::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
StatementConstruction::
finish(Execution::Interface::IProgram& cProgram_)
{
	m_cPreActions.finish(cProgram_);
}

// FUNCTION public
//	Action::StatementConstruction::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
StatementConstruction::
reset(Execution::Interface::IProgram& cProgram_)
{
	; // Nothing to do
}


// FUNCTION public
//	Action::StatementConstruction::append
//
// NOTES
//
// ARGUMENTS
//	int iOutID_
//	Execution::Interface::IAction* pAction_
//	
// RETURN
//	StatementConstruction&
//
// EXCEPTIONS

//virtual
StatementConstruction&
StatementConstruction::
append(int iDataID_, Execution::Interface::IAction* pAction_)
{
	if (!m_cStream.isEmpty()) {
		m_vecStatement.PUSHBACK(m_cStream.getString());
		m_vecType.PUSHBACK(STATEMENT);
		m_cStream.clear();
	}
	
	m_vecData.PUSHBACK(Execution::Action::DataHolder(iDataID_));
	m_vecType.PUSHBACK(DATA_ID);
	if (pAction_)
		m_cPreActions.addID(pAction_->getID());
	
	return *this;
}

// FUNCTION public
//	Action::StatementConstruction::add
//
// NOTES
//
// ARGUMENTS
//	STRING& cstrSQL_
//	
// RETURN
//	StatementConstruction&
//
// EXCEPTIONS

//virtual
StatementConstruction&
StatementConstruction::
append(const STRING& cstrSQL_)
{
	m_cStream << cstrSQL_;
	return *this;
}


// FUNCTION public
//	Action::StatementConstruction::add
//
// NOTES
//
// ARGUMENTS
//	STRING& cstrSQL_
//	
// RETURN
//	StatementConstruction&
//
// EXCEPTIONS

//virtual
StatementConstruction&
StatementConstruction::
appendPlaceHolder(int iDataID_, Execution::Interface::IAction* pAction_)
{
	if (m_bConcatinate) _SYDNEY_THROW0(Exception::NotSupported);
	if (!m_cStream.isEmpty()) {
		m_vecStatement.PUSHBACK(m_cStream.getString());
		m_vecType.PUSHBACK(STATEMENT);
		m_cStream.clear();
	}
	
	m_vecType.PUSHBACK(PLACEHOLDER);
	m_vecPlaceHolder.PUSHBACK(iDataID_);
	if (pAction_)
		m_cPreActions.addID(pAction_->getID());
	
	return *this;
}

// FUNCTION public
//	Action::StatementConstruction::add
//
// NOTES
//
// ARGUMENTS
//	STRING& cstrSQL_
//	
// RETURN
//	StatementConstruction&
//
// EXCEPTIONS

//virtual
StatementConstruction&
StatementConstruction::
appendArrayPlaceHolder(int iDataID_, Execution::Interface::IAction* pAction_)
{
	if (m_bConcatinate) _SYDNEY_THROW0(Exception::NotSupported);
	if (!m_cStream.isEmpty()) {
		m_vecStatement.PUSHBACK(m_cStream.getString());
		m_vecType.PUSHBACK(STATEMENT);
		m_cStream.clear();
	}
	
	m_vecType.PUSHBACK(ARRAY_PLACEHOLDER);
	m_vecPlaceHolder.PUSHBACK(iDataID_);
	if (pAction_)
		m_cPreActions.addID(pAction_->getID());
	
	return *this;
}


// FUNCTION public
//	Action::StatementConstruction::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	StatementConstruction*
//
// EXCEPTIONS

//static
StatementConstruction*
StatementConstruction::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::StatementConstruction);
	return new StatementConstruction;
}


// FUNCTION public
//	Action::StatementConstruction::readyToSetData
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
StatementConstruction::readyToSet()
{
	if (!m_cStream.isEmpty()) {
		m_vecStatement.PUSHBACK(m_cStream.getString());
		m_vecType.PUSHBACK(STATEMENT);
		m_cStream.clear();
	}
}



// FUNCTION public
//	Action::StatementConstruction::toStatement
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
StatementConstruction::
toStatement(Execution::Interface::IProgram& cProgram_)
{
	VECTOR<STRING>::ConstIterator iteStatement = m_vecStatement.begin();
	VECTOR<int>::ConstIterator iteType =  m_vecType.begin();
	VECTOR<Execution::Action::DataHolder>::
		ConstIterator iteData = m_vecData.begin();
	VECTOR<Execution::Action::DataHolder>::
		ConstIterator itePlaceHolder = m_vecPlaceHolder.begin();

	OSTRSTREAM cStream;
	Common::DataArrayData* pParam = 0;
	_SYDNEY_ASSERT(m_cSqlData->getCount() > 1);
	pParam = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, m_cSqlData->getElement(1).get());

	for (; iteType != m_vecType.end(); ++iteType) {
		switch (*iteType) {
		case STATEMENT:
		{
			;_SYDNEY_ASSERT(iteStatement != m_vecStatement.end());
			cStream << (*iteStatement);
			iteStatement++;
			break;
		}
		case DATA_ID:
		{
			cStream << (*iteData)->getString();
			iteData++;
			break;
		}
		case PLACEHOLDER:
		{
			cStream << " ? ";
			_SYDNEY_ASSERT(pParam);
			pParam->pushBack((*itePlaceHolder).getData());
			itePlaceHolder++;
			break;
		}
		case ARRAY_PLACEHOLDER:
		{
			const Common::Data* pData = (*itePlaceHolder).getData();
			if (pData->getType() != Common::DataType::Array
				|| pData->getElementType() != Common::DataType::Data) {
				_SYDNEY_THROW0(Exception::Unexpected);
			}
			const Common::DataArrayData* pArrayData =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData);
			
			char c = ' ';
			for (int i = 0; i < pArrayData->getCount(); ++i, c = ',') {
				cStream << c << " ? ";
				pParam->pushBack(pArrayData->getElement(i));
			}

			itePlaceHolder++;
			break;
		}
		
		default:
			_SYDNEY_ASSERT(false);
		}
	}

	
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "SQL: "<< cStream.getString() << ModEndl;
	}
#endif
	Common::StringData* pStatement =
		_SYDNEY_DYNAMIC_CAST(Common::StringData*, m_cSqlData->getElement(0).get());;
	if (m_bConcatinate) {
		cStream << ";";
		if (pStatement->isNull()) {
			pStatement->setValue(STRING(cStream.getString()));
		} else {
			pStatement->connect(&Common::StringData(cStream.getString()));
		}
	} else {
		pStatement->setValue(STRING(cStream.getString()));
	}
}


// FUNCTION public
//	Action::StatementConstruction::createStringParameter
//
// NOTES
//	STRINGパラメーターに対して前後のシングルクォーテーションの追加と
//	パラメーター内のシングルクォーテーションを重ねてエスケープします。
//
// ARGUMENTS
//	const STRING& in
//	STRING& out
//
// RETURN
//	void
//
// EXCEPTIONS

void
StatementConstruction::
createStringParameter(const STRING& in, STRING& out) const
{
	out.append(" '");
	if (in.search("'") != 0) {
		for (int i = 0; i < in.getLength(); ++i) {
			if (in[i] == '\'') {
				out.append("''");
			} else {
				out.append(in[i]);
			}
		}
	} else {
		out.append(in);
	}
	out.append("' ");
}




// FUNCTION public
//	Action::StatementConstruction::getClassID -- 
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
StatementConstruction::
getClassID() const
{
	return Class::getClassID(Class::Category::StatementConstruction);
}

// FUNCTION public
//	Action::StatementConstruction::serialize -- 
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
StatementConstruction::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	Execution::Utility::SerializeObject(archiver_, m_vecData);
	Execution::Utility::SerializeObject(archiver_, m_vecPlaceHolder);
	Execution::Utility::SerializeValue(archiver_, m_vecStatement);
	Execution::Utility::SerializeValue(archiver_, m_vecType);
	
	m_cSqlData.serialize(archiver_);
	m_cPreActions.serialize(archiver_);
	archiver_(m_bConcatinate);
}





_SYDNEY_DEXECUTION_ACTION_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
