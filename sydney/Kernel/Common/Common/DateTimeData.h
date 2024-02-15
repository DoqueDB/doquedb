// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DateTimeData.h -- 日時型データ関連のクラス定義、関数宣言
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

#ifndef __TRMEISTER_COMMON_DATETIMEDATA_H
#define __TRMEISTER_COMMON_DATETIMEDATA_H

#include "Common/Module.h"
#include "Common/DateData.h"
#include "Common/ScalarData.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::DateTimeData -- 日時型のデータを表すクラス
//
//	NOTES

class SYD_COMMON_FUNCTION DateTimeData
	: public	ScalarData
{
public:
	//コンストラクタ(1)
	explicit DateTimeData(int iPrecision_ = 3);
	//コンストラクタ(2)
	explicit DateTimeData(const ModUnicodeString& cstrCalendar_);
	//コンストラクタ(2.5)
	DateTimeData(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_);
	//コンストラクタ(3)
	DateTimeData(const ModUnicodeString& cstrCalendar_,
				 const ModUnicodeString& cstrDateDelimiter_,
				 const ModUnicodeString& cstrDateTimeDelimiter_,
				 const ModUnicodeString& cstrTimeDelimiter_,
				 const ModUnicodeString& cstrMsecDelimiter_);
	//コンストラクタ(4)
	DateTimeData(const DateData& cDate_,
				 int iHour_, int iMinute_, int iSecond_);
	//コンストラクタ(5)
	DateTimeData(const DateData& cDate_,
				 int iHour_, int iMinute_, int iSecond_,
				 int iMillisecond_, int iPrecision_ = 3);
	//コンストラクタ(6)
	DateTimeData(int iYear_, int iMonth_, int iDate_,
				 int iHour_, int iMinute_, int iSecond_);
	//コンストラクタ(7)
	DateTimeData(int iYear_, int iMonth_, int iDate_,
				 int iHour_, int iMinute_, int iSecond_,
				 int iMillisecond_, int iPrecision_ = 3);
	//コンストラクタ(8)
	explicit DateTimeData(const IntegerArrayData& cCalendar_);

	//デストラクタ
	virtual ~DateTimeData();

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

	//値を取出す
	IntegerArrayData* getValue() const;
	//年を取出す
	int getYear() const;
	//月を取出す
	int getMonth() const;
	//日を取出す
	int getDate() const;
	//時を取出す
	int getHour() const;
	//分を取出す
	int getMinute() const;
	//秒を取出す
	int getSecond() const;
	//ミリ秒を取出す
	int getMillisecond() const;
	//ミリ秒を文字列表現する際の桁数を取出す
	int getPrecision() const;

	//値を設定する(1)
	void setValue(const ModUnicodeString& cstrDate_,
				  bool bForAssign_ = false);
	//値を設定する(1')
	void setValue(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
				  bool bForAssign_ = false);
	//値を設定する(2)
	void setValue(const ModUnicodeString& cstrDate_,
				  const ModUnicodeString& cstrDateDelimiter_,
				  const ModUnicodeString& cstrDateTimeDelimiter_,
				  const ModUnicodeString& cstrTimeDelimiter_,
				  const ModUnicodeString& cstrMsecDelimiter_,
				  bool bForAssign_ = false);
	//値を設定する(2')
	void setValue(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
				  const ModUnicodeString& cstrDateDelimiter_,
				  const ModUnicodeString& cstrDateTimeDelimiter_,
				  const ModUnicodeString& cstrTimeDelimiter_,
				  const ModUnicodeString& cstrMsecDelimiter_,
				  bool bForAssign_ = false);
	//値を設定する(3)
	void setValue(const DateData& cDate_,
				  int iHour_, int iMinute_, int iSecond_,
				  bool bForAssign_ = false);
	//値を設定する(4)
	void setValue(const DateData& cDate_,
				  int iHour_, int iMinute_, int iSecond_,
				  int iMillisecond, int iPrecision = 3,
				  bool bForAssign_ = false);
	//値を設定する(5)
	void setValue(int iYear_, int iMonth_, int iDate_,
				  int iHour_, int iMinute_, int iSecond_,
				  bool bForAssign_ = false);
	//値を設定する(6)
	void setValue(int iYear_, int iMonth_, int iDate_,
				  int iHour_, int iMinute_, int iSecond_,
				  int iMillisecond_, int iPrecision_ = 3,
				  bool bForAssign_ = false);
	//値を設定する(7)
	void setValue(const IntegerArrayData& cIntegerArrayData_,
				  bool bForAssign_ = false);
	//値を設定する(8) -- char*版
	void setValue(const char* pHead_, const char* pTail_,
				  const char* pDateDelimiter_ = 0,
				  const char* pDateTimeDelimiter_ = 0,
				  const char* pTimeDelimiter_ = 0,
				  const char* pMsecDelimiter_ = 0,
				  bool bForAssign_ = false);
	//値を現在時刻に設定する
	void setCurrent();
	//年を設定する
	void setYear(int iYear_);
	//月を設定する
	void setMonth(int iMonth_);
	//日を設定する
	void setDate(int iDate_);
	//時を設定する
	void setHour(int iHour_);
	//分を設定する
	void setMinute(int iMinute_);
	//秒を設定する
	void setSecond(int iSecond_);
	//ミリ秒を設定する
	void setMillisecond(int iMillisecond_);
	//ミリ秒を文字列表現する際の桁数を設定
	void setPrecision(int iPrecision_);

	// 文字列の形式で値を得る
//	Common::ScalarData
//	virtual ModUnicodeString
//	getString() const;

	//年、月、日を区切るデリミタを設定する
	void setDateDelimiter(const ModUnicodeString& cstrDelimiter_);
	//年月日と時分秒を区切るデリミタを設定する
	void setDateTimeDelimiter(const ModUnicodeString& cstrDelimiter_);
	//時、分、秒を区切るデリミタを設定する
	void setTimeDelimiter(const ModUnicodeString& cstrDelimiter_);
	//秒、ミリ秒を区切るデリミタを設定する
	void setMsecDelimiter(const ModUnicodeString& cstrDelimiter_);

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
	virtual ModSize setDumpedValue(const char* pszValue_);
	virtual ModSize setDumpedValue(const char* pszValue_, ModSize uSize_);
	
//	virtual ModSize setDumpedValue(ModSerialIO& cSerialIO_);
	virtual ModSize setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_);

	// 値をダンプする
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(char* p) const;
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(ModSerialIO& cSerialIO_) const;

	static ModSize getArchiveSize();

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
	virtual bool
	isFixedSize_NotNull() const;
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
	virtual void
	print_NotNull() const;

	//時間のみ設定する
	void setTime(int iHour_, int iMinute_, int iSecond_,
				 int iMillisecond_, int iPrecision_,
				 bool bForAssign_ = false);
	
	//日付
	DateData m_cDateData;
	//時
	int m_iHour;
	//分
	int m_iMinute;
	//秒
	int m_iSecond;
	//ミリ秒
	int m_iMillisecond;
	//ミリ秒を文字列にする際の桁数
	int m_iPrecision;

	//年月日と時分秒を区切るデリミタ
	ModUnicodeString m_cstrDateTimeDelimiter;
	//時、分、秒を区切るデリミタ
	ModUnicodeString m_cstrTimeDelimiter;
	//秒、ミリ秒を区切るデリミタ
	ModUnicodeString m_cstrMsecDelimiter;

	// デフォルトのデリミタ
	static ModUnicodeString m_cstrDefaultDateTimeDelimiter;
	static ModUnicodeString m_cstrDefaultTimeDelimiter;
	static ModUnicodeString m_cstrDefaultMsecDelimiter;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_DATETIMEDATA_H

//
//	Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
