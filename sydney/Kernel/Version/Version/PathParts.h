// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PathParts.h -- ファイルのパス名関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_VERSION_PATHPARTS_H
#define	__SYDNEY_VERSION_PATHPARTS_H

#include "Version/Module.h"

#include "Os/Unicode.h"

_SYDNEY_BEGIN
_SYDNEY_VERSION_BEGIN

namespace PathParts
{
	//	CONST
	//	Version::PathParts::MasterData --
	//		マスタデータファイルのファイルの名前
	//
	//	NOTES

	const Os::Ucs2 MasterData[] =
		{ 'M','A','S','T','E','R','.','S','Y','D', 0 };

	//	CONST
	//	Version::PathParts::VersionLog --
	//		バージョンログファイルのファイルの名前
	//
	//	NOTES

	const Os::Ucs2 VersionLog[] =
		{ 'V','E','R','S','I','O','N','.','S','Y','D', 0 };

	//	CONST
	//	Version::PathParts::SyncLog --
	//		同期ログファイルのファイルの名前
	//
	//	NOTES

	const Os::Ucs2 SyncLog[] =
		{ 'S','Y','N','C','L','O','G','.','S','Y','D', 0 };
}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_PATHPARTS_H

//
// Copyright (c) 2000, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
