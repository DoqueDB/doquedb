// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileOption.h -- 転置ファイルオプションのヘッダーファイル
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_FILEOPTION_H
#define __SYDNEY_VECTOR_FILEOPTION_H

#include "Common/Common.h"
#include "FileCommon/FileOption.h"

_SYDNEY_BEGIN

namespace Vector
{

namespace FileOption
{

//------------------------
//	FileID の型変更に関するマクロ
//------------------------

namespace Parameter {
	typedef	LogicalFile::Parameter::Key		KeyType;
	typedef	ModUnicodeString				ValueType;
}

#define _SYDNEY_VECTOR_FILE_PARAMETER_KEY(key_)              Vector::FileOption::Parameter::KeyType((key_))
#define _SYDNEY_VECTOR_FILE_PARAMETER_FORMAT_KEY(key_, arg_) Vector::FileOption::Parameter::KeyType((key_), (arg_))
#define _SYDNEY_VECTOR_FILE_PARAMETER_VALUE(value_)          (value_)


namespace ColumnNumber
{

//
//	CONST
//	Vector::FileOption::ColumnNumber::* --
//
//	NOTES
//	各カラムの番号
//
const ModSize TupleID			= 0;
const ModSize Score				= 1;
const ModSize MatchLocations	= 2;
const ModSize ColumnNumberLimit	= 3;

} // end of namespace IsLightWeightFile

} // end of namespace FileOption

} // end of namespace Vector

_SYDNEY_END

#endif // __SYDNEY_VECTOR_FILEOPTION_H

//
//	Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
