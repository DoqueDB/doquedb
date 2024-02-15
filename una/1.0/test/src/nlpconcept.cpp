// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// 形態素解析＋正規化機能テストプログラム(NlpAnalyzer)
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
#include "ModNLP.h"
#include "ModTime.h"
#include "ModTimeSpan.h"
#include "ModLanguageSet.h"
#if 0
#include "Keyword.h"
#endif

using namespace std;
using namespace UNA;

const ModSize MAX_DAT_LEN = 655360;
const ModSize MAX_LANGUAGE_NUM = 9;

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
get_target(FILE*& file, ModUnicodeString* target,
	   ModKanjiCode::KanjiCodeType code, ModBoolean do_line)
{
	char buf[MAX_DAT_LEN];
	memset( buf, 0, MAX_DAT_LEN );
	*target = ModUnicodeString("");
	while (fgets(buf, MAX_DAT_LEN, file))
	{
		if (do_line == ModTrue)
		{
			*target = ModUnicodeString( buf, code );

			ModUnicodeChar* r = target->rsearch('\n');
			if (r != 0)
			{
				// 末尾の改行を削除
				target->truncate(r);
			}

			return ModTrue;

		}
		else
		{
			if ( target->getLength() == 0 )
			{
				*target = ModUnicodeString( buf, code );
			}
			else if (buf[0]=='\n')
			//else if (buf[0]==0x0D || buf[0]==0x0A)
			{
				ModUnicodeChar* r = target->rsearch('\n');
				if(r!=0)
				{
					// 末尾の改行を削除
					target->truncate(r);
				}
				return ModTrue;
			}
			else
			{
				ModUnicodeString line( buf, code );
				*target += line;
			}
		}

		memset( buf, 0, MAX_DAT_LEN );
	}

	if ( target->getLength() != 0 )
	{
		if ( target->at(target->getLength()-1) == 0x0A )
		{
			ModUnicodeChar* r = target->rsearch('\n');
			if(r!=0)
			{
				// 末尾の改行を削除
				target->truncate(r);
			}
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
		 << "[-U] "
		 << "[-X] " << endl
		 << "\t-h       : このメッセージを表示" << endl
		 << "\t-r DIR   : 辞書ディレクトリ(デフォルトは../unadic)" << endl
		 << "\t-m  MODE : 下位構造展開(MODE=1,2)/ステミング実施(MODE=1,3)" << endl
		 << "\t-U  MODE : mode of intializing resource ( MODE = 0,1,2,3 )" << endl
		 << "\t-w WDLEN : 最大単語長" << endl
		 << "\t-i FILE  : 入力ファイル(デフォルトは標準入力)" << endl
		 << "\t-o FILE  : 出力ファイル(デフォルトは標準出力)" << endl
		 << "\t-L       : 言語指定 en/ja+en/es のように複数指定可能" << endl
		 << "\t-c CODE  : 入出力文字コード(デフォルトはutf8)" << endl
		 << "\t             euc" << endl
		 << "\t             utf8" << endl
		 << "\t             shiftJis" << endl
		 << "\t-l       : １行ずつ処理(デフォルトは空行で区切る)" << endl
		 << "\t-p       : 原文を表示" << endl
		 << "\t-P KEY VAL: パラメータ(キー文字 値文字)をセット" << endl
		 << "\t-d       : 詳細情報を表示(区切りはデフォルト)" << endl
		 << "\t-s1 CHAR : 形態素区切り(デフォルトは改行)" << endl
		 << "\t-S1 HEX  : 形態素区切り(デフォルトは0x0a)" << endl
		 << "\t-s2 CHAR : 展開表記区切り(デフォルトはコンマ)" << endl
		 << "\t-S2 HEX  : 展開表記区切り(デフォルトは0x2c)" << endl
		 << "\t-t       : 関数実行時間を測定" << endl
		 << "\t-X       : エラーテスト(ModException以外でFAILEDを出力)" << endl
		 << "\t-v       : ベクター型結果取得 " << endl
		 << "\t-npc		: 名詞句コスト取得型関数による名詞句抽出 " << endl;
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
	ModLanguageSet nlp_lang[MAX_LANGUAGE_NUM];
	nlp_lang[0] = ModUnicodeString("ja+en");
	ModSize	nlp_lang_num=1;

	int normMode(1);
	ModUnicodeChar sep(SEPARATOR);
	ModUnicodeChar exp_sep(EXP_SEP);

	ModBoolean do_line(ModFalse);
	ModBoolean print_input(ModFalse);
	ModBoolean print_detail(ModFalse);
	ModBoolean get_time(ModFalse);
	ModBoolean get_vector(ModFalse);
	ModBoolean rv;
	ModSize maxWordLength=0;
	ModBoolean npCostMode(ModFalse);
	ModBoolean test_error(ModFalse);
	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> > params;

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
			normMode = atoi(av[++i]);
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
					if ( nlp_lang_num>=MAX_LANGUAGE_NUM){
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
		} else if (strncmp(av[i], "-P", 2) == 0) { // string param
			params.insert(av[i+1], av[i+2]);
			i+=2;
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
		} else if (strncmp(av[i], "-t", 2) == 0) { 
			get_time = ModTrue;
		} else if (strncmp(av[i], "-X", 2) == 0) { 
			test_error = ModTrue;
		} else if (strncmp(av[i], "-npc", 4) == 0) {
			npCostMode = ModTrue;
		} else if (strncmp(av[i], "-v", 2) == 0) {
		    get_vector = ModTrue;
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

	ModUnicodeString cstrMaxWordLength;
	ModUnicodeString cstrStemming;
	ModUnicodeString cstrCompoundDiv;
	ModUnicodeString cstrCarriageRet;

	try {
		ModMemoryPool::initialize(ModSizeMax >> 10);
		
		ModSize currentLang = 0;
		ModLanguageSet language = nlp_lang[currentLang];
		
		ModNlpResource* rr = new ModNlpResource();
		rr->load(nlp_path,ModFalse);
		UNA::ModNlpAnalyzer* nlp = new UNA::ModNlpAnalyzer();
		nlp->setResource(rr);

		char czMaxWordLength[10];
		sprintf(czMaxWordLength,"%d",maxWordLength);
		cstrMaxWordLength = ModUnicodeString(czMaxWordLength);
		params.insert("maxwordlen",cstrMaxWordLength);
		switch (normMode)
		{
		case 0:	// ModNlpNormOnly
			cstrStemming = "false";
			cstrCompoundDiv = "false";
			cstrCarriageRet = "false";
			break;
		case 1:	// ModNlpNormStemDiv;
			cstrStemming = "true";
			cstrCompoundDiv = "true";
			cstrCarriageRet = "false";
			break;
		case 2:	// ModNlpNormDiv;
			cstrStemming = "false";
			cstrCompoundDiv = "true";
			cstrCarriageRet = "false";
			break;
		case 3:	// ModNlpNormStem;
			cstrStemming = "true";
			cstrCompoundDiv = "false";
			cstrCarriageRet = "false";
			break;
		case 4:	// ModNlpNormRet;
			cstrStemming = "false";
			cstrCompoundDiv = "false";
			cstrCarriageRet = "true";
			break;
		case 5:	// ModNlpNormRetStemDiv;
			cstrStemming = "true";
			cstrCompoundDiv = "true";
			cstrCarriageRet = "true";
			break;
		case 6:	// ModNlpNormRetDiv;
			cstrStemming = "false";
			cstrCompoundDiv = "true";
			cstrCarriageRet = "true";
			break;
		case 7:	// ModNlpNormRetStem;
			cstrStemming = "true";
			cstrCompoundDiv = "false";
			cstrCarriageRet = "true";
			break;
		}
		params.insert("stem", cstrStemming);
		params.insert("compound", cstrCompoundDiv);
		params.insert("carriage", cstrCarriageRet);

		ModUnicodeString target;
		TOTAL_TIME = 0;

		while (get_target(fin, &target, io_code, do_line)) {
			// 入力中のU+FFF0, U+FFF1をそれぞれサロゲートペア前半, 後半の断片で置き換える
			target.replace(0xfff0, 0xd800);
			target.replace(0xfff1, 0xdc00);
			nlp->prepare(params);
			nlp->set(target, nlp_lang[currentLang]);
			currentLang = (currentLang+1)%nlp_lang_num;

			if (print_input == ModTrue) {
				fputs(target.getString(io_code), fout);
				fputc(0x0a, fout);
			}

			int num(0);
			while (1)
			{
				ModUnicodeString original;
				ModUnicodeString normalized;
				ModVector<ModUnicodeString> normVector;
				normVector.clear();
				ModVector<ModUnicodeString> orgVector;
				orgVector.clear();
				ModVector<int> posVector;
				posVector.clear();
				double npCost;

				t0 = ModTime::getCurrentTime();

				if(npCostMode == ModTrue)
				{
					if( get_vector == ModTrue )
					{
						rv = nlp->getConcept( normalized,
									original,
								    npCost,
								    normVector,
								    orgVector,
								    posVector);
					}
					else
					{
						rv = nlp->getConcept( normalized,
								original,
							    npCost);
					}
				}
				else
				{
					rv = nlp->getConcept( original,
							      normVector,
							      orgVector,
							      posVector);
				}
				t1 = ModTime::getCurrentTime();
				TOTAL_TIME = TOTAL_TIME + (t1-t0);
				
				if ( rv != ModTrue){
					break;
				}
				++ num;
				if ( get_vector == ModTrue )
				{
					ModSize i = 0;
					ModSize len = orgVector.getSize();
					for( i = 0; i < len; i++ )
					//オリジナル---中日: "私は男ッス" 以外: "I AM A MAN "
					{
						fprintf(fout,"%s",orgVector[i].getString(io_code));

						if( nlp_lang[currentLang] != ModLanguageSet("ja")
						 && nlp_lang[currentLang].round() != ModLanguageSet("zh") )
						{
							fprintf(fout," ");
						}
					}
					fprintf(fout," : ");
					if (normVector.getSize()==0)
					//normがないのでオリジナルで代用
					//(getNormMessageから呼ばれるMorph.cppのgetNormがこうなっている)
					{
						for( i = 0; i < orgVector.getSize(); i++ )
						//オリジナル---中日: "私/は/男/ッス/" 以外: "I/AM/A/MAN/"
						{
							fprintf(fout,"%s/",orgVector[i].getString(io_code));
						}
					}
					else
					{
						for( i = 0; i < normVector.getSize(); i++ )
						//正規化---中日: "私/は/男/ツス/" 以外: "i/am/a/man/"
						{
							fprintf(fout,"%s/",normVector[i].getString(io_code));
						}
					}
					// 名詞句コストの出力
					if(npCostMode == ModTrue)
					{
						fprintf(fout,", %f",npCost);
					}
					fprintf(fout,"\n");
				}
				else
				{
					if (sep != '\n' || num > 1) {
						if(npCostMode == ModFalse)
						{
							fputc(sep, fout);
						}
					}
					if (print_detail == ModTrue) {
						fprintf(fout, "%d", num);
						fputc(0x20, fout);
					}

					fputs(original.getString(io_code), fout);

					// 名詞句コストの出力
					if(npCostMode == ModTrue)
					{
						fprintf(fout,":");
						fputs(normalized.getString(io_code), fout);
						fprintf(fout,", %f",npCost);
					}
					fprintf(fout,"\n");

					if(npCostMode == ModFalse)
					{
						int iVecSize = orgVector.getSize();//popFrontするので最初に取得
						for( int i = 0; i < iVecSize; i++ )
						{
							ModUnicodeString norm;
							ModUnicodeString org;
							int pos;

							norm = normVector.getFront();
							normVector.popFront();
							org = orgVector.getFront();
							orgVector.popFront();
							pos = posVector.getFront();
							posVector.popFront();
							fprintf(fout,"(%d) ", i+1);
							fputs(norm.getString(io_code), fout);
							fputc(':', fout);
							fputs(org.getString(io_code), fout);
							fprintf(fout,"  %d\n", pos);
						}
					}
				}
			}
			if (get_vector != ModTrue || num != 0)
			{
			 	fputc(sep, fout);
				fputc(0x0a, fout);

				if (do_line == ModFalse && sep != '\n')
				{
					fputc(0x0a, fout);
				}
			}
			target.clear();
		}

		nlp->releaseResource();
		delete nlp,nlp=0;
		rr->unload();
		delete rr,rr=0;
		if (fin) fclose(fin);
		if (fout) fclose(fout);

	} catch (ModException& e) {
		ModErrorMessage << "ModException!: " << e << ModEndl;
    		cout << "ModException!:  "
	         << e.getErrorModule() << " "
       		 << e.getErrorNumber() << " "
	         << e.getErrorLevel() << "."
		 << endl;
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

