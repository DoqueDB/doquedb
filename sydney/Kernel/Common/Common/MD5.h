// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MD5.h --	MD5 related class/function declaration
// 
// Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_COMMON_MD5_H
#define	__TRMEISTER_COMMON_MD5_H

#include "Common/Module.h"
#include "Common/Object.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	NAMESPACE
//	Common::MD5 -- namespace for MD5 related declaration
//
//	NOTES
//		MD5 is based on RFC1321

namespace MD5
{
	//	CLASS
	//	Common::MD5::Value --	Data type of MD5 results
	//
	//	NOTES
	//	MD5 digest is represented by 128bit sequence

	class SYD_COMMON_FUNCTION Value : public Common::Object
	{
	public:
		//	CONST
		//	Common::MD5::Value::Length -- Length of MD5 results in bytes
		//
		//	NOTES

		//	CONST
		//	Common::MD5::Value::CharLength -- Length of MD5 results in character representation
		//
		//	NOTES
		enum {
			Length = 128 / 8,
			CharLength = Length * 2	// 1byte is represented by two hexadecimal digits
		};

		// constructor
		Value() {}

		// read value from hexadecimal representation
		void read(const char* pTop_);

		// write value in hexadecimal representation
		char* write(char* pTop_) const;

		// get value members top address
		unsigned char* getTop() {return m_vecValue;}

		// compare
		bool operator==(const Value& cOther_) const;

	private:
		// byte representation of the value
		unsigned char m_vecValue[Length];
	};

	// FUNCTION public
	//	Common::MD5::generate -- generate MD5 digest from a sequence
	//
	// NOTES
	//
	// ARGUMENTS
	//	const unsigned char* pHead_
	//	const unsigned char* pTail_
	//		starting and end pointer of the target sequence
	//	Value& cResult_
	//		return value
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	SYD_COMMON_FUNCTION
	void generate(const unsigned char* pHead_, const unsigned char* pTail_, Value& cResult_);

	// FUNCTION public
	//	Common::MD5::verify -- verify MD5 digest by a sequence
	//
	// NOTES
	//
	// ARGUMENTS
	//	const unsigned char* pHead_
	//	const unsigned char* pTail_
	//		starting and end pointer of the verified sequence
	//	const Value& cValue_
	//		MD5 digest compared
	//	
	// RETURN
	//	bool	true if verification is OK
	//
	// EXCEPTIONS

	SYD_COMMON_FUNCTION
	bool verify(const unsigned char* pHead_, const unsigned char* pTail_, const Value& cValue_);
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif	// __TRMEISTER_COMMON_MD5_H

//
// Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
