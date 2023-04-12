#include <iostream>
#include "scriptreader.hpp"
#include "sourcecollector.hpp"
#include "filewriter.hpp"

using namespace std;

void printinfo(Selection& selection) {
    std::cout << "Folgende Konfigurationen stehen zur Auswahl:" << std::endl;
    if (selection.empty())
        std::cout << "Es steht keine Auswahl zur verfuegung." << std::endl;
    for (auto& option: selection)
        std::cout << "  * " << option.name << std::endl;
}

MakeOptions configureOutput(int argc, const char** args, Selection& selection) {
    if (selection.empty())
        throw std::string("Das tcmakefile definiert keine selection");
    MakeOptions out;
    for (auto& option: selection)
        if (option.name == args[1])
            out.outputs = &option;
    if (out.outputs == nullptr)
        throw std::string("Die Auswahl wurde nicht gefunden.");
    for (int i=2;i<argc;i++)
    {
        std::string arg = args[i];
        if (arg == "debug")
            out.debug = true;
        else
            throw std::string("Unbekannte option: ")+args[i];
    }
    return out;
}

void generateOutput(int argc, const char** args, Selection& selection)
{
	MakeOptions options = configureOutput(argc,args,selection);
    Structure project;
    
    project.fill(options.outputs);
    writeMakeFile(project,options);

    std::cout << "Makefile erstellt mit:" << std::endl;
    for (auto& exe: options.outputs->exes)
        std::cout << "  " << exe.output << std::endl;
    for (auto& lib: options.outputs->libs)
        std::cout << "  " << lib.output << std::endl;
}

int main(int argc, const char** args) {
    try {
        Selection selection;
        getInstructions(selection);
        
        if (argc < 2)
            printinfo(selection);
        else
            generateOutput(argc,args,selection);
        
    } catch (std::string& msg) {
        cout << msg << std::endl;
    }
    return 0;
}
