// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LanguageData.h -- 言語指定関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_LANGUAGEDATA_H
#define __TRMEISTER_COMMON_LANGUAGEDATA_H

#include "Common/Module.h"
#include "Common/ScalarData.h"

#include "ModLanguageSet.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::LanguageData -- 言語指定データを表すクラス
//
//	NOTES

class SYD_COMMON_FUNCTION LanguageData
	: public	ScalarData
{
public:
	// デフォルトコンストラクタ
	LanguageData();
	// コンストラクタ
	explicit LanguageData(const ModLanguageSet& v);
	explicit LanguageData(const ModUnicodeString& s);
	// デストラクタ
	virtual ~LanguageData();

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

	// 言語指定集合を得る
	const ModLanguageSet&
	getValue() const;

	// 文字列の形式から言語指定集合を設定する
	void setValue(const ModUnicodeString& cstrValue_);

	// 文字列の形式で値を得る
//	Common::ScalarData
//	virtual ModUnicodeString
//	getString() const;

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
//	Common::ScalarData
//	virtual ModSize
//	setDumpedValue(const char* p);
	virtual ModSize
	setDumpedValue(const char* p, ModSize size);

//	Common::ScalarData
//	virtual ModSize
//	setDumpedValue(ModSerialIO& cSerialIO_);
	virtual ModSize
	setDumpedValue(ModSerialIO& cSerialIO_, ModSize size);

	// 値をダンプする
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(char* p) const;
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(ModSerialIO& cSerialIO_) const;

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;

private:
	// シリアル化する(自分自身が NULL 値でない)
	virtual void
	serialize_NotNull(ModArchive& archiver);

	// コピーする(自分自身が NULL 値でない)
	virtual Data::Pointer
	copy_NotNull() const;
	// キャストする(自分自身が NULL 値でない)
	virtual Pointer
	cast_NotNull(DataType::Type type, bool bForAssign_ = false) const;

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
//	Common::ScalarData
//	virtual bool
//	isApplicable_NotNull(Function::Value function);
	// 付加機能を適用する(自分自身が NULL 値でない)
//	Common::ScalarData
//	virtual Pointer
//	apply_NotNull(Function::Value function);

	// ダンプ可能か調べる(自分自身が NULL 値でない)
	virtual bool
	isAbleToDump_NotNull() const;
	// 常に固定長であるかを得る(自分自身が NULL 値でない)
//	Common::ScalarData
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

	// 値を標準出力へ出力する(自分自身が NULL 値でない)
//	Common::ScalarData
//	virtual void
//	print_NotNull() const;

	// 値
	ModLanguageSet			_v;
};

//	FUNCTION public
//	Common::LanguageData::LanguageData -- デフォルトコンストラクタ
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

inline
LanguageData::LanguageData()
	: ScalarData(DataType::Language)
{}

//	FUNCTION public
//	Common::LanguageData::LanguageData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		ModLanguageSet&		v
//			生成元となる言語指定集合
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
LanguageData::LanguageData(const ModLanguageSet& v)
	: ScalarData(DataType::Language),
	  _v(v)
{}

//	FUNCTION public
//	Common::LanguageData::LanguageData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	s
//			生成元となる言語指定の文字列表現
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
LanguageData::LanguageData(const ModUnicodeString& s)
	: ScalarData(DataType::Language),
	  _v(s)
{}

//	FUNCTION public
//	Common::LanguageData::~LangageData -- デストラクタ
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

inline
LanguageData::~LanguageData()
{}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_LANGUAGEDATA_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2007, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserve
//
