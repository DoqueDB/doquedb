// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileOption.h -- 全文検索ファイルID のヘッダーファイル
// 
// Copyright (c) 2000, 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_FILEOPTION_H
#define __SYDNEY_FULLTEXT_FILEOPTION_H

// 基底 クラス
#include "FullText/Module.h"
#include "FileCommon/FileOption.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

namespace FileOption
{

//------------------------
//	FileID の型変更に関するマクロ
//------------------------

namespace Parameter {
	typedef	LogicalFile::Parameter::Key	KeyType;
	typedef	ModUnicodeString			ValueType;

	namespace KeyNumber {
		enum Value {
			Delayed = LogicalFile::FileID::DriverNumber::FullText,
			Sectionized,
			Normalizing,
			SchemaIndexID,
			InvertedFileID,
			SectionInfoFileID,
			DelayProcFileID,
			InvFilePairNum,
			Language,
			ScoreModifier,
			OtherFieldNum,
			OtherFieldType,
			Clustered,
			UnnormalizedCharLength,
			ValueNum
		};
	}
}

#define _SYDNEY_FULLTEXT_FILE_PARAMETER_KEY(key_)              FullText::FileOption::Parameter::KeyType((key_))
#define _SYDNEY_FULLTEXT_FILE_PARAMETER_FORMAT_KEY(key_, arg_) FullText::FileOption::Parameter::KeyType((key_), (arg_))
#define _SYDNEY_FULLTEXT_FILE_PARAMETER_VALUE(value_)          (value_)

namespace DelayProc
{

//
//	CONST
//	FullText::FileOption::DelayProc::Key
//		-- 遅延処理ファイルの作成を指定するパラメータのキー
//
//	NOTES
//	遅延処理ファイルの作成を指定するパラメータのキー。
//	この値をキーに、真偽値を持つ
//	(全文検索独自のキーです)
// const char* const Key = "DelayProc";
//
//	2000/10/26
//		OpenOption.h に移動しました。
//		open で遅延処理を制御出来るようにする為
//
//	2000/11/27
//		OpenOption.h より戻ってきた。
//
const int Key = Parameter::KeyNumber::Delayed;

} // namespace DelayProc

namespace Sectionized
{

//
//	CONST
//	FullText::FileOption::Sectionized::Key --
//		セクション検索用の転置ファイルパラメータのキー
//
//	NOTES
//	セクション検索用の転置ファイルパラメータのキー。
//	このパラメータのバリューには真偽値を設定する。
//
//	 = "Sectionized"
//
const int Key = Parameter::KeyNumber::Sectionized;

} // end of namespace Sectionized

namespace IndexID
{

//
//	CONST
//	FileCommon::FileOption::IndexID::Key --
//		スキーマの索引 ID を示すパラメータのキー
//
//	NOTES
//	このパラメータのバリューには整数値を設定する。
//
const int Key = Parameter::KeyNumber::SchemaIndexID;

} // end of namespace IndexID

namespace InvertedFileID
{

//
//	CONST
//	FileCommon::FileOption::InvertedFileID::Key --
//		大転置ファイルのファイルIDを示すパラメータのキー
//
//	NOTES
//
const int Key = Parameter::KeyNumber::InvertedFileID;

} // end of namespace InvertedFileID

namespace SectionInfoFileID
{

//
//	CONST
//	FileCommon::FileOption::SectionInfoFileID::Key --
//	   	セクション情報ファイルのファイルIDを示すパラメータのキー
//
//	NOTES
//
const int Key = Parameter::KeyNumber::SectionInfoFileID;

} // end of namespace SectionInfoFileID

namespace DelayProcFileID
{

//
//	CONST
//	FileCommon::FileOption::DelayProcFileID::Key --
//	   	小転置ファイルのファイルIDを示すパラメータのキー
//
//	NOTES
//
const int Key = Parameter::KeyNumber::DelayProcFileID;

} // end of namespace SectionInfoFileID

namespace InvFilePairNum
{

//
//	CONST
//	FullText::FileOption::InvFilePairNum::Key --
//		小転置ファイル組の個数パラメータのキー
//
//	NOTES
//	小転置ファイルの組の個数パラメータのキー。
//	このパラメータのバリューには小転置ファイルの組の個数を
//	全文ファイルドライバが設定する。
//	指定されていない場合は、レジストリ値から読み取る。
//
const int Key = Parameter::KeyNumber::InvFilePairNum;

} // end of namespace DelayProcNum

namespace Language
{

//
//	CONST
//	FullText::FileOption::Language::Key --
//		言語フィールドが存在するかどうか
//
//	NOTES
//	このパラメータのバリューには言語フィールドが存在するかどうかを
//	true か false で設定する。
//
const int Key = Parameter::KeyNumber::Language;

} // end of namespace Language

namespace ScoreModifier
{

//
//	CONST
//	FullText::FileOption::ScoreModifier::Key --
//		スコア調整フィールドが存在するかどうか
//
//	NOTES
//	このパラメータのバリューにはスコア調整用のフィールドが存在するかどうかを
//	true か false で設定する。
//
const int Key = Parameter::KeyNumber::ScoreModifier;

} // end of namespace ScoreModifier

namespace OtherFieldNum
{

//
//	CONST
//	FullTest::FileOption::OtherFieldNum::Key --
//		その他情報ファイルのフィールド数
//
	
const int Key = Parameter::KeyNumber::OtherFieldNum;
}

namespace OtherFieldType
{

//
//	CONST
//	FullText::FileOption::OtherFieldType::Key --
//		その他情報ファイルのフィールドタイプ
//

const int Key = Parameter::KeyNumber::OtherFieldType;
}

namespace Clustered
{

//
//	CONST
//	FullText::FileOption::Clustered::Key --
//		クラスタリング情報を格納するかどうか
//

const int Key = Parameter::KeyNumber::Clustered;
}

namespace UnnormalizedCharLength
{
//
//	CONST
//	FullText::FileOption::UnnormalizedCharLength::Key --
//		正規化前の文字列長を格納するか？
//
const int Key = Parameter::KeyNumber::UnnormalizedCharLength;
}

} // end of namespace FileOption

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_FILEOPTION_H

//
//	Copyright (c) 2000, 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
