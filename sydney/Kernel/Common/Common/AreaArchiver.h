// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaArchiver.h -- a basic & virtual class just provide interface which every Data classes can call it in dumping data
//			it should be implemented in record2 module
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_AREAARCHIVER_H
#define __TRMEISTER_COMMON_AREAARCHIVER_H

//include file, maybe need other files
#include "Common/Module.h"
#include "Common/Object.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Common::AreaArchiver -- 
//		AreaArchiver
//	NOTES
//	We could take the class as the bridge from outer data to LinkedArea, and remove the old classes (linkobject...). 
//	Any data operation accessing to DirectArea must through it.
//
class SYD_COMMON_FUNCTION AreaArchiver : public Object
{
  public:
	//set dump value from DirectArea to Data
	//pBuf  :  the data buffer	uiSize_ :  dump size, it should be provided by Record2
	virtual ModSize read(void* pBuf_, ModSize uiSize_) = 0;
	//dump value from Data to DirectArea
	//pBuf  :  the data buffer	uiSize_ :  dump size,  Data should provide it by getDumpSize
	virtual ModSize write(const void* pBuf_, ModSize uiSize_) = 0;
	//skip and not dump value, for null data or others condition(decimal)
	//uiSize_ :  skip count		bReadOnly_ : skip for read or write
	virtual ModSize skip(ModSize uiSize_, bool bReadOnly_ = true) = 0;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif //__TRMEISTER_COMMON_AREAARCHIVER_H

/**************************************************************
IMPLEMENT DEMO

1. We should add 2 virtual functions in Common::Data header

ModSize setDumpedValue(AreaArchiver& cArchiver_, ModSize uiSize_) = 0;
ModSize dumpValue_NotNull(AreaArchiver& cArchiver_) = 0;

2. A demo in IntegerData class:

ModSize
IntegerData::
setDumpedValue(Archiver& AreaArchiver, ModSize uiSize_)
{
	//in fixed data, we can check the dumped size
	//but in variable data, we should allocate new buffer according to uiSize_
	//maybe we can provide a new function called setDumpSize
	; _TRMEISTER_ASSERT(uiSize_ == sizeof(m_iValue));
	
	cArchiver_.read(&m_iValue, uiSize_);
	setNull(false);

	//sometimes uiSize_ changed
	return uiSize_;
}

ModSize
IntegerData::
dumpValue_NotNull(AreaArchiver& cArchiver_)
{
	//in fixed data, dumpsize == sizeof(m_iValue) 
	//but in variable data, we should get the size by getDumpSize
	//or the size is explicit
	cArchiver_.write(&m_iValue, sizeof(m_iValue) );
	return sizeof(m_iValue);
}	

*/

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

