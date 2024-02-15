// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedExtendedGolombCoderArgument.h -- Ngram 分割処理器の初期化引数
// 
// Copyright (c) 1998, 1999, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedExtendedGolombCoderArgument_H__
#define __ModInvertedExtendedGolombCoderArgument_H__

#include "ModOs.h"
#include "ModSerial.h"
#include "ModMessage.h"
#include "ModOstrStream.h"
#include "ModInvertedManager.h"

//
// STRUCT
// ModInvertedExtendedGolombCoderArgument --- 拡張 Ngram 分割器の初期化引数
//
// NOTES
// 拡張 Golomb 符合化器の初期化引数。
//
struct
ModInvertedExtendedGolombCoderArgument
    : public ModSerializer, public ModInvertedObject
{
    ModInvertedExtendedGolombCoderArgument();
    ModInvertedExtendedGolombCoderArgument(
	const ModInvertedExtendedGolombCoderArgument& original);
#ifndef V1_0
	void parse(const ModString& description);
#endif
    void serialize(ModArchive& archiver);

	friend ModMessageStream& operator<<(
		ModMessageStream&,
		const ModInvertedExtendedGolombCoderArgument&);
#ifndef V1_0
	friend ModOstream& operator<<(
		ModOstream&,
		const ModInvertedExtendedGolombCoderArgument&);
#endif
    int lambda;
	int factor;
};


//
// FUNCTION
// ModInvertedExtendedGolombCoderArgument::ModInvertedExtendedGolombCoderArgument -- コンストラクタ
//
// NOTES
// コンストラクタ。
//
// ARGUMENTS
// const ModInvertedExtendedGolombCoderArgument& original
//		コピー元
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorInvalidCoderArgument	引数が不正である
//
inline
ModInvertedExtendedGolombCoderArgument::ModInvertedExtendedGolombCoderArgument()
    : lambda(0), factor(1)
{}

inline
ModInvertedExtendedGolombCoderArgument::ModInvertedExtendedGolombCoderArgument(
    const ModInvertedExtendedGolombCoderArgument& original)
    : lambda(original.lambda), factor(original.factor)
{}


//
// FUNCTION
// ModInvertedExtendedGolombCoderArgument::serialize -- シリアライズ
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
ModInvertedExtendedGolombCoderArgument::serialize(ModArchive& archiver)
{
    archiver(lambda);
    archiver(factor);
}


//
// FUNCTION
// operator<< -- 拡張 Golomb 符合化器の初期化引数のストリームへの出力演算子
//
// NOTES
// 拡張 Golomb 符合化器の初期化引数をメッセージストリーム／ストル
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
		   const ModInvertedExtendedGolombCoderArgument& argument)
{
	stream << argument.lambda << ':' << argument.factor;
	return stream;
}

#ifndef V1_0
inline ModOstream& 
operator<<(ModOstream& stream,
		   const ModInvertedExtendedGolombCoderArgument& argument)
{
	stream << argument.lambda << ':' << argument.factor;
	return stream;
}
#endif


#ifndef V1_0
//
// FUNCTION
// ModInvertedExtendedGolombCoderArgument::parse -- 記述文字列の解析
//
// NOTES
// 記述文字列を解析、それに対応した lambda, factor を設定する。
// lambda を省略した場合、lambda = 0 とする。
// factor を省略した場合、factor = 1 とする。
//
//		""			λ = 0, factor = 1 の ExtendedGolombCoder 用パラメータ	
//		"0"			λ = 0, factor = 1 の ExtendedGolombCoder 用パラメータ
//		"0:1"		λ = 0, factor = 1 の ExtendedGolombCoder 用パラメータ
//		"1:5"		λ = 1, factor = 5 の ExtendedGolombCoder 用パラメータ
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
ModInvertedExtendedGolombCoderArgument::parse(const ModString& description)
{
	ModSize count(0);
	ModSize index(0);
	ModSize length(description.getLength());
	ModString buffer;

	// デフォルトの設定
	lambda = 0;
	factor = 1;
	
	while (index <= length) {
		// スペースを記述の区切りと見なす．
		if (index == length || description[index] == ':') {
			if (buffer.getLength() == 0) {
				++index;
				continue;
			}
			int value(buffer.toInt());
			if (count == 0) {
				// lambda の指定
				if (value < 0 || 32 <= value) {
					ModErrorMessage << "description='" << description 
									<< "' is invalid: lambda=" << value
									<< ModEndl;
					ModThrowInvertedFileError(
						ModInvertedErrorInvalidCoderParameterDescription);
				}
				lambda = value;
			}
			else if (count == 1) {
				// factor の指定
				if (value < 1 || 32 <= value) {
					ModErrorMessage << "description='" << description 
									<< "' is invalid: factor=" << value
									<< ModEndl;
					ModThrowInvertedFileError(
						ModInvertedErrorInvalidCoderParameterDescription);
				}
				factor = value;
			}
			else {
				ModErrorMessage << "description='" << description 
								<< "' is invalid" << ModEndl;
				ModThrowInvertedFileError(
					ModInvertedErrorInvalidCoderParameterDescription);
			}
			buffer.clear();
			++count;
		}
		else {
			buffer += description[index];
		}
		++index;
	}
}

#endif

#endif // __ModInvertedExtendedGolombCoderArgument_H__

//
// Copyright (c) 1998, 1999, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
