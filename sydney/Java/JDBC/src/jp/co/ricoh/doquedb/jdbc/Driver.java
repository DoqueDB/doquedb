// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Driver.java -- JDBC のドライバークラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011, 2015, 2016, 2023 Ricoh Company, Ltd.
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

import java.util.logging.Logger;

import jp.co.ricoh.doquedb.exception.CannotConnect;
import jp.co.ricoh.doquedb.exception.ConnectionRanOut;
import jp.co.ricoh.doquedb.exception.NotSupported;
import jp.co.ricoh.doquedb.exception.Unexpected;

/**
 * JDBC のドライバークラス。
 *
 */
public class Driver implements java.sql.Driver
{
	/** この JDBC ドライバーのメジャーバージョン。 */
	final static int	MAJOR_VERSION	= 2;
	/** この JDBC ドライバーのマイナーバージョン。 */
	final static int	MINOR_VERSION	= 0;

	public final static int PROTOCOL_VERSION1 = 0;
	public final static int PROTOCOL_VERSION2 = 1;
	public final static int PROTOCOL_VERSION3 = 2;
	public final static int PROTOCOL_VERSION4 = 3;
	public final static int PROTOCOL_VERSION5 = 4;
	public final static int CURRENT_PROTOCOL_VERSION = PROTOCOL_VERSION5;

	/**
	 * 接続先のデータベースの URL で指定される情報クラス。
	 */
	private class URL
	{
		/** ホスト名 */
		public String	_hostName;
		/** ポート番号 */
		public int		_portNumber;
		/** データベース名 */
		public String	_databaseName;
		/** プロトコルバージョン */
		public int		_protocolVersion;
	}

	/**
	 * ホスト情報。
	 */
	public class HostInfo
	{
		/** IPアドレス */
		public java.net.InetAddress	_inetAddress;
		/** ポート番号 */
		public int					_portNumber;
		/** プロトコル */
		public int					_protocolVersion;

		/**
		 * 新しくホスト情報を作成します。
		 *
		 * @param	hostName_
		 *			ホスト名。
		 * @param	portNumber_
		 *			ポート番号。
		 * @param	protocolVersion_
		 *			プロトコルバージョン
		 * @throws	java.net.UnknownHostException
		 *			引数 <code>hostName_</code> に指定されたホストの
		 *			IP アドレスが見つからない場合。
		 */
		public HostInfo(String	hostName_,
						int		portNumber_,
						int		protocolVersion_)
			throws java.net.UnknownHostException
		{
			_inetAddress = java.net.InetAddress.getByName(hostName_);
			_portNumber = portNumber_;
			_protocolVersion = protocolVersion_;
		}

		/**
		 * ホスト情報が等しいかどうかを比較します。
		 *
		 * @param	o_
		 *			比較対象であるホスト情報。
		 * @return	等しい場合は <code>true</code> 、
		 *			異なる場合は <code>false</code> 。
		 */
		public boolean equals(Object o_)
		{
			if (o_ instanceof HostInfo) {
				HostInfo other = (HostInfo)o_;
				if (_inetAddress.equals(other._inetAddress) &&
					_portNumber == other._portNumber &&
					_protocolVersion == other._protocolVersion)
				{
					// すべてのメンバーが同じ

					return true;
				}
			}
			return false;
		}

		/**
		 * このホスト情報のハッシュコードを返します。
		 *
		 * @return	このホスト情報のハッシュコード。
		 */
		public int hashCode()
		{
			return ((_inetAddress.hashCode() + _portNumber) << 2) + _protocolVersion;
		}
	}

	/**
	 * 初期化。
	 */
	static
	{
		try {
			// ドライバマネージャに登録する
			java.sql.DriverManager.registerDriver(new Driver());
		} catch (java.sql.SQLException e) {
			e.printStackTrace();
		}
	}

	/** データソースのマップ。 */
	private static java.util.Map	_dataSourceMap = new java.util.HashMap();

	/**
	 * 新しくドライバーオブジェクトを作成します。
	 */
	public Driver()
	{
	}

	/**
	 * 指定された URL でデータベース接続を試みます。
	 * 間違ったドライバの URL の場合には <code>null</code> を返し、
	 * データベース接続に失敗した場合には <code>java.sql.SQLException</code> を
	 * スローします。
	 * <P>
	 * DoqueDB の JDBC の場合は、以下のような URL になります。
	 * <pre>
	 * 	jdbc:ricoh:doquedb://&lt;host name&gt;:&lt;port number&gt;/&lt;database name&gt;
	 * </pre>
	 *
	 * @param	url_
	 *			接続先のデータベースの URL 。
	 * @param	info_
	 *			接続引数としての、任意の文字列タグおよび値のペアのリスト。
	 * @return	URL への接続を表す
	 *			<code>java.sql.Connection</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Connection connect(String				url_,
									   java.util.Properties	info_)
		throws java.sql.SQLException
	{
		URL	url = null;
		if ((url = parse(url_)) == null) {
			// このドライバの URL ではない場合には null を返す
			return null;
		}

		java.sql.Connection	connection = null;

		boolean	connected = false;

		try {

			synchronized(_dataSourceMap) {

				// ホスト情報
				HostInfo	info =
					new HostInfo(url._hostName, url._portNumber, url._protocolVersion);
				// データソースを検索する
				jp.co.ricoh.doquedb.client.DataSource	dataSource =
					(jp.co.ricoh.doquedb.client.DataSource)
						_dataSourceMap.get(info);
				if (dataSource == null) {
					// データソースがないので作成する
					dataSource =
						new jp.co.ricoh.doquedb.client.DataSource(
															url._hostName,
															url._portNumber);
					dataSource.open(url._protocolVersion);
					// マップに挿入する
					_dataSourceMap.put(info, dataSource);
				}

				String	userName = "";
				if (info_ != null) userName = info_.getProperty("user");
				String	password = "";
				if (info_ != null) password = info_.getProperty("password");

				//セッションを得る
				jp.co.ricoh.doquedb.client.Session	session = null;
				if (dataSource.getAuthorization() == jp.co.ricoh.doquedb.port.AuthorizeMode.NONE) {
					// don't have user management
					session = dataSource.createSession(url._databaseName);
				} else {
					session = dataSource.createSession(url._databaseName, userName, password);
				}

				connection = new Connection(info,
											session,
											url_,
											userName,
											password,
											dataSource.getMasterID());

				connected = true;
			}

		} catch (java.sql.SQLException sqle) {

			// SQLException は、そのまま throw 。
			throw sqle;

		} catch (java.net.UnknownHostException uhe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			CannotConnect	cce = new CannotConnect(url._hostName,
													url._portNumber);
			cce.initCause(uhe);
			throw cce;

		} catch (java.io.IOException ioe) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (Exception e) {

			// [YET!] SQLSTATE は、
			//        connection exception - ???
			//        (08???)
			Unexpected	ue = new Unexpected();
			ue.initCause(e);
			throw ue;

		} finally {

			if (connected == false && connection != null) connection.close();
		}

		return connection;

	} // end of method connect

	/**
	 * 指定された URL に接続できるとドライバが判断するかどうかを取得します。
	 * ドライバは、URL で指定されたサブプロトコルを認識する場合は
	 * <code>true</code> 、認識できない場合は <code>false</code> を返します。
	 * <P>
	 * DoqueDB の JDBC の場合は、以下のような URL になります。
	 * <pre>
	 * 	jdbc:ricoh:doquedb:&lt;host name&gt;:&lt;port number&gt;:&lt;database name&gt;
	 * </pre>
	 *
	 * @param	url_
	 *			データベースの URL 。
	 * @return	このドライバが指定された URL を認識する場合は
	 *			<code>true</code> 、そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean acceptsURL(String	url_) throws java.sql.SQLException
	{
		return (parse(url_) != null);
	}

	/**
	 * このドライバの有効なプロパティについての情報を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、プロパティは不要なので、常に空の配列を返します。
	 *
	 * @param	url_
	 *			接続先のデータベースの URL 。
	 * @param	info_
	 *			接続オープンのために送られるタグ/値ペアの推奨リスト。
	 * @return	現在のバージョンでは、常に空の配列。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.DriverPropertyInfo[]
	getPropertyInfo(String					url_,
					java.util.Properties	info_)
		throws java.sql.SQLException
	{
		return new java.sql.DriverPropertyInfo[0];
	}

	/**
	 * このドライバのメジャーバージョンを取得します。
	 *
	 * @return	このドライバのメジャーバージョン番号。
	 */
	public int getMajorVersion()
	{
		return MAJOR_VERSION;
	}

	/**
	 * このドライバのマイナーバージョンを取得します。
	 *
	 * @return	このドライバのマイナーバージョン番号。
	 */
	public int getMinorVersion()
	{
		return MINOR_VERSION;
	}

	/**
	 * このドライバが JDBC Compliant<sup><font size=-2>TM</font></sup>
	 * であるかどうかを返します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンは JDBC 準拠ではありません。
	 * したがって、常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは <code>false</code> 。
	 */
	public boolean jdbcCompliant()
	{
		return false;
	}

	/**
	 * 可能なら <code>jp.co.ricoh.doquedb.client.DataSource</code> を
	 * 解放します。
	 *
	 * @param	info_
	 *			ホスト情報。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	static void closeDataSource(HostInfo	info_)
		throws java.sql.SQLException
	{
	}

	/**
	 * URL を分解します。
	 *
	 * @param	url_
	 *			JDBC ドライバの URL 。
	 * @return 	パースされた URL 構造体。
	 *			パースに失敗した場合は <code>null</code> を返します。
	 */
	private URL parse(String url_)
	{
		if (url_ == null) return null;

		URL	url = null;

		// jdbc:ricoh:doquedb://<host name>:<port number>/<database name>
		// をパースする。

		int	n = 0;
		int	i = url_.indexOf(':');
		if (i != -1 && url_.substring(n, i).equals("jdbc"))
		{
			n = ++i;
			i = url_.indexOf(':', n);
			if (i != -1 && url_.substring(n, i).equals("ricoh"))
			{
				n = ++i;
				i = url_.indexOf("://", n);
				if (i != -1 && (url_.substring(n, i).equals("doquedb")))
				{
					// 最新のプロトコルバージョン
					int protocolVersion = CURRENT_PROTOCOL_VERSION;
					n = i + 3;
					i = url_.indexOf(':', n);
					if (i != -1) {
						String	hostName = url_.substring(n, i);
						n = ++i;
						i = url_.indexOf('/', n);
						if (i != -1) {
							int	portNumber =
								Integer.valueOf(url_.substring(n, i)).
								intValue();
							n = ++i;
							String	databaseName = url_.substring(n);

							// 構造体を確保する
							url = new URL();
							url._hostName = hostName;
							url._portNumber = portNumber;
							url._databaseName = databaseName;
							url._protocolVersion = protocolVersion;
						}
					}
				}
			}
		}

		return url;
	}

	/**
	 * ドライバで使用されるすべてのロガーの親ロガーを返す。
	 * <B>[サポート外！]</B>
	 *
	 * @return	Loggerオブジェクト。
	 * @throws NotSupported
	 *
	 */
	@Override
	public Logger getParentLogger() throws NotSupported {
		throw new NotSupported();
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011, 2015, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
