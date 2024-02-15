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

import java.sql.SQLException;

import javax.transaction.xa.XAException;
import javax.transaction.xa.Xid;

import jp.co.ricoh.doquedb.jdbcx.XAConnection;
import jp.co.ricoh.doquedb.jdbcx.XADataSource;

public class XADataSourceTest2 extends XADataSourceTest {

	public void testGetXAConnection() {
		XADataSource xaDs = getXADataSource();
				
		try {
			//xaDs.getConnection().createStatement().executeUpdate("create table test2 (id INT)");
			XAConnection xaCon = (XAConnection) xaDs.getXAConnection();
			Xid xid = CreateXid((byte)2,(byte)0,1);
			xaCon.start(xid, XAConnection.TMNOFLAGS);
			xaCon.getConnection().createStatement().executeUpdate("insert into test values(4)");
			xaCon.end(xid, XAConnection.TMSUCCESS);
			xaCon.prepare(xid);
			Xid xid2 = CreateXid((byte)3,(byte)0,1);
			xaCon.start(xid2, XAConnection.TMNOFLAGS);
			xaCon.getConnection().createStatement().executeUpdate("insert into test values(5)");
			xaCon.end(xid2, XAConnection.TMSUCCESS);
			xaCon.prepare(xid2);
			Xid xid3 = CreateXid((byte)4,(byte)0,1);
			xaCon.start(xid3, XAConnection.TMNOFLAGS);
			xaCon.getConnection().createStatement().executeUpdate("insert into test values(6)");
			xaCon.end(xid3, XAConnection.TMSUCCESS);
			xaCon.prepare(xid3);
			
		} catch (XAException ex) {
			fail(ex.getMessage());
		} catch (SQLException ex) {
			fail(ex.getMessage());
		}
	}
	

}
