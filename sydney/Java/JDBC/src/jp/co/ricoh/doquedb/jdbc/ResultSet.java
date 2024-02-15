// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSet.java -- JDBC の結果セットクラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2016, 2023 Ricoh Company, Ltd.
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

import java.util.HashMap;

import jp.co.ricoh.doquedb.common.ArrayData;
import jp.co.ricoh.doquedb.common.Data;
import jp.co.ricoh.doquedb.common.DataArrayData;
import jp.co.ricoh.doquedb.common.DataType;
import jp.co.ricoh.doquedb.exception.ClassCast;
import jp.co.ricoh.doquedb.exception.ConnectionRanOut;
import jp.co.ricoh.doquedb.exception.EntryNotFound;
import jp.co.ricoh.doquedb.exception.NotSupported;
import jp.co.ricoh.doquedb.exception.Unexpected;

/**
 * JDBC の結果セットクラス。
 *
 */
public class ResultSet extends DefaultResultSet
{
	/** <code>jp.co.ricoh.doquedb.jdbc.Statement</code> オブジェクト。*/
	private Statement	_statement;

	/**
	 * 結果セットオブジェクト。
	 *
	 * @see	jp.co.ricoh.doquedb.client.ResultSet
	 */
	private jp.co.ricoh.doquedb.client.ResultSet	_resultSet;

	/**
	 * この結果セットのメタデータ。
	 *
	 * @see	jp.co.ricoh.doquedb.jdbc.ResultSetMetaData
	 */
	private ResultSetMetaData	_metaData;

	/**
	 * メタデータの取得を行ったかどうか
	 */
	private boolean _getMetaData;

	/**
	 * isLast メソッドにより読み込んだ、次の行データ。
	 */
	private DataArrayData	_nextRow;

	/**
	 * 現在行にisLastを実行したかどうか
	 */
	private boolean _checkIsLast;

	/**
	 * カラム名と列インデックスのハッシュマップ
	 */
	private HashMap _hashMap;

	/**
	 * 更新カウント
	 */
	private int _updateCount;

	/**
	 * 新しく結果セットオブジェクトを作成します。
	 *
	 * @param	statement_
	 *			ステートメントオブジェクト。
	 * @param	resultSet_
	 *			結果セットオブジェクト。
	 */
	ResultSet(Statement 							statement_,
			  jp.co.ricoh.doquedb.client.ResultSet	resultSet_)
	{
		super();

		this._statement = statement_;
		this._resultSet = resultSet_;
		this._metaData = null;
		this._getMetaData = false;
		this._nextRow = null;
		this._checkIsLast = false;
		this._hashMap = null;
		this._updateCount = -1;
	}

	/**
	 * <B>[制限あり]</B>
	 * カーソルを現在の位置から 1 行下に移動します。
	 * <code>java.sql.ResultSet</code> のカーソルは、初期状態では最初の行の前に
	 * 位置付けられています。メソッド <code>next</code> の最初の
	 * 呼び出しによって、最初の行が現在の行になります。
	 * 2 番目の呼び出しによって 2 行目が現在の行になり、以降同様に続きます。
	 * <P>
	 * 現在の行で入力ストリームがオープンしている場合、
	 * <code>next</code> メソッドへの呼び出しは暗黙的にそのストリームを
	 * クローズさせます。新しい行が読み込まれるときに、
	 * <code>java.sql.ResultSet</code> オブジェクトの警告チェーンは
	 * クリアされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、警告は存在しません。
	 *
	 * @return	新しい現在の行が有効な場合は <code>true</code> 、
	 *			それ以上行がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean next() throws java.sql.SQLException
	{
		if (this._resultSet == null) {
			// close() により、既に閉じられている
			// この場合、何もせずに常に false を返す。
			return false;
		}

		super.closeInputStream();

		try {

			if (super._rowNumber == -1) return false;

			boolean	last = false;

			if (this._checkIsLast == true) {

				// メソッド isLast() で既に次の行を得ている。

				super._row = this._nextRow;
				last = (super._row == null);
				this._nextRow = null;
				this._checkIsLast = false;

			} else {

				if (super._row == null)
					super._row = new DataArrayData();

				int status;
				do
				{
					 status = this._resultSet.getNextTuple(super._row);
					_getMetaData = true;

					if (status == jp.co.ricoh.doquedb.client.ResultSet.META_DATA)
					{
						_metaData = new ResultSetMetaData(
							this._resultSet.getMetaData());
					}

				} while (status
						 == jp.co.ricoh.doquedb.client.ResultSet.META_DATA);

				last = (status != jp.co.ricoh.doquedb.client.ResultSet.DATA);

			}

			if (last) {
				if (_statement.isUpdating()) {
					// set update count
					_updateCount = super.getRow();
				}
				super._row = null;
				super._rowNumber = -1;
				// 実行ステータスを得るまで読み飛ばす
				int status = this._resultSet.getStatus(false /* stop when any status is gotten */);
				if (_statement.isUpdating() == false
					&& status == jp.co.ricoh.doquedb.client.ResultSet.HAS_MORE_DATA) {
					// [NOTES]
					// isUpdating == false when ResultSet is obtained by executeQuery.
					// In this case, more than one statement can not be executed.
					// So, skip until success/error is obtained

					_resultSet.getStatus(true /* stop when status is success/error */);
				}
				return false;
			}

			super._rowNumber++;
			return true;

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
	 * 自動的にクローズされるとき、これを待つのではなく、
	 * ただちに <code>ResultSet</code> オブジェクトのデータベースと
	 * JDBC リソースを解放します。
	 * <P>
	 * <B>注:</B>
	 * <code>java.sql.ResultSet</code> オブジェクトは、
	 * このオブジェクトを生成した <code>java.sql.Statement</code>
	 * オブジェクトが閉じられるとき、再実行されるとき、
	 * または一連の複数の結果からつぎの結果を取り出すのに使用されるときに、
	 * その <code>java.sql.Statement</code> によって自動的にクローズされます。
	 * <code>java.sql.ResultSet</code> オブジェクトが
	 * ガベージコレクトされるときにも自動的にクローズされます。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void close() throws java.sql.SQLException
	{
		super.close();

		try {

			if (this._resultSet != null) this._resultSet.close();

		} catch (java.io.IOException ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	ue = new ConnectionRanOut();
			ue.initCause(ioe);
			throw ue;

		} finally {

			this._resultSet = null;
		}
	}

	/**
	 * 指定された <code>ResultSet</code> の列名を
	 * <code>ResultSet</code> 列インデックスにマッピングします。
	 *
	 * @param	columnName_
	 *			列の SQL 名。
	 * @return	指定された列名の列インデックス。
	 * @throws	java.sql.SQLException
	 *			<code>DefaultResultSet</code> オブジェクトに
	 *			<code>columnName</code> が含まれない場合、
	 *			またはデータベースアクセスエラーが発生した場合。
	 */
	public int findColumn(String columnName_) throws java.sql.SQLException
	{
		if (_metaData == null)
			throw new NotSupported();
		if (_hashMap == null)
		{
			_hashMap = new HashMap();
			// 作成する
			for (int i = 0; i < _metaData.getColumnCount(); ++i)
			{
				Integer old = (Integer)_hashMap.put(
					_metaData.getColumnName(i+1).toLowerCase(),
					new Integer(i+1));
				if (old != null)
				{
					// 同じキーがあったので戻す
					_hashMap.put(_metaData.getColumnName(i+1).toLowerCase(),
								 old);
				}
			}
		}

		Integer index = (Integer)_hashMap.get(columnName_.toLowerCase());
		if (index == null)
			throw new EntryNotFound(columnName_);

		return index.intValue();
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの実行を中断します。
	 *
	 * @throws	java.sql.SQLException
	 *				データベースアクセスエラーが発生した場合。
	 */
	void cancel() throws java.sql.SQLException
	{
		if (this._resultSet != null) {

			try {

				this._resultSet.cancel();

			} catch (java.io.IOException	ioe) {

				// [YET!] SQLSTATE は、
				//        connection exception - ???
				//        (08???)
				ConnectionRanOut	croe = new ConnectionRanOut();
				croe.initCause(ioe);
				throw croe;

			} catch (java.lang.ClassNotFoundException	cnfe) {

				// [YET!] 何らかの SQLSTATE を割り当てるべき。
				Unexpected	ue = new Unexpected();
				ue.initCause(cnfe);
				throw ue;
			}
		}
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
		return getString(findColumn(columnName_));
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
		return getBoolean(findColumn(columnName_));
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
		return getByte(findColumn(columnName_));
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
		return getShort(findColumn(columnName_));
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
		return getInt(findColumn(columnName_));
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
		return getLong(findColumn(columnName_));
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
		return getFloat(findColumn(columnName_));
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
		return getDouble(findColumn(columnName_));
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
		return getBigDecimal(findColumn(columnName_), scale_);
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
		return getBytes(findColumn(columnName_));
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
		return getDate(findColumn(columnName_));
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
		return getTime(findColumn(columnName_));
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
		return getTimestamp(findColumn(columnName_));
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
		return getAsciiStream(findColumn(columnName_));
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
		return getUnicodeStream(findColumn(columnName_));
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
		return getBinaryStream(findColumn(columnName_));
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの列の数、型、およびプロパティを
	 * 取得します。
	 *
	 * @return	この <code>ResultSet</code> オブジェクトの列の記述。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSetMetaData getMetaData()
		throws java.sql.SQLException
	{
		if (_getMetaData == false)
			isLast();
		return this._metaData;
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
		return getObject(findColumn(columnName_));
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
		return getCharacterStream(findColumn(columnName_));
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
		return getBigDecimal(findColumn(columnName_));
	}

	/**
	 * カーソルがこの <code>ResultSet</code> オブジェクトの
	 * 最終行にあるかどうかを取得します。
	 * <P>
	 * <B>注:</B> 現在の行が結果セット内の最終行であるかどうかを判定するために
	 * 1 つ先の行を取り出すので、isLast メソッドの呼び出しは
	 * 負荷が大きくなります。
	 *
	 * @return	カーソルが最終行にある場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isLast() throws java.sql.SQLException
	{
		if (this._checkIsLast == true)
		{
			return (this._nextRow == null) ? true : false;
		}
		if (super._rowNumber == -1) {
			// 既に最後の行を通り過ぎている。
			return false;
		}

		try {

			DataArrayData	row = new DataArrayData();
			int	status;
			this._checkIsLast = true;

			do
			{
				status = this._resultSet.getNextTuple(row);
				_getMetaData = true;

				if (status == jp.co.ricoh.doquedb.client.ResultSet.META_DATA)
				{
					_metaData = new ResultSetMetaData(
						this._resultSet.getMetaData());
				}
			} while (status
					 == jp.co.ricoh.doquedb.client.ResultSet.META_DATA);

			if (status != jp.co.ricoh.doquedb.client.ResultSet.DATA) {
				this._nextRow = null;
				return true;
			}

			this._nextRow = row;
			return false;

		} catch (java.io.IOException	ioe) {

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
		updateNull(findColumn(columnName_));
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
		updateBoolean(findColumn(columnName_), x_);
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
		updateByte(findColumn(columnName_), x_);
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
		updateShort(findColumn(columnName_), x_);
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
		updateInt(findColumn(columnName_), x_);
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
		updateLong(findColumn(columnName_), x_);
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
		updateFloat(findColumn(columnName_), x_);
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
		updateDouble(findColumn(columnName_), x_);
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
		updateBigDecimal(findColumn(columnName_), x_);
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
		updateString(findColumn(columnName_), x_);
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
		updateBytes(findColumn(columnName_), x_);
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
		updateDate(findColumn(columnName_), x_);
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
		updateTime(findColumn(columnName_), x_);
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
		updateTimestamp(findColumn(columnName_), x_);
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
		updateAsciiStream(findColumn(columnName_), x_, length_);
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
		updateBinaryStream(findColumn(columnName_), x_, length_);
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
		updateCharacterStream(findColumn(columnName_), reader_, length_);
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
		updateObject(findColumn(columnName_), x_, scale_);
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
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void updateObject(String	columnName_,
							 Object	x_)
		throws java.sql.SQLException
	{
		updateObject(findColumn(columnName_), x_);
	}

	/**
	 * この <code>ResultSet</code> オブジェクトを生成した
	 * <code>java.sql.Statement</code> オブジェクトを取得します。
	 * <code>java.sql.DatabaseMetaData</code> メソッドなどの別の方法で
	 * 結果セットが生成された場合、このメソッドは <code>null</code> を
	 * 返します。
	 *
	 * @return	この <code>ResultSet</code> オブジェクトを生成した
	 *			<code>Statment</code> オブジェクト。
	 *			結果セットが別の方法で生成された場合は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Statement getStatement() throws java.sql.SQLException
	{
		return this._statement;
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの現在行にある
	 * 指定された列の値を、 Java プログラミング言語の
	 * <code>java.sql.Array</code> オブジェクトとして取得します。
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
		Data	column = super.getColumn(columnIndex_);

		if (column == null) return null;

		java.sql.Array	columnValue = null;

		switch (column.getType()) {
		case DataType.ARRAY:
			if (((ArrayData)column).getElementType() != DataType.DATA) {
				throw new ClassCast();
			}
			jp.co.ricoh.doquedb.common.ColumnMetaData columnMeta = null;
			if (_metaData != null)
				columnMeta = _metaData.getColumnMetaData(columnIndex_);
			columnValue = new Array((DataArrayData)column, columnMeta);
			break;
		default:
			throw new ClassCast();
		}

		return columnValue;
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
		return getObject(findColumn(columnName_), map_);
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
		return getRef(findColumn(columnName_));
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
		return getBlob(findColumn(columnName_));
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
		return getClob(findColumn(columnName_));
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
		return getArray(findColumn(columnName_));
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
		return getDate(findColumn(columnName_), cal_);
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
		return getTime(findColumn(columnName_), cal_);
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
		return getTimestamp(findColumn(columnName_), cal_);
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
		return getURL(findColumn(columnName_));
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
		updateRef(findColumn(columnName_), x_);
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
		updateBlob(findColumn(columnName_), x_);
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
		updateClob(findColumn(columnName_), x_);
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
		updateArray(findColumn(columnName_), x_);
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
		super.close();
		if (this._resultSet != null) this._resultSet.finalize();
	}

	/**
	 * データ取得中か調べます。
	 *
	 * @return データ取得中ならtrue。
	 */
	/* package */boolean isRunning()
	{
		return (_resultSet != null
				&&
				(_resultSet.getLastStatus() == jp.co.ricoh.doquedb.client.ResultSet.META_DATA
				 ||
				 _resultSet.getLastStatus() == jp.co.ricoh.doquedb.client.ResultSet.DATA
				 ||
				 _resultSet.getLastStatus() == jp.co.ricoh.doquedb.client.ResultSet.END_OF_DATA));
	}

	/**
	 * Statusを得るまで読み飛ばします。UpdateCountも更新します。
	 *
	 * @param  boolean bAll_
	 *			trueのときHAS_MORE_DATAも読み飛ばす
	 * @return 最終的に得られたStatus
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	/* package */int skipToStatus(boolean bAll_)
		throws java.sql.SQLException
	{
		if (_resultSet != null) {
			try {
				// skip until status is obtained
				int status = _resultSet.getLastStatus();
				int count = super.getRow();
				while (status == jp.co.ricoh.doquedb.client.ResultSet.META_DATA
					   ||
					   status == jp.co.ricoh.doquedb.client.ResultSet.DATA
					   ||
					   status == jp.co.ricoh.doquedb.client.ResultSet.END_OF_DATA
					   ||
					   (bAll_ && status == jp.co.ricoh.doquedb.client.ResultSet.HAS_MORE_DATA)) {
					status = _resultSet.getNextTuple(null);
					if (status == jp.co.ricoh.doquedb.client.ResultSet.DATA) {
						++count;
					}
				}
				if (_statement.isUpdating()) {
					// set update count
					_updateCount = count;
				}
				super._rowNumber = -1;
				return status;

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
		return jp.co.ricoh.doquedb.client.ResultSet.UNDEFINED;
	}

	/**
	 * 最後まで処理が終わったResultSetから更新カウントを取得します。
	 *
	 * @return	更新カウント。
	 *			executeまたはexecuteUpdateで作られたResultSetで、
	 *			Statusを得るまで処理した後オブジェクトを破棄するまでの間に1回だけ呼び出すことができる。
	 *			それ以外の場合は-1を返す。
	 */
	/* package */int getUpdateCount()
	{
		// if once called, return -1 after second call
		int result = _updateCount;
		_updateCount = -1;
		return result;
	}

	/**
	 * 複文の結果として次のResultSetが取得できるなら新しいResultSetを取得します。
	 *
	 * @return 次のResultSetが取得できるなら新しいResultSet、取得できないならnull。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	/* package */ResultSet getNextResultSet()
		throws java.sql.SQLException
	{
		if (_resultSet != null) {
			try {
				// skip until status is obtained
				int status = _resultSet.getStatus(false /* stop when any status is gotten */);

				// check whether next data can be obtained
				if (status == jp.co.ricoh.doquedb.client.ResultSet.HAS_MORE_DATA) {
					// save current resultset
					jp.co.ricoh.doquedb.client.ResultSet resultSet = _resultSet;
					_resultSet = null;
					// return new ResultSet
					return new ResultSet(_statement, resultSet);
				}

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
		return null;
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
