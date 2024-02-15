// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// 形態素解析＋正規化機能テストプログラム(UnaAnalyzer+Normalizer+Stemmer)
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
#include "ModOstrStream.h"
#include "ModUnicodeString.h"
#include "LibUna/DicSet.h"
#include "ModNlpUnaJp/ModUNA.h"
#include "ModNlpUnaJp/ModNormRule.h"
#include "ModNlpUnaJp/ModNormString.h"
#include "ModNlpUnaJp/ModNormalizer.h"
#include "EnStem/ModEnglishWordStemmer.h"

using namespace std;

#define MAX_DAT_LEN 655360

const ModUnicodeString UNA_PATH("../unadic/una/");
const ModUnicodeString NORM_PATH("../unadic-n/");

const ModUnicodeChar SEPARATOR = 0x0a; // \n
const ModUnicodeChar EXP_SEP   = 0x2c; // ,

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
		 << "[-r1 DIR] "
		 << "[-r2 DIR] "
		 << "[-i FILE] "
		 << "[-o FILE] "
		 << "[-l] "
		 << "[-p] "
		 << "[-d] "
		 << "[-s1 CHAR] "
		 << "[-S1 HEX] "
		 << "[-s2 CHAR] "
		 << "[-S2 HEX] "
		 << "[-m  MODE] "
		 << "[-e] "
		 << "[-C] " << endl
		 << "\t-h       : このメッセージを表示" << endl
		 << "\t-i FILE  : 入力ファイル(デフォルトは標準入力)" << endl
		 << "\t-o FILE  : 出力ファイル(デフォルトは標準出力)" << endl
		 << "\t-r1 DIR  : 形態素解析辞書ディレクトリ(デフォルトは../unadic/una)" << endl
		 << "\t-r2 DIR  : 異表記辞書ディレクトリ(デフォルトは../unadic-n)" << endl
		 << "\t-c CODE  : 入出力文字コード(デフォルトはutf8)" << endl
		 << "\t             euc" << endl
		 << "\t             utf8" << endl
		 << "\t             shiftJis" << endl
		 << "\t-l       : １行ずつ処理(デフォルトは空行で区切る)" << endl
		 << "\t-p       : 原文を表示" << endl
		 << "\t-d       : 詳細情報を表示(区切りはデフォルト)" << endl
		 << "\t-s1 CHAR : 形態素区切り(デフォルトは改行)" << endl
		 << "\t-S1 HEX  : 形態素区切り(デフォルトは0x0a)" << endl
		 << "\t-s2 CHAR : 展開表記区切り(デフォルトはコンマ)" << endl
		 << "\t-S2 HEX  : 展開表記区切り(デフォルトは0x2c)" << endl
		 << "\t-m  MODE : 下位構造展開(MODE=1,2)/ステミング実施(MODE=1,3)" << endl
		 << "\t-e(xpand): 異表記展開を実施(デフォルトは正規化のみ)" << endl
		 << "\t-C(heck) : 展開時に包含関係な候補を削除する" << endl;
	return;
}

int main(int ac, char** av)
{
	ModOs::Process::setEncodingType(ModKanjiCode::literalCode);
	ModMemoryPool::setTotalLimit(256*1024);

	FILE* fin = stdin;
	FILE* fout = stdout;

	ModUnicodeString una_path(UNA_PATH);
	ModUnicodeString norm_path(NORM_PATH);

	int mode(1);
	ModUnicodeChar sep(SEPARATOR);
	ModUnicodeChar exp_sep(EXP_SEP);

	ModBoolean do_line(ModFalse);
	ModBoolean print_input(ModFalse);
	ModBoolean print_detail(ModFalse);

	ModBoolean expand(ModFalse);
	UNA::UNAJP::ModUnaExpMode expMode = UNA::UNAJP::ModUnaExpNoChk;

	ModKanjiCode::KanjiCodeType io_code = ModKanjiCode::utf8;

    for (int i = 1; i < ac; i++) {
        if (strncmp(av[i], "-h", 2) == 0) {
			print_help();
            return 0;
        }

		if (strncmp(av[i], "-r1", 3) == 0) { // UNAデータ
			una_path = av[++i];
			una_path += "/";
		} else if (strncmp(av[i], "-r2", 3) == 0) { // 異表記データ
			norm_path = av[++i];
			norm_path += "/";
		} else if (strncmp(av[i], "-m", 2) == 0) { // モード指定
			mode = atoi(av[++i]);
		} else if (strncmp(av[i], "-i", 2) == 0) { // 入力ファイル
			fin = fopen(av[++i], "rb");
		} else if (strncmp(av[i], "-o", 2) == 0) { // 出力ファイル
			fout = fopen(av[++i], "wb");
	
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

		} else if (strncmp(av[i], "-C", 2) == 0) { 
			expMode = UNA::UNAJP::ModUnaExpChkOrigStr;

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
		ModMemoryPool::initialize(ModSizeMax >> 10);

		UNA::UNAJP::ModNormRule normRule(norm_path);
		UNA::UNAJP::ModNormalizer norm(&normRule);
		norm_path += "stemmer.dat";
		UNA::ENSTEM::ModEnglishWordStemmer stem(norm_path);

		UNA::UNAJP::ModUnaResource *rr = new UNA::UNAJP::ModUnaResource(una_path);
		UNA::UNAJP::ModUnaAnalyzer *una = new UNA::UNAJP::ModUnaAnalyzer(rr, &norm, &stem);

		una->setStemmer(&stem);

		ModUnicodeString* target = 0;
		while (get_target(fin, target, io_code, do_line)) {
			// 入力中のU+FFF0, U+FFF1をそれぞれサロゲートペア前半, 後半の断片で置き換える
			target->replace(0xfff0, 0xd800);
			target->replace(0xfff1, 0xdc00);

			una->set(*target, mode);

			if (print_input == ModTrue) {
				fputs(target->getString(io_code), fout);
				fputc(0x0a, fout);
			}

			if (expand == ModTrue) {	// 展開
				ModUnicodeString* words = 0;
				ModUnicodeString x;
				int pos;
				ModSize wnum(0);
				int num(0);
				while ((wnum = una->getExpand(words,expMode,x,pos)) > 0){
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
				while (una->getNormalized(word, &norm, &stem) == ModTrue) {
                    ++ num;
                    if (sep != '\n' || num > 1) {
                        fputc(sep, fout);
                    }
                    if (print_detail == ModTrue) {
                        fprintf(fout, "%d", num);
                        fputc(0x20, fout);
                    }
 					fputs(word.getString(io_code), fout);
				}
			}
			fputc(sep, fout);
			fputc(0x0a, fout);
			if (do_line == ModFalse && sep != '\n') {
				fputc(0x0a, fout);
			}
			delete target, target = 0;
		}

		delete una, una = 0;
		delete rr, rr = 0;

        if (fin)  fclose(fin);
        if (fout) fclose(fout);

	} catch (ModException& e) {
		ModErrorMessage << "ModException!: " << e << ModEndl;
        exit(1);
	} catch (...) { 
		ModErrorMessage << "Unexpected exception!" << ModEndl;
        exit(1);
	}

	return 0;
}

