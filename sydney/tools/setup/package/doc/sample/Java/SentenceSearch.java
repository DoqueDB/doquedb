//
// Copyright (c) 2023 Ricoh Company, Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
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
// ※事前にDoqueDBをインストールし、実行しておくこと
// ※Setup.javaを実行し、青空文庫のDBを立ち上げておくこと
// 自然文検索の例

import java.sql.*;

public class SentenceSearch {
    public static void main (String[] args) {
        // Driverクラスのインポート
        try {
            Class.forName("jp.co.ricoh.doquedb.jdbc.Driver");
        } catch (ClassNotFoundException cnf) {
            System.err.println("error" + ": " + cnf);
            return;
        };

        // データベースの設定
        String url = "jdbc:ricoh:doquedb://localhost:54321/sampleJava";
        String user = "root";
        String password = "doqadmin";
        Connection conn = null;
        Statement statement = null;
        ResultSet resultSet = null;
        ResultSetMetaData metaData = null;

        // DB に接続
        try {
            conn = DriverManager.getConnection(url, user, password);
        } catch (Exception e) {
            System.err.println("error" + ": " + e);
            return;
        };

        String searchWord = "";
        String query = "";

        // 白雪姫を検索
        searchWord = "小人と暮らすお姫さまが悪いおばあさんに毒リンゴを食べさせられる話";
        query = "select docId, score(content), title, lastName, firstName, kwic(content for 150) "
            + "from AozoraBunko "
            + "where content contains freetext('" + searchWord + "') "
            + "order by score(content) "
            + "desc limit 5";
        try {
            System.out.println("自然文検索の実施");
            System.out.println("検索文：" + searchWord + "\n");
            statement = conn.createStatement();

            resultSet = statement.executeQuery(query);
            // メタデータを取得し、カラム名を表示する
            metaData = resultSet.getMetaData();
            int columnCount = metaData.getColumnCount();
            for (int i = 1; i <= columnCount; i++) {
                System.out.print(metaData.getColumnName(i) + ",");
            };
            System.out.println("\n");

            // クエリの実行結果を取得し、表示する
            while (resultSet.next()) {
                int docId = resultSet.getInt(1);
                double score = resultSet.getDouble(2);
                String title = resultSet.getString(3);
                String lastName = resultSet.getString(4);
                String firstName = resultSet.getString(5);
                String content = resultSet.getString(6);
                String output = String.join(
                    ",",
                    String.valueOf(docId),
                    String.valueOf(score),
                    title,
                    lastName,
                    firstName);
                System.out.println(output);
                System.out.println(content);
                System.out.print("\n");
            };
        } catch (Exception e) {
            System.err.println("error" + ": " + e);
            return;
        } finally {
            try {
                if (resultSet!=null) resultSet.close();
                if (statement!=null) statement.close();
            } catch (Exception e) {};
        };

        try {
            if (conn!=null) conn.close();
        } catch (Exception e) {};
    };
}
