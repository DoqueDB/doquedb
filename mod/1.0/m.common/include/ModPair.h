// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModPair.h -- ModPair のクラス定義
// 
// Copyright (c) 1997, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModPair_H__
#define __ModPair_H__

//
// TEMPLATE STRUCT
// ModPair -- ペアを表現するテンプレート構造体
//
// TEMPLATE ARGUMENTS
// class FirstType
//		ペアの第一要素のクラス名
// class SecondType
//		ペアの第二要素のクラス名
//
// NOTES
// STLに代わり、ペアを表現するテンプレート構造体である。
// 要望に寄せられる最低限の動作だけを提供している。
//
template <class FirstType, class SecondType >
struct ModPair {
    FirstType first;
    SecondType second;
	// デフォルトコンストラクタ
    ModPair();
	// コンストラクタ(1)
    ModPair(const FirstType& first_, const SecondType& second_);
	// コピーコンストラクタ
    ModPair(const ModPair<FirstType, SecondType>& original);
	// デストラクタ
	~ModPair();
	// 代入オペレータ
	ModPair<FirstType, SecondType >&
	operator=(const ModPair<FirstType, SecondType >& original);
};

//
// TEMPLATE FUNCTION
// ModPair::ModPair -- デフォルトコンストラクタ
//
// TEMPLATE ARGUMENTS
// class FirstType
//		ModPair の第一要素のクラス名
// class SecondType
//		ModPair の第二要素のクラス名
//
// NOTES
// ModPair のデフォルトコンストラクタ
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
template <class FirstType, class SecondType>
inline
ModPair<FirstType, SecondType>::ModPair()
//【注意】	以下の行を有効にすると、FirstType や SecondType に
//			入れ子クラスへのポインターがきたときに VC++ 5.0 で
//			C2440 のエラーになりコンパイルできない
//	: first(FirstType()), second(SecondType())
{
}

//
// TEMPLATE FUNCTION
// ModPair::ModPair -- 要素を指定したコンストラクタ
//
// TEMPLATE ARGUMENTS
// class FirstType
//		ModPair の第一要素のクラス名
// class SecondType
//		ModPair の第二要素のクラス名
//
// NOTES
// ModPair の要素を指定したコンストラクタ
//
// ARGUMENTS
// const FirstType& first_
//		第一要素に代入する値
// const SecondType& second_
//		第二要素に代入する値
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class FirstType, class SecondType>
inline
ModPair<FirstType, SecondType>::ModPair(const FirstType& first_,
										const SecondType& second_) :
	first(first_), second(second_)
{
}

//
// TEMPLATE FUNCTION
// ModPair::ModPair -- コピーコンストラクタ
//
// TEMPLATE ARGUMENTS
// class FirstType
//		ModPair の第一要素のクラス名
// class SecondType
//		ModPair の第二要素のクラス名
//
// NOTES
// ModPair のコピーコンストラクタ
//
// ARGUMENTS
// const ModPair<FirstType, SecondType>& original
//		コピーされる元のデータへの参照
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class FirstType, class SecondType>
inline
ModPair<FirstType, SecondType>::ModPair(const ModPair<FirstType, SecondType>& original) :
	first(original.first), second(original.second)
{
}

//
// TEMPLATE FUNCTION
// ModPair::ModPair -- デストラクタ
//
// TEMPLATE ARGUMENTS
// class FirstType
//		ModPair の第一要素のクラス名
// class SecondType
//		ModPair の第二要素のクラス名
//
// NOTES
// ModPair のデストラクタ
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
template <class FirstType, class SecondType>
inline
ModPair<FirstType, SecondType>::~ModPair()
{
}

//
// TEMPLATE FUNCTION
// ModPair::operator= -- 代入オペレータ
//
// TEMPLATE ARGUMENTS
// class FirstType
//		ModPair の第一要素のクラス名
// class SecondType
//		ModPair の第二要素のクラス名
//
// NOTES
// このオペレータは ModPair の代入を行なうために用いる。
//
// ARGUMENTS
// const ModPair<FirstType, SecondType>& original
//		代入元のデータへの参照
//
// RETURN
// 代入した後、自身への参照を返す。
//
// EXCEPTIONS
// なし
//
template <class FirstType, class SecondType >
inline ModPair<FirstType, SecondType >&
ModPair<FirstType, SecondType >::operator=(const ModPair<FirstType, SecondType >& original)
{
	this->first = original.first;
	this->second = original.second;

	return *this;
}

//	TEMPLATE FUNCTION
//	operator == -- 2 つの組の間の == 演算子
//
//	TEMPLATE ARGUMENTS
//		class FirstType
//			組の第 1 要素の型
//		class SecondType
//			組の第 2 要素の型
//
//	NOTES
//
//	ARGUMENTS
//		ModPair<FirstType, SecondType >&	l
//			r と比較する組
//		ModPair<FirstType, SecondType >&	r
//			l と比較する組
//
//	RETURN
//		ModTrue
//			l と r が等しい
//		ModFalse
//			l と r は等しくない
//
//	EXCEPTIONS

template <class FirstType, class SecondType >
inline
ModBoolean
operator ==(const ModPair<FirstType, SecondType >& l,
			const ModPair<FirstType, SecondType >& r)
{
	return (l.first == r.first && l.second == r.second) ? ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION
//	operator != -- 2 つの組の間の != 演算子
//
//	TEMPLATE ARGUMENTS
//		class FirstType
//			組の第 1 要素の型
//		class SecondType
//			組の第 2 要素の型
//
//	NOTES
//
//	ARGUMENTS
//		ModPair<FirstType, SecondType >&	l
//			r と比較する組
//		ModPair<FirstType, SecondType >&	r
//			l と比較する組
//
//	RETURN
//		ModTrue
//			l と r は等しくない
//		ModFalse
//			l と r が等しい
//
//	EXCEPTIONS

template <class FirstType, class SecondType >
inline
ModBoolean
operator !=(const ModPair<FirstType, SecondType >& l,
			const ModPair<FirstType, SecondType >& r)
{
	return operator ==(l, r) ? ModFalse : ModTrue;
}

//	TEMPLATE FUNCTION
//	operator < -- 2 つの組の間の < 演算子
//
//	TEMPLATE ARGUMENTS
//		class FirstType
//			組の第 1 要素の型
//		class SecondType
//			組の第 2 要素の型
//
//	NOTES
//
//	ARGUMENTS
//		ModPair<FirstType, SecondType >&	l
//			r と比較する組
//		ModPair<FirstType, SecondType >&	r
//			l と比較する組
//
//	RETURN
//		ModTrue
//			l のほうが r より小さい
//		ModFalse
//			l のほうが r より小さくない
//
//	EXCEPTIONS

template <class FirstType, class SecondType >
inline
ModBoolean
operator <(const ModPair<FirstType, SecondType >& l,
		   const ModPair<FirstType, SecondType >& r)
{ 
	return (l.first < r.first || (l.first == r.first && l.second < r.second)) ?
		ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION
//	operator > -- 2 つの組の間の > 演算子
//
//	TEMPLATE ARGUMENTS
//		class FirstType
//			組の第 1 要素の型
//		class SecondType
//			組の第 2 要素の型
//
//	NOTES
//
//	ARGUMENTS
//		ModPair<FirstType, SecondType >&	l
//			r と比較する組
//		ModPair<FirstType, SecondType >&	r
//			l と比較する組
//
//	RETURN
//		ModTrue
//			l のほうが r より大きい
//		ModFalse
//			l のほうが r より大きくない
//
//	EXCEPTIONS

template <class FirstType, class SecondType >
inline
ModBoolean
operator >(const ModPair<FirstType, SecondType >& l,
		   const ModPair<FirstType, SecondType >& r)
{
	return operator <(r, l);
}

//	TEMPLATE FUNCTION
//	operator <= -- 2 つの組の間の <= 演算子
//
//	TEMPLATE ARGUMENTS
//		class FirstType
//			組の第 1 要素の型
//		class SecondType
//			組の第 2 要素の型
//
//	NOTES
//
//	ARGUMENTS
//		ModPair<FirstType, SecondType >&	l
//			r と比較する組
//		ModPair<FirstType, SecondType >&	r
//			l と比較する組
//
//	RETURN
//		ModTrue
//			l は r 以下である
//		ModFalse
//			l は r 以下でない
//
//	EXCEPTIONS

template <class FirstType, class SecondType >
inline
ModBoolean
operator <=(const ModPair<FirstType, SecondType >& l,
			const ModPair<FirstType, SecondType >& r)
{
	return operator >(l, r) ? ModFalse : ModTrue;
}

//	TEMPLATE FUNCTION
//	operator >= -- 2 つの組の間の >= 演算子
//
//	TEMPLATE ARGUMENTS
//		class FirstType
//			組の第 1 要素の型
//		class SecondType
//			組の第 2 要素の型
//
//	NOTES
//
//	ARGUMENTS
//		ModPair<FirstType, SecondType >&	l
//			r と比較する組
//		ModPair<FirstType, SecondType >&	r
//			l と比較する組
//
//	RETURN
//		ModTrue
//			l は r 以上である
//		ModFalse
//			l は r 以上でない
//
//	EXCEPTIONS

template <class FirstType, class SecondType >
inline
ModBoolean
operator >=(const ModPair<FirstType, SecondType >& l,
			const ModPair<FirstType, SecondType >& r)
{
	return operator <(l, r) ? ModFalse : ModTrue;
}

//
// TEMPLATE FUNCTION
// ModPair::ModMakePair -- ペアを作る
//
// TEMPLATE ARGUMENTS
// class FirstType
//		ModPair の第一要素のクラス名
// class SecondType
//		ModPair の第二要素のクラス名
//
// NOTES
// この関数は任意のクラスを用いたペアを、クラス名を明示せずに作成するために
// 用いる。
//
// ARGUMENTS
// const FirstType first_
//		作成されるペアの第一要素に代入される値
// const SecondType second_
//		作成されるペアの第二要素に代入される値
//
// RETURN
// 第一要素に first_、第二要素に second_が代入された ModPair を返す。
//
// EXCEPTIONS
// なし
//
template <class FirstType, class SecondType >
inline ModPair<FirstType, SecondType >
ModMakePair(const FirstType& first_, const SecondType& second_)
{
    return ModPair<FirstType, SecondType >(first_, second_);
}

#endif	// __ModPair_H__
//
// Copyright (c) 1997, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
