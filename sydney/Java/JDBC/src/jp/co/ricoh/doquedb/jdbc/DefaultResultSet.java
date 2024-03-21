// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DefaultResultSet.java -- 結果セットの基本クラス
//
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2015, 2016, 2023, 2024 Ricoh Company, Ltd.
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

import java.io.InputStream;
import java.io.Reader;
import java.sql.NClob;
import java.sql.RowId;
import java.sql.SQLException;
import java.sql.SQLXML;

import jp.co.ricoh.doquedb.common.BinaryData;
import jp.co.ricoh.doquedb.common.Data;
import jp.co.ricoh.doquedb.common.DataArrayData;
import jp.co.ricoh.doquedb.common.DataType;
import jp.co.ricoh.doquedb.common.DateData;
import jp.co.ricoh.doquedb.common.DateTimeData;
import jp.co.ricoh.doquedb.common.DecimalData;
import jp.co.ricoh.doquedb.common.DoubleData;
import jp.co.ricoh.doquedb.common.FloatData;
import jp.co.ricoh.doquedb.common.Integer64Data;
import jp.co.ricoh.doquedb.common.IntegerData;
import jp.co.ricoh.doquedb.common.LanguageData;
import jp.co.ricoh.doquedb.common.StringData;
import jp.co.ricoh.doquedb.common.WordData;
import jp.co.ricoh.doquedb.exception.BadArgument;
import jp.co.ricoh.doquedb.exception.ClassCast;
import jp.co.ricoh.doquedb.exception.NotSupported;
import jp.co.ricoh.doquedb.exception.SessionNotAvailable;
import jp.co.ricoh.doquedb.exception.Unexpected;

/**
 * 結果セットの基本クラス。
 *
 */
class DefaultResultSet implements java.sql.ResultSet
{
	/**
	 * 結果セットの現在の行データ。
	 *
	 * @see	jp.co.ricoh.doquedb.common.DataArrayData
	 */
	DataArrayData	_row;

	/** 現在の行の番号。最初の行が 1 、 2 番目は 2 。 */
	int	_rowNumber;

	/** 行を構成する列の数。 */
	int	_numberOfColumns;

	/** 最後に読み込まれた列の値が <code>null</code> かどうか */
	boolean	_wasNull;

	/** <code>getBinaryStream</code> で返される入力バイナリストリーム。 */
	private java.io.ByteArrayInputStream	_binaryStream;

	/** 警告オブジェクト。 */
	private java.sql.SQLWarning	_warnings;

	/** クローズしたかどうか。 */
	private boolean	_isClosed;

	/**
	 * 新しく行オブジェクトを作成します。
	 */
	DefaultResultSet()
	{
		this._row = null;
		this._rowNumber = 0;
		this._numberOfColumns = 0;
		this._wasNull = false;
		this._binaryStream = null;
		this._warnings = null;
		this._isClosed = false;
	}

	/**
	 * カーソルを現在の位置から 1 行下に移動します。
	 *
	 * @return	新しい現在の行が有効な場合は <code>true</code> 、
	 *			それ以上行がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean next() throws java.sql.SQLException
	{
		// next メソッドはサブクラスで実装する。
		throw new Unexpected();
	}

	/**
	 * 自動的にクローズされるとき、これを待つのではなく、
	 * ただちにこの <code>ResultSet</code> オブジェクトの
	 * データベースと JDBC リソースを解放します。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void close() throws java.sql.SQLException
	{
		this._row = null;
		this._rowNumber = 0;
		this.closeInputStream();
		this._isClosed = true;
	}

	/**
	 * 最後に読み込まれた列の値が SQL <code>NULL</code> であるかどうかを
	 * 通知します。
	 * 最初に列の getter メソッドの 1 つを呼び出してその値を読み込み、
	 * つぎにメソッド <code>wasNull</code> を呼び出して読み込まれた値が
	 * SQL <code>NULL</code> かどうかを判定する必要があります。
	 *
	 * @return	最後に読み込まれた列名が SQL <code>NULL</code> の場合は
	 *			<code>true</code> 、そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean wasNull() throws java.sql.SQLException
	{
		return this._wasNull;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.lang.String</code> として
	 * 取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合
	 */
	public String getString(int	columnIndex_) throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return null; // was null

		String	columnValue = null;

		switch(column.getType()) {
		case DataType.BINARY:
			// BinaryData.toString() では、
			// "size=XXX" という文字列が返るので、
			// getValue() メソッドにより byte[] オブジェクトを取得し、
			// それを String に変換する。

			// [YET!] しかし、String のコンストラクタでバイト配列を
			//        複合化するが、良いのだろうか…？
			//        素直に "size=XXX" という文字列で良いのだろうか…？

			byte[]	bytes = ((BinaryData)column).getValue();
			columnValue = new String(bytes, 0, bytes.length);
			break;
		case DataType.ARRAY:
			throw new ClassCast();
		default:
			// それ以外のデータ型では文字列にできるはず。
			columnValue = column.toString();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>boolean</code> として取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean getBoolean(int	columnIndex_) throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return false; // was null

		boolean	columnValue = false;

		switch (column.getType()) {
		case DataType.INTEGER:
			columnValue = (((IntegerData)column).getValue() != 0);
			break;
		case DataType.INTEGER64:
			columnValue = (((Integer64Data)column).getValue() != 0);
			break;
		case DataType.DECIMAL:
			columnValue = (((DecimalData)column).getValue().compareTo(java.math.BigDecimal.ZERO) != 0);
			break;
		case DataType.FLOAT:
			columnValue = (((FloatData)column).getValue() != 0);
			break;
		case DataType.DOUBLE:
			columnValue = (((DoubleData)column).getValue() != 0);
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue().trim();
				if (stringValue.length() == 0) {
					columnValue = false;
				} else if (stringValue.compareToIgnoreCase("true") == 0) {
					columnValue = true;
				} else if (stringValue.compareToIgnoreCase("false") == 0) {
					columnValue = false;
				} else if (stringValue.compareTo("0") == 0) {
					columnValue = false;
				} else {
					columnValue = true;
				}
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>byte</code> として取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public byte getByte(int	columnIndex_) throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return 0; // was null

		byte	columnValue = 0;

		switch (column.getType()) {
		case DataType.INTEGER:
			columnValue = (byte)((IntegerData)column).getValue();
			break;
		case DataType.INTEGER64:
			columnValue = (byte)((Integer64Data)column).getValue();
			break;
		case DataType.DECIMAL:
			columnValue = (byte)((DecimalData)column).getValue().intValue();
			break;
		case DataType.FLOAT:
			columnValue = (byte)((FloatData)column).getValue();
			break;
		case DataType.DOUBLE:
			columnValue = (byte)((DoubleData)column).getValue();
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue();
				try {
					columnValue = Byte.parseByte(stringValue);
				} catch (java.lang.NumberFormatException	nfe) {
					ClassCast	cce = new ClassCast();
					cce.initCause(nfe);
					throw cce;
				}
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>short</code> として取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public short getShort(int	columnIndex_) throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return 0; // was null

		short	columnValue = 0;

		switch (column.getType()) {
		case DataType.INTEGER:
			columnValue = (short)((IntegerData)column).getValue();
			break;
		case DataType.INTEGER64:
			columnValue = (short)((Integer64Data)column).getValue();
			break;
		case DataType.DECIMAL:
			columnValue = (short)((DecimalData)column).getValue().intValue();
			break;
		case DataType.FLOAT:
			columnValue = (short)((FloatData)column).getValue();
			break;
		case DataType.DOUBLE:
			columnValue = (short)((DoubleData)column).getValue();
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue();
				try {
					columnValue = Short.parseShort(stringValue);
				} catch (java.lang.NumberFormatException	nfe) {
					ClassCast	cce = new ClassCast();
					cce.initCause(nfe);
					throw cce;
				}
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>int</code> として取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getInt(int	columnIndex_) throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return 0; // was null

		int	columnValue = 0;

		switch (column.getType()) {
		case DataType.INTEGER:
			columnValue = ((IntegerData)column).getValue();
			break;
		case DataType.INTEGER64:
			columnValue = (int)((Integer64Data)column).getValue();
			break;
		case DataType.DECIMAL:
			columnValue = (int)((DecimalData)column).getValue().intValue();
			break;
		case DataType.FLOAT:
			columnValue = (int)((FloatData)column).getValue();
			break;
		case DataType.DOUBLE:
			columnValue = (int)((DoubleData)column).getValue();
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue();
				try {
					columnValue = Integer.parseInt(stringValue);
				} catch (java.lang.NumberFormatException	nfe) {
					ClassCast	cce = new ClassCast();
					cce.initCause(nfe);
					throw cce;
				}
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>long</code> として取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public long getLong(int	columnIndex_) throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return 0; // was null

		long	columnValue = 0;

		switch (column.getType()) {
		case DataType.INTEGER:
			columnValue = (long)((IntegerData)column).getValue();
			break;
		case DataType.INTEGER64:
			columnValue = ((Integer64Data)column).getValue();
			break;
		case DataType.DECIMAL:
			columnValue = ((DecimalData)column).getValue().longValue();
			break;
		case DataType.FLOAT:
			columnValue = (long)((FloatData)column).getValue();
			break;
		case DataType.DOUBLE:
			columnValue = (long)((DoubleData)column).getValue();
			break;
		case DataType.STRING:
			try {
				columnValue =
					Long.parseLong(((StringData)column).getValue());
			} catch (java.lang.NumberFormatException	nfe) {
				ClassCast	cce = new ClassCast();
				cce.initCause(nfe);
				throw cce;
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>float</code> として取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public float getFloat(int	columnIndex_) throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return 0; // was null;

		float	columnValue = 0;

		switch (column.getType()) {
		case DataType.INTEGER:
			columnValue = (float)((IntegerData)column).getValue();
			break;
		case DataType.INTEGER64:
			columnValue = (float)((Integer64Data)column).getValue();
			break;
		case DataType.DECIMAL:
			columnValue = (float)((DecimalData)column).getValue().floatValue();
			break;
		case DataType.FLOAT:
			columnValue = ((FloatData)column).getValue();
			break;
		case DataType.DOUBLE:
			columnValue = (float)((DoubleData)column).getValue();
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue();
				try {
					columnValue = Float.parseFloat(stringValue);
				} catch (java.lang.NumberFormatException	nfe) {
					ClassCast	cce = new ClassCast();
					cce.initCause(nfe);
					throw cce;
				}
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>double</code> として取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public double getDouble(int	columnIndex_) throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return 0; // was null

		double	columnValue = 0;

		switch (column.getType()) {
		case DataType.INTEGER:
			columnValue = (double)((IntegerData)column).getValue();
			break;
		case DataType.INTEGER64:
			columnValue = (double)((Integer64Data)column).getValue();
			break;
		case DataType.DECIMAL:
			columnValue = (double)((DecimalData)column).getValue().doubleValue();
			break;
		case DataType.FLOAT:
			columnValue = (double)((FloatData)column).getValue();
			break;
		case DataType.DOUBLE:
			columnValue = ((DoubleData)column).getValue();
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue();
				try {
					columnValue = Double.parseDouble(stringValue);
				} catch (java.lang.NumberFormatException	nfe) {
					ClassCast	cce = new ClassCast();
					cce.initCause(nfe);
					throw cce;
				}
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.BigDecimal</code> として
	 * 取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	scale_
	 *			小数点以下の桁数
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @deprecated
	 */
	public java.math.BigDecimal getBigDecimal(int	columnIndex_,
											  int	scale_)
		throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_, scale_);
		if (column == null) return null; // was null

		int	roundingMode = java.math.BigDecimal.ROUND_HALF_EVEN;

		java.math.BigInteger	bigInt = null;
		java.math.BigDecimal	columnValue = null;

		switch (column.getType()) {
		case DataType.INTEGER:
			{
				long	longValue = (long)((IntegerData)column).getValue();
				bigInt = java.math.BigInteger.valueOf(longValue);
				columnValue = new java.math.BigDecimal(bigInt);
				columnValue = columnValue.setScale(scale_, roundingMode);
			}
			break;
		case DataType.INTEGER64:
			{
				long	longValue = ((Integer64Data)column).getValue();
				bigInt = java.math.BigInteger.valueOf(longValue);
				columnValue = new java.math.BigDecimal(bigInt);
				columnValue = columnValue.setScale(scale_, roundingMode);
			}
			break;
		case DataType.DECIMAL:
			{
				columnValue = ((DecimalData)column).getValue().setScale(scale_, roundingMode);
			}
			break;
		case DataType.FLOAT:
			{
				String	floatValue = ((FloatData)column).toString();
				columnValue = new java.math.BigDecimal(floatValue);
				columnValue = columnValue.setScale(scale_, roundingMode);
			}
			break;
		case DataType.DOUBLE:
			{
				String	doubleValue = ((DoubleData)column).toString();
				columnValue = new java.math.BigDecimal(doubleValue);
				columnValue = columnValue.setScale(scale_, roundingMode);
			}
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue();
				try {
					columnValue = new java.math.BigDecimal(stringValue);
				} catch (java.lang.NumberFormatException	nfe) {
					ClassCast	cce = new ClassCast();
					cce.initCause(nfe);
					throw cce;
				}
				columnValue = columnValue.setScale(scale_, roundingMode);
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>byte</code> 配列として取得します。
	 * バイトはドライバによって返された行の値を表します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public byte[] getBytes(int	columnIndex_) throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return null; // was null

		byte[]	columnValue = null;

		switch (column.getType()) {
		case DataType.BINARY:
			columnValue = ((BinaryData)column).getValue();
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Date</code> オブジェクトとして
	 * 取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Date getDate(int	columnIndex_)
		throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return null; // was null

		java.sql.Date	columnValue = null;

		switch (column.getType()) {
		case DataType.DATE:
			columnValue = ((DateData)column).getValue();
			break;
		case DataType.DATE_TIME:
			{
				java.sql.Timestamp timeStamp
					= ((DateTimeData)column).getValue();
				columnValue = new java.sql.Date(timeStamp.getTime());
			}
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue();
				try {
					columnValue = java.sql.Date.valueOf(stringValue);
				} catch (java.lang.NumberFormatException	nfe) {
					ClassCast	cce = new ClassCast();
					cce.initCause(nfe);
					throw cce;
				} catch (java.lang.IllegalArgumentException	iae) {
					ClassCast	cce = new ClassCast();
					cce.initCause(iae);
					throw cce;
				}
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Time</code> オブジェクトとして
	 * 取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Time getTime(int	columnIndex_)
		throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return null; // was null

		java.sql.Time	columnValue = null;

		switch (column.getType()) {
		case DataType.DATE_TIME:
			{
				long	timeStamp = ((DateTimeData)column).getValue().getTime();
				columnValue = new java.sql.Time(timeStamp);
			}
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue();
				try {
					columnValue = java.sql.Time.valueOf(stringValue);
				} catch (java.lang.NumberFormatException	nfe) {
					ClassCast	cce = new ClassCast();
					cce.initCause(nfe);
					throw cce;
				} catch (java.lang.IllegalArgumentException	iae) {
					ClassCast	cce = new ClassCast();
					cce.initCause(iae);
					throw cce;
				}
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の
	 * <code>java.sql.Timestamp</code> オブジェクトとして取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Timestamp getTimestamp(int	columnIndex_)
		throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return null; // was null

		java.sql.Timestamp	columnValue = null;

		switch (column.getType()) {
		case DataType.DATE:
			{
				long	timeStamp = ((DateData)column).getValue().getTime();
				columnValue = new java.sql.Timestamp(timeStamp);
			}
			break;
		case DataType.DATE_TIME:
			columnValue = ((DateTimeData)column).getValue();
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue();
				try {
					columnValue = java.sql.Timestamp.valueOf(stringValue);
				} catch (java.lang.NumberFormatException	nfe) {
					ClassCast	cce = new ClassCast();
					cce.initCause(nfe);
					throw cce;
				} catch (java.lang.IllegalArgumentException	iae) {
					ClassCast	cce = new ClassCast();
					cce.initCause(iae);
					throw cce;
				}
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * ASCII 文字のストリームとして取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	データベースの列値を 1 バイト ASCII 文字のストリームとして
	 *			送る Java 入力ストリーム。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.io.InputStream getAsciiStream(int	columnIndex_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] getCharacterStream メソッドのみサポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * 2 バイト Unicode 文字のストリームとして取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	データベースの列値を 2 バイト Unicode 文字のストリームとして
	 *			送る Java 入力ストリーム。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @deprecated
	 *			<code>getUnicodeStream</code> の代わりに
	 *			<code>getCharacterStream</code> を使用。
	 * @see	#getCharacterStream(int)
	 */
	public java.io.InputStream getUnicodeStream(int	columnIndex_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] getCharacterStream メソッドのみサポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * 未解釈のバイトのバイナリストリームとして取得します。
	 * そのストリームから一括して値を読み込めるようになります。
	 * このメソッドは、特に大きな <code>LONGVARBINARY</code> 値を取り出すのに
	 * 適しています。
	 * <P>
	 * <B>注:</B>
	 * 返されたストリーム中のデータはすべて、
	 * ほかの列の値を取得する前に読み込む必要があります。
	 * つぎの getter メソッドへの呼び出しは、暗黙的にストリームを
	 * クローズします。また、 <code>InputStream.available</code> メソッドが
	 * 呼び出されたときに、使用可能なデータがあるかないかに関係なく、
	 * ストリームは <code>0</code> を返すことがあります。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	データベースの列値を未解釈のバイトストリームとして送る
	 *			Java 入力ストリーム。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.io.InputStream getBinaryStream(int	columnIndex_)
		throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return null; // was null

		switch (column.getType()) {
		case DataType.BINARY:
			byte[]	binaryValue = ((BinaryData)column).getValue();
			this._binaryStream = new java.io.ByteArrayInputStream(binaryValue);
			break;
		default:
			throw new ClassCast();
		}

		return this._binaryStream;
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.lang.String</code> として
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getString(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getString(String	columnName_) throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>boolean</code> として取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getBoolean(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean getBoolean(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>byte</code> として取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getByte(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public byte getByte(String	columnName_) throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>short</code> として取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getShort(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public short getShort(String	columnName_) throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>int</code> として取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getInt(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getInt(String	columnName_) throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>long</code> として取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getLong(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public long getLong(String	columnName_) throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>float</code> として取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getFloat(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public float getFloat(String	columnName_) throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>double</code> として取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getDouble(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public double getDouble(String	columnName_) throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.math.BigDecimal</code> として
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getBigDecimal(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @deprecated
	 */
	public java.math.BigDecimal getBigDecimal(String	columnName_,
											  int		scale_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>byte</code> 配列として取得します。
	 * バイトはドライバによって返された行の値を表します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getBytes(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public byte[] getBytes(String	columnName_) throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Date</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getDate(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Date getDate(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Time</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getTime(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Time getTime(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の
	 * <code>java.sql.Timestamp</code> オブジェクトとして取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getTimestamp(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Timestamp getTimestamp(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * ASCII 文字のストリームとして取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	データベースの列値を 1 バイト ASCII 文字のストリームとして
	 *			送る Java 入力ストリーム。
	 *			値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.io.InputStream getAsciiStream(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * 2 バイト Unicode 文字のストリームとして取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	データベースの列値を 2 バイト Unicode 文字のストリームとして
	 *			送る Java 入力ストリーム。
	 *			値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @deprecated
	 *			<code>getUnicodeStream</code> の代わりに
	 *			<code>getCharacterStream</code> を使用。
	 * @see		#getCharacterStream(java.lang.String)
	 */
	public java.io.InputStream getUnicodeStream(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、未解釈のバイトストリームとして
	 * 取得します。
	 * そのストリームから一括して値を読み込めるようになります。
	 * このメソッドは、特に大きな <code>LONGVARBINARY</code> 値を取り出すのに
	 * 適しています。
	 * <P>
	 * <B>注:</B>
	 * 返されたストリーム中のデータはすべて、
	 * ほかの列の値を取得する前に読み込む必要があります。
	 * つぎの getter メソッドへの呼び出しは、暗黙的にストリームを
	 * クローズします。また、 <code>InputStream.available</code> メソッドが
	 * 呼び出されたときに、使用可能なデータがあるかないかに関係なく、
	 * ストリームは <code>0</code> を返すことがあります。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getBinaryStream(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	データベースの列値を未解釈のバイトストリームとして送る
	 *			Java 入力ストリーム。値が SQL <code>NULL</code> の場合、
	 *			返される値は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.io.InputStream getBinaryStream(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * この <code>ResultSet</code> オブジェクトに関する
	 * 呼び出しによって報告される最初の警告を返します。
	 * 後続の <code>ResultSet</code> オブジェクトの警告は、
	 * このメソッドが返す <code>java.sql.SQLWarning</code> オブジェクトに
	 * チェーンされます。
	 * <P>
	 * 警告チェーンは、新しい行が読み込まれるたびに自動的にクリアされます。
	 * このメソッドはクローズされた <code>ResultSet</code>
	 * オブジェクトの呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * この警告チェーンは、<code>ResultSet</code> メソッドが原因となった
	 * 警告だけを対象とします。
	 * <code>Statement</code> メソッド (読み込んでいる OUT パラメータなど) が
	 * 原因となった警告は <code>Statement</code> オブジェクトに
	 * チェーンされます。
	 *
	 * @return	報告された最初の
	 *			<code>java.sql.SQLWarning</code> オブジェクト。
	 *			ない場合は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはこのメソッドがクローズされた結果セットで
	 *			呼び出された場合。
	 */
	public java.sql.SQLWarning getWarnings() throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		return this._warnings;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトに関して報告された
	 * すべての警告をクリアします。このメソッドが呼び出されたあと、
	 * この <code>ResultSet</code> オブジェクトに対する新しい警告が
	 * 報告されるまで、 <code>getWarnings</code> メソッドは
	 * <code>null</code> を返します。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void clearWarnings() throws java.sql.SQLException
	{
		this._warnings = null;
	}

	/**
	 * <B>[制限あり]</B>
	 * この <code>ResultSet</code> オブジェクトが使用する
	 * SQL カーソルの名前を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、位置決めされた更新文をサポートしてないため、
	 * 常に <code>null</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>null</code>
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getCursorName() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルはサポートしていない。
		return null;
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 列の数、型、およびプロパティを取得します。
	 *
	 * @return	この <code>ResultSet</code> オブジェクトの列の記述。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSetMetaData getMetaData()
		throws java.sql.SQLException
	{
		// getMetaData メソッドが必要ならばサブクラスで実装する。
		return null;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>Object</code> として取得します。
	 * <P>
	 * このメソッドは、指定された列の値を Java オブジェクトとして返します。
	 * Java オブジェクトの型は、 JDBC 仕様で指定されている組み込み型の
	 * マッピングに従って、列の SQL 型に対応するデフォルトの
	 * Java オブジェクトの型になります。値が SQL <code>NULL</code> の場合、
	 * ドライバは Java <code>null</code> を返します。
	 * <P>
	 * このメソッドは、データベース固有の抽象データ型を読み込む目的にも
	 * 使用できます。JDBC 2.0 API では、 <code>getObject</code> メソッドの
	 * 動作は SQL ユーザ定義型のデータを生成するように拡張されます。
	 * 列が構造化型または個別の型の値である場合、このメソッドの動作は、
	 * <code>
	 * getObject(columnIndex, this.getStatement().getConnection().getTypeMap())
	 * </code>
	 * を呼び出した場合と同じになります。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値を保持している <code>java.lang.Object</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public Object getObject(int	columnIndex_) throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return null; // was null

		Object columnValue = null;

		switch (column.getType()) {
		case DataType.INTEGER:
			columnValue = new Integer(getInt(columnIndex_));
			break;
		case DataType.INTEGER64:
			columnValue = new Long(getLong(columnIndex_));
			break;
		case DataType.DECIMAL:
			columnValue = getBigDecimal(columnIndex_);
			break;
		case DataType.FLOAT:
			columnValue = new Float(getFloat(columnIndex_));
			break;
		case DataType.DOUBLE:
			columnValue = new Double(getDouble(columnIndex_));
			break;
		case DataType.STRING:
			columnValue = getString(columnIndex_);
			break;
		case DataType.DATE:
			columnValue = getDate(columnIndex_);
			break;
		case DataType.DATE_TIME:
			columnValue = getTimestamp(columnIndex_);
			break;
		case DataType.BINARY:
			columnValue = getBytes(columnIndex_);
			break;
		case DataType.LANGUAGE:
			columnValue = new LanguageData((LanguageData)column);
			break;
		case DataType.WORD:
			columnValue = new WordData((WordData)column);
			break;
		case DataType.ARRAY:
			columnValue = getArray(columnIndex_);
			break;
		default:
			// [YET!] SQLSTATE は、
			//        feature not supported - data type is not supported
			//        (0A503) ※ 仮
			throw new NotSupported();
		}

		return columnValue;
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>Object</code> として取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getObject(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値を保持している <code>java.lang.Object</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public Object getObject(String	columnName_) throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された <code>ResultSet</code> の列名を
	 * <code>ResultSet</code> 列インデックスにマッピングします。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>findColumn(String)</code> メソッドは、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	指定された列名の列インデックス。
	 * @throws	java.sql.SQLException
	 *			<code>ResultSet</code> オブジェクトに
	 *			<code>columnName</code> が含まれない場合、
	 *			またはデータベースアクセスエラーが発生した場合。
	 */
	public int findColumn(String	columnName_) throws java.sql.SQLException
	{
		// サブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.io.Reader</code> オブジェクトとして
	 * 取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列値を格納する <code>java.io.Reader</code> オブジェクト。
	 *			値が SQL <code>NULL</code> の場合、
	 *			返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.io.Reader getCharacterStream(int	columnIndex_)
		throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return null; // was null

		java.io.Reader	charStream = null;

		switch (column.getType()) {
		case DataType.STRING:
			charStream =
				new java.io.StringReader(((StringData)column).getValue());
			break;
		case DataType.BINARY:
			java.io.InputStream	binaryStream = getBinaryStream(columnIndex_);
			charStream = new java.io.InputStreamReader(binaryStream);
			break;
		default:
			throw new ClassCast();
		}

		return charStream;

	} // end of method getCharacterStream

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.io.Reader</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getCharacterStream(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値を格納する <code>java.io.Reader</code> オブジェクト。
	 *			値が SQL <code>NULL</code> の場合、
	 *			返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.io.Reader getCharacterStream(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の
	 * <code>java.math.BigDecimal</code> オブジェクトとして全精度で取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	全精度の列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.math.BigDecimal getBigDecimal(int	columnIndex_)
		throws java.sql.SQLException
	{
		Data	column = getColumn(columnIndex_);
		if (column == null) return null; // was null

		java.math.BigInteger	bigInt = null;
		java.math.BigDecimal	columnValue = null;

		switch (column.getType()) {
		case DataType.INTEGER:
			{
				long	longValue = (long)((IntegerData)column).getValue();
				bigInt = java.math.BigInteger.valueOf(longValue);
				columnValue = new java.math.BigDecimal(bigInt);
			}
			break;
		case DataType.INTEGER64:
			{
				long	longValue = ((Integer64Data)column).getValue();
				bigInt = java.math.BigInteger.valueOf(longValue);
				columnValue = new java.math.BigDecimal(bigInt);
			}
			break;
		case DataType.DECIMAL:
			{
				columnValue = ((DecimalData)column).getValue();
			}
			break;
		case DataType.FLOAT:
			{
				String	floatValue = ((FloatData)column).toString();
				columnValue = new java.math.BigDecimal(floatValue);
			}
			break;
		case DataType.DOUBLE:
			{
				String	doubleValue = ((DoubleData)column).toString();
				columnValue = new java.math.BigDecimal(doubleValue);
			}
			break;
		case DataType.STRING:
			{
				String	stringValue = ((StringData)column).getValue();
				try {
					columnValue = new java.math.BigDecimal(stringValue);
				} catch (java.lang.NumberFormatException	nfe) {
					ClassCast	cce = new ClassCast();
					cce.initCause(nfe);
					throw cce;
				}
			}
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の
	 * <code>java.math.BigDecimal</code> オブジェクトとして全精度で取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getBigDecimal(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	全精度の列値。値が SQL <code>NULL</code> の場合、
	 *			返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.math.BigDecimal getBigDecimal(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * カーソルがこの <code>ResultSet</code> オブジェクト内の
	 * 先頭行より前にあるかどうかを取得します。
	 *
	 * @return	カーソルが先頭行より前にある場合は <code>true</code> 、
	 *			カーソルが他の位置にあるか、
	 *			結果セットに行がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isBeforeFirst() throws java.sql.SQLException
	{
		return this._rowNumber == 0;
	}

	/**
	 * カーソルがこの <code>ResultSet</code> オブジェクト内の
	 * 最終行より後ろにあるかどうかを取得します。
	 *
	 * @return	カーソルが最終行より後ろにある場合は <code>true</code> 、
	 *			カーソルが他の位置にあるか、
	 *			結果セットに行がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isAfterLast() throws java.sql.SQLException
	{
		return this._rowNumber == -1;
	}

	/**
	 * カーソルがこの <code>ResultSet</code> オブジェクト内の
	 * 先頭行にあるかどうかを取得します。
	 *
	 * @return	カーソルが先頭行にある場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isFirst() throws java.sql.SQLException
	{
		return this._rowNumber == 1;
	}

	/**
	 * <B>[サポート外]</B>
	 * カーソルがこの <code>ResultSet</code> オブジェクトの
	 * 最終行にあるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * JDBC ドライバは現在の行が結果セット内の
	 * 最終行であるかどうかを判定するために 1 つ先の行を取り出すことが
	 * 必要な場合があるので、 <code>isLast</code> メソッドの呼び出しは
	 * 負荷が大きくなる場合があります。
	 *
	 * @return	カーソルが最終行にある場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isLast() throws java.sql.SQLException
	{
		// サブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * カーソルをこの <code>ResultSet</code> オブジェクトの先端、
	 * つまり先頭行の直前に移動します。
	 * 結果セットに行がない場合、このメソッドは無効です。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしておらず、
	 * また、サポートしている結果セットの型が
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみのため、
	 * このメソッドはサポートしていません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または結果セットの型が
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> の場合。
	 */
	public void beforeFirst() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * カーソルをこの <code>ResultSet</code> オブジェクトの終端、
	 * つまり最終行の直後に移動します。
	 * 結果セットに行がない場合、このメソッドは無効です。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしておらず、
	 * また、サポートしている結果セットの型が
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみのため、
	 * このメソッドはサポートしていません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または結果セットの型が
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> の場合。
	 */
	public void afterLast() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * カーソルをこの <code>ResultSet</code> オブジェクト内の
	 * 先頭行に移動します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしておらず、
	 * また、サポートしている結果セットの型が
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみのため、
	 * このメソッドはサポートしていません。
	 *
	 * @return	カーソルが有効な行にある場合は <code>true</code> 、
	 *			結果セットに行がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または結果セットの型が
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> の場合。
	 */
	public boolean first() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * カーソルをこの <code>ResultSet</code> オブジェクト内の
	 * 最終行に移動します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしておらず、
	 * また、サポートしている結果セットの型が
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみのため、
	 * このメソッドはサポートしていません。
	 *
	 * @return	カーソルが有効な行にある場合は <code>true</code> 、
	 *			結果セットに行がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または結果セットの型が
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> の場合。
	 */
	public boolean last() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * 現在の行の番号を取得します。
	 * 最初の行が 1 、 2 番目は 2 、などとなります。
	 *
	 * @return	現在の行の番号。現在の行がない場合は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getRow() throws java.sql.SQLException
	{
		int	rowNumber = 0;
		if (this._rowNumber != -1) rowNumber = this._rowNumber;
		return rowNumber;
	}

	/**
	 * <B>[サポート外]</B>
	 * カーソルをこの <code>ResultSet</code> オブジェクト内の
	 * 指定された行に移動します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしておらず、
	 * また、サポートしている結果セットの型が
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみのため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	row_
	 *			カーソルの移動先の行番号。
	 *			正の番号は行番号が結果セットの先頭から
	 *			カウントされることを示し、
	 *			負の番号は終端からカウントされることを示す。
	 * @return	カーソルが結果セットにある場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または結果セットの型が
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> の場合。
	 */
	public boolean absolute(int	row_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * カーソルを正または負の相対行数だけ移動します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしておらず、
	 * また、サポートしている結果セットの型が
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみのため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	rows_
	 *			現在の行から移動する行数を指定する <code>int</code> 。
	 *			正の数はカーソルを順方向に移動し、
	 *			負の数は逆方向に移動する。
	 * @return	カーソルが行にある場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			現在の行がない場合、または結果セットの型が
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> の場合。
	 */
	public boolean relative(int	rows_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * カーソルをこの <code>ResultSet</code> オブジェクト内の
	 * 前の行に移動します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしておらず、
	 * また、サポートしている結果セットの型が
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみのため、
	 * このメソッドはサポートしていません。
	 *
	 * @return	カーソルが有効な行にある場合は <code>true</code> 、
	 *			結果セットの外にある場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または結果セットの型が
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> の場合。
	 */
	public boolean previous() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[制限あり]</B>
	 * この <code>ResultSet</code> オブジェクト内の
	 * 行が処理される方向についてのヒントを提供します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしているフェッチ方向は
	 * <code>java.sql.ResultSet.FETCH_FORWARD</code> のみです。
	 *
	 * @param	direction_
	 *			指定されたフェッチ方向を指定する <code>int</code> 。
	 *			現在のバージョンでは、サポートしているフェッチ方向は
	 *			<code>java.sql.ResultSet.FETCH_FORWARD</code> のみです。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または結果セットの型が
	 *			<code>java.sql.Resultset.TYPE_FORWARD_ONLY</code> で
	 *			フェッチ方向が
	 *			<code>java.sql.ResultSet.FETCH_FORWARD</code> でない場合。
	 * @see		Statement#setFetchDirection(int)
	 * @see		#getFetchDirection()
	 */
	public void setFetchDirection(int	direction_)
		throws java.sql.SQLException
	{
		if (direction_ != java.sql.ResultSet.FETCH_FORWARD &&
			direction_ != java.sql.ResultSet.FETCH_REVERSE &&
			direction_ != java.sql.ResultSet.FETCH_UNKNOWN) {

			// [YET!] SQLSTATE は、
			//        data exception - ???
			//        (22???)
			throw new BadArgument();
		}

		if (direction_ != java.sql.ResultSet.FETCH_FORWARD) {

			// [NOT SUPPORTED!] 方向は
			//                  java.sql.ResultSet.FETCH_FORWARD
			//                  のみをサポート。
			String	reason =
				"direction of ResultSet currently supported is only " +
				"java.sql.ResultSet.FETCH_FORWARD.";
			// [YET!] SQLSTATE のサブクラスがまだ未設定。"000" は仮。
			appendWarning(reason, "000");
		}
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * フェッチ方向を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしているフェッチ方向は
	 * <code>java.sql.ResultSet.FETCH_FORWARD</code> のみなので、
	 * 常に <code>java.sql.ResultSet.FETCH_FORWARD</code> が返されます。
	 *
	 * @return	この <code>ResultSet</code> オブジェクトの
	 *			現在のフェッチ方向。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#setFetchDirection(int)
	 */
	public int getFetchDirection() throws java.sql.SQLException
	{
		return java.sql.ResultSet.FETCH_FORWARD;
	}

	/**
	 * <B>[制限あり]</B>
	 * この <code>ResultSet</code> オブジェクトで
	 * より多くの行が必要なときに
	 * データベースから取り出す必要がある行数についてのヒントを
	 * JDBC ドライバに提供します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 <code>0</code> 以外はサポートしていません。
	 *
	 * @param	rows_
	 *			フェッチする行数。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または条件
	 *			<code>0 &lt;= rows &lt;= this.getMaxRows()</code>
	 *			が満たされない場合。
	 * @see		#getFetchSize()
	 */
	public void setFetchSize(int	rows_) throws java.sql.SQLException
	{
		if (rows_ != 0) {

			// [NOT SUPPORTED!] 列数は 0 のみサポート。
			String	reason =
				"hint about the number of rows which needs to be taken out " +
				"from a database is not supported other than zero.";
			// // [YET!] SQLSTATE がまだ未設定。"000" は仮。
			appendWarning(reason, "000");
		}
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * フェッチサイズを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 <code>0</code> 以外はサポートしないため、
	 * 常に <code>0</code> を返します。
	 *
	 * @return	この <code>ResultSet</code> オブジェクトの
	 *			現在のフェッチサイズ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getFetchSize() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの型を返します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしている結果セットの型が
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみなので、常に
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> を返します。
	 *
	 * @return	結果セットの型。
	 *			現在のバージョンでは、常に
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getType() throws java.sql.SQLException
	{
		return java.sql.ResultSet.TYPE_FORWARD_ONLY;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの
	 * 並行処理モードを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしている並行処理モードが
	 * <code>java.sql.ResultSet.CONCUR_READ_ONLY</code> のみなので、常に
	 * <code>java.sql.ResultSet.CONCUR_READ_ONLY</code> を返します。
	 *
	 * @return	並行処理の種類。
	 *			現在のバージョンでは、常に
	 *			<code>java.sql.ResultSet.CONCUR_READ_ONLY</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getConcurrency() throws java.sql.SQLException
	{
		return java.sql.ResultSet.CONCUR_READ_ONLY;
	}

	/**
	 * 現在の行が更新されているかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、可視の行が更新されたことを検出できないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		DatabaseMetaData#updatesAreDetected(int)
	 */
	public boolean rowUpdated() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * 現在の行に挿入があったかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、可視の行が挿入されたことを検出できないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		DatabaseMetaData#insertsAreDetected(int)
	 */
	public boolean rowInserted() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * 行が削除されているかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、可視の行が削除されたことを検出できないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		DatabaseMetaData#deletesAreDetected(int)
	 */
	public boolean rowDeleted()
		throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * <B>[サポート外]</B>
	 * <code>null</code> を許す列に <code>null</code> 値を設定します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateNull(int	columnIndex_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>boolean</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateBoolean(int		columnIndex_,
							  boolean	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>byte</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateByte(int	columnIndex_,
						   byte	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>short</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateShort(int		columnIndex_,
							short	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>int</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateInt(int	columnIndex_,
						  int	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>long</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateLong(int	columnIndex_,
						   long	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>float</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateFloat(int		columnIndex_,
							float	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>double</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateDouble(int	columnIndex_,
							 double	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.math.BigDecimal</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateBigDecimal(int					columnIndex_,
								 java.math.BigDecimal	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.lang.String</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateString(int	columnIndex_,
							 String	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>byte</code> 配列値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateBytes(int		columnIndex_,
							byte[]	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Date</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateDate(int				columnIndex_,
						   java.sql.Date	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Time</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateTime(int				columnIndex_,
						   java.sql.Time	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Timestamp</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateTimestamp(int					columnIndex_,
								java.sql.Timestamp	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を ASCII ストリーム値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @param	length_
	 *			ストリームの長さ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateAsciiStream(int					columnIndex_,
								  java.io.InputStream	x_,
								  int					length_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列をバイナリストリーム値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @param	length_
	 *			ストリームの長さ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateBinaryStream(int					columnIndex_,
								   java.io.InputStream	x_,
								   int					length_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を文字ストリーム値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @param	length_
	 *			ストリームの長さ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateCharacterStream(int				columnIndex_,
									  java.io.Reader	x_,
									  int				length_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>Object</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @param	scale_
	 *			<code>java.sql.Types.DECIMAL</code> や
	 *			<code>java.sql.Types.NUMERIC</code> 型では、
	 *			小数点以下の桁数。
	 *			ほかのすべての型では、この値は無視される。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateObject(int	columnIndex_,
							 Object	x_,
							 int	scale_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>Object</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateObject(int	columnIndex_,
							 Object	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>null</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateNull(String	columnName_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>boolean</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateBoolean(String	columnName_,
							  boolean	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>byte</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateByte(String	columnName_,
						   byte		x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>short</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateShort(String	columnName_,
							short	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>int</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateInt(String	columnName_,
						  int		x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>long</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateLong(String	columnName_,
						   long		x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>float</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateFloat(String	columnName_,
							float	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>double</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateDouble(String	columnName_,
							 double	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.BigDecimal</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateBigDecimal(String					columnName_,
								 java.math.BigDecimal	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.lang.String</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateString(String	columnName_,
							 String	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列をバイト配列値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateBytes(String	columnName_,
							byte[]	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Date</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateDate(String			columnName_,
						   java.sql.Date	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Time</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateTime(String			columnName_,
						   java.sql.Time	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Timestamp</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateTimestamp(String				columnName_,
								java.sql.Timestamp	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を ASCII ストリーム値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @param	length_
	 *			ストリームの長さ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateAsciiStream(String				columnName_,
								  java.io.InputStream	x_,
								  int					length_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列をバイナリストリーム値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @param	length_
	 *			ストリームの長さ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateBinaryStream(String				columnName_,
								   java.io.InputStream	x_,
								   int					length_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を文字ストリーム値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	reader_
	 *			新しい列値を格納する
	 *			<code>java.io.Reader</code> オブジェクト。
	 * @param	length_
	 *			ストリームの長さ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateCharacterStream(String			columnName_,
									  java.io.Reader	reader_,
									  int				length_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>Object</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @param	scale_
	 *			<code>java.sql.Types.DECIMAL</code> や
	 *			<code>java.sql.Types.NUMERIC</code> 型では、
	 *			小数点以下の桁数。
	 *			ほかのすべての型では、この値は無視される。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateObject(String	columnName_,
							 Object	x_,
							 int	scale_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <B>[サポート外]</B>
	 * 指定された列を <code>Object</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateObject(String	columnName_,
							 Object	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 挿入行の内容を、この <code>ResultSet</code> オブジェクトおよび
	 * データベースに挿入します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			カーソルが挿入行にないときにこのメソッドが呼び出された
	 *			場合、または挿入行内の <code>null</code> を許さない列の一部に
	 *			値がない場合。
	 */
	public void insertRow() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 基になるデータベースを、
	 * この <code>ResultSet</code> オブジェクトの現在の行の新しい内容に
	 * 更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはカーソルが挿入行にあるときに
	 *			このメソッドが呼び出された場合。
	 */
	public void updateRow() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトおよび
	 * 基になるデータベースから、現在の行を削除します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはカーソルが挿入行にあるときに
	 *			このメソッドが呼び出された場合。
	 */
	public void deleteRow() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 現在の行をデータベース内の最新の値で再表示します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * このメソッドはなにもしません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはカーソルが挿入行にあるときに
	 *			このメソッドが呼び出された場合。
	 */
	public void refreshRow() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		String	reason =
			"cursor was not supported, no refreshRow method were performed.";
		// [YET!] SQLSTATE のサブクラスがまだ未設定。"000" は仮。
		appendWarning(reason, "000");
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在の行に対して行った更新を取り消します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * このメソッドは何もしません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはカーソルが挿入行にあるときに
	 *			このメソッドが呼び出された場合。
	 */
	public void cancelRowUpdates() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		String	reason =
			"cursor was not supported, " +
			"no cancelRowUpdates method were performed.";
		// [YET!] SQLSTATE のサブクラスがまだ未設定。"000" は仮。
		appendWarning(reason, "000");
	}

	/**
	 * <B>[サポート外]</B>
	 * カーソルを挿入行に移動します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または結果セットが更新可能でない場合。
	 */
	public void moveToInsertRow() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * カーソルを、記憶されているカーソル位置 (通常は現在の行) に移動します。
	 * このメソッドは、カーソルが挿入行にある場合以外は無効です。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または結果セットが更新可能でない場合。
	 */
	public void moveToCurrentRow() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトを生成した
	 * <code>Statement</code> オブジェクトを取得します。
	 * <P>
	 * <B>注:</B>
	 * <code>ResultSet</code> オブジェクトは、
	 * <code>Statement</code> オブジェクトにより生成されないので、
	 * 常に <code>null</code> を返します。
	 * <code>Statement</code> オブジェクトが生成する、
	 * 通常の JDBC の結果セットは、サブクラスである
	 * {@link ResultSet} クラスのインスタンスです。
	 *
	 * @return	常に <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Statement getStatement() throws java.sql.SQLException
	{
		// サブクラスで実装。
		return null;
	}

	/**
	 * <B>[制限あり]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>Object</code> として取得します。
	 * 値が SQL <code>NULL</code> の場合、ドライバは Java <code>null</code> を
	 * 返します。
	 * このメソッドは、取り出される SQL 構造化型または
	 * 個別の型のカスタムマッピングに、
	 * 指定された <code>java.util.Map</code> オブジェクトを使用します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カスタムマッピングをサポートしていないため、
	 * デフォルトマッピングを使用します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	map_
	 *			SQL 型名から Java プログラミング言語のクラスへの
	 *			マッピングが格納されている <code>java.util.Map</code>
	 *			オブジェクト。
	 * @return	SQL 値を表す Java プログラミング言語の <code>Object</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public Object getObject(int				columnIndex_,
							java.util.Map	map_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カスタムマッピングは未サポート。
		// [YET!] warning かな？
		return getObject(columnIndex_);
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>Object</code> として取得します。
	 * 値が SQL <code>NULL</code> の場合、
	 * ドライバは Java <code>null</code> を返します。
	 * このメソッドは、該当する場合、
	 * 指定された <code>java.util.Map</code> オブジェクトを使用します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getObject(String, java.util.Map)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnLabel
	 *			列の SQL 名。
	 * @param	type
	 *			指定された列の変換後のJavaデータ型を表すクラス
	 * @param	<T>
	 * 			このClassオブジェクトでモデル化されるクラスの型
	 * @return	指定された列の SQL 値を表す <code>Object</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public <T> T getObject(int columnLabel, Class<T> type) throws java.sql.SQLException {
		// [NOT SUPPORTED!] 未サポート。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Ref</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>REF</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	SQL <code>REF</code> 値を表す
	 *			<code>java.sql.Ref</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Ref getRef(int	columnIndex_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL REF 型は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - data type is not supported
		//        (0A503) ※ 仮
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Blob</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>BLOB</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	指定された列の SQL <code>BLOB</code> 値を表す
	 *			<code>java.sql.Blob</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Blob getBlob(int	columnIndex_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL BLOB 型は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - data type is not supported
		//        (0A503) ※ 仮
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Clob</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>CLOB</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	指定された列の SQL <code>CLOB</code> 値を表す
	 *			<code>java.sql.Clob</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Clob getClob(int columnIndex_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL CLOB 型は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - data type is not supported
		//        (0A503) ※ 仮
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Array</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getArray(int)</code> メソッドは、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnIndex_
	 * 			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	指定された列の SQL <code>ARRAY</code> 値を表す
	 *			<code>java.sql.Array</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Array getArray(int	columnIndex_)
		throws java.sql.SQLException
	{
		// getArray メソッドは必要に応じてサブクラスで実装する。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>Object</code> として取得します。
	 * 値が SQL <code>NULL</code> の場合、
	 * ドライバは Java <code>null</code> を返します。
	 * このメソッドは、該当する場合、
	 * 指定された <code>java.util.Map</code> オブジェクトを使用します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getObject(String, java.util.Map)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	map_
	 *			SQL 型名から Java プログラミング言語の
	 *			クラスへのマッピングが格納されている
	 *			<code>java.util.Map</code> オブジェクト。
	 * @return	指定された列の SQL 値を表す <code>Object</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public Object getObject(String			columnName_,
							java.util.Map	map_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>Object</code> として取得します。
	 * 値が SQL <code>NULL</code> の場合、
	 * ドライバは Java <code>null</code> を返します。
	 * このメソッドは、該当する場合、
	 * 指定された <code>java.util.Map</code> オブジェクトを使用します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getObject(String, java.util.Map)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnLabel
	 *			列の SQL 名。
	 * @param	type
	 *			指定された列の変換後のJavaデータ型を表すクラス
	 * @param	<T>
	 * 			このClassオブジェクトでモデル化されるクラスの型
	 * @return	指定された列の SQL 値を表す <code>Object</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public <T> T getObject(String columnLabel, Class<T> type) throws java.sql.SQLException {
		// [NOT SUPPORTED!] 未サポート。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Ref</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getRef(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	指定された列の SQL <code>REF</code> 値を表す
	 *			<code>java.sql.Ref</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Ref getRef(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Blob</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getBlob(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	指定された列の SQL <code>BLOB</code> 値を表す
	 *			<code>java.sql.Blob</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Blob getBlob(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Clob</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getClob(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	指定された列の SQL <code>CLOB</code> 値を表す
	 *			<code>java.sql.Clob</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Clob getClob(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Array</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getArray(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである {@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	指定された列の SQL <code>ARRAY</code> 値を表す
	 *			<code>java.sql.Array</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Array getArray(String	columnName_)
		throws java.sql.SQLException
	{
		// 列名指定の getter メソッドはサブクラスで実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Date</code> オブジェクトとして
	 * 取得します。
	 * 基になるデータベースがタイムゾーン情報を格納していない場合、
	 * このメソッドは指定されたカレンダを使って
	 * 日付に適切なミリ秒値を作成します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	cal_
	 *			日付を作成するのに使う
	 *			<code>java.util.Calendar</code> オブジェクト。
	 * @return	<code>java.sql.Date</code> オブジェクトとして表された列の値。
	 *			値が SQL <code>NULL</code> の場合、
	 *			返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Date getDate(int				columnIndex_,
								 java.util.Calendar	cal_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カレンダ指定は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Date</code> オブジェクトとして
	 * 取得します。
	 * 基になるデータベースがタイムゾーン情報を格納していない場合、
	 * このメソッドは指定されたカレンダを使って
	 * 日付に適切なミリ秒値を作成します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	cal_
	 *			日付を作成するのに使う
	 *			<code>java.util.Calendar</code> オブジェクト。
	 * @return	<code>java.sql.Date</code> オブジェクトとして表された列の値。
	 *			値が SQL <code>NULL</code> の場合、
	 *			返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Date getDate(String				columnName_,
								 java.util.Calendar	cal_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カレンダ指定は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Time</code> オブジェクトとして
	 * 取得します。
	 * 基になるデータベースがタイムゾーン情報を格納していない場合、
	 * このメソッドは指定されたカレンダを使って
	 * 時刻に適切なミリ秒値を作成します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	cal_
	 *			時刻を作成するのに使う
	 *			<code>java.util.Calendar</code> オブジェクト。
	 * @return	<code>java.sql.Time</code> オブジェクトとして表された列の値。
	 *		値が SQL <code>NULL</code> の場合、
	 *		返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Time getTime(int				columnIndex_,
								 java.util.Calendar	cal_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カレンダ指定は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.Time</code> オブジェクトとして
	 * 取得します。
	 * 基になるデータベースがタイムゾーン情報を格納していない場合、
	 * このメソッドは指定されたカレンダを使って
	 * 時刻に適切なミリ秒値を作成します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	cal_
	 *			時刻を作成するのに使う
	 *			<code>java.util.Calendar</code> オブジェクト。
	 * @return	<code>java.sql.Time</code> オブジェクトとして表された列の値。
	 *		値が SQL <code>NULL</code> の場合、
	 *		返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Time getTime(String				columnName_,
								 java.util.Calendar	cal_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カレンダ指定は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の
	 * <code>java.sql.Timestamp</code> オブジェクトとして取得します。
	 * 基になるデータベースがタイムゾーン情報を格納していない場合、
	 * このメソッドは指定されたカレンダを使って
	 * タイムスタンプに適切なミリ秒値を作成します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	cal_
	 *			タイムスタンプを作成するのに使う
	 *			<code>java.util.Calendar</code> オブジェクト。
	 * @return	<code>java.sql.Timestamp</code> オブジェクトとして表された
	 *			列の値。値が SQL <code>NULL</code> の場合、
	 *			返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Timestamp getTimestamp(int					columnIndex_,
										   java.util.Calendar	cal_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カレンダ指定は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、 Java プログラミング言語の
	 * <code>java.sql.Timestamp</code> オブジェクトとして取得します。
	 * 基になるデータベースがタイムゾーン情報を格納していない場合、
	 * このメソッドは指定されたカレンダを使って
	 * タイムスタンプに適切なミリ秒値を作成します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	cal_
	 *			タイムスタンプを作成するのに使う
	 *			<code>java.util.Calendar</code> オブジェクト。
	 * @return	<code>java.sql.Timestamp</code> オブジェクトとして表された
	 *			列の値。値が SQL <code>NULL</code> の場合、
	 *			返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Timestamp getTimestamp(String				columnName_,
										   java.util.Calendar	cal_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カレンダ指定は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.net.URL</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>DATALINK</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	<code>java.net.URL</code> オブジェクトとして表された列の値。
	 *			値が SQL <code>NULL</code> の場合、
	 *			返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または URL が無効の場合。
	 */
	public java.net.URL getURL(int	columnIndex_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL DATALINK 型は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - data type is not supported
		//        (0A503) ※ 仮
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.net.URL</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドはサポートしていません。
	 * 通常の JDBC の結果セットに対する
	 * <code>getURL(String)</code> メソッド
	 * (列の SQL 名を指定する getter メソッド) は、
	 * サブクラスである{@link ResultSet} クラスで実装されています。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	<code>java.net.URL</code> オブジェクトとして表された列の値。
	 *			値が SQL <code>NULL</code> の場合、
	 *			返される値は Java プログラミング言語の <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または URL が無効の場合。
	 */
	public java.net.URL getURL(String	columnName_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL DATALINK 型は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - data type is not supported
		//        (0A503) ※ 仮
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Ref</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateRef(int			columnIndex_,
						  java.sql.Ref	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Ref</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateRef(String		columnName_,
						  java.sql.Ref	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Blob</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateBlob(int				columnIndex_,
						   java.sql.Blob	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Blob</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateBlob(String			columnName_,
						   java.sql.Blob	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Clob</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateClob(int				columnIndex_,
						   java.sql.Clob	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Clob</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateClob(String			columnName_,
						   java.sql.Clob	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Array</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateArray(int				columnIndex_,
							java.sql.Array	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された列を <code>java.sql.Array</code> 値で更新します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @param	x_
	 *			新しい列値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateArray(String			columnName_,
							java.sql.Array	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] updater メソッドは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * getter メソッドが実行可能かをチェックし、
	 * 実行可能な場合には列のデータオブジェクトを返します。
	 * ただし、列値が SQL <code>NULL</code> 値の場合には、
	 * そのことを記憶し、 <code>null</code> を返します。
	 * また、実行ができない場合には例外を送出します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列のデータオブジェクト。
	 * @throws	java.sql.SQLException
	 *			getter メソッドが実行不可能な場合。
	 */
	jp.co.ricoh.doquedb.common.Data getColumn(int	columnIndex_)
		throws java.sql.SQLException
	{
		if (this._row == null) {
			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			throw new Unexpected();
		}

		closeInputStream();

		Data	column = this._row.getElement(columnIndex_ - 1);

		this._wasNull = (column.getType() == DataType.NULL);

		return this._wasNull ? null : column;
	}

	/**
	 * getter メソッドが実行可能かをチェックし、
	 * 実行可能な場合には列のデータオブジェクトを返します。
	 * また、実行ができない場合には例外を送出します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @param	scale_
	 *			小数点以下の桁数
	 * @return	列のデータオブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	jp.co.ricoh.doquedb.common.Data getColumn(int	columnIndex_,
											 int	scale_)
		throws java.sql.SQLException
	{
		if (scale_ < 0) {
			// [YET!] SQLSTATE は、
			//        data exception - ???
			//        (22???)
			throw new BadArgument();
		}

		return getColumn(columnIndex_);
	}

	/**
	 * オープンしている入力ストリームをクローズします。
	 */
	void closeInputStream()
	{
		if (this._binaryStream != null) {

			try {
				this._binaryStream.close();
			} catch (java.io.IOException	ioe) {
				// ignore
			}
			this._binaryStream = null;
		}
	}

	/**
	 * この <code>ResultSet</code> オブジェクトに関する警告を
	 * 追加します。
	 *
	 * @param	reason_
	 *			警告の説明。
	 * @param	subClass_
	 *			警告を識別する SQLSTATE のサブクラス。
	 */
	private void appendWarning(String	reason_,
							   String	subClass_)
	{
		java.sql.SQLWarning	warning =
			new java.sql.SQLWarning(reason_,
									"01" + subClass_);

		if (this._warnings == null) {
			this._warnings = warning;
		} else {
			this._warnings.setNextWarning(warning);
		}
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public <T> T unwrap(Class<T> iface) throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 常に<code>false</code>を返します
	 */
	@Override
	public boolean isWrapperFor(Class<?> iface) throws SQLException {
		// unwrap を実装している場合は true を返すが、
		// それ以外の場合は false を返す

		return false;
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.RowId</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>ROWID</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 */
	@Override
	public RowId getRowId(int columnIndex) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの
	 * 現在行にある指定された列の値を、
	 * Java プログラミング言語の <code>java.sql.RowId</code> オブジェクトとして
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>ROWID</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 */
	@Override
	public RowId getRowId(String columnLabel) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateRowId(int columnIndex, RowId x) throws SQLException {
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateRowId(String columnLabel, RowId x) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの保持機能を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に
	 * <code>java.sql.ResultSet.CLOSE_CUSORS_AT_COMMIT</code> を返します。
	 */
	@Override
	public int getHoldability() throws SQLException {
		// [NOT SUPPORTED!] 保持機能は、
		//                  java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT
		//                  のみをサポート。

		return java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトが
	 * クローズされているかどうかを取得します。
	 */
	@Override
	public boolean isClosed() throws SQLException {
		// クローズされているか否か

		return this._isClosed;
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNString(int columnIndex, String nString)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNString(String columnLabel, String nString)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNClob(int columnIndex, NClob nClob) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNClob(String columnLabel, NClob nClob)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 SQL <code>NCLOB</code>型はサポートしていません。
	 */
	@Override
	public NClob getNClob(int columnIndex) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 SQL <code>NCLOB</code>型はサポートしていません。
	 */
	@Override
	public NClob getNClob(String columnLabel) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 SQL <code>XML</code>型はサポートしていません。
	 */
	@Override
	public SQLXML getSQLXML(int columnIndex) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 SQL <code>XML</code>型はサポートしていません。
	 */
	@Override
	public SQLXML getSQLXML(String columnLabel) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateSQLXML(int columnIndex, SQLXML xmlObject)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateSQLXML(String columnLabel, SQLXML xmlObject)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.lang.String</code> として取得します。
	 */
	@Override
	public String getNString(int columnIndex) throws SQLException {
		// NStringもStringもJDBCでは区別されない

		return getString(columnIndex);
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.lang.String</code> として取得します。
	 */
	@Override
	public String getNString(String columnLabel) throws SQLException {
		// NStringもStringもJDBCでは区別されない

		return getString(columnLabel);
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.io.Reader</code> オブジェクトとして取得します。
	 */
	@Override
	public Reader getNCharacterStream(int columnIndex) throws SQLException {
		// NCharacterもCharacterもJDBCでは区別されない

		return getCharacterStream(columnIndex);
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.io.Reader</code> オブジェクトとして取得します。
	 */
	@Override
	public Reader getNCharacterStream(String columnLabel) throws SQLException {
		// NCharacterもCharacterもJDBCでは区別されない

		return getCharacterStream(columnLabel);
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNCharacterStream(int columnIndex, Reader x, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNCharacterStream(String columnLabel, Reader reader,
			long length) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateAsciiStream(int columnIndex, InputStream x, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateBinaryStream(int columnIndex, InputStream x, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateCharacterStream(int columnIndex, Reader x, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateAsciiStream(String columnLabel,
								  InputStream x, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateBinaryStream(String columnLabel, InputStream x,
			long length) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateCharacterStream(String columnLabel, Reader reader,
			long length) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateBlob(int columnIndex,
						   InputStream inputStream, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateBlob(String columnLabel, InputStream inputStream,
			long length) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateClob(int columnIndex, Reader reader, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateClob(String columnLabel, Reader reader, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNClob(int columnIndex, Reader reader, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNClob(String columnLabel, Reader reader, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNCharacterStream(int columnIndex, Reader x)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNCharacterStream(String columnLabel, Reader reader)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateAsciiStream(int columnIndex, InputStream x)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateBinaryStream(int columnIndex, InputStream x)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateCharacterStream(int columnIndex, Reader x)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateAsciiStream(String columnLabel, InputStream x)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateBinaryStream(String columnLabel, InputStream x)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateCharacterStream(String columnLabel, Reader reader)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateBlob(int columnIndex, InputStream inputStream)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateBlob(String columnLabel, InputStream inputStream)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateClob(int columnIndex, Reader reader) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateClob(String columnLabel, Reader reader)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNClob(int columnIndex, Reader reader)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 現在のバージョンでは、 updater メソッドはサポートしていません。
	 */
	@Override
	public void updateNClob(String columnLabel, Reader reader)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}
}

//
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2015, 2016, 2023, 2024 Ricoh Company, Ltd.
// All rights reserved.
//
