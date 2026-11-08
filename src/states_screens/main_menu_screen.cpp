//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#define DEBUG_MENU_ITEM 0

#include "states_screens/main_menu_screen.hpp"

#include "addons/news_manager.hpp"
#include "challenges/story_mode_timer.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/dialog_queue.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/keyboard_device.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "main_loop.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/demo_world.hpp"
#include "modes/overworld.hpp"
#include "modes/tutorial_utils.hpp"
#include "network/network_config.hpp"
#include "online/request_manager.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/cutscene_general.hpp"
#include "states_screens/grand_prix_editor_screen.hpp"
#include "states_screens/help/help_screen_1.hpp"
#include "states_screens/high_score_selection.hpp"
#include "states_screens/offline_kart_selection.hpp"
#include "states_screens/online/online_profile_achievements.hpp"
#include "states_screens/online/online_profile_servers.hpp"
#include "states_screens/online/online_screen.hpp"
#include "states_screens/options/options_screen_audio.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#if DEBUG_MENU_ITEM
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/grand_prix_lose.hpp"
#include "states_screens/grand_prix_win.hpp"
#endif
#include "states_screens/dialogs/message_dialog.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "main_loop.hpp"

#include <string>

#include <IrrlichtDevice.h>

#ifdef ANDROID
#include <SDL_system.h>
#endif

using namespace GUIEngine;
using namespace Online;


#ifdef ANDROID
extern bool Android_isHardwareKeyboardConnected();
extern std::string Android_getTrackPackInfo();
extern void Android_requestTrackPack();
#endif

namespace
{
    /** Turns internet access on straight from the "no internet" message and
     *  continues to the screen the player wanted, instead of sending them to
     *  the options first. */
    class EnableInternetListener : public MessageDialog::IConfirmDialogListener
    {
        bool m_online;
    public:
        EnableInternetListener(bool online) : m_online(online) {}
        virtual void onConfirm()
        {
            GUIEngine::ModalDialog::dismiss();
            UserConfigParams::m_internet_status = RequestManager::IPERM_ALLOWED;
            if (!RequestManager::isRunning())
                RequestManager::get()->startNetworkThread();
            NewsManager::get()->init(false);
            user_config->saveConfig();
            if (m_online)
                OnlineScreen::getInstance()->push();
            else
                AddonsScreen::getInstance()->push();
        }
        virtual void onCancel() { GUIEngine::ModalDialog::dismiss(); }
    };   // EnableInternetListener

#ifdef ANDROID
    class TrackPackListener : public MessageDialog::IConfirmDialogListener
    {
        GUIEngine::Widget* m_widget;
        std::string        m_name;
    public:
        TrackPackListener(GUIEngine::Widget* widget, const std::string& name)
                        : m_widget(widget), m_name(name) {}
        virtual void onConfirm()
        {
            GUIEngine::ModalDialog::dismiss();
            // The game restarts after the download, keep all progress
            user_config->saveConfig();
            PlayerManager::get()->save();
            Android_requestTrackPack();
        }
        virtual void onCancel()
        {
            GUIEngine::ModalDialog::dismiss();
            // Continue to the screen the player wanted
            MainMenuScreen::getInstance()->eventCallback(m_widget, m_name,
                                                         PLAYER_ID_GAME_MASTER);
        }
    };   // TrackPackListener
#endif
}   // namespace

// ----------------------------------------------------------------------------

MainMenuScreen::MainMenuScreen() : Screen("main_menu.stkgui")
{
}   // MainMenuScreen

// ----------------------------------------------------------------------------

void MainMenuScreen::loadedFromFile()
{
    LabelWidget* w = getWidget<LabelWidget>("info_addons");
    w->setScrollSpeed(0.5f);
    
    RibbonWidget* rw_top = getWidget<RibbonWidget>("menu_toprow");
    assert(rw_top != NULL);
    (void)rw_top;
    
    if (track_manager->getTrack("overworld") == NULL ||
        track_manager->getTrack("introcutscene") == NULL ||
        track_manager->getTrack("introcutscene2") == NULL)
    {
        getWidget<RibbonWidget>("menu_secondrow")->removeChildNamed("story");
    }

#if DEBUG_MENU_ITEM != 1
    RibbonWidget* rw = getWidget<RibbonWidget>("menu_bottomrow");
    rw->removeChildNamed("test_gpwin");
    rw->removeChildNamed("test_gplose");
    rw->removeChildNamed("test_unlocked");
    rw->removeChildNamed("test_unlocked2");
    rw->removeChildNamed("test_intro");
    rw->removeChildNamed("test_outro");
#endif
}   // loadedFromFile

// ----------------------------------------------------------------------------

void MainMenuScreen::beforeAddingWidget()
{
#ifdef IOS_STK
    // iOS app doesn't like quit button in UI
    Widget* w = getWidget("quit");
    if (w)
        w->setVisible(false);
#endif

#ifdef ANDROID
    if (SDL_IsAndroidTV())
    {
        Widget* tutorial = getWidget("startTutorial");
        if (tutorial)
            tutorial->setVisible(false);
    }
#endif
}

// ----------------------------------------------------------------------------
//
void MainMenuScreen::init()
{
    Screen::init();


    // reset in case we're coming back from a race
    NetworkConfig::get()->cleanNetworkPlayers();
    StateManager::get()->resetActivePlayers();
    input_manager->getDeviceManager()->setAssignMode(NO_ASSIGN);
    input_manager->getDeviceManager()->setSinglePlayer( NULL );
    input_manager->setMasterPlayerOnly(false);

    // Avoid incorrect behaviour in certain race circumstances:
    // If a multi-player game is played with two keyboards, the 2nd
    // player selects his kart last, and only the keyboard is used
    // to select all other settings - then if the next time the kart
    // selection screen comes up, the default device will still be
    // the 2nd player. So if the first player presses 'select', it
    // will instead add a second player (so basically the key
    // binding for the second player become the default, so pressing
    // select will add a new player). See bug 3090931
    // To avoid this, we will clean the last used device, making
    // the key bindings for the first player the default again.
    input_manager->getDeviceManager()->clearLatestUsedDevice();

#ifndef SERVER_ONLY
    IconButtonWidget* addons_button = getWidget<IconButtonWidget>("addons");
    if (addons_button && addons_manager && addons_manager->isLoading())
    {
        IconButtonWidget* w = addons_button;
        w->setActive(false);
        w->resetAllBadges();
        w->setBadge(LOADING_BADGE);
    }

    // Initialize news iteration, show dialog when there's important news
    NewsManager::get()->resetNewsPtr(NewsManager::NTYPE_MAINMENU);

    core::stringw important_message = L"";
    // The news come from the servers of the upstream project and are about
    // that project, so they are not shown in this game
    int news_len = 0;
    int chosen_id = -1;

    // Iterate through every news
    // Find the unread important message with smallest id
    while (news_len--)
    {
        int id = NewsManager::get()->getNextNewsID(NewsManager::NTYPE_MAINMENU);

        if (NewsManager::get()->isCurrentNewsImportant(NewsManager::NTYPE_MAINMENU)
            && (id < chosen_id || chosen_id == -1)
            && id > UserConfigParams::m_last_important_message_id)
        {
            chosen_id = id;
            important_message =
                NewsManager::get()->getCurrentNewsMessage(NewsManager::NTYPE_MAINMENU);
        }
    }
    if (chosen_id != -1)
    {
        UserConfigParams::m_last_important_message_id = chosen_id;
        new MessageDialog(important_message,
                        MessageDialog::MESSAGE_DIALOG_OK,
                        NULL, true);
    }   // if important_message

    // Back to the first news
    NewsManager::get()->getNextNewsID(NewsManager::NTYPE_MAINMENU);

    // Check if there's new news

    IconButtonWidget* online_icon = getWidget<IconButtonWidget>("online");
    // No "new news" badge, the news list is not shown in this game
    if (online_icon != NULL)
        online_icon->resetAllBadges();

    m_news_text = L"";
    LabelWidget* w = getWidget<LabelWidget>("info_addons");
    w->setText(m_news_text, true);
    w->update(0.01f);
#endif

    RibbonWidget* r = getWidget<RibbonWidget>("menu_bottomrow");
    // FIXME: why do I need to do this manually
    ((IconButtonWidget*)r->getChildren().get(0))->unfocused(PLAYER_ID_GAME_MASTER, NULL);
    ((IconButtonWidget*)r->getChildren().get(1))->unfocused(PLAYER_ID_GAME_MASTER, NULL);
    ((IconButtonWidget*)r->getChildren().get(2))->unfocused(PLAYER_ID_GAME_MASTER, NULL);

    r = getWidget<RibbonWidget>("menu_toprow");
    // Start on the race tile, it is the main action of this screen
    r->select("new", PLAYER_ID_GAME_MASTER);
    r->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    DemoWorld::resetIdleTime();

#ifdef IOS_STK
    // iOS app doesn't like quit button in UI
    Widget* quit = getWidget("quit");
    if (quit)
        quit->setVisible(false);
#endif
}   // init

// ----------------------------------------------------------------------------

void MainMenuScreen::onUpdate(float delta)
{
#ifndef SERVER_ONLY
    NewsManager::get()->joinDownloadThreadIfExit();

    IconButtonWidget* addons_icon = getWidget<IconButtonWidget>("addons");
    if (addons_icon != NULL)
    {
        if (addons_manager->wasError())
        {
            addons_icon->setActive(true);
            addons_icon->resetAllBadges();
            addons_icon->setBadge(BAD_BADGE);
        }
        else if (addons_manager->isLoading() && UserConfigParams::m_internet_status
            == Online::RequestManager::IPERM_ALLOWED)
        {
            // Addons manager is still initialising/downloading.
            addons_icon->setActive(false);
            addons_icon->resetAllBadges();
            addons_icon->setBadge(LOADING_BADGE);
        }
        else
        {
            addons_icon->setActive(true);
            addons_icon->resetAllBadges();
            if (addons_manager->hasNewAddons())
                addons_icon->setBadge(DOWN_BADGE);
        }
        // maybe add a new badge when not allowed to access the net
    }

    LabelWidget* w = getWidget<LabelWidget>("info_addons");
    
    if (w->getText().empty() || w->scrolledOff())
    {
        // Show important messages seperately
        // Concatrate adjacent unimportant messages together
        m_news_text = L"";
        
        // No upstream news in this game, see init()
        int news_count = 0;

        while (news_count--)
        {
            bool important = NewsManager::get()->isCurrentNewsImportant(NewsManager::NTYPE_MAINMENU);
            if (!m_news_text.empty())
            {
                m_news_text += "  +++  ";
            }
            m_news_text += NewsManager::get()->getCurrentNewsMessage(NewsManager::NTYPE_MAINMENU);

            NewsManager::get()->getNextNewsID(NewsManager::NTYPE_MAINMENU);

            if (important || NewsManager::get()->isCurrentNewsImportant(NewsManager::NTYPE_MAINMENU))
            {
                break;
            }
        }

        w->setText(m_news_text, true);
    }
    w->update(delta);

    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    if (!player)
        return;
    // Ask if user want to play tutorial when profile is newly created
    if (player->getUseFrequency() != 0)
        return;

#ifdef ANDROID
    // Don't show tutorial dialog on Android TV
    if (SDL_IsAndroidTV())
        return;
#endif

    player->incrementUseFrequency();
    class PlayTutorial :
          public MessageDialog::IConfirmDialogListener
    {
    public:
        virtual void onConfirm()
        {
            GUIEngine::ModalDialog::dismiss();
            TutorialUtils::startTutorial();
        }   // onConfirm
    };   // PlayTutorial

    MessageDialog* dialog =
    new MessageDialog(_("Would you like to play the tutorial of the game?"),
        MessageDialog::MESSAGE_DIALOG_YESNO, new PlayTutorial(),
        true/*delete_listener*/, true/*from_queue*/);
    GUIEngine::DialogQueue::get()->pushDialog(dialog,
        false/*closes_any_dialog*/);
#endif
}   // onUpdate

// ----------------------------------------------------------------------------

void MainMenuScreen::eventCallback(Widget* widget, const std::string& name,
                                   const int playerID)
{
#ifndef SERVER_ONLY
    // most interesting stuff is in the ribbons, so start there
    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);

    if (ribbon == NULL) return; // what's that event??

    // ---- A ribbon icon was clicked
    std::string selection =
        ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

#ifdef ANDROID
    // Ask once, before the first singleplayer or story mode game, if more
    // tracks can be downloaded from Google Play
    if ((selection == "new" || selection == "story") &&
        !UserConfigParams::m_track_pack_offered)
    {
        UserConfigParams::m_track_pack_offered = true;
        // Android can close the game without a normal exit, save it now
        user_config->saveConfig();
        const std::string info = Android_getTrackPackInfo();
        std::vector<std::string> parts = StringUtils::split(info, ' ');
        if (parts.size() == 2)
        {
            new MessageDialog(_("Get %s more tracks? They are a free %s MB "
                "download from Google Play.", parts[0].c_str(),
                parts[1].c_str()),
                MessageDialog::MESSAGE_DIALOG_YESNO,
                new TrackPackListener(widget, name), true);
            return;
        }
    }
#endif

    /*
    if (selection == "story")
    {
        StateManager::get()->enterGameState();
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        RaceManager::get()->setNumKarts( 0 );
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->startSingleRace("endcutscene", 999, false);

        std::vector<std::string> parts;
        parts.push_back("introcutscene");
        parts.push_back("introcutscene2");
        ((CutsceneWorld*)World::getWorld())->setParts(parts);
        //RaceManager::get()->startSingleRace("introcutscene2", 999, false);
        return;
    }
    */

#if DEBUG_MENU_ITEM
    if (selection == "test_gpwin")
    {
        StoryModeStatus* sms = PlayerManager::getCurrentPlayer()->getStoryModeStatus();
        sms->unlockFeature(const_cast<ChallengeStatus*>(sms->getChallengeStatus("gp1")),
            RaceManager::DIFFICULTY_HARD);

        StateManager::get()->enterGameState();
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        RaceManager::get()->setNumKarts(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->startSingleRace("gpwin", 999, false);
        GrandPrixWin* scene = GrandPrixWin::getInstance();
        scene->push();
        const std::pair<std::string, float> winners[] =
            {
                { "kiki", 0.6f },
                { "nolok", 1.0f },
                { "pidgin", 0.0f },
            };
        scene->setKarts(winners);
    }
    else if (selection == "test_gplose")
    {
        StateManager::get()->enterGameState();
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        RaceManager::get()->setNumKarts(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->startSingleRace("gplose", 999, false);
        GrandPrixLose* scene = GrandPrixLose::getInstance();
        scene->push();
        std::vector<std::pair<std::string, float> > losers;
        losers.emplace_back("nolok", 1.0f);
        losers.emplace_back("kiki", 0.6f);
        //losers.push_back("wilber");
        //losers.push_back("tux");
        scene->setKarts(losers);
    }
    else if (selection == "test_unlocked" || selection == "test_unlocked2")
    {
        StoryModeStatus* sms = PlayerManager::getCurrentPlayer()->getStoryModeStatus();
        sms->unlockFeature(const_cast<ChallengeStatus*>(sms->getChallengeStatus("gp1")),
            RaceManager::DIFFICULTY_HARD);

        StateManager::get()->enterGameState();
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        RaceManager::get()->setNumKarts(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->startSingleRace("featunlocked", 999, false);

        FeatureUnlockedCutScene* scene =
            FeatureUnlockedCutScene::getInstance();

        std::vector<std::string> parts;
        parts.push_back("featunlocked");
        ((CutsceneWorld*)World::getWorld())->setParts(parts);

        scene->addTrophy(RaceManager::DIFFICULTY_EASY, false);

        if (selection == "test_unlocked")
        {
            scene->addUnlockedKart(kart_properties_manager->getKart("tux"));
            scene->addUnlockedTrack(track_manager->getTrack("lighthouse"));
            scene->push();
        }
        else if (selection == "test_unlocked2")
        {
            std::vector<video::ITexture*> textures;
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("lighthouse")
                             ->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("snowtuxpeak")
                             ->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("sandtrack")
                             ->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("snowmountain")
                             ->getScreenshotFile().c_str()));

            scene->addUnlockedPictures(textures, 4.0, 3.0, L"You unlocked <actual text would go here...>");

            scene->push();
        }
    }
    else if (selection == "test_intro")
    {
        CutsceneWorld::setUseDuration(true);
        StateManager::get()->enterGameState();
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        RaceManager::get()->setNumKarts(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->startSingleRace("introcutscene", 999, false);

        std::vector<std::string> parts;
        parts.push_back("introcutscene");
        parts.push_back("introcutscene2");
        ((CutsceneWorld*)World::getWorld())->setParts(parts);
        //RaceManager::get()->startSingleRace("introcutscene2", 999, false);
        
        CutSceneGeneral* scene = CutSceneGeneral::getInstance();
        scene->push();
        return;
    }
    else if (selection == "test_outro")
    {
        CutsceneWorld::setUseDuration(true);
        StateManager::get()->enterGameState();
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        RaceManager::get()->setNumKarts(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->startSingleRace("endcutscene", 999, false);

        std::vector<std::string> parts;
        parts.push_back("endcutscene");
        ((CutsceneWorld*)World::getWorld())->setParts(parts);
        
        CutSceneGeneral* scene = CutSceneGeneral::getInstance();
        scene->push();
    }
    else
#endif
    if (selection == "new")
    {
        NetworkConfig::get()->unsetNetworking();
        KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance();
        s->setMultiplayer(false);
        s->setFromOverworld(false);
        s->push();
    }
    else if (selection == "multiplayer")
    {
#ifdef MOBILE_STK
        // The touch screen only drives one kart, every other player needs
        // a gamepad or keyboard. Say so instead of opening a screen where
        // nobody can join.
        bool has_keyboard = false;
#ifdef ANDROID
        has_keyboard = Android_isHardwareKeyboardConnected();
#endif
        if (input_manager->getDeviceManager()->getGamePadAmount() == 0 &&
            !has_keyboard)
        {
            new MessageDialog(_("Splitscreen multiplayer needs a gamepad or "
                "keyboard for every extra player. Connect one and try again."));
            return;
        }
#endif
        KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance();
        NetworkConfig::get()->unsetNetworking();
        s->setMultiplayer(true);
        s->setFromOverworld(false);
        s->push();
    }
    else if (selection == "players")
    {
        UserScreen::getInstance()->push();
    }
    else if (selection == "options")
    {
        OptionsScreenAudio::getInstance()->push();
    }
    else if (selection == "quit")
    {
        // Ask first, like the back key does
        onEscapePressed();
        return;
    }
    else if (selection == "about")
    {
        CreditsScreen::getInstance()->push();
    }
    else if (selection == "help")
    {
        HelpScreen1::getInstance()->push();
    }
    else if (selection == "startTutorial")
    {
        TutorialUtils::startTutorial();
    }
    else if (selection == "story")
    {
        NetworkConfig::get()->unsetNetworking();
        PlayerProfile *player = PlayerManager::getCurrentPlayer();

        // Start the story mode (and speedrun) timer
        story_mode_timer->startTimer();

        if (player->isFirstTime())
        {
            CutsceneWorld::setUseDuration(true);
            StateManager::get()->enterGameState();
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
            RaceManager::get()->setNumKarts( 0 );
            RaceManager::get()->setNumPlayers(0);
            RaceManager::get()->startSingleRace("introcutscene", 999, false);

            std::vector<std::string> parts;
            parts.push_back("introcutscene");
            parts.push_back("introcutscene2");
            ((CutsceneWorld*)World::getWorld())->setParts(parts);
            //RaceManager::get()->startSingleRace("introcutscene2", 999, false);
            
            CutSceneGeneral* scene = CutSceneGeneral::getInstance();
            scene->push();
            return;
        }
        else
        {
            // Unpause the story mode timer when entering back the story mode
            story_mode_timer->unpauseTimer(/* exit loading pause */ false);

            const std::string default_kart = UserConfigParams::m_default_kart;
            if (player->isLocked(default_kart))
            {
                KartSelectionScreen *next = OfflineKartSelectionScreen::getInstance();
                next->setGoToOverworldNext();
                next->setMultiplayer(false);
                next->push();
                return;
            }
            OverWorld::enterOverWorld();
        }
    }
    else if (selection == "online")
    {
        if(UserConfigParams::m_internet_status!=RequestManager::IPERM_ALLOWED)
        {
            new MessageDialog(_("Playing online needs internet access. "
                                "Turn on internet access now?"),
                              MessageDialog::MESSAGE_DIALOG_YESNO,
                              new EnableInternetListener(true), true);
            return;
        }
        OnlineScreen::getInstance()->push();
    }
    else if (selection == "addons")
    {
        // Don't go to addons if there is no internet, unless some addons are
        // already installed (so that you can delete addons without being online).
        if(UserConfigParams::m_internet_status!=RequestManager::IPERM_ALLOWED)
        {
            if (!addons_manager->anyAddonsInstalled())
            {
                new MessageDialog(_("Downloading addons needs internet "
                                    "access. Turn on internet access now?"),
                                  MessageDialog::MESSAGE_DIALOG_YESNO,
                                  new EnableInternetListener(false), true);
                return;
            }
            else
            {
                AddonsScreen::getInstance()->push();
                new MessageDialog(_("You can not download addons without internet access. "
                                    "If you want to download addons, go in the options menu, "
                                    "and check \"Connect to the Internet\".\n\n"
                                    "You can however delete already downloaded addons."));
                return;
            }
        }
        AddonsScreen::getInstance()->push();
    }
    else if (selection == "gpEditor")
    {
        GrandPrixEditorScreen::getInstance()->push();
    }
    else if (selection == "achievements")
    {
        OnlineProfileAchievements::getInstance()->push();
    }
    else if (selection == "highscores")
    {
        HighScoreSelection::getInstance()->push();
    }
#endif
}   // eventCallback

// ----------------------------------------------------------------------------

void MainMenuScreen::tearDown()
{
}   // tearDown

// ----------------------------------------------------------------------------

void MainMenuScreen::onDisabledItemClicked(const std::string& item)
{
#ifndef SERVER_ONLY
    if (item == "addons")
    {
        if (UserConfigParams::m_internet_status != RequestManager::IPERM_ALLOWED)
        {
            new MessageDialog( _("The add-ons module is currently disabled in "
                                 "the Options screen") );
        }
        else if (addons_manager->wasError())
        {
            new MessageDialog( _("Sorry, an error occurred while contacting "
                                 "the add-ons website. Make sure you are "
                                 "connected to the Internet and that "
                                 "SuperTuxKart is not blocked by a firewall"));
        }
        else if (addons_manager->isLoading())
        {
            new MessageDialog( _("Please wait while the add-ons are loading"));
        }
    }
#endif
}   // onDisabledItemClicked

// ----------------------------------------------------------------------------

bool MainMenuScreen::onEscapePressed()
{
    class ConfirmClose :
          public MessageDialog::IConfirmDialogListener
    {
    public:
        virtual void onConfirm()
        {
            GUIEngine::ModalDialog::dismiss();
            main_loop->abort();
        }   // onConfirm
    };   // ConfirmClose

    new MessageDialog(_("Are you sure you want to quit?"),
        MessageDialog::MESSAGE_DIALOG_YESNO, new ConfirmClose(),
        true/*delete_listener*/);
    return false;
}   // onEscapePressed
