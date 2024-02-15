// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// 形態素解析のスレッドテストプログラム
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
#include "ModThread.h"
#include "ModOstrStream.h"
#include "ModUnicodeString.h"
#include "ModNLP.h"
#include "ModLanguageSet.h"

using namespace std;
using namespace UNA;

const ModSize MAX_DAT_LEN = 655360;
const ModSize MAX_LANGUAGE_NUM = 9;

const ModSize MaxDataSize=65536;
const ModSize TestMaxThreadCount=10;


const ModUnicodeString NLP_PATH("../unadic/");

const ModUnicodeChar SEPARATOR = 0x0a; // \n
const ModUnicodeChar EXP_SEP   = 0x2c; // ,

struct threadArg{
	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >* params;
	ModUnicodeString reslang;
	ModUnicodeString resno;
	int resmode;
	ModNlpResource* rr;
	ModUnicodeString *target;
	FILE*           fout;
	ModLanguageSet  *nlp_lang;
	ModSize         nlp_lang_num;
	ModSize         id;
	ModSize         normMode;
	ModSize		maxWordLength;
	ModBoolean      doNormalizeMode;
	ModBoolean      get_original;
	ModBoolean	use_keyWordFunc;
	ModBoolean      expand;
	ModBoolean      np_extract;
	ModBoolean      npCostMode;
	ModBoolean      wholeText;
	ModBoolean	get_vector;
	ModBoolean      do_line;
	ModUnicodeChar  sep;
	ModUnicodeChar  exp_sep;
	/*ModNlpExpMode*/int expMode;
	ModBoolean      emulateBug;
	ModKanjiCode::KanjiCodeType io_code;
	ModBoolean      returnValue;
	ModBoolean      NormOnly;
	ModSize         LenOfNorm;
};

void print_help() {
	cerr << "Usage: "
		 << "[-h] "
		 << "[-r DIR] "
		 << "[-i FILE] "
		 << "[-o FILE] "
		 << "[-O] "
		 << "[-k] "
		 << "[-P KeyString ValueString] "
		 << "[-s1 CHAR] "
		 << "[-S1 HEX] "
		 << "[-s2 CHAR] "
		 << "[-S2 HEX] "
		 << "[-x] "
		 << "[-m  MODE] "
		 << "[-e] "
		 << "[-U  MODE] "
		 << "[-C] " << endl
		 << "\t-h       : このメッセージを表示" << endl
		 << "\t-r DIR   : 辞書ディレクトリ(デフォルトは../unadic)" << endl
		 << "\t-i FILE  : 入力ファイル(デフォルトは標準入力)" << endl
		 << "\t-o FILE  : 出力ファイル(デフォルトは標準出力)" << endl
		 << "\t-c CODE  : 入出力文字コード(デフォルトはutf8)" << endl
		 << "\t             euc" << endl
		 << "\t             utf8" << endl
		 << "\t             shiftJis" << endl
		 << "\t-O       : 原表記取得" << endl
		 << "\t-k       : 原表記取得&品詞取得(キーワード抽出用)" << endl
		 << "\t-P KEY VAL: パラメータ(キー文字 値文字)をセット" << endl
		 << "\t-s1 CHAR : 形態素区切り(デフォルトは改行)" << endl
		 << "\t-S1 HEX  : 形態素区切り(デフォルトは0x0a)" << endl
		 << "\t-s2 CHAR : 展開表記区切り(デフォルトはコンマ)" << endl
		 << "\t-S2 HEX  : 展開表記区切り(デフォルトは0x2c)" << endl
		 << "\t-x       : 正規化を行わない(デフォルトは実施)" << endl
		 << "\t-m  MODE : 下位構造展開(MODE=1,2)/ステミング実施(MODE=1,3)" << endl
		 << "\t-e(xpand): 異表記展開を実施(デフォルトは正規化のみ)" << endl
		 << "\t-U  MODE : mode of initializing resource (MODE = 0,1,2,3)" << endl
		 << "\t-C(heck) : 展開時に包含関係な候補を削除する" << endl
		 << "\t-w WDLEN : 最大単語長" << endl
		 << "\t-L LANG  : 言語指定 en/ja+en/es のように複数指定可能" << endl
		 << "\t-np      : NP抽出" << endl
		 << "\t-npc     : NP抽出(コスト算出)" << endl
		 << "\t-g       : 全テキスト取得" << endl
		 << "\t-v       : ベクター型結果取得 " << endl
		 << "\t-l       : １行ずつ処理(デフォルトは空行で区切る)" << endl
		 << "\t-b       : バグ動作のエミュレート" << endl
		 << "\t-N       : ModNormalizerのエミュレート" << endl
		 << "\t-n       : テキスト先頭からの対象とする範囲(-N 指定時有効)" << endl;
	return;
}


void* doNlpAnalyze( void* arg)
{

	struct threadArg *av = (struct threadArg*)arg;
	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> > params = *(av->params);

	ModNlpAnalyzer* nlp = new ModNlpAnalyzer();
	nlp->setResource(av->rr);

	char czMaxWordLength[10];
	sprintf(czMaxWordLength,"%d",av->maxWordLength);
	ModUnicodeString cstrMaxWordLength = ModUnicodeString(czMaxWordLength);
	params.insert("maxwordlen",cstrMaxWordLength);

	{
		ModUnicodeString cstrStemming;
		ModUnicodeString cstrCompoundDiv;
		ModUnicodeString cstrCarriageRet;
		switch (av->normMode)
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
	}
	char czExpMode[10];
	sprintf(czExpMode,"%d",av->expMode);
	ModUnicodeString cstrExpMode = czExpMode;
	params.insert("expmode",cstrExpMode);
	ModUnicodeString cstrDoNorm = av->doNormalizeMode == ModTrue ? "true" : "false";
	params.insert("donorm",cstrDoNorm);
	char czLenOfNorm[10];
	sprintf(czLenOfNorm,"%d",av->LenOfNorm);
	ModUnicodeString cstrLenOfNorm = czLenOfNorm;
	params.insert("lengthofnormalization",cstrLenOfNorm);

	char lineStr[256];
	
	// 言語の指定
	ModLanguageSet nlp_lang;
	nlp_lang = av->nlp_lang[(av->id)%(av->nlp_lang_num)];
	
	ModUnicodeString targetCopy;

	try{
		nlp->prepare(params);
		nlp->set(*(av->target),nlp_lang);
	} catch (ModException& e) {
		ModErrorMessage << "ModException!: " << e << ModEndl;
		ModErrorHandle::reset();
		av->returnValue = ModFalse;
		nlp->releaseResource();
		delete nlp, nlp = 0;
		return (void*)&(av->returnValue);
	} catch (...) {
		ModErrorMessage << "Unexpected exception!" << ModEndl;
		ModErrorHandle::reset();
		av->returnValue = ModFalse;
		nlp->releaseResource();
		delete nlp, nlp = 0;
		return (void*)&(av->returnValue);
	}

	targetCopy.clear();

	if (av->NormOnly == ModTrue)
	{		
		if (av->expand == ModTrue)
		{
			while (1)
			{	
				ModBoolean rv;
				ModVector<ModUnicodeString> expanded;

				rv = nlp->getExpandBuf( expanded );
				if ( rv != ModTrue){
					break;
				}
				if ( av->id == 5)
				{	
					for(ModSize ee = 0;ee < expanded.getSize();++ee)
						fprintf(av->fout,"%s\n",expanded[ee].getString(av->io_code));
				}
			}
		}
		else
		{
			while (1)
			{	
				ModBoolean rv;
				ModUnicodeString wholeText;

				rv = nlp->getNormalizeBuf( wholeText );
				if ( rv != ModTrue){
					break;
				}
				if ( av->id == 5)
				{	
					fprintf(av->fout,"%s",wholeText.getString(av->io_code));
				}
			}
		}
	}
	else if (av->expand == ModTrue)
	//展開
	{
		ModVector<ModUnicodeString> expanded;
		ModUnicodeString original;
		int pos;
		ModBoolean rv;
		int num(0);
		while (1){
			try{
				rv = nlp->getExpandWords(expanded,original,pos);
			} catch (ModException& e) {
				ModErrorMessage << "ModException!: " << e << ModEndl;
				ModErrorHandle::reset();
				av->returnValue = ModFalse;
				nlp->releaseResource();
				delete nlp, nlp = 0;
				return (void*)&(av->returnValue);
			} catch (...) { 
				ModErrorMessage << "Unexpected exception!" << ModEndl;
				ModErrorHandle::reset();
				av->returnValue = ModFalse;
				nlp->releaseResource();
				delete nlp, nlp = 0;
				return (void*)&(av->returnValue);
			}
			if (rv != ModTrue){
				break;
			}
			++ num;
			if ( av->id == 5)
			{	
				if (av->sep != '\n' || num > 1) {
					fputc(av->sep, av->fout);
				}
				sprintf(lineStr,"%s",expanded[0].getString(av->io_code));
 				fputs(lineStr, av->fout);
				for (ModSize xx = 1; xx < expanded.getSize(); ++xx){
					sprintf(lineStr,"%c%s",
						(av->exp_sep), expanded[xx].getString(av->io_code));
 					fputs(lineStr, av->fout);
				}
			}
		}
	}
	else if (av->np_extract == ModTrue)
	{		
		int num(0);
		while (1)
		{	
			ModBoolean rv;
			ModUnicodeString original;
			ModUnicodeString normalized; 
			double npCost;
			ModVector<ModUnicodeString> normVector;
			ModVector<ModUnicodeString> orgVector;
			ModVector<int> posVector;

			if(av->npCostMode == ModTrue)
			{
				if(av->get_vector == ModTrue)
				{
					rv = nlp->getConcept(normalized,
								original,
								npCost,
								normVector,
								orgVector,
								posVector);
				}
				else
				{
					rv = nlp->getConcept(normalized,
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
			
			if ( rv != ModTrue){
				break;
			}
			++ num;
			if ( av->id == 5)
			{	
				if ( av->get_vector == ModTrue )
				{
					if(av->npCostMode == ModTrue && orgVector.getSize() == 0 && original.getLength() != 0){
						fputs(original.getString(av->io_code), av->fout);
					}
					else{
						for( ModSize i = 0; i < orgVector.getSize(); i++ )
						//オリジナル---中日: "私/は/男/ッス/" 以外: "I/AM/A/MAN/"
						{
							fprintf(av->fout,"%s/",orgVector[i].getString(av->io_code));
						}
					}

					if(av->npCostMode == ModTrue)
					{
						fprintf(av->fout,", %f",npCost);
					}
					fprintf(av->fout,"\n");
				}
				else
				{
					if ((av->sep != '\n' || num > 1) && av->npCostMode == ModFalse) {
						fputc(av->sep, av->fout);
					}

					fputs(original.getString(av->io_code), av->fout);

					// 名詞句コストの出力
					if(av->npCostMode == ModTrue)
					{
						fprintf(av->fout,", %f",npCost);
					}
					fprintf(av->fout,"\n");

					if(av->npCostMode == ModFalse)
					{
						ModSize iVecSize = orgVector.getSize();//popFrontするので最初に取得
						for( ModSize i = 0; i < iVecSize; i++ )
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

							fprintf(av->fout,"(%d) ", i+1);
							fputs(norm.getString(av->io_code), av->fout);
							fputc(':', av->fout);
							fputs(org.getString(av->io_code), av->fout);
							fprintf(av->fout,"  %d\n", pos);
						}
					}
				}
			}
		}
	}
	else if (av->wholeText == ModTrue)
	{		
		while (1)
		{	
			ModBoolean rv;
			ModUnicodeString wholeText;
			
			rv = nlp->getWholeText( wholeText );
			if ( rv != ModTrue){
				break;
			}
			if ( av->id == 5)
			{	
				fprintf(av->fout,"%s",wholeText.getString(av->io_code));
			}
		}
	}
	else
	//正規化(getWord)
	{
		ModUnicodeString word;
		int num(0);
		while (1){
			ModBoolean rv;
			ModUnicodeString ostring;
			int pos;
			try{
				if ( av->use_keyWordFunc == ModTrue ) {
					rv = nlp->getWord(word, ostring, pos);
				} else if ( av->get_original == ModTrue ) {
					rv = nlp->getWord(word, ostring);
				} else {
					rv = nlp->getWord(word);
				}
			} catch (ModException& e) {
				ModErrorMessage << "ModException!: " << e << ModEndl;
				ModErrorHandle::reset();
				av->returnValue = ModFalse;
				nlp->releaseResource();
				delete nlp, nlp = 0;
				return (void*)&(av->returnValue);
			} catch (...) { 
				ModErrorMessage << "Unexpected exception!" << ModEndl;
				ModErrorHandle::reset();
				av->returnValue = ModFalse;
				nlp->releaseResource();
				delete nlp, nlp = 0;
				return (void*)&(av->returnValue);
			}

			if ( rv != ModTrue){
				break;
			}
			++ num;
			if ( av->id == 5)
			{	
				if (av->sep != '\n' || num > 1)
				{
					fputc(av->sep, av->fout);
				}
				if ( av->get_original == ModTrue )
				{
					sprintf(lineStr,"%s:%s",
						word.getString(av->io_code),
						ostring.getString(av->io_code));
				}
				else if ( av->use_keyWordFunc == ModTrue){
					sprintf(lineStr,"%s:%s\t%d",
						word.getString(av->io_code),
						ostring.getString(av->io_code),
						pos);				  
				}
				else
				{
					sprintf(lineStr,"%s",
						word.getString(av->io_code));
				}
 				fputs(lineStr, av->fout);
			}
		}
	}
	if ( av->id == 5)
	{	
		fputc(av->sep, av->fout);
		fputc(0x0a, av->fout);
		if (av->do_line == ModFalse && av->sep != '\n') {
		    fputc(0x0a, av->fout);
		}
	}

	nlp->releaseResource();
	delete nlp, nlp = 0;

	av->returnValue = ModTrue;
	return (void*)&(av->returnValue);
}

//
//   並行動作性のテスト
//
void
doAAA(int threadCount, void* args)
{
	ModThread thread[TestMaxThreadCount];	
	//ModThread* thread[threadCount];

	int i;
	//struct threadArg pArg[threadCount];
	struct threadArg pArg[TestMaxThreadCount];

	// target
	// 同じスレッドから、たくさんのスレッドを順にたくさん起こす。(create)
	// effect
	//      各関数がスレッドとしてそれぞれ並行に動作し、スレッド別に
	//      メッセージが出力される。
	//for (i = 0; i < threadCount; i++) {
	for (i = 0; i < TestMaxThreadCount; i++) {
		//thread[i] = new ModThread();
		pArg[i] = *(struct threadArg*)args;
		pArg[i].id = i;
		// 返り値を返す関数を起動
		//thread[i]->create(doNlpAnalyze, (void*)&pArg[i]);
		thread[i].create(doNlpAnalyze, (void*)&pArg[i]);
	}

	// target
	// スレッドの終了待ち(join)
	// effect
	// 起こした順番に終了を待つ。
	//void* vstatus;
	//unsigned int code;
	//ModException* exception;
	for (i = 0; i < threadCount; i++) {
		//const ModThread::ExitStatus& exitStatus = thread[i]->join();

		const ModThread::ExitStatus& exitStatus = thread[i].join();
		//delete thread[i];
	}
	// スレッド管理リストの内容を調べる
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

int main(int ac, char** av)
{
	ModOs::Process::setEncodingType(ModKanjiCode::literalCode);
	ModMemoryPool::setTotalLimit(256*1024);
	
	ModUnicodeString nlp_path(NLP_PATH);
	ModSize nlp_lang_num;
	ModLanguageSet nlp_lang[MAX_LANGUAGE_NUM];

	ModSize normMode(1);
	int resMode(0);
	ModBoolean do_line(ModFalse);
	ModBoolean doNormalizeMode(ModTrue);
	ModBoolean get_original(ModFalse);
	ModBoolean expand(ModFalse);
	ModBoolean np_extract(ModFalse);
	ModBoolean npCostMode(ModFalse);
	ModBoolean wholeText(ModFalse);
	ModBoolean get_vector(ModFalse);
	/*ModNlpExpMode*/ int expMode = 1;//1=ModNlpExpNoChk;
	ModKanjiCode::KanjiCodeType io_code = ModKanjiCode::utf8;
	ModUnicodeChar sep(SEPARATOR);
	ModUnicodeChar exp_sep(EXP_SEP);
	ModSize maxWordLength=0;
    ModBoolean emulateOldDmja = ModFalse;
	ModBoolean      NormOnly = ModFalse;
	ModSize         LenOfNorm = 0;

	FILE* fin = stdin;
	FILE* fout = stderr;
	nlp_lang[0] = ModUnicodeString("ja+en");
	nlp_lang_num = 1;

	ModBoolean use_keyWordFunc(ModFalse);
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
		} else if (strncmp(av[i], "-O", 2) == 0) { 
			get_original = ModTrue;
		} else if (strncmp(av[i], "-k", 2) == 0) { 
			use_keyWordFunc = ModTrue;
		} else if (strncmp(av[i], "-P", 2) == 0) { // string param
			params.insert(av[i+1], av[i+2]);
			i+=2;
		} else if (strncmp(av[i], "-s1", 3) == 0) { 
			sep = ModUnicodeChar(av[++i][0]);
		} else if (strncmp(av[i], "-S1", 3) == 0) { 
			sep = ModUnicodeChar(strtol(av[++i], 0, 16));
		} else if (strncmp(av[i], "-s2", 3) == 0) { 
			exp_sep = ModUnicodeChar(av[++i][0]);
		} else if (strncmp(av[i], "-S2", 3) == 0) { 
			exp_sep = ModUnicodeChar(strtol(av[++i], 0, 16));
		} else if (strncmp(av[i], "-x", 2) == 0) { 
			doNormalizeMode = ModFalse;
		} else if (strncmp(av[i], "-m", 2) == 0) { // モード指定
			normMode = atoi(av[++i]);
		} else if (strncmp(av[i], "-e", 2) == 0) { 
			expand = ModTrue;
		} else if (strncmp(av[i], "-U", 2) == 0) { // モード指定
			resMode = atoi(av[++i]);
		} else if (strncmp(av[i], "-C", 2) == 0) { 
			expMode = 0;//0=ModNlpExpChkOrigStr
		} else if (strncmp(av[i], "-w", 2) == 0) {
			maxWordLength = atoi(av[++i]);
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
		} else if (strncmp(av[i], "-npc", 4) == 0) { 
			npCostMode = ModTrue;
		} else if (strncmp(av[i], "-np", 3) == 0) { 
			np_extract = ModTrue;
		} else if (strncmp(av[i], "-g", 2) == 0) { 
			wholeText = ModTrue;
		} else if (strncmp(av[i], "-v", 2) == 0) { 
			get_vector = ModTrue;	
		} else if (strncmp(av[i], "-l", 2) == 0) { 
			do_line = ModTrue;
		} else if (strncmp(av[i], "-b", 2) == 0) { 
			emulateOldDmja = ModTrue;
		} else if (strncmp(av[i], "-N", 2) == 0) { 
			NormOnly = ModTrue;
		} else if (strncmp(av[i], "-n", 2) == 0) { 
			LenOfNorm = atoi(av[++i]);
		} else {
			cerr << "ERROR: unexpected parameter " << av[i] << endl;
			exit(1);
		}
	}	

	try {
		ModMemoryPool::initialize(ModSizeMax >> 10);
		
		ModSize currentLang = 0;
		ModLanguageSet lang;

		if ( nlp_lang[currentLang].isContained( ModLanguage::ja )
		   && resMode == 0 )
		{
			lang = ModLanguageSet();
		}
		else
		{
			lang = nlp_lang[currentLang];
		}

		ModNlpResource* rr = new ModNlpResource();
		rr->load(nlp_path,ModFalse);

		struct threadArg argv;

		ModUnicodeString target;

		while (get_target(fin, &target, io_code, do_line)) {
			// 入力中のU+FFF0, U+FFF1をそれぞれサロゲートペア前半, 後半の断片で置き換える
			target.replace(0xfff0, 0xd800);
			target.replace(0xfff1, 0xdc00);

			// アーギュメントのセット
			argv.params = &params;
			argv.reslang = lang.getName();
			argv.resno = "4";
			argv.resmode = resMode;
			argv.rr = rr;
			argv.target = &target;
			argv.fout = fout;
			argv.normMode = normMode;
			argv.maxWordLength = maxWordLength;
			argv.doNormalizeMode = doNormalizeMode;
			argv.get_original = get_original;
			argv.use_keyWordFunc = use_keyWordFunc;
			argv.expand = expand;
			argv.np_extract = np_extract;
			argv.npCostMode = npCostMode;
			argv.wholeText = wholeText;
			argv.get_vector = get_vector;
			argv.expMode = expMode;
			argv.io_code = io_code;
			argv.do_line = do_line;
			argv.sep = sep;
			argv.exp_sep = exp_sep;
			argv.nlp_lang = nlp_lang;
			argv.emulateBug=emulateOldDmja;
			argv.nlp_lang_num = nlp_lang_num;
			argv.NormOnly = NormOnly;
			argv.LenOfNorm = LenOfNorm;
			doAAA(TestMaxThreadCount, (void*)&argv);
			
			target.clear();
		}

		rr->unload();
		delete rr, rr = 0;
		fclose(fout);
		fclose(fin);

	} catch (ModException& e) {
		ModErrorMessage << "ModException!: " << e << ModEndl;
		cout << "ModException!: "
		<< e.getErrorModule() << " "
		<< e.getErrorNumber() << " "
		<< e.getErrorLevel() << " "
		<< e.getMessageBuffer() << "."
		<< endl;
		exit(1);
	} catch (...) { 
		ModErrorMessage << "Unexpected exception!" << ModEndl;
		exit(1);
	}

	return 0;
}
