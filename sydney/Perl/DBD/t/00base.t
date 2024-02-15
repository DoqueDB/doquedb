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
require DBI;
print "require DBI ....... ok 1 \n";

import DBI;
print "import DBI ....... ok 2\n";

require DBD::TRMeisterPP;
print "require DBD::TRMeisterPP ........ ok 3\n";

require Net::TRMeister;
print "require Net::TRMeister ........ ok 4\n";

require Encode;
print "require Encode ........ ok 5\n";

require Config;
print "require Config........ok 6\n";

require IO::Socket;
print "require IO::Socket ....... ok 7\n";

#require String::Random;
#print "require String::Random ....... ok 8\n";
#
#require Crypt::ECB;
#print "require Crypt::ECB ....... ok 9\n";
#
#require Crypt::OpenSSL::AES;
#print "require Crypt::OpenSSL::AES ....... ok 10\n";
#
#require Crypt::OpenSSL::RSA;
#print "require Crypt::OpenSSL::RSA ....... ok 11\n";

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
