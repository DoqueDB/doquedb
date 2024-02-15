// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NullData.cpp -- NULL データ関連の関数定義
// 
// Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
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
#include "Common/NullData.h"
#include "Common/UnicodeString.h"

#include "Exception/BadArgument.h"

#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
}

//	FUNCTION public
//	Common::NullData::getInstance -- NullDataのインスタンスを返す
//
//	NOTES
//		NullData のインスタンスはこの関数でしか得られない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた NullData のインスタンスを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

// static
const NullData*
NullData::getInstance()
{
	// NullData のインスタンスを新規に確保せずに
	// 即得るための予約されたインスタンス
	static const NullData reserved;

	return &reserved;
}

//	FUNCTION private
//	Common::NullData::NullData -- デフォルトコンストラクタ
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

NullData::NullData()
	: ScalarData(DataType::Null)
{}

//
//	FUNCTION public
//	Common::NullData::~NullData -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
NullData::
~NullData()
{
}

//	FUNCTION public
//	Common::NullData::serialize -- シリアル化
//
//	NOTES
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバー
//	RETURN
//	なし
//
//	EXCEPTIONS

void
NullData::serialize(ModArchive& cArchiver_)
{
	Data::serialize_NotNull(cArchiver_);
}

//
//	FUNCTION public
//	Common::NullData::copy -- コピーする
//
//	NOTES
//	自分自身のコピーを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Data::Pointer
//		自分自身のコピー
//
//	EXCEPTIONS

Data::Pointer
NullData::copy() const
{
	return NullData::getInstance();
}

//
//	FUNCTION public
//	Common::NullData::getString -- 文字列でデータを得る
//
//	NOTES
//	文字列でデータを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列にしたデータ
//
//	EXCEPTIONS
//	なし
//
ModUnicodeString
NullData::getString() const
{
	return _TRMEISTER_U_STRING("(null)");
}

// FUNCTION public
//	Common::NullData::assign -- 代入を行う
//
// NOTES
//
// ARGUMENTS
//	const Data* r
//	
// RETURN
//	bool
//
// EXCEPTIONS
//		Exception::BadArgument
//			Common::NullData には代入できない

//virtual
bool
NullData::
assign(const Data* r, bool bForAssign_ /* = true */)
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

// FUNCTION public
//	Common::NullData::equals_NoCast -- 等しいか調べる(キャストなし)
//
// NOTES
//
// ARGUMENTS
//	const Data& r
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
NullData::
equals_NoCast(const Data& r) const
{
	return r.isNull();
}

// FUNCTION public
//	Common::NullData::compareTo_NoCast -- 大小比較を行う(キャストなし)
//
// NOTES
//
// ARGUMENTS
//	const Data& r
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
NullData::
compareTo_NoCast(const Data& r) const
{
	// Nullのほうが小さいと扱う
	return r.isNull() ? 0 : -1;
}

//	FUNCTION public
//	Common::Data::setNull -- NULL 値にする
//
//	NOTES
//
//	ARGUMENTS
//		bool		v
//			true または指定されないとき
//				NULL 値にする
//			false
//				NULL 値でなくする
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			Common::NullData は NULL 値でなくせない

void
NullData::setNull(bool v)
{
	if (!v)
		_TRMEISTER_THROW0(Exception::BadArgument);

	// DEFAULTではなくなる
	setDefault(false);
}

//	FUNCTION public
//	Common::NullData::isAbleToDump -- ダンプできるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ダンプできる
//		false
//			ダンプできない
//
//	EXCEPTIONS

bool
NullData::isAbleToDump() const
{
	return Data::isAbleToDump_NotNull();
}

//	FUNCTION public
//	Common::NullData::isFixedSize -- 常に固定長であるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			固定長である
//		false
//			固定長でない
//
//	EXCEPTIONS
//		なし

bool
NullData::isFixedSize() const
{
	return Data::isFixedSize_NotNull();
}

//	FUNCTION public
//	Common::NullData::getDumpSize -- ダンプサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたダンプサイズ(B 単位)
//
//	EXCEPTIONS

ModSize
NullData::getDumpSize() const
{
	return Data::getDumpSize_NotNull();
}

//	FUNCTION public
//	Common::NullData::dumpValue -- 自分の値を与えられた領域にダンプする
//
//	NOTES
//
//	ARGUMENTS
//		char* 		dst
//			自分の値をダンプする領域の先頭
//
//	RETURN
//		ダンプサイズ(B 単位)
//

ModSize
NullData::dumpValue(char* dst) const
{
	return Data::dumpValue_NotNull(dst);
}

ModSize
NullData::dumpValue(ModSerialIO& cSerialIO_) const
{
	return Data::dumpValue_NotNull(cSerialIO_);
}

//
//	FUNCTION public
//	Common::NullData::getClassID -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::NullData クラスのクラスID
//
//	EXCEPTIONS
//	なし
//
int
NullData::
getClassID() const
{
	return ClassID::NullDataClass;
}

//
//	FUNCTION public
//	Common::NullData::print -- 値を表示する
//
//	NOTES
//	値を表示する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
NullData::
print() const
{
	cout << "null: " << endl;
}

// FUNCTION public
//	Common::NullData::hashCode -- ハッシュコードを取り出す
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
NullData::
hashCode() const
{
	return 0;
}

//
//	Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
