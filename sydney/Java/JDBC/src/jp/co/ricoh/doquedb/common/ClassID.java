// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ClassID.java -- DoqueDBのCommonのクラスID
//
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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
 * DoqueDB の Common のクラスIDと同じ番号を持つクラスID。
 *
 */
public final class ClassID
{
	// DoqueDB の Common::ClassIDと同じ数字である

	public final static int NONE							= 0;

	public final static int STATUS							= 1;
	public final static int INTEGER_DATA					= 2;
	public final static int UNSIGNED_INTEGER_DATA			= 3;
	public final static int INTEGER64_DATA					= 4;
	public final static int UNSIGNED_INTEGER64_DATA			= 5;
	public final static int FLOAT_DATA						= 6;
	public final static int DOUBLE_DATA						= 7;
	public final static int DECIMAL_DATA					= 8;
	public final static int STRING_DATA						= 9;
	public final static int DATE_DATA						= 10;
	public final static int DATE_TIME_DATA					= 11;
	public final static int INTEGER_ARRAY_DATA				= 12;
	public final static int UNSIGNED_INTEGER_ARRAY_DATA		= 13;
	public final static int STRING_ARRAY_DATA				= 14;
	public final static int DATA_ARRAY_DATA					= 15;
	public final static int BINARY_DATA						= 16;
	public final static int NULL_DATA						= 17;
	public final static int EXCEPTION_DATA					= 18;
//	public final static int PARAMETER						= 19;
//	public final static int BITSET							= 20;
	public final static int COMPRESSED_STRING_DATA			= 21;
	public final static int COMPRESSED_BINARY_DATA			= 22;
//	public final static int OBJECTID_DATA					= 23;
	public final static int REQUEST							= 24;
	public final static int LANGUAGE_DATA					= 25;
//	public final static int SQL_DATA						= 26;
	public final static int COLUMN_META_DATA				= 27;
	public final static int RESULTSET_META_DATA				= 28;
	public final static int WORD_DATA						= 29;
	public final static int ERROR_LEVEL						= 30;
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

