// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SydDataSource.java -- JDBCX のベースデータソースクラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2011, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.jdbcx;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintWriter;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.GregorianCalendar;

import javax.naming.NamingException;
import javax.naming.Reference;
import javax.naming.Referenceable;
import javax.naming.StringRefAddr;

/**
 * DataSource の基本オブジェクです。
 * 直接使用する事はありません。
 *
 */
//public abstract class SydDataSource implements Referenceable
abstract class SydDataSource implements Referenceable
{
	// 実際にデータベースに接続するのに使用するので、普通のドライバーを積み込んでください
	static{
		try{
			Class.forName("jp.co.ricoh.doquedb.jdbc.Driver");
		}catch(ClassNotFoundException ex){
			System.err.println("SQL DataSource unable to load DoqueDB SQL JDBC Driver");
		}
	}

	// DataSource/ConnectionPoolDataSourceインタフェースを実装するために必要です。
	private transient PrintWriter logger;

	// JDBC2.0Optionalパッケージ仕様に基づき定義された標準の特性
	private String serverName = "localhost";
	private String databaseName;
	private String user;
	private String password;
	private int portNumber;
	private int prepareThreshold;
	private String networkProtocol = "doquedb";

	// 独自プロパティ
	private String url;

	// タイムアウト秒
	private int timeoutSec = 0;

	/**
	 * 接続を得ます。データベースはDataSourceの特性のserverName、databaseName、portNumber、およびnetworkProtocolによって特定されます。
	 * DataSource特性のユーザとパスワードによって特定されるユーザ。
	 *
	 * @return	Connectionオブジェクト
	 * @throws	SQLException
	 * 			データベース接続を確立することができないとき起こります。
	 */
	public Connection getConnection() throws SQLException
	{
		return getConnection(user, password);
	}

	/**
	 * 接続を得ます。データベースはDataSourceの特性のserverName、databaseName、portNumber、およびnetworkProtocolによって特定されます。
	 * DataSource特性のユーザとパスワードによって特定されるユーザ。
	 *
	 * @param	user_
	 *			ユーザ。
	 * @param	password_
	 *			ユーザパスワード。
	 * @return	Connectionオブジェクト
	 * @throws	SQLException
	 * 			データベース接続を確立することができないとき起こります。
	 */
	public Connection getConnection(String user_, String password_) throws SQLException
	{
		try{
			Connection con = DriverManager.getConnection(getUrl(), user_, password_);
/*
			if(con!=null){
				System.out.print("con=" + con.toString() + "\n");
				System.out.print(user_ + " : " + password_ + "\n");
			}else{
				System.out.print("con=null");
			}
*/
			if(logger != null){
				GregorianCalendar cal1 = new GregorianCalendar();
				String dateString = "[" + cal1.getTime().toString() + "] ";
				logger.println(dateString + "Created new connection for " + user_ + " at " + getUrl());
			}
			return con;

		}catch (SQLException ex)	{
			if(logger != null){
				GregorianCalendar cal1 = new GregorianCalendar();
				String dateString = "[" + cal1.getTime().toString() + "] ";
				logger.println(dateString + "Failed to create new connection for " + user_ + " at " + getUrl() + ": " + ex);
			}
			throw ex;
		}
	}

	/**
	 * データベースへの接続試行中に、このデータソースが待機する最長時間 (秒) を取得します。
	 * 値が 0 のとき、デフォルトのシステムタイムアウトが設定されている場合はその値になります。
	 * そうでない場合はタイムアウトしないことを意味します。
	 *
	 * @return	タイムアウト秒
	 */
	public int getLoginTimeout() throws SQLException
	{
		return timeoutSec;
	}

	/**
	 * データベースへの接続試行中に、このデータソースが待機する最長時間 (秒) を設定します。
	 * 値が 0 のとき、デフォルトのシステムタイムアウトが設定されている場合はその値になります。
	 * そうでない場合はタイムアウトしないことを意味します。
	 *
	 * @param	loginTime_
	 *			タイムアウト秒
	 */
	public void setLoginTimeout(int loginTime_)// throws SQLException
	{
		if(loginTime_>=0){
			timeoutSec = loginTime_;
		}
	}

	/**
	 * この DataSource オブジェクトのログライターを取得します。
	 */
	public PrintWriter getLogWriter() throws SQLException
	{
		return logger;
	}

	/**
	 * この DataSource オブジェクトのログライターを java.io.PrintWriter オブジェクトに設定します。
	 *
	 * @param	printWriter_
	 *			新しいログライター。ログの取得を無効にする場合は null に設定する
	 */
	public void setLogWriter(PrintWriter printWriter_) throws SQLException
	{
		logger = printWriter_;
	}

	/**
	 * ホストの名前を得ます。
	 *
	 * @return	サーバ名
	 */
	public String getServerName()
	{
		return serverName;
	}

	/**
	 * ホストの名前を設定します。デフォルト値はlocalhostです。
	 *
	 * @param	serverName_
	 *			サーバ名
	 */
	public void setServerName(String serverName_)
	{
		if(serverName_ == null || serverName_.equals(""))	{
			this.serverName = "localhost";
		}else{
			this.serverName = serverName_;
		}
	}

	/**
	 * データベースの名前を得ます。
	 *
	 * @return	データベース名
	 */
	public String getDatabaseName()
	{
		return databaseName;
	}

	/**
	 * データベース名を設定します。
	 *
	 * @param	databaseName_
	 *			データベース名
	 */
	public void setDatabaseName(String databaseName_)
	{
		this.databaseName = databaseName_;
	}

	/**
	 * ユーザ名を取得します。
	 *
	 * @return	ユーザ名
	 */
	public String getUser()
	{
		return user;
	}

	/**
	 * ユーザ名を設定します。
	 *
	 * @param	user_
	 *			ユーザ名
	 */
	public void setUser(String user_)
	{
		this.user = user_;
	}

	/**
	 * ユーザパスワードを取得します。
	 *
	 * @return	ユーザパスワード
	 */
	public String getPassword()
	{
		return password;
	}

	/**
	 * ユーザパスワードを設定します。
	 *
	 * @param	password_
	 *			ユーザパスワード
	 */
	public void setPassword(String password_)
	{
		this.password = password_;
	}

	/**
	 * サーバのTCP/IP接続のポート番号を取得します。
	 *
	 * @return	ポート番号
	 */
	public int getPortNumber()
	{
		return portNumber;
	}

	/**
	 * サーバがTCP/IP接続のために開いているポート番号を設定します。
	 *
	 * @param	portNumber_
	 *			ポート番号
	 */
	public void setPortNumber(int portNumber_)
	{
		this.portNumber = portNumber_;
	}

	/**
	 * サーバ側を可能にするためのデフォルト敷居が準備するセット。
	 *
	 * @param	count_
	 *			サーバ側が準備する声明オブジェクトを再利用しなければならない回数
	 */
	public void setPrepareThreshold(int count_)
	{
		this.prepareThreshold = count_;
	}

	/**
	 * サーバ側が準備する可能にするためにデフォルト敷居を得ます。
	 *
	 * @return	カウント
	 */
	public int getPrepareThreshold()
	{
		return prepareThreshold;
	}

	/**
	 * サーバとの通信に使用するプロトコルを得ます。
	 *
	 * @return	プロトコル
	 */
	public String getNetworkProtocol()
	{
		return networkProtocol;
	}

	/**
	 * サーバとの通信に使用するプロトコルを設定します。
	 * 設定できるプロトコルは、doquedb と doquedb-s のみです。
	 *
	 * @param	networkProtocol_
	 *			プロトコル
	 */
	public void setNetworkProtocol(String networkProtocol_)
	{
		this.networkProtocol = networkProtocol_;
	}

	/**
	 * 供給された他の資産からDriverManager URLを取得。
	 *
	 * @return	URL
	 */
	public String getUrl()
	{
		if (url == null) {
			// urlを作成する
			url = "jdbc:ricoh:" + networkProtocol + "://" + serverName + ":" + portNumber + "/" + databaseName;
		}
		return url;
	}

	/**
	 * URLを設定する
	 *
	 * @param	url_
	 *			JDBCドライバーのURL
	 */
	public void setUrl(String url_)
	{
		this.url = url_;
	}

	/**
	 * Reference ファクトリ作成
	 */
	Reference createReference() {
		return new Reference(	getClass().getName(),
								SydObjectFactory.class.getName(),
								null);
	}

	public Reference getReference() throws NamingException
	{
		Reference rf = createReference();
		rf.add(new StringRefAddr( "serverName", serverName ));
		if(portNumber != 0){
			rf.add(new StringRefAddr( "portNumber", Integer.toString(portNumber) ));
		}
		rf.add(new StringRefAddr( "databaseName", databaseName ));
		if(user != null){
			rf.add(new StringRefAddr( "user", user ));
		}
		if(password != null){
			rf.add(new StringRefAddr( "password", password ));
		}
		if(prepareThreshold != 5) {
			rf.add(new StringRefAddr( "prepareThreshold", Integer.toString(prepareThreshold) ));
		}
		if (networkProtocol != null) {
			rf.add(new StringRefAddr("networkProtocol", networkProtocol));
		}
		if (url != null) {
			rf.add(new StringRefAddr("url", url));
		}
		return rf;
	}

	void writeBaseObject(ObjectOutputStream out_) throws IOException
	{
		out_.writeObject(serverName);
		out_.writeInt(portNumber);
		out_.writeInt(prepareThreshold);
		out_.writeObject(databaseName);
		out_.writeObject(user);
		out_.writeObject(password);
		out_.writeObject(networkProtocol);
		out_.writeObject(url);
	}

	void readBaseObject(ObjectInputStream in_) throws IOException, ClassNotFoundException
	{
		serverName = (String)in_.readObject();
		portNumber = in_.readInt();
		prepareThreshold = in_.readInt();
		databaseName = (String)in_.readObject();
		user = (String)in_.readObject();
		password = (String)in_.readObject();
		networkProtocol = (String)in_.readObject();
		url = (String)in_.readObject();
	}

}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
