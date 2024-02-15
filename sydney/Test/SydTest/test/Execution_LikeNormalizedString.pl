#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
while (<>) {

    exit 1 if ( /10000件挿入/ || /Execution_LikeNormalizedString/ ) ;

    $value = 0, $valuei = 0 if ( /drop table/ && $value != 2 ) ;

    $value = 1 if ( /create table/ ) ;
    $valuei = 1 if ( /create fulltext index/ ) ;
    $value = 2 if ( /insert into/  && $value == 1 && $valuei != 1 ) ;
    print "Execution_Like", $value = 0 if ( /select/ && $value == 2) ;
    print "Execution_Like" if ( /の異/ && /記/ ) ;  #異表記が使えない

}

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
