// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorReason.java -- ロード中のエラー理由を示す値
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.sydney.admin.util.load;

public class ErrorReason
{
	public static final int	NO_LOAD_TABLE_NAME = 1;					// ロードファイルのヘッダで table が指定されていない
	public static final int	NO_COLUMN_SEPARATOR = 2;				// ロードファイルのヘッダで column_separator が指定されていない
	public static final int	DUPLICATE_SEPARATOR = 3;				// ロードファイルのヘッダで column_separator と arrayelement_separator が重複している
	public static final int	NO_ARRAY_ELEMENT_SEPARATOR = 4;			// テキスト形式のロードファイルのヘッダで arrayelement_separator が指定されていない
	public static final int	NOT_SUPPORTED_ELEMENT_DATA_TYPE = 5;	// Sydney 側では対応しているが管理ツールで対応漏れとなっている要素のデータ型がある場合に発生
	public static final int	ILLEGAL_COMMIT_COUNT = 6;				// ロードファイルのヘッダで指定されている commit_count が数値に変換できない
	public static final int	NOT_ARRAY_COLUMN = 7;					// 配列型ではない列で element が指定されていた
	public static final int	NOT_SET_ELEMENT = 8;					// 配列型なのに element が指定されていない
	public static final int	BAD_SPECFILE = 9;						// SpecFile が正しくない
	public static final int FAILED_TO_GET_DB_TABLE = 10;			// DBのデータ取得に失敗した
}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
