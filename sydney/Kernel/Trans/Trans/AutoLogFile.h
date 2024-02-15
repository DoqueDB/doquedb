// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoLogFile.h -- オート論理ログファイル情報関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_TRANS_AUTOLOGFILE_H
#define	__SYDNEY_TRANS_AUTOLOGFILE_H

#include "Trans/Module.h"
#include "Trans/LogFile.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN
_SYDNEY_TRANS_LOG_BEGIN

//	CLASS
//	Trans::Log::AutoFile -- オート論理ログファイル情報を表すクラス
//
//	NOTES

class AutoFile
	: public	ModAutoPointer<File>
{
public:
	//	TYPEDEF
	//	Trans::Log::AutoFile::Super -- 親クラスを表す型
	//
	//	NOTES

	typedef	ModAutoPointer<File>	Super;

	// コンストラクター
	AutoFile(File* file);
	// コピーコンストラクター
	AutoFile(const AutoFile& src);
	// デストラクター
	~AutoFile();

	// = 演算子
	AutoFile&
	operator =(const AutoFile& src);

	// オート論理ログファイル情報を破棄する
	virtual
	void
	free();
	void
	free(bool reserve);
};

//	FUNCTION public
//	Trans::Log::AutoFile::AutoFile --
//		オート論理ログファイル情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File*	file
//			オート論理ログファイル情報が保持する
//			ある論理ログファイルに関する情報を記憶する
//			クラスを保持する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoFile::AutoFile(File* file)
	: Super(file)
{}

//	FUNCTION public
//	Trans::Log::AutoFile::AutoFile --
//		オート論理ログファイル情報を表すクラスのコピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::AutoFile&	src
//			コピー元のオート論理ログファイル情報
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoFile::AutoFile(const AutoFile& src)
	: Super((src.isOwner()) ? src->attach() : 0)
{}

//	FUNCTION public
//	Trans::Log::AutoFile::~AutoFile --
//		オート論理ログファイル情報を表すクラスのデストラクター
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
	free(true);
}

//	FUNCTION public
//	Trans::AutoFile::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::AutoFile&	src
//			自分自身に代入するオート論理ログファイル情報
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

inline
AutoFile&
AutoFile::operator =(const AutoFile& src)
{
	if (this != &src)
		if (src.isOwner())
			(void) Super::operator = (src->attach());
		else
			free(true);
	return *this;
}

//	FUNCTION public
//	Trans::AutoFile::free --
//		保持するある論理ログファイルに関する情報を記憶するクラスを破棄する
//
//	NOTES
//		オート論理ログファイル情報記述子の破棄時などに呼び出される
//
//	ARGUMENTS
//		bool				reserve
//			true
//				どこからも参照されなくなった論理ログファイル情報でも、
//				また参照されるときのためにとっておく
//			false
//				どこからも参照されなくなった論理ログファイル情報は破棄する
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
	free(true);
}

inline
void
AutoFile::free(bool reserve)
{
	if (isOwner())
		if (File* file = release())
			File::detach(file, reserve);
}

_SYDNEY_TRANS_LOG_END
_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_AUTOLOGFILE_H

//
// Copyright (c) 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
