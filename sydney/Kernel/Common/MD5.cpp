// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MD5.cpp -- MD5 related implementation
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Common";
}

#include <stdio.h>

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/MD5.h"
#include "Common/Assert.h"

#include "Exception/BadArgument.h"

#include "Os/Memory.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
	// data unit type
	typedef unsigned int Word;

	//	CONST
	//	$$$::WorkSize -- size of work buffer
	//
	//	NOTES
	//	512 bits -> 64 bytes

	//	CONST
	//	$$$::TSize -- number of elements in the table T
	//
	//	NOTES

	//	CONST
	//	$$$::RegisterSize -- number of registers
	//
	//	NOTES

	enum {
		WorkSize = 512 / 8,
		TSize = 64,
		RegisterSize = 4
	};

	//	CONST
	//	$$$::_Padding -- padding data
	//
	//	NOTES

	const unsigned char _Padding[WorkSize] = {
		0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	//	CONST
	//	$$$::_T -- T table
	//
	//	NOTES
	//	_T[i] represents int(2^32 * abs(sin(i+1))) where i+1 is measured by radian

	const Word _T[TSize] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};

	// CLASS local
	//	_MD5 -- MD5 calculation class
	//
	// NOTES
	class _MD5 : public Common::Object
	{
	public:
		// constructor
		_MD5();

		// update register
		void update(const unsigned char* pHead_, int iSize_);

		// output result
		void output(MD5::Value& cResult_);

	private:
		// append character sequence to buffer
		void append(const unsigned char* pSource_, int iSize_);

		// transform one block
		void transform();

		// copy Word to char array
		void dump(unsigned char* pDest_, Word iValue_);

		// copy char array to Word
		Word undump(const unsigned char* pSource_);

		// left rotate of bits
		Word rotateLeft(Word iValue_, int iShift_)
		{
			return (iValue_ << iShift_) | (iValue_ >> ((sizeof(Word) * 8) - iShift_));
		}

		// help functions
		//
		// [NOTES]
		//	method name and argument name don't obay the coding rule
		//	because those names are derived from RFC1321

		Word F(Word x, Word y, Word z) {return (x & y) | (~x & z);}
		Word G(Word x, Word y, Word z) {return (x & z) | (y & ~z);}
		Word H(Word x, Word y, Word z) {return x ^ y ^ z;}
		Word I(Word x, Word y, Word z) {return y ^ (x | ~z);}

		void round1(int a, int b, int c, int d, int k, int s, int i);
		void round2(int a, int b, int c, int d, int k, int s, int i);
		void round3(int a, int b, int c, int d, int k, int s, int i);
		void round4(int a, int b, int c, int d, int k, int s, int i);

		// work buffer
		unsigned char m_pszBuffer[WorkSize];
		// current tail position of buffer
		int m_iTail;
		// total length of source data
		int m_iLength;
		// calculation register
		Word m_vecRegister[RegisterSize];
	};

	// FUNCTION local
	//	$$$::_MD5::_MD5 -- 
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
	_MD5::_MD5()
		: m_iTail(0), m_iLength(0)
	{
		m_vecRegister[0] = 0x67452301;
		m_vecRegister[1] = 0xefcdab89;
		m_vecRegister[2] = 0x98badcfe;
		m_vecRegister[3] = 0x10325476;
	}

	// FUNCTION local
	//	$$$::_MD5::update -- update register
	//
	// NOTES
	//
	// ARGUMENTS
	//	const unsigned char* pHead_
	//	int iSize_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_MD5::
	update(const unsigned char* pHead_, int iSize_)
	{
		int iRestSize = WorkSize - m_iTail;
		if (iSize_ >= iRestSize) {
			append(pHead_, iRestSize);
			transform();
			int i = iRestSize;
			for (; i + WorkSize <= iSize_; i += WorkSize) {
				append(pHead_ + i, WorkSize);
				transform();
			}
			if (i < iSize_) append(pHead_ + i, iSize_ - i);
		} else {
			append(pHead_, iSize_);
		}
	}

	// FUNCTION local
	//	$$$::_MD5::output -- output result
	//
	// NOTES
	//
	// ARGUMENTS
	//	MD5::Value& cResult_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_MD5::
	output(MD5::Value& cResult_)
	{
		// create bits part
		unsigned char vecBits[8];
		dump(vecBits, m_iLength * 8);
		dump(vecBits + 4, 0);

		// padding
		int iPaddingSize = (m_iTail < WorkSize - 8
							? WorkSize - 8 - m_iTail
							: WorkSize * 2 - 8 - m_iTail);
		update(_Padding, iPaddingSize);
		// add length
		update(vecBits, 8);
		; _TRMEISTER_ASSERT(m_iTail == 0);

		// set to result
		unsigned char* pDest = cResult_.getTop();
		for (int i = 0; i < RegisterSize; ++i) {
			dump(pDest, m_vecRegister[i]);
			pDest += sizeof(Word);
		}
	}

	// FUNCTION local
	//	$$$::_MD5::append -- append character sequence to buffer
	//
	// NOTES
	//
	// ARGUMENTS
	//	const unsigned char* pSource_
	//	int iSize_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_MD5::
	append(const unsigned char* pSource_, int iSize_)
	{
		Os::Memory::copy(m_pszBuffer + m_iTail, pSource_, iSize_);
		m_iTail += iSize_;
		m_iLength += iSize_;
	}

	// FUNCTION local
	//	$$$::_MD5::transform -- transform one block
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
	_MD5::
	transform()
	{
		; _TRMEISTER_ASSERT(m_iTail == WorkSize);

		// save current register value
		Word cSave[RegisterSize];
		for (int i = 0; i < RegisterSize; ++i) {
			cSave[i] = m_vecRegister[i];
		}

		// round1
		round1(0, 1, 2, 3,  0,  7,  0);
		round1(3, 0, 1, 2,  1, 12,  1);
		round1(2, 3, 0, 1,  2, 17,  2);
		round1(1, 2, 3, 0,  3, 22,  3);

		round1(0, 1, 2, 3,  4,  7,  4);
		round1(3, 0, 1, 2,  5, 12,  5);
		round1(2, 3, 0, 1,  6, 17,  6);
		round1(1, 2, 3, 0,  7, 22,  7);

		round1(0, 1, 2, 3,  8,  7,  8);
		round1(3, 0, 1, 2,  9, 12,  9);
		round1(2, 3, 0, 1, 10, 17, 10);
		round1(1, 2, 3, 0, 11, 22, 11);

		round1(0, 1, 2, 3, 12,  7, 12);
		round1(3, 0, 1, 2, 13, 12, 13);
		round1(2, 3, 0, 1, 14, 17, 14);
		round1(1, 2, 3, 0, 15, 22, 15);

		// round2
		round2(0, 1, 2, 3,  1,  5, 16);
		round2(3, 0, 1, 2,  6,  9, 17);
		round2(2, 3, 0, 1, 11, 14, 18);
		round2(1, 2, 3, 0,  0, 20, 19);

		round2(0, 1, 2, 3,  5,  5, 20);
		round2(3, 0, 1, 2, 10,  9, 21);
		round2(2, 3, 0, 1, 15, 14, 22);
		round2(1, 2, 3, 0,  4, 20, 23);

		round2(0, 1, 2, 3,  9,  5, 24);
		round2(3, 0, 1, 2, 14,  9, 25);
		round2(2, 3, 0, 1,  3, 14, 26);
		round2(1, 2, 3, 0,  8, 20, 27);

		round2(0, 1, 2, 3, 13,  5, 28);
		round2(3, 0, 1, 2,  2,  9, 29);
		round2(2, 3, 0, 1,  7, 14, 30);
		round2(1, 2, 3, 0, 12, 20, 31);

		// round3
		round3(0, 1, 2, 3,  5,  4, 32);
		round3(3, 0, 1, 2,  8, 11, 33);
		round3(2, 3, 0, 1, 11, 16, 34);
		round3(1, 2, 3, 0, 14, 23, 35);

		round3(0, 1, 2, 3,  1,  4, 36);
		round3(3, 0, 1, 2,  4, 11, 37);
		round3(2, 3, 0, 1,  7, 16, 38);
		round3(1, 2, 3, 0, 10, 23, 39);

		round3(0, 1, 2, 3, 13,  4, 40);
		round3(3, 0, 1, 2,  0, 11, 41);
		round3(2, 3, 0, 1,  3, 16, 42);
		round3(1, 2, 3, 0,  6, 23, 43);

		round3(0, 1, 2, 3,  9,  4, 44);
		round3(3, 0, 1, 2, 12, 11, 45);
		round3(2, 3, 0, 1, 15, 16, 46);
		round3(1, 2, 3, 0,  2, 23, 47);

		// round4
		round4(0, 1, 2, 3,  0,  6, 48);
		round4(3, 0, 1, 2,  7, 10, 49);
		round4(2, 3, 0, 1, 14, 15, 50);
		round4(1, 2, 3, 0,  5, 21, 51);

		round4(0, 1, 2, 3, 12,  6, 52);
		round4(3, 0, 1, 2,  3, 10, 53);
		round4(2, 3, 0, 1, 10, 15, 54);
		round4(1, 2, 3, 0,  1, 21, 55);

		round4(0, 1, 2, 3,  8,  6, 56);
		round4(3, 0, 1, 2, 15, 10, 57);
		round4(2, 3, 0, 1,  6, 15, 58);
		round4(1, 2, 3, 0, 13, 21, 59);

		round4(0, 1, 2, 3,  4,  6, 60);
		round4(3, 0, 1, 2, 11, 10, 61);
		round4(2, 3, 0, 1,  2, 15, 62);
		round4(1, 2, 3, 0,  9, 21, 63);

		// add saved value
		for (int i = 0; i < RegisterSize; ++i) {
			m_vecRegister[i] += cSave[i];
		}

		m_iTail = 0;
	}

	// FUNCTION local
	//	$$$::_MD5::dump -- copy Word to char array
	//
	// NOTES
	//
	// ARGUMENTS
	//	unsigned char* pDest_
	//	Word iValue_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_MD5::
	dump(unsigned char* pDest_, Word iValue_)
	{
#ifndef SYD_CPU_SPARC
		// little endian -> cast word directly
		Os::Memory::copy(pDest_, syd_reinterpret_cast<const unsigned char*>(&iValue_), sizeof(Word));
#else
		// big endian -> append one by one
		for (int j = 0; j < sizeof(Word); ++j) {
			*(pDest_ + j) = static_cast<unsigned char>(iValue_ & 0xff);
			iValue_ >>= 8;
		}
#endif
	}

	// FUNCTION local
	//	$$$::_MD5::undump -- copy char array to Word
	//
	// NOTES
	//
	// ARGUMENTS
	//	const unsigned char* pSource_
	//	
	// RETURN
	//	Word
	//
	// EXCEPTIONS

	Word
	_MD5::
	undump(const unsigned char* pSource_)
	{
		Word iResult = 0;
#ifndef SYD_CPU_SPARC
		// little endian -> cast to word directly
		Os::Memory::copy(syd_reinterpret_cast<unsigned char*>(&iResult), pSource_, sizeof(Word));
#else
		// big endian -> append one by one
		for (int j = 0; j < sizeof(Word); ++j) {
			iResult += *(pSource_ + j) << (8 * j);
		}
#endif
		return iResult;
	}

	// FUNCTION local
	//	$$$::_MD5::roundX -- help function for transform
	//
	// NOTES
	//	method name and argument name don't obay the coding rule
	//	because those names are derived from RFC1321
	//
	// ARGUMENTS
	//	int a
	//	int b
	//	int c
	//	int d
	//	int k
	//	int s
	//	int i
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	inline void _MD5::round1(int a, int b, int c, int d, int k, int s, int i)
	{
		m_vecRegister[a] += F(m_vecRegister[b], m_vecRegister[c], m_vecRegister[d]);
		m_vecRegister[a] += undump(m_pszBuffer + k * sizeof(Word)) + _T[i];
		m_vecRegister[a] = rotateLeft(m_vecRegister[a], s);
		m_vecRegister[a] += m_vecRegister[b];
	}
	inline void _MD5::round2(int a, int b, int c, int d, int k, int s, int i)
	{
		m_vecRegister[a] += G(m_vecRegister[b], m_vecRegister[c], m_vecRegister[d]);
		m_vecRegister[a] += undump(m_pszBuffer + k * sizeof(Word)) + _T[i];
		m_vecRegister[a] = rotateLeft(m_vecRegister[a], s);
		m_vecRegister[a] += m_vecRegister[b];
	}
	inline void _MD5::round3(int a, int b, int c, int d, int k, int s, int i)
	{
		m_vecRegister[a] += H(m_vecRegister[b], m_vecRegister[c], m_vecRegister[d]);
		m_vecRegister[a] += undump(m_pszBuffer + k * sizeof(Word)) + _T[i];
		m_vecRegister[a] = rotateLeft(m_vecRegister[a], s);
		m_vecRegister[a] += m_vecRegister[b];
	}
	inline void _MD5::round4(int a, int b, int c, int d, int k, int s, int i)
	{
		m_vecRegister[a] += I(m_vecRegister[b], m_vecRegister[c], m_vecRegister[d]);
		m_vecRegister[a] += undump(m_pszBuffer + k * sizeof(Word)) + _T[i];
		m_vecRegister[a] = rotateLeft(m_vecRegister[a], s);
		m_vecRegister[a] += m_vecRegister[b];
	}

	//	CONST
	//	_HexadecimalTable -- Hexadecimal representation for one byte
	//
	//	NOTES

	const char* const _HexadecimalTable[256] =
	{
		"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0a", "0b", "0c", "0d", "0e", "0f",
		"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1a", "1b", "1c", "1d", "1e", "1f",
		"20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2a", "2b", "2c", "2d", "2e", "2f",
		"30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3a", "3b", "3c", "3d", "3e", "3f",
		"40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4a", "4b", "4c", "4d", "4e", "4f",
		"50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5a", "5b", "5c", "5d", "5e", "5f",
		"60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6a", "6b", "6c", "6d", "6e", "6f",
		"70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7a", "7b", "7c", "7d", "7e", "7f",
		"80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8a", "8b", "8c", "8d", "8e", "8f",
		"90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9a", "9b", "9c", "9d", "9e", "9f",
		"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af",
		"b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf",
		"c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf",
		"d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "da", "db", "dc", "dd", "de", "df",
		"e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7", "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef",
		"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff"
	};

	//	CONST
	//	_OctedTable -- Octed value for one hexadecimal representation
	//
	//	NOTES

	const unsigned char _OctedTable[256] =
	{
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	};

	// FUNCTION
	//	_readHexDigit -- convert two octets into one byte
	//
	// NOTES
	//
	// ARGUMENTS
	//	const char* pTop_
	//		pointer to hexadecimal representation of a byte
	//	unsigned char& csResult_
	//	
	// RETURN
	//	bool	true if conversion succeeded
	//
	// EXCEPTIONS
	//	Nothing

	bool _readHexDigit(const char* pTop_, unsigned char& csResult_)
	{
		unsigned char csTmp0 = _OctedTable[static_cast<unsigned char>(*pTop_)];
		unsigned char csTmp1 = _OctedTable[static_cast<unsigned char>(*(pTop_ + 1))];
		if (csTmp0 == 0xff || csTmp1 == 0xff) return false;
		csResult_ = (csTmp0 << 4) | csTmp1;
		return true;
	}
}

// FUNCTION public
//	Common::MD5::Value::read -- read value from hexadecimal representation
//
// NOTES
//
// ARGUMENTS
//	const char* pTop_
//		pointer to source memory area
//		Validity of the memory area for CharLength should be guaranteed by the caller
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
MD5::Value::
read(const char* pTop_)
{
	for (int i = 0; i < Length; ++i) {
		_readHexDigit(pTop_, m_vecValue[i]);
		pTop_ += 2;
	}
}

// FUNCTION public
//	Common::MD5::Value::write -- write MD5 digest value to a memory area
//
// NOTES
//	write method does not  output null-terminate character.
//
// ARGUMENTS
//	unsigned char* pTop_
//		pointer to target memory area
//		Validity of the memory area for CharLength should be guaranteed by the caller
//	
// RETURN
//	char*
//
// EXCEPTIONS

char*
MD5::Value::
write(char* pTop_) const
{
	for (int i = 0; i < Length; ++i) {
		Os::Memory::copy(pTop_, _HexadecimalTable[m_vecValue[i]], 2);
		pTop_ += 2;
	}
	return pTop_;
}

// FUNCTION public
//	Common::MD5::Value::operator== -- compare
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
MD5::Value::
operator==(const Value& cOther_) const
{
	return Os::Memory::compare(m_vecValue, cOther_.m_vecValue, Length) == 0;
}

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
void
MD5::
generate(const unsigned char* pHead_, const unsigned char* pTail_, Value& cResult_)
{
	; _TRMEISTER_ASSERT(pHead_);
	; _TRMEISTER_ASSERT(pTail_);
	; _TRMEISTER_ASSERT(pHead_ <= pTail_);

	_MD5 cMD5;
	cMD5.update(pHead_, static_cast<int>(pTail_ - pHead_));
	cMD5.output(cResult_);
}

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

bool
MD5::
verify(const unsigned char* pHead_, const unsigned char* pTail_, const Value& cValue_)
{
	Value cTmp;
	generate(pHead_, pTail_, cTmp);
	return cTmp == cValue_;
}

//
// Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
