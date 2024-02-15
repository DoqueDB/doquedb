// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SerialMemory.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Opt";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Opt/SerialMemory.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Exception/Unexpected.h"

#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

namespace
{
	const ModSize _iUnitSize = 1 << 10; // 1K

	const ModSize _SizeTable[] = {16,32,64};
	const SIZE _SizeTableSize = sizeof(_SizeTable) / sizeof(_SizeTable[0]);

	ModSize _getSize(SIZE iPos_)
	{
		ModSize c = (iPos_ < _SizeTableSize)
			? _SizeTable[iPos_]
			: _SizeTable[_SizeTableSize - 1];
		return c * _iUnitSize;
	}
}

//////////////////////////////
// Opt::SerialBuffer::
//////////////////////////////

// FUNCTION public
//	Opt::SerialBuffer::SerialBuffer -- 
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

SerialBuffer::
SerialBuffer()
	: m_pMemory(0),
	  m_iSize(0),
	  m_iPosition(0)
{}

// FUNCTION public
//	Opt::SerialBuffer::~SerialBuffer -- 
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

SerialBuffer::
~SerialBuffer()
{
	try {
		Os::Memory::unmap(m_pMemory, m_iSize);
	} catch(...) {
		; // ignore
	}
}

// FUNCTION public
//	Opt::SerialBuffer::allocate -- 
//
// NOTES
//
// ARGUMENTS
//	ModSize iSize_
//	
// RETURN
//	void*
//
// EXCEPTIONS

void*
SerialBuffer::
allocate(ModSize iSize_)
{
	return m_pMemory = Os::Memory::map(m_iSize = iSize_, false);
}

// FUNCTION public
//	Opt::SerialBuffer::reset -- 
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
SerialBuffer::
reset()
{
	m_iPosition = 0;
}

// FUNCTION public
//	Opt::SerialBuffer::read -- 
//
// NOTES
//
// ARGUMENTS
//	void* pBuffer_
//	ModSize iSize_
//	ReadCallback callback_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

int
SerialBuffer::
read(void* pBuffer_,
	 ModSize iSize_)
{
	; _SYDNEY_ASSERT(m_pMemory);
	; _SYDNEY_ASSERT(pBuffer_);

	ModSize iRestSize = restSize();
	if (iRestSize >= iSize_) {
		Os::Memory::copy(pBuffer_, top(), iSize_);
		proceed(iSize_);
		return iSize_;
	} else {
		if (iRestSize > 0) {
			Os::Memory::copy(pBuffer_, top(), iRestSize);
			proceed(iRestSize);
			return iRestSize;
		}
	}
	return 0;
}

// FUNCTION public
//	Opt::SerialBuffer::write -- 
//
// NOTES
//
// ARGUMENTS
//	const void* pBuffer_
//	ModSize iSize_
//	WriteCallback callback_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

int
SerialBuffer::
write(const void* pBuffer_,
	  ModSize iSize_)
{
	; _SYDNEY_ASSERT(m_pMemory);
	; _SYDNEY_ASSERT(pBuffer_);

	ModSize iRestSize = restSize();
	if (iRestSize >= iSize_) {
		Os::Memory::copy(top(), pBuffer_, iSize_);
		proceed(iSize_);
		return iSize_;
	} else {
		if (iRestSize > 0) {
			Os::Memory::copy(top(), pBuffer_, iRestSize);
			proceed(iRestSize);
			return iRestSize;
		}

	}
	return 0;	
}

////////////////////////////////
// Opt::SerialMemory::
////////////////////////////////

// FUNCTION public
//	Opt::SerialMemory::SerialMemory -- 
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

SerialMemory::
SerialMemory()
	: m_vecMemory(),
	  m_iPosition(0)
{}

// FUNCTION public
//	Opt::SerialMemory::~SerialMemory -- 
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

SerialMemory::
~SerialMemory()
{
	try {
		destruct();
	} catch (...) {}
}

// FUNCTION public
//	Opt::SerialMemory::readSerial -- 
//
// NOTES
//
// ARGUMENTS
//	void* buffer_
//	ModSize byte_
//	DataType type_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
SerialMemory::
readSerial(void* buffer_, ModSize byte_, DataType type_)
{
	void* pos = buffer_;
	ModSize iRestSize = byte_;
	while (iRestSize > 0) {
		if (m_iPosition >= m_vecMemory.GETSIZE()) {
			// no more data
			return 0;
		}
		
		int iRead = m_vecMemory[m_iPosition]->read(pos, iRestSize);
		iRestSize -= iRead;
		pos = syd_reinterpret_cast<char*>(pos) + iRead;
		if (m_vecMemory[m_iPosition]->restSize() == 0) {
			m_iPosition++;
		}

	}
	return byte_;
}

// FUNCTION public
//	Opt::SerialMemory::writeSerial -- 
//
// NOTES
//
// ARGUMENTS
//	const void* buffer_
//	ModSize byte_
//	DataType type_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
SerialMemory::
writeSerial(const void* buffer_, ModSize byte_, DataType type_)
{
	const void* pos = buffer_;
	ModSize iRest = byte_;
	while (iRest > 0) {
		if (m_vecMemory.GETSIZE() == 0 ||
			m_vecMemory.GETBACK()->restSize() == 0) {
			addMemory();
		}
		
		int iWritten = m_vecMemory.GETBACK()->write(pos, iRest);
		iRest -= iWritten;
		pos = syd_reinterpret_cast<const char*>(pos) + iWritten;
	}
	
	return byte_;
}

// FUNCTION public
//	Opt::SerialMemory::resetSerial -- 
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

//virtual
void
SerialMemory::
resetSerial()
{
	SIZE n = m_vecMemory.GETSIZE();
	for (SIZE i = 0; i < n; ++i) {
		m_vecMemory[i]->reset();
	}	
	m_iPosition = 0;
}


// FUNCTION private
//	Opt::SerialMemory::addMemory -- 
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
SerialMemory::
addMemory()
{
	m_vecMemory.PUSHBACK(new SerialBuffer());
	m_vecMemory.GETBACK()->allocate(_getSize(m_vecMemory.GETSIZE() - 1));
}

// FUNCTION private
//	Opt::SerialMemory::destruct -- 
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
SerialMemory::
destruct()
{
	SIZE n = m_vecMemory.GETSIZE();
	for (SIZE i = 0; i < n; ++i) {
		delete m_vecMemory[i];
	}
	m_vecMemory.clear();
}

_SYDNEY_OPT_END
_SYDNEY_END

//
// Copyright (c) 2011, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
