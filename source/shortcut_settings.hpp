// Copyright (C) 2026 Theoui. SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "shortcut_core.hpp"
#include "menu_names.hpp"
#include <sys/stat.h>

namespace shortcuts {
struct Paths {
    std::string root,config,overlays,packages,ultrahand,tesla;
    explicit Paths(const std::string& sd="sdmc:"):root(sd),config(sd+"/config/theouis-trainers/config.ini"),
        overlays(sd+"/config/ultrahand/overlays.ini"),packages(sd+"/config/ultrahand/packages.ini"),
        ultrahand(sd+"/config/ultrahand/config.ini"),tesla(sd+"/config/tesla/config.ini"){}
};
inline bool directory(const std::string& path){struct stat s{};if(!stat(path.c_str(),&s))return S_ISDIR(s.st_mode);return errno==ENOENT&&!mkdir(path.c_str(),0777);}
inline bool read(const std::string& path,std::string& text){return menu_names::read(path,text)!=menu_names::Read::Error;}
inline bool writeChecked(const std::string& path,const std::string& text){
    FILE* f=std::fopen(path.c_str(),"wb");if(!f)return false;bool ok=std::fwrite(text.data(),1,text.size(),f)==text.size();if(std::fclose(f))ok=false;std::string check;return ok&&menu_names::read(path,check)==menu_names::Read::Ok&&check==text;
}
// Keep the first backup, but reuse our transaction slot for subsequent edits.
inline bool replace(const std::string& path,const std::string& text){
    std::string before;const auto state=menu_names::read(path,before);if(state==menu_names::Read::Error)return false;
    if(state==menu_names::Read::Ok&&text==before)return true;
    const auto tmp=path+".theoui-shortcut.tmp",rollback=path+".theoui-shortcut.rollback",backup=path+".theoui-shortcut.bak";
    struct stat st{};if(!lstat(rollback.c_str(),&st))return false;
    if(!writeChecked(tmp,text)){std::remove(tmp.c_str());return false;}
    if(state==menu_names::Read::Ok){
        std::string ignored;const auto saved=menu_names::read(backup,ignored);
        if(saved==menu_names::Read::Error||(saved==menu_names::Read::Missing&&!writeChecked(backup,before))||std::rename(path.c_str(),rollback.c_str())){std::remove(tmp.c_str());return false;}
    }
    if(std::rename(tmp.c_str(),path.c_str())){if(state==menu_names::Read::Ok)std::rename(rollback.c_str(),path.c_str());std::remove(tmp.c_str());return false;}
    if(state==menu_names::Read::Ok)std::remove(rollback.c_str());
    return true;
}
inline std::string collision(const std::string& overlays,const std::string& packages,const std::string& ultra,const std::string& tesla,Keys mask){
    if(!mask)return {};
    for(const auto& config:{std::pair<std::string,std::string>{ultra,"ultrahand"},{tesla,"tesla"}}){
        const auto global=value(config.first,config.second,"key_combo");if(combo(global)&&(mask&combo(global))==combo(global))return "Ultrahand's main shortcut";
    }
    if(value(ultra,"ultrahand","launch_combos")=="false")return "Launch Combos disabled in Ultrahand";
    for(const auto& doc:{std::pair<std::string,bool>{overlays,true},{packages,false}}){
        for(const auto& v:ini(doc.first)){
            if(v.key=="key_combo"&&combo(v.value)==mask)return v.section;
            if(v.key!="mode_combos")continue;
            std::vector<std::string> combos,args;if(!list(v.value,combos))return "Unreadable shortcut list in "+v.section;
            if(!list(value(doc.first,v.section,"mode_args"),args))return "Unreadable modes in "+v.section;
            for(std::size_t i=0;i<combos.size();i++)if(combo(combos[i])==mask){
                if(doc.second&&v.section=="Trainers.ovl"&&i<args.size()&&unquote(args[i])==AUTO)continue;
                return v.section;
            }
        }
    }
    return {};
}
inline bool registry(const std::string& input,unsigned index,std::string& output){
    if(index>=PRESETS.size())return false;
    std::vector<std::string> args,labels,combos;
    if(!list(value(input,"Trainers.ovl","mode_args"),args)||!list(value(input,"Trainers.ovl","mode_labels"),labels)||!list(value(input,"Trainers.ovl","mode_combos"),combos))return false;
    if(labels.size()>args.size()||combos.size()>args.size())return false;
    labels.resize(args.size());combos.resize(args.size());std::size_t slot=args.size();
    for(std::size_t i=0;i<args.size();i++)if(unquote(args[i])==AUTO){if(slot!=args.size())return false;slot=i;}
    if(slot==args.size()){
        if(!index){output=input;return true;}
        args.push_back(AUTO);labels.push_back("Current game trainer");combos.push_back("");
    }
    combos[slot]=PRESETS[index].combo;
    output=menu_names::updated(input,"Trainers.ovl",{{"mode_args",joined(args)},{"mode_labels",joined(labels)},{"mode_combos",joined(combos)}});return true;
}
struct Setting {unsigned selected=0;std::string message;bool applied=false;};
inline Setting apply(const Paths& p,unsigned index){
    Setting out{loadPreset(p.config.c_str()),{}};
    if(index>=PRESETS.size()){out.message="Unknown shortcut preset";return out;}
    std::string old,packages,ultra,tesla,config;
    if(!read(p.overlays,old)||!read(p.packages,packages)||!read(p.ultrahand,ultra)||!read(p.tesla,tesla)||!read(p.config,config)){
        out.message="Could not read shortcut settings";return out;
    }
    if(config.size()>4096){out.message="Shortcut config is too large";return out;}
    const auto used=collision(old,packages,ultra,tesla,PRESETS[index].keys);
    if(!used.empty()){out.message="Shortcut unavailable: "+used;return out;}
    std::string next;if(!registry(old,index,next)){out.message="Existing launcher modes need checking";return out;}
    if(!directory(p.root+"/config")||!directory(p.root+"/config/ultrahand")||!directory(p.root+"/config/theouis-trainers")){
        out.message="Shortcut folder unavailable";return out;
    }
    if(!replace(p.overlays,next)){out.message="Could not update Ultrahand shortcut";return out;}
    const auto settings=menu_names::updated(config,"shortcuts",{{"preset",std::to_string(index)}});
    if(!replace(p.config,settings)){
        const bool rolledBack=replace(p.overlays,old);out.message=rolledBack?"Could not save preset; previous shortcut kept":"Could not save preset; reopen launcher to repair";return out;
    }
    out.selected=index;out.applied=true;out.message=index?"Saved - return to Ultrahand to refresh shortcuts":"Trainer shortcut disabled";return out;
}
inline Setting initialize(const Paths& p){
    std::string text;const auto state=menu_names::read(p.config,text);
    if(state==menu_names::Read::Error)return {0,"Could not read shortcut settings"};
    return apply(p,state==menu_names::Read::Missing?1:preset(text));
}
}
