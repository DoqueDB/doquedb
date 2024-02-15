// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileOption.h -- Header file of variable-length record file option
// 
// Copyright (c) 2000, 2002, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD2_FILEOPTION_H
#define __SYDNEY_RECORD2_FILEOPTION_H

#include "Common/Common.h"
#include "FileCommon/FileOption.h"

_SYDNEY_BEGIN

namespace Record2
{

namespace FileOption
{
//------------------------
//	FileID の型変更に関するマクロ
//------------------------

namespace Parameter {
	typedef	LogicalFile::Parameter::Key		KeyType;
	typedef	ModUnicodeString				ValueType;

	namespace KeyNumber {
		enum Value {
			MinimumObjectPerPage = LogicalFile::FileID::DriverNumber::Record,
			VariablePageSize,
			Compressed,
			ValueNum
		};
	}
}

#define _SYDNEY_RECORD_FILE_PARAMETER_KEY(key_)              Record2::FileOption::Parameter::KeyType((key_))
#define _SYDNEY_RECORD_FILE_PARAMETER_FORMAT_KEY(key_, arg_) Record2::FileOption::Parameter::KeyType((key_), (arg_))
#define _SYDNEY_RECORD_FILE_PARAMETER_VALUE(value_)          (value_)

// Minimum values of number of objects stored on page 1 of fixed length file
namespace MinimumObjectPerPage
{

//
//	CONST
//	Record2::FileOption::MinimumObjectPerPage::Key --
//		固定長ファイルの1ページに格納されるオブジェクト数
//
//	NOTES
//
const int Key = Parameter::KeyNumber::MinimumObjectPerPage;

} // end of namespace MinimumObjectPerPage

// 固定長ファイルのページサイズ
namespace DirectPageSize
{

//
//	CONST
//	Record2::FileOption::DirectPageSize::Key --
//		固定長ファイルのページサイズ
//
//	NOTES
//
const int Key = FileCommon::FileOption::PageSize::Key;

} // end of namespace DirectPageSize


// 可変長ファイルのページサイズ
namespace VariablePageSize
{

//
//	CONST
//	Record2::FileOption::VariablePageSize::Key --
//		可変長ファイルのページサイズ
//
//	NOTES
//
const int Key = Parameter::KeyNumber::VariablePageSize;

} // end of namespace VariablePageSize

// 圧縮かどうか
namespace Compressed
{

//
//	CONST
//	Record2::FileOption::Compressed::Key -- 圧縮かどうか
//
//	NOTES
//
const int Key = Parameter::KeyNumber::Compressed;

} // end of namespace VariablePageSize

}

} // end of namespace Record2

_SYDNEY_END

#endif // __SYDNEY_RECORD2_FILEOPTION_H

//
//	Copyright (c) 2000, 2002, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
