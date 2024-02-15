// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/SerialMemory.h --
// 
// Copyright (c) 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_SERIALMEMORY_H
#define __SYDNEY_OPT_SERIALMEMORY_H

#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/functional.hpp"

#include "Opt/Module.h"
#include "Opt/Algorithm.h"

#include "Common/Object.h"

#include "ModSerialIO.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

///////////////////////////////
// CLASS
//	Opt::SerialBuffer -- memory buffer unit for serialized data
//
// NOTES

class SerialBuffer
{
public:
	typedef boost::function<void(void*,
								 ModSize)> ReadCallback;
	typedef boost::function<void(const void*,
								 ModSize)> WriteCallback;

	SerialBuffer();
	~SerialBuffer();

	void* allocate(ModSize iSize_);

	int read(void* pBuffer_,
			 ModSize iSize_);
		
	int write(const void* pBuffer_,
			  ModSize iSize_);

	void reset();

	ModSize restSize() {return m_iSize - m_iPosition;}

protected:
private:
	
	void* top() {return syd_reinterpret_cast<char*>(m_pMemory) + m_iPosition;}
	void proceed(ModSize iSize_) {m_iPosition += iSize_;}

	void* m_pMemory;
	ModSize m_iSize;
	ModSize m_iPosition;
};

///////////////////////////////
// CLASS
//	Opt::SerialMemory --
//
// NOTES

class SerialMemory
	: public Common::Object,
	  public ModSerialIO
{
public:
	SerialMemory();
	~SerialMemory();

/////////////////////////
// ModSerialIO::
    virtual int 	readSerial(void* buffer_, ModSize byte_, DataType type_);
    virtual int 	writeSerial(const void* buffer_, ModSize byte_, DataType type_);
    virtual void 	resetSerial();



protected:
private:
	void addMemory();
	void destruct();


	VECTOR<SerialBuffer*> m_vecMemory;
	SIZE m_iPosition;
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_SERIALMEMORY_H

//
//	Copyright (c) 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
