rem SQLParserL.cpp と SQLParserL.h はチェックアウトしておくこと

lemon SQLParserL.lemon
del SQLParserL.cpp
perl fixlemon.pl SQLParserL.c SQLParserL.cpp
copy /Y SQLParserL.cpp ..
copy /Y SQLParserL.h ..\Statement\
