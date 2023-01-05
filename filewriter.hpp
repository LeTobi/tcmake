#ifndef FILEWRITER_HPP
#define FILEWRITER_HPP

#include "scriptreader.hpp"
#include "sourcecollector.hpp"
#include <fstream>
#include <iostream>

struct MakeOptions
{
    Outputs* outputs = nullptr;
    bool debug = false;
    std::string cpp_std = "c++17";

    std::string gcc() {
        if (debug)
            return "g++ -g";
        else
            return "g++";
    }

    tobilib::FileName objname(tobilib::FileName objfile)
    {
        if (!debug)
            return objfile;
        objfile.name = tobilib::StringPlus("_dbg_")+objfile.name;
        return objfile;
    }
};

std::string fromSource(tobilib::FileName fname) {
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

void writeMakeFile(Structure& structure, MakeOptions options) {
    Outputs& outputs = *options.outputs;
    std::set<tobilib::StringPlus> directories;

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
        fs << comp.second.out_h << ": " << fromSource(comp.second.source_h);
        tobilib::FileName mkpath;
        for (auto& dir: comp.second.out_h.path) {
            mkpath.path.push_back(dir);
            fs << " " << mkpath;
            directories.insert(mkpath.directory());
        }
        fs << std::endl;
        fs << "\tcp " << fromSource(comp.second.source_h) << " " << comp.second.out_h << std::endl;
    }

    // Objectdateien
    for (auto& comp: structure.components) {
        if (!comp.second.has_code)
            continue;
        fs << options.objname(comp.second.out_o) << ": ";
        if (comp.second.has_header)
            fs << fromSource(comp.second.source_h) << " ";
        fs << fromSource(comp.second.source_cpp) << " ";

        std::set<Component*> dependencies;
        comp.second.get_compile_dependencies(dependencies);
        for (auto& dep: dependencies) {
            fs << fromSource(dep->source_h) << " ";
        }
        fs << " bin/" << std::endl;
        directories.insert(tobilib::StringPlus("bin/"));
        fs << "\t" << options.gcc() << " -std=" << options.cpp_std << " -c ";
        for (auto& ipath: outputs.extern_include_paths)
            fs << "-I" << ipath << " ";
        fs << fromSource(comp.second.source_cpp)
            << " -o " << options.objname(comp.second.out_o)
            << std::endl;
    }

    // Bibliotheken
    for (auto& lib: outputs.libs) {
        fs << lib.output << ": ";
        for (auto& comp: structure.components)
            if (lib_contains_comp(lib,comp.first) && comp.second.has_header) {
                if (comp.second.has_code)
                    fs << options.objname(comp.second.out_o) << " ";
                if (comp.second.has_header)
                    fs << comp.second.out_h << " ";
            }
        fs << std::endl;
        for (auto& comp: structure.components)
            if (lib_contains_comp(lib,comp.first) && comp.second.has_lib)
                fs << "\tar r " << lib.output << " " << options.objname(comp.second.out_o) << std::endl;
        fs << "\tar s " << lib.output << std::endl;
    }

    // Anwendungen
    for (auto& exe: outputs.exes) {
        if (structure.components.count(structure.keyof(exe.source))==0)
            throw exe.source.fullName().toString() + " wurde nicht gefunden.";
        Component& comp = structure.components.at(structure.keyof(exe.source));
        fs << exe.output << ": " << options.objname(comp.out_o);

        std::set<Component*> dependencies;
        comp.get_linkage_dependencies(dependencies);
        for (auto& dep: dependencies)
            if (dep->has_lib)
                fs << " " << options.objname(dep->out_o);
        fs << std::endl;
        fs << "\t" << options.gcc() << " -std=" << options.cpp_std << " -o " << exe.output << " " << options.objname(comp.out_o);
        for (auto& path: outputs.extern_lib_paths)
            fs << " -L " << path;
        for (auto& dep: dependencies)
            if (dep->has_lib)
                fs << " " << options.objname(dep->out_o);
        for (auto& link: exe.links)
            fs << " -l" << link;
        fs << std::endl;
    }

    // dateisystem
    for (auto& dir: directories)
    {
        fs << std::endl;
        fs << dir << ":" << std::endl;
        fs << "\tmkdir " << dir;
    }

    fs.close();
}

#endif