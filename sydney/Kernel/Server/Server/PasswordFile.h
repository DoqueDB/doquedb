// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PasswordFile.h --
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_PASSWORDFILE_H
#define __SYDNEY_SERVER_PASSWORDFILE_H

#include "Server/Module.h"
#include "Server/UserList.h"

#include "Common/Object.h"

#include "Os/Path.h"

#include "ModAutoPointer.h"
#include "ModFile.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

//	CLASS
//	Server::PasswordFile -- read password entries from a file
//
//	NOTES
class PasswordFile : public ModFile
{
public:
	typedef ModFile Super;

	//	CLASS
	//	Server::PasswordFile::AutoRecoverer -- automatic error recover for password file
	//
	//	NOTES

	class AutoRecoverer : public Common::Object
	{
	public:
		// constructor
		AutoRecoverer(PasswordFile& cFile_);
		// destructor
		~AutoRecoverer();

		// escape current file
		void saveOld();
		// open file
		void open();
		// close file
		void close();
		// unlink old file
		void dropOld();
	private:
		// recover
		void recover();

		PasswordFile& m_cFile;
		Os::Path m_cSavePath;
		int m_iStatus;
	};

	//constructor
	PasswordFile(const ModUnicodeString& cPath_);
	//destructor
	~PasswordFile();

	// get userlist from file
	UserList* getUserList();

	// write one Entry to file
	void write(const ModUnicodeString& cstrUserName_,
			   const UserList::Entry& cEntry_);

	// check user name validity
	void checkUserName(const ModUnicodeString& cstrUserName_);

	// recover backup file if exist
	static void revertBackupFile(const ModUnicodeString& cPath_);

private:
	friend class AutoRecoverer;

	// open file
	void open(bool bReadMode_ = true);
	// close file
	void close();
	// read next block
	const char* read();
	// parse file image to get a password entry
	bool parse(UserList& cResult_);

	char* m_pBuffer;
	const char* m_pTop;
	const char* m_pTail;
	bool m_bIsEof;
};

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_PASSWORDFILE_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
