// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TermResourceManager.h -- 
// 
// Copyright (c) 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_UTILITY_TERMRESOURCEMANAGER_H
#define __SYDNEY_UTILITY_TERMRESOURCEMANAGER_H

#include "Utility/Module.h"
#include "ModTypes.h"

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

class ModTermResource;

//
//	CLASS
// 	Utility::TermResourceManager -- 質問処理ライブラリのリソースを管理する
//
//	NOTES
//
class TermResourceManager
{
public:
	// リソースを得る
	SYD_UTILITY_FUNCTION
	static const ModTermResource* get(ModSize uiResourceID_);

	// リソースを開放する
	SYD_UTILITY_FUNCTION
	static void terminate();
};

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif //__SYDNEY_UTILITY_TERMRESOURCEMANAGER_H

//
//	Copyright (c) 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
