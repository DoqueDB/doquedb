// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModTime.cpp -- 時刻関連のメソッドの定義
// 
// Copyright (c) 1998, 1999, 2023 Ricoh Company, Ltd.
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


#include "ModConfig.h"

extern "C" {
#if MOD_CONF_FUNC_GETTIMEOFDAY == 1
#include <sys/time.h>
#endif
#if MOD_CONF_FUNC_GETTIMEOFDAY == 2
#include <sys/timeb.h>
#endif
#include <time.h>
}

#include "ModTime.h"

//	FUNCTION public
//	ModTime::getCurrentTime -- 現在時刻を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求められた現在時刻
//
//	EXCEPTIONS
//		なし

// static
ModTime
ModTime::getCurrentTime()
{
#if MOD_CONF_FUNC_GETTIMEOFDAY == 0
	ModThrow(ModModuleStandard,
			 ModCommonErrorNotSupported, ModErrorLevelError);
	return ModTime();
#else
#if MOD_CONF_FUNC_GETTIMEOFDAY == 1
	struct timeval	tv;
	::gettimeofday(&tv, 0);
	ModTime	t(tv.tv_sec, tv.tv_usec / 1000);
#endif
#if MOD_CONF_FUNC_GETTIMEOFDAY == 2
	struct _timeb	tb;
	::_ftime(&tb);
	ModTime	t(tb.time, tb.millitm);
#endif
	// 必要ならばミリ秒を補正する

	t.validate();

	return t;
#endif
}

//	FUNCTION public
//	ModTime::getString -- 時刻を現地時刻に変換し、文字列の形式にする
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			included
//			ModTrue または指定されないとき
//				文字列表現にミリ秒を含める
//			ModFalse
//				文字列表現にミリ秒を含めない
//
//	RETURN
//		時刻を変換して得られた文字列
//
//	EXCEPTIONS

ModCharString
ModTime::getString(ModBoolean included) const
{
	int	year, month, day, hour, minute, second;
	ModTime::divideTime(&year, &month, &day, &hour, &minute, &second, _clock);

	ModCharString s;
	if (included == ModTrue)
		s.format("%4d/%02d/%02d-%02d:%02d:%02d +%03d(ms)",
				 year, month, day, hour, minute, second, _milliSecond);
	else
		s.format("%4d/%02d/%02d-%02d:%02d:%02d",
				 year, month, day, hour, minute, second);

	return s;
}

//	FUNCTION public
//	ModTime::makeTime --
//		現地時刻の年月日時分秒から 1970/1/1-00:00:00 からの経過秒を得る
//
//	NOTES
//
//	ARGUMENTS
//		int					year
//			生成する経過秒の西暦年
//		int					month
//			生成する経過秒の月[1, 12]
//		int					day
//			生成する経過秒の日[1, 31]
//		int					hour
//			指定されたとき
//				生成する経過秒の時[0, 23]
//			指定されないとき
//				0 が指定されたものとみなす
//		int					minute
//			指定されたとき
//				生成する経過秒の分[0, 59]
//			指定されないとき
//				0 が指定されたものとみなす
//		int					second
//			指定されたとき
//				生成する経過秒の秒[0, 59]
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		得られた 1970/1/1-00:00:00 からの経過秒
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			不正な年月日時分秒が与えられている

// static
ModTimeT
ModTime::makeTime(int year, int month, int day,
				  int hour, int minute, int second)
{
	struct tm	tm;
	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = minute;
	tm.tm_sec = second;
	tm.tm_isdst = 0;

	ModTimeT	clock = (ModTimeT) ::mktime(&tm);
	if (clock == -1) {

		// 与えられた引数が不正だった

		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	return clock;
}

//	FUNCTION public
//	ModTime::divideTime --
//		1970/1/1-00:00:00 からの経過秒を現地時刻の年月日時分秒に分解する
//
//	NOTES
//
//	ARGUMENTS
//		int*				pYear
//			0 以外の値
//				求めた年を格納する領域の先頭アドレス
//			0
//				年は求めない
//		int*				pMonth
//			0 以外の値
//				求めた月を格納する領域の先頭アドレス
//			0
//				月は求めない
//		int*				pDay
//			0 以外の値
//				求めた日を格納する領域の先頭アドレス
//			0
//				日は求めない
//		int*				pHour
//			0 以外の値
//				求めた時を格納する領域の先頭アドレス
//			0
//				時は求めない
//		int*				pMinute
//			0 以外の値
//				求めた分を格納する領域の先頭アドレス
//			0
//				分は求めない
//		int*				pSecond
//			0 以外の値
//				求めた秒を格納する領域の先頭アドレス
//			0
//				秒は求めない
//		ModTimeT			clock
//			分解する 1970/1/1-00:00:00 からの経過秒
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModTime::divideTime(int* pYear, int* pMonth, int* pDay,
					int* pHour, int* pMinute, int* pSecond,	ModTimeT clock)
{
	struct tm	tm;
	struct tm*	tp;
#if defined(MOD_NO_THREAD) || MOD_CONF_LIB_POSIX == 0

	// NT の ::localtime は MT 安全である
	// スレッド固有ストレージ(TLS) 上に得られた時刻が返される

	if ((tp = ::localtime(&clock)) == 0) {

		// 与えた時刻が 1970/1/1-00:00:00 以前だったとき、
		// 1970/1/1-00:00:00 を返すように設定しておく

		tp = &tm;
		tm.tm_year = 70;
		tm.tm_mon = 0;
		tm.tm_mday = 1;
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
	}
#else
	// POSIX では、::localtime は MT 安全でない
	// そのかわり ::localtime_r が用意されている

	(void) ::localtime_r(&clock, tp = &tm);
#endif
	if (pYear)
		*pYear = tp->tm_year + 1900;
	if (pMonth)
		*pMonth = tp->tm_mon + 1;
	if (pDay)
		*pDay = tp->tm_mday;
	if (pHour)
		*pHour = tp->tm_hour;
	if (pMinute)
		*pMinute = tp->tm_min;
	if (pSecond)
		*pSecond = tp->tm_sec;
}

//	FUNCTION public
//	ModTime::serialize -- 時刻を表すクラスののシリアライザー
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

void
ModTime::serialize(ModArchive& archiver)
{
	if (archiver.isStore()) {
		archiver << _clock;
		archiver << _milliSecond;
	} else {
		archiver >> _clock;
		archiver >> _milliSecond;

		// 必要ならばミリ秒を補正する

		this->validate();
	}
}

//	FUNCTION private
//	ModTime::validate -- 経過秒およびミリ秒の値を調整する
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
ModTime::validate()
{
	// ミリ秒を [0, 1000) の範囲に収める

	if (_milliSecond < 0) {
		_clock += _milliSecond / 1000 - 1;
		_milliSecond += (_milliSecond / 1000 - 1) * -1000;
	}
	if (_milliSecond >= 1000) {
		_clock += _milliSecond / 1000;
		_milliSecond -= (_milliSecond / 1000) * 1000;
	}
}

//
// Copyright (c) 1998, 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
