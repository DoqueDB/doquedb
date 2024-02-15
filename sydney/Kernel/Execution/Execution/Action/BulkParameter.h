// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/BulkParameter.h --
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

#ifndef __SYDNEY_EXECUTION_ACTION_BULKPARAMETER_H
#define __SYDNEY_EXECUTION_ACTION_BULKPARAMETER_H

#include "Execution/Action/Module.h"

#include "Common/Object.h"

#include "Opt/Algorithm.h"

#include "Os/Path.h"

#include "ModKanjiCode.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

//	CLASS
//	BulkParameter -- Parameter values using in bulk input/output
//
//	NOTES

class BulkParameter
	: public Common::Object
{
public:
	typedef BulkParameter This;
	typedef Common::Object Super;

	//	CLASS
	//	BulkParameter::ElementSpecification -- Element specification
	//
	//	NOTES

	class ElementSpecification
	{
	public:
		ElementSpecification() : m_vecSpec() {}
		~ElementSpecification() {}

		// set specification vector from a character string
		void setValue(const ModUnicodeChar* pHead_,
					  const ModUnicodeChar* pTail_);

		const SHORTVECTOR< PAIR<int, int> >& getValue() const
		{
			return m_vecSpec;
		}
	protected:
	private:
		// add a spec pair
		void addSpec(int iMin_, int iMax_);

		SHORTVECTOR< PAIR<int, int> > m_vecSpec;
	};

	class ElementIterator
	{
	public:
		explicit ElementIterator(const ElementSpecification& cSpec_)
			: m_vecSpec(cSpec_.getValue()), m_cIterator(m_vecSpec.begin())
		{}
		~ElementIterator() {}

		// check number
		bool isValid(int i_);
		// check status
		bool isEnd();
		// reset status
		void reset();
	protected:
	private:
		// never copy
		ElementIterator(const ElementIterator& cOther_);
		ElementIterator& operator=(const ElementIterator& cOther_);

		const SHORTVECTOR< PAIR<int, int> >& m_vecSpec;

		SHORTVECTOR< PAIR<int, int> >::CONSTITERATOR m_cIterator;
	};

	// constructors
	BulkParameter();
	~BulkParameter();

	// set values
	void setValues(const ModUnicodeString& cstrData_,
				   const ModUnicodeString& cstrWith_,
				   const ModUnicodeString& cstrHint_,
				   bool bIsInput_);

	// get values
	const Os::Path& getPath() const
	{return m_cPath;}
	const Os::Path& getErrorPath() const
	{return m_cErrorPath;}
	const ModUnicodeString& getFieldSeparator() const;
	const ModUnicodeString& getRecordSeparator() const;
	const ModUnicodeString& getElementSeparator() const;
	const char* getDateDelimiter() const
	{return m_cDateDelimiter ? &m_cDateDelimiter : 0;}
	const char* getDateTimeDelimiter() const
	{return m_cDateTimeDelimiter ? &m_cDateTimeDelimiter : 0;}
	const char* getTimeDelimiter() const
	{return m_cTimeDelimiter ? &m_cTimeDelimiter : 0;}
	const char* getMsecDelimiter() const
	{return m_cMsecDelimiter ? &m_cMsecDelimiter : 0;}
	ModKanjiCode::KanjiCodeType getEncoding() const
	{return m_eEncoding;}
	const ElementSpecification& getInputField() const
	{return m_cInputField;}
	const ElementSpecification& getInputRecord() const
	{return m_cInputRecord;}
	const ElementSpecification& getExternField() const
	{return m_cExternField;}
	int getCommitCount() const
	{return m_iCommitCount;}

	bool isNoExtern() const
	{return m_bIsNoExtern;}
	bool isNoElementNull() const
	{return m_bIsNoElementNull;}
	bool isNoDoubleQuote() const
	{return m_bIsNoDoubleQuote;}
	bool isInput() const
	{return m_bIsInput;}

	// initialize hint parser
	static void initializeParser();
	// terminate hint parser
	static void terminateParser();

	// other parameters
	int getMaxSize() const;
	int getMaxLog() const;
	int getExternalThreshold() const;
	const char* getFileKeyword() const;
	const char* getNullKeyword() const;
	const char* getDefaultKeyword() const;

	int getFileKeywordLength() const;
	int getNullKeywordLength() const;
	int getDefaultKeywordLength() const;

protected:
private:
	// do not copy
	BulkParameter(const BulkParameter& cOther_);
	BulkParameter& operator=(const BulkParameter& cOther_);

	// read spec file
	void readSpecFile(const ModUnicodeString& cstrWith_);

	// set parameter from hint or spec string
	void parseHint(const ModUnicodeString& cHint_);

	// get hint key for parseHint
	int getHintKey(const ModUnicodeChar*& pTop_,
				   const ModUnicodeChar* pTail_);

	// get hint value for parseHint
	void getHintValue(const ModUnicodeChar*& pDataTop_,
					  const ModUnicodeChar*& pDataTail_,
					  const ModUnicodeChar*& pTop_,
					  const ModUnicodeChar* pTail_);

	// set parameter value from a value
	void setParameter(int iKey_,
					  const ModUnicodeChar* pDataTop_,
					  const ModUnicodeChar* pDataTail_);

	// parameters
	Os::Path m_cPath;					// Path to data file
	Os::Path m_cErrorPath;				// Path to error file
	ModUnicodeString m_cFieldSeparator;	// field separator
	ModUnicodeString m_cRecordSeparator; // record separator
	ModUnicodeString m_cElementSeparator; // element separator
	char m_cDateDelimiter;				// delimiter for date field
	char m_cDateTimeDelimiter;			// delimiter between date field and time field
	char m_cTimeDelimiter;				// delimiter for time field
	char m_cMsecDelimiter;				// delimiter between time field and millisec field
	ModKanjiCode::KanjiCodeType m_eEncoding;
										// character encoding type
	ElementSpecification m_cInputField;	// input field specification
	ElementSpecification m_cInputRecord; // input record specification
	ElementSpecification m_cExternField; // external field specification
	int m_iCommitCount;					// commit count (-1 means whole)

	bool m_bIsNoExtern;					// true if file is not special token
	bool m_bIsNoElementNull;	  		// true if null does not means [null] but means null for array columns
	bool m_bIsNoDoubleQuote;			// true if double quote has no effect
	bool m_bIsInput;					// true if reading
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_BULKPARAMETER_H

//
//	Copyright (c) 2006, 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
