// Copyright (C) 2026 Theoui. SPDX-License-Identifier: GPL-2.0-or-later
#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include "legacy_install.hpp"
#include "shortcut_settings.hpp"
#include "shortcut_runtime.hpp"

namespace {
constexpr const char* MENU="sdmc:/switch/.overlays/ovlmenu.ovl";
shortcuts::Setting shortcutSetting;
std::string launchNotice;

bool launch(const std::string& path){
    bool valid=false;tsl::hlp::doWithSDCardHandle([&]{catalog::Entry entry;valid=catalog::read(path,entry);});
    if(!valid){launchNotice="File missing or invalid - refresh trainers";return false;}
    tsl::setNextOverlay(path);tsl::Overlay::get()->close();return true;
}
class ShortcutGui:public tsl::Gui {
    tsl::elm::CategoryHeader* message=nullptr;
    std::vector<tsl::elm::ListItem*> rows;
public:
    tsl::elm::Element* createUI()override{
        auto* frame=new tsl::elm::OverlayFrame("Shortcut settings","Theoui's Trainers 1.0");
        auto* list=new tsl::elm::List();
        list->addItem(new tsl::elm::CategoryHeader("Choose a shortcut for the current game"));
        message=new tsl::elm::CategoryHeader(shortcutSetting.message);list->addItem(message);
        for(unsigned i=0;i<shortcuts::PRESETS.size();i++){
            auto* item=new tsl::elm::ListItem(shortcuts::PRESETS[i].label,i==shortcutSetting.selected?"Selected":"");rows.push_back(item);
            item->setClickListener([this,i](u64 keys){
                if(!(keys&HidNpadButton_A))return false;
                tsl::hlp::doWithSDCardHandle([&]{shortcutSetting=shortcuts::apply(shortcuts::Paths{},i);});
                shortcut_runtime::load();
                message->setText(shortcutSetting.message);
                for(unsigned n=0;n<rows.size();n++)rows[n]->setValue(n==shortcutSetting.selected?"Selected":"");
                return true;
            });list->addItem(item);
        }
        list->addItem(new tsl::elm::CategoryHeader("L3 / R3 = press the left / right stick"));
        auto* back=new tsl::elm::ListItem("Return to Ultrahand");
        back->setClickListener([](u64 keys){if(!(keys&HidNpadButton_A))return false;launch(MENU);return true;});list->addItem(back);
        frame->setContent(list);return frame;
    }
};
class LauncherGui:public tsl::Gui {
    tsl::elm::CategoryHeader* message=nullptr;
    tsl::elm::ListItem* shortcutItem=nullptr;
    std::string shown;
public:
    tsl::elm::Element* createUI()override{
        install::Result installation;catalog::Scan trainers;
        tsl::hlp::doWithSDCardHandle([&]{const install::Paths paths;
            installation=install::prepare(paths);trainers=catalog::scan(paths.folder);
        });
        auto* frame=new tsl::elm::OverlayFrame("Theoui's Trainers 1.0","by Theoui");
        auto* list=new tsl::elm::List();
        shown="Installed trainers";
        if(!trainers.ready)shown="Trainer folder unavailable - check SD";
        else if(!installation.names)shown="Old menu entries could not be hidden";
        else if(!installation.migration)shown="Some old trainers need a manual copy";
        else if(trainers.invalid)shown="Some .ovl files are invalid";
        else if(trainers.limited)shown="Trainer list limit reached";
        else if(trainers.entries.empty())shown="No trainers installed";
        if(!launchNotice.empty())shown=launchNotice;
        message=new tsl::elm::CategoryHeader(shown);list->addItem(message);
        for(const auto& entry:trainers.entries){
            auto version=entry.version;if(!version.empty()&&version.front()!='v')version="v"+version;
            auto* item=new tsl::elm::ListItem(entry.name,version);
            item->setClickListener([path=entry.path](u64 keys){if(!(keys&HidNpadButton_A))return false;launch(path);return true;});list->addItem(item);
        }
        if(trainers.entries.empty())list->addItem(new tsl::elm::CategoryHeader("Add .ovl files to the trainers folder"));
        list->addItem(new tsl::elm::CategoryHeader("Menu"));
        auto* settings=new tsl::elm::ListItem("Shortcut settings",shortcuts::PRESETS[shortcutSetting.selected].label);shortcutItem=settings;
        settings->setClickListener([](u64 keys){if(!(keys&HidNpadButton_A))return false;tsl::changeTo<ShortcutGui>();return true;});list->addItem(settings);
        auto* refresh=new tsl::elm::ListItem("Refresh trainers");
        refresh->setClickListener([](u64 keys){if(!(keys&HidNpadButton_A))return false;launch(shortcuts::HUB);return true;});list->addItem(refresh);
        auto* back=new tsl::elm::ListItem("Return to Ultrahand");
        back->setClickListener([](u64 keys){if(!(keys&HidNpadButton_A))return false;launch(MENU);return true;});list->addItem(back);
        frame->setContent(list);return frame;
    }
    void update()override{
        shortcutItem->setValue(shortcuts::PRESETS[shortcutSetting.selected].label);
        if(!launchNotice.empty()&&shown!=launchNotice){shown=launchNotice;message->setText(shown);}
    }
};
class LauncherOverlay:public tsl::Overlay {
public:
    void initServices()override{
        tsl::hlp::doWithSDCardHandle([&]{
            install::prepare(install::Paths{});shortcutSetting=shortcuts::initialize(shortcuts::Paths{});
        });shortcut_runtime::load();
        if(!shortcutSetting.applied)launchNotice=shortcutSetting.message;
    }
    bool onContextShortcut()override{
        const auto game=shortcut_runtime::application();
        if(!game.valid){launchNotice="No running game detected - choose manually";return false;}
        catalog::Choice choice;tsl::hlp::doWithSDCardHandle([&]{choice=catalog::choose(catalog::scan(install::Paths{}.folder),game.title);});
        if(choice.path.empty()){launchNotice=choice.reason;return false;}
        const auto again=shortcut_runtime::application();
        if(!again.valid||again.pid!=game.pid||again.title!=game.title){launchNotice="Game changed - try the shortcut again";return false;}
        return launch(choice.path);
    }
    std::unique_ptr<tsl::Gui> loadInitialGui()override{return initially<LauncherGui>();}
};
}
int main(int argc,char** argv){return tsl::loop<LauncherOverlay>(argc,argv);}
