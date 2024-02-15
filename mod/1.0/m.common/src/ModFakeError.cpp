// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModFakeError.cpp -- 疑似エラー発生関連のメンバ定義
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


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "ModFakeError.h"
#include "ModDefaultManager.h"
#include "ModAutoMutex.h"
#include "ModCommonMutex.h"
#include "ModParameter.h"

//
// VARIABLE
// ModFakeError::specString -- パラメータや環境変数で設定される仕様を保存
//
// NOTES
// パラメータや環境変数で設定される仕様を保存するためのバッファ
//
char ModFakeError::specString[1024];

//
// VARIABLE
// ModFakeError::spec -- 疑似エラーの仕様文字列分析結果を入れる
//
// NOTES
// 疑似エラーの仕様文字列分析結果を入れるデータ構造
//
ModFakeError::Spec ModFakeError::spec[128];

//
// FUNCTION
// ModFakeError::initialize -- 疑似エラー発生クラスの初期化
//
// NOTES
// この関数は疑似エラー発生の機構を初期化するのに用いる。
// 環境変数 ModFakeErrorSpec が設定されていればそれから
// なければパラメータ ModFakeErrorSpec から、疑似エラーについての
// 仕様を読み込んで必要なデータ構造を作成する。
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
// その他
// この関数はマルチスレッド環境でないことが保証されている状況下で
// 実行される。
//
void
ModFakeError::initialize()
{
	ModParameter parameter(ModFalse);

	char* envValue = ::getenv("ModFakeErrorSpec");
	if (envValue == NULL) {
		// 環境変数がなかったのでパラメータからとる
		if (parameter.getString(ModFakeError::specString, "ModFakeErrorSpec")
			== ModFalse) {
			// パラメータにもなかったのでこの機構は使わない
			ModFakeError::specString[0] = '\0';
			return;
		}
	} else {
		::strcpy(ModFakeError::specString, envValue);
	}
	ModFakeError::parseSpec();
}

//
// FUNCTION
// ModFakeError::terminate -- 疑似エラー発生クラスの終了処理
//
// NOTES
// この関数は疑似エラー発生の機構の終了処理に用いる。
// 実際には何もしない。
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
// その他
//
void
ModFakeError::terminate()
{
}

//
// FUNCTION
// ModFakeError::check -- 疑似エラーの判定
//
// NOTES
// この関数は初期化で登録された疑似エラー仕様にしたがって
// 関数の呼びだし回数を記録し、条件を満たすかどうかの判定をする。
//
// ARGUMENTS
// const char* name_
//		判定対象の関数名
// int errno_
//		エラーと判定されたときにセットするOSエラー番号
// int defaultErrno
//		errno_が-1でspecでも指定されていないときに用いるOSエラー番号
//
// RETURN
// 条件が満たされているときに ModTrue を返す。
// 満たされていないときは ModFalse を返す。
//
// EXCEPTION
// なし
//
ModBoolean
ModFakeError::check(const char* name_, int errno_, int defaultErrno)
{
	// 変更はないはずなのでロックの外でも大丈夫
	if (ModFakeError::specString[0] == '\0') {
		// 疑似エラー使用せず
		return ModFalse;
	}

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	ModFakeError::Spec* sp = &(ModFakeError::spec[0]);
	ModBoolean returnValue = ModFalse;

	for (; sp->name != 0; ++sp) {
		if (::strcmp(name_, sp->name) != 0) continue;

		// 一致したらまず呼びだし回数を増加させる
		++(sp->count);

		// 設定されている比較方法によって比較する

		int	cmp = 0;
		switch (sp->compare) {
		case Spec::equal:
			cmp = sp->count == sp->limit;		break;
		case Spec::less:
			cmp = sp->count < sp->limit;		break;
		case Spec::greater:
			cmp = sp->count > sp->limit;		break;
		case Spec::lessEqual:
			cmp = sp->count <= sp->limit;		break;
		case Spec::greaterEqual:
			cmp = sp->count >= sp->limit;		break;
#ifdef MOD_DEBUG
		default:
			break;
#endif
		}
		if (cmp) {
			returnValue = ModTrue;
			errno =	(errno_ >= 0) ? errno_ :
				(sp->errorNumber > 0) ? sp->errorNumber : defaultErrno;
		}
	}

	if (returnValue == ModTrue) {
		ModErrorMessage << "Fake error occured for " << name_ << ModEndl;
		ModErrorMessage << "errno is set to " << errno << ModEndl;
	}
	return returnValue;
}

//
// FUNCTION
// ModFakeError::getCount -- 疑似エラーの判定に使う関数の呼びだし回数
//
// NOTES
// この関数はcheckによって記録された呼びだし回数を得るのに用いる。
//
// ARGUMENTS
// const char* name_
//		判定対象の関数名
//
// RETURN
// 呼びだし回数を返す。
//
// EXCEPTION
// なし
//
ModUInt64
ModFakeError::getCount(const char* name_)
{
	// 変更はないはずなのでロックの外でも大丈夫
	if (ModFakeError::specString[0] == '\0') {
		// 疑似エラー使用せず
		return 0;
	}

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	ModFakeError::Spec* sp = &(ModFakeError::spec[0]);

	for (; sp->name; ++sp)
		if (::strcmp(name_, sp->name) == 0)

			// 一致したら呼びだし回数を返す

			return sp->count;
	return 0;
}

//
// FUNCTION
// ModFakeError::reInitialize -- 疑似エラー発生クラスの再初期化
//
// NOTES
// この関数は疑似エラー発生の機構を再初期化するのに用いる。
// 引数の文字列が 0 でなければそれから、
// なければパラメータ ModFakeErrorSpec から、疑似エラーについての
// 仕様を読み込んで必要なデータ構造を作成する。
// 実行カウンタはリセットされる。
//
// ARGUMENTS
// const char* specString_
//		仕様文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
// その他
// この関数はMT unsafe なのでマルチスレッド環境でないことが保証されて
// いる状況下でのみ実行すること。
//
void
ModFakeError::reInitialize(const char* specString_)
{
	if (specString_ == 0) {
		ModParameter parameter(ModFalse);
		// 引数ががなかったのでパラメータからとる
		if (parameter.getString(ModFakeError::specString, "ModFakeErrorSpec")
			== ModFalse) {
			// パラメータにもなかったのでこの機構は使わない
			ModFakeError::specString[0] = '\0';
			return;
		}
	} else {
		::strcpy(ModFakeError::specString, specString_);
	}
	ModFakeError::parseSpec();
}

//
// FUNCTION private
// ModFakeError::parseSpec -- スペック文字列を解析する
//
// NOTES
// specString の内容から spec を作成する。
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
void
ModFakeError::parseSpec()
{
	// スペックの解析
	int index = 0;
	char* cp = ModFakeError::specString;
	char* cp2;
	ModFakeError::Spec* sp = &(ModFakeError::spec[0]);
	while (*cp != '\0') {
		sp->name = cp;
		sp->count = 0;
		// 関数名の終りまでポインターを移動させる
		for (; *cp != '\0' && *cp != '(' && *cp != '=' && *cp != '<' && *cp != '>' && *cp != ';'; ++cp);
		// 関数名の終りを覚えておく
		cp2 = cp;
		if (*cp == '(') {
			// errno指定あり
			++cp;
			sp->errorNumber = ModCharTrait::toInt(cp);
			for (; *cp != ')'; ++cp);
			++cp;
		} else {
			// errno指定なし
			sp->errorNumber = 0;
		}
		if (*cp == '\0' || *cp == ';') { // 回数無指定は0回以上と同じ
			sp->compare = Spec::greater;
			sp->limit = 0;
		} else {
			// 条件あり
			if (::memcmp(cp, "<=", 2) == 0 || ::memcmp(cp, "=<", 2) == 0) {
				sp->compare = Spec::lessEqual;
				cp += 2;
			} else if (::memcmp(cp, ">=", 2) == 0 || ::memcmp(cp, "=>", 2) == 0) {
				sp->compare = Spec::greaterEqual;
				cp += 2;
			} else if (::memcmp(cp, "==", 2) == 0) {
				sp->compare = Spec::equal;
				cp += 2;
			} else if (*cp == '=') {
				sp->compare = Spec::equal;
				++cp;
			} else if (*cp == '<') {
				sp->compare = Spec::less;
				++cp;
			} else if (*cp == '>') {
				sp->compare = Spec::greater;
				++cp;
			} else {
				ModErrorMessage << "Illegal ModFakeErrorSpec: "
								<< ModFakeError::specString << ModEndl;
				break;
			}

			if ((unsigned char)*cp < '0' || (unsigned char)*cp > '9') {
				ModErrorMessage << "Illegal ModFakeErrorSpec: "
								<< ModFakeError::specString << ModEndl;
				break;
			}

			sp->limit = ModCharTrait::toModFileSize(cp);

			for (; *cp != '\0' && *cp != ';'; ++cp);
		}
		
		++sp;

		if (*cp2 == '\0') break;
		else *cp2 = '\0';

		if (*cp == '\0') break;

		++cp;
	}
	sp->name = 0;
}

//
// Copyright (c) 1998, 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
