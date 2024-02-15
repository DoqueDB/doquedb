// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
//
// Unicode 文字情報ファイルのクラス
// 
// Copyright (c) 2023 Ricoh Company, Ltd.
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

#include "UnicodeDataRowUnicodeData.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

UnicodeDataRowUnicodeData::CodeValue
UnicodeDataRowUnicodeData::getCodeValue() const
{
	return (UnicodeDataRowUnicodeData::CodeValue)
		strtol(getField(0), (char**)NULL, 16);
}

int
UnicodeDataRowUnicodeData::getDecimalDigitValue() const
{
	return (int)strtol(getField(6), (char**)NULL, 10);
}

int
UnicodeDataRowUnicodeData::getDigitValue()	const
{
	return (int)strtol(getField(7), (char**)NULL, 10);
}

int
UnicodeDataRowUnicodeData::getNumericValue() const
{
	return (int)strtol(getField(8), (char**)NULL, 10);
}

bool
UnicodeDataRowUnicodeData::isMirrored() const
{
	return (!strcmp(getField(9), "Y") ? true : false);
}

UnicodeDataRowUnicodeData::CodeValue
UnicodeDataRowUnicodeData::getUpperCaseCodeValue() const
{
	return (UnicodeDataRowUnicodeData::CodeValue)
		strtol(getField(12), (char**)NULL, 16);
}

UnicodeDataRowUnicodeData::CodeValue
UnicodeDataRowUnicodeData::getLowerCaseCodeValue() const
{
	return (UnicodeDataRowUnicodeData::CodeValue)
		strtol(getField(13), (char**)NULL, 16);
}

UnicodeDataRowUnicodeData::CodeValue
UnicodeDataRowUnicodeData::getTitleCaseCodeValue() const
{
	return (UnicodeDataRowUnicodeData::CodeValue)
		strtol(getField(14), (char**)NULL, 16);
}
