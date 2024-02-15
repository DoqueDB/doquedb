// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedParameterizedExpGolombCoderArgument.h -- パラメータ付き Golomb 符合化器の初期化引数
// 
// Copyright (c) 1997, 1998, 1999, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedParameterizedExpGolombCoderArgument_H__
#define __ModInvertedParameterizedExpGolombCoderArgument_H__

#include "ModOs.h"
#include "ModSerial.h"
#include "ModMessage.h"
#include "ModOstrStream.h"
#include "ModInvertedManager.h"
#include "ModInvertedException.h"

//
// STRUCT
// ModInvertedParameterizedExpGolombCoderArgument -- パラメータ付き Golomb 符合化器の初期化引数
//
// NOTES
// パラメータ付き Golomb 符合化器の初期化引数。
//
struct
ModInvertedParameterizedExpGolombCoderArgument
    : public ModSerializer, public ModInvertedObject
{
    ModInvertedParameterizedExpGolombCoderArgument(const int lambda = 0);
    ModInvertedParameterizedExpGolombCoderArgument(
	const ModInvertedParameterizedExpGolombCoderArgument& original);
#ifndef V1_0
	void parse(const ModString& description);
#endif
    void serialize(ModArchive& archiver);

	friend ModMessageStream& operator<<(
		ModMessageStream&,
		const ModInvertedParameterizedExpGolombCoderArgument&);
#ifndef V1_0
	friend ModOstream& operator<<(
		ModOstream&,
		const ModInvertedParameterizedExpGolombCoderArgument&);
#endif

    int lambda;
};


//
// FUNCTION
// ModInvertedParameterizedExpGolombCoderArgument::ModInvertedParameterizedExpGolombCoderArgument -- コンストラクタ
//
// NOTES
// コンストラクタ。
//
// ARGUMENTS
// const int lambda_
//		パラメータ
// const ModInvertedParameterizedExpGolombCoderArgument& original
//		コピー元
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorInvalidCoderArgument	引数が不正である
//
inline
ModInvertedParameterizedExpGolombCoderArgument::ModInvertedParameterizedExpGolombCoderArgument(const int lambda_)
    : lambda(lambda_)
{
	if (lambda < 0 || 32 <= lambda) {
		ModErrorMessage << "invalid argument:" << lambda << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInvalidCoderArgument);
	}
}

inline
ModInvertedParameterizedExpGolombCoderArgument::ModInvertedParameterizedExpGolombCoderArgument(
    const ModInvertedParameterizedExpGolombCoderArgument& original)
    : lambda(original.lambda)
{
	// コピーコンストラクタでは不正な引数がくるはずがない
}


//
// FUNCTION
// ModInvertedParameterizedExpGolombCoderArgument::serialize -- シリアライズ
//
// NOTES
// シリアライズする。
//
// ARGUMENTS
// ModArchive& archiver
//		アーカイバ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void 
ModInvertedParameterizedExpGolombCoderArgument::serialize(ModArchive& archiver)
{
    archiver(lambda);
}


//
// FUNCTION
// operator<< -- パラメータ付き Golomb 符合化器の初期化引数のストリームへの出力演算子
//
// NOTES
// パラメータ付き Golomb 符合化器の初期化引数をメッセージストリーム／ストル
// ストリームへ出力する。
//
// ARGUMENTS
// ModMessageStream& stream
//		出力するメッセージストリーム
// ModOstrStream& stream
//		出力するストルストリーム
// const ModInvertedParameterizedExpGolombCoderArgument& argument
//		出力データ
//
// RETURN
// メッセージストリーム
//
// EXCEPTIONS
// なし
//
inline ModMessageStream& 
operator<<(ModMessageStream& stream,
		   const ModInvertedParameterizedExpGolombCoderArgument& argument)
{
	stream << argument.lambda;
	return stream;
}

#ifndef V1_0
inline ModOstream& 
operator<<(ModOstream& stream,
		   const ModInvertedParameterizedExpGolombCoderArgument& argument)
{
	stream << argument.lambda;
	return stream;
}
#endif


#ifndef V1_0
//
// FUNCTION
// ModInvertedParameterizedExpGolombCoderArgument::parse -- 記述文字列の解析
//
// NOTES
// 記述文字列を解析、それに対応した lambda を設定する。
// パラメータの記述は整数値（0 以上）とする。
//
//		"0"		λ = 0 の ParameterizedExpGolombCoder 用パラメータ
//		"1"		λ = 1 の ParameterizedExpGolombCoder 用パラメータ
//
// ARGUMENTS
// const ModString& description
//		記述文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorInvalidCoderParameterDescription);
//		パラメータ記述が不正である
//
inline void 
ModInvertedParameterizedExpGolombCoderArgument::parse(const ModString& description)
{
	lambda = description.toInt();

	if (lambda < 0 || 32 <= lambda) {
		ModErrorMessage << "description='" << description 
						<< "' is invalid: lambda=" << lambda << ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidCoderParameterDescription);
	}
}
#endif

#endif // __ModInvertedParameterizedExpGolombCoderArgument_H__

//
// Copyright (c) 1997, 1998, 1999, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//


