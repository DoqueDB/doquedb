// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// 形態素解析＋かかりうけ機能テストプログラム(NlpAnalyzer)
// 
// Copyright (c) 2003, 2023, 2023 Ricoh Company, Ltd.
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
#include "ModVector.h"
#include "ModTime.h"
#include "ModLanguageSet.h"

using namespace std;

const ModSize MAX_DAT_LEN = 655360;
const ModSize MAX_LANGUAGE_NUM = 9;

const ModUnicodeString NLP_PATH("../unadic/");

const ModUnicodeChar SEPARATOR1 = 0x23; // #
const ModUnicodeChar SEPARATOR2 = 0x0a;	// \n
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
cerr	<< "Usage: "
	<< "[-help] "
	<< "[-r DIR] "
	<< "[-in FILE] "
	<< "[-out FILE] "
	<< "[-m MODE] "
	<< "[-x] "
	<< "[-P KeyString ValueString] "
	<< "[-sep CHAR] "
	<< "[-SEP HEX] "
	<< "[-T TSTR]"
	<< "[-line] "
	<< "[-t] "
	<< "[-u] "
	<< "[-L Language] "
	<< "[-print] " << endl
	<< "\t-h      : このメッセージを表示" << endl
	<< "\t-r DIR  : 辞書ディレクトリ(デフォルトは../unadic)" << endl
	<< "\t-i FILE : 入力ファイル(デフォルトは標準入力)" << endl
	<< "\t-o FILE : 出力ファイル(デフォルトは標準出力)" << endl
	<< "\t-c CODE : 入出力文字コード(デフォルトはutf8)" << endl
	<< "\t           euc" << endl
	<< "\t           utf8" << endl
	<< "\t           shiftJis" << endl
	<< "\t-C CODE : 出力文字コード(デフォルトはutf8)" << endl
	<< "\t           euc" << endl
	<< "\t           utf8" << endl
	<< "\t           shiftJis" << endl
	<< "\t-m MODE : 下位構造展開(MODE=1,2)/ステミング実施(MODE=1,3)(デフォルトは1)" << endl
	<< "\t-x      : 異表記正規化OFFにする(デフォルトはON) " << endl
	<< "\t-P KEY VAL: パラメータ(キー文字 値文字)をセット" << endl
	<< "\t-v      : ベクター型結果取得 " << endl
	<< "\t-O      : ベクター型結果取得&元表記取得 " << endl
	<< "\t-t      : 処理時間計測 " << endl
	<< "\t-u      : 統合品詞取得 " << endl
	<< "\t-H      : 形態素品詞名取得 " << endl
	<< "\t-T TSTR : タグ文字列指定 " << endl
	<< "\t-s CHAR : 解析結果出力時の区切り文字(デフォルトは'#')" << endl
	<< "\t-S HEX  : 解析結果出力時の区切り文字(デフォルトは0x23)" << endl
	<< "\t-l      : １行ずつ処理(デフォルトは空行で区切る)" << endl
	<< "\t-p      : 原文を表示" << endl
	<< "\t-L Lang : set language" << endl
	<< "\t-M      : mkExpStrData" << endl
	<< "\t-B      : 辞書ベース名取得 (-M以外のとき) " << endl;
	return;
}

int main(int ac, char** av)
{
	ModOs::Process::setEncodingType(ModKanjiCode::literalCode);
	ModMemoryPool::setTotalLimit(256*1024);

	FILE* fin = stdin;
	FILE* fout = stdout;

	ModUnicodeString nlp_path(NLP_PATH);

	int normMode(1);
	ModUnicodeChar sep(SEPARATOR1);
	ModUnicodeString tagStr("");

	ModTime t0;
	ModTime t1;
	ModBoolean do_line(ModFalse);
	ModBoolean doNormalizeMode(ModTrue);
	ModBoolean print_input(ModFalse);
	ModBoolean get_vector(ModFalse);
	ModBoolean get_time(ModFalse);
	ModBoolean get_upos(ModFalse);
	ModBoolean get_hin(ModFalse);
	ModBoolean get_original(ModFalse);
	ModBoolean get_dicname(ModFalse);
	ModBoolean ignore(ModTrue);
	ModBoolean mkExpStrData(ModFalse);
	ModKanjiCode::KanjiCodeType in_code = ModKanjiCode::utf8;
	ModKanjiCode::KanjiCodeType out_code = ModKanjiCode::utf8;

	ModSize maxWordLength=0;
	ModBoolean emulateOldDmja=ModFalse;
	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> > params;

	ModLanguageSet nlp_lang[MAX_LANGUAGE_NUM];
	nlp_lang[0] = ModUnicodeString("ja+en");
	ModSize	nlp_lang_num=1;

	for (int i = 1; i < ac; i++) {
		if (strncmp(av[i], "-h", 2) == 0) {
			print_help();
			return 0;
		}
		if (strncmp(av[i], "-i", 2) == 0) { // 入力ファイル
			fin = fopen(av[++i], "rb");
		} else if (strncmp(av[i], "-T", 2) == 0) { // タグ文字列
			tagStr = av[++i];
		} else if (strncmp(av[i], "-o", 2) == 0) { // 出力ファイル
			fout = fopen(av[++i], "wb");

		} else if (strncmp(av[i], "-r", 2) == 0) { // データディレクトリ
			nlp_path = av[++i];
			nlp_path += "/";

		} else if (strncmp(av[i], "-c", 2) == 0) { // 入出力文字コード
			if (strcmp(av[++i], "euc") == 0) {
				in_code = ModKanjiCode::euc;
				out_code = ModKanjiCode::euc;
			} else if (strcmp(av[i], "shiftJis") == 0) {
				in_code = ModKanjiCode::shiftJis;
				out_code = ModKanjiCode::shiftJis;
			} else if (strcmp(av[i], "utf8") == 0) { // デフォルト
				in_code = ModKanjiCode::utf8;
				out_code = ModKanjiCode::utf8;
			} else {
				cerr << "ERROR: unexpected char code" << endl;
				exit(1);
			}

		} else if (strncmp(av[i], "-C", 2) == 0) { // 出力文字コード
			if (strcmp(av[++i], "euc") == 0) {
				out_code = ModKanjiCode::euc;
			} else if (strcmp(av[i], "shiftJis") == 0) {
				out_code = ModKanjiCode::shiftJis;
			} else if (strcmp(av[i], "utf8") == 0) { // デフォルト
				out_code = ModKanjiCode::utf8;
			} else {
				cerr << "ERROR: unexpected char code" << endl;
				exit(1);
			}

		} else if (strncmp(av[i], "-s", 2) == 0) {
			sep = ModUnicodeChar(av[++i][0]);

		} else if (strncmp(av[i], "-S", 2) == 0) {
			sep = ModUnicodeChar(strtol(av[++i], 0, 16));

		} else if (strncmp(av[i], "-m", 2) == 0) {
			normMode = atoi(av[++i]);

		} else if (strncmp(av[i], "-x", 2) == 0) {
			doNormalizeMode = ModFalse;

		} else if (strncmp(av[i], "-l", 2) == 0) {
			do_line = ModTrue;

		} else if (strncmp(av[i], "-p", 2) == 0) {
			print_input = ModTrue;

		} else if (strncmp(av[i], "-P", 2) == 0) { // string param
			params.insert(av[i+1], av[i+2]);
			i+=2;
		} else if (strncmp(av[i], "-v", 2) == 0) {
			get_vector = ModTrue;

		} else if (strncmp(av[i], "-t", 2) == 0) {
			get_time = ModTrue;

		} else if (strncmp(av[i], "-u", 2) == 0) { // 統合品詞
			get_upos = ModTrue;

		} else if (strncmp(av[i], "-H", 2) == 0) { // 形態素品詞名取得
			get_hin = ModTrue;

		} else if (strncmp(av[i], "-g", 2) == 0) { // 正規化で長さ0は無視
			ignore = ModFalse;

		} else if (strncmp(av[i], "-O", 2) == 0) {
			get_original = ModTrue;

		} else if (strncmp(av[i], "-b", 2) == 0) { 
			emulateOldDmja = ModTrue;

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
		} else if (strncmp(av[i], "-M", 2) == 0) { 
			mkExpStrData = ModTrue;

		} else if (strncmp(av[i], "-B", 2) == 0) {	// 辞書ベース名取得
			get_dicname = ModTrue;

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

	ModUnicodeString cstrLang;
	ModUnicodeString cstrMaxWordLength;
	ModUnicodeString cstrNormMode;
	ModUnicodeString cstrTag;
	ModUnicodeString cstrDoNorm;
	ModUnicodeString cstrStemming;
	ModUnicodeString cstrCompoundDiv;
	ModUnicodeString cstrCarriageRet;

	try {
		ModMemoryPool::initialize(ModSizeMax >> 10);

		ModSize currentLang = 0;
		UNA::ModNlpResource* rr = new UNA::ModNlpResource();
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
		cstrTag = tagStr;
		params.insert("tag",cstrTag);
		cstrDoNorm = doNormalizeMode == ModTrue ? "true" : "false";
		params.insert("donorm",cstrDoNorm);

		ModUnicodeString target;

		TOTAL_TIME = 0;
		while (get_target(fin, &target, in_code, do_line)) {
			// 入力中のU+FFF0, U+FFF1をそれぞれサロゲートペア前半, 後半の断片で置き換える
			target.replace(0xfff0, 0xd800);
			target.replace(0xfff1, 0xdc00);

			nlp->prepare(params);
			nlp->set(target, nlp_lang[currentLang]);
			if (print_input == ModTrue) {
				fputs(target.getString(out_code), fout);
				fputc(0x0a, fout);
			}

			// かかりうけ出力
			while ( 1){
				if (mkExpStrData){
					// -Mが指定されたとき (-v, -O, -u, -H, -Bは無視)
					ModVector<ModUnicodeString> normVector;
					ModVector<int> posVector;
					t0 = ModTime::getCurrentTime();
					if ( nlp->getBlock(normVector,posVector)==ModFalse)
						break;
					t1 = ModTime::getCurrentTime();
					ModVector<ModUnicodeString>::Iterator form = normVector.begin();
					for ( ;form<normVector.end();++form)
					{
						fprintf(fout,"##");
						fputs(form->getString(out_code),fout);
					}
					fprintf(fout,"##");
				}
				// 形態素品詞名取得
				else if (get_hin == ModTrue)
				{
					// -Mが指定されず、-Hが指定されたとき (-v, -O, -uは無視)
				  	ModVector<ModUnicodeString> normVector;	// 形態素語形ベクトル
					ModVector<ModUnicodeString>	hinVector;  // 形態素品詞名ベクトル
					ModVector<ModUnicodeString>	dicNameVector;  // 辞書ベース名ベクター

					t0 = ModTime::getCurrentTime();	

					try{
						if ( nlp->getBlock(normVector, hinVector)==ModFalse) {
							break;
						}
						if (get_dicname == ModTrue) {
							nlp->getDicName(dicNameVector);
						}
					} catch (ModException& e) {
						ModErrorMessage << "ModException!: " << e << ModEndl;
						exit(1);
					}
					catch (...)
					{
						ModErrorMessage << "Unexpected exception!" << ModEndl;
						cout << "FAILED2" << endl;
						exit(1);
					}

					t1 = ModTime::getCurrentTime();
					
					ModVector<ModUnicodeString>::Iterator norm = normVector.begin();
					ModVector<ModUnicodeString>::Iterator hin = hinVector.begin();
					ModVector<ModUnicodeString>::Iterator name;
					if (get_dicname == ModTrue) {
						name = dicNameVector.begin();
					}

					for ( ;norm<normVector.end();++norm,++hin)
					{
						fputs(norm->getString(out_code),fout);
						fprintf(fout,"#");
						fputs(hin->getString(out_code),fout);
						if (get_dicname == ModTrue) {
							fputc(sep, fout);
							fputs(name->getString(out_code),fout);
							++name;
						}
						fprintf(fout,"\n");
					}
				}
				else if ( get_vector == ModFalse && get_original == ModFalse && get_upos == ModFalse)
				{
					// -M, -H, -v, -O, -uがいずれも指定されなかったとき
					ModUnicodeString norm;
					t0 = ModTime::getCurrentTime();
					if ( nlp->getBlock(norm,sep,SEPARATOR2)==ModFalse)
						break;
					t1 = ModTime::getCurrentTime();
					if (get_dicname == ModTrue) {
						ModVector<ModUnicodeString>	dicNameVector;
						nlp->getDicName(dicNameVector);
						ModVector<ModUnicodeString>::Iterator name = dicNameVector.begin();
						char *p = (char *)(norm.getString(out_code));
						while (*p != '\0') {
							// normの改行までとdicNameVectorの要素を並べて出力する
							while (*p != '\0' && *p != '\r' && *p != '\n') {
								fputc(*p, fout);
								p++;
							}
							fputc(sep, fout);
							fputs(name->getString(out_code),fout);
							while (*p != '\0' && (*p == '\r' || *p == '\n')) {
								fputc(*p, fout);
								p++;
							}
							name++;
						}
					} else {
						fputs(norm.getString(out_code), fout);
					}
				}
				else if (get_original == ModFalse && get_upos == ModFalse)
				{
					// -M, -H, -O, -uが指定されず、-vが指定されたとき
					ModVector<ModUnicodeString> normVector;
					ModVector<int> posVector;
					ModVector<ModUnicodeString>	dicNameVector;
					t0 = ModTime::getCurrentTime();
					if ( nlp->getBlock(normVector,posVector)==ModFalse)
						break;
					t1 = ModTime::getCurrentTime();
					if (get_dicname == ModTrue) {
						nlp->getDicName(dicNameVector);
					}
					ModVector<ModUnicodeString>::Iterator form = normVector.begin();
					ModVector<int>::Iterator pos = posVector.begin();
					ModVector<ModUnicodeString>::Iterator name;
					if (get_dicname == ModTrue) {
						name = dicNameVector.begin();
					}
					for ( ;pos<posVector.end();++pos,++form)
					{
						fputs(form->getString(out_code),fout);
						fprintf(fout,"(%d)",(*pos));
						if (get_dicname == ModTrue) {
							fputc(sep, fout);
							fputs(name->getString(out_code),fout);
							++name;
						}
						fprintf(fout,"\n");
					}
				}
				// 統合品詞
				else if (get_upos == ModTrue)
				{
					// -M, -Hが指定されず、-uが指定されたとき (-O, -vは無視)
				  	ModVector<ModUnicodeString> normVector;	// 形態素語形ベクトル
					ModVector<ModUnicodeString> origVector;	// 形態素語形ベクトル(元表記)
					ModVector<int>				posVector;  // 形態素品詞ベクトル
					ModVector<int>				costVector;	// 単語コストベクトル
					ModVector<int>				uposVector;	// UNA 統合品詞
					ModVector<ModUnicodeString> dicNameVector; // 辞書ベース名ベクター
					
					t0 = ModTime::getCurrentTime();	

					try{
						if ( nlp->getBlock(normVector, origVector, posVector, costVector, uposVector, ignore)==ModFalse) {
							break;
						}
						if (get_dicname == ModTrue) {
							nlp->getDicName(dicNameVector);
						}
					} catch (ModException& e) {
						ModErrorMessage << "ModException!: " << e << ModEndl;
						exit(1);
					}
					catch (...)
					{
						ModErrorMessage << "Unexpected exception!" << ModEndl;
						cout << "FAILED2" << endl;
						exit(1);
					}

					t1 = ModTime::getCurrentTime();
					
					ModVector<ModUnicodeString>::Iterator norm = normVector.begin();
					ModVector<ModUnicodeString>::Iterator ostr = origVector.begin();
					ModVector<int>::Iterator pos  = posVector.begin();
					ModVector<int>::Iterator cost  = costVector.begin();
					ModVector<int>::Iterator upos = uposVector.begin();
					ModVector<ModUnicodeString>::Iterator name;
					if (get_dicname == ModTrue) {
						name = dicNameVector.begin();
					}

					for ( ;pos<posVector.end();++pos,++norm,++ostr,++cost,++upos)
					{
						fputs(ostr->getString(out_code),fout);
						fprintf(fout,"#");
						fprintf(fout,"(%d)",(*pos));
						fprintf(fout,"#");
						fprintf(fout,"(%08x)",(*upos));
						fprintf(fout,"#");
						fprintf(fout,"(%08x)",(*cost));
						if (get_dicname == ModTrue) {
							fputc(sep, fout);
							fputs(name->getString(out_code),fout);
							++name;
						}
						fprintf(fout,"\n");
					}
				}
				// -M, -H, -uが指定されず、-Oが指定されたとき (-vは無視)
				else
				{
					ModVector<ModUnicodeString> normVector;
					ModVector<ModUnicodeString> origVector;
					ModVector<int> posVector;
					ModVector<ModUnicodeString> dicNameVector; // 辞書ベース名ベクター
					t0 = ModTime::getCurrentTime();
					if ( nlp->getBlock(normVector,origVector,posVector)==ModFalse)
						break;
					t1 = ModTime::getCurrentTime();
					if (get_dicname == ModTrue) {
						nlp->getDicName(dicNameVector);
					}
					ModVector<ModUnicodeString>::Iterator form = normVector.begin();
					ModVector<ModUnicodeString>::Iterator ostr = origVector.begin();
					ModVector<int>::Iterator pos = posVector.begin();
					ModVector<ModUnicodeString>::Iterator name;
					if (get_dicname == ModTrue) {
						name = dicNameVector.begin();
					}
					for ( ;pos<posVector.end();++pos,++form,++ostr)
					{
						fputs(form->getString(out_code),fout);
						fprintf(fout,":");
						fputs(ostr->getString(out_code),fout);
						fprintf(fout,":");
						fprintf(fout,"(%d)",(*pos));
						if (get_dicname == ModTrue) {
							fputc(sep, fout);
							fputs(name->getString(out_code),fout);
							++name;
						}
						fprintf(fout,"\n");
					}
				}

				TOTAL_TIME = TOTAL_TIME + (t1 - t0);
			}
			fputc(sep, fout);
			fputc(0x0a, fout);
			if (do_line == ModFalse) {
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
		exit(1);
	} catch (...) {
		ModErrorMessage << "Unexpected exception!" << ModEndl;
		exit(1);
	}

	if ( get_time == ModTrue){
		print_time();
	}

	return 0;
}

