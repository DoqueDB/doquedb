TARGET = sydbuild.jar
SRCS = \
	src/jp/co/ricoh/sydney/build/MessageGenerator.java	  \
	src/jp/co/ricoh/sydney/build/MessageGeneratorForJava.java \
	src/jp/co/ricoh/sydney/build/MessageGeneratorForPerl.java

all: $(TARGET)

sydbuild.jar: $(SRCS)
	mkdir -p classes
	javac -sourcepath src -d classes $(SRCS)
	jar cvf sydbuild.jar -C classes .
