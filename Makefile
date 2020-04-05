tcmake: main.cpp scriptreader.hpp sourcecollector.hpp filewriter.hpp stringplus.o filename.o
	g++ main.cpp -o tcmake -std=c++11 stringplus.o filename.o

stringplus.o:
	g++ -std=c++11 tobilib/stringplus/stringplus.cpp -c -o stringplus.o

filename.o:
	g++ -std=c++11 tobilib/stringplus/filename.cpp -c -o filename.o