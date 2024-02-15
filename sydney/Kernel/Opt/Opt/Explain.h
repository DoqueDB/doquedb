// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Explain.h -- definition for explain plan tree
// 
// Copyright (c) 2007, 2008, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_EXPLAIN_H
#define __SYDNEY_OPT_EXPLAIN_H

#include "Opt/Module.h"

#include "Common/Object.h"

#include "Lock/Duration.h"
#include "Lock/Mode.h"

#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Communication
{
	class Connection;
}
namespace Statement
{
	class ExplainOption;
}

_SYDNEY_OPT_BEGIN

class Explain
	: public Common::Object
{
public:
	struct Option
	{
		// ENUM
		//	Opt::Explain::Option::Value -- explain option
		// NOTES
		typedef unsigned int Value;
		enum {
			None		= 0,			// = Statement::ExplainOption::None
			Execute		= 1,			// = Statement::ExplainOption::Execute
			Explain		= 1 << 1,		// do explain
			File		= 1 << 2,		// (hint) output file information
			Cost		= 1 << 3,		// (hint) output cost estimation
			Lock		= 1 << 4,		// (hint) output lock information
			Data		= 1 << 5,		// (hint) output data information
			All			= ~static_cast<Value>(0)
		};
	};
	//get option value
	static Option::Value getOption(const Statement::ExplainOption& cStatement_);

	// constructor
	Explain(Option::Value iOption_, Communication::Connection* pConnection_)
		: m_iOption(iOption_), m_pConnection(pConnection_), m_cStream(),
		  m_iIndentLevel(0), m_iLineLength(0), m_bNewLine(true),
		  m_vecNewLineForced(), m_iNoNewLineLevel(0)
	{}
	// destructor
	~Explain() {}

	// check option
	bool isOn(Option::Value iOption_) const {return (m_iOption & iOption_) == iOption_;}

	// initialize
	void initialize();
	// terminate
	void terminate();
	// put result to client
	void flush();

	// get result as string
	ModUnicodeChar* getString();

	// stream is empty?
	bool isEmpty();

	// set indent level
	Explain& pushIndent();
	Explain& popIndent(bool bNewLine_ = false);
	Explain& forceNewLine();

	// set no new line
	Explain& pushNoNewLine();
	Explain& popNoNewLine();

	// add new line
	Explain& newLine(bool bForce_ = false);

	// add string
	Explain& put(const ModUnicodeString& cstrValue_);
	Explain& put(const char* pszValue_);
	Explain& put(double dblValue_);
	Explain& put(int iValue_);
	Explain& put(ModSize uValue_);
	Explain& put(Lock::Mode::Value eValue_);
	Explain& put(Lock::Duration::Value eValue_);

	// add char
	Explain& putChar(char cValue_);

private:
	// check line length to insert new line
	void checkLength(ModSize iLength_);

	Option::Value m_iOption;
	Communication::Connection* m_pConnection;
	ModUnicodeOstrStream m_cStream;
	int m_iIndentLevel;
	int m_iLineLength;
	bool m_bNewLine;
	ModVector<bool> m_vecNewLineForced;
	int m_iNoNewLineLevel;
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_EXPLAIN_H

//
// Copyright (c) 2007, 2008, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
