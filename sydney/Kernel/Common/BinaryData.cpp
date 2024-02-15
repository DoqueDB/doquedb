// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BinaryData.cpp -- バイナリ型データ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "Common/CompressedData.h"
#ifdef OBSOLETE
#include "Common/CompressedStreamBinaryData.h"
#endif
#include "Common/NullData.h"
#include "Common/SQLData.h"
#include "Common/UnicodeString.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/NullNotAllowed.h"
#include "Os/Memory.h"

#include "ModHasher.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"

#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
	namespace _BinaryData
	{
		unsigned char
		decodeBase16Char(ModUnicodeChar c)
		{
			if (!ModUnicodeCharTrait::isXdigit(c))
				_TRMEISTER_THROW0(Exception::BadArgument);
			return ModUnicodeCharTrait::isDigit(c) ?
				(c - UnicodeChar::usZero) :
				(ModUnicodeCharTrait::toUpper(c) - UnicodeChar::usLargeA + 10);
		}

		ModUnicodeChar
		encodeBase16Char(unsigned char v)
		{
			if (v > 0x0f)
				_TRMEISTER_THROW0(Exception::BadArgument);
			return static_cast<ModUnicodeChar>(
				(v < 10) ? (UnicodeChar::usZero + v) :
				(UnicodeChar::usLargeA + (v - 10)));
		}
	}
}

//	FUNCTION public
//	Common::BinaryData::BinaryData -- デフォルトコンストラクタ
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

BinaryData::BinaryData()
	: ScalarData(DataType::Binary),
	  m_pValue(0),
	  m_uiSize(0),
	  m_uiAllocatedSize(0)
{}

//
//	FUNCTION public
//	Common::BinaryData::BinaryData -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const void* pData_
//		バイナリデータ
//	unsigned int uiSize_
//		サイズ
//	bool bAllocate_ = true
//		trueのとき内部でコピーする
//		falseのとき引数でもらったアドレスをそのまま流用する
//	unsigned int uiAllocatedSize_ = 0
//		bAllocate_がfalseのとき、もらったアドレスのallocateしたときのサイズを渡す
//
//	RETURN
//	なし
//
//	EXCEPTIONS

BinaryData::BinaryData(const void* pData_, unsigned int uiSize_,
					   bool bAllocate_, unsigned int uiAllocatedSize_)
	: ScalarData(DataType::Binary),
	  m_pValue(0),
	  m_uiSize(0),
	  m_uiAllocatedSize(0)
{
	setValue(pData_, uiSize_, bAllocate_, uiAllocatedSize_);
}

//
//	FUNCTION public
//	Common::BinaryData::~BinaryData -- デストラクタ
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
BinaryData::
~BinaryData()
{
	if (m_uiAllocatedSize) {
		ModDefaultManager::free(m_pValue, m_uiAllocatedSize);
	}
}

//	FUNCTION public
//	Common::BinaryData::BinaryData -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Common::BinaryData& cBinaryData_
//			コピーするバイナリデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

BinaryData::BinaryData(const BinaryData& cBinaryData_)
	: ScalarData(cBinaryData_),
	  m_pValue(0),
	  m_uiSize(0),
	  m_uiAllocatedSize(0)
{
	setValue(cBinaryData_);
}

//	FUNCTION public
//	Common::BinaryData::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Common::BinaryData& cBinaryData_
//			自分自身に設定するバイナリデータ
//
//	RETURN
//		自分自身のリファレンス
//
//	EXCEPTIONS

BinaryData&
BinaryData::operator =(const BinaryData& cBinaryData_)
{
	if (&cBinaryData_ != this)
		setValue(cBinaryData_);

	return *this;
}

//
//	FUNCTION public
//	Common::BinaryData::assign -- メモリを特定値でアサインする
//
//	NOTES
//
//	ARGUMENTS
//	char c
//		格納するデータ
//	unsigned int uiSize_
//		確保するサイズ(バイト)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BinaryData::assign(unsigned char c, unsigned int uiSize_)
{
	if (m_uiAllocatedSize != 0 && m_uiAllocatedSize < uiSize_)
	{
		ModDefaultManager::free(m_pValue, m_uiAllocatedSize);
		m_uiAllocatedSize = 0;
	}
	if (m_uiAllocatedSize == 0)
	{
		m_uiAllocatedSize = uiSize_;
		m_pValue = ModDefaultManager::allocate(m_uiAllocatedSize);
	}
	m_uiSize = uiSize_;
	ModOsDriver::Memory::set(m_pValue, c, m_uiSize);
}

//	FUNCTION private
//	Common::BinaryData::serialize_NotNull -- シリアル化する
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive& cArchiver_
//			使用するアーカイバ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
BinaryData::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore()) {
		// 書出し
		// 読む方とのバランスを取る
		cArchiver_ << m_uiSize;
		if (m_uiSize > 0)
			cArchiver_.writeArchive(m_pValue, m_uiSize);
	} else {
		if (m_uiAllocatedSize) {
			// すでに読み込まれているものを解放する
			ModDefaultManager::free(m_pValue, m_uiAllocatedSize);
			m_uiAllocatedSize = 0;
			m_pValue = 0;
		}
		// これを読まないとサイズが判断できない
		cArchiver_ >> m_uiSize;
		m_uiAllocatedSize = m_uiSize;
		// 読出し
		if (m_uiSize > 0) {
			m_pValue = ModDefaultManager::allocate(m_uiSize);
			cArchiver_.readArchive(m_pValue, m_uiSize);
		}
	}
}

//	FUNCTION private
//	Common::BinaryData::copy_NotNull -- コピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自分自身のコピー
//
//	EXCEPTIONS

Data::Pointer
BinaryData::copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new BinaryData(*this);
}

//	FUNCTION private
//	Common::BinaryData::getString_NotNull -- 文字列の形式で値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた文字列
//
//	EXCEPTIONS

ModUnicodeString
BinaryData::getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	// バイナリなのでサイズのみを表示する

	ModUnicodeOstrStream ostr;
	ostr << _TRMEISTER_U_STRING("size=") << m_uiSize;
	return ModUnicodeString(ostr.getString());
}

//	FUNCTION public
//	Common::BinaryData::getValue -- 値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた値
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

void*
BinaryData::getValue()
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return getValue_NotNull();
}

void*
BinaryData::getValue_NotNull()
{
	return m_pValue;
}

const void*
BinaryData::getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return getValue_NotNull();
}

const void*
BinaryData::getValue_NotNull() const
{
	return m_pValue;
}

//	FUNCTION public
//	Common::BinaryData::getSize -- サイズを得る
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
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

unsigned int
BinaryData::getSize() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return getSize_NotNull();
}

unsigned int
BinaryData::getSize_NotNull() const
{
	return m_uiSize;
}

//	FUNCTION public
//	Common::BinaryData::getAllocatedSize -- 領域確保されたサイズを得る
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
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

unsigned int
BinaryData::getAllocatedSize() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_uiAllocatedSize;
}

//
//	FUNCTION public
//	Common::BinaryData::setValue -- データを設定する
//
//	NOTES
//	データを設定する。
//
//	ARGUMENTS
//	const void* pData_
//		バイナリデータ
//	unsigned int uiSize_
//		サイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
BinaryData::
setValue(const void* pData_, unsigned int uiSize_)
{
	// 自身でallocateした領域にコピーする
	setValue(pData_, uiSize_, true);
}

//
//	FUNCTION public
//	Common::BinaryData::setValue -- データを設定する
//
//	NOTES
//	データを設定する。
//
//	ARGUMENTS
//	const void* pData_
//		バイナリデータ
//	unsigned int uiSize_
//		サイズ
//	bool bAllocate_ = true
//		trueのとき内部でコピーする
//		falseのとき引数でもらったアドレスをそのまま流用する
//		このときpData_はModDefaultManagerでallocateしたもので
//		なければならない
//	unsigned int uiAllocatedSize_ = 0
//		bAllocate_がfalseのとき、もらったアドレスのallocateしたときのサイズを渡す
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
BinaryData::
setValue(const void* pData_, unsigned int uiSize_,
		 bool bAllocate_, unsigned int uiAllocatedSize_)
{
	if (m_uiSize)
	{
		if (m_uiAllocatedSize) {
			ModDefaultManager::free(m_pValue, m_uiAllocatedSize);
		}
		m_pValue = 0;
		m_uiSize = 0;
		m_uiAllocatedSize = 0;
	}
	void* pValue = 0;
	unsigned int uiAllocatedSize = 0;
	if (uiSize_ > 0)
	{
		if (bAllocate_)
		{
			pValue = ModDefaultManager::allocate(uiSize_);
			uiAllocatedSize = uiSize_;
			Os::Memory::copy(pValue, pData_, uiSize_);
		} else {
			pValue = const_cast<void*>(pData_);
			uiAllocatedSize = uiAllocatedSize_;
		}
	}
	m_pValue = pValue;
	m_uiSize = uiSize_;
	m_uiAllocatedSize = uiAllocatedSize;

	// NULL 値でなくする

	setNull(false);
}

//	FUNCTION private
//	Common::BinaryData::compareTo_NoCast -- 大小比較を行う
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
BinaryData::compareTo_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->compareTo(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::Binary);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const BinaryData& data = _SYDNEY_DYNAMIC_CAST(const BinaryData&, r);

	if (getSize() == data.getSize()) {
		if (getSize()) {
			int i = Os::Memory::compare(getValue(), data.getValue(), getSize());
			return (i > 0) ? 1 : (i < 0) ? -1 : 0;
		} else
			return 0;
	}
	// サイズが異なるときは、小さなほうの部分を比較して、
	// その部分が等しければ、サイズの大きいほうが大きいとみなす

	return (getSize() < data.getSize()) ?
		((getSize() &&
		  Os::Memory::compare(
			  getValue(), data.getValue(), getSize()) > 0) ? 1 : -1) :
		((data.getSize() &&
		  Os::Memory::compare(
			  getValue(), data.getValue(), data.getSize()) < 0) ? -1 : 1);
}

//	FUNCTION private
//	Common::BinaryData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//		シリアライズのためのクラスIDを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたクラスID
//
//	EXCEPTIONS
//		なし

int
BinaryData::getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::BinaryDataClass;
}

//	FUNCTION public
//	Common::BinaryData::connect --
//		自分自身の末尾に与えられたバイナリデータをつなげる
//
//	NOTES
//
//	ARGUMENTS
//		Common::BinaryData*	pBinaryData_
//			自分自身の末尾につなげる
//			バイナリデータが格納された領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			NULL 値が与えられた
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

void
BinaryData::connect(const BinaryData* pBinaryData_)
{
	; _TRMEISTER_ASSERT(pBinaryData_);

	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);
	if (pBinaryData_->isNull())
		_TRMEISTER_THROW0(Exception::BadArgument);

	connect_NotNull(pBinaryData_);
}

void
BinaryData::connect_NotNull(const BinaryData* pBinaryData_)
{
	; _TRMEISTER_ASSERT(pBinaryData_);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!pBinaryData_->isNull());

	if (unsigned int uiSize = pBinaryData_->getSize()) {

		const unsigned int uiNewSize = m_uiSize + uiSize;
		void* pValue = ModDefaultManager::allocate(uiNewSize);
		; _TRMEISTER_ASSERT(pValue);

		try {
			Os::Memory::copy(pValue, m_pValue, m_uiSize);
			Os::Memory::copy(syd_reinterpret_cast<char*>(pValue) + m_uiSize, pBinaryData_->getValue(), uiSize);
			setValue(pValue, uiNewSize, false, uiNewSize);
		} catch (...) {
			ModDefaultManager::free(pValue, uiNewSize);
			_TRMEISTER_RETHROW;
		}
	}
}

// FUNCTION public
//	Common::BinaryData::connect --
//			自分自身の末尾に与えられたバイナリデータをつなげる
//			(バイナリを直接指定するバージョン)
//
// NOTES
//
// ARGUMENTS
//	const void* pData_
//	unsigned int uiSize_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BinaryData::
connect(const void* pData_, unsigned int uiSize_)
{
	; _TRMEISTER_ASSERT(pData_);
	; _TRMEISTER_ASSERT(!isNull());

	if (uiSize_) {

		const unsigned int uiNewSize = m_uiSize + uiSize_;
		void* pValue = ModDefaultManager::allocate(uiNewSize);
		; _TRMEISTER_ASSERT(pValue);

		try {
			Os::Memory::copy(pValue, m_pValue, m_uiSize);
			Os::Memory::copy(syd_reinterpret_cast<char*>(pValue) + m_uiSize, pData_, uiSize_);
			setValue(pValue, uiNewSize, false, uiNewSize);
		} catch (...) {
			ModDefaultManager::free(pValue, uiNewSize);
			_TRMEISTER_RETHROW;
		}
	}
}

//	FUNCTION private
//	Common::BinaryData::print_NotNull -- 値を表示する
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

void
BinaryData::print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	cout << "binary: " << "size=" << m_uiSize << endl;
}

//
//	FUNCTION protected
//	Common::BinaryData::setValue --
//		与えられたバイナリデータを自分自身に設定する
//
//	NOTES
//
//	ARGUMENTS
//		Common::BinaryData&		cOther_
//			自分自身に設定するバイナリデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
BinaryData::setValue(const BinaryData& cOther_)
{
	if (cOther_.isNull())
		setNull();
	else {
		setFunction(cOther_.getFunction());
		setValue(cOther_.m_pValue, cOther_.m_uiSize, true);
	}
}

// FUNCTION public
//	Common::BinaryData::assign_NoCast -- 代入を行う(キャストなし)
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
BinaryData::
assign_NoCast(const Data& r)
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (r.isNull()) {
		setNull();
		return false;
	}
#endif

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const BinaryData& data = _SYDNEY_DYNAMIC_CAST(const BinaryData&, r);
	const void* pRhs = data.getValue();
	unsigned int uiSize = data.getSize();

	setValue(pRhs, uiSize);
	return true;
}

//	FUNCTION private
//	Common::BinaryData::isApplicable_NotNull -- 付加機能を適用可能か調べる
//
//	NOTES
//		圧縮に対応する
//
//	ARGUMENTS
//		Common::Data::Function::Value	function
//			調べる付加機能
//
//	RETURN
//		true
//			適用できる
//		false
//			適用できない
//
//	EXCEPTIONS

bool
BinaryData::isApplicable_NotNull(Function::Value function)
{
	; _TRMEISTER_ASSERT(!isNull());

	return (function & Function::Compressed) &&
		!(function & ~Function::Compressed) &&
		CompressedData::isCompressable(getValue(), getSize());
#ifdef OBSOLETE
	return (function & Function::Compressed) &&
		!(function & ~(Function::Compressed | Function::Stream)) &&
		CompressedData::isCompressable(getValue(), getSize());
#endif
}

//	FUNCTION private
//	Common::BinaryData::apply_NotNull -- 付加機能を適用したバイナリデータを得る
//
//	NOTES
//		圧縮に対応する
//
//	ARGUMENTS
//		Common::Data::Function::Value	function
//			適用する付加機能
//
//	RETURN
//		付加機能を適用した Common::Data
//
//	EXCEPTIONS
//		Exception::NotSupported
//			バイナリデータに適用できない付加機能が指定された

Data::Pointer
BinaryData::apply_NotNull(Function::Value function)
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (function) {
	case Function::Compressed:
		return new CompressedBinaryData(getValue(), getSize());
#ifdef OBSOLETE
	case Function::Compressed | Function::Stream:
		return new CompressedStreamBinaryData(getValue(), getSize());
#endif
	}

	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION private
//	Common::BinaryData::isAbleToDump_NotNull -- ダンプできるか調べる
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

//virtual
bool
BinaryData::isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::BinaryData::getDumpSize_NotNull -- ダンプサイズを得る
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

//virtual
ModSize
BinaryData::getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return m_uiSize;
}

//	FUNCTION public
//	Common::BinaryData::setDumpedValue --
//		値がダンプされた領域から値を読み出し、自分自身に設定する
//
//	NOTES
//
//	ARGUMENTS
//		char*		src
//			値がダンプされた領域の先頭アドレス
//		ModSize		size
//			ダンプされた値のサイズ(B 単位)
//
//	RETURN
//		ダンプされた値のサイズ(B 単位)
//
//	EXCEPTIONS

//virtual
ModSize
BinaryData::setDumpedValue(const char* src, ModSize size)
{
	setValue(src, size);
	return size;
}

ModSize 
BinaryData::setDumpedValue(ModSerialIO& cSerialIO_, ModSize uiSize_)
{
	m_pValue = 0;
	m_uiSize = m_uiAllocatedSize = uiSize_;
	
	if (uiSize_)
	{
		m_pValue = ModDefaultManager::allocate(uiSize_);
		cSerialIO_.readSerial(m_pValue, uiSize_, ModSerialIO::dataTypeVariable);
	}

	// NULL 値でなくする
	setNull(false);
	
	return uiSize_;
}

ModSize
BinaryData::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	if (m_uiSize > 0)
		cSerialIO_.writeSerial(m_pValue, m_uiSize, ModSerialIO::dataTypeVariable);

	return m_uiSize;
}

//	FUNCTION private
//	Common::BinaryData::dumpValue_NotNull --
//		自分の値を与えられた領域にダンプする
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
//	EXCEPTIONS

//virtual
ModSize
BinaryData::dumpValue_NotNull(char* dst) const
{
	; _TRMEISTER_ASSERT(!isNull());

	if (m_uiSize > 0)
		(void) Os::Memory::copy(dst, m_pValue, m_uiSize);

	return m_uiSize;
}

// 実際の値を用いてSQLDataを得る(getSQLTypeの下請け)
//virtual
bool
BinaryData::
getSQLTypeByValue(SQLData& cType_)
{
	if (!isNull()) {
		// データサイズをLengthにセットする
		cType_.setLength(m_uiSize);
	}
	return true;
}

//	FUNCTION public
//	Common::BinaryData::decodeString --
//		テキスト符号化された文字列を復号化し、バイナリにする
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeChar*		src
//			復号化するテキスト符号化された文字列が格納された領域の先頭アドレス
//		ModSize				n
//			復号化するテキスト符号化された文字列の文字数
//		int					base
//			基数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
BinaryData::decodeString(const ModUnicodeChar* src, ModSize n, int base)
{
	; _TRMEISTER_ASSERT(src);

	if (base != 16)

		// 現状は BASE16 しかサポートしていない

		_TRMEISTER_THROW0(Exception::NotSupported);

	// 必要な領域のサイズを計算する

	const ModSize size = (n >> 1) + (n % 2);

	if (m_uiAllocatedSize < size) {

		// 現在の値を格納する領域では新しい値を格納するには足りないので、
		// 足りる領域を確保しなおす

		if (m_uiAllocatedSize) {
			ModDefaultManager::free(m_pValue, m_uiAllocatedSize);
			m_pValue = 0;
			m_uiAllocatedSize = 0;
		}

		m_pValue = ModDefaultManager::allocate(size);
		; _TRMEISTER_ASSERT(m_pValue);
		m_uiAllocatedSize = size;
	}

	m_uiSize = 0;

	unsigned char* p = static_cast<unsigned char*>(m_pValue);
	unsigned char* q = p + size;
	ModSize i = 0;

	for (; p < q; ++p) {
		; _TRMEISTER_ASSERT(i < n);
		*p = _BinaryData::decodeBase16Char(src[i]);
		*p <<= 4;

		if (++i < n)
			*p |= _BinaryData::decodeBase16Char(src[i]);
	}

	m_uiSize = size;
}

//
//	FUNCTION public
//	Common::BinaryData::encodeString --
//		バイナリを、テキスト符号化された文字列に変換する
//
//	NOTES
//
//	ARGUMENTS
//		int					base_
//			基数
//
//	RETURN
//		ModUnicodeString
//			テキスト符号化された文字列
//
//	EXCEPTIONS
//
ModUnicodeString
BinaryData::encodeString(int base_) const
{
	if (base_ != 16)

		// 現状は BASE16 しかサポートしていない

		_TRMEISTER_THROW0(Exception::NotSupported);

	const unsigned char* p = static_cast<const unsigned char*>(m_pValue);
	const unsigned char* q = p + m_uiSize;

	ModUnicodeOstrStream s;
	for (; p < q; ++p)
	{
		s << _BinaryData::encodeBase16Char(*p >> 4);
		s << _BinaryData::encodeBase16Char(*p & 0x0f);
	}

	return ModUnicodeString(s.getString());
}

// FUNCTION public
//	Common::BinaryData::hashCode -- ハッシュコードを取り出す
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
BinaryData::
hashCode() const
{
	if (isNull()) return 0;

	ModSize hashValue = m_uiSize;
	ModSize g;
	if (m_uiSize) {
		for (ModSize offset = 0; offset < m_uiSize; offset += m_uiSize/3 + 1) {
			hashValue <<= 4;
			hashValue += static_cast<ModSize>(*(reinterpret_cast<unsigned char*>(m_pValue) + offset));
			if (g = hashValue & ((ModSize) 0xf << (ModSizeBits - 4))) {
				hashValue ^= g >> (ModSizeBits - 8);
				hashValue ^= g;
			}
		}
	}
	return hashValue;
}

//
//	Copyright (c) 2000, 2001, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
