// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DefaultData.cpp -- DEFAULT データ関連の関数定義Function definition related to DEFAULT data
// 
// Copyright (c) 2006, 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "Common/DefaultData.h"
#include "Common/UnicodeString.h"

#include "Exception/BadArgument.h"

#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//	FUNCTION public
//	Common::DefaultData::getInstance -- DefaultDataのインスタンスを返す
//
//	NOTES
//		DefaultData のインスタンスはこの関数でしか得られない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた DefaultData のインスタンスを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

// static
const DefaultData*
DefaultData::getInstance()
{
	// DefaultData のインスタンスを新規に確保せずに
	// 即得るための予約されたインスタンス
	static const DefaultData reserved;

	return &reserved;
}

//	FUNCTION private
//	Common::DefaultData::DefaultData -- デフォルトコンストラクタ
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

DefaultData::DefaultData()
	: ScalarData(DataType::Default)
{}

//
//	FUNCTION public
//	Common::DefaultData::~DefaultData -- デストラクタ
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
DefaultData::
~DefaultData()
{
}

//	FUNCTION public
//	Common::DefaultData::serialize -- シリアル化
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
DefaultData::serialize(ModArchive& cArchiver_)
{
	Data::serialize_NotNull(cArchiver_);
}

//
//	FUNCTION public
//	Common::DefaultData::copy -- コピーする
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
DefaultData::copy() const
{
	return DefaultData::getInstance();
}

//
//	FUNCTION public
//	Common::DefaultData::getString -- 文字列でデータを得る
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
DefaultData::getString() const
{
	return _TRMEISTER_U_STRING("(default)");
}

// FUNCTION public
//	Common::DefaultData::assign -- 代入を行う
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
//			Common::DefaultData には代入できない

//virtual
bool
DefaultData::
assign(const Data* r, bool bForAssign_ /* = true */)
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

// FUNCTION public
//	Common::DefaultData::equals_NoCast -- 等しいか調べる(キャストなし)
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
DefaultData::
equals_NoCast(const Data& r) const
{
	// Defaultは比較できない
	_TRMEISTER_THROW0(Exception::BadArgument);
}

// FUNCTION public
//	Common::DefaultData::compareTo_NoCast -- 大小比較を行う(キャストなし)
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
DefaultData::
compareTo_NoCast(const Data& r) const
{
	// Defaultは比較できない
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//	FUNCTION public
//	Common::DefaultData::isAbleToDump -- ダンプできるか調べる
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
DefaultData::isAbleToDump() const
{
	return Data::isAbleToDump_NotNull();
}

//	FUNCTION public
//	Common::DefaultData::isFixedSize -- 常に固定長であるかを得る
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
DefaultData::isFixedSize() const
{
	return Data::isFixedSize_NotNull();
}

//	FUNCTION public
//	Common::DefaultData::getDumpSize -- ダンプサイズを得る
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
DefaultData::getDumpSize() const
{
	return Data::getDumpSize_NotNull();
}

//	FUNCTION public
//	Common::DefaultData::dumpValue -- 自分の値を与えられた領域にダンプする
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
DefaultData::dumpValue(char* dst) const
{
	return Data::dumpValue_NotNull(dst);
}

ModSize
DefaultData::dumpValue(ModSerialIO& cSerialIO_) const
{
	return Data::dumpValue_NotNull(cSerialIO_);
}

//
//	FUNCTION public
//	Common::DefaultData::getClassID -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::DefaultData クラスのクラスID
//
//	EXCEPTIONS
//	なし
//
int
DefaultData::
getClassID() const
{
	return ClassID::DefaultDataClass;
}

//
//	FUNCTION public
//	Common::DefaultData::print -- 値を表示する
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
DefaultData::
print() const
{
	cout << "default: " << endl;
}

// FUNCTION public
//	Common::DefaultData::hashCode -- ハッシュコードを取り出す
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
DefaultData::
hashCode() const
{
	if (isNull()) return 0;

	return static_cast<ModSize>(0xdefa);
}

//
//	Copyright (c) 2006, 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
