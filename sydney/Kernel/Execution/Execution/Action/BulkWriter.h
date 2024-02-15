// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/BulkWriter.h --
// 
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_BULKWRITER_H
#define __SYDNEY_EXECUTION_ACTION_BULKWRITER_H

#include "Execution/Action/Module.h"

#include "Common/Data.h"
#include "Common/BinaryData.h"
#include "Common/Object.h"
#include "Common/StringData.h"

#include "ModTypes.h"
#include "ModCharString.h"

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
//	BulkWriter -- Read/write from File
//
//	NOTES

class BulkWriter
	: public Common::Object
{
public:
	typedef BulkWriter This;
	typedef Common::Object Super;

	// constructors
	BulkWriter(BulkFile& cFile_, const BulkParameter& cParameter_);
	// destructor
	~BulkWriter();

	// write a tuple data
	void putTuple(const Common::DataArrayData& cResult_);

	// inilialize
	void initialize();
	// terminating
	void terminate();

protected:
private:
	// do not copy
	BulkWriter(const BulkWriter& cOther_);
	BulkWriter& operator=(const BulkWriter& cOther_);

	// put one data
	void putData(const Common::Data& cData_);

	// put one data to an external file
	void putDataToFile(const Common::Data& cData_, bool bArrayElement_);

	void putArrayData(const Common::Data& cData_);
	void putScalarData(const Common::Data& cData_, bool bArrayElement_);

	// write string data
	void putString(BulkFile& cFile_, const ModUnicodeString& cstrValue_, bool bEscape_ = false);

	// create external file name
	void createFileName(bool bArrayElement_);

	// convert separator string into output format
	static ModCharString getOutputSeparator(const ModUnicodeString& cstrSeparator_);

	BulkFile& m_cFile;					// File object
	const BulkParameter& m_cParameter;	// parameters

	ModUnicodeString m_cstrFileName;	// file name
	ModUnicodeString m_cstrFullPath;	// work area for full path name of external files

	BulkFile* m_pDataFile;				// File object for external data
	bool m_bWriteToFile;				// true until writing to external file

	// separators
	ModCharString m_cFieldSeparator;	// field separator
	ModCharString m_cRecordSeparator; // record separator
	ModCharString m_cElementSeparator; // element separator

	ModCharString m_cNullKeyword;
	ModCharString m_cFileKeyword;

	// counters (used to provide external file name)
	unsigned int m_uField;
	unsigned int m_uRecord;
	unsigned int m_uElement;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_BULKWRITER_H

//
//	Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
