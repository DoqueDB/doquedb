// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SQLTypes.java -- DoqueDBのSQLデータ型
//
// Copyright (c) 2004, 2005, 2023 Ricoh Company, Ltd.
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
 * DoqueDBのSQLデータ型
 *
 * この値は、ColumnMetaData.h の Type の値を同じでなければならない。
 *
 */
public class SQLTypes
{
	/** 不明値 */
	public final static int UNKNOWN = 0;

	/** 文字列型 */
	public final static int CHARACTER = 1;
	public final static int CHARACTER_VARYING = 2;
	public final static int NATIONAL_CHARACTER = 3;
	public final static int NATIONAL_CHARACTER_VARYING = 4;

	/** バイナリ型 */
	public final static int BINARY = 5;
	public final static int BINARY_VARYING = 6;

	/** ラージオブジェクト型 */
	public final static int CHARACTER_LARGE_OBJECT = 7;
	public final static int NATIONAL_CHARACTER_LARGE_OBJECT = 8;
	public final static int BINARY_LARGE_OBJECT = 9;

	/** 整数型 */
	public final static int NUMERIC = 10;
	public final static int SMALL_INT = 11;
	public final static int INTEGER = 12;
	public final static int BIG_INT = 13;

	/** 小数点型 */
	public final static int DECIMAL = 14;
	public final static int FLOAT = 15;
	public final static int REAL = 16;
	public final static int DOUBLE_PRECISION = 17;

	/** ブール型 */
	public final static int BOOLEAN = 18;

	/** 日時型 */
	public final static int DATE = 19;
	public final static int TIME = 20;
	public final static int TIMESTAMP = 21;

	/** 言語型(DoqueDB独自) */
	public final static int LANGUAGE = 22;
	/** ワード(DoqueDB独自) */
	public final static int WORD = 23;

}

//
// Copyright (c) 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
