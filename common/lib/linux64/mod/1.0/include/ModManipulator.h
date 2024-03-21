// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModManipulator.h -- メッセージ記録用共通ライブラリ
// 
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __ModManipulator_H__
#define __ModManipulator_H__

#include "ModCommonDLL.h"

class ModMessageStream;

//
// CLASS
// ModManipulator -- 各種ストリームの引数付マニピュレーターのためのクラス
//
// NOTES
// このクラスはストリームに対する引数付のマニピュレーターが返す型として
// 用いる。
// 

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModManipulator
{
public:
	typedef ModMessageStream& (*Manipulator)(ModMessageStream&, int);

	// コンストラクター
	ModManipulator(Manipulator function_, int argument_);
	// デストラクター
	~ModManipulator()
	{}

	friend ModCommonDLL ModMessageStream&
		operator <<(ModMessageStream& stream, ModManipulator& manipulator);

public:
	Manipulator function;
	int argument;
};

//
// FUNCTION
// ModManipulator::ModManipulator -- コンストラクタ
//
// NOTES
// ModManipulator の唯一のコンストラクタ。実際のマニピュレーター関数と
// それに与える引数をセットする。
//
// ARGUMENTS
// Manipulator function_
//		実際に動作するマニピュレーター
// int argument_
//		function_ にストリームとともに与える引数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModManipulator::ModManipulator(Manipulator function_, int argument_) :
	function(function_), argument(argument_)
{}

#endif	// __ModManipulator_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
