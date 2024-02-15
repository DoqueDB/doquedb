// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModTermExceptionMessage.h --
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

ModExceptionMessageAssoc ModModuleTermMessageArray[] = {
	{ ModTermErrorFileOpenFail, "open file error" },
	{ ModTermErrorFileReadFail, "read file error" },
	{ ModTermErrorPoolSize, "pool size error" },
	{ ModTermErrorRxCompile, "rx compile error" },
	{ 0, (char*)0 }
};
static ModExceptionMessage ModModuleTermMessage(ModModuleTerm, ModModuleTermMessageArray);

//
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
