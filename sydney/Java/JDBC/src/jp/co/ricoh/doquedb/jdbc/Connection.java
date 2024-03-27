// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.java -- JDBC のコネクションクラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2015, 2016, 2023, 2024 Ricoh Company, Ltd.
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

import java.sql.Array;
import java.sql.Blob;
import java.sql.Clob;
import java.sql.NClob;
import java.sql.SQLClientInfoException;
import java.sql.SQLException;
import java.sql.SQLXML;
import java.sql.Struct;
import java.util.Properties;
import java.util.concurrent.Executor;

import jp.co.ricoh.doquedb.exception.AlreadyBeginTransaction;
import jp.co.ricoh.doquedb.exception.BadArgument;
import jp.co.ricoh.doquedb.exception.BadTransaction;
import jp.co.ricoh.doquedb.exception.ConnectionRanOut;
import jp.co.ricoh.doquedb.exception.NotBeginTransaction;
import jp.co.ricoh.doquedb.exception.NotSupported;
import jp.co.ricoh.doquedb.exception.SessionNotAvailable;
import jp.co.ricoh.doquedb.exception.Unexpected;

/**
 * JDBC のコネクションクラス。
 * 特定のデータベースとの接続 (セッション) を表現します。
 * 接続のコンテキスト内で SQL 文が実行され結果が返されます。
 *
 * @author Takuya Hiraoka
 * @version 1.0
 */
public class Connection implements java.sql.Connection
{
	/** READ ONLY USING SNAPSHOT トランザクション遮断レベル */
	public final static int	TRANSACTION_USING_SNAPSHOT = 0x100;

	/** READ WRITE トランザクションモード。 */
	final static int	TRANSACTION_MODE_READ_WRITE = 1;

	/** READ ONLY トランザクションモード。 */
	final static int	TRANSACTION_MODE_READ_ONLY = 2;

	/** READ ONLY USING SNAPSHOT トランザクションモード。 */
	final static int	TRANSACTION_MODE_READ_ONLY_USING_SNAPSHOT = 3;

	/** ホスト情報。 */
	private Driver.HostInfo	_info;
	/* for backward compatibility */
	private java.net.InetAddress	_inetAddress;
	private int						_portNumber;

	/**
	 * セッションオブジェクト。
	 *
	 * @see	jp.co.ricoh.doquedb.client.Session
	 */
	private jp.co.ricoh.doquedb.client.Session	_session;

	/** クローズしたかどうか。 */
	private boolean	_isClosed;

	/** 自動コミットかどうか。 */
	private boolean	_autoCommit;

	/** 読み込み専用モードかどうか。 */
	private boolean	_readOnly;

	/** 読み込み専用モードかどうかを設定したか。 */
	private boolean	_setReadMode;

	/** トランザクション中かどうか。 */
	private boolean	_inTransaction;

	/** トランザクション遮断レベル。 */
	private int	_isolationLevel;

	/**
	 * データベースメタデータ。
	 *
	 * @see	jp.co.ricoh.doquedb.jdbc.DatabaseMetaData */
	private DatabaseMetaData	_databaseMetaData;

	/** 接続先のデータベースの URL 。 */
	private String	_url;

	/** ユーザ名。 */
	private String	_user;

	/** ユーザパスワード。 */
	private String	_password;

	/** 警告オブジェクト。 */
	private java.sql.SQLWarning	_warnings;

	/** マスター ID */
	private int	_masterID;

	/**
	 * 新しくコネクションオブジェクトを作成する。
	 *
	 * @param	info_
	 *			ホスト情報。
	 * @param	session_
	 *			セッションオブジェクト。
	 * @param	url_
	 *			接続先のデータベースの URL 。
	 * @param	user_
	 *			ユーザ名。
	 * @param	password_
	 *			ユーザパスワード。
	 * @param	masterID_
	 *			マスター ID 。
	 * @see	jp.co.ricoh.doquedb.client.Session
	 */
	Connection(Driver.HostInfo						info_,
			   jp.co.ricoh.doquedb.client.Session	session_,
			   String								url_,
			   String								user_,
			   String								password_,
			   int									masterID_)
	{
		this._info = info_;
		this._session = session_;
		this._isClosed = false;
		this._autoCommit = true;
		this._readOnly = false;
		this._setReadMode = false;
		this._inTransaction = false;
		this._url = url_;
		this._user = user_;
		this._password = password_;
		this._warnings = null;
		this._databaseMetaData = new DatabaseMetaData(this);
		try {
			this._isolationLevel =
				this._databaseMetaData.getDefaultTransactionIsolation();
		} catch (java.sql.SQLException	sqle) {
			this._isolationLevel = TRANSACTION_READ_COMMITTED;
		}
		this._masterID = masterID_;
	}
	/* backward compatibility */
	public Connection(java.net.InetAddress					inetAddress_,
					  int									portNumber_,
					  jp.co.ricoh.doquedb.client.Session	session_,
					  String								url_,
					  String								user_,
					  String								password_,
					  int									masterID_)
	{
		this(null, session_, url_, user_, password_, masterID_);
		_inetAddress = inetAddress_;
		_portNumber = portNumber_;
	}

	/**
	 * SQL 文をデータベースに送るための
	 * <code>java.sql.Statement</code> オブジェクトを生成します。
	 * パラメータなしの SQL 文は通常、 <code>java.sql.Statement</code>
	 * オブジェクトを使用して実行されます。
	 * 同じ SQL 文が多数回実行される場合は、
	 * <code>java.sql.PreparedStatement</code>
	 * オブジェクトを使用する方が効率的です。
	 * <P>
	 * 返される <code>Statement</code> オブジェクトを
	 * 使って作成された結果セットは、
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> の型で、
	 * <code>java.sql.ResultSet.CONCUR_READ_ONLY</code> の並行処理レベルを
	 * 持ちます。
	 *
	 * @return	新しいデフォルト <code>java.sql.Statement</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Statement createStatement() throws java.sql.SQLException
	{
		return createStatement(java.sql.ResultSet.TYPE_FORWARD_ONLY,
							   java.sql.ResultSet.CONCUR_READ_ONLY,
							   java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT);
	}

	/**
	 * パラメータ付き SQL 文をデータベースに送るための
	 * <code>java.sql.PreparedStatement</code> オブジェクトを作成します。
	 * <P>
	 * IN パラメータ付きまたは IN パラメータなしの SQL 文は、
	 * プリコンパイルして、 <code>java.sql.PreparedStatement</code>
	 * オブジェクトに格納できます。
	 * したがって、このオブジェクトは、この文を複数回、効率的に
	 * 実行するのに使用できます。
	 * <P>
	 * 返される <code>Statement</code> オブジェクトを
	 * 使って作成された結果セットは、
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> の型で、
	 * <code>java.sql.ResultSet.CONCUR_READ_ONLY</code> の並行処理レベルを
	 * 持ちます。
	 *
	 * @param	sql_
	 *			1 つ以上の '?' IN パラメータプレースフォルダを
	 *			含めることができる SQL 文。
	 * @return	プリコンパイルされた SQL 文を含む新しいデフォルトの
	 *			<code>java.sql.PreparedStatement</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.PreparedStatement prepareStatement(String	sql_)
		throws java.sql.SQLException
	{
		return prepareStatement(sql_,
								java.sql.ResultSet.TYPE_FORWARD_ONLY,
								java.sql.ResultSet.CONCUR_READ_ONLY,
								java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT);
	}

	/**
	 * <B>[サポート外]</B>
	 * データベースのストアドプロシージャを呼び出すための
	 * <code>java.sql.CallableStatement</code> オブジェクトを生成します。
	 * <code>java.sql.CallableStatement</code> オブジェクトは、
	 * その IN と OUT パラメータを設定するメソッドと
	 * ストアドプロシージャの呼び出しを実行するメソッドを提供します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ストアドプロシージャをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			1 つ以上の '?' パラメータプレースホルダーを含めるとこができる
	 *			SQL 文。
	 * @return	プリコンパイルされた SQL 文を含む新しいデフォルトの
	 *			<code>java.sql.CallableStatement</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.CallableStatement prepareCall(String	sql_)
		throws java.sql.SQLException
	{
		return prepareCall(sql_,
						   java.sql.ResultSet.TYPE_FORWARD_ONLY,
						   java.sql.ResultSet.CONCUR_READ_ONLY,
						   java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT);
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された SQL 文をシステムの本来の SQL 文法に変換します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、何もせずに与えられた SQL 文を
	 * そのまま返します。
	 *
	 * @param	sql_
	 *			1 つ以上の '?' パラメータプレースホルダーを
	 *			含めることができる SQL 文。
	 * @return	この文の元のフォーム。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String nativeSQL(String	sql_) throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		// [YET!] 本当はパースすべき。
		return sql_;
	}

	/**
	 * この <code>Connection</code> オブジェクトの
	 * 自動コミットモードを指定された状態に設定します。
	 * 接続が自動コミットモードの場合、そのすべての SQL 文は実行され、
	 * 個別のトランザクションとしてコミットされます。
	 * そうでない場合、その SQL 文は、 <code>commit</code> メソッドまたは
	 * <code>rollback</code> メソッドへの呼び出しによって終了される
	 * トランザクションにグループ化されます。
	 * デフォルトでは、新しい接続は自動コミットモードです。
	 * <P>
	 * コミットは、文の完了または次の実行の発生のどちらが先に起こっても
	 * 発生します。
	 * 文が、 <code>java.sql.ResultSet</code> オブジェクトを返す場合には、
	 * <code>java.sql.ResultSet</code> オブジェクトの最後の行が取り出されるか
	 * クローズされたときに文は完了します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * トランザクションの途中でこのメソッドが呼び出されると、
	 * そのトランザクションはコミットされます。
	 *
	 * @param	autoCommit_
	 *			自動コミットモードを有効にする場合は <code>true</code> 、
	 *			無効にする場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#getAutoCommit()
	 */
	public void setAutoCommit(boolean	autoCommit_)
		throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		if (this._autoCommit == false && this._inTransaction) commit();
		this._autoCommit = autoCommit_;
	}

	/**
	 * この <code>Connection</code> オブジェクトの
	 * 現在の自動コミットモードを取得します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 *
	 * @return	この <code>Connection</code> オブジェクトの
	 *			現在の自動コミットモードの状態。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#setAutoCommit(boolean)
	 */
	public boolean getAutoCommit() throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		return this._autoCommit;
	}

	/**
	 * 直前のコミット/ロールバック以降に行われた変更をすべて有効とし、
	 * この <code>Connection</code> オブジェクトが現在保持する
	 * データベースロックをすべて解除します。
	 * このメソッドは、自動コミットモードが無効にされているときにのみ
	 * 使用します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはこの <code>Connection</code> オブジェクトが
	 *			自動コミットモードである場合。
	 * @see		#setAutoCommit(boolean)
	 */
	public void commit() throws java.sql.SQLException
	{
		if (this._autoCommit) throw new NotBeginTransaction();

		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		if (this._inTransaction) {

			try {

				jp.co.ricoh.doquedb.client.ResultSet r
					= this._session.executeStatement("commit", null);
				if (r.getStatus() ==
					jp.co.ricoh.doquedb.client.ResultSet.ERROR) {

					// [YET!] 何らかの SQLSTATE と例外を割り当てるべき。
					throw new Unexpected();
				}

				this._inTransaction = false;

			} catch (java.io.IOException ioe) {

				// [YET!] SQLSTATE は、
				//        connection exception - ???
				//        (08???)
				ConnectionRanOut	croe = new ConnectionRanOut();
				croe.initCause(ioe);
				throw croe;

			} catch (ClassNotFoundException cnfe) {

				// [YET!] 何らかの SQLSTATE を割り当てるべき。
				Unexpected	ue = new Unexpected();
				ue.initCause(cnfe);
				throw ue;
			}
		}
	}

	/**
	 * 現在のトランザクションで行われた変更をすべて元に戻し、
	 * この <code>Connection</code> オブジェクトが現在保持する
	 * データベースロックをすべて解除します。
	 * このメソッドは、自動コミットモードが無効にされているときにのみ
	 * 使用します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはこの <code>Connection</code> オブジェクトが
	 *			自動コミットモードである場合。
	 * @see		#setAutoCommit(boolean)
	 */
	public void rollback() throws java.sql.SQLException
	{
		if (this._autoCommit) throw new NotBeginTransaction();

		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		if (this._inTransaction) {

			try {

				jp.co.ricoh.doquedb.client.ResultSet r
					= this._session.executeStatement("rollback", null);
				if (r.getStatus() ==
					jp.co.ricoh.doquedb.client.ResultSet.ERROR) {

					// [YET!] 何らかの SQLSTATE と例外を割り当てるべき。
					throw new Unexpected();
				}

				this._inTransaction = false;

			} catch (java.io.IOException ioe) {

				// [YET!] SQLSTATE は、
				//        connection exception - ???
				//        (08???)
				ConnectionRanOut	croe = new ConnectionRanOut();
				croe.initCause(ioe);
				throw croe;

			} catch (ClassNotFoundException cnfe) {

				// [YET!] 何らかの SQLSTATE を割り当てるべき。
				Unexpected	ue = new Unexpected();
				ue.initCause(cnfe);
				throw ue;
			}
		}
	}

	/**
	 * 自動的な解除を待たずに、ただちにこの <code>Connection</code>
	 * オブジェクトのデータベースと JDBC リソースを解除します。
	 * <P>
	 * すでにクローズされた <code>Connection</code> オブジェクトで
	 * <code>close</code> メソッドを呼び出すと、操作は行われません。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void close() throws java.sql.SQLException
	{
		if (this._isClosed == false) {

			try {

				// セッションを解放する
				this._session.close();

			} catch (java.io.IOException ioe) {

				// [YET!] SQLSTATE は、
				//        connection exception - ???
				//        (08???)
				ConnectionRanOut	croe = new ConnectionRanOut();
				croe.initCause(ioe);
				throw croe;
			}

			// 可能なら DataSource を解放する
			Driver.closeDataSource(this._info);

			this._isClosed = true;
		}
	}

	/**
	 * この <code>Connection</code> オブジェクトが
	 * クローズされているかどうかを取得します。
	 *
	 * @return	この <code>Connection</code> オブジェクトが
	 *			クローズされている場合は <code>true</code> 、
	 *			まだオープンの状態の場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isClosed() throws java.sql.SQLException
	{
		return this._isClosed;
	}

	/**
	 * この <code>Connection</code> オブジェクトが接続を表す
	 * データベースに関するメタデータを格納する
	 * <code>java.sql.DatabaseMetaData</code> オブジェクトを取得します。
	 * メタデータは、データベースのテーブル、
	 * サポートしている SQL 文法、ストアドプロシージャ、
	 * およびこの接続の能力などについての情報を含んでいます。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 *
	 * @return	この <code>Connection</code> に対する
	 *			<code>java.sql.DatabaseMetaData</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.DatabaseMetaData getMetaData() throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		return this._databaseMetaData;
	}

	/**
	 * この <code>Connection</code> を読み込み専用モードに設定して、
	 * データベース最適化を実行するドライバのヒントとします。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * トランザクションの途中では、このメソッドを呼び出すことはできません。
	 *
	 * @param	readOnly_
	 *			<code>true</code> の場合は読み込み専用モードを使用可能にし、
	 *			<code>false</code> の場合は使用不可にします。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはこのメソッドがトランザクションの途中で呼び出された場合。
	 */
	public void setReadOnly(boolean	readOnly_) throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		if (this._inTransaction) {
			// [YET!] SQLSTATE は、
			//        invalid transaction - inappropriate access mode for
			//                              branch transaction
			//        (25003) かな？
			throw new AlreadyBeginTransaction();
		}

		if (this._isolationLevel == TRANSACTION_USING_SNAPSHOT) {

			if (readOnly_) {
				// setTransactionIsolation() で set transaction 済なので、ここでは何もしない
				return;
			} else {
				// using snapshot の場合、read write にはできない。
				throw new BadTransaction();
			}
		}

		if (_setReadMode == false || _readOnly != readOnly_)
		{
			// まだモードを設定していない場合、または、モードが違う場合のみ実行する

			try {

				String	sql;
				if (readOnly_) {
					sql = "set transaction read only";
				} else {
					// 本当はデフォルトに戻したいがないのでとりあえずこのまま
					sql = "set transaction read write";
				}
				jp.co.ricoh.doquedb.client.ResultSet r =
					this._session.executeStatement(sql, null);
				if (r.getStatus()
					== jp.co.ricoh.doquedb.client.ResultSet.ERROR)
				{
					// [YET!] 何らかの SQLSTATE と例外を割り当てるべき。
					throw new Unexpected();
				}

			} catch (java.io.IOException ioe) {

				// [YET!] SQLSTATE は、
				//        connection exception - ???
				//        (08???)
				ConnectionRanOut	croe = new ConnectionRanOut();
				croe.initCause(ioe);
				throw croe;

			} catch (ClassNotFoundException cnfe) {

				// [YET!] 何らかの SQLSTATE を割り当てるべき。
				Unexpected	ue = new Unexpected();
				ue.initCause(cnfe);
				throw ue;
			}

			_setReadMode = true;
		}

		this._readOnly = readOnly_;
	}

	/**
	 * この <code>Connection</code> オブジェクトが
	 * 読み込み専用モードかどうかを取得します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 *
	 * @return	この <code>Connection</code> オブジェクトが
	 *			読み込み専用モードの場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isReadOnly() throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		return this._readOnly;
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>Connection</code> オブジェクトのデータベースに
	 * 作業のためのサブスペースを選択するために、カタログ名を設定します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていないため、
	 * この要求を無視します。
	 *
	 * @param	catalog_
	 *			作業のためのカタログ名。
	 *			カタログは、この <code>Connection</code> オブジェクトの
	 *			データベースのサブスペース。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void setCatalog(String	catalog_) throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		// [NOT SUPPORTED!] カタログは未サポート。
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>Connection</code> オブジェクトの現在のカタログ名を
	 * 取得します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていないため、
	 * 常に <code>null</code> を返します。
	 *
	 * @return	現在のカタログ名。
	 *			現在のバージョンでは、常に <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getCatalog() throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		// [NOT SUPPORTED!] カタログは未サポート。
		return null;
	}

	/**
	 * この <code>Connection</code> のトランザクション遮断レベルを
	 * 指定されたものに変更することを試みます。
	 * 指定できるトランザクション遮断レベルの定数は、
	 * <code>java.sql.Connection</code> インタフェースで定義されています。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * トランザクションの途中では、このメソッドを呼び出すことはできません。
	 *
	 * @param	isolationLevel_
	 *			次の <code>java.sql.Connection</code> 定数のうちの 1 つ。
	 *			<code>TRANSACTION_READ_UNCOMMITTED</code> 、
	 *			<code>TRANSACTION_READ_COMMITTED</code> 、
	 *			<code>TRANSACTION_REPEATABLE_READ</code> 、または
	 *			<code>TRANSACTION_SERIALIZABLE</code> 。
	 *			( <code>TRANSACTION_NONE</code> は
	 *			 トランザクションがサポートされていないことを指定するので
	 *			 使用できない。)
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定されたパラメータが <code>java.sql.Connection</code>
	 *			定数のどれでもない場合、
	 *			またはこのメソッドがトランザクションの途中で呼び出された場合。
	 */
	public void setTransactionIsolation(int	isolationLevel_)
		throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		if (this._inTransaction) {
			// [YET!] SQLSTATE は、
			//        invalid transaction state - ???
			//        (25???)
			throw new AlreadyBeginTransaction();
		}

		StringBuilder	buf
			= new StringBuilder("set transaction isolation level ");
		switch (isolationLevel_) {
		case TRANSACTION_READ_UNCOMMITTED:
			buf.append("read uncommitted");
			break;
		case TRANSACTION_READ_COMMITTED:
			buf.append("read committed");
			break;
		case TRANSACTION_REPEATABLE_READ:
			buf.append("repeatable read");
			break;
		case TRANSACTION_SERIALIZABLE:
			buf.append("serializable");
			break;
		case TRANSACTION_USING_SNAPSHOT:
			buf = new StringBuilder("set transaction read only, using snapshot");
			this._readOnly = true;
			break;
		default:
			// [YET!] SQLSTATE は、
			//        invalid transaction state - ???
			//        (25???)
			throw new BadArgument();
		}

		try {

			jp.co.ricoh.doquedb.client.ResultSet r
				= this._session.executeStatement(buf.toString(), null);
			if (r.getStatus() == jp.co.ricoh.doquedb.client.ResultSet.ERROR) {

				// [YET!] 何らかの SQLSTATE と例外を割り当てるべき。
				throw new Unexpected();
			}

		} catch (java.io.IOException ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (ClassNotFoundException cnfe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			Unexpected	ue = new Unexpected();
			ue.initCause(cnfe);
			throw ue;
		}

		this._isolationLevel = isolationLevel_;
	}

	/**
	 * この <code>Connection</code> オブジェクトの
	 * 現在のトランザクション遮断レベルを取得します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 *
	 * @return	現在のトランザクション遮断レベル。
	 *			次の <code>Connection</code> 定数のうちの 1 つ。
	 *			<code>TRANSACTION_READ_UNCOMMITTED</code> 、
	 *			<code>TRANSACTION_READ_COMMITTED</code> 、
	 *			<code>TRANSACTION_REPEATABLE_READ</code> 、
	 *			<code>TRANSACTION_SERIALIZABLE</code> 、または
	 *			<code>TRANSACTION_NONE</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getTransactionIsolation() throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		return this._isolationLevel;
	}

	/**
	 * この <code>Connection</code> オブジェクトに関する呼び出しによって
	 * 報告される最初の警告を取得します。
	 * 2 つ以上の警告がある場合、後続の警告は最初の警告にチェーンされ、
	 * 直前に取得された警告の <code>SQLWarning.getNextWarning</code>
	 * メソッドを呼び出すことによって取得されます。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 引き続く警告は、
	 * この <code>SQLWarning</code> にチェーンされます。
	 *
	 * @return	最初の <code>java.sql.SQLWarning</code> オブジェクト。
	 *			ない場合は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			またはこのメソッドがクローズされた接続について
	 *			呼び出された場合。
	 */
	public java.sql.SQLWarning getWarnings() throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		return this._warnings;
	}

	/**
	 * この <code>Connection</code> オブジェクトに関して通知された
	 * すべての警告をクリアします。このメソッドを呼び出したあと、
	 * この <code>Connection</code> オブジェクトに対する新しい警告が
	 * 通知されるまで、<code>getWarnings</code> は
	 * <code>null</code> を返します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void clearWarnings() throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		this._warnings = null;
	}

	/**
	 * <B>[制限あり]</B>
	 * 指定された型と並行処理で <code>java.sql.ResultSet</code> オブジェクトを
	 * 生成する <code>java.sql.Statement</code> オブジェクトを生成します。
	 * このメソッドは上記の <code>createStatement</code> メソッドと同じですが、
	 * デフォルトの結果セットの型および並行処理をオーバーライドできます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 結果セットの型は
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 * 並行処理の種類は <code>java.sql.ResultSet.CONCUR_READ_ONLY</code>
	 * のみをサポートします。
	 *
	 * @param	resultSetType_
	 *			結果セットの型。現在のバージョンでは、
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみを
	 *			サポートします。
	 * @param	resultSetConcurrency_
	 *			並行処理の種類。現在のバージョンでは、
	 *			<code>java.sql.ResultSet.CONCUR_READ_ONLY</code> のみを
	 *			サポートします。
	 * @return	指定された型および並行処理で <code>java.sql.ResultSet</code>
	 *			オブジェクトを生成する
	 *			新しい <code>java.sql.Statement</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定されたパラメータが型および並行処理を示す
	 *			<code>java.sql.ResultSet</code> 定数でない場合。
	 */
	public java.sql.Statement createStatement(int	resultSetType_,
											  int	resultSetConcurrency_)
		throws java.sql.SQLException
	{
		return createStatement(resultSetType_,
							   resultSetConcurrency_,
							   java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT);
	}

	/**
	 * <B>[制限あり]</B>
	 * 指定された型と並行処理で <code>java.sql.ResultSet</code> オブジェクトを
	 * 生成する <code>java.sql.PreparedStatement</code> オブジェクトを
	 * 生成します。
	 * このメソッドは上記の <code>prepareStatement</code> メソッドと
	 * 同じですが、デフォルトの結果セットの型および並行処理を
	 * オーバーライドできます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、結果セットの型は
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 * 並行処理の種類は <code>java.sql.ResultSet.CONCUR_READ_ONLY</code>
	 * のみをサポートします。
	 *
	 * @param	sql_
	 *			1 つ以上の '?' IN パラメータを含めることができる SQL 文。
	 * @param	resultSetType_
	 *			結果セットの型。現在のバージョンでは、
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみを
	 *			サポートします。
	 * @param	resultSetConcurrency_
	 *			並行処理の種類。現在のバージョンでは、
	 *			<code>java.sql.ResultSet.CONCUR_READ_ONLY</code> のみを
	 *			サポートします。
	 * @return	プリコンパイルされた SQL 文を含む
	 *			新しい <code>java.sql.PreparedStatement</code> オブジェクト。
	 *			指定された型および並行処理で <code>java.sql.ResultSet</code>
	 *			オブジェクトを生成する。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定されたパラメータが型および並行処理を示す
	 *			<code>java.sql.ResultSet</code> 定数でない場合。
	 */
	public
	java.sql.PreparedStatement prepareStatement(String	sql_,
												int		resultSetType_,
												int		resultSetConcurrency_)
		throws java.sql.SQLException
	{
		return prepareStatement(sql_,
								resultSetType_,
								resultSetConcurrency_,
								java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT);
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された型と並行処理で <code>java.sql.ResultSet</code> オブジェクトを
	 * 生成する <code>java.sql.CallableStatement</code> オブジェクトを
	 * 生成します。
	 * このメソッドは上記の <code>prepareCall</code> メソッドと同じですが、
	 * デフォルトの結果セットの型および並行処理をオーバーライドできます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ストアドプロシージャをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			1 つ以上の '?' IN パラメータを含めることができる SQL 文。
	 * @param	resultSetType_
	 *			結果セットのタイプ。
	 * @param	resultSetConcurrency_
	 *			並行処理の種類。
	 * @return	プリコンパイルされた SQL 文を含む
	 *			新しい <code>java.sql.CallableStatement</code> オブジェクト。
	 *			指定された型および並行処理で <code>java.sql.ResultSet</code>
	 *			オブジェクトを生成する。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public
	java.sql.CallableStatement prepareCall(String	sql_,
										   int		resultSetType_,
										   int		resultSetConcurrency_)
		throws java.sql.SQLException
	{
		return prepareCall(sql_,
						   resultSetType_,
						   resultSetConcurrency_,
						   java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT);
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>Connection</code> に関連した <code>java.util.Map</code>
	 * オブジェクトを取得します。
	 * アプリケーションがエントリを追加していないかぎり、
	 * 空のマップが返されます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カスタムマッピングをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @return	この <code>Connection</code> オブジェクトに関連した
	 *			<code>java.util.Map</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#setTypeMap(java.util.Map)
	 */
	public java.util.Map getTypeMap() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カスタムマッピングは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * この <code>Connection</code> オブジェクトの型マップとして指定された
	 * <code>java.util.Map</code> オブジェクトをインストールします。
	 * 型マップは、SQL 構造化型および個別の型のカスタムマッピングに
	 * 使用されます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カスタムマッピングをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	map_
	 *			この <code>Connection</code> オブジェクトの
	 *			デフォルトの型マップの代わりとしてインストールする
	 *			<code>java.util.Map</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定されたパラメータが <code>java.util.Map</code>
	 *			でない場合。
	 * @see		#getTypeMap()
	 */
	public void setTypeMap(java.util.Map	map_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カスタムマッピングは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[制限あり]</B>
	 * この <code>Connection</code> オブジェクトを使用して生成された
	 * <code>java.sql.ResultSet</code> オブジェクトの保持機能を
	 * 指定された保持機能へ変更します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 現バージョンでは、保持機能は
	 * <code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code>
	 * のみをサポートします。
	 *
	 * @param	holdability_
	 *			<code>java.sql.ResultSet</code> の保持機能定数。
	 *			現在のバージョンでは、
	 *			<code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code> のみを
	 *			サポートします。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定されたパラメータが保持機能を示す
	 *			<code>java.sql.ResultSet</code> 定数でない場合、
	 *			または指定された保持機能がサポートされていない場合。
	 * @see		#getHoldability()
	 * @see		jp.co.ricoh.doquedb.jdbc.ResultSet
	 */
	public void setHoldability(int	holdability_) throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		if (this._databaseMetaData.supportsResultSetHoldability(holdability_)
			== false) {
			// [YET!] SQLSTATE は、
			//        feature not supported - going to perform the function
			//                                which the JDBC driver is not
			//                                supporting
			//        (0A502) ※ 仮。もっと細かく分けるべきか？
			throw new NotSupported();
		}
	}

	/**
	 * この <code>Connection</code> オブジェクトを使用して生成された
	 * <code>java.sql.ResultSet</code> オブジェクトの
	 * 現在の保持機能を取得します。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に
	 * <code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code> を返します。
	 *
	 * @return	保持機能。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getHoldability() throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		// [NOT SUPPORTED!] 保持機能は、
		//                  java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT
		//                  のみをサポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		return java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT;
	}

	/**
	 * <B>[サポート外]</B>
	 * 現在のトランザクションで名前のないセーブポイントを作成し、それを
	 * 表す新しい <code>java.sql.Savepoint</code> オブジェクトを返します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、セーブポイントをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @return	新しい <code>java.sql.Savepoint</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または現在この <code>Connection</code> オブジェクトが
	 *			自動コミットモードである場合。
	 */
	public java.sql.Savepoint setSavepoint() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] セーブポイントは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 現在のトランザクションで指定された名前のセーブポイントを作成し、
	 * それを表す新しい <code>java.sql.Savepoint</code> オブジェクトを
	 * 返します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、セーブポイントをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	name_
	 *			セーブポイントの名前。
	 * @return	新しい <code>java.sql.Savepoint</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または現在この <code>Connection</code> オブジェクトが
	 *			自動コミットモードである場合。
	 */
	public java.sql.Savepoint setSavepoint(String	name_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] セーブポイントは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された <code>java.sql.Savepoint</code> オブジェクトが
	 * 設定されたあとに行われたすべての変更を元に戻します。
	 * <P>
	 * このメソッドは自動コミットが無効のときにだけ使用します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、セーブポイントをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	savepoint_
	 *			ロールバックする <code>java.sql.Savepoint</code>オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			<code>java.sql.Savepoint</code> オブジェクトが
	 *			有効でなくなった場合、
	 *			または現在この <code>Connection</code> オブジェクトが
	 *			自動コミットモードである場合。
	 */
	public void rollback(java.sql.Savepoint	savepoint_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] セーブポイントは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 現在のトランザクションから指定された
	 * <code>java.sql.Savepoint</code> オブジェクトを削除します。
	 * 削除されたあとでセーブポイントを参照すると
	 * <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、セーブポイントをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	savepoint_
	 *			削除する <code>java.sql.Savepoint</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定された <code>java.sql.Savepoint</code> オブジェクトが
	 *			現在のトランザクションで有効なセーブポイントでない場合。
	 */
	public void releaseSavepoint(java.sql.Savepoint	savepoint_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] セーブポイントは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[制限あり]</B>
	 * 指定された型、並行処理、および保持機能で
	 * <code>java.sql.ResultSet</code> オブジェクトを生成する
	 * <code>java.sql.Statement</code> オブジェクトを生成します。
	 * このメソッドは上記の <code>createStatement</code> メソッドと同じですが、
	 * デフォルトの結果セットの型、並行処理、および保持機能を
	 * オーバーライドできます。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 結果セットの型は
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 * 並行処理の種類は <code>java.sql.ResultSet.CONCUR_READ_ONLY</code> 、
	 * 保持機能の種類は <code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code>
	 * のみをサポートします。
	 *
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
	 * @return	指定された型、並行処理、および保持機能で
	 *			<code>java.sql.ResultSet</code> オブジェクトを生成する
	 *			新しい <code>java.sql.Statement</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定されたパラメータが型、並行処理、および保持機能を示す
	 *			<code>java.sql.ResultSet</code> 定数でない場合。
	 */
	public java.sql.Statement createStatement(int	resultSetType_,
											  int	resultSetConcurrency_,
											  int	resultSetHoldability_)
		throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		int	resultSetType =
			getAlternativeResultSetType(resultSetType_);
		int	resultSetConcurrency =
			getAlternativeResultSetConcurrency(resultSetConcurrency_);
		int	resultSetHoldability =
			getAlternativeResultSetHoldability(resultSetHoldability_);

		Statement	statement = new Statement(this,
											  resultSetType,
											  resultSetConcurrency,
											  resultSetHoldability);

		return statement;
	}

	/**
	 * <B>[制限あり]</B>
	 * 指定された型、並行処理、および保持機能で
	 * <code>java.sql.ResultSet</code> オブジェクトを生成する
	 * <code>java.sql.PreparedStatement</code> オブジェクトを生成します。
	 * <P>
	 * このメソッドは上記の <code>prepareStatement</code> メソッドと
	 * 同じですが、デフォルトの結果セットの型、並行処理、および保持機能を
	 * オーバーライドできます。
	 * <P>
	 * このメソッドはクローズされた接続の呼び出しには使用しません。
	 * 使用すると <code>java.sql.SQLException</code> がスローされます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 結果セットの型は
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 * 並行処理の種類は <code>java.sql.ResultSet.CONCUR_READ_ONLY</code> 、
	 * 保持機能の種類は <code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code>
	 * のみをサポートします。
	 *
	 * @param	sql_
	 *			1 つ以上の '?' IN パラメータプレースフォルダを
	 *			含めることができる SQL 文。
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
	 * @return	プリコンパイルされた SQL 文を含む
	 *			新しい <code>java.sql.PreparedStatement</code> オブジェクト。
	 *			指定された型、並行処理、および保持機能で
	 *			<code>java.sql.ResultSet</code> オブジェクトを生成する。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定されたパラメータが型、並行処理、および保持機能を示す
	 *			<code>java.sql.ResultSet</code> 定数でない場合。
	 */
	public
	java.sql.PreparedStatement prepareStatement(String	sql_,
												int		resultSetType_,
												int		resultSetConcurrency_,
												int		resultSetHoldability_)
		throws java.sql.SQLException
	{
		// クローズされていればアクセスできない
		if (this._isClosed) throw new SessionNotAvailable();

		if (sql_ == null || sql_.length() == 0) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			throw new BadArgument();
		}

		int	resultSetType =
			getAlternativeResultSetType(resultSetType_);
		int	resultSetConcurrency =
			getAlternativeResultSetConcurrency(resultSetConcurrency_);
		int	resultSetHoldability =
			getAlternativeResultSetHoldability(resultSetHoldability_);

		try {

			jp.co.ricoh.doquedb.client.PrepareStatement prepareStatement =
				this._session.createPrepareStatement(sql_);

			PreparedStatement	preparedStatement =
				new PreparedStatement(this,
									  prepareStatement,
									  resultSetType,
									  resultSetConcurrency,
									  resultSetHoldability);

			return preparedStatement;

		} catch (java.io.IOException ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (ClassNotFoundException cnfe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			Unexpected	ue = new Unexpected();
			ue.initCause(cnfe);
			throw ue;
		}
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定された型と並行処理で <code>java.sql.ResultSet</code> オブジェクトを
	 * 生成する <code>java.sql.CallableStatement</code> オブジェクトを
	 * 生成します。
	 * このメソッドは上記の <code>prepareCall</code> メソッドと同じですが、
	 * デフォルトの結果セットの型、結果セットの並行処理の種類、
	 * および保持機能をオーバーライドできます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ストアドプロシージャをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			1 つ以上の '?' パラメータプレースホルダーを
	 *			含めるとこができる SQL 文。
	 * @param	resultSetType_
	 *			結果セットの型。
	 * @param	resultSetConcurrency_
	 *			並行処理の種類。
	 * @param	resultSetHoldability_
	 *			保持機能の種類。
	 * @return	プリコンパイルされた SQL 文を含む
	 *			新しい <code>java.sql.CallableStatement</code> オブジェクト。
	 *			指定された型、並行処理、および保持機能で
	 *			<code>java.sql.ResultSet</code> オブジェクトを生成する。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定されたパラメータが型、並行処理、および保持機能を
	 *			示す <code>java.sql.ResultSet</code> 定数でない場合。
	 */
	public
	java.sql.CallableStatement prepareCall(String	sql_,
										   int		resultSetType_,
										   int		resultSetConcurrency_,
										   int		resultSetHoldability_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ストアドプロシージャは未サポート。
		// [YET!] SQLSTATE は、
		//        feature not supported - going to perform the function which
		//                                the JDBC driver is not supporting
		//        (0A502) ※ 仮。もっと細かく分けるべきか？
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 自動生成キーを取得する機能を持つデフォルトの
	 * <code>java.sql.PreparedStatement</code> オブジェクトを生成します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			1 つ以上の '?' パラメータプレースホルダーを
	 *			含めることができる SQL 文。
	 * @param	autoGeneratedKey_
	 *			自動生成キーを返すかどうかを示すフラグ。
	 * @return	プリコンパイルされた SQL 文を含む
	 *			<code>java.sql.PreparedStatement</code> オブジェクト。
	 *			自動生成キーを返す機能を持つ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合、
	 *			または指定されたパラメータが自動生成キーを返すかどうかを示す
	 *			<code>java.sql.Statement</code> 定数でない場合。
	 */
	public
	java.sql.PreparedStatement prepareStatement(String	sql_,
												int		autoGeneratedKey_)
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
	 * 指定された配列によって指定された自動生成キーを返す機能を持つ
	 * デフォルトの <code>java.sql.PreparedStatement</code> オブジェクトを
	 * 生成します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			1 つ以上の '?' パラメータプレースホルダーを
	 *			含めることができるSQL文。
	 * @param	columnIndexes_
	 *			挿入された行から返される列を示す列インデックスの配列。
	 * @return	プリコンパイルされた文を含む
	 *			新しい <code>java.sql.PreparedStatement</code> オブジェクト。
	 *			指定された列インデックス配列によって指定される自動生成キーを
	 *			返す機能を持つ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.PreparedStatement prepareStatement(String	sql_,
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
	 * 指定された配列によって指定された自動生成キーを返す機能を持つ
	 * デフォルトの <code>java.sql.PreparedStatement</code> オブジェクトを
	 * 生成します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * このメソッドはサポートしていません。
	 *
	 * @param	sql_
	 *			1 つ以上の '?' パラメータプレースホルダーを
	 *			含めることができる SQL 文。
	 * @param	columnNames_
	 *			挿入された行から返される列を示す列名の配列。
	 * @return	プリコンパイルされた文を含む
	 *			新しい <code>java.sql.PreparedStatement</code>オブジェクト。
	 *			指定された列名の配列によって指定される自動生成キーを
	 *			返す機能を持つ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.PreparedStatement prepareStatement(String	sql_,
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
	 * <B>[独自拡張]</B>
	 * DBユーザーを新規登録します。
	 * <P>
	 * <B>注:</B>
	 * DoqueDBのJDBCドライバーにおける独自拡張です。
	 * <code>Driver.getConnection()</code>で得たオブジェクトを
	 * <code>jp.co.ricoh.doquedb.jdbc.Connection</code> にキャストして使用してください。
	 *
	 * @param	userName_
	 *			新規登録するユーザーの名称。
	 *			文字種はコントロール文字、空白文字、コロン、カンマを除いたASCII文字、
	 *			最大文字数は16文字まで。
	 * @param	password_
	 *			新規登録するユーザーの初期パスワード。
	 * @param	userID_
	 *			新規登録するユーザーのユーザーID。
	 *			省略するとサーバー内のユーザーIDの最大値に1加えた値が使用されます。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void createUser(String userName_, String password_)
		throws java.sql.SQLException
	{
		createUser(userName_, password_, jp.co.ricoh.doquedb.port.UserID.AUTO);
	}

	public void createUser(String userName_, String password_, int userID_)
		throws java.sql.SQLException
	{
		try {
			_session.createUser(userName_, password_, userID_);

		} catch (java.io.IOException ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (ClassNotFoundException cnfe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			Unexpected	ue = new Unexpected();
			ue.initCause(cnfe);
			throw ue;
		}
	}


	/**
	 * <B>[独自拡張]</B>
	 * DBユーザーを削除します。
	 * <P>
	 * <B>注:</B>
	 * DoqueDBのJDBCドライバーにおける独自拡張です。
	 * <code>Driver.getConnection()</code>で得たオブジェクトを
	 * <code>jp.co.ricoh.doquedb.jdbc.Connection</code> にキャストして使用してください。
	 *
	 * @param	userName_
	 *			削除するユーザーの名称。
	 *			この<code>Connection</code>を得る際に使用したユーザーは削除できません。
	 * @param	dropBehavior_
	 *			すべてのデータベース内に対する削除するユーザーのPrivilege情報の扱い。
	 *			<code>jp.co.ricoh.doquedb.port.DropBehavior</code>に定義されている定数で指定してください。
	 *			省略すると<code>jp.co.ricoh.doquedb.port.DropBehavior.IGNORE</code>が指定されたものとして扱われます。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void dropUser(String userName_, int dropBehavior_)
		throws java.sql.SQLException
	{
		try {
			_session.dropUser(userName_, dropBehavior_);

		} catch (java.io.IOException ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (ClassNotFoundException cnfe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			Unexpected	ue = new Unexpected();
			ue.initCause(cnfe);
			throw ue;
		}
	}

	public void dropUser(String userName_)
		throws java.sql.SQLException
	{
		dropUser(userName_, jp.co.ricoh.doquedb.port.DropBehavior.IGNORE);
	}

	/**
	 * <B>[独自拡張]</B>
	 * 自分自身のパスワードを変更します。
	 * <P>
	 * <B>注:</B>
	 * DoqueDBのJDBCドライバーにおける独自拡張です。
	 * <code>Driver.getConnection()</code>で得たオブジェクトを
	 * <code>jp.co.ricoh.doquedb.jdbc.Connection</code> にキャストして使用してください。
	 *
	 * @param	password_
	 *			変更後のパスワード。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void changeOwnPassword(String password_)
		throws java.sql.SQLException
	{
		try {
			_session.changeOwnPassword(password_);

		} catch (java.io.IOException ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (ClassNotFoundException cnfe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			Unexpected	ue = new Unexpected();
			ue.initCause(cnfe);
			throw ue;
		}
	}


	/**
	 * <B>[独自拡張]</B>
	 * DBユーザーのパスワードを変更します。
	 * <P>
	 * <B>注:</B>
	 * DoqueDBのJDBCドライバーにおける独自拡張です。
	 * <code>Driver.getConnection()</code>で得たオブジェクトを
	 * <code>jp.co.ricoh.doquedb.jdbc.Connection</code> にキャストして使用してください。
	 *
	 * @param	userName_
	 *			パスワードを変更するユーザーの名称。
	 * @param	password_
	 *			変更後のパスワード。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void changePassword(String userName_, String password_)
		throws java.sql.SQLException
	{
		try {
			_session.changePassword(userName_, password_);

		} catch (java.io.IOException ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (ClassNotFoundException cnfe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			Unexpected	ue = new Unexpected();
			ue.initCause(cnfe);
			throw ue;
		}
	}

	/**
	 * セッションオブジェクトを得る。
	 *
	 * @return	セッションオブジェクト
	 */
	jp.co.ricoh.doquedb.client.Session getSession()
	{
		return this._session;
	}

	/**
	 * 接続先のデータベースの URL を取得します。
	 *
	 * @return	接続先のデータベースの URL 。
	 */
	String getURL()
	{
		return this._url;
	}

	/**
	 * ユーザ名を取得します。
	 *
	 * @return	ユーザ名。
	 */
	String getUserName()
	{
		return this._user;
	}

	/**
	 * パスワードを取得します。
	 *
	 * @return	パスワード。
	 */
	String getPassword()
	{
		return this._password;
	}

	/**
	 * トランザクション中であるかどうかを調べます。
	 *
	 * @return	トランザクション中である場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 */
	boolean inTransaction()
	{
		return this._inTransaction;
	}

	/**
	 * トランザクションを開始します。
	 * このメソッドは、自動コミットモードが無効にされているときにのみ
	 * 使用します。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	void beginTransaction() throws java.sql.SQLException
	{
		int	transactionMode = TRANSACTION_MODE_READ_WRITE;
		if (this._readOnly) {
			if (this._isolationLevel == TRANSACTION_USING_SNAPSHOT) {
				transactionMode = TRANSACTION_MODE_READ_ONLY_USING_SNAPSHOT;
			} else {
				transactionMode = TRANSACTION_MODE_READ_ONLY;
			}
		}

		beginTransaction(transactionMode);
	}

	/**
	 * トランザクションを開始します。
	 * このメソッドは、自動コミットモードが無効にされているときにのみ
	 * 使用します。
	 *
	 * @param	transactionMode_
	 *			トランザクションモード。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	void beginTransaction(int	transactionMode_)
		throws java.sql.SQLException
	{
		if (this._autoCommit) return;

		try {

			StringBuilder	query = new StringBuilder("start transaction");

			switch (transactionMode_) {
			case TRANSACTION_MODE_READ_WRITE:
				query.append(" read write");
				break;
			case TRANSACTION_MODE_READ_ONLY:
				query.append(" read only");
				break;
			case TRANSACTION_MODE_READ_ONLY_USING_SNAPSHOT:
				query.append(" read only, using snapshot");
				break;
			default:
				break;
			}

			switch (this._isolationLevel) {
			case java.sql.Connection.TRANSACTION_READ_COMMITTED:
				query.append(", isolation level read committed");
				break;
			case java.sql.Connection.TRANSACTION_READ_UNCOMMITTED:
				query.append(", isolation level read uncommitted");
				break;
			case java.sql.Connection.TRANSACTION_REPEATABLE_READ:
				query.append(", isolation level repeatable read");
				break;
			case java.sql.Connection.TRANSACTION_SERIALIZABLE:
				query.append(", isolation level serializable");
				break;
			default:
				break;
			}

			jp.co.ricoh.doquedb.client.ResultSet	r =
				this._session.executeStatement(query.toString(), null);

			if (r.getStatus() == jp.co.ricoh.doquedb.client.ResultSet.ERROR) {

				// [YET!] 何らかの SQLSTATE と例外を割り当てるべき。
				throw new Unexpected();
			}

			this._inTransaction = true;

		} catch (java.io.IOException ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (ClassNotFoundException cnfe) {

			// [YET!] 何らかの SQLSTATE を割り当てるべき。
			Unexpected	ue = new Unexpected();
			ue.initCause(cnfe);
			throw ue;
		}
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
	 * マスター ID を返す。
	 */
	public int getMasterID()
	{
		return _masterID;
	}

	/**
	 * この <code>Connection</code> オブジェクトに関する警告を追加します。
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
	 * 結果セットの型を返します。
	 * 指定された結果セットの型がサポート外の場合、
	 * この <code>Connection</code> オブジェクトに関する警告を追加し、
	 * 最も近い代替値を返します。
	 * <P>
	 * <B>注:</B> 現在のバージョンでは、
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみをサポートします。
	 * したがって、常に
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> を返します。
	 *
	 * @param	resultSetType_
	 *			結果セットの型。
	 * @return	現在のバージョンでは、常に
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 。
	 * @throws	jp.co.ricoh.doquedb.exception.BadArgument
	 *			指定された結果セットの型が、以下のいずれでもない場合。
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_SENSITIVE</code> 。
	 */
	private int getAlternativeResultSetType(int	resultSetType_)
		throws BadArgument
	{
		if (resultSetType_ != java.sql.ResultSet.TYPE_FORWARD_ONLY &&
			resultSetType_ != java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE &&
			resultSetType_ != java.sql.ResultSet.TYPE_SCROLL_SENSITIVE) {

			// [YET!] SQLSTATE は、
			//        data exception - ???
			//        (22???)
			throw new BadArgument();
		}

		int	resultSetType = resultSetType_;

		if (resultSetType != java.sql.ResultSet.TYPE_FORWARD_ONLY) {

			// [NOT SUPPORTED!] 結果セットの型は
			//                  java.sql.ResultSetTYPE_FORWARD_ONLY
			//                  のみをサポート。
			String	reason =
				"type of ResultSet currently supported is only " +
				"java.sql.ResultSet.TYPE_FORWARD_ONLY.";
			// [YET!] SQLSTATE のサブクラスがまだ未設定。"000" は仮。
			appendWarning(reason, "000");

			resultSetType = java.sql.ResultSet.TYPE_FORWARD_ONLY;
		}

		return resultSetType;
	}

	/**
	 * 並行処理の種類を返します。
	 * 指定された並行処理がサポート外の場合、
	 * この <code>Connection</code> オブジェクトに関する警告を追加し、
	 * もっとも近い代替値を返します。
	 * <P>
	 * <B>注:</B> 現在のバージョンでは、
	 * <code>java.sql.ResultSet.CONCUR_READ_ONLY</code> のみをサポートします。
	 * したがって、常に
	 * <code>java.sql.ResultSet.CONCUR_READ_ONLY</code> を返します。
	 *
	 * @param	resultSetConcurrency_
	 *			並行処理の種類。
	 * @return	現在のバージョンでは、常に
	 *			<code>java.sql.ResultSet.CONCUR_READ_ONLY</code> 。
	 * @throws	jp.co.ricoh.doquedb.exception.BadArgument
	 *			指定された並行処理が、以下のいずれでもない場合。
	 *			<code>java.sql.ResultSet.CONCUR_READ_ONLY</code> 、
	 *			<code>java.sql.ResultSet.CONCUR_UPDATABLE</code> 。
	 */
	private int getAlternativeResultSetConcurrency(int	resultSetConcurrency_)
		throws BadArgument
	{
		if (resultSetConcurrency_ != java.sql.ResultSet.CONCUR_READ_ONLY &&
			resultSetConcurrency_ != java.sql.ResultSet.CONCUR_UPDATABLE) {

			// [YET!] SQLSTATE は、
			//        data exception - ???
			//        (22???)
			throw new BadArgument();
		}

		int	resultSetConcurrency = resultSetConcurrency_;

		if (resultSetConcurrency != java.sql.ResultSet.CONCUR_READ_ONLY) {

			// [NOT SUPPORTED!] 並行処理は
			//                  java.sql.ResultSet.CONCUR_READ_ONLY
			//                  のみをサポート。
			String	reason =
				"concurrency of ResultSet currently supported is only " +
				"java.sql.ResultSet.CONCUR_READ_ONLY.";
			// [YET!] SQLSTATE のサブクラスがまだ未設定。"000" は仮。
			appendWarning(reason, "000");

			resultSetConcurrency = java.sql.ResultSet.CONCUR_READ_ONLY;
		}

		return resultSetConcurrency;
	}

	/**
	 * 保持機能の種類を返します。
	 * 指定された保持機能がサポート外の場合、
	 * この <code>Connection</code> オブジェクトに関する警告を追加し、
	 * もっとも近い代替値を返します。
	 * <P>
	 * <B>注:</B> 現在のバージョンでは、
	 * <code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code>
	 * のみをサポートします。
	 * したがって、常に
	 * <code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code> を返します。
	 *
	 * @param	resultSetHoldability_
	 *			保持機能の種類。
	 * @return	現在のバージョンでは、常に
	 *			<code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code> 。
	 * @throws	jp.co.ricoh.doquedb.exception.BadArgument
	 *			指定された保持機能が、以下のいずれでもない場合。
	 *			<code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code> 、
	 *			<code>java.sql.ResultSet.HOLD_CURSORS_OVER_COMMIT</code> 。
	 */
	private int getAlternativeResultSetHoldability(int	resultSetHoldability_)
		throws BadArgument
	{
		if (resultSetHoldability_ !=
			java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT &&
			resultSetHoldability_ !=
			java.sql.ResultSet.HOLD_CURSORS_OVER_COMMIT) {

			// [YET!] SQLSTATE は、
			//        data exception - ???
			//        (22???)
			throw new BadArgument();
		}

		int	resultSetHoldability = resultSetHoldability_;

		if (resultSetHoldability !=
			java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT) {

			// [NOT SUPPORTED!] 保持機能は
			//                  java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT
			//                  のみサポート。
			String	reason =
				"holdability of ResultSet currently supported is only " +
				"java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT.";
			// [YET!] SQLSTATE のサブクラスがまだ未設定。"000" は仮。
			appendWarning(reason, "000");

			resultSetHoldability = java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT;
		}

		return resultSetHoldability;
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
	public boolean isWrapperFor(Class<?> iface) throws SQLException	{
		// unwrap を実装している場合は true を返すが、
		// それ以外の場合は false を返す

		return false;
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public Clob createClob() throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public Blob createBlob() throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public NClob createNClob() throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public SQLXML createSQLXML() throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * 接続が有効か否か
	 */
	@Override
	public boolean isValid(int timeout) throws SQLException {
		// closeされていなければ有効とする

		return (_isClosed == false) ? true : false;
	}

	/**
	 * <B>サポート外</B>
	 */
	@Override
	public void setClientInfo(String name, String value)
			throws SQLClientInfoException {
		// サポート外だが、SQLFeatureNotSupportedException を
		// スローすることはできない
	}

	/**
	 * <B>サポート外</B>
	 */
	@Override
	public void setClientInfo(Properties properties)
			throws SQLClientInfoException {
		// サポート外だが、SQLFeatureNotSupportedException を
		// スローすることはできない
	}

	/**
	 * <B>サポート外</B>
	 */
	@Override
	public String getClientInfo(String name) throws SQLException {
		// サポート外なので、常に null を返す
		return null;
	}

	/**
	 * <B>サポート外</B>
	 */
	@Override
	public Properties getClientInfo() throws SQLException {
		// サポート外なので、常に null を返す
		return null;
	}

	/**
	 * <code>Array</code> オブジェクトを生成するファクトリメソッドです
	 */
	@Override
	public Array createArrayOf(String typeName, Object[] elements)
			throws SQLException {
		// typeName は利用しない

		return new jp.co.ricoh.doquedb.jdbc.Array(elements);
	}


	/**
	 * <B>サポート外</B>
	 */
	@Override
	public Struct createStruct(String typeName, Object[] attributes)
			throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * 現在のタイムアウト制限を返す
	 * @return タイムアウト制限。０は制限なし。
	 */
	@Override
	public int getNetworkTimeout() throws SQLException {
		// 制限なしのため 0
		return 0;
	}
	/**
	 * <B>サポート外</B>
	 */
	@Override
	public void setNetworkTimeout(Executor executor, int milliseconds) throws SQLException {
		// サポート外なので、常に エラーを返す
		throw new NotSupported();
	}

	/**
	 * オープン接続を終了する。接続がクローズとマークされ、すべてのリソースを解放する。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。または、executorがnullの場合。
	 */
	@Override
	public void abort(Executor executor) throws SQLException {
		// TODO: JAVA8対応では実装しない
	}

	/**
	 * <B>サポート外</B>
	 */
	@Override
	public String getSchema() throws SQLException {
		// <code>Connection</code> オブジェクトはスキーマをサポートしないため常に null を返す
		return null;
	}

	/**
	 * <B>サポート外</B>
	 */
	@Override
	public void setSchema(String schema) throws SQLException {
		// <code>Connection</code> オブジェクトはスキーマをサポートしないため何も実行せず終了する
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2015, 2016, 2023, 2024 Ricoh Company, Ltd.
// All rights reserved.
//
