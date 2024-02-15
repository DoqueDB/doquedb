// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	UnicodeFile.cpp -- UnicodeFile 実装ファイル
// 
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
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

#include "LibUna/UnicodeFile.h"

_UNA_USING

//
// FUNCTION public
//	UnicodeFile::UnicodeFile
//		-- UnicodeFile クラスのコンストラクタ
//
// NOTES
//
// ARGUMENTS
//		const ModCharString& path_
//			パス名
//		const ModUnicodestring& path_
//			パス名
//		ModFile::OpenMode mode_,
//			オープンモード
//		const ModKanjiCode::KanjiCodeType code_ = ModKanjiCode::utf8,
//			ファイルの文字コード
//		long control_ = ModFile::createFlag
//			作成フラグ
//		long permission_ = 0666,
//			アクセス制限フラグ
//		ModCodec* codec_ = 0
//			コーデック
//
// RETURN
//		なし
//
// EXCEPTIONS
//
UnicodeFile::UnicodeFile(const ModUnicodeString& path_,
						 ModFile::OpenMode mode_,
						 const ModKanjiCode::KanjiCodeType code_,
						 int control_, int permission_,
						 ModCodec* codec_)
	 : _code(code_), _file(path_, mode_, control_, permission_, codec_)
{
	readFile();
}

UnicodeFile::UnicodeFile(const ModCharString& path_,
						 const ModKanjiCode::KanjiCodeType code_,
						 ModCodec* codec_)
	 : _code(code_), _file(path_, codec_)
{
	readFile();
}

//
// FUNCTION public
//	UnicodeFile::~UnicodeFile
//		-- UnicodeFile クラスのデストラクタ
//
// NOTES
//
// ARGUMENTS
//		なし
//
// RETURN
//		なし
//
// EXCEPTIONS
//
UnicodeFile::~UnicodeFile()
{
}	

//
// FUNCTION pubic
//	UnicodeFile::readFile -- ファイル内容読込み
//
// NOTES
//	_content にファイルの全内容を読込む
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
void
UnicodeFile::readFile()
{
	// ファイルサイズ取得
	int size = static_cast<int>(_file.getFileSize());

	// 初回の BOM を読み飛ばす
	if ( _file.getCurrentPosition() == 0 ) {
		_file.seek(3, ModFile::seekSet);
		size -= 3;
	}

	// 読込み開始
	char* buf = new char[++size];

	size = _file.read(buf, size);
	buf[size] = '\0';

	_content = ModUnicodeString(buf, _code);
	_readpos = _content;

	delete [] buf;
}

//
// FUNCTION pubic
//	UnicodeFile::getline -- 行の読み出し
//
// NOTES
//
// ARGUMENTS
//		ModUnicodeString& str_
//			読み出した行
//		char sep_ = UnicodeChar::usCtrlRet
//			行の区切り。デフォルトで改行コード
//
//	RETURN
//		int	読み出した文字長さ
//
// EXCEPTIONS
//
int
UnicodeFile::getLine(ModUnicodeString& str_, ModUnicodeChar sep_)
{
	str_.clear();

	const ModUnicodeChar* end = static_cast<const ModUnicodeChar*>(_content) + _content.getLength();

	// 直前の \r と \n は飛ばす
	for ( ; _readpos < end; ++_readpos ) {
		if ( *_readpos != ModUnicodeChar('\r') &&
			 *_readpos != ModUnicodeChar('\n') )
			break;
	}



 	// sep_ まで読み飛ばす
	const ModUnicodeChar* start = _readpos;
	for ( ; _readpos < end; ++_readpos ) {
		if ( *_readpos == ModUnicodeChar('\r') &&
			*(_readpos+1) == ModUnicodeChar('\n') ) {
			break;
		}
	}

	str_ = ModUnicodeString(start, (ModSize)(_readpos - start));

	return str_.getLength();
}

// open check
ModBoolean
UnicodeFile::isOpened() const
{
	return _file.isOpened(); 
}

//
//	Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
