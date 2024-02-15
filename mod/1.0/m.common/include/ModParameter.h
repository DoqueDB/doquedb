// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModParameter.h -- パラメーター関連のクラス定義
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModParameter_H__
#define __ModParameter_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModString.h"
#include "ModUnicodeString.h"
#include "ModCommonInitialize.h"
#include "ModMap.h"

#if MOD_CONF_REGISTRY == 0
#include <stdio.h>
#endif

class ModParameterSource;

//	CONST
//	ModParameterEnvironmentNameSizeMax --
//		パラメーターファイルのパス名を格納する環境変数の
//		名前の最大サイズ + 1 (B 単位)
//
const ModSize	ModParameterEnvironmentNameSizeMax = 256;

//	CONST
//	ModParameterEnvironmentValueSizeMax --
//		パラメーターファイルのパス名を格納する環境変数の
//		値の最大サイズ + 1 (B 単位)
//
const ModSize	ModParameterEnvironmentValueSizeMax = 1024;

//	CONST
//	ModParameterFileNameListSizeMax --
//		パラメーターファイルのパス名を格納する環境変数に
//		一度に設定可能なパス名の最大数
//
const ModSize	ModParameterFileNameListSizeMax = 32;

//	CONST
//	ModParameterFileLineSizeMax --
//		パラメーターファイルの一行の最大サイズ(B 単位)
//
const ModSize	ModParameterFileLineSizeMax = 1024;

// CLASS
// ModParameter -- パラメータを扱うクラス
//
// NOTES
// このクラスはパラメータを扱うのに用いる。
// パラメータはライブラリとは別に一定の書式で用意されるもので、
// どのように用意されるかはOSによって異なる。
// たとえば Linux ではパラメータ名と値を一行に並べたテキストファイルである。
//
// テキストファイルの名前は環境変数に':'で
// 区切って並べることで指定する。
// 環境変数名はdefaultではModParameterPathだが、
// コンストラクタの引数で変更することが出来る。
// また、上記の環境変数名の後ろに'Suffix'をつけた環境変数が定義されているときは
// ファイル名の後ろにその値を付加したものが使われる。
// たとえば ModParameterPath が "local.prm:mod.prm" のとき、
// 通常では local.prm、mod.prm がこの順でパスとして適用されるが、
// ModParameterPathSuffix に "-test" が代入されているときは、
// 代わりにlocal.prm-test、mod.prm-test が左記の順で適用される。
//
// パラメータファイルが読み取り専用であることが明らかな場合には
// 各行の長さは不定で構わないが、setXXXを用いることがある場合は、
// パラメーターファイルの各行は以下のような固定長の書式に従わなければならない。
// 1-47カラム:   キー名の後ろを空白文字(0x20)で埋めたもの
// 48カラム:     空白文字(0x20)
// 49-127カラム: パラメーター値の文字列表現の前あるいは後ろを
//               空白文字(0x20)で埋めたもの
// 128カラム:    改行文字(0x0a)
// すなわち、setXXX において、パラメータ名の長さは48バイト以内、
// 値の長さは80バイト以内に制限される。
//
// 他のモジュールに依存しないようにするため、ModObject のサブクラスにはしない。
//
class ModParameter
{
public:

	// ParameterValueが扱う数値型
	typedef ModInt64 NumericType;
	typedef ModUInt64 UnsignedNumericType;

	// ParameterValue内部で保持する列挙型
	enum ParameterType {
		typeNone = 0,			// 未定義値
		typeString,				// 文字列
		typeNumeric,			// 符号つき整数
		typeUnsignedNumeric,	// 符号なし整数
		typeBoolean				// 真偽値
	};

    // CLASS
    // ParameterValue -- パラメータ値を格納するクラス
	//
	// パラメータ値を保持する型。
	// 内部でしか使わないが、 NT の制約のため publicとする。

	//【注意】
	// 以下の条件のいずれかに該当する要素は
	// dllexport(ModCommonDLL)しなければならない。
	// 1) privateではなく、inlineでもないメソッド
	// 2) privateなメソッドで、
	//    privateではないがinlineであるメソッドから呼び出されるもの
	// 3) privateかつstaticなメンバーで、
	//	  privateではないがinlineであるメソッドから参照されるもの

	class ParameterValue
	{
	public:
		// コンストラクター
		ModCommonDLL ParameterValue(ModBoolean autoDelete_ = ModTrue);
		// デストラクター
		ModCommonDLL ~ParameterValue();

		// value.pStringの明示的な解放
		void freePString();
		// value.pStringの所有権を手放し、デストラクト時に
		// 自動的に解放されないようにする
		void releasePString() { autoDelete = ModFalse; };

		// 値をセットする関数
		ModCommonDLL void setParameterValue(const ModUnicodeString& mus);
		ModCommonDLL void setParameterValue(const char* str);
		ModCommonDLL void setParameterValue(const int n);
		ModCommonDLL void setParameterValue(const unsigned int ui);
		ModCommonDLL void setParameterValue(const unsigned long ul);
		ModCommonDLL void setParameterValue(const ModBoolean b);
		ModCommonDLL void setParameterValue(const NumericType n);
		ModCommonDLL void setParameterValue(const UnsignedNumericType un);

		// 各種型に変換して値を取り出す関数
		ModCommonDLL void getParameterValue(char* result) const;
		ModCommonDLL void getParameterValue(ModUnicodeString& result) const;
		ModCommonDLL void getParameterValue(ModWideString& result) const;
		ModCommonDLL void getParameterValue(int& result) const;
		ModCommonDLL void getParameterValue(unsigned int& result) const;
		ModCommonDLL void getParameterValue(unsigned long& result) const;
		ModCommonDLL void getParameterValue(ModFileSize& result) const;
		ModCommonDLL void getParameterValue(ModBoolean& result) const;

		ModCommonDLL const char* toTmpString() const;
		int getType() {return type;} // debug用

		// fileNoの読み書き
		void setFileNo(int n) {fileNo = n;}
		int getFileNo() {return fileNo;}

		// クラスの内容を文字列表現にする
		ModCommonDLL ModBoolean toLineExpression(char* const pszResult,
												 const int limitLength) const;

	private:
		//メンバ変数
		int fileNo;	// 何番目のファイルにあったか
		ParameterType type;					// パラメータの型
		union {								// パラメータの値
			ModString* pString;				// : 文字列(utf-8)
			NumericType numeric;			// : 符号つき数値
			UnsignedNumericType uNumeric;	// : 符号なし数値
			ModBoolean boolean;				// : 真偽値
		} value;
		char* array;						// pStringが使えないときの予備
		ModBoolean autoDelete;  // destruct時に*pStringを解放する/しない
	}; 
    // end of ParameterValue's definition

	class ModCommonDLL Object
	{
		friend class ModParameter;
	public:
		// Objectの同一性を判定するために使用する内部クラス
		class SourceInfo
			: public ModPair<char*, ModSize>
		{
		public:
			SourceInfo();
			SourceInfo(const ModParameterSource& source);
			~SourceInfo();
			void set(const ModParameterSource& source);
			ModBoolean operator==(const SourceInfo& info) const;
		};

		// コンストラクター
		// doPreLoadは明示的につけている箇所があるので残す
		Object(ModBoolean doPreLoad = ModTrue);
		Object(const char* environmentName,
			   ModBoolean doPreLoad = ModTrue);
		Object(const ModParameterSource& source,
			   ModBoolean doPreLoad = ModTrue);
		// デストラクター
		~Object() { this->terminate(); };

		// ファイルの存在を調べる
		static ModBoolean checkFile(const char* fileName);

		// 文字列型のパラメータを返り値に取得する
		ModCharString getString(const char* key);
		ModWideString getWideString(const char* key);
		// (非 ASCII 文字を取得する場合は ModOs::Process::_encodingType と
		//  同じ文字コードのマルチバイト文字列を getString で取得できます。
		//  getWideString は使わないようにしてください。)
		ModUnicodeString getUnicodeString(const char* key);
		// 数値型のパラメータを返り値に取得する
		int getInteger(const char* key);
		unsigned int getUnsignedInteger(const char* key);
		unsigned long getUnsignedLong(const char* key);
		ModSize getModSize(const char* key);
		ModFileSize getModFileSize(const char* key);
		// 論理値型のパラメータを返り値に取得する
		ModBoolean getBoolean(const char* key);

		// 文字列型のパラメータを引数に取得する
		ModBoolean getString(char* array, const char* key);
		// 注意: 引数`array'にはあらかじめメモリを割り当てておくこと
		ModBoolean getString(ModCharString& string, const char* key);
		ModBoolean getWideString(ModWideString& string, const char* key);
		// (非 ASCII 文字を取得する場合は ModOs::Process::_encodingType と
		//  同じ文字コードのマルチバイト文字列を getString で取得できます。
		//  getWideString は使わないようにしてください。)
		ModBoolean getUnicodeString(ModUnicodeString& string, const char* key);
		// 数値型のパラメータを引数に取得する
		ModBoolean getInteger(int& value, const char* key);
		ModBoolean getUnsignedInteger(unsigned int& value, const char* key);
		ModBoolean getUnsignedLong(unsigned long& value, const char* key);
		ModBoolean getModSize(ModSize& value, const char* key);
		ModBoolean getModFileSize(ModFileSize& value, const char* key);
		// 論理値型のパラメータを引数に取得する
		ModBoolean getBoolean(ModBoolean& boolean, const char* key);

		// 文字列型のパラメータを登録する
		ModBoolean setString(const char* key, const char* string);
		ModBoolean setString(const char* key, const ModCharString& string);
		ModBoolean setWideString(const char* key, const ModWideString& string);
		ModBoolean setWideString(const char* key, const ModWideChar* string);
		// (非 ASCII 文字をセットする場合は ModOs::Process::_encodingType と
		//  同じ文字コードのマルチバイト文字列を setString に渡して下さい。
		//  setWideString は使わないようにしてください。)
		ModBoolean setUnicodeString(const char* key,
									const ModUnicodeString& string);
		// 数値型のパラメータを登録する
		ModBoolean setInteger(const char* key, const int value);
		ModBoolean setUnsignedLong(const char* key, const unsigned long value);
		ModBoolean setUnsignedInteger(const char* key, const unsigned int value);
		// 論理型のパラメータを登録する
		ModBoolean setBoolean(const char* key, const ModBoolean flag);

		// instparamで使う

		// マップの型(instparamでも使う)
		typedef ModMap<ModString, ParameterValue, ModLess<ModString> > Map;
		// パラメータファイルの1行を解析する
		static ModBoolean parseLine(ParameterValue& pv, 
									char* line, const char* keyName);
		// ファイルに書き込む
		static ModBoolean writeFile(const ParameterValue& pv,
									const char* path, const char* keyName);
		// マップを直接得る(主にテスト用)
		Map* getMap() {	return _map; };
		const ModBoolean isDoPreLoad() const {return _doPreLoad;};

		static Object* attach(const ModParameterSource& source,
							  ModBoolean doPreLoad);
		static void detach(Object*& pParameter);

	private:
		// 初期化する
		void initialize(const ModParameterSource& source);
		// パス名を組み立てる
		void setFullPathName(const int index, const char* parentPath, 
							 const char* name, const char* suffixValue);
		// 後始末する
		void terminate();

		// パラメーターの値を得る
		ModBoolean getValue(ParameterValue& pv, const char* key = 0);
		// パラメーターの値を記録する
		ModBoolean setValue(ParameterValue& pv, const char* key = 0);

		// ファイルから読み込む
		static ModBoolean readFile(ParameterValue& pv,
								   const char* path, const char* keyName = 0);
		// ファイルから読み込んでマップに入れる
		void preLoad();
		// マップから読み出す
		ModBoolean getFromMap(ParameterValue& pv, const char* key) const;
		// マップに書き込む
		void setToMap(ParameterValue& pv, const char* key);

		// ファイルを開けようとする
		static FILE* openFile(const char* path, const char* mode);
		// パラメータファイルに1行書き込む
		static ModBoolean writeLine(const ParameterValue& pv, 
									FILE* fp, const char* line);

		// Objectの同一性を判定する
		ModBoolean isEquivalent(const ModParameterSource& source,
								ModBoolean doPreLoad) const;

		// パラメーターファイルのパス名を格納する配列
		char*	_fileNameList[ModParameterFileNameListSizeMax];
		// 上の配列に値が入っている末尾の添字
		int _lastIndex;

		// 先読みしたパラメーター値を格納するマップ
		Map* _map;				
		// パラメーターを初期化時にすべて先読みするか
		ModBoolean	_doPreLoad;

		// 参照回数を管理する
		int _refCount;
		// Objectの同一性を判定するためのデータ
		SourceInfo _sourceInfo;

		// 無指定時に使う環境変数名

		// システムパラメーターファイルのパス名の設定されている
		// デフォルトの環境変数名
		static const char* const	_systemEnvironmentName;
		// システムパラメーターファイルのデフォルトのファイル名
		static const char* const	_systemFileName;
		// 永続化しないシステムパラメーターファイルのファイル名
		static const char* const	_temporarySystemFileName;
		// パラメーターファイルのパス名かキー名の設定されている
		// デフォルトの環境変数名
		static const char* const	_environmentName;
		// デフォルトパラメーターファイルのデフォルトのファイル名
		static const char* const	_defaultFileName;
		// システムまたはデフォルトパラメーターファイルの親のパス名
		static const char* const	_parentPath;

		// キーおよびパラメータの制限長
		static const int systemKeyLength;
		static const int systemParameterLength;

		// リンクリストに用いる
		Object* _next;
		static Object* _allObjects;
	};
	// end of Object's definition

	// コンストラクター
	ModCommonDLL
	ModParameter(ModBoolean doPreLoad = ModTrue);
	ModCommonDLL
	ModParameter(const char* environmentName, ModBoolean doPreLoad = ModTrue);
	ModCommonDLL
	ModParameter(const ModParameterSource& source,
				 ModBoolean doPreLoad = ModTrue);
	// デストラクター
	ModCommonDLL
	~ModParameter();

	// パラメーターに関する初期化と後処理を行う
	ModCommonDLL
	static void initialize();
	ModCommonDLL
	static void terminate();

	// ファイルの存在を調べる
	static ModBoolean checkFile(const char* fileName)
	{return Object::checkFile(fileName);}

	// 文字列型のパラメータを返り値に取得する
	ModCharString getString(const char* key)
	{return _parameter->getString(key);}
	ModWideString getWideString(const char* key)
	{return _parameter->getWideString(key);}
		// (非 ASCII 文字を取得する場合は ModOs::Process::_encodingType と
		//  同じ文字コードのマルチバイト文字列を getString で取得できます。
		//  getWideString は使わないようにしてください。)
	ModUnicodeString getUnicodeString(const char* key)
	{return _parameter->getUnicodeString(key);}
	// 数値型のパラメータを返り値に取得する
	int getInteger(const char* key)
	{return _parameter->getInteger(key);}
	unsigned int getUnsignedInteger(const char* key)
	{return _parameter->getUnsignedInteger(key);}
	unsigned long getUnsignedLong(const char* key)
	{return _parameter->getUnsignedLong(key);}
	ModSize getModSize(const char* key)
	{return _parameter->getModSize(key);}
	ModFileSize getModFileSize(const char* key)
	{return _parameter->getModFileSize(key);}
	// 論理値型のパラメータを返り値に取得する
	ModBoolean getBoolean(const char* key)
	{return _parameter->getBoolean(key);}

	// 文字列型のパラメータを引数に取得する
	ModBoolean getString(char* array, const char* key)
	{return _parameter->getString(array, key);}
		// 注意: 引数`array'にはあらかじめメモリを割り当てておくこと
	ModBoolean getString(ModCharString& string, const char* key)
	{return _parameter->getString(string, key);}
	ModBoolean getWideString(ModWideString& string, const char* key)
	{return _parameter->getWideString(string, key);}
		// (非 ASCII 文字を取得する場合は ModOs::Process::_encodingType と
		//  同じ文字コードのマルチバイト文字列を getString で取得できます。
		//  getWideString は使わないようにしてください。)
	ModBoolean getUnicodeString(ModUnicodeString& string, const char* key)
	{return _parameter->getUnicodeString(string, key);}
	// 数値型のパラメータを引数に取得する
	ModBoolean getInteger(int& value, const char* key)
	{return _parameter->getInteger(value, key);}
	ModBoolean getUnsignedInteger(unsigned int& value, const char* key)
	{return _parameter->getUnsignedInteger(value, key);}
	ModBoolean getUnsignedLong(unsigned long& value, const char* key)
	{return _parameter->getUnsignedLong(value, key);}
	ModBoolean getModSize(ModSize& value, const char* key)
	{return _parameter->getModSize(value, key);}
	ModBoolean getModFileSize(ModFileSize& value, const char* key)
	{return _parameter->getModFileSize(value, key);}
	// 論理値型のパラメータを引数に取得する
	ModBoolean getBoolean(ModBoolean& boolean, const char* key)
	{return _parameter->getBoolean(boolean, key);}

	// 文字列型のパラメータを登録する
	ModBoolean setString(const char* key, const char* string)
	{return _parameter->setString(key, string);}
	ModBoolean setString(const char* key, const ModCharString& string)
	{return _parameter->setString(key, string);}
	ModBoolean setWideString(const char* key, const ModWideString& string)
	{return _parameter->setWideString(key, string);}
	ModBoolean setWideString(const char* key, const ModWideChar* string)
	{return _parameter->setWideString(key, string);}
		// (非 ASCII 文字をセットする場合は ModOs::Process::_encodingType と
		//  同じ文字コードのマルチバイト文字列を setString に渡して下さい。
		//  setWideString は使わないようにしてください。)
	ModBoolean setUnicodeString(const char* key,
								const ModUnicodeString& string)
	{return _parameter->setUnicodeString(key, string);}
	// 数値型のパラメータを登録する
	ModBoolean setInteger(const char* key, const int value)
	{return _parameter->setInteger(key, value);}
	ModBoolean setUnsignedLong(const char* key, const unsigned long value)
	{return _parameter->setUnsignedLong(key, value);}
	ModBoolean setUnsignedInteger(const char* key, const unsigned int value)
	{return _parameter->setUnsignedInteger(key, value);}
	// 論理型のパラメータを登録する
	ModBoolean setBoolean(const char* key, const ModBoolean flag)
	{return _parameter->setBoolean(key, flag);}

	// instparamで使う

	// マップの型(instparamでも使う)
 	typedef Object::Map Map;
	// マップを直接得る(主にテスト用)
	Map* getMap() {	return _parameter->getMap(); };
	const ModBoolean isDoPreLoad() const {return _parameter->isDoPreLoad();};

	// 内部表現に用いられる文字コード
	ModCommonDLL
	static ModKanjiCode::KanjiCodeType _codingType;

private:

	Object* _parameter;
	static ModParameter* _defaultParameter;
};

// メッセージ表示用のマクロ
#define _SafeMessage(stream) if (ModMessageSelection::isInitialized()) stream
#define _SafeModMessage      _SafeMessage(ModMessage)
#define _SafeModDebugMessage _SafeMessage(ModDebugMessage)
#define _SafeModErrorMessage _SafeMessage(ModErrorMessage)

//	FUNCTION public
//	ModParameter::ParameterValue:freePString
//		-- 内部の文字列バッファを明示的に解放する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
inline
void ModParameter::ParameterValue::freePString()
{
	if (array == 0 && type == typeString && value.pString != 0)
		delete value.pString, value.pString = 0;
}

#endif	// __ModParameter_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
