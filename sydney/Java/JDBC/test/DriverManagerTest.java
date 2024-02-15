// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DriverManagerTest.java -- java.sql.DriverManager クラス経由でのテスト
// 
// Copyright (c) 2004, 2007, 2023 Ricoh Company, Ltd.
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
import junit.framework.*;
import java.sql.*;

import jp.co.ricoh.doquedb.exception.*;

public class DriverManagerTest extends TestBase
{
	public DriverManagerTest(String nickname)
	{
		super(nickname);
	}

	// 不正なホスト名の URL を返す
	private String getIllegalHostNameURL()
	{
		return "jdbc:ricoh:doquedb://xxx_HOST_xxx:54321/TEST";
	}

	// 不正なポート番号の URL を返す
	private String getIllegalPortNumberURL()
	{
		return "jdbc:ricoh:doquedb://localhost:0/TEST";
	}

	// 不正なデータベース名の URL を返す
	private String getIllegalDatabaseNameURL()
	{
		return "jdbc:ricoh:doquedb://localhost:54321/xxx_DB_xxx";
	}

	// DriverManager.getConnection(String) のテスト
	public void test_getConnection1() throws Exception
	{
		// 認識可能なはずの URL を設定すれば接続できるはず
		Connection	c = null;
		String	NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		String	SQLState = "";
		try {
			c = DriverManager.getConnection(URL);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// 認識不能なはずの URL を設定すると SQLException が発生するはず

		// URL 未設定
		NoSuitableSQLState = "08001";	// java.sql.DriverManager が決めている SQLState
		SQLState = "";
		try {
			c = DriverManager.getConnection(null);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// まったくでたらめな URL
		SQLState = "";
		try {
			c = DriverManager.getConnection("foo");
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// 不正なホスト名
		String	CannotConnectSQLState = (new CannotConnect("xxx_HOST_xxx", 54321)).getSQLState();
		SQLState = "";
		try {
			c = DriverManager.getConnection(getIllegalHostNameURL());
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(CannotConnectSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// 不正なポート番号
		String	ConnectionRanOutSQLState = (new ConnectionRanOut()).getSQLState();
		SQLState = "";
		try {
			c = DriverManager.getConnection(getIllegalPortNumberURL());
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(ConnectionRanOutSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// v15.0 では、存在しないデータベース名を指定しても接続時には検出できない
		assertNotNull(c = DriverManager.getConnection(getIllegalDatabaseNameURL(), _user, _password));
		if (c != null) {
			c.close();
			c = null;
		}
	}

	// DriverManager.getConnection(String, java.util.Properties) のテスト
	public void test_getConnection2() throws Exception
	{
		Connection	c = null;

		// 認識可能なはずの URL を設定すれば、プロパティ未指定でも接続できるはず
		String	NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		String	SQLState = "";
		try {
			c = DriverManager.getConnection(URL, null);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// 認識可能なはずの URL を設定すれば、空のプロパティでも接続できるはず
		java.util.Properties	props = new java.util.Properties();

		NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		SQLState = "";
		try {
			c = DriverManager.getConnection(URL, props);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		if (c != null) {
			c.close();
			c = null;
		}
		
		// 登録されていないユーザー名を指定すると SQLException が発生するはず(v16.2)
		props.put("user", "user");

		NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		SQLState = "";
		try {
			c = DriverManager.getConnection(URL, props);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		if (c != null) {
			c.close();
			c = null;
		}

		// 認識可能なはずの URL を設定すれば、パスワードを指定しなくても接続できるはず
		props.put("user", _user);

		NoSuitableSQLState = "08501";	// jp.co.ricoh.doquedb.exception.ErrorCode.AUTHORIZATION_FAILED
		SQLState = "";
		try {
			c = DriverManager.getConnection(URL, props);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		if (c != null) {
			c.close();
			c = null;
		}

		// 認識可能なはずの URL を設定すれば接続できるはず
		props.put("password", _password);
		assertNotNull(c = DriverManager.getConnection(URL, props));
		if (c != null) {
			c.close();
			c = null;
		}

		// 認識不能なはずの URL を設定すると SQLException が発生するはず

		// URL 未設定
		NoSuitableSQLState = "08001";	// java.sql.DriverManager が決めている SQLState
		SQLState = "";
		try {
			c = DriverManager.getConnection(null, props);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// まったくでたらめな URL
		SQLState = "";
		try {
			c = DriverManager.getConnection("foo", props);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// 不正なホスト名
		String	CannotConnectSQLState = (new CannotConnect("xxx_HOST_xxx", 54321)).getSQLState();
		SQLState = "";
		try {
			c = DriverManager.getConnection(getIllegalHostNameURL(), props);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(CannotConnectSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// 不正なポート番号
		String	ConnectionRanOutSQLState = (new ConnectionRanOut()).getSQLState();
		SQLState = "";
		try {
			c = DriverManager.getConnection(getIllegalPortNumberURL(), props);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(ConnectionRanOutSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// v15.0 では、存在しないデータベース名を指定しても接続時には検出できない
		assertNotNull(c = DriverManager.getConnection(getIllegalDatabaseNameURL(), props));
		if (c != null) {
			c.close();
			c = null;
		}
	}

	// DriverManager.getConnection(String, String, String) のテスト
	public void test_getConnection3() throws Exception
	{
		Connection	c = null;

		// 認識可能なはずの URL を設定すれば、ユーザ名もパスワードも指定しなくても接続できるはず
		String	NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		String	SQLState = "";
		try {
			c = DriverManager.getConnection(URL, null, null);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		if (c != null) {
			c.close();
			c = null;
		}

		// 認識可能なはずの URL を設定すれば、ユーザ名が空でパスワードを指定しなくても接続できるはず
		NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		SQLState = "";
		try {
			c = DriverManager.getConnection(URL, "", null);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		if (c != null) {
			c.close();
			c = null;
		}

		// 認識可能なはずの URL を設定すれば、ユーザ名を指定せずパスワードが空でも接続できるはず
		NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		SQLState = "";
		try {
			c = DriverManager.getConnection(URL, null, "");
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		if (c != null) {
			c.close();
			c = null;
		}

		// 認識可能なはずの URL を設定すれば、ユーザ名もパスワードも空でも接続できるはず
		NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		SQLState = "";
		try {
			c = DriverManager.getConnection(URL, "", "");
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		if (c != null) {
			c.close();
			c = null;
		}
		
		// パスワードを間違えると SQLException が発生するはず(v16.2)
		NoSuitableSQLState = "08501";	// jp.co.ricoh.doquedb.exception.ErrorCode.AUTHORIZATION_FAILED
		SQLState = "";
		try {
			c = DriverManager.getConnection(URL, _user, "user");
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		if (c != null) {
			c.close();
			c = null;
		}
		
		// 登録されていないユーザー名を指定すると SQLException が発生するはず(v16.2)
		NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		SQLState = "";
		try {
			c = DriverManager.getConnection(URL, "user", _password);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		if (c != null) {
			c.close();
			c = null;
		}

		// 認識不能なはずの URL を設定すると SQLException が発生するはず

		// URL 未設定
		NoSuitableSQLState = "08001";	// java.sql.DriverManager が決めている SQLState
		SQLState = "";
		try {
			c = DriverManager.getConnection(null, _user, _password);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// まったくでたらめな URL
		SQLState = "";
		try {
			c = DriverManager.getConnection("foo", _user, _password);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// 不正なホスト名
		String	CannotConnectSQLState = (new CannotConnect("xxx_HOST_xxx", 54321)).getSQLState();
		SQLState = "";
		try {
			c = DriverManager.getConnection(getIllegalHostNameURL(), _user, _password);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(CannotConnectSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// 不正なポート番号
		String	ConnectionRanOutSQLState = (new ConnectionRanOut()).getSQLState();
		SQLState = "";
		try {
			c = DriverManager.getConnection(getIllegalPortNumberURL(), _user, _password);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(ConnectionRanOutSQLState, SQLState);
		if (c != null) {
			c.close();
			c = null;
		}

		// v15.0 では、存在しないデータベース名を指定しても接続時には検出できない
		assertNotNull(c = DriverManager.getConnection(getIllegalDatabaseNameURL(), _user, _password));
		if (c != null) {
			c.close();
			c = null;
		}
	}

	// DriverManager.getDriver() のテスト
	public void test_getDriver() throws Exception
	{
		// 認識可能なはずの URL を設定すれば Driver オブジェクトが得られるはず
		assertNotNull(DriverManager.getDriver(URL));

		// 認識不能なはずの URL を設定すると SQLException が発生するはず

		// URL 未設定
		String	NoSuitableSQLState = "08001";	// java.sql.DriverManager が決めている SQLState
		String	SQLState = "";
		try {
			DriverManager.getDriver(null);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		// まったくでたらめな URL
		SQLState = "";
		try {
			DriverManager.getDriver("foo");
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		// 不正なホスト名を指定しても Driver オブジェクトは得られるはず
		assertNotNull(DriverManager.getDriver(getIllegalHostNameURL()));

		// 不正なポート番号を指定しても Driver オブジェクトは得られるはず
		assertNotNull(DriverManager.getDriver(getIllegalPortNumberURL()));

		// 不正なデータベース名を指定しても Driver オブジェクトは得られるはず
		assertNotNull(DriverManager.getDriver(getIllegalDatabaseNameURL()));
	}

	// データベースを準備する
	public void setUp()	throws Exception
	{
		super.setUp();
	}

	// データベースを削除する
	public void tearDown() throws Exception
	{
		super.tearDown();
	}
}

//
// Copyright (c) 2004, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
