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

#include "UnicodeDataRowBlocks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 任意のブロックの開始位置
unsigned short
UnicodeDataRowBlocks::getFirstCodeValue() const
{
	return (unsigned short)strtol(getField(0), (char**)NULL, 16);
}

// 任意のブロックの終了位置
unsigned short
UnicodeDataRowBlocks::getLastCodeValue()	const
{
	return (unsigned short)strtol(getField(1), (char**)NULL, 16);
}
