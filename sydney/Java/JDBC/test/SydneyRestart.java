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

public class SydneyRestart
{
	private static String	SYS_URL = "jdbc:ricoh:doquedb://localhost:54321/$$SystemDB";

	public static void main(String[]	Args_)
	{
		java.sql.Connection	c = null;
		java.sql.Statement	s = null;
		java.sql.ResultSet	rs = null;

		try {

			Class.forName("jp.co.ricoh.doquedb.jdbc.Driver");

			for (int i = 0; i < 3; i++) {

				if (i > 0) {
					byte[]	buff = new byte[1024];
					System.out.print("Sydney サービスを再起動し、[Enter] キーを押してください。");
					System.in.read(buff);
				}

				c = DriverManager.getConnection(SYS_URL);
				s = c.createStatement();
				rs = s.executeQuery("select * from System_Database");
				while (rs.next());
				rs.close();	rs = null;
				s.close();	s = null;
				c.close();	c = null;
			}

		} catch (java.sql.SQLException	sqle) {

			sqle.printStackTrace();

		} catch (java.lang.ClassNotFoundException	cnfe) {

			cnfe.printStackTrace();

		} catch (java.io.IOException	ioe) {

			ioe.printStackTrace();

		} finally {

			try {

				if (rs != null) rs.close();
				if (s != null) s.close();
				if (c != null) c.close();

			} catch (java.sql.SQLException	sqle) {

				sqle.printStackTrace();
			}
		}
	}
}
