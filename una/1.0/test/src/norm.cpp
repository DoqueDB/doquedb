// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// 異表記正規化機能テストプログラム(Normalizer)
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
#include "datapath.h"
#include "ModAutoPointer.h"
#include "ModOstrStream.h"
#include "ModTime.h"
#include "ModTimeSpan.h"
#include "ModNlpUnaJp/ModNormRule.h"
#include "ModNlpUnaJp/ModNormString.h"
#include "ModNlpUnaJp/ModNormalizer.h"
#include "ModNlpUnaJp/ModNormC.h"
using namespace std;

#define MAX_DAT_LEN 655360

const ModUnicodeChar DELIM0 = 0x7b;	// {
const ModUnicodeChar DELIM1 = 0x2c;	// ,
const ModUnicodeChar DELIM2 = 0x7d;	// }
const ModUnicodeChar DELIM3 = 0x5c;	// \\

const ModUnicodeChar EXP_SEP = 0x0a; // \n

static ModTimeSpan  TOTAL_TIME;

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

void print_time()
{
	cout << "Total Time:  "
		 << TOTAL_TIME.getDays() << " "
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
		 << "[-r DIR] "
		 << "[-i FILE] "
		 << "[-o FILE] "
		 << "[-l] "
		 << "[-a [neECbXO]] "
		 << "[-s CHAR] "
		 << "[-S HEX] "
		 << "[-d0 CHAR] "
		 << "[-d1 CHAR] "
		 << "[-d2 CHAR] "
		 << "[-d3 CHAR] "
		 << "[-D0 HEX] "
		 << "[-D1 HEX] "
		 << "[-D2 HEX] "
		 << "[-D3 HEX] "
		 << "[-L NUM] "
		 << "[-b NUM] "
		 << "[-e NUM] "
		 << "[-m NUM] "
		 << "[-t] "
		 << "[-I] "
		 << "[-x]" << endl
		 << "\t-h       : このメッセージを表示" << endl
		 << "\t-r DIR   : 辞書ディレクトリ(デフォルトは../unadic)" << endl
		 << "\t-i FILE  : 入力ファイル(デフォルトは標準入力)" << endl
		 << "\t-o FILE  : 出力ファイル(デフォルトは標準出力)" << endl
		 << "\t-c CODE  : 入出力文字コード(デフォルトはutf8)" << endl
		 << "\t             euc" << endl
		 << "\t             utf8" << endl
		 << "\t             shiftJis" << endl
		 << "\t-l       : 行単位に処理(デフォルトは空行まで)" << endl
		 << "\t-a       : 実行動作(デフォルトはn)" << endl
		 << "\t             n(orm): 正規化" << endl
		 << "\t             e(xp) : 展開" << endl
		 << "\t             E(xp) : 正規化せずに展開" << endl
		 << "\t             C(hk) : 展開(省略モード)" << endl
		 << "\t             b(oth): 正規化(前処理出力)" << endl
		 << "\t             X(tr) : 前処理出力からの正規化表記抽出" << endl
		 << "\t             O(rg) : 前処理出力からの原表記抽出" << endl
		 << "\t-s  CHAR : 展開表記区切り(デフォルトは改行)" << endl
		 << "\t-S  HEX  : 展開表記区切り(デフォルトは0x0a)" << endl
		 << "\t-d0 CHAR : 前処理出力区切り0(デフォルトは'{')" << endl
		 << "\t-d1 CHAR : 前処理出力区切り1(デフォルトは',')" << endl
		 << "\t-d2 CHAR : 前処理出力区切り2(デフォルトは'}')" << endl
		 << "\t-d3 CHAR : エスケープ文字(デフォルトは'\\')" << endl
		 << "\t-D0 HEX  : 前処理出力区切り0(デフォルトは0x7b)" << endl
		 << "\t-D1 HEX  : 前処理出力区切り1(デフォルトは0x2c)" << endl
		 << "\t-D2 HEX  : 前処理出力区切り2(デフォルトは0x7d)" << endl
		 << "\t-D3 HEX  : エスケープ文字(デフォルトは0x5c)" << endl
		 << "\t-L  NUM  : テキスト分割長(デフォルトは100000)" << endl
		 << "\t-b  NUM  : 正規化開始位置(デフォルトは行頭)" << endl
		 << "\t-e  NUM  : 正規化終了位置(デフォルトは行末)" << endl
		 << "\t-m  NUM  : 確保するメモリー量(デフォルトはModSizeMax)" << endl
		 << "\t-t       : 処理時間測定" << endl
		 << "\t-I       : Cインターフェース使用" << endl
		 << "\t-x       : エラーテスト(ModException以外でFAILEDを出力)" << endl;
	return;
}

int main(int ac, char** av)
{
	ModOs::Process::setEncodingType(ModKanjiCode::literalCode);

	FILE* fin = stdin;
	FILE* fout = stdout;

	ModUnicodeString data_dir(dataDirPath);
	ModBoolean change_dir(ModFalse);

	ModBoolean expand(ModFalse), extract(ModFalse), exp_only(ModFalse);
	UNA::UNAJP::ModNormOutMode out_mode(UNA::UNAJP::ModNormalized);
	UNA::UNAJP::ModNormExpMode exp_mode(UNA::UNAJP::ModNormExpNoChk);

	ModUnicodeChar d0(DELIM0);
	ModUnicodeChar d1(DELIM1);
	ModUnicodeChar d2(DELIM2);
	ModUnicodeChar d3(DELIM3);
	ModUnicodeChar sep(EXP_SEP);
	ModSize begin(0), end(0), len(100000), mem_size(ModSizeMax >> 10);
	ModBoolean get_time(ModFalse), test_error(ModFalse);
	ModBoolean do_line(ModFalse);
	ModBoolean expStrData(ModFalse);
	bool cInterface=false;

	ModKanjiCode::KanjiCodeType io_code = ModKanjiCode::utf8;
	ModUnicodeString* target = 0;

	for (int i = 1; i < ac; i++) {
		if (strncmp(av[i], "-h", 2) == 0) {
			print_help();
            return 0;
        }

		if (strncmp(av[i], "-r", 2) == 0) { // 辞書ディレクトリの変更
			change_dir = ModTrue;
			data_dir = av[++i];
			data_dir += "/";

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
				return(1);
			}

		} else if (strncmp(av[i], "-l", 2) == 0) { // 行単位に処理
			do_line = ModTrue;

		} else if (strncmp(av[i], "-a", 2) == 0) {
			if (strncmp(av[++i], "n", 1) == 0) { // デフォルト
				;
			} else if (strncmp(av[i], "e", 1) == 0) { // 展開
				expand = ModTrue;
			} else if (strncmp(av[i], "E", 1) == 0) { // 展開オンリー
				expand = ModTrue;
				exp_only = ModTrue;
			} else if (strncmp(av[i], "C", 1) == 0) { // 省略モード
				expand = ModTrue;
				exp_mode = UNA::UNAJP::ModNormExpChkOrigStr;
			} else if (strncmp(av[i], "b", 1) == 0) { // 前処理出力
				out_mode = UNA::UNAJP::ModNormBoth;
			} else if (strncmp(av[i], "X", 1) == 0) { // 情報抽出
				extract = ModTrue;
			} else if (strncmp(av[i], "O", 1) == 0) { // 原表記抽出
				extract = ModTrue;
				out_mode = UNA::UNAJP::ModNormOriginal;
			} else {
				cerr << "ERROR: unexpected action" << endl;
				return(1);
			}

		} else if (strncmp(av[i], "-s", 2) == 0) {
			sep = ModUnicodeChar(av[++i][0]);

		} else if (strncmp(av[i], "-S", 2) == 0) {
			sep = ModUnicodeChar(strtol(av[++i], 0, 16));

		} else if (strncmp(av[i], "-d0", 3) == 0) { // デリミタ指定(文字)
			d0 = ModUnicodeChar(av[++i][0]);
		} else if (strncmp(av[i], "-d1", 3) == 0) {
			d1 = ModUnicodeChar(av[++i][0]);
		} else if (strncmp(av[i], "-d2", 3) == 0) {
			d2 = ModUnicodeChar(av[++i][0]);
		} else if (strncmp(av[i], "-d3", 3) == 0) {
			d3 = ModUnicodeChar(av[++i][0]);

		} else if (strncmp(av[i], "-D0", 3) == 0) { // デリミタ指定(16進)
            d0 = ModUnicodeChar(strtol(av[++i], 0, 16));
		} else if (strncmp(av[i], "-D1", 3) == 0) {
            d1 = ModUnicodeChar(strtol(av[++i], 0, 16));
		} else if (strncmp(av[i], "-D2", 3) == 0) {
            d2 = ModUnicodeChar(strtol(av[++i], 0, 16));
		} else if (strncmp(av[i], "-D3", 3) == 0) {
            d3 = ModUnicodeChar(strtol(av[++i], 0, 16));

		} else if (strncmp(av[i], "-L", 2) == 0) { // 分割長
			len = atoi(av[++i]);

		} else if (strncmp(av[i], "-b", 2) == 0) { // 正規化開始位置
			begin = atoi(av[++i]);
		} else if (strncmp(av[i], "-e", 2) == 0) { // 正規化終了位置
			end = atoi(av[++i]);

		} else if (strncmp(av[i], "-m", 2) == 0) { // メモリ指定
            mem_size = atoi(av[++i]);

		} else if (strncmp(av[i], "-t", 2) == 0) { // 時間測定
			get_time = ModTrue;

		} else if (strncmp(av[i], "-I", 2) == 0) { // Cインターフェース使用
			cInterface = true;

		} else if (strncmp(av[i], "-x", 2) == 0) { // エラーテスト
			test_error = ModTrue;

		} else if (strncmp(av[i], "-k", 2) == 0) { // 文字列展開データ作成用正規化処理
			expStrData = ModTrue;

		} else {
			cerr << "ERROR: unexpected parameter" << endl;
			return(1);
		}
	}

    if (!fin) {
        cerr << "ERROR: input file not opened." << endl;
        return(1);
    }
    if (!fout) {
        cerr << "ERROR: output file not opened." << endl;
        return(1);
    }


if(!cInterface)
{
	try {
		ModMemoryPool::initialize(mem_size);
		ModAutoPointer<UNA::UNAJP::ModNormRule> normRule;

		if (change_dir == ModTrue) { // 辞書ディレクトリの変更
			normRule = new UNA::UNAJP::ModNormRule(data_dir);
		} else { // datapath.hで指定されたディレクトリ
			normRule = new UNA::UNAJP::ModNormRule(ModUnicodeString(ruleDicPath),
									   ModUnicodeString(ruleAppPath),
									   ModUnicodeString(expDicPath),
									   ModUnicodeString(expAppPath),
									   ModUnicodeString(connectPath),
									   ModUnicodeString(unknownTablePath),
									   ModUnicodeString(unknownCostPath),
									   ModUnicodeString(normalTablePath),
									   ModUnicodeString(preMapPath),
									   ModUnicodeString(postMapPath),
									   ModUnicodeString(combiMapPath));
		}
		
		UNA::UNAJP::ModNormalizer norm(normRule);
		ModTime t0, t1;

		while (get_target(fin, target, io_code, do_line)) {
			// 入力中のU+FFF0, U+FFF1をそれぞれサロゲートペア前半, 後半の断片で置き換える
			target->replace(0xfff0, 0xd800);
			target->replace(0xfff1, 0xdc00);

			if (expand == ModTrue) { // 展開
				ModUnicodeString* results;

				t0 = ModTime::getCurrentTime();
				int cnt = norm.expandBuf(*target, results, 
										 exp_mode, begin, end,
										 exp_only);
				t1 = ModTime::getCurrentTime();

				if (test_error == ModFalse && get_time == ModFalse) {
					for (ModSize i = 0; i < cnt; i ++) {
						if (i > 0) {
							fputc(sep, fout);
						}
						fputs(results[i].getString(io_code), fout);
					}
					fputc(0x0a, fout);
					if (cnt > 0 && sep == '\n') {
						fputc(0x0a, fout);
					}
				}
				if (cnt > 0) {
					delete [] results, results = 0;
				}

			} else {
				ModUnicodeString result;

				if (extract == ModTrue) { // 情報抽出
					t0 = ModTime::getCurrentTime();
					norm.extractInit(*target, out_mode, d0, d1, d2, d3);
					t1 = ModTime::getCurrentTime();

					ModUnicodeChar c;
					while ((c = norm.extractGetc()) != 0) {
						result.append(c);
					}
				} else { // 正規化のみ
                    t0 = ModTime::getCurrentTime();
					if (expStrData == ModTrue) {
						norm.setExpStrMode(ModTrue);
						result = norm.normalizeBuf(*target, begin, end,
											   out_mode, d0, d1, d2,
											   d3, len);
						norm.setExpStrMode(ModFalse);
					} else {
						result = norm.normalizeBuf(*target, begin, end,
											   out_mode, d0, d1, d2,
											   d3, len);
					}
					t1 = ModTime::getCurrentTime();
				}
				if (test_error == ModFalse && get_time == ModFalse) {
					fputs(result.getString(io_code), fout);
					fputc(0x0a, fout);
				}
			}

			if (get_time == ModTrue) {
				TOTAL_TIME += (t1 - t0);
			}

			delete target, target = 0;
		}
	} catch (ModException& e) {
		delete target, target = 0;
		ModErrorMessage << "ModException!: " << e << ModEndl;
		cout << "ModException!: "
		<< e.getErrorModule() << " "
		<< e.getErrorNumber() << " "
		<< e.getErrorLevel() << "."
		<< endl;
		return(1);
	} catch (...) {
		delete target, target = 0;
		ModErrorMessage << "Unexpected exception!" << ModEndl;
		if (test_error == ModTrue) {
			cout << "FAILED" << endl;
		}
		return(1);
	}
	if (get_time == ModTrue) {
		print_time();
	}
	if (test_error == ModTrue) {
		cout << "FAILED" << endl;
		return(1);
	}
	if (fin)  fclose(fin);
	if (fout) fclose(fout);
}
else
//Cインターフェース
{

	try
	{
		ModMemoryPool::initialize(mem_size);
		UNA::UNAJP::ModNormRule* normRule;

		if (change_dir == ModTrue) { // 辞書ディレクトリの変更
			normRule = (UNA::UNAJP::ModNormRule*)modNormInit(data_dir.getString(),0);
		} else { // datapath.hで指定されたディレクトリ
			normRule = (UNA::UNAJP::ModNormRule*)modNormInit2(
									ruleDicPath,
									ruleAppPath,
									expDicPath,
									expAppPath,
									connectPath,
									unknownTablePath,
									unknownCostPath,
									normalTablePath,
									preMapPath,
									postMapPath,
									combiMapPath,
									0);
		}

		UNA::UNAJP::ModNormalizer* norm = (UNA::UNAJP::ModNormalizer*)modNormCreateNormalizer(normRule);
		ModTime t0, t1;

		ModUnicodeString* target = 0;
		while (get_target(fin, target, io_code, do_line)) {

			if (expand == ModTrue) { // 展開
				ModUnicodeString* results;
				int cnt = 0;

				t0 = ModTime::getCurrentTime();
				results = (ModUnicodeString*)modNormExpand(norm,target->getString(ModKanjiCode::utf8),&cnt);
				t1 = ModTime::getCurrentTime();

				if (test_error == ModFalse && get_time == ModFalse) {
					for (ModSize i = 0; i < cnt; i ++) {
						if (i > 0) {
							fputc(sep, fout);
						}
						const char* resultsUTF8 = modNormGetExpandedString(results,i);
						ModUnicodeString resultsUni(resultsUTF8,ModKanjiCode::utf8);
						fputs(resultsUni.getString(io_code), fout);
					}
					fputc(0x0a, fout);
					if (cnt > 0 && sep == '\n') {
						fputc(0x0a, fout);
					}
				}
				if (cnt > 0) {
					modNormDestroyExpanded(results);
				}

			} else { // 正規化のみ
				ModAutoPointer<ModUnicodeString> result;

				t0 = ModTime::getCurrentTime();
				result = (ModUnicodeString*)modNormNormalize(norm,target->getString(ModKanjiCode::utf8));
				t1 = ModTime::getCurrentTime();
				if (test_error == ModFalse && get_time == ModFalse) {
					const char* resultUTF8 = modNormGetString(result.get());
					ModUnicodeString resultUni(resultUTF8,ModKanjiCode::utf8);
					fputs(resultUni.getString(io_code), fout);
					fputc(0x0a, fout);
				}
				modNormDestroyResult(result.release());
			}

			if (get_time == ModTrue) {
				TOTAL_TIME += (t1 - t0);
			}

			delete target, target = 0;
		}
		modNormDestroyNormalizer(norm);
		modNormTerm(normRule);
	} catch (ModException& e) {
		ModErrorMessage << "ModException!: " << e << ModEndl;
		cout << "ModException!: "
		<< e.getErrorModule() << " "
		<< e.getErrorNumber() << " "
		<< e.getErrorLevel() << "."
		<< endl;
		return(1);
	} catch (...) { 
		ModErrorMessage << "Unexpected exception!" << ModEndl;
		if (test_error == ModTrue) {
			cout << "FAILED" << endl;
		}
		return(1);
	}
	if (get_time == ModTrue) {
		print_time();
	}
	if (test_error == ModTrue) {
		cout << "FAILED" << endl;
		return(1);
	}
	if (fin)  fclose(fin);
	if (fout) fclose(fout);
}

	return 0;
}
