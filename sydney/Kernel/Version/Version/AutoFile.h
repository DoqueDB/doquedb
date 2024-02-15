// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoFile.h -- オートバージョンファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_AUTOFILE_H
#define	__SYDNEY_VERSION_AUTOFILE_H

#include "Version/Module.h"
#include "Version/File.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_VERSION_BEGIN

//	CLASS
//	Version::AutoFile -- オートバージョンファイル記述子を表すクラス
//
//	NOTES

class AutoFile
	: public	ModAutoPointer<File>
{
public:
	// コンストラクター
	AutoFile(File* file, bool reserve);
	// デストラクター
	~AutoFile();

	// バッファファイル記述子を破棄する
	void					free(bool reserve);

//	virtual void			free();
//	は呼び出すことはないので定義しない

private:
	// また参照されるときのために破棄せずにとっておくか
	const bool				_reserve;
};

//	FUNCTION public
//	Version::AutoFile::AutoFile --
//		オートバージョンファイル記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::File*		file
//			オートバージョンファイル記述子が保持する
//			バージョンファイル記述子を格納する領域の先頭アドレス
//		bool				reserve
//			true
//				このクラスの破棄時に、
//				保持するバージョンファイル記述子がどこからも参照されなくても、
//				また参照されるときのためにとっておく
//			false
//				このクラスの破棄時に、保持するバージョンファイル記述子が
//				どこからも参照されなければ、破棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoFile::AutoFile(File* file, bool reserve)
	: ModAutoPointer<File>(file),
	  _reserve(reserve)
{}

//	FUNCTION public
//	Version::AutoFile::~AutoFile --
//		オートバージョンファイル記述子を表すクラスのデストラクター
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
	free(_reserve);
}

//	FUNCTION public
//	Version::AutoFile::free -- 保持するバージョンファイル記述子を破棄する
//
//	NOTES
//		オートバージョンファイル記述子の破棄時などに呼び出される
//
//	ARGUMENTS
//		bool			reserve
//			true
//				どこからも参照されなくなったバージョンファイル記述子でも、
//				また参照されるときのために破棄せずにとっておく
//			false
//				どこからも参照されなくなったバージョンファイル記述子は破棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
AutoFile::free(bool reserve)
{
	if (isOwner())
		if (File* file = release())
			File::detach(file, reserve);
}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_AUTOFILE_H

//
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
