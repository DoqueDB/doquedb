// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
//	ModCommonException.h --- 汎用ライブラリー関連の例外定義
// 
// Copyright (c) 1997, 2001, 2023 Ricoh Company, Ltd.
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

#ifndef __ModCommonException_H__
#define __ModCommonException_H__

//	ENUM
//	ModMemoryErrorNumber -- メモリー管理モジュール関連のエラー番号を表す列挙型
//
//	NOTES

enum ModMemoryErrorNumber
{
	ModMemoryErrorUndefined			= 1000,		// 初期値

	ModMemoryErrorPoolLimitSize		= 1001,		// プールの上限を超える
	ModMemoryErrorOverPoolLimit		= 1002,		// プールの上限以上の値を指定
	ModMemoryErrorEmergencyLimit	= 1003,		// 非常用メモリーの上限を超える
	ModMemoryErrorOsAlloc			= 1004,		// 仮想 OS のメモリー獲得に失敗
	ModMemoryErrorHandleLimit		= 1005,		// メモリーハンドルの
												// 上限を超える
	ModMemoryErrorWrongSize			= 1006,		// サイズがおかしい
	ModMemoryErrorFreeUnAllocated 	= 1007,		// 獲得していない領域を破棄
	ModMemoryErrorNegotiateFailed 	= 1008,		// メモリー削減交渉に失敗
	ModMemoryErrorHandleNotFound	= 1009,		// メモリーハンドルが
												// 見つからない
	ModMemoryErrorNegotiateRegistered = 1010,	// メモリー削減交渉関数が
												// 既に登録済み
	ModMemoryErrorNotFreeEmergencyArea = 1011,	// メモリー削減交渉関数内で
												// 獲得したメモリーを
												// 関数内で破棄していない
	ModMemoryErrorEmergencyAreaUsed	= 1012		// 非常用メモリーを使用中
};

//	ENUM
//	ModOsErrorNumber -- 汎用 OS モジュール関連のエラー番号を表す列挙型
//
// 	NOTES

enum ModOsErrorNumber
{
	ModOsErrorUndefined				= 2000,		// 初期値

	// 仮想 OS ドライバーのエラー
	//
	// できるだけ ModOsErrorUndefined + errno の値で表現する
	// ただし、OS に依存しないものを中心に定義してある

	ModOsErrorFileNotFound			= 2002,		// ファイルが存在しない
	ModOsErrorInterrupt				= 2004,		// シグナルその他により中断
	ModOsErrorIOError				= 2005,		// I/O エラーが起きた
	ModOsErrorBadFileDescriptor		= 2009,		// ファイルディスクリプター無効
	ModOsErrorSystemMemoryExhaust	= 2012,		// システムのメモリー不足
	ModOsErrorPermissionDenied		= 2013,		// アクセス権エラー
	ModOsErrorBusy					= 2016,		// ビジー
	ModOsErrorFileExist				= 2017,		// ファイルが存在する
	ModOsErrorNotDirectory			= 2020,		// ディレクトリーでない
	ModOsErrorIsDirectory			= 2021,		// ディレクトリーである
	ModOsErrorOpenTooManyFiles		= 2024,		// オープン中のファイル過多
	ModOsErrorTooBigFile			= 2027,		// ファイルサイズの制限超過
	ModOsErrorNotSpace				= 2028,		// デバイスに空き領域がない
	ModOsErrorBrokenPipe			= 2032, 	// 接続が終了していた
	ModOsErrorDeadLockInJoin		= 2045,		// 自分自身の終了は待てない
	ModOsErrorResourceExhaust		= 2063,		// システムのリソースが不足
	ModOsErrorTooLongFilename		= 2078,		// ファイル名が長すぎる
	ModOsErrorNotEmpty				= 2093,		// ディレクトリー削除できない
	ModOsErrorNotSocket				= 2095,		// ソケットでない
	ModOsErrorAddressInUse			= 2125,		// アドレスが既に使用されている
	ModOsErrorInvalidAddress		= 2126,		// アドレスが無効
	ModOsErrorNetworkUnreach		= 2128,		// ネットワークにつながらない
	ModOsErrorIsConnected			= 2133,		// ソケットは接続済み
	ModOsErrorConnectTimeout		= 2145,		// 接続確立前にタイムアウト
	ModOsErrorConnectRefused		= 2146,		// 接続は拒否された
	ModOsErrorConnectAlready		= 2149,		// 前の接続が終了していない
	ModOsErrorInProgress			= 2150,		// 現在処理中

	// WIN32API 依存部分 (未使用)
	//
	ModOsErrorFileCorrupt			= 2201,		// ERROR_FILE_CORRUPT
	ModOsErrorDiskCorrupt			= 2202,		// ERROR_DISK_CORRUPT

	// 仮想 OS ドライバーのその他のエラー
	//
	// 上記の番号が提供されないエラーで、
	// OS のエラー番号がエラーメッセージとして出力される
	//
	// WIN32API の場合、GetLastError の Error Code のマニュアルを参照するとよい

	ModOsErrorOtherReason			= 2999,		// その他、分類なし

	// MOD 独自のエラー番号(ModOsErrorUndefined + 1000 以上)

	// gethostbyname のエラー(netdb.h より)
	ModOsErrorHostNotFound			= 3001,		// ホストがみつからない
	ModOsErrorServerFailed			= 3002,		// 信頼すべきホストがない、
												// サーバーエラー
	// 上記以外
	ModOsErrorFileAlreadyOpened		= 3003,		// 既にオープンされている
	ModOsErrorFileNotOpened			= 3004,		// オープンされていない
	ModOsErrorWouldBlock			= 3005,		// ノンブロック時にブロック
	ModOsErrorAlreadyBound			= 3006,		// ソケットはバインド済み
	ModOsErrorSocketInvalid			= 3007,		// 無効なソケットである
	ModOsErrorThreadNotFound		= 3008,		// スレッドが見つからない
	ModOsErrorKillSelfThread		= 3009,		// 自スレッドを強制終了
	ModOsErrorSetThreadAttribute	= 3010,		// スレッド属性の設定失敗

	// WinSock のエラー
	ModOsErrorWinSockVersion		= 3201,		// WinSock のバージョンが
												// サポートされていない
	ModOsErrorWinSockNotReady		= 3202,		// WinSockの準備ができていない
	ModOsErrorWinSockNetworkDown	= 3203,		// ネットワークがダウン
	ModOsErrorWinSockConnectAborted	= 3204,		// タイムアウトもしくは
												// リセット、クローズ必要
	ModOsErrorWinSockNotConnect		= 3205, 	// 接続していない
	ModOsErrorWinSockTooManyProcess	= 3206,		// WinSock プロセス数の
												// 制限を超えた
//	ModOsErrorWinSockNotInitialized = 3207,		// ソケット全般が未初期化

	// ファイル、ソケット、メモリ(ModOsErrorUndefind + 2000 以上)

	ModOsErrorNotSetCodec			= 4001,		// コーデック関数がない

	// スレッド、ミューテックス、条件変数(ModOsErrorUndefined + 3000 以上)

	ModOsErrorNotLocked				= 5001,		// ロックされていない
	ModOsErrorStillLocked			= 5002,		// ミューテックスロック残存
	ModOsErrorTooManyThreads		= 5003,		// スレッドが多すぎる
	ModOsErrorAlreadySignaled		= 5004,		// 既にシグナル状態
	ModOsErrorObjectAbandon			= 5005,		// 終了済オブジェクトが
												// 確保している
	ModOsErrorThreadStillAlive		= 5006,		// スレッドはまだ動作中
    ModOsErrorDeadLock				= 5007,		// デッドロックを検出
	ModOsErrorNotMainThread			= 5008,		// メインスレッドではない
	ModOsErrorTooManyUnlocked		= 5009,		// アンロックしすぎ
	ModOsErrorModThreadNotFound		= 5010,		// MOD スレッドがみつからない
	ModOsErrorMainThread			= 5011,		// メインスレッドである

	// アーカイバー、シリアライザー(ModOsErrorUndefined + 4000 以上)

	ModOsErrorOutOfMode			  	= 6001,		// モード指定が不適切
	ModOsErrorEndOfFile			  	= 6002,		// ファイルの終端を検知した
	ModOsErrorWriteProtocolInCodec	= 6003,		// プロトコル部書き込みエラー
	ModOsErrorWriteDataInCodec		= 6004,		// データ部書き込みエラー
	ModOsErrorReadProtocolInCodec	= 6005,		// プロトコル部読み出しエラー
	ModOsErrorReadDataInCodec		= 6006,		// データ部読み出しエラー	
	ModOsErrorWriteNoSpace			= 6007		// 書き込む場所がない

	// メッセージ出力(ModOsErrorUndefined + 5000 以上)
};

#endif	// __ModCommonException_H__

//
// Copyright (c) 1997, 2001, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
