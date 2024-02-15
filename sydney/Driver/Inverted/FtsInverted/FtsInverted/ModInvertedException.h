// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
//	ModInvertedException.h --- 転置ファイルモジュールにおける例外の定義
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedException_H__
#define __ModInvertedException_H__

#include "ModError.h"

//
// MACRO
// ModThrowInvertedFileRetry -- 転置ファイル関連のRetryレベルの例外を送出する
//
// NOTES
// 転置ファイル関連のRetryレベルの例外を送出する。
//
#define	ModThrowInvertedFileRetry(n) \
	ModThrow(ModModuleInvertedFile, n, ModErrorLevelRetry)

//
// MACRO
// ModThrowInvertedFileOk -- 転置ファイル関連のOkレベルの例外を送出する
//
// NOTES
// 転置ファイル関連のOkレベルの例外を送出する。
//
#define ModThrowInvertedFileOk(n) \
	ModThrow(ModModuleInvertedFile, n, ModErrorLevelOk)

//
// MACRO
// ModThrowInvertedFileWarning -- 転置ファイル関連のWarningレベルの例外を送出する
//
// NOTES
// 転置ファイル関連のWarningレベルの例外を送出する。
//
#define ModThrowInvertedFileWarning(n) \
	ModThrow(ModModuleInvertedFile, n, ModErrorLevelWarning)

//
// MACRO
// ModThrowInvertedFileError -- 転置ファイル関連のErrorレベルの例外を送出する
//
// NOTES
// 転置ファイル関連のErrorレベルの例外を送出する。
//
#define ModThrowInvertedFileError(n) \
	ModThrow(ModModuleInvertedFile, n, ModErrorLevelError)

//
// MACRO
// ModThrowInvertedFileFatal -- 転置ファイル関連のFatalレベルの例外を送出する
//
// NOTES
// 転置ファイル関連のFatalレベルの例外を送出する。
//
#define ModThrowInvertedFileFatal(n) \
	ModThrow(ModModuleInvertedFile, n, ModErrorLevelFatal)

//
// MACRO
// ModThrowInvertedFile -- 転置ファイル関連の例外を送出する
//
// NOTES
// 転置ファイル関連の例外を送出する。
//
#define	ModThrowInvertedFile(n, e) \
	ModThrow(ModModuleInvertedFile, n, e.getErrorLevel())

//
// MACRO
// ModRethrowInvertedFile -- 転置ファイル関連の例外を再送出する
//
// NOTES
// 転置ファイル関連の例外を再送出する。
//
#define ModRethrowInvertedFile(e) \
	ModRethrow(e)


//
// MACRO
// ModRethrowFatal -- Fatal レベルの例外を再送出する
//
// NOTES
// Fatal レベルの例外であれば再送出する。
//
#define ModRethrowFatal(e) \
	if (e.getErrorLevel() == ModErrorLevelFatal) {						\
		ModRethrow(e);													\
	}


//
// ENUM
// ModInvertedErrorNumber -- メモリ管理モジュール専用のエラー番号
//
// NOTES
// 転置ファイルモジュール(ModModuleInverted)で利用するエラー番号である。
//
enum ModInvertedErrorNumber {
	ModInvertedErrorInternal						= 52000,
										// 内部的な不整合
	ModInvertedErrorInvalidPageSize					/* = 52001*/,
										// 不正なページサイズ
	ModInvertedErrorInvalidMaxFileSize				/* = 52002*/,
										// 不正なファイル最大サイズ
	ModInvertedErrorFileOverflown					/* = 52003*/,
										// ファイルがあふれた
	ModInvertedErrorFileBroken						/* = 52004*/,
										// ファイルが壊れた
	ModInvertedErrorNotInvertedFile					/* = 52005*/,
										// 与えられたファイルは転置ファイルでない
	ModInvertedErrorInvalidDocumentID				/* = 52006*/,
										// 不正な文書ID
	ModInvertedErrorTooLongIndexKey					/* = 52007*/,
										// 索引語が長すぎる
	ModInvertedErrorFileAttached					/* = 52008*/,
										// 転置ファイルがアタッチされている
	ModInvertedErrorLogInsertFail					/* = 52009*/,
										// ログの挿入に失敗
	ModInvertedErrorLogGetFail						/* = 52010*/,
										// ログの取得に失敗
	ModInvertedErrorInvalidLog						/* = 52011*/,
										// 不正な論理ログ
	ModInvertedErrorInvalidVersion					/* = 52012*/,
										// 不正な転置ファイルバージョン
	ModInvertedErrorBatchInsertNotInitialized		/* = 52013*/,
										// バッチ挿入が初期化されていない
	ModInvertedErrorBatchInsertNotTerminated		/* = 52014*/,
										// バッチ挿入が終了していない
	ModInvertedErrorBatchExpungeNotInitialized		/* = 52015*/,
										// バッチ削除が初期化されていない
	ModInvertedErrorBatchExpungeNotTerminated		/* = 52016*/,
										// バッチ削除が終了していない
	ModInvertedErrorInvalidOverflowFileNum			/* = 52017*/,
										// 不正なオーバーフローファイル数
	ModInvertedErrorInvalidIdBlockUnitNum			/* = 52018*/,
										// 不正なIDブロックユニット数
	ModInvertedErrorNotSupported					/* = 52019*/,
										// 実装されていない機能
	ModInvertedErrorInvalidMatchMode				/* = 52020*/,
										// 不正な照合モード（検索単位指定）
	ModInvertedErrorInvalidIndexingType				/* = 52021*/,
										// 不正な索引付けタイプ
	ModInvertedErrorInvalidLocChunkUnitNum			/* = 52022*/,
										// 不正なLOCチャンクユニット数
	ModInvertedErrorMergeNotTerminated				/* = 52023*/,
										// マージが終了していない

	// ページマネージャ関連
	ModInvertedErrorPageManagerPageNotAttached		/* = 52024*/,
										// 指定されたページはアタッチされていない
	ModInvertedErrorPageManagerPageAttached			/* = 52025*/,
										// 指定されたページはアタッチされている

	// Ｂ木ファイル関連
	ModInvertedErrorBtreeFileKeyNotFound			= 52100,
										// キーが見つからない
	ModInvertedErrorBtreeFilePageIDNotCorrect		/* = 52101*/,
										// キーとページIDが一致しない

	// リーフファイル関連
	ModInvertedErrorLeafFileInvalidReconfigureParamater	= 52200,
										// 不正なページ分割パラメータ

	// オーバーフローファイル関連 -- 現在はなし

	// トークナイザ（テキスト分割器）関連
	ModInvertedErrorInvalidTokenizerType			= 52400,
										// 不正なタイプ
	ModInvertedErrorInvalidTokenizerArgument		/*= 52401*/,
										// 不正な初期化引数
	ModInvertedErrorInvalidTokenizerParameterDescription /*= 52402*/,
										// 不正なパラメータ記述
	ModInvertedErrorAmiFail							/*= 52403*/,
										// ami が失敗した
	ModInvertedErrorGetNormalizerFail				/* = 52404*/,
										// 正規化器の取得に失敗した
	ModInvertedErrorNormalizerFail					/* = 52405*/,
										// 正規化器が失敗した
	ModInvertedErrorInvalidNormalizeMode			/* = 52406*/,
										// 不正な正規化指定
	ModInvertedErrorGetStemmerFail					/* = 52407*/,
										// ステマーの取得に失敗した
	ModInvertedErrorGetAnalyzerFail					/* = 52408*/,
										// 解析器の取得に失敗した
	ModInvertedErrorInvalidLanguageSetNum			/* = 52409*/,
										// セクションの数と言語セットの数が異なる

	// 符合化器関連
	ModInvertedErrorInvalidCoderType				= 52500,
										// 不正なタイプ
	ModInvertedErrorInvalidCoderArgument			/*= 52501*/,
										// 不正な初期化引数
	ModInvertedErrorInvalidCoderParameterDescription /*= 52502*/,
										// 不正なパラメータ記述

	// 転置リスト／転置リスト反復子関連
	ModInvertedErrorLongListNoSpace					= 52600,
										// ShortList に戻して処理する場合
										// （内部処理用で、外部には送出しない）
	
	
	// 位置リスト反復子関連 -- 現在はなし
	
	// 検索関連
	ModInvertedErrorQueryValidateFail				= 52800,
										// 問い合わせ変換に失敗
	ModInvertedErrorRetrieveFail					/* = 52801*/,
										// 検索に失敗
	ModInvertedErrorInvalidScoreCalculator			/* = 52802*/,
										// 不正なスコア計算器
	ModInvertedErrorInvalidScoreCalculatorParameter	/* = 52803*/,
										// 不正なスコア計算器パラメータ
	ModInvertedErrorInvalidScoreCombiner			/* = 52804*/,
										// 不正なスコア合成器
	ModInvertedErrorInvalidScoreCombinerParameter	/* = 52805*/,
										// 不正なスコア合成器パラメータ
	ModInvertedErrorInvalidScoreNegator				/* = 52806*/,
										// 不正なスコア否定器
	ModInvertedErrorInvalidScoreNegatorParameter	/* = 52807*/,
										// 不正なスコア否定器パラメータ

	// 最後におくこと -- 実際には未使用
	ModInvertedErrorEnd								= 52999
};


//
// エラーメッセージのための定義
//
#ifndef SYD_INVERTED

#include "ModExceptionMessage.h"
extern ModExceptionMessageAssoc ModModuleInvertedFileMessageArray[];
static ModExceptionMessage invertedMessage(ModModuleInvertedFile,
										   ModModuleInvertedFileMessageArray);

#endif

#endif // __ModInvertedException_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
