// Copyright (C) 2026 Theoui. SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "shortcut_core.hpp"
#include "shortcut_settings.hpp"

namespace shortcut_runtime {
static_assert(shortcuts::LS==HidNpadButton_StickL&&shortcuts::RS==HidNpadButton_StickR);
static_assert(shortcuts::L==HidNpadButton_L&&shortcuts::R==HidNpadButton_R&&shortcuts::ZL==HidNpadButton_ZL&&shortcuts::ZR==HidNpadButton_ZR&&shortcuts::UP==HidNpadButton_Up);
struct Application {u64 pid=0,title=0;bool valid=false;};
inline Application application(){
    Application app;u64 after=0;
    if(R_FAILED(pmdmntGetApplicationProcessId(&app.pid))||!app.pid)return app;
    Result opened=0;tsl::hlp::doWithSmSession([&]{opened=pminfoInitialize();});if(R_FAILED(opened))return app;
    const auto result=pminfoGetProgramId(&app.title,app.pid);pminfoExit();
    app.valid=R_SUCCEEDED(result)&&app.title&&R_SUCCEEDED(pmdmntGetApplicationProcessId(&after))&&after==app.pid;
    return app;
}
inline void load(){
    unsigned index=0;tsl::hlp::doWithSDCardHandle([&]{
        const shortcuts::Paths p;index=shortcuts::loadPreset();std::string overlays,packages,ultra,tesla;
        if(!shortcuts::read(p.overlays,overlays)||!shortcuts::read(p.packages,packages)||!shortcuts::read(p.ultrahand,ultra)||!shortcuts::read(p.tesla,tesla)||
           !shortcuts::collision(overlays,packages,ultra,tesla,shortcuts::PRESETS[index].keys).empty())index=0;
    });tsl::cfg::contextCombo.store(shortcuts::PRESETS[index].keys);
}
inline bool redirect(std::uint64_t own,std::string& why){
    const auto current=application();if(current.valid&&current.title==own)return false;
    bool exists=false;tsl::hlp::doWithSDCardHandle([&]{if(FILE* f=std::fopen(shortcuts::HUB,"rb")){exists=true;std::fclose(f);}});
    if(!exists){why="Trainer launcher missing - install Theoui's Trainers";return false;}
    tsl::setNextOverlay(shortcuts::HUB,shortcuts::AUTO);tsl::Overlay::get()->close();return true;
}
}
