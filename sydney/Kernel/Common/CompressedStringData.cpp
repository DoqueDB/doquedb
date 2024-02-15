// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedStringData.cpp -- 圧縮された文字列型データ関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2009, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/ClassID.h"
#include "Common/CompressedBinaryData.h"
#include "Common/CompressedStringData.h"
#ifdef OBSOLETE
#include "Common/CompressedStreamStringData.h"
#endif
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Common/DoubleData.h"
#include "Common/FloatData.h"
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/LanguageData.h"
#include "Common/NullData.h"
#include "Common/ObjectIDData.h"
#include "Common/UnicodeString.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/ClassCast.h"
#include "Exception/NotSupported.h"
#include "Os/Memory.h"

#include "ModAutoPointer.h"
#include "ModDefaultManager.h"
#include "ModUnicodeString.h"
#include "ModUnicodeCharTrait.h"
#include "ModCharTrait.h"

#include <iostream>
#include <stdlib.h>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//	FUNCTION private
//	Common::CompressedStringData::serialize_NotNull -- シリアル化
//
//	NOTES
//	シリアル化を行う
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
CompressedStringData::
serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	// バイトオーダーの問題があるので
	// 圧縮したままネットワーク境界を越えないようにする。
	// ドライバーで伸張する方法では更新などで
	// きれいにいかないのでシリアライズに細工することにした
	// 2003/06/16 
	// 下記の方法でうまくいくためにはClassID.cppでこのクラスのgetClassIDに対応する
	// オブジェクトはStringDataにしなければならない

	if (cArchiver_.isStore()) {
		// 書き出しはStringDataとして行う
		(void) getValue();
		m_pStringData->serialize(cArchiver_);

	} else {
		// ClassID.cppでこのクラスのgetClassIDに対応する
		// オブジェクトはStringDataにしているので
		// ここにはこない
		; _TRMEISTER_ASSERT(false);
/*
		// 読み出し
		// 書き出し側でStringDataとしてシリアライズしているので
		// ここに来ることはない
		// 仮に来たら従来と同様に処理するしかない

		ScalarData::serialize_NotNull(cArchiver_);
		CompressedData::serialize(cArchiver_);
*/
	}
}

//	FUNCTION private
//	Common::CompressedStringData::copy_NotNull -- コピーする
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
CompressedStringData::
copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new CompressedStringData(*this);
}

//	FUNCTION private
//	Common::CompressedStringData::cast_NotNull -- キャストする
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type	type
//			このデータ型にキャストする
//
//	RETURN
//		キャストされたデータへのポインタ
//
//	EXCEPTIONS
//		Exception::ClassCast
//			不可能な型へキャストしようとした

Data::Pointer
CompressedStringData::cast_NotNull(DataType::Type type, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	if (type != DataType::Binary)
		return StringData(getValue()).cast(type, bForAssign_);

	; _TRMEISTER_ASSERT(type == DataType::Binary);
	return (getCompressedValue()) ?
		new CompressedBinaryData(
			getCompressedValue(), getCompressedSize(), getValueSize()) :
		new BinaryData();
}

//	FUNCTION private
//	Common::CompressedStringData::getValue_NotNull -- 格納されている値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		文字列を表すクラスへのレファレンス
//
//	EXCEPTIONS

const ModUnicodeString&
CompressedStringData::getValue_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	if (!m_pStringData.get()) {

		// 伸張後のデータが生成されていない

		// 伸張されたときのサイズを得る

		const ModSize uncompressedSize = getValueSize();

		if (!uncompressedSize)

			// 伸張後のデータの大きさは 0 なので、
			// 空の文字列データを生成しておく

			m_pStringData = new StringData();

		else {

			// 格納しているデータを、まず、伸張する

			ModUnicodeChar* buf
				= static_cast<ModUnicodeChar*>(
					ModDefaultManager::allocate(uncompressedSize));
			; _TRMEISTER_ASSERT(buf);

			try {
				uncompress(buf);

				// 伸張したデータを文字列データにする
				// uncompressされた結果はucs2

				m_pStringData =	new StringData(
					buf, uncompressedSize / sizeof(ModUnicodeChar));

			} catch (...) {

				ModDefaultManager::free(buf, uncompressedSize);
				_TRMEISTER_RETHROW;
			}

			ModDefaultManager::free(buf, uncompressedSize);
		}
	}

	return m_pStringData->getValue();
}

//	FUNCTION public
//	Common::CompressedStringData::setValue -- 与えられた値を格納する
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	v
//			圧縮して格納する文字列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
CompressedStringData::setValue(const ModUnicodeString& v)
{
	// まず、(伸張された)データを格納していれば、クリアする

	m_pStringData = static_cast<StringData*>(0);

	// ucs2で取り出す

	const ModUnicodeChar* buf = v;
	ModSize bufferSize = v.getLength() * sizeof(ModUnicodeChar);

	// 与えられたデータを圧縮して格納する

	compress(buf, bufferSize);

	// NULL 値でなくする

	setNull(false);
}

//	FUNCTION private
//	Common::CompressedStringData::getString_NotNull -- 格納されている文字列を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		文字列を表すクラスへのレファレンス
//
//	EXCEPTIONS

ModUnicodeString
CompressedStringData::getString_NotNull() const
{
	return getValue();
}

//	FUNCTION private
//	Common::CompressedStringData::equals_NoCast -- 等しいか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//
//	RETURN
//		true
//			自分自身と与えられたデータは等しい
//		false
//			自分自身と与えられたデータは等しくない
//
//	EXCEPTIONS

bool
CompressedStringData::equals_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->equals(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::String);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	if (!(r.getFunction() & Function::Compressed))

		// 与えられたデータは圧縮されていない文字列型データなので、
		// 自分自身は解凍しないと等しいかわからない

		return !compareTo_NoCast(r);

	// 与えられたデータは圧縮されているので、
	// 解凍しないでそのまま等しいか調べる

	const CompressedStringData& data =
		_SYDNEY_DYNAMIC_CAST(const CompressedStringData&, r);

	return getCompressedSize() == data.getCompressedSize() &&
		!(getCompressedSize() &&
		  Os::Memory::compare(getCompressedValue(),
							  data.getCompressedValue(), getCompressedSize()));
}

//	FUNCTION private
//	Common::CompressedStringData::compareTo_NoCast -- 大小比較を行う
//
//	NOTES
//		比較するデータは自分自身と同じ型である必要がある
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//
//	RETURN
//		0
//			左辺と右辺は等しい
//		-1
//			左辺のほうが小さい
//		1
//			右辺のほうが小さい
//
//	EXCEPTIONS
//		なし

int
CompressedStringData::compareTo_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->compareTo(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::String);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	// 自分自身が伸張されていなければ、伸張しておく

	(void) getValue();
	; _TRMEISTER_ASSERT(m_pStringData.get());

	const StringData& data = _SYDNEY_DYNAMIC_CAST(const StringData&, r);

	return m_pStringData->compareTo_NoCast(data);
}

//
//	FUNCTION private
//	Common::CompressedStringData::connect_NotNull -- 文字列連結を行う
//
//	NOTES
//	文字列連結を行う。
//
//	ARGUMENTS
//	const Common::StringData* pStringData_
//		連結する文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
CompressedStringData::
connect_NotNull(const StringData* pStringData_)
{
	; _TRMEISTER_ASSERT(pStringData_);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!pStringData_->isNull());

	// 連結は圧縮しなおす
	ModUnicodeString cstrValue = getValue();
	cstrValue += pStringData_->getValue();

	setValue(cstrValue);
}

//
//	FUNCTION private
//	Common::CompressedStringData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::CompressedStringData クラスのクラスID
//
//	EXCEPTIONS
//	なし
//
int
CompressedStringData::
getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::CompressedStringDataClass;
}

//
//	FUNCTION private
//	Common::CompressedStringData::print_NotNull -- 値を表示する
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
CompressedStringData::
print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeString cstrTmp(getValue());
	cout << "string: " << cstrTmp.getString(LiteralCode) << endl;
}

//
//	FUNCTION private
//	Common::CompressedStringData::isApplicable_NotNull --
//		付加機能を適用可能かを得る
//
//	NOTES
//		圧縮ストリームに対応する
//
//	ARGUMENTS
//		Common::Data::Function::Value iFunction_
//			適用しようとしている付加機能
//
//	RETURN
//		true ... 適用できる
//		false... 適用できない
//
//	EXCEPTIONS

bool
CompressedStringData::
isApplicable_NotNull(Function::Value iFunction_)
{
	; _TRMEISTER_ASSERT(!isNull());

#ifdef OBSOLETE
	return (iFunction_ == Function::Stream);
#endif
	return false;
}

//
//	FUNCTION private
//	Common::CompressedStringData::apply_NotNull --
//		付加機能を適用したCommon::Dataを得る
//
//	NOTES
//		圧縮と圧縮ストリームに対応する
//
//	ARGUMENTS
//		Common::Data::Function::Value iFunction_
//			適用する付加機能
//
//	RETURN
//		付加機能を適用したCommon::Data
//
//	EXCEPTIONS
//		Exception::NotSupported
//			applyに対応していない

Data::Pointer
CompressedStringData::
apply_NotNull(Function::Value iFunction_)
{
	; _TRMEISTER_ASSERT(!isNull());

#ifdef OBSOLETE
	switch (iFunction_) {
	case Function::Stream:
	{
		return new CompressedStreamStringData(getCompressedValue(), getCompressedSize(), getValueSize());
	}
	default:
		break;
	}
#endif
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION protected
//	Common::CompressedStringData::reset --
//		圧縮データを作り直す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//		圧縮対象の文字列のサイズ(B 単位)
//
//	EXCEPTIONS

ModSize
CompressedStringData::reset()
{
	ModSize bufferSize = 0;

	if (m_pStringData.get()) {

		// 伸張したデータがあるときのみ処理する

		// 与えられたデータを圧縮して格納する

		const ModUnicodeString& cstrValue = m_pStringData->getValue();
		bufferSize = cstrValue.getLength() * sizeof(ModUnicodeChar);
		compress(static_cast<const ModUnicodeChar*>(cstrValue), bufferSize);
	}

	return bufferSize;
}

//
//	FUNCTION protected
//	Common::CompressedStringData::setValue --
//		内容を複写する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::CompressedStringData& cOther_
//		複写もとのデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
CompressedStringData::
setValue(const CompressedStringData& cOther_)
{
	CompressedData::setValue(cOther_);
	StringData::setValue(cOther_);
	m_pStringData = cOther_.m_pStringData;
}

//	FUNCTION private
//	Common::CompressedStringData::getDumpSize_NotNull --
//		ダンプサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS

//virtual
ModSize
CompressedStringData::
getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return getCompressedSize();
}

//virtual
ModSize
CompressedStringData::
getDumpSize_NotNull(EncodingForm::Value encodingForm) const
{
	// 指定された符号化形式は無視される

	return getDumpSize_NotNull();
}

//	FUNCTION public
//	Common::CompressedStringData::setDumpedValue --
//		dumpされたデータから自身の値をsetする
//
//	NOTES
//		コンストラクト時に、伸長した場合の長さを設定する必要がある
//
//	ARGUMENTS
//		const char* pszValue_
//			dumpされた領域の先頭
//		ModSize uSize_
//			サイズを指定する必要のあるデータの場合指定する
//		Common::StringData::EncodingForm::Value	encodingForm
//			ダンプされた文字列データの符号化方式
//
//	RETURN
//		値に使ったバイト数
//
//	EXCEPTIONS

ModSize
CompressedStringData::setDumpedValue(const char* src, ModSize n)
{
	setCompressedValue(src, n);

	const ModSize size = reset();

	// NULL 値でなくする

	setNull(false);

	return n;
}

ModSize
CompressedStringData::setDumpedValue(
	const char* src, ModSize n, EncodingForm::Value encodingForm)
{
	// 指定された符号化形式は無視される

	return setDumpedValue(src, n);
}

//	FUNCTION private
//	Common::CompressedStringData::dumpValue_NotNull --
//		自身の値をdumpする
//
//	NOTES
//
//	ARGUMENTS
//		char* pszResult_
//			dumpする領域の先頭
//
//	RETURN
//		値に使ったバイト数
//
//	EXCEPTIONS

//virtual
ModSize
CompressedStringData::
dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	if (getCompressedSize() > 0) {
		Os::Memory::copy(pszResult_, getCompressedValue(), getCompressedSize());
	}
	return getCompressedSize();
}

ModSize
CompressedStringData::dumpValue_NotNull(
	char* dst, EncodingForm::Value encodingForm) const
{
	// 指定された符号化形式は無視される

	return dumpValue_NotNull(dst);
}

//
//	Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
