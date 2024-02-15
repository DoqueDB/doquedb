// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/BulkParser.h --
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

#ifndef __SYDNEY_EXECUTION_ACTION_BULKPARSER_H
#define __SYDNEY_EXECUTION_ACTION_BULKPARSER_H

#include "Execution/Action/Module.h"
#include "Execution/Action/BulkSeparator.h"

#include "Common/Data.h"
#include "Common/BinaryData.h"
#include "Common/Object.h"
#include "Common/SQLData.h"
#include "Common/StringData.h"

#include "Opt/Algorithm.h"

#include "ModKanjiCode.h"
#include "ModTypes.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN

namespace Common
{
	class Data;
	class DataArrayData;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

class BulkFile;
class BulkParameter;

//	CLASS
//	BulkParser -- Read/write from File
//
//	NOTES

class BulkParser
	: public Common::Object
{
public:
	typedef BulkParser This;
	typedef Common::Object Super;

	// constructors
	BulkParser(BulkFile& cFile_, const BulkParameter& cParameter_);
	// destructor
	~BulkParser();

	// get a tuple data
	bool getTuple(Common::DataArrayData& cResult_,
				  const VECTOR<Common::SQLData>& vecDataType_);
	// skip tuple data
	bool skipTuple(int iSkip_, bool bError_ = false);

	// has more data?
	bool hasMoreData();

	// inilialize
	void initialize(bool bIgnoreWquote_ = false);
	// terminating
	void terminate();

	// get current record position
	ModFileOffset getRecordTop() {return m_iRecordTop;}
	ModFileOffset getRecordTail() {return m_iRecordTail;}

protected:
private:
	// do not copy
	BulkParser(const BulkParser& cOther_);
	BulkParser& operator=(const BulkParser& cOther_);

	// status in parsing
	struct Status
	{
		enum Value
		{
			None = 0,					// initial status
			Field,						// field separator found
			Record,						// record separator found
			Element,					// element separator found
			Continue,					// further data needed
			HeadDoubleQuote,			// left double quote found
			DoubleQuote,				// between double quote
			TailDoubleQuote,			// right double quote found
			File,						// 'FILE' keyword
			Failed,						// failed
			FailedWithoutSkip,			// failed (skip not needed)
			EndOfFile,					// all have been read
			ValueNum
		};
	};

	// read one block
	void read();

	// read file to get a status
	void seekTo(Status::Value eStatus_);

	// move top pointer to the specified position
	void moveTop(const char* pNewTop_);

	// first token from previous separator?
	bool isFirstData(Status::Value eValue_);

	// data should be connected?
	bool isContinuousData(Status::Value eValue_);

	// end of data?
	bool isEndOfData(Status::Value eValue_);

	// error?
	bool isFailed(Status::Value eValue_);

	// get one data
	Status::Value getData(const Common::SQLData& cType_,
						  Common::Data& cData_);

	// get one data reading from an external file
	Status::Value getDataFromFile(const Common::SQLData& cType_,
								  Common::Data& cData_);

	Status::Value getArrayData(const Common::SQLData& cType_,
							   Common::Data& cData_);
	Status::Value getScalarData(const Common::SQLData& cType_,
								Common::Data& cData_);

	// convert data from buffer
	bool convertData(const Common::SQLData& cType_,
					 Common::Data& cData_,
					 const char* pTop_,
					 const char* pTail_);
	// convert data from a file
	bool convertDataFromFile(const Common::SQLData& cType_,
							 Common::Data& cData_,
							 const ModUnicodeString& cFileName_);

	// convert usual formed data
	bool convertNormalData(const Common::SQLData& cType_,
						   Common::Data& cData_,
						   const char* pTop_,
						   const char* pTail_);

	bool convertString(const Common::SQLData& cType_,
					   Common::Data& cData_,
					   const char* pTop_,
					   const char* pTail_);
	bool convertInteger(const Common::SQLData& cType_,
						Common::Data& cData_,
						const char* pTop_,
						const char* pTail_);
	bool convertFloat(const Common::SQLData& cType_,
					  Common::Data& cData_,
					  const char* pTop_,
					  const char* pTail_);
	bool convertBigInt(const Common::SQLData& cType_,
					   Common::Data& cData_,
					   const char* pTop_,
					   const char* pTail_);
	bool convertBinary(const Common::SQLData& cType_,
					   Common::Data& cData_,
					   const char* pTop_,
					   const char* pTail_);
	bool convertDateTime(const Common::SQLData& cType_,
						 Common::Data& cData_,
						 const char* pTop_,
						 const char* pTail_);
	bool convertLanguage(const Common::SQLData& cType_,
						 Common::Data& cData_,
						 const char* pTop_,
						 const char* pTail_);
	bool convertDecimal(const Common::SQLData& cType_,
						Common::Data& cData_,
						const char* pTop_,
						const char* pTail_);

	// set value of string data
	bool setStringValue(const Common::SQLData& cType_,
						Common::StringData& cData_,
						const char* pTop_,
						const char* pTail_);
	// check consistency to type
	bool checkData(const Common::SQLData& cType_,
				   Common::Data& cData_);
 
	// check utf8 file header
	bool isUtf8Header(const char* pTop_,
					  const char* pTail_);

	// search for separator after a value
	Status::Value checkSeparator(const Common::SQLData& cType_,
								 const char*& pTail_,
								 const char*& pNextTop_);

	// current position
	ModFileOffset getCurrentPosition(BulkFile* pFile_,
									 const char* p_);

	BulkFile& m_cFile;					// File object
	const BulkParameter& m_cParameter;	// parameters

	BulkSeparator m_cSeparator;
	const char* m_pTop;
	const char* m_pTail;
	Status::Value m_ePrevStatus;

	ModFileOffset m_iRecordTop;
	ModFileOffset m_iRecordTail;

	Common::StringData m_cWork; 		// work area
	Common::StringData m_cFileName;		// file name
	ModUnicodeString m_cstrFullPath;	// work area for full path name of external files

	BulkFile* m_pDataFile;				// File object for 'FILE' keyword
	bool m_bReadFromFile;				// true until reading according ot 'FILE' keyword
	bool m_bIgnoreWquote;				// true if doublequote is treated as data
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_BULKPARSER_H

//
//	Copyright (c) 2006, 2007, 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
