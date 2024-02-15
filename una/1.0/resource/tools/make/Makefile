#
# Makefile -
# 
# Copyright (c) 2023 Ricoh Company, Ltd.
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

# パスの設定
MAKE_DIR = ../tools/make

# ツールの設定
INSTALL  = cp -rp
MKDIR    = mkdir -p
RMDIR    = rm -fr

# 出力先ディレクトリ
OUT_DIR  = ./unadic

all:
	make -f $(MAKE_DIR)/make-tools all
	make -f $(MAKE_DIR)/make-tools install
	make -f $(MAKE_DIR)/make-stem  all
	make -f $(MAKE_DIR)/make-stem  install
	make -f $(MAKE_DIR)/make-norm  all
	make -f $(MAKE_DIR)/make-norm  install
	make -f $(MAKE_DIR)/make-una   all
	make -f $(MAKE_DIR)/make-una   install

clean:
	make -f $(MAKE_DIR)/make-stem clean
	make -f $(MAKE_DIR)/make-norm clean
	make -f $(MAKE_DIR)/make-una  clean
	$(RMDIR) ./$(OUT_DIR)
