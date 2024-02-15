// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PatternChecker.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Utility";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Utility/PatternChecker.h"

#include "Common/Assert.h"

#include "ModUnicodeString.h"

_TRMEISTER_USING
_TRMEISTER_UTILITY_USING

namespace
{
	// fixed status
	const ModSize _InitialStatus = 0;

	// The word separator which divides a pattern into words
	// [NOTE] 0xFFFF is one of a disused code point in Unicode.
	const ModUnicodeChar _usWordSeparator = 0xFFFF;
}

//
//	FUNCTION public
//	Utility::PatternChecker::PatternChecker -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
PatternChecker::PatternChecker()
	: m_vecpGoto(), m_vecpOutput(), m_vecuiFailure(), m_vecuiLength(),
	  m_bWordChecker(false)
{}

//
//	FUNCTION public
//	Utility::PatternChecker::~PatternChecker -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
PatternChecker::~PatternChecker()
{
	clear();
}

// 
//	FUNCTION public
//	Utility::PatternChecker::initialize -- initialize tables
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
PatternChecker::initialize()
{
	; _TRMEISTER_ASSERT(m_vecpGoto.getSize() == 0);
	; _TRMEISTER_ASSERT(m_vecpOutput.getSize() == 0);
	
	m_bWordChecker = false;
	
	// initialize graph
	GotoMap* map = new GotoMap();
	m_vecpGoto.pushBack(map);
	
	// initialize output table
	OutputVec* vec = new OutputVec();
	m_vecpOutput.pushBack(vec);
}

//
//	FUNCTION public
//	Utility::PatternChecker::addPattern -- add pattern
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
PatternChecker::addPattern(const ModUnicodeString& cstrPattern_)
{
	; _TRMEISTER_ASSERT(cstrPattern_.getLength() > 0);
	
	ModSize uiPatternID = m_vecuiLength.getSize();
	ModSize uiPatternLength = 0;
	// create goto graph and output table
	createGoto(cstrPattern_, uiPatternID, uiPatternLength);
	// [NOTE] cstrPattern_ may include the word separator characters,
	//  so you don't use cstrPattern_.getLength().
	m_vecuiLength.pushBack(uiPatternLength);
}

// 
//	FUNCTION public
//	Utility::PatternChecker::prepare -- fix the AC Machine
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
PatternChecker::prepare()
{
	// create failure table
	createFailure();
}

//
//	FUNCTION public
//	Utility::PatternChecker::clear -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
PatternChecker::clear()
{
	for (ModVector<GotoMap*>::Iterator i = m_vecpGoto.begin();
		 i < m_vecpGoto.end(); ++i)
	{
		delete *i, *i = 0;
	}
	m_vecpGoto.erase(m_vecpGoto.begin(), m_vecpGoto.end());
	for (ModVector<OutputVec*>::Iterator i = m_vecpOutput.begin();
		 i < m_vecpOutput.end(); ++i)
	{
		delete *i, *i = 0;
	}
	m_vecpOutput.erase(m_vecpOutput.begin(), m_vecpOutput.end());
	m_vecuiFailure.erase(m_vecuiFailure.begin(), m_vecuiFailure.end());
	m_vecuiLength.erase(m_vecuiLength.begin(), m_vecuiLength.end());
}

// 
//	FUNCTION public
//	Utility::PatternChecker::check -- check character string to keywords (ModUnicodeChar version)
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
PatternChecker::check(const ModUnicodeString& cstrSrc_,
					  PatternLocationVec& vecPatternLocation_) const
{
	; _TRMEISTER_ASSERT(vecPatternLocation_.getSize() == 0);
	
	if (cstrSrc_.getLength() > 0)
	{
		ModSize uiStatus = _InitialStatus;
		const ModUnicodeChar* p = cstrSrc_;
		const ModUnicodeChar* const s = p;
		const ModUnicodeChar* const e = cstrSrc_.getTail();
		ModSize uiWordSeparatorCount = 0;
		for (; p < e; ++p)
		{
			if (*p == _usWordSeparator)
			{
				++uiWordSeparatorCount;
			}
			GotoMap::Iterator i;
			bool bFound = false;
			while (true)
			{
				i = m_vecpGoto[uiStatus]->find(*p);
				if (i != m_vecpGoto[uiStatus]->end())
				{
					bFound = true;
					break;
				}
				else
				{
					if (uiStatus == _InitialStatus)
					{
						break;
					}
					uiStatus = m_vecuiFailure[uiStatus];
				}
			}
			
			if (bFound == true)
			{
				// Go to next node
				uiStatus = (*i).second;

				// Set PatternLocationElements
				OutputVec::ConstIterator j = m_vecpOutput[uiStatus]->begin();
				const OutputVec::ConstIterator jEnd = m_vecpOutput[uiStatus]->end();
				for (; j < jEnd; ++j)
				{
					; _TRMEISTER_ASSERT(
						p - s >= uiWordSeparatorCount + m_vecuiLength[*j] - 1);
					vecPatternLocation_.pushBack(
						PatternLocationElement(
							static_cast<ModSize>(p - s)
							- uiWordSeparatorCount - (m_vecuiLength[*j] - 1),
							*j));
				}
			}
		}

		// Sort by location of patterns.
		ModSort(vecPatternLocation_.begin(), vecPatternLocation_.end());
	}
}

//
//	FUNCTION public
//	Utility::PatternChecker::getPatternLength --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
PatternChecker::getPatternLength(ModSize uiPatternID_) const
{
	return m_vecuiLength[uiPatternID_];
}

//
//	FUNCTION public static
//	Utility::PatternChecker::getWordSeparator --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModUnicodeChar
PatternChecker::getWordSeparator()
{
	return _usWordSeparator;
}

//
//	FUNCTION private
//	Utility::PatternChecker::createGoto -- assign goto graph and tables
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
PatternChecker::createGoto(const ModUnicodeString& cstrPattern_,
						   ModSize uiPatternID_,
						   ModSize& uiPatternLength_)
{
	; _TRMEISTER_ASSERT(uiPatternLength_ == 0);
	
	const ModUnicodeChar* p = cstrPattern_;
	const ModUnicodeChar* const e = cstrPattern_.getTail();
	ModSize uiStatus = _InitialStatus;
	bool bUseNew = false;
	for (; p != e; ++p)
	{
		// [NOTE] A pattern may include word separators.
		if (*p == _usWordSeparator)
		{
			m_bWordChecker = true;
		}
		else
		{
			++uiPatternLength_;
		}
		
		if (bUseNew == false)
		{
			GotoMap::Iterator i = m_vecpGoto[uiStatus]->find(*p);
			if (i != m_vecpGoto[uiStatus]->end())
			{
				// Found the edge.
				
				// Go to next node.
				uiStatus = (*i).second;
				continue;
			}
			bUseNew = true;
		}
		
		// Create new node.
		GotoMap* map = new GotoMap();
		m_vecpGoto.pushBack(map);
		// Create new edge.
		m_vecpGoto[uiStatus]->insert(*p, m_vecpGoto.getSize() - 1);
		// Go to next node.
		uiStatus = m_vecpGoto.getSize() - 1;
		
		// Update output table.
		OutputVec* vec = new OutputVec();
		m_vecpOutput.pushBack(vec);
	}
	
	m_vecpOutput[uiStatus]->pushBack(uiPatternID_);
}

// 
//	FUNCTION private
//	Utility::PatternChecker::createFailure -- assign failure table
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
PatternChecker::createFailure()
{
	// Initialize failure table.
	m_vecuiFailure.assign(m_vecpGoto.getSize(), _InitialStatus);

	ModVector<ModSize> vecQueue;
	vecQueue.reserve(m_vecpGoto.getSize());
	for (GotoMap::Iterator i = m_vecpGoto[_InitialStatus]->begin();
		 i != m_vecpGoto[_InitialStatus]->end(); ++i)
	{
		// add queue
		vecQueue.pushBack((*i).second);
	}
	// breadth first search
	for (ModSize q = 0; q < vecQueue.getSize(); ++q)
	{
		ModSize uiQueueTop = vecQueue.at(q);
		for (GotoMap::Iterator i = m_vecpGoto[uiQueueTop]->begin();
			 i != m_vecpGoto[uiQueueTop]->end(); ++i)
		{
			ModUnicodeChar usKey = (*i).first;
			ModSize uiStatus = (*i).second;
			
			// add queue
			vecQueue.pushBack(uiStatus);

			// check whether uiTemp can go to next status by the key
			ModSize uiTemp = uiQueueTop;
			while (uiTemp != _InitialStatus)
			{
				uiTemp = m_vecuiFailure[uiTemp];
				GotoMap::Iterator j = m_vecpGoto[uiTemp]->find(usKey);
				if (j != m_vecpGoto[uiTemp]->end())
				{
					// Found edge of usKey.
					m_vecuiFailure[uiStatus] = (*j).second;
					uiTemp = (*j).second;
					break;
				}
			}

			// Merge output.
			; _TRMEISTER_ASSERT(uiTemp != _InitialStatus ||
								m_vecpOutput[uiTemp]->getSize() == 0);
			m_vecpOutput[uiStatus]->insert(m_vecpOutput[uiStatus]->end(),
										   m_vecpOutput[uiTemp]->begin(),
										   m_vecpOutput[uiTemp]->end());
		}
	}
}

//
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
