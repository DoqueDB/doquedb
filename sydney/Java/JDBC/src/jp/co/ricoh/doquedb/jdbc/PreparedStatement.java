// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PreparedStatement.java -- JDBC のプリペアステートメントクラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2015, 2016, 2023 Ricoh Company, Ltd.
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
import jp.co.ricoh.doquedb.common.DateTimeData;
import jp.co.ricoh.doquedb.common.DecimalData;
import jp.co.ricoh.doquedb.common.DoubleData;
import jp.co.ricoh.doquedb.common.FloatData;
import jp.co.ricoh.doquedb.common.Integer64Data;
import jp.co.ricoh.doquedb.common.IntegerData;
import jp.co.ricoh.doquedb.common.LanguageData;
import jp.co.ricoh.doquedb.common.NullData;
import jp.co.ricoh.doquedb.common.StringData;
import jp.co.ricoh.doquedb.exception.BadArgument;
import jp.co.ricoh.doquedb.exception.Cancel;
import jp.co.ricoh.doquedb.exception.ConnectionRanOut;
import jp.co.ricoh.doquedb.exception.NotSupported;
import jp.co.ricoh.doquedb.exception.Unexpected;

/**
 * JDBC のプリペアステートメントクラス。
 *
 */
public class PreparedStatement
	extends Statement
	implements java.sql.PreparedStatement
{
	/**
	 * コンパイル結果オブジェクト。
	 *
	 * @see	jp.co.ricoh.doquedb.client.PrepareStatement
	 */
	private jp.co.ricoh.doquedb.client.PrepareStatement	_prepareStatement;

	/** パラメータ列。 */
	private DataArrayData		_parameter;

	/** バッチ更新のためのパラメータ群を格納するベクター。 */
	private java.util.Vector	_batchParameters;

	/**
	 * 新しくプリペアステートメントオブジェクトを作成します。
	 *
	 * @param	connection_
	 *			コネクションオブジェクト。
	 * @param	prepareStatement_
	 *			コンパイル結果オブジェクト。
	 * @param	resultSetType_
	 *			結果セットの型。現在のバージョンでは、
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみを
	 *			サポートします。
	 * @param	resultSetConcurrency_
	 *			並行処理の種類。現在のバージョンでは、
	 *			<code>java.sql.ResultSet.CONCUR_READ_ONLY</code> のみを
	 *			サポートします。
	 * @param	resultSetHoldability_
	 *			保持機能の種類。現在のバージョンでは、
	 *			<code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code> のみを
	 *			サポートします。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see	jp.co.ricoh.doquedb.jdbc.Connection
	 * @see	jp.co.ricoh.doquedb.client.PrepareStatement
	 */
	PreparedStatement(
		jp.co.ricoh.doquedb.jdbc.Connection			connection_,
		jp.co.ricoh.doquedb.client.PrepareStatement	prepareStatement_,
		int											resultSetType_,
		int											resultSetConcurrency_,
		int											resultSetHoldability_)
		throws java.sql.SQLException
	{
		super(connection_,
			  resultSetType_,
			  resultSetConcurrency_,
			  resultSetHoldability_);

		_prepareStatement = prepareStatement_;
		this._parameter = new DataArrayData();
		this._batchParameters = null;
	}

	/**
	 * この <code>PreparedStatement</code> オブジェクトの
	 * SQL クエリーを実行し、そのクエリーによって生成された
	 * <code>java.sql.ResultSet</code> オブジェクトを返します。
	 *
	 * @return	クエリーによって作成されたデータを含む
	 *			<code>java.sql.ResultSet</code> オブジェクト。
	 *			<code>null</code> にはならない。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または SQL 文が <code>java.sql.ResultSet</code> オブジェクトを
	 *			返さない場合。
	 */
	public java.sql.ResultSet executeQuery() throws java.sql.SQLException
	{
		return executeStatement(true /* read only */, false /* no update */);
	}

	/**
	 * この <code>PreparedStatement</code> オブジェクトの
	 * SQL <code>INSERT</code> 文、 <code>UPDATE</code> 文、
	 * または <code>DELETE</code> 文を実行します。
	 * さらに、DDL 文のような何も返さない SQL 文を実行することもできます。
	 *
	 * @return	<code>INSERT</code> 文、 <code>UPDATE</code> 文、
	 *			<code>DELETE</code> 文の場合は行数。
	 *			何も返さない SQL 文の場合は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または SQL 文が <code>java.sql.ResultSet</code> オブジェクトを
	 *			返す場合。
	 */
	public int executeUpdate() throws java.sql.SQLException
	{
		executeStatement(false /* not read only */, true /* updating */);

		// skip until status is obtained with count up
		// [NOTES]
		// If status become HAS_MORE_DATA, more than one statements has been passed.
		// According to JDBC specifications, this case should be an error.
		// However, because JDBC driver can not parse SQL statement,
		//     JDBC driver does not cause errors and executes all the SQL statements.
		// The result will be sum of all the update counts.
		int status = _resultSet.skipToStatus(true /* skip has_more_data too */);

		if (status == jp.co.ricoh.doquedb.client.ResultSet.CANCELED) {
			throw new Cancel();
		}
		int count = super._resultSet.getUpdateCount();

		super.closeResultSet();

		return count;
	}

	/**
	 * <B>[制限あり]</B>
	 * 指定されたパラメータを SQL <code>NULL</code> に設定します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 <code>sqlType_</code> は無視されます。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	sqlType_
	 *			<code>java.sql.Types</code> で定義される、
	 *			JDBC 型と呼ばれる、汎用の SQL 型を識別するためのコード。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setNull(int	parameterIndex_,
						int	sqlType_)
		throws java.sql.SQLException
	{
		setNull(parameterIndex_);
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータを指定された Java の <code>boolean</code> 値に
	 * 設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.BIT</code> 値に変換します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * SQL <code>BIT</code> 型をサポートしていないので、
	 * このメソッドはサポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setBoolean(int		parameterIndex_,
						   boolean	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL BIT 型は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - data type is not supported
		//        (0A503) ※ 仮
		throw new NotSupported();
	}

	/**
	 * 指定されたパラメータを指定された Java の <code>byte</code> 値に
	 * 設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.TINYINT</code> 値に
	 * 変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setByte(int		parameterIndex_,
						byte	x_)
		throws java.sql.SQLException
	{
		setInt(parameterIndex_, x_);
	}

	/**
	 * 指定されたパラメータを指定された Java の <code>short</code> 値に
	 * 設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.SMALLINT</code> 値に
	 * 変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setShort(int	parameterIndex_,
						 short	x_)
		throws java.sql.SQLException
	{
		setInt(parameterIndex_, x_);
	}

	/**
	 * 指定されたパラメータを指定された Java の <code>int</code> 値に
	 * 設定します。データベースに送るときに、ドライバはこれを
	 * SQL <code>java.sql.Types.INTEGER</code> 値に変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setInt(int	parameterIndex_,
					   int	x_)
		throws java.sql.SQLException
	{
		this._parameter.setElement(parameterIndex_ - 1, new IntegerData(x_));
	}

	/**
	 * 指定されたパラメータを指定された Java の <code>long</code> 値に
	 * 設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.BIGINT</code> 値に変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setLong(int		parameterIndex_,
						long	x_)
		throws java.sql.SQLException
	{
		this._parameter.setElement(parameterIndex_ - 1, new Integer64Data(x_));
	}

	/**
	 * 指定されたパラメータを指定された Java の <code>float</code> 値に
	 * 設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.FLOAT</code> 値に変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setFloat(int	parameterIndex_,
						 float	x_)
		throws java.sql.SQLException
	{
		this._parameter.setElement(parameterIndex_ - 1, new FloatData(x_));
	}

	/**
	 * 指定されたパラメータを指定された Java の <code>double</code> 値に
	 * 設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.DOUBLE</code> 値に変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setDouble(int		parameterIndex_,
						  double	x_)
		throws java.sql.SQLException
	{
		this._parameter.setElement(parameterIndex_ - 1, new DoubleData(x_));
	}

	/**
	 * 指定されたパラメータを指定された <code>java.math.BigDecimal</code> 値に
	 * 設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.NUMERIC</code> 値に
	 * 変換します。
	 * <P>
	 * <B>注:</B>
	 * ただし変換された値は SQL <code>java.sql.Types.DECIMAL</code> 型になります。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setBigDecimal(int					parameterIndex_,
							  java.math.BigDecimal	x_)
		throws java.sql.SQLException
	{
		this._parameter.setElement(parameterIndex_ - 1, new DecimalData(x_));
	}

	/**
	 * 指定されたパラメータを指定された Java の
	 * <code>java.lang.String</code> 値に設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.VARCHAR</code> または
	 * <code>java.sql.Types.LONGVARCHAR</code> 値
	 * (ドライバの <code>java.sql.Types.VARCHAR</code> 値に関する
	 * 制限に関する引数のサイズに依存) に変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setString(int		parameterIndex_,
						  String	x_)
		throws java.sql.SQLException
	{
		if (x_ == null) {
			setNull(parameterIndex_);
			return;
		}

		this._parameter.setElement(parameterIndex_ - 1, new StringData(x_));
	}

	/**
	 * 指定されたパラメータを指定された Java のバイト配列に設定します。
	 * データベースに送るときに、ドライバはこれを
	 * SQL <code>java.sql.Types.VARBINARY</code>
	 * または <code>java.sql.Types.LONGVARBINARY</code>
	 * (ドライバの <code>java.sql.Types.VARBINARY</code> 値
	 * に関する制限に関する引数のサイズに依存) に変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setBytes(int	parameterIndex_,
						 byte[]	x_)
		throws java.sql.SQLException
	{
		if (x_ == null) {
			setNull(parameterIndex_);
			return;
		}

		this._parameter.setElement(parameterIndex_ - 1,
								   new BinaryData(x_));
	}

	/**
	 * <B>[制限あり]</B>
	 * 指定されたパラメータを指定された <code>java.sql.Date</code> 値に
	 * 設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.DATE</code> 値に変換します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * SQL <code>DATETIME</code> 値に変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setDate(int				parameterIndex_,
						java.sql.Date	x_)
		throws java.sql.SQLException
	{
		if (x_ == null) {
			setNull(parameterIndex_);
			return;
		}

		setTimestamp(parameterIndex_,
					 new java.sql.Timestamp(x_.getTime()));
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータを指定された <code>java.sql.Time</code> 値に
	 * 設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.TIME</code> 値に変換します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>TIME</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setTime(int				parameterIndex_,
						java.sql.Time	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL TIME 型は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - data type is not supported
		//        (0A503) ※ 仮
		throw new NotSupported();
	}

	/**
	 * <B>[制限あり]</B>
	 * 指定されたパラメータを指定された <code>java.sql.Timestamp</code> 値に
	 * 設定します。データベースに送るときに、
	 * ドライバはこれを SQL <code>java.sql.Types.TIMESTAMP</code> 値に
	 * 変換します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * SQL <code>DATETIME</code> 値に変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setTimestamp(int				parameterIndex_,
							 java.sql.Timestamp	x_)
		throws java.sql.SQLException
	{
		if (x_ == null) {
			setNull(parameterIndex_);
			return;
		}

		this._parameter.setElement(parameterIndex_ - 1, new DateTimeData(x_));
	}

	/**
	 * 指定されたパラメータを、指定されたバイト数を持つ指定された
	 * 入力ストリームに設定します。
	 * <code>java.sql.Types.LONGVARCHAR</code> パラメータに
	 * 非常に大きな ASCII 値が入力されるときには、
	 * <code>java.io.InputStream</code> を介して送るのが現実的です。
	 * ファイルの終わりに達するまで必要に応じてストリームからデータが
	 * 読み込まれます。 JDBC ドライバは、データを ASCII から
	 * データベースの char 形式に変換します。
	 * <P>
	 * <B>注:</B>
	 * このストリームオブジェクトは、標準の Java ストリームオブジェクト、
	 * または標準インタフェースを実装する独自のサブクラスの
	 * どちらでもかまいません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			ASCII パラメータ値を含む Java 入力ストリーム。
	 * @param	length_
	 *			ストリームのバイト数。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setAsciiStream(int					parameterIndex_,
							   java.io.InputStream	x_,
							   int					length_)
		throws java.sql.SQLException
	{
		if (x_ == null) {
			setNull(parameterIndex_);
			return;
		}

		try {

			byte[]	buffer = new byte[length_];
			x_.read(buffer, 0, length_);
			StringData	x = new StringData(new String(buffer, 0, length_));

			this._parameter.setElement(parameterIndex_ - 1, x);

		} catch (java.io.IOException	ioe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			//        system error - ???
			//        (60???)
			throw new Unexpected();
		}
	}

	/**
	 * 指定されたパラメータを、指定されたバイト数を持つ指定された
	 * 入力ストリームに設定します。 Unicode 文字は 2 バイトからなり、
	 * 最初のバイトが上位バイト、 2 番目が下位バイトです。
	 * <code>java.sql.Types.LONGVARCHAR</code> パラメータに
	 * 非常に大きな Unicode 値が入力されるときには、
	 * <code>java.io.InputStream</code> を介して送るのが現実的です。
	 * ファイルの終わりに達するまで必要に応じてストリームからデータが
	 * 読み込まれます。 JDBC ドライバは、データを Unicode から
	 * データベースの char 形式に変換します。
	 * <P>
	 * <B>注:</B>
	 * このストリームオブジェクトは、標準の Java ストリームオブジェクト、
	 * または標準インタフェースを実装する独自のサブクラスの
	 * どちらでもかまいません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			Unicode パラメータを 2 バイトの Unicode 文字として格納する
	 *			<code>java.io.InputStream</code> オブジェクト。
	 * @param	length_
	 *			ストリームのバイト数。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @deprecated
	 */
	public void setUnicodeStream(int					parameterIndex_,
								 java.io.InputStream	x_,
								 int					length_)
		throws java.sql.SQLException
	{
		if (x_ == null) {
			setNull(parameterIndex_);
			return;
		}

		try {

			byte[]	buffer = new byte[length_];
			x_.read(buffer, 0, length_);
			StringData	x =
				new StringData(new String(buffer, 0, length_, "UTF-8"));
			this._parameter.setElement(parameterIndex_ - 1, x);

		} catch (java.io.IOException	ioe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			//        system error - ???
			//        (60???)
			throw new Unexpected();
		}
	}

	/**
	 * 指定されたパラメータを、指定されたバイト数を持つ指定された
	 * 入力ストリームに設定します。
	 * <code>java.sql.Types.LONGVARBINARY</code> パラメータに、
	 * 非常に大きなバイナリ値が入力されるときには、
	 * <code>java.io.InputStream</code> オブジェクトを介して送るのが
	 * 現実的です。ファイルの終わりに達するまで必要に応じて
	 * ストリームからデータが読み込まれます。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値を含む Java 入力ストリーム。
	 * @param	length_
	 *			ストリームのバイト数。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setBinaryStream(int					parameterIndex_,
								java.io.InputStream	x_,
								int					length_)
		throws java.sql.SQLException
	{
		if (x_ == null) {
			setNull(parameterIndex_);
			return;
		}

		try {

			byte[]	buffer = new byte[length_];
			x_.read(buffer, 0, length_);
			BinaryData x = new BinaryData(buffer);

			this._parameter.setElement(parameterIndex_ - 1, x);

		} catch (java.io.IOException	ioe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			//        system error - ???
			//        (60???)
			throw new BadArgument();
		}
	}

	/**
	 * 現在のパラメータ値をすぐにクリアします。
	 * <P>
	 * 通常、文を繰り返し使用するために、パラメータ値は強制的に残されます。
	 * パラメータ値を設定すると、前の値は自動的にクリアされます。
	 * しかし、現在のパラメータ値によって使用されたリソースを
	 * ただちに解放した方が役に立つ場合があります。
	 * これは、<code>clearParameters</code> メソッドを呼び出して
	 * 実行することができます。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void clearParameters() throws java.sql.SQLException
	{
		this._parameter.clear();
	}

	/**
	 * <B>[制限あり]</B>
	 * 指定されたパラメータの値を、指定されたオブジェクトを使用して設定します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 <code>sqlType_</code> および
	 * <code>scale_</code> は無視されます。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			入力パラメータ値を含むオブジェクト。
	 * @param	sqlType_
	 *			データベースに送られる、
	 *			<code>java.sql.Types</code> で定義される、
	 *			JDBC 型と呼ばれる、汎用の SQL 型を識別するためのコード。
	 *			スケール引数で、さらに型を限定できる。
	 * @param	scale_
	 *			<code>java.sql.Types.DECIMAL</code> や
	 *			<code>java.sql.Types.NUMERIC</code> 型では、小数点以下の桁数。
	 *			ほかのすべての型では、この値は無視される。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setObject(int		parameterIndex_,
						  Object	x_,
						  int		sqlType_,
						  int		scale_)
		throws java.sql.SQLException
	{
		setObject(parameterIndex_, x_, sqlType_);
	}

	/**
	 * <B>[制限あり]</B>
	 * 指定されたパラメータの値を、指定されたオブジェクトで設定します。
	 * このメソッドは、上記のメソッド <code>setObject</code> に似ていますが、
	 * スケールに 0 を仮定している点が異なります。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 <code>sqlType_</code> は無視されます。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			入力パラメータ値を含むオブジェクト。
	 * @param	sqlType_
	 *			データベースに送られる、
	 *			<code>java.sql.Types</code> で定義される、
	 *			JDBC 型と呼ばれる、汎用の SQL 型を識別するためのコード。
	 *			スケール引数で、さらに型を限定できる。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setObject(int		parameterIndex_,
						  Object	x_,
						  int		sqlType_)
		throws java.sql.SQLException
	{
		if (sqlType_ == java.sql.Types.NULL)
			setObject(parameterIndex_, null);
		else
			setObject(parameterIndex_, x_);
	}

	/**
	 * 指定されたパラメータの値を、指定されたオブジェクトを使用して設定します。
	 * 2 番目のパラメータは、<code>java.lang.Object</code> の型で
	 * なければなりません。
	 * したがって、組み込み型の場合、等価な <code>java.lang</code>
	 * オブジェクトを使用する必要があります。
	 * <P>
	 * JDBC は、Java の <code>java.lang.Object</code> 型から
	 * SQL 型への標準マッピングを
	 * 指定しています。指定された引数は、データベースに送られる前に、
	 * 対応する SQL 型に変換されます。
	 * <P>
	 * このメソッドは、ドライバ固有の Java 型を使用して、データベース固有の
	 * 抽象データ型を渡すために使用することに注意してください。
	 * オブジェクトがインタフェース <code>SQLData</code> を実装するクラスの
	 * 場合、JDBC ドライバは <code>SQLData.writeSQL</code> メソッドを
	 * 呼び出して、そのオブジェクトを SQL データストリームへ書き込む必要が
	 * あります。また、オブジェクトが <code>java.sql.Ref</code> 、
	 * <code>java.sql.Blob</code> 、 <code>java.sql.Clob</code> 、
	 * <code>java.sql.Struct</code> 、または <code>java.sql.Array</code> を
	 * 実装するクラスの場合、ドライバは対応する SQL 型の値として
	 * オブジェクトをデータベースに渡さなければなりません。
	 * <P>
	 * たとえば、オブジェクトが上記のインタフェースを複数実装するクラスで
	 * あるなど、あいまいさがある場合、このメソッドは例外をスローします。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			入力パラメータ値を含むオブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定されたオブジェクトの型があいまいである場合。
	 */
	public void setObject(int		parameterIndex_,
						  Object	x_)
		throws java.sql.SQLException
	{
		if (x_ == null) {
			setNull(parameterIndex_);
			return;
		}

		if (x_ instanceof java.lang.Boolean) {

			this.setBoolean(parameterIndex_, ((Boolean)x_).booleanValue());

		} else if (x_ instanceof java.lang.Integer) {

			this.setInt(parameterIndex_, ((Integer)x_).intValue());

		} else if (x_ instanceof java.lang.Long) {

			this.setLong(parameterIndex_, ((Long)x_).longValue());

		} else if (x_ instanceof java.lang.Float) {

			this.setFloat(parameterIndex_, ((Float)x_).floatValue());

		} else if (x_ instanceof java.lang.Double) {

			this.setDouble(parameterIndex_, ((Double)x_).doubleValue());

		} else if (x_ instanceof java.math.BigDecimal) {

			this.setBigDecimal(parameterIndex_, (java.math.BigDecimal)x_);

		} else if (x_ instanceof java.lang.String) {

			this.setString(parameterIndex_, (String)x_);

		} else if (x_ instanceof java.sql.Date) {

			this.setDate(parameterIndex_, (java.sql.Date)x_);

		} else if (x_ instanceof java.sql.Time) {

			this.setTime(parameterIndex_, (java.sql.Time)x_);

		} else if (x_ instanceof java.sql.Timestamp) {

			this.setTimestamp(parameterIndex_, (java.sql.Timestamp)x_);

		} else if (x_ instanceof byte[]) {

			this.setBytes(parameterIndex_, (byte[])x_);

		} else if (x_ instanceof LanguageData) {

			this.setLanguage(parameterIndex_, (LanguageData)x_);

		} else if (x_ instanceof java.sql.Array) {

			this.setArray(parameterIndex_, (java.sql.Array)x_);

		} else if (x_ instanceof java.sql.Blob) {

			this.setBlob(parameterIndex_, (java.sql.Blob)x_);

		} else if (x_ instanceof java.sql.Clob) {

			this.setClob(parameterIndex_, (java.sql.Clob)x_);

		} else if (x_ instanceof java.sql.Ref) {

			this.setRef(parameterIndex_, (java.sql.Ref)x_);

		} else if (x_ instanceof java.net.URL) {

			this.setURL(parameterIndex_, (java.net.URL)x_);

		} else {

			// [YET!] SQLSTATE は、
			//        feature not supported - data type is not supported
			//        (0A503) ※ 仮
			throw new NotSupported();
		}
	}

	/**
	 * この <code>PreparedStatement</code> オブジェクトの、
	 * あらゆる種類の SQL 文を実行します。 <code>PreparedStatement</code>
	 * オブジェクトで用意された文には複数の結果を返すものがあります。
	 * <code>execute</code> メソッドは、 <code>executeQuery</code> メソッドと
	 * <code>executeUpdate</code> メソッドによって処理される、
	 * より簡単な形式の文と同様に、複雑な文も処理します。
	 * <P>
	 * <code>execute</code> メソッドは、 <code>boolean</code> を返し
	 * 最初の結果の形式を示します。 <code>getResultSet</code> メソッドまたは
	 * <code>getUpdateCount</code> メソッドを呼び出して結果を取得します。
	 * そして、 <code>getMoreResults</code> メソッドを呼び出して
	 * 引き続き任意の結果の取得 (複数可) に移動します。
	 *
	 * @return	最初の結果が <code>java.sql.ResultSet</code> オブジェクトの
	 *			場合は <code>true</code> 。更新カウントであるか、
	 *			または結果がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはこのメソッドに引数が指定された場合。
	 */
	public boolean execute()
		throws java.sql.SQLException
	{
		if (super._connection != null && super._connection.getMasterID() < jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION4) {
			throw new NotSupported();
		}
		executeStatement(super._connection.isReadOnly(), true /* updating */);
		return super._resultSet.isLast() == false;
	}

	/**
	 * この <code>PreparedStatement</code> オブジェクトのコマンドのバッチに、
	 * パラメータのセットを追加します。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void addBatch()
		throws java.sql.SQLException
	{
		if (this._batchParameters == null) {
			this._batchParameters = new java.util.Vector();
		}

		this._batchParameters.add(this._parameter.clone());
		this.clearParameters();
	}

	/**
	 * この <code>PreparedStatement</code> オブジェクトの現在の
	 * パラメータリストを空にします。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void clearBatch() throws java.sql.SQLException
	{
		this._batchParameters = null;
	}

	/**
	 * 設定されているバッチ更新用のパラメータをデータベースに送信して
	 * バッチ更新を実行し、すべてのパラメータが正常に実行されると、
	 * 更新カウントの配列を返します。
	 * <P>
	 * <B>注:</B>
	 * 一旦更新に失敗すると以後のコマンドは実行されません。
	 * また、自動コミットモードを解除するかどうかはアプリケーションに
	 * 委ねます。
	 * <P>
	 * <B>注:</B>
	 * バッチ更新後、この <code>PreparedStatement</code> オブジェクトの現在の
	 * パラメータリストは自動的に空となります。
	 *
	 * @return	バッチ内のコマンドごとに 1 つの要素が格納されている
	 *			更新カウントの配列。
	 *			配列の要素はコマンドがバッチに追加された順序で並べられる。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 *			データベースに送信されたコマンドの 1 つが正常に
	 *			実行されなかった場合や、結果セットを返そうとすると
	 *			<code>BatchUpdateException</code> ( <code>SQLException</code>
	 *			のサブクラス) がスローされる。
	 */
	public int[] executeBatch() throws java.sql.SQLException
	{
		if (this._batchParameters == null) return new int[0];

		int	numberOfParameters = this._batchParameters.size();
		int[]	updateCounts = new int[numberOfParameters];
		int	i = 0;

		try {

			for (i = 0; i < numberOfParameters; i++) {
				this._parameter =
					(DataArrayData)this._batchParameters.elementAt(i);
				updateCounts[i] = this.executeUpdate();
			}

		} catch (java.sql.SQLException	sqle) {

			int[]	justBeforeUpdateCounts = new int[i];
			for (int j = 0; j < i; j++) {
				justBeforeUpdateCounts[j] = updateCounts[j];
			}
			throw new java.sql.BatchUpdateException(sqle.getMessage(),
													sqle.getSQLState(),
													sqle.getErrorCode(),
													justBeforeUpdateCounts);
		} finally {

			this.clearBatch();
		}

		return updateCounts;
	}

	/**
	 * 指定されたパラメータを、指定された文字数である指定された
	 * <code>java.io.Reader</code> オブジェクトに設定します。
	 * <code>java.sql.Types.LONGVARCHAR</code> パラメータに
	 * 非常に大きな UNICODE 値を入力するときには、
	 * <code>java.io.Reader</code> オブジェクトを介して
	 * 送るほうが現実的です。ファイルの終わりに達するまで必要に応じて
	 * ストリームからデータが読み込まれます。JDBC ドライバは、
	 * データを UNICODE からデータベースの char 形式に変換します。
	 * <P>
	 * <B>注:</B>
	 * このストリームオブジェクトは、標準の Java ストリームオブジェクト、
	 * または標準インタフェースを実装する独自のサブクラスの
	 * どちらでもかまいません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	reader_
	 *			Unicode データを格納する
	 *			<code>java.io.Reader</code> オブジェクト。
	 * @param	length_
	 *			ストリーム内の文字数。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setCharacterStream(int				parameterIndex_,
								   java.io.Reader	reader_,
								   int				length_)
		throws java.sql.SQLException
	{
		if (reader_ == null) {
			setNull(parameterIndex_);
			return;
		}

		try {

			char[]	buffer = new char[length_];
			reader_.read(buffer, 0, length_);
			StringData	x = new StringData(new String(buffer, 0, length_));

			this._parameter.setElement(parameterIndex_ - 1, x);

		} catch (java.io.IOException	ioe) {

			// [YET!] SQLSTATE は、
			//        system error - ???
			//        (60???)
			throw new BadArgument();
		}
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータを指定された
	 * <code>java.sql.Types.REF(&lt;structured-type&gt;)</code> 値に
	 * 設定します。
	 * データベースに送るときに、ドライバはこれを
	 * SQL <code>java.sql.Types.REF</code> 値に
	 * 変換します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>REF</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			SQL <code>java.sql.Types.REF</code> 値。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setRef(int			parameterIndex_,
					   java.sql.Ref	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL REF 型は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - data type is not supported
		//        (0A503) ※ 仮
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータを指定された <code>java.sql.Blob</code>
	 * オブジェクトに設定します。データベースに送るときに、ドライバはこれを
	 * SQL <code>java.sql.Types.BLOB</code> 値に変換します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>BLOB</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			SQL <code>java.sql.Types.BLOB</code> 値をマッピングする
	 *			<code>java.sql.Blob</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setBlob(int				parameterIndex_,
						java.sql.Blob	x_)
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
	 * 指定されたパラメータを指定された <code>java.sql.Clob</code>
	 * オブジェクトに設定します。データベースに送るときに、ドライバはこれを
	 * SQL <code>java.sql.Types.CLOB</code> 値に変換します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>CLOB</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			SQL <code>java.sql.Types.CLOB</code> 値をマッピングする
	 *			<code>java.sql.Clob</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setClob(int				parameterIndex_,
						java.sql.Clob	x_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL CLOB 型は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - data type is not supported
		//        (0A503) ※ 仮
		throw new NotSupported();
	}

	/**
	 * 指定されたパラメータを指定された <code>java.sql.Array</code>
	 * オブジェクトに設定します。データベースに送るときに、ドライバはこれを
	 * SQL <code>java.sql.Types.ARRAY</code> 値に変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			SQL <code>java.sql.Types.ARRAY</code> 値をマッピングする
	 *			<code>java.sql.Array</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setArray(int			parameterIndex_,
						 java.sql.Array	x_)
		throws java.sql.SQLException
	{
		if (x_ == null) {
			setNull(parameterIndex_);
			return;
		}

		if (x_ instanceof jp.co.ricoh.doquedb.jdbc.Array) {

			DataArrayData x
				= ((jp.co.ricoh.doquedb.jdbc.Array)x_).getDataArrayData();
			this._parameter.setElement(parameterIndex_ - 1, x);

		} else {

			DataArrayData	arrayColumn = new DataArrayData();

			Object[]	x = (Object[])x_.getArray();
			int	numberOfElements = x.length;
			for (int i = 0; i < numberOfElements; i++) {

				Data	element = Array.convertToTRMeisterData(x[i]);
				if (element == null) {
					// [YET!] SQLSTATE は、
					//        feature not supported - data type is
					//                                not supported
					//        (0A503) ※ 仮
					throw new NotSupported();
				}
				arrayColumn.addElement(element);
			}

			this._parameter.setElement(parameterIndex_ - 1, arrayColumn);
		}
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>PreparedStatement</code> が実行されるときに返される
	 * <code>java.sql.ResultSet</code> オブジェクトの列に関する情報を格納する
	 * <code>java.sql.ResultSetMetaData</code> オブジェクトを取得します。
	 * <P>
	 * <code>java.sql.PreparedStatement</code> オブジェクトは
	 * プリコンパイルされるので、
	 * 実行されずに返される <code>java.sql.ResultSet</code> について
	 * 知ることが可能です。
	 * したがって、 <code>java.sql.PreparedStatement</code> について
	 * <code>getMetaData</code> メソッドの実行を待ち、それから返された
	 * <code>java.sql.ResultSet</code> について
	 * <code>ResultSet.getMetaData</code> メソッドを呼び出すのではなく、
	 * <code>getMetaData</code> メソッドを呼び出すことができます。
	 * <P>
	 * <B>注:</B>
	 * このメソッドの使用は、ドライバによっては
	 * 基本となる DBMS サポートが不足しているため
	 * 負荷が大きくなる場合があります。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @return	<code>java.sql.ResultSet</code> オブジェクトの列の記述、
	 *			またはドライバが <code>java.sql.ResultSetMetaData</code>
	 *			オブジェクトを返すことができない場合は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSetMetaData getMetaData()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!]
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された <code>java.util.Calendar</code> オブジェクトを使用して、
	 * 指定されたパラメータを指定された <code>java.sql.Date</code> 値に
	 * 設定します。ドライバは <code>java.util.Calendar</code> オブジェクトを
	 * 使用して SQL <code>java.sql.Types.DATE</code> 値を作成し、
	 * 続いてそれをデータベースに送ります。
	 * <code>java.util.Calendar</code> オブジェクトを使用すると、
	 * ドライバはカスタムタイムゾーンを考慮して日付を計算できます。
	 * <code>java.util.Calendar</code> オブジェクトを指定しない場合、
	 * ドライバは、アプリケーションで実行される仮想マシンのタイムゾーンである
	 * デフォルトのタイムゾーンを使用します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @param	cal_
	 *			ドライバが日付を作成するために使用する
	 *			<code>java.util.Calendar</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setDate(int					parameterIndex_,
						java.sql.Date		x_,
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
	 * 指定された <code>java.util.Calendar</code> オブジェクトを使用して、
	 * 指定されたパラメータを指定された <code>java.sql.Time</code> 値に
	 * 設定します。ドライバは <code>java.util.Calendar</code> オブジェクトを
	 * 使用して SQL <code>java.sql.Types.TIME</code> 値を作成し、
	 * 続いてそれをデータベースに送ります。
	 * <code>java.util.Calendar</code> オブジェクトを使用すると、
	 * ドライバはカスタムタイムゾーンを考慮して時刻を計算できます。
	 * <code>java.util.Calendar</code> オブジェクトを指定しない場合、
	 * ドライバは、アプリケーションで実行される仮想マシンのタイムゾーンである
	 * デフォルトのタイムゾーンを使用します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @param	cal_
	 *			ドライバが時刻を作成するために使用する
	 *			<code>java.util.Calendar</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setTime(int					parameterIndex_,
						java.sql.Time		x_,
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
	 * 指定された <code>java.util.Calendar</code> オブジェクトを使用して、
	 * 指定されたパラメータを指定された <code>java.sql.Timestamp</code> 値に
	 * 設定します。ドライバは <code>java.util.Calendar</code> オブジェクトを
	 * 使用して SQL <code>java.sql.Types.TIMESTAMP</code> 値を作成し、
	 * 続いてそれをデータベースに送ります。
	 * <code>java.util.Calendar</code> オブジェクトを使用すると、
	 * ドライバはカスタムタイムゾーンを考慮してタイムスタンプを計算できます。
	 * <code>java.util.Calendar</code> オブジェクトを指定しない場合、
	 * ドライバは、アプリケーションで実行される仮想マシンのタイムゾーンである
	 * デフォルトのタイムゾーンを使用します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @param	cal_
	 *			ドライバがタイムスタンプを作成するために使用する
	 *			<code>java.util.Calendar</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setTimestamp(int				parameterIndex_,
							 java.sql.Timestamp	x_,
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
	 * <B>[制限あり]</B>
	 * 指定されたパラメータを SQL <code>NULL</code> に設定します。
	 * ユーザ定義型および <code>java.sql.Types.REF</code> 型のパラメータでは、
	 * このバージョンのメソッド <code>setNull</code> メソッドを
	 * 使用しなければなりません。
	 * ユーザ定義型の例には、 <code>java.sql.Types.STRUCT</code> 、
	 * <code>java.sql.Types.DISTINCT</code> 、
	 * <code>java.sql.Types.JAVA_OBJECT</code> 、
	 * および名前付き配列があります。
	 * <P>
	 * <B>注:</B>
	 * 移植性を実現するためには、アプリケーションは <code>NULL</code> の
	 * ユーザ定義パラメータまたは <code>java.sql.Types.REF</code> パラメータを
	 * 指定するときに SQL 型コードおよび完全指定の SQL 型名を
	 * 指定しなければなりません。
	 * ユーザ定義型の場合、名前はパラメータ自体の型名です。
	 * <code>java.sql.Types.REF</code> パラメータの場合、
	 * 名前は参照される型の型名です。
	 * JDBC ドライバが型コードまたは型名の情報を必要としない場合、
	 * それは無視されます。ユーザ定義パラメータおよび
	 * <code>java.sql.Types.REF</code> パラメータを対象としていますが、
	 * このメソッドは任意の JDBC 型の <code>null</code> パラメータを
	 * 設定するために使用できます。
	 * パラメータがユーザ定義の型または <code>java.sql.Types.REF</code> 型を
	 * 持たない場合、指定された <code>typeName_</code> は無視されます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * <code>sqlType_</code> および <code>typeName_</code> は無視されます。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	sqlType_
	 *			<code>java.sql.Types</code> で定義される、
	 *			JDBC 型と呼ばれる、汎用の SQL 型を識別するためのコード。
	 * @param	typeName_
	 *			SQL ユーザ定義型の完全指定の名前。
	 *			パラメータがユーザ定義型でも
	 *			<code>java.sql.Types.REF</code> でもない場合は
	 *			無視される。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setNull(int		parameterIndex_,
						int		sqlType_,
						String	typeName_)
		throws java.sql.SQLException
	{
		setNull(parameterIndex_);
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータを指定された <code>java.net.URL</code> 値に
	 * 設定します。データベースに送るときに、ドライバはこれを
	 * SQL <code>java.sql.Types.DATALINK</code> 値に変換します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>DATALINK</code> 型を
	 * サポートしていないため、このメソッドはサポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			設定される java.net.URL オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setURL(int			parameterIndex_,
					   java.net.URL	x_)
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
	 * この <code>PreparedStatement</code> オブジェクトの
	 * パラメータの数、型、およびプロパティを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @return	この <code>PreparedStatement</code> オブジェクトの
	 *			パラメータの数、型、およびプロパティについての情報を格納する
	 *			<code>ParameterMetaData</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		jp.co.ricoh.doquedb.jdbc.ParameterMetaData
	 */
	public java.sql.ParameterMetaData getParameterMetaData()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!]
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 単一の <code>java.sql.ResultSet</code>オブジェクトを返す、
	 * 指定された SQL文を実行します。
	 * <P>
	 * <B>注:</B>
	 * このメソッドは、
	 * <code>PreparedStatement</code> オブジェクトに対して使用できません。
	 *
	 * @param	sql_
	 *			データベースに送られる SQL 文。
	 *			通常静的 SQL <code>SELECT</code> 文。
	 * @return	指定されたクエリーによって作成されたデータを含む
	 *			<code>java.sql.ResultSet</code> オブジェクト。
	 *			<code>null</code> にはならない。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定された SQL 文が単一の
	 *			<code>java.sql.ResultSet</code> オブジェクト以外のものを
	 *			生成する場合。
	 */
	public java.sql.ResultSet executeQuery(String sql_)
		throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された SQL 文を実行します。SQL 文は、<code>INSERT</code> 文、
	 * <code>UPDATE</code> 文、 <code>DELETE</code> 文、
	 * または SQL DDL 文のような何も返さない SQL 文の場合があります。
	 * <P>
	 * <P>
	 * <B>注:</B>
	 * このメソッドは、
	 * <code>PreparedStatement</code> オブジェクトに対して使用できません。
	 *
	 * @param	sql_
	 *			SQL <code>INSERT</code> 文、 <code>UPDATE</code> 文、
	 *			または <code>DELETE</code> 文、
	 *			あるいは何も返さない SQL 文。
	 * @return	<code>INSERT</code> 文、 <code>UPDATE</code> 文、
	 *			<code>DELETE</code> 文の場合は行数。
	 *			何も返さない SQL 文の場合は <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定された SQL 文が
	 *			<code>java.sql.ResultSet</code> オブジェクトを生成する場合。
	 */
	public int executeUpdate(String sql_)
		throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * 自動的にクローズされるときに <code>java.sql.Statement</code>
	 * オブジェクトのデータベースと JDBC リソースが解放されるのを
	 * 待つのではなく、ただちにそれらを解放します。
	 * データベースのリソースを占有するのを避けるために、
	 * 通常は、作業が終了したらすぐにリソースを解放するようにしてください。
	 * <P>
	 * すでにクローズされた <code>java.sql.Statement</code> オブジェクトで
	 * <code>close</code> メソッドを呼び出すと、操作は行われません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void close() throws java.sql.SQLException
	{
		super.close();

		if (_prepareStatement != null) {

			try {

				_prepareStatement.close();

			} catch (java.io.IOException e) {

				// ignore

			} finally {

				_prepareStatement = null;
			}
		}
	}

	/**
	 * 指定されたパラメータを SQL <code>NULL</code> に設定します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setNull(int	parameterIndex_) throws java.sql.SQLException
	{
		this._parameter.setElement(parameterIndex_ - 1, new NullData());
	}

	/**
	 * このオブジェクトへの参照がないと、ガベージコレクションによって
	 * 判断された時に、ガベージコレクタによって呼び出されます。
	 *
	 * @throws	java.lang.Throwable
	 *			このメソッドで生じた例外。
	 */
	protected void finalize() throws java.lang.Throwable
	{
		this.close();
	}

	/**
	 * sessionに対してexecutePrepareStatementを実行します。
	 * PreparedStatementのexecuteQuery、executeUpdate, executeから呼ばれます。
	 *
	 * @param	readOnly_
	 *			暗黙のトランザクションが必要なときREAD ONLYにするならtrue。
	 * @param	updating_
	 *			結果のResultSetが更新カウントの設定を必要とする場合はtrue。
	 * @return	指定されたクエリーによって作成されたデータを含む
	 *			<code>java.sql.ResultSet</code> オブジェクト。
	 *			<code>null</code> にはならない。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	private java.sql.ResultSet executeStatement(boolean readOnly_,
												boolean updating_)
		throws java.sql.SQLException
	{
		try {

			// When auto-commit mode is not set and no transaction is started,
			// a transaction should be started here.

			if (_connection.getAutoCommit() == false &&
				_connection.inTransaction() == false) {

				// start new transaction according to connection setting
				_connection.beginTransaction();
			}

			jp.co.ricoh.doquedb.client.ResultSet	resultSet =
				super._connection.getSession().executePrepareStatement(
												   _prepareStatement, _parameter);

			super.setResultSet(new ResultSet(this, resultSet));

			super._updating = updating_;

			return super._resultSet;

		} catch (java.io.IOException ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (java.lang.ClassNotFoundException cnfe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			Unexpected	ue = new Unexpected();
			ue.initCause(cnfe);
			throw ue;
		}
	}

	/**
	 * 指定されたパラメータを指定された
	 * <code>jp.co.ricoh.doquedb.common.LanguageData</code> 値に設定します。
	 * データベースに送るときに、ドライバはこれを
	 * <code>jp.co.ricoh.doquedb.common.DataType.LANGUAGE</code> 値に
	 * 変換します。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @param	x_
	 *			パラメータ値。
	 * @throws java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	private void setLanguage(int	parameterIndex_,
							 LanguageData	x_)
		throws java.sql.SQLException
	{
		this._parameter.setElement(parameterIndex_ - 1, x_);
	}

	/**
	 * この <code>PreparedStatement</code> オブジェクトがクローズ
	 * されているかどうかを取得します。
	 */
	@Override
	public boolean isClosed() throws SQLException {

		return (_prepareStatement == null) ? true : false;
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setPoolable(boolean poolable) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * 常に <code>false</code> を返す
	 */
	@Override
	public boolean isPoolable() throws SQLException {
		// サポート外
		return false;
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
	 */
	@Override
	public void setRowId(int parameterIndex, RowId x) throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * 指定されたパラメータを、指定された Java の <code>String</code>
	 * オブジェクトに設定します。
	 * データベースに送るときに、ドライバはこでをSQL <code>NCHAR</code>、
	 * <code>NVARCHAR</code> 値に変換します。
	 */
	@Override
	public void setNString(int parameterIndex, String value)
			throws SQLException {
		setString(parameterIndex, value);
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setNCharacterStream(int parameterIndex, Reader value,
			long length) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setNClob(int parameterIndex, NClob value) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setClob(int parameterIndex, Reader reader, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setBlob(int parameterIndex,
						InputStream inputStream, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setNClob(int parameterIndex, Reader reader, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setSQLXML(int parameterIndex, SQLXML xmlObject)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setAsciiStream(int parameterIndex, InputStream x, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setBinaryStream(int parameterIndex, InputStream x, long length)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setCharacterStream(int parameterIndex, Reader reader,
			long length) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setAsciiStream(int parameterIndex, InputStream x)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setBinaryStream(int parameterIndex, InputStream x)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setCharacterStream(int parameterIndex, Reader reader)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setNCharacterStream(int parameterIndex, Reader value)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setClob(int parameterIndex, Reader reader) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setBlob(int parameterIndex, InputStream inputStream)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void setNClob(int parameterIndex, Reader reader)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2015, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
