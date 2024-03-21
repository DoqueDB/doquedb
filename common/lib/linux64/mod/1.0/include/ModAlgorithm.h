// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModAlgorithm.h -- 汎用アルゴリズムのテンプレート関数定義
// 
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModAlgorithm_H__
#define __ModAlgorithm_H__

#include "ModConfig.h"
#include "ModCommon.h"

//	TEMPLATE FUNCTION
//	ModSwap -- 2 つの変数の値を互いに入れ替える
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			操作する変数の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&		l
//			r の値を代入する変数
//		ValueType&		r
//			l の値を代入する変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
inline
void
ModSwap(ValueType& l, ValueType& r)
{
	ValueType tmp(l); l = r; r = tmp;
}

//////////////////////////////////////////////
//	ある領域上でのオブジェクトの生成、破棄	//
//////////////////////////////////////////////

//	TEMPLATE FUNCTION
//	ModConstruct --
//		ポインターの指す自由記憶領域上に与えられたオブジェクトを生成する
//
//	TEMPLATE ARGUMENTS
//		class T
//			自由記憶領域上に生成するオブジェクトのクラス
//
//	NOTES
//
//	ARGUMENTS
//		T*					p
//			オブジェクトを生成する自由記憶領域の先頭アドレス
//		const T&			v
//			自由記憶領域上に生成するオブジェクトの初期値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

#include <new>

template <class T>
inline
void
ModConstruct(T* p, const T& v)
{
	(void) new(p) T(v);
}

#if MOD_CONF_PARTIAL_SPECIALIZATION_STYLE == 1

// テンプレート引数が任意のポインター型の部分特別バージョンを定義して、
// その場合はたんに代入するようにする

template <class T>
inline void ModConstruct<T*>(T** p, const T*& v)
{ *p = const_cast<T*&>(v); }
#else

// この定義で意図したことになっているのか、いまいちよくわからない

template <class T>
inline void ModConstruct(T** p, const T*& v)
{ *p = const_cast<T*&>(v); }
#endif

// テンプレート引数が基本型の特別バージョンを定義して、
// その場合もたんに代入するようにする

ModTemplateNull
inline void ModConstruct(ModBoolean* p, const ModBoolean& v)
{ *p = const_cast<ModBoolean&>(v); }

ModTemplateNull
inline void ModConstruct(char* p, const char& v)
{ *p = const_cast<char&>(v); }

ModTemplateNull
inline void ModConstruct(unsigned char* p, const unsigned char& v)
{ *p = const_cast<unsigned char&>(v); }

ModTemplateNull
inline void ModConstruct(short* p, const short& v)
{ *p = const_cast<short&>(v); }

ModTemplateNull
inline void ModConstruct(unsigned short* p, const unsigned short& v)
{ *p = const_cast<unsigned short&>(v); }

ModTemplateNull
inline void ModConstruct(int* p, const int& v)
{ *p = const_cast<int&>(v); }

ModTemplateNull
inline void ModConstruct(unsigned int* p, const unsigned int& v)
{ *p = const_cast<unsigned int&>(v); }

ModTemplateNull
inline void ModConstruct(long* p, const long& v)
{ *p = const_cast<long&>(v); }

ModTemplateNull
inline void ModConstruct(unsigned long* p, const unsigned long& v)
{ *p = const_cast<unsigned long&>(v); }

ModTemplateNull
inline void ModConstruct(ModInt64* p, const ModInt64& v)
{ *p = const_cast<ModInt64&>(v); }

ModTemplateNull
inline void ModConstruct(ModUInt64* p, const ModUInt64& v)
{ *p = const_cast<ModUInt64&>(v); }

ModTemplateNull
inline void ModConstruct(float* p, const float& v)
{ *p = const_cast<float&>(v); }

ModTemplateNull
inline void ModConstruct(double* p, const double& v)
{ *p = const_cast<double&>(v); }

ModTemplateNull
inline void ModConstruct(long double* p, const long double& v)
{ *p = const_cast<long double&>(v); }

//	TEMPLATE FUNCTION
//	ModDestroy -- ポインターの指す自由記憶領域上のオブジェクトを破棄する
//
//	TEMPLATE ARGUMENTS
//		class T
//			自由記憶領域上に存在するオブジェクトのクラス
//
//	NOTES
//
//	ARGUMENTS
//		T*					p
//			破棄するオブジェクトが存在する自由記憶領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class T>
inline
void
ModDestroy(T* p)
{
	p->~T();
}

// テンプレート引数が任意のポインター型の部分特別バージョンを定義して、
// その場合はなにもしないようにする

#if MOD_CONF_PARTIAL_SPECIALIZATION_STYLE == 1
template <class T> inline void ModDestroy<T*>(T** p) { }
#else
template <class T> inline void ModDestroy(T** p) { }
#endif


// テンプレート引数が基本型の特別バージョンを定義して、
// その場合もなにもしないようにする

ModTemplateNull inline void ModDestroy(ModBoolean* p) { }
ModTemplateNull inline void ModDestroy(char*) { }
ModTemplateNull inline void ModDestroy(unsigned char*) { }
ModTemplateNull inline void ModDestroy(short*) { }
ModTemplateNull inline void ModDestroy(int*) { }
ModTemplateNull inline void ModDestroy(unsigned int*) { }
ModTemplateNull inline void ModDestroy(long*) { }
ModTemplateNull inline void ModDestroy(unsigned long*) { }
ModTemplateNull inline void ModDestroy(ModInt64*) { }
ModTemplateNull inline void ModDestroy(ModUInt64*) { }
ModTemplateNull inline void ModDestroy(float*) { }
ModTemplateNull inline void ModDestroy(double*) { }
ModTemplateNull inline void ModDestroy(long double*) { }

//////////////////////
//	シーケンス操作	//
//////////////////////

//	TEMPLATE FUNCTION
//	ModForEach -- ある範囲のシーケンス中のそれぞれの値を操作する
//
//	TEMPLATE ARGUMENTS
//		class ForwardIterator
//			前へひとつづつ進めることのできる反復子
//		class Operator
//			操作に使用する関数
//
//	NOTES
//		シーケンス中の先頭から、ひとつひとつ値を操作する
//
//	ARGUMENTS
//		ForwardIterator		first
//			シーケンスの先頭を指す反復子
//		ForwardIterator		last
//			シーケンスの末尾のひとつ次の位置を指す反復子
//		Operator			op
//			シーケンス中の値ごとに実行する操作関数
//			シーケンス中の値のみを引数とする必要がある
//
//	RETURN
//		操作に使用した関数
//
//	EXCEPTIONS

template <class ForwardIterator, class Operator>
inline
Operator
ModForEach(ForwardIterator first, ForwardIterator last, Operator op)
{
	for (; first != last; ++first)
		op(*first);
	return op;
}

//	TEMPLATE FUNCTION
//	ModReplace -- ある範囲のシーケンス中のある値を違う値に置き換える
//
//	TEMPLATE ARGUMENTS
//		class ForwardIterator
//			前へひとつづつ進めることのできる反復子
//		class T
//			反復子の指すオブジェクトのクラス
//
//	NOTES
//
//	ARGUMENTS
//		ForwardIterator		first
//			シーケンスの先頭を指す反復子
//		ForwardIterator		last
//			シーケンスの末尾のひとつ次の位置を指す反復子
//		T&					cond
//			シーケンス中の値のうちこの値と等しいものを value で置き換える
//		T&					value
//			シーケンス中の値のうち cond と等しいものをこの値で置き換える
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ForwardIterator, class T>
inline
void
ModReplace(ForwardIterator first, ForwardIterator last,
		   const T& cond, const T& value)
{
	for (; first != last; ++first)
		if (*first == cond)
			*first = value;
}

///////////////////////////////////////////////////////////////////////////////

//
// CONST private
// ModStlThreshold -- クイックソートを適用する閾値(private)
//
// NOTES
// この定数は ModSort においてクイックソートと挿入ソートのどちらを
// 適用するかを決めるために用いる。
//
const int ModStlThreshold = 16;

//----------------------
// 関数クラス
//----------------------
//
// TEMPLATE CLASS
// ModBinaryFunction -- ２項演算子を提供するクラス
//
// TEMPLATE ARGUMENTS
// class ValueType1
//		前項の型
// class ValueType2
//		後項の型
// class ResultType
//		返り値の型
//
// NOTES
// このクラスは２項演算子を一般化したものである。
// 演算子の２つのオペランドと返り値の型を得られる。
// 現在はこのクラスから派生した ModLess をデフォルトの比較関数として
// 提供するためだけに用意されている。
//

template <class ValueType1, class ValueType2, class ResultType>
class ModBinaryFunction
{
public:
	typedef ValueType1 firstArgumentType;
	typedef ValueType2 secondArgumentType;
	typedef ResultType resultType;
};

//
// TEMPLATE CLASS
// ModLess -- < の動作を提供するクラス
//
// TEMPLATE ARGUMENTS
// class ValueType
//		オペランドの型
//
// NOTES
// このクラスは Generic Algorithm において比較関数を表すクラスを使う際、
// そのデフォルトのクラスを提供するものである。
// ModLess<T>(v1, v2) は T(v1) < T(v2) の結果を返す。
//

template <class ValueType>
class ModLess
	: public	ModBinaryFunction<ValueType, ValueType, ModBoolean>
{
public:
//	ModLess();
	// 遅くするだけなので inverseFlag はやめる 98.6.26
	// ModLess(ModBoolean inverseFlag_);
	ModBoolean operator()(const ValueType& value1,
						  const ValueType& value2) const;
protected:
	// 遅くするだけなので inverseFlag はやめる 98.6.26
	// ModBoolean inverseFlag;			// '<' の比較ではなく '>' の比較にする
};

//
// TEMPLATE FUNCTION
// ModLess::ModLess -- デフォルトコンストラクタ
//
// TEMPLATE ARGUMENTS
// class ValueType
//		比較される値の型。operator< が定義されている型でなければならない。
//
// NOTES
// デフォルトコンストラクタを提供する。
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

//template <class ValueType>
//inline
//ModLess<ValueType>::ModLess()
//	: inverseFlag(ModFalse)
//{ }

// 遅くするだけなので inverseFlag はやめる 98.6.26
//
// TEMPLATE FUNCTION
// ModLess::ModLess -- コンストラクタ
//
// TEMPLATE ARGUMENTS
// class ValueType
//		比較される値の型。operator< が定義されている型でなければならない。
//
// NOTES
// 真偽を逆にするフラグを指定したコンストラクタを提供する。
// 引数に ModTrue が指定されている場合は operator()の返り値を
// '<'の結果と逆にする。
// !!比較処理を遅くするだけでいいことがないのでやめる 98.6.26!!
//
// ARGUMENTS
// ModBoolean inverseFlag_
//		返り値の真偽を逆にすることを示すフラグ。
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
//template <class ValueType>
//inline
//ModLess<ValueType>::ModLess(ModBoolean inverseFlag_) :
//	inverseFlag(inverseFlag_)
//{
//}

//
// TEMPLATE FUNCTION
// ModLess::operator() -- ModLess で比較するのに使うオペレータ
//
// TEMPLATE ARGUMENTS
// class ValueType
//		比較される値の型。operator< が定義されている型でなければならない。
//
// NOTES
// この関数は ModLess を使って比較を行なうために用いる。
//
// ARGUMENTS
// const ValueType& value1
//		'<'の第一項に相当する値
// const ValueType& value2
//		'<'の第二項に相当する値
//
// RETURN
// value1 < value2 が成り立つ場合は ModTrue を、
// 成り立たない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
template <class ValueType>
inline ModBoolean
ModLess<ValueType>::operator()(const ValueType& value1,
							   const ValueType& value2) const
{
// 遅くするだけなので inverseFlag はやめる 98.6.26
//	return ((this->inverseFlag == ModFalse) ?
//			(value1 < value2) : (value1 > value2)) ? ModTrue : ModFalse;
	return (value1 < value2)?ModTrue:ModFalse;
}

//
// TEMPLATE CLASS
// ModGreater -- > の動作を提供するクラス
//
// TEMPLATE ARGUMENTS
// class ValueType
//		オペランドの型
//
// NOTES
// このクラスは '>' の比較関数を提供するものである。
// ModGreater<T>(v1, v2) は T(v1) > T(v2) の結果を返す。
//

template <class ValueType>
class ModGreater
	: public	ModBinaryFunction<ValueType, ValueType, ModBoolean>
{
public:
//	ModGreater();
	// 遅くするだけなので inverseFlag はやめる 98.6.26
	// ModGreater(ModBoolean inverseFlag_);
	ModBoolean operator()(const ValueType& value1,
						  const ValueType& value2) const;
protected:
	// 遅くするだけなので inverseFlag はやめる 98.6.26
	// ModBoolean inverseFlag;
};

//
// TEMPLATE FUNCTION
// ModGreater::ModGreater -- コンストラクタ
//
// TEMPLATE ARGUMENTS
// class ValueType
//		比較される値の型。operator> が定義されている型でなければならない。
//
// NOTES
// デフォルトコンストラクタ、および真偽を逆にするフラグを指定した
// コンストラクタを提供する。
// 引数に ModTrue が指定されている場合は operator()の返り値を
// '>'の結果と逆にする。
// !!比較処理を遅くするだけでいいことがないのでやめる 98.6.26!!
//
// ARGUMENTS
// ModBoolean inverseFlag_
//		返り値の真偽を逆にすることを示すフラグ。省略可能。
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

//template <class ValueType>
//inline
//ModGreater<ValueType>::ModGreater()
//{ }

//
// TEMPLATE FUNCTION
// ModGreater::operator() -- ModGreater で比較するのに使うオペレータ
//
// TEMPLATE ARGUMENTS
// class ValueType
//		比較される値の型。operator< が定義されている型でなければならない。
//
// NOTES
// この関数は ModGreater を使って比較を行なうために用いる。
//
// ARGUMENTS
// const ValueType& value1
//		'<'の第一項に相当する値
// const ValueType& value2
//		'<'の第二項に相当する値
//
// RETURN
// value1 < value2 が成り立つ場合は ModTrue を、
// 成り立たない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
template <class ValueType>
inline ModBoolean
ModGreater<ValueType>::operator()(const ValueType& value1,
								  const ValueType& value2) const
{
//	return ((this->inverseFlag == ModFalse) ?
//			(value1 > value2) : (value1 < value2)) ? ModTrue : ModFalse;
	return (value1 > value2) ? ModTrue : ModFalse;
}

#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
//
//----------------------
// イテレータ関連
//----------------------
//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModValueType -- 一般的なポインタの指す型を得る
//
// TEMPLATE ARGUMENTS
// class ValueType
//		iterator が指す型。ModValueType にこれへのポインタが渡されることにより
//		自動的にセットされる。呼出し側が明示的にセットすることはない。
//
// NOTES
// この関数は int* などの一般のポインタを使ったイテレータに対して
// それの指す型を得るために用いる。
// ModVector などに対しては再定義される。
//
// ARGUMENTS
// ValueType* iterator
//		それの指す型を得たいポインタ
//
// RETURN
// ValueType* にキャストした 0 を返す。
//
// EXCEPTIONS
// なし
//

template <class ValueType>
inline
ValueType*
ModValueType(ValueType* iterator)
{
	return (ValueType*) 0;
}
#endif

//----------------------
// ヒープ関連
//----------------------
//--------------------
// 下請け関数
//--------------------
//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModPushHeap -- ヒープに一つ加える(比較関数つき、内部関数)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class Compare
//		比較関数を提供するクラス
// class ValueType
//		RandomAccessIterator が指す値の型
//
// NOTES
// この関数は一つ空きがあるヒープ構造に値を一つ加えるのに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープの先頭を指すイテレータ
// int holeIndex
//		ヒープ構造の中で値の入っていない場所を指すインデックス
// int topIndex
//		ヒープ構造が崩れている範囲のトップを指すインデックス
// ValueType value
//		ヒープに新たに加えたい値
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class Compare, class ValueType>
inline
void
ModPushHeap(RandomAccessIterator first, int holeIndex,
			int topIndex, ValueType value, Compare compare)
{
	int parentIndex = (holeIndex - 1) / 2;
	while (holeIndex > topIndex && compare(*(first + parentIndex), value)) {
		*(first + holeIndex) = *(first + parentIndex);
		holeIndex = parentIndex;
		parentIndex = (holeIndex - 1) / 2;
	}
	*(first + holeIndex) = value;
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModPushHeap -- ヒープに一つ加える(内部関数)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class ValueType
//		RandomAccessIterator が指す値の型
//
// NOTES
// この関数は一つ空きがあるヒープ構造に値を一つ加えるのに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープの先頭を指すイテレータ
// int holeIndex
//		ヒープ構造の中で値の入っていない場所を指すインデックス
// int topIndex
//		ヒープ構造が崩れている範囲のトップを指すインデックス
// ValueType value
//		ヒープに新たに加えたい値
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class ValueType>
inline
void
ModPushHeap(RandomAccessIterator first, int holeIndex,
			int topIndex, ValueType value)
{
	ModPushHeap(first, holeIndex, topIndex, value, ModLess<ValueType>());
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModAdjustHeap -- ヒープを作りなおす(比較関数つき、内部関数)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class Compare
//		比較関数を提供するクラス
// class ValueType
//		RandomAccessIterator が指す値の型
//
// NOTES
// この関数は一時的に構造が崩れたヒープを作りなおすのに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープの先頭を指すイテレータ
// int holeIndex
//		ヒープ構造の中で値の入っていない場所を指すインデックス
// int length
//		ヒープの全体の長さ
// ValueType value
//		ヒープに新たに加えたい値
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class Compare, class ValueType>
inline
void
ModAdjustHeap(RandomAccessIterator first, int holeIndex,
			  int length, ValueType value, Compare compare)
{
	int topIndex = holeIndex;
	int secondChild = 2 * (holeIndex + 1);
	while (secondChild < length) {
		if (compare(*(first + secondChild), *(first + (secondChild - 1)))) {
			--secondChild;
		}
		*(first + holeIndex) = *(first + secondChild);
		holeIndex = secondChild;
		secondChild = 2 * (secondChild + 1);
	}
	if (secondChild == length) {
		*(first + holeIndex) = *(first + (secondChild - 1));
		holeIndex = secondChild - 1;
	}
	ModPushHeap(first, holeIndex, topIndex, value, compare);
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModAdjustHeap -- ヒープを作りなおす(内部関数)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class ValueType
//		RandomAccessIterator が指す値の型
//
// NOTES
// この関数は一時的に構造が崩れたヒープを作りなおすのに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープの先頭を指すイテレータ
// int holeIndex
//		ヒープ構造の中で値の入っていない場所を指すインデックス
// int length
//		ヒープの全体の長さ
// ValueType value
//		ヒープに新たに加えたい値
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class ValueType>
inline
void
ModAdjustHeap(RandomAccessIterator first, int holeIndex,
			  int length, ValueType value)
{
	ModAdjustHeap(first, holeIndex, length, value, ModLess<ValueType>());
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModMakeHeapWithValueType -- ModMakeHeap の下請け関数(内部関数)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class Compare
//		比較関数を提供するクラス
// class ValueType
//		RandomAccessIterator が指す値の型
//
// NOTES
// ランダムアクセス可能な配列から、ヒープを作成する。
// この関数は ModMakeHeap の下請け関数であり、直接呼ばれることはない。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープを作る対象となる部分配列の先頭
// RandomAccessIterator last
//		ヒープを作る対象となる部分配列の最後尾の次の要素
// ValueType* dummyValue
//		ValueType に型をアサインするためのダミー引数
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
template <class RandomAccessIterator, class Compare>
#else
template <class RandomAccessIterator, class Compare, class ValueType>
#endif
inline
void
ModMakeHeapWithValueType(RandomAccessIterator first, RandomAccessIterator last,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
						 ValueType* dummyValue,
#endif
						 Compare compare)
{
	if (last - first < 2) {
		// 要素数が 1 なら作る必要がない。
		return;
	}

	int length = (int)(last - first);			// 配列の要素数
	int parentIndex = length / 2 - 1;	// あるノードの親のあるインデックス
										// 最右の１段階の親から始める
	while (1) {
		// first + parentIndex を抜きだし、ヒープを作り直す
		ModAdjustHeap(first, parentIndex, length,
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
					  ModTypename RandomAccessIterator::
#endif
					  ValueType(*(first + parentIndex)),
					  compare);
		if (parentIndex == 0) {
			// root に到達
			return;
		}
		--parentIndex;
	}
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModMakeHeapWithValueType -- ModMakeHeap の下請け関数(内部関数)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class ValueType
//		RandomAccessIterator が指す値の型
//
// NOTES
// ランダムアクセス可能な配列から、ヒープを作成する。
// 比較関数は ModLess(すなわち '<')が使用される。
// この関数は ModMakeHeap の下請け関数であり、直接呼ばれることはない。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープを作る対象となる部分配列の先頭
// RandomAccessIterator last
//		ヒープを作る対象となる部分配列の最後尾の次の要素
// ValueType* dummyValue
//		ValueType に型をアサインするためのダミー引数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
template <class RandomAccessIterator>
#else
template <class RandomAccessIterator, class ValueType>
#endif
inline
void
ModMakeHeapWithValueType(RandomAccessIterator first, RandomAccessIterator last
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
						 , ValueType* dummyValue
#endif
	)
{
	ModMakeHeapWithValueType(first, last,
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
							 ModLess<ModTypename RandomAccessIterator::ValueType>()
#else
							 dummyValue, ModLess<ValueType>()
#endif
		);
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModPopHeapWithValueType -- ModPopHeap の下請け関数(内部関数)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class Compare
//		比較関数を提供するクラス
// class ValueType
//		RandomAccessIterator が指す値の型
//
// NOTES
// ヒープの根の要素をとりだし、ヒープを作り直す。
// この関数は ModPopHeap の下請け関数であり、直接呼ばれることはない。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープを構成する配列の先頭
// RandomAccessIterator last
//		ヒープを構成する配列の最後尾の次の要素
// ValueType* dummyValue
//		ValueType に型をアサインするためのダミー引数
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
template <class RandomAccessIterator, class Compare>
#else
template <class RandomAccessIterator, class Compare, class ValueType>
#endif
inline
void
ModPopHeapWithValueType(RandomAccessIterator first, RandomAccessIterator last,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
						ValueType* dummyValue,
#endif
						Compare compare)
{
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	ModTypename RandomAccessIterator::
#endif
	ValueType temporaryValue(*(last - 1));
	*(last - 1) = *first;
	ModAdjustHeap(first, int(0), int(last - first - 1),
				  temporaryValue, compare);
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModPopHeapWithValueType -- ModPopHeap の下請け関数(内部関数)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class ValueType
//		RandomAccessIterator が指す値の型
//
// NOTES
// ヒープの根の要素をとりだし、ヒープを作り直す。
// 比較関数は ModLess(すなわち '<')が使用される。
// この関数は ModPopHeap の下請け関数であり、直接呼ばれることはない。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープを構成する配列の先頭
// RandomAccessIterator last
//		ヒープを構成する配列の最後尾の次の要素
// ValueType* dummyValue
//		ValueType に型をアサインするためのダミー引数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
template <class RandomAccessIterator>
#else
template <class RandomAccessIterator, class ValueType>
#endif
inline
void
ModPopHeapWithValueType(RandomAccessIterator first, RandomAccessIterator last
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
						, ValueType* dummyValue
#endif
	)
{
	ModPopHeapWithValueType(first, last,
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
							ModLess<ModTypename RandomAccessIterator::ValueType>()
#else
							dummyValue, ModLess<ValueType>()
#endif
		);
}

//--------------------
// 公開関数
//--------------------
//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModMakeHeap -- ヒープを作る(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// ランダムアクセス可能な配列から、ヒープを作成する。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープを作る対象となる部分配列の先頭
// RandomAccessIterator last
//		ヒープを作る対象となる部分配列の最後尾の次の要素
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
// その他
// 下位の例外はそのまま再送出する。
//

template <class RandomAccessIterator, class Compare>
inline
void
ModMakeHeap(RandomAccessIterator first, RandomAccessIterator last,
			Compare compare)
{
	ModMakeHeapWithValueType(first, last,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
							 ModValueType(first),
#endif
							 compare);
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModMakeHeap -- ヒープを作る
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
//
// NOTES
// ランダムアクセス可能な配列から、ヒープを作成する。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープを作る対象となる部分配列の先頭
// RandomAccessIterator last
//		ヒープを作る対象となる部分配列の最後尾の次の要素
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator>
inline
void
ModMakeHeap(RandomAccessIterator first, RandomAccessIterator last)
{
	ModMakeHeapWithValueType(first, last
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
							 , ModValueType(first)
#endif
		);
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModSortHeap -- ヒープを使ったソート(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数はヒープを使って配列をソートするために用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープを構成する配列の先頭
// RandomAccessIterator last
//		ヒープを構成する配列の最後尾の次の要素
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class Compare>
inline
void
ModSortHeap(RandomAccessIterator first, RandomAccessIterator last,
			Compare compare)
{
	for (; last - first > 1; --last)
		ModPopHeap(first, last, compare);
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModSortHeap -- ヒープをソートする
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
//
// NOTES
// この関数はヒープを使って配列をソートするために用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		ヒープを構成する配列の先頭
// RandomAccessIterator last
//		ヒープを構成する配列の最後尾の次の要素
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator>
inline
void
ModSortHeap(RandomAccessIterator first, RandomAccessIterator last)
{
	for (; last - first > 1; --last)
		ModPopHeap(first, last);
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModPopHeap -- ヒープから一つ取り出す(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// first と last - 1 を入れ換え、first と last - 2 の間でヒープを作りなおす。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理の対象となる部分配列の先頭
// RandomAccessIterator last
//		処理の対象となる部分配列の最後尾の次の要素
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class Compare>
inline
void
ModPopHeap(RandomAccessIterator first, RandomAccessIterator last,
		   Compare compare)
{
	ModPopHeapWithValueType(first, last,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
							ModValueType(first),
#endif
							compare);
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModPopHeap -- ヒープから一つ取り出す
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ヒープの要素になるオブジェクトに対するランダムアクセスイテレータ
//
// NOTES
// first と last - 1 を入れ換え、first と last - 2 の間でヒープを作りなおす。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理の対象となる部分配列の先頭
// RandomAccessIterator last
//		処理の対象となる部分配列の最後尾の次の要素
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator>
inline
void
ModPopHeap(RandomAccessIterator first, RandomAccessIterator last)
{
	ModPopHeapWithValueType(first, last
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
							, ModValueType(first)
#endif
		);
}

//----------------
// ソート
//----------------
//--------------------
// 内部関数
//--------------------
//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModMedian -- 中間値を得る(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class ValueType
//		対象となるオブジェクトの型
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数は 3 つの値で大きさで並べて中間に位置するデータを得るために用いる。
//
// ARGUMENTS
// ValueType value1
//		比較されるデータ
// ValueType value2
//		比較されるデータ
// ValueType value3
//		比較されるデータ
// Compare compare
//		比較関数
// 
// RETURN
// value1、value2、value3 のうち大きさで並べて中間に位置するデータの
// コピーを返す。
//
// EXCEPTIONS
// なし
//

template <class ValueType, class Compare>
inline
ValueType
ModMedian(ValueType value1, ValueType value2, ValueType value3,
		  Compare compare)
{
	return (compare(value1, value2)) ?
		((compare(value2, value3)) ? value2 :
		 (compare(value1, value3)) ? value3 : value1) :
		((compare(value1, value3)) ? value1 :
		 (compare(value2, value3)) ? value3 : value2);
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModMedian -- 中間値を得る
//
// TEMPLATE ARGUMENTS
// class ValueType
//		対象となるオブジェクトの型
//
// NOTES
// この関数は 3 つの値で大きさで並べて中間に位置するデータを得るために用いる。
//
// ARGUMENTS
// ValueType value1
//		比較されるデータ
// ValueType value2
//		比較されるデータ
// ValueType value3
//		比較されるデータ
// 
// RETURN
// value1、value2、value3 のうち大きさで並べて中間に位置するデータの
// コピーを返す。
//
// EXCEPTIONS
// なし
//

template <class ValueType>
inline
ValueType
ModMedian(ValueType value1, ValueType value2, ValueType value3)
{
	return ModMedian(value1, value2, value3, ModLess<ValueType>());
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModUnguardedPartition -- ModQuickSortLoop の下請け関数(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象となるデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象となるデータの型
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数は ModQuickSortLoop の下請けとして、配列のある範囲に対して
// クイックソートのアルゴリズム(ある値より大きい値と小さい値を交換する)を
// 行なうために用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		対象となるデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		対象となるデータ配列の終了位置を指すイテレータ
// const ValueType& pivot
//		比較の基準となるデータ
// Compare compare
//		比較関数
//
// RETURN
// pivot を超えないもっとも後ろにあるデータを指すイテレータを返す。
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class ValueType, class Compare>
inline
RandomAccessIterator
ModUnguardedPartition(RandomAccessIterator first, RandomAccessIterator last,
					  const ValueType& pivot, Compare compare)
{
	while (1) {
		// 先頭から
		while (compare(*first, pivot)) {
			++first;
		}
		--last;
		while (compare(pivot, *last)) {
			--last;
		}
		if (!(first < last)) {
			break;
		}
		ValueType	temporaryValue(*first);
		*first = *last;
		*last = temporaryValue;

		++first;
	}
	return first;
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModUnguardedPartition -- ModQuickSortLoop の下請け関数
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象となるデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象となるデータの型
//
// NOTES
// この関数は ModQuickSortLoop の下請けとして、配列のある範囲に対して
// クイックソートのアルゴリズム(ある値より大きい値と小さい値を交換する)を
// 行なうために用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		対象となるデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		対象となるデータ配列の終了位置を指すイテレータ
// const ValueType& pivot
//		比較の基準となるデータ
//
// RETURN
// pivot を超えないもっとも後ろにあるデータを指すイテレータを返す。
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class ValueType>
inline
RandomAccessIterator
ModUnguardedPartition(RandomAccessIterator first, RandomAccessIterator last,
					  const ValueType& pivot)
{
	return ModUnguardedPartition(first, last, pivot, ModLess<ValueType>());
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModQuickSortLoop -- sort の下請け関数のクイックソート(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象となるデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象となるデータの型
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数は ModSort の下請けとして、ModStlThreshold 以上の要素数を
// 持つものを対象にクイックソートを行なうために用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理対象のデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		処理対象のデータ配列の終了位置を指すイテレータ
// ValueType* dummyValue
//		ValueType に型名をアサインするためのダミー引数
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
		  class ValueType,
#endif
		  class Compare>
inline
void
ModQuickSortLoop(RandomAccessIterator first, RandomAccessIterator last,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
				 ValueType* dummyValue,
#endif
				 Compare compare)
{
	while (last - first > ModStlThreshold) {
		RandomAccessIterator cut(
			ModUnguardedPartition(first, last,
								  ModMedian(*(first + (last - first) / 4),
											*(first + (last - first) / 2),
											*(first + (last - first) / 2 + (last - first) / 4),
											compare),
								  compare));
		if (cut - first >= last - cut) {
			ModQuickSortLoop(cut, last,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
							 dummyValue,
#endif
							 compare);
			last = cut;
		} else {
			ModQuickSortLoop(first, cut,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
							 dummyValue,
#endif
							 compare);
			first = cut;
		}
	}
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModQuickSortLoop -- sort の下請け関数のクイックソート
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象となるデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象となるデータの型
//
// NOTES
// この関数は ModSort の下請けとして、ModStlThreshold 以上の要素数を
// 持つものを対象にクイックソートを行なうために用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理対象のデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		処理対象のデータ配列の終了位置を指すイテレータ
// ValueType* dummyValue
//		ValueType に型名をアサインするためのダミー引数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
template <class RandomAccessIterator>
#else
template <class RandomAccessIterator, class ValueType>
#endif
inline
void
ModQuickSortLoop(RandomAccessIterator first, RandomAccessIterator last
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
				 , ValueType* dummyValue
#endif
	)
{
	ModQuickSortLoop(first, last,
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
					 ModLess<ModTypename RandomAccessIterator::ValueType>()
#else
					 dummyValue, ModLess<ValueType>()
#endif
		);
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModUnguardedLinearInsert -- 線形挿入の下請け関数(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象となるデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象となるデータの型
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// 線形挿入の下請けとして value より大きい値をひとつずつ後ろにずらし、
// 空いたところに value を挿入するために用いる。
//
// ARGUMENTS
// RandomAccessIterator last
//		処理対象のデータ配列の終了位置を指すイテレータ
// ValueType value
//		挿入する値
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class ValueType, class Compare>
inline
void
ModUnguardedLinearInsert(RandomAccessIterator last, ValueType value,
						 Compare compare)
{
	RandomAccessIterator next(last);
	while (compare(value, *--next)) {
		*last = *next;
		last = next;
	}
	*last = value;
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModUnguardedLinearInsert -- 線形挿入の下請け関数
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象となるデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象となるデータの型
//
// NOTES
// 線形挿入の下請けとして value より大きい値をひとつずつ後ろにずらし、
// 空いたところに value を挿入するために用いる。
//
// ARGUMENTS
// RandomAccessIterator last
//		処理対象のデータ配列の終了位置を指すイテレータ
// ValueType value
//		挿入する値
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class ValueType>
inline
void
ModUnguardedLinearInsert(RandomAccessIterator last, ValueType value)
{
	ModUnguardedLinearInsert(last, value, ModLess<ValueType>());
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModLinearInsert -- 線形挿入(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象のデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象のデータの型
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数は線形挿入を行なうために用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理対象となるデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		処理対象となるデータ配列の終了位置を指すイテレータ
// ValueType* dummy
//		ValueType に型をアサインするためのダミー引数
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
		  class ValueType,
#endif
		  class Compare>
inline
void
ModLinearInsert(RandomAccessIterator first, RandomAccessIterator last,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
				ValueType* dummy,
#endif
				Compare compare)
{
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	ModTypename RandomAccessIterator::
#endif
	ValueType value(*last);
	if (compare(value, *first)) {
		ModCopyBackward(first, last, last + 1);
		*first = value;
	} else {
		ModUnguardedLinearInsert(last, value, compare);
	}
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModLinearInsert -- 線形挿入
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象のデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象のデータの型
//
// NOTES
// この関数は線形挿入を行なうために用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理対象となるデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		処理対象となるデータ配列の終了位置を指すイテレータ
// ValueType* dummy
//		ValueType に型をアサインするためのダミー引数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
template <class RandomAccessIterator>
#else
template <class RandomAccessIterator, class ValueType>
#endif
inline
void
ModLinearInsert(RandomAccessIterator first, RandomAccessIterator last
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
				, ValueType* dummy
#endif
	)
{
	ModLinearInsert(first, last,
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
					ModLess<ModTypename RandomAccessIterator::ValueType>()
#else
					dummy, ModLess<ValueType>()
#endif
		);
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModUnguardedInsertionSort -- 挿入ソートの下請け関数(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象のデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象のデータの型
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数は挿入ソートの下請け関数として基本ループを回すのに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理対象のデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		処理対象のデータ配列の終了位置を指すイテレータ
// ValueType* dummy
//		ValueType に型をアサインするためのダミー引数
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
		  class ValueType,
#endif
		  class Compare>
inline
void
ModUnguardedInsertionSort(RandomAccessIterator first,
						  RandomAccessIterator last,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
						  ValueType* dummy,
#endif
						  Compare compare)
{
	RandomAccessIterator i(first);

	for (; !(i == last); ++i) {
		ModUnguardedLinearInsert(i,
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
								 *i,
#else
								 ValueType(*i),
#endif
								 compare);
	}
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModUnguardedInsertionSort -- 挿入ソートの下請け関数
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象のデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象のデータの型
//
// NOTES
// この関数は挿入ソートの下請け関数として基本ループを回すのに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理対象のデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		処理対象のデータ配列の終了位置を指すイテレータ
// ValueType* dummy
//		ValueType に型をアサインするためのダミー引数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
template <class RandomAccessIterator>
#else
template <class RandomAccessIterator, class ValueType>
#endif
inline
void
ModUnguardedInsertionSort(RandomAccessIterator first,
						  RandomAccessIterator last
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
						  , ValueType* dummy
#endif
	)
{
	ModUnguardedInsertionSort(first, last,
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
							  ModLess<ModTypename RandomAccessIterator::ValueType>()
#else
							  dummy, ModLess<ValueType>()
#endif
		);
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModInsertionSort -- 挿入ソート(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象のデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象のデータの型
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数は挿入ソートの下請け関数として基本ループを回すのに用いる。
// Unguarded に比べて引数に対する条件判定などがある分遅い。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理対象のデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		処理対象のデータ配列の終了位置を指すイテレータ
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class Compare>
inline
void
ModInsertionSort(RandomAccessIterator first, RandomAccessIterator last,
				 Compare compare)
{
	if (first == last) {
		return;
	}
	RandomAccessIterator i(first + 1);
	for (; !(i == last); ++i) {
		ModLinearInsert(first, i,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
						ModValueType(first),
#endif
						compare);
	}
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModInsertionSort -- 挿入ソート
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象のデータを指すランダムアクセスイテレータの型
// class ValueType
//		処理対象のデータの型
//
// NOTES
// この関数は挿入ソートの下請け関数として基本ループを回すのに用いる。
// Unguarded に比べて引数に対する条件判定などがある分遅い。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理対象のデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		処理対象のデータ配列の終了位置を指すイテレータ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator>
inline
void
ModInsertionSort(RandomAccessIterator first, RandomAccessIterator last)
{
	if (first == last) {
		return;
	}
	RandomAccessIterator i(first + 1);
	for (; !(i == last); ++i) {
		ModLinearInsert(first, i
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
						, ModValueType(first)
#endif
			);
	}
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModFinalInsertionSort -- ある程度ソートされた後に挿入ソート(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象のデータを指すランダムアクセスイテレータの型
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数は ModSort の下請け関数として、要素数が ModStlThreshold 以下に
// なったものに対して挿入ソートを行なうために用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理対象のデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		処理対象のデータ配列の終了位置を指すイテレータ
// Compare compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class Compare>
inline
void
ModFinalInsertionSort(RandomAccessIterator first, RandomAccessIterator last,
					  Compare compare)
{
	if (last - first > ModStlThreshold) {
		ModInsertionSort(first, first + ModStlThreshold, compare);
		ModUnguardedInsertionSort(first + ModStlThreshold, last,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
								  ModValueType(first),
#endif
								  compare);
	} else {
		ModInsertionSort(first, last, compare);
	}
}

//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModFinalInsertionSort -- ある程度ソートされた後に挿入ソート
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		処理対象のデータを指すランダムアクセスイテレータの型
//
// NOTES
// この関数は ModSort の下請け関数として、要素数が ModStlThreshold 以下に
// なったものに対して挿入ソートを行なうために用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		処理対象のデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		処理対象のデータ配列の終了位置を指すイテレータ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator>
inline
void
ModFinalInsertionSort(RandomAccessIterator first, RandomAccessIterator last)
{
	if (last - first > ModStlThreshold) {
		ModInsertionSort(first, first + ModStlThreshold);
		ModUnguardedInsertionSort(first + ModStlThreshold, last
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
								  , ModValueType(first)
#endif
			);
	} else {
		ModInsertionSort(first, last);
	}
}

//--------------------
// 公開関数
//--------------------
//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModSort -- ソート(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ソートする配列に対するランダムアクセスイテレータ
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数は与えられた配列をソートするのに用いる。
// ソートは要素数が ModStlThreshold 以下になるレベルまで
// クイックソートで行ない、それ以後は挿入ソートになる。
//
// ARGUMENTS
// RandomAccessIterator first
//		ソートの対象となる配列の先頭
// RandomAccessIterator last
//		ソートの対象となる配列の最後尾の次
// Compare compare
// 		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class Compare>
inline
void
ModSort(RandomAccessIterator first, RandomAccessIterator last,
		Compare compare)
{
	ModQuickSortLoop(first, last,
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
					 ModValueType(first),
#endif
					 compare);
	ModFinalInsertionSort(first, last, compare);
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModSort -- ソート
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		ソートする配列に対するランダムアクセスイテレータ
//
// NOTES
// この関数は与えられた配列をソートするのに用いる。
// ソートは要素数が ModStlThreshold 以下になるレベルまで
// クイックソートで行ない、それ以後は挿入ソートになる。
//
// ARGUMENTS
// RandomAccessIterator first
//		ソートの対象となる配列の先頭
// RandomAccessIterator last
//		ソートの対象となる配列の最後尾の次
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator>
inline
void
ModSort(RandomAccessIterator first, RandomAccessIterator last)
{
	ModQuickSortLoop(first, last
#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
					 , ModValueType(first)
#endif
		);
	ModFinalInsertionSort(first, last);
}

//----------------
// コピー
//----------------
//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModCopy -- コピーする
//
// TEMPLATE ARGUMENTS
// class InputIterator
//		コピーする元を指すインプットイテレータの型
// class OutputIterator
//		コピー先を指すアウトプットイテレータの型
//
// NOTES
// この関数は一般的な複製のアルゴリズムを使うときに用いる。
//
// ARGUMENTS
// InputIterator first
//		コピー元のデータ開始位置を指すイテレータ
// InputIterator last
//		コピー元のデータ終了位置を指すイテレータ
// OutputIterator result
//		コピー先の領域開始位置を指すイテレータ
//
// RETURN
// コピー先のデータ終了位置を指すイテレータを返す。
//
// EXCEPTIONS
// なし
//

template <class InputIterator, class OutputIterator>
inline
OutputIterator
ModCopy(InputIterator first, InputIterator last, OutputIterator result)
{
	for (; first != last; ++result, ++first)
		*result = *first;
	return result;
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModCopyBackward -- 後向きにコピーする
//
// TEMPLATE ARGUMENTS
// class BidirectionalIterator
//		コピー元のデータを指す双方向イテレータの型
// class OutputBidirectionalIterator
//		コピー先のデータを指す出力双方向イテレータの型
//
// NOTES
// この関数は領域の後ろから走査しながらコピーする一般的なアルゴリズムを
// 使うときに用いる。
//
// ARGUMENTS
// BidirectionalIterator first
//		コピー元データの開始位置(配列最後尾)を指すイテレータ
// BidirectionalIterator last
//		コピー元データの終了位置(配列先頭の１つ前)を指すイテレータ
// OutputBidirectionalIterator result
//		コピー先データを格納する開始位置(最後尾の次)を指すイテレータ
//
// RETURN
// コピー先データの終了位置(先頭)を指すイテレータを返す。
//
// EXCEPTIONS
// なし
//

template <class BidirectionalIterator, class OutputBidirectionalIterator>
inline
OutputBidirectionalIterator
ModCopyBackward(BidirectionalIterator first, BidirectionalIterator last,
				OutputBidirectionalIterator result)
{
	for (; first != last; *--result = *--last) ;
	return result;
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModFill -- 埋める
//
// TEMPLATE ARGUMENTS
// class ForwardIterator
//		埋める対象の領域を指すフォワードイテレータの型
// class ValueType
//		ForwardIterator の指す、埋めるデータの型
//
// NOTES
// この関数はある領域をひとつの値で埋める一般的なアルゴリズムを
// 使うときに用いる。
//
// ARGUMENTS
// ForwardIterator first
//		データを埋める領域の開始位置を指すイテレータ
// ForwardIterator last
//		データを埋める領域の終了位置を指すイテレータ
// const ValueType& value
//		埋めるデータへの参照
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class ForwardIterator, class ValueType>
inline
void
ModFill(ForwardIterator first, ForwardIterator last, const ValueType& value)
{
	for (; first != last; ++first)
		*first = value;
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModFillWithNumber -- 数を指定して埋める
//
// TEMPLATE ARGUMENTS
// class ForwardIterator
//		埋める対象の領域を指すフォワードイテレータの型
// class ValueType
//		ForwardIterator の指す、埋めるデータの型
//
// NOTES
// この関数はある領域を指定した数だけひとつの値で埋める一般的なアルゴリズムを
// 使うときに用いる。
//
// ARGUMENTS
// ForwardIterator first
//		データを埋める領域の開始位置を指すイテレータ
// ModSize number
//		埋めるデータの数
// const ValueType& value
//		埋めるデータへの参照
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class ForwardIterator, class ValueType>
inline
void
ModFillWithNumber(ForwardIterator first, ModSize n, const ValueType& value)
{
	for (; n--; ++first)
		*first = value;
}

//---------------
// 削除
//---------------
//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModRemove -- 削除する
//
// TEMPLATE ARGUMENTS
// class ForwardIterator
//		削除対象となるオブジェクトを指すフォワードイテレータの型
// class ValueType
//		削除対象となるデータの型
//
// NOTES
// この関数はある領域から指定したデータと一致するものを削除するために用いる。
//
// ARGUMENTS
// ForwardIterator first
//		削除対象となる配列の開始位置を指すイテレータ
// ForwardIterator last
//		削除対象となる配列の終了位置を指すイテレータ
// const ValueType& value
//		削除の比較対象となるデータへの参照。
//		配列上のデータをこの引数として使うと期待の結果が得られないことがある。
//
// RETURN
// 削除した結果残った配列の終了位置を指すイテレータを返す。
//
// EXCEPTIONS
// なし
//

template <class ForwardIterator, class ValueType>
inline
ForwardIterator
ModRemove(ForwardIterator first, ForwardIterator last, const ValueType& value)
{
	while (first != last && !(*first == value)) {
		++first;
	}
	if (first == last) {
		return first;
	}

	ForwardIterator next(first);
	for (++next; next != last; ++next) {
		if (!(*next == value)) {
			*first = *next;
			++first;
		}
	}
	return first;
}

//------------------
// フィルタリング
//------------------
//--------------------
// 下請け関数
//--------------------
//
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModFindAdjacent -- 重複を見つける
//
// TEMPLATE ARGUMENTS
// class InputIterator
//		処理対象となるデータを指す入力イテレータの型
//
// NOTES
// この関数はデータ配列から同一のデータが複数存在するものを探すために用いる。
//
// ARGUMENTS
// InputIterator first
//		対象となるデータ配列の開始位置を指すイテレータ
// InputIterator last
//		対象となるデータ配列の終了位置を指すイテレータ
//
// RETURN
// 同一のデータが複数存在するもののうちもっとも first に近いものを指す
// イテレータを返す。
// 該当するデータが存在しない場合は last を返す。
//
// EXCEPTIONS
// なし
//

template <class InputIterator>
InputIterator
ModFindAdjacent(InputIterator first, InputIterator last)
{
	if (first == last) {
		return last;
	}

	InputIterator next(first);
	for (++next; next != last; ++next) {
		if (*first == *next) {
			return first;
		}
		first = next;
	}
	return last;
}

//--------------------
// 公開関数
//--------------------
//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModUnique -- 重複した要素をひとつにする
//
// TEMPLATE ARGUMENTS
// class ForwardIterator
//		unique処理を行なう配列上を走査するフォワードイテレータの型
//
// NOTES
// この関数は与えられた配列から重複する要素を除くのに用いる。
//
// ARGUMENTS
// ForwardIterator first
//		unique処理を行なう配列の開始位置を指すイテレータ
// ForwardIterator last
//		unique処理を行なう配列の終了位置を指すイテレータ
//
// RETURN
// 処理が終わった配列の終了位置を指すイテレータを返す。
//
// EXCEPTIONS
// なし
//

template <class ForwardIterator>
inline
ForwardIterator
ModUnique(ForwardIterator first, ForwardIterator last)
{
	first = ModFindAdjacent(first, last);
	if (first == last) {
		return first;
	}
	ForwardIterator result(first);
	while (++first != last) {
		if (!(*result == *first)) {
			*++result = *first;
		}
	}
	return ++result;
}

//----------------
// 探索
//----------------
//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModLowerBoundWithint -- 下限を探索(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		探索の対象となるオブジェクトを指すランダムアクセスイテレータの型
// class Compare
//		比較関数を提供するクラス
// class ValueType
//		探索するデータの型
//
// NOTES
// この関数はソートされているデータ配列に対して指定した値を超えない
// データのうちもっとも後ろにあるものを得るのに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		探索するデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		探索するデータ配列の終了位置を指すイテレータ
// const ValueType& value
//		閾値となるデータへの参照
// Compare compare
//		比較関数
//
// RETURN
// value を超えないデータのうちもっとも後ろにあるものを指すイテレータを返す。
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class Compare, class ValueType>
inline
RandomAccessIterator
ModLowerBound(RandomAccessIterator first, RandomAccessIterator last,
			  const ValueType& value, Compare compare)
{
	int length = (int)(last - first);
	while (length > 0) {
		int						halfIndex = length / 2;
		RandomAccessIterator	middle(first);
		middle += halfIndex;

		if (compare(*middle, value)) {
			first = middle;
			++first;
			length = length - halfIndex - 1;
		} else {
			length = halfIndex;
		}
	}
	return first;
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModLowerBound -- 下限を探索
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		探索の対象となるオブジェクトを指すランダムアクセスイテレータの型
// class ValueType
//		探索するデータの型
//
// NOTES
// この関数はソートされているデータ配列に対して指定した値を超えない
// データのうちもっとも後ろにあるものを得るのに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		探索するデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		探索するデータ配列の終了位置を指すイテレータ
// const ValueType& value
//		閾値となるデータへの参照
//
// RETURN
// value を超えないデータのうちもっとも後ろにあるものを指すイテレータを返す。
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class ValueType>
inline
RandomAccessIterator
ModLowerBound(RandomAccessIterator first, RandomAccessIterator last,
			  ValueType value)
{
	return ModLowerBound(first, last, value, ModLess<ValueType>());
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModUpperBound -- 上限を探索(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		探索の対象となるオブジェクトを指すランダムアクセスイテレータの型
// class Compare
//		比較関数を提供するクラス
// class ValueType
//		探索するデータの型
//
// NOTES
// この関数はソートされているデータ配列に対して指定した値以下の
// データのうちもっとも前にあるものを得るのに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		探索するデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		探索するデータ配列の終了位置を指すイテレータ
// const ValueType& value
//		閾値となるデータへの参照
// Compare compare
//		比較関数
//
// RETURN
// value 以下のデータのうちもっとも前にあるものを指すイテレータを返す。
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class Compare, class ValueType>
inline
RandomAccessIterator
ModUpperBound(RandomAccessIterator first, RandomAccessIterator last,
			  ValueType value, Compare compare)
{
	int length = (int)(last - first);
	while (length > 0) {
		int						halfIndex = length / 2;
		RandomAccessIterator	middle(first);
		middle += halfIndex;

		if (compare(value, *middle)) {
			length = halfIndex;
		} else {
			first = middle;
			++first;
			length = length - halfIndex - 1;
		}
	}
	return first;
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModUpperBound -- 上限を探索
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		探索の対象となるオブジェクトを指すランダムアクセスイテレータの型
// class ValueType
//		探索するデータの型
//
// NOTES
// この関数はソートされているデータ配列に対して指定した値以下の
// データのうちもっとも前にあるものを得るのに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		探索するデータ配列の開始位置を指すイテレータ
// RandomAccessIterator last
//		探索するデータ配列の終了位置を指すイテレータ
// const ValueType& value
//		閾値となるデータへの参照
//
// RETURN
// value 以下のデータのうちもっとも前にあるものを指すイテレータを返す。
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class ValueType>
inline
RandomAccessIterator
ModUpperBound(RandomAccessIterator first, RandomAccessIterator last,
			  ValueType value)
{
	return ModUpperBound(first, last, value, ModLess<ValueType>());
}


//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModBinarySearch -- 二分探索(比較関数つき)
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		探索の対象となるオブジェクトを指すランダムアクセスイテレータの型
// class ValueType
//		探索するデータの型
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数は二分探索によって指定したデータが存在するかの判定を行なう
// 一般的なアルゴリズムを使うときに用いる。
//
// ARGUMENTS
// RandomAccessIterator first
//		探索する対象データの開始位置を指すイテレータ
// RandomAccessIterator last
//		探索する対象データの終了位置を指すイテレータ
// const ValueType& value
//		探索するデータへの参照
// Compare compare
//		比較関数
//
// RETURN
// value と一致するデータがある場合は ModTrue を、ない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class ValueType, class Compare>
inline
ModBoolean
ModBinarySearch(RandomAccessIterator first, RandomAccessIterator last,
				const ValueType& value, Compare compare)
{
	RandomAccessIterator bound(ModLowerBound(first, last, value, compare));
	return (bound != last && !compare(value, *bound)) ? ModTrue : ModFalse;
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModBinarySearch -- 二分探索
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		探索の対象となるオブジェクトを指すランダムアクセスイテレータの型
// class ValueType
//		探索するデータの型
//
// NOTES
// この関数は二分探索によって指定したデータが存在するかの判定を行なう
// 一般的なアルゴリズムを使うときに用いる。
// 比較関数は ModLess<ValueType>() が使用される。
//
// ARGUMENTS
// RandomAccessIterator first
//		探索する対象データの開始位置を指すイテレータ
// RandomAccessIterator last
//		探索する対象データの終了位置を指すイテレータ
// const ValueType& value
//		探索するデータへの参照
//
// RETURN
// value と一致するデータがある場合は ModTrue を、ない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

template <class RandomAccessIterator, class ValueType>
inline
ModBoolean
ModBinarySearch(RandomAccessIterator first, RandomAccessIterator last,
				ValueType value)
{
	return ModBinarySearch(first, last, value, ModLess<ValueType>());
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModMerge -- マージ
//
// TEMPLATE ARGUMENTS
// class InputIterator
//		処理対象となるデータを指す入力イテレータの型
// class OutputIterator
//		処理結果を出力する場所を指す出力イテレータの型
//
// NOTES
// この関数はソート済みの2つの列をマージして1つのソート済みの列を
// 作成するために用いる。
//
// ARGUMENTS
// InputIterator first1
//		マージされる列の先頭を指すイテレータ
// InputIterator last1
//		マージされる列の終了位置を指すイテレータ
// InputIterator first2
//		マージされるもう一つの列の先頭を指すイテレータ
// InputIterator last2
//		マージされるもう一つの列の終了位置を指すイテレータ
// OutputIterator result
//		マージした結果を出力する列の先頭を指すイテレータ
//
// RETURN
// 出力した列の終了位置を指すイテレータを返す.
//
// EXCEPTIONS
// なし
//

template <class InputIterator, class OutputIterator>
inline
OutputIterator
ModMerge(InputIterator first1, InputIterator last1,
		 InputIterator first2, InputIterator last2,
		 OutputIterator result)
{
	for (; first1 != last1 && first2 != last2; ++result)
		if (*first2 < *first1) {
			*result = *first2;
			++first2;
		} else {
			*result = *first1;
			++first1;
		}

	return ModCopy(first2, last2, ModCopy(first1, last1, result));
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModMerge -- マージ
//
// TEMPLATE ARGUMENTS
// class InputIterator
//		処理対象となるデータを指す入力イテレータの型
// class OutputIterator
//		処理結果を出力する場所を指す出力イテレータの型
// class Compare
//		比較関数を提供するクラス
//
// NOTES
// この関数はソート済みの2つの列をマージして1つのソート済みの列を
// 作成するために用いる。
//
// ARGUMENTS
// InputIterator first1
//		マージされる列の先頭を指すイテレータ
// InputIterator last1
//		マージされる列の終了位置を指すイテレータ
// InputIterator first2
//		マージされるもう一つの列の先頭を指すイテレータ
// InputIterator last2
//		マージされるもう一つの列の終了位置を指すイテレータ
// OutputIterator result
//		マージした結果を出力する列の先頭を指すイテレータ
// Compare compare
//		比較関数
//
// RETURN
// 出力した列の終了位置を指すイテレータを返す.
//
// EXCEPTIONS
// なし
//

template <class InputIterator, class OutputIterator, class Compare>
inline
OutputIterator
ModMerge(InputIterator first1, InputIterator last1,
		 InputIterator first2, InputIterator last2,
		 OutputIterator result, Compare compare)
{
	for (; first1 != last1 && first2 != last2; ++result)
		if (compare(*first2, *first1)) {
			*result = *first2;
			++first2;
		} else {
			*result = *first1;
			++first1;
		}

	return ModCopy(first2, last2, ModCopy(first1, last1, result));
}

///////////////////////////////////////////////////////////////////////////////

//////////////////////////
//	 最大、最小値取得	//
//////////////////////////

//	TEMPLATE FUNCTION
//	ModMax -- 2 つの値の大きいほうを得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			比較する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&			l
//			r と比較する値
//		ValueType&			r
//			l と比較する値
//		Compare				compare
//			指定されたとき
//				2 つの値を引数とする
//				関数 compare(l, r) で比較します
//				この関数は l より r  が大きいときのみ、
//				真になる値を返す必要があります
//			指定されないとき
//				2 つの値を引数とする
//				関数 operator <(l, r) で比較します
//
//	RETURN
//		大きいほうの値、ただし、両方が等しいときは l
//
//	EXCEPTIONS

template <class ValueType>
inline
const ValueType&
ModMax(const ValueType& l, const ValueType& r)
{
	return (l < r) ? r : l;
}

template <class ValueType, class Compare>
inline
const ValueType&
ModMax(const ValueType& l, const ValueType& r, Compare compare)
{
	return (compare(l, r)) ? r : l;
}

//	TEMPLATE FUNCTION
//	ModMin -- 2 つの値の小さいほうを得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			比較する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&			l
//			r と比較する値
//		ValueType&			r
//			l と比較する値
//		Compare				compare
//			指定されたとき
//				2 つの値を引数とする
//				関数 compare(l, r) で比較します
//				この関数は l より r  が大きいときのみ、
//				真になる値を返す必要があります
//			指定されないとき
//				2 つの値を引数とする
//				関数 operator <(l, r) で比較します
//
//	RETURN
//		小さいほうの値、ただし、両方が等しいときは l
//
//	EXCEPTIONS

template <class ValueType>
inline
const ValueType&
ModMin(const ValueType& l, const ValueType& r)
{
	return (r < l) ? r : l;
}

template <class ValueType, class Compare>
inline
const ValueType&
ModMin(const ValueType& l, const ValueType& r, Compare compare)
{
	return (compare(r, l)) ? r : l;
}

//	TEMPLATE FUNCTION
//	ModMaxElement -- ある範囲のシーケンス中から最大値を得る
//
//	TEMPLATE ARGUMENTS
//		class ForwardIterator
//			前へひとつづつ進めることのできる反復子
//		class Compare
//			比較に使用する関数
//
//	NOTES
//		シーケンス中の先頭から、シーケンス中の値数 - 1 回比較を行い、
//		シーケンス中の最大値を求める
//		シーケンス中に最大値が複数あるときは、
//		シーケンスの先頭に近いものを得る
//
//	ARGUMENTS
//		ForwardIterator		first
//			シーケンスの先頭を指す反復子
//		ForwardIterator		last
//			シーケンスの末尾のひとつ次の位置を指す反復子
//		Compare				compare
//			指定されたとき
//				シーケンス中の 2 つの値を引数とする
//				関数 compare(l, r) で比較します
//				この関数は l より r  が大きいときのみ、
//				真になる値を返す必要があります
//			指定されないとき
//				シーケンス中の 2 つの値を引数とする
//				関数 operator <(l, r) で比較します
//
//	RETURN
//		最大値の位置を指す反復子
//
//	EXCEPTIONS

template <class ForwardIterator>
inline
ForwardIterator
ModMaxElement(ForwardIterator first, ForwardIterator last)
{
	ForwardIterator		max = first;
	if (first != last)
		while (++first != last)
			if (*max < *first)
				max = first;
	return max;
}

template <class ForwardIterator, class Compare>
inline
ForwardIterator
ModMaxElement(ForwardIterator first, ForwardIterator last, Compare compare)
{
	ForwardIterator		max = first;
	if (first != last)
		while (++first != last)
			if (compare(*max, *first))
				max = first;
	return max;
}

//	TEMPLATE FUNCTION
//	ModMinElement -- ある範囲のシーケンス中から最小のものを得る
//
//	TEMPLATE ARGUMENTS
//		class ForwardIterator
//			前へひとつづつ進めることのできる反復子
//		class Compare
//			比較に使用する関数
//
//	NOTES
//		シーケンス中の先頭から、シーケンス中の値数 - 1 回比較を行い、
//		シーケンス中の最小値を求める
//		シーケンス中に最小のものが複数あるときは、
//		シーケンスの先頭に近いもののほうを得る
//
//	ARGUMENTS
//		ForwardIterator		first
//			シーケンスの先頭を指す反復子
//		ForwardIterator		last
//			シーケンスの末尾のひとつ次の位置を指す反復子
//		Compare				compare
//			指定されたとき
//				シーケンス中の 2 つの値を引数とする
//				関数 compare(l, r) で比較します
//				この関数は l より r  が大きいときのみ、
//				真になる値を返す必要があります
//			指定されないとき
//				シーケンス中の 2 つの値を引数とする
//				関数 operator <(l, r) で比較します
//
//
//	RETURN
//		最小のものの位置を指す反復子
//
//	EXCEPTIONS

template <class ForwardIterator>
inline
ForwardIterator
ModMinElement(ForwardIterator first, ForwardIterator last)
{
	ForwardIterator		min = first;
	if (first != last)
		while (++first != last)
			if (*first < *min)
				min = first;
	return min;
}

template <class ForwardIterator, class Compare>
inline
ForwardIterator
ModMinElement(ForwardIterator first, ForwardIterator last, Compare compare)
{
	ForwardIterator		min = first;
	if (first != last)
		while (++first != last)
			if (compare(*first, *min))
				min = first;
	return min;
}

///////////////////////////////////////////////////////////////////////////////

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModByteToKbyte -- バイトからキロバイトへの変換
//
// TEMPLATE ARGUMENTS
// class ValueType
//		対象となるオブジェクトの型
//
// NOTES
// 	この関数はバイト単位の値をキロバイト単位に変換するために用いる。
//	1024で割った値が返される。
//
// ARGUMENTS
// const ValueType byte
//		バイト単位の値
//
// RETURN
// byte をキロバイト単位の値にした値
//
// EXCEPTIONS
// なし
//

template <class ValueType>
inline
const ValueType
ModByteToKbyte(const ValueType byte)
{
	return byte >> 10;
}

//
// TEMPLATE FUNCTION
// [GenericAlgorithm]::ModKbyteToByte -- キロバイトからバイトへの変換
//
// TEMPLATE ARGUMENTS
// class ValueType
//		対象となるオブジェクトの型
//
// NOTES
// 	この関数はキロバイト単位の値をバイト単位に変換するために用いる。
//	1024をかけた値が返される。オーバーフローチェックはされない。
//
// ARGUMENTS
// const ValueType kbyte
//		キロバイト単位の値
//
// RETURN
// kbyte をバイト単位に換算した値
//
// EXCEPTIONS
// なし
//

template <class ValueType>
inline
const ValueType
ModKbyteToByte(const ValueType kbyte)
{
	return kbyte << 10;
}

#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModPartialSortWithValueType -- 
//					Partial sort function for case the Value Type should be inputted. 
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		Random Access Iterator of input 
// class Compare
//		compare function
// class ValueType
//		Type of value
//
// NOTES
// 
// ARGUMENTS
// RandomAccessIterator first
//		First iterator
// RandomAccessIterator middle
//		Data from first to middle will be sorted
// RandomAccessIterator last
//		Last iterator
// Compare compare
// 		Compare function
// ValueType* dummyValue,
//		A dummy to indicate value type
//
// RETURN
// void
//
// EXCEPTIONS
// no
//
template <class RandomAccessIterator, class Compare, class ValueType>
inline
void
ModPartialSortWithValueType(RandomAccessIterator first, 
				RandomAccessIterator middle,
				RandomAccessIterator last,
				ValueType* dummyValue,
				Compare compare
			)
{
	ModMakeHeap(first, middle, compare);
	
	for(RandomAccessIterator i = middle; i < last; ++i)
	{
		if(compare(*i, *first))
		{
			ValueType tmpValue = *i;
			*i = *first; 
			ModAdjustHeap(first, int(0), int(middle - first),
						tmpValue, compare);
		}		
	}	
	
	ModSortHeap(first, middle, compare);
}

// TEMPLATE FUNCTION private
// [GenericAlgorithm]::ModPartialSortWithValueType -- 
//					Partial sort function for case the Value Type should be inputted. 
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		Random Access Iterator of input 
// class Compare
//		compare function
// class ValueType
//		Type of value
//
// NOTES
// 
// ARGUMENTS
// RandomAccessIterator first
//		First iterator
// RandomAccessIterator middle
//		Data from first to middle will be sorted
// RandomAccessIterator last
//		Last iterator
// ValueType* dummyValue,
//		A dummy to indicate value type
//
// RETURN
// void
//
// EXCEPTIONS
// no
//
template <class RandomAccessIterator, class Compare, class ValueType>
inline
void
ModPartialSortWithValueType(RandomAccessIterator first, 
				RandomAccessIterator middle,
				RandomAccessIterator last,
				ValueType* dummyValue)
{
	ModPartialSortWithValueType(first, middle, last, dummyValue,ModLess<ValueType>());

}

#endif

// TEMPLATE FUNCTION 
// [GenericAlgorithm]::ModPartialSort -- 
//					Partial sort function to ensure top k element is sorted. 
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		Random Access Iterator of input 
// class Compare
//		compare function
//
// NOTES
// For first, middle, last, following condition must be satisfied.
//	first <= middle <= last
// 
// ARGUMENTS
// RandomAccessIterator first
//		First iterator
// RandomAccessIterator middle
//		Data from first to middle will be sorted
// RandomAccessIterator last
//		Last iterator
// Compare compare
// 		Compare function
//
// RETURN
// void
//
// EXCEPTIONS
// no
//
template <class RandomAccessIterator, class Compare>
inline
void
ModPartialSort(RandomAccessIterator first, 
				RandomAccessIterator middle,
				RandomAccessIterator last,
				Compare compare
			)
{
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	ModMakeHeap(first, middle, compare);
	
	for(RandomAccessIterator i = middle; i < last; ++i)
	{
		if(compare(*i, *first))
		{
			ModTypename RandomAccessIterator::ValueType tmpValue = *i;
			*i = *first; 
			ModAdjustHeap(first, int(0), int(middle - first),
						tmpValue, compare);
		}		
	}	
	
	ModSortHeap(first, middle, compare);
#else
	ModPartialSortWithValueType(first, middle, last, ModValueType(first), compare);
#endif

}

// TEMPLATE FUNCTION 
// [GenericAlgorithm]::ModPartialSort -- 
//					Partial sort function to ensure top k element is sorted. 
//
// TEMPLATE ARGUMENTS
// class RandomAccessIterator
//		Random Access Iterator of input 
//
// NOTES
// For first, middle, last, following condition must be satisfied.
//	first <= middle <= last
//
// ARGUMENTS
// RandomAccessIterator first
//		First iterator
// RandomAccessIterator middle
//		Data from first to middle will be sorted
// RandomAccessIterator last
//		Last iterator
//
// RETURN
// void
//
// EXCEPTIONS
// no
//
template <class RandomAccessIterator>
inline
void
ModPartialSort(RandomAccessIterator first, 
		RandomAccessIterator middle,
		RandomAccessIterator last
		)
{
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	ModPartialSort(first, middle, last, 
						ModLess<ModTypename RandomAccessIterator::ValueType>());
#else
	ModPartialSortWithValueType(first, middle, last, ModValueType(first));
#endif

}

#endif	// __ModAlgorithm_H__

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
