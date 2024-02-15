// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.cpp -- 共通基底クラス
// 
// Copyright (c) 1999, 2000, 2002, 2003, 2004, 2012, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/ClassID.h"
#include "Common/Externalizable.h"
#include "Common/MessageStream.h"
#include "Common/Object.h"
#include "Common/UnicodeString.h"

#include "Exception/NotSupported.h"
#include "Os/AutoCriticalSection.h"

#include "ModUnicodeString.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{

namespace _Object
{
	// 以下の情報の排他制御用のラッチ
	Os::CriticalSection		_latch;
	// 初期化の回数
	int						_initialized = 0;
}

}

//
//	FUNCTION public
//	Common::Object::toString -- 文字列でデータを取り出す
//
//	NOTES
//	文字列でデータを取り出す。主にデバッグ時に用いられる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		データをあらわす文字列
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ModUnicodeString
Object::toString() const
{
	return _TRMEISTER_U_STRING("Please overwrite this method(toString()).");
}

//
//	FUNCTION public
//	Common::Object::hashCode -- ハッシュコードを得る
//
//	NOTES
//	ハッシュコードを得る。
//	ハッシュテーブルに入れたいクラスはこのメソッドを上書きする。
//	ここではException::NotSupported例外を投げる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		ハッシュコード
//
//	EXCEPTIONS
//	Exception::NotSupported
//		このメソッドは継承側で実装しなければならない
//
ModSize
Object::hashCode() const
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//	FUNCTION public
//	Common::Object::initialize -- 初期化を行う
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

// static
void
Object::initialize()
{
	Os::AutoCriticalSection	latch(_Object::_latch);

	if (_Object::_initialized++ == 0) {
		Externalizable::insertFunction(
			Externalizable::CommonClasses, getClassInstance);
	}
}

//	FUNCTION public
//	Common::Object::terminate -- 終了処理を行う
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

// static
void
Object::terminate()
{}

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN
//	FUNCTION global
//	operator<< -- ModMessageにデータをあらわす文字列を書く
//
//	NOTES
//	ModMessageにデータをあらわす文字列を書き出す。
//	主にデバッグ時に用いられる。
//
//	ARGUMENTS
//	ModMessageStream& cStream
//		メッセージストリーム
//	const Common::Object& cObject
//		オブジェクト
//
//	RETURN
//	ModMessageStream&
//		メッセージストリームの参照
//
//	EXCEPTIONS

ModMessageStream&
operator<<(ModMessageStream& cStream, const Object& cObject)
{
	return cStream << cObject.toString();
}

//	FUNCTION global
//	operator<< -- MessageStreamにデータをあらわす文字列を書く
//
//	NOTES
//	MessageStream にデータをあらわす文字列を書き出す。
//	主にデバッグ時に用いられる。
//
//	ARGUMENTS
//	MessageStream& cStream
//		メッセージストリーム
//	const Common::Object& cObject
//		オブジェクト
//
//	RETURN
//	MessageStream&
//		メッセージストリームの参照
//
//	EXCEPTIONS

ModOstream&
operator<<(ModOstream& cStream_, const Object& cObject_)
{
	return cStream_ << cObject_.toString();
}
_TRMEISTER_COMMON_END
_TRMEISTER_END

//
//	Copyright (c) 1999, 2000, 2002, 2003, 2004, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
