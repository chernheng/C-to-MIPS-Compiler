  
CPPFLAGS += -std=c++17 -W -Wall -g -Wno-unused-parameter
CPPFLAGS += -I inc

src/compiler_parser.tab.cpp src/compiler_parser.tab.hpp : src/compiler_parser.y
	bison -v -d src/compiler_parser.y -o src/compiler_parser.tab.cpp

src/compiler_lexer.yy.cpp : src/compiler_lexer.flex src/compiler_parser.tab.hpp
	flex -o src/compiler_lexer.yy.cpp  src/compiler_lexer.flex

bin/print_canonical : src/print_canonical.o src/compiler_parser.tab.o src/compiler_lexer.yy.o
	mkdir -p bin
	g++ $(CPPFLAGS) -o bin/print_canonical $^
	
clean :
	rm -f src/*.o
	rm src/*.tab.cpp
	rm src/*.yy.cpp