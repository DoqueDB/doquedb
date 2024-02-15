// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Explain.cpp -- Output plan tree explanation
// 
// Copyright (c) 2007, 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Opt";
	const char srcFile[] = __FILE__;
}

#include <stdio.h>

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Opt/Explain.h"

#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"
#include "Common/DataArrayData.h"
#include "Common/ResultSetMetaData.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"

#include "Communication/Connection.h"

#include "Exception/SQLSyntaxError.h"

#include "Os/Limits.h"

#include "Statement/ExplainOption.h"
#include "Statement/Hint.h"
#include "Statement/HintElement.h"
#include "Statement/HintElementList.h"
#include "Statement/Literal.h"
#include "Statement/Token.h"

#include "ModCharTrait.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

namespace
{
	// max line length in output
	const int _iMaxLengthTable[] = {
		70, 70, 70, 70, 70, // indent level 0~4
		90, 90, 90, 90, 90, // indent level 5~9
	   110,110,110,110,110, // indent level 10~14
	};
	const int _iMaxLengthTableSize = sizeof(_iMaxLengthTable) / sizeof(int);
	const int _iMaxLineLength = 130;
	const int _iMaxContinueLength = 50;

	// get line length of character string
	ModSize _stringWidth(const ModUnicodeString& cstrString_);

	// parse hint
	Explain::Option::Value _parseHint(const Statement::Object& cStatement_);
	// check one string
	bool _checkString(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
					  const ModUnicodeString& cKey_);
	// parse one string
	bool _parseString(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
					  Explain::Option::Value& iResult_);

	// FUNCTION local
	//	$$$::_stringWidth -- get line length of character string
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeString& cstrString_
	//	
	// RETURN
	//	ModSize
	//
	// EXCEPTIONS
	ModSize
	_stringWidth(const ModUnicodeString& cstrString_)
	{
		ModSize n = cstrString_.getLength();
		const ModUnicodeChar* p = cstrString_;
		const ModUnicodeChar* pTail = cstrString_.getTail();
		for (; p < pTail; ++p) {
			if (!ModUnicodeCharTrait::isAscii(*p)) ++n;
		}
		return n;
	}

	// FUNCTION local
	//	$$$::_parseHint -- parse hint
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Statement::Object& cStatement_
	//	
	// RETURN
	//	Explain::Option::Value
	//
	// EXCEPTIONS
	Explain::Option::Value
	_parseHint(const Statement::Object& cStatement_)
	{
		Explain::Option::Value iResult = Explain::Option::None;
		switch (cStatement_.getType()) {
		case Statement::ObjectType::Literal:
			{
				// following code assumes no keyword include single quotation mark
				const Statement::Literal& cLiteral =
					_SYDNEY_DYNAMIC_CAST(const Statement::Literal&, cStatement_);
				if (cLiteral.isStringLiteral()) {
					const Statement::Token& cToken = cLiteral.getToken();
					if (_parseString(cToken.getHead(), cToken.getTail(), iResult)) {
						break; // success
					}
				}
				// Illegal hint
				ModUnicodeOstrStream cStream;
				cStream << "Explain: unknown hint element";
				_SYDNEY_THROW1(Exception::SQLSyntaxError, cStream.getString());
			}
		case Statement::ObjectType::StringValue:
		case Statement::ObjectType::ItemReference:
			{
				// Illegal hint
				ModUnicodeOstrStream cStream;
				cStream << "Explain: unknown hint element";
				_SYDNEY_THROW1(Exception::SQLSyntaxError, cStream.getString());
			}
		case Statement::ObjectType::HintElement:
			{
				const Statement::HintElement& cElement =
					_SYDNEY_DYNAMIC_CAST(const Statement::HintElement&, cStatement_);
				int n = cElement.getHintPrimaryCount();
				for (int i = 0; i < n; ++i) {
					iResult |= _parseHint(*(cElement.getHintPrimaryAt(i)));
				}
				break;
			}
		case Statement::ObjectType::HintElementList:
			{
				const Statement::HintElementList& cList =
					_SYDNEY_DYNAMIC_CAST(const Statement::HintElementList&, cStatement_);
				int n = cList.getCount();
				for (int i = 0; i < n; ++i) {
					iResult |= _parseHint(*(cList.getAt(i)));
				}
				break;
			}
		}
		return iResult;
	}

	// FUNCTION local
	//	$$$::_checkString -- check one string
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeChar* pHead_
	//	const ModUnicodeChar* pTail_
	//	const ModUnicodeString& cKey_
	//	
	// RETURN
	//		bool
	//
	// EXCEPTIONS
	bool
	_checkString(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
				 const ModUnicodeString& cKey_)
	{
		int iLength = cKey_.getLength();
		if (pTail_ - pHead_ >= iLength) {
			return (ModUnicodeCharTrait::compare(pHead_, cKey_,
												 ModFalse /* case insensitive */,
												 iLength) == 0)
				? true : false;
		}
		return false;
	}

	// FUNCTION local
	//	$$$::_parseString -- parse hint string
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeChar* pHead_
	//	const ModUnicodeChar* pTail_
	//	Explain::Option::Value& iResult_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS
	bool
	_parseString(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
				 Explain::Option::Value& iResult_)
	{					  
		const ModUnicodeChar* p = pHead_;
		while (p < pTail_) {
			switch (*p) {
			case Common::UnicodeChar::usSmallC:
			case Common::UnicodeChar::usLargeC:
				{
					// COST?
					if (_checkString(p, pTail_, ModUnicodeString("cost"))) {
						iResult_ |= Explain::Option::Cost;
						p += 4;
						break;
					}
					// fail
					return false;
				}
			case Common::UnicodeChar::usSmallD:
			case Common::UnicodeChar::usLargeD:
				{
					// DATA?
					if (_checkString(p, pTail_, ModUnicodeString("data"))) {
						iResult_ |= Explain::Option::Data;
						p += 4;
						break;
					}
					// fail
					return false;
				}
			case Common::UnicodeChar::usSmallF:
			case Common::UnicodeChar::usLargeF:
				{
					// FILE?
					if (_checkString(p, pTail_, ModUnicodeString("file"))) {
						iResult_ |= Explain::Option::File;
						p += 4;
						break;
					}
					// fail
					return false;
				}
			case Common::UnicodeChar::usSmallL:
			case Common::UnicodeChar::usLargeL:
				{
					// LOCK?
					if (_checkString(p, pTail_, ModUnicodeString("lock"))) {
						iResult_ |= Explain::Option::Lock;
						p += 4;
						break;
					}
					// fail
					return false;
				}
			default:
				// fail
				return false;
			}
			if (p < pTail_) {
				// skip white space
				if (*p == Common::UnicodeChar::usSpace
					|| *p == Common::UnicodeChar::usCtrlTab
					|| *p == Common::UnicodeChar::usCtrlRet
					|| *p == Common::UnicodeChar::usCtrlCr) {
					while (++p < pTail_ &&
						   (*p == Common::UnicodeChar::usSpace
							|| *p == Common::UnicodeChar::usCtrlTab
							|| *p == Common::UnicodeChar::usCtrlRet
							|| *p == Common::UnicodeChar::usCtrlCr)) {
						; // empty
					}
				} else {
					// fail
					return false;
				}
			}
		}
		return true;
	}
}

// FUNCTION
//	Opt::Explain::getOption -- get option value
//
// NOTES
//
// ARGUMENTS
//	const Statement::ExplainOption& cStatement_
//	
// RETURN
//	Explain::Option::Value
//
// EXCEPTIONS

// static
Explain::Option::Value
Explain::
getOption(const Statement::ExplainOption& cStatement_)
{
	Option::Value iResult = static_cast<Option::Value>(cStatement_.getOption());
	iResult |= Option::Explain;
	// analyze hint
	Statement::Hint* pHint = cStatement_.getHint();
	if (pHint) {
		; _SYDNEY_ASSERT(pHint->getHintElementCount() == 1);
		Statement::HintElement* pElement = pHint->getHintElementAt(0);
		iResult |= _parseHint(*pElement);
	}
	return iResult;
}

// FUNCTION public
//	Opt::Explain::initialize -- initialize
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
Explain::
initialize()
{
	if (m_pConnection) {
		// write metadata
		Common::ColumnMetaData cColumnMeta(Common::SQLData(Common::SQLData::Type::NChar,
														   Common::SQLData::Flag::Variable,
														   0, 0));
		cColumnMeta.setColumnAliasName(ModUnicodeString("Plan"));
		cColumnMeta.setReadOnly();

		Common::ResultSetMetaData cMetaData;
		cMetaData.reserve(1);
		cMetaData.pushBack(cColumnMeta);
		m_pConnection->writeObject(&cMetaData);
	}
	// initialize new line forced vector
	m_vecNewLineForced.pushBack(false); // for 0-level
}

// FUNCTION public
//	Opt::Explain::terminate -- terminate
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
Explain::
terminate()
{
	if (m_pConnection) {
		// send 0 to tell end of data
		m_pConnection->writeObject(0);
	}
}

// FUNCTION public
//	Opt::Explain::flush -- put result to client
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
Explain::
flush()
{
	if (m_pConnection) {
		// send result
		Common::DataArrayData cTuple;
		cTuple.pushBack(new Common::StringData(getString()));
		m_pConnection->writeObject(&cTuple);
	}
	// clear stream
	m_cStream.clear();
	m_iIndentLevel = 0;
	m_iLineLength =0;
	m_bNewLine = true;
	m_vecNewLineForced.clear();
	m_iNoNewLineLevel = 0;

	// initialize new line forced vector again
	m_vecNewLineForced.pushBack(false); // for 0-level
}

// FUNCTION public
//	Opt::Explain::getString -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeChar*
//
// EXCEPTIONS

ModUnicodeChar*
Explain::
getString()
{
	return m_cStream.getString();
}

// FUNCTION public
//	Opt::Explain::isEmpty -- stream is empty?
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
Explain::
isEmpty()
{
	return m_cStream.isEmpty();
}

// FUNCTION public
//	Opt::Explain::pushIndent -- increase indent level
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
pushIndent()
{
	++m_iIndentLevel;
	m_vecNewLineForced.pushBack(false);
	return *this;
}

// FUNCTION public
//	Opt::Explain::popIndent -- descrease indent level
//
// NOTES
//
// ARGUMENTS
//	bool bNewLine_ = false
//		If true, new line is forced for outer level
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
popIndent(bool bNewLine_ /* = false */)
{
	if (--m_iIndentLevel < 0) m_iIndentLevel = 0;
	m_vecNewLineForced.popBack();
	; _SYDNEY_ASSERT(m_vecNewLineForced.isEmpty() == false);
	if (bNewLine_) {
		forceNewLine();
	}
	return *this;
}

// FUNCTION public
//	Opt::Explain::forceNewLine -- force new line in next put
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
forceNewLine()
{
	if (!m_vecNewLineForced.isEmpty()) {
		m_vecNewLineForced.getBack() = true;
	}
	return *this;
}

// FUNCTION public
//	Opt::Explain::pushNoNewLine -- set no new line
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
pushNoNewLine()
{
	++m_iNoNewLineLevel;
	return *this;
}

// FUNCTION public
//	Opt::Explain::popNoNewLine -- unset no new line
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
popNoNewLine()
{
	if (--m_iNoNewLineLevel < 0) m_iNoNewLineLevel = 0;
	return *this;
}

// FUNCTION public
//	Opt::Explain::newLine -- add new line
//
// NOTES
//
// ARGUMENTS
//	bool bForce_ = false
//
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
newLine(bool bForce_ /* = false */)
{
	static const int _iIndentTableSize = 4;
	static const int _iIndentUnit = 4;
	static const char* const _pszIndent[_iIndentTableSize+1] =
	{
		"",
		"    ",
		"        ",
		"            ",
		"                "
	};
	if (bForce_ || m_vecNewLineForced.getBack()
		|| m_iLineLength > _iMaxContinueLength) {
		m_cStream << "\n";
		if (m_iIndentLevel > 0) {
			int d = m_iIndentLevel / _iIndentTableSize;
			int r = m_iIndentLevel % _iIndentTableSize;
			for (int i = 0; i < d; ++i) {
				m_cStream << _pszIndent[_iIndentTableSize];
			}
			m_cStream << _pszIndent[r];
		}
		m_iLineLength = m_iIndentLevel * _iIndentUnit;

		// reset new line forcing
		m_vecNewLineForced.getBack() = false;
		// new line has begun
		m_bNewLine = true;
	} else {
		// use space instead of newline
		m_cStream << " ";
		++m_iLineLength;
	}
	return *this;
}

// FUNCTION public
//	Opt::Explain::put -- add string
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrValue_
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
put(const ModUnicodeString& cstrValue_)
{
	checkLength(_stringWidth(cstrValue_));
	m_cStream << cstrValue_;
	m_bNewLine = false;
	return *this;
}

// FUNCTION public
//	Opt::Explain::put -- add string
//
// NOTES
//
// ARGUMENTS
//	const char* pszValue_
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
put(const char* pszValue_)
{
	checkLength(ModCharTrait::length(pszValue_));
	m_cStream << pszValue_;
	m_bNewLine = false;
	return *this;
}

// FUNCTION public
//	Opt::Explain::put -- add string
//
// NOTES
//
// ARGUMENTS
//	double dblValue_
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
put(double dblValue_)
{
	static const int _iResultLength = 10; // x.xxxe-xxx
	checkLength(static_cast<ModSize>(_iResultLength));

	// change precision
	char pszBuffer[_iResultLength + 1]; 
	::sprintf(pszBuffer, "%.3e", dblValue_);
	m_cStream << pszBuffer;
	m_bNewLine = false;
	return *this;
}

// FUNCTION public
//	Opt::Explain::put -- add string
//
// NOTES
//
// ARGUMENTS
//	int iValue_
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
put(int iValue_)
{
	checkLength(static_cast<ModSize>(Os::Limits<int>::getDig() + (iValue_ < 0 ? 1 : 0)));
	m_cStream << iValue_;
	m_bNewLine = false;
	return *this;
}

// FUNCTION public
//	Opt::Explain::put -- add string
//
// NOTES
//
// ARGUMENTS
//	ModSize uValue_
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
put(ModSize uValue_)
{
	checkLength(static_cast<ModSize>(Os::Limits<ModSize>::getDig()));
	m_cStream << uValue_;
	m_bNewLine = false;
	return *this;
}

// FUNCTION public
//	Opt::Explain::put -- add string
//
// NOTES
//
// ARGUMENTS
//	Lock::Mode::Value eValue_
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
put(Lock::Mode::Value eValue_)
{
	checkLength(5); // max length: 'VSVIX'
	m_cStream << eValue_;
	m_bNewLine = false;
	return *this;
}

// FUNCTION public
//	Opt::Explain::put -- add string
//
// NOTES
//
// ARGUMENTS
//	Lock::Duration::Value eValue_
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
put(Lock::Duration::Value eValue_)
{
	checkLength(9); // max length: 'Statement'
	m_cStream << eValue_;
	m_bNewLine = false;
	return *this;
}

// FUNCTION public
//	Opt::Explain::putChar -- add char
//
// NOTES
//
// ARGUMENTS
//	char cValue_
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

Explain&
Explain::
putChar(char cValue_)
{
	m_cStream << cValue_;
	return *this;
}

// FUNCTION private
//	Opt::Explain::checkLength -- check line length to insert new line
//
// NOTES
//
// ARGUMENTS
//	ModSize iLength_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Explain::
checkLength(ModSize iLength_)
{
	if (!m_bNewLine && m_iNoNewLineLevel == 0
		&& m_iLineLength + iLength_ > ((m_iIndentLevel >= _iMaxLengthTableSize)
									   ? _iMaxLineLength
									   : _iMaxLengthTable[m_iIndentLevel])) {
		// force to insert new line
		newLine(true);
	}
	m_iLineLength += iLength_;
}

//
//	Copyright (c) 2007, 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
