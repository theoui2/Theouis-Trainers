// Copyright (C) 2026 Theoui. SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace shortcuts {
using Keys=std::uint64_t;
constexpr Keys LS=1ULL<<4,RS=1ULL<<5,L=1ULL<<6,R=1ULL<<7,ZL=1ULL<<8,ZR=1ULL<<9,UP=1ULL<<13;
constexpr Keys PHYSICAL=0x0F00FFFFULL;
constexpr std::uint64_t BL2=0x010096F00FF22000ULL,TOTK=0x0100F2C0115B6000ULL;
constexpr const char* HUB="sdmc:/switch/.overlays/Trainers.ovl";
constexpr const char* AUTO="--theoui-auto";
constexpr const char* CONFIG="sdmc:/config/theouis-trainers/config.ini";
struct Preset {const char* label;const char* combo;Keys keys;};
inline constexpr std::array<Preset,7> PRESETS{{
    {"Off","",0},{"L3 + R3","LS+RS",LS|RS},{"L + R + L3","L+R+LS",L|R|LS},
    {"L + R + R3","L+R+RS",L|R|RS},{"ZL + ZR + L3","ZL+ZR+LS",ZL|ZR|LS},
    {"ZL + ZR + R3","ZL+ZR+RS",ZL|ZR|RS},{"L + R + D-Up","L+R+DUP",L|R|UP}
}};
inline std::string trim(const std::string& s){const auto a=s.find_first_not_of(" \t\r\n");return a==std::string::npos?std::string{}:s.substr(a,s.find_last_not_of(" \t\r\n")-a+1);}
inline std::string lower(std::string s){for(char& c:s)if(c>='A'&&c<='Z')c+=32;return s;}
inline std::string unquote(std::string s){s=trim(s);if(s.size()>=2&&((s.front()=='\''&&s.back()=='\'')||(s.front()=='"'&&s.back()=='"')))s=s.substr(1,s.size()-2);return s;}
struct Value {std::string section,key,value;};
inline std::vector<Value> ini(const std::string& input){
    std::vector<Value> out;std::string section;
    for(std::size_t pos=0;pos<input.size();){
        auto end=input.find('\n',pos);if(end==std::string::npos)end=input.size();const auto line=trim(input.substr(pos,end-pos));pos=end+1;
        if(line.empty()||line[0]==';'||line[0]=='#')continue;
        if(line.front()=='['&&line.back()==']'){section=trim(line.substr(1,line.size()-2));continue;}
        const auto eq=line.find('=');if(eq!=std::string::npos)out.push_back({section,lower(trim(line.substr(0,eq))),unquote(line.substr(eq+1))});
    }
    return out;
}
inline std::string value(const std::string& input,const std::string& section,const std::string& key){
    std::string out;for(const auto& v:ini(input))if(v.section==section&&v.key==key)out=v.value;return out;
}
inline bool list(std::string input,std::vector<std::string>& out){
    out.clear();input=trim(input);if(input.empty())return true;
    if(input.size()<2||input.front()!='('||input.back()!=')')return false;
    input=input.substr(1,input.size()-2);if(trim(input).empty())return true;
    char quote=0;std::size_t start=0;
    for(std::size_t i=0;i<=input.size();i++){
        const char c=i<input.size()?input[i]:',';
        if(c=='\''||c=='"'){if(!quote)quote=c;else if(quote==c)quote=0;}
        if(c==','&&!quote){out.push_back(trim(input.substr(start,i-start)));start=i+1;}
    }
    return !quote;
}
inline std::string joined(const std::vector<std::string>& values){std::string out="(";for(std::size_t i=0;i<values.size();i++){if(i)out+=",";out+=values[i];}return out+")";}
inline Keys combo(std::string text){
    text=unquote(text);Keys out=0;
    const std::array<std::pair<const char*,Keys>,18> names{{{"a",1},{"b",2},{"x",4},{"y",8},{"ls",LS},{"rs",RS},{"l",L},{"r",R},{"zl",ZL},{"zr",ZR},{"plus",1ULL<<10},{"minus",1ULL<<11},{"dleft",1ULL<<12},{"dup",UP},{"dright",1ULL<<14},{"ddown",1ULL<<15},{"sl",(1ULL<<24)|(1ULL<<26)},{"sr",(1ULL<<25)|(1ULL<<27)}}};
    for(std::size_t pos=0;pos<text.size();){auto end=text.find('+',pos);if(end==std::string::npos)end=text.size();const auto key=lower(trim(text.substr(pos,end-pos)));pos=end+1;bool found=false;
        for(const auto& [name,mask]:names)if(key==name){out|=mask;found=true;break;}
        if(!found)return 0;
    }
    return out;
}
inline bool titleId(std::string s,std::uint64_t& out){
    out=0;s=unquote(s);if(s.size()==18&&s.substr(0,2)=="0x")s=s.substr(2);if(s.size()!=16)return false;
    for(char c:s){unsigned v;if(c>='0'&&c<='9')v=unsigned(c-'0');else if(c>='A'&&c<='F')v=unsigned(c-'A'+10);else if(c>='a'&&c<='f')v=unsigned(c-'a'+10);else{out=0;return false;}out=(out<<4)|v;}
    return out!=0;
}
inline unsigned preset(const std::string& text){const auto s=value(text,"shortcuts","preset");if(s.size()==1&&s[0]>='0'&&s[0]<='6')return unsigned(s[0]-'0');return 0;}
inline unsigned loadPreset(const char* path=CONFIG){
    FILE* f=std::fopen(path,"rb");if(!f)return 0;std::array<char,4097> data{};const auto n=std::fread(data.data(),1,data.size(),f);const bool ok=!std::ferror(f)&&n<data.size();std::fclose(f);return ok?preset(std::string(data.data(),n)):0;
}
// Require release after loading/changing a preset, and after every trigger.
class Gesture {
    Keys previous=0;bool armed=false;
public:
    bool poll(Keys mask,Keys held,Keys down){
        if(mask!=previous){previous=mask;armed=false;}
        if(!mask)return false;
        if(!(held&mask)){armed=true;return false;}
        if(armed&&(held&PHYSICAL)==mask&&(down&mask)){armed=false;return true;}
        return false;
    }
};
}
