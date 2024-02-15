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

package jp.co.ricoh.sydney.admin.util.zip;

public class ZipFileEntry {
    private int pathLevel = 0;
    private String file = null;
    private String path = null;
    private String pathFile = null;
    private String absolutePath = null;
    private String absoluteFile = null;
    
    public ZipFileEntry() {}

    public int getPathLevel() {return this.pathLevel;}
    public void setPathLevel(int pathLevel) {this.pathLevel = pathLevel;}

    public String getFile() {return this.file;}
    public void setFile(String file) {this.file = file;}

    public String getPath() {return this.path;}
    public void setPath(String path) {this.path = path;}

    public String getPathFile() {return this.pathFile;}
    public void setPathFile(String pathFile) {this.pathFile = pathFile;}

    public String getAbsolutePath() {return this.absolutePath;}
    public void setAbsolutePath(String absolutePath) {this.absolutePath = absolutePath;}

    public String getAbsoluteFile() {return this.absoluteFile;}
    public void setAbsoluteFile(String absoluteFile) {this.absoluteFile = absoluteFile;}
        
}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
