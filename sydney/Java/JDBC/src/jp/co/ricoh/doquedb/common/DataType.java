// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataType.java -- データ型のタイプ
//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.common;

/**
 * データ型のタイプ
 *
 */
public final class DataType
{
	// DoqueDBのCommon::DataTypeと同じ数字である
	// Javaにはunsignedが存在しないので、その部分はコメントアウトしてある

	public final static int DATA				= 1000;
	public final static int INTEGER				= 1001;
//	public final static int UNSIGNED_INTEGER	= 1002;
	public final static int INTEGER64			= 1003;
//	public final static int UNSIGNED_INTEGER64	= 1004;
	public final static int STRING				= 1005;
	public final static int FLOAT				= 1006;
	public final static int DOUBLE				= 1007;
	public final static int DECIMAL				= 1008;
	public final static int DATE				= 1009;
	public final static int DATE_TIME			= 1010;
	public final static int BINARY				= 1011;
	public final static int BITSET				= 1012;
	public final static int OBJECTID			= 1013;
	public final static int LANGUAGE			= 1014;
	public final static int COLUMN_META_DATA	= 1015;
	public final static int WORD				= 1016;

	public final static int ARRAY				= 2000;

	public final static int NULL				= 3000;

	public final static int UNDEFINED			= 9999;
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
