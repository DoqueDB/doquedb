// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.sydney.admin.util;

import java.util.ArrayList;
import java.util.List;

public class StringSplit {
    public StringSplit() {

    }

    public static String[] split(String value, String delimiter) {
        List<String> list = splitList(value, delimiter);
        return (String[])list.toArray(new String[list.size()]);
    }

    public static List<String> splitList(String value, String delimiter) {
        int i = 0;

        // 分割した文字を格納する変数
        int len = delimiter.length();
        List<String> list = new ArrayList<String>();
        int j = value.indexOf(delimiter);
        for (; j >= 0; ) {
          list.add(value.substring(i, j));
          //i = j + 1;
          i = j + len;
          j = value.indexOf(delimiter, i);
        }
        list.add(value.substring(i));
        return list;
    }

}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
