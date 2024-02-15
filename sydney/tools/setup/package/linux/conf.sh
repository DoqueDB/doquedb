#
#         TRMeister configuration file
#
# ==================================================================
#   NOTE:
#    When more than one TRMeisters are installed in one server,
#    following variables have to have different values each other.
#       * installpath
#       * databasepath
#       * portnumber
# ==================================================================
#
# -----------------------------
# Install directry path
# -----------------------------
installpath=/var/lib/DoqueDB

# -----------------------------
# Database directory path
# -----------------------------
databasepath=/var/lib/DoqueDB/db

# -----------------------------
# The port number used by SydServer
# -----------------------------
portnumber=54321

# -----------------------------
# The name of the user to run SydServer as
# -----------------------------
user=root

# -----------------------------
# Buffer maximum size
#   -- TRMeister module will consume process size almost same as this size + 10-40 MB.
#   -- (see user's manual ``Parameter'')
#   -- A character denoting unit (ex. M or G) can be attached.
#   -- The default value is '20M'
# -----------------------------
# ** 20MB(default) **
# buffersize=20M
# ** 128MB **
# buffersize=128M
# ** 256MB **
# buffersize=256M
# ** 512MB **
# buffersize=512M
# ** 1GB **
# buffersize=1G

# -----------------------------
# Batch insert buffer maximum size
#   -- Set large value to this size in case of creating fulltext indexes for large size tables.
#   -- Also set large value to 'buffersize'. (see user's manual ``Parameter'')
#   -- The default value is 60M
# -----------------------------
# ** 60MB(default) **
# batchsize=62914560
# ** 128MB **
# batchsize=134217728
# ** 256MB **
# batchsize=268435456

# -----------------------------
# Checkpoint period (interval between checkpoints in milliseconds)
#   -- Short period makes performance slower but also makes recovery time shorter and file size smaller.
#   -- Long period makes performance faster but also makes recovery time longer and fise size larger.
#   -- The default value is 1800000 (= 30min.)
# -----------------------------
# ** 30min.(default) **
# checkpointperiod=1800000
# ** 10min. **
# checkpointperiod=600000
# ** 60min. **
# checkpointperiod=3600000
# ** no checkpoint (!!use only for read-only case!!) **
# checkpointperiod=2147483647

# -----------------------------
# Set normalize to 1 if you want the 'like' predicate not to distinguish
# capital/small letters, half/full width letters and hiragana/katakana.
# If a character string column has an index, some special hint should be specified.
# Call us (Ricoh 'TRMeister' team) for detail.
# -----------------------------
normalize=0

# -----------------------------
# Set remaindata to 1 if you want the uninstaller to remain the database files.
# In such a case, next installation will be fail unless the remained files would be moved
# or the value of databasepath (and databasepath_e) would be changed.
# -----------------------------
remaindata=0

# -----------------------------
# Set remaindll to a value except for 1 if you want the uninstaller to remove
# all the files under the installed directory.
# -----------------------------
remaindll=0

