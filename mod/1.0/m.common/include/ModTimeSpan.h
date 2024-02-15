// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModTimeSpan.h -- ModTimeSpan のクラス定義
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

#ifndef	__ModTimeSpan_H__
#define __ModTimeSpan_H__

#include "ModSerial.h"
#include "ModDefaultManager.h"

//
// CLASS
// ModTimeSpan -- 時間差を表す機能クラス
//
// NOTES
// このクラスは時間差を表すのに用いる機能クラスである。
// 実際にはメモリハンドルを明示した型ModTimeを使う。
//

// ModTimeSpanに変更して、サブクラスとしてメモリハンドルクラスを
// 明示したクラスを作成するには、operator=()の再定義や
// オブジェクトを返すメソッドへの対応が必要なことがわかったので断念し、
// 直接、ModDefaultObjectのサブクラスとして作成する。
//

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModTimeSpan
	: public	ModSerializer,
	  public	ModDefaultObject
{
public:
	friend class ModTime;

	ModTimeSpan(const time_t seconds = 0, const int milliSeconds_ = 0);
	ModTimeSpan(const time_t days_,
				const int hours_, const int minutes_, const int seconds_,
				const int milliSeconds_ = 0);
	ModTimeSpan(const ModTimeSpan& src);		// コンストラクター

	// いろんな形で時刻を得る
	time_t getDays() const;
	int getHours() const;
	int getMinutes() const;
	int getSeconds() const;
	int getMilliSeconds() const;

	time_t getTotalHours() const;
	time_t getTotalMinutes() const;
	time_t getTotalSeconds() const;
	time_t getTotalMilliSeconds() const;


	ModTimeSpan&			operator =(const ModTimeSpan& src);
												// = 演算子
	ModTimeSpan				operator +(const ModTimeSpan& r) const;
												// + 演算子
	ModTimeSpan				operator -(const ModTimeSpan& r) const;
												// - 演算子
	ModTimeSpan&			operator +=(const ModTimeSpan& src);
												// += 演算子
	ModTimeSpan&			operator -=(const ModTimeSpan& src);
												// -= 演算子
	ModBoolean				operator ==(const ModTimeSpan& r) const;
												// == 演算子
	ModBoolean				operator !=(const ModTimeSpan& r) const;
												// != 演算子
	ModBoolean				operator <(const ModTimeSpan& r) const;
												// < 演算子
	ModBoolean				operator >(const ModTimeSpan& r) const;
												// > 演算子
	ModBoolean				operator <=(const ModTimeSpan& r) const;
												// <= 演算子
	ModBoolean				operator >=(const ModTimeSpan& r) const;
												// >= 演算子

	void					serialize(ModArchive& archiver);
												// シリアル化する
private:
	ModTimeSpan&			validate();			// 時間を表す秒および
												// ミリ秒を調整する

	time_t					second;				// 秒
	int						milliSecond;		// ミリ秒
};

//
// FUNCTION
// ModTimeSpan::ModTimeSpan -- time_t とミリ秒によるコンストラクタ
//
// NOTES
// 秒とミリ秒を指定したコンストラクタ
//
// ARGUMENTS
// const time_t seconds_
//		時間差の秒単位
// const int milliSeconds_
//		時間差のミリ秒単位
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

inline
ModTimeSpan::ModTimeSpan(const time_t seconds_, const int milliSeconds_)
	: second(seconds_), milliSecond(milliSeconds_)
{
	// 必要であれば、調整する

	(void) this->validate();
}

//
// FUNCTION
// ModTimeSpan::ModTimeSpan -- 日数と時分秒ミリ秒によるコンストラクタ
//
// NOTES
// 日数と時分秒ミリ秒を指定したコンストラクタ
//
// ARGUMENTS
// const time_t days_
//		時間差の日数単位
// const int hours_
//		時間差の時単位
// const int minutes_
//		時間差の分単位
// const int seconds_
//		時間差の秒単位
// const int milliSeconds_
//		時間差のミリ秒単位
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

inline
ModTimeSpan::ModTimeSpan(const time_t days_,
						 const int hours_, const int minutes_,
						 const int seconds_, const int milliSeconds_)
	: milliSecond(milliSeconds_)
{
	this->second = ((days_ * 24 + hours_) * 60 + minutes_) * 60 + seconds_;

	// 必要であれば、調整する

	(void) this->validate();
}

//
// FUNCTION
// ModTimeSpan::ModTimeSpan -- コピーコンストラクタ
//
// NOTES
// ModTimeSpan 型のコピーコンストラクタ
//
// ARGUMENTS
// const ModTimeSpan& orginal
//		コピー元のデータへの参照
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

inline
ModTimeSpan::ModTimeSpan(const ModTimeSpan& src)
	: second(src.second),
	  milliSecond(src.milliSecond)
{ }

//
// FUNCTION
// ModTimeSpan::getDays -- 時間差の日数を得る
//
// NOTES
// 時間差を日数、時分秒で表した時の日数を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 時間差を日数、時分秒で表した時の日数を返す。
//
// EXCEPTIONS
// なし
//
inline time_t
ModTimeSpan::getDays() const
{
	return (time_t)(this->second / 60 / 60 / 24);
}

//
// FUNCTION
// ModTimeSpan::getHours -- 時間差の時間を得る
//
// NOTES
// 時間差を日数、時分秒で表した時の時を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 時間差を日数、時分秒で表した時の時を返す。
//
// EXCEPTIONS
// なし
//
inline int
ModTimeSpan::getHours() const
{
	return (int)((this->second / 60 / 60) % 24);
}

//
// FUNCTION
// ModTimeSpan::getMinutes -- 時間差の分を得る
//
// NOTES
// 時間差を日数、時分秒で表した時の分を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 時間差を日数、時分秒で表した時の分を返す。
//
// EXCEPTIONS
// なし
//
inline int
ModTimeSpan::getMinutes() const
{
	return (int)((this->second / 60) % 60);
}

//
// FUNCTION
// ModTimeSpan::getSeconds -- 時間差の秒を得る
//
// NOTES
// 時間差を日数、時分秒で表した時の秒を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 時間差を日数、時分秒で表した時の秒を返す。
//
// EXCEPTIONS
// なし
//
inline int
ModTimeSpan::getSeconds() const
{
	return (int)(this->second % 60);
}

//
// FUNCTION
// ModTimeSpan::getMilliSeconds -- 時間差のミリ秒を得る
//
// NOTES
// 時間差を日数、時分秒ミリ秒で表した時のミリ秒を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 時間差を日数、時分秒ミリ秒で表した時のミリ秒を返す。
//
// EXCEPTIONS
// なし
//

inline
int
ModTimeSpan::getMilliSeconds() const
{
	return this->milliSecond;
}

//
// FUNCTION
// ModTimeSpan::getTotalHours -- 時間で測った時間差を得る
//
// NOTES
// 時間で測った時間差を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 時間で測った時間差を返す。
//
// EXCEPTIONS
// なし
//
inline time_t
ModTimeSpan::getTotalHours() const
{
	return (time_t)(this->second / 60 / 60);
}

//
// FUNCTION
// ModTimeSpan::getTotalMinutes -- 分で測った時間差を得る
//
// NOTES
// 分で測った時間差を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 分で測った時間差を返す。
//
// EXCEPTIONS
// なし
//
inline time_t
ModTimeSpan::getTotalMinutes() const
{
	return (time_t)(this->second / 60);
}

//
// FUNCTION
// ModTimeSpan::getTotalSeconds -- 秒で測った時間差を得る
//
// NOTES
// 秒で測った時間差を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 秒で測った時間差を返す。
//
// EXCEPTIONS
// なし
//
inline time_t
ModTimeSpan::getTotalSeconds() const
{
	return (time_t)(this->second);
}

//
// FUNCTION
// ModTimeSpan::getTotalMilliSeconds -- ミリ秒で測った時間差を得る
//
// NOTES
// ミリ秒で測った時間差を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// ミリ秒で測った時間差を返す。
//
// EXCEPTIONS
// なし
//

inline
time_t
ModTimeSpan::getTotalMilliSeconds() const
{
	return (time_t)((this->second * 1000) + this->milliSecond);
}

//	FUNCTION public
//	ModTimeSpan::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		src
//			自分自身に代入する時間差
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
ModTimeSpan&
ModTimeSpan::operator =(const ModTimeSpan& src)
{
	this->second = src.second;
	this->milliSecond = src.milliSecond;

	return *this;
}


//	FUNCTION public
//	ModTimeSpan::operator + -- + 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		r
//			自分自身へ加える時間差
//
//	RETURN
//		自分自身へ時間差を加算後の時間差
//
//	EXCEPTIONS
//		なし

inline
ModTimeSpan
ModTimeSpan::operator +(const ModTimeSpan& r) const
{
	// 必要であれば、調整する

	return ModTimeSpan(this->second + r.second,
					   this->milliSecond + r.milliSecond).validate();
}

//	FUNCTION public
//	ModTimeSpan::operator - -- - 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		r
//			自分自身から減じる時間差
//
//	RETURN
//		自分自身から時間差を減算後の時間差
//
//	EXCEPTIONS
//		なし

inline
ModTimeSpan
ModTimeSpan::operator -(const ModTimeSpan& r) const
{
	// 必要であれば、調整する

	return ModTimeSpan(this->second - r.second,
					   this->milliSecond - r.milliSecond).validate();
}

//	FUNCTION public
//	ModTimeSpan::operator += -- += 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		src
//			自分自身へ加える時間差
//
//	RETURN
//		時間差を加算後の自分自身
//
//	EXCEPTIONS
//		なし

inline
ModTimeSpan&
ModTimeSpan::operator +=(const ModTimeSpan& src)
{
	this->second += src.second;
	this->milliSecond += src.milliSecond;

	// 必要であれば、調整する

	return this->validate();
}

//	FUNCTION public
//	ModTimeSpan::operator -= -- -= 演算子
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
ModTimeSpan&
ModTimeSpan::operator -=(const ModTimeSpan& src)
{
	this->second -= src.second;
	this->milliSecond -= src.milliSecond;

	// 必要であれば、調整する

	return this->validate();
}

//	FUNCTION public
//	ModTimeSpan::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		r
//			自分自身と比較する時間差
//
//	RETURN
//		ModTrue
//			与えられた時間差は自分自身と等しい
//		ModFalse
//			自分自身より異なる
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTimeSpan::operator ==(const ModTimeSpan& r) const
{
	return (this->second == r.second && this->milliSecond == r.milliSecond) ?
		ModTrue	: ModFalse;
}

//	FUNCTION public
//	ModTimeSpan::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		r
//			自分自身と比較する時間差
//
//	RETURN
//		ModTrue
//			与えられた時間差は自分自身と異なる
//		ModFalse
//			自分自身より等しい
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTimeSpan::operator !=(const ModTimeSpan& r) const
{
	return (*this == r) ? ModFalse : ModTrue;
}

//	FUNCTION public
//	ModTimeSpan::operator < -- < 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		r
//			自分自身と比較する時間差
//
//	RETURN
//		ModTrue
//			与えられた時間差は自分自身より小さい
//		ModFalse
//			自分自身以上である
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTimeSpan::operator <(const ModTimeSpan& r) const
{
	return (this->second < r.second ||
			(this->second == r.second && this->milliSecond < r.milliSecond)) ?
		ModTrue : ModFalse;
}

//	FUNCTION public
//	ModTimeSpan::operator > -- > 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		r
//			自分自身と比較する時間差
//
//	RETURN
//		ModTrue
//			与えられた時間差は自分自身より大きい
//		ModFalse
//			自分自身以下である
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTimeSpan::operator >(const ModTimeSpan& r) const
{
	return (this->second > r.second ||
			(this->second == r.second && this->milliSecond > r.milliSecond)) ?
		ModTrue : ModFalse;
}

//	FUNCTION public
//	ModTimeSpan::operator <= -- <= 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		r
//			自分自身と比較する時間差
//
//	RETURN
//		ModTrue
//			与えられた時間差は自分自身以上である
//		ModFalse
//			自分自身より小さい
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTimeSpan::operator <=(const ModTimeSpan& r) const
{
	return (*this > r) ? ModFalse :	ModTrue;
}

//	FUNCTION public
//	ModTimeSpan::operator >= -- >= 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModTimeSpan&		r
//			自分自身と比較する時間差
//
//	RETURN
//		ModTrue
//			与えられた時間差は自分自身以下である
//		ModFalse
//			自分自身より大きい
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModTimeSpan::operator >=(const ModTimeSpan& r) const
{
	return (*this < r) ? ModFalse : ModTrue;
}

//	FUNCTION public
//	ModTimeSpan::serialize -- 時間差を表すクラスのシリアライザー
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModTimeSpan::serialize(ModArchive& archiver)
{
	if (archiver.isStore()) {
		archiver << this->second;
		archiver << this->milliSecond;
	} else {
		archiver >> this->second;
		archiver >> this->milliSecond;

		// 必要であれば、調整する

		(void) this->validate();
	}
}

//	FUNCTION private
//	ModTimeSpan::validate -- 時間差を表す秒およびミリ秒の値を調整する
//
//	NOTES
//		自分自身に設定された時間差を表す秒及びミリ秒を正しく調整する
//		ミリ秒は (-1000, 1000) の範囲に収められ、
//		秒の符号とミリ秒の符号が同じになるように、秒とミリ秒を計算しなおす
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		調整された自分自身
//
//	EXCEPTIONS
//		なし

inline
ModTimeSpan&
ModTimeSpan::validate()
{
	// ミリ秒を (-1000, 1000) の範囲に収める

	if (this->milliSecond <= -1000) {
		this->second += this->milliSecond / 1000;
		this->milliSecond += (this->milliSecond / 1000) * -1000;
	} else if (this->milliSecond >= 1000) {
		this->second += this->milliSecond / 1000;
		this->milliSecond -= (this->milliSecond / 1000) * 1000;
	}

	// 秒の符号にミリ秒の符号をあわせる

	if (this->second > 0 && this->milliSecond < 0) {
		this->second--;
		this->milliSecond += 1000;
	} else if (this->second < 0 && this->milliSecond > 0) {
		this->second++;
		this->milliSecond -= 1000;
	}

	// 調整後の自分自身を返す

	return *this;
}

#endif	// __ModTimeSpan_H__

//
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
