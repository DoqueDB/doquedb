// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// 呼ばれない関数のテストプログラム()
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
#include "ModCharString.h"
#include "ModUnicodeString.h"

#include "ModLanguageSet.h"
#include "EnStem/ModEnglishWordStemmer.h"
#include "LibUna/DicSet.h"
#include "ModNlpUnaJp/ModUnaMiddle.h"

using namespace std;

#define MAX_DAT_LEN 655360

const ModUnicodeChar SEPARATOR = 0x0a; // \n
const ModUnicodeChar EXP_SEP   = 0x2c; // ,

int main()
{
	FILE* fout;
	fout = fopen("notcallfunc.result", "w");
	if (!fout) {
		cerr << "ERROR: output file not opened." << endl;
		exit(1);
	}

	ModOs::Process::setEncodingType(ModKanjiCode::literalCode);
	ModMemoryPool::setTotalLimit(256*1024);

	try {
		ModMemoryPool::initialize(ModSizeMax >> 10);

		// ModEnglishWordStemmer クラスのテスト

		ModCharString out_path("./stem.test");
		ModCharString stem_path("../src/dat.stem/codeTest/path/path.txt2");
		UNA::ENSTEM::ModEnglishWordStemmerDataPath path(stem_path);
		UNA::ENSTEM::ModEnglishWordStemmer stemmer(path, out_path);

		ModCharString stem_path2("../unadic/stem/stemmer.dat");
		UNA::ENSTEM::ModEnglishWordStemmer stemmer2(stem_path2);
	} catch (ModException& e) {
		ModErrorMessage << "ModException!: " << e << ModEndl;
		cout << "ModException!: "
		<< e.getErrorModule() << " "
		<< e.getErrorNumber() << " "
		<< e.getErrorLevel() << " "
		<< e.getMessageBuffer() << "."
		<< endl;
		fputs("NG(ModException!)", fout);
		exit(1);
	} catch (...) { 
		ModErrorMessage << "Unexpected exception!" << ModEndl;
		fputs("NG(Unexpected exception!)", fout);
		exit(1);
	}
	fputs("OK", fout);
	if (fout) fclose(fout);

	return 0;
}

