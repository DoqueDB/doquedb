// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// MOD/UNAバージョン出力
// 
// Copyright (c) 2023 Ricoh Company, Ltd.
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
#include "ModVersion.h"
#include "ModNLP.h"

using namespace std;

int main(int ac, char** av)
{
	ModUnicodeString modVersion = ModVersion::getVersion();
	ModUnicodeString unaVersion = UNA::ModNlpAnalyzer::getVersion();

	cout << "MOD version: " << modVersion.getString(ModKanjiCode::utf8) << endl;
	cout << "UNA version: " << unaVersion.getString(ModKanjiCode::utf8) << endl;
	return 0;
}

