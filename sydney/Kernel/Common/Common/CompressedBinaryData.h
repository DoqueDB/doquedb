// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedBinaryData -- 圧縮されたバイナリ型データ関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2004, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_COMPRESSEDBINARYDATA_H
#define __TRMEISTER_COMMON_COMPRESSEDBINARYDATA_H

#include "Common/Module.h"
#include "Common/BinaryData.h"
#include "Common/CompressedData.h"
#include "Common/ObjectPointer.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::CompressedBinaryData -- 圧縮されたバイナリ型のデータを表すクラス
//
//	NOTES

class SYD_COMMON_FUNCTION CompressedBinaryData
	: public			BinaryData,				// 先である必要がある
	  virtual public	CompressedData
{
public:
	//コンストラクタ(1)
	CompressedBinaryData();
	//コンストラクタ(2)
	CompressedBinaryData(ModSize iValueSize_);
	//コンストラクタ(3)
	CompressedBinaryData(const void* pCompressedData_, ModSize iCompressedSize_, ModSize iValueSize_);
	//コンストラクタ(4)
	CompressedBinaryData(const void* pUncompressedData_, ModSize iValueSize_);
	//コピーコンストラクタ
	explicit CompressedBinaryData(const CompressedBinaryData& cstrOther_);
	//デストラクタ
	virtual ~CompressedBinaryData();

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
//	Common::BinaryData
//	virtual void*
//	getValue();
//	virtual const void*
//	getValue() const;
	// サイズを得る
//	Common::BinaryData
//	virtual unsigned int
//	getSize() const;

	//データを設定する
	virtual void setValue(const void* pData_, unsigned int uiSize_);

	// 等しいか調べる
//	Common::Data
//	virtual bool
//	equals(const Data* r) const;
	// 大小比較を行う
//	Common::Data
//	virtual int
//	compareTo(const Data* r) const;

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
//	Common::BinaryData
//	virtual void
//	connect(const BinaryData* pBinaryData_);

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

//	Common::ScalarData
//	virtual ModSize 
//	dumpValue(ModSerialIO& cSerialIO_) const;

	// 値をダンプする
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(char* p) const;

//CompressedData::
//	// 値を設定する
//	void		setCompressedValue(const void* pCompressedData_, ModSize iCompressedSize_);
//	void		setCompressedValue(const void* pCompressedData_, ModSize iCompressedSize_, ModSize iValueSize_);
//
//	// 伸張されたときのサイズを得る
//	ModSize getValueSize() const;
//
//	// 圧縮されたデータを得る
//	const void* getCompressedValue() const;
//	// 圧縮されたサイズを得る
//	ModSize getCompressedSize() const;
//
//	// 圧縮されているかを得る
//	bool		isCompressed() const;

protected:
//CompressedData::
//	// 圧縮する
//	void compress(const char* pszSource_, ModSize iSize_);
//	// 伸張する
//	void uncompress(char* pszDestination_);
//	// 圧縮データを得る
//	const ObjectPointer<BinaryData>&
//				getCompressedData() const;
//
//	// 圧縮データを消去する
//	void		clear();
//
	// 圧縮データを作り直す
	virtual ModSize
	reset();

	// 内容を複写する
	void		setValue(const CompressedBinaryData& cOther_);

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
	equals_NoCast(const Data& r) const;
	// 大小比較を行う(キャストなし)
	virtual int
	compareTo_NoCast(const Data& r) const;

	// 単項演算を行う(自分自身が NULL 値でない)
//	Common::ScalarData
//	virtual bool
//	operateWith_NotNull(DataOperation::Type op, Pointer& result) const;
//	virtual bool
//	operateWith_NotNull(DataOperation::Type op);
	// 四則演算を行う(キャストなし)
//	Common::ScalarData
//	virtual bool
//	operateWith_NoCast(DataOperation::Type op, const Data& r);

	// 付加機能を適用可能か調べる(自分自身が NULL 値でない)
	virtual bool
	isApplicable_NotNull(Function::Value function);
	// 付加機能を適用する(自分自身が NULL 値でない)
	virtual Pointer
	apply_NotNull(Function::Value function);

	// ダンプ可能か調べる(自分自身が NULL 値でない)
//	Common::ScalarData::
//	virtual bool
//	isAbleToDump_NotNull() const;
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
//	Common::BinaryData
//	virtual void
//	connect_NotNull(const BinaryData* pBinaryData_);

	// 値を標準出力へ出力する(自分自身が NULL 値でない)
	virtual void
	print_NotNull() const;

	//データ
	// getValueなどで作成されるのでmutable
	mutable ObjectPointer<BinaryData>	m_pBinaryData;
};

//	FUNCTION public
//	Common::CompressedBinaryData::CompressedBinaryData
//		-- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedBinaryData::
CompressedBinaryData()
	: CompressedData(),
	  m_pBinaryData(static_cast<BinaryData*>(0))
{
	setFunction(Function::Compressed);
}

//	FUNCTION public
//	Common::CompressedBinaryData::CompressedBinaryData
//		-- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModSize iValueSize_
//			伸張後のデータサイズ
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedBinaryData::
CompressedBinaryData(ModSize iValueSize_)
	: CompressedData(iValueSize_),
	  m_pBinaryData(static_cast<BinaryData*>(0))
{
	setFunction(Function::Compressed);
}

//	FUNCTION public
//	Common::CompressedBinaryData::CompressedBinaryData
//		-- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const void* pCompressedData_
//			圧縮済みデータへのポインター
//		ModSize iCompressedSize_
//			圧縮済みデータのサイズ
//		ModSize iValueSize_
//			伸張後のデータサイズ
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedBinaryData::
CompressedBinaryData(const void* pCompressedData_, ModSize iCompressedSize_, ModSize iValueSize_)
	: CompressedData(pCompressedData_, iCompressedSize_, iValueSize_),
	  m_pBinaryData(static_cast<BinaryData*>(0))
{
	setFunction(Function::Compressed);
}

//	FUNCTION public
//	Common::CompressedBinaryData::CompressedBinaryData
//		-- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const void* pUncompressedData_
//			圧縮していないデータへのポインター
//		ModSize iValueSize_
//			データサイズ
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedBinaryData::
CompressedBinaryData(const void* pUncompressedData_, ModSize iValueSize_)
	: CompressedData(),
	  m_pBinaryData(static_cast<BinaryData*>(0))
{
	setFunction(Function::Compressed);
	setValue(pUncompressedData_, iValueSize_);
}

//	FUNCTION public
//	Common::CompressedBinaryData::CompressedBinaryData
//		-- コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Common::CompressedBinaryData& cstrOther_
//			代入するバイナリ
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedBinaryData::
CompressedBinaryData(const CompressedBinaryData& cOther_)
{
	setValue(cOther_);
}

//	FUNCTION public
//	Common::CompressedBinaryData::~CompressedBinaryData
//		-- デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedBinaryData::
~CompressedBinaryData()
{
	m_pBinaryData = static_cast<BinaryData*>(0);
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_COMPRESSEDBINARYDATA_H

//
//	Copyright (c) 2001, 2004, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
