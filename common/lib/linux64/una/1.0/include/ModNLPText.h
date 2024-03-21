// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNLPText.h -- Hedaer file of interface
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
#ifndef __UNA_MODNLPTEXT_H
#define __UNA_MODNLPTEXT_H

#include "ModDefaultManager.h"

class ModUnicodeString;

//
// CLASS
//	ModNlpText -- This class is abstract class for the argument of set(2) method.
//
// NOTES
//	This class is abstruct class for the argument of set(2) method.
//
class ModNlpText : public ModDefaultObject
{
public:
//
// FUNCTION public
//	ModNlpText::ModNlpText -- constructor
//
// NOTES
//	default constructor of ModNlpText
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
/*!
	Class constructor.
	Notes: default constructor of ModNlpText
	@param[in]	ulMaxLen_	 Maximum buffer size.
*/
	ModNlpText(ModSize ulMaxLen_ = 65535);
//
// FUNCTION public
//	ModNlpText::ModNlpText -- destructor
//
// NOTES
//	destructor of ModNlpText
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
/*!
	Class destructor.
*/
	~ModNlpText();
//
// FUNCTION public
//	ModNlpText::read -- read next text
//
// NOTES
//	read next text.
//
// ARGUMENTS
//	ModUnicodeString& cstrPartOfTarget_
//		O: text to be read. The length of text should be less than ulReadLength_.
//	ModSize ulReadLength_
//		I: length to be read
//
// RETURN
//	ModBoolean
//		ModTrue:  succeed in getting text
//		ModFalse: no more text
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	read next text
	@param[in]	cstrPartOfTarget_	 text to be read. The length of text should be less than ulReadLength_.
	@param[in]	ulReadLength_	 length to be read
	\return	ModTrue: succeed in getting text
					ModFalse: no more text
*/
	virtual ModBoolean read(ModUnicodeString& cstrPartOfTarget_,ModSize ulReadLength_ = 32767) = 0;
//
// FUNCTION public
//	ModNlpText::getMaxLen -- return the value of _ulMaxLen
//
// NOTES
//	return the value of _ulMaxLen
//
// ARGUMENTS
//	none
//
// RETURN
//	ModSize
//		_ulMaxLen
//
// EXCEPTIONS
//	none
//
/*!
	return the value of _ulMaxLen
	\return	ModSize: _ulMaxLen
*/
	ModSize getMaxLen();
private:
	ModSize _ulMaxLen;	// Maximum buffer size. The value that non-processed text length adds 
						// ulReadLength_ of read method made must be less than this value.
						// (e.g. when this value is 20 and remained text length in analyzer is 5,
						// maximum value of ulReadLength_ is 15)
};


inline
ModNlpText::ModNlpText(ModSize ulMaxLen_)
: _ulMaxLen(ulMaxLen_)
{}

inline
ModNlpText::~ModNlpText()
{}

inline
ModSize
ModNlpText::getMaxLen()
{
	return _ulMaxLen;
}
#endif//__UNA_MODNLPTEXT_H

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
