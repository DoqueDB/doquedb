// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModTime.h -- 時刻関連のクラスの定義
// 
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModTime_H__
#define __ModTime_H__

#include "ModCommonDLL.h"
#include "ModTimeSpan.h"
#include "ModSerial.h"
#include "ModDefaultManager.h"
#include "ModCharString.h"

//	TYPEDEF
//	ModTimeT -- 1970/1/1-00:00:00 からの経過秒数を表す型
//
//	NOTES

typedef time_t				ModTimeT;

//	CLASS
//	ModTime -- 時刻を表すクラス
//
//	NOTES
//		最初は、経過秒とミリ秒以外に時刻の年月日時分秒もメンバーに持っていた
//
//		MOD 内部では経過秒とミリ秒しか通常扱わないのに、
//		それらを変更するたびに年月日時分秒を求め直さなければならず、
//		そのための ::localtime(3C) の処理が思ったより重いため、
//		メンバーから年月日時分秒をなくした
//
//		他のクラスと同様に ModPureTime のようなメモリーハンドル非明示クラスと
//		明示クラスに分けるには、operator = の再定義や、オブジェクトを返す
//		メソッドへの対応が必要なことがわかったので断念する

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModTime
	: public	ModSerializer,
	  public	ModDefaultObject
{
public:
	ModTime(ModTimeT clock = 0, int milliSecond = 0);
	ModTime(int year, int month, int day,
			int hour = 0, int minute = 0, int second = 0, int milliSecond = 0);
	ModTime(const ModTime& src);				// コンストラクター

	ModTimeT				getTime() const;	// 1970/1/1-00:00:00 からの
												// 経過秒数を得る
	ModCommonDLL
	static ModTime			getCurrentTime();	// 現在時刻を得る

	int						getYear() const;	// 年を得る
	int						getMonth() const;	// 月を得る
	int						getDay() const;		// 日を得る
	int						getHour() const;	// 時を得る
	int						getMinute() const;	// 分を得る
	int						getSecond() const;	// 秒を得る
	int						getMilliSecond() const;
												// ミリ秒を得る

	ModCommonDLL
	ModCharString			getString(ModBoolean included = ModTrue) const;
												// 文字列の形式に変換する

	void					setTime(ModTimeT clock, int milliSecond = 0);
	void					setTime(int year, int month, int day,
									int hour = 0, int minute = 0,
									int second = 0, int milliSecond = 0);
												// 時刻を設定する

	ModTime&				operator =(const ModTime& src);
												// = 演算子
	ModTime					operator +(const ModTimeSpan& r) const;
												// + 演算子
	ModTime					operator -(const ModTimeSpan& r) const;
	ModTimeSpan				operator -(const ModTime& r) const;
												// - 演算子
	ModTime&				operator +=(const ModTimeSpan& src);
												// += 演算子
	ModTime&				operator -=(const ModTimeSpan& src);
												// -= 演算子
	ModBoolean				operator ==(const ModTime& r) const;
												// == 演算子
	ModBoolean				operator !=(const ModTime& r) const;
												// != 演算子
	ModBoolean				operator <(const ModTime& r) const;
												// < 演算子
	ModBoolean				operator >(const ModTime& r) const;
												// > 演算子
	ModBoolean				operator <=(const ModTime& r) const;
												// <= 演算子
	ModBoolean				operator >=(const ModTime& r) const;
												// >= 演算子

	ModCommonDLL
	void					serialize(ModArchive& archiver);
												// シリアル化する

	ModCommonDLL
	static ModTimeT			makeTime(int year, int month, int day,
									 int hour = 0, int minute = 0,
									 int second = 0);
												// 年月日時分秒を
												// 1970/1/1-00:00:00 からの
												// 経過秒へ変換する
	ModCommonDLL
	static void				divideTime(int* pYear, int* pMonth, int* pDay,
									   int* pHour, int* pMinute, int* pSecond,
									   ModTimeT clock);
												// 1970/1/1-00:00:00 からの
												// 経過秒を年月日時分秒へ
												// 分解する
private:
	ModCommonDLL
	void					validate();			// 経過秒および
												// ミリ秒の値を調整する

	ModTimeT				_clock;				// 1970/1/1-00:00:00 からの
												// 経過秒
	int						_milliSecond;		// ミリ秒
};

//	FUNCTION public
//	ModTime::ModTime -- 時刻を表すクラスのコンストラクター(0)
//
//	NOTES
//		1970/1/1-00:00:00 からの経過秒数およびミリ秒から時刻を生成する
//
//	ARGUMENTS
//		ModTimeT			clock
//			指定されたとき
//				生成する時刻の 1970/1/1-00:00:00 からの経過秒数
//			指定されないとき
//				0 が指定されたものとみなす
//		int					milliSecond
//			指定されたとき
//				生成する時刻にするために、第 1 引数に補正すべきミリ秒
//				値が [0, 999] の範囲を超えているときは、内部で調整される
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
ModTime::ModTime(ModTimeT clock, int milliSecond)
	: _clock(clock),
	  _milliSecond(milliSecond)
{
	// 必要ならばミリ秒を補正する

	this->validate();
}

//	FUNCTION public
//	ModTime::ModTime -- 時刻を表すクラスのコンストラクター(1)
//
//	NOTES
//		年、月、日、時、分、秒、ミリ秒から時刻を生成する
//
//	ARGUMENTS
//		int					year
//			生成する時刻の西暦年
//		int					month
//			生成する時刻の月[1, 12]
//		int					day
//			生成する時刻の日[1, 31]
//		int					hour
//			指定されたとき
//				生成する時刻の時[0, 23]
//			指定されないとき
//				0 が指定されたものとみなす
//		int					minute
//			指定されたとき
//				生成する時刻の分[0, 59]
//			指定されないとき
//				0 が指定されたものとみなす
//		int					second
//			指定されたとき
//				生成する時刻の秒[0, 59]
//			指定されないとき
//				0 が指定されたものとみなす
//		int					milliSecond
//			指定されたとき
//				生成する時刻のミリ秒
//				値が [0, 999] の範囲を超えているときは、内部で調整される
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			不正な年月日時分秒が与えられている
//			(ModTime::makeTime より)

inline
ModTime::ModTime(int year, int month, int day,
				 int hour, int minute, int second, int milliSecond)
	: _milliSecond(milliSecond)
{
	// 指定された年月日時分秒から 1970/1/1-00:00:00 からの経過秒数を求める

	_clock = ModTime::makeTime(year, month, day, hour, minute, second);

	// 必要ならばミリ秒を補正する

	this->validate();
}

//	FUNCTION public
//	ModTime::ModTime -- 時刻を表すクラスのコピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModTime&			src
//			コピーする時刻
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
ModTime::ModTime(const ModTime& src)
	: _clock(src._clock),
	  _milliSecond(src._milliSecond)
{ }

//	FUNCTION public
//	ModTime::getTime -- 時刻の 1970/1/1-00:00:00 からの経過秒数を得る
//
//	NOTES
//		ミリ秒以下は切り捨てられる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		1970/1/1-00:00:00 からの経過秒数
//
//	EXCEPTIONS
//		なし

inline
ModTimeT
ModTime::getTime() const
{
	return _clock;
}

//	FUNCTION public
//	ModTime::getYear -- 時刻の年を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めた年
//
//	EXCEPTIONS
//		なし

inline
int
ModTime::getYear() const
{
	int	year;
	ModTime::divideTime(&year, 0, 0, 0, 0, 0, _clock);
	return year;
}

//	FUNCTION public
//	ModTime::getMonth -- 時刻の月を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めた月
//
//	EXCEPTIONS
//		なし

inline
int
ModTime::getMonth() const
{
	int	month;
	ModTime::divideTime(0, &month, 0, 0, 0, 0, _clock);
	return month;
}

//	FUNCTION public
//	ModTime::getDay -- 時刻の日を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めた日
//
//	EXCEPTIONS
//		なし

inline
int
ModTime::getDay() const
{
	int	day;
	ModTime::divideTime(0, 0, &day, 0, 0, 0, _clock);
	return day;
}

//	FUNCTION public
//	ModTime::getHour -- 時刻の時を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めた時
//
//	EXCEPTIONS
//		なし

inline
int
ModTime::getHour() const
{
	int	hour;
	ModTime::divideTime(0, 0, 0, &hour, 0, 0, _clock);
	return hour;
}

//	FUNCTION public
//	ModTime::getMinute -- 時刻の分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めた分
//
//	EXCEPTIONS
//		なし

inline
int
ModTime::getMinute() const
{
	int	minute;
	ModTime::divideTime(0, 0, 0, 0, &minute, 0, _clock);
	return minute;
}

//	FUNCTION public
//	ModTime::getSecond -- 時刻の秒を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めた秒
//
//	EXCEPTIONS
//		なし

inline
int
ModTime::getSecond() const
{
	int	second;
	ModTime::divideTime(0, 0, 0, 0, 0, &second, _clock);
	return second;
}

//	FUNCTION public
//	ModTime::getMilliSecond -- 時刻のミリ秒を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めたミリ秒
//
//	EXCEPTIONS
//		なし

inline
int
ModTime::getMilliSecond() const
{
	return _milliSecond;
}

//	FUNCTION public
//	ModTime::setTime -- 1970/1/1-00:00:00 およびミリ秒を設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeT			clock
//			設定する 1970/1/1-00:00:00 からの経過秒数
//		int					milliSecond
//			指定されたとき
//				設定するミリ秒
//				値が [0, 999] の範囲を超えているときは、内部で調整される
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModTime::setTime(ModTimeT clock, int milliSecond)
{
	_clock = clock;
	_milliSecond = milliSecond;

	// 必要ならばミリ秒を補正する

	this->validate();
}

//	FUNCTION public
//	ModTime::setTime -- 年月日時分秒およびミリ秒を設定する
//
//	NOTES
//
//	ARGUMENTS
//		int					year
//			設定する西暦年
//		int					month
//			設定する月[1, 12]
//		int					day
//			設定する日[1, 31]
//		int					hour
//			指定されたとき
//				設定する時[0, 23]
//			指定されないとき
//				0 が指定されたものとみなす
//		int					minute
//			指定されたとき
//				設定する分[0, 59]
//			指定されないとき
//				0 が指定されたものとみなす
//		int					second
//			指定されたとき
//				設定する秒[0, 59]
//			指定されないとき
//				0 が指定されたものとみなす
//		int					milliSecond
//			指定されたとき
//				設定するミリ秒
//				値が [0, 999] の範囲を超えているときは、内部で調整される
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			不正な年月日時分秒が与えられている
//			(ModTime::makeTime より)

inline
void
ModTime::setTime(int year, int month, int day,
				 int hour, int minute, int second, int milliSecond)
{
	_milliSecond = milliSecond;

	// 指定された年月日時分秒から 1970/1/1-00:00:00 からの経過秒数を求める

	_clock = ModTime::makeTime(year, month, day, hour, minute, second);

	// 必要ならばミリ秒を補正する

	this->validate();
}

//	FUNCTION public
//	ModTime::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTime&			src
//			自分自身に代入する時刻
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
ModTime&
ModTime::operator =(const ModTime& src)
{
	_clock = src._clock;
	_milliSecond = src._milliSecond;
	return *this;
}

//	FUNCTION public
//	ModTime::operator + -- + 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		r
//			自分自身に加える時間差
//
//	RETURN
//		自分自身に時間差を加算後の時刻
//
//	EXCEPTIONS
//		なし

inline
ModTime
ModTime::operator +(const ModTimeSpan& r) const
{
	ModTime	t(_clock + r.second, _milliSecond + r.milliSecond);

	// 必要ならばミリ秒を補正する

	t.validate();

	return t;
}

//	FUNCTION public
//	ModTime::operator - -- - 演算子(1)
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		r
//			自分自身から減じる時間差
//
//	RETURN
//		自分自身に時間差を減算後の時刻
//
//	EXCEPTIONS
//		なし

inline
ModTime
ModTime::operator -(const ModTimeSpan& r) const
{
	ModTime t(_clock - r.second, _milliSecond - r.milliSecond);

	// 必要ならばミリ秒を補正する

	t.validate();

	return t;
}

//	FUNCTION public
//	ModTime::operator - -- - 演算子(2)
//
//	NOTES
//		自分の表す時刻と与えられた時刻の時間差を求める
//
//	ARGUMENTS
//		ModTime&			r
//			自分自身から減じる時刻
//
//	RETURN
//		自分自身に時刻を減算後の時間差
//
//	EXCEPTIONS
//		なし

inline
ModTimeSpan
ModTime::operator -(const ModTime& r) const
{
	ModTimeSpan	span(_clock - r._clock, _milliSecond - r._milliSecond);

	// 必要ならばミリ秒を補正する

	span.validate();

	return span;
}

//	FUNCTION public
//	ModTime::operator += -- += 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		src
//			自分自身に加える時間差
//
//	RETURN
//		時間差を加算後の自分自身
//
//	EXCEPTIONS
//		なし

inline
ModTime&
ModTime::operator +=(const ModTimeSpan& src)
{
	_clock += src.second;
	_milliSecond += src.milliSecond;

	// 必要ならばミリ秒を補正する

	this->validate();

	return *this;
}

//	FUNCTION public
//	ModTime::operator -= -- -= 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		src
//			自分自身から減じる時間差
//
//	RETURN
//		時間差を減算後の自分自身
//
//	EXCEPTIONS
//		なし

inline
ModTime&
ModTime::operator -=(const ModTimeSpan& src)
{
	_clock -= src.second;
	_milliSecond -= src.milliSecond;

	// 必要ならばミリ秒を補正する

	this->validate();

	return *this;
}

//	FUNCTION public
//	ModTime::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTime&			r
//			自分自身と比較する時刻
//
//	RETURN
//		ModTrue
//			与えられた時刻と自分自身は等しい
//		ModFalse
//			自分自身と等しくない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTime::operator ==(const ModTime& r) const
{
	return (_clock == r._clock &&
			_milliSecond == r._milliSecond) ? ModTrue : ModFalse;
}

//	FUNCTION public
//	ModTime::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTime&			r
//			自分自身と比較する時刻
//
//	RETURN
//		ModTrue
//			与えられた時刻と自分自身は等しくない
//		ModFalse
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTime::operator !=(const ModTime& r) const
{
	return (*this == r) ? ModFalse : ModTrue;
}

//	FUNCTION public
//	ModTime::operator < -- < 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTime&			r
//			自分自身と比較する時刻
//
//	RETURN
//		ModTrue
//			与えられた時刻のほうが自分自身より大きい
//		ModFalse
//			自分自身以下である
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTime::operator <(const ModTime& r) const
{
	return (_clock < r._clock ||
			(_clock == r._clock && _milliSecond < r._milliSecond)) ?
		ModTrue : ModFalse;
}

//	FUNCTION public
//	ModTime::operator > -- > 演算子
//
//	NOTES
//
// ARGUMENTS
//		ModTime&			r
//			自分自身と比較する時刻
//
//	RETURN
//		ModTrue
//			与えられた時刻のほうが自分自身より小さい
//		ModFalse
//			自分自身以上である
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTime::operator >(const ModTime& r) const
{
	return (_clock > r._clock ||
			(_clock == r._clock && _milliSecond > r._milliSecond)) ?
		ModTrue : ModFalse;
}

//	FUNCTION public
//	ModTime::operator <= -- <= 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTime&			r
//			自分自身と比較する時刻
//
//	RETURN
//		ModTrue
//			与えられた時刻は自分自身以上である
//		ModFalse
//			自分自身より小さい
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTime::operator <=(const ModTime& r) const
{
	return (*this > r) ? ModFalse :	ModTrue;
}

//	FUNCTION public
//	ModTime::operator >= -- >= 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTime&			r
//			自分自身と比較する時刻
//
//	RETURN
//		ModTrue
//			与えられた時刻は自分自身以下である
//		ModFalse
//			自分自身より大きい
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTime::operator >=(const ModTime& r) const
{
	return (*this < r) ? ModFalse : ModTrue;
}

#endif	// __ModTime_H__

//
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
