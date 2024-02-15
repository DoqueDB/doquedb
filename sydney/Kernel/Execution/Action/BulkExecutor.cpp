// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/BulkExecutor.cpp --
// 
// Copyright (c) 2006, 2007, 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Execution/Action/BulkExecutor.h"
#include "Execution/Action/BulkFile.h"
#include "Execution/Action/BulkParameter.h"
#include "Execution/Action/BulkParser.h"
#include "Execution/Action/BulkWriter.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Common/UnicodeString.h"

#include "Exception/InvalidDataFile.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

// FUNCTION public
//	Action::BulkExecutor::BulkExecutor -- 
//
// NOTES
//
// ARGUMENTS
//	const BulkParameter& cParameter_
//	const VECTOR<Common::SQLData>& vecDataType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

BulkExecutor::
BulkExecutor(const BulkParameter& cParameter_,
			 const VECTOR<Common::SQLData>& vecDataType_)
	: Super(),
	  m_cParameter(cParameter_),
	  m_vecDataType(vecDataType_),
	  m_pFile(0),
	  m_pErrorFile(0),
	  m_pTempFile(0),
	  m_pParser(0),
	  m_pWriter(0),
	  m_pRecordSelector(0),
	  m_iLine(0)
{}

// FUNCTION public
//	Action::BulkExecutor::~BulkExecutor -- 
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

BulkExecutor::
~BulkExecutor()
{
	try {
		terminate();
	} catch (...) {
		// ignore the exception
		SydInfoMessage << "Destructor caught an exception. ignored." << ModEndl;
	}
}

// FUNCTION public
//	Action::BulkExecutor::reset -- reset iteration
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
BulkExecutor::
reset()
{
	m_pFile->reset();
	m_iLine = 0;
}

// FUNCTION public
//	Action::BulkExecutor::next -- goto next tuple
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
BulkExecutor::
next()
{
	return m_pParser->hasMoreData();
}

// FUNCTION public
//	Action::BulkExecutor::get -- get current tuple
//
// NOTES
//
// ARGUMENTS
//	Common::DataArrayData& cTuple_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkExecutor::
get(Common::DataArrayData& cTuple_)
{
	; _SYDNEY_ASSERT(m_vecDataType.GETSIZE() == cTuple_.getCount());

	bool bResult = false;
	do {
		// is the record to be skipped?
		++m_iLine;
		if (m_pRecordSelector) {
			if (!m_pRecordSelector->isValid(m_iLine)) {
				if (m_pRecordSelector->isEnd()) break;
				m_pParser->skipTuple(1);
				continue;
			}
		}
		// get a tuple
		if (m_pParser->getTuple(cTuple_, m_vecDataType)) {
			bResult = true;
			break;
		} else {
			error();
		}
	} while (next());
	return bResult;
}

// FUNCTION public
//	Action::BulkExecutor::put -- put a tuple
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData& cTuple_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkExecutor::
put(const Common::DataArrayData& cTuple_)
{
	m_pWriter->putTuple(cTuple_);
}

// FUNCTION public
//	Action::BulkExecutor::isValid -- current is valid position?
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
BulkExecutor::
isValid()
{
	return m_pParser->hasMoreData();
}

// FUNCTION public
//	Action::BulkExecutor::initialize -- initialize object
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
BulkExecutor::
initialize()
{
	if (!m_pFile) {
		m_pFile = new BulkFile(m_cParameter.getPath(), m_cParameter.isInput());
		if (!m_pParser) {
			// parser is initialized for output too, so that separator can be checked
			m_pParser = new BulkParser(*m_pFile, m_cParameter);
			m_pParser->initialize(m_cParameter.isNoDoubleQuote());
		}
		if (!m_cParameter.isInput() && !m_pWriter) {
			m_pWriter = new BulkWriter(*m_pFile, m_cParameter);
			m_pWriter->initialize();
		}
	}
	if (m_cParameter.isInput() && !m_pRecordSelector
		&& !m_cParameter.getInputRecord().getValue().ISEMPTY()) {
		m_pRecordSelector = new BulkParameter::ElementIterator(m_cParameter.getInputRecord());
	}
	// create errorfile and tempfile only when they are needed
}

// FUNCTION public
//	Action::BulkExecutor::terminate -- terminate object
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
BulkExecutor::
terminate()
{
	if (m_pFile) {
		if (m_pWriter) {
			m_pWriter->terminate();
			delete m_pWriter, m_pWriter = 0;
		}
		if (m_pParser) {
			m_pParser->terminate();
			delete m_pParser, m_pParser = 0;
		}
		delete m_pFile, m_pFile = 0;
	}
	if (m_pRecordSelector) {
		delete m_pRecordSelector, m_pRecordSelector = 0;
	}
	if (m_pErrorFile) {
		m_pErrorFile->terminate();
		delete m_pErrorFile, m_pErrorFile = 0;
	}
	if (m_pTempFile) {
		m_pTempFile->terminate();
		delete m_pTempFile, m_pTempFile = 0;
	}
	m_iLine = 0;
}

// FUNCTION private
//	Action::BulkExecutor::error -- error handling
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
BulkExecutor::
error()
{
	if (m_cParameter.isInput()
		&& (m_pErrorFile || (m_cParameter.getErrorPath().getLength() > 0))) {

		// prepare file object at first call
		if (!m_pErrorFile) {
			m_pErrorFile = new BulkFile(m_cParameter.getErrorPath(), false /* readwrite */);
			m_pErrorFile->initialize();
		}
		if (!m_pTempFile) {
			m_pTempFile = new BulkFile(m_cParameter.getPath(), true /* read only */);
			m_pTempFile->initialize();
		}

		// output error part to error file
		m_pErrorFile->putData(*m_pTempFile, m_pParser->getRecordTop(), m_pParser->getRecordTail());
	} else {
		// raise an exception
		_SYDNEY_THROW1(Exception::InvalidDataFile, m_iLine);
	}
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2006, 2007, 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
