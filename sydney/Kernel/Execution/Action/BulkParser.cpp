// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/BulkParser.cpp --
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"

#include "Execution/Action/BulkParser.h"
#include "Execution/Action/BulkFile.h"
#include "Execution/Action/BulkParameter.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/DataInstance.h"
#include "Common/DateTimeData.h"
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/LanguageData.h"
#include "Common/Message.h"
#include "Common/StringData.h"
#include "Common/Thread.h"
#include "Common/UnicodeString.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "ModCharString.h"
#include "ModCharTrait.h"
#include "ModKanjiCode.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

// FUNCTION public
//	Execution::Action::BulkParser::BulkParser -- constructors
//
// NOTES
//
// ARGUMENTS
//	BulkFile& cFile_
//	const BulkParameter& cParameter_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

BulkParser::
BulkParser(BulkFile& cFile_,
		   const BulkParameter& cParameter_)
	: Super(),
	  m_cFile(cFile_),
	  m_cParameter(cParameter_),
	  m_cSeparator(),
	  m_pTop(0),
	  m_pTail(0),
	  m_ePrevStatus(Status::None),
	  m_iRecordTop(0),
	  m_iRecordTail(0),
	  m_cWork(),
	  m_cFileName(),
	  m_cstrFullPath(),
	  m_pDataFile(0),
	  m_bReadFromFile(false),
	  m_bIgnoreWquote(false)
{}

// FUNCTION public
//	Execution::Action::BulkParser::~BulkParser -- 
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

BulkParser::
~BulkParser()
{
	try {
		terminate();
	} catch (...) {
		// ignore exceptions in destructor
		;
	}
}

// FUNCTION public
//	Execution::Action::BulkParser::getTuple -- get a tuple data
//
// NOTES
//
// ARGUMENTS
//	Common::DataArrayData& cResult_
//	const VECTOR<Common::SQLData>& vecDataType_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
getTuple(Common::DataArrayData& cResult_,
		 const VECTOR<Common::SQLData>& vecDataType_)
{
	// use field element iterator
	BulkParameter::ElementIterator f(m_cParameter.getInputField());

	// current record top is previous record tail
	m_iRecordTop = m_iRecordTail;

	bool bResult = true;
	int i = 0;
	int iElement = 0;
	VECTOR<Common::SQLData>::CONSTITERATOR iterator = vecDataType_.begin();
	const VECTOR<Common::SQLData>::CONSTITERATOR last = vecDataType_.end();
	while (iterator != last) {
		// is the element specified to be skipped?
		if (!f.isValid(++i)) {
			seekTo(Status::Field);
			continue;
		}
		Common::Data::Pointer pElement = cResult_.getElement(iElement++);
		; _SYDNEY_ASSERT(pElement.get());

		Status::Value eResult = getData(*iterator, *pElement);
		if (eResult == Status::Record) {
			bResult = (iElement == cResult_.getCount());
			break;
		}
		if (isFailed(eResult)) {
			// conversion failed
			if (eResult == Status::Failed) {
				seekTo(Status::Record);
			}
			bResult = false;
			break;
		}
		++iterator;
	}
	if (iterator == last) {
		// record separator have not met
		seekTo(Status::Record);
	}

	return bResult;
}

// FUNCTION public
//	Execution::Action::BulkParser::skipTuple -- skip tuple data
//
// NOTES
//
// ARGUMENTS
//	int iSkip_
//	bool bError_ /* = false */
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
skipTuple(int iSkip_,
		  bool bError_ /* = false */)
{
	for (int i = 0; i < iSkip_; ++i) {
		m_iRecordTop = m_iRecordTail;
		seekTo(Status::Record);
	}
	return true;
}

// FUNCTION public
//	Execution::Action::BulkParser::hasMoreData -- has more data?
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
BulkParser::
hasMoreData()
{
	return !m_cFile.isEndOfFile()
		|| (m_pTop < m_pTail);
}

// FUNCTION public
//	Execution::Action::BulkParser::initialize -- inilialize
//
// NOTES
//
// ARGUMENTS
//	bool bIgnoreWquote_ /* = false */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkParser::
initialize(bool bIgnoreWquote_ /* = false */)
{
	m_bIgnoreWquote = bIgnoreWquote_;

	m_cSeparator.initialize(bIgnoreWquote_);

	// special tokens
	if (!m_cParameter.isNoExtern()) {
		m_cSeparator.addKeyword(ModUnicodeString(m_cParameter.getFileKeyword()),
								Status::File, false /* ignore case */);
	}
	// data separators
	m_cSeparator.addKeyword(m_cParameter.getFieldSeparator(), Status::Field);
	m_cSeparator.addKeyword(m_cParameter.getRecordSeparator(), Status::Record);
	m_cSeparator.addKeyword(m_cParameter.getElementSeparator(), Status::Element);

	m_cSeparator.prepare();

	m_cFile.initialize();
}

// FUNCTION public
//	Execution::Action::BulkParser::terminate -- terminating
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
BulkParser::
terminate()
{
	m_cFile.terminate();
	m_cSeparator.terminate();
	delete m_pDataFile, m_pDataFile = 0;
}

// FUNCTION private
//	Execution::Action::BulkParser::read -- read one block
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
BulkParser::
read()
{
	m_pTop = m_cFile.read(m_pTop);
	m_pTail = m_cFile.getTail();

	if ((m_iRecordTail == 0) && (m_cParameter.getEncoding() == ModKanjiCode::utf8)) {
		// check header bytes in first read
		// [Note]
		// In Windows, notepad insert 3 bytes at the head of a file
		// when it is saved in UTF-8.
		if (isUtf8Header(m_pTop, m_pTail)) {
			moveTop(m_pTop + 3);
			m_iRecordTop = m_iRecordTail;
		}
	}
}

// FUNCTION private
//	Execution::Action::BulkParser::seekTo -- read file to get a status
//
// NOTES
//
// ARGUMENTS
//	Status::Value eStatus_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkParser::
seekTo(Status::Value eStatus_)
{
	// read from file
	read();

	while (m_pTop < m_pTail) {
		const char* pResult = m_cSeparator.check(m_pTop, m_pTail, m_cParameter.getEncoding(),
												 true /* ignore head wquote */);
		BulkSeparator::Status::Value eSeparatorStatus = m_cSeparator.getStatus();
		if (eSeparatorStatus == BulkSeparator::Status::Match) {
			Status::Value eResult = static_cast<Status::Value>(m_cSeparator.getMatchValue());
			// reset separator
			m_cSeparator.reset();
			// set top cursor to the point after the separator
			moveTop(pResult + 1);

			if ((eResult == eStatus_)
				||
				(eResult == Status::Record)) {
				m_ePrevStatus = eResult;
				break;
			}

		} else if (eSeparatorStatus == BulkSeparator::Status::TailDoubleQuote) {
			// set position to the next of tail double quote
			moveTop(pResult + 1);

		} else if (eSeparatorStatus == BulkSeparator::Status::IllegalCharacter) {

			// illegal character is found -> just skip
			moveTop(pResult + 1);

		} else {
			// set top cursor to undetermined position
			moveTop(pResult - m_cSeparator.getMatchLength());
		}

		// read from file again
		read();
	}
}

// FUNCTION private
//	Execution::Action::BulkParser::moveTop -- move top pointer to the specified position
//
// NOTES
//
// ARGUMENTS
//	const char* pNewTop_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkParser::
moveTop(const char* pNewTop_)
{
	// modify record tail position
	m_iRecordTail += (pNewTop_ - m_pTop);
	// move top pointer
	m_pTop = pNewTop_;
}

// FUNCTION private
//	Execution::Action::BulkParser::isFirstData -- first token from previous separator?
//
// NOTES
//
// ARGUMENTS
//	Status::Value eValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
isFirstData(Status::Value eValue_)
{
	static const bool _FirstTable[Status::ValueNum] =
	{
		true,			// None
		true,			// Field
		true,			// Record
		true,			// Element
		false,			// Continue
		false,			// HeadDoubleQuote
		false,			// DoubleQuote
		false,			// TailDoubleQuote
		false,			// File
		true,			// Failed
		true,			// FailedWithoutSkip
		false			// EndOfFile
	};
	return _FirstTable[eValue_];
}

// FUNCTION private
//	Execution::Action::BulkParser::isContinuousData -- data should be connected?
//
// NOTES
//
// ARGUMENTS
//	Status::Value eValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
isContinuousData(Status::Value eValue_)
{
	static const bool _ContinuousTable[Status::ValueNum] =
	{
		false,			// None
		false,			// Field
		false,			// Record
		false,			// Element
		true,			// Continue
		true,			// HeadDoubleQuote
		true,			// DoubleQuote
		true,			// TailDoubleQuote
		false,			// File
		false,			// Failed
		false,			// FailedWithoutSkip
		false			// EndOfFile
	};
	return _ContinuousTable[eValue_];
}

// FUNCTION private
//	Execution::Action::BulkParser::isEndOfData -- end of data?
//
// NOTES
//
// ARGUMENTS
//	Status::Value eValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
isEndOfData(Status::Value eValue_)
{
	static const bool _EODTable[Status::ValueNum] =
	{
		false,			// None
		true,			// Field
		true,			// Record
		false,			// Element
		false,			// Continue
		false,			// HeadDoubleQuote
		false,			// DoubleQuote
		false,			// TailDoubleQuote
		false,			// File
		true,			// Failed
		true,			// FailedWithoutSkip
		true			// EndOfFile
	};
	return _EODTable[eValue_];
}

// FUNCTION private
//	Execution::Action::BulkParser::isFailed -- error?
//
// NOTES
//
// ARGUMENTS
//	Status::Value eValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
isFailed(Status::Value eValue_)
{
	static const bool _EODTable[Status::ValueNum] =
	{
		false,			// None
		false,			// Field
		false,			// Record
		false,			// Element
		false,			// Continue
		false,			// HeadDoubleQuote
		false,			// DoubleQuote
		false,			// TailDoubleQuote
		false,			// File
		true,			// Failed
		true,			// FailedWithoutSkip
		true			// EndOfFile
	};
	return _EODTable[eValue_];
}

// FUNCTION private
//	Execution::Action::BulkParser::getData -- get one data
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	
// RETURN
//	BulkParser::Status::Value
//
// EXCEPTIONS

BulkParser::Status::Value
BulkParser::
getData(const Common::SQLData& cType_,
		Common::Data& cData_)
{
	Status::Value eResult = Status::EndOfFile; // if nothing is read, use this
	do {
		// read from file
		read();

		// if no more data, break
		if (m_pTop == m_pTail) break;

		if (cType_.isArrayType()) {
			m_ePrevStatus = eResult = getArrayData(cType_, cData_);
		} else {
			m_ePrevStatus = eResult = getScalarData(cType_, cData_);
		}

	} while (!isEndOfData(eResult));

	return eResult;
}

// FUNCTION private
//	Execution::Action::BulkParser::getDataFromFile -- get one data reading from an external file
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	
// RETURN
//	BulkParser::Status::Value
//
// EXCEPTIONS

BulkParser::Status::Value
BulkParser::
getDataFromFile(const Common::SQLData& cType_,
				Common::Data& cData_)
{
	if (m_pTop < m_pTail) {
		// FILE keyword should be followed by space or double quote
		const char* p = m_pTop;
		if (*p != ' ' && *p != '"') {
			SydInfoMessage << "Invalid usage of FILE keyword at "
						   << getCurrentPosition(&m_cFile, p)
						   << " byte in " << m_cFile.getFullPathNameW()
						   << "." << ModEndl;
			return Status::Failed;
		} 
		// skip heading white space
		if (*p == ' ') {
			do {
				++p;
			} while (p < m_pTail && *p == ' ');
			moveTop(p);
		}
		// get file name
		m_cFileName.setNull();
		read();
		if (m_pTop >= m_pTail) {
			SydInfoMessage << "No parameter for FILE at "
						   << getCurrentPosition(&m_cFile, p)
						   << " byte in " << m_cFile.getFullPathNameW()
						   << "." << ModEndl;
			return Status::FailedWithoutSkip;
		}

		Status::Value eResult = getScalarData(Common::SQLData::create(Common::DataType::String),
											  m_cFileName);

		if (m_cFileName.isNull()) {
			// empty file name
			SydInfoMessage << "No parameter for FILE at "
						   << getCurrentPosition(&m_cFile, p)
						   << " byte in " << m_cFile.getFullPathNameW()
						   << "." << ModEndl;
			return Status::FailedWithoutSkip;

		} else if (!isFailed(eResult)) {
			// Get fullpath name from the path of data file
			m_cFile.getFullPath(m_cFileName.getValue(), m_cstrFullPath);
			// read data from the file
			if (!convertDataFromFile(cType_, cData_, m_cstrFullPath)) {
				// conversion failed
				SydInfoMessage << "Convert data from file failed at "
							   << getCurrentPosition(&m_cFile, p)
							   << " byte in " << m_cFile.getFullPathNameW()
							   << "." << ModEndl;
				if (eResult == Status::Record) {
					return Status::FailedWithoutSkip; // seekTo not needed
				} else {
					return Status::Failed;
				}
			}
		}
		return eResult;
	}
	return Status::Failed;
}

// FUNCTION private
//	Execution::Action::BulkParser::getArrayData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	
// RETURN
//	BulkParser::Status::Value
//
// EXCEPTIONS

BulkParser::Status::Value
BulkParser::
getArrayData(const Common::SQLData& cType_,
			 Common::Data& cData_)
{
	; _SYDNEY_ASSERT(cType_.isArrayType());

	Common::DataArrayData& cResult = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData&, cData_);
	if ((m_ePrevStatus != Status::Element) && !isContinuousData(m_ePrevStatus)) {
		// clear the data
		cResult.clear();
	}
	// get data while return value is Element
	Common::Data::Pointer pElement;
	if (isContinuousData(m_ePrevStatus)) {
		// use last data
		; _SYDNEY_ASSERT(cResult.getCount() > 0);
		pElement = cResult.getValue().getBack();

	} else {
		// create new data
		int iMax = cType_.getMaxCardinality();
		if (iMax > 0 && cResult.getCount() >= iMax) {
			// The number of elements goes execeeds the max cardinality
			SydInfoMessage << "The number of elements exceeds the maximum cardinality at "
						   << m_iRecordTop
						   << " byte in " << m_cFile.getFullPathNameW()
						   << "." << ModEndl;
			return Status::Failed;
		}
		pElement = Common::DataInstance::create(cType_.getElementType());
		cResult.pushBack(pElement);
	}
	Status::Value eResult = getScalarData(cType_, *pElement);
	if (!isContinuousData(eResult)) {
		// last element has been converted

		// check whether the last element is only one
		bool bSingleElement = ((eResult == Status::Record || eResult == Status::Field)
							   &&
							   (cResult.getCount() == 1));

		if (pElement->isDefault()) {
			// if element is default,
			// if element is single element, then array itselt is default
			// otherwise, element is null
			if (bSingleElement) {
				cResult.clear();
				cResult.setDefault();

			} else {
				pElement->setNull();
			}
		} else if (pElement->isNull() && bSingleElement
				   && m_cParameter.isNoElementNull()) {
			// if the single element is null and NoElementNull is specified, array itself is null
			cResult.clear();
			cResult.setNull();
		}
	}
	return eResult;
}

// FUNCTION private
//	Execution::Action::BulkParser::getScalarData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	
// RETURN
//	BulkParser::Status::Value
//
// EXCEPTIONS

BulkParser::Status::Value
BulkParser::
getScalarData(const Common::SQLData& cType_,
			  Common::Data& cData_)
{
	const char* pTail = 0;
	const char* pNextTop = 0;
	Status::Value eResult = checkSeparator(cType_, pTail, pNextTop);

	if (isFailed(eResult)) {
		if (pNextTop) moveTop(pNextTop);
		return eResult;
	}
	; _SYDNEY_ASSERT(pTail);
	; _SYDNEY_ASSERT(pNextTop);

	if (eResult == Status::File) {
		// read from file
		moveTop(pNextTop);

		m_bReadFromFile = true;
		eResult = getDataFromFile(cType_, cData_);
		m_bReadFromFile = false;
		return eResult;
	}

	; _SYDNEY_ASSERT(pTail >= m_pTop);

	if (!convertData(cType_, cData_, m_pTop, pTail)) {
		// conversion failed
		return Status::Failed;
	}
	if (!isContinuousData(eResult) && !cData_.isNull() && !cData_.isDefault()) {
		// check consistency to type
		if (!checkData(cType_, cData_)) {
			// check failed
			return Status::Failed;
		}
	}

	// move pointers
	moveTop(pNextTop);

	return eResult;
}

// FUNCTION private
//	Execution::Action::BulkParser::convertData -- convert one data
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertData(const Common::SQLData& cType_,
			Common::Data& cData_,
			const char* pTop_,
			const char* pTail_)
{
	if (!isContinuousData(m_ePrevStatus)) {
		// take special patterns first
		if ((pTop_ == pTail_)
			||
			(pTail_ - pTop_ == m_cParameter.getDefaultKeywordLength()
			 && ModCharTrait::compare(pTop_, m_cParameter.getDefaultKeyword(),
									  ModFalse /* ignore case */,
									  static_cast<ModSize>(pTail_ - pTop_)) == 0)) {
			// empty token means DEFAULT
			// DEFAULT keyword also means DEFAULT
			if (m_ePrevStatus != Status::Element) {
				cData_.setDefault();
				return true;
			} else {
				// DEFAULT cannot be used as an element
				SydInfoMessage << "Data conversion failed: default specification for an element at "
							   << getCurrentPosition(&m_cFile, pTop_)
							   << " byte in " << m_cFile.getFullPathNameW()
							   << ModEndl;
				return false;
			}

		} else if (pTail_ - pTop_ == m_cParameter.getNullKeywordLength()
				   && ModCharTrait::compare(pTop_, m_cParameter.getNullKeyword(),
											ModFalse /* ignore case */,
											static_cast<ModSize>(pTail_ - pTop_)) == 0) {
			// NULL
			cData_.setNull();
			return true;
		}
	}
	if (!convertNormalData(cType_, cData_, pTop_, pTail_)) {
		ModCharString cTmp(pTop_, static_cast<ModSize>(pTail_ - pTop_));
		SydInfoMessage << "Data conversion failed: type=" << cType_
					   << " data="
					   << ModUnicodeString(cTmp.getBuffer(),
										   MIN(m_cParameter.getMaxLog(), static_cast<int>(cTmp.getLength())),
										   m_cParameter.getEncoding())
					   << " at " << getCurrentPosition(&m_cFile, pTop_)
					   << " byte in " << m_cFile.getFullPathNameW()
					   << ModEndl;
		return false;
	}
	return true;
}

// FUNCTION private
//	Execution::Action::BulkParser::convertDataFromFile -- convert data from a file
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const ModUnicodeString& cFileName_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertDataFromFile(const Common::SQLData& cType_,
					Common::Data& cData_,
					const ModUnicodeString& cFileName_)
{
	// create BulkFile for the file name
	if (!m_pDataFile) {
		m_pDataFile = new BulkFile(cFileName_, true /* read */);
	} else {
		m_pDataFile->setPath(cFileName_);
	}
	m_pDataFile->initialize();

	// open file
	if (!m_pDataFile->open(false /* not throw */)) {
		SydInfoMessage << "Invalid FILE specification (" << cFileName_ << ")." << ModEndl;
		return false;
	}
	const char* pTop = 0;
	const char* pTail = 0;
	pTop = m_pDataFile->read(pTop);
	pTail = m_pDataFile->getTail();

	if (isUtf8Header(pTop, pTail)) {
		pTop += 3;
	}

	// save Status data so that convertNormalData can use it
	Status::Value eStatusSave = m_ePrevStatus;
	m_ePrevStatus = Status::None;

	// enable ignore wquote action temporarily
	bool bWquoteSave = m_bIgnoreWquote;
	m_bIgnoreWquote = true;

	bool bResult = true;

	while (pTop < pTail) {
		if ((cData_.getType() == Common::DataType::String)
			&& (cType_.getType() != Common::SQLData::Type::Char)) {
			// search for valid character separation
			const char* p = pTop;
			while (p < pTail) {
				ModSize iCharSize = ModKanjiCode::getCharacterSize(*p, m_cParameter.getEncoding());
				if (iCharSize == 0) {
					// illegal character sequence for the encoding
					SydInfoMessage << "Illegal character sequence found at "
								   << getCurrentPosition(m_pDataFile, p)
								   << " byte in " << cFileName_
								   << "." << ModEndl;
					return false;
				}
				if (static_cast<ModSize>(pTail - p) < iCharSize) {
					break;
				}
				p += iCharSize;
			}
			pTail = p;
		}
		if (!convertNormalData(cType_, cData_, pTop, pTail)) {
			// conversion failed
			SydInfoMessage << "Data conversion failed: filename=" << cFileName_ << ModEndl;
			bResult = false;
			break;
		}
		// read again
		pTop = m_pDataFile->read(pTail);
		pTail = m_pDataFile->getTail();

		if ((cData_.getType() == Common::DataType::String)
			&& (cType_.getFlag() == Common::SQLData::Flag::Fixed
				|| cType_.getFlag() == Common::SQLData::Flag::Variable)
			&& (cType_.getLength() > 0)
			&& (cType_.getLength() <= _SYDNEY_DYNAMIC_CAST(Common::StringData&, cData_).getLength())) {
			// no more data needed
			break;
		}
		m_ePrevStatus = Status::Continue;
	}

	// close data file
	m_pDataFile->close();
	// recover Status data from saved value
	m_ePrevStatus = eStatusSave;
	m_bIgnoreWquote = bWquoteSave;

	return bResult;
}

// FUNCTION private
//	Execution::Action::BulkParser::convertNormalData -- convert usual formed data
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertNormalData(const Common::SQLData& cType_,
				  Common::Data& cData_,
				  const char* pTop_,
				  const char* pTail_)
{
	switch (cType_.getType()) {
	case Common::SQLData::Type::Char:
	case Common::SQLData::Type::NChar:
	case Common::SQLData::Type::UniqueIdentifier:
	case Common::SQLData::Type::NText:
	case Common::SQLData::Type::Fulltext:
	case Common::SQLData::Type::CLOB:
	case Common::SQLData::Type::NCLOB:
		{
			; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::String);
			return convertString(cType_, cData_, pTop_, pTail_);
		}
	case Common::SQLData::Type::Int:
		{
			; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Integer);
			return convertInteger(cType_, cData_, pTop_, pTail_);
		}
	case Common::SQLData::Type::Float:
		{
			; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Double);
			return convertFloat(cType_, cData_, pTop_, pTail_);
		}
	case Common::SQLData::Type::DateTime:
	case Common::SQLData::Type::Timestamp:
		{
			; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::DateTime);
			return convertDateTime(cType_, cData_, pTop_, pTail_);
		}
	case Common::SQLData::Type::Binary:
	case Common::SQLData::Type::Image:
	case Common::SQLData::Type::BLOB:
		{
			; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Binary);
			return convertBinary(cType_, cData_, pTop_, pTail_);
		}
	case Common::SQLData::Type::Language:
		{
			; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Language);
			return convertLanguage(cType_, cData_, pTop_, pTail_);
		}
	case Common::SQLData::Type::Decimal:
		{
			; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Decimal);
			return convertDecimal(cType_, cData_, pTop_, pTail_);
		}
	case Common::SQLData::Type::BigInt:
		{
			; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Integer64);
			return convertBigInt(cType_, cData_, pTop_, pTail_);
		}
	case Common::SQLData::Type::Date:
	case Common::SQLData::Type::Time:
	case Common::SQLData::Type::UInt:
	case Common::SQLData::Type::Word:
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	// never reach
}

// FUNCTION private
//	Execution::Action::BulkParser::convertString -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertString(const Common::SQLData& cType_,
			  Common::Data& cData_,
			  const char* pTop_,
			  const char* pTail_)
{
	Common::StringData& cResult = _SYDNEY_DYNAMIC_CAST(Common::StringData&, cData_);
	if (!isContinuousData(m_ePrevStatus) || cResult.isNull()) {
		// assign directly
		if (m_bIgnoreWquote == false) {
			// if head is double quote, eliminate that
			if (*pTop_ == '"') ++pTop_; // utf8, euc, shift_jis work
		}

		if (!setStringValue(cType_, cResult, pTop_, pTail_)) {
			// conversion failed
			return false;
		}
	} else {
		// assign to work data and connect it
		if (!setStringValue(cType_, m_cWork, pTop_, pTail_)) {
			// conversion failed
			return false;
		}
		// length limit check
		if ((cResult.getLength() + m_cWork.getLength())
			> static_cast<ModSize>(m_cParameter.getMaxSize()) / 2) {
			// size limit exceeds
			SydInfoMessage << "String size exceeds limit." << ModEndl;
			return false;
		}
		cResult.connect(&m_cWork);
	}
	return true;
}

// FUNCTION private
//	Execution::Action::BulkParser::convertInteger -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertInteger(const Common::SQLData& cType_,
			   Common::Data& cData_,
			   const char* pTop_,
			   const char* pTail_)
{
	return Common::StringData::getInteger(cData_, pTop_, pTail_);
}

// FUNCTION private
//	Execution::Action::BulkParser::convertFloat -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertFloat(const Common::SQLData& cType_,
			 Common::Data& cData_,
			 const char* pTop_,
			 const char* pTail_)
{
	return Common::StringData::getFloat(cData_, pTop_, pTail_);
}

// FUNCTION private
//	Execution::Action::BulkParser::convertBigInt -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertBigInt(const Common::SQLData& cType_,
			  Common::Data& cData_,
			  const char* pTop_,
			  const char* pTail_)
{
	return Common::StringData::getInteger(cData_, pTop_, pTail_);
}

// FUNCTION private
//	Execution::Action::BulkParser::convertBinary -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertBinary(const Common::SQLData& cType_,
			  Common::Data& cData_,
			  const char* pTop_,
			  const char* pTail_)
{
	Common::BinaryData& cResult = _SYDNEY_DYNAMIC_CAST(Common::BinaryData&, cData_);
	if (!isContinuousData(m_ePrevStatus)) {
		// set directly
		cResult.setValue(pTop_, static_cast<unsigned int>(pTail_ - pTop_));

	} else {
		// connect to existing data

		// length limit check
		if ((cResult.getSize() + (pTail_ - pTop_))
			> static_cast<ModSize>(m_cParameter.getMaxSize())) {
			// size limit exceeds
			SydInfoMessage << "Binary size exceeds limit." << ModEndl;
			return false;
		}
		cResult.connect(pTop_, static_cast<unsigned int>(pTail_ - pTop_));
	}
	return true;
}

// FUNCTION private
//	Execution::Action::BulkParser::convertDateTime -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertDateTime(const Common::SQLData& cType_,
				Common::Data& cData_,
				const char* pTop_,
				const char* pTail_)
{
	Common::DateTimeData& cResult = _SYDNEY_DYNAMIC_CAST(Common::DateTimeData&, cData_);
	cResult.setValue(pTop_, pTail_,
					 m_cParameter.getDateDelimiter(),
					 m_cParameter.getDateTimeDelimiter(),
					 m_cParameter.getTimeDelimiter(),
					 m_cParameter.getMsecDelimiter());
	return !(cResult.isNull());
}

// FUNCTION private
//	Execution::Action::BulkParser::convertLanguage -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertLanguage(const Common::SQLData& cType_,
				Common::Data& cData_,
				const char* pTop_,
				const char* pTail_)
{
	Common::LanguageData& cResult = _SYDNEY_DYNAMIC_CAST(Common::LanguageData&, cData_);
	Common::StringData cValue;
	if (convertString(cType_, cValue, pTop_, pTail_)) {
		if (cValue.getLength()) {
			try {
				cResult.setValue(cValue.getValue());
			} catch (ModException& e) {
				if (e.getErrorNumber() == ModCommonErrorBadArgument) {
					// can't convert to language
					Common::Thread::resetErrorCondition();
					return false;
				}
				// otherwise, rethrow
				throw;
			}
		}
		return true;
	}
	return false;
}

// FUNCTION private
//	Execution::Action::BulkParser::convertDecimal -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
convertDecimal(const Common::SQLData& cType_,
			   Common::Data& cData_,
			   const char* pTop_,
			   const char* pTail_)
{
	return Common::StringData::getInteger(cData_, pTop_, pTail_);
}

// FUNCTION private
//	Execution::Action::BulkParser::setStringValue -- set value of string data
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::StringData& cData_
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

bool
BulkParser::
setStringValue(const Common::SQLData& cType_,
			   Common::StringData& cData_,
			   const char* pTop_,
			   const char* pTail_)
{
	switch (cType_.getType()) {
	case Common::SQLData::Type::Char:
		{
			// Check whether illegal character is included
			for (const char* p = pTop_; p < pTail_; ++p) {
				if (!ModCharTrait::isAscii(*p)) {
					SydInfoMessage << "Illegal code for (var)char type." << ModEndl;
					return false;
				}
			}
			cData_.setValue(pTop_, static_cast<ModSize>(pTail_ - pTop_),
							ModKanjiCode::unknown);
			break;
		}
	default:
		{
			// get character count
			int iSize = 0;
			for (const char* p = pTop_; p < pTail_;) {
				++iSize;
				ModSize iCharacterSize =
					ModKanjiCode::getCharacterSize(*p, m_cParameter.getEncoding());
				if (iCharacterSize == 0) {
					// Illegal character sequence
					SydInfoMessage << "Illegal character sequence found." << ModEndl;
					return false;
				}
				p += iCharacterSize;

				// mod 1.13 surrogate pair
				if (m_cParameter.getEncoding() == ModKanjiCode::utf8 &&
					iCharacterSize == 4) {
					++iSize;
				}
			}
			cData_.setValue(pTop_, iSize, m_cParameter.getEncoding());
			break;
		}
	}
	return true;
}

// FUNCTION private
//	Execution::Action::BulkParser::checkData -- check consistency to type
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	Common::Data& cData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
checkData(const Common::SQLData& cType_,
		  Common::Data& cData_)
{
	; _SYDNEY_ASSERT(!cData_.isNull());
	; _SYDNEY_ASSERT(!cData_.isDefault());

	switch (cType_.getType()) {
	case Common::SQLData::Type::Char:
	case Common::SQLData::Type::NChar:
	case Common::SQLData::Type::UniqueIdentifier:
	case Common::SQLData::Type::NText:
	case Common::SQLData::Type::Fulltext:
	case Common::SQLData::Type::CLOB:
	case Common::SQLData::Type::NCLOB:
		{
			; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::String);
			Common::StringData& cStringData = _SYDNEY_DYNAMIC_CAST(Common::StringData&, cData_);
			int nSrc = cStringData.getLength();
			int nDst;
			if (!cType_.getCastStringLength(nDst, cType_, cData_)) {
				return false;
			}
			if (nDst < nSrc) {
				// When nDst is smaller than nSrc, truncate the data
				const ModUnicodeChar* pSrc = cStringData.getValue();
				ModUnicodeString& cValue = const_cast<ModUnicodeString&>(cStringData.getValue());
				cValue.truncate(pSrc + nDst);
			} else if (nSrc < nDst) {
				// When nDst is greater than nSrc, append space
				ModUnicodeString& cValue = const_cast<ModUnicodeString&>(cStringData.getValue());
				cValue.reallocate(nDst);
				do {
					cValue.append(Common::UnicodeChar::usSpace);
				} while (++nSrc < nDst);
			}
			break;
		}
	case Common::SQLData::Type::Int:
	case Common::SQLData::Type::Float:
	case Common::SQLData::Type::DateTime:
	case Common::SQLData::Type::Timestamp:
	case Common::SQLData::Type::Binary:
	case Common::SQLData::Type::Image:
	case Common::SQLData::Type::BLOB:
	case Common::SQLData::Type::Language:
	case Common::SQLData::Type::Decimal:
	case Common::SQLData::Type::BigInt:
	case Common::SQLData::Type::Date:
	case Common::SQLData::Type::Time:
	case Common::SQLData::Type::UInt:
	case Common::SQLData::Type::Word:
	default:
		{
			// do nothing
			break;
		}
	}
	return true;
}

// FUNCTION private
//	Execution::Action::BulkParser::isUtf8Header -- 
//
// NOTES
//
// ARGUMENTS
//	const char* pTop_
//	const char* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkParser::
isUtf8Header(const char* pTop_,
			 const char* pTail_)
{
	return ((pTail_ - pTop_ > 3)
			&& (static_cast<unsigned char>(*pTop_) == static_cast<unsigned char>(0xef))
			&& (static_cast<unsigned char>(*(pTop_+1)) == static_cast<unsigned char>(0xbb))
			&& (static_cast<unsigned char>(*(pTop_+2)) == static_cast<unsigned char>(0xbf)));
}

// FUNCTION private
//	Execution::Action::BulkParser::checkSeparator -- search for separator after a value
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	const char*& pTail_
//	const char*& pNextTop_
//	
// RETURN
//	BulkParser::Status::Value
//
// EXCEPTIONS

BulkParser::Status::Value
BulkParser::
checkSeparator(const Common::SQLData& cType_,
			   const char*& pTail_,
			   const char*& pNextTop_)
{
	Status::Value eResult = Status::None;
	const char* pTop = m_pTop;
	Status::Value ePrevStatus = m_ePrevStatus;

	while (true) {
		const char* pResult = m_cSeparator.check(pTop, m_pTail, m_cParameter.getEncoding());
		BulkSeparator::Status::Value eSeparatorStatus = m_cSeparator.getStatus();

		// convert to data using the part
		// getMatchLength returns following size
		// 1. matched ... matched separator length - 1
		// 2. on matching ... matching substring length - 1
		// 3. otherwise ... zero
		//
		// pResult points to following position
		// 1. matched ... last character of the separator
		// 2. left/right double quote ... the double quote
		// 3. otherwise ... pTail

		pTail_ = pResult - m_cSeparator.getMatchLength();

		if (eSeparatorStatus == BulkSeparator::Status::Match) {
			eResult = static_cast<Status::Value>(m_cSeparator.getMatchValue());
			// reset separator
			m_cSeparator.reset();

			// check the data type
			if (eResult == Status::Element && !m_bReadFromFile && !cType_.isArrayType()) {
				// error -- element separator for non-array data
				SydInfoMessage << "Element separator is used in non-array data at "
							   << getCurrentPosition(&m_cFile, pResult)
							   << " byte in " << m_cFile.getFullPathNameW()
							   << "." << ModEndl;
				return Status::Failed;
			}

			if (eResult == Status::File) {
				// If FILE keyword is found but heading data already exists, treat as normal data
				if ((pTail_ != pTop) || !isFirstData(ePrevStatus) || m_bReadFromFile) {
					// set pTail to the next position of the pattern
					pTail_ = pResult + 1;
					eResult = Status::Continue;
				}
			}

			// next top should be next char of the pattern
			pNextTop_ = pResult + 1;

		} else if (eSeparatorStatus == BulkSeparator::Status::HeadDoubleQuote) {
			if (pResult != pTop) {
				// head double quote should be started at the top of region
				SydInfoMessage << "Invalid value before double quote at "
							   << getCurrentPosition(&m_cFile, pResult)
							   << " byte in " << m_cFile.getFullPathNameW()
							   << "." << ModEndl;
				// anyway, next seek have to be started from next char
				pNextTop_ = pResult + 1;
				return Status::Failed;
			}
			eResult = Status::HeadDoubleQuote;
			pNextTop_ = pResult + 1;
		} else if (eSeparatorStatus == BulkSeparator::Status::DoubleQuote) {
			eResult = Status::DoubleQuote;
			pNextTop_ = pTail_;
		} else if (eSeparatorStatus == BulkSeparator::Status::TailDoubleQuote) {
			eResult = Status::TailDoubleQuote;
			pNextTop_ = pResult + 1;
		} else if (eSeparatorStatus == BulkSeparator::Status::IllegalCharacter) {
			// illegal caharcter is found
			SydInfoMessage << "Illegal character is found at "
						   << getCurrentPosition(&m_cFile, pResult)
						   << " byte in " << m_cFile.getFullPathNameW() << ModEndl;
			// anyway, next seek have to be started from next char
			pNextTop_ = pResult + 1;
			return Status::Failed;

		} else {
			eResult = Status::Continue;
			if (eSeparatorStatus == BulkSeparator::Status::Continue
				&& !m_cFile.isEndOfFile()) {
				// middle of pattern
				--pTail_;				// move back one more here
			}
			pNextTop_ = pTail_;

			// If data is considered as continue, but end of file is reached, use whole
			if (m_cFile.isEndOfFile()) {
				pTail_ = pResult;
			}
			// reset separator here so that next loop can check the separator from the top
			m_cSeparator.reset();
		}

		if (ePrevStatus == Status::TailDoubleQuote) {
			// tail double quote should be followed by one of ;
			// 1. double quote (head)
			// 2. separator
			if ((eSeparatorStatus != BulkSeparator::Status::Match
				 && eSeparatorStatus != BulkSeparator::Status::HeadDoubleQuote)
				|| pTail_ != pTop /* not empty before the separator */) {
				SydInfoMessage << "Invalid value after double quote at "
							   << getCurrentPosition(&m_cFile, pResult)
							   << " byte in " << m_cFile.getFullPathNameW()
							   << "." << ModEndl;
				return Status::Failed;
			}
		}

		if (eResult == Status::HeadDoubleQuote) {
			// check separator again from next char
			ePrevStatus = eResult;
			pTop = pNextTop_;
		} else {
			// go out of the loop
			break;
		}
	}
	return eResult;
}

// FUNCTION private
//	Execution::Action::BulkParser::getCurrentPosition -- current position
//
// NOTES
//
// ARGUMENTS
//	BulkFile* pFile_,
//	const char* p_
//	
// RETURN
//	ModFileOffset
//
// EXCEPTIONS

ModFileOffset
BulkParser::
getCurrentPosition(BulkFile* pFile_,
				   const char* p_)
{
	return pFile_->getTopPosition() + (p_ - pFile_->getTop());
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
