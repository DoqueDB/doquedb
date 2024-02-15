// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DecimalData.h--- the implementation of decimal data type
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_DECIMALDATA_H
#define __TRMEISTER_COMMON_DECIMALDATA_H

#include "Common/DoubleData.h"
#include "Common/FloatData.h"
#include "Common/Module.h"
#include "Common/ScalarData.h"
#include "Common/StringData.h"
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerData.h"

#include "ModVector.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::DecimalData -- Class that shows data of exact numeric type with aribitrary precision and scale
//
//	NOTES

class SYD_COMMON_FUNCTION DecimalData
	: public	ScalarData
{
public:

	// TYPEDEF
	// DecimalData::DigitUnit -- type of unit for Decimal sequence
	//
	// NOTES
	typedef int DigitUnit;

	// TYPEDEF
	// DecimalData::DoubleDigitUnit -- two units for Decimal sequence 
	//
	// NOTES
    typedef ModInt64 DoubleDigitUnit;

	//class
	//DecimalData::Decimal -- the definition of Decimal structure
	//
	//NOTES
	class Decimal
		: public	ModSerializer
	{
	public:
		//constructor
		Decimal();

		Decimal(const Decimal& cDecimal_);
		//destructor
		~Decimal();

		//operator overloading
		Decimal& operator =(const Decimal& cDecimal_);
	
		// obtain the member vairables
		int getInteger() const	{	return m_iIntegerPartLength;	}
		int getFraction() const	{	return m_iFractionPartLength;	}
		int getDigitLength() const	{	return m_vecDigit.getSize();	}
		bool isNegative() const		{	return m_bIsNegative;  }
		ModVector<DigitUnit>& getDigitVector()	{	return m_vecDigit;		}
		const ModVector<DigitUnit>& getDigitVector() const	{	return m_vecDigit;		}

		//set member varibales
		void setRange(int iIntegerLen_, int iFractionLen_);
		void setInteger(int iIntegerLen_){	m_iIntegerPartLength = iIntegerLen_;}
		void setFraction(int iFractionLen_);
		void setSign(bool bIsNegative_);
		void setValue(const Decimal& cDecimal_);
		void resetDigitVector(int iSize_);
		void setDigitVector(const Decimal& cOperand0_,  const Decimal& cOperand1_, DataOperation::Type eOperation_);
		
		//make zero
		void makeZero(int iIntegerLen_, int iFractionLen_);

		//swap two data
		bool swap(Decimal& cOtherDecimal_);

		//max value of defined integer part and fraction part
		void setToMaxDecimal(int iInteger_, int iFraction_);

		//remove the leading zeroes from m_vecDigit
		ModVector<DigitUnit>::Iterator  removeLeadingZeroes(int& iIntegerPartResult_);

		//the result size of arithmetic operations
		int calculateResultSize(const Decimal& cOtherDecimal_, DataOperation::Type eOperation_) const;

		//To make the cereal. 
		virtual void serialize(ModArchive& archiver_);	

		// take hash code
		ModSize hashCode() const;

		//compare two decimals
		int  doCompare(const Decimal& cOtherDecimal_) const;
		//mul two decimals
		bool doMul(const Decimal& cDecimal1_, const Decimal& cDeimal2_);
		//add two decimals
		bool doAdd(const Decimal& cDecimal1_, const Decimal& cDeimal2_);
		//div two decimals
		bool doDiv(Decimal& cDecimal1_, const Decimal& cDeimal2_);
		//sub two decimals
		bool doSub(const Decimal& cDecimal1_, const Decimal& cDeimal2_);

	private:
		/* 
		m_iIntegerPartLength: the digit number before decimal point;
		m_iFractionPartLength: the digit number after decimal point
		m_vecDigit: to store the digits
		m_bIsNegative: 0: negative, 1: positive
		*/
		int m_iIntegerPartLength;
		int m_iFractionPartLength;
		ModVector<DigitUnit> m_vecDigit;
		bool m_bIsNegative;
	};//end of class Decimal

	// Constructor
	DecimalData();
	DecimalData(int iPrecision_  , int iScale_);
	DecimalData(const DecimalData& cDecimalData_);
    //Destructor
    virtual ~DecimalData();

	//operator overloading
	DecimalData& operator =(const DecimalData& cDecimalData_);

	virtual bool isCompatible(const Data* r) const;

	virtual bool getSQLType(SQLData& cResult_);

	virtual void setSQLType(const SQLData& cType_);

	// To make itself to the cereal. 
	// Common::ScalarData
	// virtual void
	// serialize(ModArchive & archiver);

	// To copy itself 
	// Common::ScalarData
	// virtual Data::Pointer
	// copy() const;
	
	// To cast to other data types
	//Common::ScalarData
	//virtual Pointer
	//cast(DataType::Type type) const;


	//The value is obtained. 
	const Decimal& getValue() const;

	//The Precision is obtained.
	int getPrecision() const;
                                                   
	//The Scale is obtained.
	int getScale() const;

	static ModSize getDumpSizeBy(int iPrecision_, int iScale_);

	//set the range and value
	void setRange(int iPrecision_, int iScale_);
	void setValue(const DecimalData& cDecimal_);

	void setToMaxDecimalData(int iPrecision_, int iScale_);
	void setToMinDecimalData(int iPrecision_, int iScale_);
	bool canNoLostCastTo(int iPrecision_, int iScale_) const;

	// Precision and scale calculation
	static bool getOperationPrecisionScale(int iPrecision1_, int iScale1_, int iPrecision2_, int iScale2_, 
										   int& iPrecision_, int& iScale_,
										   Common::DataOperation::Type eOperation);
	//cast from other types
	bool castFromInteger64(const Integer64Data& cData_,bool bForAssign_ = false);
	bool castFromInteger(const IntegerData& cData_,bool bForAssign_ = false);
	bool castFromUnsignedInteger(const UnsignedIntegerData& cData_,bool bForAssign_ = false);
	bool castFromDouble(const DoubleData& cData_,bool bForAssign_ = false);
	bool castFromString(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_, bool bForAssign_ = false);
	bool castFromString(const char* pHead_, const char* pTail_, bool bForAssign_ = false);

	//	Common::ScalarData
	//	virtual ModUnicodeString
	//	getString() const;

	//	Common::Data
	//	virtual bool
	//	equals(const Data* r) const;

	//	Common::Data
	//	virtual int
	//	compareTo(const Data* r) const;

	//	Common::Data
	//	virtual bool
	//	assign(const Data* r);

	//	Common::Data
	//	virtual bool
	//	operateWith(DataOperation::Type op, const Data* r, Pointer& result) const;
	
	//	Common::ScalarData
	//	virtual bool
	//	operateWith(DataOperation::Type op, Pointer& result) const;
	//	virtual bool
	//	operateWith(DataOperation::Type op);

	
	//	Common::ScalarData
	//	virtual int
	//	getClassID() const;

		
	//	Common::ScalarData
	//	virtual void
	//	print() const;
		
		
	//	Common::ScalarData
	//	virtual bool
	//	isApplicable(Function::Value function);
		
	//	Common::ScalarData
	//	virtual Pointer
	//	apply(Function::Value function);

		
	//	Common::ScalarData
	//	virtual bool
	//	isAbleToDump() const;
		
	//	Common::ScalarData
	//	virtual bool
	//	isFixedSize() const;
		
	//	Common::ScalarData
	//	virtual ModSize
	//	getDumpSize() const;

	virtual ModSize setDumpedValue(const char* pszValue_);
	virtual ModSize setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_);

	//	Common::ScalarData
	//	virtual	ModSize
	//	dumpValue(char* p) const;

	//	Common::ScalarData
	//	virtual	ModSize
	//	dumpValue(ModSerialIO& cSerialIO_) const;

	//get the archive size
	//static ModSize getArchiveSize();

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;

private:
	//Making to cereal 
	virtual void
	serialize_NotNull(ModArchive& cArchiver_);

	//Own copy is returned. 
	virtual Data::Pointer
	copy_NotNull() const;
	
	//Cast to a specified type.
	virtual Pointer
	cast_NotNull(DataType::Type eType_, bool bForAssign_ = false) const;

	//get the decimal value by character string
	virtual ModUnicodeString
	getString_NotNull() const;  

	// get the decimal value by primitive numeric type
	virtual int getInt_NotNull() const;
	virtual unsigned int getUnsignedInt_NotNull() const;
	
	//whether two decimal data equals
	virtual bool
	equals_NoCast(const Data& result_) const{	return !compareTo_NoCast(result_);	}

	//Two data compares.
	virtual int
	compareTo_NoCast(const Data& result_) const;

	//The data is substituted
	virtual bool
	assign_NoCast(const Data& result_);
	
	//The prefix operation is done.
	virtual bool
	operateWith_NotNull(DataOperation::Type eOperation_, Data::Pointer& result_) const;

	//	Common::ScalarData
	//	virtual bool
	//	operateWith_NotNull(DataOperation::Type op);
	
	//The arithmetic operation is done.
	virtual bool
	operateWith_NoCast(DataOperation::Type eOperation_, const Data& result_);

	//	Common::ScalarData
	//	virtual bool
	//	isApplicable_NotNull(Function::Value function);
	
	//	Common::ScalarData
	//	virtual Pointer
	//	apply_NotNull(Function::Value function);
#if 0	
	// Whether decimaldata can be dumped.
	virtual bool
	isAbleToDump_NotNull() const;

	//Whether decimaldata is always a fixed length.
	virtual bool
	isFixedSize_NotNull() const;
#endif

	// the dump size
	virtual ModSize
	getDumpSize_NotNull() const;
	static ModSize
	getRealDumpSize_NotNull(int iPrecision_, int iScale_);
	// the value is dumped
	virtual	ModSize
	dumpValue_NotNull(char* dst) const;

	virtual ModSize 
	dumpValue_NotNull(ModSerialIO& cSerialIO_) const;

	//get the ClassID of DecimalData
	virtual int
	getClassID_NotNull() const;

	//The value is displayed.
#if 0	
	//virtual void
	print_NotNull() const;
#endif
	//member variables
	//data
	Decimal m_dValue;
	// the precision of decimaldata
	int m_iPrecision;
	// the scale of decimaldata
	int m_iScale;
};
_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_DECIMALDATA_H

// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
