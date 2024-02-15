// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedBinaryData.cpp -- 圧縮されたバイナリ型データ関連の関数定義
// 
// Copyright (c) 2001, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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
#include "Common/ClassID.h"
#include "Common/CompressedBinaryData.h"
#ifdef OBSOLETE
#include "Common/CompressedStreamBinaryData.h"
#endif
#include "Common/NullData.h"
#include "Common/UnicodeString.h"

#include "Exception/ClassCast.h"
#include "Exception/NotSupported.h"
#include "Os/Memory.h"

#include "ModCompress.h"
#include "ModDefaultManager.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//	FUNCTION private
//	Common::CompressedBinaryData::serialize_NotNull -- シリアル化
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
CompressedBinaryData::
serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	// 親クラスはBinaryDataだが、
	// BinaryDataの中身は使っていないので
	// さらに親のDataでシリアライズする

	ScalarData::serialize_NotNull(cArchiver_);

	// CompressedDataのシリアライズも呼ぶ
	CompressedData::serialize(cArchiver_);
}

//	FUNCTION private
//	Common::CompressedBinaryData::copy_NotNull -- コピーする
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
CompressedBinaryData::
copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new CompressedBinaryData(*this);
}

//	FUNCTION private
//	Common::CompressedBinaryData::getValue_NotNull -- 値を得る
//
//	NOTES
//	値を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	void*
//		バイナリデータの先頭へのポインタ
//
//	EXCEPTIONS

void*
CompressedBinaryData::
getValue_NotNull()
{
	; _TRMEISTER_ASSERT(!isNull());

	// 必要なら伸張したデータを作る
	(void*)static_cast<const CompressedBinaryData*>(this)->getValue();

	// constでないポインターを返すときは
	// 呼び出し側で編集する可能性があるので
	// 圧縮データを格納する部分をクリアしておく
	// →圧縮データが必要になったら再び圧縮される
	//
	// ★注意★
	// 圧縮できなかった場合はm_pBinaryDataは
	// getCompressedDataの返り値と同じものを指しているので
	// 消去する必要はない
	// ゆえにisCompressed()のときのみ処理すればよい
	if (isCompressed()) {
		clear();
	}
	; _TRMEISTER_ASSERT(m_pBinaryData.get());
	return m_pBinaryData->getValue();
}

//	FUNCTION private
//	Common::CompressedBinaryData::getValue_NotNull -- 値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		バイナリデータを格納する領域の先頭アドレス
//
//	EXCEPTIONS

const void*
CompressedBinaryData::getValue_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	if (!m_pBinaryData.get()) {

		// 伸張後のデータが生成されていない

		// 伸張されたときのサイズを得る

		const ModSize uncompressedSize = getValueSize();

		if (!uncompressedSize)

			// 伸張後のデータの大きさは 0 なので、
			// 空のバイナリデータを生成しておく

			m_pBinaryData = new BinaryData();

		else if (isCompressed()) {

			// 格納しているデータは圧縮されているので、まず、伸張する

			void* buf =	ModDefaultManager::allocate(uncompressedSize);
			; _TRMEISTER_ASSERT(buf);

			try {
				uncompress(buf);

				// 伸張したデータをバイナリデータにする

				m_pBinaryData = new BinaryData(
					buf, uncompressedSize, false, uncompressedSize);

			} catch (...) {

				ModDefaultManager::free(buf, uncompressedSize);
				_TRMEISTER_RETHROW;
			}
		} else {

			// 格納しているデータは圧縮されていないので、
			// そのままオブジェクトポインタをコピーする

			; _TRMEISTER_ASSERT(getCompressedData().get());
			m_pBinaryData = getCompressedData();
		}
	}

	return m_pBinaryData->getValue();
}

//	FUNCTION private
//	Common::CompressedBinaryData::getSize_NotNull --
//		格納されているデータの伸張したときのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたサイズ(B 単位)
//
//	EXCEPTIONS

unsigned int
CompressedBinaryData::getSize_NotNull() const
{
	return getValueSize();
}

//	FUNCTION public
//	Common::CompressedBinaryData::setValue -- 与えられた値を格納する
//
//	NOTES
//
//	ARGUMENTS
//		void*				buf
//			格納する値が格納された領域の先頭アドレス
//		unsigned int		size
//			格納する値が格納された領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
CompressedBinaryData::setValue(const void* buf, unsigned int size)
{
	// まず、(伸張された)データを格納していれば、クリアする

	m_pBinaryData = static_cast<BinaryData*>(0);

	// 与えられたデータを圧縮して、格納する

	compress(buf, size);

	// NULL 値でなくする

	setNull(false);
}

//
//	FUNCTION private
//	Common::CompressedBinaryData::getString_NotNull -- 文字列データを得る
//
//	NOTES
//	文字列でデータを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeBinary
//		文字列にしたデータ
//
//	EXCEPTIONS
//	なし
//
ModUnicodeString
CompressedBinaryData::
getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeOstrStream ostr;
	ostr << _TRMEISTER_U_STRING("size=") << getValueSize();
	return ModUnicodeString(ostr.getString());
}

//	FUNCTION private
//	Common::CompressedBinaryData::equals_NoCast -- 等しいか調べる
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
CompressedBinaryData::equals_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->equals(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::Binary);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	if (!(r.getFunction() & Function::Compressed))

		// 与えられたデータは圧縮されていない文字列型データなので、
		// 自分自身は解凍しないと等しいかわからない

		return !compareTo_NoCast(r);

	// 与えられたデータは圧縮されているので、
	// 解凍しないでそのまま等しいか調べる

	const CompressedBinaryData& data =
		_SYDNEY_DYNAMIC_CAST(const CompressedBinaryData&, r);

	return getCompressedSize() == data.getCompressedSize() &&
		!(getCompressedSize() &&
		  Os::Memory::compare(getCompressedValue(),
							  data.getCompressedValue(), getCompressedSize()));
}

//	FUNCTION private
//	Common::CompressedBinaryData::compareTo_NoCast -- 大小比較を行う
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
CompressedBinaryData::compareTo_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->equals(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::Binary);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	// 自分自身が伸張されていなければ、伸張しておく

	(void) getValue();
	; _TRMEISTER_ASSERT(m_pBinaryData.get());

	const BinaryData& data = _SYDNEY_DYNAMIC_CAST(const BinaryData&, r);

	return m_pBinaryData->compareTo_NoCast(data);
}

//
//	FUNCTION private
//	Common::CompressedBinaryData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::CompressedBinaryData クラスのクラスID
//
//	EXCEPTIONS
//	なし
//
int
CompressedBinaryData::
getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::CompressedBinaryDataClass;
}

//
//	FUNCTION private
//	Common::CompressedBinaryData::print_NotNull -- 値を表示する
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
CompressedBinaryData::
print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	cout << "binary: " << "size=" << getValueSize() << endl;
}

//
//	FUNCTION private
//	Common::CompressedBinaryData::isApplicable_NotNull --
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
CompressedBinaryData::
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
//	Common::CompressedBinaryData::apply_NotNull --
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
CompressedBinaryData::
apply_NotNull(Function::Value iFunction_)
{
	; _TRMEISTER_ASSERT(!isNull());

#ifdef OBSOLETE
	switch (iFunction_) {
	case Function::Stream:
	{
		return new CompressedStreamBinaryData(getCompressedValue(), getCompressedSize(), getValueSize());
	}
	default:
		break;
	}
#endif
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION protected
//	Common::CompressedBinaryData::reset --
//		圧縮データを作り直す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//		圧縮対象のバイナリデータのサイズ(B 単位)
//
//	EXCEPTIONS

ModSize
CompressedBinaryData::reset()
{
	ModSize bufferSize = 0;

	if (m_pBinaryData.get()) {

		// 伸張したデータがあるときのみ処理する

		// 圧縮して格納する

		bufferSize = m_pBinaryData->getSize();
		compress(static_cast<const char*>(
					 m_pBinaryData->getValue()), bufferSize);
	}

	return bufferSize;
}

//
//	FUNCTION protected
//	Common::CompressedBinaryData::setValue --
//		内容を複写する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::CompressedBinaryData& cOther_
//		複写もとのデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
CompressedBinaryData::
setValue(const CompressedBinaryData& cOther_)
{
	CompressedData::setValue(cOther_);
	BinaryData::setValue(cOther_);
	m_pBinaryData = cOther_.m_pBinaryData;
}

//	FUNCTION private
//	Common::CompressedBinaryData::getDumpSize_NotNull --
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
CompressedBinaryData::
getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return getCompressedSize();
}

//	FUNCTION public
//	Common::CompressedBinaryData::setDumpedValue --
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
//
//	RETURN
//		値に使ったバイト数
//
//	EXCEPTIONS

//virtual
ModSize
CompressedBinaryData::
setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	setCompressedValue(pszValue_, uSize_);

	// NULL 値でなくする

	setNull(false);

	return uSize_;
}

//virtual
ModSize
CompressedBinaryData::setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	char* pszBuf = new char[uSize_];
	cSerialIO_.readSerial(pszBuf, uSize_, ModSerialIO::dataTypeVariable);
	setCompressedValue(pszBuf, uSize_);
	delete []pszBuf;

	setNull(false);
	return uSize_;
}

//virtual
ModSize
CompressedBinaryData::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	if (getCompressedSize() > 0) {
		cSerialIO_.writeSerial(getCompressedValue(), getCompressedSize(), ModSerialIO::dataTypeVariable);
	}
	return getCompressedSize();
}

//	FUNCTION private
//	Common::CompressedBinaryData::dumpValue_NotNull --
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
CompressedBinaryData::
dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	if (getCompressedSize() > 0) {
		Os::Memory::copy(pszResult_, getCompressedValue(), getCompressedSize());
	}
	return getCompressedSize();
}

//
//	Copyright (c) 2001, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
