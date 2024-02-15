// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMessage.h -- メッセージ記録用共通ライブラリ
// 
// Copyright (c) 1996, 1997, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ModMessage_H__
#define __ModMessage_H__

#include "ModConfig.h"

#include "ModCommonDLL.h"
//#include "ModOs.h"
//#include "ModOsMutex.h"
#include "ModManipulator.h"
#include "ModWideChar.h"
#include "ModTypes.h"
#include "ModKanjiCode.h"
#include "ModLanguage.h"
#include "ModCommonInitialize.h"

class ModCharString;
class ModWideString;
class ModParameter;
class ModException;
class ModLanguageSet;

#ifdef MOD_CONF_DEFINE_FRIENDFUNCTION
class ModMessageStream;

ModCommonDLL ModMessageStream&	ModFlush(ModMessageStream& stream);
ModCommonDLL ModMessageStream&	ModEndl(ModMessageStream& stream);
ModCommonDLL ModMessageStream&	ModHex(ModMessageStream& stream);
ModCommonDLL ModMessageStream&	ModDec(ModMessageStream& stream);
ModCommonDLL ModMessageStream&	ModWc(ModMessageStream& stream);
ModCommonDLL ModMessageStream&	ModEuc(ModMessageStream& stream);
ModCommonDLL ModMessageStream&	ModShiftJis(ModMessageStream& stream);
ModCommonDLL ModMessageStream&	ModLiteralCode(ModMessageStream& stream);

ModCommonDLL ModManipulator&	ModSetW(int width_);
ModCommonDLL ModManipulator&	ModSetFill(int fill_);

ModMessageStream&	ModSetWidthInternal(ModMessageStream& stream, int width_);
ModMessageStream&	ModSetFillInternal(ModMessageStream& stream, int fill_);
#endif

//
// CLASS
// ModMessageStream -- メッセージ出力のためのクラス
//
// NOTES
// このクラスは障害情報やデバッグ情報を出力するのに用いる。
// ユーザが直接触ることはなく、ModMessage、ModErrorMessage、ModDebugMessage
// の3つの define を通して ModMessageSelection のメンバ関数の返り値として
// 間接的に使用する。
// したがって、ここで定義されるメンバ関数は ModMessage << "message"; のように
// 使用される。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModMessageStream
{
public:
	friend class ModMessageSelection;
	friend class ModMessageStreamWrapper;

	// この列挙型は ModMessageStream においてその出力先を表すために用いる。
	enum MessageType {
		toNone,							// 出力しない
		toStandardOut,					// cout に出力
		toStandardError,				// cerr に出力
		toSystemLog,					// syslog、イベントログに出力
		toSLog							// SLog に出力 (未使用)
	};

	// マニピュレーターの型定義
	typedef ModMessageStream& (*Manipulator)(ModMessageStream&);
	typedef ModManipulator& (*ManipulatorWithInt)(int);

	ModCommonDLL
	ModMessageStream(MessageType type_ = toNone);
	ModCommonDLL
	~ModMessageStream();

	//
	// コマンド名を設定する関数
	// ModMessageStream を使う以前に呼び出すのが望ましい。
	// この関数を使わずに ModMessageStream を使うと
	// "mod" が代わりに用いられる。
	// category はイベントログに出す「分類」を指定する。
	// 省略するか0,1以外の値を指定するとパラメータを見にいく。
	//
	ModCommonDLL
	static void setCommandName(const char* commandName_, int category = -1);

	//
	// 出力オペレータ
	// 実際に出力されるのは ModEndl か ModFlush を用いたとき
	//
	ModCommonDLL
	ModMessageStream& operator<<(char character);
	ModCommonDLL
	ModMessageStream& operator<<(unsigned char character);
	ModCommonDLL
	ModMessageStream& operator<<(int value);
	ModCommonDLL
	ModMessageStream& operator<<(unsigned int value);
	ModCommonDLL
	ModMessageStream& operator<<(long value);
	ModCommonDLL
	ModMessageStream& operator<<(unsigned long value);
	ModCommonDLL
	ModMessageStream& operator<<(ModInt64 value);
	ModCommonDLL
	ModMessageStream& operator<<(ModUInt64 value);
	ModCommonDLL
	ModMessageStream& operator<<(float value);
	ModCommonDLL
	ModMessageStream& operator<<(double value);
	ModCommonDLL
	ModMessageStream& operator<<(ModUnicodeChar value);
	ModCommonDLL
	ModMessageStream& operator<<(ModLanguageCode value);
	ModCommonDLL
	ModMessageStream& operator<<(const ModLanguageSet& value);
	ModCommonDLL
	ModMessageStream& operator<<(const ModCharString& string);
	ModCommonDLL
	ModMessageStream& operator<<(const ModWideString& string);
	ModCommonDLL
	ModMessageStream& operator<<(const ModUnicodeString& string);
	ModCommonDLL
	ModMessageStream& operator<<(const char* string);
	ModCommonDLL
	ModMessageStream& operator<<(const ModWideChar* string);
	ModCommonDLL
	ModMessageStream& operator<<(const ModUnicodeChar* string);
	ModCommonDLL
	ModMessageStream& operator<<(ModException& exception);
	ModCommonDLL
	ModMessageStream& operator<<(Manipulator manipulator);

	//
	// マニピュレータ
	// 引数を伴うものにはメンバ関数として用意されるものと、
	// operator<< の引数として用いられる外部関数とがある。
	//

	// メンバ関数で用意されるマニピュレータ
	ModCommonDLL
	void setWidth(int width);
	ModCommonDLL
	void setFill(int fill);
	ModCommonDLL
	void setWc();						// ModWideCharが入力されることを宣言
	ModCommonDLL
	void setEuc();						// 入力の漢字コードを EUC にする
	ModCommonDLL
	void setShiftJis();					// 入力の漢字コードを ShiftJIS にする
	ModCommonDLL
	void setLiteralCode();				// 入力の漢字コードを文字列リテラルのコードにする

	// operator<< の引数に用いられるマニピュレータ
	friend ModCommonDLL ModMessageStream&	ModFlush(ModMessageStream& stream);
	friend ModCommonDLL ModMessageStream&	ModEndl(ModMessageStream& stream);
	friend ModCommonDLL ModMessageStream&	ModHex(ModMessageStream& stream);
	friend ModCommonDLL ModMessageStream&	ModDec(ModMessageStream& stream);
	friend ModCommonDLL ModMessageStream&	ModWc(ModMessageStream& stream);
	friend ModCommonDLL ModMessageStream&	ModEuc(ModMessageStream& stream);
	friend ModCommonDLL ModMessageStream&
		ModShiftJis(ModMessageStream& stream);
	friend ModCommonDLL ModMessageStream&
		ModLiteralCode(ModMessageStream& stream);

	// operator<< の引数に用いられるマニピュレータ
	friend ModCommonDLL ModManipulator&		ModSetW(int width_);
	friend ModCommonDLL ModManipulator&		ModSetFill(int fill_);

private:
	// 数値の出力形式
	enum NumberType {
		typeDec = 0,					// 10進
		typeHex = 1						// 16進
	};

	// メッセージの種類
	enum MessageLevel {
		levelDebug = 0,					// デバッグ用
		levelInformation = 1,			// 通常の情報
		levelWarning = 2,				// 警告(非使用)
		levelError = 3					// エラーメッセージ
	};

	//
	// コピーは許さないため、コピーコンストラクタと代入オペレータは
	// ここで宣言しておいて定義しない
	//
	ModMessageStream(const ModMessageStream& original);
	ModMessageStream& operator=(const ModMessageStream& original);

	// 必要なら処理前にflushする
	void check();

	// 空かどうかの判定
	ModBoolean isEmpty() const;

	// 空にする
	void clear();

	// バッファの内容を出力し、空にする
	void doFlush();

	// 現在の文字数を得る
	ModSize getSize() const;

	// メッセージに付加するファイル名、行番号を作成する
	void initializeMessage(const char* fileName, int lineNo,
						   MessageLevel level);

	// メッセージに付加する現在時刻を作成する
	void setHeader();

	// フィリングを行なう
	ModBoolean doFill(int width_);

	// ストリームも引数にとる内部関数
	friend ModMessageStream&
		ModSetWidthInternal(ModMessageStream& stream, int width_);
	friend ModMessageStream&
		ModSetFillInternal(ModMessageStream& stream, int fill_);

	// 出力を排他するためのロックをかける
	ModCommonDLL
	void lock();
	// 出力を排他するためのロックを外す
	ModCommonDLL
	void unlock();

	// 出力文字列バッファ
#define ModMessageBufferSize	(1024)	// バッファサイズ
	char buffer[ModMessageBufferSize];	// 出力文字列バッファ
	char* currentPointer;				// 現在の入力位置

	char			fileNameBuf[ModMessageBufferSize];
	int				lineNumber;
	MessageLevel	messageLevel;
	int				errorNumber;

#if MOD_CONF_SYSTEM_LOG == 1

	// システムコンソールログのための定義

#define ModMessageSyslogSize	(1024)	// syslog に一度に渡せるバイト数
	char jisBuffer[ModMessageSyslogSize]; // syslog に渡す文字列を作るバッファ
	int priority;						// syslog に渡す priority
	int logHeaderSize;
#endif

	// syslog と同様のヘッダをつけるためのバッファ
#define ModMessageHeaderSize	(256)	// ヘッダ文字の最大数
	char headerBuffer[ModMessageHeaderSize];
	unsigned int milliSecond;			// 実行時刻のミリ秒単位
	int headerSize;						// ヘッダに入れられたの文字数

	int accessCount;					// 前のflushから stream の呼ばれた回数

	ModThreadId threadId;				// 出力中のスレッドID
	int currentStatus;					// 出力中の状態

	// その他出力用属性
	MessageType type;					// 出力先を表す

	int width;							// 表示幅
	int fill;							// パッディングに使う文字
	NumberType numberType;				// 数値の出力形式
	ModKanjiCode::KanjiCodeType kanjiCode; // 入力される char* の漢字コード
	ModBoolean noHeader;				// header出力済みフラグ
	ModBoolean wideChar;				// 数値をModWideCharで出すフラグ

	// ヘッダに使う情報
#define ModCommandNameLength (256)
#define ModHostNameLength (256)
	static char commandName[ModCommandNameLength - 1];
	static char hostName[ModHostNameLength - 1];
	static ModBoolean commandNameIsSet;
};

//
// 実装がしばしば変更される可能性があるので
// ModMessageStream のメンバ関数の inline での実装は避ける
//

//
// CLASS
// ModMessageStreamWrapper -- メッセージ出力を排他するためのクラス
//
// NOTES
// このクラスはauto変数でしか使ってはいけない
//
class ModCommonDLL ModMessageStreamWrapper
{
public:
	ModMessageStreamWrapper(ModMessageStream& stream)
		: m_stream(stream)
	{
		m_stream.lock();
	}
	ModMessageStreamWrapper(const ModMessageStreamWrapper& wrapper)
		: m_stream(wrapper.m_stream)
	{
		m_stream.lock();
	}
	~ModMessageStreamWrapper()
	{
		m_stream.unlock();
	}
	ModMessageStream& getStream() {return m_stream;}

private:
	ModMessageStream& m_stream;
};

//
// CLASS
// ModMessageSelection -- メッセージの振り分け先を決めるのに使うクラス
//
// NOTES
// このクラスは ModMessage、ModErrorMessage、ModDebugMessage の実際の
// 出力先を切替えるために用いる。
// また、ModMessage などに用いる ModMessageStream がただ一度だけ
// 作成されることを保証するのにも用いる。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModMessageSelection
{
public:
	friend class ModMessageStream;

	ModCommonDLL
	static ModMessageStreamWrapper normal(const char* fileName, int lineNo);
	ModCommonDLL
	static ModMessageStreamWrapper error(const char* fileName, int lineNo);
	ModCommonDLL
	static ModMessageStreamWrapper debug(const char* fileName, int lineNo);

	ModCommonDLL
	static ModBoolean isInitialized();

	ModCommonDLL
	static void initialize();
	ModCommonDLL
	static void terminate();

private:
	// instance を作らないように private で定義して実装しない
	ModMessageSelection();
	~ModMessageSelection();

	// パラメータの値からストリームを選択する関数
	static ModMessageStream* setStream(ModMessageStream::MessageType defaultType,
									   int defaultParameter,
									   const char* parameterName);
	// ストリームを初期化して返す関数(normal、error、debugの下請け)
	static ModMessageStreamWrapper setupStream(ModMessageStream*& streamPointer,
											   const char* fileName, int lineNo,
											   ModMessageStream::MessageLevel level);

	static int initializeCount;			// 初期化カウント
	static ModMessageStream *coutMessage; // cout へ送るストリーム
	static ModMessageStream *cerrMessage; // cerr へ送るストリーム
	static ModMessageStream *logMessage; // syslog、eventlog へ送るストリーム
	static ModMessageStream *slogMessage; // slog へ送るストリーム
	static ModMessageStream noneMessage; // どこにも送らないストリーム

	// パラメータによりセットされる変数
	static ModMessageStream *normalStream; // normal の返すストリーム
	static ModMessageStream *errorStream; // error の返すストリーム
	static ModMessageStream *debugStream; // debug の返すストリーム

	static ModBoolean headerFlag;		// ヘッダ部を出力するか

	// メッセージの動作を制御するパラメータのありかを示す環境変数名
	static const char* parameterEnvironment;
	// メッセージの動作を制御するパラメータ
	static ModParameter *parameter;

	// 初期化されていることを示すフラグ
	static ModBoolean initializedFlag;
};

#define ModMessage		if (0); else ModMessageSelection::normal(__FILE__, __LINE__).getStream()
#define ModErrorMessage if (0); else ModMessageSelection::error(__FILE__, __LINE__).getStream()
#define ModDebugMessage if (0); else ModMessageSelection::debug(__FILE__, __LINE__).getStream()

//
// CLASS
// ModManipulatorSelection -- マニピュレーターの選択を行なうためのクラス
//
// NOTES
// このクラスは ModMessageStream に使うマニピュレーターを選択する
// ために用いる。
// 

class ModCommonDLL ModManipulatorSelection
{
public:
	static void initialize();
	static void terminate();

	static ModManipulator* setWidth;
	static ModManipulator* setFill;

	static int initializeCount;
};

#endif	// __ModMessage_H__

//
// Copyright (c) 1996, 1997, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
