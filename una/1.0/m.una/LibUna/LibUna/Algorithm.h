// - * - Mode: C++; tab-width: 4; c-basic-offset: 4;- * -
// vi:set ts=4 sw=4:
//
// Algorithm.h -- Algorithm の定義ファイル
// 
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ALGORITHM__HEADER__
#define __ALGORITHM__HEADER__

#include "Type.h"
#include "ModAlgorithm.h"
#include "ModTypes.h"

namespace UNA {

class Morph;
class Bitset;
	
//
// namespace
// Algorithm -- 内部で使用するアルゴリズム関数郡
//
namespace Algorithm {

	//
	// FUNCTION public
	//	Algorithm::findMorphPos
	//		-- 形態素取得
	//
	// NOTES
	//		Common::Morph に特化した関数。
	//		n_ 文字目の形態素を取得する
	//
	// ARGUMENTS
	//		M start_
	//			検索開始形態素
	//		M end_
	//			検索終了形態素
	//		ModSize n_
	//			検索する文字長さ
	//
	// RETURN
	// 		M	n_ 文字目が含まれている形態素
	//
	// EXCEPTIONS
	//
	template < class M >
	M
	findStringPosInMorph(M start_, M end_, ModSize n_)
	{
		ModSize sum = 0;
		for ( M tgt = start_; tgt != end_; ++tgt ) {
			if ( sum <= n_ && n_ < sum + (*tgt).getNormLen() ) {
				return tgt;
			}
			sum += (*tgt).getNormLen();
		}
		return end_;
	}

	//
	// FUNCTION public
	//	Algorithm::IsInclude
	//		--	TypeRange<T> の範囲内に T が存在するかどうかを調べるテンプレート関数
	//
	// NOTES
	//	T と TypeRange<T>._len の和算を行い、TypeRange<T>の範囲を特定するため、
	//	TypeRange<T>の T は、ポインタ型であることを必要条件とする。
	//
	// ARGUMENTS
	//	Type::Range<T> range_
	//		TypeRange<T>型のインスタンス
	//	T item_
	//		調査対象のT型オブジェクト
	//
	// RETURN
	//	ModBoolean
	//		存在する場合	: ModTrue
	//		存在しない場合	: ModFalse
	//
	// EXCEPTIONS
	//
	template < class T >
	//inline
	ModBoolean IsInclude(const Type::Range<T>& range_, const T& item_)
	{
		return (range_._start <= item_ && static_cast<T>(range_._start + range_._len) > item_) ? ModTrue : ModFalse;
	}

	//
	// CLASS
	// IsIncludeFunction -- 形態素に特化したファンクションクラス
	//
	template < class MORPH, class POS >
	class IsIncludeFunction
	{
	public:
		ModBoolean operator()(const MORPH& morph_,
							  const POS item_) const
		{
			return morph_.isIncludeCharPos(item_);
		}
	};

	//
	// FUNCTION public
	//	Algorithm::log10
	//		-- 引数の常用対数を返す。
	//
	// NOTES
	//	
	// ARGUMENTS
	//	double d
	//		計算対象の値
	//
	// RETURN
	//	double
	//		計算結果
	//
	// EXCEPTIONS
	//
	double log10(double d);

	/////////////////////////////////////////////////////////////////////////
	// CLASS
	//		Operator -- operate object
	//				主に Common::Data で利用される	
	//				ModBinaryFunction の operator() が virtual 宣言されていない為、
	//				objectで保存できない為作成した。
	//
	template < class T >
	class Operator
	{
	public:
		enum Type {
			Equal,		// Equal
			And,		// And
			Or,			// Or
			ExOr,		// Exclusive Or
			Fin
		};
		T& operator()(T& value1_, T& value2_) const
		{
			switch ( _type ) {
			case And:		value1_ &= value2_;		break;
		 // 日本語の場合のみ、Or,ExOrを使用する
			case Or:		value1_ |= value2_;		break;
			case ExOr:		value1_ ^= value2_;		break;
			default:
				;
			}
			return value1_;
		}
		Operator(Type type_) : _type(type_) {}
	private:
		Type _type;
	};

	// TYPEDEF
	//		BitsetOperator -- Bitset 用の演算子オブジェクト
	typedef		Operator<Bitset>		BitsetOperator;


	// CLASS
	//		And, Or,  -- ビット演算関数オブジェクト
	template <class T >
	class Equal : public ModBinaryFunction< T, T, T >
	{
	public:
		T& operator()(T& value1, T& value2) const
		{
			return (value1 = value2);
		}
	};
	template <class T >
	class And : public ModBinaryFunction< T, T, T >
	{
	public:
		T& operator()(T& value1, T& value2) const
		{
			return (value1 &= value2);
		}
	};

	template <class T >
	class Or : public  ModBinaryFunction< T, T, T >
	{
	public:
		T& operator()(T& value1, T& value2) const
		{
			return (value1 |= value2);
		}
	};

	template <class T >
	class ExOr : public ModBinaryFunction< T, T, T >
	{
	public:
		T& operator()(T& value1, T& value2) const
		{
			return (value1 ^= value2);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Algorithm::SubLess
	//		-- ポインタの実体同士の Less 関数	
	//
	// NOTES
	//	ValueType はポインタであること
	//
	template < class ValueType >
	class SubLess
		 : ModBinaryFunction<ValueType, ValueType, ModBoolean>
	{
	public:
		ModBoolean operator()(const ValueType& value1_,
				  const ValueType& value2_) const
		{
			return (*value1_ < *value2_) ? ModTrue : ModFalse;
		}
	};

} // end of namespace Algorithm

}

#endif // __ALGORITHM__HEADER__

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
