tcmake: main.cpp scriptreader.hpp sourcecollector.hpp filewriter.hpp
	g++ main.cpp -o tcmake -std=c++11 -ltc