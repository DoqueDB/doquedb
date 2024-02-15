// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TestBase.java -- 
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

public class TestBase extends TestCase
{
	protected static String _hostName = "localhost";
	protected static int _portNumber = 54321;
	protected static String	URL
	= "jdbc:ricoh:doquedb://" + _hostName + ":" + _portNumber + "/TEST";
	protected static String _user = "root"; 
	protected static String _password = "doqadmin"; 

	static
	{
		try {
			Class.forName("jp.co.ricoh.doquedb.jdbc.Driver");
			//Class.forName("jp.co.ricoh.sydney.jdbc.Driver");
		} catch (Exception e) {
		}
	}
	
	public TestBase(String nickname)
	{
		super(nickname);
	}

	// ドライバを得る
	protected Driver getDriver() throws Exception
	{
		return DriverManager.getDriver(getURL());
	}
	
	// コネクションを得る
	protected Connection getConnection() throws Exception
	{
		return DriverManager.getConnection(getURL(), _user, _password);
	}

	// データベースを準備する
	public void setUp()	throws Exception
	{
		Connection c = getConnection();
		Statement s = c.createStatement();
		s.executeUpdate("create database TEST");
		s.close();
		c.close();
	}

	// データベースを削除する
	public void tearDown() throws Exception
	{
		Connection c = getConnection();
		Statement s = c.createStatement();
		s.executeUpdate("drop database TEST");
		s.close();
		c.close();
	}

	// 文字列の比較結果が一致しない場合に、ログに双方の文字列が
	// 省略されずに出力するための assertEquals()
	public static void assertEquals(	String	expected,
										String	actual)
	{
		assertEquals((Object)expected, actual);
	}

	public static void assertEquals(	double	expected,
										double	actual)
	{
		int	result = Double.compare(expected, actual);
		if (result != 0) System.out.println("expected : " + expected + ", actual : " + actual);
		assertZero(result);
	}

	// 数値が 0 かどうかをチェックする
	protected static void assertZero(	int	actual)
	{
		assertEquals(0, actual);
	}

	// SQLWarning のチェック
	protected static void assertSQLWarning(	SQLWarning	w,
											int			errorCode,
											String		SQLState,
											String		message,
											boolean		existNext)
	{
		assertNotNull(w);
		assertEquals(errorCode, w.getErrorCode());
		assertEquals(SQLState, w.getSQLState());
		if (message != null && message.length() > 0) assertEquals(message, w.getMessage());
		if (existNext)	assertNotNull(w.getNextWarning());
		else			assertNull(w.getNextWarning());
	}
	protected static void assertSQLWarning(	SQLWarning	w,
											Integer		errorCode,
											String		SQLState,
											String		message,
											boolean		existNext)
	{
		assertSQLWarning(	w,
							errorCode.intValue(),
							SQLState,
							message,
							existNext);
	}
	protected static void assertSQLWarning(	SQLWarning			w,
											java.util.Vector	errorCodes,
											java.util.Vector	SQLStates,
											java.util.Vector	messages)
	{
		int	numberOfWarnings = errorCodes.size();
		for (int i = 0; i < numberOfWarnings; i++) {

			boolean	existNext = (i < (numberOfWarnings - 1));
			assertSQLWarning(	w,
								(Integer)errorCodes.elementAt(i),
								(String)SQLStates.elementAt(i),
								(String)messages.elementAt(i),
								existNext);

			if (existNext) w = w.getNextWarning();
		}
		assertNull(w.getNextWarning());
	}

	// Get an URL for uncrypted connection.
	protected String getURL()
	{
		return URL;
	}
	// Get a host name.
	protected String getHostName()
	{
		return _hostName;
	}
	// Get a port number.
	protected int getPortNumber()
	{
		return _portNumber;
	}
}

//
// Copyright (c) 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
