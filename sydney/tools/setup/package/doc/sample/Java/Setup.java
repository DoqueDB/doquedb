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
// 青空文庫のデータをもとにDBを立ち上げ、本文の全文索引を作成する

import java.sql.*;

public class Setup {
    public static void main (String[] args) {
        // Driverクラスのインポート
        try {
            Class.forName("jp.co.ricoh.doquedb.jdbc.Driver");
        } catch (ClassNotFoundException cne) {
            System.err.println("error" + ": " + cne);
            return;
        };

        // データベースの設定
        String url = "jdbc:ricoh:doquedb://localhost:54321/";
        String user = "root";
        String password = "doqadmin";
        Connection conn = null;
        Statement statement = null;
        ResultSet resultSet = null;

        // カレントディレクトリの取得
        String dir = System.getProperty("user.dir");
        
        // データベースの作成
        try {
            System.out.println("データベースの作成を開始");
            conn = DriverManager.getConnection(url, user, password);
            statement = conn.createStatement();
            String query = "create database sampleJava";
            resultSet = statement.executeQuery(query);
        } catch (Exception e) {
            System.err.println("error" + ": " + e);
            return;
        } finally {
            try {
                if (resultSet!=null) resultSet.close();
                if (statement!=null) statement.close();
                if (conn!=null) conn.close();
            } catch (Exception e) {};
        };

        // 立ち上げた DB に再接続
        url = "jdbc:ricoh:doquedb://localhost:54321/sampleJava";
        try {
            conn = DriverManager.getConnection(url, user, password);
        } catch (Exception e) {
            System.err.println("error" + ": " + e);
            return;
        };

        // テーブルの作成
        try {
            System.out.println("テーブルの作成を開始");
            statement = conn.createStatement();
            String query = "create table "
                + "AozoraBunko ("
                    + "docId int, "
                    + "title nvarchar(256), "
                    + "lastName nvarchar(128), "
                    + "firstName nvarchar(128), "
                    + "url varchar(128), "
                    + "content ntext, "
                    + "primary key(docId))";
            resultSet = statement.executeQuery(query);
        } catch (Exception e) {
            System.err.println("error" + ": " + e);
            return;
        } finally {
            try {
                if (resultSet!=null) resultSet.close();
                if (statement!=null) statement.close();
            } catch (Exception e) {};
        };

        // バッチインサートの実施
        try {
            System.out.println("バッチインサートを開始");
            statement = conn.createStatement();
            String query = String.format(
                "insert into AozoraBunko "
                + "input from path '%s/../data/insert.csv' "
                + "hint 'code=\"utf-8\" InputField=(1,2,16,17,51,57)'"
                , dir);
            resultSet = statement.executeQuery(query);
        } catch (Exception e) {
            System.err.println("error" + ": " + e);
            return;
        } finally {
            try {
                if (resultSet!=null) resultSet.close();
                if (statement!=null) statement.close();
            } catch (Exception e) {};
        };

        // 全文索引の作成
        try {
            System.out.println("全文索引の作成を開始");
            statement = conn.createStatement();
            String query =  "create fulltext index INDEX1 on AozoraBunko(content) "
                + "hint 'kwic, "
                    + "delayed, "
                    + "inverted=(normalized=(stemming=false, deletespace=false), "
                    + "indexing=dual, "
                    + "tokenizer=DUAL:JAP:ALL:1 @NORMRSCID:1 @UNARSCID:1)'";
            resultSet = statement.executeQuery(query);
        } catch (Exception e) {
            System.err.println("error" + ": " + e);
            return;
        } finally {
            try {
                if (resultSet!=null) resultSet.close();
                if (statement!=null) statement.close();
            } catch(Exception e) {};
        };

        try {
            if (conn!=null) conn.close();
        } catch (Exception e) {};
    };
}
