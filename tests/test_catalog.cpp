#include "../source/legacy_install.hpp"
#include <cassert>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <unistd.h>

namespace fs=std::filesystem;
static void put(std::vector<unsigned char>& b,std::size_t at,std::uint64_t value,unsigned size){for(unsigned i=0;i<size;i++)b[at+i]=value>>(8*i);}
static void write(const fs::path& path,const std::string& data){fs::create_directories(path.parent_path());FILE* f=std::fopen(path.c_str(),"wb");assert(f);assert(std::fwrite(data.data(),1,data.size(),f)==data.size());assert(!std::fclose(f));}
static std::string contents(const fs::path& path){std::string s;assert(menu_names::read(path.string(),s)==menu_names::Read::Ok);return s;}
static std::string nro(const std::string& name="Game trainer",const std::string& version="0.1",bool assets=true){
    std::vector<unsigned char> data(assets?0x80+0x38+0x4000:0x80);put(data,0x10,0x304F524E,4);put(data,0x18,0x80,4);
    if(assets){put(data,0x80,0x54455341,4);put(data,0x80+24,0x38,8);put(data,0x80+32,0x4000,8);
        std::memcpy(data.data()+0x80+0x38,name.data(),std::min(name.size(),std::size_t(511)));
        std::memcpy(data.data()+0x80+0x38+0x3060,version.data(),std::min(version.size(),std::size_t(15)));}
    return std::string(reinterpret_cast<const char*>(data.data()),data.size());
}
struct Fixture {
    fs::path root;install::Paths paths;
    Fixture(const fs::path& base,unsigned n):root(base/std::to_string(n)),paths(root.string()){
        fs::create_directories(paths.overlays);fs::create_directories(paths.folder);
    }
    void plugin(const std::string& name,const std::string& label="Game",const std::string& version="0.1"){write(fs::path(paths.folder)/name,nro(label,version));}
};
int main(int argc,char** argv){
    char tmp[]="/tmp/theoui-hub-XXXXXX";assert(mkdtemp(tmp));const fs::path base=tmp;unsigned count=0;catalog::Entry entry;
    {Fixture f(base,count);f.plugin("NewGame.ovl","Future Game","2.3");auto s=catalog::scan(f.paths.folder);assert(s.ready&&s.entries.size()==1&&s.entries[0].name=="Future Game"&&s.entries[0].version=="2.3"&&s.entries[0].path==f.paths.folder+"/NewGame.ovl");++count;}
    {Fixture f(base,count);f.plugin("SimpleTrainerSwitch.ovl","Generic trainer");assert(catalog::read(f.paths.folder+"/SimpleTrainerSwitch.ovl",entry)&&entry.name=="Borderlands 2");++count;}
    {Fixture f(base,count);f.plugin("SimpleTrainerTOTK.ovl");auto s=catalog::scan(f.paths.folder);assert(s.entries[0].name=="Zelda - Tears of the Kingdom");++count;}
    {Fixture f(base,count);f.plugin("custom.OVL","Native title");write(fs::path(f.paths.folder)/"custom.ini","[other]\nname=Wrong\n[trainer]\nname=\"Custom game\"\nversion=9.9\n");auto s=catalog::scan(f.paths.folder);assert(s.entries[0].name=="Custom game"&&s.entries[0].version=="0.1");++count;}
    {Fixture f(base,count);write(fs::path(f.paths.folder)/"Plain.ovl",nro("","",false));auto s=catalog::scan(f.paths.folder);assert(s.entries.size()==1&&s.entries[0].name=="Plain"&&s.entries[0].version.empty());++count;}
    {Fixture f(base,count);f.plugin("lower.ovl","zeta");f.plugin("UPPER.ovl","Alpha");auto s=catalog::scan(f.paths.folder);assert(s.entries.size()==2&&s.entries[0].filename=="UPPER.ovl");++count;}
    {Fixture f(base,count);f.plugin("Trainers.ovl");f.plugin("ovlmenu.ovl");f.plugin(".hidden.ovl");write(fs::path(f.paths.folder)/"readme.ini","data");assert(catalog::scan(f.paths.folder).entries.empty());++count;}
    {Fixture f(base,count);write(fs::path(f.paths.folder)/"nested/game.ovl",nro());write(fs::path(f.paths.overlays)/"Other.ovl",nro());assert(catalog::scan(f.paths.folder).entries.empty());++count;}
    {Fixture f(base,count);write(fs::path(f.paths.folder)/"Bad.ovl","not an overlay");auto s=catalog::scan(f.paths.folder);assert(s.entries.empty()&&s.invalid==1);++count;}
    {Fixture f(base,count);auto data=nro();std::memset(data.data()+0x18,0xFF,4);write(fs::path(f.paths.folder)/"Bad.ovl",data);assert(!catalog::read(f.paths.folder+"/Bad.ovl",entry));++count;}
    {Fixture f(base,count);auto data=nro();std::memset(data.data()+0x80+24,0xFF,8);write(fs::path(f.paths.folder)/"Safe.ovl",data);assert(catalog::read(f.paths.folder+"/Safe.ovl",entry)&&entry.name=="Safe"&&entry.version.empty());++count;}
    {Fixture f(base,count);assert(!catalog::read(f.paths.folder+"/Missing.ovl",entry));assert(!catalog::read(std::string(513,'x'),entry));++count;}
    {Fixture f(base,count);f.plugin("Game.ovl");write(fs::path(f.paths.folder)/"Game.ini",std::string(4097,'X'));assert(catalog::read(f.paths.folder+"/Game.ovl",entry)&&entry.name=="Game");++count;}
    {Fixture f(base,count);auto s=catalog::scan(f.paths.folder);assert(s.entries.empty());f.plugin("Added.ovl","Added Game","1.0");s=catalog::scan(f.paths.folder);assert(s.entries.size()==1&&s.entries[0].version=="1.0");f.plugin("Added.ovl","Added Game","1.1");assert(catalog::scan(f.paths.folder).entries[0].version=="1.1");++count;}
    {Fixture f(base,count);for(unsigned i=0;i<130;i++)write(fs::path(f.paths.folder)/(std::to_string(i)+".ovl"),nro("","",false));auto s=catalog::scan(f.paths.folder);assert(s.entries.size()==128&&s.limited);++count;}
    {Fixture f(base,count);auto target=f.root/"outside.ovl";write(target,nro());fs::create_symlink(target,fs::path(f.paths.folder)/"Linked.ovl");assert(catalog::scan(f.paths.folder).entries.empty());++count;}
    {Fixture f(base,count);auto r=install::prepare(f.paths);assert(r.ready&&r.names&&r.migration&&!r.moved);auto config=contents(f.paths.config+"/overlays.ini");assert(config.find("[Trainers.ovl]")!=std::string::npos&&config.find("hide=false")!=std::string::npos);assert(!fs::exists(f.paths.config+"/packages.ini"));++count;}
    {Fixture f(base,count);write(fs::path(f.paths.overlays)/"SimpleTrainerSwitch.ovl",nro("Old BL2"));write(fs::path(f.paths.overlays)/"SimpleTrainerTOTK.ovl",nro("Old TOTK"));write(fs::path(f.paths.overlays)/"SimpleTrainerTOTK.ini","[trainer]\nname=My TOTK\n");auto r=install::prepare(f.paths);assert(r.ready&&r.names&&r.migration&&r.moved==2);assert(!fs::exists(fs::path(f.paths.overlays)/"SimpleTrainerSwitch.ovl"));assert(!fs::exists(fs::path(f.paths.overlays)/"SimpleTrainerTOTK.ovl"));assert(catalog::scan(f.paths.folder).entries.size()==2);assert(catalog::read(f.paths.folder+"/SimpleTrainerTOTK.ovl",entry)&&entry.name=="My TOTK");++count;}
    {Fixture f(base,count);write(fs::path(f.paths.overlays)/"SimpleTrainerSwitch.ovl",nro("old","0.1"));f.plugin("SimpleTrainerSwitch.ovl","new","0.2");const auto updated=contents(fs::path(f.paths.folder)/"SimpleTrainerSwitch.ovl");auto r=install::prepare(f.paths);assert(r.ready&&r.names&&!r.moved);assert(contents(fs::path(f.paths.folder)/"SimpleTrainerSwitch.ovl")==updated);assert(fs::exists(fs::path(f.paths.overlays)/"SimpleTrainerSwitch.ovl"));auto config=contents(f.paths.config+"/overlays.ini");assert(config.find("[SimpleTrainerSwitch.ovl]\nhide=true")!=std::string::npos);assert(catalog::scan(f.paths.folder).entries[0].version=="0.2");++count;}
    {Fixture f(base,count);write(f.paths.config+"/overlays.ini","[Trainers.ovl]\nstar=true\npriority=1\nhide=true\n[FPSLocker.ovl]\nhide=false\nstar=true\n");write(f.paths.oldPackage,"old package content");write(f.paths.config+"/packages.ini","[Trainers]\nstar=true\nhide=false\n[Other]\nhide=false\n");auto r=install::prepare(f.paths);assert(r.names);const auto config=contents(f.paths.config+"/overlays.ini");assert(config.find("star=true\npriority=1\nhide=false")!=std::string::npos);assert(config.substr(config.find("[FPSLocker.ovl]"))=="[FPSLocker.ovl]\nhide=false\nstar=true\n");assert(contents(f.paths.config+"/packages.ini").find("[Trainers]\nstar=true\nhide=true")!=std::string::npos);assert(contents(f.paths.oldPackage)=="old package content");++count;}
    {Fixture f(base,count);write(fs::path(f.paths.overlays)/"OtherGame.ovl",nro());auto r=install::prepare(f.paths);assert(r.ready&&r.migration&&r.moved==0&&fs::exists(fs::path(f.paths.overlays)/"OtherGame.ovl"));assert(contents(f.paths.config+"/overlays.ini").find("OtherGame.ovl")==std::string::npos);++count;}
    {Fixture f(base,count);write(fs::path(f.paths.overlays)/"TOTKTrainerLab.ovl",nro("Lab","0.1"));auto r=install::prepare(f.paths);assert(r.moved==1&&!fs::exists(fs::path(f.paths.overlays)/"TOTKTrainerLab.ovl"));auto s=catalog::scan(f.paths.folder);assert(s.entries.size()==1&&s.entries[0].name=="Zelda TOTK (Lab)");++count;}
    {Fixture f(base,count);write(fs::path(f.paths.overlays)/"SimpleTrainerSwitch.ovl",nro());write(fs::path(f.paths.folder)/"SimpleTrainerSwitch.ovl","bad destination");auto r=install::prepare(f.paths);assert(!r.migration&&fs::exists(fs::path(f.paths.overlays)/"SimpleTrainerSwitch.ovl"));assert(contents(f.paths.config+"/overlays.ini").find("[SimpleTrainerSwitch.ovl]")==std::string::npos);++count;}
    {Fixture f(base,count);auto r=install::prepare(f.paths);assert(r.ready&&r.names);const auto config=contents(f.paths.config+"/overlays.ini");r=install::prepare(f.paths);assert(r.ready&&r.names&&r.moved==0&&contents(f.paths.config+"/overlays.ini")==config);assert(!fs::exists(f.paths.config+"/overlays.ini.theoui.bak"));++count;}
    {Fixture f(base,count);fs::remove_all(f.root/"switch/TheouisTrainers");write(f.root/"switch/TheouisTrainers","blocked");write(fs::path(f.paths.overlays)/"SimpleTrainerSwitch.ovl",nro());auto r=install::prepare(f.paths);assert(!r.ready&&fs::exists(fs::path(f.paths.overlays)/"SimpleTrainerSwitch.ovl"));++count;}
    {Fixture f(base,count);fs::create_directories(f.paths.config+"/overlays.ini");f.plugin("Game.ovl");auto r=install::prepare(f.paths);assert(r.ready&&!r.names&&catalog::scan(f.paths.folder).entries.size()==1);++count;}
    if(argc>1){assert(catalog::read(argv[1],entry)&&entry.name=="Borderlands 2"&&entry.version=="0.1");++count;}
    if(argc>2){assert(catalog::read(argv[2],entry)&&entry.name=="Theoui's Trainers"&&entry.version=="1.0");++count;}
    fs::remove_all(base);std::cout<<count<<" catalogue / metadata / discovery / safe migration scenarios passed.\n";
}
