// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnaParam.h -- define UNA's parameter key
// 
// Copyright (c) 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __UNA_PARAM_H
#define __UNA_PARAM_H

namespace UNA
{
namespace Param
{
//	The parameter key of set(const ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >& param_).
//	These key value is ModUnicodeString and to specify null string is to specify default.

namespace MaxWordLen
{
//
//	CONST
//	UNA::Param::MaxWordLen::Key --
//	key for MaxWordLength
//
//	NOTES
//	the key is for max word length
//	e.g.
//	"256": 256 characters (not bytes)
//	"0": No limit, this is default.
//
const char* const Key = "maxwordlen";
} // end of namespace MaxWordLen

namespace ExpMode
{
//
//	CONST
//	UNA::Param::ExpMode::Key --
//	key for ExpMode
//
//	NOTES
//	the key is for ExpMode
//	"0":ModNlpExpChkOrigStr
//	"1":ModNlpExpNoChk(default)
//
const char* const Key = "expmode";
} // end of namespace ExpMode

namespace Space
{
//
//	CONST
//	UNA::Param::Space::Key --
//	handling of treatment of space letter
//
//	NOTES
//	the key for handling of space
//	"0": ModNlpAsIs        --- treatment of space is not changed.(default)
//	                       e.g. If delete previously,delete this time, too. If no effect priviously,no efect this time, too. 
//	                            If previous treatment doesn't exist (when "0" is set first), previous treatment is "3"(ModNlpReset).
//	"1": ModNlpNoNormalize --- space letter is no effect.
//	"2": ModNlpDelete      --- space letter is deleted.
//	"3": ModNlpReset       --- space letter is treated according to rule of norm resource.
//
const char* const Key = "space";
} // end of namespace Space

namespace DoNorm
{
//
//	CONST
//	UNA::Param::DoNorm::Key --
//	the key for specifying to normalize
//
//	NOTES
//	This key expresses the normalize on/off switch.
//	This key exists to be compatible with v9 except occasion of getExpandTexts.
//	"true":normilize on(default)
//	other string: normalize off
//	This key value is passed through following v9 methods as ModTrue(if "true")/ModFalse(if other string).
//	getWord(ModUnicodeString&,const ModBoolean)
//	getWord(ModUnicodeString&,ModUnicodeString&,const ModBoolean)
//	getBlock(ModUnicodeString&,const ModUnicodeChar,const ModUnicodeChar,const ModBoolean)
//	getBlock(ModVector<ModUnicodeString>&,ModVector<int>&,const ModBoolean)
//	getBlock(ModVector<ModUnicodeString>&,ModVector<ModUnicodeString>&,ModVector<int>& posVector,const ModBoolean)
//	In getExpandTexts method,
//	this key value is used as switch whether the behaver of getExpandTexts is "expand only" or "normalize and expand"
//
const char* const Key = "donorm";
} // end of namespace DoNorm

namespace AnaNo
{
//
//	CONST
//	UNA::Param::AnaNo::Key --
//	key for AnaNo
//
//	NOTES
//	the key is for specifying analyzer by force.
//	"unajp": Japanese morph analyzer and NP extract.
//	Default is none.
//
const char* const Key = "anano";
} // end of namespace AnaNo

namespace Disable
{
//
//	CONST
//	UNA::Param::Disable::Key --
//	key for Disable
//
//	NOTES
//	the key is for specifying to disable analyzer.
//	"unajp": Japanese morph analyzer and NP extract.
//	Default is none.
//
const char* const Key = "disable";
} // end of namespace Disable

namespace Stemming
{
//
//	CONST
//	UNA::Param::Stemming::Key --
//	key for Stemming
//
//	NOTES
//	This key is to specify whether stemming is done.
//	"true": stemming is done
//	other string: stemming isn't done.(default)
//
const char* const Key = "stem";
} // end of namespace Stemming

namespace CompoundDiv
{
//
//	CONST
//	UNA::Param::CompoundDiv::Key --
//	key for dividing compound words.
//
//	NOTES
//	This key is to specify whether compound words are divided.
//	"true": compaund words is divided
//	other string: dividing compaund words isn't done.(default)
//
const char* const Key = "compound";
} // end of namespace CompoundDiv

namespace CarriageRet
{
//
//	CONST
//	UNA::Param::CarriageRet::Key --
//	key for a word including carriage return
//
//	NOTES
//	This key is to specify whether words including carriage return are detected.
//	"true": words including carriage return are detected
//	other string: detecting words including carriage return isn't done.(default)
//
const char* const Key = "carriage";
} // end of namespace CarriageRet

namespace LengthOfNormalization
{
//
//	CONST
//	UNA::Param::LengthOfNormalization::Key --
//	show the target length of Normalization
//
//	NOTES
//	effective for getWholeText and getWholeExpandTexts
//	This key is valid only when NormalizeOnly key is defined.
//	"0": target text is whole text(default)
//	"1"-: target text is the text from position=0 to position=legthOfNormalization - 1
//	* modules except for unaWd don't support this parameter!="0"
//
const char* const Key = "lengthofnormalization";
} // end of namespace LengthOfNormalization

namespace MaxExpTargetStrLen
{
//
//	CONST
//	UNA::Param::MaxExpTargetStrLen::Key --
//	key for setting the maximum length of character string for expanding
//
//	NOTES
//	effective for getExpandStrings
//	This key is valid only when ExpData key is defined.
//	e.g.
//	"32": 32 characters (not pattern)
//	"40": 40 characters, this is default.
//
const char* const Key = "maxexptargetstrlen";
} // end of namespace MaxExpTargetStrLen

namespace MaxExpPatternNum
{
//
//	CONST
//	UNA::Param::MaxExpPatternNum::Key --
//	key for setting the maximum number of expanded patterns
//
//	NOTES
//	effective for getExpandStrings
//	This key is valid only when ExpData key is defined.
//	e.g.
//	"32": 32 patterns
//	"40": 40 patterns, this is default.
//
const char* const Key = "maxexppatternnum";
} // end of namespace MaxExpPatternNum
} // end of namespace Param
} // end of namespace UNA
#endif //__UNA_PARAM_H

//
// Copyright (c) 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
