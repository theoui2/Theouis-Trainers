#include "../source/menu_names.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
int main(){using namespace menu_names;unsigned checks=0;
    const std::string sample="; user settings\r\n[Trainers.ovl]\r\ncustom_name=Trainers\r\ncustom_version=0.1\r\nstar=true\r\npriority=1\r\nhide=false\r\nlaunch_args=--foo bar\r\n\r\n[FPSLocker.ovl]\r\ncustom_name=FPSLocker\r\ncustom_version=3.3.2\r\nstar=false\r\n";
    const auto next=updated(sample,"Trainers.ovl");
    assert(next.find("custom_name=Theoui's Trainers\r\ncustom_version=1.0\r\nstar=true\r\npriority=1\r\nhide=false\r\nlaunch_args=--foo bar")!=std::string::npos);
    assert(next.substr(next.find("[FPSLocker.ovl]"))==sample.substr(sample.find("[FPSLocker.ovl]")));assert(updated(next,"Trainers.ovl")==next);++checks;
    const auto empty=updated("","Trainers");assert(empty=="[Trainers]\ncustom_name=Theoui's Trainers\ncustom_version=1.0\n");++checks;
    const auto only=updated("[Trainers]\nstar=true\n\n[Alchemist]\nhide=true\n","Trainers");
    assert(only.find("star=true")!=std::string::npos&&only.find("custom_version=1.0\n[Alchemist]\nhide=true")!=std::string::npos);++checks;
    assert(updated("[Trainers]\ncustom_name=Trainers\ncustom_version=0.1","Trainers").find("custom_version=1.0\n")!=std::string::npos);++checks;
    const auto dup=updated("[Trainers]\ncustom_name=Old\n[Other]\nstar=true\n[Trainers]\ncustom_version=Old\n","Trainers");
    assert(dup.find("custom_name=Old")==std::string::npos&&dup.find("custom_version=Old")==std::string::npos&&dup.find("[Other]\nstar=true\n")!=std::string::npos);++checks;
    char temp[]="/tmp/trainers-names-XXXXXX";assert(mkdtemp(temp));auto path=std::filesystem::path(temp)/"overlays.ini";
    {std::ofstream f(path,std::ios::binary);f<<sample;}
    assert(menu_names::apply(path.string(),"Trainers.ovl"));std::string loaded,backup;
    assert(read(path.string(),loaded)==Read::Ok&&loaded==next);assert(read(path.string()+".theoui.bak",backup)==Read::Ok&&backup==sample);++checks;
    assert(menu_names::apply(path.string(),"Trainers.ovl")&&!std::filesystem::exists(path.string()+".theoui.bak.1"));++checks;
    auto missing=std::filesystem::path(temp)/"packages.ini";assert(menu_names::apply(missing.string(),"Trainers"));assert(read(missing.string(),loaded)==Read::Ok&&loaded==empty);++checks;
    auto bad=std::filesystem::path(temp)/"missing"/"overlays.ini";assert(!menu_names::apply(bad.string(),"Trainers.ovl")&&!std::filesystem::exists(bad));++checks;
    std::filesystem::remove_all(temp);std::cout<<checks<<" menu-name update and settings-preservation scenarios passed.\n";
}
