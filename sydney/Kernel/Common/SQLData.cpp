// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SQLData.cpp -- SQLデータ型
// 
// Copyright (c) 1999, 2002, 2004, 2006, 2007, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Common";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Common/SQLData.h"
#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/ClassID.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/DataInstance.h"
#include "Common/DataType.h"
#include "Common/DecimalData.h"
#include "Common/Message.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"

#include "Exception/ArrayRightTruncation.h"
#include "Exception/ClassCast.h"
#include "Exception/CharacterNotInRepertoire.h"
#include "Exception/InvalidCharacter.h"
#include "Exception/NotSupported.h"
#include "Exception/StringRightTruncation.h"
#include "Exception/Unexpected.h"

#include "Os/Memory.h"

#include "ModAlgorithm.h"
#include "ModDefaultManager.h"
#include "ModOstrStream.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
	// A table to obtain compatible types
	// Compatible types are different according to the purpose of types; for comparison or for assignment
	// If two types can be casted each other such as character string types and numeric types,
	// it is assumed that right operand is casted to left operand in comparison cases.
	// This table was moved from Plan::ColumnType so that related methods can be used from Schema module

	const SQLData::Type::Value _CompatibleTypeTable[][SQLData::Type::ValueNum][SQLData::Type::ValueNum] =
	{
#define XXX SQLData::Type::NoType
#define Chr SQLData::Type::Char
#define NCh SQLData::Type::NChar
#define Int SQLData::Type::Int
#define Flt SQLData::Type::Float
#define Dtt SQLData::Type::DateTime
#define UID SQLData::Type::UniqueIdentifier
#define Bin SQLData::Type::Binary
#define Img SQLData::Type::Image
#define NTx SQLData::Type::NText
#define Ftx SQLData::Type::Fulltext
#define Lan SQLData::Type::Language
#define BLB SQLData::Type::BLOB
#define CLB SQLData::Type::CLOB
#define NLB SQLData::Type::NCLOB
#define Dec SQLData::Type::Decimal
#define Dat SQLData::Type::Date
#define Tim SQLData::Type::Time
#define Tms SQLData::Type::Timestamp
#define UIt SQLData::Type::UInt
#define Wrd SQLData::Type::Word
#define Big SQLData::Type::BigInt

		{ // not for comparison

		// XXX  Chr  NCh  Int  Flt  Dtt  UID  Bin  Img  NTx  Ftx  Lan  BLB  CLB  NLB  Dec  Dat  Tim  Tms  UIt  Wrd  Big
		{  XXX, Chr, NCh, Int, Flt, Dtt, UID, Bin, Img, NTx, Ftx, Lan, BLB, CLB, NLB, Dec, Dat, Tim, Tms, UIt, Wrd, Big},	// XXX
		{  Chr, Chr, NCh, Chr, Chr, Chr, Chr, Bin, Img, NTx, Ftx, Chr, BLB, CLB, NLB, Chr, Chr, Chr, Chr, Chr, XXX, Chr},	// Chr
		{  NCh, NCh, NCh, NCh, NCh, NCh, NCh, Bin, Img, NTx, Ftx, NCh, BLB, CLB, NLB, NCh, NCh, NCh, NCh, NCh, XXX, NCh},	// NCh
		{  Int, Chr, NCh, Int, Flt, XXX, XXX, XXX, XXX, NTx, Ftx, XXX, XXX, XXX, XXX, Dec, XXX, XXX, XXX, Int, XXX, Big},	// Int
		{  Flt, Chr, NCh, Flt, Flt, XXX, XXX, XXX, XXX, NTx, Ftx, XXX, XXX, XXX, XXX, Flt, XXX, XXX, XXX, Flt, XXX, Flt},	// Flt
		{  Dtt, Chr, NCh, XXX, XXX, Dtt, XXX, XXX, XXX, Dtt, Dtt, XXX, XXX, XXX, XXX, XXX, Dtt, Dtt, Dtt, XXX, XXX, XXX},	// Dtt
		{  UID, Chr, NCh, XXX, XXX, XXX, UID, Bin, Img, NTx, Ftx, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// UID
		{  Bin, Bin, Bin, XXX, XXX, XXX, Bin, Bin, Img, Bin, Bin, XXX, BLB, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// Bin
		{  Img, Img, Img, XXX, XXX, XXX, Img, Img, Img, Img, Img, XXX, BLB, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// Img
		{  NTx, NTx, NTx, NTx, NTx, XXX, NTx, Bin, Img, NTx, Ftx, XXX, BLB, CLB, NLB, NTx, NTx, NTx, NTx, NTx, XXX, NTx},	// NTx
		{  Ftx, Ftx, Ftx, Ftx, Ftx, XXX, Ftx, Bin, Img, NTx, Ftx, XXX, BLB, CLB, NLB, Ftx, Ftx, Ftx, Ftx, Ftx, XXX, Ftx},	// Ftx
		{  Lan, Chr, NCh, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, Lan, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// Lan
		{  BLB, BLB, BLB, XXX, XXX, XXX, XXX, BLB, BLB, BLB, BLB, XXX, BLB, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// BLB
		{  CLB, CLB, CLB, XXX, XXX, XXX, XXX, XXX, XXX, CLB, CLB, XXX, XXX, CLB, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// CLB
		{  NLB, NLB, NLB, XXX, XXX, XXX, XXX, XXX, XXX, NLB, NLB, XXX, XXX, XXX, NLB, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// NLB
		{  Dec, Chr, NCh, Dec, Flt, XXX, XXX, XXX, XXX, NTx, Ftx, XXX, XXX, XXX, XXX, Dec, XXX, XXX, XXX, Dec, XXX, Dec},	// Dec
		{  Dat, Chr, NCh, XXX, XXX, Dtt, XXX, XXX, XXX, NTx, Ftx, XXX, XXX, XXX, XXX, XXX, Dat, XXX, Tms, XXX, XXX, XXX},	// Dat
		{  Tim, Chr, NCh, XXX, XXX, Dtt, XXX, XXX, XXX, NTx, Ftx, XXX, XXX, XXX, XXX, XXX, XXX, Tim, Tms, XXX, XXX, XXX},	// Tim
		{  Tms, Chr, NCh, XXX, XXX, Dtt, XXX, XXX, XXX, NTx, Ftx, XXX, XXX, XXX, XXX, XXX, Tms, Tms, Tms, XXX, XXX, XXX},	// Tms
		{  UIt, Chr, NCh, Int, Flt, XXX, XXX, XXX, XXX, NTx, Ftx, XXX, XXX, XXX, XXX, Dec, XXX, XXX, XXX, UIt, XXX, Big},	// UIt
		{  Wrd, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, Wrd, XXX},	// Wrd
		{  Big, Chr, NCh, Big, Flt, XXX, XXX, XXX, XXX, NTx, Ftx, XXX, XXX, XXX, XXX, Dec, XXX, XXX, XXX, Big, XXX, Big},	// Big

		}, // not for comparison

		{ // for comparison -- use left for both castable

		// right
		// XXX  Chr  NCh  Int  Flt  Dtt  UID  Bin  Img  NTx  Ftx  Lan  BLB  CLB  NLB  Dec  Dat  Tim  Tms  UIt  Wrd  Big     // left
		{  XXX, Chr, NCh, Int, Flt, Dtt, UID, Bin, Img, NTx, Ftx, Lan, XXX, XXX, XXX, Dec, Dat, Tim, Tms, UIt, Wrd, Big},	// XXX
		{  Chr, Chr, NCh, Chr, Chr, Chr, Chr, Bin, Img, NTx, Ftx, Chr, XXX, XXX, XXX, Chr, Chr, Chr, Chr, Chr, XXX, Chr},	// Chr
		{  NCh, NCh, NCh, NCh, NCh, NCh, NCh, Bin, Img, NTx, Ftx, NCh, XXX, XXX, XXX, NCh, NCh, NCh, NCh, NCh, XXX, NCh},	// NCh
		{  Int, Int, Int, Int, Int, XXX, XXX, XXX, XXX, Int, Int, XXX, XXX, XXX, XXX, Dec, XXX, XXX, XXX, Int, XXX, Big},	// Int
		{  Flt, Flt, Flt, Flt, Flt, XXX, XXX, XXX, XXX, Flt, Flt, XXX, XXX, XXX, XXX, Flt, XXX, XXX, XXX, Flt, XXX, Flt},	// Flt
		{  Dtt, Dtt, Dtt, XXX, XXX, Dtt, XXX, XXX, XXX, Dtt, Dtt, XXX, XXX, XXX, XXX, XXX, Dtt, Dtt, Dtt, XXX, XXX, XXX},	// Dtt
		{  UID, Chr, NCh, XXX, XXX, XXX, UID, Bin, Img, NTx, Ftx, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// UID
		{  Bin, Bin, Bin, XXX, XXX, XXX, Bin, Bin, Img, Bin, Bin, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// Bin
		{  Img, Img, Img, XXX, XXX, XXX, Img, Img, Img, Img, Img, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// Img
		{  NTx, NTx, NTx, NTx, NTx, XXX, NTx, Bin, Img, NTx, Ftx, XXX, XXX, XXX, XXX, NTx, NTx, NTx, NTx, NTx, XXX, NTx},	// NTx
		{  Ftx, Ftx, Ftx, Ftx, Ftx, XXX, Ftx, Bin, Img, NTx, Ftx, XXX, XXX, XXX, XXX, Ftx, Ftx, Ftx, Ftx, Ftx, XXX, Ftx},	// Ftx
		{  Lan, Lan, Lan, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, Lan, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// Lan
		{  XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// BLB
		{  XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// CLB
		{  XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// NLB
		{  Dec, Dec, Dec, Dec, Dec, XXX, XXX, XXX, XXX, Dec, Dec, XXX, XXX, XXX, XXX, Dec, XXX, XXX, XXX, Dec, XXX, Dec},	// Dec
		{  Dat, Dat, Dat, XXX, XXX, Dtt, XXX, XXX, XXX, Dat, Dat, XXX, XXX, XXX, XXX, XXX, Dat, XXX, Tms, XXX, XXX, XXX},	// Dat
		{  Tim, Tim, Tim, XXX, XXX, Dtt, XXX, XXX, XXX, Tim, Tim, XXX, XXX, XXX, XXX, XXX, XXX, Tim, Tms, XXX, XXX, XXX},	// Tim
		{  Tms, Tms, Tms, XXX, XXX, Dtt, XXX, XXX, XXX, Tms, Tms, XXX, XXX, XXX, XXX, XXX, Tms, Tms, Tms, XXX, XXX, XXX},	// Tms
		{  UIt, UIt, UIt, UIt, UIt, XXX, XXX, XXX, XXX, UIt, UIt, XXX, XXX, XXX, XXX, Dec, XXX, XXX, XXX, UIt, XXX, Big},	// UIt
		{  Wrd, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, Wrd, XXX},	// Wrd
		{  Big, Big, Big, Big, Big, XXX, XXX, XXX, XXX, Big, Big, XXX, XXX, XXX, XXX, Dec, XXX, XXX, XXX, Big, XXX, Big},	// Big

		}, // for comparison
#undef XXX
#undef Chr
#undef NCh
#undef Int
#undef Flt
#undef Dtt
#undef UID
#undef Bin
#undef Img
#undef NTx
#undef Ftx
#undef Lan
#undef BLB
#undef CLB
#undef NLB
#undef Dec
#undef Dat
#undef Tim
#undef Tms
#undef UIt
#undef Wrd
#undef Big
	};

	// 互換性のあるフラグを得るためのテーブルTable to obtain compatible flag
	const SQLData::Flag::Value _CompatibleFlagTable[][SQLData::Flag::ValueNum] =
	{
#define N SQLData::Flag::None
#define f SQLData::Flag::OldFixed
#define v SQLData::Flag::OldVariable
#define U SQLData::Flag::Unlimited
#define F SQLData::Flag::Fixed
#define V SQLData::Flag::Variable
		// N  f  v  U  F  V
		{  N, U, U, U, F, V},	// N
		{  U, U, U, U, U, U},	// f
		{  U, U, U, U, U, U},	// v
		{  U, U, U, U, U, U},	// U
		{  F, U, U, U, F, V},	// F
		{  V, U, U, U, V, V},	// V
#undef N
#undef f
#undef v
#undef U
#undef F
#undef V
	};

	// 互換性のあるCollationを得るためのテーブル
	const Collation::Type::Value _CompatibleCollationTable[][Collation::Type::ValueNum] =
	{
#define IMP Collation::Type::Implicit
#define PAD Collation::Type::PadSpace
#define NOP Collation::Type::NoPad
		// IMP  PAD  NOP
		{  IMP, PAD, NOP},	// IMP
		{  PAD, PAD, NOP},	// PAD
		{  NOP, NOP, NOP},	// NOP
#undef IMP
#undef PAD
#undef NOP
	};

	// 代入可能性を得るためのテーブルTable to obtain substitution possibility
	enum {
		AssignableNone,
		AssignableCharacterString,
		AssignableBinaryString,
		AssignableDecimalString,
		NotAssignable,
	} _AssignableTable[][SQLData::Type::ValueNum] =
	{
#define o AssignableNone
#define C AssignableCharacterString
#define B AssignableBinaryString
#define D AssignableDecimalString
#define xxx NotAssignable
		// To
		// XXX  Chr  NCh  Int  Flt  Dtt  UID  Bin  Img  NTx  Ftx  Lan  BLB  CLB  NLB  Dec  Dat  Tim  Tms  UIt  Wrd  Big		// From
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// XXX
		{  xxx,   C, xxx, xxx, xxx, xxx,   C, xxx, xxx, xxx, xxx, xxx, xxx,   C, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Chr
		{  xxx, xxx,   C, xxx, xxx, xxx, xxx, xxx, xxx,   C,   C, xxx, xxx, xxx,   C, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// NCh
		{  xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Int
		{  xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Flt
		{  xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx},	// Dtt
		{  xxx,   C, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// UID
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx,   B,   B, xxx, xxx, xxx,   B, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Bin
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx,   B,   o, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Img
		{  xxx, xxx,   C, xxx, xxx, xxx, xxx, xxx, xxx,   o,   C, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// NTx
		{  xxx, xxx,   C, xxx, xxx, xxx, xxx, xxx, xxx,   C,   o, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Ftx
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Lan
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx,   B,   o, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// BLB
		{  xxx,   C, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// CLB
		{  xxx, xxx,   C, xxx, xxx, xxx, xxx, xxx, xxx,   o,   o, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// NLB
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   D, xxx, xxx, xxx, xxx, xxx, xxx},	// Dec
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx},	// Dat
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx},	// Tim
		{  xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx},	// Tms
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx},	// UIt
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx},	// Wrd
		{  xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o},	// Big
#undef o
#undef C
#undef B
#undef xxx
	};

	// 比較可能性を得るためのテーブルTable to obtain comparison possibility
	bool _ComparableTable[][SQLData::Type::ValueNum] =
	{
#define o true
#define xxx false
		// XXX  Chr  NCh  Int  Flt  Dtt  UID  Bin  Img  NTx  Ftx  Lan  BLB  CLB  NLB  Dec  Dat  Tim  Tms  UIt  Wrd  Big
		{    o,   o,   o,   o,   o,   o,   o,   o,   o,   o,   o,   o, xxx, xxx, xxx,   o,   o,   o,   o,   o,   o,   o},	// XXX
		{    o,   o,   o, xxx, xxx, xxx,   o, xxx, xxx,   o,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Chr
		{    o,   o,   o, xxx, xxx, xxx,   o, xxx, xxx,   o,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// NCh
		{    o, xxx, xxx,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx,   o, xxx,   o},	// Int
		{    o, xxx, xxx,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx,   o, xxx,   o},	// Flt
		{    o, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx,   o, xxx, xxx, xxx},	// Dtt
		{    o,   o,   o, xxx, xxx, xxx,   o, xxx, xxx,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// UID
		{    o, xxx, xxx, xxx, xxx, xxx, xxx,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Bin
		{    o, xxx, xxx, xxx, xxx, xxx, xxx,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Img
		{    o,   o,   o, xxx, xxx, xxx,   o, xxx, xxx,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// NTx
		{    o,   o,   o, xxx, xxx, xxx,   o, xxx, xxx,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Ftx
		{    o,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// Lan
		{    o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// BLB
		{    o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// CLB
		{    o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx},	// NLB
		{    o, xxx, xxx,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx,   o, xxx,   o},	// Dec
		{    o, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx,   o, xxx, xxx, xxx},	// Dat
		{    o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx},	// Tim
		{    o, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx,   o, xxx, xxx, xxx},	// Tms
		{    o, xxx, xxx,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx,   o, xxx,   o},	// UIt
		{    o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx},	// Wrd
		{    o, xxx, xxx,   o,   o, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx, xxx,   o, xxx, xxx, xxx,   o, xxx,   o},	// Big
#undef o
#undef xxx
	};

	// A table to obtain maximum precision value
	// -1 means no limit or not concerned
	int _MaxPrecisionTable[SQLData::Type::ValueNum] =
	{
		-1,	//NoType
		-1,	//Char,
		-1,	//NChar,
		-1,	//Int,
		-1,	//Float,
		-1,	//DateTime,
		-1,	//UniqueIdentifier,
		-1,	//Binary,
		-1,	//Image,
		-1,	//NText,
		-1,	//Fulltext,
		-1,	//Language,
		-1,	//BLOB,
		-1,	//CLOB,
		-1,	//NCLOB,
		36,	//Decimal,
		-1,	//Date,
		-1,	//Time,
		-1,	//Timestamp,
		-1,	//UInt,
		-1,	//Word,
		-1	//BigInt,
	};

	const struct
	{
		const char* const _pszName;
		bool _bUseLength;
		bool _bUseScale;
	} _cTypes[SQLData::Type::ValueNum] =
	{
		{"NoType", false, false},
		{"char", true, false},
		{"nchar", true, false},
		{"int", false, false},
		{"float", false, false},
		{"datetime", false, false},
		{"uniqueidentifier", false, false},
		{"binary", true, false},
		{"image", false, false},
		{"ntext", false, false},
		{"fulltext", false, false},
		{"language", false, false},
		{"blob", false, false},
		{"clob", false, false},
		{"nclob", false, false},
		{"decimal", true, true},
		{"date", false, false},
		{"time", false, false},
		{"timestamp", false, false},
		{"uint", false, false},
		{"word", false, false},
		{"bigint", false, false}
	};
	const char* const _pszCharTypeName[] =
	{
		"char",
		"char",
		"varchar",
		"varchar",
		"char",
		"varchar",
	};
	const char* const _pszNCharTypeName[] =
	{
		"nchar",
		"nchar",
		"nvarchar",
		"nvarchar",
		"nchar",
		"nvarchar",
	};
	const char* const _pszBinaryTypeName[] =
	{
		"binary",
		"binary",
		"varbinary",
		"binary",
		"binary",
		"varbinary",
	};

}

//
//	FUNCTION public
//	Common::SQLData::SQLData -- コンストラクタConstructor
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SQLData::SQLData()
	: m_eType(Type::NoType),
	  m_eFlag(Flag::None),
	  m_iLength(0),
	  m_iScale(0),
	  m_iCardinality(0),
	  m_eCollation(Collation::Type::Implicit)
{}

//
//	FUNCTION public
//	Common::SQLData::SQLData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Common::SQLData::Type::Value eType_
//		データ型
//	Common::SQLData::Flag::Value eFlag_
//		データ属性
//	int iLength_
//		長さ
//	int iScale_
//		スケール
//	Common::Collation::Type::Value eCollation_ = Implicit
//		比較方式
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SQLData::SQLData(Type::Value eType_,
				 Flag::Value eFlag_,
				 int iLength_,
				 int iScale_,
				 Collation::Type::Value eCollation_)
	: m_eType(eType_),
	  m_eFlag(eFlag_),
	  m_iLength(iLength_),
	  m_iScale(iScale_),
	  m_iCardinality(0),
	  m_eCollation(eCollation_)
{}

//
//	FUNCTION public
//	Common::SQLData::SQLData -- コンストラクタ
//
//	NOTES
//	配列の要素数を設定することのできるコンストラクター
//
//	ARGUMENTS
//	Common::SQLData::Type::Value eType_
//		データ型
//	Common::SQLData::Flag::Value eFlag_
//		データ属性
//	int iLength_
//		長さ
//	int iScale_
//		スケール
//	int iMaxCardinality_
//		配列要素数
//	Common::Collation::Type::Value eCollation_ = Implicit
//		比較方式
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SQLData::SQLData(Type::Value eType_,
				 Flag::Value eFlag_,
				 int iLength_,
				 int iScale_,
				 int iMaxCardinality_,
				 Collation::Type::Value eCollation_)
	: m_eType(eType_),
	  m_eFlag(eFlag_),
	  m_iLength(iLength_),
	  m_iScale(iScale_),
	  m_iCardinality(iMaxCardinality_),
	  m_eCollation(eCollation_)
{}

//
//	FUNCTION 
//	Common::SQLData::operator= -- 代入オペレーター
//
//	NOTES
//
//	ARGUMENTS
//	const SQLData& cOther_
//
//	RETURN
//	SQLData&
//		自分自身の参照
//
//	EXCEPTIONS
//
SQLData&
SQLData::operator=(const SQLData& cOther_)
{
	if (this != &cOther_) {
		m_eType = cOther_.m_eType;
		m_eFlag = cOther_.m_eFlag;
		m_iLength = cOther_.m_iLength;
		m_iScale = cOther_.m_iScale;
		m_iCardinality = cOther_.m_iCardinality;
		m_eCollation = cOther_.m_eCollation;
	}
	return *this;
}

// FUNCTION public
//	Common::SQLData::operator< -- 比較演算子
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
operator<(const SQLData& cOther_) const
{
	if (this != &cOther_) {
		if (m_eType != cOther_.m_eType) {
			return m_eType < cOther_.m_eType;
		}
		if (m_eFlag != cOther_.m_eFlag) {
			return m_eFlag < cOther_.m_eFlag;
		}
		if (m_iLength != cOther_.m_iLength) {
			return m_iLength < cOther_.m_iLength;
		}
		if (m_iScale != cOther_.m_iScale) {
			return m_iScale < cOther_.m_iScale;
		}
		if (m_iCardinality != cOther_.m_iCardinality) {
			return m_iCardinality < cOther_.m_iCardinality;
		}
		return m_eCollation < cOther_.m_eCollation;
	}
	return false;
}

//
//	FUNCTION public
//	Common::SQLData::clear -- メンバーを初期化する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SQLData::clear()
{
	m_eType = Type::NoType;
	m_eFlag = Flag::None;
	m_iLength = 0;
	m_iScale = 0;
	m_iCardinality = 0;
	m_eCollation = Collation::Type::Implicit;
}

// FUNCTION public
//	Common::SQLData::toSQLStatement -- SQL文の形式で得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

ModUnicodeString
SQLData::
toSQLStatement() const
{
	ModUnicodeOstrStream stream;
	switch (getType()) {
	default:
		{
			stream << _cTypes[getType()]._pszName;
			break;
		}
	case Type::Char:
		{
			stream << _pszCharTypeName[getFlag()];
			break;
		}
	case Type::NChar:
		{
			stream << _pszNCharTypeName[getFlag()];
			break;
		}
	case Type::Binary:
		{
			stream << _pszBinaryTypeName[getFlag()];
			break;
		}
	}

	if (_cTypes[getType()]._bUseScale) {
		stream << '(' << getLength() << ',' << getScale() << ')';
	} else if (_cTypes[getType()]._bUseLength) {
		stream << '(';
		if (getFlag() == Flag::Unlimited
			|| (getFlag() == Flag::Variable && getLength() == 0)) {
			stream << "no limit";
		} else {
			stream << getLength();
		}
		stream << ')';
	}
	if (isArrayType()) {
		stream << " array[";
		if (getMaxCardinality() < 0) {
			stream << "no limit";
		} else {
			stream << getMaxCardinality();
		}
		stream << "]";
	}
	return stream.getString();
}

//
//	FUNCTION public
//	Common::SQLData::getType -- SQLデータ型を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::SQLData::Type
//		SQLデータ型
//
//	EXCEPTIONS
//
SQLData::Type::Value
SQLData::getType() const
{
	return m_eType;
}

//
//	FUNCTION public
//	Common::SQLData::setType -- SQLデータ型を設定する
//
//	NOTES
//
//	ARGUMENTS
//	Common::SQLData::Type::Value eType_
//		SQLデータ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SQLData::setType(Type::Value eDataType_)
{
	m_eType = eDataType_;
}

//
//	FUNCTION public
//	Common::SQLData::getLength -- Length を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		長さ
//
//	EXCEPTIONS
//
int
SQLData::getLength() const
{
	return m_iLength;
}

//
//	FUNCTION public
//	Common::SQLData::setLength -- Length を設定する
//
//	NOTES
//
//	ARGUMENTS
//	int iLength_
//		長さ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SQLData::setLength(int iLength_)
{
	m_iLength = iLength_;
}

//
//	FUNCTION public
//	Common::SQLData::getScale -- Scale を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		スケール
//
//	EXCEPTIONS
//
int
SQLData::getScale() const
{
	return m_iScale;
}

//
//	FUNCTION public
//	Common::SQLData::setScale -- Scale を設定する
//
//	NOTES
//
//	ARGUMENTS
//	int iScale_
//		スケール
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SQLData::setScale(int iScale_)
{
	m_iScale = iScale_;
}

//
//	FUNCTION public
//	Common::SQLData::getFlag -- Flag を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::SQLData::Flag::Value
//	   その他性質
//
//	EXCEPTIONS
//
SQLData::Flag::Value
SQLData::getFlag() const
{
	return m_eFlag;
}

//
//	FUNCTION public
//	Common::SQLData::setFlag -- Flag を設定する
//
//	NOTES
//
//	ARGUMENTS
//	Common::SQLData::Flag::Value
//		その他性質
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SQLData::setFlag(Flag::Value eFlag_)
{
	m_eFlag = eFlag_;
}

//
//	FUNCTION public
//	Common::SQLData::setMaxCardinality -- Array を設定する array is set.
//
//	NOTES
//
//	ARGUMENTS
//	int iCardinality_
//		配列の要素数Number of array elements
//		-1  ：NOLIMIT
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SQLData::setMaxCardinality(int iCardinality_)
{
	m_iCardinality = iCardinality_;
}

//
//	FUNCTION public
//	Common::SQLData::getMaxCardinality -- Array を取得
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		配列の要素数
//		-1  ：NOLIMIT
//
//	EXCEPTIONS
//
int
SQLData::getMaxCardinality() const
{
	return m_iCardinality;
}

// FUNCTION public
//	Common::SQLData::isArrayType -- Arrayか否かを得るIt is obtained whether it is Array. 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
isArrayType() const
{
	return m_iCardinality != 0;
}

//
//	FUNCTION public
//	Common::SQLData::getCollation -- Collation を取得
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Collation::Type::Value
//		比較方式を表すenum
//
//	EXCEPTIONS
//
Collation::Type::Value
SQLData::getCollation() const
{
	return m_eCollation;
}

//
//	FUNCTION public
//	Common::SQLData::setCollation -- Collation を設定する
//
//	NOTES
//
//	ARGUMENTS
//	Common::Collation::Type::Value eCollation_
//		比較方式を表すenum
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SQLData::setCollation(Collation::Type::Value eCollation_)
{
	m_eCollation = eCollation_;
}

//
//	FUNCTION public
//	Common::SQLData::getClassID -- クラスIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		SQLDataのクラスID
//
//	EXCEPTIONS
//
int
SQLData::getClassID() const
{
	return ClassID::SQLDataClass;
}

//
//	FUNCTION public
//	Common::SQLData::serialize -- シリアル化
//
//	NOTES
//
//	ARGUMENTS
//	ModArchive& archiver_
//	   アーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SQLData::serialize(ModArchive& archiver_)
{
	// ★注意★
	// m_eCollationは互換性のためにシリアライズ対象になっていない
	// SchemaモジュールでColumnのヒントを見て別途セットする
//The hint of Column is seen in the Schema module that is not the Shiriaraiz object for 
//interchangeability and m_eCollation is set separately. 

	if (archiver_.isStore())
	{
		int tmp = getType();
		archiver_ << tmp;
		tmp = getFlag();
		archiver_ << tmp;
		archiver_ << m_iLength;
		archiver_ << m_iScale;
		archiver_ << m_iCardinality;
	}
	else
	{
		int tmp;
		archiver_ >> tmp;
		m_eType = static_cast<Type::Value>(tmp);
		archiver_ >> tmp;
		m_eFlag = static_cast<Flag::Value>(tmp);
		archiver_ >> m_iLength;
		archiver_ >> m_iScale;
		archiver_ >> m_iCardinality;
	}
}

// FUNCTION public
//	Common::SQLData::getElementType -- 配列型のとき要素の型を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	SQLData
//
// EXCEPTIONS

SQLData
SQLData::
getElementType() const
{
	// コピーしてMaxCardinalityを0にすればよい
	SQLData cResult(*this);
	cResult.setMaxCardinality(0);
	return cResult;
}

// FUNCTION public
//	Common::SQLData::getCompatibleType -- 上位互換性のある型を作るThe type with the upward compatibility is made. 
//
// NOTES
//	Plan::ColumnTypeから引越し
//
// ARGUMENTS
//	const SQLData& cType1_
//	const SQLData& cType2_
//	SQLData& cResult_
//	bool bForComparison_ = false
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
SQLData::
getCompatibleType(const SQLData& cType1_, const SQLData& cType2_,
				  SQLData& cResult_, bool bForComparison_/* = false */, Common::DataOperation::Type eOperation /* = Common::DataOperation::Unknown */)
{
	Type::Value eCompatibleType = _CompatibleTypeTable[bForComparison_ ? 1 : 0][cType1_.getType()][cType2_.getType()];

	if (eCompatibleType == Type::Decimal)
	{
		cResult_.setType(eCompatibleType);
		cResult_.setFlag(_CompatibleFlagTable[cType1_.getFlag()][cType2_.getFlag()]);
		cResult_.setMaxCardinality((ModMin(cType1_.getMaxCardinality(), cType2_.getMaxCardinality()) == -1) ?
			-1 : ModMax(cType1_.getMaxCardinality(), cType2_.getMaxCardinality()));
		cResult_.setCollation(_CompatibleCollationTable[cType1_.getCollation()][cType2_.getCollation()]);

		if (eOperation != Common::DataOperation::Unknown)
		{
			int iPrecision = 0;
			int iScale = 0;
			DecimalData::getOperationPrecisionScale(cType1_.getLength(), cType1_.getScale(), cType2_.getLength(), cType2_.getScale(),
								            		iPrecision, iScale, eOperation);
			cResult_.setLength(ModMin(iPrecision,getMaxPrecision(eCompatibleType)));
			cResult_.setScale(ModMin(iScale,getMaxPrecision(eCompatibleType)));
		}
		else 
		{
			cResult_.setLength(0);
			cResult_.setScale(0);
		}

		return true;
	}
	
	if (eCompatibleType != Type::NoType) 
	{
		// 上位互換性があれば結果を作る
		cResult_.setType(eCompatibleType);
		cResult_.setFlag(_CompatibleFlagTable[cType1_.getFlag()][cType2_.getFlag()]);
		switch (cResult_.getFlag()) {
		case Flag::Fixed:
			{
				cResult_.setLength(ModMax(cType1_.getLength(), cType2_.getLength()));
				break;
			}
		case Flag::Variable:
			{
				cResult_.setLength((ModMin(cType1_.getLength(), cType2_.getLength()) == 0) ?
								   0 : ModMax(cType1_.getLength(), cType2_.getLength()));
				break;
			}
		default:
			{
				cResult_.setLength(0);
				break;
			}
		}
		cResult_.setScale(ModMax(cType1_.getScale(), cType2_.getScale()));
		cResult_.setMaxCardinality((ModMin(cType1_.getMaxCardinality(), cType2_.getMaxCardinality()) == -1) ?
								   -1 : ModMax(cType1_.getMaxCardinality(), cType2_.getMaxCardinality()));
		cResult_.setCollation(_CompatibleCollationTable[cType1_.getCollation()][cType2_.getCollation()]);
		return true;
	}

	// 互換性がなければ失敗
	return false;
}

bool
SQLData::
getSQLDataTypeOwnIfDecimal(const SQLData& cTypeOwn_, const SQLData& cTypeDecimal_, SQLData& cResultOwn_)
{
	// 上位互換性があれば結果を作る
	Type::Value eCompatibleType = _CompatibleTypeTable[0][cTypeOwn_.getType()][cTypeDecimal_.getType()];
	if (eCompatibleType == Type::Decimal && cTypeOwn_.getType() != Type::Decimal)
	{
		; _TRMEISTER_ASSERT(cTypeDecimal_.getType() == Type::Decimal);
		cResultOwn_.setType(eCompatibleType);
		cResultOwn_.setFlag(_CompatibleFlagTable[cTypeOwn_.getFlag()][cTypeDecimal_.getFlag()]);
		cResultOwn_.setMaxCardinality((ModMin(cTypeOwn_.getMaxCardinality(), cTypeDecimal_.getMaxCardinality()) == -1) ?
			-1 : ModMax(cTypeOwn_.getMaxCardinality(), cTypeDecimal_.getMaxCardinality()));
		cResultOwn_.setCollation(_CompatibleCollationTable[cTypeOwn_.getCollation()][cTypeDecimal_.getCollation()]);

		cResultOwn_.setLength(cTypeOwn_.getLength());
		cResultOwn_.setScale(cTypeOwn_.getScale());	
		return true;
	}

	return false;
}

// FUNCTION public
//	Common::SQLData::isCompatibleType -- 型に互換性があるかを得る
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cType1_
//	const SQLData& cType2_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
SQLData::
isCompatibleType(const SQLData& cType1_, const SQLData& cType2_)
{
	return _CompatibleTypeTable[0][cType1_.getType()][cType2_.getType()] != Type::NoType;
										// この関数は代入可能性を調べるときしか呼ばれないので
	//Because this function is called only at time when the substitution possibility is examined
										// _CompatibleTypeTableは[0]を引く
										// Plan::ColumnTypeから一部引越し
}

// FUNCTION public
//	Common::SQLData::isAssignable -- castなしに代入可能かCan you substitute it without cast?
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cType1_
//	const SQLData& cType2_
//		cType1_がcType2_にcastなしで代入可能かを調べる
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
SQLData::
isAssignable(const SQLData& cType1_, const SQLData& cType2_, Common::DataOperation::Type eOperation_ /*= Common::DataOperation::Unknown*/)
{
	// Plan::ColumnTypeから一部引越し
	if (cType1_.isArrayType() == cType2_.isArrayType()) {

		// Arrayなら代入先のCardinalityの方が小さければ代入できない
		if (cType1_.isArrayType()
			&& cType2_.getMaxCardinality() > 0 // not unlimited
			&& cType1_.getMaxCardinality() > cType2_.getMaxCardinality())
			return false;

		// 表を引く
		switch (_AssignableTable[cType1_.getType()][cType2_.getType()]) {
		case AssignableNone:
			{
				// 無条件に代入可能
				return true;
			}
		case AssignableDecimalString:
			{
				if (cType1_.getFlag() != cType2_.getFlag()
					|| cType1_.getLength() != cType2_.getLength()
					|| cType1_.getScale() != cType2_.getScale())
				{
					if (eOperation_ >= Common::DataOperation::MonadicStart)
						return false;
				}

				// 無条件に代入可能
				return true;
			}
			break;
		case NotAssignable:
			{
				// 無条件に代入不可
				return false;
			}
		case AssignableCharacterString:
			{
				// 文字列の場合
				if (cType2_.getFlag() == Flag::Variable) {
					// 代入先がVariableならCollationをチェックする
					if (cType1_.getCollation() != Common::Collation::Type::NoPad
						&& cType2_.getCollation() != Common::Collation::Type::NoPad)
						// 空白を除去する必要があるので常にcastを挟む
						return false;
				}
				// thru.
			}
		case AssignableBinaryString:
			{
				// バイナリ列または文字列(上のcaseから降りてくる)の場合

				// Variableなら代入先のサイズが大きければcast不要
				// Fixedならサイズが等しい必要がある
				//... -> Fixedの長さ調整をどこでやるかによって変わってくる
				switch (cType2_.getFlag()) {
				case Flag::Variable:
					{
						// 代入先のサイズのほうが大きければcast不要
						return cType2_.getLength() == 0
							|| (cType1_.getLength() > 0 && cType1_.getLength() <= cType2_.getLength());
					}
				case Flag::Fixed:
					{
						// サイズが等しい必要がある
						return cType1_.getLength() == cType2_.getLength();
					}
				case Flag::Unlimited:
				case Flag::OldFixed:
				case Flag::OldVariable:
					{
						// 常にcast不要
						return true;
					}
				default:
					break;
				} // switch
			} // case AssingableBinaryString
		default:
			break;
		} // switch
	}
	return false;
}

// FUNCTION public
//	Common::SQLData::isComparable -- castなしに比較可能か
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cType1_
//	const SQLData& cType2_
//		cType1_がcType2_とcastなしで比較可能かを調べる
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
SQLData::
isComparable(const SQLData& cType1_, const SQLData& cType2_)
{
	// 配列かどうか、Collationが一緒かを調べてから表をひく
	// Plan::ColumnTypeから引越し

	return (cType1_.isArrayType() == cType2_.isArrayType()
			&& cType1_.getCollation() == cType2_.getCollation()
			&& _ComparableTable[cType1_.getType()][cType2_.getType()]);
}

// FUNCTION public
//	Common::SQLData::getMaxPrecision -- Get maximum value of precision
//
// NOTES
//
// ARGUMENTS
//	SQLData::Type::Value eType_
//	
// RETURN
//	int
//
// EXCEPTIONS

//static
int
SQLData::
getMaxPrecision(SQLData::Type::Value eType_)
{
	return _MaxPrecisionTable[eType_];
}

// FUNCTION public
//	Common::SQLData::create -- create SQLType object from data type
//
// NOTES
//
// ARGUMENTS
//	DataType::Type eType_
//	
// RETURN
//	SQLData
//
// EXCEPTIONS

//static
SQLData
SQLData::
create(DataType::Type eType_)
{
	SQLData cResult;
	(void) Common::Data::getSQLType(eType_, cResult);
	return cResult;
}

// FUNCTION public
//	Common::SQLData::cast -- この型にキャストする
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_ = false
//	bool bNoThrow_ = false
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
cast(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
	 bool bForComparison_, bool bNoThrow_) const
{
 	if (isArrayType())
		return castArray(cSourceType_, pData_, pTarget_, bForComparison_, bNoThrow_);
	else
		return castScalar(cSourceType_, pData_, pTarget_, bForComparison_, bNoThrow_);
}

// FUNCTION public
//	Common::SQLData::castScalar -- この型にキャストする(Scalar版)It is Cast in this type. (Scalar version)
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_
//	bool bNoThrow_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castScalar(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
		   bool bForComparison_, bool bNoThrow_) const
{
#define   O 0
#define XXX &SQLData::castCannot
#define sst &SQLData::castStringString
#define sus &SQLData::castStringUnicodeString
#define nst &SQLData::castNumericString
#define lst &SQLData::castLanguageString 
#define bbn &SQLData::castBinaryBinary
#define tst &SQLData::castToString
#define tus &SQLData::castToUnicodeString
#define tbn &SQLData::castToBinary

	// cast時にチェックするメソッドを得るためのテーブルTable to obtain method of check at cast
	static CastMethod _CastMethodTable[][SQLData::Type::ValueNum] =
	{
		// To Type
		// XXX  Chr  NCh  Int  Flt  Dtt  UID  Bin  Img  NTx  Ftx  Lan  BLB  CLB  NLB  Dec  Dat  Tim  Tms  UIt  Wrd  Big		// From Type
		{  XXX, tst, tus,   O,   O,   O, XXX, tbn,   O,   O,   O, XXX,   O,   O,   O,   O,   O,   O,   O,   O, XXX,   O},	// Int
		{  XXX, tst, tus,   O,   O,   O, XXX, tbn,   O,   O,   O, XXX,   O,   O,   O,   O,   O,   O,   O,   O, XXX,   O},	// UIt
		{  XXX, tst, tus,   O,   O,   O, XXX, tbn,   O,   O,   O, XXX,   O,   O,   O,   O,   O,   O,   O,   O, XXX,   O},	// I64
		{  XXX, tst, tus,   O,   O,   O, XXX, tbn,   O,   O,   O, XXX,   O,   O,   O,   O,   O,   O,   O,   O, XXX,   O},	// U64
		{  XXX, sst, sus, nst, nst,   O, sst, tbn,   O,   O,   O, lst,   O,   O,   O, nst,   O,   O,   O, nst, XXX, nst},	// Str
		{  XXX, tst, tus,   O,   O,   O, XXX, tbn,   O,   O,   O, XXX,   O,   O,   O,   O,   O,   O,   O,   O, XXX,   O},	// Flt
		{  XXX, tst, tus,   O,   O,   O, XXX, tbn,   O,   O,   O, XXX,   O,   O,   O,   O,   O,   O,   O,   O, XXX,   O},	// Dbl
		{  XXX, tst, tus,   O,   O,   O, XXX, tbn,   O,   O,   O, XXX,   O,   O,   O,   O,   O,   O,   O,   O, XXX,   O},	// Dec
		{  XXX, tst, tus,   O,   O,   O, XXX, tbn,   O,   O,   O, XXX,   O,   O,   O,   O,   O,   O,   O,   O, XXX,   O},	// Dat
		{  XXX, tst, tus,   O,   O,   O, XXX, tbn,   O,   O,   O, XXX,   O,   O,   O,   O,   O,   O,   O,   O, XXX,   O},	// Tim
		{  XXX, tst, tus,   O,   O,   O, tst, bbn,   O,   O,   O, XXX,   O,   O,   O,   O,   O,   O,   O,   O, XXX,   O},	// Bin
		{    O, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// Bit
		{    O, tst, tus,   O,   O,   O, XXX, tbn,   O,   O,   O, XXX,   O,   O,   O,   O, XXX, XXX, XXX,   O, XXX,   O},	// OID
		{  XXX, tst, tus, XXX, XXX, XXX, XXX, tbn,   O,   O,   O,   O,   O,   O,   O, XXX, XXX, XXX, XXX,   O, XXX, XXX},	// Lan
		{    O, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX},	// Met
		{    O, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX,   O, XXX},	// Wrd
	};
#undef   O
#undef XXX
#undef sst
#undef nst
#undef lst
#undef bbn
#undef tst
#undef tbn

	if (pData_->isNull()) {
		pTarget_->setNull();
		return true;

	} else if (pData_->isDefault()) {
		pTarget_->setDefault();
		return true;

	} else {
		DataType::Type eType = pData_->getType();
		if (Data::isScalar(eType)
			&& _CastMethodTable[eType - DataType::MinScalar][getType()] != 0)
			return (this->*_CastMethodTable[eType - DataType::MinScalar][getType()])(cSourceType_, pData_, pTarget_,
																					 bForComparison_, bNoThrow_);
		else
			return castNormal(cSourceType_, pData_, pTarget_, bForComparison_, bNoThrow_);
	}
}

// この型にキャストする(Array版)
bool
SQLData::
castArray(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
		  bool bForComparison_, bool bNoThrow_) const
{
	; _TRMEISTER_ASSERT(isArrayType());

	if (pData_->isNull()) {
		pTarget_->setNull();
		return true;
	}
	if (pData_->isDefault()) {
		pTarget_->setDefault();
		return true;
	}

	if (pData_->getType() != DataType::Array
		|| pData_->getElementType() != DataType::Data) {
		// DataArrayDataしかキャストできない
		if (!bNoThrow_)
			_TRMEISTER_THROW0(Exception::ClassCast);
		else
			return false;
	}

	const DataArrayData& cArrayData =
		_SYDNEY_DYNAMIC_CAST(const DataArrayData&, *pData_);

	DataArrayData& cResult =
		_SYDNEY_DYNAMIC_CAST(DataArrayData&, *pTarget_);
	cResult.clear();
	cResult.setNull(false);

	if (int n = cArrayData.getCount()) {
		int max = getMaxCardinality();
		if (max > 0 && n > max) {
			// ArrayのCardinalityを超えている
			if (!bNoThrow_)
				_TRMEISTER_THROW0(Exception::ArrayRightTruncation);
			else
				return false;
		}
		cResult.reserve(n);
		int i = 0;
		SQLData cElementType = cSourceType_.getElementType();
		SQLData cTargetType = getElementType();
		do {
			Data::Pointer pElement = DataInstance::create(cTargetType);
			castScalar(cElementType, cArrayData.getElement(i), pElement, bForComparison_, bNoThrow_);
			cResult.pushBack(pElement);
		} while (++i < n);
	}
	return true;
}

// FUNCTION public
//	Common::SQLData::castNormal -- 普通にCommon::Dataのキャスト機構を使う
//The Cast mechanism of Common::Data is usually used. 
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_
//	bool bNoThrow_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castNormal(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
		   bool bForComparison_, bool bNoThrow_) const
{
	// assignの内部で必要に応じてcastされるCast is done if necessary in assign. 
	pTarget_->assign(pData_.get(), !bForComparison_);
	return true;
}

// FUNCTION public
//	Common::SQLData::castCannot -- キャストできない場合に実行される
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_
//	bool bNoThrow_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castCannot(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
		   bool bForComparison_, bool bNoThrow_) const
{
	if (!bNoThrow_)
		_TRMEISTER_THROW0(Exception::ClassCast);
	return false;
}

// FUNCTION public
//	Common::SQLData::castStringString -- 文字列から文字列へのキャスト
//Cast from character string to character string
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_, bool bNoThrow_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castStringString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
				 bool bForComparison_, bool bNoThrow_) const
{
	return castStringString_Internal(cSourceType_, pData_, pTarget_, bForComparison_, bNoThrow_);
}
bool
SQLData::
castStringUnicodeString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
						bool bForComparison_, bool bNoThrow_) const
{
	return castStringUnicodeString_Internal(cSourceType_, pData_, pTarget_, bForComparison_, bNoThrow_);
}

// FUNCTION public
//	Common::SQLData::castNumericString -- 文字列から数値型へのキャスト
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_
//	bool bNoThrow_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castNumericString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
				  bool bForComparison_, bool bNoThrow_) const
{
	; _TRMEISTER_ASSERT(pData_->getType() == DataType::String);

	const StringData& cString = _SYDNEY_DYNAMIC_CAST(const StringData&, *pData_);

	// 数値として正しい形式かを調べる(比較のときは調べない)A correct form is examined as a numerical value. (When comparing it, do not examine it. )
	bool bRet = true;
	ModSize n = cString.getLength();
	if (!bForComparison_ && n) {
		const ModUnicodeChar* p = cString.getValue();
		const ModUnicodeChar* q = cString.getValue().getTail();

		bRet = SQLData::checkNumericString(p, q,  bNoThrow_);
	}
	// OKなら普通のキャストIf it is OK, it is usual Cast. 
	if (bRet)
		return castNormal(cSourceType_, pData_, pTarget_, bForComparison_, bNoThrow_);
	else
		return false;
}

bool
SQLData::
checkNumericString(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_, bool bNoThrow_)
{
	const ModUnicodeChar* p = pHead_;
	const ModUnicodeChar* q = pTail_;

	ModSize n = static_cast<ModSize>(q-p);
	while (*p == UnicodeChar::usSpace && n > 1) {
		++p; --n;
	}
	while (*(q-1) == UnicodeChar::usSpace && n > 1) {
		--q; --n;
	}
	if (n > 1) {
		if (*p == UnicodeChar::usPlus || *p == UnicodeChar::usHyphen) {
			++p;
			--n;
		}
	}
	bool bExponent = false;
	bool bPeriod = false;
	do {
		if ((*p == UnicodeChar::usLargeE || *p == UnicodeChar::usSmallE)
			&& !bExponent && n > 1) {
				bExponent = true;
				++p;
				--n;
				if (n > 1 && (*p == UnicodeChar::usPlus || *p == UnicodeChar::usHyphen)) {
					++p;
					--n;
				}
			}
			if (*p == UnicodeChar::usPeriod
				&& !bPeriod && !bExponent) {
					bPeriod = true;
					++p;
					--n;
				}
				if (!ModUnicodeCharTrait::isDigit(*p)) {
					if (!bNoThrow_)
						_TRMEISTER_THROW0(Exception::InvalidCharacter);
					else
						return false;
				}
				++p;
	} while ((--n > 0)&&(p<=q));

	return true;
}

// FUNCTION public
//	Common::SQLData::getCastStringLength -- Get target length of string
//
// NOTES
//
// ARGUMENTS
//	int& nDst_
//	const SQLData& cSourceType_
//	const Common::Data& cData_
//	bool bForComparison_ = false
//	bool bNoThrow_ = false
//	bool bTruncateIsNotAllowed_ = false
//	
// RETURN
//	bool
//		true ... succeeded
//		false... failed but no exception are thrown
//
// EXCEPTIONS

bool
SQLData::
getCastStringLength(int& nDst_,
					const SQLData& cSourceType_, const Common::Data& cData_,
					bool bForComparison_ /* = false */,
					bool bNoThrow_ /* = false */,
					bool bTruncateIsNotAllowed_ /* = false */) const
{
	; _TRMEISTER_ASSERT(cData_.getType() == DataType::String);

	const StringData& cString = _SYDNEY_DYNAMIC_CAST(const StringData&, cData_);

	// top address and length of source string
	const ModUnicodeChar* pSrc = cString.getValue();
	const int nSrc = cString.getLength();
	nDst_ = nSrc;

	switch (getFlag()) {
	case Flag::Variable:
		{
			bool bTruncateSpace =
				(getCollation() != Collation::Type::NoPad
				 && cSourceType_.getCollation() != Collation::Type::NoPad);

			if (bTruncateSpace) {
				// Trailing spaces are deleted
				if (nDst_) {
					const ModUnicodeChar* p = pSrc + (nDst_ - 1);
					if (*p == UnicodeChar::usSpace) {
						do {
							--nDst_;
							--p;
						} while (nDst_ > 0 && *p == UnicodeChar::usSpace);
					}
				}
			}

			if (getLength() <= 0)
				// Unlimited length -> no other process
				break;

			int iLength = getLength();

			// If casting is not for comparison, compare length with source length
			if (!bForComparison_ && iLength < nDst_) {
				// When type length is smaller, cause an error or output warning according to argument
				if (bTruncateIsNotAllowed_) {
					if (!bNoThrow_)
						_TRMEISTER_THROW0(Exception::StringRightTruncation);
					else
						return false;
				}

				// Search for trailing spaces to determine whether warning should be output or not
				// If bTruncateSpace = true, trailing spaces has been trimed in above code
				bool bNonSpaceFound = bTruncateSpace;

				if (!bNonSpaceFound) {
					// Search for trailing spaces
					const ModUnicodeChar* p = pSrc + iLength;
					ModSize i = iLength;
					do {
						if (*p++ != UnicodeChar::usSpace) {
							bNonSpaceFound = true;
							break;
						}
					} while (++i < static_cast<ModSize>(nDst_));
				}

				if (bNonSpaceFound) {
					//...
					//YET
					// Warning; string data, right truncation
					SydInfoMessage << "Warning: string data, right truncation" << ModEndl;
				}
			}
			nDst_ = ModMin(iLength, nDst_);
			break;
		}
	case Flag::Fixed:
		{
			// When target length is greater than source length, put spaces
			int iLength = getLength();

			if (!bForComparison_ && iLength < nDst_) {
				// When type length is smaller, cause an error or output warning according to argument
				if (bTruncateIsNotAllowed_) {
					if (!bNoThrow_)
						_TRMEISTER_THROW0(Exception::StringRightTruncation);
					else
						return false;
				}

				// Search for trailing spaces to determine whether warning should be output or not
				const ModUnicodeChar* p = pSrc + iLength;
				ModSize i = iLength;
				do {
					if (*p++ != UnicodeChar::usSpace) {
						//...
						//YET
						// Warning; string data, right truncation
						SydInfoMessage << "Warning: string data, right truncation" << ModEndl;
						break;
					}
				} while (++i < static_cast<ModSize>(nDst_));
			}
			nDst_ = iLength;
			break;
		}
	case Flag::Unlimited:
	case Flag::OldFixed:
	case Flag::OldVariable:
		{
			// always nDst = nSrc
			break;
		}
	default:
		{
			// never reach
			; _TRMEISTER_ASSERT(false);
			_TRMEISTER_THROW0(Exception::Unexpected);
		}
	}
	return true;
}

// FUNCTION public
//	Common::SQLData::castLanguageString -- 文字列からLanguage型へのキャスト
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_
//	bool bNoThrow_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castLanguageString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
				   bool bForComparison_, bool bNoThrow_) const
{
	// 普通にキャストして例外なら読み替える
	try {
		return castNormal(cSourceType_, pData_, pTarget_, bForComparison_, bNoThrow_);
	} catch (ModException&) {
		if (!bNoThrow_)
			_TRMEISTER_THROW0(Exception::InvalidCharacter);
	}
	return false;
}

// FUNCTION public
//	Common::SQLData::castBinaryBinary -- バイナリ型からバイナリ型へのキャスト
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_, bool bNoThrow_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castBinaryBinary(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
				 bool bForComparison_, bool bNoThrow_) const
{
	return castBinaryBinary_Internal(cSourceType_, pData_, pTarget_, bForComparison_, bNoThrow_);
}

// FUNCTION public
//	Common::SQLData::castToString -- 文字列型以外から文字列型へのキャスト
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_
//	bool bNoThrow_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castToString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
			 bool bForComparison_, bool bNoThrow_) const
{
	// Stringにキャストした後に文字列から文字列へのキャストを呼ぶ
	Data::Pointer pTmpData = new StringData;
	SQLData cStringType;
	Common::Data::getSQLType(pTmpData->getType(), cStringType);

	return cStringType.castNormal(cSourceType_, pData_, pTmpData, bForComparison_, bNoThrow_)
		&& castStringString_Internal(cStringType, pTmpData, pTarget_, bForComparison_, bNoThrow_, true /* no truncate */);
}

bool
SQLData::
castToUnicodeString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
					bool bForComparison_, bool bNoThrow_) const
{
	// Stringにキャストした後に文字列から文字列へのキャストを呼ぶ
	Data::Pointer pTmpData = new StringData;
	SQLData cStringType;
	Common::Data::getSQLType(pTmpData->getType(), cStringType);

	return cStringType.castNormal(cSourceType_, pData_, pTmpData, bForComparison_, bNoThrow_)
		&& castStringUnicodeString_Internal(cStringType, pTmpData, pTarget_, bForComparison_, bNoThrow_, true /* no truncate */);
}

// FUNCTION public
//	Common::SQLData::castToBinary -- バイナリ型以外からバイナリ型へのキャスト
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_
//	bool bNoThrow_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castToBinary(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
			 bool bForComparison_, bool bNoThrow_) const
{
	// Binaryにキャストした後にバイナリーからバイナリーへのキャストを呼ぶ
	Data::Pointer pTmpData = new BinaryData;
	SQLData cBinaryType;
	Common::Data::getSQLType(pTmpData->getType(), cBinaryType);

	return cBinaryType.castNormal(cSourceType_, pData_, pTmpData, bForComparison_, bNoThrow_)
		&& castBinaryBinary_Internal(cBinaryType, pTmpData, pTarget_, bForComparison_, bNoThrow_, true /* no truncate */);
}

// FUNCTION public
//	Common::SQLData::castStringString_Internal -- castStringStringの下請け
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_
//	bool bNoThrow_
//	bool bNoTruncate_ /* = false */
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castStringString_Internal(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
						  bool bForComparison_, bool bNoThrow_,
						  bool bNoTruncate_ /* = false */) const
{
	; _TRMEISTER_ASSERT(pData_->getType() == DataType::String);

	const StringData& cString = _SYDNEY_DYNAMIC_CAST(const StringData&, *pData_);

	// ASCIIでない文字があれば例外(比較のときはチェックしない)
	ModSize n = cString.getLength();
	if (!bForComparison_ && n) {
		const ModUnicodeChar* p = cString.getValue();
		do {
			if (!ModUnicodeCharTrait::isAscii(*p)) {
				if (!bNoThrow_)
					_TRMEISTER_THROW0(Exception::CharacterNotInRepertoire);
				else
					return false;
			}
			++p;
		} while (--n > 0);
	}

	// 後はNCHARと同じ処理
	return castStringUnicodeString_Internal(cSourceType_, pData_, pTarget_, bForComparison_, bNoThrow_, bNoTruncate_);
}

// FUNCTION public
//	Common::SQLData::castStringUnicodeString_Internal -- castStringUnicodeStringの下請け
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_
//	bool bNoThrow_
//	bool bNoTruncate_ /* = false */
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castStringUnicodeString_Internal(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
								 bool bForComparison_, bool bNoThrow_,
								 bool bNoTruncate_ /* = false */) const
{
	; _TRMEISTER_ASSERT(pData_->getType() == DataType::String);

	// Get target length
	int nDst;
	if (!getCastStringLength(nDst, cSourceType_, *pData_, bForComparison_, bNoThrow_, bNoTruncate_)) {
		return false;
	}

	// To reduce copying costs, use setValue instead of assign method
	const StringData& cString = _SYDNEY_DYNAMIC_CAST(const StringData&, *pData_);

	// Head pointer of source data
	const ModUnicodeChar* pSrc = cString.getValue();
	int nSrc = cString.getLength();

	; _TRMEISTER_ASSERT(pTarget_->getType() == DataType::String);
	StringData& cDst = _SYDNEY_DYNAMIC_CAST(StringData&, *pTarget_);

	if (pTarget_->getFunction() == Data::Function::None) {
		// member's ModUnicodeString can be modified only when Data has no function
		cDst.setNull(false);
		const_cast<ModUnicodeString&>(cDst.getValue()).reallocate(nDst);
		cDst.setValue(pSrc, ModMin(nSrc, nDst));
		// If target length is greater than source, append space
		while (nSrc < nDst) {
			const_cast<ModUnicodeString&>(cDst.getValue()).append(UnicodeChar::usSpace);
			++nSrc;
		}
	} else {
		// If any functions are applied, member cannot be accessed
		ModUnicodeString cstrData;
		cstrData.reallocate(nDst);
		cstrData.append(pSrc, ModMin(nSrc, nDst));
		// If target length is greater than source, append space
		while (nSrc < nDst) {
			cstrData.append(UnicodeChar::usSpace);
			++nSrc;
		}
		cDst.setValue(cstrData);
	}
	return true;
}

// FUNCTION public
//	Common::SQLData::castBinaryBinary_Internal -- castBinaryBinaryの下請け
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cSourceType_
//	const Data::Pointer& pData_
//	const Data::Pointer& pTarget_
//	bool bForComparison_
//	bool bNoThrow_
//	bool bNoTruncate_ /* = false */
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLData::
castBinaryBinary_Internal(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
						  bool bForComparison_, bool bNoThrow_,
						  bool bNoTruncate_ /* = false */) const
{
	; _TRMEISTER_ASSERT(pData_->getType() == DataType::Binary);

	// キャスト先の属性により動作が変わる
	switch (getFlag()) {
	case Flag::Variable:
		{
			if (getLength() <= 0)
				// 無制限なので代入でよい
				break;

			// 上限つきのBinary型はサポート範囲外
			_TRMEISTER_THROW0(Exception::NotSupported);
		}
	case Flag::Fixed:
		{
			const BinaryData& cBinary = _SYDNEY_DYNAMIC_CAST(const BinaryData&, *pData_);
			unsigned int uiSize = cBinary.getSize();
			unsigned int uiNewSize = getLength();

			// サイズが等しい場合は代入でよいので異なるときのみ処理する
			if (uiSize != uiNewSize) {
				BinaryData& cTarget = _SYDNEY_DYNAMIC_CAST(BinaryData&, *pTarget_);

				if (uiSize < uiNewSize) {
					// 代入もとのほうが短い場合は足りない分を0x00で埋める
					void* pValue = ModDefaultManager::allocate(uiNewSize);
					; _TRMEISTER_ASSERT(pValue);
					try {
						Os::Memory::copy(pValue, cBinary.getValue(), uiSize);
						Os::Memory::reset(syd_reinterpret_cast<char*>(pValue) + uiSize, uiNewSize - uiSize);
						cTarget.setValue(pValue, uiNewSize, false, uiNewSize);

					} catch (...) {
						ModDefaultManager::free(pValue, uiNewSize);
						_TRMEISTER_RETHROW;
					}
				} else {
					// 代入もとの方が長い場合は先頭からコピーする
					cTarget.setValue(cBinary.getValue(), uiNewSize);
					if (!bForComparison_) {
						if (bNoTruncate_) {
							if (!bNoThrow_)
								_TRMEISTER_THROW0(Exception::StringRightTruncation);
							else
								false;
						}
						// 残りに0x00以外があるか調べる
						for (unsigned int i = uiNewSize; i < uiSize; ++i) {
							if (*(syd_reinterpret_cast<const char*>(cBinary.getValue()) + i) != 0x00) {
								//...
								//YET
								// Warning; string data, right truncation
								SydInfoMessage << "Warning: string data, right truncation" << ModEndl;
								break;
							}
						}
					}
				}
				return true;
			}
			break;
		}
	case Flag::OldFixed:
	case Flag::OldVariable:
	case Flag::Unlimited:
		{
			// 常にcast不要
			break;
		}
	default:
		{
			// ありえない
			; _TRMEISTER_ASSERT(false);
			_TRMEISTER_THROW0(Exception::Unexpected);
		}
	}
	return castNormal(cSourceType_, pData_, pTarget_, bForComparison_, bNoThrow_);
}

// FUNCTION public
//	operator<< -- operator<< to output sql type to messages
//
// NOTES
//
// ARGUMENTS
//	ModMessageStream& cStream_
//	const _TRMEISTER::Common::SQLData& cType_
//	
// RETURN
//	ModMessageStream&
//
// EXCEPTIONS

ModMessageStream&
operator<<(ModMessageStream& cStream_, const _TRMEISTER::Common::SQLData& cType_)
{
	cStream_ << _cTypes[cType_.getType()]._pszName;
	if (_cTypes[cType_.getType()]._bUseScale) {
		cStream_ << '(' << cType_.getLength() << ',' << cType_.getScale() << ')';
	} else if (_cTypes[cType_.getType()]._bUseLength) {
		if (cType_.getFlag() == _TRMEISTER::Common::SQLData::Flag::Unlimited
			|| (cType_.getFlag() == _TRMEISTER::Common::SQLData::Flag::Variable
				&& cType_.getLength() == 0)) {
			cStream_ << "(no limit)";
		} else {
			cStream_ << '(' << cType_.getLength() << ')';
		}
	}
	if (cType_.isArrayType()) {
		cStream_ << "[]";
	}
	return cStream_;
}

// FUNCTION public
//	operator<< -- operator<< to output sql type to messages
//
// NOTES
//
// ARGUMENTS
//	ModOstream& cStream_
//	const _TRMEISTER::Common::SQLData& cType_
//	
// RETURN
//	ModOstream&
//
// EXCEPTIONS

ModOstream&
operator<<(ModOstream& cStream_, const _TRMEISTER::Common::SQLData& cType_)
{
	cStream_ << _cTypes[cType_.getType()]._pszName;
	if (_cTypes[cType_.getType()]._bUseScale) {
		cStream_ << '(' << cType_.getLength() << ',' << cType_.getScale() << ')';
	} else if (_cTypes[cType_.getType()]._bUseLength) {
		if (cType_.getFlag() == _TRMEISTER::Common::SQLData::Flag::Unlimited
			|| (cType_.getFlag() == _TRMEISTER::Common::SQLData::Flag::Variable
				&& cType_.getLength() == 0)) {
			cStream_ << "(no limit)";
		} else {
			cStream_ << '(' << cType_.getLength() << ')';
		}
	}
	if (cType_.isArrayType()) {
		cStream_ << "[]";
	}
	return cStream_;
}


//
//	Copyright (c) 1999, 2002, 2004, 2006, 2007, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
