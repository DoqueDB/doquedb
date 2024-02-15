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


import java.sql.ResultSet;
import java.sql.SQLException;

import javax.transaction.xa.XAException;
import javax.transaction.xa.Xid;


import jp.co.ricoh.doquedb.jdbcx.XAConnection;
import jp.co.ricoh.doquedb.jdbcx.XADataSource;
import junit.framework.TestCase;

public class XADataSourceTest extends TestCase {
	
	private XADataSource xaDs;
	private XADataSource xaDs2;
	
	protected void setUp() throws Exception {
		super.setUp();
		xaDs = new XADataSource();
		xaDs.setPortNumber(54321);
		xaDs.setDatabaseName("defaultDB");
		try
		{
			xaDs.getConnection().createStatement().executeUpdate("create table test (id INT)");
			
		}
		catch(SQLException ex)
		{
			System.out.println(ex.getMessage());
		}
		try
		{
			xaDs.getConnection().createStatement().executeUpdate("delete from test");
		}
		catch(SQLException ex)
		{
			System.out.println(ex.getMessage());
		}
		xaDs2 = new XADataSource();
		xaDs2.setPortNumber(54321);
		xaDs2.setDatabaseName("defaultDB");
		
	}

	protected void tearDown() throws Exception {
		super.tearDown();
		try
		{
			xaDs.getConnection().createStatement().executeUpdate("drop table test");
		}
		catch (SQLException ex)
		{
			System.out.println(ex.getMessage());
		}
	}

	public void testGetXAConnection() {
		try
		{
			PrepareAndCommitAndRollbackTest();
			CommitOnePhaseAndRollbackTest();
			NoSupportTest();
			RecoverAndForgetTest();
		}
		catch(Exception ex)
		{
			System.out.println(ex.getMessage());
		}

	}
	protected Xid CreateXid(byte globalTransactionID_, byte branchQualifier_, int formatID_) {
		
		byte[] globalTransactionID = new byte[1];
		globalTransactionID[0] = globalTransactionID_;
		byte[] branchQualifier = new byte[1];
		branchQualifier[0] = branchQualifier_;
		int formatID = formatID_;
		
		return new jp.co.ricoh.doquedb.jdbcx.Xid(globalTransactionID, branchQualifier, formatID);
	}
	
	private void PrepareAndCommitAndRollbackTest() throws Exception {
		System.out.println("PrepareAndCommitAndRollbackTest start");
		
		XAConnection xaCon = (XAConnection) xaDs.getXAConnection();
		XAConnection xaCon2 = (XAConnection)xaDs2.getXAConnection();
		
		Xid xid = CreateXid((byte)0,(byte)0,1);
		try {
			xaCon.start(xid, XAConnection.TMNOFLAGS);
			System.out.println("start1");
		}
		catch(XAException ex)
		{
			System.out.print("TMNOFLAGS:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		xaCon.getConnection().createStatement().executeUpdate("insert into test values(1)");
		try {
			xaCon.end(xid, XAConnection.TMFAIL);
			System.out.println("end1");
		}
		catch(XAException ex)
		{
			System.out.print("TMFAIL:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		
		Xid xid2 = CreateXid((byte)1,(byte)0,1);
		try {
			xaCon2.start(xid2, XAConnection.TMNOFLAGS);
			System.out.println("start2");
		}
		catch(XAException ex)
		{
			System.out.print("TMNOFLAGS:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		xaCon2.getConnection().createStatement().executeUpdate("insert into test values(2)");
		xaCon2.getConnection().createStatement().executeUpdate("insert into test values(3)");
		try {
			xaCon2.end(xid2, XAConnection.TMSUCCESS);
			System.out.println("end2");
		}
		catch(XAException ex)
		{
			System.out.print("TMSUCCESS:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		try {
			xaCon.prepare(xid);
			System.out.println("prepare1");
		}
		catch(XAException ex)
		{
			System.out.print("prepare1:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		try {
			xaCon2.prepare(xid2);
			System.out.println("prepare2");
		}
		catch(XAException ex)
		{
			System.out.print("prepare2:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		
		
		ResultSet result = xaDs.getConnection().createStatement().executeQuery("select * from test order by id");
		assertEquals("まだコミットされていない", false, result.next());
		
		try {
			xaCon.rollback(xid);
			System.out.println("rollback1");
		}
		catch(XAException ex)
		{
			System.out.print("rollback1:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		try {
			xaCon2.commit(xid2, false);
			System.out.println("commit2");
		}
		catch(XAException ex)
		{
			System.out.print("commit2:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		ResultSet result2 = xaDs.getConnection().createStatement().executeQuery("select * from test order by id");
		if (result2.next())
		{
			assertEquals("SELECT結果", 2, result2.getInt(1));
			System.out.print("SELECT結果");
			System.out.println(result2.getInt(1));
		}
		if (result2.next())
		{
			assertEquals("SELECT結果", 3, result2.getInt(1));
			System.out.print("SELECT結果");
			System.out.println(result2.getInt(1));
		}
		if (result2.next())
		{
			System.out.print("SELECT結果");
			System.out.println(result2.getInt(1));
		}
	}
	
	private void CommitOnePhaseAndRollbackTest() throws Exception {
		System.out.println("CommitOnePhaseAndRollbackTest start");
		
		XAConnection xaCon = (XAConnection) xaDs.getXAConnection();
		XAConnection xaCon2 = (XAConnection)xaDs2.getXAConnection();
		
		Xid xid = CreateXid((byte)0,(byte)0,1);
		try {
			xaCon.start(xid, XAConnection.TMNOFLAGS);
			System.out.println("start1");
		}
		catch(XAException ex)
		{
			System.out.print("TMNOFLAGS:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		xaCon.getConnection().createStatement().executeUpdate("insert into test values(4)");
		try {
			xaCon.end(xid, XAConnection.TMSUCCESS);
			System.out.println("end1");
		}
		catch(XAException ex)
		{
			System.out.print("TMSUCCESS:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		Xid xid2 = CreateXid((byte)1,(byte)0,1);
		try
		{
			xaCon2.start(xid2, XAConnection.TMNOFLAGS);
			System.out.println("start2");
		}
		catch(XAException ex)
		{
			System.out.print("TMNOFLAGS:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}	
		xaCon2.getConnection().createStatement().executeUpdate("insert into test values(5)");
		xaCon2.getConnection().createStatement().executeUpdate("insert into test values(6)");
		try
		{
			xaCon2.end(xid2, XAConnection.TMSUCCESS);
			System.out.println("end2");
		}
		catch(XAException ex)
		{
			System.out.print("TMSUCCESS:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		ResultSet result = xaDs.getConnection().createStatement().executeQuery("select * from test order by id desc");
		if (result.next())
			assertEquals("まだコミットされていない", 3, result.getInt(1));
		if (result.next())
			assertEquals("まだコミットされていない", 2, result.getInt(1));
		try
		{
			xaCon.rollback(xid);
			System.out.println("rollback1");
		}
		catch(XAException ex)
		{
			System.out.print("rollback1:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		try {
			xaCon2.commit(xid2, true);
			System.out.println("commit2");
		}
		catch(XAException ex)
		{
			System.out.print("commit2:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		
		ResultSet result2 = xaDs.getConnection().createStatement().executeQuery("select * from test order by id desc");
		if (result2.next())
			assertEquals("結果", 6, result2.getInt(1));
		if (result2.next())
			assertEquals("結果", 5, result2.getInt(1));
	}

	private void NoSupportTest() throws Exception {
		System.out.println("NoSupport test start");
		
		XAConnection xaCon = (XAConnection) xaDs.getXAConnection();
		XAConnection xaCon2 = (XAConnection)xaDs2.getXAConnection();
		
		Xid xid = CreateXid((byte)0,(byte)0,1);
		try {
			xaCon.start(xid, XAConnection.TMJOIN);
		}
		catch(XAException ex)
		{
			System.out.print("TMJOIN:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		try
		{
			xaCon2.start(xid, XAConnection.TMRESUME);
		}
		catch(XAException ex)
		{
			System.out.print("TMRESUME:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
		try
		{
			xaCon2.end(xid, XAConnection.TMSUSPEND);
		}
		catch(XAException ex)
		{
			System.out.print("TMSUSPEND:");
			System.out.print(ex.errorCode);
			System.out.println(ex.getMessage());
		}
	}

	private void RecoverAndForgetTest() throws SQLException, XAException  {
		System.out.println("recover&forget test start");
		XAConnection xaCon = null;
		
		xaCon = (XAConnection) xaDs.getXAConnection();
		
		Xid[] xids = null;
		Xid[] xids2 = null;
		Xid[] xids3 = null;
		xids = xaCon.recover(XAConnection.TMSTARTRSCAN);
		xids2 = xaCon.recover(XAConnection.TMNOFLAGS);
		xids3 = xaCon.recover(XAConnection.TMENDRSCAN);
		System.out.print("TMSTARTRSCAN:");
		System.out.println(xids.length);
		System.out.print("TMNOFLAGS:");
		System.out.println(xids2.length);
		System.out.print("TMENDRSCAN:");
		System.out.println(xids3.length);
		for ( int i = 0 ; i < xids.length ; i++) {
			try {
				xaCon.rollback(xids[i]);
			}catch(XAException ex){
				System.out.print("rollback:");
				System.out.println(ex.getMessage());
			}
		}
		
		for ( int i = 0 ; i < xids.length ; i++) {
			try {
				xaCon.commit(xids[i], false);
			}catch(XAException ex){
				System.out.print("commit:");
				System.out.println(ex.getMessage());
			}
		}
		
		for ( int i = 0 ; i < xids.length ; i++) {
			try {
				xaCon.forget(xids[i]);
			}catch(XAException ex){
				System.out.print("forget:");
				System.out.println(ex.getMessage());
			}
		}
	}
	
	protected XADataSource getXADataSource() {
		return xaDs;
	}
}
