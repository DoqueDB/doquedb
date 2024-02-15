// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement.java -- JDBC のステートメントクラス
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

import java.sql.SQLException;

import jp.co.ricoh.doquedb.common.DataArrayData;
import jp.co.ricoh.doquedb.exception.BadArgument;
import jp.co.ricoh.doquedb.exception.Cancel;
import jp.co.ricoh.doquedb.exception.ConnectionRanOut;
import jp.co.ricoh.doquedb.exception.NotSupported;
import jp.co.ricoh.doquedb.exception.SessionNotAvailable;
import jp.co.ricoh.doquedb.exception.Unexpected;

/**
 * JDBC のステートメントクラス。
 * 静的 SQL 文を実行し、作成された結果を返すために使用されるオブジェクトです。
 *
 */
public class Statement implements java.sql.Statement
{
	/**
	 * この <code>Statement</code> オブジェクトを生成した、
	 * コネクションオブジェクト。
	 *
	 * @see	jp.co.ricoh.doquedb.jdbc.Connection
	 */
	jp.co.ricoh.doquedb.jdbc.Connection	_connection;

	/**
	 * 結果セット。
	 *
	 * @see	jp.co.ricoh.doquedb.jdbc.ResultSet
	 */
	jp.co.ricoh.doquedb.jdbc.ResultSet	_resultSet;

	/** executeUpdateまたはexecuteを実行したか */
	protected boolean _updating;

	/** 結果セットの型。 */
	private int	_resultSetType;

	/** 並行処理の種類。 */
	private int	_resultSetConcurrency;

	/** 保持機能の種類。 */
	private int	_resultSetHoldability;

	/** バッチ更新のための SQL 文を格納するベクター。 */
	private java.util.Vector	_batchCommands;

	/** クローズしたかどうか。 */
	private boolean	_isClosed;

	/** <code>_resultSet</code> がクローズされたときにこの <code>Statement</code> がクローズされるかどうか*/
	private boolean _isCloseOnCompletion;

	/** 警告オブジェクト。 */
	private java.sql.SQLWarning	_warnings;

	/**
	 * 新しくステートメントオブジェクトを作成します。
	 *
	 * @param	connection_
	 *			コネクションオブジェクト。
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
	 */
	Statement(Connection	connection_,
			  int			resultSetType_,
			  int			resultSetConcurrency_,
			  int			resultSetHoldability_)
	{
		this._connection = connection_;
		this._resultSetType = resultSetType_;
		this._resultSetConcurrency = resultSetConcurrency_;
		this._resultSetHoldability = resultSetHoldability_;
		this._resultSet = null;
		this._updating = false;
		this._batchCommands = null;
		this._isClosed = false;
		this._isCloseOnCompletion = false;
		this._warnings = null;
	}

	/**
	 * 単一の <code>java.sql.ResultSet</code> オブジェクトを返す、
	 * 指定された SQL 文を実行します。
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
	public java.sql.ResultSet executeQuery(String	sql_)
		throws java.sql.SQLException
	{
		return executeStatement(sql_, null, true /* read only */, false /* no update */);
	}

	/**
	 * <B>[制限あり]</B>
	 * 指定された SQL 文を実行します。SQL 文は、<code>INSERT</code> 文、
	 * <code>UPDATE</code> 文、 <code>DELETE</code> 文、
	 * または SQL DDL 文のような何も返さない SQL 文の場合があります。
	 * <P>
	 * <B>注:</B>
	 * <code>create table</code> 文などのスキーマ操作を伴う SQL 文は、
	 * 自動コミットモードで実行する必要があります。
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
	public int executeUpdate(String	sql_) throws java.sql.SQLException
	{
		executeStatement(sql_, null, false /* not read only */, true /* updating */);

		// skip until status is obtained with count up
		// [NOTES]
		// If status become HAS_MORE_DATA, more than one statements has been passed.
		// According to JDBC specifications, this case should be an error.
		// However, because JDBC driver can not parse SQL statement,
		//     DoqueDB JDBC driver does not cause errors and executes all the SQL statements.
		// The result will be sum of all the update counts.
		int status = _resultSet.skipToStatus(true /* skip has_more_data too */);

		if (status == jp.co.ricoh.doquedb.client.ResultSet.CANCELED) {
			throw new Cancel();
		}
		int count = _resultSet.getUpdateCount();

		closeResultSet();

		this._warnings = null;

		return count;
	}

	/**
	 * 自動的にクローズされるときに <code>Statement</code>オブジェクトの
	 * データベースと JDBC リソースが解放されるのを待つのではなく、
	 * ただちにそれらを解放します。
	 * データベースのリソースを占有するのを避けるために、
	 * 通常は、作業が終了したらすぐにリソースを解放するようにしてください。
	 * <P>
	 * すでにクローズされた <code>Statement</code> オブジェクトで
	 * <code>close</code> メソッドを呼び出すと、操作は行われません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void close() throws java.sql.SQLException
	{
		closeResultSet();
	}

	/**
	 * この <code>Statement</code> オブジェクトによって生成される
	 * <code>java.sql.ResultSet</code> オブジェクトの文字およびバイナリの
	 * 各列値に対し返される最大バイト数を取得します。
	 * この制限値は、
	 * <code>BINARY</code> 、
	 * <code>VARBINARY</code> 、
	 * <code>LONGVARBINARY</code> 、
	 * <code>CHAR</code> 、
	 * <code>VARCHAR</code> 、
	 * および <code>LONGVARCHAR</code> の各列にだけ適用されます。
	 * 制限値を超えたデータは通知なしに破棄されます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に <code>0</code> (無制限)を返します。
	 *
	 * @return	文字値およびバイナリ値を格納する列に対する
	 *			現在の列サイズの制限値。ゼロは無制限を意味する。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#setMaxFieldSize(int)
	 */
	public int getMaxFieldSize() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 無制限のみサポート。
		return 0;
	}

	/**
	 * <B>[サポート外]</B>
	 * 文字またはバイナリの値を格納する <code>java.sql.ResultSet</code> 列に
	 * 対する最大バイト数の制限値を、指定されたバイト数に設定します。
	 * この制限値は、
	 * <code>BINARY</code> 、
	 * <code>VARBINARY</code> 、
	 * <code>LONGVARBINARY</code> 、
	 * <code>CHAR</code> 、
	 * <code>VARCHAR</code> 、
	 * <code>LONGVARCHAR</code> の各フィールドだけに適用されます。
	 * 制限値を超えたデータは通知なしに破棄されます。
	 * 移植性を最大限にするには、256 より大きい値を使用します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	max_
	 *			バイト単位の新しい列サイズの制限値。ゼロは無制限を意味する。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または条件 <code>max_ >= 0</code> が満たされない場合。
	 * @see		#getMaxFieldSize()
	 */
	public void setMaxFieldSize(int	max_) throws java.sql.SQLException
	{
		if (max_ < 0) throw new BadArgument();

		// [NOT SUPPORTED!] なにもせず。
	}

	/**
	 * この <code>Statement</code> オブジェクトによって生成される
	 * <code>java.sql.ResultSet</code> オブジェクトが含むことのできる
	 * 最大の行数を取得します。制限値を超えた行は通知なしに除外されます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に <code>0</code> (無制限) を返します。
	 *
	 * @return	この <code>Statement</code> オブジェクトによって生成される
	 *			<code>java.sql.ResultSet</code> オブジェクトの現在の最大行数。
	 *			ゼロは無制限を意味する。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#setMaxRows(int)
	 */
	public int getMaxRows() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 無制限のみサポート。
		return 0;
	}

	/**
	 * <B>[サポート外]</B>
	 * 任意の <code>java.sql.ResultSet</code> オブジェクトが含むことのできる
	 * 最大行数の制限値を、指定された数に設定します。
	 * 制限値を超えた行は通知なしに除外されます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	max_
	 *			新しい最大行数の制限値。ゼロは無制限を意味する。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または条件 <code>max_ >= 0</code> が満たされない場合。
	 * @see		#getMaxRows()
	 */
	public void setMaxRows(int	max_) throws java.sql.SQLException
	{
		if (max_ < 0) throw new BadArgument();

		// [NOT SUPPORTED!] なにもせず。
	}

	/**
	 * <B>[サポート外]</B>
	 * エスケープの処理をオンまたはオフに設定します。
	 * エスケープスキャンニングがオンの場合 (デフォルト)、
	 * ドライバは SQL 文をデータベースに送る前にエスケープ置き換えを
	 * 実行します。
	 * <P>
	 * <B>注:</B>
	 * 用意された文は、通常呼び出す前に構文解析されているので、
	 * <code>PreparedStatements</code> に対しエスケープ処理を無効にしても
	 * 効果はありません。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	enable_
	 *			エスケープ処理を有効にする場合は <code>true</code> 、
	 *			無効にする場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setEscapeProcessing(boolean	enable_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] エスケープの処理は未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * ドライバが <code>Statement</code> オブジェクトの実行を待つ秒数を
	 * 取得します。
	 * この時間を経過すると、 <code>SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に <code>0</code> (無制限) を返します。
	 *
	 * @return	現在のクエリータイムアウトの制限値の秒数。
	 *			ゼロは無制限を意味する。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#setQueryTimeout(int)
	 */
	public int getQueryTimeout() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 無制限のみサポート。
		return 0;
	}

	/**
	 * <B>[サポート外]</B>
	 * ドライバが <code>Statement</code> オブジェクトの実行を待つ秒数を、
	 * 指定された秒数に設定します。
	 * この時間を経過すると、 <code>SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	seconds_
	 *			新しいクエリータイムアウトの制限値の秒数。
	 *			ゼロは無制限を意味する。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または条件 <code>seconds_ >= 0</code> が満たされない場合。
	 * @see		#getQueryTimeout()
	 */
	public void setQueryTimeout(int	seconds_) throws java.sql.SQLException
	{
		if (seconds_ < 0) throw new BadArgument();

		// [NOT SUPPORTED!] なにもせず。
	}

	/**
	 * DBMS およびドライバの両方が SQL 文の終了をサポートする場合に、
	 * この <code>Statement</code> オブジェクトを取り消します。
	 * このメソッドは、 1 つのスレッドが別のスレッドによって実行中の文を
	 * 取り消すのに使用できます。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public synchronized void cancel() throws java.sql.SQLException
	{
		if (this._resultSet != null) this._resultSet.cancel();
	}

	/**
	 * この <code>Statement</code> オブジェクトに関する呼び出しによって
	 * 報告される最初の警告を取得します。
	 * 後続の <code>Statement</code> オブジェクトの警告は、
	 * この <code>SQLWarning</code> オブジェクトにチェーンされます。
	 * <P>
	 * 警告チェーンは、文が (再) 実行されるたびに自動的にクリアされます。
	 * このメソッドはクローズされた <code>Statement</code> オブジェクトの
	 * 呼び出しには使用しません。使用するとSQLException がスローされます。
	 * <P>
	 * <B>注:</B>
	 * <code>java.sql.ResultSet</code> オブジェクトを処理中の場合、
	 * <code>java.sql.ResultSet</code> オブジェクトの読み込みに関連する警告は
	 * そのオブジェクトを生成した <code>Statement</code> オブジェクトではなく
	 * すべて <code>java.sql.ResultSet</code> オブジェクトにチェーンされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは警告は返しません。
	 *
	 * @return	最初の <code>SQLWarning</code> オブジェクト。
	 *			警告がない場合は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはこのメソッドがクローズされた文で呼び出された場合。
	 */
	public java.sql.SQLWarning getWarnings() throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		return this._warnings;
	}

	/**
	 * この <code>Statement</code> オブジェクトに関して報告された
	 * すべての警告をクリアします。このメソッドの呼び出しのあと、
	 * この <code>Statement</code> オブジェクトに対する新しい警告が
	 * 通知されるまで、<code>getWarnings</code> メソッドは
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
	 * 後続の <code>Statement</code> オブジェクトの <code>execute</code>
	 * メソッドによって使用される SQL カーソル名を指定された
	 * <code>String</code> に設定します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * このメソッドは何も実行しません。
	 *
	 * @param	name_
	 *			新しいカーソル名。接続内で一意にする必要がある。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setCursorName(String	name_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		String	reason = "cursor is not supporting.";
		// [YET!] SQLSTATE のサブクラスがまだ未設定。"000" は仮。
		appendWarning(reason, "000");
	}

	/**
	 * 複数の結果を返す可能性のある指定された SQL 文を実行します。
	 *
	 * @param	sql_
	 *			任意のSQL文
	 * @return	次の結果がResultSetオブジェトの場合は <code>true</code> 、
	 *			更新カウンタであるか、
	 *			または結果がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @throws	jp.co.ricoh.doquedb.exception.NotSupported
	 *			このメソッドをサポートしていないバージョンのサーバーに接続している場合。
	 * @see		#getResultSet()
	 * @see		#getUpdateCount()
	 * @see		#getMoreResults()
	 */
	public boolean execute(String sql_) throws java.sql.SQLException
	{
		if (_connection != null && _connection.getMasterID() < jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION4) {
			throw new NotSupported();
		}
		executeStatement(sql_, null, _connection.isReadOnly(), true /* updating */);
		return _resultSet.isLast() == false;
	}

	/**
	 * <code>java.sql.ResultSet</code> オブジェクトとして現在の結果を
	 * 取得します。
	 *
	 * @return	<code>java.sql.ResultSet</code>オブジェクトとしての現在の結果。
	 *			更新カウントであるか、
	 *			または結果がない場合は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @throws	jp.co.ricoh.doquedb.exception.NotSupported
	 *			このメソッドをサポートしていないバージョンのサーバーに接続している場合。
	 * @see		#execute(java.lang.String)
	 */
	public java.sql.ResultSet getResultSet() throws java.sql.SQLException
	{
		if (_connection != null && _connection.getMasterID() < jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION4) {
			throw new NotSupported();
		}
		return _resultSet;
	}

	/**
	 * 更新カウントとして現在の結果を取得します。
	 *
	 * @return	更新カウントとしての現在の結果。
	 *			結果は <code>java.sql.ResultSet</code> オブジェクトであるか、
	 *			または結果がない場合は -1 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @throws	jp.co.ricoh.doquedb.exception.NotSupported
	 *			このメソッドをサポートしていないバージョンのサーバーに接続している場合。
	 * @see		#execute(java.lang.String)
	 */
	public int getUpdateCount() throws java.sql.SQLException
	{
		if (_connection != null && _connection.getMasterID() < jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION4) {
			throw new NotSupported();
		}
		if (_resultSet != null && _resultSet.isRunning() == false) {
			return _resultSet.getUpdateCount();
		}
		return -1;
	}

	/**
	 * <code>Statement</code> オブジェクトの次の結果に移動します。
	 *
	 * @return	次の結果が <code>java.sql.ResultSet</code>オブジェクトの場合は
	 *			<code>true</code> 。更新カウントであるか、
	 *			または結果がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @throws	jp.co.ricoh.doquedb.exception.NotSupported
	 *			このメソッドをサポートしていないバージョンのサーバーに接続している場合。
	 * @see		#execute(java.lang.String)
	 */
	public boolean getMoreResults() throws java.sql.SQLException
	{
		if (_connection != null && _connection.getMasterID() < jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION4) {
			throw new NotSupported();
		}
		if (_resultSet != null) {
			// current _resultSet will be closed in setResultSet
			setResultSet(_resultSet.getNextResultSet());
			return _resultSet != null;
		}
		return false;
	}

	/**
	 * <B>[制限あり]</B>
	 * この <code>Statement</code> オブジェクトを使用して作成された
	 * <code>java.sql.ResultSet</code> オブジェクトの行が
	 * 処理される方向についてのヒントをドライバに提供します。デフォルト値は
	 * <code>java.sql.ResultSet.FETCH_FORWARD</code> です。
	 * <P>
	 * このメソッドは、この <code>Statement</code> オブジェクトによって
	 * 生成される結果セットのデフォルトのフェッチ方向を設定します。
	 * 各結果セットは、それ自身のフェッチ方向を取得および設定するための
	 * 独自のメソッドを持ちます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、方向は
	 * <code>java.sql.ResultSet.FETCH_FORWARD</code> のみをサポートします。
	 *
	 * @param	direction_
	 *			行を処理する初期方向。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定された方向が
	 *			<code>java.sql.ResultSet.FETCH_FORWARD</code> 、
	 *			<code>java.sql.ResultSet.FETCH_REVERSE</code> 、または
	 *			<code>java.sql.ResultSet.FETCH_UNKNOWN</code> の
	 *			どれでもない場合。
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
	 * この <code>Statement</code> オブジェクトから生成された結果セットの
	 * デフォルトである、データベーステーブルから行をフェッチする方向を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に
	 * <code>java.sql.ResultSet.FETCH_FORWARD</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に
	 *			<code>java.sql.ResultSet.FETCH_FORWARD</code> 。
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
	 * より多くの行が必要なときに、データベースから取り出す必要がある
	 * 行数についてのヒントを JDBC ドライバに提供します。
	 * 指定された行数は、この <code>Statement</code> を使って作成された
	 * 結果セットにだけ影響します。指定された値が 0 の場合、
	 * ヒントは無視されます。デフォルト値は 0 です。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 <code>0</code> 以外はサポートしません。
	 *
	 * @param	rows_
	 *			フェッチする行数。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または条件
	 *			<code>0 <= rows_ <= this.getMaxRows()</code>
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
			// [YET!] SQLSTATE のサブクラスがまだ未設定。"000" は仮。
			appendWarning(reason, "000");
		}
	}

	/**
	 * この <code>Statement</code> オブジェクトから生成された
	 * <code>java.sql.ResultSet</code> オブジェクトの
	 * デフォルトのフェッチサイズである、結果セットの行数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#setFetchSize(int)
	 */
	public int getFetchSize() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * この <code>Statement</code> オブジェクトから生成された
	 * <code>java.sql.ResultSet</code> オブジェクトの
	 * 結果セットの並行性を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に
	 * <code>java.sql.ResultSet.CONCUR_READ_ONLY</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に
	 *			<code>java.sql.ResultSet.CONCUR_READ_ONLY</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getResultSetConcurrency() throws java.sql.SQLException
	{
		return this._resultSetConcurrency;
	}

	/**
	 * この <code>Statement</code> オブジェクトから生成された
	 * <code>java.sql.ResultSet</code> オブジェクトの
	 * 結果セットの型を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 。
	 * @throws	java.sql.SQLException
	 * 			データベースアクセスエラーが発生した場合。
	 */
	public int getResultSetType() throws java.sql.SQLException
	{
		return this._resultSetType;
	}

	/**
	 * この <code>Statement</code> オブジェクトの現在のコマンドのリストに
	 * 指定された SQL コマンドを追加します。
	 * このリストのコマンドは <code>executeBatch</code> メソッドを
	 * 呼び出すことにより、バッチとして実行できます。
	 *
	 * @param	sql_
	 *			通常静的 SQL <code>INSERT</code> 文、
	 *			または <code>UPDATE</code>文。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#executeBatch()
	 */
	public void addBatch(String	sql_) throws java.sql.SQLException
	{
		if (sql_ == null || sql_.length() == 0) return;

		if (this._batchCommands == null) {
			this._batchCommands = new java.util.Vector();
		}

		this._batchCommands.add(sql_);
	}

	/**
	 * この <code>Statement</code> オブジェクトの現在の SQL コマンドリストを
	 * 空にします。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#addBatch(java.lang.String)
	 */
	public void clearBatch() throws java.sql.SQLException
	{
		this._batchCommands = null;
	}

	/**
	 * コマンドのバッチをデータベースに送信して実行し、
	 * すべてのコマンドが正常に実行されると、更新カウントの配列を返します。
	 * <P>
	 * <B>注:</B>
	 * 一旦更新に失敗すると以後のコマンドは実行されません。
	 * また、自動コミットモードを解除するかどうかはアプリケーションに
	 * 委ねます。
	 * <P>
	 * <B>注:</B>
	 * バッチ更新後、この <code>Statement</code> オブジェクトの現在の
	 * SQL コマンドリストは自動的に空となります。
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
		if (this._batchCommands == null) return new int[0];

		int	numberOfCommands = this._batchCommands.size();
		int[]	updateCounts = new int[numberOfCommands];
		int	i = 0;

		try {

			for (i = 0; i < numberOfCommands; i++) {
				String	sql = (String)this._batchCommands.elementAt(i);
				updateCounts[i] = this.executeUpdate(sql);
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
	 * この <code>Statement</code> オブジェクトを生成した
	 * <code>Connection</code> オブジェクトを取得します。
	 *
	 * @return	この文を生成した接続。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Connection getConnection() throws java.sql.SQLException
	{
		return this._connection;
	}

	/**
	 * <B>[サポート外]</B>
	 * <code>Statement</code> オブジェクトの次の結果に移動します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、このメソッドはサポートしていません。
	 *
	 * @param	current_
	 *			<code>getResultSet</code> を使用して取得した
	 *			現在の <code>java.sql.ResultSet</code> オブジェクトに生じる
	 *			状態を示す <code>java.sql.Statement</code> 定数。
	 *			<code>java.sql.Statement.CLOSE_CURRENT_RESULT</code> 、
	 *			<code>java.sql.Statement.KEEP_CURRENT_RESULT</code> 、または
	 *			<code>java.sql.Statement.CLOSE_ALL_RESULTS</code>
	 *			のうちの 1 つ。
	 * @return	次の結果が <code>java.sql.ResultSet</code> オブジェクトの
	 *			場合は <code>true</code> 。更新カウントであるか、
	 *			または結果がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#execute(java.lang.String)
	 */
	public boolean getMoreResults(int	current_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] execute をサポートしていないため。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>Statement</code> オブジェクトを実行した結果として作成された
	 * 自動生成キーを取得します。
	 * この <code>Statement</code> オブジェクトがキーを生成しなかった場合は、
	 * 空の <code>java.sql.ResultSet</code> オブジェクトが返されます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @return	この <code>Statement</code> オブジェクトの実行で生成された
	 *			自動生成キーを含む
	 *			<code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getGeneratedKeys() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 自動生成キーは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された SQL 文を実行し、この <code>Statement</code> オブジェクトに
	 * よって生成された自動生成キーを検索可能にするかどうかについて
	 * 指定されたフラグでドライバに通知します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			SQL <code>INSERT</code> 文、 <code>UPDATE</code> 文、
	 *			または <code>DELETE</code> 文、あるいは何も返さない SQL 文で
	 *			なければならない。
	 * @param	autoGeneratedKeys_
	 *			自動生成キーが検索可能にされるかどうかを示すフラグ。
	 *			定数 <code>Statement.RETURN_GENERATED_KEYS</code> または
	 *			<code>Statement.NO_GENERATED_KEYS</code> 。
	 * @return	<code>INSERT</code> 文、 <code>UPDATE</code> 文、
	 *			<code>DELETE</code> 文の場合は行数。
	 *			何も返さない SQL 文の場合は 0 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、指定された SQL 文が
	 *			<code>java.sql.ResultSet</code> オブジェクトを返す場合、
	 *			または指定された定数が許可されていない場合。
	 */
	public int executeUpdate(String	sql_,
							 int	autoGeneratedKeys_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 自動生成キーは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された SQL 文を実行し、指定された配列で示された自動生成キーを
	 * 検索可能にするかどうかについてドライバに通知します。
	 * SQL 文が <code>INSERT</code> 文でない場合、ドライバはその配列を
	 * 無視します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			SQL <code>INSERT</code> 文、 <code>UPDATE</code> 文、
	 *			または <code>DELETE</code> 文、あるいは SQL DDL 文のような
	 *			何も返さない SQL 文。
	 * @param	columnIndexes_
	 *			挿入された行から返される列を示す列インデックスの配列。
	 * @return	<code>INSERT</code> 文、 <code>UPDATE</code> 文、
	 *			<code>DELETE</code> 文の場合は行数。
	 *			何も返さない SQL 文の場合は 0 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、または SQL 文が
	 *			<code>java.sql.ResultSet</code> オブジェクトを返す場合。
	 */
	public int executeUpdate(String	sql_,
							 int[]	columnIndexes_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 自動生成キーは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された SQL 文を実行し、指定された配列で示された自動生成キーを
	 * 検索可能にするかどうかについてドライバに通知します。
	 * SQL 文が <code>INSERT</code> 文でない場合、ドライバはその配列を
	 * 無視します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			SQL <code>INSERT</code> 文、 <code>UPDATE</code> 文、
	 *			または <code>DELETE</code> 文、あるいは何も返さない SQL 文。
	 * @param	columnNames_
	 *			挿入された行から返される列の名前の配列。
	 * @return	<code>INSERT</code> 文、 <code>UPDATE</code> 文、
	 *			<code>DELETE</code> 文の場合は行数。
	 *			何も返さない SQL 文の場合は 0 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int executeUpdate(String		sql_,
							 String[]	columnNames_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 自動生成キーは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 複数の結果を返す可能性のある指定された SQL 文を実行し、
	 * すべての自動生成キーを検索可能にするかどうかについて
	 * ドライバに通知します。SQL 文が <code>INSERT</code> 文でない場合、
	 * ドライバはこの通知を無視します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			任意の SQL 文。
	 * @param	autoGeneratedKeys_
	 *			<code>getGeneratedKeys</code> メソッドを使用して、
	 *			自動生成キーを検索可能にするかどうかを示す定数。
	 *			定数 <code>java.sql.Statement.RETURN_GENERATED_KEYS</code>
	 *			または <code>jaav.sql.Statement.NO_GENERATED_KEYS</code> 。
	 * @return	最初の結果が <code>java.sql.ResultSet</code> オブジェクトの
	 *			場合は <code>true</code> 。更新カウントであるか、
	 *			または結果がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#getResultSet()
	 * @see		#getUpdateCount()
	 * @see		#getMoreResults()
	 * @see		#getGeneratedKeys()
	 */
	public boolean execute(String	sql_,
						   int		autoGeneratedKeys_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 自動生成キーは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 複数の結果を返す可能性のある指定された SQL 文を実行し、
	 * 指定された配列で示された自動生成キーを検索可能にするかどうかについて
	 * ドライバに通知します。この配列は検索可能にされる自動生成キーを含む
	 * ターゲットテーブルの列のインデックスを含みます。
	 * 指定された SQL 文が <code>INSERT</code> 文でない場合、
	 * ドライバはその配列を無視します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			任意の SQL 文。
	 * @param	columnIndexes_
	 *			<code>getGeneratedKeys</code> メソッドの呼び出しによって
	 *			検索可能にされる挿入行の列インデックスの配列
	 * @return	最初の結果が <code>java.sql.ResultSet</code> オブジェクトの
	 *			場合は <code>true</code> 。更新カウントであるか、
	 *			または結果がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#getResultSet()
	 * @see		#getUpdateCount()
	 * @see		#getMoreResults()
	 * @see		#getGeneratedKeys()
	 */
	public boolean execute(String	sql_,
						   int[]	columnIndexes_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 自動生成キーは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 複数の結果を返す可能性のある指定された SQL 文を実行し、
	 * 指定された配列で示された自動生成キーを検索可能にするかどうかについて
	 * ドライバに通知します。この配列は検索可能にされる自動生成キーを含む
	 * ターゲットテーブルの列の名前を含みます。
	 * 指定された SQL 文が <code>INSERT</code> 文でない場合、
	 * ドライバはその配列を無視します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			任意の SQL 文。
	 * @param	columnNames_
	 *			<code>getGeneratedKeys</code> メソッドの呼び出しによって
	 *			検索可能にされる挿入行の列名の配列。
	 * @return	最初の結果が <code>java.sql.ResultSet</code> オブジェクトの
	 *			場合は <code>true</code> 。更新カウントであるか、
	 *			または結果がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#getResultSet()
	 * @see		#getUpdateCount()
	 * @see		#getMoreResults()
	 * @see		#getGeneratedKeys()
	 */
	public boolean execute(String	sql_,
						   String[]	columnNames_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 自動生成キーは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * この <code>Statement</code> オブジェクトから生成された
	 * <code>java.sql.ResultSet</code> オブジェクトの結果セットの保持機能を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に
	 * <code>java.sql.ResultSet.CLOSE_CUSORS_AT_COMMIT</code> を返します。
	 *
	 * @return	保持機能。
	 *			現在のバージョンでは、常に <code>java.sql.ResultSet.
	 *			CLOSE_CUSORS_AT_COMMIT</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getResultSetHoldability() throws java.sql.SQLException
	{
		return this._resultSetHoldability;
	}

	/**
	 * この <code>Statement</code> オブジェクトが生成した
	 * <code>java.sql.ResultSet</code> オブジェクトとして、
	 * <code>java.sql.ResultSet</code> オブジェクトを設定します。
	 *
	 * @param	resultSet_
	 *			この <code>Statement</code> オブジェクトが生成した
	 *			<code>java.sql.ResultSet</code> オブジェクトとして設定する、
	 *			<code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	synchronized void setResultSet(ResultSet	resultSet_)
		throws java.sql.SQLException
	{
		if (this._resultSet != null) this._resultSet.close();
		this._resultSet = resultSet_;
		this._isClosed = false;
	}

	/**
	 * この <code>Statement</code> オブジェクトの
	 * <code>java.sql.ResultSet</code> オブジェクトを解放します。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	synchronized void closeResultSet() throws java.sql.SQLException
	{
		if (this._resultSet != null) this._resultSet.close();
		this._resultSet = null;
		this._isClosed = true;
	}

	/**
	 * この <code>Statement</code> オブジェクトがexecuteUpdateまたはexecuteを実行したかを得ます。
	 *
	 * @return	executeUpdateまたはexecuteを実行したならtrue。
	 */
	boolean isUpdating()
	{
		return _updating;
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
		if (this._resultSet != null) this._resultSet.finalize();
	}

	/**
	 * connectionに対してexecuteStatementを実行します。
	 * StatementのexecuteQuery、executeUpdate, executeから呼ばれます。
	 *
	 * @param	sql_
	 *			SQL文。
	 * @param	parameter_
	 *			動的割り当てパラメーター。
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
	private java.sql.ResultSet executeStatement(String sql_,
												DataArrayData parameter_,
												boolean readOnly_,
												boolean updating_)
		throws java.sql.SQLException
	{
		if (sql_ == null || sql_.length() == 0) {
			throw new BadArgument();
		}

		try {

			// When auto-commit mode is not set and no transaction is started,
			// a transaction should be started here.

			if (_connection.getAutoCommit() == false &&
				_connection.inTransaction() == false) {

				// start new transaction according to connection setting
				_connection.beginTransaction();
			}

			jp.co.ricoh.doquedb.client.ResultSet	resultSet =
				this._connection.getSession().executeStatement(sql_, parameter_);
			setResultSet(new ResultSet(this, resultSet));

			this._updating = updating_;
			this._warnings = null;

			return _resultSet;

		} catch (java.io.IOException	ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (ClassNotFoundException	cnfe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			Unexpected	ue = new Unexpected();
			ue.initCause(cnfe);
			throw ue;
		}
	}

	/**
	 * この <code>Statement</code> オブジェクトに関する警告を追加します。
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
	 * この <code>Statement</code> オブジェクトが
	 * クローズされているかどうかを取得します。
	 */
	@Override
	public boolean isClosed() throws SQLException {
		return this._isClosed;
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
		// 常に false
		return false;
	}

	/**
	 * このStatementに依存するすべての結果セットがクローズされたときにこのStatementがクローズされることを示す。
	 * 複数回呼び出してもこのStatementへの効果は切り替わらない。
	 */
	@Override
	public void closeOnCompletion() throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * このStatementに依存するすべての結果セットがクローズされたときにこのStatementがクローズされるかどうかを示す値を取得します。
	 * @return	このStatementに依存するすべての結果セットがクローズされたときにこのStatementがクローズされる場合は <code>true</code>。
	 * 			それ以外は <code>false</code>。
	 */
	@Override
	public boolean isCloseOnCompletion() throws SQLException {
		// 常に false
		return false;
	}
}
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2015, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
