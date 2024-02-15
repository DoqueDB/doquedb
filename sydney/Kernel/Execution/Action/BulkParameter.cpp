// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/BulkParameter.cpp --
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Action/BulkParameter.h"
#include "Execution/Action/BulkFile.h"
#include "Execution/Action/BulkParser.h"
#include "Execution/Action/BulkSeparator.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"

#include "Exception/InvalidBulkParameter.h"
#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"

#include "Os/AutoCriticalSection.h"
#include "Os/File.h"
#include "Os/Limits.h"
#include "Os/Path.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

namespace
{
	/////////
	// special tokens
	/////////
	// FILE is used as a separator
	const char* const _pszFile = "FILE";
	const int _iFileKeywordLength = 4;
	// others are compared directly
	const char* const _pszNull = "NULL";
	const int _iNullKeywordLength = 4;
	const char* const _pszDefault = "DEFAULT";
	const int _iDefaultKeywordLength = 7;

	// default separator
	const ModUnicodeString _cstrDefaultFieldSeparator(",");
	const ModUnicodeString _cstrDefaultRecordSeparator("\\n");
	const ModUnicodeString _cstrDefaultElementSeparator(";");

	// spec separator
	const ModUnicodeString _cstrSpecFieldSeparator("");
	const ModUnicodeString _cstrSpecRecordSeparator("\\n");
	const ModUnicodeString _cstrSpecElementSeparator("");
	const ModUnicodeChar _cSpecComment = Common::UnicodeChar::usSharp;

	// encoding values
	const ModUnicodeString _cstrUtf8("utf-8");
	
	// enum denoting each keywords
	// [NOTES] If any entry in this enum are added, _cHintKeyTable below should be modified
	//			BulkSeparator::_MaxSeparatorCount also should be modified
	struct HintKey
	{
		enum Value
		{
			FieldSeparator = 0,
			RecordSeparator,
			ElementSeparator,
			DateDelimiter,
			DateTimeDelimiter,
			TimeDelimiter,
			MsecDelimiter,
			Code,
			InputField,
			InputRecord,
			ExternField,
			CommitCount,
			ErrorData,
			NoExtern,
			NoElementNull,
			NoDoubleQuote,
			ValueNum,

			// special symbol
			Terminator
		};
	};

	const bool _bNoValueTable[HintKey::ValueNum] =
	{
		false,							// FieldSeparator
		false,							// RecordSeparator
		false,							// ElementSeparator
		false,							// DateDelimiter
		false,							// DateTimeDelimiter
		false,							// TimeDelimiter
		false,							// MsecDelimiter
		false,							// Code
		false,							// InputField
		false,							// InputRecord
		false,							// ExternField
		false,							// CommitCount
		false,							// ErrorData
		true,							// NoExtern
		true,							// NoElementNull
		true,							// NoDoubleQuote
	};

	// key name
	const char* _cHintKeyTable[HintKey::ValueNum] =
	{
		"FieldSeparator",
		"RecordSeparator",
		"ElementSeparator",
		"DateDelimiter",
		"DateTrailer",
		"TimeDelimiter",
		"MsecDelimiter",
		"Code",
		"InputField",
		"InputRecord",
		"ExternField",
		"CommitCount",
		"ErrorData",
		"NoExtern",
		"NoElementNull",
		"NoDoubleQuote",
	};

	// latch
	Os::CriticalSection _latch;
	// separtor for hint key
	BulkSeparator _cHintSeparator;
	// flag whether separator is initialized
	bool _bInitialized = false;

	// FUNCTION public
	//	$$::_getInteger -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeChar*& p_
	//		[in] top of target string
	//		[out] end of number part
	//	
	// RETURN
	//	int
	//
	// EXCEPTIONS

	int _getInteger(const ModUnicodeChar*& p_)
	{
#define IS_NUMBER(__c__) (((__c__) >= Common::UnicodeChar::usZero) \
						  && (((__c__) <= Common::UnicodeChar::usNine)))

		if (!IS_NUMBER(*p_)) {
			// Not a number
			_SYDNEY_THROW0(Exception::InvalidBulkParameter);
		}
		const ModUnicodeChar* pHead = p_;
		// scan to the last number
		while (IS_NUMBER(*(p_ + 1))) {++p_;}

		// create result scanning from the tail of number sequence
		int iResult = 0;
		int iPower = 1;
		int iLimit = Os::Limits<int>::getMax();

		const ModUnicodeChar* pTail = p_;
		while (pHead <= pTail) {
			int v = (*pTail) - Common::UnicodeChar::usZero;
			if (iLimit / iPower < v) {
				// Exceeds the limit
				_SYDNEY_THROW0(Exception::InvalidBulkParameter);
			}
			iResult += v * iPower;
			iLimit -= v * iPower;
			iPower *= 10;
			--pTail;
		}
		if (iResult == 0) {
			// number should be larger than zero
			_SYDNEY_THROW0(Exception::InvalidBulkParameter);
		}
		return iResult;
#undef IS_NUMBER
	}

}

///////////////////////
// ElementSpecification
///////////////////////

// FUNCTION public
//	Execution::Action::BulkParameter::ElementSpecification::setValue -- set element specification value from a string
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkParameter::ElementSpecification::
setValue(const ModUnicodeChar* pHead_,
		 const ModUnicodeChar* pTail_)
{
	if (pHead_ == pTail_) {
		return;
	}

	// head should be '(' and char before tail should be ')'
	if ((*pHead_ != Common::UnicodeChar::usLparent)
		||
		(*(pTail_ - 1) != Common::UnicodeChar::usRparent)) {
		_SYDNEY_THROW0(Exception::InvalidBulkParameter);
	}

	int iMin = 1;
	int iMax = 0;
	int* piTarget = &iMin;
	for (const ModUnicodeChar* p = pHead_ + 1; p < pTail_ - 1; ++p) {
		while (*p == Common::UnicodeChar::usSpace) ++p;
		if (*p == Common::UnicodeChar::usComma) {
			if (piTarget == &iMin) {
				iMax = iMin;
			}
			addSpec(iMin, iMax);
			piTarget = &iMin;
		} else if (*p == Common::UnicodeChar::usHyphen) {
			piTarget = &iMax;
			iMax = Os::Limits<int>::getMax(); // default value
		} else {
			*piTarget = _getInteger(p);
		}
	}
	if (piTarget == &iMin) {
		iMax = iMin;
	}
	addSpec(iMin, iMax);
}

// FUNCTION public
//	Execution::Action::BulkParameter::ElementSpecification::addSpec -- add a spec pair
//
// NOTES
//
// ARGUMENTS
//	int iMin_
//	int iMax_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkParameter::ElementSpecification::
addSpec(int iMin_, int iMax_)
{
	if ((iMax_ == 0) || (iMin_ > iMax_)
		|| (!m_vecSpec.ISEMPTY()
			&& (m_vecSpec.GETBACK().second >= iMin_))) {
		// iMin_ == 0 means no number before the comma
		// iMin_ should not be greater than iMax_
		// iMin must be greater than previous iMax
		_SYDNEY_THROW0(Exception::InvalidBulkParameter);
	}
	m_vecSpec.PUSHBACK(PAIR<int, int>(iMin_, iMax_));
}

///////////////////////
// ElementIterator
///////////////////////

// FUNCTION public
//	Execution::Action::BulkParameter::ElementIterator::isValid -- check number
//
// NOTES
//	check a number according to the spec string.
//	if spec string is '1-3', then 1, 2, 3 return true.
//	if spec string is '1,5', then 1, 5 return true.
//
// ARGUMENTS
//	int i_
//		checked number
//		this number should monotonically increase until reset is called
//	
// RETURN
//	bool
//		true ... the number matches the spec
//
// EXCEPTIONS

bool
BulkParameter::ElementIterator::
isValid(int i_)
{
	if (m_vecSpec.ISEMPTY()) {
		return true;
	}

	if ((m_cIterator != m_vecSpec.end())
		&& ((*m_cIterator).second < i_)) {
		++m_cIterator;
	}

	return (m_cIterator != m_vecSpec.end())
		&& ((*m_cIterator).first <= i_);
}

// FUNCTION public
//	Action::BulkParameter::ElementIterator::isEnd -- check status
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
BulkParameter::ElementIterator::
isEnd()
{
	return m_cIterator == m_vecSpec.end();
}

// FUNCTION public
//	Execution::Action::BulkParameter::ElementIterator::reset -- reset status
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
BulkParameter::ElementIterator::
reset()
{
	m_cIterator = m_vecSpec.begin();
}

///////////////////////
// BulkParameter
///////////////////////

// FUNCTION public
//	Execution::Action::BulkParameter::BulkParameter -- constructors
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

BulkParameter::
BulkParameter()
	: Super(),
	  m_cPath(),
	  m_cErrorPath(),
	  m_cFieldSeparator(),
	  m_cRecordSeparator(),
	  m_cElementSeparator(),
	  m_cDateDelimiter(0),
	  m_cDateTimeDelimiter(0),
	  m_cTimeDelimiter(0),
	  m_cMsecDelimiter(0),
	  m_eEncoding(ModKanjiCode::literalCode),
	  m_cInputField(),
	  m_cInputRecord(),
	  m_cExternField(),
	  m_iCommitCount(-1),
	  m_bIsNoExtern(false),
	  m_bIsNoElementNull(false),
	  m_bIsNoDoubleQuote(false),
	  m_bIsInput(false)
{}

// FUNCTION public
//	Execution::Action::BulkParameter::~BulkParameter -- 
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

BulkParameter::
~BulkParameter()
{}

// FUNCTION public
//	Execution::Action::BulkParameter::setValues -- set values
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrData_
//		Data file path. just used as a reference.
//	const ModUnicodeString& cstrWith_
//		Spec file path. contents are read and parameters are set 
//	const ModUnicodeString& cstrHint_
//		Parameters are set according to the hint string
//	bool bIsInput_
//		a flag denoting the bulk file is used for input or output
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//	Exception::SQLSyntaxError ... hint string has illegal format

void
BulkParameter::
setValues(const ModUnicodeString& cstrData_,
		  const ModUnicodeString& cstrWith_,
		  const ModUnicodeString& cstrHint_,
		  bool bIsInput_)
{
	if (cstrData_.getLength() == 0) {
		_SYDNEY_THROW0(Exception::InvalidBulkParameter);
	}
	m_cPath = cstrData_;

	// initialize hint parser
	initializeParser();

	if (cstrHint_.getLength() > 0) {
		// set hint first because encoding should be used in further process
		parseHint(cstrHint_);
	}

	if (cstrWith_.getLength() > 0) {
		// Read spec character string from the file
		readSpecFile(cstrWith_);
	}
	m_bIsInput = bIsInput_;
}

// FUNCTION public
//	Action::BulkParameter::getFieldSeparator -- get field separator
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const ModUnicodeString&
//
// EXCEPTIONS

const ModUnicodeString&
BulkParameter::
getFieldSeparator() const
{
	return (m_cFieldSeparator.getLength() == 0)
		? _cstrDefaultFieldSeparator : m_cFieldSeparator;
}

// FUNCTION public
//	Action::BulkParameter::getRecordSeparator -- get record separator
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const ModUnicodeString&
//
// EXCEPTIONS

const ModUnicodeString&
BulkParameter::
getRecordSeparator() const
{
	return (m_cRecordSeparator.getLength() == 0)
		? _cstrDefaultRecordSeparator : m_cRecordSeparator;
}

// FUNCTION public
//	Action::BulkParameter::getElementSeparator -- get element separator
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const ModUnicodeString&
//
// EXCEPTIONS

const ModUnicodeString&
BulkParameter::
getElementSeparator() const
{
	return (m_cElementSeparator.getLength() == 0)
		? _cstrDefaultElementSeparator : m_cElementSeparator;
}

// FUNCTION public static
//	Execution::Action::BulkParameter::initializeParser -- initialize hint parser
//
// NOTES
//	This method is called once in the system initialization
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
BulkParameter::
initializeParser()
{
	if (_bInitialized == false) {
		Os::AutoCriticalSection l(_latch);
		if (_bInitialized == false) {
			_cHintSeparator.initialize();

			// create parser
			for (int i = 0; i < HintKey::ValueNum; ++i) {
				_cHintSeparator.addKeyword(ModUnicodeString(_cHintKeyTable[i]),
										   i,
										   false /* case insensitive */);
			}
			// add terminator
			_cHintSeparator.addKeyword(ModUnicodeString(" "), HintKey::Terminator);
			_cHintSeparator.addKeyword(ModUnicodeString("\\n"), HintKey::Terminator);
			_cHintSeparator.addKeyword(ModUnicodeString("\\t"), HintKey::Terminator);
	
			_cHintSeparator.prepare();

			_bInitialized = true;
		}
	}
}

// FUNCTION public
//	Execution::Action::BulkParameter::terminateParser -- terminate hint parser
//
// NOTES
//	This method is called once in the system termination
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
BulkParameter::
terminateParser()
{
	_cHintSeparator.terminate();
}

// FUNCTION public
//	Execution::Action::BulkParameter::getMaxSize -- size limit for data
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
BulkParameter::
getMaxSize() const
{
	return Opt::Configuration::getBulkMaxSize().get();
}

// FUNCTION public
//	Execution::Action::BulkParameter::getMaxLog -- maximum length of data written in message
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
BulkParameter::
getMaxLog() const
{
	return Opt::Configuration::getBulkMaxDataLengthInLog().get();
}

// FUNCTION public
//	Execution::Action::BulkParameter::getExternalThreshold -- threshold of string length for external file
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
BulkParameter::
getExternalThreshold() const
{
	return Opt::Configuration::getBulkExternalThreshold().get();
}

// FUNCTION public
//	Execution::Action::BulkParameter::getFileKeyword -- FILE keyword
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
BulkParameter::
getFileKeyword() const
{
	return _pszFile;
}

// FUNCTION public
//	Execution::Action::BulkParameter::getNullKeyword -- NULL keyword
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
BulkParameter::
getNullKeyword() const
{
	return _pszNull;
}

// FUNCTION public
//	Execution::Action::BulkParameter::getDefaultKeyword -- DEFAULT keyword
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
BulkParameter::
getDefaultKeyword() const
{
	return _pszDefault;
}

// FUNCTION public
//	Execution::Action::BulkParameter::getFileKeywordLength -- length of FILE keyword
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
BulkParameter::
getFileKeywordLength() const
{
	return _iFileKeywordLength;
}

// FUNCTION public
//	Execution::Action::BulkParameter::getNullKeywordLength -- length of NULL keyword
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
BulkParameter::
getNullKeywordLength() const
{
	return _iNullKeywordLength;
}

// FUNCTION public
//	Execution::Action::BulkParameter::getDefaultKeywordLength -- length of DEFAULT keyword
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
BulkParameter::
getDefaultKeywordLength() const
{
	return _iDefaultKeywordLength;
}

// FUNCTION public
//	Execution::Action::BulkParameter::readSpecFile -- read spec file
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrWith_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkParameter::
readSpecFile(const ModUnicodeString& cstrWith_)
{
	BulkFile cFile(cstrWith_, true /* read */);

	// create spec parameter
	BulkParameter cParameter;
	cParameter.m_cFieldSeparator = _cstrSpecFieldSeparator;
	cParameter.m_cRecordSeparator = _cstrSpecRecordSeparator;
	cParameter.m_cElementSeparator = _cstrSpecElementSeparator;
	cParameter.m_eEncoding = m_eEncoding;
	cParameter.m_bIsInput = true;

	// prepare data to get the specifications
	Common::DataArrayData cArray;
	Common::StringData cElement;
	cArray.pushBack(Common::Data::Pointer(static_cast<const Common::Data*>(&cElement)));
	VECTOR<Common::SQLData> vecType;
	vecType.PUSHBACK(Common::SQLData(Common::SQLData::Type::NChar,
									 Common::SQLData::Flag::Unlimited,
									 0,
									 0));

	// parser
	BulkParser cParser(cFile, cParameter);
	cParser.initialize(true /* ignore double quote */);

	while (cParser.getTuple(cArray, vecType)) {
		if (cElement.getValue()[0] == _cSpecComment) {
			continue;
		}
		// set parameter by an element
		parseHint(cElement.getValue());
	}

	cParser.terminate();
}

// FUNCTION public
//	Execution::Action::BulkParameter::parseHint -- set parameter from hint or spec string
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cHint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkParameter::
parseHint(const ModUnicodeString& cHint_)
{
#define SKIP_WHITESPACE(_p_) while (*(_p_) == ' ' || *(_p_) == '\n' || *(_p_) == '\t') {++(_p_);}

	const ModUnicodeChar* pTop = static_cast<const ModUnicodeChar*>(cHint_);
	const ModUnicodeChar* pTail = cHint_.getTail();

	SKIP_WHITESPACE(pTop);

	while (pTop < pTail) {
		// search for keywords
		int iKey = getHintKey(pTop, pTail);
										// pTop went to the next of the key
		const ModUnicodeChar* pDataTop = 0;
		const ModUnicodeChar* pDataTail = 0;

		if (!_bNoValueTable[iKey]) {
			SKIP_WHITESPACE(pTop);
			if (*pTop != Common::UnicodeChar::usEqual) {
				// keyword should be followed by '='
				_SYDNEY_THROW0(Exception::InvalidBulkParameter);
			}
			++pTop;
			SKIP_WHITESPACE(pTop);

			getHintValue(pDataTop, pDataTail, pTop, pTail);
										// pTop went to the next of the value
		}
		SKIP_WHITESPACE(pTop);

		setParameter(iKey, pDataTop, pDataTail);
	}
}

// FUNCTION public
//	Execution::Action::BulkParameter::getHintKey -- get hint key for parseHint
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeChar*& pTop_
//		[in] top of the string to be parsed
//		[out] tail of the hint key found
//	const ModUnicodeChar* pTail_
//		[in] tail of the string to be parsed
//	
// RETURN
//	int
//
// EXCEPTIONS

int
BulkParameter::
getHintKey(const ModUnicodeChar*& pTop_, const ModUnicodeChar* pTail_)
{
	_cHintSeparator.reset();
	const ModUnicodeChar* p = _cHintSeparator.check(pTop_, pTail_);
	if ((_cHintSeparator.getStatus() != BulkSeparator::Status::Match)
		||
		(p - _cHintSeparator.getMatchLength() != pTop_)) {
		// no keywords found or unexpected characters found before the keyword
		_SYDNEY_THROW0(Exception::InvalidBulkParameter);
	}

	int iKey = _cHintSeparator.getMatchValue();
	; _SYDNEY_ASSERT(iKey >= 0 && iKey < HintKey::ValueNum);
								// Terminator will not hit because of SKIP_WHITESPACE
	// goto next char of key
	pTop_ = p + 1;
	return iKey;
}

// FUNCTION public
//	Execution::Action::BulkParameter::getHintValue -- get hint value for parseHint
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeChar*& pDataTop_
//		[out] top of the hint value found
//	const ModUnicodeChar*& pDataTail_
//		[out] tail of the hint value found
//	const ModUnicodeChar*& pTop_
//		[in] top of the string to be parsed
//		[out] top of the next section to be parsed
//	const ModUnicodeChar* pTail_
//		[in] tail of the string to be parsed
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkParameter::
getHintValue(const ModUnicodeChar*& pDataTop_,
			 const ModUnicodeChar*& pDataTail_,
			 const ModUnicodeChar*& pTop_,
			 const ModUnicodeChar* pTail_)
{
	// search for terminator
	_cHintSeparator.reset();
	const ModUnicodeChar* pNext = _cHintSeparator.check(pTop_, pTail_, true /* ignore head wquote */);

	// data end is determined as follows;
	// 1. status is TailDoubleQuote and data head is also double quote
	//    -> pNext (the position of tail double quote)
	// 2. pNext equals to pTail
	//    -> pNext (the end of whole string) -> end parsing
	// 3. status is Match and matched value is HintKey::Terminator
	//    -> pNext - matched length (the top of matched pattern)
	// 4. otherwise, fail

	BulkSeparator::Status::Value eStatus = _cHintSeparator.getStatus();
	if (pNext == pTail_
		&&
		(*pTop_ != Common::UnicodeChar::usWquate)) {
		pDataTop_ = pTop_;
		pDataTail_ = pNext;
		pTop_ = pNext;

	} else if ((eStatus == BulkSeparator::Status::TailDoubleQuote)
			   &&
			   (*pTop_ == Common::UnicodeChar::usWquate)) {
		pDataTop_ = pTop_ + 1; // eliminate head double quote
		pDataTail_ = pNext;
		pTop_ = pNext + 1;

	} else if ((eStatus == BulkSeparator::Status::Match)
			   &&
			   (_cHintSeparator.getMatchValue() == HintKey::Terminator)
			   &&
			   (*pTop_ != Common::UnicodeChar::usWquate)) {
		pDataTop_ = pTop_;
		pDataTail_ = pNext - _cHintSeparator.getMatchLength();
		pTop_ = pNext + 1;

	} else {
		_SYDNEY_THROW0(Exception::InvalidBulkParameter);
	}
}

// FUNCTION public
//	Execution::Action::BulkParameter::setParameter -- set parameter value from a value
//
// NOTES
//
// ARGUMENTS
//	int iKey_
//	const ModUnicodeChar* pDataTop_
//	const ModUnicodeChar* pDataTail_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkParameter::
setParameter(int iKey_,
			 const ModUnicodeChar* pDataTop_,
			 const ModUnicodeChar* pDataTail_)
{
	switch (iKey_) {
	case HintKey::FieldSeparator:
		{
			if (m_cFieldSeparator.getLength() == 0) {
				m_cFieldSeparator.allocateCopy(
					pDataTop_,
					static_cast<ModSize>(pDataTail_ - pDataTop_));
			}
			break;
		}
	case HintKey::RecordSeparator:
		{
			if (m_cRecordSeparator.getLength() == 0) {
				m_cRecordSeparator.allocateCopy(
					pDataTop_,
					static_cast<ModSize>(pDataTail_ - pDataTop_));
			}
			break;
		}
	case HintKey::ElementSeparator:
		{
			if (m_cElementSeparator.getLength() == 0) {
				m_cElementSeparator.allocateCopy(
					pDataTop_,
					static_cast<ModSize>(pDataTail_ - pDataTop_));
			}
			break;
		}
	case HintKey::DateDelimiter:
		{
			if (m_cDateDelimiter == 0) {
				if (pDataTail_ - pDataTop_ != 1) {
					// date delimiter is not one character
					_SYDNEY_THROW0(Exception::InvalidBulkParameter);
				}
				if (!ModUnicodeCharTrait::isAscii(*pDataTop_)) {
					// date delimiter is not an ascii character
					_SYDNEY_THROW0(Exception::InvalidBulkParameter);
				}
				m_cDateDelimiter = static_cast<char>(*pDataTop_);
			}
			break;
		}
	case HintKey::DateTimeDelimiter:
		{
			if (m_cDateTimeDelimiter == 0) {
				if (pDataTail_ - pDataTop_ != 1) {
					// date trailer is not one character
					_SYDNEY_THROW0(Exception::InvalidBulkParameter);
				}
				if (!ModUnicodeCharTrait::isAscii(*pDataTop_)) {
					// date trailer is not an ascii character
					_SYDNEY_THROW0(Exception::InvalidBulkParameter);
				}
				m_cDateTimeDelimiter = static_cast<char>(*pDataTop_);
			}
			break;
		}
	case HintKey::TimeDelimiter:
		{
			if (m_cTimeDelimiter == 0) {
				if (pDataTail_ - pDataTop_ != 1) {
					// time delimiter is not one character
					_SYDNEY_THROW0(Exception::InvalidBulkParameter);
				}
				if (!ModUnicodeCharTrait::isAscii(*pDataTop_)) {
					// time delimiter is not an ascii character
					_SYDNEY_THROW0(Exception::InvalidBulkParameter);
				}
				m_cTimeDelimiter = static_cast<char>(*pDataTop_);
			}
			break;
		}
	case HintKey::MsecDelimiter:
		{
			if (m_cMsecDelimiter == 0) {
				if (pDataTail_ - pDataTop_ != 1) {
					// msec delimiter is not one character
					_SYDNEY_THROW0(Exception::InvalidBulkParameter);
				}
				if (!ModUnicodeCharTrait::isAscii(*pDataTop_)) {
					// msec delimiter is not an ascii character
					_SYDNEY_THROW0(Exception::InvalidBulkParameter);
				}
				m_cMsecDelimiter = static_cast<char>(*pDataTop_);
			}
			break;
		}
	case HintKey::Code:
		{
			// only "utf-8" can be specified
			if (pDataTop_ == pDataTail_) {
				m_eEncoding = ModKanjiCode::literalCode;
			} else if (_cstrUtf8.compare(
						   pDataTop_,
						   static_cast<ModSize>(pDataTail_ - pDataTop_)) == 0) {
				m_eEncoding = ModKanjiCode::utf8;
			} else {
				// unknown code specification
				_SYDNEY_THROW0(Exception::InvalidBulkParameter);
			}
			break;
		}
	case HintKey::InputField:
		{
			if (m_cInputField.getValue().ISEMPTY()) {
				m_cInputField.setValue(pDataTop_, pDataTail_);
			}
			break;
		}
	case HintKey::InputRecord:
		{
			if (m_cInputRecord.getValue().ISEMPTY()) {
				m_cInputRecord.setValue(pDataTop_, pDataTail_);
			}
			break;
		}
	case HintKey::ExternField:
		{
			if (m_cExternField.getValue().ISEMPTY()) {
				m_cExternField.setValue(pDataTop_, pDataTail_);
			}
			break;
		}
	case HintKey::CommitCount:
		{
			if (m_iCommitCount < 0) {
				m_iCommitCount = _getInteger(pDataTop_);
				if (pDataTop_ + 1 != pDataTail_) {
					// not a number found
					_SYDNEY_THROW0(Exception::InvalidBulkParameter);
				}
			}
			break;
		}
	case HintKey::ErrorData:
		{
			if (m_cErrorPath.getLength() == 0) {
				m_cErrorPath.allocateCopy(
					pDataTop_,
					static_cast<ModSize>(pDataTail_ - pDataTop_));
			}
			break;
		}
	case HintKey::NoExtern:
		{
			m_bIsNoExtern = true;
			break;
		}
	case HintKey::NoElementNull:
		{
			m_bIsNoElementNull = true;
			break;
		}
	case HintKey::NoDoubleQuote:
		{
			m_bIsNoDoubleQuote = true;
			break;
		}
	default:
		{
			// never reach
			; _SYDNEY_ASSERT(false);
			break;
		}
	}
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2006, 2007, 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
