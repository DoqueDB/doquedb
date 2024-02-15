// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// 形態素解析機能テストプログラム(unacore)
// 
// Copyright (c) 2002, 2022, 2023 Ricoh Company, Ltd.
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

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include "ModUnicodeString.h"
#include "ModOstrStream.h"
#include "ModNlpUnaJp/unakapi.h"

using namespace std;

#define MAX_DAT_LEN 655360

const ModUnicodeString DATA_PATH("../unadic/una/");

const ModUnicodeChar SEPARATOR = 0x0a; // \n

// 内部の関数
void print_help();
ModBoolean get_target(FILE*& file, ModUnicodeString*& target,
		ModKanjiCode::KanjiCodeType code, ModBoolean do_line);

// ModUnaMiddleAnalyzerクラス相当
void Analyzer_init(const ModUnicodeString &path, ModBoolean use_unaapp);
void Analyzer_term();
void Analyzer_set(const ModUnicodeString& target, const ModSize mode);
ModBoolean Analyzer_get(ModUnicodeString& result);


// メイン関数
int main(int ac, char** av)
{
	ModOs::Process::setEncodingType(ModKanjiCode::literalCode);
	ModMemoryPool::setTotalLimit(256*1024);

	FILE* fin = stdin;
	FILE* fout = stdout;

	ModUnicodeString path(DATA_PATH);
	ModBoolean use_unaapp(ModFalse);

	int mode(1);
	ModUnicodeChar sep(SEPARATOR);

	ModBoolean do_line(ModFalse);
	ModBoolean print_input(ModFalse);
	ModBoolean print_appInfo(ModFalse);

	ModKanjiCode::KanjiCodeType io_code = ModKanjiCode::utf8;

	for (int i = 1; i < ac; i++) {
		if (strncmp(av[i], "-h", 2) == 0) {
			print_help();
			return 0;
		}
		if (strncmp(av[i], "-i", 2) == 0) { // 入力ファイル
			fin = fopen(av[++i], "rb");
		} else if (strncmp(av[i], "-o", 2) == 0) { // 出力ファイル
			fout = fopen(av[++i], "wb");
		} else if (strncmp(av[i], "-r", 2) == 0) { // 出力ファイル
			path = av[++i];
			path += "/";
		} else if (strncmp(av[i], "-a", 2) == 0) { 
			use_unaapp = ModTrue;
		} else if (strncmp(av[i], "-c", 2) == 0) { // 入出力文字コード
			if (strcmp(av[++i], "euc") == 0) {
				io_code = ModKanjiCode::euc;
			} else if (strcmp(av[i], "shiftJis") == 0) {
				io_code = ModKanjiCode::shiftJis;
			} else if (strcmp(av[i], "utf8") == 0) { // デフォルト
				;
			} else {
				cerr << "ERROR: unexpected char code" << endl;
				exit(1);
			}
		} else if (strncmp(av[i], "-m", 2) == 0) { 
			mode = atoi(av[++i]);
		} else if (strncmp(av[i], "-d", 2) == 0) { 
			print_appInfo = ModTrue;
		} else if (strncmp(av[i], "-s", 2) == 0) {
			sep = ModUnicodeChar(av[++i][0]);
		} else if (strncmp(av[i], "-S", 2) == 0) {
			sep = ModUnicodeChar(strtol(av[++i], 0, 16));
		} else if (strncmp(av[i], "-l", 2) == 0) { 
			do_line = ModTrue;
		} else if (strncmp(av[i], "-p", 2) == 0) { 
			print_input = ModTrue;
		} else {
			cerr << "ERROR: unexpected parameter" << endl;
			exit(1);
		}
	}

	if (!fin) {
		cerr << "ERROR: input file not opened." << endl;
		exit(1);
	}
	if (!fout) {
		cerr << "ERROR: output file not opened." << endl;
		exit(1);
	}

	try {
		Analyzer_init(path, use_unaapp);
		ModUnicodeString* target = 0;

		while (get_target(fin, target, io_code, do_line)) {
			// 入力中のU+FFF0, U+FFF1をそれぞれサロゲートペア前半, 後半の断片で置き換える
			target->replace(0xfff0, 0xd800);
			target->replace(0xfff1, 0xdc00);

			Analyzer_set(*target,mode);
			ModUnicodeString word;

			if (print_input == ModTrue) {
				fputs(target->getString(io_code), fout);
				fputc(0x0a, fout);
			}
			int num(0);
			while (Analyzer_get(word) == ModTrue) {
				// 品詞などの情報を削除
				if ( print_appInfo != ModTrue){
					ModUnicodeChar* p = word.search('(');
					if (word[0] != '(' && p != 0) {
						word.truncate(p);
					}
					else { /* 半角カッコがエントリに入っている場合の処理 */
						/* 半角カッコのエントリ中に'('は２箇所しか現れない */
						ModUnicodeChar* rp = word.rsearch('('); /* 末尾から捜す */
						if (word[0] == '(' && rp != p && rp !=0) {
							word.truncate(rp);
						}
					}
				}
				++ num;
				if (sep != '\n' || num > 1) {
					fputc(sep, fout);
				}
				fputs(word.getString(io_code), fout);
			}
			fputc(sep, fout);
			fputc(0x0a, fout);
			if (do_line == ModFalse && sep != '\n') {
				fputc(0x0a, fout);
			}
			delete target, target = 0;
		}

		if (fin)  fclose(fin);
		if (fout) fclose(fout);
		Analyzer_term();

	} catch (ModException& e) {
		ModErrorMessage << "ModException!: " << e << ModEndl;
		exit(1);
	} catch (...) { 
		ModErrorMessage << "Unexpected exception!" << ModEndl;
		exit(1);
	}

	return 0;
}

// ヘルプメッセージを出力する
void print_help() {
	cerr << "Usage: "
		<< "[-h] "
		<< "[-r DIR] "
		<< "[-a] "
		<< "[-i FILE] "
		<< "[-o FILE] "
		<< "[-l] "
		<< "[-p] "
		<< "[-s CHAR] "
		<< "[-S HEX] "
		<< "[-m [01]] " << endl
		<< "\t-h      : このメッセージを表示" << endl
		<< "\t-r DIR  : 辞書ディレクトリ(デフォルトは../unadic/una)" << endl
		<< "\t-a      : unaapp.dicを使用(デフォルトはunaapp2.dic)" << endl
		<< "\t-i FILE : 入力ファイル(デフォルトは標準入力)" << endl
		<< "\t-o FILE : 出力ファイル(デフォルトは標準出力)" << endl
		<< "\t-c CODE : 入出力文字コード(デフォルトはutf8)" << endl
		<< "\t            euc" << endl
		<< "\t            utf8" << endl
		<< "\t            shiftJis" << endl
		<< "\t-l      : １行ずつ処理(デフォルトは空行まで)" << endl
		<< "\t-p      : 原文を表示" << endl
		<< "\t-s CHAR : 形態素区切り(デフォルトは改行)" << endl
		<< "\t-S HEX  : 形態素区切り(デフォルトは0x0a)" << endl
		<< "\t-m [01] : 解析モード(デフォルトは1)" << endl
		<< "\t            0: まとめ上げ結果を出力" << endl
		<< "\t            1: 下位構造(ある場合)を出力" << endl;
	return;
}

// 対象のテキストを得る
ModBoolean
get_target(
	FILE*& file,                        // 入力ファイルポインタ
	ModUnicodeString*& target,          // 対象テキスト
	ModKanjiCode::KanjiCodeType code,   // 漢字コード
	ModBoolean do_line)                 // １行ごとに得るか否か
{
	char buf[MAX_DAT_LEN];

	while (fgets(buf, MAX_DAT_LEN, file)) {
		ModUnicodeString line(buf, code);

		if (do_line == ModTrue) {
			ModUnicodeChar* r = line.rsearch('\n');
			if (r != 0) {   // 末尾の改行を削除
				line.truncate(r);
			}
			target = new ModUnicodeString(line);
			return ModTrue;
		} else {
			if (target == 0) {
				target = new ModUnicodeString(line);
			} else if (line[0] == '\n') {
				ModUnicodeChar* r = target->rsearch('\n');
				if (r != 0) {   // 末尾の改行を削除
					 target->truncate(r);
				}
				return ModTrue;
			} else  {
				*target += line;
			}
		}
	}

	if (target != 0) {
		ModUnicodeChar* r = target->rsearch('\n');
		if (r != 0) {   // 末尾の改行を削除
			target->truncate(r);
		}
		return ModTrue;
	}

	return ModFalse;
}


// 
//  以下は　ModUnaMiddleAnalyzer相当の関数群
//

#define LOCAL_MOR_SIZE 256   // ローカルな形態素解析バッファのサイズ

unaKApiHandleT*       Analyzer_handle;			// una用のハンドラ
const ModUnicodeChar* Analyzer_target;			// 対象文
ModInt32              Analyzer_targetLength;	// その長さ
ModSize               Analyzer_mode;			// 解析モード
unaMorphT*            Analyzer_morBuf;			// 形態素解析バッファ
ModInt32              Analyzer_morNum;			// 解析形態素数
ModInt32              Analyzer_morCur;			// 現在の対象形態素番号
unaMorphT*            Analyzer_subMorBuf;		// 下位形態素バッファ
ModInt32              Analyzer_subMorNum;		// その格納数
ModInt32              Analyzer_subMorCur;		// 現在の対象番号

// 以下言語データのイメージ
char* Analyzer_morDic;					// UNA辞書のイメージ
char* Analyzer_appDic;					// UNAアプリ情報のイメージ
char* Analyzer_conTbl;					// UNA接続表のイメージ 
char* Analyzer_engTbl;					// UNA英語トークン表のイメージ
char* Analyzer_unkTbl;					// UNA未登録語表のイメージ
char* Analyzer_unkCost;					// UNA未登録語コストのイメージ
char* Analyzer_cnvTbl;					// 文字列標準化表のイメージ

// 言語データ読み込み関数
static void SimpleAnalyzer_load( const ModUnicodeString& dicPath_, 
	const ModUnicodeString& appPath_, const ModUnicodeString& cntPath_, 
	const ModUnicodeString& egtPath_, const ModUnicodeString& untPath_, 
	const ModUnicodeString& uncPath_, const ModUnicodeString& nrmPath_);

// 中断関数
extern "C" {
static int ctrl_c_stop();
}

//
// FUNCTION
// Analyzer_term -- デストラクタ
//
// NOTES
// 解析器のデストラクタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void Analyzer_term()
{
	unaKApi_term(Analyzer_handle);
	delete Analyzer_handle, Analyzer_handle = 0;
	delete [] Analyzer_morBuf, Analyzer_morBuf = 0;
	delete [] Analyzer_subMorBuf, Analyzer_subMorBuf = 0;
}

//
// FUNCTION
// SimpleAnalyzer_load -- ロード
//
// NOTES
// 解析用リソースをロードする。
// パスが空文字列のリソースはロードしない（この場合は例外にならない）。
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//      不正なパス名
//
void SimpleAnalyzer_load(
	const ModUnicodeString& dicPath_,    // 解析辞書へのパス
	const ModUnicodeString& appPath_,    // アプリケーション情報へのパス
	const ModUnicodeString& cntPath_,    // 接続表へのパス
	const ModUnicodeString& egtPath_,    // 英語トークン情報へのパス
	const ModUnicodeString& untPath_,    // 未登録語表へのパス
	const ModUnicodeString& uncPath_,    // 未登録語コストへのパス
	const ModUnicodeString& nrmPath_)    // 文字列標準化表へのパス
{
	int ret;

	try {
		if (dicPath_.getLength() != 0) {
			ModUnicodeString a(dicPath_);
			ret = unaKApi_readFileImg(a.getString(
				ModOs::Process::getEncodingType()),&Analyzer_morDic);
			if (ret<0){
				ModErrorMessage << "dicPath invalid: " << ret << ": "
					<< dicPath_ << ModEndl;
				ModThrow(ModModuleStandard,ModCommonErrorBadArgument,
				ModErrorLevelError);
			}
		}
		if (appPath_.getLength() != 0) {
			ModUnicodeString a(appPath_);
			ret = unaKApi_readFileImg(a.getString(
				ModOs::Process::getEncodingType()),&Analyzer_appDic);
			if (ret<0){
				ModErrorMessage << "appPath invalid: " << ret << ": " 
					<< appPath_ << ModEndl;
				ModThrow(ModModuleStandard,
				ModCommonErrorBadArgument, ModErrorLevelError); 
			} 
		}
		if (cntPath_.getLength() != 0) { 
			ModUnicodeString a(cntPath_);
			ret = unaKApi_readFileImg(a.getString(
				ModOs::Process::getEncodingType()),&Analyzer_conTbl);
			if (ret<0){
				ModErrorMessage << "cntPath invalid: " << ret << ": "
					<< cntPath_ << ModEndl;
				ModThrow(ModModuleStandard,ModCommonErrorBadArgument,
				ModErrorLevelError);
			}
		}
		if (egtPath_.getLength() != 0) { 
			ModUnicodeString a(egtPath_);
			ret = unaKApi_readFileImg(a.getString(
                ModOs::Process::getEncodingType()),&Analyzer_engTbl);
			if (ret<0){
				ModErrorMessage << "egtPath invalid: " << ret << ": " 
					<< egtPath_ << ModEndl;
				ModThrow(ModModuleStandard, 
				ModCommonErrorBadArgument, ModErrorLevelError); 
			}
		} 
		if (untPath_.getLength() != 0) { 
			ModUnicodeString a(untPath_);
			ret = unaKApi_readFileImg(a.getString(
				ModOs::Process::getEncodingType()),&Analyzer_unkTbl);
			if (ret<0){
				ModErrorMessage << "untPath invalid: " << ret << ": " 
					<< untPath_ << ModEndl; 
				ModThrow(ModModuleStandard, 
				ModCommonErrorBadArgument, ModErrorLevelError); 
			} 
		} 
		if (uncPath_.getLength() != 0) { 
			ModUnicodeString a(uncPath_);
			ret = unaKApi_readFileImg(a.getString(
				ModOs::Process::getEncodingType()),&Analyzer_unkCost);
			if (ret<0){
				ModErrorMessage << "uncPath invalid: " << ret << ": " 
					<< uncPath_ << ModEndl; 
				ModThrow(ModModuleStandard,ModCommonErrorBadArgument,
				ModErrorLevelError);
			}
		}
		if (nrmPath_.getLength() != 0) { 
			ModUnicodeString a(nrmPath_);
			ret = unaKApi_readFileImg(a.getString(
				ModOs::Process::getEncodingType()),&Analyzer_cnvTbl);
			if (ret<0){
				ModErrorMessage << "nrmPath invalid: " << ret << ": " 
					<< nrmPath_ << ModEndl;
				ModThrow(ModModuleStandard, ModCommonErrorBadArgument, 
				ModErrorLevelError);
			}
		}
	} catch (ModException& e) {
		if (Analyzer_morDic != 0) {
			unaKApi_freeImg(Analyzer_morDic);
			Analyzer_morDic = 0;
		}
		if (Analyzer_appDic != 0) {
			unaKApi_freeImg(Analyzer_appDic);
			Analyzer_appDic = 0;
		} 
		if (Analyzer_conTbl != 0) {
			unaKApi_freeImg(Analyzer_conTbl);
			Analyzer_conTbl = 0;
		}
		if (Analyzer_engTbl != 0) {
			unaKApi_freeImg(Analyzer_engTbl);
			Analyzer_engTbl = 0;
		}
		if (Analyzer_unkTbl != 0) {
			unaKApi_freeImg(Analyzer_unkTbl);
			Analyzer_unkTbl = 0;
		}
		if (Analyzer_unkCost != 0) {
			unaKApi_freeImg(Analyzer_unkCost);
			Analyzer_unkCost = 0;
		}
		if (Analyzer_cnvTbl != 0) {
			unaKApi_freeImg(Analyzer_cnvTbl);
			Analyzer_cnvTbl = 0;
		}

		ModRethrow(e);
	}
}

// FUNCTION
// Analyzer_set -- 解析対象テキストの設定
//
// NOTES
// 解析対象テキストを設定する
//
// ARGUMENTS
// const ModUnicodeString& target_
//              解析対象テキスト
// const ModSize mode_
//              解析モード
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
Analyzer_set(const ModUnicodeString& target_, const ModSize mode_)
{
	Analyzer_target = target_;
	Analyzer_targetLength = target_.getLength(); 
	Analyzer_mode = mode_;
	Analyzer_morNum = Analyzer_morCur = 0;
	Analyzer_subMorNum = Analyzer_subMorCur = 0;
}

//
// FUNCTION
// Analyzer_get -- 解析結果の取得
//
// NOTES
// 解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString& data_
//      解析結果
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
ModBoolean
Analyzer_get(ModUnicodeString& data_)
{
	data_.clear();
	ModInt32 res;
	unaAppInfoT* appInfo;

	if (Analyzer_subMorCur < Analyzer_subMorNum) { // 下位構造がある場合
		res = unaKApi_getAppInfo(Analyzer_handle,Analyzer_subMorBuf+
		Analyzer_subMorCur,&appInfo);
		if (res != UNA_ENG_TOKEN && res != UNA_KNOWN_WORD) {
			ModErrorMessage << "ERROR" << ModEndl;
			ModThrow(ModModuleStandard, ModCommonErrorUndefined, ModErrorLevelError);
		} 
		if (appInfo->len > 0) { // アプリ情報の長さが１以上
			data_.append(reinterpret_cast<ModUnicodeChar*>(appInfo->info),
			(appInfo->len)/sizeof(ModUnicodeChar));
		}
		++Analyzer_subMorCur;
		return ModTrue;
	}

	// 下位構造が無い場合
	if (Analyzer_morCur == Analyzer_morNum) { // 解析済み形態素を使い切った
		Analyzer_morCur = 0;
		Analyzer_morNum = 0;
		ModInt32 processedLength;
		if (Analyzer_targetLength > 0) {
			res = unaKApi_moAna(Analyzer_handle, Analyzer_target, 
			Analyzer_targetLength, Analyzer_morBuf, 
			&Analyzer_morNum, LOCAL_MOR_SIZE, &processedLength, 
			ctrl_c_stop,UNA_TRUE,UNA_TRUE, UNA_FALSE);
			if (res != UNA_OK) { 
				ModErrorMessage << "unaKApi_moAna error: " << res << ": " 
					<< Analyzer_target << ModEndl; 
				ModThrow(ModModuleStandard,ModCommonErrorUndefined, ModErrorLevelError);
			} 
			; ModAssert(Analyzer_targetLength >= processedLength); 
			Analyzer_targetLength -= processedLength; 
			Analyzer_target += processedLength; 
		}
		if (Analyzer_targetLength == 0 && Analyzer_morNum == 0) {
			return ModFalse;
		}
	}

	// 下位構造が無く、解析済み形態素バッファはある
	res = unaKApi_getAppInfo(Analyzer_handle, Analyzer_morBuf 
		+ Analyzer_morCur, &appInfo);
	if (res == UNA_UNKNOWN_WORD) { // アプリ情報がとれない単語
		// 表記を取り出す
		ModUnicodeChar* hyouki;
		ModSize hyoukiLength;
		if (unaKApi_getHyoki(Analyzer_handle,Analyzer_morBuf+Analyzer_morCur,
			&hyouki,&hyoukiLength) < 0){ 
			ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
			ModThrow(ModModuleStandard, ModCommonErrorUndefined, ModErrorLevelError);
		} 
		data_.append(hyouki, hyoukiLength);
	} else { // アプリ情報が取れる単語 
		if (res != UNA_KNOWN_WORD && res != UNA_ENG_TOKEN) { // エラー
			ModErrorMessage << "unaKApi_getAppInfo error: " 
				<< res << ": " << Analyzer_target << ModEndl;
			; ModAssert(res < 0 /*&& res == UNA_NO_APP_DIC*/);
			ModThrow(ModModuleStandard, ModCommonErrorUndefined, ModErrorLevelError);
		}

		if ( Analyzer_mode!=0 ){ // 下位構造展開モード
			// 下位構造ごとにアプリ情報を取り直す
			Analyzer_subMorNum = Analyzer_subMorCur = 0;
			res = unaKApi_getSubMorph(Analyzer_handle,Analyzer_morBuf+Analyzer_morCur,
			Analyzer_subMorBuf, &Analyzer_subMorNum, LOCAL_MOR_SIZE);
			if (Analyzer_subMorNum != 0) { // 下位構造があった
				res =unaKApi_getAppInfo(Analyzer_handle,Analyzer_subMorBuf+
					Analyzer_subMorCur,&appInfo);
				++Analyzer_subMorCur;
			}
		}

		// アプリ情報がとれた
		if (appInfo->len > 0) { // アプリ情報の長さが１以上
			data_.append( reinterpret_cast<ModUnicodeChar*>(appInfo->info),
			(appInfo->len)/sizeof(ModUnicodeChar));
		}
	}
	++Analyzer_morCur;

	return ModTrue;
}

//
// FUNCTION
// Analyzer_init -- コンストラクタ
//
// NOTES
// 解析器のコンストラクタ
//
// ARGUMENTS
// const ModUnicodeString* const path
//              リソース
// const ModBoolean useNormTbl_
//              文字正規化表を使用するかの指示
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorNotInitialized
//              リソースが初期化されていない
void Analyzer_init(const ModUnicodeString &path_, ModBoolean use_unaapp_)
{
	ModUnicodeString dicPath(path_);
	ModUnicodeString appPath(path_);
	ModUnicodeString conPath(path_);
	ModUnicodeString egtPath(path_);
	ModUnicodeString untPath(path_);
	ModUnicodeString uncPath(path_);
	ModUnicodeString nrmPath(path_);

	dicPath += "unawrd.dic";
	if(use_unaapp_ == ModTrue){
		appPath += "unaapp.dic";
	} else {
		appPath += "unaapp2.dic";
	}
	conPath += "connect.tbl";
	egtPath += "engmk.tbl";
	untPath += "unkmk.tbl";
	uncPath += "unkcost.tbl";
	nrmPath += "unastd.tbl";

	SimpleAnalyzer_load(dicPath, appPath, conPath, egtPath, untPath, 
		uncPath, nrmPath);

	Analyzer_target = 0;
	Analyzer_morBuf = 0; 
	Analyzer_morNum = 0; 
	Analyzer_morCur = 0;
	Analyzer_subMorBuf = 0; 
	Analyzer_subMorNum = 0; 
	Analyzer_subMorCur = 0;
	ModInt32 ret;

	Analyzer_handle = new unaKApiHandleT; 
	unaKApiDicImgT dicImg;
	dicImg.dicName  = (char*)"una";
	dicImg.morphDic = Analyzer_morDic;
	dicImg.appInfo  = Analyzer_appDic;
	dicImg.dicPrio  = 1;
	ret = unaKApi_init(Analyzer_handle, (const unaKApiDicImgT *)&dicImg, 1,
		Analyzer_conTbl, (char*)0, Analyzer_engTbl,
		Analyzer_unkTbl, Analyzer_unkCost, Analyzer_cnvTbl,0);
	if (ret < 0) {
		ModErrorMessage << "Analyzer: " << ret << ModEndl;
		delete Analyzer_handle;
		ModThrow(ModModuleStandard, ModCommonErrorNotInitialized, 
		ModErrorLevelError); 
	} 
	try {
		Analyzer_morBuf = new unaMorphT[LOCAL_MOR_SIZE]; 
		Analyzer_subMorBuf = new unaMorphT[LOCAL_MOR_SIZE];
	} catch (ModException& e) {
		ModErrorMessage << "Analyzer: " << e << ModEndl;
		// ハンドルも破棄する
		unaKApi_term(Analyzer_handle);
		delete Analyzer_handle, Analyzer_handle = 0;
		delete [] Analyzer_morBuf, Analyzer_morBuf = 0;
		delete [] Analyzer_subMorBuf, Analyzer_subMorBuf = 0;
		ModRethrow(e);
	}
}

//
// FUNCTION private
// ctrl_c_stop -- 中断処理関数
//
// NOTES
// unaKApi_moAna に渡す中断処理関数。今回は空関数。
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
extern "C" {
static int ctrl_c_stop()
{
	return 0;
}
}
