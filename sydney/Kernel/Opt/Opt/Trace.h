// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Trace.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_TRACE_H
#define __SYDNEY_OPT_TRACE_H

#include "Opt/Module.h"
#include "Opt/Declaration.h"

class ModMessageStream;
class ModOstream;

_SYDNEY_BEGIN

namespace Common
{
	class Data;
}

_SYDNEY_OPT_BEGIN

///////////////////////////////////////////////////////////////
// CLASS
//	Opt::Trace -- utility functions for trace
//
// NOTES
//	moved from Execution/Utility
class Trace
{
public:
	// CLASS
	//	Opt::Trace::Handler --
	//
	// NOTES
	class Handler
	{
	public:
		Handler(const Common::Data& cData_);
		~Handler() {}

		ModMessageStream& writeData(ModMessageStream& cStream_) const;
		ModOstream& writeData(ModOstream& cStream_) const;
	protected:
	private:
		const Common::Data& m_cData;
	};

	// get string representation of data for tracing
	static Handler toString(const Common::Data& cData_);
protected:
private:
	// never create instance
	Trace();
	~Trace();
};

_SYDNEY_OPT_END
_SYDNEY_END

// for message stream
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 const _SYDNEY::Opt::Trace::Handler& cHandler_);
ModOstream& operator<<(ModOstream& cStream_,
					   const _SYDNEY::Opt::Trace::Handler& cHandler_);

#endif // __SYDNEY_OPT_TRACE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
