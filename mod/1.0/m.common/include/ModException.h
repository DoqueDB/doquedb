// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
//	ModException.h --- 	例外の定義
// 
// Copyright (c) 1997, 2002, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __ModException_H__
#define __ModException_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"

//
// モジュールは汎用OSに属する。
// したがって、エラーはModOsXXXである。
// 本来、クラスはModOsObjectのサブクラスとして作成すべきであるが、
// 例外クラスはModOsManagerよりも下位であり、お互いにインクルードし合うので
// うまくいかない。幸いModExceptionの使い方では直接newすることはないはず
// なので、特にModOsObjectのサブクラスとすることはしていない。
//

//
// TYPEDEF
// ModErrorNumber -- エラー番号型
//
// NOTES
// この型は、エラー番号を表すために用いる。
// 共通エラー番号ModCommonErrorNumberの他にモジュール別のエラー番号が
// それぞれ列挙型で定義される。それらをまとめてModErrorNumber型とする。
// 
typedef int ModErrorNumber;

//	ENUM
//	ModCommonErrorNumber -- 共通のエラー番号を表す列挙型
//
//	NOTES
//		MOD 内部で共通に利用可能なエラー番号を定義する
//		エラー番号の値の範囲は以下のようになっている
//
//		共通エラー番号
//			0 以上 ModMemoryErrorUndefined 未満
//		メモリー管理モジュール関連
//			ModMemoryErrorUndefined 以上 ModOsErrorUndefined 未満
//		汎用 OS モジュール関連
//			ModOsErrorUndefined 以上 ModModuleStandard * 1000 未満
//		各モジュール
//			ModModuleXXX * 10000 以上 その値 + 10000 未満

enum ModCommonErrorNumber
{
	ModCommonErrorUndefined			= 0,		// 初期値

	ModCommonErrorUnexpected		= 1,		// 予期しないエラー
	ModCommonErrorAssert			= 2,		// アサート条件を満たさない
	ModCommonErrorMemoryExhaust		= 3,		// システムメモリーが足りない
	ModCommonErrorBadArgument		= 4,		// 引数がおかしい
	ModCommonErrorNotInitialized 	= 5,		// 未初期化、後処理済
	ModCommonErrorOutOfRange		= 6,		// 範囲を超えている
	ModCommonErrorNotOverLoad		= 7,		// 関数が未オーバーロード
	ModCommonErrorNotEnoughMemory	= 8,		// メモリー削減要求に
												// 応じられない
	ModCommonErrorEntryNotFound		= 9,		// エントリーがみつからない
	ModCommonErrorNotSupported		= 10		// 実装されていない
};

//
// ENUM
// ModErrorLevel -- エラーレベルの種類
//
// NOTES
// この列挙型はエラーの深刻さを表し、発生したエラーの処理
// としてどうすれば適当かを示す。
// ModException::getErrorLevel()で得られる
// 
enum ModErrorLevel{
	ModErrorLevelOk = 2,		// エラーではないが例外をとばしたい特別な場合。
	                            // エラーのトレースメッセージは出力されない。
	// 以下はエラー。
	ModErrorLevelRetry = 1,
	ModErrorLevelWarning = 0,
	ModErrorLevelError = -1,
	ModErrorLevelFatal = -2
}; 

//
// ENUM
// ModModule -- モジュール型
//
// NOTES
// この型は、モジュールを表すために用いる。
// 論理ファイルなど、モジュールが動的に追加される場合は
// どこかで番号を割り振る機構が必要
// 
enum ModModule {
	ModModuleUndefined		= -1,	// 無効なモジュール

	// 汎用モジュール関連
									//【注意】	モジュール番号 0 は、
									//			エラー番号 ModCommonErrorXXX が
									// 使用するため、使用不可

	ModModuleMemory			= 1,	// メモリー管理
	ModModuleOs				= 2,	// 汎用 OS、OS ドライバー

									//【注意】	モジュール番号 3 から 9 は、
									//			エラー番号 ModOsErrorXXX が
									//			使用するため、使用不可

	ModModuleStandard		= 10,	// STL、文字列など

	// マネージャー関連
	ModModuleBuffer			= 20,	// バッファー管理関連
	ModModuleLog			= 21,	// ログ処理全般関連
	ModModuleLogicalFile	= 22,	// 論理ファイル関連
	ModModulePhysicalFile	= 23,	// 物理ファイル関連
	ModModuleRequest		= 24,	// リクエスト処理関連
	ModModuleTransaction	= 25,	// トランザクション関連
	ModModuleLock			= 26,	// ロック処理関連
	ModModuleSystem			= 27,	// システム管理関連

	// ドライバー関連
	ModModuleHashFile		= 50,	// ハッシュファイル
	ModModuleHeapFile		= 51,	// ヒープファイル
	ModModuleInvertedFile	= 52,	// 転置ファイル
	ModModuleVectorFile		= 53,	// ベクターファイル
	ModModuleTextStorage	= 54,	// Text Storage

	// 関連文書検索用
	ModModuleTerm			= 70,	// 関連語

	// アプリケーション
	ModModuleAppServer		= 100,	// ○○サーバーなど
	ModModuleAppClient		= 101,	// ○○クライアント

	ModModuleMax					// モジュール番号の最大値
};

//
// VARIABLE
// ModExceptionMessageLength -- 例外メッセージ作成用バッファ
//
// NOTES
//	例外クラスで用意する、作成した出力メッセージ格納用のバッファの長さを
//	表す。
//
const int ModExceptionMessageLength = 1280;

//
// VARIABLE
// ModExceptionFileLength -- ファイルパス保存用バッファ
//
// NOTES
//	例外クラスで用意する、例外を投げた箇所のファイル名を保持する
//	バッファの長さを表す。
//
const int ModExceptionFileLength = 1024; // PATH_MAX in unix

//
// CLASS
// ModException -- 例外クラス
//
// NOTES
//	エラーが発生したモジュール番号、その内容を示すエラー番号、エラーレベルを
//	あらわす。MODの関数は全てこの例外クラスを用いてエラーの内容をやりとりする。
// 	メモリエラーが発生した場合を考え、あらかじめメッセージ出力用のバッファを
//	確保しておくためにModStringではなくcharの配列を持っておく。
//	本クラスはスレッドオブジェクトが正常に起動されると、スレッドに一つ
//	用意される。
//
//	本クラスはnewされないはずであり、特にOsObjectのサブクラスとはしない。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModException
{
	friend class ModErrorHandle;
public:
	// コンストラクター
	ModException();
	// コピーコンストラクター
	ModException(const ModException& x);

	// 内容を設定する
	ModCommonDLL
	void setError(ModModule module_, ModErrorNumber errorNumber_, 
				  ModErrorLevel errorLevel_, int osError_,
				  ModBoolean settingFlag = ModTrue);
	// アクセサ
	ModModule getErrorModule() const;
	ModErrorNumber getErrorNumber() const;
	ModErrorLevel getErrorLevel() const;
	char* getMessageBuffer() const;
	ModBoolean				isError() const;	// エラー状態か調べる

	const char* getPath() const;
	int getLineNumber() const;
	int getOsError() const;

	// ログメッセージ作成
	// スローされた時にログに出力されるメッセージをセットし、返す
	// メッセージの方で出力するので引数は不要。
	// char* setMessage(int line, const char* file);
	ModCommonDLL
	char* setMessage();

	// スローされた場所の情報をセットする
	ModCommonDLL
	void setThrowInfo(int line, const char* file);

	// メモリ獲得が失敗したとき例外を送出するため
	ModCommonDLL
	void* operator new(size_t size);

private:
	// 直接は設定されず、ModErrorを通しそのスレッドを表す例外にだけ設定される
	void setErrorStatus(ModBoolean v);

	ModCommonDLL
	void copyPath(const ModException& original_);

	ModModule				module;
	ModErrorNumber			errorNumber;
	ModErrorLevel			errorLevel;
	ModBoolean				status;				// エラー状態かどうか
	char					message[ModExceptionMessageLength];
	char					path[ModExceptionFileLength];
	int						lineNumber;
	int						osError;
};

// FUNCTION public
// ModException::ModException -- 例外クラスのコンストラクタ
//
// NOTES
//	例外クラスのコンストラクタである。各メンバを初期化する。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし

inline
ModException::ModException()
	: module(ModModuleOs),
	  errorNumber(ModCommonErrorUndefined), 
	  errorLevel(ModErrorLevelError),
	  status(ModFalse),
	  lineNumber(0),
	  osError(0)
{
	// 全部 0 埋めすると遅いので、先頭の 1 文字だけ \0 にする
	//
	//【注意】	ModOsDriver::memset() は
	//			ヘッダーファイルの相互参照になるため使用できない

	*this->message = '\0';
	*this->path = '\0';
}

//
// FUNCTION public
// ModException::ModException -- 例外クラスのコピーコンストラクタ
//
// NOTES
//	例外クラスのコピーコンストラクタである。メッセージバッファ以外の
//	メンバ値をコピーする。メッセージ部分は出力時に一時的に利用されるだけ
//	のバッファにすぎないのでコピーしない。
//
// ARGUMENTS
//	const ModException& original
//		コピー元の例外オブジェクト
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
inline
ModException::ModException(const ModException& original)
{
	this->module = original.module;
	this->errorNumber = original.errorNumber;
	this->errorLevel = original.errorLevel;
	this->status = original.status;
	this->lineNumber = original.lineNumber;
	this->osError = original.osError;
	if (*original.path) copyPath(original);

	// message内容はコピーしない。メッセージ出力時に一時的に使うだけだから。
}

//
// FUNCTION public
// ModException::getErrorModule -- エラーモジュールを得る
//
// NOTES
//	エラーの発生したモジュール番号を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
inline
ModModule
ModException::getErrorModule() const
{
	return this->module;
}

//
// FUNCTION public
// ModException::getErrorNumber -- エラー番号を得る
//
// NOTES
//	エラー内容を示すエラー番号を返す。エラーの内容は、モジュール番号、
//	エラー番号の組で特定される。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
inline
ModErrorNumber
ModException::getErrorNumber() const
{
	return this->errorNumber;
}

//
// FUNCTION public
// ModException::getErrorLevel -- エラーレベルを得る
//
// NOTES
//	発生したエラーのレベルを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
inline
ModErrorLevel
ModException::getErrorLevel() const
{
	return this->errorLevel;
}

//
// FUNCTION public
// ModException::getMessageBuffer -- メッセージ格納領域のアドレスを得る
//
// NOTES
//	メッセージ格納領域の先頭アドレスを返す。
//	メッセージ格納領域は、例外送出時にエラーメッセージを作成する場合に一時的に
//	利用される。あくまでも一時的な利用のため、どんどん上書きされる。
//	例外のエラー処理では、格納領域の内容を参照しても、その内容が
//	エラー番号と必ずしも一致しているとは限らない。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
inline
char*
ModException::getMessageBuffer() const
{
	return (char*)this->message;
}

//	FUNCTION public
//	ModException::isError -- エラー状態か調べる
//
//	NOTES
//		例外クラスがエラー状態を表しているか調べる
//		スレッドごとに保持される例外クラスだけが、
//		ModErrorHandle::setError をかいして例外状態に設定され、
//		ModErrorHandle::reset により解除される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			エラー状態である
//		ModFalse
//			エラー状態でない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModException::isError() const
{
	return this->status;
}

// FUNCTION public
//	ModException::getPath -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

inline
const char*
ModException::
getPath() const
{
	return (const char*)this->path;
}

// FUNCTION public
//	ModException::getLineNumber -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

inline
int
ModException::
getLineNumber() const
{
	return this->lineNumber;
}

// FUNCTION public
//	ModException::getOsError -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

inline
int
ModException::
getOsError() const
{
	return this->osError;
}

//
// FUNCTION private
// ModException::setErrorStatus -- エラー状態を変更する
//
// NOTES
//	エラー状態を変更する。ModFalseを指定するとエラーを示している状態から
//	正常状態にリセットする。ModTrueを指定するとエラー状態が設定される。
//	エラー状態では、
//	メモリ獲得に失敗すると非常用メモリを使って動作するモードとなる。
//	エラー状態のリセットでは、
//	ついでにエラー番号も未定義値にリセットし、メッセージ領域もクリアする。
//	エラー状態は、実行スレッドのエラー状態を表すために用意されている
//	例外オブジェクトに対してのみ、クラスModErrorを経由して設定・リセットされる。
//	そのためにプライベート関数としている。
//
//	(注、もし例外のキャッチでコピーコンストラクタが呼び出されなくなったら
//	直接ModExceptionに設定し、チェックできる)
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//

inline
void
ModException::setErrorStatus(ModBoolean v)
{
	if ((this->status = v) == ModFalse) {

		// エラー状態を解除する

		this->errorNumber = ModCommonErrorUndefined;

		// 全部 0 埋めすると遅いので、先頭の 1 文字だけ \0 にする
		//
		//【注意】	ModOsDriver::memset() は
		//			ヘッダーファイルの相互参照になるため使用できない

		*this->message = '\0';
	}
}

#endif	// __ModException_H__

//
// Copyright (c) 1997, 2002, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
