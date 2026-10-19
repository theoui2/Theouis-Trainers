// Copyright (C) 2026 Theoui. SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cerrno>
#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace menu_names {
inline constexpr const char* TITLE="Theoui's Trainers";
inline constexpr const char* VERSION="1.0";
inline std::string trim(const std::string& value){
    const auto first=value.find_first_not_of(" \t\r\n");
    return first==std::string::npos?std::string{}:value.substr(first,value.find_last_not_of(" \t\r\n")-first+1);
}
// Preserve unrelated entries, favourite/visibility/launch settings and comments.
using Fields=std::vector<std::pair<std::string,std::string>>;
inline std::string updated(const std::string& input,const std::string& section,const Fields& fields){
    if(fields.empty())return input;
    const std::string newline=input.find("\r\n")!=std::string::npos?"\r\n":"\n";
    std::string out;bool target=false,seen=false;std::vector<bool> keys(fields.size(),false);
    const auto finish=[&]{
        if(!target)return;
        if(!out.empty()&&out.back()!='\n')out+=newline;
        for(std::size_t i=0;i<fields.size();i++)if(!keys[i])out+=fields[i].first+"="+fields[i].second+newline;
    };
    for(std::size_t start=0;start<input.size();){
        const auto end=input.find('\n',start);const auto count=end==std::string::npos?input.size()-start:end-start+1;
        const auto line=input.substr(start,count),plain=trim(line);start+=count;
        if(!plain.empty()&&plain.front()=='['&&plain.back()==']'){
            finish();target=trim(plain.substr(1,plain.size()-2))==section;seen|=target;std::fill(keys.begin(),keys.end(),false);
        }
        const auto equal=plain.find('=');const auto key=equal==std::string::npos?std::string{}:trim(plain.substr(0,equal));
        std::size_t index=0;for(;index<fields.size();index++)if(fields[index].first==key)break;
        if(target&&index<fields.size()){out+=key+"="+fields[index].second+newline;keys[index]=true;}
        else out+=line;
    }
    finish();
    if(!seen){if(!out.empty()&&out.back()!='\n')out+=newline;out+="["+section+"]"+newline;for(const auto& p:fields)out+=p.first+"="+p.second+newline;}
    return out;
}
inline std::string updated(const std::string& input,const std::string& section){return updated(input,section,{{"custom_name",TITLE},{"custom_version",VERSION}});}
enum class Read{Ok,Missing,Error};
inline Read read(const std::string& path,std::string& out){
    out.clear();FILE* f=std::fopen(path.c_str(),"rb");
    if(!f)return errno==ENOENT?Read::Missing:Read::Error;
    bool ok=std::fseek(f,0,SEEK_END)==0;const long size=ok?std::ftell(f):-1;
    ok=ok&&size>=0&&size<=256*1024&&std::fseek(f,0,SEEK_SET)==0;
    if(ok){out.resize(std::size_t(size));ok=out.empty()||std::fread(out.data(),1,out.size(),f)==out.size();}
    if(std::fclose(f))ok=false;
    return ok?Read::Ok:Read::Error;
}
inline bool apply(const std::string& path,const std::string& section,const Fields& fields){
    std::string previous;const auto state=read(path,previous);if(state==Read::Error)return false;
    const auto next=updated(previous,section,fields);if(state==Read::Ok&&next==previous)return true;
    const auto temporary=path+".theoui.tmp";FILE* f=std::fopen(temporary.c_str(),"wb");if(!f)return false;
    bool ok=std::fwrite(next.data(),1,next.size(),f)==next.size();if(std::fclose(f))ok=false;
    std::string check;if(!ok||read(temporary,check)!=Read::Ok||check!=next){std::remove(temporary.c_str());return false;}
    std::string backup;
    if(state==Read::Ok){
        bool free=false;
        for(unsigned i=0;i<100;i++){
            backup=path+".theoui.bak"+(i?"."+std::to_string(i):std::string{});
            std::string ignored;const auto available=read(backup,ignored);
            if(available==Read::Missing){free=true;break;}
            if(available==Read::Error)break;
        }
        if(!free||std::rename(path.c_str(),backup.c_str())){std::remove(temporary.c_str());return false;}
    }
    if(std::rename(temporary.c_str(),path.c_str())){
        if(!backup.empty())std::rename(backup.c_str(),path.c_str());
        std::remove(temporary.c_str());return false;
    }
    return true;
}
inline bool apply(const std::string& path,const std::string& section){return apply(path,section,{{"custom_name",TITLE},{"custom_version",VERSION}});}
}
