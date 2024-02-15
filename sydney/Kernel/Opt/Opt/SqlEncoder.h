// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SqlEncoder.h -- definition for explain plan tree
// 
// Copyright (c) 2007, 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_SQLENCODER_H
#define __SYDNEY_OPT_SQLENCODER_H

#include "Opt/Module.h"

#include "Common/Object.h"

#include "Plan/Interface/IScalar.h"


#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

class SqlEncoder
	: public Common::Object
{

public:

	// constructor
	SqlEncoder()
		: m_cStream(),
		  m_pDelimiter(0),
		  m_bNode(false)
	{}
	// destructor
	~SqlEncoder() {}


	// get result as string
	ModUnicodeChar* getString() {return m_cStream.getString();}
	void setDelimiter(const char* pDelimiter) {m_pDelimiter = pDelimiter;}

	// add string
	void appendString(const ModUnicodeString& cstrValue_);
	void appendNode(Plan::Interface::IScalar* pScalar_);

//	void appendRow(const Plan::Interface::IRow* pRow_);

	void appendSpace() {appendString(" ");}
	void appendDelimiter() {appendString(m_pDelimiter);}
	void appendLeftParenthesis() {appendString("(");}
	void appendRightParenthesis() {appendString(")");}

private:
	ModUnicodeOstrStream m_cStream;
	const char* m_pDelimiter;
	bool m_bNode;
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_SQLENCODER_H

//
// Copyright (c) 2007, 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
