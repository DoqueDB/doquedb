// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Instance.java -- ClassIDに相当するクラスのインスタンスを得る
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
 * {@link ClassID クラスID}に相当するクラスのインスタンスを得る
 *
 */
public final class Instance
{
	/**
	 * {@link ClassID クラスID}に相当するSerializableのサブクラスの
	 * インスタンスを得る
	 *
	 * @param id_	{@link ClassID クラスID}
	 * @return Serializableのサブクラス
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 */
	public static Serializable get(int id_)
		throws ClassNotFoundException
	{
		Serializable object = null;
		switch (id_)
		{
		case ClassID.NONE:
			break;
		case ClassID.STATUS:
			object = new Status();
			break;
		case ClassID.INTEGER_DATA:
			object = new IntegerData();
			break;
		case ClassID.UNSIGNED_INTEGER_DATA:
			object = new IntegerData();	//Javaにはunsignedはないので
			break;
		case ClassID.INTEGER64_DATA:
			object = new Integer64Data();
			break;
		case ClassID.UNSIGNED_INTEGER64_DATA:
			object = new Integer64Data(); //Javaにはunsignedはないので
			break;
		case ClassID.FLOAT_DATA:
			object = new FloatData();
			break;
		case ClassID.DOUBLE_DATA:
			object = new DoubleData();
			break;
		case ClassID.DECIMAL_DATA:
			object = new DecimalData();
			break;
		case ClassID.STRING_DATA:
			object = new StringData();
			break;
		case ClassID.DATE_DATA:
			object = new DateData();
			break;
		case ClassID.DATE_TIME_DATA:
			object = new DateTimeData();
			break;
		case ClassID.INTEGER_ARRAY_DATA:
			object = new IntegerArrayData();
			break;
		case ClassID.UNSIGNED_INTEGER_ARRAY_DATA:
			object = new IntegerArrayData();	// Javaにはunsignedはないので
			break;
		case ClassID.STRING_ARRAY_DATA:
			object = new StringArrayData();
			break;
		case ClassID.DATA_ARRAY_DATA:
			object = new DataArrayData();
			break;
		case ClassID.BINARY_DATA:
			object = new BinaryData();
			break;
		case ClassID.NULL_DATA:
			object = NullData.getInstance();
			break;
		case ClassID.EXCEPTION_DATA:
			object = new ExceptionData();
			break;
//		case ClassID.PARAMETER:
//			object = new Parameter();
//			break;
//		case ClassID.BITSET:
//			object = new BitSet();
//			break;
		case ClassID.COMPRESSED_STRING_DATA:
			// CompressedStringDataは伸長されてくるので...
			object = new StringData();
			break;
		case ClassID.COMPRESSED_BINARY_DATA:
			object = new CompressedBinaryData();
			break;
//		case ClassID.OBJECTID_DATA:
//			object = new ObjectIDData();
//			break;
		case ClassID.REQUEST:
			object = new Request();
			break;
		case ClassID.LANGUAGE_DATA:
			object = new LanguageData();
			break;
//		case ClassID.SQL_DATA:
//			object = new SQLData();
//			break;
		case ClassID.COLUMN_META_DATA:
			object = new ColumnMetaData();
			break;
		case ClassID.RESULTSET_META_DATA:
			object = new ResultSetMetaData();
			break;
		case ClassID.WORD_DATA:
			object = new WordData();
			break;
		case ClassID.ERROR_LEVEL:
			object = new ErrorLevel();
			break;
		default:
			throw new ClassNotFoundException();
		}

		return object;
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
