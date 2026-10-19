// Copyright (C) 2026 Theoui. SPDX-License-Identifier: GPL-2.0-or-later
// Exercise the production runtime adapter with fake Switch IPC and overlay APIs.
#include <atomic>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
using u64=std::uint64_t;using Result=int;
constexpr u64 HidNpadButton_StickL=1ULL<<4,HidNpadButton_StickR=1ULL<<5,HidNpadButton_L=1ULL<<6,HidNpadButton_R=1ULL<<7,HidNpadButton_ZL=1ULL<<8,HidNpadButton_ZR=1ULL<<9,HidNpadButton_Up=1ULL<<13;
#define R_FAILED(value) ((value)!=0)
#define R_SUCCEEDED(value) ((value)==0)
namespace fake {
std::vector<u64> pids;unsigned calls=0,opens=0,exits=0,closes=0;u64 title=0;int openError=0,titleError=0,pidError=0;
std::string next,args;
void reset(u64 id){pids={100,100};calls=opens=exits=closes=0;title=id;openError=titleError=pidError=0;next.clear();args.clear();}
}
Result pmdmntGetApplicationProcessId(u64* out){*out=fake::pids[std::min<std::size_t>(fake::calls++,fake::pids.size()-1)];return fake::pidError;}
Result pminfoInitialize(){++fake::opens;return fake::openError;}
void pminfoExit(){++fake::exits;}
Result pminfoGetProgramId(u64* out,u64){*out=fake::title;return fake::titleError;}
namespace tsl {
namespace cfg {inline std::atomic<u64> contextCombo{0};}
namespace hlp {template<class F>void doWithSmSession(F&& f){f();}template<class F>void doWithSDCardHandle(F&& f){f();}}
void setNextOverlay(const std::string& path,const std::string& args){fake::next=path;fake::args=args;}
class Overlay {public:static Overlay* get(){static Overlay instance;return &instance;}void close(){++fake::closes;}};
}
#include "../source/shortcut_runtime.hpp"

int main(){
    using namespace shortcuts;namespace fs=std::filesystem;
    unsigned count=0;std::string why;
    {fake::reset(BL2);const auto app=shortcut_runtime::application();assert(app.valid&&app.pid==100&&app.title==BL2&&fake::calls==2&&fake::opens==1&&fake::exits==1);++count;}
    {fake::reset(TOTK);fake::pids={100,200};assert(!shortcut_runtime::application().valid&&fake::exits==1);++count;}
    {fake::reset(BL2);fake::openError=1;assert(!shortcut_runtime::application().valid&&fake::opens==1&&!fake::exits);++count;}
    {fake::reset(BL2);fake::titleError=1;assert(!shortcut_runtime::application().valid&&fake::exits==1);++count;}
    {fake::reset(BL2);fake::pidError=1;assert(!shortcut_runtime::application().valid&&!fake::opens);++count;}
    {fake::reset(0);assert(!shortcut_runtime::application().valid&&fake::exits==1);++count;}
    {fake::reset(BL2);assert(!shortcut_runtime::redirect(BL2,why)&&fake::next.empty()&&!fake::closes);++count;}
    {fake::reset(TOTK);assert(!shortcut_runtime::redirect(TOTK,why)&&fake::next.empty()&&!fake::closes);++count;}
    char tmp[]="/tmp/theoui-shortcut-runtime-XXXXXX";assert(mkdtemp(tmp));const auto original=fs::current_path();const fs::path base=tmp;fs::current_path(base);
    {fake::reset(TOTK);assert(!shortcut_runtime::redirect(BL2,why)&&!fake::closes&&why.find("missing")!=std::string::npos);++count;}
    fs::create_directories("sdmc:/switch/.overlays");assert(writeChecked(HUB,"test overlay placeholder"));
    {fake::reset(TOTK);assert(shortcut_runtime::redirect(BL2,why)&&fake::next==HUB&&fake::args==AUTO&&fake::closes==1);++count;}
    {fake::reset(BL2);assert(shortcut_runtime::redirect(TOTK,why)&&fake::next==HUB&&fake::args==AUTO&&fake::closes==1);++count;}
    {fake::reset(0x0100123456789000ULL);assert(shortcut_runtime::redirect(BL2,why)&&fake::next==HUB);++count;}
    {fake::reset(0);fake::pidError=1;assert(shortcut_runtime::redirect(BL2,why)&&fake::next==HUB);++count;}
    {shortcut_runtime::load();assert(!tsl::cfg::contextCombo.load());++count;}
    {const auto setting=initialize(Paths{});assert(setting.applied);shortcut_runtime::load();assert(tsl::cfg::contextCombo.load()==(LS|RS));++count;}
    {const auto setting=apply(Paths{},5);assert(setting.applied);shortcut_runtime::load();assert(tsl::cfg::contextCombo.load()==(ZL|ZR|RS));++count;}
    {assert(writeChecked("sdmc:/config/ultrahand/packages.ini","[Other]\nkey_combo=ZL+ZR+RS\n"));shortcut_runtime::load();assert(!tsl::cfg::contextCombo.load());++count;}
    {assert(apply(Paths{},0).applied);shortcut_runtime::load();assert(!tsl::cfg::contextCombo.load());++count;}
    fs::current_path(original);fs::remove_all(base);
    std::cout<<count<<" runtime game-ID / process-race / reopen / redirect / saved-preset scenarios passed.\n";
}
