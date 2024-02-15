// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoFile.h -- オートバッファファイル関連のクラス定義、関数宣言
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

#ifndef __SYDNEY_BUFFER_AUTOFILE_H
#define	__SYDNEY_BUFFER_AUTOFILE_H

#include "Buffer/Module.h"
#include "Buffer/File.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

#ifdef DUMMY
#else
//	CLASS
//	Buffer::AutoFile -- オートバッファファイル記述子を表すクラス
//
//	NOTES

class AutoFile
	: public	ModAutoPointer<File>
{
public:
	// コンストラクター
	AutoFile(File* file);
	// デストラクター
	~AutoFile();

	// バッファファイル記述子を破棄する
	virtual void			free();
};

//	FUNCTION public
//	Buffer::AutoFile::AutoFile --
//		オートバッファファイル記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::File*		file
//			オートバッファファイル記述子が保持する
//			バッファファイル記述子を格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoFile::AutoFile(File* file)
	: ModAutoPointer<File>(file)
{}

//	FUNCTION public
//	Buffer::AutoFile::~AutoFile --
//		オートバッファファイル記述子を表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoFile::~AutoFile()
{
	free();
}

//	FUNCTION public
//	Buffer::AutoFile::free -- 保持するバッファファイル記述子を破棄する
//
//	NOTES
//		オートバッファファイル記述子の破棄時などに呼び出される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
AutoFile::free()
{
	if (isOwner())
		if (File* file = release())
			File::detach(file);
}
#endif

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_AUTOFILE_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
