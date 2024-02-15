// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedStringData -- 圧縮された文字列型データ関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2003, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_COMPRESSEDSTRINGDATA_H
#define __TRMEISTER_COMMON_COMPRESSEDSTRINGDATA_H

#include "Common/Module.h"
#include "Common/CompressedData.h"
#include "Common/ObjectPointer.h"
#include "Common/StringData.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::CompressedStringData -- 圧縮された文字列型のデータを表すクラス
//
//	NOTES

class SYD_COMMON_FUNCTION CompressedStringData
	: public			StringData,				// 先である必要がある
	  virtual public	CompressedData
{
public:
	//コンストラクタ(1)
	CompressedStringData();
	//コンストラクタ(2)
	CompressedStringData(ModSize iValueSize_);
	//コンストラクタ(3)
	CompressedStringData(const void* pData_, ModSize iCompressedSize_, ModSize iValueSize_);
	//コンストラクタ(4)
	explicit CompressedStringData(const ModUnicodeString& cstrValue_);
	//コピーコンストラクタ
	explicit CompressedStringData(const CompressedStringData& cstrOther_);
	//デストラクタ
	virtual ~CompressedStringData();

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
//	Common::StringData
//	virtual const ModUnicodeString&
//	getValue() const;

	// 値を設定する
	virtual void
	setValue(const ModUnicodeString& cstrValue_);

	// 等しいか調べる
//	Common::Data
//	virtual bool
//	equals(const Data* r) const;
	// 大小比較を行う
//	Common::Data
//	virtual int
//	compareTo(const Data* r) const;

	// 自分自身に対してパターンがマッチするか調べる
//	Common::StringData
//	virtual bool
//	like(const StringData* pattern,
//		 ModUnicodeChar escape = UnicodeChar::usNull) const;
//	virtual bool
//	like(const StringData* pattern,
//		 NormalizingMethod::Value normalizing,
//		 ModUnicodeChar escape = UnicodeChar::usNull) const;

	// 連結する
//	Common::StringData
//	virtual void
//	connect(const StringData* pStringData_);

	// 値を標準出力へ出力する
//	Common::ScalarData
//	virtual void
//	print() const;

	// クラスIDを得る
//	Common::ScalarData
//	virtual int
//	getClassID() const;

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
//	setDumpedValue(const char* src);
	virtual ModSize
	setDumpedValue(const char* src, ModSize n);
	virtual ModSize
	setDumpedValue(
		const char* src, ModSize n,	EncodingForm::Value encodingForm);

	// 値をダンプする
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(char* p) const;
//	Common::StringData
//	virtual ModSize
//	dumpValue(char* dst, EncodingForm::Value encodingForm) const;

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

	// 値を設定する
	void
	setValue(const CompressedStringData& cOther_);

private:
	// シリアル化する(自分自身が NULL 値でない)
	virtual void
	serialize_NotNull(ModArchive& archiver);

	// コピーする(自分自身が NULL 値でない)
	virtual Pointer
	copy_NotNull() const;
	// キャストする(自分自身が NULL 値でない)
	virtual Pointer
	cast_NotNull(DataType::Type type, bool bForAssign_ = false) const;
//	Common::StringData
//	virtual Pointer
//	cast_NotNull(const Data& target) const;

	// 文字列の形式で値を得る(自分自身が NULL 値でない)
	virtual ModUnicodeString
	getString_NotNull() const;

	// 値を得る(自分自身が NULL 値でない)
	virtual const ModUnicodeString&
	getValue_NotNull() const;

	// 等しいか調べる(キャストなし)
	virtual bool
	equals_NoCast(const Data& r) const;
	// 大小比較を行う(キャストなし)
	virtual int
	compareTo_NoCast(const Data& r) const;

	// 自分自身に対してパターンがマッチするか調べる(自分自身が NULL 値でない)
//	virtual bool
//	like_NotNull(const StringData* pattern, ModUnicodeChar escape) const;
//	virtual bool
//	like_NotNull(const StringData* pattern,
//				 NormalizingMethod::Value normalizing,
//				 ModUnicodeChar escape) const;
	
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
	virtual ModSize
	getDumpSize_NotNull(EncodingForm::Value encodingForm) const;
	// 値をダンプする(自分自身が NULL 値でない)
	virtual	ModSize
	dumpValue_NotNull(char* dst) const;
	virtual	ModSize
	dumpValue_NotNull(char* dst, EncodingForm::Value encodingForm) const;

	// クラスIDを得る(自分自身が NULL 値でない)
	virtual int
	getClassID_NotNull() const;

	// 末尾にバイナリデータをつなげる
	virtual void
	connect_NotNull(const StringData* pStringData_);

	// 値を標準出力へ出力する(自分自身が NULL 値でない)
	virtual void
	print_NotNull() const;

	// (伸張された)データ
	mutable ObjectPointer<StringData>	m_pStringData;
};

//	FUNCTION public
//	Common::CompressedStringData::CompressedStringData
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
CompressedStringData::
CompressedStringData()
	: CompressedData(),
	  m_pStringData(static_cast<StringData*>(0))
{
	setFunction(Function::Compressed);
}

//	FUNCTION public
//	Common::CompressedStringData::CompressedStringData
//		-- コンストラクター
//
//	NOTES
//		このコンストラクターで作ったオブジェクトは
//		setValueしてから他のメソッドを呼ぶこと
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
CompressedStringData::
CompressedStringData(ModSize iValueSize_)
	: CompressedData(iValueSize_),
	  m_pStringData(static_cast<StringData*>(0))
{
	setFunction(Function::Compressed);
}

//	FUNCTION public
//	Common::CompressedStringData::CompressedStringData
//		-- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const void* pData_
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
CompressedStringData::
CompressedStringData(const void* pData_, ModSize iCompressedSize_, ModSize iValueSize_)
	: CompressedData(pData_, iCompressedSize_, iValueSize_),
	  m_pStringData(static_cast<StringData*>(0))
{
	setFunction(Function::Compressed);
}

//	FUNCTION public
//	Common::CompressedStringData::CompressedStringData
//		-- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const ModUnicodeString& cstrValue_
//			代入する文字列
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedStringData::
CompressedStringData(const ModUnicodeString& cstrValue_)
	: CompressedData(),
	  m_pStringData(static_cast<StringData*>(0))
{
	setFunction(Function::Compressed);
	setValue(cstrValue_);
}

//	FUNCTION public
//	Common::CompressedStringData::CompressedStringData
//		-- コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Common::CompressedStringData& cstrOther_
//			代入する文字列
//
//	RETURN
//		なし
//
//	EXCEPTION

inline
CompressedStringData::
CompressedStringData(const CompressedStringData& cOther_)
{
	setValue(cOther_);
}

//	FUNCTION public
//	Common::CompressedStringData::~CompressedStringData
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
CompressedStringData::
~CompressedStringData()
{
	m_pStringData = static_cast<StringData*>(0);
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_COMPRESSEDSTRINGDATA_H

//
//	Copyright (c) 2001, 2003, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
