// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PatternChecker.h --
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

#ifndef __SYDNEY_UTILITY_PATTERNCHECKER_H
#define __SYDNEY_UTILITY_PATTERNCHECKER_H

#include "Utility/Module.h"

#include "ModMap.h"
#include "ModPair.h"
#include "ModVector.h"
#include "ModUnicodeChar.h"

class ModUnicodeString;

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

//	CLASS
//	PatternChecker -- Search for pattern
//
//	NOTES
//	This algorithm uses AC(Aho-Corasick) method.
//	See "Kenji Kita eds. (2002) Information Retrieval Algorithms, Kyoritsu Shuppan"
//	for details.
//
//	HOW TO USE
//	PatternChecker c;
//	c.initialize();
//	c.addPattern(cstrPattern0_); // cstrPattern0_'s ID is 0.
//	c.addPattern(cstrPattern1_); // cstrPattern1_'s ID is 1.
//	...
//	c.prepare();
//	c.check(cstrSrc0_, vecPatternLocation0_);
//	c.check(cstrSrc1_, vecPatternLocation1_);
//	...
//	c.clear();
//	c.initialize();
//	c.addPattern(...);
//	...
//	c.prepare();
//	c.check(...);
//	...
//
class PatternChecker
{
public:
	// Vector of pattern's location and ID
	// The location is top of the pattern by 0-base.
	typedef ModPair<ModSize, ModSize> PatternLocationElement;
	typedef ModVector<PatternLocationElement> PatternLocationVec;

	PatternChecker();
	virtual ~PatternChecker();

	// initialize tables
	void initialize();
	
	// add pattern
	void addPattern(const ModUnicodeString& cstrPattern_);

	// fix the AC Machine
	void prepare();

	// clear graph and tables
	void clear();
	
	// check character string to keywords
	void check(const ModUnicodeString& cstrSrc_,
			   PatternLocationVec& vecPatternLocation_) const;
	
	// Get pattern's length
	ModSize getPatternLength(ModSize uiPatternID_) const;
	const ModVector<ModSize>& getPatternLength() const { return m_vecuiLength; }

	// wordchecker ?
	bool isWordChecker() const { return m_bWordChecker; }

	// Get word separator
	static ModUnicodeChar getWordSeparator();
	
protected:
private:
	// Map of one pattern and status
	typedef ModMap<ModUnicodeChar, ModSize, ModLess<ModUnicodeChar> > GotoMap;
	// Vector of PatternID
	typedef ModVector<ModSize> OutputVec;
	
	// assign goto and output tables
	void createGoto(const ModUnicodeString& cKey_,
					ModSize uiPatternID_,
					ModSize& uiPatternLength_);
	// assign failure table
	void createFailure();
	
	
	// These count is equal to status.
	ModVector<GotoMap*> m_vecpGoto;
	ModVector<OutputVec*> m_vecpOutput;
	// Vector of status
	ModVector<ModSize> m_vecuiFailure;

	// The count is equal to pattern.
	// Vector of pattern's length
	ModVector<ModSize> m_vecuiLength;

	// The type of some patterns are word, not all string.
	bool m_bWordChecker;
};

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif // __TRMEISTER_UTILITY_PATTERNCHECKER_H

//
//	Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
