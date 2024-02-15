// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BulkExecutor.h --
// 
// Copyright (c) 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_BULKEXECUTOR_H
#define __SYDNEY_EXECUTION_ACTION_BULKEXECUTOR_H

#include "Execution/Action/Module.h"
#include "Execution/Action/BulkParameter.h"

#include "Common/Object.h"
#include "Common/SQLData.h"

#include "ModFile.h"

class ModUnicodeString;

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

class BulkFile;
class BulkParser;
class BulkWriter;

class BulkExecutor
	: public Common::Object
{
public:
	typedef BulkExecutor This;
	typedef Common::Object Super;

	// constructors
	BulkExecutor(const BulkParameter& cParameter_,
				 const VECTOR<Common::SQLData>& vecDataType_);
	~BulkExecutor();

	// reset iteration
	void reset();
	// goto next tuple
	bool next();
	// get current tuple
	bool get(Common::DataArrayData& cTuple_);
	// put a tuple
	void put(const Common::DataArrayData& cTuple_);
	// current is valid position?
	bool isValid();

	// initialize object
	void initialize();
	// terminate object
	void terminate();

protected:
private:
	// do not copy
	BulkExecutor(const BulkExecutor& cOther_);
	BulkExecutor& operator=(const BulkExecutor& cOther_);

	// error handling
	void error();

	const BulkParameter& m_cParameter;
	const VECTOR<Common::SQLData>& m_vecDataType;
	BulkFile* m_pFile;					// data file
	BulkFile* m_pErrorFile;				// error file
	BulkFile* m_pTempFile;				// data file for output to error file
	BulkParser* m_pParser;
	BulkWriter* m_pWriter;

	BulkParameter::ElementIterator* m_pRecordSelector;
	int m_iLine;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_BULKEXECUTOR_H

//
//	Copyright (c) 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
