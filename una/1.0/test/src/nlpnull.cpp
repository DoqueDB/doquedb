// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// 形態素解析＋正規化機能メモリサイズチェック用
// プログラム(UNA未使用時のメモリサイズ)
// 
// Copyright (c) 2003, 2022, 2023 Ricoh Company, Ltd.
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
#include "ModOstrStream.h"
#include "ModUnicodeString.h"
#include "ModTime.h"
#include "ModTimeSpan.h"
#include "ModLanguageSet.h"

using namespace std;

#define MAX_DAT_LEN 655360

const ModUnicodeString NLP_PATH("../unadic/");

const ModUnicodeChar SEPARATOR = 0x0a; // \n
const ModUnicodeChar EXP_SEP   = 0x2c; // ,

static ModTimeSpan TOTAL_TIME;

void print_time()
{
	cout << "Total Time:  "
		<< TOTAL_TIME.getDays() << " "
		<< TOTAL_TIME.getHours() << " "
		<< TOTAL_TIME.getMinutes() << " "
		<< TOTAL_TIME.getSeconds() << "."
		<< setw(3) << setfill('0')
		<< TOTAL_TIME.getMilliSeconds()
		<< endl;
	return;
}

ModBoolean
get_target(FILE*& file, ModUnicodeString*& target,
		   ModKanjiCode::KanjiCodeType code, ModBoolean do_line) {

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

void print_help() {
	cerr << "Usage: "
		 << "[-h] "
		 << "[-r DIR] "
		 << "[-i FILE] "
		 << "[-o FILE] "
		 << "[-l] "
		 << "以下のオプションはすべて無効. nlpnormとできるだけ "
		 << "同じコードにするために出力しているだけ "
		 << "[-w maxWordLength] "
		 << "[-p] "
		 << "[-x] "
		 << "[-d] "
		 << "[-t] "
		 << "[-s1 CHAR] "
		 << "[-S1 HEX] "
		 << "[-s2 CHAR] "
		 << "[-S2 HEX] "
		 << "[-m  MODE] "
		 << "[-e] "
		 << "[-C] "
		 << "[-X] "
		 << "\t-h       : このメッセージを表示" << endl
		 << "\t-r DIR   : 辞書ディレクトリ(デフォルトは../unadic)" << endl
		 << "\t-i FILE  : 入力ファイル(デフォルトは標準入力)" << endl
		 << "\t-o FILE  : 出力ファイル(デフォルトは標準出力)" << endl
		 << "\t-c CODE  : 入出力文字コード(デフォルトはutf8)" << endl
		 << "\t             euc" << endl
		 << "\t             utf8" << endl
		 << "\t             shiftJis" << endl
		 << "\t-l       : １行ずつ処理(デフォルトは空行で区切る)" << endl
		 << "\t-L       : 言語指定 en/ja+en/es のように複数指定可能" << endl
		 << "\t-t       : 関数実行時間を測定" << endl
		 << "\t-O       : 原表記取得" << endl
		 << "\t-k       : 原表記取得&品詞取得(キーワード抽出用)" << endl
		 << "\t-p       : 原文を表示" << endl
		 << "\t-x       : 正規化を行わない(デフォルトは実施)" << endl
		 << "\t-d       : 詳細情報を表示(区切りはデフォルト)" << endl
		 << "\t-s1 CHAR : 形態素区切り(デフォルトは改行)" << endl
		 << "\t-S1 HEX  : 形態素区切り(デフォルトは0x0a)" << endl
		 << "\t-s2 CHAR : 展開表記区切り(デフォルトはコンマ)" << endl
		 << "\t-S2 HEX  : 展開表記区切り(デフォルトは0x2c)" << endl
		 << "\t-m  MODE : 下位構造展開(MODE=1,2)/ステミング実施(MODE=1,3)" << endl
		 << "\t-e(xpand): 異表記展開を実施(デフォルトは正規化のみ)" << endl
		 << "\t-C(heck) : 展開時に包含関係な候補を削除する" << endl
		 << "\t-X       : エラーテスト(ModException以外でFAILEDを出力)" << endl;
	return;
}

int main(int ac, char** av)
{
	ModOs::Process::setEncodingType(ModKanjiCode::literalCode);
	ModMemoryPool::setTotalLimit(256*1024);
	ModTime t0;
	ModTime t1;

	FILE* fin = stdin;
	FILE* fout = stdout;

	ModUnicodeString nlp_path(NLP_PATH);
	ModLanguageSet nlp_lang[5];
	nlp_lang[0] = ModUnicodeString("ja+en");
	ModSize	nlp_lang_num=1;

	int mode(1);
	ModUnicodeChar sep(SEPARATOR);
	ModUnicodeChar exp_sep(EXP_SEP);

	ModBoolean do_line(ModFalse);
	ModBoolean print_input(ModFalse);
	ModBoolean print_detail(ModFalse);
	ModBoolean norm_mode(ModTrue);
	ModBoolean get_time(ModFalse);
	ModBoolean get_original(ModFalse);
	ModBoolean use_keyWordFunc(ModFalse);
	ModBoolean rv;
	ModSize maxWordLength=0;

	ModBoolean expand(ModFalse);
	ModBoolean test_error(ModFalse);

	ModKanjiCode::KanjiCodeType io_code = ModKanjiCode::utf8;


	for (int i = 1; i < ac; i++) {
		if (strncmp(av[i], "-h", 2) == 0) {
			print_help();
			return 0;
		}
		if (strncmp(av[i], "-r", 2) == 0) { // データディレクトリ
			nlp_path = av[++i];
			nlp_path += "/";
		} else if (strncmp(av[i], "-m", 2) == 0) { // モード指定
			mode = atoi(av[++i]);
		} else if (strncmp(av[i], "-w", 2) == 0) { // モード指定
			maxWordLength = atoi(av[++i]);
	
		} else if (strncmp(av[i], "-i", 2) == 0) { // 入力ファイル
			fin = fopen(av[++i], "rb");
		} else if (strncmp(av[i], "-o", 2) == 0) { // 出力ファイル
			fout = fopen(av[++i], "wb");
		} else if (strncmp(av[i], "-L", 2) == 0) { // 言語
			i++;
			int st=0;
			nlp_lang_num=0;
			for ( int ed=0;;++ed){ // 複数言語指定の解析
				if ( av[i][ed]=='/' ||av[i][ed]==0){
					nlp_lang[nlp_lang_num]=ModUnicodeString((av[i]+st),(ed-st));
					nlp_lang_num++;
					if ( nlp_lang_num>=5){
						break;
					}
					st = ed+1;
					if ( av[i][ed]==0)
						break;
				}
			}
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

		} else if (strncmp(av[i], "-l", 2) == 0) { 
			do_line = ModTrue;

		} else if (strncmp(av[i], "-p", 2) == 0) { 
			print_input = ModTrue;

		} else if (strncmp(av[i], "-x", 2) == 0) { 
			norm_mode = ModFalse;

		} else if (strncmp(av[i], "-d", 2) == 0) { 
			print_detail = ModTrue;
			sep = SEPARATOR;
			exp_sep = EXP_SEP;

		} else if (strncmp(av[i], "-s1", 3) == 0) { 
			if (print_detail == ModFalse) { 
				sep = ModUnicodeChar(av[++i][0]);
			}
		} else if (strncmp(av[i], "-S1", 3) == 0) { 
			if (print_detail == ModFalse) { 
				sep = ModUnicodeChar(strtol(av[++i], 0, 16));
			}

		} else if (strncmp(av[i], "-s2", 3) == 0) { 
			if (print_detail == ModFalse) { 
				exp_sep = ModUnicodeChar(av[++i][0]);
			}
		} else if (strncmp(av[i], "-S2", 3) == 0) { 
			if (print_detail == ModFalse) { 
				exp_sep = ModUnicodeChar(strtol(av[++i], 0, 16));
			}
		} else if (strncmp(av[i], "-e", 2) == 0) { 
			expand = ModTrue;
		} else if (strncmp(av[i], "-t", 2) == 0) { 
			get_time = ModTrue;
		} else if (strncmp(av[i], "-O", 2) == 0) { 
			get_original = ModTrue;
		} else if (strncmp(av[i], "-k", 2) == 0) { 
			use_keyWordFunc = ModTrue;
		} else if (strncmp(av[i], "-X", 2) == 0) { 
			test_error = ModTrue;
		} else {
			cerr << "ERROR: unexpected parameter " << av[i] << endl;
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
		ModMemoryPool::initialize(ModSizeMax >> 10);

		ModUnicodeString* target = 0;
		TOTAL_TIME = 0;
		ModSize currentLang=0;
		int dummyCount;
		while (get_target(fin, target, io_code, do_line)) {
			try{
				dummyCount = 100;  /* ダミーの単語数 */
			} catch (ModException& e) {
				ModErrorMessage << "ModException!: " << e << ModEndl;
				ModErrorHandle::reset();
				currentLang = (currentLang+1)%nlp_lang_num;
				delete target, target = 0;
				continue;
			} catch (...) { 
				ModErrorMessage << "Unexpected exception!" << ModEndl;
				ModErrorHandle::reset();
				currentLang = (currentLang+1)%nlp_lang_num;
				delete target, target = 0;
				continue;
			}
			currentLang = (currentLang+1)%nlp_lang_num;

			if (print_input == ModTrue) {
				fputs(target->getString(io_code), fout);
				fputc(0x0a, fout);
			}

			if (expand == ModTrue) {	// 展開
				ModUnicodeString* words = 0;
				ModSize wnum(0);
				int num(0);
				while (1){
					try{
						t0 = ModTime::getCurrentTime();
						dummyCount --;
						if ( dummyCount <0){
							dummyCount = 100;
							wnum = 0;
						}
						else{
							wnum = 1;
						}
						t1 = ModTime::getCurrentTime();
					} catch (ModException& e) {
						ModErrorMessage << "ModException!: " << e << ModEndl;
						ModErrorHandle::reset();
						continue;
					} catch (...) { 
						ModErrorMessage << "Unexpected exception!" << ModEndl;
						ModErrorHandle::reset();
						continue;
					}
					TOTAL_TIME = TOTAL_TIME + (t1-t0);
					if ( wnum<=0){
						break;
					}
					++ num;
					if (sep != '\n' || num > 1) {
						fputc(sep, fout);
					}
					if (print_detail == ModTrue) {
						fprintf(fout, "%d", num);
						fputc(0x20, fout);
					}
					fputs(words[0].getString(io_code), fout);
					for (int xx = 1; xx < wnum; ++xx){
						fputc(exp_sep, fout);
						fputs(words[xx].getString(io_code), fout);
					}
					// 忘れずに・・・
					if (wnum >0){
						delete [] words, words = 0;
					}
				}

			} else {	// 正規化のみ
				ModUnicodeString word;
				int num(0);
				while (1){
					ModUnicodeString ostring;
					int pos;
					dummyCount --;
					if ( dummyCount <0){
						dummyCount = 100;
						rv = ModFalse;
					}
					else{
						rv = ModTrue;
					}
					if ( use_keyWordFunc == ModTrue){
						ModUnicodeChar *ostr;
						ModSize len;
						t0 = ModTime::getCurrentTime();
						t1 = ModTime::getCurrentTime();
						ostring = ModUnicodeString(ostr,len);
					}
					else if ( get_original == ModFalse){
						t0 = ModTime::getCurrentTime();
						t1 = ModTime::getCurrentTime();
					}
					else{
						t0 = ModTime::getCurrentTime();
						t1 = ModTime::getCurrentTime();
					}

					TOTAL_TIME = TOTAL_TIME + (t1-t0);
					if ( rv != ModTrue){
						break;
					}
					++ num;
					if (sep != '\n' || num > 1) {
						fputc(sep, fout);
					}
					if (print_detail == ModTrue) {
						fprintf(fout, "%d", num);
						fputc(0x20, fout);
					}
 					fputs(word.getString(io_code), fout);
					if ( get_original == ModTrue ){
						fputc(':', fout);
						fputs(ostring.getString(io_code), fout);
					}
					else if ( use_keyWordFunc == ModTrue){
						fputc(':', fout);
						fputs(ostring.getString(io_code), fout);
						fprintf(fout,"  %d", pos);
					}
				}
			}
			fputc(sep, fout);
			fputc(0x0a, fout);
			if (do_line == ModFalse && sep != '\n') {
				fputc(0x0a, fout);
			}
			delete target, target = 0;
		}


		if (fin) fclose(fin);
		if (fout) fclose(fout);

	} catch (ModException& e) {
		ModErrorMessage << "ModException!: " << e << ModEndl;
			cout << "ModException!:  "
				<< e.getErrorModule() << " "
				<< e.getErrorNumber() << " "
				<< e.getErrorLevel() << " ";
		cout << e.getMessageBuffer();
		cout << "." << endl;
		exit(1);
	} catch (...) { 
		ModErrorMessage << "Unexpected exception!" << ModEndl;
		if (test_error == ModTrue) {
			cout << "FAILED" << endl;
		}
		exit(1);
	}

	if (test_error == ModTrue) {
		cout << "FAILED" << endl;
	}
	if ( get_time == ModTrue){
		print_time();
	}
	return 0;
}

