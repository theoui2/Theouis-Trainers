// Copyright (C) 2026 Theoui. SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "catalog.hpp"
#include "menu_names.hpp"
#include <cerrno>

namespace install {
struct Paths {
    std::string root,overlays,folder,config,oldPackage;
    explicit Paths(const std::string& sd="sdmc:"):root(sd),overlays(sd+"/switch/.overlays"),
        folder(sd+"/switch/TheouisTrainers/trainers"),config(sd+"/config/ultrahand"),oldPackage(sd+"/switch/.packages/Trainers/package.ini"){}
};
struct Result {bool ready=false,names=true,migration=true;unsigned moved=0;};
inline bool exists(const std::string& p){struct stat s{};return !lstat(p.c_str(),&s);}
inline bool directory(const std::string& path){
    struct stat s{};if(!stat(path.c_str(),&s))return S_ISDIR(s.st_mode);
    return errno==ENOENT&&!mkdir(path.c_str(),0777);
}
inline Result prepare(const Paths& p){
    Result out;
    if(!directory(p.root+"/switch/TheouisTrainers")||!directory(p.folder))return out;
    out.ready=true;
    const bool config=directory(p.root+"/config")&&directory(p.config);
    out.names=config&&menu_names::apply(p.config+"/overlays.ini","Trainers.ovl",
        {{"custom_name",menu_names::TITLE},{"custom_version",menu_names::VERSION},{"hide","false"}});
    for(const char* name:{"SimpleTrainerSwitch.ovl","SimpleTrainerTOTK.ovl","TOTKTrainerLab.ovl"}){
        const auto old=catalog::join(p.overlays,name),next=catalog::join(p.folder,name);
        if(!exists(old))continue;
        catalog::Entry entry;
        if(!exists(next)&&catalog::read(old,entry)){
            if(!std::rename(old.c_str(),next.c_str())){
                ++out.moved;
                const auto from=old.substr(0,old.size()-4)+".ini",to=next.substr(0,next.size()-4)+".ini";
                if(exists(from)&&!exists(to)&&std::rename(from.c_str(),to.c_str()))out.migration=false;
            }else out.migration=false;
        }
        // A newly installed trainer always wins. Retain an older root copy
        // as a hidden backup rather than replacing the update with it.
        if(exists(old)&&catalog::read(next,entry)){
            if(!config||!menu_names::apply(p.config+"/overlays.ini",name,{{"hide","true"}}))out.names=false;
        }else if(exists(old))out.migration=false;
    }
    if(exists(p.oldPackage)&&(!config||!menu_names::apply(p.config+"/packages.ini","Trainers",{{"hide","true"}})))out.names=false;
    return out;
}
}
