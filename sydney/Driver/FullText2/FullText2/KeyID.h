// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KeyID.h --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_KEYID_H
#define __SYDNEY_FULLTEXT2_KEYID_H

#include "FullText2/Module.h"
#include "LogicalFile/FileID.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	STRUCT
//	FullText2::VersionNum
//
//	NOTES
//
struct VersionNum
{
	// バージョン
	enum
	{
		Version1 = 0,
		Version2,
		Version3,
		
		// ここからFullText2
		Version4,
		Version5,		// PageManageFile2利用

		// バージョン数
		ValueNum,
		// 現在のバージョン
		CurrentVersion = Version4,
	};
};

//
//	STRUCT
//	FullText2::KeyID --
//
//	NOTES
//	LogicalFile::FileIDのキー値のみを定義したファイル
//	libTermをUtilityで使うようにしたため FileID.h から分離した
//
struct KeyID
{
	enum Value
	{
		// [NOTE] キーを追加する時は末尾に追加する。

		// 文字列キーの数(Integer)
		KeyCount = LogicalFile::FileID::DriverNumber::FullText2,

		// 遅延更新かどうか(Integer)
		DelayedMode,
		// セクション検索かどうか(Boolean)
		IsSectionized,
		// 言語列があるか(Boolean)
		IsLanguage,
		// スコア調整カラムがあるか(Boolean)
		IsScoreField,
		// 粗いKWICを取得するかどうか(Boolean)
		IsRoughKwic,
			
		// リーフページのページサイズ(Integer)
		LeafPageSize,
		// オーバーフローページのページサイズ(Integer)
		OverflowPageSize,

		// インデックスタイプ(Integer)
		IndexingType,
		// トークナイズパラメータ(String)
		TokenizeParameter,
		// 異表記正規化(Boolean)
		IsNormalized,

		// 文書ID圧縮器(String)
		IdCoder,
		// 頻度圧縮器(String)
		FrequencyCoder,
		// ビット長圧縮器(String)
		LengthCoder,
		// 位置情報圧縮器(String)
		LocationCoder,
		// 文書ID圧縮器 - Dual時セパレータ用(String)
		WordIdCoder,
		// 頻度圧縮器 - Dual時セパレータ用(String)
		WordFrequencyCoder,
		// ビット長圧縮器 - Dual時セパレータ用(String)
		WordLengthCoder,
		// 位置情報圧縮器 - Dual時セパレータ用(String)
		WordLocationCoder,

		// ステミング(Boolean)
		IsStemming,
		// 改行を跨って正規化を行うかどうか(Boolean)
		IsCarriage,
		// スペースを除去するかどうか(Boolean)
		IsDeleteSpace,

		// 抽出パラメータ(String)
		Extractor,
		// デフォルト言語指定(String)
		Language,

		// B木ファイルのページサイズ(Integer)
		BtreePageSize,

		// ファイル分散数(Integer)
		Distribute,

		// clusteredキーワード(Boolean)
		IsClustered,
		// 特徴数(Integer)
		Feature,

		// 位置情報を格納しないかどうか(Boolean)
		IsNolocation,
		// TF値を格納しないかどうか(Boolean)
		IsNoTF,

		// 削除フラグを利用するかどうか(Boolean)
		IsExpungeFlag,

		// 最大文書長(Integer)
		MaxWordLength,

		// バキュームを利用するかどうか(Boolean)
		IsVacuum
	};
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_KEYID_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
