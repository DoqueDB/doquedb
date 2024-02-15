// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DecimalData.cpp -- 
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyInclude.h"

#include "Common/Assert.h"
#include "Common/ClassID.h"
#include "Common/DecimalData.h"
#include "Common/Message.h"
#include "Common/NullData.h"
#include "Common/StringData.h"
#include "Common/SQLData.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/UnicodeString.h"

#include "Exception/BadArgument.h"
#include "Exception/ClassCast.h"
#include "Exception/DivisionByZero.h"
#include "Exception/NullNotAllowed.h"
#include "Exception/NumericValueOutOfRange.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/InvalidCharacter.h"
#include "Os/Limits.h"
#include "Os/Math.h"
#include "Os/Memory.h"

#include "ModHasher.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"
#include "ModAlgorithm.h"

#include <iostream>
#include <stdio.h>

using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace {

	// CONST
	// _iDigitPerDecimal -- the number of digits in a unit.
	//
	// NOTES

	const int _iDigitPerDecimal = 9;

	// CONST
	// _iDigitMask -- the mask of _iDigitPerDecimal.
	//
	// NOTES

	const DecimalData::DigitUnit _iDigitMask = 100000000;
											   
	// CONST
	// _iDigitBase -- the base value of _iDigitPerDecimal.
	//
	// NOTES

	const DecimalData::DigitUnit _iDigitBase = 1000000000;
	
	// CONST
	// _iDigitMax -- the max value of _iDigitPerDecimal.
	//
	// NOTES

	const DecimalData::DigitUnit _iDigitMax = _iDigitBase-1;

	// CONST
	//  _iDoubleBufferSize -- the buffer size of double to sprintf
	//
	// NOTES

	const int _iDoubleBufferSize = 24;

	// STATIC CONST
	// _powers10 -- the powers based on ten.
	//
	// NOTES

	const DecimalData::DigitUnit _powers10[_iDigitPerDecimal+1] = {
		1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

	// STATIC
	// fraction_max -- the max fraction.
	//
	// NOTES

	const DecimalData::DigitUnit fraction_max[_iDigitPerDecimal-1] = {
		900000000, 990000000, 999000000, 999900000, 999990000, 999999000, 999999900, 999999990 };
	
	const int dig2bytes[_iDigitPerDecimal+1] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 4};

	ModSize _fourBytesRoundUp(ModSize iByteNum_)
	{
		return (iByteNum_ + 3) / 4 * 4;
	}

	// FUNCTION
	// _roundUp -- get the number of DigitUnit for a digits
	//
	// NOTES

	int _roundUp(int iDigitNum_)
	{
		return (iDigitNum_ + _iDigitPerDecimal - 1) / _iDigitPerDecimal;
	}

	// FUNCTION
	// subAdd -- add two DecimalData::DigitUnit and return the result
	//
	// NOTES

	DecimalData::DigitUnit _subAdd(const DecimalData::DigitUnit& dec1, const DecimalData::DigitUnit& dec2, int& carry)  /* assume carry <= 1 */  
	{   
		DecimalData::DigitUnit tmpUnit = dec1 + dec2 + carry;                                       
		carry = (tmpUnit >= _iDigitBase) ? 1 : 0;
		if (carry > 0) //if there is a carry     
			tmpUnit -= _iDigitBase;                                                
		return tmpUnit;   
	}

	// FUNCTION
	// subSub -- sub two DecimalData::DigitUnit and return the result
	//
	// NOTES

	DecimalData::DigitUnit _subSub(const DecimalData::DigitUnit& dec1, const DecimalData::DigitUnit& dec2, int& carry) //result = dec1 - dec2
	{
		DecimalData::DigitUnit tmpUnit = dec1 - dec2 - carry;    
		carry = (tmpUnit < 0)? 1: 0;
        if (carry > 0)     //if there is a carry                
			tmpUnit += _iDigitBase;                                                
        return tmpUnit;  
	}	

	void _NByteDump(char* pDest_, DecimalData::DigitUnit iDigitUnit_, int iBytes_)
	{
		char* p = pDest_ + (iBytes_ - 1);
		unsigned int iTmp = static_cast<unsigned int>(iDigitUnit_);

		switch (iBytes_) {
		case 4:
			*p-- = iTmp & 0xff;
			iTmp >>= 8;
		case 3:
			*p-- = iTmp & 0xff;
			iTmp >>= 8;
		case 2:
			*p-- = iTmp & 0xff;
			iTmp >>= 8;
		case 1:
			*p = iTmp & 0xff;
			break;
		default:
			; _TRMEISTER_ASSERT(0);
			break;
		}
	}

	DecimalData::DigitUnit _NByteSetDumped(const char* pSource_, int iBytes_)
	{
		const char* p = pSource_;
		unsigned int iResult = 0;
		int iShift = 8 * (iBytes_ - 1);

		for (int i = 0; i < iBytes_; ++i) {
			DecimalData::DigitUnit iTempMask = 0x00;
			for (int j = i; j<iBytes_; j++)
				iTempMask = (iTempMask<<8) + 0xff;

			DecimalData::DigitUnit iTempValue = static_cast<unsigned int>(*p++) << iShift;
			iResult += iTempValue & iTempMask;
			iShift -= 8;
		}

		return iResult;
	}

	// TEMPLATE FUNCTION local
	//	$$$::_hasMoreDigit -- 
	//
	// TEMPLATE ARGUMENTS
	//	class _T_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	_T_ cValue_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS

	template <class _T_>
	bool
	_hasMoreDigit(_T_ cValue_)
	{
		return cValue_ < -9 || 9 < cValue_;
	}
	template <>
	bool
	_hasMoreDigit<unsigned int>(unsigned int cValue_)
	{
		return cValue_ > 9;
	}
	template <>
	bool
	_hasMoreDigit<ModUInt64>(ModUInt64 cValue_)
	{
		return cValue_ > 9;
	}

	// TEMPLATE FUNCTION local
	//	$$$::_countDigit -- 
	//
	// TEMPLATE ARGUMENTS
	//	class _T_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	_T_ cValue_
	//	
	// RETURN
	//	int
	//
	// EXCEPTIONS

	template <class _T_>
	int
	_countDigit(_T_ cValue_)
	{
		int iResult = 1;
		while (_hasMoreDigit(cValue_)) {
			++iResult;
			cValue_ /= 10;
		}
		return iResult;
	}

	// FUNCTION public
	//	$$$::_getDigitIterator -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const DecimalData::Decimal& cDecimal_
	//	int iPosition_
	//	
	// RETURN
	//	ModVector<DecimalData::DigitUnit>::ConstIterator
	//
	// EXCEPTIONS

	ModVector<DecimalData::DigitUnit>::ConstIterator
	_getDigitIterator(const DecimalData::Decimal& cDecimal_, int iPosition_)
	{
		return cDecimal_.getDigitVector().begin() + _roundUp(iPosition_);
	}
	ModVector<DecimalData::DigitUnit>::Iterator
	_getDigitIterator(DecimalData::Decimal& cDecimal_, int iPosition_)
	{
		return cDecimal_.getDigitVector().begin() + _roundUp(iPosition_);
	}

	//
	//	TEMPLATE FUNCTION public
	//	$$$::_castFromInt -- the value is got by casting from int.
	//
	//	NOTES
	//  VC6 maybe doesn't support template member functions.
	//  So this is defined in unnamed namespace.
	//  See $$$::_Cast::toInteger() in StringData.cpp.
	//
	//	TEMPLATE ARGUMENTS
	//	class _T_
	//
	//	ARGUMENTS
	//	DecimalData::Decimal& cValue_,
	//	int& iPrecision_,
	//	int& iScale_,
	//  bool bForAssign_
	//       false: no exception thrown; true:throw exception
	//
	//	RETURN
	//	bool
	//		true
	//          The operation result is not NULL. 
	//      false
	//          The operation result is NULL.    
	//
	//	EXCEPTIONS
	//	bool
	//
	template <class _T_>
	bool _castFromInt(_T_ cValue_,
					  DecimalData::Decimal& cDecimalValue_,
					  int& iPrecision_,
					  int& iScale_,
					  bool bForAssign_)
	{
		// to get the digit number of the IntegerData
		bool bNegative = (cValue_ < 0);

		int iIntegerPartNum = _countDigit(cValue_);

		if ((0 == cDecimalValue_.getInteger())&&(0 == cDecimalValue_.getFraction()))
		{
			cDecimalValue_.setRange(iIntegerPartNum, 0);
			iPrecision_ = iIntegerPartNum;
			iScale_ = 0;
		}
		if (iIntegerPartNum > cDecimalValue_.getInteger())
		{
			if (cValue_ == 0)
			{
				cDecimalValue_.makeZero(cDecimalValue_.getInteger(),cDecimalValue_.getFraction());
				return true;
			}
			if (bForAssign_)
				_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
			else
				return false;
		}

		; _SYDNEY_ASSERT(cDecimalValue_.getInteger() > 0);

		ModVector<DecimalData::DigitUnit>::Iterator iterator =
			_getDigitIterator(cDecimalValue_, cDecimalValue_.getInteger());
		const ModVector<DecimalData::DigitUnit>::Iterator head =
			_getDigitIterator(cDecimalValue_, 0);

		_T_ cNext = cValue_;
		do {
			DecimalData::DigitUnit iUnit =
				static_cast<DecimalData::DigitUnit>(cNext % _iDigitBase);
			if (iUnit < 0) iUnit = -iUnit;
			*--iterator = iUnit;
			cNext /= _iDigitBase;
		} while (iterator != head);

		cDecimalValue_.setSign(bNegative);
		return true;

	}

	// TEMPLATE FUNCTION public
	//	Common::bool _castToInt -- 
	//
	// TEMPLATE ARGUMENTS
	//	class _T_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	_T_ cValue_
	//	const DecimalData::Decimal& cDecimalValue_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class _T_>
	bool _castToInt(_T_& cValue_,
					const DecimalData::Decimal& cDecimalValue_)
	{
		_T_ cResult(0);
		ModVector<DecimalData::DigitUnit>::ConstIterator iterator =
			_getDigitIterator(cDecimalValue_, 0);
		const ModVector<DecimalData::DigitUnit>::ConstIterator last =
			_getDigitIterator(cDecimalValue_, cDecimalValue_.getInteger());

		if (cDecimalValue_.isNegative()) {
			for (; iterator != last; ++iterator) {
				DecimalData::DigitUnit iUnit = *iterator;
				if (cResult < (Os::Limits<_T_>::getMin() / _iDigitBase)) {
					return false;
				}
				cResult *= _iDigitBase;

				if (cResult < (Os::Limits<_T_>::getMin() + iUnit)) {
					return false;
				}
				cResult -= iUnit;
			}
		} else {
			for (; iterator != last; ++iterator) {
				DecimalData::DigitUnit iUnit = *iterator;
				if (cResult > (Os::Limits<_T_>::getMax() / _iDigitBase)) {
					return false;
				}
				cResult *= _iDigitBase;

				if (cResult > (Os::Limits<_T_>::getMax() - iUnit)) {
					return false;
				}
				cResult += iUnit;
			}
		}
		cValue_ = cResult;
		return true;
	}

	// TEMPLATE FUNCTION public
	//	Common::bool _castToFloat -- 
	//
	// TEMPLATE ARGUMENTS
	//	class _T_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	_T_ cValue_
	//	const DecimalData::Decimal& cDecimalValue_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	template <class _T_>
	bool _castToFloat(_T_& cValue_,
					  const DecimalData::Decimal& cDecimalValue_,
					  int iStart_, int iEnd_)
	{
		_T_ cResult(0);
		ModVector<DecimalData::DigitUnit>::ConstIterator iterator =
			_getDigitIterator(cDecimalValue_, iStart_);
		const ModVector<DecimalData::DigitUnit>::ConstIterator last =
			iterator + _roundUp(iEnd_);

		if (cDecimalValue_.isNegative()) {
			for (; iterator != last; ++iterator) {
				DecimalData::DigitUnit iUnit = *iterator;
				if (cResult < (-Os::Limits<_T_>::getMax() / _iDigitBase)) {
					return false;
				}
				cResult *= _iDigitBase;

				if (cResult < (-Os::Limits<_T_>::getMax() + iUnit)) {
					return false;
				}
				cResult -= iUnit;
			}
		} else {
			for (; iterator != last; ++iterator) {
				DecimalData::DigitUnit iUnit = *iterator;
				if (cResult > (Os::Limits<_T_>::getMax() / _iDigitBase)) {
					return false;
				}
				cResult *= _iDigitBase;

				if (cResult > (Os::Limits<_T_>::getMax() - iUnit)) {
					return false;
				}
				cResult += iUnit;
			}
		}
		cValue_ = cResult;
		return true;
	}

}  // namespace $$$

//
//	FUNCTION public
//	Common::DecimalData::Decimal::Decimal -- constructor
//
//	NOTES
//	constructor
//
//	ARGUMENTS
//	
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
DecimalData::Decimal::
Decimal()
{
	m_iIntegerPartLength = 0;
	m_iFractionPartLength = 0;
	m_bIsNegative = false;
}

DecimalData::Decimal::
Decimal(const Decimal& cDecimal_)
{
	setValue(cDecimal_);
}

//
//	FUNCTION public
//	Common::DecimalData::Decimal::~Decimal -- destructor
//
//	NOTES
//	destructor
//
//	ARGUMENTS
//	None
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
DecimalData::Decimal::
~Decimal()
{
	m_vecDigit.clear();
	m_iIntegerPartLength = 0;
	m_iFractionPartLength = 0;
	m_bIsNegative = false;
}
		
//
//	FUNCTION public
//	Common::Decimal::Decimal::setValue -- set value
//
//	NOTES
//	set value
//
//	ARGUMENTS
//	
//		
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
void
DecimalData::Decimal::
setValue(const Decimal& cDecimal_)
{
	m_iIntegerPartLength = cDecimal_.m_iIntegerPartLength;
	m_iFractionPartLength = cDecimal_.m_iFractionPartLength;

	if (0 == getDigitLength() 
		|| getDigitLength() != cDecimal_.getDigitLength())
		resetDigitVector(cDecimal_.getDigitLength());

	m_vecDigit = cDecimal_.m_vecDigit;
	setSign(cDecimal_.m_bIsNegative);
}

void
DecimalData::Decimal::
setRange(int iIntegerLen_, int iFractionLen_ )
{
	setInteger(iIntegerLen_);
	setFraction(iFractionLen_);
	resetDigitVector(_roundUp(iIntegerLen_) + _roundUp(iFractionLen_));
}

void 
DecimalData::Decimal::
setSign(bool bIsNegative_)
{	
	m_bIsNegative = bIsNegative_;

	if (m_bIsNegative)
	{
		;_TRMEISTER_ASSERT(m_vecDigit.getSize() > 0);
		bool bEqualZero = true;
		for(ModSize i = 0; i < m_vecDigit.getSize(); i++)
		{
			if (m_vecDigit[i] != 0)
			{
				bEqualZero = false;
				break;
			}
		}

		if (bEqualZero)
			m_bIsNegative = false;
	}
}

void
DecimalData::Decimal::
setFraction(int iFractionLen_)
{ 
	m_iFractionPartLength = iFractionLen_; 
	if (iFractionLen_ < 0) 	
		m_iFractionPartLength = 0;
}

void
DecimalData::Decimal::
resetDigitVector(int iSize_)
{
	; _TRMEISTER_ASSERT(iSize_ >= 0);
	m_vecDigit.clear();
	if (iSize_ > 0)
		m_vecDigit.assign(iSize_, 0);
}

void
DecimalData::Decimal::
setDigitVector(const Decimal& cOperand0_,  const Decimal& cOperand1_, DataOperation::Type eOperation_)
{
	resetDigitVector(cOperand0_.calculateResultSize(cOperand1_, eOperation_));
}

//
//	FUNCTION public
//	Common::DecimalData::Decimal::operator = --operator overloading
//
//	NOTES
//  operator overloading
//
//	ARGUMENTS
//	DecimalData& cDecimalData_
//		the referent data
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
DecimalData::Decimal& 
DecimalData::Decimal::
operator = (const Decimal& cOther_)
{
	if (&cOther_ != this)
		setValue(cOther_);
	
	return *this;
}
//
//	FUNCTION public
//	Common::DecimalData::Decimal::reset -- reset the data
//
//	NOTES
//  reset the data
//
//	ARGUMENTS
//	None
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
void
DecimalData::Decimal::
makeZero(int iIntegerLen_, int iFractionLen_)
{
	m_iIntegerPartLength = iIntegerLen_;
	m_iFractionPartLength = iFractionLen_;
	if (iIntegerLen_ == 0 && iFractionLen_ == 0)
		m_iIntegerPartLength = 1;
	m_bIsNegative = false;
	m_vecDigit.clear();
	m_vecDigit.assign(_roundUp(m_iIntegerPartLength) + _roundUp(m_iFractionPartLength), 0);
}

//
//	FUNCTION public
//	Common::DecimalData::Decimal::swap = -- swap the data
//
//	NOTES
//  swap the data
//
//	ARGUMENTS
//	Decimal& dec_
//        the another decimaldata
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
bool 
DecimalData::Decimal::
swap(Decimal& cOtherDecimal_)
{
	Decimal cTemp(*this);
	
	*this = cOtherDecimal_; 
	cOtherDecimal_ = cTemp;
    
	return true;
}
//
//	FUNCTION public
//	Common::DecimalData::Decimal::calculateResultSize ----the result size of arithmetic operations
//
//	NOTES
//  the result size of arithmetic operations
//
//	ARGUMENTS
//	Decimal& dec_
//        the other operands 
//  DataOperation::Type eOperation_
//        arithmetic operation
//
//	RETURN
//	int
//       the size of result in Digit Unit
//
//	EXCEPTIONS
//	None
//
int 
DecimalData::Decimal::
calculateResultSize(const DecimalData::Decimal& cOtherDecimal_, DataOperation::Type eOperation_) const
{
	switch(eOperation_)
	{
	case DataOperation::Addition:
		return _roundUp(ModMax(m_iIntegerPartLength, cOtherDecimal_.m_iIntegerPartLength) + 1)
			+ _roundUp(ModMax(m_iFractionPartLength, cOtherDecimal_.m_iFractionPartLength));		
		break;
	case DataOperation::Subtraction:
		return _roundUp(ModMax(m_iIntegerPartLength,cOtherDecimal_.m_iIntegerPartLength) + 1)
			+ _roundUp(ModMax(m_iFractionPartLength, cOtherDecimal_.m_iFractionPartLength));
		break;
	case DataOperation::Multiplication:
		return _roundUp(m_iIntegerPartLength + cOtherDecimal_.m_iIntegerPartLength)
			+ _roundUp(m_iFractionPartLength) + _roundUp(cOtherDecimal_.m_iFractionPartLength);
		break;
	case DataOperation::Division:
		return _roundUp(m_iIntegerPartLength + cOtherDecimal_.m_iFractionPartLength)
			+ _roundUp(m_iFractionPartLength + cOtherDecimal_.m_iIntegerPartLength + 1);
		break;
	default:
		;_TRMEISTER_ASSERT(false);
		break;
	}
	return -1;
}


//
//	FUNCTION public
//	Common::DecimalData::Decimal::maxDecimal ---- max decimal of defined integer part and fraction part
//
//	NOTES
//  max decimal of defined integer part and fraction part
//
//	ARGUMENTS
//	int iInteger_
//        the integer part
//  int iFraction_
//        the fraction part
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
void 
DecimalData::Decimal::
setToMaxDecimal(int iInteger_, int iFraction_)
{
	;_TRMEISTER_ASSERT(iInteger_ >= 0 && iFraction_ >= 0);

	resetDigitVector(iInteger_+iFraction_);
	ModVector<DecimalData::DigitUnit>::Iterator  buffer = m_vecDigit.begin();
    
	m_bIsNegative = false;

	m_iIntegerPartLength = iInteger_;

	int iIntegerPartLen = iInteger_;
	if (iIntegerPartLen > 0)
	{
		int firstDigits= iIntegerPartLen % _iDigitPerDecimal;

		if (firstDigits>0)
			*buffer++ = _powers10[firstDigits] - 1; /* get 9 99 999 ... */

		for(iIntegerPartLen/=_iDigitPerDecimal; iIntegerPartLen>0; iIntegerPartLen--)
			*buffer++ = _iDigitMax;
	}

	m_iFractionPartLength = iFraction_;

	int iFractionPartLen = iFraction_;
	if (iFractionPartLen > 0)
	{
		int lastDigits= iFraction_ % _iDigitPerDecimal;

		for (iFractionPartLen/=_iDigitPerDecimal; iFractionPartLen>0; iFractionPartLen--)
			*buffer++ = _iDigitMax;

		if (lastDigits > 0)
			*buffer = fraction_max[lastDigits - 1];
	}
}

//
//	FUNCTION public
//	Common::DecimalData::Decimal::removeLeadingZeroes ---- remove the leading zeroes of m_vecDigit
//
//	NOTES
//  remove the leading zeroes of m_vecDigit
//
//	ARGUMENTS
//	int& intgResult_
//        the size of returned integer part
//
//	RETURN
//	ModVector<DecimalData::DigitUnit>::Iterator 
//        the new m_vecDigit that is removed leading zeroes
//
//	EXCEPTIONS
//  None           
//
ModVector<DecimalData::DigitUnit>::Iterator 
DecimalData::Decimal::
removeLeadingZeroes(int& iIntegerPartResult_)
{
	;_TRMEISTER_ASSERT(m_vecDigit.getSize() > 0);

	int iIntergerPartLen = m_iIntegerPartLength;
	ModVector<DecimalData::DigitUnit>::Iterator cBuffer = m_vecDigit.begin();
	int removedPartLen = ((iIntergerPartLen - 1) % _iDigitPerDecimal) + 1;

	while (iIntergerPartLen > 0 && 0 == *cBuffer)
	{
		iIntergerPartLen -= removedPartLen;
		removedPartLen = _iDigitPerDecimal;
		cBuffer++;
	}

	if (iIntergerPartLen > 0)
	{
		for (removedPartLen = (iIntergerPartLen - 1) % _iDigitPerDecimal; *cBuffer < _powers10[removedPartLen]; removedPartLen--)
			iIntergerPartLen--;
		;_TRMEISTER_ASSERT(iIntergerPartLen>0);
	}
	else
		iIntergerPartLen = 0;
	
	iIntegerPartResult_ = iIntergerPartLen;

	return cBuffer;
}


//	FUNCTION public
//	Common::DecimalData::Decimal::doCompare -- compare two decimal with the same sign
//
//	NOTES
//            compare two decimal.
//
//	ARGUMENTS
//	DecimalData::Decimal& dec_
//         the referent data
//
//	RETURN
//		0
// 			The left side and the right side are equal. 
//		-1
// 			The left side is smaller.  left < right
//		1 
// 			The right side is smaller. left > right
//
//	EXCEPTIONS
//	None
//
int 
DecimalData::Decimal::
doCompare(const DecimalData::Decimal& cOtherDecimal_) const//two positive decimal compare
{
	;_TRMEISTER_ASSERT(m_vecDigit.getSize() > 0);
	;_TRMEISTER_ASSERT(cOtherDecimal_.m_vecDigit.getSize() > 0);

	int iIntegerPartLength1 = _roundUp(m_iIntegerPartLength);
	int iIntegerPartLength2 = _roundUp(cOtherDecimal_.m_iIntegerPartLength);
	int iFractionPartLength1 = _roundUp(m_iFractionPartLength);
	int iFractionPartLength2 = _roundUp(cOtherDecimal_.m_iFractionPartLength);

	ModVector<DecimalData::DigitUnit>::ConstIterator iterator1 = m_vecDigit.begin();
	ModVector<DecimalData::DigitUnit>::ConstIterator last1 = m_vecDigit.end();
	ModVector<DecimalData::DigitUnit>::ConstIterator iterator2 = cOtherDecimal_.m_vecDigit.begin();
	ModVector<DecimalData::DigitUnit>::ConstIterator last2 = cOtherDecimal_.m_vecDigit.end();

	// compare until compared operands have different integer length
	while (iIntegerPartLength1 > iIntegerPartLength2) 
	{
		if (*iterator1 == 0) 
		{
			++iterator1;
			--iIntegerPartLength1;
		}
		else
		{	// this is greater
			return 1;
		}
	}
	while (iIntegerPartLength2 > iIntegerPartLength1) 
	{
		if (*iterator2 == 0) 
		{
			++iterator2;
			--iIntegerPartLength2;
		}
		else 
		{
			// other is greater
			return -1;
		}
	}
	// now, non-zero integer part have same length
	; _TRMEISTER_ASSERT(iIntegerPartLength1 == iIntegerPartLength2);

	// compare until integer part rests
	while (iIntegerPartLength1 > 0) 
	{
		if (*iterator1 == *iterator2)
		{
			++iterator1;
			++iterator2;
			--iIntegerPartLength1;
		} 
		else 
		{
			return (*iterator1 > *iterator2) ? 1 : -1;
		}
	}
	// now, integer part is equal
	// compare fraction part
	int iFractionPartLength = ModMin(iFractionPartLength1, iFractionPartLength2);
	while (iFractionPartLength > 0) 
	{
		if (*iterator1 == *iterator2) 
		{
			++iterator1;
			++iterator2;
			--iFractionPartLength;
			--iFractionPartLength1;
			--iFractionPartLength2;
		}
		else 
		{
			return (*iterator1 > *iterator2) ? 1 : -1;
		}
	}
	while (iFractionPartLength1 > 0)
	{
		if (*iterator1 == 0) 
		{
			++iterator1;
			--iFractionPartLength1;
		}
		else 
		{
			return 1;
		}

	}

	while (iFractionPartLength2 > 0)
	{
		if (*iterator2 == 0) 
		{
			++iterator2;
			--iFractionPartLength2;
		}
		else 
		{
			return -1;
		}

	}

	return 0;

}


//	FUNCTION public
//	Common::DecimalData::Decimal::doAdd -- add two decimals with the same sign
//
//	NOTES
//  add two decimals
//
//	ARGUMENTS
//	DecimalData::Decimal& cDecimal1_
//         operand1
//	DecimalData::Decimal& cDecimal2_
//         operand2
//  bool bNoThrow_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//	
//
bool
DecimalData::Decimal::
doAdd(const DecimalData::Decimal& cDecimal1_, const DecimalData::Decimal& cDecimal2_)//the same sign & the memory size of result.m_dBuffer
{
	;_TRMEISTER_ASSERT(cDecimal1_.m_vecDigit.getSize() > 0);
	;_TRMEISTER_ASSERT(cDecimal2_.m_vecDigit.getSize() > 0);		

	int iIntegerPartLen1 = _roundUp(cDecimal1_.m_iIntegerPartLength);
	int iIntegerPartLen2 = _roundUp(cDecimal2_.m_iIntegerPartLength);
	int iIntegerResult = ModMax(iIntegerPartLen1, iIntegerPartLen2);
    
	int iFractionPartLen1 = _roundUp(cDecimal1_.m_iFractionPartLength);
	int iFractionPartLen2 = _roundUp(cDecimal2_.m_iFractionPartLength);
	int iFractionResult = ModMax(iFractionPartLen1, iFractionPartLen2);
	
	// is there a need for extra word because of array ?  
	// three cases: the IntegerPartLen1 of cDecimal1_ > the IntegerPartLen2 of cDecimal2_
	//              the IntegerPartLen1 of cDecimal1_ < the IntegerPartLen2 of cDecimal2_
	//              the IntegerPartLen1 of cDecimal1_ = the IntegerPartLen2 of cDecimal2_ 
	DecimalData::DigitUnit dFirstDigitUnit = (iIntegerPartLen1 > iIntegerPartLen2) ? cDecimal1_.m_vecDigit[0]:
														(iIntegerPartLen2 > iIntegerPartLen1) ? cDecimal2_.m_vecDigit[0]:
															cDecimal1_.m_vecDigit[0] + cDecimal2_.m_vecDigit[0];

	// there is
	if ( dFirstDigitUnit > _iDigitMax  - 1 ) 
	{
		iIntegerResult++;
		m_vecDigit[0] = 0;
	}

	bool bTruncated = false;
	int iBufferLength = m_vecDigit.getSize();

	//judge the value of iIntegerResult + iFractionResult and the assigned space -- iBufferLength;
	if ((iIntegerResult + iFractionResult) > iBufferLength)                            
	{                                                          
        if (iIntegerResult > iBufferLength)                           
		{                                                        
			return false;
		}                                                           
		else       //iInteger_Result <= result.m_iBufferLength                                                 
		{                                                           
			iFractionResult = iBufferLength - iIntegerResult;                
			bTruncated = true;
		}                                                           
	}              
	
	m_iFractionPartLength = ModMax(cDecimal1_.m_iFractionPartLength, cDecimal2_.m_iFractionPartLength);
	m_iIntegerPartLength = iIntegerResult * _iDigitPerDecimal;

	ModVector<DecimalData::DigitUnit>::Iterator buf = m_vecDigit.begin() + iIntegerResult + iFractionResult;
	if (!bTruncated)
	{
		if (m_iFractionPartLength > iFractionResult * _iDigitPerDecimal)
			m_iFractionPartLength = iFractionResult * _iDigitPerDecimal;
		if (iFractionPartLen1 > iFractionResult)
			iFractionPartLen1 = iFractionResult;
		if (iFractionPartLen2 > iFractionResult)
			iFractionPartLen2 = iFractionResult;
		if (iIntegerPartLen1 > iIntegerResult)
			iIntegerPartLen1 = iIntegerResult;
		if (iIntegerPartLen2 > iIntegerResult)
			iIntegerPartLen2 = iIntegerResult;
	}
	// the calc has three parts: max(iFractionPartLen)...min(iFractionPartLen) , min(iFractionPartLen)..min(iIntegerPartLen) and  min(iIntegerPartLen)...max(iIntegerPartLen)
	ModVector<DecimalData::DigitUnit>::ConstIterator buf1; // the operand which has bigger iFractionPartLen
	ModVector<DecimalData::DigitUnit>::ConstIterator buf2; // the operand which has smaller iFractionPartLen
	ModVector<DecimalData::DigitUnit>::ConstIterator stop; // the first stop for max(iFractionPartLen)...min(iFractionPartLen)
	ModVector<DecimalData::DigitUnit>::ConstIterator stop2;// the second stop for min(iFractionPartLen)..min(iIntegerPartLen)
	// max(iFractionPartLen)...min(iFractionPartLen)
	if (iFractionPartLen1 > iFractionPartLen2)
	{
		buf1 = cDecimal1_.m_vecDigit.begin() + iIntegerPartLen1 + iFractionPartLen1;
		stop = cDecimal1_.m_vecDigit.begin() + iIntegerPartLen1 + iFractionPartLen2;
		buf2 = cDecimal2_.m_vecDigit.begin() + iIntegerPartLen2 + iFractionPartLen2;
		stop2 = cDecimal1_.m_vecDigit.begin() + (iIntegerPartLen1 > iIntegerPartLen2 ? iIntegerPartLen1 - iIntegerPartLen2 : 0);
	}
	else
	{
		buf1 = cDecimal2_.m_vecDigit.begin() + iIntegerPartLen2 + iFractionPartLen2;
		stop = cDecimal2_.m_vecDigit.begin() + iIntegerPartLen2 + iFractionPartLen1;
		buf2 = cDecimal1_.m_vecDigit.begin() + iIntegerPartLen1 + iFractionPartLen1;
		stop2 = cDecimal2_.m_vecDigit.begin() + (iIntegerPartLen2 > iIntegerPartLen1 ? iIntegerPartLen2 - iIntegerPartLen1 : 0);
	}

	while (buf1 > stop)
	{
		*--buf = *--buf1;
	}

	// min(iFractionPartLen)..min(iIntegerPartLen)
	DecimalData::DigitUnit dCarry = 0;
	while (buf1 > stop2)
	{
		*--buf = _subAdd(*--buf1, *--buf2, dCarry);
	}

	// min(iIntegerPartLen)...max(iIntegerPartLen)
	// now, stop is the second stop for min(iIntegerPartLen)...max(iIntegerPartLen)
	stop = (iIntegerPartLen1 > iIntegerPartLen2) ? cDecimal1_.m_vecDigit.begin() : cDecimal2_.m_vecDigit.begin();
	buf1 = (iIntegerPartLen1 > iIntegerPartLen2) ? (stop + iIntegerPartLen1 - iIntegerPartLen2):
							                       (stop + iIntegerPartLen2 - iIntegerPartLen1);

	while (buf1 > stop)		
	{
		const DigitUnit zeroDec = 0;
		*--buf = _subAdd(*--buf1, zeroDec, dCarry);
	} 

	if (dCarry > 0) //if there is a carry
	{
		*--buf = 1;
	}

	setSign(cDecimal1_.m_bIsNegative);//set the sign

	return true;
}

//	FUNCTION public
//	Common::DecimalData::Decimal::doSub -- sub two decimals
//
//	NOTES
//  sub two decimals
//
//	ARGUMENTS
//	DecimalData::Decimal& cDecimal1_
//         operand1
//	DecimalData::Decimal& cDecimal2_
//         operand2
//  bool bNoThrow_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//

bool
DecimalData::Decimal::
doSub(const DecimalData::Decimal& cDecimal1_, const DecimalData::Decimal& cDecimal2_)//the same sign & the memory size of result.m_dBuffer
{
	;_TRMEISTER_ASSERT(cDecimal1_.m_vecDigit.getSize() > 0);
	;_TRMEISTER_ASSERT(cDecimal2_.m_vecDigit.getSize() > 0);
	;_TRMEISTER_ASSERT(m_vecDigit.getSize() > 0);
	
	bool bIsNegative = cDecimal1_.m_bIsNegative;

	//to ensure that always dec1> dec2  (and iIntegerPartLen1 >= iIntegerPartLen2)	
	int iBigger = cDecimal1_.doCompare(cDecimal2_);

	const DecimalData::Decimal* pOperand1 = &cDecimal1_;
	const DecimalData::Decimal* pOperand2 = &cDecimal2_;
	if (iBigger == -1)// if cDecimal1_ < cDecimal2_, swap them
	{
		ModSwap(pOperand1, pOperand2);
		bIsNegative = !bIsNegative;
	}
	else if (iBigger == 0)// if cDecimal1_ = cDecimal2_, return zero
	{
		makeZero(1, ModMax(cDecimal1_.m_iFractionPartLength, cDecimal2_.m_iFractionPartLength));
		return true;
	}

	int iIntegerPartLen1 = _roundUp(pOperand1->m_iIntegerPartLength);
	int iIntegerPartLen2 = _roundUp(pOperand2->m_iIntegerPartLength);
	int iFractionPartLen1 = _roundUp(pOperand1->m_iFractionPartLength);
	int iFractionPartLen2 = _roundUp(pOperand2->m_iFractionPartLength);
	int iIntegerPartLen = ModMax(iIntegerPartLen1, iIntegerPartLen2);
	int iFractionPartLen = ModMax(iFractionPartLen1,iFractionPartLen2);

	ModVector<DecimalData::DigitUnit>::ConstIterator buf1 = pOperand1->m_vecDigit.begin();
	ModVector<DecimalData::DigitUnit>::ConstIterator start1 = buf1;
	ModVector<DecimalData::DigitUnit>::ConstIterator stop1 = buf1 + iIntegerPartLen1;

	ModVector<DecimalData::DigitUnit>::ConstIterator buf2 = pOperand2->m_vecDigit.begin();
	ModVector<DecimalData::DigitUnit>::ConstIterator start2 = buf2;
	ModVector<DecimalData::DigitUnit>::ConstIterator stop2 = buf2 + iIntegerPartLen2;

	if (*buf1 == 0) //remove leading zeroes
	{
		while(buf1 < stop1 && *buf1 == 0)
			buf1++;
		start1 = buf1;
		iIntegerPartLen1 = stop1 - buf1;
	}

	if (*buf2 == 0) //remove leading zeroes
	{
		while(buf2 < stop2 && *buf2 == 0)
			buf2++;
		start2 = buf2;
		iIntegerPartLen2 = stop2 - buf2;
	}

	int bTruncated = false;
	int iBufferLength = m_vecDigit.getSize();

	//fix-iIntegerPartLen-iFractionPartLen-error
	if ((iIntegerPartLen1+iFractionPartLen) > iBufferLength)                            
	{   
		if (iIntegerPartLen1 > iBufferLength)                                
		{                                                                    
			return false;
		}                                                            
		else                                                        
		{                                                           
			iFractionPartLen = iBufferLength - iIntegerPartLen1;                                        
			bTruncated = true;
		}   
	}

	ModVector<DecimalData::DigitUnit>::Iterator buf = m_vecDigit.begin() + iIntegerPartLen + iFractionPartLen;
	m_iFractionPartLength = ModMax(pOperand1->m_iFractionPartLength, pOperand2->m_iFractionPartLength); 
	m_iIntegerPartLength = iIntegerPartLen * _iDigitPerDecimal;

	if (bTruncated)
	{
		if (m_iFractionPartLength > iFractionPartLen * _iDigitPerDecimal)
			m_iFractionPartLength = iFractionPartLen * _iDigitPerDecimal ;
		if (iFractionPartLen1 > iFractionPartLen)
			iFractionPartLen1 = iFractionPartLen;
		if (iFractionPartLen2 > iFractionPartLen)
			iFractionPartLen2 = iFractionPartLen;
		if (iIntegerPartLen2 > iIntegerPartLen1)
			iIntegerPartLen2 = iIntegerPartLen1;
	}

	// the calc has three parts: max(iFractionPartLen)...min(iFractionPartLen), min(iFractionPartLen)..iIntegerPartLen2
	//                           and iIntegerPartLen2 ... iIntegerPartLen1
	DecimalData::DigitUnit dCarry = 0;
	// part1 - max(iFractionPartLen)...min(iFractionPartLen)
	if (iFractionPartLen1 > iFractionPartLen2)
	{
		buf1 = start1 + iIntegerPartLen1 + iFractionPartLen1;
		stop1 = start1 + iIntegerPartLen1 + iFractionPartLen2;
		buf2 = start2 + iIntegerPartLen2 + iFractionPartLen2;
		while (iFractionPartLen-- > iFractionPartLen1) // if the result has more fraction digits
		{
			*--buf = 0;
		}
		while (buf1 > stop1)
		{
			*--buf = *--buf1;
		}
	}
	else //if (iFractionPartLen1 <= iFractionPartLen2)
	{
		buf1 = start1 + iIntegerPartLen1 + iFractionPartLen1;
		buf2 = start2 + iIntegerPartLen2 + iFractionPartLen2;
		stop2 = start2 + iIntegerPartLen2 + iFractionPartLen1;
		while (iFractionPartLen-- > iFractionPartLen2)
		{
			*--buf = 0;
		}
		while (buf2 > stop2)
		{
			const DigitUnit zeroDec = 0;
			*--buf = _subSub(zeroDec, *--buf2, dCarry);
		}
	}

	//part2 min(iFractionPartLen)..iIntegerPartLen2
	while (buf2 > start2)
	{
		*--buf = _subSub(*--buf1, *--buf2, dCarry);
	}

	//part3 iIntegerPartLen2 ... iIntegerPartLen1
	while (dCarry > 0 && buf1 > start1)
	{
		const DigitUnit zeroDec = 0;
		*--buf = _subSub(*--buf1, zeroDec, dCarry);
	}
	while (buf1 > start1)  
	{
		*--buf = *--buf1;
	}

	while (buf > m_vecDigit.begin()) // fill the heading zeroes
	{
		*--buf = 0;
	}

	setSign(bIsNegative);
	return true;
}

//	FUNCTION public
//	Common::DecimalData::Decimal::doMul -- multiply two decimals
//
//	NOTES
//  multiply two decimals
//
//	ARGUMENTS
//	DecimalData::Decimal& cDecimal1_
//         operand1
//	DecimalData::Decimal& cDecimal2_
//         operand2
//  bool bNoThrow_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
DecimalData::Decimal::
doMul(const DecimalData::Decimal& cDecimal1_, const DecimalData::Decimal& cDecimal2_)//the memory size of result.m_dBuffer
{
	int iIntegerPartLen1 = _roundUp(cDecimal1_.m_iIntegerPartLength);
	int iIntegerPartLen2 = _roundUp(cDecimal2_.m_iIntegerPartLength);
	int iFractionPartLen1 = _roundUp(cDecimal1_.m_iFractionPartLength);
	int iFractionPartLen2 = _roundUp(cDecimal2_.m_iFractionPartLength);
	int iIntegerPartLen = _roundUp(cDecimal1_.m_iIntegerPartLength + cDecimal2_.m_iIntegerPartLength);
	int iFractionPartLen = iFractionPartLen1 + iFractionPartLen2;

	int iBufferLength = m_vecDigit.getSize();
	;_TRMEISTER_ASSERT(iBufferLength > 0);

	int adjustedIntegerPartLen = iIntegerPartLen;
	int adjusetedFractionPartLen = iFractionPartLen;

	bool bTruncated = true;
	if ((iIntegerPartLen + iFractionPartLen) > iBufferLength )  
	{                                                             
		if (iIntegerPartLen > iBufferLength)                                
		{                                                         
			return false;
		}                                                           
		else                                                        
		{                                                           
			iFractionPartLen = iBufferLength - iIntegerPartLen;                                        
			bTruncated = false;
		}                                                           
	}                                                          

	m_iFractionPartLength = cDecimal1_.m_iFractionPartLength + cDecimal2_.m_iFractionPartLength;
	m_iIntegerPartLength = iIntegerPartLen * _iDigitPerDecimal;

	if (!bTruncated)
	{
		if (m_iFractionPartLength > iFractionPartLen * _iDigitPerDecimal)
			m_iFractionPartLength = iFractionPartLen * _iDigitPerDecimal;
		if (m_iIntegerPartLength > iIntegerPartLen * _iDigitPerDecimal)
			m_iIntegerPartLength = iIntegerPartLen * _iDigitPerDecimal;

		if (adjustedIntegerPartLen > iIntegerPartLen)
		{
			adjustedIntegerPartLen -= iIntegerPartLen;
			adjusetedFractionPartLen = adjustedIntegerPartLen >> 1;
			iIntegerPartLen1 -= adjusetedFractionPartLen;
			iIntegerPartLen2 -= adjustedIntegerPartLen - adjusetedFractionPartLen;
			iFractionPartLen1 = iFractionPartLen2 = 0; //iFractionPartLen = 0 here
		}
		else
		{
			adjusetedFractionPartLen -= iFractionPartLen;
			adjustedIntegerPartLen = adjusetedFractionPartLen >> 1;
			iFractionPartLen1 -= adjustedIntegerPartLen;
			iFractionPartLen2 -= adjusetedFractionPartLen - adjustedIntegerPartLen;
		}
	}

	ModVector<DecimalData::DigitUnit>::ConstIterator buf1 =
		cDecimal1_.m_vecDigit.begin() + iIntegerPartLen1 + iFractionPartLen1 - 1;
	ModVector<DecimalData::DigitUnit>::ConstIterator stop1 = cDecimal1_.m_vecDigit.begin();
	ModVector<DecimalData::DigitUnit>::ConstIterator buf2 =
		cDecimal2_.m_vecDigit.begin() + iIntegerPartLen2 + iFractionPartLen2 - 1;;
	ModVector<DecimalData::DigitUnit>::ConstIterator start2 = buf2;
	ModVector<DecimalData::DigitUnit>::ConstIterator stop2 = cDecimal2_.m_vecDigit.begin();
	
	ModVector<DecimalData::DigitUnit>::Iterator buf = m_vecDigit.begin();
	ModVector<DecimalData::DigitUnit>::Iterator start =
		m_vecDigit.begin() + iIntegerPartLen + iFractionPartLen - 1;
	ModFill(m_vecDigit.begin(), m_vecDigit.end(), 0);
	
	DecimalData::DigitUnit dCarry;
	for (; buf1 >= stop1; buf1--, start--)
	{
		dCarry = 0;

		for (buf = start, buf2 = start2; buf2 >= stop2; buf2--, buf--)
		{
			DecimalData::DigitUnit highDigitUnit;
			DecimalData::DigitUnit lowDigitUnit;
			DecimalData::DoubleDigitUnit twoUnits = static_cast<DecimalData::DoubleDigitUnit>(*buf1) * static_cast<DecimalData::DoubleDigitUnit>(*buf2);
			highDigitUnit = static_cast<DecimalData::DigitUnit>(twoUnits / _iDigitBase);
			lowDigitUnit = static_cast<DecimalData::DigitUnit>(twoUnits - static_cast<DecimalData::DoubleDigitUnit>(highDigitUnit) * _iDigitBase);
			DecimalData::DigitUnit tmpUnit = *buf + lowDigitUnit + dCarry;
			dCarry = (tmpUnit >= _iDigitBase);
			if (dCarry > 0)
			{
				tmpUnit -= _iDigitBase;
			}
			if (tmpUnit >= _iDigitBase)
			{                                                 
				tmpUnit -= _iDigitBase;                        
				dCarry++;                  
			}                
			*buf = tmpUnit;      
			dCarry += highDigitUnit;
		}

		for (; dCarry > 0; buf--) // if there is carry
		{
			if (buf < m_vecDigit.begin())
			{
				_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
			}
			DecimalData::DigitUnit tmpUnit = *buf + dCarry;
			dCarry = (tmpUnit >= _iDigitBase);
			if (dCarry > 0)
			{
				tmpUnit -= _iDigitBase;
			}
			*buf = tmpUnit;
		}
	}

	//check for -0.00 case
	if (m_bIsNegative)
	{
		ModVector<DecimalData::DigitUnit>::Iterator buf3 = m_vecDigit.begin();
		ModVector<DecimalData::DigitUnit>::Iterator end3 = m_vecDigit.begin() + iIntegerPartLen + iFractionPartLen;
		;_TRMEISTER_ASSERT(buf3 != end3);

		for (;;)
		{
			if (*buf3)
				break;
			if (++buf3 == end3)
			{
				makeZero(1, m_iFractionPartLength);
				break;
			}
		}
	}

	buf1 = m_vecDigit.begin();
	int iDigitMoved = iIntegerPartLen +_roundUp(m_iFractionPartLength);
	while (!*buf1 && (m_iIntegerPartLength > _iDigitPerDecimal))
	{
		buf1++;
		m_iIntegerPartLength -= _iDigitPerDecimal;
		iDigitMoved--;
	}
	if (m_vecDigit.begin() < buf1)
	{
		ModVector<DecimalData::DigitUnit>::Iterator curDigitUnit = m_vecDigit.begin();
		for (; iDigitMoved--; curDigitUnit++, buf1++)
		{
			*curDigitUnit = *buf1;
		}
	}
	
	setSign(cDecimal1_.m_bIsNegative != cDecimal2_.m_bIsNegative);
	return true;
}

//	FUNCTION public
//	Common::DecimalData::Decimal::doDiv -- div two decimals
//
//	NOTES
//  div two decimals
//
//	ARGUMENTS
//	DecimalData::Decimal& cDecimal1_
//         operand1
//	DecimalData::Decimal& cDecimal2_
//         operand2
//  bool bNoThrow_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
DecimalData::Decimal::
doDiv(DecimalData::Decimal& cDecimal1_, const DecimalData::Decimal& cDecimal2_)//the memory size of result.m_dBuffer
{
	int iFractionPartLen1 = _roundUp(cDecimal1_.m_iFractionPartLength) * _iDigitPerDecimal;
	int iPrecision1 = cDecimal1_.m_iIntegerPartLength + iFractionPartLen1;
	int iFractionPartLen2 = _roundUp(cDecimal2_.m_iFractionPartLength) * _iDigitPerDecimal;
	int iPrecision2 = cDecimal2_.m_iIntegerPartLength + iFractionPartLen2;

	ModVector<DecimalData::DigitUnit>::Iterator buf1 = cDecimal1_.m_vecDigit.begin();
	ModVector<DecimalData::DigitUnit>::ConstIterator buf2 = cDecimal2_.m_vecDigit.begin();

	int iBufferLength = m_vecDigit.getSize();
	;_TRMEISTER_ASSERT(iBufferLength > 0);

	//remove the leading zeros
	//1. remove leading zeroes based on _iDigitPerDecimal digits
	int iZeroesRemoved = ((iPrecision2 - 1 ) % _iDigitPerDecimal) + 1; //[1,9] not [0,8]
	while (iPrecision2 > 0 && (*buf2 == 0))
	{
		iPrecision2 -= iZeroesRemoved;
		iZeroesRemoved = _iDigitPerDecimal;
		buf2++;
	}
	if (iPrecision2 <= 0)
	{
		_TRMEISTER_THROW0(Exception::DivisionByZero);
	}
	//2. remove leading zeroes based on one digit
	for (iZeroesRemoved = ((iPrecision2 - 1) % _iDigitPerDecimal);
		 *buf2 < _powers10[iZeroesRemoved--]; iPrecision2--); 
	;_TRMEISTER_ASSERT(iPrecision2 > 0);

	iZeroesRemoved = ((iPrecision1 - 1 ) % _iDigitPerDecimal) + 1;
	while (iPrecision1 > 0 && *buf1 == 0)
	{
		iPrecision1 -= iZeroesRemoved;
		iZeroesRemoved = _iDigitPerDecimal;
		buf1++;
	}
	if (iPrecision1 <= 0)
	{
		makeZero(1, cDecimal2_.m_iFractionPartLength);
		return true;
	}
	for (iZeroesRemoved = ((iPrecision1 - 1) % _iDigitPerDecimal);
		 *buf1 < _powers10[iZeroesRemoved--]; iPrecision1--);
	;_TRMEISTER_ASSERT(iPrecision1 > 0);

	int iIntegerPartLen;
	int iIntegerNum = (iPrecision1 - iFractionPartLen1) - (iPrecision2 - iFractionPartLen2) + (*buf1 >= *buf2);
	if (iIntegerNum < 0)
	{
		iIntegerNum /= _iDigitPerDecimal;
		iIntegerPartLen = 0;
	}
	else
		iIntegerPartLen = _roundUp(iIntegerNum);
	
	int iFractionPartLen = _roundUp(cDecimal1_.m_iFractionPartLength + cDecimal2_.m_iIntegerPartLength + 1);
										//s1-s2+p2+1

	// calculate N1/N2
	//	N1 is in the buf1 and has iPrecision1 digits
	//	N2 is in the buf2 and has iPrecision2 digits
	//scales are iFractionPartLen1 and iFractionPartLen accordingly.
	//so the result will have
	//iIntegerPartLen = iPrecision1 - iFractionPartLength1 + iFractionPartLength2;
	//iFractionPartLen = iFractionPartLength1 - iFractionPartLength2 + iPrecision2 + 1;

	m_iIntegerPartLength = iIntegerPartLen * _iDigitPerDecimal;
	m_iFractionPartLength = cDecimal1_.m_iFractionPartLength + cDecimal2_.m_iIntegerPartLength + 1;
	ModVector<DecimalData::DigitUnit>::Iterator buf = m_vecDigit.begin();
	ModVector<DecimalData::DigitUnit>::Iterator stop = buf + iIntegerPartLen + iFractionPartLen;

	while (iIntegerNum++ < 0)
		*buf++ = 0;

	int iLength1 = _roundUp(iPrecision1) + _roundUp(2 * iFractionPartLen2 + 1) + 1;
	iLength1 = (iLength1 >= 3) ? iLength1 : 3;

	ModVector<DecimalData::DigitUnit> vecTmp;
	vecTmp.reserve(iLength1);
	for (int i = 0; i < _roundUp(iPrecision1); i++)
		vecTmp.pushBack(buf1[i]);
	for (int i = _roundUp(iPrecision1); i < iLength1; i++)
		vecTmp.pushBack(0);
	
	ModVector<DecimalData::DigitUnit>::Iterator start1 = vecTmp.begin();
	ModVector<DecimalData::DigitUnit>::Iterator stop1 = start1 + iLength1;
	ModVector<DecimalData::DigitUnit>::ConstIterator start2 = buf2;
	ModVector<DecimalData::DigitUnit>::ConstIterator stop2 = buf2 + _roundUp(iPrecision2) - 1;

	//remove tailing zeroes of cDecimal2_
	while (*stop2 == 0 && stop2 >= start2)
		stop2--;
	int iLength2 = stop2++ - start2;
	/*
    calculating normValue2 (normalized *start2) - we need *start2 to be large
    (at least > DIG_BASE/2), but unlike Knuth's Alg. D we don't want to
    normalize input numbers (as we don't make a copy of the divisor).
    Thus we normalize first dec1 of buf2 only, and we'll normalize *start1
    on the fly for the purpose of guesstimation only.
    It's also faster, as we're saving on normalization of buf2
	 */
	DecimalData::DoubleDigitUnit normFactor = _iDigitBase/(*start2 + 1);
	DecimalData::DigitUnit normValue2 = static_cast<DecimalData::DigitUnit>(normFactor * start2[0]);
	if (iLength2 > 0)
		normValue2 += static_cast<DecimalData::DigitUnit>(normFactor * start2[1]/_iDigitBase);

	int iLoop = 0;
	int dcarry;
	if (*start1 < *start2) {//not enough
		dcarry = *start1++;
		iLoop++;
	} else
		dcarry = 0;

	DecimalData::DoubleDigitUnit firstUnit1;
	DecimalData::DoubleDigitUnit secondUnit1;
	DecimalData::DoubleDigitUnit guessUnitValue;
	DecimalData::DigitUnit carry;
	//  main loop 
	while (buf < stop)
	{
		//  short-circuit, if possible 
		if (iLoop >= iLength1)
		{
			*buf++ = 0;
			iLoop++;
		}
		else if (dcarry == 0 && *start1 < *start2)
		{
			*buf++ = 0;
			dcarry = *start1;
			start1++;
			iLoop++;
		}
		else
		{
			//D3: make a guess
			firstUnit1 = start1[0] + static_cast<DecimalData::DoubleDigitUnit>(dcarry) * _iDigitBase;
			if (start1 + 1 < stop1) {
				secondUnit1 = start1[1];
			} else {
				secondUnit1 = 0;
			}
			guessUnitValue = (normFactor * firstUnit1 + normFactor * secondUnit1 / _iDigitBase) / normValue2;
			if (guessUnitValue >= _iDigitBase)
				guessUnitValue = _iDigitBase - 1;
			if (iLength2 > 0 && start2 + 1 < stop2)
			{
				//  this is a suspicious trick - normalization is removed here 
				if (start2[1] * guessUnitValue > (firstUnit1 - guessUnitValue * start2[0]) * _iDigitBase + secondUnit1)
				{
					guessUnitValue--;
				}
				if (start2[1] * guessUnitValue > (firstUnit1 - guessUnitValue * start2[0]) * _iDigitBase + secondUnit1)
				{
					guessUnitValue--;
				}
				;_TRMEISTER_ASSERT(start2[1] * guessUnitValue <= (firstUnit1 - guessUnitValue*start2[0]) * _iDigitBase + secondUnit1);
			}
			// D4: multiply and subtract 
			buf2 = stop2;
			buf1 = start1 + iLength2;
			//;_TRMEISTER_ASSERT(buf1 < stop1);
			if (buf1 >= stop1)
			{			
				*buf++ = static_cast<DecimalData::DigitUnit>(guessUnitValue);
				dcarry = *start1;		
				start1++;
				iLoop++;
				continue;
			}

			for (carry = 0; buf2 > start2; buf1--)
			{
				DecimalData::DigitUnit highDigitUnit;
				DecimalData::DigitUnit lowDigitUnit;

				firstUnit1 = guessUnitValue * (*--buf2);
				highDigitUnit = static_cast<DecimalData::DigitUnit>(firstUnit1 / _iDigitBase);
				lowDigitUnit = static_cast<DecimalData::DigitUnit>(firstUnit1 
					                      - static_cast<DecimalData::DoubleDigitUnit>(highDigitUnit) * _iDigitBase);

				DecimalData::DigitUnit tmpUnit = *buf1 - lowDigitUnit - carry; 
				carry = (tmpUnit < 0);
				if (carry > 0)                                         
					tmpUnit += _iDigitBase;                                                
				if (tmpUnit < 0)                                          
				{                                                            
					tmpUnit += _iDigitBase;                                               
					carry++;                            
				}                                                            
				*buf1 = tmpUnit;     

				carry += highDigitUnit;
			}

			carry = dcarry < carry;
			// D5: check the remainder 
			if (carry > 0)
			{
				//  D6: correct the guessUnitValue */
				guessUnitValue--;
				buf2 = stop2;
				buf1 = start1 + iLength2;
				for (carry = 0; buf2 > start2; buf1--)
				{
					*buf1 = _subAdd(*buf1, *--buf2,carry);
				}
			}
			*buf++ = static_cast<DecimalData::DigitUnit>(guessUnitValue);
			dcarry = *start1;		
			start1++;
			iLoop++;
		}
	}

	setSign(cDecimal1_.m_bIsNegative != cDecimal2_.m_bIsNegative);

	return true;
}

//
//	FUNCTION public
//	Common::DecimalData::Decimal::serialize_NotNull -- Making to cereal
//
//	NOTES
//  Making to cereal 
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		Archiver
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
//virtual
void 
DecimalData::Decimal::
serialize(ModArchive& archiver_)
{
	if (archiver_.isStore())
	{
		archiver_ << m_iIntegerPartLength;
		archiver_ << m_iFractionPartLength;
		archiver_ << m_bIsNegative;

		ModSize size = m_vecDigit.getSize();
		archiver_ << size;
		for (ModSize i = 0; i < size; i++)
		{
			archiver_ << m_vecDigit[i];
		}
	}
	else
	{
		archiver_ >> m_iIntegerPartLength;
		archiver_ >> m_iFractionPartLength;

		bool bIsNegative = false;
		archiver_ >> bIsNegative;

		ModSize size = 0;
		archiver_ >> size;
		resetDigitVector(size);

		for (ModSize i = 0; i < size; i++)
		{
			DecimalData::DigitUnit digit = 0;
			archiver_ >> digit;
			m_vecDigit[i] = digit;
		}

		setSign(bIsNegative);
	}
}

// FUNCTION public
//	Common::DecimalData::Decimal::hashCode -- take hash code
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

ModSize
DecimalData::Decimal::
hashCode() const
{
	ModSize hashCode = (m_iIntegerPartLength << 2)
		+ m_iFractionPartLength
		+ (m_bIsNegative ? 1 : 0);

	ModSize hashValue = 0;
	ModSize g;

	ModVector<DigitUnit>::ConstIterator iterator = m_vecDigit.begin();
	const ModVector<DigitUnit>::ConstIterator last = m_vecDigit.end();
	while (iterator != last) {
		hashValue <<= 4;
		hashValue += static_cast<ModSize>(*iterator);
		if (g = hashValue & ((ModSize) 0xf << (ModSizeBits - 4))) {
			hashValue ^= g >> (ModSizeBits - 8);
			hashValue ^= g;
		}
		++iterator;
	}
	return hashValue;
}

//
//	FUNCTION public
//	Common::DecimalData::DecimalData -- Constructor
//
//	NOTES
//	Constructor
//
//	ARGUMENTS
//	int prec
//		the precision
//	int scale
//		the scale
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
DecimalData::
DecimalData()
: ScalarData(DataType::Decimal),
  m_dValue(),
  m_iPrecision(0),
  m_iScale(0)
{
}

DecimalData::
DecimalData(int iPrecision_, int iScale_)
: ScalarData(DataType::Decimal)
{
	setRange(iPrecision_, iScale_);
}

//
//	FUNCTION public
//	Common::DecimalData::DecimalData -- Constructor
//
//	NOTES
//	Constructor
//
//	ARGUMENTS
//	const Decimal & dec
//		anohter DecialData
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
DecimalData::
DecimalData(const DecimalData& dec_)
: ScalarData(DataType::Decimal)
{
	setValue(dec_);
}

//
//	FUNCTION public
//	Common::DecimalData::~DecimalData -- Destructor
//
//	NOTES
//	Destructor
//
//	ARGUMENTS
//	None
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
//virtual
DecimalData::
 ~DecimalData()
{
}

//
//	FUNCTION public
//	Common::DecimalData::getValue -- The value of a decimal object is obtained
//
//	NOTES
//	The value of a decimal object is obtained
//
//	ARGUMENTS
//	None
//
//	RETURN
//	Decimal
//		data
//
//	EXCEPTIONS
//		Exception::NullNotAllowed       
//			Oneself is NULL value.

const DecimalData::Decimal&
DecimalData::
getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_dValue;
}

//
//	FUNCTION public
//	Common::DecimalData::getPrecision -- The precision of a decimal object is obtained
//
//	NOTES
//	The precision of a decimal object is obtained
//
//	ARGUMENTS
//	None
//
//	RETURN
//	int
//		the value of precision
//

int 
DecimalData::
getPrecision() const
{
	return m_iPrecision;
}

//
//	FUNCTION public
//	Common::DecimalData::getScale -- The scale of a decimal object is obtained
//
//	NOTES
//	The scale of a decimal object is obtained
//
//	ARGUMENTS
//	None
//
//	RETURN
//	int
//		the value of scale
//


int 
DecimalData::
getScale() const
{
	return m_iScale;
}


//static
ModSize
DecimalData::
getDumpSizeBy(int iPrecision_, int iScale_)
{
	return _fourBytesRoundUp(getRealDumpSize_NotNull(iPrecision_, iScale_));
}


//
//	FUNCTION public
//	Common::DecimalData::setValue -- set the value
//
//	NOTES
//	set the value
//
//	ARGUMENTS
//	int prec_
//		the precision of the data
//  int scale_
//		the scale of the data
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
void 
DecimalData::
setRange(const int iPrecision_, const int iScale_)
{
	m_iPrecision = iPrecision_;
	m_iScale = iScale_;

	m_dValue.setRange(iPrecision_ - iScale_, iScale_);
}

//
//	FUNCTION public
//	Common::DecimalData::setValue -- set the value
//
//	NOTES
//	set the value
//
//	ARGUMENTS
//	Decimal &dec_
//		reference to another decimal object
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
void 
DecimalData::
setValue(const DecimalData& cDecimalData_)
{
	//m_iPrecision = ModMin(dec_.m_iPrecision,m_iPrecision);
	//m_iScale = ModMin(dec_.m_iScale,m_iScale);
	m_iPrecision = cDecimalData_.m_iPrecision;
	m_iScale = cDecimalData_.m_iScale;
	m_dValue = cDecimalData_.m_dValue;
	setNull(false);
}

void 
DecimalData::
setToMaxDecimalData(int iPrecision_, int iScale_)
{
	m_iPrecision = iPrecision_;
	m_iScale = iScale_;

	int iInteger = iPrecision_ - iScale_;
	int iFraction = iScale_;
	if (iScale_ < 0)
		iFraction = 0;

	m_dValue.setToMaxDecimal(iInteger, iFraction);
	setNull(false);
}

void
DecimalData::
setToMinDecimalData(int iPrecision_, int iScale_)
{
	m_iPrecision = iPrecision_;
	m_iScale = iScale_;

	int iInteger = iPrecision_ - iScale_;
	int iFraction = iScale_;
	if (iScale_ < 0)
		iFraction = 0;

	m_dValue.setToMaxDecimal(iInteger, iFraction);
	m_dValue.setSign(true);
	setNull(false);
}

//
//	FUNCTION public
//	Common::DecimalData::operator = --operator overloading
//
//	NOTES
//  operator overloading
//
//	ARGUMENTS
//	DecimalData& cDecimalData_
//		the referent data
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
DecimalData& 
DecimalData::
operator =(const DecimalData& cDecimalData_)
{
	if (&cDecimalData_ != this)
		setValue(cDecimalData_);

	return *this;
}

// FUNCTION public
//	Common::DecimalData::isCompatible -- 型の互換性を得る
//
// NOTES
//
// ARGUMENTS
//	const Data* r
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
DecimalData::
isCompatible(const Data* r) const
{
	if (getType() == r->getType()) {
		const DecimalData* p = _SYDNEY_DYNAMIC_CAST(const DecimalData*, r);
		return getPrecision() == p->getPrecision()
			&& getScale() == p->getScale();
	}
	return false;
}

// FUNCTION public
//	Common::DecimalData::getSQLType -- virtual 
//
// NOTES
//
// ARGUMENTS
//	SQLData& cResult_
//	
// RETURN
//	bool 
//
// EXCEPTIONS

//virtual 
bool 
DecimalData::
getSQLType(SQLData& cResult_)
{
	;_TRMEISTER_ASSERT(getType() == DataType::Decimal);
	cResult_.setType(SQLData::Type::Decimal);
	cResult_.setFlag(Common::SQLData::Flag::Fixed); //variable or fixed
	cResult_.setLength(m_iPrecision);
	cResult_.setScale(m_iScale);
	return true; 
}

// FUNCTION public
//	Common::DecimalData::setSQLType -- クラス内部構造をSQLTypeに合わせる
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
DecimalData::
setSQLType(const SQLData& cType_)
{
	if (cType_.getType() == SQLData::Type::Decimal) {
		setRange(cType_.getLength(), cType_.getScale());
	}
}

//static 
bool 
DecimalData::
getOperationPrecisionScale(int iPrecision1_, int iScale1_, int iPrecision2_, int iScale2_, int& iPrecision_, int& iScale_,
						   Common::DataOperation::Type eOperation)
{
	int iIntgerPartLen = 0;
	switch(eOperation) 
	{
	case DataOperation::Addition:
	case DataOperation::Subtraction:
		iIntgerPartLen = ModMax(iPrecision1_ - iScale1_ , iPrecision2_ - iScale2_) + 1;
		iPrecision_ = iIntgerPartLen + ModMax(iScale1_, iScale2_);
		iScale_ = ModMax(iScale1_, iScale2_);
		if (iPrecision_ > SQLData::getMaxPrecision(SQLData::Type::Decimal))
		{
			iScale_ = ModMax(0, SQLData::getMaxPrecision(SQLData::Type::Decimal) - (iPrecision_ - iScale_));
			iPrecision_ = SQLData::getMaxPrecision(SQLData::Type::Decimal);
		}
		break;
	case DataOperation::Multiplication:
		iPrecision_ = iPrecision1_ + iPrecision2_;
		iScale_ = iScale1_ + iScale2_;
		if (iPrecision_ > SQLData::getMaxPrecision(SQLData::Type::Decimal))
		{
			iScale_ = ModMax(0, SQLData::getMaxPrecision(SQLData::Type::Decimal) - (iPrecision_ - iScale_));
			iPrecision_ = SQLData::getMaxPrecision(SQLData::Type::Decimal);
		}
		break;
	case DataOperation::Division:
		iPrecision_ = iPrecision1_ + iPrecision2_ + 1;
		iScale_ = iScale1_ - iScale2_ + iPrecision2_ + 1;
		if (iPrecision_ > SQLData::getMaxPrecision(SQLData::Type::Decimal))
		{
			iScale_ = ModMax(0, SQLData::getMaxPrecision(SQLData::Type::Decimal) - (iPrecision_ - iScale_));
			iPrecision_ = SQLData::getMaxPrecision(SQLData::Type::Decimal);
		}
		break;
	default:
		; _TRMEISTER_ASSERT(false);
		return false;
	}

	return true;
}

bool 
DecimalData::
castFromInteger64(const Integer64Data& cData_, bool bForAssign_)
{
	ModInt64 llValue = cData_.getValue();
	_castFromInt(llValue, m_dValue, m_iPrecision, m_iScale, bForAssign_);
	return true;
}

//
//	FUNCTION public
//	Common::DecimalData::castFromInteger -- the value is got by casting from integer.
//
//	NOTES
//  the value is got by casting from integer.
//
//	ARGUMENTS
//	IntegerData& cData_
//           the integer data
//  bool bForAssign_
//       false: no exception thrown; true:throw exception
//
//	RETURN
//	bool
//		true
//          The operation result is not NULL. 
//      false
//          The operation result is NULL.    
//
//	EXCEPTIONS
//	None
//
bool 
DecimalData::
castFromInteger(const IntegerData& cData_,bool bForAssign_)
{
	int iValue = cData_.getValue();
	_castFromInt(iValue, m_dValue, m_iPrecision, m_iScale, bForAssign_);
	return true;
}
//
//	FUNCTION public
//	Common::DecimalData::castFromUnsignedInteger -- the value is got by casting from unsigned integer.
//
//	NOTES
//  the value is got by casting from unsigned integer.
//
//	ARGUMENTS
//	UnsignedIntegerData& cData_
//           the unsigned integer data
//  bool bForAssign_
//       false: no exception thrown; true:throw exception
//
//	RETURN
//	bool
//		true
//          The operation result is not NULL. 
//      false
//          The operation result is NULL.    
//
//	EXCEPTIONS
//	None
//
bool
DecimalData::
castFromUnsignedInteger(const UnsignedIntegerData& cData_,bool bForAssign_)
{
	unsigned int iValue = cData_.getValue();
	_castFromInt(iValue, m_dValue, m_iPrecision, m_iScale, bForAssign_);
	m_dValue.setSign(false);
	return true;
}

//
//	FUNCTION public
//	Common::DecimalData::castFromString -- the value is got by casting from string.
//
//	NOTES
//  the value is got by casting from string.
//
//	ARGUMENTS
//	ModUnicodeChar* pHead_
//         the head of the string
//  ModUnicodeChar* pTail_
//         the tail of the string
//  bool bForAssign_
//       false: no exception thrown; true:throw exception
//
//	RETURN
//	bool
//		true
//          The operation result is not NULL. 
//      false
//          The operation result is NULL.    
//
//	EXCEPTIONS
//	None
//
bool
DecimalData::
castFromString(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_, bool bForAssign_)
{
	const ModUnicodeChar* p = pHead_;
	const ModUnicodeChar* q = pTail_;
	ModSize n = static_cast<ModSize>(q-p);
	while (n > 1 && *p == UnicodeChar::usSpace) {
		++p; --n;
	}
	while (n > 1 && *(q-1) == UnicodeChar::usSpace) {
		--q; --n;
	}
	
	bool isNegative = false;//+
	if (n > 1) 
	{
		if (*p == UnicodeChar::usPlus || *p == UnicodeChar::usHyphen) 
		{
			if (*p == UnicodeChar::usHyphen)
				isNegative = true;
			++p;
			--n;
		}
	}

	const ModUnicodeChar* pBuffer = p;
	const ModUnicodeChar* period = 0;
	const ModUnicodeChar * fractionEnd = q;
	ModUnicodeString indexStr;
	int iIntegerLength = 0;
	int iFractionLength = 0;
	bool bZero = true;
	bool bExponent = false;
	do {
		if ((*p == UnicodeChar::usLargeE || *p == UnicodeChar::usSmallE)
			&& !bExponent && n > 1) 
		{
			bExponent = true;
			fractionEnd = p;
			++p;
			--n;
			if (n > 1 && (*p == UnicodeChar::usPlus || *p == UnicodeChar::usHyphen)) 
			{
				indexStr.append(*p);
				++p;
				--n;
			}
		}
		if (*p == UnicodeChar::usPeriod && period == 0 && !bExponent) 
		{
			period = p;
			++p;
			--n;
		}

		if (ModUnicodeCharTrait::isDigit(*p)) 
		{
			if (bExponent)
				indexStr.append(*p);
			else
			{
				if (!period)
				{
					if (iIntegerLength > 0 || *p != UnicodeChar::usZero)
						iIntegerLength++;
					else
						pBuffer++;
				}
				else //if (!bExponent)
					iFractionLength++;
				if (bZero && (*p != UnicodeChar::usZero))
					bZero = false;
			}
		}
		else
		{
			if (bForAssign_)
				_TRMEISTER_THROW0(Exception::InvalidCharacter);
			else
				return false;
		}
		++p;
	} while ((--n > 0)&&(p<=q));

	int iIndex = ModUnicodeCharTrait::toInt(indexStr);
	if (bZero)
	{
		if ((0 == m_dValue.getInteger()) && (0 == m_dValue.getFraction()))
		{
			m_dValue.setRange(1, 0);
			m_iPrecision = 1;
			m_iScale = 0;
		}

		m_dValue.makeZero(m_dValue.getInteger(), m_dValue.getFraction());
		return true;
	}

	//the real integer part and fraction part
	bool bIntegerPartIsZero = false;
	if (iIndex < 0  && iIntegerLength <= (-iIndex))
	{
		bIntegerPartIsZero = true;
	}

	int iIntegerDigit = (bIntegerPartIsZero? 1: iIntegerLength+iIndex);
	int iFractionDigit = (iFractionLength < iIndex)? 0: iFractionLength - iIndex;

	ModUnicodeString strDigits;
	strDigits.reallocate(iIntegerDigit + iFractionDigit);
	if (bIntegerPartIsZero)
	{
		strDigits.append(UnicodeChar::usZero);
		for (int j=0; j<-iIndex-iIntegerLength; j++)
			strDigits.append(UnicodeChar::usZero);
	}

	if (period == 0)
		strDigits.append(pBuffer, static_cast<ModSize>(fractionEnd-pBuffer));
	else
	{
		if (pBuffer != period)
			strDigits.append(pBuffer, static_cast<ModSize>(period-pBuffer));
		strDigits.append(period+1, static_cast<ModSize>(fractionEnd-period-1));
	}
	for (int j=0; j<iIndex - iFractionLength; j++)
		strDigits.append(UnicodeChar::usZero);

	const ModUnicodeChar* integerStart = &strDigits[0];
	const ModUnicodeChar * integerEnd = integerStart + iIntegerDigit;
	const ModUnicodeChar * fractionStart = integerEnd;
	fractionEnd = &strDigits[iIntegerDigit + iFractionDigit-1];

	// if not predefined the precision and scale or no column information
	if ((0 == m_dValue.getInteger()) && (0 == m_dValue.getFraction()))
	{
		m_dValue.setRange(iIntegerDigit, iFractionDigit);
		m_iPrecision = iIntegerDigit + iFractionDigit;
		m_iScale = iFractionDigit;
	}

	int iIntPartNum = m_dValue.getInteger();
	if ((iIntegerDigit > iIntPartNum) && (!bIntegerPartIsZero))
	{
		if (bForAssign_)
			_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
		else
			return false;
	}

	int iFracPartNum = m_dValue.getFraction();

	// transfer to memory buffer
	int integerDec = _roundUp(iIntPartNum);
	int fractionDec = _roundUp(iFracPartNum);

	DecimalData::DigitUnit unitValue;
	ModVector<DecimalData::DigitUnit>::Iterator buf = m_dValue.getDigitVector().begin() + integerDec;
	int i=0;
	for (unitValue = 0, i = 0; (iIntPartNum>0 && integerEnd>integerStart); iIntPartNum--)
	{
		unitValue += ((*--integerEnd) - UnicodeChar::usZero)*_powers10[i];
		if (++i == _iDigitPerDecimal)
		{
			*--buf = unitValue;
			unitValue = 0, i = 0;
		}
	}
	if (i > 0)
		*--buf = unitValue;

	if (iFracPartNum > 0)
	{
		buf = m_dValue.getDigitVector().begin() + integerDec;
		for (unitValue = 0, i = 0; (iFracPartNum>0 && fractionStart<=fractionEnd); iFracPartNum--)
		{
			unitValue = (*fractionStart++ - UnicodeChar::usZero) + unitValue * 10;

			if (++i == _iDigitPerDecimal)
			{
				*buf++ = unitValue;
				unitValue = 0;
				i = 0;
			}
		}

		if (i)
			*buf = unitValue * _powers10[_iDigitPerDecimal - i];
	}

	m_dValue.setSign(isNegative);
	return true;
}

bool
DecimalData::
castFromString(const char* pHead_, const char* pTail_, bool bForAssign_ )
{
	const char* p = pHead_;
	const char* q = pTail_;
	ModSize n = static_cast<ModSize>(q-p);
	while (n > 1 && *p ==  ' ') {
		++p; --n;
	}
	while (n > 1 && *(q-1) ==  ' ') {
		--q; --n;
	}
	
	bool isNegative = false;//+
	if (n > 1) 
	{
		if (*p == '+'|| *p == '-') 
		{
			if (*p == '-')
				isNegative = true;
			++p;
			--n;
		}
	}

	const char* pBuffer = p;
	const char* period = 0;
	const char* fractionEnd = q;
	ModCharString indexStr;
	int iIntegerLength = 0;
	int iFractionLength = 0;
	bool bZero = true;
	bool bExponent = false;
	do {
		if ((*p == 'E' || *p == 'e') && !bExponent && n > 1) 
		{
			bExponent = true;
			fractionEnd = p;
			++p;
			--n;
			if (n > 1 && (*p == '+' || *p == '-')) 
			{
				indexStr.append(*p);
				++p;
				--n;
			}
		}
		if (*p == '.' && period == 0 && !bExponent) 
		{
			period = p;
			++p;
			--n;
		}

		if (ModCharTrait::isDigit(*p)) 
		{
			if (bExponent)
				indexStr.append(*p);
			else
			{
				if (!period)
				{
					if (iIntegerLength > 0 || *p != '0')
						iIntegerLength++;
					else
						pBuffer++;
				}
				else //if (!bExponent)
					iFractionLength++;
				if (bZero && (*p != '0'))
					bZero = false;
			}
		}
		else
		{
			if (bForAssign_)
				_TRMEISTER_THROW0(Exception::InvalidCharacter);
			else
				return false;
		}
		++p;
	} while ((--n > 0)&&(p<=q));

	if (bZero)
	{
		if ((0 == m_dValue.getInteger()) && (0 == m_dValue.getFraction()))
		{
			m_dValue.setRange(1, 0);
			m_iPrecision = 1;
			m_iScale = 0;
		}

		m_dValue.makeZero(m_dValue.getInteger(), m_dValue.getFraction());
		return true;
	}
	int iIndex = atoi(indexStr);
	//the real integer part and fraction part
	bool bIntegerPartIsZero = false;
	if (iIndex < 0  && iIntegerLength <= (-iIndex))
	{
		bIntegerPartIsZero = true;
	}

	int iIntegerDigit = (bIntegerPartIsZero? 1: iIntegerLength+iIndex);
	int iFractionDigit = (iFractionLength < iIndex)? 0: iFractionLength - iIndex;

	ModCharString strDigits;
	strDigits.reallocate(iIntegerDigit + iFractionDigit);
	if (bIntegerPartIsZero)
	{
		strDigits.append('0');
		for (int j=0; j<-iIndex-iIntegerLength; j++)
			strDigits.append('0');
	}

	if (period == 0)
		strDigits.append(pBuffer, static_cast<ModSize>(fractionEnd-pBuffer));
	else
	{
		if (pBuffer != period)
			strDigits.append(pBuffer, static_cast<ModSize>(period-pBuffer));
		strDigits.append(period+1, static_cast<ModSize>(fractionEnd-period-1));
	}
	for (int j=0; j<iIndex - iFractionLength; j++)
		strDigits.append('0');

	const char* integerStart = &strDigits[0];
	const char*  integerEnd = integerStart + iIntegerDigit;
	const char*  fractionStart = integerEnd;
	fractionEnd = &strDigits[iIntegerDigit + iFractionDigit-1];

	// if not predefined the precision and scale or no column information
	if ((0 == m_dValue.getInteger()) && (0 == m_dValue.getFraction()))
	{
		m_dValue.setRange(iIntegerDigit, iFractionDigit);
		m_iPrecision = iIntegerDigit + iFractionDigit;
		m_iScale = iFractionDigit;
	}

	int iIntPartNum = m_dValue.getInteger();
	if ((iIntegerDigit > iIntPartNum) && (!bIntegerPartIsZero))
	{
		if (bForAssign_)
			_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
		else
			return false;
	}

	int iFracPartNum = m_dValue.getFraction();

	// transfer to memory buffer
	int integerDec = _roundUp(iIntPartNum);
	int fractionDec = _roundUp(iFracPartNum);

	DecimalData::DigitUnit unitValue;
	ModVector<DecimalData::DigitUnit>::Iterator buf = m_dValue.getDigitVector().begin() + integerDec;
	int i;
	for (unitValue = 0, i = 0; (iIntPartNum>0 && integerEnd>integerStart); iIntPartNum--)
	{
		unitValue += ((*--integerEnd) - '0')*_powers10[i];
		if (++i == _iDigitPerDecimal)
		{
			* --buf = unitValue;
			unitValue = 0, i = 0;
		}
	}
	if (i > 0)
		*--buf = unitValue;

	if (iFracPartNum > 0)
	{
		buf = m_dValue.getDigitVector().begin() + integerDec;
		for (unitValue = 0, i = 0; (iFracPartNum>0 && fractionStart<=fractionEnd); iFracPartNum--)
		{
			unitValue = (*fractionStart++ - '0') + unitValue * 10;

			if (++i == _iDigitPerDecimal)
			{
				*buf++ = unitValue;
				unitValue = 0;
				i = 0;
			}
		}

		if (i)
			*buf = unitValue * _powers10[_iDigitPerDecimal - i];
	}

	m_dValue.setSign(isNegative);
	return true;
}

//
//	FUNCTION public
//	Common::DecimalData::castFromDouble -- the value is got by casting from double.
//
//	NOTES
//  the value is got by casting from double.
//
//	ARGUMENTS
//	DoubleData& cData_
//           the double data
//  bool bForAssign_
//       false: no exception thrown; true:throw exception
//
//	RETURN
//	bool
//		true
//          The operation result is not NULL. 
//      false
//          The operation result is NULL.    
//
//	EXCEPTIONS
//	None
//
bool
DecimalData::
castFromDouble(const DoubleData& cData_, bool bForAssign_/* = false*/)
{
	double dblValue = cData_.getValue();
	char buffer[_iDoubleBufferSize];
	int j = ::sprintf(buffer, "%.14E", dblValue);

	return castFromString(buffer, &buffer[j], bForAssign_);
}


//
//	FUNCTION private
//	Common::DecimalData::serialize_NotNull -- Making to cereal
//
//	NOTES
//  Making to cereal 
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		Archiver
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	Others, 
//     The exception of the subordinate position is sent again as it is.
//
//virtual 
void
DecimalData::
serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//Writing
		cArchiver_ << m_iPrecision;
		cArchiver_ << m_iScale;
		cArchiver_ << m_dValue;
	}
	else
	{
		//Reading
		cArchiver_ >> m_iPrecision;
		cArchiver_ >> m_iScale;
		cArchiver_ >> m_dValue;
	}
}

//
//	FUNCTION private
//	Common::DecimaltData::copy_NotNull -- copy function
//
//	NOTES
//	Own copy is returned. 
//
//	ARGUMENTS
//	None
//
//	RETURN
//	Common::Data::Pointer
//		Own copy
//
//	EXCEPTIONS
//	None
//
//virtual 
Data::Pointer
DecimalData::
copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new DecimalData(*this);
}

//	FUNCTION private
//	Common::DecimalData::cast_NotNull --  Cast to a specified type.
//
//	NOTES
//		Cast to a specified type
//
//	ARGUMENTS
//	DataType::Type type
//         the specified type
//  bool 
//       bForAssign_ = false  
//       false: no exceptin information;      true: throw exception 
//
//	RETURN
//	Data::Pointer
//      Data got by casting.
//
//	EXCEPTIONS
//  Exception::NumericValueOutOfRange
//      The range expressible Cast ahead was exceeded.
//      Others
//      The exception of the subordinate position is sent again as it is.

//virtual 
Data::Pointer
DecimalData::
cast_NotNull(DataType::Type eType_, bool bForAssign_ /*= false*/) const
{
	; _TRMEISTER_ASSERT(!isNull());

	bool bInRange = true;
	switch (eType_) // bigint, char, varchar, binary
	{
		case DataType::Integer:
		{
			int iValue = 0;
			if (_castToInt(iValue, m_dValue)) {
				return new IntegerData(iValue);
			}
			break;
		}
	
		case DataType::UnsignedInteger:
		{
			unsigned int uValue = 0;
			if (_castToInt(uValue, m_dValue)) {
				return new UnsignedIntegerData(uValue);
			}
			break;
		}
		
		case DataType::Integer64:
		{
			ModInt64 illValue = 0;
			if (_castToInt(illValue, m_dValue)) {
				return new Integer64Data(illValue);
			}
			break;
		}
		
		case DataType::String: //char varchar
		{
			return new StringData(getString());
		}
		
		case DataType::Double:
		{
			double dblValue = 0;
			// cast integer part
			if (_castToFloat(dblValue, m_dValue,
							 0, m_dValue.getInteger()) == false) {
				break;
			}

			if (m_dValue.getFraction()) {
				// add fraction part
				double dblFractionValue = 0;

				if (_castToFloat(dblFractionValue, m_dValue,
								 m_dValue.getInteger(),
								 m_dValue.getFraction()) == false) {
					break;
				}
				dblFractionValue /=
					Os::Math::pow(_iDigitBase,
								  _roundUp(m_dValue.getFraction()));
				dblValue += dblFractionValue;
			}
			return new DoubleData(dblValue);
		}
		case DataType::Decimal:
			return copy();
			break;
		case DataType::Null:
			return NullData::getInstance();
			break;
		default:
			_TRMEISTER_THROW0(Exception::ClassCast);
			break;
	}

	if (bForAssign_)
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	else
		return NullData::getInstance();
}

//	FUNCTION private
//	Common::DecimalData::getString_NotNull -- get the decimal value by character string
//
//	NOTES
//		get the decimal value by character string.
//
//	ARGUMENTS
//	None
//
//	RETURN
//	ModUnicodeString
//      Return data by character string
//
//	EXCEPTIONS
//		
//virtual 
ModUnicodeString
DecimalData::
getString_NotNull() const//removing the leading zeroes
{
	int iIntegerPartNum = 0; //the length of integer part excluding leading zeroes
	Decimal dValue(m_dValue);//In the following, dValue will be modified and there is no effect on m_dValue

	//remove the leading zeroes
	ModVector<DecimalData::DigitUnit>::Iterator buffer = dValue.removeLeadingZeroes(iIntegerPartNum);

	int iFractionPartLen = m_dValue.getFraction();
	int iLength = m_dValue.isNegative() + (iIntegerPartNum > 0 ? iIntegerPartNum : 1) 
		                                + iFractionPartLen + (iFractionPartLen > 0 ? 1 : 0);

	ModUnicodeString cResult;
	cResult.reallocate(iLength);

	if (m_dValue.isNegative())
		cResult.append(UnicodeChar::usHyphen);

	char chTemp[_iDigitPerDecimal+1];
	ModVector<DecimalData::DigitUnit>::Iterator iterator = buffer;
	ModVector<DecimalData::DigitUnit>::Iterator last = buffer + _roundUp(iIntegerPartNum);
	if (iIntegerPartNum > 0) // the integer part is not zero
	{
		for (iterator; iterator != last; ++iterator)
		{
			if (iterator != buffer)
				::sprintf(chTemp, "%09d", *iterator);
			else
				::sprintf(chTemp, "%d", *iterator);

			cResult += ModUnicodeString(chTemp, _iDigitPerDecimal);
		}
	}
	else
	{
		cResult.append(UnicodeChar::usZero);
	}

	if (iFractionPartLen > 0)
	{
		cResult.append(UnicodeChar::usPeriod);
		
		last = buffer + _roundUp(iIntegerPartNum) + _roundUp(iFractionPartLen);

		for (iterator; iterator != last; ++iterator)
		{
			::sprintf(chTemp, "%09d", *iterator);

			if (iterator != last-1)
				cResult += ModUnicodeString(chTemp, _iDigitPerDecimal);
			else
			{
				int iSize = iFractionPartLen%_iDigitPerDecimal;
				iSize = (iSize>0)? iSize: _iDigitPerDecimal;
				cResult += ModUnicodeString(chTemp, iSize);
			}
		}
	}

    return cResult;
}

// FUNCTION public
//	Common::DecimalData::getInt_NotNull -- get the decimal value by primitive numeric type
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
DecimalData::
getInt_NotNull() const
{
	int iValue = 0;
	if (_castToInt(iValue, m_dValue)) {
		return iValue;
	}
	_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
}

// FUNCTION public
//	Common::DecimalData::getUnsignedInt_NotNull -- get the decimal value by primitive numeric type
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	unsigned int
//
// EXCEPTIONS

//virtual
unsigned int
DecimalData::
getUnsignedInt_NotNull() const
{
	unsigned int uValue = 0;
	if (_castToInt(uValue, m_dValue)) {
		return uValue;
	}
	_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
}

//	FUNCTION private
//	Common::DecimalData::compareTo_NoCast -- Two data compares.
//
//	NOTES
//		Two data compares. The type of the compared data should be equal to oneself.
//
//	ARGUMENTS
//		Common::Data&	result_
//			Data given to the right side
//
//	RETURN
//		0
// 			The left side and the right side are equal. 
//		-1
// 			The left side is smaller. 
//		1
// 			The right side is smaller.
//
//	EXCEPTIONS
//		None

//virtual 
int
DecimalData::
compareTo_NoCast(const Data& result_) const
{
	if (isNull())
		return NullData::getInstance()->compareTo(&result_);

	; _TRMEISTER_ASSERT(result_.getType() == DataType::Decimal);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!result_.isNull());

	const DecimalData& data = _SYDNEY_DYNAMIC_CAST(const DecimalData&, result_);

	if (m_dValue.isNegative() == data.m_dValue.isNegative())
	{
		int iBigger = m_dValue.doCompare(data.m_dValue);
		return (m_dValue.isNegative() ? -iBigger : iBigger); // isNegative = false : positive
	}
		
	return (m_dValue.isNegative() > data.m_dValue.isNegative() ? -1 : 1);
}
//
//	FUNCTION private
//	Common::DecimalData::assign_NoCast -- The data is substituted..
//
//	NOTES
//	The data is substituted.
//
//	ARGUMENTS
//  Common::Data&	r
//       Data given to the right side
//
//	RETURN
//	bool
//		true
//          The operation result is not NULL. 
//      false
//          The operation result is NULL. 
//
//	EXCEPTIONS
//	None
	
//virtual 
bool
DecimalData::
assign_NoCast(const Data& r)
{
#ifdef OBSOLETE 
	if (r.isNull()) {
		setNull();
		return false;
	}
#endif

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const DecimalData& data = _SYDNEY_DYNAMIC_CAST(const DecimalData&, r);

	int iOwnInteger = m_dValue.getInteger();
	int iOwnFaction = m_dValue.getFraction();
	if ((iOwnInteger == 0 && iOwnFaction == 0)
		|| (iOwnInteger == data.m_dValue.getInteger() && iOwnFaction == data.m_dValue.getFraction())) 
	{
		setValue(data);
		return true;
	}
	//remove the leading zeroes from data(ie.from r)
	Decimal dValue(data.m_dValue);
	int iIntPartNumRemoveZeroes = 0;	
	ModVector<DecimalData::DigitUnit>::Iterator buffer = dValue.removeLeadingZeroes(iIntPartNumRemoveZeroes);
	if (m_dValue.getInteger() < iIntPartNumRemoveZeroes)
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);	

	//m_dValue.setValueByAssign(dValue, iIntPartNumRemoveZeroes);
	m_dValue.resetDigitVector(m_dValue.getDigitVector().getSize());
	ModVector<DecimalData::DigitUnit>::Iterator pStartOwn = m_dValue.getDigitVector().begin() + _roundUp(m_dValue.getInteger());
	ModVector<DecimalData::DigitUnit>::ConstIterator pStartOther = dValue.getDigitVector().begin() + _roundUp(dValue.getInteger());
	ModVector<DecimalData::DigitUnit>::Iterator iteratorOwn = pStartOwn;
	ModVector<DecimalData::DigitUnit>::ConstIterator iteratorOther = pStartOther;
	int iStep = _roundUp(iIntPartNumRemoveZeroes);
	while (iStep > 0)
	{
		*--iteratorOwn = *--iteratorOther;
		iStep--;
	}

	iteratorOwn = pStartOwn;
	iteratorOther = pStartOther;
	iStep = ModMin(_roundUp(dValue.getFraction()), _roundUp(m_dValue.getFraction()));
	int iLeftFactNumber = 0;
	if (dValue.getFraction() > m_dValue.getFraction())
	{
		iLeftFactNumber = (_iDigitPerDecimal - m_dValue.getFraction() % _iDigitPerDecimal) % _iDigitPerDecimal;
	}

	int iPowerNumber = static_cast<int>(_powers10[iLeftFactNumber]);
	while (iStep > 0)
	{
		if (iStep == 1 && iLeftFactNumber > 0)
			*iteratorOwn++ = (*iteratorOther++ / iPowerNumber) * iPowerNumber;
		else
			*iteratorOwn++ = *iteratorOther++;
		iStep--;
	}	

	m_dValue.setSign(dValue.isNegative());
	return true;
}
//
//	FUNCTION private
//	Common::DecimalData::operateWith_NotNull -- The prefix operation is done.
//
//	NOTES
//	The prefix operation is done.
//
//	ARGUMENTS
//  DataOperation::Type	op
//       The given operation type
//  Common::Data&	r
//       Data given to the right side
//
//	RETURN
//	bool
//		true
//          The operation result is not NULL. 
//      false
//          The operation result is NULL. 
//
//	EXCEPTIONS
//	None	

//virtual 
bool
DecimalData::
operateWith_NotNull(DataOperation::Type eOperation_, Data::Pointer& result_) const
{
	;_TRMEISTER_ASSERT(!isNull());

	switch (eOperation_)
	{
	case DataOperation::Negation:
		result_ = new DecimalData(*this);
		_SYDNEY_DYNAMIC_CAST(DecimalData*, result_.get())->m_dValue.setSign(!this->m_dValue.isNegative());
		return true;
		break;
	case DataOperation::AbsoluteValue:
		result_ = new DecimalData(*this);
		if (m_dValue.isNegative() == true)
			_SYDNEY_DYNAMIC_CAST(DecimalData*, result_.get())->m_dValue.setSign(!this->m_dValue.isNegative());
		return true;
		break;
	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	result_ = NullData::getInstance();
	return false;;	
}
//
//	FUNCTION private
//	Common::DecimalData::operateWith_NoCast -- The arithmetic operation is done.
//
//	NOTES
//	The arithmetic operation is done.
//
//	ARGUMENTS
//  DataOperation::Type	op
//       The given operation type
//  Common::Data&	r
//       Data given to the right side
//
//	RETURN
//	bool
//		true
//          The operation result is not NULL. 
//      false
//          The operation result is NULL. 
//
//	EXCEPTIONS
//	None
//
//virtual 
bool
DecimalData::
operateWith_NoCast(DataOperation::Type eOperation_, const Data& result_)
{
	; _TRMEISTER_ASSERT(result_.getType() == DataType::Decimal);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!result_.isNull());

	const DecimalData& cOtherData_ = _SYDNEY_DYNAMIC_CAST(const DecimalData&, result_);

	DecimalData::Decimal cResult;
	cResult.setDigitVector(m_dValue, cOtherData_.m_dValue, eOperation_);

	switch (eOperation_)
	{
	case DataOperation::Addition:
		{
			if (m_dValue.isNegative() == cOtherData_.m_dValue.isNegative())
			{
				if (cResult.doAdd(m_dValue, cOtherData_.m_dValue)) {
					m_dValue = cResult;	
					m_iScale = cResult.getFraction();
					m_iPrecision = cResult.getInteger() + m_iScale;
					return true;
				}
			}
			else
			{			
				if (cResult.doSub(m_dValue, cOtherData_.m_dValue)) {
					m_dValue = cResult;	
					m_iScale = cResult.getFraction();
					m_iPrecision = cResult.getInteger() + m_iScale;
					return true;
				}
			}
			break;
		}
	case DataOperation::Subtraction:
		{	
			if (m_dValue.isNegative() == cOtherData_.m_dValue.isNegative())
			{				
				if (cResult.doSub(m_dValue, cOtherData_.m_dValue)) {
					m_dValue = cResult;	
					m_iScale = cResult.getFraction();
					m_iPrecision = cResult.getInteger() + m_iScale;
					return true;
				}
			}
			else
			{
				if (cResult.doAdd(m_dValue, cOtherData_.m_dValue)) {
					m_dValue = cResult;	
					m_iScale = cResult.getFraction();
					m_iPrecision = cResult.getInteger() + m_iScale;
					return true;
				}
			}
			break;
		}

	case DataOperation::Multiplication:
		{
			if (cResult.doMul(m_dValue, cOtherData_.m_dValue)) {
				m_dValue = cResult;	
				m_iScale = cResult.getFraction();
				m_iPrecision = cResult.getInteger() + m_iScale;
				return true;
			}
			break;
		}
	case DataOperation::Division:
		{
			if (cResult.doDiv(m_dValue, cOtherData_.m_dValue)) {
				m_dValue = cResult;	
				m_iScale = cResult.getFraction();
				m_iPrecision = cResult.getInteger() + m_iScale;
				return true;
			}
			break;
		}
	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	setNull();
	return false;
}

//
//	FUNCTION private
//	Common::DecimalData::isAbleToDump_NotNull -- Whether decimaldata can be dumped.
//
//	NOTES
//	Whether decimaldata can be dumped.
//
//	ARGUMENTS
//	None
//
//	RETURN
//	bool
//		true
//          The operation result is not NULL. 
//      false
//          The operation result is NULL. 
//
//	EXCEPTIONS
//	None
//

//virtual 
#if 0
bool
DecimalData::
isAbleToDump_NotNull() const
{
	return true;
}
#endif
//
//	FUNCTION private
//	Common::DecimalData::isFixedSize_NotNull -- Whether decimaldata is always a fixed length.
//
//	NOTES
//	Whether decimaldata is always a fixed length.
//
//	ARGUMENTS
//	None
//
//	RETURN
//	bool
//		true
//          The operation result is not NULL. 
//      false
//          The operation result is NULL. 
//
//	EXCEPTIONS
//	None
//
//virtual 
#if 0
bool
DecimalData::
isFixedSize_NotNull() const 
{
	; _TRMEISTER_ASSERT(!isNull());
	return true;
}
#endif
//	FUNCTION public
//	Common::DecimalData::setDumpedValue --
//		Set does the value from data that dump is done. 
//
//	NOTES
//
//	ARGUMENTS
//		const char* pszValue_
//			Head of area where dump was done
//		ModSize uSize_( can be absent)
//			A correct value is verified when specifying it.
//
//	RETURN

//	ModSize 
//		Number of bytes used for value
//
//	EXCEPTIONS
//  None
//
//virtual
ModSize 
DecimalData::
setDumpedValue(const char* pszValue_)
{
	ModSize iDumpSize = getDumpSize_NotNull();
	int sizeUnit = sizeof(DigitUnit);
	DigitUnit cHighestBit = 0x80000000;
	;_TRMEISTER_ASSERT(iDumpSize <= (ModSize)m_dValue.getDigitLength() * sizeUnit);

	int iIntegerPartHead = m_iPrecision - m_iScale - (m_iPrecision - m_iScale)/ _iDigitPerDecimal * _iDigitPerDecimal;
	int iFractionPartTail = m_iScale -  m_iScale / _iDigitPerDecimal * _iDigitPerDecimal;

	DigitUnit cMask = 0;
	ModVector<DecimalData::DigitUnit>::Iterator buffer = m_dValue.getDigitVector().begin();

	const char* pValue = pszValue_;
	DigitUnit iFirstDigitUnit = 0;
	if (iIntegerPartHead > 0)
	{
		int iBytes = dig2bytes[iIntegerPartHead];
		iFirstDigitUnit = _NByteSetDumped(pValue, iBytes);
		//Os::Memory::copy(&iFirstDigitUnit, pValue, iBytes);
		pValue += iBytes;

		DigitUnit cTempBit = cHighestBit >> ((sizeUnit-iBytes)*8);
		cMask = (iFirstDigitUnit & cTempBit) ? 0 : -1;
		iFirstDigitUnit ^= cMask;
		iFirstDigitUnit ^= cTempBit;

		DigitUnit iTempMask = 0xff;
		for (int i = 1; i<iBytes; i++)
			iTempMask = (iTempMask<<8) + 0xff;

		iFirstDigitUnit &= iTempMask;
		*buffer++ = iFirstDigitUnit;
		cHighestBit = 0;
	}

	ModVector<DecimalData::DigitUnit>::Iterator end = m_dValue.getDigitVector().end();
	for (; buffer < end-1; buffer++)
	{
		;_TRMEISTER_ASSERT(sizeof(DigitUnit) == sizeUnit);
		DigitUnit iDigitUnit = _NByteSetDumped(pValue, sizeUnit);
		//Os::Memory::copy(&iDigitUnit, pValue, sizeUnit);
		
		if (cHighestBit != 0)
			cMask = (iDigitUnit & cHighestBit) ? 0 : -1;

		iDigitUnit ^= cMask;
		iDigitUnit ^= cHighestBit;
		*buffer = iDigitUnit;

		pValue += sizeUnit;
		cHighestBit = 0;
	}
	
	if (buffer < end)
	{
		DigitUnit iEndDigitUnit = 0;
		if (iFractionPartTail > 0)
		{
			int iBytes = dig2bytes[iFractionPartTail];
			iEndDigitUnit = _NByteSetDumped(pValue, iBytes);
			//Os::Memory::copy(&iEndDigitUnit, pValue, iBytes);
			if (cHighestBit != 0)
			{
				DigitUnit cTempBit = cHighestBit >> ((sizeUnit-iBytes)*8);
				cMask = (iEndDigitUnit & cTempBit) ? 0 : -1;
				iEndDigitUnit ^= cTempBit;
			}
			iEndDigitUnit ^= cMask;

			DigitUnit iTempMask = 0xff;
			for (int i = 1; i<iBytes; i++)
				iTempMask = (iTempMask<<8) + 0xff;

			iEndDigitUnit &= iTempMask;
			iEndDigitUnit *= _powers10[_iDigitPerDecimal - iFractionPartTail];
			pValue+= iBytes;
		}
		else
		{
			iEndDigitUnit = _NByteSetDumped(pValue, sizeUnit);
			//Os::Memory::copy(&iEndDigitUnit, pValue, sizeUnit);
			if (cHighestBit != 0)
				cMask = (iEndDigitUnit & cHighestBit) ? 0 : -1;
			iEndDigitUnit ^= cMask;
			iEndDigitUnit ^= cHighestBit;
			pValue+= sizeUnit;
		}
		*buffer = iEndDigitUnit;
	}

	m_dValue.setSign(cMask != 0);
	setNull(false);

	return iDumpSize;
}

ModSize 
DecimalData::
setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	ModSize iDumpSize = getDumpSize_NotNull();
	;_TRMEISTER_ASSERT(iDumpSize == uSize_);

	int sizeUnit = sizeof(DigitUnit);
	DigitUnit cHighestBit = 0x80000000;
	;_TRMEISTER_ASSERT(iDumpSize <= (ModSize)m_dValue.getDigitLength()*sizeUnit);

	int iIntegerPartHead = m_iPrecision - m_iScale - (m_iPrecision - m_iScale)/ _iDigitPerDecimal * _iDigitPerDecimal;
	int iFractionPartTail = m_iScale -  m_iScale / _iDigitPerDecimal * _iDigitPerDecimal;

	DigitUnit cMask = 0;
	ModVector<DecimalData::DigitUnit>::Iterator buffer = m_dValue.getDigitVector().begin();

	DigitUnit iFirstDigitUnit = 0;
	if (iIntegerPartHead > 0)
	{
		int iBytes = dig2bytes[iIntegerPartHead];
		cSerialIO_.readSerial(&iFirstDigitUnit, iBytes, ModSerialIO::dataTypeVariable);

		DigitUnit cTempBit = cHighestBit >> ((sizeUnit-iBytes)*8);
		cMask = (iFirstDigitUnit & cTempBit) ? 0 : -1;
		iFirstDigitUnit ^= cMask;
		iFirstDigitUnit ^= cTempBit;

		DigitUnit iTempMask = 0xff;
		for (int i = 1; i<iBytes; i++)
			iTempMask = (iTempMask<<8) + 0xff;

		iFirstDigitUnit &= iTempMask;
		*buffer++ = iFirstDigitUnit;
		cHighestBit = 0;
	}

	ModVector<DecimalData::DigitUnit>::Iterator end = m_dValue.getDigitVector().end();
	for (; buffer < end-1; buffer++)
	{
		;_TRMEISTER_ASSERT(sizeof(DigitUnit) == sizeUnit);
		DigitUnit iDigitUnit = 0;
		cSerialIO_.readSerial(&iDigitUnit, sizeUnit, ModSerialIO::dataTypeVariable);

		if (cHighestBit != 0)
			cMask = (iDigitUnit & cHighestBit) ? 0 : -1;

		iDigitUnit ^= cMask;
		iDigitUnit ^= cHighestBit;
		*buffer = iDigitUnit;

		cHighestBit = 0;
	}

	if (buffer < end)
	{
		DigitUnit iEndDigitUnit = 0;
		if (iFractionPartTail > 0)
		{
			int iBytes = dig2bytes[iFractionPartTail];
			cSerialIO_.readSerial(&iEndDigitUnit, iBytes, ModSerialIO::dataTypeVariable);
			if (cHighestBit != 0)
			{
				DigitUnit cTempBit = cHighestBit >> ((sizeUnit-iBytes)*8);
				cMask = (iEndDigitUnit & cTempBit) ? 0 : -1;
				iEndDigitUnit ^= cTempBit;
			}
			iEndDigitUnit ^= cMask;

			DigitUnit iTempMask = 0xff;
			for (int i = 1; i<iBytes; i++)
				iTempMask = (iTempMask<<8) + 0xff;

			iEndDigitUnit &= iTempMask;
			iEndDigitUnit *= _powers10[_iDigitPerDecimal - iFractionPartTail];
		}
		else
		{
			cSerialIO_.readSerial(&iEndDigitUnit, sizeUnit, ModSerialIO::dataTypeVariable);
			if (cHighestBit != 0)
				cMask = (iEndDigitUnit & cHighestBit) ? 0 : -1;
			iEndDigitUnit ^= cMask;
			iEndDigitUnit ^= cHighestBit;
		}
		*buffer = iEndDigitUnit;
	}

	m_dValue.setSign(cMask != 0);
	setNull(false);

	ModSize iRealSize = getRealDumpSize_NotNull(m_iPrecision, m_iScale);
	if (iRealSize < iDumpSize)
		cSerialIO_.readSerial(&cMask, iDumpSize - iRealSize, ModSerialIO::dataTypeVariable);
	return iDumpSize;
}

// FUNCTION public
//	Common::DecimalData::hashCode -- ハッシュコードを取り出す
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
DecimalData::
hashCode() const
{
	if (isNull()) return 0;

	return (static_cast<ModSize>(m_iPrecision) << 8)
		+ (static_cast<ModSize>(m_iScale) << 4)
		+ m_dValue.hashCode();
}

//
//	FUNCTION private
//	Common::DecimalData::getDumpSize_NotNull --  the dump size
//
//	NOTES
//	the dump size
//
//	ARGUMENTS
//	None
//
//	RETURN
//	ModSize
//		Number of bytes used for dumped value
//
//	EXCEPTIONS
//	None
//

//virtual 
ModSize
DecimalData::
getDumpSize_NotNull() const
{
	return _fourBytesRoundUp(getRealDumpSize_NotNull(m_iPrecision, m_iScale));
}

//static
ModSize
DecimalData::
getRealDumpSize_NotNull(int iPrecision_, int iScale_)
{
	int iIntegerPart = (iPrecision_ - iScale_) / _iDigitPerDecimal;
	int iFractionPart = iScale_ / _iDigitPerDecimal;
	int iIntegerPartHead = (iPrecision_ - iScale_) - iIntegerPart * _iDigitPerDecimal;
	int iFractionPartTail = iScale_ - iFractionPart * _iDigitPerDecimal;
	
	;_TRMEISTER_ASSERT(iScale_ >= 0 && iPrecision_ > 0 && iScale_ <= iPrecision_);
	return iIntegerPart * sizeof (DecimalData::DigitUnit) + dig2bytes[iIntegerPartHead] +
		iFractionPart * sizeof (DecimalData::DigitUnit) + dig2bytes[iFractionPartTail];
}

//
//	FUNCTION private
//	Common::DecimalData::dumpValue_NotNull --  the value is dumped
//
//	NOTES
//	the value is dumped
//
//	ARGUMENTS
//	char* dst
//      Head of area where dump is done
//
//	RETURN
//	ModSize
//		Number of bytes used for dumped value
//
//	EXCEPTIONS
//	None
//

//virtual	
ModSize
DecimalData::
dumpValue_NotNull(char* dst) const
{	
	;_TRMEISTER_ASSERT(dst != 0);
	;_TRMEISTER_ASSERT(m_iPrecision >= m_iScale);

	int sizeUnit = sizeof(DigitUnit);
	DigitUnit cHighestBit = 0x80000000;

	DigitUnit cMask = m_dValue.isNegative() ? -1 : 0;

	ModSize iDumpSize = getDumpSize_NotNull();
	;_TRMEISTER_ASSERT(iDumpSize <= (ModSize)m_dValue.getDigitLength()*sizeUnit);
	Os::Memory::set(&dst[0], static_cast<unsigned char>(cMask), iDumpSize);
	
	int iIntegerPartHead = m_iPrecision - m_iScale - (m_iPrecision - m_iScale)/ _iDigitPerDecimal * _iDigitPerDecimal;
	int iFractionPartTail = m_iScale -  m_iScale / _iDigitPerDecimal * _iDigitPerDecimal;

	ModVector<DecimalData::DigitUnit>::ConstIterator buffer = m_dValue.getDigitVector().begin();
	ModVector<DecimalData::DigitUnit>::ConstIterator stop = m_dValue.getDigitVector().end();

	DigitUnit iDigitUnit = 0;
	if (iIntegerPartHead > 0)
	{
		int iBytes = dig2bytes[iIntegerPartHead];

		iDigitUnit = *buffer++;
		DigitUnit cTempBit = cHighestBit >> ((sizeUnit-iBytes)*8);
		iDigitUnit ^= cTempBit;
		iDigitUnit ^= cMask;

		_NByteDump(dst, iDigitUnit, iBytes);
		//Os::Memory::copy(dst, &iDigitUnit, iBytes);

		cHighestBit = 0;
		dst += iBytes;
	}

	for (; buffer < stop-1; dst += sizeUnit)
	{
		iDigitUnit = *buffer++;
		iDigitUnit ^= cHighestBit;
		iDigitUnit ^= cMask;
		_NByteDump(dst, iDigitUnit, sizeUnit);
		//Os::Memory::copy(dst, &iDigitUnit, sizeUnit);

		cHighestBit = 0;
	}

	if (buffer < stop)
	{
		iDigitUnit = *buffer;

		if (iFractionPartTail > 0)
		{
			iDigitUnit = iDigitUnit / _powers10[_iDigitPerDecimal - iFractionPartTail];
			int iBytes = dig2bytes[iFractionPartTail];
			if (cHighestBit != 0)
			{
				DigitUnit cTempBit = cHighestBit >> ((sizeUnit-iBytes)*8);
				iDigitUnit ^= cTempBit;
			}
			iDigitUnit ^= cMask;
			_NByteDump(dst, iDigitUnit, iBytes);
			//Os::Memory::copy(dst, &iDigitUnit, iBytes);
		}
		else
		{
			iDigitUnit ^= cHighestBit;
			iDigitUnit ^= cMask;
			_NByteDump(dst, iDigitUnit, sizeUnit);
			//Os::Memory::copy(dst, &iDigitUnit, sizeUnit);
		}
	}

	return iDumpSize;
}


//virtual	
ModSize
DecimalData::
dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{	
	;_TRMEISTER_ASSERT(m_iPrecision >= m_iScale);

	int sizeUnit = sizeof(DigitUnit);
	DigitUnit cHighestBit = 0x80000000;

	DigitUnit cMask = m_dValue.isNegative() ? -1 : 0;

	ModSize iDumpSize = getDumpSize_NotNull();
	;_TRMEISTER_ASSERT(iDumpSize <= (ModSize)m_dValue.getDigitLength()*sizeUnit);

	int iIntegerPartHead = m_iPrecision - m_iScale - (m_iPrecision - m_iScale)/ _iDigitPerDecimal * _iDigitPerDecimal;
	int iFractionPartTail = m_iScale -  m_iScale / _iDigitPerDecimal * _iDigitPerDecimal;

	ModVector<DecimalData::DigitUnit>::ConstIterator buffer = m_dValue.getDigitVector().begin();
	ModVector<DecimalData::DigitUnit>::ConstIterator stop = m_dValue.getDigitVector().end();

	DigitUnit iDigitUnit = 0;
	if (iIntegerPartHead > 0)
	{
		int iBytes = dig2bytes[iIntegerPartHead];

		iDigitUnit = *buffer++;
		DigitUnit cTempBit = cHighestBit >> ((sizeUnit-iBytes)*8);
		iDigitUnit ^= cTempBit;
		iDigitUnit ^= cMask;

		cSerialIO_.writeSerial(&iDigitUnit, iBytes, ModSerialIO::dataTypeVariable);

		cHighestBit = 0;
	}

	for (; buffer < stop-1; buffer++)
	{
		iDigitUnit = *buffer;
		iDigitUnit ^= cHighestBit;
		iDigitUnit ^= cMask;
		cSerialIO_.writeSerial(&iDigitUnit, sizeUnit, ModSerialIO::dataTypeVariable);

		cHighestBit = 0;
	}

	if (buffer < stop)
	{
		iDigitUnit = *buffer;

		if (iFractionPartTail > 0)
		{
			iDigitUnit = iDigitUnit / _powers10[_iDigitPerDecimal - iFractionPartTail];
			int iBytes = dig2bytes[iFractionPartTail];
			if (cHighestBit != 0)
			{
				DigitUnit cTempBit = cHighestBit >> ((sizeUnit-iBytes)*8);
				iDigitUnit ^= cTempBit;
			}
			iDigitUnit ^= cMask;
			cSerialIO_.writeSerial(&iDigitUnit, iBytes, ModSerialIO::dataTypeVariable);
		}
		else
		{
			iDigitUnit ^= cHighestBit;
			iDigitUnit ^= cMask;
			cSerialIO_.writeSerial(&iDigitUnit, sizeUnit, ModSerialIO::dataTypeVariable);
		}
	}

	ModSize iRealSize = getRealDumpSize_NotNull(m_iPrecision, m_iScale);
	if (iRealSize < iDumpSize)
		cSerialIO_.writeSerial(&cMask, iDumpSize - iRealSize, ModSerialIO::dataTypeVariable);
	return iDumpSize;
}


//
//	FUNCTION private
//	Common::DecimalData::getClassID_NotNull -- get the ClassID of DecimalData
//
//	NOTES
//	Class ID for DecimalData is obtained.
//
//	ARGUMENTS
//	None
//
//	RETURN
//	int
//		ClassID of DecimalData class
//
//	EXCEPTIONS
//	None
//

//virtual 
int
DecimalData::
getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::DecimalDataClass;
}

//
//	FUNCTION private
//	Common::DecimalData::print_NotNull -- The value is displayed.
//
//	NOTES
//	The value is displayed.
//
//	ARGUMENTS
//	None
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//

//virtual 
#if 0
//void
DecimalData::
print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());
	cout << "string: " << getString().getString(Common::LiteralCode) << endl;
}

#endif

bool DecimalData::
canNoLostCastTo(int iPrecision_, int iScale_) const
{
	int iIntPartNum = 0;
	Decimal dValue(m_dValue);
	ModVector<DecimalData::DigitUnit>::Iterator buffer = dValue.removeLeadingZeroes(iIntPartNum);
	if (iIntPartNum <= (iPrecision_-iScale_))
	{
		if (m_iScale <= iScale_)
			return true;
		else
		{
			ModVector<DecimalData::DigitUnit>::Iterator iterator = buffer + _roundUp(iIntPartNum);
			ModVector<DecimalData::DigitUnit>::Iterator end = buffer + _roundUp(iIntPartNum) + _roundUp(m_iScale);

			int iEffectiveFracIndex = -1;
			for (int i=0; iterator < end; i++)
			{
				if (*iterator++ != 0)
					iEffectiveFracIndex = i;
			}
			
			if (iEffectiveFracIndex == -1)
				return true;

 			if (iEffectiveFracIndex <= _roundUp(iScale_)-1)
			{
				DigitUnit digit = *(buffer + _roundUp(iIntPartNum) + iEffectiveFracIndex);
				int iFractionPartTail = iScale_ -  iScale_ / _iDigitPerDecimal * _iDigitPerDecimal;
				int iPowerNumber = static_cast<int>(_powers10[_iDigitPerDecimal - iFractionPartTail]);
				if (digit == digit/iPowerNumber*iPowerNumber)
					return true;	
			}
		}
	}

	return false;
}

// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
