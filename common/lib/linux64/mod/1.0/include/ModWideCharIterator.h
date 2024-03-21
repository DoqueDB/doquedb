// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModWideCharIterator.h -- ModWideCharIterator のクラス定義
// 
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModWideCharIterator_H__
#define __ModWideCharIterator_H__

#include "ModConfig.h"
#include "ModCommon.h"
#include "ModWideChar.h"
#include "ModCharTrait.h"
#include "ModWideCharTrait.h"
#include "ModKanjiCode.h"

//
// TEMPLATE CLASS
// ModWideCharIterator -- マルチバイト文字列を文字単位で走査するイテレータ
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// このクラスは char や ModWideChar の配列で表されたマルチバイト文字列を
// 文字単位に走査するために用いる。
// テンプレートクラスにしたのは ModWideChar* を表すものと区別するためと、
// char* に対しても使えることを明示するためである。
// おもに char* に対して使うことを意図している。
// これはフォワードイテレータなのでランダムアクセスが必要な場合は
// ModString を使う必要がある。
// 

// ModPureWideCharIteratorに変更して、サブクラスとしてメモリハンドルクラスを
// 明示したクラスを作成するには、operator=()の再定義など
// への対応が必要なことがわかったので断念し、
// 直接、ModDefaultObjectのサブクラスとして作成する。
template <class CharType>
class ModWideCharIterator : public ModDefaultObject {
public:
	// デフォルトコンストラクタ
	ModWideCharIterator();
	// コンストラクタ
	ModWideCharIterator(const CharType* pointer_);
	// コピーコンストラクタ
	ModWideCharIterator(const ModWideCharIterator<CharType>& original);
	// デストラクタ
	~ModWideCharIterator();

	// 代入オペレータ
	ModWideCharIterator<CharType>& operator=(const CharType* value);
	ModWideCharIterator<CharType>& operator=(const ModWideCharIterator<CharType>& original);

	//
	// イテレータを使ったサーチや比較関数
	//
	// 与えられた文字や文字列の場所に移動する
	ModWideCharIterator<CharType>& seek(const CharType character);
	ModWideCharIterator<CharType>& seek(const CharType* string);

	// 文字列の終了を判定する
	ModBoolean isEnd() const;

	//
	// --- 実装で明示的なインスタンス化を行なっているものは後に書く ---
	//
	// 値を得るオペレータ
	ModWideChar operator*() const;

	// ++オペレータ
	ModWideCharIterator<CharType>& operator++();

	// 単純な変換オペレータ
	operator const CharType*() const;

private:
	CharType* pointer;
};

//
// TEMPLATE FUNCTION
// ModWideCharIterator::ModWideCharIterator -- デフォルトコンストラクタ
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// ModWideCharIterator のデフォルトコンストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class CharType>
inline
ModWideCharIterator<CharType>::ModWideCharIterator()
	:pointer(0)
{
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::ModWideCharIterator -- 初期値を指定したコンストラクタ
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// ModWideCharIterator の初期値の文字列を指定したコンストラクタ
//
// ARGUMENTS
// const CharType* pointer_
//		初期値に使う文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class CharType>
inline
ModWideCharIterator<CharType>::ModWideCharIterator(const CharType* pointer_)
{
	this->pointer = (CharType*)pointer_;
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::ModWideCharIterator -- コピーコンストラクタ
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// ModWideCharIterator のコピーコンストラクタ
//
// ARGUMENTS
// const ModWideCharIterator<CharType>& original
//		コピー元のデータへの参照
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class CharType>
inline
ModWideCharIterator<CharType>::ModWideCharIterator(const ModWideCharIterator<CharType>& original)
{
	this->pointer = original.pointer;
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::~ModWideCharIterator -- デストラクタ
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// ModWideCharIterator のデストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class CharType>
inline
ModWideCharIterator<CharType>::~ModWideCharIterator()
{
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::operator= -- 代入オペレータ
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// このオペレータは CharType へのポインタを ModWideCharIterator に
// 代入するために用いる。
//
// ARGUMENTS
// const CharType* pointer_
//
// RETURN
// pointer_ の指す位置を指すように変更し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//
template <class CharType>
inline ModWideCharIterator<CharType>&
ModWideCharIterator<CharType>::operator=(const CharType* pointer_)
{
	this->pointer = (CharType*)pointer_;
	return *this;
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::operator= -- 代入オペレータ
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// このオペレータは他の ModWideCharIterator 型の値を代入するために用いる。
//
// ARGUMENTS
// const ModWideCharIterator<CharType>& original
//
// RETURN
// original と同じ位置を指すように変更し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//
template <class CharType>
inline ModWideCharIterator<CharType>&
ModWideCharIterator<CharType>::operator=(const ModWideCharIterator<CharType>& original)
{
	this->pointer = original.pointer;
	return *this;
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::seek -- 配列上の文字の位置を探索する
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// この関数は ModWideCharIterator が指す文字列の中から
// 指定した文字を探し、その位置を指すように自身を変更するのに用いる。
//
// ARGUMENTS
// const CharType character
//		探索対象の文字
//
// RETURN
// 見つかった場合はその文字を指すように変更し、自身への参照を返す。
// 見つからなかった場合は文字列の終端を指すように変更し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//
template <class CharType>
inline ModWideCharIterator<CharType>&
ModWideCharIterator<CharType>::seek(const CharType character)
{
	for (; *(this->pointer) != character && *(this->pointer) != 0;
		 this->pointer++);
	return *this;
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::seek -- 配列上の文字列の位置を探索する
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// この関数は ModWideCharIterator が指す文字列の中から CharType の
// 配列で与えられる文字列と一致する部分を探索するのに用いる。
//
// ARGUMENTS
// const CharType* string
//		探索する文字列へのポインタ(最後は 0 でなければならない)
//
// RETURN
// 見つかった場合はその先頭の文字を指すように変更し、自身への参照を返す。
// 見つからなかった場合は文字列の終端を指すように変更し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//
template <class CharType>
inline ModWideCharIterator<CharType>&
ModWideCharIterator<CharType>::seek(const CharType* string)
{
	CharType* cp1;
	const CharType* cp2;
	CharType* cp3;

	while (1) {
		for (; *(this->pointer) != *string && *(this->pointer) != 0;
			 this->pointer++);
		if (*(this->pointer) ==  0) {
			break;
		}
		for (cp1 = this->pointer + 1, cp2 = string + 1, cp3 = 0;
			 *cp1 == *cp2 && *cp2 != 0;
			 cp1++, cp2++) {
			if (cp3 == 0 && *cp1 == *string) {
				cp3 = cp1;
			}
		}
		if (*cp2 ==  0) {
			break;
		}
		if (cp3 == 0) {
			this->pointer = cp1;
		} else {
			this->pointer = cp3;
		}
	}
	return *this;
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::isEnd -- 文字列の終了位置を指しているかを判定する
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// この関数はイテレータが文字列の終了位置を指しているかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// イテレータが Mod[Wide]CharTrait::null() を指している場合は ModTrue を返し、
// 指していない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
template <class CharType>
inline ModBoolean
ModWideCharIterator<CharType>::isEnd() const
{
	return (this->pointer == 0 || *(this->pointer) == (CharType)0)
		?ModTrue:ModFalse;
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::operator* -- *オペレータ
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// このオペレータは現在 ModWideCharIterator の指している位置の文字を
// ModWideChar として得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 現在指している文字を ModWideChar に変換したものを返す。
//
// EXCEPTIONS
// なし
//

// char と ModWideChar で定義が違う。コメントは共通にしておいた。
ModTemplateNull
inline
ModWideChar
ModWideCharIterator<char>::operator*() const
{
	return ModWideCharTrait::makeWideChar(this->pointer, ModKanjiCode::euc);
}

ModTemplateNull
inline
ModWideChar
ModWideCharIterator<ModWideChar>::operator*() const
{
	return *(this->pointer);
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::operator++ -- ++オペレータ
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// この関数は ModWideCharIterator が次の文字を指すように変更するために用いる。
// char 配列を対象にしている場合、マルチバイト文字はそのバイト数分進められる。
//
// ARGUMENTS
// なし
//
// RETURN
// 次の文字を指すように変更し、自身への参照を返す。
//
// EXCEPTIONS
// なし
//

// char と ModWideChar で定義は別だがコメントは共通にしておいた
ModTemplateNull
inline
ModWideCharIterator<char>&
ModWideCharIterator<char>::operator++()
{
	this->pointer += ModWideCharTrait::byteSize(this->pointer,
												ModKanjiCode::euc);
	return *this;
}

ModTemplateNull
inline
ModWideCharIterator<ModWideChar>&
ModWideCharIterator<ModWideChar>::operator++()
{
	this->pointer++;
	return *this;
}

//
// TEMPLATE FUNCTION
// ModWideCharIterator::operator const CharType* -- const CharType* へのキャスト
//
// TEMPLATE ARGUMENTS
// class CharType
//		走査する対象となる配列を構成する型。通常は char が用いられる。
//
// NOTES
// このオペレータは ModWideCharIterator の指す配列を
// const CharType* にキャストするのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 現在指しているポインタを const CharType* にキャストして返す。
//
// EXCEPTIONS
// なし
//

// char と ModWideChar で定義は別だがコメントは共通にしておいた
ModTemplateNull
inline
ModWideCharIterator<char>::operator const char*() const
{
	return (const char*)this->pointer;
}

ModTemplateNull
inline
ModWideCharIterator<ModWideChar>::operator const ModWideChar*() const
{
	return (const ModWideChar*)this->pointer;
}

#endif	// __ModWideCharIterator_H__

//
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
