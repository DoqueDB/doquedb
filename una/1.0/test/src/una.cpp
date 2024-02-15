// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// 形態素解析機能テストプログラム(UnaAnalyzer)
// 
// Copyright (c) 2000, 2022, 2023 Ricoh Company, Ltd.
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
#include "LibUna/DicSet.h"
#include "ModNlpUnaJp/ModUnaMiddle.h"

using namespace std;

#define MAX_DAT_LEN 655360

const ModUnicodeString DATA_PATH("../unadic/");

const ModUnicodeChar SEPARATOR = 0x0a; // \n

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
		 << "[-p] "
		 << "[-s CHAR] "
		 << "[-S HEX] " << endl
		 << "\t-h      : このメッセージを表示" << endl
		 << "\t-r DIR  : 辞書ディレクトリ(デフォルトは../unadic)" << endl
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
		 << "\t-x      : 前処理非実施モード(デフォルトは実施)" << endl;
	return;
}

int main(int ac, char** av)
{
	ModOs::Process::setEncodingType(ModKanjiCode::literalCode);
	ModMemoryPool::setTotalLimit(256*1024);

	FILE* fin = stdin;
	FILE* fout = stdout;

	ModUnicodeString path(DATA_PATH);

	int mode(1);
	ModUnicodeChar sep(SEPARATOR);

	ModBoolean do_line(ModFalse);
	ModBoolean print_input(ModFalse);
	ModBoolean print_detail(ModFalse);
	ModBoolean exec_std(ModTrue);

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

		} else if (strncmp(av[i], "-s", 2) == 0) {
			if (print_detail == ModFalse) { 
				sep = ModUnicodeChar(av[++i][0]);
			}

		} else if (strncmp(av[i], "-S", 2) == 0) {
			if (print_detail == ModFalse) { 
				sep = ModUnicodeChar(strtol(av[++i], 0, 16));
			}

		} else if (strncmp(av[i], "-l", 2) == 0) { 
			do_line = ModTrue;

		} else if (strncmp(av[i], "-p", 2) == 0) { 
			print_input = ModTrue;

		} else if (strncmp(av[i], "-x", 2) == 0) { 
			exec_std = ModFalse;

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
		UNA::UNAJP::ModUnaResource* rr = new UNA::UNAJP::ModUnaResource(path);
		UNA::UNAJP::ModUnaMiddleAnalyzer* una = new UNA::UNAJP::ModUnaMiddleAnalyzer(rr);

		ModUnicodeString* target = 0;
		while (get_target(fin, target, io_code, do_line)) {
			// 入力中のU+FFF0, U+FFF1をそれぞれサロゲートペア前半, 後半の断片で置き換える
			target->replace(0xfff0, 0xd800);
			target->replace(0xfff1, 0xdc00);

			una->set(*target, mode);

			ModUnicodeString word;

			if (print_input == ModTrue) {
				fputs(target->getString(io_code), fout);
				fputc(0x0a, fout);
			}
			int num(0);
			while (una->get(word, exec_std) == ModTrue) {
				if (print_detail == ModFalse) {
					// 品詞などの情報を削除
					ModUnicodeChar* p = word.search('(');
					if (word[0] != '(' && p != 0) {
						word.truncate(p);
					}
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

