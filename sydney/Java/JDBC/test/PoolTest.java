// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PoolTest.java -- プールコネクションのテスト
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintWriter;

public class PoolTest extends TestBase
{
	public PoolTest(String	nickname_)
	{
		super(nickname_);
	}

	public void test_Pool() throws Exception
	{
		jp.co.ricoh.doquedb.jdbcx.DataSource	dataSource = new jp.co.ricoh.doquedb.jdbcx.DataSource();
		if(dataSource!=null){
			System.out.print(dataSource.toString() + "\n");
			dataSource.setUrl("jdbc:ricoh:doquedb://localhost:54321/TEST");
//			dataSource.setServerName("localhost");
//			dataSource.setPortNumber(54321);
//			dataSource.setDatabaseName("TEST");
			dataSource.setUser("root");
			dataSource.setPassword("doqadmin");
			int nCount = dataSource.getMaxConnections();
			System.out.print("MaxConnections = " + nCount + "\n");
			dataSource.setMaxConnections(5);
			nCount = dataSource.getMaxConnections();
			System.out.print("MaxConnections = " + nCount + "\n");
			dataSource.setLoginTimeout(5);

			// LogWriterのテスト
			PrintWriter pw = dataSource.getLogWriter();
			if(pw!=null){
				System.out.print("LogWriter is not null\n");
			}
			File fl = new File("d:\\temp\\pooltest.log");
			FileOutputStream fos = null;
			try{
				fos = new FileOutputStream(fl, true);
			}catch(FileNotFoundException e){
				e.printStackTrace();
			}
			PrintWriter npw = new PrintWriter(fos);
			dataSource.setLogWriter(npw);

			java.sql.Connection co1 = dataSource.getConnection();
			System.out.print("co1 = " + co1.toString() + "\n");
			java.sql.Statement st1 = co1.createStatement();
			int nRet = st1.executeUpdate("create table pooltest( n_Data int, t_Data ntext);");
			nRet = st1.executeUpdate("insert into pooltest(n_Data, t_Data)values( 1, 'test1');insert into pooltest(n_Data, t_Data)values( 2, 'test2');insert into pooltest(n_Data, t_Data)values( 3, 'test3');");
			System.out.print("nRet = " + nRet + "\n");
			st1.close();st1=null;
			co1.close();co1=null;

			// 同じConnectionが取得出来る事
			java.sql.Connection co2 = dataSource.getConnection();
			System.out.print("co2 = " + co2.toString() + "\n");
			java.sql.Statement st2 = co2.createStatement();
			java.sql.ResultSet rs2 = st2.executeQuery("select count(*) from pooltest");
			int n_Data = 0;
			if(rs2.next()){
				n_Data=rs2.getInt(1);
			}
			System.out.print("n_Data = " + n_Data + "\n");
			rs2.close();rs2=null;
			st2.close();st2=null;
			co2.close();co2=null;

			for(int nLoop=0;nLoop<3;nLoop++){
				// 別もののConnectionがmax=5個まで取得出来る個と
				co1 = dataSource.getConnection();
				System.out.print("co1 = " + co1.toString() + "\n");
				co2 = dataSource.getConnection();
				System.out.print("co2 = " + co2.toString() + "\n");
				java.sql.Connection co3 = dataSource.getConnection();
				System.out.print("co3 = " + co3.toString() + "\n");
				java.sql.Connection co4 = dataSource.getConnection();
				System.out.print("co4 = " + co4.toString() + "\n");
				java.sql.Connection co5 = dataSource.getConnection();
				System.out.print("co5 = " + co5.toString() + "\n");
				java.sql.Connection co6 = null;
				try{
					co6 = dataSource.getConnection();
				}catch(jp.co.ricoh.doquedb.exception.LockTimeout e){
					// ok
					System.out.print("OK! Pool max time out" + "\n");
				}

				co1.close();co1=null;
				co6 = dataSource.getConnection();
				System.out.print("co6 = " + co6.toString() + "\n");
				co2.close();co1=null;
				co3.close();co1=null;
				co4.close();co1=null;
				co5.close();co1=null;
				co6.close();co1=null;
			}

			// LogWriter終了
			pw = dataSource.getLogWriter();
			if(pw!=null){
				pw.close();
			}
		}else{
			System.out.print("DataSource is null\n");
		}
	}




	public void test_Pool_Error() throws Exception
	{
		// 不正ホスト
		{
			jp.co.ricoh.doquedb.jdbcx.DataSource	dataSource = new jp.co.ricoh.doquedb.jdbcx.DataSource();
			if(dataSource!=null){
				// 不正Port
				dataSource.setServerName("hogehoge");
				dataSource.setPortNumber(54321);
				dataSource.setDatabaseName("TEST");
				dataSource.setUser("root");
				dataSource.setPassword("doqadmin");
				dataSource.setMaxConnections(5);
				dataSource.setLoginTimeout(5);

				try{
					java.sql.Connection co1 = dataSource.getConnection();
				}catch(SQLException e){
					System.out.print("OK! Bad HostName\n");
				}
			}else{
				System.out.print("DataSource is null\n");
			}
		}

		// 不正ポート
		{
			jp.co.ricoh.doquedb.jdbcx.DataSource	dataSource = new jp.co.ricoh.doquedb.jdbcx.DataSource();
			if(dataSource!=null){
				// 不正Port
				dataSource.setPortNumber(12345);
				dataSource.setDatabaseName("TEST");
				dataSource.setUser("user1");
				dataSource.setPassword("user1");
				dataSource.setMaxConnections(5);
				dataSource.setLoginTimeout(5);

				try{
					java.sql.Connection co1 = dataSource.getConnection();
				}catch(SQLException e){
					System.out.print("OK! Bad Port Number\n");
				}
			}else{
				System.out.print("DataSource is null\n");
			}
		}

		// 不正データベース名
		{
			jp.co.ricoh.doquedb.jdbcx.DataSource	dataSource = new jp.co.ricoh.doquedb.jdbcx.DataSource();
			if(dataSource!=null){
				dataSource.setServerName("localhost");
				dataSource.setPortNumber(54321);
				dataSource.setDatabaseName("auau");
				dataSource.setUser("root");
				dataSource.setPassword("doqadmin");
				dataSource.setMaxConnections(5);
				dataSource.setLoginTimeout(5);

				java.sql.Connection co1 = dataSource.getConnection();
				java.sql.Statement st1 = co1.createStatement();
				try{
					int nRet = st1.executeUpdate("create table pooltest( n_Data int, t_Data ntext);");
				}catch(SQLException e){
					System.out.print("OK! Bad DataBase Name\n");
				}
				st1.close();st1=null;
				co1.close();co1=null;
				dataSource.close();
			}else{
				System.out.print("DataSource is null\n");
			}
		}

		// ホスト省略
		{
			jp.co.ricoh.doquedb.jdbcx.DataSource	dataSource = new jp.co.ricoh.doquedb.jdbcx.DataSource();
			if(dataSource!=null){
//					dataSource.setServerName("localhost");
				dataSource.setPortNumber(54321);
				dataSource.setDatabaseName("TEST");
				dataSource.setUser("root");
				dataSource.setPassword("doqadmin");
				dataSource.setMaxConnections(5);
				dataSource.setLoginTimeout(5);

				java.sql.Connection co1 = dataSource.getConnection();
				java.sql.Statement st1 = co1.createStatement();
				int nRet = st1.executeUpdate("create table pooltest( n_Data int, t_Data ntext);");
				nRet = st1.executeUpdate("insert into pooltest(n_Data, t_Data)values( 1, 'test1');insert into pooltest(n_Data, t_Data)values( 2, 'test2');insert into pooltest(n_Data, t_Data)values( 3, 'test3');");
				st1.close();st1=null;
				co1.close();co1=null;
				dataSource.close();
				System.out.print("OK! ServerName is not set\n");
			}else{
				System.out.print("DataSource is null\n");
			}
		}
		// 不正ユーザー
		{
			jp.co.ricoh.doquedb.jdbcx.DataSource	dataSource = new jp.co.ricoh.doquedb.jdbcx.DataSource();
			if(dataSource!=null){
				
				dataSource.setPortNumber(54321);
				dataSource.setDatabaseName("TEST");
				dataSource.setUser("root1");
				dataSource.setPassword("doqadmin");
				dataSource.setMaxConnections(5);
				dataSource.setLoginTimeout(5);
				try{
					java.sql.Connection co1 = dataSource.getConnection();
					System.out.println(co1.toString());
					co1.close();
				}catch(SQLException e){
					System.out.print("OK! Bad user\n");
				}
			}else{
				System.out.print("DataSource is null\n");
			}
		}
		// 不正パスワード
		{
			jp.co.ricoh.doquedb.jdbcx.DataSource	dataSource = new jp.co.ricoh.doquedb.jdbcx.DataSource();
			if(dataSource!=null){
				
				dataSource.setPortNumber(54321);
				dataSource.setDatabaseName("TEST");
				dataSource.setUser("root");
				dataSource.setPassword("doqadmin1");
				dataSource.setMaxConnections(5);
				dataSource.setLoginTimeout(5);
				try{
					java.sql.Connection co1 = dataSource.getConnection();
					System.out.println(co1.toString());
					co1.close();
				}catch(SQLException e){
					System.out.print("OK! Bad password\n");
				}
			}else{
				System.out.print("DataSource is null\n");
			}
		}
	}
}

//
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
