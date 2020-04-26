#ifndef FILEWRITER_HPP
#define FILEWRITER_HPP

#include "scriptreader.hpp"
#include "sourcecollector.hpp"
#include <fstream>
#include <iostream>

std::string rootPath(tobilib::FileName fname) {
    fname.path.push_front("..");
    return fname.fullName();
}

bool lib_contains_comp(const Outputs::Lib& lib, std::string key) {
    for (auto& path: lib.paths) {
        if (path.contains(key))
            return true;
    }
    return false;
}

void writeMakeFile(Structure& structure, Outputs& outputs) {
    // Öffnen
    std::fstream fs ("build/makefile",std::fstream::out);
    if (!fs.good())
        throw std::string("Makefile konnte nicht erstellt werden. (Fehlt build path?)");

    // Outputs
    fs << "output: ";
    for (auto& lib: outputs.libs)
        fs << lib.output << " ";
    for (auto& exe: outputs.exes)
        fs << exe.output << " ";
    fs << std::endl;

    // Headerfiles
    for (auto& comp: structure.components) {
        if (!comp.second.has_header)
            continue;
        fs << comp.second.out_h << ": " << rootPath(comp.second.source_h) << std::endl;
        tobilib::FileName mkpath;
        for (auto& dir: comp.second.out_h.path) {
            mkpath.path.push_back(dir);
            fs << "\tmkdir -p " << mkpath << std::endl;
        }
        fs << "\tcp " << rootPath(comp.second.source_h) << " " << comp.second.out_h << std::endl;
    }

    // Objectdateien
    for (auto& comp: structure.components) {
        if (!comp.second.has_lib)
            continue;
        fs << comp.second.out_o << ": ";
        fs << rootPath(comp.second.source_h) << " ";
        fs << rootPath(comp.second.source_cpp) << " ";

        std::set<Component*> dependencies;
        comp.second.r_depend(dependencies);
        for (auto& dep: dependencies) {
            fs << rootPath(dep->source_h) << " ";
        }
        fs << std::endl;
        fs << "\tmkdir -p bin" << std::endl;
        fs << "\tg++ -std=c++11 -c "
            << rootPath(comp.second.source_cpp)
            << " -o " << comp.second.out_o
            << std::endl;
    }

    // Bibliotheken
    for (auto& lib: outputs.libs) {
        fs << lib.output << ": ";
        for (auto& comp: structure.components)
            if (lib_contains_comp(lib,comp.first)) {
                if (comp.second.has_code)
                    fs << comp.second.out_o << " ";
                if (comp.second.has_header)
                    fs << comp.second.out_h << " ";
            }
        fs << std::endl;
        for (auto& comp: structure.components)
            if (lib_contains_comp(lib,comp.first) && comp.second.has_lib)
                fs << "\tar r " << lib.output << " " << comp.second.out_o << std::endl;
        fs << "\tar s " << lib.output << std::endl;
    }

    // Anwendungen
    for (auto& exe: outputs.exes) {
        if (structure.components.count(structure.keyof(exe.source))==0)
            throw exe.source.fullName().toString() + " wurde nicht gefunden.";
        Component& comp = structure.components.at(structure.keyof(exe.source));
        fs << exe.output << ": " << rootPath(exe.source) << " ";

        std::set<Component*> dependencies;
        comp.r_depend(dependencies);
        for (auto& dep: dependencies)
            if (dep->has_lib)
                fs << dep->out_o << " ";
            else
                fs << rootPath(dep->source_h) << " ";
        fs << std::endl;
        fs << "\tg++ -std=c++11 " << rootPath(exe.source) << " -o " << exe.output << " ";
        for (auto& link: exe.links)
            fs << "-l" << link << " ";
        for (auto& dep: dependencies)
            if (dep->has_lib)
                fs << dep->out_o << " ";
        fs << std::endl;
    }

    fs.close();
}

#endif