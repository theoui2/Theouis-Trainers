// Copyright (C) 2026 Theoui. SPDX-License-Identifier: GPL-2.0-or-later
#include "../source/shortcut_settings.hpp"
#include "../source/catalog.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>

namespace fs=std::filesystem;
static unsigned count=0;
static void write(const fs::path& p,const std::string& s){fs::create_directories(p.parent_path());assert(shortcuts::writeChecked(p.string(),s));}
static std::string contents(const fs::path& p){std::string out;assert(shortcuts::read(p.string(),out));return out;}
struct Fixture {
    fs::path root;shortcuts::Paths paths;
    explicit Fixture(const fs::path& base):root(base/std::to_string(count++)),paths(root.string()){fs::create_directories(root);}
};
static std::string nro(){std::string data(0x80,'\0');data.replace(0x10,4,"NRO0");data[0x18]=char(0x80);return data;}

int main(){
    using namespace shortcuts;
    char tmp[]="/tmp/theoui-shortcuts-XXXXXX";assert(mkdtemp(tmp));const fs::path base=tmp;
    {for(unsigned i=1;i<PRESETS.size();i++){
        assert(combo(PRESETS[i].combo)==PRESETS[i].keys);Gesture g;const auto keys=PRESETS[i].keys;
        assert(!g.poll(keys,keys,keys));assert(!g.poll(keys,keys,0));assert(!g.poll(keys,0,0));
        assert(g.poll(keys,keys,keys));assert(!g.poll(keys,keys,keys));assert(!g.poll(keys,keys,0));
        assert(!g.poll(keys,0,0));assert(g.poll(keys,keys,keys));++count;
    }}
    {Gesture g;assert(!g.poll(LS|RS,0,0));assert(!g.poll(LS|RS,LS,LS));assert(g.poll(LS|RS,LS|RS,RS));assert(!g.poll(LS|RS,LS,0));assert(!g.poll(LS|RS,LS|RS,RS));assert(!g.poll(LS|RS,0,0));assert(g.poll(LS|RS,LS|RS,LS|RS));++count;}
    {Gesture g;assert(!g.poll(LS|RS,0,0));assert(!g.poll(LS|RS,LS|RS|1,LS|RS|1));assert(!g.poll(LS|RS,LS|RS,0));assert(!g.poll(LS|RS,0,0));assert(g.poll(LS|RS,LS|RS|(1ULL<<17),LS|RS));++count;}
    {Gesture g;assert(!g.poll(LS|RS,0,0));assert(g.poll(LS|RS,LS|RS,LS|RS));assert(!g.poll(L|R|LS,L|R|LS,L|R|LS));assert(!g.poll(L|R|LS,0,0));assert(g.poll(L|R|LS,L|R|LS,L|R|LS));assert(!g.poll(0,0,0));assert(!g.poll(0,LS|RS,LS|RS));++count;}
    {assert(preset("[shortcuts]\r\npreset=6\r\n")==6);for(const auto& s:{"-1","7","999999","x","1x",""})assert(preset(std::string("[shortcuts]\npreset=")+s)==0);assert(preset("[other]\npreset=1\n")==0);++count;}
    {std::uint64_t id=0;assert(titleId("010096F00FF22000",id)&&id==BL2);assert(titleId("\"0100f2c0115b6000\"",id)&&id==TOTK);for(const auto& s:{"0000000000000000","010096F00FF2200","010096F00FF220000","010096F00FF2200Z"})assert(!titleId(s,id)&&!id);++count;}
    {std::vector<std::string> values;assert(list("('a,b', --other)",values)&&values.size()==2);assert(unquote(values[0])=="a,b");assert(!list("(unterminated,'quote)",values));assert(combo("L+not_a_button")==0);++count;}
    {Fixture f(base);auto s=initialize(f.paths);assert(s.applied&&s.selected==1&&loadPreset(f.paths.config.c_str())==1);const auto registryText=contents(f.paths.overlays);assert(value(registryText,"Trainers.ovl","mode_args")=="(--theoui-auto)");assert(value(registryText,"Trainers.ovl","mode_combos")=="(LS+RS)");auto again=initialize(f.paths);assert(again.applied&&contents(f.paths.overlays)==registryText);}
    {Fixture f(base);const std::string before="; keep\r\n[Trainers.ovl]\r\nstar=true\r\npriority=2\r\nkey_combo=ZL+DLEFT\r\nmode_args=(--other)\r\nmode_labels=('Other mode')\r\nmode_combos=(ZR+DLEFT)\r\nlaunch_args=--user-setting\r\n[Other.ovl]\r\nhide=false\r\nstar=true\r\n";write(f.paths.overlays,before);auto s=apply(f.paths,1);assert(s.applied);auto after=contents(f.paths.overlays);assert(after.find("; keep\r\n")!=std::string::npos);assert(after.substr(after.find("[Other.ovl]"))==before.substr(before.find("[Other.ovl]")));assert(value(after,"Trainers.ovl","key_combo")=="ZL+DLEFT");assert(value(after,"Trainers.ovl","launch_args")=="--user-setting");assert(value(after,"Trainers.ovl","mode_args")=="(--other,--theoui-auto)");assert(value(after,"Trainers.ovl","mode_combos")=="(ZR+DLEFT,LS+RS)");s=apply(f.paths,0);assert(s.applied&&s.selected==0);after=contents(f.paths.overlays);assert(value(after,"Trainers.ovl","mode_combos")=="(ZR+DLEFT,)");assert(value(after,"Trainers.ovl","key_combo")=="ZL+DLEFT");}
    {Fixture f(base);assert(initialize(f.paths).applied);for(unsigned i=0;i<160;i++){auto s=apply(f.paths,1+i%6);assert(s.applied&&s.selected==1+i%6);}std::vector<std::string> modes;assert(list(value(contents(f.paths.overlays),"Trainers.ovl","mode_args"),modes)&&modes.size()==1);}
    {Fixture f(base);write(f.paths.overlays,"[Other.ovl]\nkey_combo=RS+LS\nstar=true\n");const auto before=contents(f.paths.overlays);auto s=initialize(f.paths);assert(!s.applied&&s.selected==0&&s.message.find("Other.ovl")!=std::string::npos);assert(contents(f.paths.overlays)==before&&!fs::exists(f.paths.config));}
    {Fixture f(base);write(f.paths.overlays,"[Other.ovl]\nmode_args=(--widget)\nmode_combos=(LS+RS)\n");assert(!initialize(f.paths).applied);}
    {Fixture f(base);write(f.paths.packages,"[My package]\nkey_combo=LS+RS\n");assert(!initialize(f.paths).applied);}
    {Fixture f(base);write(f.paths.ultrahand,"[ultrahand]\nkey_combo=LS\n");assert(!initialize(f.paths).applied);}
    {Fixture f(base);write(f.paths.tesla,"[tesla]\nkey_combo=LS+RS\n");assert(!initialize(f.paths).applied);}
    {Fixture f(base);write(f.paths.overlays,"[Trainers.ovl]\nkey_combo=LS+RS\n");assert(!initialize(f.paths).applied);}
    {Fixture f(base);write(f.paths.ultrahand,"[ultrahand]\nlaunch_combos=false\n");assert(!initialize(f.paths).applied);assert(apply(f.paths,0).applied);}
    {Fixture f(base);write(f.paths.overlays,"[Trainers.ovl]\nmode_args=(--other)\nmode_combos=(LS+RS,ZL)\n");const auto before=contents(f.paths.overlays);assert(!apply(f.paths,1).applied&&contents(f.paths.overlays)==before);}
    {Fixture f(base);assert(initialize(f.paths).applied);const auto before=contents(f.paths.overlays);fs::create_directory(f.paths.config+".theoui-shortcut.tmp");auto s=apply(f.paths,2);assert(!s.applied&&s.selected==1&&loadPreset(f.paths.config.c_str())==1);assert(contents(f.paths.overlays)==before&&s.message.find("previous shortcut kept")!=std::string::npos);}
    {Fixture f(base);assert(initialize(f.paths).applied);const auto before=contents(f.paths.overlays);write(f.paths.overlays+".theoui-shortcut.rollback","retained recovery data");assert(!apply(f.paths,2).applied&&contents(f.paths.overlays)==before);}
    {Fixture f(base);write(f.paths.config,std::string(4097,'x'));assert(!apply(f.paths,1).applied&&loadPreset(f.paths.config.c_str())==0);}
    {Fixture f(base);const auto folder=f.root/"trainers";write(folder/"SimpleTrainerSwitch.ovl",nro());write(folder/"SimpleTrainerTOTK.ovl",nro());write(folder/"TOTKTrainerLab.ovl",nro());auto scan=catalog::scan(folder.string());assert(scan.entries.size()==3);assert(catalog::choose(scan,BL2).path==(folder/"SimpleTrainerSwitch.ovl").string());assert(catalog::choose(scan,TOTK).path==(folder/"SimpleTrainerTOTK.ovl").string());assert(catalog::choose(scan,0).path.empty());assert(catalog::choose(scan,0x0100000000000001ULL).path.empty());}
    {Fixture f(base);const auto folder=f.root/"trainers";write(folder/"NewGame.ovl",nro());write(folder/"NewGame.ini","[trainer]\nname=Future Game\ntitle_id=0100123456789000\n");auto scan=catalog::scan(folder.string());assert(scan.entries[0].name=="Future Game"&&scan.entries[0].titleId==0x0100123456789000ULL);assert(!catalog::choose(scan,0x0100123456789000ULL).path.empty());}
    {Fixture f(base);const auto folder=f.root/"trainers";write(folder/"SimpleTrainerSwitch.ovl",nro());write(folder/"SimpleTrainerSwitch.ini","[trainer]\ntitle_id=invalid\n");auto scan=catalog::scan(folder.string());assert(scan.entries[0].titleId==0&&catalog::choose(scan,BL2).path.empty());}
    {Fixture f(base);const auto folder=f.root/"trainers";write(folder/"SimpleTrainerSwitch.ovl",nro());write(folder/"Second.ovl",nro());write(folder/"Second.ini","[trainer]\ntitle_id=010096F00FF22000\n");auto scan=catalog::scan(folder.string());auto choice=catalog::choose(scan,BL2);assert(choice.path.empty()&&choice.reason.find("Multiple")!=std::string::npos);scan.limited=true;assert(catalog::choose(scan,BL2).path.empty());}
    fs::remove_all(base);std::cout<<count<<" shortcut gesture / settings / collision / game-routing scenarios passed.\n";
}
