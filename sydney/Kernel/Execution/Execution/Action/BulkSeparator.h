// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/BulkSeparator.h --
// 
// Copyright (c) 2006, 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_BULKSEPARATOR_H
#define __SYDNEY_EXECUTION_ACTION_BULKSEPARATOR_H

#include "Execution/Action/Module.h"

#include "Common/Object.h"

#include "ModKanjiCode.h"
#include "ModUnicodeChar.h"

class ModUnicodeString;

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

namespace
{
	class _PatternChecker; // defined in cpp file
}

//	CLASS
//	BulkSeparator -- Search for separator
//
//	NOTES
//		MT-unsafe

class BulkSeparator
	: Common::Object
{
public:
	typedef BulkSeparator This;
	typedef Common::Object Super;

	struct Status
	{
		enum Value
		{
			None,						// initial status
			Match,						// pattern found
			Continue,					// intermediate of a pattern
			HeadDoubleQuote,			// beginning of double quote
			DoubleQuote,				// in double quote (continue)
			TailDoubleQuote,			// end of double quote
			IllegalCharacter,			// illegal character is found
			ValueNum
		};
	};

	BulkSeparator()
		: Super(),
		  m_pChecker(0)
	{}
	~BulkSeparator();

	// add a keyword (keyword must be consist of ascii characters)
	void addKeyword(const ModUnicodeString& cKey_, int iValue_,
					bool bCaseSensitive_ = true);

	// fix the matching automaton
	void prepare();

	// check character string to keywords
	// return: end of processed part
	const char* check(const char* pTop_, const char* pTail_,
					  ModKanjiCode::KanjiCodeType eEncoding_,
					  bool bIgnoreHeadWQuote_ = false);
	const ModUnicodeChar* check(const ModUnicodeChar* pTop_,
								const ModUnicodeChar* pTail_,
								bool bIgnoreHeadWQuote_ = false);

	// matching status
	Status::Value getStatus();
	// matching value
	int getMatchValue();
	// matching keyword length
	int getMatchLength();

	// reset status
	void reset();

	// initialize object
	void initialize(bool bIgnoreWquote_ = false);
	// clear object
	void terminate();

protected:
private:
	// replace \n and \t to special characters
	void replaceKeyword(const ModUnicodeString& cKey_, const ModUnicodeChar* pFirstMatch_,
						ModUnicodeString& cResult_,
						ModUnicodeString& cResult2_,
						ModUnicodeString& cResult3_);

	// executing object (for suppressing implementation)
	_PatternChecker* m_pChecker;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_BULKSEPARATOR_H

//
//	Copyright (c) 2006, 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
