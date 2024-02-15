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
import java.sql.*;

//	create table T (
//		ID			int,
//		Name		nvarchar(100),
//		ThreadID	nvarchar(100)
//	)

class Client implements Runnable {

	private static String	URL = "jdbc:ricoh:doquedb://localhost:54321/TEST";

	private String	threadID;
	private int	start;

	public Client(int	start_) {
		start = start_;
		StringBuffer	tid = new StringBuffer("T");
		if (start_ < 10) tid.append("0");
		if (start_ < 100) tid.append("0");
		tid.append(start);
	}

	public void run() {

		long	i = start;
		while (true) {

			try {

				Connection	c = DriverManager.getConnection(URL);
				c.setAutoCommit(false);
				c.setReadOnly(false);
				Statement	s = c.createStatement();
				s.executeUpdate("insert into T (ID, Name, ThreadID) values (" + i + ", '" + URL + "', '" + threadID + "')");
				c.commit();
				c.setReadOnly(true);
				ResultSet	rs = s.executeQuery("select count(*) from T");
				rs.next();
				rs.close();
				c.commit();
				c.setReadOnly(false);
				s.executeUpdate("update T set Name = 'hogehoge' where ID = " + (i - 2));
				c.commit();
				s.close();
				c.close();
			} catch (java.sql.SQLException	sqle) {
				System.err.println("EXCEPTION OCCURED : " + sqle.getMessage());
				sqle.printStackTrace();
			}
			i++;
		}
	}
}

public class ManyClientsAbort
{
	private static String	SYS_URL = "jdbc:ricoh:doquedb://localhost:54321/$$SystemDB";
	private static String	URL = "jdbc:ricoh:doquedb://localhost:54321/TEST";

	public static void main(String[]	Args_)
	{
		java.sql.Connection	c = null;
		java.sql.Statement	s = null;

		try {

			Class.forName("jp.co.ricoh.doquedb.jdbc.Driver");
			c = DriverManager.getConnection(SYS_URL);
			s = c.createStatement();
			s.executeUpdate("create database TEST");
			s.close();
			c.close();

			c = DriverManager.getConnection(URL);
			s = c.createStatement();
			s.executeUpdate("create table T (ID int, Name nvarchar(100), ThreadID nvarchar(100))");
			s.close();
			c.close();

			for (int i = 0; i < 3; i++) {
				Runnable	clientRun = new Client(i);
				Thread	clientThread = new Thread(clientRun);
				clientThread.start();
			}

			Thread.sleep(30000);

			System.exit(1);

		} catch (java.sql.SQLException	sqle) {

			sqle.printStackTrace();

		} catch (java.lang.ClassNotFoundException	cnfe) {

			cnfe.printStackTrace();

		} catch (java.lang.InterruptedException	ie) {

			ie.printStackTrace();

		} finally {

			try {

				if (s != null) s.close();
				if (c != null) c.close();

			} catch (java.sql.SQLException	sqle) {

				sqle.printStackTrace();
			}
		}
	}
}
