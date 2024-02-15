// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/BulkWriter.cpp --
// 
// Copyright (c) 2007, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include <stdio.h>					// sprintf()

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Execution/Action/BulkWriter.h"
#include "Execution/Action/BulkFile.h"
#include "Execution/Action/BulkParameter.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/DataInstance.h"
#include "Common/DateTimeData.h"
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "ModCharString.h"
#include "ModCharTrait.h"
#include "ModKanjiCode.h"
#include "ModUnicodeCharTrait.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

namespace
{
	// FUNCTION local
	//	$$$::_addDirectory -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	unsigned int uValue_
	//	ModUnicodeString& cTarget_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_addDirectory(unsigned int uValue_,
				  ModUnicodeString& cTarget_,
				  bool bNeverExceeds_ = false)
	{
		const unsigned int uMaxFilesInDirectory = 1 << 12; // 4096
		char pszBuf[10];

		if (bNeverExceeds_) {
			if (uValue_ >= uMaxFilesInDirectory) {
				_SYDNEY_THROW0(Exception::NotSupported);
			}

			::sprintf(pszBuf, "%03x", uValue_);
			cTarget_.append(ModUnicodeString(pszBuf));

		} else {
			unsigned int uValue0 = uValue_ % uMaxFilesInDirectory;
			uValue_ /= uMaxFilesInDirectory;
			unsigned int uValue1 = uValue_ % uMaxFilesInDirectory;
			uValue_ /= uMaxFilesInDirectory;
			unsigned int uValue2 = uValue_;
			;_SYDNEY_ASSERT(uValue_ < uMaxFilesInDirectory);

			::sprintf(pszBuf, "%03x", uValue2);
			cTarget_.append(ModUnicodeString(pszBuf));
			::sprintf(pszBuf, "/%03x", uValue1);
			cTarget_.append(ModUnicodeString(pszBuf));
			::sprintf(pszBuf, "/%03x", uValue0);
			cTarget_.append(ModUnicodeString(pszBuf));
		}
	}
}

// FUNCTION public
//	Execution::Action::BulkWriter::BulkWriter -- constructors
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

BulkWriter::
BulkWriter(BulkFile& cFile_, const BulkParameter& cParameter_)
	: Super(),
	  m_cFile(cFile_),
	  m_cParameter(cParameter_),
	  m_cstrFileName(),
	  m_cstrFullPath(),
	  m_pDataFile(0),
	  m_bWriteToFile(false),
	  m_cFieldSeparator(),
	  m_cRecordSeparator(),
	  m_cElementSeparator(),
	  m_cNullKeyword(),
	  m_cFileKeyword(),
	  m_uField(0),
	  m_uRecord(0),
	  m_uElement(0)

{}

// FUNCTION public
//	Execution::Action::BulkWriter::~BulkWriter -- 
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

BulkWriter::
~BulkWriter()
{
	try {
		terminate();
	} catch (...) {
		// ignore exceptions in destructor
		;
	}
}

// FUNCTION public
//	Execution::Action::BulkWriter::putTuple -- put tuple data (when it is known there are no array)
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkWriter::
putTuple(const Common::DataArrayData& cResult_)
{
	unsigned int n = cResult_.getCount();

	for (m_uField = 0; m_uField < n; ++m_uField) {
		Common::Data::Pointer pElement = cResult_.getElement(m_uField);
		; _SYDNEY_ASSERT(pElement.get());

		if (m_uField > 0) {
			m_cFile.write(m_cFieldSeparator);
		}
		putData(*pElement);
	}
	m_cFile.write(m_cRecordSeparator);
	++m_uRecord;
}

// FUNCTION public
//	Execution::Action::BulkWriter::initialize -- inilialize
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
BulkWriter::
initialize()
{
	// prepare separators for output
	m_cFieldSeparator = getOutputSeparator(m_cParameter.getFieldSeparator());
	m_cRecordSeparator = getOutputSeparator(m_cParameter.getRecordSeparator());
	m_cElementSeparator = getOutputSeparator(m_cParameter.getElementSeparator());

	m_cNullKeyword = m_cParameter.getNullKeyword();
	m_cFileKeyword = m_cParameter.getFileKeyword();
	m_cFileKeyword.append(' ');

	m_cFile.initialize();
}

// FUNCTION public
//	Execution::Action::BulkWriter::terminate -- terminating
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
BulkWriter::
terminate()
{
	if (m_cFile.isOpened()) {
		m_cFile.flush(); // flush buffer
		m_cFile.close();
	}
	m_cFile.terminate();
	delete m_pDataFile, m_pDataFile = 0;
	m_uRecord = 0;
}

// FUNCTION private
//	Execution::Action::BulkWriter::putData -- put one data
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkWriter::
putData(const Common::Data& cData_)
{
	if (cData_.getType() == Common::DataType::Array) {
		putArrayData(cData_);
	} else {
		putScalarData(cData_, false /* not array element */);
	}
}

// FUNCTION private
//	Execution::Action::BulkWriter::putDataToFile -- put one data to an external file
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	bool bArrayElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkWriter::
putDataToFile(const Common::Data& cData_, bool bArrayElement_)
{
	createFileName(bArrayElement_);

	// Get fullpath name from the path of data file
	m_cFile.getFullPath(m_cstrFileName, m_cstrFullPath);

	// create BulkFile for the file name
	if (!m_pDataFile) {
		m_pDataFile = new BulkFile(m_cstrFullPath, false /* write */);
	} else {
		m_pDataFile->setPath(m_cstrFullPath);
	}
	m_pDataFile->initialize();

	switch (cData_.getType()) {
	case Common::DataType::String:
		{
			const ModUnicodeString& cstrValue =
				_SYDNEY_DYNAMIC_CAST(const Common::StringData&, cData_).getValue();

			putString(*m_pDataFile, cstrValue, false /* no need to escape */);
			break;
		}
	case Common::DataType::Binary:
		{
			const Common::BinaryData& cBinaryData =
				_SYDNEY_DYNAMIC_CAST(const Common::BinaryData&, cData_);
			if (cBinaryData.getSize() > m_cParameter.getMaxSize()) {
				// size limit exceeds
				SydInfoMessage << "Binary size exceeds limit." << ModEndl;
				_SYDNEY_THROW0(Exception::NotSupported);
			}
			const char* pData = syd_reinterpret_cast<const char*>(cBinaryData.getValue());
			m_pDataFile->write(pData, cBinaryData.getSize());
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	m_pDataFile->flush(); // flush the buffer
	m_pDataFile->close();
}

// FUNCTION private
//	Execution::Action::BulkWriter::putArrayData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkWriter::
putArrayData(const Common::Data& cData_)
{
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Array);

	const Common::DataArrayData& cResult = _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, cData_);

	int n = cResult.isNull() ? 0 : cResult.getCount();

	for (m_uElement = 0; m_uElement < n; ++m_uElement) {
		Common::Data::Pointer pElement = cResult.getElement(m_uElement);
		; _SYDNEY_ASSERT(pElement.get());

		if (m_uElement > 0) {
			m_cFile.write(m_cElementSeparator);
		}
		putScalarData(*pElement, true /* array element */);
	}
}

// FUNCTION private
//	Execution::Action::BulkWriter::putScalarData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	bool bArrayElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkWriter::
putScalarData(const Common::Data& cData_, bool bArrayElement_)
{
	if (cData_.isNull()) {
		m_cFile.write(m_cNullKeyword);
		return;
	}

	switch (cData_.getType()) {
	case Common::DataType::String:
		{
			const ModUnicodeString& cstrValue =
				_SYDNEY_DYNAMIC_CAST(const Common::StringData&, cData_).getValue();

			// If string length is greater than threshold, use external file
			if (cstrValue.getLength() > m_cParameter.getExternalThreshold()) {
				putDataToFile(cData_, bArrayElement_);
				m_cFile.write(m_cFileKeyword);
				putString(m_cFile, m_cstrFileName);
			} else {
				putString(m_cFile, cstrValue, true /* need escape double quotes */);
			}
			break;
		}
	case Common::DataType::Binary:
		{
			putDataToFile(cData_, bArrayElement_);
			m_cFile.write(m_cFileKeyword);
			putString(m_cFile, m_cstrFileName);
			break;
		}
	case Common::DataType::Integer:
	case Common::DataType::UnsignedInteger:
	case Common::DataType::Double:
	case Common::DataType::DateTime:
	case Common::DataType::Language:
	case Common::DataType::Decimal:
	case Common::DataType::Integer64:
	case Common::DataType::UnsignedInteger64:
		{
			putString(m_cFile, cData_.getString());
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

// FUNCTION private
//	Execution::Action::BulkWriter::putString -- write string data
//
// NOTES
//
// ARGUMENTS
//	BulkFile& cFile_
//	const ModUnicodeString& cstrValue_
//	bool bEscape_ /* = false */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkWriter::
putString(BulkFile& cFile_, const ModUnicodeString& cstrValue_, bool bEscape_ /* = false */)
{
	if (cstrValue_.getLength() > m_cParameter.getMaxSize() / 2) {
		// size limit exceeds
		SydInfoMessage << "String size exceeds limit." << ModEndl;
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	const char* pData = 0;
	int iSize = 0;
	ModUnicodeString cstrValue;

	if (bEscape_) {
		const ModUnicodeChar* pTop = static_cast<const ModUnicodeChar*>(cstrValue_);
		const ModUnicodeChar* pTail = cstrValue_.getTail();
		if (pTop == pTail) {
			// Empty string have to be represented by ""
			cstrValue.reallocate(2);
			cstrValue.append(Common::UnicodeChar::usWquate);
			cstrValue.append(Common::UnicodeChar::usWquate);

		} else {
			const ModUnicodeChar* pWQuote = ModUnicodeCharTrait::find(pTop,
																	  Common::UnicodeChar::usWquate,
																	  static_cast<ModSize>(pTail - pTop));
			if (pWQuote) {
				cstrValue.reallocate(cstrValue_.getLength() + 3);
				cstrValue.append(Common::UnicodeChar::usWquate);
				do {
					cstrValue.append(pTop, static_cast<ModSize>(pWQuote - pTop + 1));
					// escape double-quote by doubling that
					cstrValue.append(Common::UnicodeChar::usWquate);
					pTop = pWQuote + 1;
					pWQuote = (pTop >= pTail) ? 0
						: ModUnicodeCharTrait::find(pTop,
													Common::UnicodeChar::usWquate,
													static_cast<ModSize>(pTail - pTop));
				} while (pWQuote);

				if (pTop < pTail) {
					cstrValue.append(pTop, static_cast<ModSize>(pTail - pTop));
				}
				cstrValue.append(Common::UnicodeChar::usWquate);

			} else {
				// Just enclosed by two double quotes
				cstrValue.reallocate(cstrValue_.getLength() + 2);
				cstrValue.append(Common::UnicodeChar::usWquate);
				cstrValue.append(pTop, static_cast<ModSize>(pTail - pTop));
				cstrValue.append(Common::UnicodeChar::usWquate);
			}
		}
	} else {
		// no need to escape double quotes
		cstrValue = cstrValue_;
	}

	pData = cstrValue.getString(m_cParameter.getEncoding());
	iSize = ModCharTrait::length(pData);
	cFile_.write(pData, iSize);
}

// FUNCTION private
//	Execution::Action::BulkWriter::createFileName -- create external file name
//
// NOTES
//
// ARGUMENTS
//	bool bArrayElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkWriter::
createFileName(bool bArrayElement_)
{
	m_cstrFileName.clear();
	m_cstrFileName.reallocate(10 + 12 * 3 + 4);

	// create record part
	m_cstrFileName.append("external/r");
	_addDirectory(m_uRecord, m_cstrFileName);

	// create field part
	m_cstrFileName.append("/f");
	_addDirectory(m_uField, m_cstrFileName, true/* never exceeds limit */);

	if (bArrayElement_) {
		// create element part
		m_cstrFileName.append("/e");
		_addDirectory(m_uElement, m_cstrFileName);
	}
	// add suffix
	m_cstrFileName.append(ModUnicodeString(".dat"));
}

// FUNCTION private
//	Execution::Action::BulkWriter::getOutputSeparator -- convert separator string into output format
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrSeparator_
//	
// RETURN
//	ModCharString
//
// EXCEPTIONS

//static
ModCharString
BulkWriter::
getOutputSeparator(const ModUnicodeString& cstrSeparator_)
{
	ModCharString cstrResult;
	cstrResult.reallocate(cstrSeparator_.getLength() + 1);

	const ModUnicodeChar* p = cstrSeparator_;
	const ModUnicodeChar* pTail = cstrSeparator_.getTail();
	for (; p < pTail; ++p) {
		if (!ModUnicodeCharTrait::isAscii(*p)) {
			// Separator must be ascii characters
			SydInfoMessage
				<< "Bulk output: separator should consist of ascii characters."
				<< ModEndl;
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		if (*p != Common::UnicodeChar::usBackSlash) {
			cstrResult.append(static_cast<const char>(*p));
		} else {
			if (++p < pTail) {
				switch (*p) {
				case Common::UnicodeChar::usSmallT:
					{
						cstrResult.append("\t");
						continue;
					}
				case Common::UnicodeChar::usSmallN:
					{
						cstrResult.append("\n");
						continue;
					}
				default:
					{
						break;
					}
				}
			}
			// invalid backslack sequence
			SydInfoMessage
				<< "Bulk output: separator should not contain backslash except for '\\n' or '\\t'"
				<< ModEndl;
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	return cstrResult;
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2007, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
