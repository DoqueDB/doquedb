// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MetaDataResultSet.java -- メタデータによる結果セットのクラス
//
// Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
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

import jp.co.ricoh.doquedb.common.DataArrayData;
import jp.co.ricoh.doquedb.common.StringArrayData;
import jp.co.ricoh.doquedb.exception.ColumnNotFound;
import jp.co.ricoh.doquedb.exception.NotSupported;

/**
 * メタデータによる結果セットのクラス。
 *
 */
class MetaDataResultSet extends DefaultResultSet
{
	/**
	 * メタデータによる結果セットの実体である配列。
	 * 配列内の各要素も配列 ( <code>DataArrayData</code> ) であり、
	 * それぞれがひとつの行となる。
	 *
	 * @see	jp.co.ricoh.doquedb.common.DataArrayData
	 */
	private DataArrayData	_resultSet;

	/**
	 * 結果セットの行数。
	 */
	private int	_numberOfRows;

	/**
	 * 各列の名前。
	 */
	private StringArrayData	_columnNames;

	/**
	 * 新しく空のメタデータによる結果セットオブジェクトを作成します。
	 */
	MetaDataResultSet()
	{
		super();

		// 空の結果セットを作成する。
		this._resultSet = new DataArrayData();
		this._numberOfRows = 0;
		this._columnNames = null;
	}

	/**
	 * 新しくメタデータによる結果セットオブジェクトを作成します。
	 *
	 * @param	resultSet_
	 *			メタデータによる結果セット。
	 *			各要素が <code>DataArrayData</code> の配列。
	 */
	MetaDataResultSet(DataArrayData	resultSet_)
	{
		super();

		this._resultSet = resultSet_;
		this._numberOfRows = this._resultSet.getCount();
		this._columnNames = null;

		// 列数を設定する。
		if (this._resultSet.getCount() > 0) {
			DataArrayData	firstColumn =
				(DataArrayData)this._resultSet.getElement(0);
			super._numberOfColumns = firstColumn.getCount();
		}
	}

	/**
	 * 新しくメタデータによる結果セットオブジェクトを作成します。
	 *
	 * @param	resultSet_
	 *			メタデータによる結果セット。
	 *			各要素が <code>DataArrayData</code> の配列。
	 * @param	columnNames_
	 *			各列の名前。
	 */
	MetaDataResultSet(DataArrayData	resultSet_,
								StringArrayData	columnNames_)
	{
		this(resultSet_);

		this._columnNames = columnNames_;
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
		super.closeInputStream();

		if (super._rowNumber == -1) return false;

		if (super._rowNumber >= this._numberOfRows) {

			super._rowNumber = -1;
			super._row = null;
			return false;
		}

		super._row =
			(DataArrayData)this._resultSet.getElement(super._rowNumber);
		super._rowNumber++;

		return true;
	}

	/**
	 * 自動的にクローズされるとき、これを待つのではなく、
	 * ただちにこの <code>MetaDataResultSet</code> オブジェクトの
	 * データベースと JDBC リソースを解放します。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void close() throws java.sql.SQLException
	{
		super.close();

		this._resultSet = null;
		this._numberOfRows = 0;
	}

	/**
	 * カーソルがこの <code>MetaDataResultSet</code> オブジェクトの
	 * 最終行にあるかどうかを取得します。
	 *
	 * @return	カーソルが最終行にある場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isLast() throws java.sql.SQLException
	{
		return (super._rowNumber == this._numberOfRows);
	}

	/**
	 * 指定された <code>ResultSet</code> の列名を
	 * <code>ResultSet</code> 列インデックスにマッピングします。
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
		if (this._columnNames == null) throw new NotSupported();

		for (int i = 0; i < super._numberOfColumns; i++) {

			String	columnName = this._columnNames.getElement(i);
			if (columnName.compareToIgnoreCase(columnName_) == 0) {
				return i + 1;
			}
		}

		throw new ColumnNotFound(columnName_);
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.lang.String</code> として取得します。
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
		return super.getString(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>boolean</code> として取得します。
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
		return super.getBoolean(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>byte</code> として取得します。
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
		return super.getByte(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>short</code> として取得します。
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
		return super.getShort(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>int</code> として取得します。
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
		return super.getInt(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>long</code> として取得します。
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
		return super.getLong(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>float</code> として取得します。
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
		return super.getFloat(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>double</code> として取得します。
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
		return super.getDouble(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.math.BigDecimal</code> として取得します。
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
		return super.getBigDecimal(this.findColumn(columnName_), scale_);
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>byte</code> 配列として取得します。
	 * バイトはドライバによって返された行の値を表します。
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
		return super.getBytes(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.sql.Date</code> オブジェクトとして取得します。
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
		return super.getDate(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.sql.Time</code> オブジェクトとして取得します。
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
		return super.getTime(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.sql.Timestamp</code> オブジェクトとして取得します。
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
		return super.getTimestamp(this.findColumn(columnName_));
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 ASCII 文字のストリームとして取得します。
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
		return super.getAsciiStream(this.findColumn(columnName_));
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 2 バイト Unicode 文字のストリームとして取得します。
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
		return super.getUnicodeStream(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、未解釈のバイトストリームとして取得します。
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
		return super.getBinaryStream(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>Object</code> として取得します。
	 * <P>
	 * このメソッドは、指定された列の値を Java オブジェクトとして返します。
	 * Java オブジェクトの型は、 JDBC 仕様で指定されている組み込み型の
	 * マッピングに従って、列の SQL 型に対応するデフォルトの
	 * Java オブジェクト型になります。値が SQL <code>NULL</code> の場合、
	 * ドライバは Java <code>null</code> を返します。
	 * <P>
	 * このメソッドは、データベース固有の抽象データ型を読み込む目的にも
	 * 使用できます。
	 * <P>
	 * JDBC 2.0 API では、<code>getObject</code> メソッドの動作は
	 * SQL ユーザ定義型のデータを生成するように拡張されます。
	 * 列が構造化型または個別の型の値である場合、このメソッドの動作は、
	 * <code>
	 * getObject(columnIndex, this.getStatement().getConnection().getTypeMap())
	 * </code>
	 * を呼び出した場合と同じになります。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	列値を保持している <code>java.lang.Object</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public Object getObject(String	columnName_) throws java.sql.SQLException
	{
		return super.getObject(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.io.Reader</code> オブジェクトとして取得します。
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
		return super.getCharacterStream(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.math.BigDecimal</code> オブジェクトとして全精度で取得します。
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
		return super.getBigDecimal(this.findColumn(columnName_));
	}

	/**
	 * <B>[制限あり]</B>
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、Java プログラミング言語の
	 * <code>Object</code> として取得します。
	 * 値が SQL <code>NULL</code> の場合、
	 * ドライバは Java <code>null</code> を返します。
	 * このメソッドは、該当する場合、
	 * 指定された <code>java.util.Map</code> オブジェクトを使用します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カスタムマッピングをサポートしていないため、
	 * デフォルトマッピングを使用します。
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
		return super.getObject(this.findColumn(columnName_), map_);
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、Java プログラミング言語の
	 * <code>java.sql.Ref</code> オブジェクトとして取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>REF</code> 型をサポートしていないため、
	 * このメソッドはサポートしていません。
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
		return super.getRef(this.findColumn(columnName_));
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、Java プログラミング言語の
	 * <code>java.sql.Blob</code> オブジェクトとして取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * SQL <code>BLOB</code> 型をサポートしていないため、
	 * このメソッドはサポートしていません。
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
		return super.getBlob(this.findColumn(columnName_));
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、Java プログラミング言語の
	 * <code>java.sql.Clob</code> オブジェクトとして取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * SQL <code>CLOB</code> 型をサポートしていないため、
	 * このメソッドはサポートしていません。
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
		return super.getClob(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、Java プログラミング言語の
	 * <code>java.sql.Array</code> オブジェクトとして取得します。
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
		return super.getArray(this.findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.sql.Date</code> オブジェクトとして取得します。
	 * 基になるデータベースがタイムゾーン情報を格納していない場合、
	 * このメソッドは指定されたカレンダを使って
	 * 日付に適切なミリ秒値を作成します。
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
		return super.getDate(this.findColumn(columnName_), cal_);
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.sql.Time</code> オブジェクトとして取得します。
	 * 基になるデータベースがタイムゾーン情報を格納していない場合、
	 * このメソッドは指定されたカレンダを使って
	 * 時刻に適切なミリ秒値を作成します。
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
		return super.getTime(this.findColumn(columnName_), cal_);
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.sql.Timestamp</code> オブジェクトとして取得します。
	 * 基になるデータベースがタイムゾーン情報を格納していない場合、
	 * このメソッドは指定されたカレンダを使って
	 * タイムスタンプに適切なミリ秒値を作成します。
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
		return super.getTimestamp(this.findColumn(columnName_), cal_);
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.net.URL</code> オブジェクトとして取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * SQL <code>DATALINK</code> 型をサポートしていないため、
	 * このメソッドはサポートしていません。
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
		return super.getURL(this.findColumn(columnName_));
	}

	/**
	 * このオブジェクトへの参照がないと、ガベージコレクションによって
	 * 判断されたときに、ガベージコレクタによって呼び出されます。
	 *
	 * @throws	java.lang.Throwable
	 *			このメソッドで生じた例外。
	 */
	protected void finalize() throws java.lang.Throwable
	{
		this.close();
	}
}

//
// Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
