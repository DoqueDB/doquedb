// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/BulkFile.h --
// 
// Copyright (c) 2006, 2007, 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_BULKFILE_H
#define __SYDNEY_EXECUTION_ACTION_BULKFILE_H

#include "Execution/Action/Module.h"

#include "ModCharString.h"
#include "ModFile.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

//	CLASS
//	BulkFile -- Read/write from File
//
//	NOTES

class BulkFile
	: public ModFile
{
public:
	typedef ModFile Super;

	// constructors
	BulkFile(const ModUnicodeString& cPath_, bool bIsRead_);
	~BulkFile();

	// read next block
	const char* read(const char* pRest_ = 0);
	// get top
	const char* getTop() {return m_pTop;}
	// get tail
	const char* getTail() {return m_pTail;}

	// write char string
	void write(const char* pData_, int iLength_);
	void write(const ModCharString& cstrValue_)
	{
		write(cstrValue_.getBuffer(), cstrValue_.getLength());
	}

	// put data from file (for error file)
	void putData(BulkFile& cFile_, ModFileOffset iTop_, ModFileOffset iTail_);

	// flush data
	void flush();

	// all data has read?
	bool isEndOfFile();

	// reset iteration
	void reset();

	// change path name
	void setPath(const ModUnicodeString& cPath_);
	// obtain full path name from the parent path of the file
	void getFullPath(const ModUnicodeString& cPath_, ModUnicodeString& cResult_);

	// open
	bool open(bool bThrow_ = true);
	//Super::
	//void close();

	// inilialize
	void initialize();
	// initialize buffer object
	void initializeBuffer();
	// terminating
	void terminate();

	// create parent directry
	void mkdir();

	// get current position of buffer top
	ModFileOffset getTopPosition();

protected:
private:
	// do not copy
	BulkFile(const BulkFile& cOther_);
	BulkFile& operator=(const BulkFile& cOther_);

	// set m_cParentPath value
	void setParentPath();

	void* m_pBuffer;					// buffer
	char* m_pTop;						// top of read data
	char* m_pTail;						// tail of read data
	int m_iBufferSize;					// buffer size
	bool m_bIsRead;						// read or write
	bool m_bIsEof;						// reached to end of file
	ModFileOffset m_iTailPosition;		// current buffer tail position
	ModUnicodeString m_cParentPath;		// parent path name
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_BULKFILE_H

//
//	Copyright (c) 2006, 2007, 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
