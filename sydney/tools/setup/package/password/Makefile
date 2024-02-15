TARGET = \
	user.pswd

all: $(TARGET)

$(TARGET): src/$*.txt
	perl makemd5.pl $? > $@
