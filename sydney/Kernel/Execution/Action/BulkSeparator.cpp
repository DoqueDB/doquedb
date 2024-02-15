// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/BulkSeparator.cpp --
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/BulkSeparator.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Exception/NotSupported.h"

#include "Opt/Algorithm.h"

#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

namespace
{
	// character table size
	const int _NumOfChar = 128;
	// max separator length
	const int _MaxSeparatorLength = 20;
	// max number of separators
	const int _MaxSeparatorCount = 18; // normal: record, field and element (x2: 0x0a and 0x0d+0x0a)
									   // hint parser: HintKey(15), ' ', '\n', '\t'
	// max status size
	const int _NumOfStatus = (_MaxSeparatorLength * _MaxSeparatorCount) + 4;
										// +4: initial, double quote(head, in, tail)

	// fixed status
	const int _FailedStatus = 0;
	const int _HeadDoubleQuoteStatus = 1;
	const int _DoubleQuoteStatus = 2;
	const int _TailDoubleQuoteStatus = 3;
	const int _InitialStatus = 4;

	class _PatternChecker
	{
	public:
		_PatternChecker();
		~_PatternChecker();

		// add keyword
		void addKeyword(const ModUnicodeString& cKey_, int iValue_, bool bCaseSensitive_);

		// fix the AC Machine
		void prepare();

		// check character string to keywords
		const char* check(const char* pTop_, const char* pTail_,
						  ModKanjiCode::KanjiCodeType eEncoding_,
						  bool bIgnoreHeadWQuote_ = false);
		const ModUnicodeChar* check(const ModUnicodeChar* pTop_,
									const ModUnicodeChar* pTail_,
									bool bIgnoreHeadWQuote_ = false);

		// matching status
		BulkSeparator::Status::Value getStatus();
		// matching value
		int getMatchValue();
		// matching keyword length
		int getMatchLength();

		// reset status
		void reset();

		// initialize tables
		void initialize(bool bIgnoreWquote_);
	protected:
	private:
		// assign goto and output tables
		void createGoto(const ModUnicodeString& cKey_, int iValue_, bool bCaseSensitive_);
		// assign failure table
		void createFailure();

		// automaton
		int m_veciGoto[_NumOfStatus][_NumOfChar];
		int m_veciFailure[_NumOfStatus];
		int m_veciOutput[_NumOfStatus];
		int m_veciLength[_NumOfStatus];

		int m_iNewStatus;

		// used in checking
		int m_iStatus;
	};

	// FUNCTION public
	//	$$$::_PatternChecker::_PatternChecker -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	Nothing
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	_PatternChecker::
	_PatternChecker()
		: m_iNewStatus(_InitialStatus),
		  m_iStatus(_InitialStatus)
	{}

	// FUNCTION public
	//	$$$::_PatternChecker::~_PatternChecker -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	Nothing
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	_PatternChecker::
	~_PatternChecker()
	{}

	// FUNCTION public
	//	$$$::_PatternChecker::addKeyword -- add keyword
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeString& cKey_
	//	int iValue_
	//	bool bCaseSensitive_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_PatternChecker::
	addKeyword(const ModUnicodeString& cKey_, int iValue_, bool bCaseSensitive_)
	{
		if (cKey_.getLength() > 0) {
			// check the length of key
			int iKeyLength = cKey_.getLength();
			if (iKeyLength > _MaxSeparatorLength) {
				SydInfoMessage
					<< "Bulk parameter: too long separator."
					<< ModEndl;
				_SYDNEY_THROW0(Exception::NotSupported);
			}
			// create goto and output table
			createGoto(cKey_, iValue_, bCaseSensitive_);
		}
	}

	// FUNCTION public
	//	$$$::_PatternChecker::prepare -- fix the AC Machine
	//
	// NOTES
	//
	// ARGUMENTS
	//	Nothing
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_PatternChecker::
	prepare()
	{
		// create failure table
		createFailure();
	}

	// FUNCTION public
	//	$$$::_PatternChecker::initialize -- initialize tables
	//
	// NOTES
	//
	// ARGUMENTS
	//	bool bIgnoreWquote_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_PatternChecker::
	initialize(bool bIgnoreWquote_)
	{
		// initialize tables

		// for special statuses
		if (!bIgnoreWquote_) {
			m_veciFailure[_FailedStatus] = _InitialStatus;
			m_veciFailure[_HeadDoubleQuoteStatus] = _InitialStatus;
			m_veciFailure[_DoubleQuoteStatus] = _InitialStatus;
			m_veciFailure[_TailDoubleQuoteStatus] = _InitialStatus;
			m_veciOutput[_FailedStatus] = -1;
			m_veciOutput[_HeadDoubleQuoteStatus] = -1;
			m_veciOutput[_DoubleQuoteStatus] = -1;
			m_veciOutput[_TailDoubleQuoteStatus] = -1;
			m_veciLength[_FailedStatus] = 0;
			m_veciLength[_HeadDoubleQuoteStatus] = 0;
			m_veciLength[_DoubleQuoteStatus] = 0;
			m_veciLength[_TailDoubleQuoteStatus] = 0;
			for (int j = 0; j < _NumOfChar; ++j) {
				// in double quote, all characters except for double quote don't change status
				m_veciGoto[_FailedStatus][j] = _FailedStatus;
				m_veciGoto[_HeadDoubleQuoteStatus][j] = _DoubleQuoteStatus;
				m_veciGoto[_DoubleQuoteStatus][j] = _DoubleQuoteStatus;
				m_veciGoto[_TailDoubleQuoteStatus][j] = _InitialStatus;
			}
			m_veciGoto[_HeadDoubleQuoteStatus][Common::UnicodeChar::usWquate] = _TailDoubleQuoteStatus;
			m_veciGoto[_DoubleQuoteStatus][Common::UnicodeChar::usWquate] = _TailDoubleQuoteStatus;
			m_veciGoto[_TailDoubleQuoteStatus][Common::UnicodeChar::usWquate] = _HeadDoubleQuoteStatus;
		}
		// for other status
		for (int i = _InitialStatus; i < _NumOfStatus; ++i) {
			m_veciFailure[i] = _InitialStatus;
			m_veciOutput[i] = -1;
			m_veciLength[i] = 0;
			for (int j = 0; j < _NumOfChar; ++j) {
				m_veciGoto[i][j] = _FailedStatus;
			}
		}
		if (!bIgnoreWquote_) {
			m_veciGoto[_InitialStatus][Common::UnicodeChar::usWquate] = _HeadDoubleQuoteStatus;
		}
		m_iNewStatus = _InitialStatus;
	}

	// FUNCTION private
	//	$$$::_PatternChecker::createGoto -- assign goto and output tables
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeString& cKey_
	//	int iValue_
	//	bool bCaseSensitive_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_PatternChecker::
	createGoto(const ModUnicodeString& cKey_, int iValue_, bool bCaseSensitive_)
	{
		const ModUnicodeChar* p = static_cast<const ModUnicodeChar*>(cKey_);
		const ModUnicodeChar* pTail = cKey_.getTail();
		int iStatus = _InitialStatus;
		int iLength = 0;
		bool bUseNew = false;
		for (; p < pTail; ++p, ++iLength) {
			if ((*p) >= static_cast<ModUnicodeChar>(_NumOfChar - 1)) {
				// only 1 byte character is allowed to consist separator
				// (2008/02/01) 127(DEL) is used for special purpose.
				SydInfoMessage
					<< "Bulk parameter: separator should consist of ascii characters."
					<< ModEndl;
				_SYDNEY_THROW0(Exception::NotSupported);
			}

			if (!bUseNew && (m_veciGoto[iStatus][*p] != _FailedStatus)) {
				iStatus = m_veciGoto[iStatus][*p];
				continue;
			} else {
				bUseNew = true;
			}
			if (bCaseSensitive_) {
				m_veciGoto[iStatus][*p] = ++m_iNewStatus;
			} else {
				++m_iNewStatus;
				m_veciGoto[iStatus][ModUnicodeCharTrait::toUpper(*p)] = m_iNewStatus;
				m_veciGoto[iStatus][ModUnicodeCharTrait::toLower(*p)] = m_iNewStatus;
			}
			iStatus = m_iNewStatus;

			m_veciLength[iStatus] = iLength;
		}
		if (m_veciOutput[iStatus] != -1) {
			// overwrapped keywords
			SydInfoMessage
				<< "Bulk parameter: separator should not overwrapped each other"
				<< ModEndl;
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		m_veciOutput[iStatus] = iValue_;
	}

	// FUNCTION private
	//	$$$::_PatternChecker::createFailure -- assign failure table
	//
	// NOTES
	//
	// ARGUMENTS
	//	Nothing
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_PatternChecker::
	createFailure()
	{
		SHORTVECTOR<int> vecQueue;
		int iQueueTop = 0;
		int iQueueTail = 0;

		vecQueue.reserve(_NumOfStatus);

		for (int i = 0; i < _NumOfChar; ++i) {
			int iStatus = m_veciGoto[_InitialStatus][i];
			if (iStatus == _FailedStatus) {
				// set goto=0 for initial status
				m_veciGoto[_InitialStatus][i] = _InitialStatus;

			} else if (iStatus > _InitialStatus) {
				// add queue
				++iQueueTail;
				vecQueue.PUSHBACK(iStatus);
				// f =  initial status
				m_veciFailure[iStatus] = _InitialStatus;
				// copy status to 'TailDoubleQuote'
				m_veciGoto[_TailDoubleQuoteStatus][i] = iStatus;

			} else {
				;
			}
		}
		// breadth first search
		while (iQueueTop < iQueueTail) {
			int iPrev = vecQueue[iQueueTop++];

			for (int i = 0; i < _NumOfChar; ++i) {
				// check whether iPrev can go to next status by <i>
				int iStatus = m_veciGoto[iPrev][i];
				if (iStatus > _InitialStatus) {
					// add queue
					++iQueueTail;
					vecQueue.PUSHBACK(iStatus);

					// search for existing link by <i>
					int iLink = m_veciFailure[iPrev];
					while (m_veciGoto[iLink][i] == _FailedStatus) {
						iLink = m_veciFailure[iLink];
					}
					m_veciFailure[iStatus] = m_veciGoto[iLink][i];

					// check value duplication
					int iNewOutput = m_veciOutput[m_veciFailure[iStatus]];

					if (m_veciOutput[iStatus] == -1) {
						m_veciOutput[iStatus] = iNewOutput;

					} else if ((iNewOutput != -1)
							   && (m_veciOutput[iStatus] != iNewOutput)) {
						// overwrapped keywords
						SydInfoMessage
							<< "Bulk parameter: separator should not overwrapped each other"
							<< ModEndl;
						_SYDNEY_THROW0(Exception::NotSupported);
					}
				}
			}
		}
	}

	// FUNCTION public
	//	$$$::_PatternChecker::check -- check character string to keywords
	//
	// NOTES
	//
	// ARGUMENTS
	//	const char* pTop_
	//	const char* pTail_
	//	ModKanjiCode::KanjiCodeType eEncoding_
	//	bool bIgnoreHeadWQuote_ /* = false */
	//	
	// RETURN
	//	const char*
	//
	// EXCEPTIONS

	const char*
	_PatternChecker::
	check(const char* pTop_, const char* pTail_, ModKanjiCode::KanjiCodeType eEncoding_,
		  bool bIgnoreHeadWQuote_ /* = false */)
	{
		const char* p = pTop_;
		while(p < pTail_) {
			ModSize iCharSize = ModKanjiCode::getCharacterSize(*p, eEncoding_);
			if (static_cast<ModSize>(pTail_ - p) < iCharSize) {
				// need more data but pattern do not match
				if (m_iStatus > _InitialStatus) {
					m_iStatus = _InitialStatus;
				}
				break;
			}
			if (iCharSize == 0) {
				// Illegal data found
				m_iStatus = _FailedStatus;
				// move cursor
				++p;
				break;
			}
			if (static_cast<ModUnicodeChar>(*p) >= static_cast<ModUnicodeChar>(_NumOfChar)) {
				// just skip
				// status should be reset
				// status is reset using GOTO table
				; _SYDNEY_ASSERT(m_iStatus >= 0);
				; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
				while (m_veciGoto[m_iStatus][_NumOfChar - 1] == _FailedStatus) {
					; _SYDNEY_ASSERT(m_iStatus >= 0);
					; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
					m_iStatus = m_veciFailure[m_iStatus];
				}
				m_iStatus = m_veciGoto[m_iStatus][_NumOfChar - 1];
				p += iCharSize;
				continue;
			}

			; _SYDNEY_ASSERT(m_iStatus >= 0);
			; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
			while (m_veciGoto[m_iStatus][*p] == _FailedStatus) {
				; _SYDNEY_ASSERT(m_iStatus >= 0);
				; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
				m_iStatus = m_veciFailure[m_iStatus];
			}
			; _SYDNEY_ASSERT(m_iStatus >= 0);
			; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
			m_iStatus = m_veciGoto[m_iStatus][*p];

			; _SYDNEY_ASSERT(m_iStatus >= 0);
			; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
			if (m_veciOutput[m_iStatus] >= 0) {
				// match
				break;
			}
			if ((!bIgnoreHeadWQuote_ && m_iStatus == _HeadDoubleQuoteStatus)
				|| m_iStatus == _TailDoubleQuoteStatus) {
				// end of double quote part
				break;
			}
			// goto next char
			++p;
		}
		// anyway, return current pointer
		return p;
	}

	// FUNCTION public
	//	$$$::_PatternChecker::check -- check character string to keywords (ModUnicodeChar version)
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeChar* pTop_
	//	const ModUnicodeChar* pTail_
	//	bool bIgnoreHeadWQuote_ /* = false */
	//	
	// RETURN
	//	const ModUnicodeChar*
	//
	// EXCEPTIONS

	const ModUnicodeChar*
	_PatternChecker::
	check(const ModUnicodeChar* pTop_, const ModUnicodeChar* pTail_,
		  bool bIgnoreHeadWQuote_ /* = false */)
	{
		const ModUnicodeChar* p = pTop_;
		for (; p < pTail_; ++p) {
			if ((*p) >= static_cast<ModUnicodeChar>(_NumOfChar)) {
				// non-ascii characters are ignored
				// status is reset using GOTO table
				; _SYDNEY_ASSERT(m_iStatus >= 0);
				; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
				while (m_veciGoto[m_iStatus][_NumOfChar - 1] == _FailedStatus) {
					; _SYDNEY_ASSERT(m_iStatus >= 0);
					; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
					m_iStatus = m_veciFailure[m_iStatus];
				}
				; _SYDNEY_ASSERT(m_iStatus >= 0);
				; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
				m_iStatus = m_veciGoto[m_iStatus][_NumOfChar - 1];
				continue;
			}
			; _SYDNEY_ASSERT(m_iStatus >= 0);
			; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
			while (m_veciGoto[m_iStatus][*p] == _FailedStatus) {
				; _SYDNEY_ASSERT(m_iStatus >= 0);
				; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
				m_iStatus = m_veciFailure[m_iStatus];
			}
			; _SYDNEY_ASSERT(m_iStatus >= 0);
			; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
			m_iStatus = m_veciGoto[m_iStatus][*p];

			; _SYDNEY_ASSERT(m_iStatus >= 0);
			; _SYDNEY_ASSERT(m_iStatus < _NumOfStatus);
			if (m_veciOutput[m_iStatus] >= 0) {
				// match
				break;
			}
			if ((!bIgnoreHeadWQuote_ && m_iStatus == _HeadDoubleQuoteStatus)
				|| m_iStatus == _TailDoubleQuoteStatus) {
				// end of double quote part
				break;
			}
		}
		// anyway, return current pointer
		return p;
	}

	// FUNCTION public
	//	$$$::_PatternChecker::getStatus -- matching status
	//
	// NOTES
	//
	// ARGUMENTS
	//	Nothing
	//
	// RETURN
	//	BulkSeparator::Status::Value
	//
	// EXCEPTIONS

	BulkSeparator::Status::Value
	_PatternChecker::
	getStatus()
	{
		if (m_veciOutput[m_iStatus] >= 0) {
			return BulkSeparator::Status::Match;
		} else if (m_iStatus > _InitialStatus) {
			return BulkSeparator::Status::Continue;
		} else if (m_iStatus == _HeadDoubleQuoteStatus) {
			return BulkSeparator::Status::HeadDoubleQuote;
		} else if (m_iStatus == _DoubleQuoteStatus) {
			return BulkSeparator::Status::DoubleQuote;
		} else if (m_iStatus == _TailDoubleQuoteStatus) {
			return BulkSeparator::Status::TailDoubleQuote;
		} else if (m_iStatus == _FailedStatus) {
			return BulkSeparator::Status::IllegalCharacter;
		} else {
			return BulkSeparator::Status::None;
		}
	}

	// FUNCTION public
	//	$$$::_PatternChecker::getMatchValue -- matching value
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
	_PatternChecker::
	getMatchValue()
	{
		return m_veciOutput[m_iStatus];
	}

	// FUNCTION public
	//	$$$::_PatternChecker::getMatchLength -- matching keyword length
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
	_PatternChecker::
	getMatchLength()
	{
		return m_veciLength[m_iStatus];
	}

	// FUNCTION public
	//	$$$::_PatternChecker::reset -- reset status
	//
	// NOTES
	//
	// ARGUMENTS
	//	Nothing
	//
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_PatternChecker::
	reset()
	{
		m_iStatus = _InitialStatus;
	}
} // namespace

// FUNCTION public
//	Action::BulkSeparator::~BulkSeparator -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

BulkSeparator::
~BulkSeparator()
{
	try {
		terminate();
	} catch (...) {
		// ignore exceptions in destructor
		;
	}
}

// FUNCTION public
//	Action::BulkSeparator::addKeyword -- add a keyword (keyword must be consist of ascii characters)
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cKey_
//	int iValue_
//	bool bCaseSensitive_ /* = true */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkSeparator::
addKeyword(const ModUnicodeString& cKey_, int iValue_,
		   bool bCaseSensitive_ /* = true */)
{
	if (cKey_.getLength() == 0) {
		return;
	}

	// search for backslash
	const ModUnicodeChar* p = cKey_.search(Common::UnicodeChar::usBackSlash);
	if (!p) {
		m_pChecker->addKeyword(cKey_, iValue_, bCaseSensitive_);
	} else {
		ModUnicodeString cTmpKey;
		ModUnicodeString cTmpKey2; // for expand '\n'
		ModUnicodeString cTmpKey3; // for expand '\n'

		replaceKeyword(cKey_, p, cTmpKey, cTmpKey2, cTmpKey3);
		m_pChecker->addKeyword(cTmpKey, iValue_, bCaseSensitive_);
		if (cTmpKey2.getLength() > 0) {
			m_pChecker->addKeyword(cTmpKey2, iValue_, bCaseSensitive_);
		}
		if (cTmpKey3.getLength() > 0) {
			m_pChecker->addKeyword(cTmpKey3, iValue_, bCaseSensitive_);
		}
	}
}

// FUNCTION public
//	Action::BulkSeparator::prepare -- fix the matching automaton
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkSeparator::
prepare()
{
	m_pChecker->prepare();
}

// FUNCTION public
//	Action::BulkSeparator::check -- check character string to keywords
//
// NOTES
//
// ARGUMENTS
//	const char* pTop_
//	const char* pTail_
//	ModKanjiCode::KanjiCodeType eEncoding_
//	bool bIgnoreHeadWQuote_ /* = false */
//	
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
BulkSeparator::
check(const char* pTop_, const char* pTail_, ModKanjiCode::KanjiCodeType eEncoding_,
	  bool bIgnoreHeadWQuote_ /* = false */)
{
	return m_pChecker->check(pTop_, pTail_, eEncoding_, bIgnoreHeadWQuote_);
}

// FUNCTION public
//	Action::BulkSeparator::check -- 
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeChar* pTop_
//	const ModUnicodeChar* pTail_
//	bool bIgnoreHeadWQuote_ /* = false */
//	
// RETURN
//	const ModUnicodeChar*
//
// EXCEPTIONS

const ModUnicodeChar*
BulkSeparator::
check(const ModUnicodeChar* pTop_, const ModUnicodeChar* pTail_,
	  bool bIgnoreHeadWQuote_ /* = false */)
{
	return m_pChecker->check(pTop_, pTail_, bIgnoreHeadWQuote_);
}

// FUNCTION public
//	Action::BulkSeparator::getStatus -- matching status
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	BulkSeparator::Status::Value
//
// EXCEPTIONS

BulkSeparator::Status::Value
BulkSeparator::
getStatus()
{
	return m_pChecker->getStatus();
}

// FUNCTION public
//	Action::BulkSeparator::getMatchValue -- matching value
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
BulkSeparator::
getMatchValue()
{
	return m_pChecker->getMatchValue();
}

// FUNCTION public
//	Action::BulkSeparator::getMatchLength -- matching keyword length
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
BulkSeparator::
getMatchLength()
{
	return m_pChecker->getMatchLength();
}

// FUNCTION public
//	Action::BulkSeparator::reset -- reset status
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkSeparator::
reset()
{
	m_pChecker->reset();
}

// FUNCTION public
//	Action::BulkSeparator::initialize -- initialize object
//
// NOTES
//
// ARGUMENTS
//	bool bIgnoreWquote_ /* = false */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkSeparator::
initialize(bool bIgnoreWquote_ /* = false */)
{
	m_pChecker = new _PatternChecker;
	m_pChecker->initialize(bIgnoreWquote_);
}

// FUNCTION public
//	Action::BulkSeparator::terminate -- clear object
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkSeparator::
terminate()
{
	delete m_pChecker, m_pChecker = 0;
}

// FUNCTION private
//	Action::BulkSeparator::replaceKeyword -- replace \n and \t to special characters
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cKey_
//	const ModUnicodeChar* pFirstMatch_
//	ModUnicodeString& cResult_
//	ModUnicodeString& cResult2_
//	ModUnicodeString& cResult3_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkSeparator::
replaceKeyword(const ModUnicodeString& cKey_, const ModUnicodeChar* pFirstMatch_,
			   ModUnicodeString& cResult_, ModUnicodeString& cResult2_, ModUnicodeString& cResult3_)
{
	// allocate same size
	cResult_.reallocate(cKey_.getLength());

	const ModUnicodeChar* p = pFirstMatch_; // first backslash
	const ModUnicodeChar* pTop = static_cast<const ModUnicodeChar*>(cKey_);
	const ModUnicodeChar* pTail = cKey_.getTail();

	// copy before the backslash
	if (p > pTop) {
		cResult_.append(pTop, static_cast<ModSize>(p - pTop));
	}
	do {
		// goto next
		++p;

		// convert '\n' and '\t' to special characters
		switch (*p) {
		case Common::UnicodeChar::usSmallT:
			{
				cResult_.append(Common::UnicodeChar::usCtrlTab);
				if (cResult2_.getLength() > 0) {
					cResult2_.append(Common::UnicodeChar::usCtrlTab);
				}
				if (cResult3_.getLength() > 0) {
					cResult3_.append(Common::UnicodeChar::usCtrlTab);
				}
				break;
			}
		case Common::UnicodeChar::usSmallN:
			{
				// prepare tmpkey2
				if (cResult2_.getLength() == 0) {
					cResult2_.reallocate(cKey_.getLength());
					if (p - 1 > pTop) {
						cResult2_.append(pTop, static_cast<ModSize>(p - 1 - pTop));
					}
				}
				// prepare tmpkey3
				if (cResult3_.getLength() == 0) {
					cResult3_.reallocate(cKey_.getLength());
					if (p - 1 > pTop) {
						cResult3_.append(pTop, static_cast<ModSize>(p - 1 - pTop));
					}
				}
				// create key for sole 0x0a, 0x0d + 0x0a and 0x0d + 0x0d + 0x0a
				cResult_.append(Common::UnicodeChar::usCtrlCr);
				cResult_.append(Common::UnicodeChar::usCtrlRet);

				cResult2_.append(Common::UnicodeChar::usCtrlRet);

				// some editor might produce 0d0d0a as line terminator.
				// (maybe by a bug of the editor)
				cResult3_.append(Common::UnicodeChar::usCtrlCr);
				cResult3_.append(Common::UnicodeChar::usCtrlCr);
				cResult3_.append(Common::UnicodeChar::usCtrlRet);
				break;
			}
		default:
			{
				// invalid backslash sequence
				SydInfoMessage
					<< "Bulk parameter: separator should not contain backslash except for '\\n' or '\\t'"
					<< ModEndl;
				_SYDNEY_THROW0(Exception::NotSupported);
			}
		}

		// goto next
		++p;
		if (p < pTail) {
			// search for next backslash
			const ModUnicodeChar* q =
				ModUnicodeCharTrait::find(p, Common::UnicodeChar::usBackSlash, static_cast<ModSize>(pTail - p));
			if (!q) {
				q = pTail;
			}
			// copy before the backslash
			if (q > p) {
				cResult_.append(p, static_cast<ModSize>(q - p));
				if (cResult2_.getLength() > 0) {
					cResult2_.append(p, static_cast<ModSize>(q - p));
				}
				if (cResult3_.getLength() > 0) {
					cResult3_.append(p, static_cast<ModSize>(q - p));
				}
			}
			p = q;
		}
	} while (p < pTail);
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
