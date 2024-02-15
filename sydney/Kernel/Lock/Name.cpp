// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Name.cpp -- ロック名関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Lock";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Lock/Name.h"

_SYDNEY_USING
_SYDNEY_LOCK_USING

namespace
{
}

//	FUNCTION public
//	Lock::Name::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name&			r
//			自分自身と比較するロック名
//
//	RETURN
//		true
//			与えられたロック名は自分自身と等しい
//		false
//			等しくない
//
//	EXCEPTIONS
//		なし

bool
Name::operator ==(const Name& r) const
{
	return getCategory() == r.getCategory() &&
		_buf[2] == r._buf[2] &&	_buf[1] == r._buf[1] && _buf[0] == r._buf[0];
}

//	FUNCTION public
//	Lock::Name::getParent -- 自分自身の表すオブジェクトの親の名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた親の名前
//
//	EXCEPTIONS
//		なし

const Name
Name::getParent() const
{
	//【注意】	汚いが、型フィールドとしてメンバー _category を
	//			定義しているのは、この関数を提供するためである

	switch (getCategory()) {
	case Category::Table:
	case Category::LogicalLog:
		return DatabaseName(_buf[0]);
	case Category::Tuple:
	case Category::File:
		return TableName(_buf[0], _buf[1]);
	}
	return Name();
}

//	FUNCTION public
//	Lock::Name::isDescendant --
//		自分自身の表すオブジェクトが与えられた名前の表す
//		オブジェクトの子孫であるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name&			r
//			自分自身が子孫であるか調べるオブジェクトの名前
//
//	RETURN
//		true
//			自分自身は与えられた名前の表すオブジェクトの子孫である
//		false
//			子孫でない
//
//	EXCEPTIONS
//		なし

bool
Name::isDescendant(const Name& r) const
{
	//【注意】	汚いが、型フィールドとしてメンバー _category を
	//			定義しているのは、この関数を提供するためである

	switch (r.getCategory()) {
	case Category::Database:
		switch (getCategory()) {
		case Category::Table:
		case Category::Tuple:
		case Category::File:
		case Category::LogicalLog:
			return r._buf[0] == _buf[0];
		}
		break;
	case Category::Table:
		switch (getCategory()) {
		case Category::Tuple:
		case Category::File:
			return r._buf[0] == _buf[0] && r._buf[1] == _buf[1];
		}
		break;
	}
	return false;
}

//	FUNCTION public
//	Lock::Name::getValue -- ロック名の値を得る
//
//	NOTES
//		ロック名の値とは名前から生成したハッシュ値
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた値
//
//	EXCEPTIONS
//		なし

Name::Value
Name::getValue() const
{
	//【注意】	汚いが、型フィールドとしてメンバー _category を
	//			定義しているのは、この関数を提供するためである

	switch (getCategory()) {
	case Category::Database:

		// db:        XXXXXXXX
		// return:    ########

		return static_cast<Value>(_buf[0]);

	case Category::Table:

		// db:      XXXXXXXX
		// table:     YYYYYYYY
		// return:    ########

		return (static_cast<Value>(_buf[0]) << 8) + static_cast<Value>(_buf[1]);

	case Category::Tuple:

		// db:   XXXXXXXX
		// table: YYYYYYYY
		// tuple:     ZZZZZZZZ
		// return:    ########

		return (((static_cast<Value>(_buf[0]) << 4) +
				 static_cast<Value>(_buf[1])) << 16) +
			static_cast<Value>(_buf[2]);

	case Category::LogicalLog:

		// db:     XXXXXXXX
		// return:    ########

		return static_cast<Value>(_buf[0]) << 12;

	case Category::File:

		// db:    XXXXXXXX
		// table:   YYYYYYYY
		// file:      ZZZZZZZZ
		// return:    ########

		return (((static_cast<Value>(_buf[0]) << 8) +
				 static_cast<Value>(_buf[1])) << 8) +
			static_cast<Value>(_buf[2]);
	}
	return 0;
}

//	FUNCTION private
//	Lock::Name::construct -- ロック名を表すクラスのコンストラクター下位関数
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Category	category
//			生成するロック名の種別
//		Lock::Name::Part*	parts
//			生成するロック名の名前を格納したバッファ領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Name::construct(Category::Value	category, const Part* parts)
{
	_category = category;
	if (parts) {
		_buf[0] = *(parts + 0);
		_buf[1] = *(parts + 1);
		_buf[2] = *(parts + 2);
	} else
		_buf[0] =
		_buf[1] =
		_buf[2] = 0;
}

//	FUNCTION private
//	Lock::Name::construct -- ロック名を表すクラスのコンストラクター下位関数
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Category	category
//			生成するロック名の種別
//		Lock::Name::Part	part0
//			生成するロック名の先頭から0番目の部分
//		Lock::Name::Part	part1
//			生成するロック名の先頭から1番目の部分
//		Lock::Name::Part	part2
//			生成するロック名の先頭から2番目の部分
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Name::construct(Category::Value category, Part part0, Part part1, Part part2)
{
	_category = category;
	_buf[0] = part0;
	_buf[1] = part1;
	_buf[2] = part2;
}

//
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
