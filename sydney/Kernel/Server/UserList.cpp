// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UserList.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Server";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Server/UserList.h"
#include "Server/PasswordFile.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Communication/User.h"

#include "Exception/BadPasswordFile.h"
#include "Exception/DuplicateUser.h"
#include "Exception/DuplicateUserID.h"
#include "Exception/UserNotFound.h"
#include "Exception/UserIDOutOfRange.h"

#include "Os/AutoRWLock.h"
#include "Os/Limits.h"

_SYDNEY_USING
_SYDNEY_SERVER_USING

/////////////////////////////////////////////////////////
// UserList::Entry
/////////////////////////////////////////////////////////

// FUNCTION public
//	Server::UserList::Entry::check -- check password
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrPassword_
//	
// RETURN
//	bool	true if password is correct
//
// EXCEPTIONS

bool
UserList::Entry::
check(const ModUnicodeString& cstrPassword_)
{
	// if it is invalid user, it always fails
	if (isInvalid()) {
		return false;
	}

	// convert into char string
	// [NOTES] getString requires non-const ModUnicodeString

	ModUnicodeString tmp(cstrPassword_);
	const unsigned char* pTop =
		syd_reinterpret_cast<const unsigned char*>(tmp.getString(ModKanjiCode::utf8));
	ModSize iSize = tmp.getStringBufferSize() - 1; // eliminate null-terminate character

	// verify
	return Common::MD5::verify(pTop, pTop + iSize, m_cPassword);
}

// FUNCTION public
//	Server::UserList::Entry::setPassword -- change password
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrPassword_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UserList::Entry::
setPassword(const ModUnicodeString& cstrPassword_)
{
	// convert password into char string
	// [NOTES] getString requires non-const ModUnicodeString

	ModUnicodeString tmp(cstrPassword_);
	const unsigned char* pTop =
		syd_reinterpret_cast<const unsigned char*>(tmp.getString(ModKanjiCode::utf8));
	ModSize iSize = tmp.getStringBufferSize() - 1; // eliminate null-terminate character

	// get MD5 representation
	Common::MD5::generate(pTop, pTop + iSize, m_cPassword);
}

// FUNCTION public
//	Server::UserList::Entry::setPassword -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::MD5::Value& cValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UserList::Entry::
setPassword(const Common::MD5::Value& cValue_)
{
	m_cPassword = cValue_;
}

/////////////////////////////////////////////////////////
// UserList
/////////////////////////////////////////////////////////

// FUNCTION public
//	Server::UserList::UserList -- constructor
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

UserList::
UserList()
	: m_mapEntry(), m_mapID()
{}

// FUNCTION public
//	Server::UserList::~UserList -- destructor
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

UserList::
~UserList()
{
	try {
		m_mapEntry.clear();
		m_mapID.clear();
	} catch (...) {
		/* ignore */
	}
}

// FUNCTION public
//	Server::UserList::add -- add an entry
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrName_
//	const Entry::Pointer& pEntry_
//	bool bNoCheck_ = false
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UserList::
add(const ModUnicodeString& cstrName_, const Entry::Pointer& pEntry_,
	bool bNoCheck_ /* = false */)
{
	Os::AutoRWLock l(m_cLock, Os::RWLock::Mode::Write);
	if (!bNoCheck_ && m_mapID.find(pEntry_->getID()) != m_mapID.end()) {
		// same ID already defined
		SydInfoMessage << "Duplicate user ID: " << pEntry_->getID() << ModEndl;
		_SYDNEY_THROW0(Exception::BadPasswordFile);
	}
	m_mapEntry.insert(cstrName_, pEntry_);
	m_mapID.insert(pEntry_->getID(), cstrName_);
}

// FUNCTION public
//	Server::UserList::get -- get an entry
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrName_
//	Entry::Pointer& pEntry_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
UserList::
get(const ModUnicodeString& cstrName_, Entry::Pointer& pEntry_)
{
	Os::AutoRWLock l(m_cLock, Os::RWLock::Mode::Read);
	EntryMap::Iterator iterator = m_mapEntry.find(cstrName_);
	if (iterator != m_mapEntry.end()) {
		pEntry_ = (*iterator).second;
		return true;
	}
	return false;
}

// FUNCTION public
//	Server::UserList::getNext -- get an entry
//
// NOTES
//	used for enumerate all the users in order of ID
//
// ARGUMENTS
//	Entry::ID iID_
//	ModUnicodeString& cstrName_
//	Entry::Pointer& pEntry_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
UserList::
getNext(Entry::ID iID_, ModUnicodeString& cstrName_, Entry::Pointer& pEntry_)
{
	// RWLock should be locked by the caller
#ifdef DEBUG
	// write lock should fail
	if (m_cLock.trylock(Os::RWLock::Mode::Write)) {
		m_cLock.unlock(Os::RWLock::Mode::Write);
		; _SYDNEY_ASSERT(false);
	}
#endif

	// get iterator which points the smallest value
	//    among such values that are greater or equal to iID
	IDMap::Compare comp;
	IDMap::Iterator iterator = ModLowerBound(m_mapID.begin(),
											 m_mapID.end(),
											 iID_,
											 comp);
	if (iterator != m_mapID.end()) {
		cstrName_ = (*iterator).second;
		EntryMap::Iterator i = m_mapEntry.find(cstrName_);
		; _SYDNEY_ASSERT(i != m_mapEntry.end());
		pEntry_ = (*i).second;
		return true;
	}
	return false;
}

// FUNCTION public
//	Server::UserList::addUser -- add user and persist
//
// NOTES
//
// ARGUMENTS
//	PasswordFile& cFile_
//	const ModUnicodeString& cstrUserName_
//	const ModUnicodeString& cstrPassword_
//	int iUserID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UserList::
addUser(PasswordFile& cFile_,
		const ModUnicodeString& cstrUserName_,
		const ModUnicodeString& cstrPassword_,
		int iUserID_)
{
	Os::AutoRWLock l(m_cLock, Os::RWLock::Mode::Write);

	// check user name validity
	cFile_.checkUserName(cstrUserName_);

	// check whether same name is already appeared
	if (m_mapEntry.find(cstrUserName_) != m_mapEntry.end()) {
		SydInfoMessage << "Duplicate user name: " << cstrUserName_ << ModEndl;
		_SYDNEY_THROW1(Exception::DuplicateUser, cstrUserName_);
	}

	Entry::ID iID;
	if (iUserID_ == Communication::User::ID::Auto) {
		// get new ID
		iID = Communication::User::ID::Min;
		if (m_mapID.getSize() > 0) {
			iID = (*(m_mapID.begin() + (m_mapID.getSize() - 1))).first;
			if (iID >= Communication::User::ID::Max) {
				// ID reaches maximum value
				_SYDNEY_THROW0(Exception::UserIDOutOfRange);
			}
			++iID;
		}
	} else {
		// use specified ID
		if (iUserID_ < Communication::User::ID::Min
			|| iUserID_ > Communication::User::ID::Max) {
			SydInfoMessage << "Bad userID specification: " << iUserID_ << ModEndl;
			_SYDNEY_THROW0(Exception::UserIDOutOfRange);
		}
		if (m_mapID.find(iUserID_) != m_mapID.end()) {
			// same ID already defined
			SydInfoMessage << "User ID: " << iUserID_ << " is already used." << ModEndl;
			_SYDNEY_THROW1(Exception::DuplicateUserID, iUserID_);
		}
		iID = iUserID_;
	}

	Entry::Pointer pEntry = new Entry(Common::MD5::Value(), iID, Entry::Authorization::DBUser);

	// set password
	pEntry->setPassword(cstrPassword_);

	// add to list
	add(cstrUserName_, pEntry);

	try {
		// persist to password file
		persist(cFile_);
	} catch (...) {
		erase(cstrUserName_, iID);
		_SYDNEY_RETHROW;
	}
}

// FUNCTION public
//	Server::UserList::deleteUser -- delete user and persist
//
// NOTES
//
// ARGUMENTS
//	PasswordFile& cFile_
//	const ModUnicodeString& cstrUserName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UserList::
deleteUser(PasswordFile& cFile_, const ModUnicodeString& cstrUserName_)
{
	Os::AutoRWLock l(m_cLock, Os::RWLock::Mode::Write);

	// find entry
	EntryMap::Iterator iterator = m_mapEntry.find(cstrUserName_);

	if (iterator == m_mapEntry.end()) {
		SydInfoMessage << "No user: " << cstrUserName_ << ModEndl;
		_SYDNEY_THROW1(Exception::UserNotFound, cstrUserName_);
	}

	// get ID
	Entry::Pointer pEntry = (*iterator).second;
	Entry::ID iID = pEntry->getID();

	// delete the entry from map
	erase(cstrUserName_, iID);

	try {
		// persist to password file
		persist(cFile_);
	} catch (...) {
		add(cstrUserName_, pEntry, true /* no check */);
		_SYDNEY_RETHROW;
	}
}

// FUNCTION public
//	Server::UserList::changePassword -- change password
//
// NOTES
//
// ARGUMENTS
//	PasswordFile& cFile_
//	const ModUnicodeString& cstrUserName_
//	const ModUnicodeString& cstrPassword_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UserList::
changePassword(PasswordFile& cFile_,
			   const ModUnicodeString& cstrUserName_, const ModUnicodeString& cstrPassword_)
{
	Os::AutoRWLock l(m_cLock, Os::RWLock::Mode::Write);

	// find entry
	EntryMap::Iterator iterator = m_mapEntry.find(cstrUserName_);

	if (iterator == m_mapEntry.end()) {
		SydInfoMessage << "No user: " << cstrUserName_ << ModEndl;
		_SYDNEY_THROW1(Exception::UserNotFound, cstrUserName_);
	}

	// modify password
	Entry::Pointer pEntry = (*iterator).second;
	Common::MD5::Value cValue = pEntry->getPassword(); // save old value
	pEntry->setPassword(cstrPassword_);

	try {
		// persist to password file
		persist(cFile_);
	} catch (...) {
		// revert old value
		pEntry->setPassword(cValue);
		_SYDNEY_RETHROW;
	}
}

// FUNCTION private
//	Server::UserList::persist -- persist list to file
//
// NOTES
//
// ARGUMENTS
//	PasswordFile& cFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UserList::
persist(PasswordFile& cFile_)
{
	PasswordFile::AutoRecoverer cRecoverer(cFile_);

	// escape current file to other place
	cRecoverer.saveOld();
	// open file
	cRecoverer.open();
	// write entry one by one (in ID order)
	IDMap::Iterator iterator = m_mapID.begin();
	const IDMap::Iterator last = m_mapID.end();
	for (; iterator != last; ++iterator) {
		const ModUnicodeString& cName = (*iterator).second;
		EntryMap::Iterator i = m_mapEntry.find(cName);
		; _SYDNEY_ASSERT(i != m_mapEntry.end());
		cFile_.write(cName, *((*i).second));
	}
	// close file
	cRecoverer.close();
	// unlink old file
	cRecoverer.dropOld();
}

// FUNCTION private
//	Server::UserList::erase -- erase an entry
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrName_
//	Entry::ID iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UserList::
erase(const ModUnicodeString& cstrName_, Entry::ID iID_)
{
	m_mapEntry.erase(cstrName_);
	m_mapID.erase(iID_);
}

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
