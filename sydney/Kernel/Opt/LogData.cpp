// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/LogData.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Opt";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Opt/LogData.h"
#include "Opt/Environment.h"
#include "Opt/Trace.h"

#include "Analysis/Operation/Recovery.h"

#include "Execution/Externalizable.h"

#ifdef USE_OLDER_VERSION
#include "Plan/LogData.h" // for backward compatibility
#endif

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

// FUNCTION public
//	Opt::LogData::LogData -- 
//
// NOTES
//
// ARGUMENTS
//	const Trans::Log::Data::Category::Value eCategory_
//	const Common::DataArrayData& cData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

LogData::
LogData(const Trans::Log::Data::Category::Value eCategory_,
		const Common::DataArrayData& cData_)
	: Super(eCategory_), m_cData()
{
	// copy pointer without copying value
	// NOTE: here copy constructor cannot be used
	//       because it copies values instead of just copying pointers
	m_cData = cData_;
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Opt::LogData::getAnalyzer -- get analyzer
//
// NOTES
//	for old optimizer
//
// ARGUMENTS
//	bool bIsRollback_
//	
// RETURN
//	Analysis::Analyzer*
//
// EXCEPTIONS

Analysis::Analyzer*
LogData::
getAnalyzer(bool bIsRollback_) const
{
	return Plan::LogData::getAnalyzer(m_cData.getElement(Format::LogType)->getInt(),
									  m_cData,
									  bIsRollback_);
}
#endif

// FUNCTION public
//	Opt::LogData::getAnalyzer2 -- get analyzer
//
// NOTES
//
// ARGUMENTS
//	bool bIsRollback_
//	
// RETURN
//	const Analysis::Operation::Recovery*
//
// EXCEPTIONS

const Analysis::Operation::Recovery*
LogData::
getAnalyzer2(bool bIsRollback_) const
{
	return Analysis::Operation::Recovery::create(m_cData.getElement(Format::LogType)->getInt(),
												 bIsRollback_);
}

// FUNCTION public
//	Opt::LogData::getClassID -- 
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
LogData::
getClassID() const
{
	return Execution::Externalizable::Category::LogData +
		Common::Externalizable::PlanClasses;
										// for backward compatibility
}

// FUNCTION public
//	Opt::LogData::serialize -- 
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
LogData::
serialize(ModArchive& archiver_)
{
	Super::serialize(archiver_);
	m_cData.serialize(archiver_);
}

// FUNCTION public
//	Opt::LogData::toString -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
LogData::
toString() const
{
	OSTRSTREAM stream;
	stream << Super::toString() << ":";
	stream << Opt::Trace::toString(getData());
	return stream.getString();
}

_SYDNEY_OPT_END
_SYDNEY_END

// FUNCTION public
//	operator<< -- for message stream
//
// NOTES
//
// ARGUMENTS
//	ModMessageStream& cStream_
//	const _SYDNEY::Opt::LogData& cLogData_
//	
// RETURN
//	ModMessageStream&
//
// EXCEPTIONS

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 const _SYDNEY::Opt::LogData& cLogData_)
{
	return cStream_ << _SYDNEY::Opt::Trace::toString(cLogData_.getData());
}

// FUNCTION public
//	operator<< -- 
//
// NOTES
//
// ARGUMENTS
//	ModOstream& cStream_
//	const _SYDNEY::Opt::LogData& cLogData_
//	
// RETURN
//	ModOstream&
//
// EXCEPTIONS

ModOstream& operator<<(ModOstream& cStream_,
					   const _SYDNEY::Opt::LogData& cLogData_)
{
	return cStream_ << _SYDNEY::Opt::Trace::toString(cLogData_.getData());
}

//
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
