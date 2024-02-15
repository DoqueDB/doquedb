// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DriverTest.java -- jp.co.ricoh.doquedb.jdbc.Driver クラスのテスト
// 
// Copyright (c) 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
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

public class DriverTest extends TestBase
{
	// 現在の JDBC ドライバのメジャーバージョン
	// ※ jp.co.ricoh.doquedb.jdbc.Driver.MAJOR_VERSION が更新された場合、
	// 　 この値も等値に更新する必要がある。
	private final static int	CURRENT_MAJOR_VERSION = 2;

	// 現在の JDBC ドライバのマイナーバージョン
	// ※ jp.co.ricoh.doquedb.jdbc.Driver.MINOR_VERSION が更新された場合、
	// 　 この値も等値に更新する必要がある。
	private final static int	CURRENT_MINOR_VERSION = 0;

	public DriverTest(String nickname)
	{
		super(nickname);
	}

	// Driver.acceptsURL() のテスト
	public void test_acceptsURL() throws Exception
	{
		Driver driver = super.getDriver();

		// 認識可能なはずの URL
		assertTrue(driver.acceptsURL(URL));

		// 以下、認識不能なはずの URL
		assertFalse(driver.acceptsURL("foo"));
	}

	// Driver.getPropertyInfo() のテスト
	public void test_getPropertyInfo() throws Exception
	{
		Driver	driver = super.getDriver();

		// 現在のバージョンでは、必ず空の配列を返すはず
		DriverPropertyInfo	propInfo[] = driver.getPropertyInfo(URL, null);
		assertZero(propInfo.length);

		// 例え、認識不能な URL でも、必ず空の配列を返すはず
		propInfo = driver.getPropertyInfo("foo", null);
		assertZero(propInfo.length);
	}

	// Driver.getMajorVersion() のテスト
	public void test_getMajorVersion() throws Exception
	{
		Driver	driver = super.getDriver();

		// 必ず CURRENT_MAJOR_VERSION と等しいはず
		assertEquals(CURRENT_MAJOR_VERSION, driver.getMajorVersion());
	}

	// Driver.getMinorVersion() のテスト
	public void test_getMinorVersion() throws Exception
	{
		Driver	driver = super.getDriver();

		// 必ず CURRENT_MINOR_VERSION と等しいはず
		assertEquals(CURRENT_MINOR_VERSION, driver.getMinorVersion());
	}

	// Driver.jdbcCompliant() のテスト
	public void test_jdbcCompliant() throws Exception
	{
		Driver	driver = super.getDriver();

		// 現在のバージョンでは、JDBC Compliant ではないはず
		assertFalse(driver.jdbcCompliant());
	}

	// Driver.connect() のテスト
	public void test_connect() throws Exception
	{
		Driver	driver = super.getDriver();

		Connection	c = null;

		// 認識可能なはずの URL を設定すれば、プロパティ未指定でも接続できるはず
		String	NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		String	SQLState = "";
		try {
			c = driver.connect(URL, null);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		// 認識可能なはずの URL を設定すれば、空のプロパティでも接続できるはず
		java.util.Properties	props = new java.util.Properties();

		NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		SQLState = "";
		try {
			c = driver.connect(URL, props);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);
		
		// 認識可能なはずの URL を設定すれば、パスワードを指定しなくても接続できるはず
		props.setProperty("user", _user);

		NoSuitableSQLState = "08501";	// jp.co.ricoh.doquedb.exception.ErrorCode.AUTHORIZATION_FAILED
		SQLState = "";
		try {
			c = driver.connect(URL, props);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		// 認識可能なはずの URL を設定すれば接続できるはず
		props.setProperty("password", _password);
		assertNotNull(c = driver.connect(URL, props));
		c.close();
		
		// 登録されていないユーザー名を指定すると SQLException が発生するはず(v16.2)
		props.put("user", "user");
		NoSuitableSQLState = "28501";	// jp.co.ricoh.doquedb.exception.ErrorCode.USER_NOT_FOUND
		SQLState = "";
		try {
			c = driver.connect(URL, props);
		} catch (SQLException sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NoSuitableSQLState, SQLState);

		if (c != null) {
			c.close();
			c = null;
		}

		// 認識不能なはずの URL では例外が発生せずに null が返されるだけのはず
		assertNull(c = driver.connect("foo", props));
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
// Copyright (c) 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
