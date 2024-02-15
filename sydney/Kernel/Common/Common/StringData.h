// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StringData -- 文字列型データ関連のクラス定義、関数宣言
// 
// Copyright (c) 1999, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_STRINGDATA_H
#define __TRMEISTER_COMMON_STRINGDATA_H

#include "Common/Module.h"
#include "Common/Collation.h"
#include "Common/ScalarData.h"

#include "ModUnicodeString.h"

class ModUnicodeRegularExpression;

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::StringData -- 文字列型のデータを表すクラス
//
//	NOTES

class SYD_COMMON_FUNCTION StringData
	: public	ScalarData
{
	friend class CompressedStringData;
public:
	struct EncodingForm
	{
		//	ENUM
		//	Common::StringData::EncodingForm::Value --
		//		符号化方式を表す値の列挙型Enumeration type of value in which encoding method is shown
		//
		//	NOTES

		enum Value
		{
			UTF8 =			ModKanjiCode::utf8,
			UCS2 =			ModKanjiCode::ucs2,
			Unknown =		ModKanjiCode::unknown
		};
	};

	struct NormalizingMethod
	{
		//	ENUM
		//	Common::StringData::NormalizingMethod::Value --
		//		正規化手段を表す値の列挙型Enumeration type of value in which regularized means is shown
		//
		//	NOTES

		enum Value
		{
			// 正規化しない
			None =			0,
			// 組み込み
			BuiltIn,
			// UNA
			UNA,
			// 組み込み + かな同一視
			BuiltIn_HK,
			// 値の数
			Count,
			// 不明
			Unknown =		Count
		};
	};

	// デフォルトコンストラクタ
	StringData(EncodingForm::Value encodingForm = EncodingForm::UCS2,
			   Collation::Type::Value collation = Collation::Type::Implicit);
	// コンストラクタ
	explicit StringData(
		const ModUnicodeString& cstrValue_,
		EncodingForm::Value encodingForm = EncodingForm::UCS2,
		Collation::Type::Value collation = Collation::Type::Implicit);
	explicit StringData(
		const ModUnicodeChar* src, ModSize n = 0,
		EncodingForm::Value encodingForm = EncodingForm::UCS2,
		Collation::Type::Value collation = Collation::Type::Implicit);
#ifdef OBSOLETE
	explicit StringData(const char* src, ModSize n = 0,
						ModKanjiCode::KanjiCodeType code = ModKanjiCode::ucs2,
						Collation::Type::Value collation = Collation::Type::Implicit);
#endif
	// デストラクタ
	virtual ~StringData();

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
	virtual Pointer
	castToDecimal(bool bForAssign_ = false) const;

	// 文字列の形式で値を得る
//	Common::ScalarData
//	virtual ModUnicodeString
//	getString() const;

	// 値を得る
	virtual const ModUnicodeString&
	getValue() const;
	// 値を得る(自分自身が NULL 値でない)
	virtual const ModUnicodeString&
	getValue_NotNull() const;
	// 末尾の終端文字を除いた長さを得る
	ModSize
	getLength() const;
	// 符号化方式を得る
	EncodingForm::Value
	getEncodingForm() const;
	// Collationを得る
	Collation::Type::Value
	getCollation() const;

	// 符号化方式を設定する
	void setEncodingForm(EncodingForm::Value eEncodingForm_);
	// Collationを設定する
	void setCollation(Collation::Type::Value eCollation_);

	// 値を設定する
	virtual void
	setValue(const ModUnicodeString& cstrValue_);
	virtual void
	setValue(const ModUnicodeChar* src, ModSize n = 0);
	virtual void
	setValue(const char* src, ModSize n = 0,
			 ModKanjiCode::KanjiCodeType code = ModKanjiCode::ucs2);

	// 等しいか調べる
//	Common::Data
//	virtual bool
//	equals(const Data* r) const;
	// 大小比較を行う
//	Common::Data
//	virtual int
//	compareTo(const Data* r) const;

	// 自分自身に対して正規表現がマッチするか調べるWhether the regular expression matches to oneself is examined. 
	virtual bool
	similar(ModUnicodeRegularExpression* pattern) const;

	// 連結する
	virtual void
	connect(const StringData* pStringData_);

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
	virtual ModSize
	getDumpSize(EncodingForm::Value encodingForm) const;
	// 自分自身にダンプされた値を設定する
//	Common::Data
//	virtual ModSize
//	setDumpedValue(const char* src);
	virtual ModSize
	setDumpedValue(const char* src, ModSize n);
	virtual ModSize
	setDumpedValue(
	const char* src, ModSize n,	EncodingForm::Value encodingForm);

//	Common::Data
//	virtual ModSize
//	setDumpedValue(ModSerialIO& cSerialIO_);
	virtual ModSize
	setDumpedValue(ModSerialIO& cSerialIO_, ModSize n);
	virtual ModSize
	setDumpedValue(ModSerialIO& cSerialIO_, ModSize n,	EncodingForm::Value encodingForm_);

	// 値をダンプする
	using ScalarData::dumpValue;
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(char* dst) const;
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(ModSerialIO& cSerialIO_) const;	
	virtual ModSize
	dumpValue(char* dst, EncodingForm::Value encodingForm) const;
	virtual ModSize
	dumpValue(ModSerialIO& cSerialIO_, EncodingForm::Value encodingForm) const;

	// 自分自身に対して正規表現がマッチするか調べる(自分自身が NULL 値でない)
	virtual bool
	similar_NotNull(ModUnicodeRegularExpression* pattern) const;

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;

	// 数値に変換する
	static Pointer getInteger(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
							  Common::DataType::Type eType_, bool bForAssign_);
	static Pointer getFloat(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
							Common::DataType::Type eType_, bool bForAssign_);

	// Common::Dataをあらかじめ用意するバージョン
	static bool getInteger(Data& cData_,
						   const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_);
	static bool getFloat(Data& cData_,
						 const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_);
	// char*版
	static bool getInteger(Data& cData_, const char* pHead_, const char* pTail_);
	static bool getFloat(Data& cData_, const char* pHead_, const char* pTail_);
	// privitiveデータで得るバージョン
	static bool getInteger(int& iValue_,
						   const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_);
	static bool getFloat(double& dblValue_,
						 const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_);
	// primitiveデータで得る -- char*版
	static bool getInteger(int& iValue_, const char* pHead_, const char* pTail_);
	static bool getFloat(double& dblValue_, const char* pHead_, const char* pTail_);

protected:
	// 値を設定する
	void
	setValue(const StringData& cOther_);

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
	virtual Pointer
	cast_NotNull(const Data& target, bool bForAssign_ = false) const;

	// 文字列の形式で値を得る(自分自身が NULL 値でない)
	virtual ModUnicodeString
	getString_NotNull() const;

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
	virtual ModSize
	getDumpSize_NotNull(EncodingForm::Value encodingForm) const;
	// 値をダンプする(自分自身が NULL 値でない)
	virtual	ModSize
	dumpValue_NotNull(char* dst) const;
	virtual ModSize
	dumpValue_NotNull(char* dst, EncodingForm::Value encodingForm) const;

	virtual	ModSize
	dumpValue_NotNull(ModSerialIO& cSerialIO_) const;
	virtual ModSize
	dumpValue_NotNull(ModSerialIO& cSerialIO_, EncodingForm::Value encodingForm_) const;

	// クラスIDを得る(自分自身が NULL 値でない)
	virtual int
	getClassID_NotNull() const;

	// 末尾にバイナリデータをつなげる
	virtual void
	connect_NotNull(const StringData* pStringData_);

	// 値を標準出力へ出力する(自分自身が NULL 値でない)
	virtual void
	print_NotNull() const;

	// 実際の値を用いてSQLDataを得る(getSQLTypeの下請け)
	virtual bool getSQLTypeByValue(SQLData& cType_);

	// 値
	ModUnicodeString		m_cstrValue;
	// 符号化方式
	EncodingForm::Value		_encodingForm;
	// Collation
	Collation::Type::Value		_collation;
};

//	FUNCTION public
//	Common::StringData::StringData -- デフォルトコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Common::StringData::EncodingForm::Value	encodingForm
//			指定されたとき
//				文字列の符号化方式
//			指定されないとき
//				Common::StringData::EncodingForm::UCS2 が指定されたものとする
//		Common::StringData::Collation::Type::Value collation
//			指定されたとき
//				文字列の比較方式
//			指定されないとき
//				Common::StringData::Collation::Type::Implicit が指定されたものとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
StringData::StringData(EncodingForm::Value encodingForm,
					   Collation::Type::Value collation)
	: ScalarData(DataType::String),
	  _encodingForm(encodingForm),
	  _collation(collation)
{}

//	FUNCTION public
//	Common::StringData::StringData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	cstrValue_
//			設定する文字列
//		Common::StringData::EncodingForm::Value	encodingForm
//			指定されたとき
//				文字列の符号化方式
//			指定されないとき
//				Common::StringData::EncodingForm::UCS2 が指定されたものとする
//		Common::StringData::Collation::Type::Value collation
//			指定されたとき
//				文字列の比較方式
//			指定されないとき
//				Common::StringData::Collation::Type::Implicit が指定されたものとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
StringData::StringData(
	const ModUnicodeString& cstrValue_, EncodingForm::Value encodingForm,
	Collation::Type::Value collation)
	: ScalarData(DataType::String),
	  m_cstrValue(cstrValue_),
	  _encodingForm(encodingForm),
	  _collation(collation)
{}

//	FUNCTION public
//	Common::StringData::StringData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeChar*		src
//			設定する文字列を格納する領域の先頭アドレス
//		ModSize				n
//			指定されたとき
//				設定する文字列の文字数
//			指定されないとき
//				与えられた領域の先頭から最初に見つかった終端文字までを格納する
//		Common::StringData::EncodingForm::Value	encodingForm
//			指定されたとき
//				文字列の符号化方式
//			指定されないとき
//				Common::StringData::EncodingForm::UCS2 が指定されたものとする
//		Common::StringData::Collation::Type::Value collation
//			指定されたとき
//				文字列の比較方式
//			指定されないとき
//				Common::StringData::Collation::Type::Implicit が指定されたものとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
StringData::StringData(
	const ModUnicodeChar* src, ModSize n, EncodingForm::Value encodingForm,
	Collation::Type::Value collation)
	: ScalarData(DataType::String),
	  m_cstrValue(src, n),
	  _encodingForm(encodingForm),
	  _collation(collation)
{}

#ifdef OBSOLETE
//	FUNCTION public
//	Common::StringData::StringData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		char*				src
//			設定する文字列を格納する領域の先頭アドレス
//		ModSize				n
//			設定する文字列の文字数
//		ModKanjiCode::KanjiCodeType	code
//			設定する文字列の文字コード
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
StringData::StringData(
	const char* src, ModSize n, ModKanjiCode::KanjiCodeType code)
	: ScalarData(DataType::String),
	  m_cstrValue(src, n, code)
{}
#endif

//	FUNCTION public
//	Common::StringData::~StringData -- デストラクタ
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
StringData::~StringData()
{}

//	FUNCTION private
//	Common::StringData::getString_NotNull -- 文字列値を得る
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

inline
ModUnicodeString
StringData::getString_NotNull() const
{
	return getValue();
}

//	FUNCTION public
//	Common::StringData::setValue -- 値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeChar*		src
//			設定する文字列を格納する領域の先頭アドレス
//		ModSize				n
//			設定する文字列の文字数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
StringData::setValue(const ModUnicodeChar* src, ModSize n)
{
	//【注意】	以前は ModUnicodeString::allocateCopy の
	//			符号化方式の指定なしのものを呼び出していた
	//
	//			符号化方式の指定のなしのものと、
	//			符号化方式として UCS2 を指定したものでは動作が異なる
	//
	//			符号化方式を指定しないと、
	//			複写元が指定された文字数無くても、
	//			指定された文字数ぶんをコピーするため、
	//			複写元によっては UMR が起きる可能性がある

	setDumpedValue((const char*)src,
				   n * sizeof(ModUnicodeChar), EncodingForm::UCS2);
}

//	FUNCTION public
//	Common::StringData::setValue -- 値を設定する
//
//	NOTES
//		bulk insertのためにOBSOLETEを外した
//
//	ARGUMENTS
//		char*				src
//			設定する文字列を格納する領域の先頭アドレス
//		ModSize				n
//			設定する文字列の文字数
//		ModKanjiCode::KanjiCodeType	code
//			設定する文字列の文字コード
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
StringData::setValue(
	const char* src, ModSize n, ModKanjiCode::KanjiCodeType code)
{
	if (n) {
		m_cstrValue.allocateCopy(src, n, code);
	} else {
		m_cstrValue.clear();
	}

	setNull(false);
}

//	FUNCTION private
//	Common::StringData::getDumpSize_NotNull --
//		保持する値をダンプしたときの文字列データのサイズを得る
//
//	NOTES
//		保持する値を getEncodingForm() の符号化方式で変換し、
//		ダンプしたときのサイズを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたサイズ(B 単位)
//
//	EXCEPTIONS

// virtual
inline
ModSize
StringData::getDumpSize_NotNull() const
{
	return getDumpSize_NotNull(getEncodingForm());
}

//	FUNCTION public
//	Common::StringData::setDumpedValue --
//		ダンプされた文字列データを使って値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		char*				src
//			ダンプされた文字列データを格納する領域の先頭アドレスであり、
//			getEncodingForm() で符号化されている必要がある
//		ModSize				n
//			ダンプされた文字列データを格納する領域のサイズ(B 単位)
//
//	RETURN
//		設定されたダンプされた文字列データのサイズ(B 単位)
//
//	EXCEPTIONS

// virtual
inline
ModSize
StringData::setDumpedValue(const char* src, ModSize n)
{
	return setDumpedValue(src, n, getEncodingForm());
}

// virtual
inline
ModSize
StringData::setDumpedValue(ModSerialIO& cSerialIO_, ModSize n)
{
	return setDumpedValue(cSerialIO_, n, getEncodingForm());
}

//	FUNCTION private
//	Common::StringData::dumpValue_NotNull -- 保持する値をダンプする
//
//	NOTES
//		保持する値は getEncodingForm() の符号化方式で変換され、ダンプされる
//
//	ARGUMENTS
//		char*				dst
//			保持する値がダンプされる領域の先頭アドレス
//
//	RETURN
//		ダンプされた文字列データのサイズ(B 単位)
//
//	EXCEPTIONS

// virtual
inline
ModSize
StringData::dumpValue_NotNull(char* dst) const
{
	return dumpValue_NotNull(dst, getEncodingForm());
}

inline
ModSize
StringData::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	return dumpValue_NotNull(cSerialIO_, getEncodingForm());
}

//	FUNCTION public
//	Common::StringData::getLength -- 末尾の終端文字を除いた長さを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた長さ
//
//	EXCEPTIONS

inline
ModSize
StringData::getLength() const
{
	return getValue().getLength();
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_STRINGDATA_H

//
//	Copyright (c) 1999, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
