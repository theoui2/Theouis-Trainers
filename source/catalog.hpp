// Copyright (C) 2026 Theoui. SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <vector>
#include "shortcut_core.hpp"

namespace catalog {
struct Entry {std::string filename,path,name,version;std::uint64_t titleId=0;};
struct Scan {std::vector<Entry> entries;unsigned invalid=0;bool ready=false,limited=false;};
inline std::string trim(const std::string& s){
    const auto a=s.find_first_not_of(" \t\r\n");return a==std::string::npos?std::string{}:s.substr(a,s.find_last_not_of(" \t\r\n")-a+1);
}
inline std::string lower(std::string s){for(char& c:s)if(c>='A'&&c<='Z')c+=32;return s;}
inline bool overlay(const std::string& name){
    const auto n=lower(name);return n.size()>4&&n.front()!='.'&&n.ends_with(".ovl")&&n!="trainers.ovl"&&n!="ovlmenu.ovl";
}
inline std::string join(const std::string& folder,const std::string& name){return folder+"/"+name;}
inline bool regular(const std::string& p){struct stat st{};return !lstat(p.c_str(),&st)&&S_ISREG(st.st_mode);}
inline std::string text(const unsigned char* p,std::size_t size){
    std::string out;for(std::size_t i=0;i<size&&p[i];i++)if(p[i]>=32&&p[i]!=127)out+=char(p[i]);
    return trim(out);
}
inline std::uint32_t u32(const unsigned char* p){std::uint32_t n=0;for(unsigned i=0;i<4;i++)n|=std::uint32_t(p[i])<<(8*i);return n;}
inline std::uint64_t u64(const unsigned char* p){std::uint64_t n=0;for(unsigned i=0;i<8;i++)n|=std::uint64_t(p[i])<<(8*i);return n;}
inline bool at(FILE* f,std::uint64_t extent,std::uint64_t offset,void* p,std::size_t size){
    return offset<=extent&&size<=extent-offset&&offset<=LONG_MAX&&!std::fseek(f,long(offset),SEEK_SET)&&std::fread(p,1,size,f)==size;
}
struct Sidecar {std::string name;std::uint64_t titleId=0;bool titleProvided=false;};
inline Sidecar sidecar(const std::string& path){
    FILE* f=std::fopen((path.substr(0,path.size()-4)+".ini").c_str(),"rb");if(!f)return {};
    std::array<unsigned char,4097> bytes{};const auto count=std::fread(bytes.data(),1,bytes.size(),f);const bool ok=!std::ferror(f)&&count<bytes.size();std::fclose(f);
    if(!ok)return {};
    Sidecar out;
    for(const auto& v:shortcuts::ini(std::string(reinterpret_cast<const char*>(bytes.data()),count))){
        if(lower(v.section)!="trainer")continue;
        if(v.key=="name"&&v.value.size()<=128)out.name=text(reinterpret_cast<const unsigned char*>(v.value.data()),v.value.size());
        if(v.key=="title_id"){out.titleProvided=true;if(!shortcuts::titleId(v.value,out.titleId))out.titleId=0;}
    }
    return out;
}
inline bool read(const std::string& path,Entry& out){
    out={};if(path.size()>=512||!regular(path))return false;
    FILE* f=std::fopen(path.c_str(),"rb");if(!f)return false;
    const bool sized=!std::fseek(f,0,SEEK_END);const long end=sized?std::ftell(f):-1;
    std::array<unsigned char,0x80> header{};
    const bool valid=end>=0x80&&at(f,std::uint64_t(end),0,header.data(),header.size())&&u32(header.data()+0x10)==0x304F524E&&
        u32(header.data()+0x18)>=header.size()&&u32(header.data()+0x18)<=std::uint64_t(end);
    if(!valid){std::fclose(f);return false;}
    out.path=path;out.filename=path.substr(path.find_last_of('/')+1);
    const auto size=u32(header.data()+0x18);std::array<unsigned char,0x38> assets{};
    if(at(f,end,size,assets.data(),assets.size())&&u32(assets.data())==0x54455341){
        const auto offset=u64(assets.data()+24),length=u64(assets.data()+32);
        if(offset>=assets.size()&&offset<=std::uint64_t(end)-size&&length>=0x3070&&length<=std::uint64_t(end)-size-offset){
            std::array<unsigned char,512> label{};std::array<unsigned char,16> version{};
            for(unsigned language=0;language<16&&out.name.empty();language++)
                if(at(f,end,size+offset+language*0x300,label.data(),label.size()))out.name=text(label.data(),label.size());
            if(at(f,end,size+offset+0x3060,version.data(),version.size()))out.version=text(version.data(),version.size());
        }
    }
    std::fclose(f);
    const auto custom=sidecar(path);const auto filename=lower(out.filename);
    if(!custom.name.empty())out.name=custom.name;
    else if(filename=="simpletrainerswitch.ovl")out.name="Borderlands 2";
    else if(filename=="simpletrainertotk.ovl")out.name="Zelda - Tears of the Kingdom";
    else if(filename=="totktrainerlab.ovl")out.name="Zelda TOTK (Lab)";
    if(out.name.empty())out.name=out.filename.substr(0,out.filename.size()-4);
    if(custom.titleProvided)out.titleId=custom.titleId;
    else if(filename=="simpletrainerswitch.ovl")out.titleId=shortcuts::BL2;
    else if(filename=="simpletrainertotk.ovl")out.titleId=shortcuts::TOTK;
    return true;
}
inline Scan scan(const std::string& folder){
    Scan result;DIR* dir=opendir(folder.c_str());if(!dir)return result;result.ready=true;unsigned visited=0;
    while(auto* item=readdir(dir)){
        if(++visited>4096){result.limited=true;break;}
        const std::string name=item->d_name;if(!overlay(name))continue;
        Entry entry;if(!read(join(folder,name),entry)){++result.invalid;continue;}
        if(result.entries.size()==128){result.limited=true;break;}
        result.entries.push_back(std::move(entry));
    }
    closedir(dir);
    std::sort(result.entries.begin(),result.entries.end(),[](const auto& a,const auto& b){
        const auto x=lower(a.name),y=lower(b.name);return x==y?a.filename<b.filename:x<y;
    });
    return result;
}
struct Choice {std::string path,reason;};
inline Choice choose(const Scan& trainers,std::uint64_t title){
    if(!trainers.ready)return {{},"Trainer folder unavailable"};
    if(trainers.limited)return {{},"Trainer list incomplete - choose manually"};
    if(!title)return {{},"No running game detected - choose manually"};
    Choice out;
    for(const auto& e:trainers.entries)if(e.titleId==title){
        if(!out.path.empty())return {{},"Multiple trainers for this game - choose manually"};
        out.path=e.path;
    }
    if(out.path.empty())out.reason="No trainer installed for this game";
    return out;
}
}
