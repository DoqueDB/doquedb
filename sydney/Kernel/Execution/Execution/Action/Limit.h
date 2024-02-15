// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/Limit.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_LIMIT_H
#define __SYDNEY_EXECUTION_ACTION_LIMIT_H

#include "Execution/Action/Module.h"
#include "Execution/Declaration.h"
#include "Execution/Action/DataHolder.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::Limit -- wrapping class for limit specification
//
//	NOTES
class Limit
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef Limit This;

	Limit()
		: Super(),
		  m_cLimit(),
		  m_cOffset(),
		  m_iLimit(-1),
		  m_iOffset(-1)
	{}
	Limit(int iLimitID_,
		  int iOffsetID_,
		  bool bIntermediate_ = false)
		: Super(),
		  m_cLimit(iLimitID_),
		  m_cOffset(iOffsetID_),
		  m_bIntermediate(bIntermediate_),
		  m_iLimit(-1),
		  m_iOffset(-1)
	{}
	Limit(const PAIR<int, int>& cLimitPair_,
		  bool bIntermediate_ = false)
		: Super(),
		  m_cLimit(cLimitPair_.first),
		  m_cOffset(cLimitPair_.second),
		  m_bIntermediate(bIntermediate_),
		  m_iLimit(-1),
		  m_iOffset(-1)
	{}
	~Limit() {}

	// explain
	void explain(Opt::Environment* pEnvironment_,
				 Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);
	// initialize necessary members
	void initialize(Interface::IProgram& cProgram_);
	// end using members
	void terminate(Interface::IProgram& cProgram_);

	// calculate limit value
	void setValues(Interface::IProgram& cProgram_);

	// accessor
	int getLimit() {return m_iLimit;}
	int getOffset() {return m_iOffset;}

	// check validity
	bool isValid() {return m_cLimit.isValid();}
	// check initialized
	bool isInitialized() {return m_cLimit.isInitialized();}

	// serialize this class
	void serialize(ModArchive& archiver_);

protected:
private:
	// data holding limit and offset value
	Action::DataHolder m_cLimit;
	Action::DataHolder m_cOffset;

	// is intermediate plan?
	bool m_bIntermediate;

	// calculated limit and offset value
	int m_iLimit; // note: limit here is limit + offset
	int m_iOffset;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_LIMIT_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
