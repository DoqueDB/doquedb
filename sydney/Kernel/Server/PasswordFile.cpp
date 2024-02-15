// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PasswordFile.cpp --
// 
// Copyright (c) 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#include "Server/PasswordFile.h"
#include "Server/UserList.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/StringData.h"

#include "Exception/BadPasswordFile.h"
#include "Exception/FileNotFound.h"
#include "Exception/InvalidUserName.h"
#include "Exception/ModLibraryError.h"
#include "Exception/PermissionDenied.h"
#include "Exception/TooLongUserName.h"
#include "Exception/UserRequired.h"

#include "Os/File.h"
#include "Os/Memory.h"

#include "ModAutoPointer.h"
#include "ModCharString.h"
#include "ModPair.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_SERVER_USING

namespace
{
	// CONSTANT
	//	_iBufferSize -- buffer size (4K)
	//
	// NOTES
	const int _iBufferSize = 4096;

	// CONSTANT
	//	_csSeparator -- separator
	//
	// NOTES
	const char _csSeparator = ':';

	// CONSTANT
	//	_cTerminator -- terminator
	//
	// NOTES
	const ModCharString _cTerminator("\n");

	//	CONSTANT
	//	$$$::_iMaxUserNameLength -- maximum length for user name
	//
	//	NOTES

	int _iMaxUserNameLength = 16;

	namespace _Character
	{
		// Character categories
		enum {
			Unused = 0,
			UserName,
			NameAndPassword,
			IDAndNameAndPassword,
			FieldSeparator,
			MemberSeparator,
			RecordTerminator,
			ValueNum
		} _Category[0x80] =
		{
#define x Unused
#define U UserName
#define P NameAndPassword
#define I IDAndNameAndPassword
#define F FieldSeparator
#define M MemberSeparator
#define T RecordTerminator
			//00-0F
			x,x,x,x,x,x,x,x,x,x,T,x,x,T,x,x,
			//10-1F
			x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,
			//20-2F
			x,U,x,U,U,U,U,U,U,U,U,U,M,x,x,x,
			//30-3F
			I,I,I,I,I,I,I,I,I,I,F,U,U,U,U,U,
			//40-4F
			U,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
			//50-5F
			P,P,P,P,P,P,P,P,P,P,P,U,U,U,U,U,
			//60-6F
			U,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
			//70-7F
			P,P,P,P,P,P,P,P,P,P,P,U,U,U,U,x
#undef x
#undef U
#undef P
#undef I
#undef F
#undef M
#undef T
		};
	} // namespace _Character

	namespace _Parse
	{
		// pasing status
		enum _Status {
			Novice = 0,
			UserName,
			Separator1,
			Password,
			BadPassword,
			Separator2,
			ID,
			Separator3,
			Category,
			Separator4,
			Member,
			MemberSeparator,
			Terminator,
			Error,
			ValueNum
		} _AutoMaton[ValueNum][_Character::ValueNum] =
		{
#define o Novice
#define U UserName
#define S1 Separator1
#define P Password
#define BP BadPassword
#define S2 Separator2
#define I ID
#define S3 Separator3
#define C Category
#define S4 Separator4
#define M Member
#define MS MemberSeparator
#define T Terminator
#define e Error
			//x  U  P  I  F  M  T
			{ e, U, U, U, e, e, o }, //Novice
			{ e, U, U, U,S1, e, e }, //UserName
			{ e, e, P, P,S2, e, e }, //Separator1
			{ e,BP, P, P,S2, e, e }, //Password
			{ e,BP,BP,BP,S2, e, e }, //BadPassword
			{ e, e, e, I, e, e, e }, //Separator2
			{ e, e, e, I,S3, e, e }, //ID
			{ e, e, e, C,S4, e, e }, //Separator3
			{ e, e, e, C,S4, e, e }, //Category
			{ e, M, M, M, e, e, T }, //Separator4
			{ e, M, M, M, e,MS, T }, //Member
			{ e, M, M, M, e, e, e }, //MemberSeparator
			{ e, e, e, e, e, e, e }, //Terminator
			{ e, e, e, e, e, e, e }  //Error
#undef o
#undef U
#undef S1
#undef P
#undef BP
#undef S2
#undef I
#undef S3
#undef C
#undef S4
#undef M
#undef MS
#undef T
#undef e
		};

		// FUNCTION local
		//	$$$::_Parse::oneEntry -- parse one entry
		//
		// NOTES
		//
		// ARGUMENTS
		//	const char* pTop_
		//	const char* pTail_
		//	const char*& pResult_
		//	ModPair<const char*, const char*>& cName_
		//	ModPair<const char*, const char*>& cPassword_
		//	ModPair<const char*, const char*>& cID_
		//	ModPair<const char*, const char*>& cCategory_
		//	
		// RETURN
		//	bool	false if insufficent record
		//
		// EXCEPTIONS

		bool oneEntry(const char* pTop_, const char* pTail_,
					  const char*& pResult_,
					  ModPair<const char*, const char*>& cName_,
					  ModPair<const char*, const char*>& cPassword_,
					  ModPair<const char*, const char*>& cID_,
					  ModPair<const char*, const char*>& cCategory_)
		{
			_Status eStatus = Novice;
			cName_.first = cName_.second = pTop_;
			cPassword_.first = cPassword_.second = 0;
			cID_.first = cID_.second = 0;
			cCategory_.first = cCategory_.second = 0;

			bool bBadPassword = false;
			for (const char* p = pTop_; p < pTail_; ++p) {
				if ((*p & 0x80) == 0) {
					switch (eStatus = _AutoMaton[eStatus][_Character::_Category[*p]]) {
					case Novice:
						{
							cName_.second = ++(cName_.first);
							break;
						}
					case UserName:
						{
							++(cName_.second);
							break;
						}
					case Separator1:
						{
							cPassword_.first = cPassword_.second = p + 1;
							break;
						}
					case Password:
						{
							++(cPassword_.second);
							break;
						}
					case BadPassword:
						{
							bBadPassword = true;
							++(cPassword_.second);
							break;
						}
					case Separator2:
						{
							cID_.first = cID_.second = p + 1;
							break;
						}
					case ID:
						{
							++(cID_.second);
							break;
						}
					case Separator3:
						{
							if (!bBadPassword) {
								cCategory_.first = cCategory_.second = p + 1;
							}
							break;
						}
					case Category:
						{
							if (!bBadPassword) {
								++(cCategory_.second);
							}
							break;
						}
					case Separator4:
					case Member:
					case MemberSeparator:
						{
							// do nothing
							break;
						}
					case Terminator:
						{
							// end of one entry
							pResult_ = p;
							return true;
						}
					case Error:
					default:
						{
							// error
							SydInfoMessage << "Bad character(code=" << static_cast<int>(*p)
										   << ") in password file" << ModEndl;
							_SYDNEY_THROW0(Exception::BadPasswordFile);
						}
					}
				}
			}
			// insufficient contents -> set beginning point of name as result
			pResult_ = cName_.first;
			return false;
		}
	} // namespase _Parse

	namespace _Recover
	{
		//	ENUM
		//	$$$::_Recover::Status -- status in persisting to password file
		//
		//	NOTES
		enum Status
		{
			Initial,
			Saved,
			Opened,
			Closed,
			Dropped
		};

		//	CONSTANT
		//	$$$::_Recover::_cstrSavePostfix -- postfix attached to saved file
		//
		//	NOTES
		const ModUnicodeString _cstrSavePostfix("_BAK");

	} // namespace _Recover
} // namespace

/////////////////////////////////////////////
// Server::PasswordFile::AutoRecoverer
/////////////////////////////////////////////

// FUNCTION public
//	Server::PasswordFile::AutoRecoverer::AutoRecoverer -- constructor
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

PasswordFile::AutoRecoverer::
AutoRecoverer(PasswordFile& cFile_)
	: m_cFile(cFile_), m_cSavePath(), m_iStatus(_Recover::Initial)
{}

// FUNCTION public
//	Server::PasswordFile::AutoRecoverer::~AutoRecoverer -- destructor
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

PasswordFile::AutoRecoverer::
~AutoRecoverer()
{
	try {
		recover();
	} catch (...) {
		// ignore exceptions
		;
	}
}

// FUNCTION public
//	Server::PasswordFile::AutoRecoverer::saveOld -- escape current file
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
PasswordFile::AutoRecoverer::
saveOld()
{
	// move password file to BAK file
	m_cSavePath = m_cFile.getFullPathNameW();
	m_cSavePath.append(_Recover::_cstrSavePostfix);

	Os::File::rename(static_cast<const Os::Path&>(m_cFile.getFullPathNameW()),
					 m_cSavePath);
	m_iStatus = _Recover::Saved;
}

// FUNCTION public
//	Server::PasswordFile::AutoRecoverer::open -- open file
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
PasswordFile::AutoRecoverer::
open()
{
	m_cFile.open(false /* write mode */);
	m_iStatus = _Recover::Opened;
}

// FUNCTION public
//	Server::PasswordFile::AutoRecoverer::close -- close file
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
PasswordFile::AutoRecoverer::
close()
{
	m_cFile.close();
	m_iStatus = _Recover::Closed;
}

// FUNCTION public
//	Server::PasswordFile::AutoRecoverer::dropOld -- unlink old file
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
PasswordFile::AutoRecoverer::
dropOld()
{
	; _SYDNEY_ASSERT(m_cSavePath.getLength() > 0);

	// remove saved file
	Os::File::remove(m_cSavePath);
	m_iStatus = _Recover::Dropped;
}

// FUNCTION public
//	Server::PasswordFile::AutoRecoverer::recover -- recover
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
PasswordFile::AutoRecoverer::
recover()
{
	switch (m_iStatus) {
	case _Recover::Initial:
	case _Recover::Dropped:
	default:
		{
			// do nothing
			break;
		}
	case _Recover::Opened:
		{
			// close file
			m_cFile.close();
			// thru.
		}
	case _Recover::Closed:
	case _Recover::Saved:
		{
			// move back
			Os::File::rename(m_cSavePath,
							 static_cast<const Os::Path&>(m_cFile.getFullPathNameW()),
							 true /* force */);
			break;
		}
	}
}

/////////////////////////////////////////////
// Server::PasswordFile
/////////////////////////////////////////////

// FUNCTION public
//	Server::PasswordFile::PasswordFile -- constructor
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cPath_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

PasswordFile::
PasswordFile(const ModUnicodeString& cPath_)
	: Super(cPath_),
	  m_pBuffer(0), m_pTop(0), m_pTail(0), m_bIsEof(false)
{}

// FUNCTION public
//	Server::PasswordFile::~PasswordFile -- destructor
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

PasswordFile::
~PasswordFile()
{
	try {
		close();
	} catch (...) {
		// ignore exceptions
		;
	}
}

// FUNCTION public
//	Server::PasswordFile::getUserList -- get userlist from file
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	UserList*
//
// EXCEPTIONS

UserList*
PasswordFile::
getUserList()
{
	ModAutoPointer<UserList> pResult = new UserList;
	open();
	while (parse(*pResult)) {}
	close();

	return pResult.release();
}

// FUNCTION public
//	Server::PasswordFile::write -- write one Entry to file
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrUserName_
//	const UserList::Entry& cEntry_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
PasswordFile::
write(const ModUnicodeString& cstrUserName_,
	  const UserList::Entry& cEntry_)
{
	; _SYDNEY_ASSERT(isOpened());
	; _SYDNEY_ASSERT(m_pBuffer);

	// write user name
	char* p = m_pBuffer;

	const ModUnicodeChar* pName = static_cast<const ModUnicodeChar*>(cstrUserName_);
	const ModUnicodeChar* pNameTail = cstrUserName_.getTail();
	for (; pName < pNameTail; ++pName, ++p) {
		*p = static_cast<char>(*pName);
	}
	// write separator
	*p++ = _csSeparator;

	// write password
	if (cEntry_.isInvalid()) {
		// write dummy password for invalid user
		*p++ = 'N';
		*p++ = 'P';
	} else {
		p = cEntry_.getPassword().write(p);
	}
	// write separator
	*p++ = _csSeparator;

	// write ID
	ModCharString cID;
	cID.format("%d", cEntry_.getID());
	Os::Memory::copy(p, cID.getBuffer(), cID.getLength());
	p += cID.getLength();
	// write separator
	*p++ = _csSeparator;

	// category
	if (cEntry_.isInvalid()) {
		// write dummy category for invalid user
		*p++ = '1';
	} else {
		ModCharString cCategory;
		cCategory.format("%d", static_cast<int>(cEntry_.getCategory()));
		Os::Memory::copy(p, cCategory.getBuffer(), cCategory.getLength());
		p += cCategory.getLength();
	}
	// write separator
	*p++ = _csSeparator;

	// members -> not used yet

	// write terminator
	Os::Memory::copy(p, _cTerminator.getBuffer(), _cTerminator.getLength());
	p += _cTerminator.getLength();

	// write to file
	Super::write(syd_reinterpret_cast<const void*>(m_pBuffer),
				 static_cast<ModSize>(p - m_pBuffer));
}

// FUNCTION public
//	Server::PasswordFile::checkUserName -- check user name validity
//
// NOTES
//	This method is called before adding a user
//
// ARGUMENTS
//	const ModUnicodeString& cstrUserName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
PasswordFile::
checkUserName(const ModUnicodeString& cstrUserName_)
{
	if (cstrUserName_.getLength() == 0) {
		SydInfoMessage << "Empty user name" << ModEndl;
		_SYDNEY_THROW0(Exception::UserRequired);
	}
	if (static_cast<int>(cstrUserName_.getLength()) > _iMaxUserNameLength) {
		SydInfoMessage << "Too long user name specified: " << cstrUserName_ << ModEndl;
		_SYDNEY_THROW0(Exception::TooLongUserName);
	}
	const ModUnicodeChar* p = static_cast<const ModUnicodeChar*>(cstrUserName_);
	const ModUnicodeChar* pTail = cstrUserName_.getTail();
	for (; p < pTail; ++p) {
		if (*p < 0x80) {
			switch (_Character::_Category[*p]) {
			case _Character::UserName:
			case _Character::NameAndPassword:
			case _Character::IDAndNameAndPassword:
				{
					continue;
				}
			default:
				{
					break;
				}
			}
		}
		// reaching here means invalid character
		SydInfoMessage << "Invalid character (" << *p << ") in user name." << ModEndl;
		_SYDNEY_THROW0(Exception::InvalidUserName);
	}
}

// FUNCTION public
//	Server::PasswordFile::revertBackupFile -- recover backup file if exist
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cPath_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
PasswordFile::
revertBackupFile(const ModUnicodeString& cPath_)
{
	// create fullpath name
	ModUnicodeString cstrFullPath;
	ModOsDriver::File::getFullPathName(cPath_, cstrFullPath);

	// create backup file name
	Os::Path cSavePath(cstrFullPath);
	cSavePath.append(_Recover::_cstrSavePostfix);

	// Check whether backup file name is accessible
	if (Os::File::access(cSavePath, Os::File::AccessMode::File)) {
		// when backup file exists, it means changing password file was failed
		// -> revert backup file
		Os::File::rename(cSavePath,
						 static_cast<const Os::Path&>(cstrFullPath),
						 true /* force */);
	}
}

// FUNCTION private
//	Server::PasswordFile::open -- open file
//
// NOTES
//
// ARGUMENTS
//	bool bReadMode_ = true
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
PasswordFile::
open(bool bReadMode_ /* = true */)
{
	if (isOpened()) {
		return;
	}

	try {
		// call superclass's implementation
		Super::open(bReadMode_ ? readMode : writeMode,
					bReadMode_ ? 0 : (writeThroughFlag
									  | createFlag
									  | exclusiveFlag));

	} catch (ModException& e) {

		SydInfoMessage << "Password file("
					   << getFullPathNameW()
					   << "): "
					   << Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)
					   << ModEndl;

		switch (e.getErrorNumber()) {
		case ModOsErrorFileNotFound:
			{
				_SYDNEY_THROW1(Exception::FileNotFound, getFullPathNameW());
			}
		case ModOsErrorPermissionDenied:
			{
				_SYDNEY_THROW1(Exception::PermissionDenied, getFullPathNameW());
			}
		default:
			{
				break;
			}
		}
		_SYDNEY_THROW1(Exception::ModLibraryError, e);
	}

	try {
		// initialize buffer
		if (m_pBuffer == 0) {
			// create buffer object
			m_pBuffer = syd_reinterpret_cast<char*>(Os::Memory::allocate(_iBufferSize));
			Os::Memory::reset(m_pBuffer, _iBufferSize);
		}
		m_pTop = syd_reinterpret_cast<const char*>(m_pBuffer);
		m_pTail = m_pTop;
	} catch (...) {
		close();
		throw;
	}
}

// FUNCTION private
//	Server::PasswordFile::close -- close file
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
PasswordFile::
close()
{
	if (isOpened()) {
		Super::close();
	}
	if (m_pBuffer) {
		void* p = m_pBuffer;
		Os::Memory::free(p);
		m_pBuffer = 0;
		m_pTop = m_pTail = 0;
	}
}

// FUNCTION private
//	Server::PasswordFile::read -- read next block
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
PasswordFile::
read()
{
	; _SYDNEY_ASSERT(isOpened());
	; _SYDNEY_ASSERT(m_pTop);

	if (m_bIsEof || (m_pTail - m_pTop > _iBufferSize / 2)) {
		// if no more data or unprocessed data is larger than half of buffer, do nothing
		return m_pTop;
	}

	char* pTarget = m_pBuffer;
	int iSize = _iBufferSize;

	if (m_pTop != m_pTail) {
		// If unprocessed data is there, move it to top
		int iRestSize = static_cast<int>(m_pTail - m_pTop);
		Os::Memory::move(m_pBuffer, m_pTop, iRestSize);
		m_pTop = syd_reinterpret_cast<const char*>(m_pBuffer);
		iSize -= iRestSize;
		pTarget += iRestSize;
	}
	// read to fill the buffer
	int iReadSize = Super::read(pTarget, iSize);
	m_pTail = pTarget + iReadSize;

	if (iReadSize < iSize) {
		// reached to the end of file
		m_bIsEof = true;
	}
	return m_pTop;
}

// FUNCTION private
//	Server::PasswordFile::parse -- parse file image to get a password entry
//
// NOTES
//
// ARGUMENTS
//	UserList& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
PasswordFile::
parse(UserList& cResult_)
{
	// scan file buffer to obtain name and password parts
	ModPair<const char*, const char*> cNamePart;
	ModPair<const char*, const char*> cPasswordPart;
	ModPair<const char*, const char*> cIDPart;
	ModPair<const char*, const char*> cCategoryPart;

 retry:
	{
		// read from file
		read();

		if (m_pTop == m_pTail) return false; // end of parsing

		const char* pResult = 0;
		if (!_Parse::oneEntry(m_pTop, m_pTail, pResult, cNamePart, cPasswordPart, cIDPart, cCategoryPart)) {
			// one record has not been finished in a buffer
			if (m_pTop == pResult) {
				// if no heading ignorable characters, it can't be parsed
				// [NOTES]
				// As the result, too many members in a group will be failed.
				SydErrorMessage << "Invalid entry for password file." << ModEndl;
				_SYDNEY_THROW0(Exception::BadPasswordFile);
			}
			m_pTop = pResult;
			// retry
			goto retry;
		}
		m_pTop = pResult;
	}

	// create new entry

	// ID
	int iID = 0;
	if (!Common::StringData::getInteger(iID, cIDPart.first, cIDPart.second)) {
		// ID part can't be converted to integer
		SydErrorMessage << "ID part don't form valid value("
						<< ModCharString(cIDPart.first,
										 static_cast<ModSize>(
											 cIDPart.second - cIDPart.first))
						<< ")." << ModEndl;
		_SYDNEY_THROW0(Exception::BadPasswordFile);
	}

	// Category
	UserList::Entry::Authorization::Value eCategory = UserList::Entry::Authorization::Invalid;
	// password
	Common::MD5::Value cMD5;
	if (cCategoryPart.first != 0) {
		// if cCategoryPart is not set, it means password was bad
		int iTmp = 0;
		if (!Common::StringData::getInteger(iTmp, cCategoryPart.first, cCategoryPart.second)) {
			// Category part can't be converted to integer
			SydErrorMessage << "Category part don't form valid value("
							<< ModCharString(cCategoryPart.first,
											 static_cast<ModSize>(cCategoryPart.second - cCategoryPart.first))
							<< ")." << ModEndl;
			_SYDNEY_THROW0(Exception::BadPasswordFile);
		}
		if (iTmp < 0 || iTmp >= UserList::Entry::Authorization::ValueNum) {
			// Category value is not supported
			SydErrorMessage << "Category value is invalid(" << iTmp << ModEndl;
			_SYDNEY_THROW0(Exception::BadPasswordFile);
		}
		eCategory = static_cast<UserList::Entry::Authorization::Value>(iTmp);

		if (cPasswordPart.first == cPasswordPart.second) {
			// empty passwod part -> generate MD5 value from empty string
			Common::MD5::generate(syd_reinterpret_cast<const unsigned char*>(cPasswordPart.first),
								  syd_reinterpret_cast<const unsigned char*>(cPasswordPart.second),
								  cMD5);
		} else if (cPasswordPart.second - cPasswordPart.first == Common::MD5::Value::CharLength) {
			// MD5 value
			cMD5.read(cPasswordPart.first);
		} else {
			// Password part don't be formed in MD5 representation
			SydErrorMessage << "Password part don't form valid value("
							<< ModCharString(cPasswordPart.first,
											 static_cast<ModSize>(
												 cPasswordPart.second - cPasswordPart.first))
							<< ")." << ModEndl;
			_SYDNEY_THROW0(Exception::BadPasswordFile);
		}
	}
	// user name
	ModUnicodeString cUserName(cNamePart.first,
							   static_cast<ModSize>(
								   cNamePart.second - cNamePart.first),
							   ModKanjiCode::unknown /* no conv */);

	cResult_.add(cUserName, UserList::Entry::Pointer(new UserList::Entry(cMD5, iID, eCategory)));
	return true;
}

//
//	Copyright (c) 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
