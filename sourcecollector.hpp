#ifndef SOURCECOLLECTOR_HPP
#define SOURCECOLLECTOR_HPP

#include <dirent.h>
#include "tobilib/stringplus/filename.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include <exception>

// Component definiert eine Programmkomponente bestehend aus
// Source-File, Header-File und (zu generieren) Object-File
// Dateipfad ist angegeben relativ zu tcmakefile
struct Component {
    bool has_header = false;
    bool has_code = false;
    bool has_lib = false; // has_header && has_code

    // source_x relativ zum Projektordner
    tobilib::FileName source_cpp;
    tobilib::FileName source_h;

    // out_x: relativ zum erstellten Makefile
    tobilib::FileName out_h;
    tobilib::FileName out_o;

    std::set<Component*> depend_public; // Im h-file: relevant für andere .o komponenten
    std::set<Component*> depend_private; // Im cpp-file: irrelelvant für andere .o komponenten

    // rekursive abhängigkeit
    void get_compile_dependencies(std::set<Component*>& output);
    void get_linkage_dependencies(std::set<Component*>& output);

private:
    void recursive_gather(std::set<Component*>& output, bool include_private, bool is_first);
};

class Structure {
public:
    // Ich gehe davon aus, dass Pointer auf Elemente gültig bleiben
    // Component::depend verwendet Pointer
    std::map<std::string, Component> components;

    // keyof: eindeutiger String für Komponente
    static tobilib::StringPlus keyof(const tobilib::FileName&);

    // Liste von Ordnern mit Headerfile
    std::vector<tobilib::FileName> directories;

    // Sucht Komponenten im Dateisystem
    void fill();

private:
    // add_directory: durchsucht ordner rekursiv
    void add_directory(tobilib::StringPlus,bool* out_hasheader=nullptr);
    // get dependencies: Sucht die Abhängigkeiten im File-Code
    void get_dependencies(Component&);
    void get_dependencies(Component&, std::set<Component*>& ,tobilib::FileName);
};

void Component::get_compile_dependencies(std::set<Component*>& output)
{
    recursive_gather(output,false,true);
}

void Component::get_linkage_dependencies(std::set<Component*>& output)
{
    recursive_gather(output,true,true);
}

void Component::recursive_gather(std::set<Component*>& output, bool include_private, bool is_first) {
    for(auto& dep: depend_public) {
        if (output.count(dep)!=0)
            continue;
        output.insert(dep);
        dep->recursive_gather(output,include_private,false);
    }

    if (!is_first && !include_private)
        return;

    for (auto& dep: depend_private) {
        if (output.count(dep)!=0)
            continue;
        output.insert(dep);
        dep->recursive_gather(output,include_private,false);
    }
}

void Structure::fill() {
    add_directory("./");
    for (auto& item: components)
        get_dependencies(item.second);
}

tobilib::StringPlus Structure::keyof(const tobilib::FileName& file) {
    return file.directory()+file.name;
}

// @param out_hasheader: wird auf true gesetzt falls headerfile enthalten. sonst unverändert.
void Structure::add_directory(tobilib::StringPlus path, bool* out_hasheader) {
    // ignore build path
    if (tobilib::FileName(path)=="build/")
        return;

    // Open directory and iterate through elements
    DIR* dir = opendir(path.toString().c_str());
    bool hasheader = false;
    if (dir==NULL)
        return;
    for (dirent* entry=readdir(dir);entry!=NULL;entry=readdir(dir)) {
        tobilib::StringPlus fstring = entry->d_name;
        if (fstring!=".." && fstring!=".")
            add_directory(path+fstring+"/",&hasheader);
        tobilib::FileName fname(path+fstring);

        if (fname.extension=="hpp" || fname.extension=="h"){
            Component& comp = components[keyof(fname)];
            comp.source_h = fname;
            comp.has_header = true;
            comp.has_lib = comp.has_code;
            comp.out_h = fname;
            comp.out_h.path.push_front("include");
            hasheader=true;
        }
        if (fname.extension=="cpp" || fname.extension=="c") {
            Component& comp = components[keyof(fname)];
            comp.source_cpp = fname;
            comp.has_code = true;
            comp.has_lib = comp.has_header;
            comp.out_o.path.push_back("bin");
            comp.out_o.name = fname.directory().replace_all("/","_")+fname.name;
            comp.out_o.extension = "o";
        }
    }
    if (hasheader) {
        directories.insert(directories.begin(),path);
        if (out_hasheader)
            *out_hasheader = true;
    }
    closedir(dir);
}

void Structure::get_dependencies(Component& comp) {
    if (comp.has_header)
        get_dependencies(comp,comp.depend_public,comp.source_h);
    if (comp.has_code)
        get_dependencies(comp,comp.depend_private,comp.source_cpp);
}

void Structure::get_dependencies(Component& self, std::set<Component*>& list, tobilib::FileName file) {
    tobilib::StringPlus filecontent;
    try {
        filecontent = tobilib::StringPlus::fromFile(file);
    } catch (std::exception e) {
        std::cout << "WARNUNG: (" << file.fullName() << ") " << e.what();
    }
    std::vector<int> includes = filecontent.find_all("#include");
    for (auto& include: includes) {
        int border1 = filecontent.find("\"",include);
        int border2 = filecontent.find("\"",border1+1);
        int endl = filecontent.find('\n',include);
        if (endl==tobilib::StringPlus::npos)
            endl = filecontent.size();
        if (border1==tobilib::StringPlus::npos || border1>endl)
            continue;
        if (border2==tobilib::StringPlus::npos || border2>endl) {
            std::cout << "WARNUNG: (" << file.fullName() << ") Include nicht abgeschlossen" << std::endl;
            continue;
        }
        tobilib::FileName depfile = file+filecontent.substr(border1+1,border2-border1-1);
        if (components.count(keyof(depfile)) == 0)
            continue;
        Component& dep = components.at(keyof(depfile));
        if (&self!=&dep)
            list.insert(&dep);
    }
}

#endif