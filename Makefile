build/tcmake: main.cpp scriptreader.hpp sourcecollector.hpp filewriter.hpp build/stringplus.o build/filename.o build
	g++ main.cpp -o build/tcmake -std=c++11 build/stringplus.o build/filename.o

build/stringplus.o: build
	g++ -std=c++11 tobilib/stringplus/stringplus.cpp -c -o build/stringplus.o

build/filename.o: build
	g++ -std=c++11 tobilib/stringplus/filename.cpp -c -o build/filename.o

build:
	mkdir build