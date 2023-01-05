#ifndef SCRIPTREADER_HPP
#define SCRIPTREADER_HPP

#include "tobilib/stringplus/filename.h"
#include <string>
#include <vector>
#include <fstream>

struct Outputs {
    std::string name;

    struct Lib {
        tobilib::FileName output;
        std::vector<tobilib::FileName> paths;
        
    };
    struct Exe {
        tobilib::FileName output;
        tobilib::FileName source;
        std::vector<std::string> links;
    };

    std::vector<Lib> libs;
    std::vector<Exe> exes;
    std::vector<std::string> extern_include_paths;
    std::vector<std::string> extern_lib_paths;
};

typedef std::vector<Outputs> Selection;

bool getInstructions(Selection& selection) {
    std::fstream file ("tcmakefile",std::fstream::in);
    std::skipws(file);
    if (!file.good())
        throw std::string("es ist kein tcmakefile vorhanden");
    while (file.good())
    {
        std::string instruction;
        file >> instruction;
        if (instruction.empty())
            continue;
        if (instruction=="selection")
        {
            selection.emplace_back();
            file >> selection.back().name;
        }
        else if (selection.empty()) {
            throw std::string("Eine selektion wurde nicht benannt");
        }
        else if (instruction=="lib")
        {
            Outputs::Lib lib;
            file >> lib.output;
            selection.back().libs.push_back(lib);
        }
        else if (instruction=="add") {
            if (selection.back().libs.empty())
                throw std::string("Keine Bibliothek definiert. (add)");
            tobilib::FileName path;
            file >> path;
            selection.back().libs.back().paths.push_back(path);
        }
        else if (instruction=="exe")
        {
            Outputs::Exe exe;
            file >> exe.output;
            file >> exe.source;
            selection.back().exes.push_back(exe);
        }
        else if (instruction=="link")
        {
            std::string link;
            file >> link;
            if (selection.back().exes.empty())
                throw std::string("Ein link konnte nicht zugeordnet werden. (exe fehlt)");
            selection.back().exes.back().links.push_back(link);
        }
        else if (instruction=="include")
        {
            std::string path;
            file >> path;
            if (!selection.back().exes.empty() || !selection.back().libs.empty())
                throw std::string("die 'include' Anweisung gehoert zur selection und muss vor 'exe' und 'lib' stehen.");
            selection.back().extern_include_paths.push_back(path);
        }
        else if (instruction=="libpath")
        {
            std::string path;
            file >> path;
            if (!selection.back().exes.empty() || !selection.back().libs.empty())
                throw std::string("die 'libpath' Anweisung gehoert zur selection und muss vor 'exe' und 'lib' stehen.");
            selection.back().extern_lib_paths.push_back(path);
        }
        else
        {
            throw std::string("unbekannte Anweisung: ") + instruction;
        }
    }
    file.close();
    return true;
}

#endif