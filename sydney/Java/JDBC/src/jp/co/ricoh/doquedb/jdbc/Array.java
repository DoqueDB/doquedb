// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Array.java -- SQL ARRAY 型の Java プログラミング言語でのマッピング
//
// Copyright (c) 2003, 2004, 2005, 2007, 2009, 2015, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.jdbc;

import java.sql.SQLException;

import jp.co.ricoh.doquedb.common.BinaryData;
import jp.co.ricoh.doquedb.common.Data;
import jp.co.ricoh.doquedb.common.DataArrayData;
import jp.co.ricoh.doquedb.common.DataType;
import jp.co.ricoh.doquedb.common.DateData;
import jp.co.ricoh.doquedb.common.DateTimeData;
import jp.co.ricoh.doquedb.common.DecimalData;
import jp.co.ricoh.doquedb.common.DoubleData;
import jp.co.ricoh.doquedb.common.Integer64Data;
import jp.co.ricoh.doquedb.common.IntegerData;
import jp.co.ricoh.doquedb.common.LanguageData;
import jp.co.ricoh.doquedb.common.NullData;
import jp.co.ricoh.doquedb.common.StringData;
import jp.co.ricoh.doquedb.common.WordData;
import jp.co.ricoh.doquedb.exception.BadArgument;
import jp.co.ricoh.doquedb.exception.NotSupported;

/**
 * SQL <code>java.sql.Types.ARRAY</code> 型の
 * Java プログラミング言語でのマッピング。
 */
public class Array implements java.sql.Array
{
	/** カラムのメタデータ */
	jp.co.ricoh.doquedb.common.ColumnMetaData _metaData;

	/** データ */
	DataArrayData _array;

	/**
	 * 新しく SQL <code>java.sql.Types.ARRAY</code> 型のマッピングを
	 * 作成します。
	 *
	 * @param	array_
	 *			配列型の列オブジェクト。
	 */
	public Array(Object[]	array_)
	{
		_array = new DataArrayData();
		int	numberOfElements = array_.length;
		for (int i = 0; i < numberOfElements; i++) {
			_array.addElement(convertToTRMeisterData(array_[i]));
		}
	}

	/**
	 * 新しく SQL <code>java.sql.Types.ARRAY</code> 型のマッピングを
	 * 作成します。
	 *
	 * @param	arrayData_
	 *			配列型の列のデータ。
	 */
	public Array(DataArrayData	arrayData_)
	{
		this._array = arrayData_;
		this._metaData = null;
	}

	/**
	 * 新しく SQL <code>java.sql.Types.ARRAY</code> 型のマッピングを
	 * 作成します。
	 *
	 * @param	arrayData_
	 *			配列型の列のデータ。
	 */
	public Array(DataArrayData	arrayData_,
				 jp.co.ricoh.doquedb.common.ColumnMetaData metaData_)
	{
		this._array = arrayData_;
		this._metaData = metaData_;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * 配列の要素の SQL の型名を取得します。
	 * 要素が組み込み型の場合、このメソッドは
	 * 要素のデータベース特有の型名を返します。
	 * 要素がユーザ定義型 (UDT) の場合、このメソッドは
	 * 完全指定の SQL の型名を返します。
	 *
	 * @return	組み込み基底型の場合はデータベース特有の名前である
	 *			<code>java.lang.String</code> 、UDT の基底型の場合は
	 *			完全指定の SQL の型名。
	 * @throws	java.sql.SQLException
	 *			型名へのアクセス中にエラーが発生した場合。
	 */
	public String getBaseTypeName() throws java.sql.SQLException
	{
		if (_metaData == null)
			throw new NotSupported();
		return _metaData.getTypeName();
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * 配列の要素の JDBC の型を取得します。
	 *
	 * @return	この <code>Array</code> オブジェクトによって指定された
	 *			配列の要素の型コードである、
	 *			<code>java.sql.Types</code> クラスで定義される、
	 *			JDBC 型と呼ばれる、汎用の SQL 型を識別するためのコード。
	 * @throws	java.sql.SQLException
	 *			基底型へのアクセス中にエラーが発生した場合。
	 */
	public int getBaseType() throws java.sql.SQLException
	{
		if (_metaData == null)
			throw new NotSupported();
		return ResultSetMetaData.getJDBCType(_metaData.getType());
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の内容を、
	 * Java プログラミング言語の配列の形式で取り出します。
	 * このバージョンの <code>getArray</code> メソッドは、
	 * 型マップのカスタマイズのための接続に関連した型マップを使います。
	 *
	 * @return	この <code>Array</code> オブジェクトによって指定された
	 *			SQL <code>java.sql.Types.ARRAY</code> 値の順序付き要素が
	 *			格納されている、 Java プログラミング言語の配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	public Object getArray() throws java.sql.SQLException
	{
		int	numberOfElements = _array.getCount();

		if (numberOfElements == 0) {

			// 空の配列

			switch (getDataType()) {
			case DataType.INTEGER:
				return new Integer[0];
			case DataType.INTEGER64:
				return new Long[0];
			case DataType.DECIMAL:
				return new java.math.BigDecimal[0];
			case DataType.DOUBLE:
				return new Double[0];
			case DataType.STRING:
				return new String[0];
			case DataType.BINARY:
				return new byte[0][0];
			case DataType.DATE:
				return new java.sql.Date[0];
			case DataType.DATE_TIME:
				return new java.sql.Timestamp[0];
			case DataType.LANGUAGE:
				return new LanguageData[0];
			case DataType.WORD:
				return new WordData[0];
			default:
				throw new NotSupported();
			}
		}

		return getArray(1, numberOfElements);
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の内容を取り出します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カスタムマッピングをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	map_
	 *			SQL の型名の Java プログラミング言語のクラスへのマップが
	 *			格納されている <code>java.util.Map</code> オブジェクト。
	 * @return	このオブジェクトによって指定された SQL 配列の
	 *			順序付き要素が格納されている、Java プログラミング言語の配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	public Object getArray(java.util.Map	map_) throws java.sql.SQLException
	{
		return getArray(1, _array.getCount(), map_);
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を取り出します。
	 * 指定された <code>index_</code> で始まり、
	 * 最大で <code>count_</code> の連続する SQL 配列要素が格納されます。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 1 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	public Object getArray(long	index_, int count_)
		throws java.sql.SQLException
	{
		int	numberOfElements = _array.getCount();
		if (index_ < 1 || index_ > numberOfElements) {
			// [YET!] SQLSTATE は、
			//        data exception - ???
			//        (22???)
			throw new BadArgument();
		}

		// 最初の要素のインデックスを 0 にかえる。
		int	index = (int)(index_ - 1);

		int	count = count_;
		if (count_ > numberOfElements - index) {
			count = numberOfElements - index;
		}

		if (count == 0) return null;

		Object	array = null;

		switch (getDataType())
		{
		case DataType.INTEGER:
			array = getIntArray(index, count);
			break;
		case DataType.INTEGER64:
			array = getLongArray(index, count);
			break;
		case DataType.DECIMAL:
			array = getBigDecimalArray(index, count);
			break;
		case DataType.DOUBLE:
			array = getDoubleArray(index, count);
			break;
		case DataType.STRING:
			array = getStringArray(index, count);
			break;
		case DataType.BINARY:
			array = getBytesArray(index, count);
			break;
		case DataType.DATE:
			array = getDateArray(index, count);
			break;
		case DataType.DATE_TIME:
			array = getDateTimeArray(index, count);
			break;
		case DataType.LANGUAGE:
			array = getLangArray(index, count);
			break;
		case DataType.WORD:
			array = getWordArray(index, count);
			break;
		default:
			// [NOT SUPPORTED!] 上記以外は未サポート。
			// [YET!] SQLSTATE は、
			//        feature not supported - data type is not supported
			//        (0A503) ※ 仮
			throw new NotSupported();
		}

		return array;
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を取り出します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カスタムマッピングをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 1 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @param	map_
	 *			SQL の型名とそれらがマッピングされる
	 *			Java プログラミング言語のクラスが格納されている
	 *			<code>java.util.Map</code> オブジェクト。
	 * @return	この <code>Array</code> オブジェクトによって指定された
	 *			SQL <code>java.sql.Types.ARRAY</code> 値の、
	 *			要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する要素が格納されている配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	public Object getArray(long				index_,
						   int				count_,
						   java.util.Map	map_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カスタムマッピングは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の要素が格納されている
	 * 結果セットを取得します。
	 * 適切な場合は、配列の要素は接続の型マップを使ってマッピングされます。
	 * そうでない場合は、標準のマップが使われます。
	 * <P>
	 * 結果セットは配列要素ごとに 1 行を含み、各行には 2 つの列があります。
	 * 2 番目の列には要素の値が格納され、1 番目の列には配列内の対応する
	 * 要素のインデックスが格納されます (最初の配列要素のインデックスは 1 )。
	 * 行は、インデックスに基づく昇順で並べられます。
	 *
	 * @return	この <code>Array</code> オブジェクトによって指定された
	 *			配列の要素ごとに 1 行が格納されている
	 *			<code>java.sql.ResultSet</code> オブジェクト。
	 *			行はインデックスに基づく昇順で並べられている。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	public java.sql.ResultSet getResultSet() throws java.sql.SQLException
	{
		return new ArrayColumnResultSet(_array,
										0, _array.getCount(),
										_metaData);
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の要素を含む結果セットを
	 * 取得します。
	 * <P>
	 * 結果セットは配列要素ごとに 1 行を含み、各行には 2 つの列があります。
	 * 2 番目の列には要素の値が格納され、 1 番目の列には配列内の対応する
	 * 要素のインデックスが格納されます (最初の配列要素のインデックスは 1 )。
	 * 行は、インデックスに基づく昇順で並べられます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カスタムマッピングをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	map_
	 *			SQL ユーザ定義型の Java プログラミング言語のクラスへの
	 *			マッピングを格納する。
	 * @return	この <code>Array</code> オブジェクトによって指定された
	 *			配列の要素ごとに 1 行が格納されている
	 *			<code>java.sql.ResultSet</code> オブジェクト。
	 *			行はインデックスに基づく昇順で並べられている。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	public java.sql.ResultSet getResultSet(java.util.Map	map_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カスタムマッピングは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * インデックス <code>index_</code> で始まり、
	 * 最大で <code>count_</code> の連続した部分配列の要素が格納されている
	 * 結果セットを取得します。マップに基底型のエントリが格納されている場合、
	 * このメソッドは接続の型マップを使って配列の要素をマッピングします。
	 * そうでない場合は、標準のマップが使われます。
	 * <P>
	 * 結果セットは、このオブジェクトで指定された SQL 配列の要素ごとに
	 * 1 行を含み、最初の行にはインデックス <code>index_</code> にある要素が
	 * 格納されます。結果セットには、最大で <code>count_</code> の行が
	 * インデックスに基づく昇順で格納されます。各行には 2 つの列があります。
	 * 2 番目の列には要素の値が格納され、 1 番目の列には配列内の要素に対応する
	 * インデックスが格納されます。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 1 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	この <code>Array</code> オブジェクトによって指定された
	 *			SQL 配列の、インデックス <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続した要素が格納されている
	 *			<code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	public java.sql.ResultSet getResultSet(long	index_,
										   int	count_)
		throws java.sql.SQLException
	{
		int	numberOfElements = _array.getCount();
		if (index_ < 1 || index_ > numberOfElements) {
			// [YET!] SQLSTATE は、
			//        data exception - ???
			//        (22???)
			throw new BadArgument();
		}

		// 最初の要素のインデックスを 0 にかえる。
		int	index = (int)(index_ - 1);

		int	count = count_;
		if (count_ > numberOfElements - index) {
			count = numberOfElements - index;
		}

		return new ArrayColumnResultSet(_array,
										index, count,
										_metaData);
	}

	/**
	 * <B>[サポート外]</B>
	 * インデックス <code>index_</code> で始まり、
	 * 最大で <code>count_</code> の連続した要素が格納されている
	 * 部分配列の要素を保持する結果セットを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カスタムマッピングをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 1 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @param	map_
	 *			SQL 型名の Java プログラミング言語のクラスへのマップが
	 *			格納されている <code>java.util.Map</code> オブジェクト。
	 * @return	この <code>Array</code> オブジェクトによって指定された
	 *			SQL 配列の、インデックス <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続した要素が格納されている
	 *			<code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	public java.sql.ResultSet getResultSet(long				index_,
										   int				count_,
										   java.util.Map	map_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カスタムマッピングは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * 文字列表現を得る
	 */
	public String toString()
	{
		return _array.toString();
	}

	/**
	 * 要素オブジェクトを DoqueDB データに変換します。
	 *
	 * @param	element_
	 *			要素オブジェクト。
	 * @return	要素の値を格納した DoqueDB データオブジェクト。
	 */
	static Data convertToTRMeisterData(Object	element_)
	{
		Data	element = null;
		if (element_ == null) {

			element = new NullData();

		} else if (element_ instanceof Integer) {

			element = new IntegerData(((Integer)element_).intValue());

		} else if (element_ instanceof Long) {

			element = new Integer64Data(((Long)element_).longValue());

		} else if (element_ instanceof java.math.BigDecimal) {

			element = new DecimalData((java.math.BigDecimal)element_);

		} else if (element_ instanceof Double) {

			element = new DoubleData(((Double)element_).doubleValue());

		} else if (element_ instanceof Float) {

			element = new DoubleData(((Float)element_).doubleValue());

		} else if (element_ instanceof String) {

			element = new StringData((String)element_);

		} else if (element_ instanceof byte[]) {

			element = new BinaryData((byte[])element_);

		} else if (element_ instanceof java.sql.Date) {

			element = new DateData((java.sql.Date)element_);

		} else if (element_ instanceof java.sql.Timestamp) {

			element = new DateTimeData((java.sql.Timestamp)element_);

		} else if (element_ instanceof LanguageData) {

			element = new LanguageData((LanguageData)element_);

		} else if (element_ instanceof WordData) {

			element = new WordData((WordData)element_);
		}

		return element;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定されたものを
	 * <code>jp.co.ricoh.doquedb.common.DataArrayData</code> で取り出す
	 */
	DataArrayData getDataArrayData()
	{
		return _array;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を
	 * <code>java.lang.Integer</code> オブジェクトの配列として、取り出します。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 0 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている <code>java.lang.Integer</code>
	 *			オブジェクトの配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	private Integer[] getIntArray(int	index_,
								  int	count_)
		throws java.sql.SQLException
	{
		Integer[]	intArray = new Integer[count_];

		for (int i = 0; i < count_; i++) {
			Data	element = _array.getElement(index_ + i);
			if (element instanceof NullData) {
				intArray[i] = null;
			} else {
				intArray[i] = new Integer(((IntegerData)element).getValue());
			}
		}

		return intArray;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を
	 * <code>java.lang.Long</code> オブジェクトの配列として、取り出します。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 0 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている <code>java.lang.Long</code>
	 *			オブジェクトの配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	private Long[] getLongArray(int	index_,
								int	count_)
		throws java.sql.SQLException
	{
		Long[]	longArray = new Long[count_];

		for (int i = 0; i < count_; i++) {
			Data	element = _array.getElement(index_ + i);
			if (element instanceof NullData) {
				longArray[i] = null;
			} else {
				longArray[i] = new Long(((Integer64Data)element).getValue());
			}
		}

		return longArray;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を
	 * <code>java.math.BigDecimal</code> オブジェクトの配列として、取り出します。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 0 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている <code>java.lang.Long</code>
	 *			オブジェクトの配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	private java.math.BigDecimal[] getBigDecimalArray(int	index_,
													  int	count_)
		throws java.sql.SQLException
	{
		java.math.BigDecimal[]	decimalArray = new java.math.BigDecimal[count_];

		for (int i = 0; i < count_; i++) {
			Data	element = _array.getElement(index_ + i);
			if (element instanceof NullData) {
				decimalArray[i] = null;
			} else {
				decimalArray[i] = ((DecimalData)element).getValue().plus(); // copy
			}
		}

		return decimalArray;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を
	 * <code>java.lang.Double</code> オブジェクトの配列として、取り出します。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 0 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている <code>java.lang.Double</code>
	 *			オブジェクトの配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	private Double[] getDoubleArray(int	index_,
									int	count_)
	{
		Double[]	doubleArray = new Double[count_];

		for (int i = 0; i < count_; i++) {
			Data	element = _array.getElement(index_ + i);
			if (element instanceof NullData) {
				doubleArray[i] = null;
			} else {
				doubleArray[i] = new Double(((DoubleData)element).getValue());
			}
		}

		return doubleArray;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を
	 * <code>java.lang.String</code> オブジェクトの配列として、取り出します。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 0 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている <code>java.lang.String</code>
	 *			オブジェクトの配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	private String[] getStringArray(int	index_,
									int	count_)
	{
		String[]	stringArray = new String[count_];

		for (int i = 0; i < count_; i++) {
			Data	element = _array.getElement(index_ + i);
			if (element instanceof NullData) {
				stringArray[i] = null;
			} else {
				stringArray[i] = new String(((StringData)element).getValue());
			}
		}

		return stringArray;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を
	 * <code>byte[]</code> オブジェクトの配列として、取り出します。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 0 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている <code>byte[]</code>
	 *			オブジェクトの配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	private byte[][] getBytesArray(int	index_,
								   int	count_)
	{
		byte[][]	bytesArray = new byte[count_][];

		for (int i = 0; i < count_; i++) {
			Data	element = _array.getElement(index_ + i);
			if (element instanceof NullData) {
				bytesArray[i] = null;
			} else {
				bytesArray[i] = ((BinaryData)element).getValue();
			}
		}

		return bytesArray;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を
	 * <code>java.sql.Time</code> オブジェクトの配列として、取り出します。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 0 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている <code>java.sql.Time</code>
	 *			オブジェクトの配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	private java.sql.Date[] getDateArray(int	index_,
										 int	count_)
	{
		java.sql.Date[]	timeArray = new java.sql.Date[count_];

		for (int i = 0; i < count_; i++) {
			Data	element = _array.getElement(index_ + i);
			if (element instanceof NullData) {
				timeArray[i] = null;
			} else {
				java.sql.Date	date =
					((DateData)element).getValue();
				timeArray[i] = date;
			}
		}

		return timeArray;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を
	 * <code>java.sql.Time</code> オブジェクトの配列として、取り出します。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 0 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている <code>java.sql.Time</code>
	 *			オブジェクトの配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	private java.sql.Timestamp[] getDateTimeArray(int	index_,
												  int	count_)
	{
		java.sql.Timestamp[] timeArray = new java.sql.Timestamp[count_];

		for (int i = 0; i < count_; i++) {
			Data	element = _array.getElement(index_ + i);
			if (element instanceof NullData) {
				timeArray[i] = null;
			} else {
				java.sql.Timestamp	time =
					((DateTimeData)element).getValue();
				timeArray[i] = time;
			}
		}

		return timeArray;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を
	 * <code>jp.co.ricoh.doquedb.common.LanguageData</code> オブジェクトの
	 * 配列として、取り出します。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 0 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている
	 *			<code>jp.co.ricoh.doquedb.common.LanguageData</code>
	 *			オブジェクトの配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	private LanguageData[] getLangArray(int	index_,
										int	count_)
	{
		LanguageData[]	langArray = new LanguageData[count_];

		for (int i = 0; i < count_; i++) {
			Data	element = _array.getElement(index_ + i);
			if (element instanceof NullData) {
				langArray[i] = null;
			} else {
				langArray[i] = new LanguageData((LanguageData)element);
			}
		}

		return langArray;
	}

	/**
	 * この <code>Array</code> オブジェクトによって指定された
	 * SQL <code>java.sql.Types.ARRAY</code> 値の一部を
	 * <code>jp.co.ricoh.doquedb.common.WordData</code> オブジェクトの
	 * 配列として、取り出します。
	 *
	 * @param	index_
	 *			最初に取り出す要素の配列インデックス。
	 *			最初の要素のインデックスは 0 。
	 * @param	count_
	 *			取り出す連続する SQL 配列要素の数。
	 * @return	要素 <code>index_</code> で始まり、
	 *			最大で <code>count_</code> の連続する
	 *			SQL 配列要素が格納されている
	 *			<code>jp.co.ricoh.doquedb.common.WordData</code>
	 *			オブジェクトの配列。
	 * @throws	java.sql.SQLException
	 *			配列へのアクセス中にエラーが発生した場合。
	 */
	private WordData[] getWordArray(int	index_,
									int	count_)
	{
		WordData[]	wordArray = new WordData[count_];

		for (int i = 0; i < count_; i++) {
			Data	element = _array.getElement(index_ + i);
			if (element instanceof NullData) {
				wordArray[i] = null;
			} else {
				wordArray[i] = new WordData((WordData)element);
			}
		}

		return wordArray;
	}

	/** タイプを得る */
	private int getDataType()
	{
		int type;
		if (_metaData == null)
		{
			// メタデータがないので、データから
			type = _array.getElement(0).getType();
		}
		else
		{
			type = jp.co.ricoh.doquedb.common.ColumnMetaData.getDataType(
				_metaData.getType());
		}
		return type;
	}

	/**
	 * このメソッドは、<code>Array</code> オブジェクトを解放して、
	 * 保持されているリソースを解放します。
	 */
	@Override
	public void free() throws SQLException {
		// これ必要？
		_metaData = null;
		_array = null;
	}
}

//
// Copyright (c) 2003, 2004, 2005, 2007, 2009, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
