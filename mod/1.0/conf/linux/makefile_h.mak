#
# conf/linux/makefile_h.mak ---
# 
# Copyright (c) 2003, 2022, 2023 Ricoh Company, Ltd.
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

INCLCOMM = $(TOOLSDIR)/include
MV = mv
INCLFLAG = \
	-m \
	-I../include \
	-I../../include \
	-I../../../include \
	$(LOCALINCL)

Makefile.h: $(SOURCES)
	$(INCLCOMM) $(INCLFLAG) $(SOURCES) > Makefile.h.tmp
	$(MV) Makefile.h.tmp Makefile.h

Makefile:
Makefile.c:
