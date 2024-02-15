// 
// Copyright (c) 2023 Ricoh Company, Ltd.
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


import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;

import javax.sql.ConnectionEvent;
import javax.sql.ConnectionEventListener;
import javax.sql.PooledConnection;
import javax.transaction.xa.XAException;
import javax.transaction.xa.Xid;

import jp.co.ricoh.doquedb.jdbcx.ConnectionPoolDataSource;
import jp.co.ricoh.doquedb.jdbcx.XAConnection;
import jp.co.ricoh.doquedb.jdbcx.XADataSource;
import junit.framework.TestCase;

public class PooledConnectionTest extends TestCase {
	
	private ConnectionPoolDataSource cpd;
	
	protected void setUp() throws Exception {
		super.setUp();

		cpd = new ConnectionPoolDataSource();
		cpd.setDatabaseName("testdb");
		cpd.setPassword("doqadmin");
		cpd.setPortNumber(54321);
		cpd.setServerName("localhost");
		cpd.setUser("root");

		
	}

	protected void tearDown() throws Exception {
		super.tearDown();
	}

	public void testGetPooledConnection() {
		try
		{
			PooledConnection pc = cpd.getPooledConnection();
			
			Connection con1 = pc.getConnection();
			System.out.println("con1:" + con1.toString());
			con1.close();
			//cpd.setUser("root");
			Connection con2 = pc.getConnection();
			System.out.println("con2:" + con2.toString());
			con2.close();
			pc.close();
			
//			ConnectionPoolDataSource cpd2 = new ConnectionPoolDataSource();
//			cpd2.setDatabaseName("testdb");
//			cpd2.setPortNumber(54321);
//			cpd2.setPassword("doqadmin");
//			cpd2.setUser("sydadmin");
//			cpd2.setServerName("localhost");
			pc = cpd.getPooledConnection();
			
			Connection con3 = pc.getConnection();
			System.out.println("con3:" + con3.toString());
			Connection con4 = pc.getConnection();
			System.out.println("con4:" + con4.toString());
		}
		catch(SQLException ex)
		{
			System.out.println(ex.getMessage());
		}


	}

}
