#include <iostream>
#include "scriptreader.hpp"
#include "sourcecollector.hpp"
#include "filewriter.hpp"

using namespace std;

Outputs& select(Selection& selection) {
    std::cout << "Folgende Konfigurationen stehen zur Auswahl:" << std::endl;
    for (auto& option: selection)
        std::cout << "  * " << option.name << std::endl;
    std::cout << ">";
    std::string choice;
    while (true) {
        std::cin >> choice;
        for (auto& option: selection)
            if (option.name==choice)
                return option;
        std::cout << " Existiert nicht" << std::endl << ">";
    }
}

int main() {
    try {
        Selection selection;
        getInstructions(selection);
        Structure project;
        project.fill();
        
        Outputs& out = select(selection);
        writeMakeFile(project,out);

        std::cout << "Makefile erstellt mit:" << std::endl;
        for (auto& exe: out.exes)
            std::cout << "  " << exe.output << std::endl;
        for (auto& lib: out.libs)
            std::cout << "  " << lib.output << std::endl;
    } catch (std::string& msg) {
        cout << msg << std::endl;
    }
    return 0;
}