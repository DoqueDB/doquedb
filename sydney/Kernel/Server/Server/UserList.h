// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UserList.h --
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

#ifndef __SYDNEY_SERVER_USERLIST_H
#define __SYDNEY_SERVER_USERLIST_H

#include "Server/Module.h"

#include "Common/SafeExecutableObject.h"
#include "Common/MD5.h"
#include "Common/ObjectPointer.h"
#include "Common/VectorMap.h"

#include "Os/RWLock.h"

#include "ModAlgorithm.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

class PasswordFile;

//	CLASS
//	Server::UserList -- manages user list
//
//	NOTES
class UserList : public Common::Object
{
public:
	
	//	CLASS
	//	Server::UserList::Entry -- hold a user information
	//
	//	NOTES
	class Entry : public Common::SafeExecutableObject
	{
	public:
		//	TYPEDEF
		//	Server::UserList::Entry::Pointer -- objectpointer for entry
		//
		//	NOTES
		typedef Common::ObjectPointer<Entry> Pointer;

		struct Authorization
		{
			//	ENUM
			//	Server::UserList::Entry::Authorization::Value -- authorization category 
			//
			//	NOTES
			//	For now, it is fixed to Super user or DB User.
			enum Value
			{
				SuperUser = 0,
				DBUser,
				OSUser,
				Invalid = 9999,
				ValueNum
			};
		};

		//	CLASS
		//	Server::UserList::Entry::Compare -- comparison class for user name
		//
		//	NOTES
		class Compare
		{
		public:
			Compare() {}
			bool operator()(const ModUnicodeString& cstrValue1_,
							const ModUnicodeString& cstrValue2_) const
			{
				return cstrValue1_.compare(cstrValue2_, ModFalse /* case insensitive */) < 0;
			}
		};

		//	TYPEDEF
		//	Server::UserList::Entry::ID -- user ID
		//
		//	NOTES
		typedef int ID;

		// constructor
		Entry(const Common::MD5::Value& cPassword_, ID iID_, Authorization::Value eCategory_ = Authorization::DBUser)
			: SafeExecutableObject(),
			  m_cPassword(cPassword_), m_iID(iID_), m_eCategory(eCategory_)
		{}

		// destructor
		~Entry() {}

		// check password
		bool check(const ModUnicodeString& cstrPassword_);

		// change password
		void setPassword(const ModUnicodeString& cPassword_);
		void setPassword(const Common::MD5::Value& cValue_);

		// get password
		const Common::MD5::Value& getPassword() const {return m_cPassword;}
		// get user ID
		ID getID() const {return m_iID;}
		// get authorization category
		Authorization::Value getCategory() const {return m_eCategory;}

		// is superuser?
		bool isSuperUser() const {return m_eCategory == Authorization::SuperUser;}
		// is invalid user?
		bool isInvalid() const {return m_eCategory == Authorization::Invalid;}

	private:
		Common::MD5::Value m_cPassword;
		ID m_iID;
		Authorization::Value m_eCategory;
	};

	//constructor
	UserList();
	//destructor
	~UserList();

	// add an entry
	void add(const ModUnicodeString& cstrName_, const Entry::Pointer& pEntry_,
			 bool bNoCheck_ = false);
	// get an entry
	bool get(const ModUnicodeString& cstrName_, Entry::Pointer& pEntry_);
	// get an entry
	bool getNext(Entry::ID iID_, ModUnicodeString& cstrName_, Entry::Pointer& pEntry_);

	// add user and persist
	void addUser(PasswordFile& cFile_,
				 const ModUnicodeString& cstrUserName_,
				 const ModUnicodeString& cstrPassword_,
				 int iUserID_);
	// delete user and persist
	void deleteUser(PasswordFile& cFile_, const ModUnicodeString& cstrUserName_);
	// change password
	void changePassword(PasswordFile& cFile_,
						const ModUnicodeString& cstrUserName_, const ModUnicodeString& cstrPassword_);

	// get lock object
	Os::RWLock& getLock() {return m_cLock;}

private:
	// persist list to file
	void persist(PasswordFile& cFile_);
	// erase an entry
	void erase(const ModUnicodeString& cstrName_, Entry::ID iID_);

	typedef Common::VectorMap<ModUnicodeString, Entry::Pointer, Entry::Compare> EntryMap;
	typedef Common::VectorMap<Entry::ID, ModUnicodeString, ModLess<Entry::ID> > IDMap;
	EntryMap m_mapEntry;
	IDMap m_mapID;

	Os::RWLock m_cLock;
};

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_USERLIST_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
