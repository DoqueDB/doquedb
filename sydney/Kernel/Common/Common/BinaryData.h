// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BinaryData.h -- バイナリ型データ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2004, 2006, 2007, 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_BINARYDATA_H
#define __TRMEISTER_COMMON_BINARYDATA_H

#include "Common/Module.h"
#include "Common/ScalarData.h"

#include "ModUnicodeChar.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::BinaryData -- バイナリ型のデータを表すクラス
//
//	NOTES

class SYD_COMMON_FUNCTION BinaryData
	: public	ScalarData
{
	friend class CompressedBinaryData;
public:
	//コンストラクタ(1)
	BinaryData();
	//コンストラクタ(2)
	BinaryData(const void* pData_, unsigned int uiSize_,
			   bool bAllocate_ = true, unsigned int uiAllocatedSize_ = 0);
	//デストラクタ
	virtual ~BinaryData();
	//コピーコンストラクタ
	BinaryData(const BinaryData& cBinaryData_);
	//代入オペレータ
	BinaryData& operator =(const BinaryData& cBinaryData_);

	//メモリを特定値でアサインする
	using Data::assign;
	void assign(unsigned char c, unsigned int uiSize_);

	// シリアル化する
//	Common::ScalarData
//	virtual void
//	serialize(ModArchive& archiver);

	// コピーする
//	Common::ScalarData
//	virtual Data::Pointer
//	copy() const;
	// キャストする
//	Common::ScalarData	
//	virtual Pointer
//	cast(DataType::Type type) const;
//	virtual Pointer
//	cast(const Data& target) const;

	// 文字列の形式で値を得る
//	Common::ScalarData
//	virtual ModUnicodeString
//	getString() const;

	// 値を得る
	virtual void*
	getValue();
	virtual const void*
	getValue() const;
	// サイズを得る
	virtual unsigned int
	getSize() const;
	// 領域確保されたサイズを得る
	unsigned int
	getAllocatedSize() const;

	//データを設定する
	virtual void setValue(const void* pData_, unsigned int uiSize_);
	//外部でアロケートしたデータをそのまま使うかを指定できるバージョン
	void setValue(const void* pData_, unsigned int uiSize_, bool bAllocate_,
				  unsigned int uiAllocatedSize_ = 0);

	// 等しいか調べる
//	Common::Data
//	virtual bool
//	equals(const Data* r) const;
	// 大小比較を行う
//	Common::Data
//	virtual int
//	compareTo(const Data* r) const;

	// 代入を行う
//	Common::Data
//	virtual bool
//	assign(const Data* r);
	// 四則演算を行う
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op, const Data* r, Pointer& result) const;
	// 単項演算を行う
//	Common::ScalarData
//	virtual bool
//	operateWith(DataOperation::Type op, Pointer& result) const;
//	virtual bool
//	operateWith(DataOperation::Type op);

	// クラスIDを得る
//	Common::ScalarData
//	virtual int
//	getClassID() const;

	// 末尾にバイナリデータをつなげる
	virtual void
	connect(const BinaryData* pBinaryData_);

	// バイナリを直接指定するバージョン
	void connect(const void* pData_, unsigned int uiSize_);

	// 値を標準出力へ出力する
//	Common::ScalarData
//	virtual void
//	print() const;

	// 付加機能を適用可能か調べる
//	Common::ScalarData
//	virtual bool
//	isApplicable(Function::Value function);
	// 付加機能を適用する
//	Common::ScalarData
//	virtual Pointer
//	apply(Function::Value function);

	// ダンプ可能か調べる
//	Common::ScalarData
//	virtual bool
//	isAbleToDump() const;
	// 常に固定長であるかを得る
//	Common::ScalarData
//	virtual bool
//	isFixedSize() const;
	// ダンプサイズを得る
//	Common::ScalarData
//	virtual ModSize
//	getDumpSize() const;
	// 自分自身にダンプされた値を設定する
//	Common::Data
//	virtual ModSize
//	setDumpedValue(const char* pszValue_);
	virtual ModSize
	setDumpedValue(const char* pszValue_, ModSize uSize_);

//	virtual ModSize
//	setDumpedValue(ModSerialIO& cSerialIO_);
	virtual ModSize 
	setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_);

//	virtual ModSize 
//	dumpValue(ModSerialIO& cSerialIO_) const;

	// 値をダンプする
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(char* p) const;

	// テキスト符号化された文字列を復号化し、バイナリにする
	void
	decodeString(const ModUnicodeChar* src, ModSize n, int base = 16);

	// バイナリを、テキスト符号化された文字列に変換する
	ModUnicodeString encodeString(int base = 16) const;

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;

protected:
	// 内容を複写する
	void		setValue(const BinaryData& cOther_);

private:
	// シリアル化する(自分自身が NULL 値でない)
	virtual void
	serialize_NotNull(ModArchive& archiver);

	// コピーする(自分自身が NULL 値でない)
	virtual Pointer
	copy_NotNull() const;
	// キャストする(自分自身が NULL 値でない)
//	Common::ScalarData
//	virtual Pointer
//	cast_NotNull(DataType::Type type) const;
//	virtual Pointer
//	cast_NotNull(const Data& target) const;

	// 文字列の形式で値を得る(自分自身が NULL 値でない)
	virtual ModUnicodeString
	getString_NotNull() const;

	// 値を得る(自分自身が NULL 値でない)
	virtual void*
	getValue_NotNull();
	virtual const void*
	getValue_NotNull() const;
	// サイズを得る(自分自身が NULL 値でない)
	virtual unsigned int
	getSize_NotNull() const;

	// 等しいか調べる(キャストなし)
	virtual bool
	equals_NoCast(const Data& r) const
	{
		return !compareTo_NoCast(r);
	}
	// 大小比較を行う(キャストなし)
	virtual int
	compareTo_NoCast(const Data& r) const;

	// 代入を行う(キャストなし)
	virtual bool
	assign_NoCast(const Data& r);
	// 単項演算を行う(自分自身が NULL 値でない)
//	Common::ScalarData
//	virtual bool
//	operateWith_NotNull(DataOperation::Type op, Pointer& result) const;
//	virtual bool
//	operateWith_NotNull(DataOperation::Type op);
//	// 四則演算を行う(キャストなし)
//	virtual bool
//	operateWith_NoCast(DataOperation::Type op, const Data& r);

	// 付加機能を適用可能か調べる(自分自身が NULL 値でない)
	virtual bool
	isApplicable_NotNull(Function::Value function);
	// 付加機能を適用する(自分自身が NULL 値でない)
	virtual Pointer
	apply_NotNull(Function::Value function);

	// ダンプ可能か調べる(自分自身が NULL 値でない)
	virtual bool
	isAbleToDump_NotNull() const;
	// 常に固定長であるかを得る(自分自身が NULL 値でない)
//	Common::ScalarData::
//	virtual bool
//	isFixedSize_NotNull() const;
	// ダンプサイズを得る(自分自身が NULL 値でない)
	virtual ModSize
	getDumpSize_NotNull() const;
	// 値をダンプする(自分自身が NULL 値でない)
	virtual	ModSize
	dumpValue_NotNull(char* p) const;
	
	virtual	ModSize
	dumpValue_NotNull(ModSerialIO& cSerialIO_) const;

	// クラスIDを得る(自分自身が NULL 値でない)
	virtual int
	getClassID_NotNull() const;

	// 末尾にバイナリデータをつなげる
	virtual void
	connect_NotNull(const BinaryData* pBinaryData_);

	// 値を標準出力へ出力する(自分自身が NULL 値でない)
	virtual void
	print_NotNull() const;

	// 実際の値を用いてSQLDataを得る(getSQLTypeの下請け)
	virtual bool getSQLTypeByValue(SQLData& cType_);

	//データ
	void* m_pValue;
	//サイズ
	unsigned int m_uiSize;
	unsigned int m_uiAllocatedSize;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_BINARYDATA_H

//
//	Copyright (c) 2000, 2001, 2004, 2006, 2007, 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
