// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/LogData.h --
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_LOGDATA_H
#define __SYDNEY_OPT_LOGDATA_H

#include "Opt/Module.h"
#include "Opt/Declaration.h"
#include "Opt/Environment.h"

#include "Analysis/Declaration.h"

#include "Common/DataArrayData.h"

#include "Trans/LogData.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

////////////////////////////////////
//	CLASS
//	Opt::LogData -- holding logical logData
//
//	NOTES
class LogData
	: public Trans::Log::ModificationData
{
public:
	typedef LogData This;
	typedef Trans::Log::ModificationData Super;

	// ENUM
	//	Opt::LogData::Type::Value -- logdata type
	//
	// NOTES

	struct Type
	{
		enum Value
		{
			Undefined	= 0,

			Insert		= 1,
			Delete		= 2,
			Update		= 3,

			Delete_Undo	= 12,			// Delete (undo included)
			Update_Undo	= 13,			// Update (undo included)

			ValueNum
		};
	};

	// ENUM
	//	Opt::LogData::Type::Value -- logdata format
	//
	// NOTES

	struct Format
	{
		enum Value
		{
			LogType = 0,
			TableID,
			RowID,
			ColumnID,
			Data1,
			Data2,
			SingleValueNum = Data2,
			ValueNum,
			DoubleValueNum = ValueNum
		};
	};

	// constructor
	LogData() : Super(), m_cData() {}
	LogData(const Trans::Log::Data::Category::Value eCategory_,
			const Common::DataArrayData& cData_);

	// destructor
	~LogData() {} 

	// accessor
	const Common::DataArrayData& getData() const {return m_cData;}

	// get analyzer
#ifdef USE_OLDER_VERSION
	Analysis::Analyzer* getAnalyzer(bool bIsRollback_) const;
#endif
	const Analysis::Operation::Recovery* getAnalyzer2(bool bIsRollback_) const;

/////////////////////////////////////
// Common::Externalizable::
	int getClassID() const;

////////////////////////////
// ModSerializer::
	void serialize(ModArchive& archiver_);

//////////////////////////
// Trans::LogData::
	virtual ModUnicodeString toString() const;

protected:
private:
	Common::DataArrayData m_cData;
};

_SYDNEY_OPT_END
_SYDNEY_END

// for message stream
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 const _SYDNEY::Opt::LogData& cLogData_);
ModOstream& operator<<(ModOstream& cStream_,
					   const _SYDNEY::Opt::LogData& cLogData_);

#endif // __SYDNEY_OPT_LOGDATA_H

//
//	Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
