// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// 文字列展開機能テストプログラム(NlpAnalyzer)
// 　最大展開文字列長、最大展開文字列パターン数の指定パラメータのテスト
// 
// Copyright (c) 2010, 2022, 2023 Ricoh Company, Ltd.
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

using namespace std;

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
			{
				ModUnicodeChar* r = target->rsearch('\n');
				if(r!=0)
				{
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
		 << "[-m MODE] "
		 << "[-p] "
		 << "[-t] "
		 << "[-e ExpStrMode] "
		 << "[-s MaxExpTargetStrLen] "
		 << "[-u MaxExpPatternNum] "
		 << "[-SS] "
		 << "[-SD] "
		 << "[-MR] "
		 << "[-MRL LEVEL] "
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
	 	 << "\t-m MODE : 下位構造展開(MODE=1,2)/ステミング実施(MODE=1,3)(デフォルトは1)" << endl
		 << "\t-p       : 原文を表示" << endl
		 << "\t-t       : 関数実行時間を測定" << endl
		 << "\t-e NUM  : 文字列展開モードの指定(NUM=0,1,2)" << endl
		 << "\t-s NUM  : 最大展開文字長の指定" << endl
		 << "\t-u NUM  : 最大展開文字パターン数の指定" << endl
		 << "\t-SS      : 空白文字の削除処理を行わない" << endl
		 << "\t-SD      : 空白文字の削除処理を行う" << endl
		 << "\t-MR      : 省メモリ動作 MRL=1相当" << endl
		 << "\t-MRL LEVEL : 省メモリ動作のレベル指定 0-2" << endl
		 << "\t-X       : エラーテスト(ModException以外でFAILEDを出力)" << endl;
	return;
}

int main(int ac, char** av)
{
	ModOs::Process::setEncodingType(ModKanjiCode::literalCode);
	ModMemoryPool::setTotalLimit(256*1024);
	ModTime t0;
	ModTime t1;

	FILE* fin;
	FILE* fout;

	ModUnicodeString nlp_path(NLP_PATH);
	ModLanguageSet nlp_lang[MAX_LANGUAGE_NUM];
	ModSize	nlp_lang_num=1;

	int normMode(0);
	ModUnicodeChar sep(SEPARATOR);
	ModUnicodeChar exp_sep(EXP_SEP);

	ModBoolean do_line(ModFalse);
	ModBoolean print_input(ModFalse);
	ModBoolean get_time(ModFalse);
	ModBoolean mr(ModFalse);
	ModSize memoryReducingLevel=0;

	ModBoolean test_error(ModFalse);
	ModSize expStrMode = 0;
	ModSize maxExpTargetStrLen = 40;
	ModSize maxExpPatternNum = 40;

	char czSpace[10];
	strcpy(czSpace,"0"); //0:ModNlpAsIs
	ModKanjiCode::KanjiCodeType io_code = ModKanjiCode::utf8;

	ModBoolean emulateOldDmja=ModFalse;
	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> > params;


	for (int i = 1; i < ac; i++) {
		if (strncmp(av[i], "-h", 2) == 0) {
			print_help();
			return 0;
		}
		if (strncmp(av[i], "-r", 2) == 0) { // データディレクトリ
			nlp_path = av[++i];
			nlp_path += "/";
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
		} else if (strncmp(av[i], "-m", 2) == 0) {
			normMode = atoi(av[++i]);
		} else if (strncmp(av[i], "-l", 2) == 0) {
			do_line = ModTrue;
		} else if (strncmp(av[i], "-SS", 3) == 0) {
			strcpy(czSpace,"3");
		} else if (strncmp(av[i], "-SD", 3) == 0) {
			strcpy(czSpace,"2");
		} else if (strncmp(av[i], "-MRL", 4) == 0) {
			memoryReducingLevel = atoi(av[++i]);
		} else if (strncmp(av[i], "-MR", 3) == 0) {
			mr = ModTrue;
			memoryReducingLevel =1;
		} else if (strncmp(av[i], "-p", 2) == 0) {
			print_input = ModTrue;
		} else if (strncmp(av[i], "-t", 2) == 0) {
			get_time = ModTrue;
		} else if (strncmp(av[i], "-e", 2) == 0) { // getExpandStrings
			expStrMode = atoi(av[++i]);
		} else if (strncmp(av[i], "-s", 2) == 0) { // 最大展開文字列長
			maxExpTargetStrLen = atoi(av[++i]);
		} else if (strncmp(av[i], "-u", 2) == 0) { // 最大展開文字列パターン数
			maxExpPatternNum = atoi(av[++i]);
		} else if (strncmp(av[i], "-X", 2) == 0) {
			test_error = ModTrue;
		} else if (strncmp(av[i], "-b", 2) == 0) { 
			emulateOldDmja = ModTrue;
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

	ModBoolean bMem;
	ModUnicodeString cstrNormMode;
	ModUnicodeString cstrStemming;
	ModUnicodeString cstrCompoundDiv;
	ModUnicodeString cstrCarriageRet;
	ModUnicodeString cstrTag;
	ModUnicodeString cstrSpace;
	ModUnicodeString cstrExpStrMode;
	ModUnicodeString cstrMaxExpTargetStrLen;
	ModUnicodeString cstrMaxExpPatternNum;

	UNA::ModNlpResource* rr = 0;
	UNA::ModNlpAnalyzer* nlp = 0;

	try {
		ModMemoryPool::initialize(ModSizeMax >> 10);
		ModSize currentLang = 0;


		bMem = memoryReducingLevel == 1 ? ModTrue : ModFalse;
		rr = new UNA::ModNlpResource();
		rr->load(nlp_path, bMem);

		// set flag for specifying ModNLPAnalyzer to use T-lib only
		nlp = new UNA::ModNlpAnalyzer();
		nlp->setResource(rr);

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

		char czExpStrMode[10];
		sprintf(czExpStrMode,"%d", expStrMode);
		cstrExpStrMode = czExpStrMode;
		params.insert("expstrmode", cstrExpStrMode);

		char czMaxExpTargetStrLen[10];
		sprintf(czMaxExpTargetStrLen,"%d", maxExpTargetStrLen);
		cstrMaxExpTargetStrLen = czMaxExpTargetStrLen;
		params.insert("maxexptargetstrlen", cstrMaxExpTargetStrLen);
		char czMaxExpPatternNum[10];
		sprintf(czMaxExpPatternNum,"%d", maxExpPatternNum);
		cstrMaxExpPatternNum = czMaxExpPatternNum;
		params.insert("maxexppatternnum", cstrMaxExpPatternNum);

		cstrSpace = czSpace;
		params.insert("space", cstrSpace);

		ModUnicodeString target;
		TOTAL_TIME = 0;

		while (get_target(fin, &target, io_code, do_line)) {
			// 入力中のU+FFF0, U+FFF1をそれぞれサロゲートペア前半, 後半の断片で置き換える
			target.replace(0xfff0, 0xd800);
			target.replace(0xfff1, 0xdc00);
			try{
				nlp->prepare(params);
				nlp->set(target, nlp_lang[currentLang]);
			} catch (ModException& e) {
				ModErrorMessage << "ModException!: " << e << ModEndl;
				ModErrorHandle::reset();
				currentLang = (currentLang+1)%nlp_lang_num;
				continue;
			} catch (...) { 
				ModErrorMessage << "Unexpected exception!" << ModEndl;
				ModErrorHandle::reset();
				currentLang = (currentLang+1)%nlp_lang_num;
				continue;
			}
			currentLang = (currentLang+1)%nlp_lang_num;

			UNA::ModNlpAnalyzer::ExpandResult  expandResult = UNA::ModNlpAnalyzer::ExpandResultNone;
			ModVector<ModUnicodeString> expanded;

			try{
				t0 = ModTime::getCurrentTime();

				if(nlp->isExpStrDataLoad()){
					nlp->getExpandStrings(expandResult, expanded);
				} else{
					break;
				}

				t1 = ModTime::getCurrentTime();
				TOTAL_TIME = TOTAL_TIME + (t1-t0);
			} catch (ModException& e) {
				ModErrorMessage << "ModException!: " << e << ModEndl;
				ModErrorHandle::reset();
				continue;
			} catch (...) {
				ModErrorMessage << "Unexpected exception!" << ModEndl;
				ModErrorHandle::reset();
					continue;
			}

			fprintf(fout, "expandResult:%d\n", expandResult);
			for(ModSize i = 0; i < expanded.getSize(); ++i){
				fprintf(fout, "%s\n", expanded[i].getString(io_code));
			}
			fputc(sep, fout);
			fputc(0x0a, fout);
			if (sep != '\n') {
				fputc(0x0a, fout);
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
		if(nlp!=0)
		{
			nlp->releaseResource();
			delete nlp,nlp=0;
		}
		if(rr!=0)
		{
			rr->unload();
			delete rr,rr=0;
		}
		ModErrorHandle::reset();
		exit(1);
	} catch (...) {
		if(nlp!=0)
		{
			nlp->releaseResource();
			delete nlp,nlp=0;
		}
		if(rr!=0)
		{
			rr->unload();
			delete rr,rr=0;
		}
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
