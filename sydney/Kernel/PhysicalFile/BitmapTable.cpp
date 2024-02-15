// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitmapTable.cpp --
//		空き領域管理機能付き物理ファイル用領域率ビットマップ変換表
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "PhysicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "PhysicalFile/BitmapTable.h"

_SYDNEY_USING

using namespace PhysicalFile;

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::BitmapTableクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//
//	CONST public
//	PhysicalFile::BitmapTable::InvalidBitmapValue --
//		無効な領域率ビットマップの値
//
//	NOTES
//	無効な領域率ビットマップの値。
//
// static
const unsigned char
BitmapTable::InvalidBitmapValue = 0xFF;

//
//	CONST public
//	PhysicalFile::BitmapTable::ToBitmapValue --
//		未使用領域率[％]と空き領域率[％]から
//		領域率ビットマップの値（8ビット）への変換表
//
//	NOTES
//	未使用領域率[％]と空き領域率[％]から
//	領域率ビットマップの値（8ビット）への変換表。
//
// static
const unsigned char
BitmapTable::ToBitmapValue[BitmapTable::RateNum][BitmapTable::RateNum] =
{
										// 未使用率	空き領域率

	0x00,								// 未確保	未確保
	BitmapTable::InvalidBitmapValue,	// 未確保	4％以下
	BitmapTable::InvalidBitmapValue,	// 未確保	5〜9％
	BitmapTable::InvalidBitmapValue,	// 未確保	10〜14％
	BitmapTable::InvalidBitmapValue,	// 未確保	15〜19％
	BitmapTable::InvalidBitmapValue,	// 未確保	20〜39％
	BitmapTable::InvalidBitmapValue,	// 未確保	40〜59％
	BitmapTable::InvalidBitmapValue,	// 未確保	60〜79％
	BitmapTable::InvalidBitmapValue,	// 未確保	80％以上

	BitmapTable::InvalidBitmapValue,	// 4％以下	未確保
	0x01,								// 4％以下	4％以下
	BitmapTable::InvalidBitmapValue,	// 4％以下	5〜9％
	BitmapTable::InvalidBitmapValue,	// 4％以下	10〜14％
	BitmapTable::InvalidBitmapValue,	// 4％以下	15〜19％
	BitmapTable::InvalidBitmapValue,	// 4％以下	20〜39％
	BitmapTable::InvalidBitmapValue,	// 4％以下	40〜59％
	BitmapTable::InvalidBitmapValue,	// 4％以下	60〜79％
	BitmapTable::InvalidBitmapValue,	// 4％以下	80％以上

	BitmapTable::InvalidBitmapValue,	// 5〜9％	未確保
	0x03,								// 5〜9％	4％以下
	0x02,								// 5〜9％	5〜9％
	BitmapTable::InvalidBitmapValue,	// 5〜9％	10〜14％
	BitmapTable::InvalidBitmapValue,	// 5〜9％	15〜19％
	BitmapTable::InvalidBitmapValue,	// 5〜9％	20〜39％
	BitmapTable::InvalidBitmapValue,	// 5〜9％	40〜59％
	BitmapTable::InvalidBitmapValue,	// 5〜9％	60〜79％
	BitmapTable::InvalidBitmapValue,	// 5〜9％	80％以上

	BitmapTable::InvalidBitmapValue,	// 10〜14％	未確保
	0x05,								// 10〜14％	4％以下
	0x06,								// 10〜14％	5〜9％
	0x04,								// 10〜14％	10〜14％
	BitmapTable::InvalidBitmapValue,	// 10〜14％	15〜19％
	BitmapTable::InvalidBitmapValue,	// 10〜14％	20〜39％
	BitmapTable::InvalidBitmapValue,	// 10〜14％	40〜59％
	BitmapTable::InvalidBitmapValue,	// 10〜14％	60〜79％
	BitmapTable::InvalidBitmapValue,	// 10〜14％	80％以上

	BitmapTable::InvalidBitmapValue,	// 15〜19％	未確保
	0x09,								// 15〜19％	4％以下
	0x0A,								// 15〜19％	5〜9％
	0x0C,								// 15〜19％	10〜14％
	0x08,								// 15〜19％	15〜19％
	BitmapTable::InvalidBitmapValue,	// 15〜19％	20〜39％
	BitmapTable::InvalidBitmapValue,	// 15〜19％	40〜59％
	BitmapTable::InvalidBitmapValue,	// 15〜19％	60〜79％
	BitmapTable::InvalidBitmapValue,	// 15〜19％	80％以上

	BitmapTable::InvalidBitmapValue,	// 20〜39％	未確保
	0x11,								// 20〜39％	4％以下
	0x12,								// 20〜39％	5〜9％
	0x14,								// 20〜39％	10〜14％
	0x18,								// 20〜39％	15〜19％
	0x10,								// 20〜39％	20〜39％
	BitmapTable::InvalidBitmapValue,	// 20〜39％	40〜59％
	BitmapTable::InvalidBitmapValue,	// 20〜39％	60〜79％
	BitmapTable::InvalidBitmapValue,	// 20〜39％	80％以上

	BitmapTable::InvalidBitmapValue,	// 40〜59％	未確保
	0x21,								// 40〜59％	4％以下
	0x22,								// 40〜59％	5〜9％
	0x24,								// 40〜59％	10〜14％
	0x28,								// 40〜59％	15〜19％
	0x30,								// 40〜59％	20〜39％
	0x20,								// 40〜59％	40〜59％
	BitmapTable::InvalidBitmapValue,	// 40〜59％	60〜79％
	BitmapTable::InvalidBitmapValue,	// 40〜59％	80％以上

	BitmapTable::InvalidBitmapValue,	// 60〜79％	未確保
	0x41,								// 60〜79％	4％以下
	0x42,								// 60〜79％	5〜9％
	0x44,								// 60〜79％	10〜14％
	0x48,								// 60〜79％	15〜19％
	0x50,								// 60〜79％	20〜39％
	0x60,								// 60〜79％	40〜59％
	0x40,								// 60〜79％	60〜79％
	BitmapTable::InvalidBitmapValue,	// 60〜79％	80％以上

	BitmapTable::InvalidBitmapValue,	// 80％以上	未確保
	0x81,								// 80％以上	4％以下
	0x82,								// 80％以上	5〜9％
	0x84,								// 80％以上	10〜14％
	0x88,								// 80％以上	15〜19％
	0x90,								// 80％以上	20〜39％
	0xA0,								// 80％以上	40〜59％
	0xC0,								// 80％以上	60〜79％
	0x80								// 80％以上	80％以上
};

//
//	CONST public
//	PhysicalFile::BitmapTable::ToUnuseAreaRate --
//		領域率ビットマップの値（8ビット）から未使用領域率[％]への変換表
//
//	NOTES
//	領域率ビットマップの値（8ビット）から未使用領域率[％]への変換表。
//
// static
const BitmapTable::Rate
BitmapTable::ToUnuseAreaRate[256] =
{
								// 領域率ビットマップの値
								// （カッコ内はインデックス）

	BitmapTable::Unuse,			// 00000000(0)
	BitmapTable::Rate_04,		// 00000001(1)
	BitmapTable::Rate05_09,		// 00000010(2)
	BitmapTable::Rate05_09,		// 00000011(3)
	BitmapTable::Rate10_14,		// 00000100(4)
	BitmapTable::Rate10_14,		// 00000101(5)
	BitmapTable::Rate10_14,		// 00000110(6)
	BitmapTable::InvalidRate,	// 00000111(7)
	BitmapTable::Rate15_19,		// 00001000(8)
	BitmapTable::Rate15_19,		// 00001001(9)
	BitmapTable::Rate15_19,		// 00001010(10)
	BitmapTable::InvalidRate,	// 00001011(11)
	BitmapTable::Rate15_19,		// 00001100(12)
	BitmapTable::InvalidRate,	// 00001101(13)
	BitmapTable::InvalidRate,	// 00001110(14)
	BitmapTable::InvalidRate,	// 00001111(15)
	BitmapTable::Rate20_39,		// 00010000(16)
	BitmapTable::Rate20_39,		// 00010001(17)
	BitmapTable::Rate20_39,		// 00010010(18)
	BitmapTable::InvalidRate,	// 00010011(19)
	BitmapTable::Rate20_39,		// 00010100(20)
	BitmapTable::InvalidRate,	// 00010101(21)
	BitmapTable::InvalidRate,	// 00010110(22)
	BitmapTable::InvalidRate,	// 00010111(23)
	BitmapTable::Rate20_39,		// 00011000(24)
	BitmapTable::InvalidRate,	// 00011001(25)
	BitmapTable::InvalidRate,	// 00011010(26)
	BitmapTable::InvalidRate,	// 00011011(27)
	BitmapTable::InvalidRate,	// 00011100(28)
	BitmapTable::InvalidRate,	// 00011101(29)
	BitmapTable::InvalidRate,	// 00011110(30)
	BitmapTable::InvalidRate,	// 00011111(31)
	BitmapTable::Rate40_59,		// 00100000(32)
	BitmapTable::Rate40_59,		// 00100001(33)
	BitmapTable::Rate40_59,		// 00100010(34)
	BitmapTable::InvalidRate,	// 00100011(35)
	BitmapTable::Rate40_59,		// 00100100(36)
	BitmapTable::InvalidRate,	// 00100101(37)
	BitmapTable::InvalidRate,	// 00100110(38)
	BitmapTable::InvalidRate,	// 00100111(39)
	BitmapTable::Rate40_59,		// 00101000(40)
	BitmapTable::InvalidRate,	// 00101001(41)
	BitmapTable::InvalidRate,	// 00101010(42)
	BitmapTable::InvalidRate,	// 00101011(43)
	BitmapTable::InvalidRate,	// 00101100(44)
	BitmapTable::InvalidRate,	// 00101101(45)
	BitmapTable::InvalidRate,	// 00101110(46)
	BitmapTable::InvalidRate,	// 00101111(47)
	BitmapTable::Rate40_59,		// 00110000(48)
	BitmapTable::InvalidRate,	// 00110001(49)
	BitmapTable::InvalidRate,	// 00110010(50)
	BitmapTable::InvalidRate,	// 00110011(51)
	BitmapTable::InvalidRate,	// 00110100(52)
	BitmapTable::InvalidRate,	// 00110101(53)
	BitmapTable::InvalidRate,	// 00110110(54)
	BitmapTable::InvalidRate,	// 00110111(55)
	BitmapTable::InvalidRate,	// 00111000(56)
	BitmapTable::InvalidRate,	// 00111001(57)
	BitmapTable::InvalidRate,	// 00111010(58)
	BitmapTable::InvalidRate,	// 00111011(59)
	BitmapTable::InvalidRate,	// 00111100(60)
	BitmapTable::InvalidRate,	// 00111101(61)
	BitmapTable::InvalidRate,	// 00111110(62)
	BitmapTable::InvalidRate,	// 00111111(63)
	BitmapTable::Rate60_79,		// 01000000(64)
	BitmapTable::Rate60_79,		// 01000001(65)
	BitmapTable::Rate60_79,		// 01000010(66)
	BitmapTable::InvalidRate,	// 01000011(67)
	BitmapTable::Rate60_79,		// 01000100(68)
	BitmapTable::InvalidRate,	// 01000101(69)
	BitmapTable::InvalidRate,	// 01000110(70)
	BitmapTable::InvalidRate,	// 01000111(71)
	BitmapTable::Rate60_79,		// 01001000(72)
	BitmapTable::InvalidRate,	// 01001001(73)
	BitmapTable::InvalidRate,	// 01001010(74)
	BitmapTable::InvalidRate,	// 01001011(75)
	BitmapTable::InvalidRate,	// 01001100(76)
	BitmapTable::InvalidRate,	// 01001101(77)
	BitmapTable::InvalidRate,	// 01001110(78)
	BitmapTable::InvalidRate,	// 01001111(79)
	BitmapTable::Rate60_79,		// 01010000(80)
	BitmapTable::InvalidRate,	// 01010001(81)
	BitmapTable::InvalidRate,	// 01010010(82)
	BitmapTable::InvalidRate,	// 01010011(83)
	BitmapTable::InvalidRate,	// 01010100(84)
	BitmapTable::InvalidRate,	// 01010101(85)
	BitmapTable::InvalidRate,	// 01010110(86)
	BitmapTable::InvalidRate,	// 01010111(87)
	BitmapTable::InvalidRate,	// 01011000(88)
	BitmapTable::InvalidRate,	// 01011001(89)
	BitmapTable::InvalidRate,	// 01011010(90)
	BitmapTable::InvalidRate,	// 01011011(91)
	BitmapTable::InvalidRate,	// 01011100(92)
	BitmapTable::InvalidRate,	// 01011101(93)
	BitmapTable::InvalidRate,	// 01011110(94)
	BitmapTable::InvalidRate,	// 01011111(95)
	BitmapTable::Rate60_79,		// 01100000(96)
	BitmapTable::InvalidRate,	// 01100001(97)
	BitmapTable::InvalidRate,	// 01100010(98)
	BitmapTable::InvalidRate,	// 01100011(99)
	BitmapTable::InvalidRate,	// 01100100(100)
	BitmapTable::InvalidRate,	// 01100101(101)
	BitmapTable::InvalidRate,	// 01100110(102)
	BitmapTable::InvalidRate,	// 01100111(103)
	BitmapTable::InvalidRate,	// 01101000(104)
	BitmapTable::InvalidRate,	// 01101001(105)
	BitmapTable::InvalidRate,	// 01101010(106)
	BitmapTable::InvalidRate,	// 01101011(107)
	BitmapTable::InvalidRate,	// 01101100(108)
	BitmapTable::InvalidRate,	// 01101101(109)
	BitmapTable::InvalidRate,	// 01101110(110)
	BitmapTable::InvalidRate,	// 01101111(111)
	BitmapTable::InvalidRate,	// 01110000(112)
	BitmapTable::InvalidRate,	// 01110001(113)
	BitmapTable::InvalidRate,	// 01110010(114)
	BitmapTable::InvalidRate,	// 01110011(115)
	BitmapTable::InvalidRate,	// 01110100(116)
	BitmapTable::InvalidRate,	// 01110101(117)
	BitmapTable::InvalidRate,	// 01110110(118)
	BitmapTable::InvalidRate,	// 01110111(119)
	BitmapTable::InvalidRate,	// 01111000(120)
	BitmapTable::InvalidRate,	// 01111001(121)
	BitmapTable::InvalidRate,	// 01111010(122)
	BitmapTable::InvalidRate,	// 01111011(123)
	BitmapTable::InvalidRate,	// 01111100(124)
	BitmapTable::InvalidRate,	// 01111101(125)
	BitmapTable::InvalidRate,	// 01111110(126)
	BitmapTable::InvalidRate,	// 01111111(127)
	BitmapTable::Rate80_,		// 10000000(128)
	BitmapTable::Rate80_,		// 10000001(129)
	BitmapTable::Rate80_,		// 10000010(130)
	BitmapTable::InvalidRate,	// 10000011(131)
	BitmapTable::Rate80_,		// 10000100(132)
	BitmapTable::InvalidRate,	// 10000101(133)
	BitmapTable::InvalidRate,	// 10000110(134)
	BitmapTable::InvalidRate,	// 10000111(135)
	BitmapTable::Rate80_,		// 10001000(136)
	BitmapTable::InvalidRate,	// 10001001(137)
	BitmapTable::InvalidRate,	// 10001010(138)
	BitmapTable::InvalidRate,	// 10001011(139)
	BitmapTable::InvalidRate,	// 10001100(140)
	BitmapTable::InvalidRate,	// 10001101(141)
	BitmapTable::InvalidRate,	// 10001110(142)
	BitmapTable::InvalidRate,	// 10001111(143)
	BitmapTable::Rate80_,		// 10010000(144)
	BitmapTable::InvalidRate,	// 10010001(145)
	BitmapTable::InvalidRate,	// 10010010(146)
	BitmapTable::InvalidRate,	// 10010011(147)
	BitmapTable::InvalidRate,	// 10010100(148)
	BitmapTable::InvalidRate,	// 10010101(149)
	BitmapTable::InvalidRate,	// 10010110(150)
	BitmapTable::InvalidRate,	// 10010111(151)
	BitmapTable::InvalidRate,	// 10011000(152)
	BitmapTable::InvalidRate,	// 10011001(153)
	BitmapTable::InvalidRate,	// 10011010(154)
	BitmapTable::InvalidRate,	// 10011011(155)
	BitmapTable::InvalidRate,	// 10011100(156)
	BitmapTable::InvalidRate,	// 10011101(157)
	BitmapTable::InvalidRate,	// 10011110(158)
	BitmapTable::InvalidRate,	// 10011111(159)
	BitmapTable::Rate80_,		// 10100000(160)
	BitmapTable::InvalidRate,	// 10100001(161)
	BitmapTable::InvalidRate,	// 10100010(162)
	BitmapTable::InvalidRate,	// 10100011(163)
	BitmapTable::InvalidRate,	// 10100100(164)
	BitmapTable::InvalidRate,	// 10100101(165)
	BitmapTable::InvalidRate,	// 10100110(166)
	BitmapTable::InvalidRate,	// 10100111(167)
	BitmapTable::InvalidRate,	// 10101000(168)
	BitmapTable::InvalidRate,	// 10101001(169)
	BitmapTable::InvalidRate,	// 10101010(170)
	BitmapTable::InvalidRate,	// 10101011(171)
	BitmapTable::InvalidRate,	// 10101100(172)
	BitmapTable::InvalidRate,	// 10101101(173)
	BitmapTable::InvalidRate,	// 10101110(174)
	BitmapTable::InvalidRate,	// 10101111(175)
	BitmapTable::InvalidRate,	// 10110000(176)
	BitmapTable::InvalidRate,	// 10110001(177)
	BitmapTable::InvalidRate,	// 10110010(178)
	BitmapTable::InvalidRate,	// 10110011(179)
	BitmapTable::InvalidRate,	// 10110100(180)
	BitmapTable::InvalidRate,	// 10110101(181)
	BitmapTable::InvalidRate,	// 10110110(182)
	BitmapTable::InvalidRate,	// 10110111(183)
	BitmapTable::InvalidRate,	// 10111000(184)
	BitmapTable::InvalidRate,	// 10111001(185)
	BitmapTable::InvalidRate,	// 10111010(186)
	BitmapTable::InvalidRate,	// 10111011(187)
	BitmapTable::InvalidRate,	// 10111100(188)
	BitmapTable::InvalidRate,	// 10111101(189)
	BitmapTable::InvalidRate,	// 10111110(190)
	BitmapTable::InvalidRate,	// 10111111(191)
	BitmapTable::Rate80_,		// 11000000(192)
	BitmapTable::InvalidRate,	// 11000001(193)
	BitmapTable::InvalidRate,	// 11000010(194)
	BitmapTable::InvalidRate,	// 11000011(195)
	BitmapTable::InvalidRate,	// 11000100(196)
	BitmapTable::InvalidRate,	// 11000101(197)
	BitmapTable::InvalidRate,	// 11000110(198)
	BitmapTable::InvalidRate,	// 11000111(199)
	BitmapTable::InvalidRate,	// 11001000(200)
	BitmapTable::InvalidRate,	// 11001001(201)
	BitmapTable::InvalidRate,	// 11001010(202)
	BitmapTable::InvalidRate,	// 11001011(203)
	BitmapTable::InvalidRate,	// 11001100(204)
	BitmapTable::InvalidRate,	// 11001101(205)
	BitmapTable::InvalidRate,	// 11001110(206)
	BitmapTable::InvalidRate,	// 11001111(207)
	BitmapTable::InvalidRate,	// 11010000(208)
	BitmapTable::InvalidRate,	// 11010001(209)
	BitmapTable::InvalidRate,	// 11010010(210)
	BitmapTable::InvalidRate,	// 11010011(211)
	BitmapTable::InvalidRate,	// 11010100(212)
	BitmapTable::InvalidRate,	// 11010101(213)
	BitmapTable::InvalidRate,	// 11010110(214)
	BitmapTable::InvalidRate,	// 11010111(215)
	BitmapTable::InvalidRate,	// 11011000(216)
	BitmapTable::InvalidRate,	// 11011001(217)
	BitmapTable::InvalidRate,	// 11011010(218)
	BitmapTable::InvalidRate,	// 11011011(219)
	BitmapTable::InvalidRate,	// 11011100(220)
	BitmapTable::InvalidRate,	// 11011101(221)
	BitmapTable::InvalidRate,	// 11011110(222)
	BitmapTable::InvalidRate,	// 11011111(223)
	BitmapTable::InvalidRate,	// 11100000(224)
	BitmapTable::InvalidRate,	// 11100001(225)
	BitmapTable::InvalidRate,	// 11100010(226)
	BitmapTable::InvalidRate,	// 11100011(227)
	BitmapTable::InvalidRate,	// 11100100(228)
	BitmapTable::InvalidRate,	// 11100101(229)
	BitmapTable::InvalidRate,	// 11100110(230)
	BitmapTable::InvalidRate,	// 11100111(231)
	BitmapTable::InvalidRate,	// 11101000(232)
	BitmapTable::InvalidRate,	// 11101001(233)
	BitmapTable::InvalidRate,	// 11101010(234)
	BitmapTable::InvalidRate,	// 11101011(235)
	BitmapTable::InvalidRate,	// 11101100(236)
	BitmapTable::InvalidRate,	// 11101101(237)
	BitmapTable::InvalidRate,	// 11101110(238)
	BitmapTable::InvalidRate,	// 11101111(239)
	BitmapTable::InvalidRate,	// 11110000(240)
	BitmapTable::InvalidRate,	// 11110001(241)
	BitmapTable::InvalidRate,	// 11110010(242)
	BitmapTable::InvalidRate,	// 11110011(243)
	BitmapTable::InvalidRate,	// 11110100(244)
	BitmapTable::InvalidRate,	// 11110101(245)
	BitmapTable::InvalidRate,	// 11110110(246)
	BitmapTable::InvalidRate,	// 11110111(247)
	BitmapTable::InvalidRate,	// 11111000(248)
	BitmapTable::InvalidRate,	// 11111001(249)
	BitmapTable::InvalidRate,	// 11111010(250)
	BitmapTable::InvalidRate,	// 11111011(251)
	BitmapTable::InvalidRate,	// 11111100(252)
	BitmapTable::InvalidRate,	// 11111101(253)
	BitmapTable::InvalidRate,	// 11111110(254)
	BitmapTable::InvalidRate	// 11111111(255)
};

//
//	CONST public
//	PhysicalFile::BitmapTable::ToFreeAreaRate --
//		領域率ビットマップの値（8ビット）から空き領域率[％]への変換表
//
//	NOTES
//	領域率ビットマップの値（8ビット）から空き領域率[％]への変換表。
//
// static
const BitmapTable::Rate
BitmapTable::ToFreeAreaRate[256] =
{
								// 領域率ビットマップの値
								// （カッコ内はインデックス）

	BitmapTable::Unuse,			// 00000000(0)
	BitmapTable::Rate_04,		// 00000001(1)
	BitmapTable::Rate05_09,		// 00000010(2)
	BitmapTable::Rate_04,		// 00000011(3)
	BitmapTable::Rate10_14,		// 00000100(4)
	BitmapTable::Rate_04,		// 00000101(5)
	BitmapTable::Rate05_09,		// 00000110(6)
	BitmapTable::InvalidRate,	// 00000111(7)
	BitmapTable::Rate15_19,		// 00001000(8)
	BitmapTable::Rate_04,		// 00001001(9)
	BitmapTable::Rate05_09,		// 00001010(10)
	BitmapTable::InvalidRate,	// 00001011(11)
	BitmapTable::Rate10_14,		// 00001100(12)
	BitmapTable::InvalidRate,	// 00001101(13)
	BitmapTable::InvalidRate,	// 00001110(14)
	BitmapTable::InvalidRate,	// 00001111(15)
	BitmapTable::Rate20_39,		// 00010000(16)
	BitmapTable::Rate_04,		// 00010001(17)
	BitmapTable::Rate05_09,		// 00010010(18)
	BitmapTable::InvalidRate,	// 00010011(19)
	BitmapTable::Rate10_14,		// 00010100(20)
	BitmapTable::InvalidRate,	// 00010101(21)
	BitmapTable::InvalidRate,	// 00010110(22)
	BitmapTable::InvalidRate,	// 00010111(23)
	BitmapTable::Rate15_19,		// 00011000(24)
	BitmapTable::InvalidRate,	// 00011001(25)
	BitmapTable::InvalidRate,	// 00011010(26)
	BitmapTable::InvalidRate,	// 00011011(27)
	BitmapTable::InvalidRate,	// 00011100(28)
	BitmapTable::InvalidRate,	// 00011101(29)
	BitmapTable::InvalidRate,	// 00011110(30)
	BitmapTable::InvalidRate,	// 00011111(31)
	BitmapTable::Rate40_59,		// 00100000(32)
	BitmapTable::Rate_04,		// 00100001(33)
	BitmapTable::Rate05_09,		// 00100010(34)
	BitmapTable::InvalidRate,	// 00100011(35)
	BitmapTable::Rate10_14,		// 00100100(36)
	BitmapTable::InvalidRate,	// 00100101(37)
	BitmapTable::InvalidRate,	// 00100110(38)
	BitmapTable::InvalidRate,	// 00100111(39)
	BitmapTable::Rate15_19,		// 00101000(40)
	BitmapTable::InvalidRate,	// 00101001(41)
	BitmapTable::InvalidRate,	// 00101010(42)
	BitmapTable::InvalidRate,	// 00101011(43)
	BitmapTable::InvalidRate,	// 00101100(44)
	BitmapTable::InvalidRate,	// 00101101(45)
	BitmapTable::InvalidRate,	// 00101110(46)
	BitmapTable::InvalidRate,	// 00101111(47)
	BitmapTable::Rate20_39,		// 00110000(48)
	BitmapTable::InvalidRate,	// 00110001(49)
	BitmapTable::InvalidRate,	// 00110010(50)
	BitmapTable::InvalidRate,	// 00110011(51)
	BitmapTable::InvalidRate,	// 00110100(52)
	BitmapTable::InvalidRate,	// 00110101(53)
	BitmapTable::InvalidRate,	// 00110110(54)
	BitmapTable::InvalidRate,	// 00110111(55)
	BitmapTable::InvalidRate,	// 00111000(56)
	BitmapTable::InvalidRate,	// 00111001(57)
	BitmapTable::InvalidRate,	// 00111010(58)
	BitmapTable::InvalidRate,	// 00111011(59)
	BitmapTable::InvalidRate,	// 00111100(60)
	BitmapTable::InvalidRate,	// 00111101(61)
	BitmapTable::InvalidRate,	// 00111110(62)
	BitmapTable::InvalidRate,	// 00111111(63)
	BitmapTable::Rate60_79,		// 01000000(64)
	BitmapTable::Rate_04,		// 01000001(65)
	BitmapTable::Rate05_09,		// 01000010(66)
	BitmapTable::InvalidRate,	// 01000011(67)
	BitmapTable::Rate10_14,		// 01000100(68)
	BitmapTable::InvalidRate,	// 01000101(69)
	BitmapTable::InvalidRate,	// 01000110(70)
	BitmapTable::InvalidRate,	// 01000111(71)
	BitmapTable::Rate15_19,		// 01001000(72)
	BitmapTable::InvalidRate,	// 01001001(73)
	BitmapTable::InvalidRate,	// 01001010(74)
	BitmapTable::InvalidRate,	// 01001011(75)
	BitmapTable::InvalidRate,	// 01001100(76)
	BitmapTable::InvalidRate,	// 01001101(77)
	BitmapTable::InvalidRate,	// 01001110(78)
	BitmapTable::InvalidRate,	// 01001111(79)
	BitmapTable::Rate20_39,		// 01010000(80)
	BitmapTable::InvalidRate,	// 01010001(81)
	BitmapTable::InvalidRate,	// 01010010(82)
	BitmapTable::InvalidRate,	// 01010011(83)
	BitmapTable::InvalidRate,	// 01010100(84)
	BitmapTable::InvalidRate,	// 01010101(85)
	BitmapTable::InvalidRate,	// 01010110(86)
	BitmapTable::InvalidRate,	// 01010111(87)
	BitmapTable::InvalidRate,	// 01011000(88)
	BitmapTable::InvalidRate,	// 01011001(89)
	BitmapTable::InvalidRate,	// 01011010(90)
	BitmapTable::InvalidRate,	// 01011011(91)
	BitmapTable::InvalidRate,	// 01011100(92)
	BitmapTable::InvalidRate,	// 01011101(93)
	BitmapTable::InvalidRate,	// 01011110(94)
	BitmapTable::InvalidRate,	// 01011111(95)
	BitmapTable::Rate40_59,		// 01100000(96)
	BitmapTable::InvalidRate,	// 01100001(97)
	BitmapTable::InvalidRate,	// 01100010(98)
	BitmapTable::InvalidRate,	// 01100011(99)
	BitmapTable::InvalidRate,	// 01100100(100)
	BitmapTable::InvalidRate,	// 01100101(101)
	BitmapTable::InvalidRate,	// 01100110(102)
	BitmapTable::InvalidRate,	// 01100111(103)
	BitmapTable::InvalidRate,	// 01101000(104)
	BitmapTable::InvalidRate,	// 01101001(105)
	BitmapTable::InvalidRate,	// 01101010(106)
	BitmapTable::InvalidRate,	// 01101011(107)
	BitmapTable::InvalidRate,	// 01101100(108)
	BitmapTable::InvalidRate,	// 01101101(109)
	BitmapTable::InvalidRate,	// 01101110(110)
	BitmapTable::InvalidRate,	// 01101111(111)
	BitmapTable::InvalidRate,	// 01110000(112)
	BitmapTable::InvalidRate,	// 01110001(113)
	BitmapTable::InvalidRate,	// 01110010(114)
	BitmapTable::InvalidRate,	// 01110011(115)
	BitmapTable::InvalidRate,	// 01110100(116)
	BitmapTable::InvalidRate,	// 01110101(117)
	BitmapTable::InvalidRate,	// 01110110(118)
	BitmapTable::InvalidRate,	// 01110111(119)
	BitmapTable::InvalidRate,	// 01111000(120)
	BitmapTable::InvalidRate,	// 01111001(121)
	BitmapTable::InvalidRate,	// 01111010(122)
	BitmapTable::InvalidRate,	// 01111011(123)
	BitmapTable::InvalidRate,	// 01111100(124)
	BitmapTable::InvalidRate,	// 01111101(125)
	BitmapTable::InvalidRate,	// 01111110(126)
	BitmapTable::InvalidRate,	// 01111111(127)
	BitmapTable::Rate80_,		// 10000000(128)
	BitmapTable::Rate_04,		// 10000001(129)
	BitmapTable::Rate05_09,		// 10000010(130)
	BitmapTable::InvalidRate,	// 10000011(131)
	BitmapTable::Rate10_14,		// 10000100(132)
	BitmapTable::InvalidRate,	// 10000101(133)
	BitmapTable::InvalidRate,	// 10000110(134)
	BitmapTable::InvalidRate,	// 10000111(135)
	BitmapTable::Rate15_19,		// 10001000(136)
	BitmapTable::InvalidRate,	// 10001001(137)
	BitmapTable::InvalidRate,	// 10001010(138)
	BitmapTable::InvalidRate,	// 10001011(139)
	BitmapTable::InvalidRate,	// 10001100(140)
	BitmapTable::InvalidRate,	// 10001101(141)
	BitmapTable::InvalidRate,	// 10001110(142)
	BitmapTable::InvalidRate,	// 10001111(143)
	BitmapTable::Rate20_39,		// 10010000(144)
	BitmapTable::InvalidRate,	// 10010001(145)
	BitmapTable::InvalidRate,	// 10010010(146)
	BitmapTable::InvalidRate,	// 10010011(147)
	BitmapTable::InvalidRate,	// 10010100(148)
	BitmapTable::InvalidRate,	// 10010101(149)
	BitmapTable::InvalidRate,	// 10010110(150)
	BitmapTable::InvalidRate,	// 10010111(151)
	BitmapTable::InvalidRate,	// 10011000(152)
	BitmapTable::InvalidRate,	// 10011001(153)
	BitmapTable::InvalidRate,	// 10011010(154)
	BitmapTable::InvalidRate,	// 10011011(155)
	BitmapTable::InvalidRate,	// 10011100(156)
	BitmapTable::InvalidRate,	// 10011101(157)
	BitmapTable::InvalidRate,	// 10011110(158)
	BitmapTable::InvalidRate,	// 10011111(159)
	BitmapTable::Rate40_59,		// 10100000(160)
	BitmapTable::InvalidRate,	// 10100001(161)
	BitmapTable::InvalidRate,	// 10100010(162)
	BitmapTable::InvalidRate,	// 10100011(163)
	BitmapTable::InvalidRate,	// 10100100(164)
	BitmapTable::InvalidRate,	// 10100101(165)
	BitmapTable::InvalidRate,	// 10100110(166)
	BitmapTable::InvalidRate,	// 10100111(167)
	BitmapTable::InvalidRate,	// 10101000(168)
	BitmapTable::InvalidRate,	// 10101001(169)
	BitmapTable::InvalidRate,	// 10101010(170)
	BitmapTable::InvalidRate,	// 10101011(171)
	BitmapTable::InvalidRate,	// 10101100(172)
	BitmapTable::InvalidRate,	// 10101101(173)
	BitmapTable::InvalidRate,	// 10101110(174)
	BitmapTable::InvalidRate,	// 10101111(175)
	BitmapTable::InvalidRate,	// 10110000(176)
	BitmapTable::InvalidRate,	// 10110001(177)
	BitmapTable::InvalidRate,	// 10110010(178)
	BitmapTable::InvalidRate,	// 10110011(179)
	BitmapTable::InvalidRate,	// 10110100(180)
	BitmapTable::InvalidRate,	// 10110101(181)
	BitmapTable::InvalidRate,	// 10110110(182)
	BitmapTable::InvalidRate,	// 10110111(183)
	BitmapTable::InvalidRate,	// 10111000(184)
	BitmapTable::InvalidRate,	// 10111001(185)
	BitmapTable::InvalidRate,	// 10111010(186)
	BitmapTable::InvalidRate,	// 10111011(187)
	BitmapTable::InvalidRate,	// 10111100(188)
	BitmapTable::InvalidRate,	// 10111101(189)
	BitmapTable::InvalidRate,	// 10111110(190)
	BitmapTable::InvalidRate,	// 10111111(191)
	BitmapTable::Rate60_79,		// 11000000(192)
	BitmapTable::InvalidRate,	// 11000001(193)
	BitmapTable::InvalidRate,	// 11000010(194)
	BitmapTable::InvalidRate,	// 11000011(195)
	BitmapTable::InvalidRate,	// 11000100(196)
	BitmapTable::InvalidRate,	// 11000101(197)
	BitmapTable::InvalidRate,	// 11000110(198)
	BitmapTable::InvalidRate,	// 11000111(199)
	BitmapTable::InvalidRate,	// 11001000(200)
	BitmapTable::InvalidRate,	// 11001001(201)
	BitmapTable::InvalidRate,	// 11001010(202)
	BitmapTable::InvalidRate,	// 11001011(203)
	BitmapTable::InvalidRate,	// 11001100(204)
	BitmapTable::InvalidRate,	// 11001101(205)
	BitmapTable::InvalidRate,	// 11001110(206)
	BitmapTable::InvalidRate,	// 11001111(207)
	BitmapTable::InvalidRate,	// 11010000(208)
	BitmapTable::InvalidRate,	// 11010001(209)
	BitmapTable::InvalidRate,	// 11010010(210)
	BitmapTable::InvalidRate,	// 11010011(211)
	BitmapTable::InvalidRate,	// 11010100(212)
	BitmapTable::InvalidRate,	// 11010101(213)
	BitmapTable::InvalidRate,	// 11010110(214)
	BitmapTable::InvalidRate,	// 11010111(215)
	BitmapTable::InvalidRate,	// 11011000(216)
	BitmapTable::InvalidRate,	// 11011001(217)
	BitmapTable::InvalidRate,	// 11011010(218)
	BitmapTable::InvalidRate,	// 11011011(219)
	BitmapTable::InvalidRate,	// 11011100(220)
	BitmapTable::InvalidRate,	// 11011101(221)
	BitmapTable::InvalidRate,	// 11011110(222)
	BitmapTable::InvalidRate,	// 11011111(223)
	BitmapTable::InvalidRate,	// 11100000(224)
	BitmapTable::InvalidRate,	// 11100001(225)
	BitmapTable::InvalidRate,	// 11100010(226)
	BitmapTable::InvalidRate,	// 11100011(227)
	BitmapTable::InvalidRate,	// 11100100(228)
	BitmapTable::InvalidRate,	// 11100101(229)
	BitmapTable::InvalidRate,	// 11100110(230)
	BitmapTable::InvalidRate,	// 11100111(231)
	BitmapTable::InvalidRate,	// 11101000(232)
	BitmapTable::InvalidRate,	// 11101001(233)
	BitmapTable::InvalidRate,	// 11101010(234)
	BitmapTable::InvalidRate,	// 11101011(235)
	BitmapTable::InvalidRate,	// 11101100(236)
	BitmapTable::InvalidRate,	// 11101101(237)
	BitmapTable::InvalidRate,	// 11101110(238)
	BitmapTable::InvalidRate,	// 11101111(239)
	BitmapTable::InvalidRate,	// 11110000(240)
	BitmapTable::InvalidRate,	// 11110001(241)
	BitmapTable::InvalidRate,	// 11110010(242)
	BitmapTable::InvalidRate,	// 11110011(243)
	BitmapTable::InvalidRate,	// 11110100(244)
	BitmapTable::InvalidRate,	// 11110101(245)
	BitmapTable::InvalidRate,	// 11110110(246)
	BitmapTable::InvalidRate,	// 11110111(247)
	BitmapTable::InvalidRate,	// 11111000(248)
	BitmapTable::InvalidRate,	// 11111001(249)
	BitmapTable::InvalidRate,	// 11111010(250)
	BitmapTable::InvalidRate,	// 11111011(251)
	BitmapTable::InvalidRate,	// 11111100(252)
	BitmapTable::InvalidRate,	// 11111101(253)
	BitmapTable::InvalidRate,	// 11111110(254)
	BitmapTable::InvalidRate	// 11111111(255)
};

// static
const BitmapTable::Rate
BitmapTable::ToRate[101] =
{
	BitmapTable::Rate_04,	//   0 [%]
	BitmapTable::Rate_04,	//   1
	BitmapTable::Rate_04,	//   2
	BitmapTable::Rate_04,	//   3
	BitmapTable::Rate_04,	//   4
	BitmapTable::Rate05_09,	//   5
	BitmapTable::Rate05_09,	//   6
	BitmapTable::Rate05_09,	//   7
	BitmapTable::Rate05_09,	//   8
	BitmapTable::Rate05_09,	//   9
	BitmapTable::Rate10_14,	//  10
	BitmapTable::Rate10_14,	//  11
	BitmapTable::Rate10_14,	//  12
	BitmapTable::Rate10_14,	//  13
	BitmapTable::Rate10_14,	//  14
	BitmapTable::Rate15_19,	//  15
	BitmapTable::Rate15_19,	//  16
	BitmapTable::Rate15_19,	//  17
	BitmapTable::Rate15_19,	//  18
	BitmapTable::Rate15_19,	//  19
	BitmapTable::Rate20_39,	//  20
	BitmapTable::Rate20_39,	//  21
	BitmapTable::Rate20_39,	//  22
	BitmapTable::Rate20_39,	//  23
	BitmapTable::Rate20_39,	//  24
	BitmapTable::Rate20_39,	//  25
	BitmapTable::Rate20_39,	//  26
	BitmapTable::Rate20_39,	//  27
	BitmapTable::Rate20_39,	//  28
	BitmapTable::Rate20_39,	//  29
	BitmapTable::Rate20_39,	//  30
	BitmapTable::Rate20_39,	//  31
	BitmapTable::Rate20_39,	//  32
	BitmapTable::Rate20_39,	//  33
	BitmapTable::Rate20_39,	//  34
	BitmapTable::Rate20_39,	//  35
	BitmapTable::Rate20_39,	//  36
	BitmapTable::Rate20_39,	//  37
	BitmapTable::Rate20_39,	//  38
	BitmapTable::Rate20_39,	//  39
	BitmapTable::Rate40_59,	//  40
	BitmapTable::Rate40_59,	//  41
	BitmapTable::Rate40_59,	//  42
	BitmapTable::Rate40_59,	//  43
	BitmapTable::Rate40_59,	//  44
	BitmapTable::Rate40_59,	//  45
	BitmapTable::Rate40_59,	//  46
	BitmapTable::Rate40_59,	//  47
	BitmapTable::Rate40_59,	//  48
	BitmapTable::Rate40_59,	//  49
	BitmapTable::Rate40_59,	//  50
	BitmapTable::Rate40_59,	//  51
	BitmapTable::Rate40_59,	//  52
	BitmapTable::Rate40_59,	//  53
	BitmapTable::Rate40_59,	//  54
	BitmapTable::Rate40_59,	//  55
	BitmapTable::Rate40_59,	//  56
	BitmapTable::Rate40_59,	//  57
	BitmapTable::Rate40_59,	//  58
	BitmapTable::Rate40_59,	//  59
	BitmapTable::Rate60_79,	//  60
	BitmapTable::Rate60_79,	//  61
	BitmapTable::Rate60_79,	//  62
	BitmapTable::Rate60_79,	//  63
	BitmapTable::Rate60_79,	//  64
	BitmapTable::Rate60_79,	//  65
	BitmapTable::Rate60_79,	//  66
	BitmapTable::Rate60_79,	//  67
	BitmapTable::Rate60_79,	//  68
	BitmapTable::Rate60_79,	//  69
	BitmapTable::Rate60_79,	//  70
	BitmapTable::Rate60_79,	//  71
	BitmapTable::Rate60_79,	//  72
	BitmapTable::Rate60_79,	//  73
	BitmapTable::Rate60_79,	//  74
	BitmapTable::Rate60_79,	//  75
	BitmapTable::Rate60_79,	//  76
	BitmapTable::Rate60_79,	//  77
	BitmapTable::Rate60_79,	//  78
	BitmapTable::Rate60_79,	//  79
	BitmapTable::Rate80_,	//  80
	BitmapTable::Rate80_,	//  81
	BitmapTable::Rate80_,	//  82
	BitmapTable::Rate80_,	//  83
	BitmapTable::Rate80_,	//  84
	BitmapTable::Rate80_,	//  85
	BitmapTable::Rate80_,	//  86
	BitmapTable::Rate80_,	//  87
	BitmapTable::Rate80_,	//  88
	BitmapTable::Rate80_,	//  89
	BitmapTable::Rate80_,	//  90
	BitmapTable::Rate80_,	//  91
	BitmapTable::Rate80_,	//  92
	BitmapTable::Rate80_,	//  93
	BitmapTable::Rate80_,	//  94
	BitmapTable::Rate80_,	//  95
	BitmapTable::Rate80_,	//  96
	BitmapTable::Rate80_,	//  97
	BitmapTable::Rate80_,	//  98
	BitmapTable::Rate80_,	//  99
	BitmapTable::Rate80_	// 100
};

//
//	Copyright (c) 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
