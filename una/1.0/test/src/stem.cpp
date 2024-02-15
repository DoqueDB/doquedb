// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// 英単語正規化機能テストプログラム(Stemmer+Normalizer)
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
#include "ModAutoPointer.h"
#include "ModOstrStream.h"
#include "ModVector.h"
#include "ModTime.h"
#include "ModTimeSpan.h"
#include "EnStem/ModEnglishWordStemmer.h"
#include "ModNlpUnaJp/ModNormRule.h"
#include "ModNlpUnaJp/ModNormString.h"
#include "ModNlpUnaJp/ModNormalizer.h"

using namespace std;

#define MAX_DAT_LEN 655360

const ModUnicodeString DATA_PATH("../unadic-n/");

const ModUnicodeChar EXP_SEP = 0x0a; // \n

static ModTimeSpan 	TOTAL_TIME;

void print_time()
{
	cout << TOTAL_TIME.getDays() << " "
		 << TOTAL_TIME.getHours() << " "
		 << TOTAL_TIME.getMinutes() << " "
		 << TOTAL_TIME.getSeconds() << "." 
		 << setw(3) << setfill('0') 
		 <<	TOTAL_TIME.getMilliSeconds()
		 << endl;
	return;
}

void print_help() {
	cerr << "Usage: "
		 << "[-h] "
		 << "[-M FILE1 FILE2] "
		 << "[-r DIR] "
		 << "[-i FILE] "
		 << "[-o FILE] "
		 << "[-a [sel]] "
		 << "[-n] "
		 << "[-s CHAR] "
		 << "[-S HEX] "
		 << "[-m NUM] "
		 << "[-t] " 
		 << "[-x]" << endl
		 << "\t-h      : このメッセージを表示" << endl
		 << "\t-M FILE1 FILE2 : 実行データ作成" << endl
		 << "\t            FILE1: ソースデータ指定ファイル" << endl
		 << "\t            FILE2: 実行データ書き出し用ファイル" << endl
		 << "\t-r DIR  : 辞書ディレクトリ(デフォルトは../unadic-n)" << endl
		 << "\t-i FILE : 入力ファイル(デフォルトは標準入力)" << endl
		 << "\t-o FILE : 出力ファイル(デフォルトは標準出力)" << endl
		 << "\t-c CODE : 入出力文字コード(デフォルトはutf8)" << endl
		 << "\t            euc" << endl
		 << "\t            utf8" << endl
		 << "\t            shiftJis" << endl
		 << "\t-a      : 実行動作(デフォルトはs)" << endl
		 << "\t            s(tem)  : 正規化" << endl
		 << "\t            e(xpand): 展開" << endl
		 << "\t            l(ook)  : 辞書引き" << endl
		 << "\t-n      : 正規化実施(デフォルトは非実施)" << endl
		 << "\t-s CHAR : 展開表記区切り(デフォルトは改行)" << endl
		 << "\t-S HEX  : 展開表記区切り(デフォルトは0x0a)" << endl
		 << "\t-m NUM  : 確保するメモリー量(デフォルトはModSizeMax)" << endl
		 << "\t-t      : 処理時間測定" << endl
		 << "\t-x      : エラーテスト(ModException以外でFAILEDを出力)" << endl;
}

int main(int ac, char** av)
{
	ModOs::Process::setEncodingType(ModKanjiCode::literalCode);

	FILE* fin = stdin;
	FILE* fout = stdout;

	ModUnicodeString data_path(DATA_PATH);
	ModUnicodeString make_path, save_path, load_path;

    ModUnicodeChar sep(EXP_SEP);

	ModBoolean make(ModFalse), look(ModFalse), expand(ModFalse);
	ModBoolean get_time(ModFalse), test_error(ModFalse);
	ModSize mem_size(ModSizeMax >> 10);	// KB 単位

	ModBoolean do_norm(ModFalse);
	UNA::UNAJP::ModNormOutMode norm_mode(UNA::UNAJP::ModNormalized);

	ModKanjiCode::KanjiCodeType io_code = ModKanjiCode::utf8;

	for (int i = 1; i < ac; i ++) {
		if (strncmp(av[i], "-h", 2) == 0) {
			print_help();
			return 1;
		}

		if (strncmp(av[i], "-M", 2) == 0) { // 実行データ作成
			make = ModTrue;
			make_path = av[++ i];
			save_path = av[++ i];

		} else if (strncmp(av[i], "-r", 2) == 0) { // 辞書ディレクトリ指定
			data_path = av[++ i];
			data_path += "/";

		} else if (strncmp(av[i], "-i", 2) == 0) { // 入力ファイル
            fin = fopen(av[++i], "rb");
		} else if (strncmp(av[i], "-o", 2) == 0) { // 出力ファイル
            fout = fopen(av[++i], "wb");

		} else if (strncmp(av[i], "-c", 2) == 0) { // 文字コード
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

		} else if (strncmp(av[i], "-a", 2) == 0) { // 実行動作
			if (strncmp(av[++i], "s", 1) == 0) { // デフォルト
				;
			} else if (strncmp(av[i], "e", 1) == 0) { // 展開
				expand = ModTrue;
            } else if (strncmp(av[i], "l", 1) == 0) { // 辞書引き
				look = ModTrue;
			} else{
				cerr << "ERROR: unexpected action" << endl;
				exit(1);
			}

		} else if (strncmp(av[i], "-n", 2) == 0) { // 正規化実施
			do_norm = ModTrue;

		} else if (strncmp(av[i], "-s", 2) == 0) {
			sep = ModUnicodeChar(av[++i][0]);
		} else if (strncmp(av[i], "-S", 2) == 0) {
			sep = ModUnicodeChar(strtol(av[++i], 0, 16));

		} else if (strncmp(av[i], "-m", 2) == 0) { // メモリ指定
			mem_size = atoi(av[++i]);

		} else if (strncmp(av[i], "-t", 2) == 0) { // 時間測定
			get_time = ModTrue;
		} else if (strncmp(av[i], "-x", 2) == 0) { // エラーテスト
			test_error = ModTrue;
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
		ModMemoryPool::setTotalLimit(mem_size);
		ModAutoPointer<UNA::UNAJP::ModNormRule> normRule;
		ModAutoPointer<UNA::UNAJP::ModNormalizer> norm;

		if (make == ModTrue) { // 実行データ作成
			UNA::ENSTEM::ModEnglishWordStemmerDataPath path(make_path);
			UNA::ENSTEM::ModEnglishWordStemmer stemmer(path, save_path);
			if (test_error == ModTrue) {
				cout << "FAILED" << endl;
			}
			return 0;
		}

		if (do_norm == ModTrue) {
			normRule  = new UNA::UNAJP::ModNormRule(data_path);
			norm      = new UNA::UNAJP::ModNormalizer(normRule);
		}
		load_path = data_path + "stemmer.dat";
		UNA::ENSTEM::ModEnglishWordStemmer stemmer(load_path);

		ModTime t0, t1;

		char buf[MAX_DAT_LEN];
		while (fgets(buf, MAX_DAT_LEN, fin)) {
			// １行ずつ処理する
			ModUnicodeString target(buf, io_code);
			ModUnicodeChar* r = target.rsearch('\n');
			if (r != 0) {   // 末尾の改行を削除
				target.truncate(r);
			}
			
			if (do_norm == ModTrue) {
				target = norm->normalizeBuf(target, 0, 0, norm_mode);
			}

			if (expand == ModTrue) {
				ModVector<ModUnicodeString> results;

				t0 = ModTime::getCurrentTime();
				stemmer.expand(target, results);
				t1 = ModTime::getCurrentTime();

				if (test_error == ModFalse && get_time == ModFalse) {
					for (ModSize i = 0; i < results.getSize(); i++) {
						if (i > 0) {
							fputc(sep, fout);
						}
						fputs(results[i].getString(io_code), fout);
					}
					fputc(0x0a, fout);
					if (sep == '\n') {
						fputc(0x0a, fout);
					}
				}

			} else if (look == ModTrue) {
				ModBoolean result;

				t0 = ModTime::getCurrentTime();
				result = stemmer.look(target);
				t1 = ModTime::getCurrentTime();

				if (test_error == ModFalse && get_time == ModFalse) {
					if (result == ModTrue) {
						fputs("1", fout);
					} else {
						fputs("0", fout);
					}
					fputc(0x0a, fout);
				}

			} else {
				ModUnicodeString result;

				t0 = ModTime::getCurrentTime();
				stemmer.stem(target, result);
				t1 = ModTime::getCurrentTime();

				if (test_error == ModFalse && get_time == ModFalse) {
					fputs(result.getString(io_code), fout);
					fputc(0x0a, fout);
				}
 			}

			if (get_time == ModTrue) {
				TOTAL_TIME += (t1 - t0);
			}
		}
	}
 	catch (ModException& e) {
		ModErrorMessage << "ModException!: " << e << ModEndl;
		cout << "ModException!: "
		<< e.getErrorModule() << " "
		<< e.getErrorNumber() << " "
		<< e.getErrorLevel() << "."
		<< endl;
		exit(1);
    }
 	catch (...) {
		ModErrorMessage << "Unexpected exception!" << ModEndl;
		if (test_error == ModTrue) {
			cout << "FAILED" << endl;
		}
		exit(1);
	}
	if (get_time == ModTrue) {
		print_time();
	}
	if (test_error == ModTrue) {
		cout << "FAILED" << endl;
		exit(1);
	}
	if (fin)  fclose(fin);
	if (fout) fclose(fout);
	return 0;
}

//
// end of file
//
