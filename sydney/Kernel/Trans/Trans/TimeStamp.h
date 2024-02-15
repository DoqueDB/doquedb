// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TimeStamp.h -- タイムスタンプ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_TRANS_TIMESTAMP_H
#define	__SYDNEY_TRANS_TIMESTAMP_H

#include "Trans/Module.h"
#include "Trans/Externalizable.h"

#include "Common/Object.h"

#include "ModTypes.h"

_SYDNEY_BEGIN

namespace Os
{
	class Path;
}

_SYDNEY_TRANS_BEGIN

namespace Log
{
	class File;
	class TimeStampAssignData;
}
namespace Manager
{
	class TimeStamp;
}

//	CLASS
//	Trans::TimeStamp -- タイムスタンプを表すクラス
//
//	NOTES

class TimeStamp
	: public	Common::Object,				//【注意】	最初に継承すること
	  public	Externalizable
{
	friend class Log::File;
	friend class Manager::TimeStamp;
public:
	//	TYPEDEF
	//	Trans::TimeStamp::Value -- タイムスタンプの値を表す型
	//
	//	NOTES

	typedef	ModUInt64		Value;

	// デフォルトコンストラクター
	TimeStamp();
	// コンストラクター
	explicit TimeStamp(Value v);

	// デストラクター
	~TimeStamp();

	// Trans::TimeStamp::Value へのキャスト演算子
	operator				Value() const;

	// = 演算子
	TimeStamp&				operator =(const TimeStamp& v);
	TimeStamp&				operator =(Value v);

	// ++ 前置演算子
	TimeStamp&				operator ++();
#ifdef OBSOLETE
	// ++ 後置演算子
	TimeStamp				operator ++(int dummy);
	// += 演算子
	TimeStamp&				operator +=(Value n);
#endif
	// -- 前置演算子
	TimeStamp&				operator --();
#ifdef OBSOLETE
	// -- 後置演算子
	TimeStamp				operator --(int dummy);
	// -= 演算子
	TimeStamp&				operator -=(Value n);
#endif
	// == 演算子
	bool					operator ==(const TimeStamp& r) const;
	// != 演算子
	bool					operator !=(const TimeStamp& r) const;
#ifdef OBSOLETE
	// >= 演算子
	bool					operator >=(const TimeStamp& r) const;
	// <= 演算子
	bool					operator <=(const TimeStamp& r) const;
#endif
	// > 演算子
	bool					operator >(const TimeStamp& r) const;
	// < 演算子
	bool					operator <(const TimeStamp& r) const;

#ifndef SYD_COVERAGE
	// 文字列で取り出す
	SYD_TRANS_FUNCTION
	virtual ModUnicodeString toString() const;
#endif
	// ハッシュ値を計算する
	virtual ModSize
	hashCode() const;

	// このクラスをシリアル化する
	SYD_TRANS_FUNCTION
	virtual	void			serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	virtual int				getClassID() const;

	// 新しいタイムスタンプを割り当てる
	SYD_TRANS_FUNCTION
	static TimeStamp		assign(bool isNoLog = false);
	// 既存のタイムスタンプを割り当てる
	SYD_TRANS_FUNCTION
	static TimeStamp		assign(const TimeStamp& src, bool isNoLog = false);

	// 障害回復のために REDO する
	SYD_TRANS_FUNCTION
	static void				redo(const Log::TimeStampAssignData& data);

	// 不正なタイムスタンプか
	bool					isIllegal() const;
	static bool				isIllegal(Value v);

	// システムが初期化されたときのタイムスタンプを得る
	SYD_TRANS_FUNCTION
	static const TimeStamp&	getSystemInitialized();
	// 永続化されているタイムスタンプを得る
	SYD_TRANS_FUNCTION
	static TimeStamp		getPersisted();
	// 最後に割り当てられたタイムスタンプを得る
	SYD_TRANS_FUNCTION
	static TimeStamp		getLast();

private:
#ifdef OBSOLETE
	// コンストラクター
	TimeStamp(unsigned int birthNum, unsigned int seqNum);
#endif

	// 値を永続化するファイルを生成する
	static void
	createFile(const Os::Path& path);
	// 値を永続化するファイルをマウントする
	static void
	mountFile(const Os::Path& path, bool readOnly);
	// 値を永続化するファイルを破棄する
	static void
	destroyFile(const Os::Path& path);
	// 値を永続化するファイルをアンマウントする
	static void
	unmountFile(const Os::Path& path, bool readOnly);
	// 値を永続化するファイルの名前を変更する
	static void
	renameFile(const Os::Path& path, const Os::Path& newPath);

	// 値をファイルに永続化する
	void
	store(const Os::Path& path) const;
	// 値をファイルから取り出す
	void
	load(const Os::Path& path);

	// 値
	ModStructuredUInt64		_value;
};

//	FUNCTION public
//	Trans::TimeStamp::TimeStamp --
//		タイムスタンプを表すクラスのデフォルトコンストラクター
//
//	NOTES
//		不正なタイムスタンプを表す
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
TimeStamp::TimeStamp()
{
	_value.full = ~static_cast<Value>(0);
}

//	FUNCTION public
//	Trans::TimeStamp::TimeStamp --
//		タイムスタンプを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp::Value	v
//			タイムスタンプの値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
TimeStamp::TimeStamp(Value v)
{
	_value.full = v;
}

#ifdef OBSOLETE
//	FUNCTION private
//	Trans::TimeStamp::TimeStamp --
//		タイムスタンプを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		birthNum
//			インストールされてからシステムがなん回起動されたか
//		unsigned int		seqNum
//			システムが起動されてからなん回タイムスタンプを取得したか
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
TimeStamp::TimeStamp(unsigned int birthNum, unsigned int seqNum)
{
	_value.halfs.high = birthNum, _value.halfs.low = seqNum;
}
#endif

//	FUNCTION public
//	Trans::TimeStamp::~TimeStamp --
//		タイムスタンプを表すクラスのデストラクター
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
TimeStamp::~TimeStamp()
{}

//	FUNCTION public
//	Trans::TimeStamp::operator Trans::TimeStamp::Value --
//		Trans::TimeStamp::Value へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		タイムスタンプの値
//
//	EXCEPTIONS
//		なし

inline
TimeStamp::operator TimeStamp::Value() const
{
	return _value.full;
}

//	FUNCTION public
//	Trans::TimeStamp::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	v
//			自分自身に代入するタイムスタンプ
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
TimeStamp&
TimeStamp::operator =(const TimeStamp& v)
{
	_value.full = v._value.full;
	return *this;
}

//	FUNCTION public
//	Trans::TimeStamp::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp::Value	v
//			自分自身に代入するタイムスタンプの値
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
TimeStamp&
TimeStamp::operator =(Value v)
{
	_value.full = v;
	return *this;
}

//	FUNCTION public
//	Trans::TimeStamp::operator ++ -- ++ 前置演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		直後のタイムスタンプを表す自分自身
//
//	EXCEPTIONS
//		なし

inline
TimeStamp&
TimeStamp::operator ++()
{
	++_value.full;
	return *this;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Trans::TimeStamp::operator ++ -- ++ 後置演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		直後のタイムスタンプを表す前の自分自身を複製したもの
//
//	EXCEPTIONS
//		なし

inline
TimeStamp
TimeStamp::operator ++(int dummy)
{
	TimeStamp	tmp(*this);
	++*this;
	return tmp;
}

//	FUNCTION public
//	Trans::TimeStamp::operator += -- += 演算子
//
//	NOTES
//		加算する値が負数の場合、すべて正数にキャストされる
//
//	ARGUMENTS
//		Trans::TimeStamp::Value	n
//			自分自身に加算する値
//
//	RETURN
//		加算後の自分自身
//
//	EXCEPTIONS
//		なし

inline
TimeStamp&
TimeStamp::operator +=(Value n)
{
	_value.full += n;
	return *this;
}
#endif

//	FUNCTION public
//	Trans::TimeStamp::operator -- -- -- 前置演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		直前のタイムスタンプを表す自分自身
//
//	EXCEPTIONS
//		なし

inline
TimeStamp&
TimeStamp::operator --()
{
	--_value.full;
	return *this;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Trans::TimeStamp::operator -- -- -- 後置演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		直前のタイムスタンプを表す前の自分自身を複製したもの
//
//	EXCEPTIONS
//		なし

inline
TimeStamp
TimeStamp::operator --(int dummy)
{
	TimeStamp	tmp(*this);
	--*this;
	return tmp;
}

//	FUNCTION public
//	Trans::TimeStamp::operator -= -- -= 演算子
//
//	NOTES
//		減算する値が負数の場合、すべて正数にキャストされる
//
//	ARGUMENTS
//		Trans::TimeStamp::Value	n
//			自分自身に減算する値
//
//	RETURN
//		減算後の自分自身
//
//	EXCEPTIONS
//		なし

inline
TimeStamp&
TimeStamp::operator -=(Value n)
{
	_value.full -= n;
	return *this;
}
#endif

//	FUNCTION public
//	Trans::TimeStamp::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	r
//			自分自身と比較するタイムスタンプ
//
//	RETURN
//		true
//			自分自身と等しい
//		false
//			等しくない
//
//	EXCEPTIONS
//		なし

inline
bool
TimeStamp::operator ==(const TimeStamp& r) const
{
	return _value.full == r._value.full;
}

//	FUNCTION public
//	Trans::TimeStamp::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	r
//			自分自身と比較するタイムスタンプ
//
//	RETURN
//		true
//			自分自身と等しくない
//		false
//			等しい
//
//	EXCEPTIONS
//		なし

inline
bool
TimeStamp::operator !=(const TimeStamp& r) const
{
	return _value.full != r._value.full;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Trans::TimeStamp::operator >= -- >= 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	r
//			自分自身と比較するタイムスタンプ
//
//	RETURN
//		true
//			自分自身は以上である
//		false
//			小さい
//
//	EXCEPTIONS
//		なし

inline
bool
TimeStamp::operator >=(const TimeStamp& r) const
{
	return !isIllegal() && !r.isIllegal() && _value.full >= r._value.full;
}

//	FUNCTION public
//	Trans::TimeStamp::operator <= -- <= 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	r
//			自分自身と比較するタイムスタンプ
//
//	RETURN
//		true
//			自分自身は以下である
//		false
//			大きい
//
//	EXCEPTIONS
//		なし

inline
bool
TimeStamp::operator <=(const TimeStamp& r) const
{
	return !isIllegal() && !r.isIllegal() && _value.full <= r._value.full;
}
#endif

//	FUNCTION public
//	Trans::TimeStamp::operator > -- > 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	r
//			自分自身と比較するタイムスタンプ
//
//	RETURN
//		true
//			自分自身は大きい
//		false
//			以下である
//
//	EXCEPTIONS
//		なし

inline
bool
TimeStamp::operator >(const TimeStamp& r) const
{
	return !isIllegal() && !r.isIllegal() && _value.full > r._value.full;
}

//	FUNCTION public
//	Trans::TimeStamp::operator < -- < 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	r
//			自分自身と比較するタイムスタンプ
//
//	RETURN
//		true
//			自分自身は小さい
//		false
//			以上である
//
//	EXCEPTIONS
//		なし

inline
bool
TimeStamp::operator <(const TimeStamp& r) const
{
	return !isIllegal() && !r.isIllegal() && _value.full < r._value.full;
}

//	FUNCTION public
//	Trans::TimeStamp::hashCode -- ハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

inline
ModSize
TimeStamp::hashCode() const
{
	return _value.halfs.low;
}

//	FUNCTION public
//	Trans::TimeStamp::getClassID -- このクラスのクラス ID を得る
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
int
TimeStamp::getClassID() const
{
	return Externalizable::Category::TimeStamp +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::TimeStamp::isIllegal -- 不正なタイムスタンプか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			不正である
//		false
//			不正でない
//
//	EXCEPTIONS
//		なし

inline
bool
TimeStamp::isIllegal() const
{
	return isIllegal(_value.full);
}

//	FUNCTION public
//	Trans::TimeStamp::isIllegal -- 不正なタイムスタンプ値か
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp::Value	v
//			不正か調べるタイムスタンプ値
//
//	RETURN
//		true
//			不正である
//		false
//			不正でない
//
//	EXCEPTIONS
//		なし

// static
inline
bool
TimeStamp::isIllegal(Value v)
{
	return v == ~static_cast<Value>(0);
}

//	CONST
//	Trans::IllegalTimeStamp -- 不正なタイムスタンプ
//
//	NOTES

const TimeStamp				IllegalTimeStamp;

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_TIMESTAMP_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
