// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnicodeFile.h -- Definition file of UnicodeFile class
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

#ifndef __UNICODEFILE__HEADER__
#define __UNICODEFILE__HEADER__

#include "LibUna/UnicodeChar.h"
#include "ModFile.h"
#include "LibUna/Module.h"

_UNA_BEGIN

//
//	CLASS
//		UnicodeFile -- Unicode ファイルアクセスクラス
//
	class UnicodeFile 
	{
	public:
		// コンストラクタ、デストラクタ
		UnicodeFile(const ModCharString& path_,
						 const ModKanjiCode::KanjiCodeType code_,
						 ModCodec* codec_);
		UnicodeFile(const ModUnicodeString& path_,
					ModFile::OpenMode mode_,
					const ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8,
					int control_ = ModFile::createFlag, int permission_ = 0666,
					ModCodec* codec_ = 0);

		~UnicodeFile();
	
		// 行の読み出し
		int getLine(ModUnicodeString& str_,	ModUnicodeChar sep_ = UnicodeChar::usCtrlRet);
	
		// open チェック
		ModBoolean isOpened() const;
	
	protected:
	
		// ファイル内容読込み
		void readFile();
	
	private:
	
		// ファイルのファイルコード
		ModKanjiCode::KanjiCodeType	_code;
	
		// ファイルの中身を文字列化して持つ
		//	余計なエンコードを防ぐため、ファイルを開いた直後に一気に読込む
		ModUnicodeString _content;
		const ModUnicodeChar* _readpos;

		// 管理するファイル	
		//	本当は直接継承して使用したいが
		//	なんかメモリが壊れるので一時的に
		//	メンバに持つことにする。
		ModFile				_file;
	
	};

_UNA_END
#endif // __UNICODEFILE__HEADER__

//
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
