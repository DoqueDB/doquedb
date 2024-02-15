// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModParameterLinux.cpp -- パラメータ関連のメンバ定義Linux固有分
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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
extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
}
#include "ModAutoMutex.h"
#include "ModParameter.h"
#include "ModDefaultManager.h"
#include "ModOsDriver.h"
#include "ModMultiByteString.h"
#include "ModKanjiCode.h"

//	CONST private
//	ModParameter::_systemEnvironmentName --
//		システムパラメーターファイルのパス名を設定する環境変数名
//
const char* const
ModParameter::Object::_systemEnvironmentName = "ModSystemParameterPath";

//	CONST private
//	ModParameter::_systemFileName --
//		システムパラメーターファイルのファイル名
//
//	NOTES
//		環境変数 ModParameter::_systemEnvironmentName で指定されない場合、
//		この値が使用される
//
const char* const
ModParameter::Object::_systemFileName = "system.prm";

//	CONST private
//	ModParameter::_temporarySystemFileName --
//		永続化しないシステムパラメーターファイルのファイル名
//
//	NOTES
//		システムパラメーターファイル名がこの文字列と一致する場合、
//		ファイルへの書き込みをしない
//
const char* const
ModParameter::Object::_temporarySystemFileName = "temporary";

//	CONST private
//	ModParameter::Object::_environmentName --
//		デフォルトパラメータファイルのパス名を設定する環境変数名
//
const char* const
ModParameter::Object::_environmentName = "ModParameterPath";

//	CONST private
//	ModParameter::Object::_defaultFileName --
//		デフォルトパラメーターファイルのファイル名
//
const char*	const
ModParameter::Object::_defaultFileName = "default.prm";

//	CONST private
//	ModParameter::Object::_parentPath --
//		システムパラメーターファイルまたはデフォルトパラメーターファイルの
//		親ディレクトリーのデフォルトのパス名
//
const char*	const
ModParameter::Object::_parentPath = ".";

// CONST private
// ModParameter::Object::systemKeyLength -- システムパラメータのキー制限長
//
// NOTES
// この定数は setXXX で使うシステムパラメータに用いるキーの長さを表す。
// これを定めることにより、複数のプロセスから同時にアクセスされたときに
// おかしくならないようにする。
//
const int
ModParameter::Object::systemKeyLength = 48;

// CONST private
// ModParameter::Object::systemParameterLength -- システムパラメータのパラメータ制限長
//
// NOTES
// この定数は setXXX で使うシステムパラメータに用いる値の長さを表す。
// これを定めることにより、複数のプロセスから同時にアクセスされたときに
// おかしくならないようにする。
//
const int
ModParameter::Object::systemParameterLength = 80;

//	FUNCTION private
//	ModParameter::Object::openFile -- 所定のファイルを開けようとする
//
//	NOTES
//
//	ARGUMENTS
//		const char* path
//			開けようとするファイルのパス
//		const char* mode
//			オープンモード。書式はfopen(3)に同じ
//
//	RETURN
//		FILE* : 成功→fp, 失敗→NULL
//
FILE* ModParameter::Object::openFile(const char* path, const char* mode)
{
	int retryCount = 5;
	FILE* fp;
retry:
	if ((fp = ::fopen(path, mode)) == NULL) {
		int	saved = errno;
		if (saved == EAGAIN && retryCount--)
			goto retry;
//-		_SafeModErrorMessage << "Can't open " << path << "(" 
//-							 << ::strerror(saved) << ")." << ModEndl;
		return 0;
	}
	return fp;
}

//	FUNCTION private
//	ModParameter::Object::preLoad -- 各々のファイルを読んでマップに入れる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
void
ModParameter::Object::preLoad()
{
	// パラメーターファイルを先読みする
	_map = new ModParameter::Object::Map();
	for (int index = 0; index <= _lastIndex; index++) {

		// パラメーターファイルをオープンする
		FILE* fp = openFile(_fileNameList[index], "r");
		if (!fp) continue;

        // 読み込む文字列の文字コードはModOs::Process::getEncodingType()
		char line[ModParameterFileLineSizeMax]; 
		
		while (fgets(line, ModParameterFileLineSizeMax, fp) != NULL) {
			ParameterValue pv;
			pv.setFileNo(index);
			if (parseLine(pv, line, NULL) == ModFalse) continue;

			// itemを作らずに直接_map->insert(ModString, pv)を呼ぶと、
			// insertにpv.value.pStringを壊されてしまう
			ModPair<ModString, ParameterValue> item;
			item.first = ModString(line);
			item.second = pv;
			ModPair<Map::Iterator, ModBoolean> pair = _map->insert(item);

			if (pair.second == ModFalse) { // エントリが既にあった
				//  重複するキー値の優先順位はfileNoで決定する
				if ((*(pair.first)).second.getFileNo() == pv.getFileNo()) {
					// このファイル中のエントリを上書き
					(*(pair.first)).second.freePString();
					(*(pair.first)).second = pv;
					pv.releasePString();
				} else { // いま持っている値を棄てる
					pv.freePString();
				}
			} else {
				// pvを_mapに登録したので所有権を手放す
				pv.releasePString();
			}
			// item内のpvがpStringを握ったままなので放す
			item.second.releasePString();
		} // end of while

		fclose(fp);
	} // end of for(index)
}

//	FUNCTION private
//	ModParameter::Object::checkFile -- パラメーターファイルの存在を確認する
//
//	NOTES
//
//	ARGUMENTS
//		const char* path
//			調べるファイルの名前
//
//	RETURN
//		ModTrue
//			指定されたパラメーターファイルは存在する
//		ModFalse
//			指定されたパラメーターファイルは存在しない
//
ModBoolean
ModParameter::Object::checkFile(const char* path)
{
	// パラメーターファイルの情報を取得し、存在するかどうか調べる
	struct stat buf;
	if (::stat(path, &buf) == -1) {
		_SafeModDebugMessage << "file \"" << path
							 << "\" does not exist." << ModEndl;
		return ModFalse;
	}
	return ModTrue;
}

//	FUNCTION private
//	ModParameter::Object::getFromMap -- マップからパラメーター値を取得する
//
//	NOTES
//
//	ARGUMENTS
//		ParameterValue& pv
//			[out]取得した結果の格納先
//		const char* key
//			[in]取得するパラメータのキー値
//
//	RETURN
//		ModTrue: 取得できた
//		ModFalse: 取得できなかった
//
ModBoolean 
ModParameter::Object::getFromMap(ParameterValue& pv, const char* key) const
{
	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	Map::ConstIterator iterator =
		((const Map*)_map)->find(ModCharString(key));
	if (iterator == ((const Map*)_map)->end()) {
		// 指定された名前のパラメーターはマップに登録されていない
		return ModFalse;
	}
	pv = (*iterator).second;
	pv.releasePString();
	return ModTrue;
}

//	FUNCTION private
//	ModParameter::Object::setToMap -- 与えられたパラメーター値をマップに登録する
//
//	NOTES
//
//	ARGUMENTS
//		ParameterValue& pv
//			[in]与えるパラメータ値
//		const char* key
//			[in]登録先のパラメータのキー値
//
//	RETURN
//		なし
//
void
ModParameter::Object::setToMap(ParameterValue& pv, const char* key)
{
	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	pv.setFileNo(0);
	ModParameter::ParameterValue& mapped = (*_map)[ModCharString(key)];
	// [2002-11-05] Windows版との整合性のため、常に上書きすることにする
	mapped.freePString();
	mapped = pv;
	// 与えられた文字列がマップに登録されたので
	// pv.value.pStringの所有権をmappedに渡す
	pv.releasePString();
}

// FUNCTION private
// ModParameter::Object::readFile
//  -- ファイルからパラメータを読み込む(Solaris)
//
// NOTES
// この関数はファイルからパラメータを読み込むのに用いる。
//
// ARGUMENTS
// ParameterValue& pv
//		結果の格納先
// const char* path
//		ファイル名
// const char* keyName
//		値を得たいパラメータのキー値。
//      同じファイルに同じキー値を持つ行が複数ある場合、
//      最後のものが有効になる。
//
// RETURN
// キー値に合致するパラメータがあればModTrueを返し、なければModFalseを返す。
//
// EXCEPTIONS
// なし
//
ModBoolean
ModParameter::Object::readFile(ParameterValue& pv,
					   const char* path, const char* keyName)
{
	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	// パラメーターファイルをオープンする
	FILE* fp = openFile(path, "r");
	if (!fp) return ModFalse;

	char line[ModParameterFileLineSizeMax];	// 文字列(文字コードはEUC)
	ModBoolean result = ModFalse;

	// ファイルを一行ずつ読み込む
	while (fgets(line, ModParameterFileLineSizeMax, fp) != NULL) {
		// 行の解析を行う。
		// 成功した場合は line はキー値部分のみを指し、
		// 対応する値は pv に格納される。
		// (注意) keyNameは常にASCII文字なので、文字コードを意識しなくてよい。
		if (parseLine(pv, line, keyName) == ModTrue) {
			;ModAssert(ModCharTrait::compare(line, keyName) == 0);
			result = ModTrue;
		} 
	}
	fclose(fp);

	return result;
}

// FUNCTION private
// ModParameter::Object::writeFile -- システムパラメータを書き込む
//
// NOTES
// この関数は setXXX 関数においてパラメータ値を
// システムパラメータファイルに書き込むのに用いる。
//
// ARGUMENTS
// const ParameterValue& pv
//		書き込むパラメータ値
// const char* path
//		書き込み先のファイル
// const char* keyName
//		パラメータのキー値
//
// RETURN
// 成功した場合は ModTrue を返し、失敗した場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
ModBoolean
ModParameter::Object::writeFile(const ParameterValue& pv, const char* path,
						const char* keyName)
{
    ;ModAssert(keyName);

	ModAutoMutex<ModOsMutex> m(ModCommonMutex::getMutex());
	m.lock();

	// ファイルはあるか？
	FILE* fp;
	int errnoSave;
	struct stat statBuffer;
	if (stat(path, &statBuffer) < 0) {
		// なかったら作る
		fp = fopen(path, "w");
		if (fp == NULL) {
			errnoSave = errno;
			_SafeModErrorMessage << "Can't create " << path
								 << "(" << strerror(errnoSave) << ")."
								 << ModEndl;
			return ModFalse;
		}
		fclose(fp); // いったん閉じる
	}

	// パラメーターファイルを修正するためにオープンする
	fp = openFile(path, "r+b");
	if (!fp) return ModFalse;

	// writeLineを分けるとエラー処理が簡単になる
	ModBoolean result = writeLine(pv, fp, keyName);
	fclose(fp);
	return result;
}

//	FUNCTION private
//	ModParameter::Object::writeLine:: --
//		設定したパラメータ値を文字列化してファイルに書き込む
//
//	NOTES
//
//	ARGUMENTS
//	const ParameterValue& pv
//		書き込むパラメータ値
//	const char* path
//		書き込み先のファイル
//	const char* keyName
//		パラメータのキー値
//
// RETURN
// 成功した場合は ModTrue を返し、失敗した場合は ModFalse を返す。
// 
ModBoolean
ModParameter::Object::writeLine(const ParameterValue& pv,
						FILE* fp, const char* keyName)
{

	// keyNameの長さをチェックする
	ModSize keyLength = ModCharTrait::length(keyName);
	if (keyLength >= systemKeyLength) {
		_SafeModErrorMessage << "Too long key name. (" << keyName << ")"
							 << ModEndl;
		return ModFalse;
	}
	// parameterの長さをチェックする
	char tmpValue[systemParameterLength+1];
	if (pv.toLineExpression(tmpValue, systemParameterLength) == ModFalse) {
		return ModFalse;
	}

	errno = 0;
	int errnoSave;

	char tmpKeyName[systemKeyLength];
	fpos_t pos; 
	ModBoolean flag = ModFalse;

	while (fread(tmpKeyName, systemKeyLength, 1, fp) == 1) {
		// キー文字列は常にASCIIなので、文字コード変換は不要
		if (ModCharTrait::compare(tmpKeyName, keyName, keyLength) == 0
			&& ModCharTrait::isSpace(tmpKeyName[keyLength]) == ModTrue) {
			// キー文字列をもつ「最後の」行にwriteするように改めた
			(void)fgetpos(fp, &pos);
			flag = ModTrue;
		}
		// 次のレコードに移る
		(void)fseek(fp, systemParameterLength, SEEK_CUR);
	}

	if (errno != 0) {
		errnoSave = errno;
		_SafeModErrorMessage 
			<< "Can't read parameter file."
			<< "(" << strerror(errnoSave) << ")" << ModEndl;
		return ModFalse;
	}

	if (flag==ModTrue) { // 既存のエントリがあるのでそこに書き込む
		(void)fsetpos(fp, &pos);
		if (fwrite(tmpValue, systemParameterLength, 1, fp) != 1) {
			errnoSave = errno;
			_SafeModErrorMessage 
				<< "Can't write parameter value."
				<< "(" << strerror(errnoSave) << ")" << ModEndl;
			return ModFalse;
		}
	} else { // エントリがなかったので新たに書き込む
		// 配列をクリアする
		ModOsDriver::Memory::set(tmpKeyName, ' ', systemKeyLength);
		(void) ModCharTrait::copy(tmpKeyName, keyName, keyLength);
		if (fwrite(tmpKeyName, systemKeyLength, 1, fp) != 1) {
			errnoSave = errno;
			_SafeModErrorMessage
				<< "Can't write key name."
				<< "(" << strerror(errnoSave) << ")" << ModEndl;
			return ModFalse;
		}
		if (fwrite(tmpValue, systemParameterLength, 1, fp) != 1) {
			errnoSave = errno;
			_SafeModErrorMessage 
				<< "Can't write parameter value."					
				<< "(" << strerror(errnoSave) << ")" << ModEndl;
			return ModFalse;
		}
	}
	return ModTrue;
}

//	FUNCTION private
//	ModParameter::ParameterValue::toLineExpression
//		-- parameterの型に応じて書き出す文字列を作成する
//
//	NOTES
//
//	ARGUMENTS
//	char* const pszResult
//		[out]文字列表現の格納先
//	const int limitLength
//		[in]pszResultに格納できる文字数(null終端を含む)
//
// RETURN
// 成功した場合は ModTrue を返し、失敗した場合は ModFalse を返す。
// 
ModBoolean
ModParameter::ParameterValue::toLineExpression(char* const pszResult, 
											   const int limitLength) const
{
	// 配列をクリアする
	ModOsDriver::Memory::set(pszResult, ' ', limitLength-1);
	pszResult[limitLength-1] = '\n';
	pszResult[limitLength] = ModCharTrait::null();

	switch (type) {
	case typeString:
	{
		const char* iterSrc = toTmpString();
		char* iterResult = pszResult;
		char cur;

		*iterResult++ = '"';
		while((cur = *iterSrc++) != ModCharTrait::null()) {
			switch(cur) {
			case '\b':
				*iterResult++ = '\\'; *iterResult++ = 'b'; break;
			case '\f':
				*iterResult++ = '\\'; *iterResult++ = 'f'; break;
			case '\n':
				*iterResult++ = '\\'; *iterResult++ = 'n'; break;
			case '\r':
				*iterResult++ = '\\'; *iterResult++ = 'r'; break;
			case '\t':
				*iterResult++ = '\\'; *iterResult++ = 't'; break;
			case '\v':
				*iterResult++ = '\\'; *iterResult++ = 'v'; break;
			case '\\':
			case '"':
				*iterResult++ = '\\';
			default:
				*iterResult++ = cur; break;
			}			
			if (iterResult-pszResult >= limitLength-1) {
				_SafeModErrorMessage << "Too long key value." << "("
									 << ModCharTrait::length(toTmpString())
									 << ")" << ModEndl;
				ModOsDriver::Memory::set(pszResult, ' ', limitLength-1);
				pszResult[limitLength-1] = '\n';
				pszResult[limitLength] = ModCharTrait::null();
				return ModFalse;
			}
		}
		*iterResult = '"';
		break;
	}
	case typeNumeric:
		sprintf(pszResult, "%*lld\n", limitLength - 1, value.numeric);
		break;

	case typeUnsignedNumeric:
		sprintf(pszResult, "%*llu\n", limitLength - 1, value.uNumeric);
		break;

	case typeBoolean:
		if (value.boolean == ModTrue) {
			sprintf(pszResult, "True%*c\n", limitLength - 4 - 1, ' ');
		} else {
			sprintf(pszResult, "False%*c\n", limitLength - 5 - 1, ' ');
		}
		break;

	default:
		_SafeModErrorMessage << "Illegal parameter type." << ModEndl;
		return ModFalse;
	}

	return ModTrue;
}

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
