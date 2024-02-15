// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// 形態素解析＋正規化機能テストプログラム(NlpAnalyzer)
// 
// Copyright (c) 2005, 2022, 2023 Ricoh Company, Ltd.
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
#ifdef WIN32
#include <windows.h>
#include "psapi.h"
#endif
#include "ModAutoPointer.h"
#include "ModOstrStream.h"
#include "ModUnicodeString.h"
#include "ModNLP.h"
#include "ModNLPText.h"
#include "ModTime.h"
#include "ModTimeSpan.h"
#include "ModLanguageSet.h"

using namespace std;

const ModSize MAX_DAT_LEN = 655360;
const ModSize MAX_LANGUAGE_NUM = 9;

const ModUnicodeString NLP_PATH("../unadic/");

const ModUnicodeChar SEPARATOR = 0x0a; // \n
const ModUnicodeChar EXP_SEP   = 0x2c; // ,
const ModUnicodeChar VEC_SEP   = 0x23; // #

static ModTimeSpan TOTAL_TIME;

////////////////////////////////////////////////////////////////////////////////////////////
class TRMText : public ModNlpText
{
public:
	TRMText(FILE*& file,ModKanjiCode::KanjiCodeType code, ModBoolean do_line, ModSize ulMaxLen_ = 65535);
	~TRMText();
	ModBoolean read(ModUnicodeString& cstrPartOfTarget_,ModSize ulReadLength_);
private:
	ModBoolean get_target(ModUnicodeString* target_,ModSize ulReadLength_);
	ModBoolean _do_line;
	ModUnicodeString* _pcstrBuffer;
	ModSize _ulOffset;
};

TRMText::TRMText(FILE*& file,ModKanjiCode::KanjiCodeType code, ModBoolean do_line, ModSize ulMaxLen_)
 : ModNlpText(ulMaxLen_),
   _do_line(do_line),
   _pcstrBuffer(0),
   _ulOffset(0)
{
	int maxBufSize = MAX_DAT_LEN;
	int orgMaxBufSize = maxBufSize;
	char* buf = static_cast<char*>(ModDefaultManager::allocate(maxBufSize));
	*buf = '\0';
	char* orgBuf = buf;
	while (fgets(buf, maxBufSize, file))
	{
		if((maxBufSize -= strlen(buf))<=1)
		{
			ModErrorMessage << "Buffer size is small to read file contents." << ModEndl;
		}
		buf += strlen(buf);
	}
	ModAutoPointer <ModUnicodeString> cBuffer = new ModUnicodeString(orgBuf,0,code);
	ModDefaultManager::free(orgBuf,orgMaxBufSize);

	_pcstrBuffer = cBuffer.release();
}

TRMText::~TRMText()
{
	delete _pcstrBuffer,_pcstrBuffer=0;
}

ModBoolean
TRMText::read(ModUnicodeString& cstrPartOfTarget_,ModSize ulReadLength_)
{
	if (ulReadLength_ <= 0 || _ulOffset >=_pcstrBuffer->getLength()) return ModFalse;
	if (get_target(&cstrPartOfTarget_,ulReadLength_)==ModFalse)
		return ModFalse;
#ifdef NLPTEST_DEBUG
	ModDebugMessage << cstrPartOfTarget_ <<  ModEndl;
#endif
	return ModTrue;
}


ModBoolean
TRMText::get_target(ModUnicodeString* target, ModSize ulReadLength_)
{
	target->clear();

	if (_do_line == ModTrue)
	//改行まででreturnする。但し改行コードは取る
	{
		for (int iLineOffset=0;/*forever*/;++iLineOffset)
		{
			if((_ulOffset+iLineOffset >= _pcstrBuffer->getLength())
			 ||(static_cast<const ModUnicodeChar*>(*_pcstrBuffer)[_ulOffset+iLineOffset]==0x000A)
			 ||(iLineOffset>=ulReadLength_))
			{
				*target = _pcstrBuffer->copy(_ulOffset,iLineOffset + sizeof('\n'));
				_ulOffset += (iLineOffset + sizeof('\n'));
				ModUnicodeChar* r = target->rsearch('\n');
				if (r != 0)
				{
					// 末尾の改行を削除
					target->truncate(r);
				}
				return ModTrue;
			}
		}
	}
	else
	{
		int iLineOffset=0;
		int iTextOffset=0;
		for (iTextOffset=0;iTextOffset<ulReadLength_;iTextOffset++)
		{
			if((_ulOffset+iLineOffset >= _pcstrBuffer->getLength())
			 ||(static_cast<const ModUnicodeChar*>(*_pcstrBuffer)[_ulOffset+iLineOffset]==0x000A))
			{
				if ( target->getLength() == 0 )
				{
					if (_ulOffset+iLineOffset >= _pcstrBuffer->getLength())
					{
						*target = _pcstrBuffer->copy(_ulOffset,iLineOffset);
						_ulOffset += iLineOffset;
						break;
					}
					else
					{
						*target = _pcstrBuffer->copy(_ulOffset,iLineOffset + sizeof('\n'));
						_ulOffset += (iLineOffset + sizeof('\n'));
						iLineOffset = 0;
						continue;
					}
				}
				else if (iLineOffset==0)
				//このパターンは複数行を返す
				//テストではこのパターンは使用しない
				{
					if (_ulOffset+iLineOffset < _pcstrBuffer->getLength())
					{
						_ulOffset += sizeof('\n');
					}
					ModUnicodeChar* r = target->rsearch('\n');
					if(r!=0)
					{
						target->truncate(r);
					}

					return ModTrue;
				}
				else
				{
					if (_ulOffset+iLineOffset >= _pcstrBuffer->getLength())
					{
						ModUnicodeString line = _pcstrBuffer->copy(_ulOffset,iLineOffset);
						*target += line;
						_ulOffset += iLineOffset;
						break;
					}
					else
					{
						ModUnicodeString line = _pcstrBuffer->copy(_ulOffset,iLineOffset+ sizeof('\n'));
						*target += line;
						_ulOffset += (iLineOffset + sizeof('\n'));
						iLineOffset = 0;
						continue;
					}
				}
			}
			iLineOffset++;
		}
		if (iTextOffset == ulReadLength_)
		{
			if (iLineOffset!=0)
			{
				if ( target->getLength() == 0 )
				{
					*target = _pcstrBuffer->copy(_ulOffset,iLineOffset);
				}
				else
				{
					ModUnicodeString line = _pcstrBuffer->copy(_ulOffset,iLineOffset);
					*target += line;
				}
				_ulOffset += iLineOffset;
			}
			else
			//この場合はリードバッファが一杯になっているのだが同時に
			//最後がLF終わりなのですでにループ内で処理が行われている。
			//また、本当のバッファが最後以外はLFをとってはいけない。
			{
				if ( target->getLength() == 0 )
				{
					return ModFalse;
				}
				if (_pcstrBuffer->getLength() != _ulOffset)
				{
					return ModTrue;
				}
			}
		}

		if ( target->getLength() != 0 )
		{
			ModUnicodeChar* r = target->rsearch('\n');
			if((r!=0)
			 &&(static_cast<const ModUnicodeChar*>(*target)[target->getLength()-1]==0x000A))
			{
				// 末尾の改行を削除
				//改行を削除するとたとえば、"ああ\nいいい"という文字列が
				//読み込まれているとき "ああ"になってしまう場合があったが、
				//末尾がLFかのチェックも加えるようにしたので今はそういうことはない
				target->truncate(r);
			}
			return ModTrue;
		}
		return ModFalse;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

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

void print_help() {
	cerr << "Usage: "
		 << "[-h] "
		 << "[-r DIR] "
		 << "[-i FILE] "
		 << "[-o FILE] "
		 << "[-l] "
		 << "[-w maxWordLength] "
		 << "[-p] "
		 << "[-P KeyString ValueString] "
		 << "[-j] "
		 << "[-x] "
		 << "[-d] "
		 << "[-t] "
		 << "[-T tagString] "
		 << "[-s1 CHAR] "
		 << "[-S1 HEX] "
		 << "[-s2 CHAR] "
		 << "[-S2 HEX] "
		 << "[-m  MODE] "
		 << "[-e] "
		 << "[-C] "
		 << "[-SS] "
		 << "[-SD] "
		 << "[-2D] "
		 << "[-MR] "
		 << "[-MRL LEVEL] "
		 << "[-U] "
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
		 << "\t-T TSTR  : タグ<TSTR>を検出する" << endl
		 << "\t-j       : 言語指定が日本語を含まない時に日本語辞書保持" << endl
		 << "\t-O       : 原表記取得" << endl
		 << "\t-k       : 原表記取得&品詞取得(キーワード抽出用)" << endl
		 << "\t-p       : 原文を表示" << endl
		 << "\t-P KEY VAL: パラメータ(キー文字 値文字)をセット" << endl
		 << "\t-x       : 正規化を行わない(デフォルトは実施)" << endl
		 << "\t-d       : 詳細情報を表示(区切りはデフォルト)" << endl
		 << "\t-s1 CHAR : 形態素区切り(デフォルトは改行)" << endl
		 << "\t-S1 HEX  : 形態素区切り(デフォルトは0x0a)" << endl
		 << "\t-s2 CHAR : 展開表記区切り(デフォルトはコンマ)" << endl
		 << "\t-S2 HEX  : 展開表記区切り(デフォルトは0x2c)" << endl
		 << "\t-m  MODE : 下位構造展開(MODE=1,2)/ステミング実施(MODE=1,3)" << endl
		 << "\t-U  MODE : mode of intializing resource (MODE = 0,1,2,3)" << endl
		 << "\t-SS      : 空白文字の削除処理を行わない" << endl
		 << "\t-e(xpand): 異表記展開を実施(デフォルトは正規化のみ)" << endl
		 << "\t-C(heck) : 展開時に包含関係な候補を削除する" << endl
		 << "\t-SD      : 空白文字の削除処理を行う" << endl
		 << "\t-2D      : 2つの正規化データが存在するデータセットをサポートする" << endl
		 << "\t-MR      : 省メモリ動作 MRL=1相当" << endl
		 << "\t-MRL LEVEL : 省メモリ動作のレベル指定 0-2" << endl
		 << "\t-K1 MIN  : この値より小さくなったときにリードする" << endl
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
	ModLanguageSet nlp_lang[MAX_LANGUAGE_NUM];
	ModSize	nlp_lang_num=1;

	int normMode(1);
	ModUnicodeChar sep(SEPARATOR);
	ModUnicodeChar exp_sep(EXP_SEP);
	ModUnicodeChar vec_sep(VEC_SEP);
	ModUnicodeString tstr("");

	ModBoolean do_line(ModFalse);
	ModBoolean print_input(ModFalse);
	ModBoolean print_detail(ModFalse);
	ModBoolean doNormalizeMode(ModTrue);
	ModBoolean get_vector(ModFalse);
	ModBoolean get_time(ModFalse);
	ModBoolean get_original(ModFalse);
	ModBoolean use_keyWordFunc(ModFalse);
	ModBoolean keep_jDic(ModFalse);
	ModBoolean support2D(ModFalse);
	ModBoolean rv = ModTrue;
	ModBoolean mr(ModFalse);
	ModSize maxWordLength=0;
	ModSize memoryReducingLevel=0;
	ModSize ulMaxLen_=65535;

	ModBoolean expand(ModFalse);
	ModBoolean np_extract(ModFalse);
	/*ModNlpExpMode*/ int expMode = 1;//1=ModNlpExpNoChk
	ModBoolean test_error(ModFalse);
	ModSize funcNo(0);
	ModBoolean getWholeText(ModFalse);
	ModBoolean      NormOnly = ModFalse;
	ModSize         LenOfNorm = 0;
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

		} else if (strncmp(av[i], "-g", 2) == 0) { // getWholeText
			getWholeText = ModTrue;

		} else if (strncmp(av[i], "-N", 2) == 0) { 
			NormOnly = ModTrue;

		} else if (strncmp(av[i], "-np", 3) == 0) { 
			np_extract = ModTrue;

		} else if (strncmp(av[i], "-n", 2) == 0) { 
			LenOfNorm = atoi(av[++i]);

		} else if (strncmp(av[i], "-m", 2) == 0) { // モード指定
			normMode = atoi(av[++i]);

		} else if (strncmp(av[i], "-w", 2) == 0) { // モード指定
			maxWordLength = atoi(av[++i]);

		} else if (strncmp(av[i], "-i", 2) == 0) { // 入力ファイル
			fin = fopen(av[++i], "rb");

		} else if (strncmp(av[i], "-o", 2) == 0) { // 出力ファイル
			fout = fopen(av[++i], "wb");

		} else if (strncmp(av[i], "-T", 2) == 0) { // タグ文字列
			tstr = av[++i];

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

		} else if (strncmp(av[i], "-j", 2) == 0) {
			keep_jDic = ModTrue;

		} else if (strncmp(av[i], "-SS", 3) == 0) {
			strcpy(czSpace,"3");

		} else if (strncmp(av[i], "-SD", 3) == 0) {
			strcpy(czSpace,"2");

		} else if (strncmp(av[i], "-2D", 3) == 0) {
			support2D=ModTrue;

		} else if (strncmp(av[i], "-MRL", 4) == 0) {
			memoryReducingLevel = atoi(av[++i]);

		} else if (strncmp(av[i], "-MR", 3) == 0) {
			mr = ModTrue;
			memoryReducingLevel =1;

		} else if (strncmp(av[i], "-K1", 3) == 0) {
			ulMaxLen_ = atoi(av[++i]);

		} else if (strncmp(av[i], "-p", 2) == 0) {
			print_input = ModTrue;

		} else if (strncmp(av[i], "-P", 2) == 0) { // string param
			params.insert(av[i+1], av[i+2]);
			i+=2;

		} else if (strncmp(av[i], "-x", 2) == 0) {
			doNormalizeMode = ModFalse;

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
		} else if (strncmp(av[i], "-s", 2) == 0) {
			vec_sep = ModUnicodeChar(av[++i][0]);

		} else if (strncmp(av[i], "-S", 2) == 0) {
			vec_sep = ModUnicodeChar(strtol(av[++i], 0, 16));

		} else if (strncmp(av[i], "-e", 2) == 0) {
			expand = ModTrue;

		} else if (strncmp(av[i], "-t", 2) == 0) {
			get_time = ModTrue;

		} else if (strncmp(av[i], "-O", 2) == 0) {
			get_original = ModTrue;

		} else if (strncmp(av[i], "-k", 2) == 0) {
			use_keyWordFunc = ModTrue;

		} else if (strncmp(av[i], "-C", 2) == 0) {
			expMode = 0;//0=ModNlpExpChkOrigStr

		} else if (strncmp(av[i], "-X", 2) == 0) {
			test_error = ModTrue;

		} else if (strncmp(av[i], "-b", 2) == 0) { 
			emulateOldDmja = ModTrue;
		} else if (strncmp(av[i], "-v", 2) == 0) {
			get_vector = ModTrue;

		} else if (strncmp(av[i], "-F", 2) == 0) {
			funcNo = atoi(av[++i]);

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
	ModUnicodeString cstrLang;
	ModUnicodeString cstrMaxWordLength;
	ModUnicodeString cstrExpMode;
	ModUnicodeString cstrTag;
	ModUnicodeString cstrSpace;
	ModUnicodeString cstrDoNorm;
	ModUnicodeString cstrStemming;
	ModUnicodeString cstrCompoundDiv;
	ModUnicodeString cstrCarriageRet;

	try 
	{
		ModMemoryPool::initialize(ModSizeMax >> 10);
		ModSize currentLang = 0;

		bMem = memoryReducingLevel == 1 ? ModTrue : ModFalse;
		UNA::ModNlpResource* rr;
		rr = new UNA::ModNlpResource();
		rr->load(nlp_path,bMem);
		
		// set flag for specifying ModNLPAnalyzer to use T-lib only
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
		char czExpMode[10];
		sprintf(czExpMode,"%d",expMode);
		cstrExpMode = czExpMode;
		params.insert("expmode",cstrExpMode);
		cstrTag = tstr;
		params.insert("tag",cstrTag);
		cstrSpace = czSpace;
		params.insert("space",cstrSpace);
		cstrDoNorm = doNormalizeMode == ModTrue ? "true" : "false";
		params.insert("donorm",cstrDoNorm);

		TOTAL_TIME = 0;
		TRMText cTarget(fin,io_code,do_line,ulMaxLen_);

		try{
			nlp->prepare(params);
			nlp->set(&cTarget, nlp_lang[currentLang]);
		} catch (ModException& e) {
			ModErrorMessage << "ModException!: " << e << ModEndl;
			ModErrorHandle::reset();
			currentLang = (currentLang+1)%nlp_lang_num;
		} catch (...) { 
			ModErrorMessage << "Unexpected exception!" << ModEndl;
			ModErrorHandle::reset();
			currentLang = (currentLang+1)%nlp_lang_num;
		}
		currentLang = (currentLang+1)%nlp_lang_num;
		if (NormOnly == ModTrue)
		{		
			if (expand == ModTrue)
			{
				while (1)
				{	
					ModBoolean rv;
					ModVector<ModUnicodeString> expanded;

					t0 = ModTime::getCurrentTime();
					rv = nlp->getExpandBuf( expanded );
					t1 = ModTime::getCurrentTime();
					TOTAL_TIME = TOTAL_TIME + (t1-t0);
					if ( rv != ModTrue){
						break;
					}
					for(int ee = 0;ee < expanded.getSize();++ee)
						fprintf(fout,"%s\n",expanded[ee].getString(io_code));
				}
			}
			else
			{
				while (1)
				{	
					ModBoolean rv;
					ModUnicodeString wholeText;

					t0 = ModTime::getCurrentTime();
					rv = nlp->getNormalizeBuf( wholeText );
					t1 = ModTime::getCurrentTime();
					TOTAL_TIME = TOTAL_TIME + (t1-t0);
					if ( rv != ModTrue){
						break;
					}
					fprintf(fout,"%s",wholeText.getString(io_code));
				}
			}
			fputc(sep, fout);
			fputc(0x0a, fout);
			if (do_line == ModFalse && sep != '\n') {
				fputc(0x0a, fout);
			}
		}
		else if (expand == ModTrue) {	// 展開
			ModVector<ModUnicodeString> expanded;
			ModUnicodeString original;
			int pos;
			ModBoolean rv;
			int num(0);
			while (1){
				try{
					t0 = ModTime::getCurrentTime();
					rv = nlp->getExpandWords(expanded,original,pos);
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

				if (rv != ModTrue){
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
				fputs(expanded[0].getString(io_code), fout);
				for (int xx = 1; xx < expanded.getSize(); ++xx){
					fputc(exp_sep, fout);
					fputs(expanded[xx].getString(io_code), fout);
				}
			}
			fputc(sep, fout);
			fputc(0x0a, fout);
			if (do_line == ModFalse && sep != '\n') {
				fputc(0x0a, fout);
			}
		}
		else if (np_extract == ModTrue)
		//NP抽出
		{
			int num(0);
			while (1)
			{	
				ModUnicodeString original;
				ModVector<ModUnicodeString> normVector;
				ModVector<ModUnicodeString> orgVector;
				ModVector<int> posVector;

				t0 = ModTime::getCurrentTime();
				rv = nlp->getConcept( original,
						      normVector,
						      orgVector,
						      posVector);
				t1 = ModTime::getCurrentTime();
				TOTAL_TIME = TOTAL_TIME + (t1-t0);
				
				if ( rv != ModTrue){
					break;
				}
				++ num;
				if ( get_vector == ModTrue )
				{
					int i = 0;
					for( i = 0; i < orgVector.getSize(); i++ )
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
					fprintf(fout,"\n");
				}
				else
				{
					if (sep != '\n' || num > 1) {
						fputc(sep, fout);
					}
					if (print_detail == ModTrue) {
						fprintf(fout, "%d", num);
						fputc(0x20, fout);
					}

					fputs(original.getString(io_code), fout);
					fprintf(fout,"\n");

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
			if (get_vector != ModTrue || num != 0)
			{
			 	fputc(sep, fout);
				fputc(0x0a, fout);
				if (do_line == ModFalse && sep != '\n')
				{
					fputc(0x0a, fout);
				}
			}
		}
		else if (funcNo == 3)//3 is getBlock
		{
			while ( 1){
				if ( get_vector == ModFalse && get_original == ModFalse)
				{
					ModUnicodeString norm;
					t0 = ModTime::getCurrentTime();
					if ( nlp->getBlock(norm,vec_sep,ModUnicodeChar(0x0a))==ModFalse)
						break;
					t1 = ModTime::getCurrentTime();
					fputs(norm.getString(io_code), fout);
				}
				else if (get_original == ModFalse)
				{
					ModVector<ModUnicodeString> normVector;
					ModVector<int> posVector;
					t0 = ModTime::getCurrentTime();
					if ( nlp->getBlock(normVector,posVector)==ModFalse)
						break;
					t1 = ModTime::getCurrentTime();
					ModVector<ModUnicodeString>::Iterator form = normVector.begin();
					ModVector<int>::Iterator pos = posVector.begin();
					for ( ;pos<posVector.end();++pos,++form)
					{
						fputs(form->getString(io_code),fout);
						fprintf(fout,"(%d)\n",(*pos));
					}
				}
				else
				{
					ModVector<ModUnicodeString> normVector;
					ModVector<ModUnicodeString> origVector;
					ModVector<int> posVector;
					t0 = ModTime::getCurrentTime();
					if ( nlp->getBlock(normVector,origVector,posVector)==ModFalse)
						break;
					t1 = ModTime::getCurrentTime();
					ModVector<ModUnicodeString>::Iterator form = normVector.begin();
					ModVector<ModUnicodeString>::Iterator ostr = origVector.begin();
					ModVector<int>::Iterator pos = posVector.begin();
					for ( ;pos<posVector.end();++pos,++form,++ostr)
					{
						fputs(form->getString(io_code),fout);
						fprintf(fout,":");
						fputs(ostr->getString(io_code),fout);
						fprintf(fout,":");
						fprintf(fout,"(%d)\n",(*pos));
					}
				}
				TOTAL_TIME = TOTAL_TIME + (t1 - t0);
			}
			fputc(vec_sep, fout);
			fputc(0x0a, fout);
			if (do_line == ModFalse) {
				fputc(0x0a, fout);
			}
		}
		else if (getWholeText == ModTrue)
		{
			while(1)
			{
				ModUnicodeString text;
				t0 = ModTime::getCurrentTime();
				rv = nlp->getWholeText(text);
				t1 = ModTime::getCurrentTime();
				TOTAL_TIME = TOTAL_TIME + (t1-t0);
				if ( rv != ModTrue){
					break;
				}
				
				fputs(text.getString(io_code), fout);
			}
			fputc(sep, fout);
			fputc(0x0a, fout);
			if (do_line == ModFalse && sep != '\n') {
				fputc(0x0a, fout);
			}
		}
		else
		//正規化のみ
		{
			ModUnicodeString word;
			int num(0);
			while (1){
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
				if ( get_original == ModFalse ){
					t0 = ModTime::getCurrentTime();
					rv = nlp->getWord(word);
					t1 = ModTime::getCurrentTime();
					fputs(word.getString(io_code), fout);
				}
				else if ( get_original == ModTrue && use_keyWordFunc == ModFalse ){
					ModUnicodeString ostring;
					t0 = ModTime::getCurrentTime();
					rv = nlp->getWord(word, ostring);
					t1 = ModTime::getCurrentTime();
					fputs(word.getString(io_code), fout);
					fputc(':', fout);
					fputs(ostring.getString(io_code), fout);
				}
				else{
					ModUnicodeString ostring;
					int pos;
					t0 = ModTime::getCurrentTime();
					rv = nlp->getWord(word, ostring, pos);
					t1 = ModTime::getCurrentTime();
					fputs(word.getString(io_code), fout);
					fputc(':', fout);
					fputs(ostring.getString(io_code), fout);
					fprintf(fout,"  %d", pos);
				}
				
				TOTAL_TIME = TOTAL_TIME + (t1-t0);
			}
			fputc(sep, fout);
			fputc(0x0a, fout);
			if (do_line == ModFalse && sep != '\n') {
				fputc(0x0a, fout);
			}
		}

		nlp->releaseResource();
		delete nlp,nlp=0;
		rr->unload();
		delete rr,rr= 0;
		if (fin) fclose(fin);
		if (fout) fclose(fout);
	}//try
	catch (ModException& e)
	{
		ModErrorMessage << "ModException!: " << e << ModEndl;
    		cout << "ModException!:  "
	         << e.getErrorModule() << " "
       		 << e.getErrorNumber() << " "
	         << e.getErrorLevel() << "."
		 << endl;
		exit(1);
	}
	catch (...)
	{
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
