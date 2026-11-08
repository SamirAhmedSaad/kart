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

#include "states_screens/race_setup_screen.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/arenas_screen.hpp"
#include "states_screens/easter_egg_screen.hpp"
#include "states_screens/ghost_replay_selection.hpp"
#include "states_screens/soccer_setup_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_and_gp_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

const int CONFIG_CODE_NORMAL    = 0;
const int CONFIG_CODE_TIMETRIAL = 1;
const int CONFIG_CODE_FTL       = 2;
const int CONFIG_CODE_3STRIKES  = 3;
const int CONFIG_CODE_EASTER    = 4;
const int CONFIG_CODE_SOCCER    = 5;
const int CONFIG_CODE_GHOST     = 6;
const int CONFIG_CODE_LAP_TRIAL = 7;

using namespace GUIEngine;

// -----------------------------------------------------------------------------

RaceSetupScreen::RaceSetupScreen() : Screen("race_setup.stkgui")
{
}   // RaceSetupScreen

// -----------------------------------------------------------------------------

void RaceSetupScreen::loadedFromFile()
{
}   // loadedFromFile

// -----------------------------------------------------------------------------

void RaceSetupScreen::init()
{
    Screen::init();
    input_manager->setMasterPlayerOnly(true);
    RibbonWidget* w = getWidget<RibbonWidget>("difficulty");
    assert( w != NULL );

    RaceManager::get()->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);
    if (UserConfigParams::m_difficulty == RaceManager::DIFFICULTY_BEST &&
        PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
    {
        w->setSelection(RaceManager::DIFFICULTY_HARD, PLAYER_ID_GAME_MASTER);
    }
    else
    {
        w->setSelection( UserConfigParams::m_difficulty, PLAYER_ID_GAME_MASTER );
    }

    DynamicRibbonWidget* w2 = getWidget<DynamicRibbonWidget>("gamemode");
    assert( w2 != NULL );
    w2->clearItems();

    // ---- Add game modes: short names under the icons, the description of
    // the selected one goes into the mode_info label
    m_mode_descriptions.clear();
    w2->addItem(RaceManager::getNameOf(RaceManager::MINOR_MODE_NORMAL_RACE), IDENT_STD,
                RaceManager::getIconOf(RaceManager::MINOR_MODE_NORMAL_RACE));
    //FIXME: avoid duplicating descriptions from the help menu!
    m_mode_descriptions[IDENT_STD] =
        _("All blows allowed, so catch weapons and make clever use of them!");

    w2->addItem(RaceManager::getNameOf(RaceManager::MINOR_MODE_TIME_TRIAL), IDENT_TTRIAL,
                RaceManager::getIconOf(RaceManager::MINOR_MODE_TIME_TRIAL));
    //FIXME: avoid duplicating descriptions from the help menu!
    m_mode_descriptions[IDENT_TTRIAL] =
        _("Contains no powerups, so only your driving skills matter!");

    if (PlayerManager::getCurrentPlayer()->isLocked(IDENT_FTL))
    {
        w2->addItem(RaceManager::getNameOf(RaceManager::MINOR_MODE_FOLLOW_LEADER),
            "locked", RaceManager::getIconOf(RaceManager::MINOR_MODE_FOLLOW_LEADER), true);
        m_mode_descriptions["locked"] =
            _("Locked : solve active challenges to gain access to more!");
    }
    else
    {
        w2->addItem(RaceManager::getNameOf(RaceManager::MINOR_MODE_FOLLOW_LEADER), IDENT_FTL,
                    RaceManager::getIconOf(RaceManager::MINOR_MODE_FOLLOW_LEADER), false);
        //I18N: short definition for follow-the-leader game mode
        m_mode_descriptions[IDENT_FTL] =
            _("Keep up with the leader kart but don't overtake it!");
    }

    w2->addItem(_("Battle"), IDENT_STRIKES, RaceManager::getIconOf(RaceManager::MINOR_MODE_FREE_FOR_ALL));
    //FIXME: avoid duplicating descriptions from the help menu!
    m_mode_descriptions[IDENT_STRIKES] =
        _("Hit others with weapons until they lose all their lives.");

    // Soccer, egg hunt, ghost replay and lap trial are not offered, this
    // game keeps to racing and battling against the computer

    w2->updateItemDisplay();

    // restore saved game mode
    switch (UserConfigParams::m_game_mode)
    {
    case CONFIG_CODE_NORMAL :
        w2->setSelection(IDENT_STD, PLAYER_ID_GAME_MASTER, true);
        break;
    case CONFIG_CODE_TIMETRIAL :
        w2->setSelection(IDENT_TTRIAL, PLAYER_ID_GAME_MASTER, true);
        break;
    case CONFIG_CODE_FTL :
        w2->setSelection(IDENT_FTL, PLAYER_ID_GAME_MASTER, true);
        break;
    case CONFIG_CODE_3STRIKES :
        w2->setSelection(IDENT_STRIKES, PLAYER_ID_GAME_MASTER, true);
        break;
    default:
        // A mode which is not offered anymore was used last
        w2->setSelection(IDENT_STD, PLAYER_ID_GAME_MASTER, true);
        break;
    }

    w2->setItemCountHint(4);
    m_last_mode = w2->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    updateModeInfo();

    {
        RibbonWidget* w = getWidget<RibbonWidget>("difficulty");
        assert(w != NULL);

        int index = w->findItemNamed("best");
        Widget* hardestWidget = &w->getChildren()[index];

        if (PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
        {
            hardestWidget->setBadge(LOCKED_BADGE);
            hardestWidget->setActive(false);
        }
        else
        {
            hardestWidget->unsetBadge(LOCKED_BADGE);
            hardestWidget->setActive(true);
        }
    }
}   // init

// -----------------------------------------------------------------------------
void RaceSetupScreen::eventCallback(Widget* widget, const std::string& name,
                                    const int playerID)
{
    if (name == "difficulty")
    {
        assignDifficulty();
    }
    else if (name == "gamemode")
    {
        // The first tap picks a mode, tapping it again or Next starts it
        DynamicRibbonWidget* w = dynamic_cast<DynamicRibbonWidget*>(widget);
        const std::string selected = w->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        const bool again = selected == m_last_mode;
        m_last_mode = selected;
        updateModeInfo();
        if (again)
            startSelectedMode();
    }
    else if (name == "next_bar")
    {
        startSelectedMode();
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }

}   // eventCallback

// -----------------------------------------------------------------------------
/** Shows the description of the selected game mode under the mode icons. */
void RaceSetupScreen::updateModeInfo()
{
    std::map<std::string, irr::core::stringw>::const_iterator it =
        m_mode_descriptions.find(m_last_mode);
    getWidget<LabelWidget>("mode_info")->setText(
        it == m_mode_descriptions.end() ? L"" : it->second, false);
}   // updateModeInfo

// -----------------------------------------------------------------------------
/** Continues to the track or arena selection for the selected game mode. */
void RaceSetupScreen::startSelectedMode()
{
    {
        assignDifficulty();

        DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("gamemode");
        const std::string selectedMode = w->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selectedMode == IDENT_STD)
        {
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_NORMAL_RACE);
            UserConfigParams::m_game_mode = CONFIG_CODE_NORMAL;
            TracksAndGPScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_TTRIAL)
        {
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
            UserConfigParams::m_game_mode = CONFIG_CODE_TIMETRIAL;
            TracksAndGPScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_FTL)
        {
            // Make sure there are at least three karts, otherwise FTL doesn't
            if(RaceManager::get()->getNumberOfKarts()<3)
                RaceManager::get()->setNumKarts(3);

            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_FOLLOW_LEADER);
            UserConfigParams::m_game_mode = CONFIG_CODE_FTL;
            TracksAndGPScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_STRIKES)
        {
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_3_STRIKES);
            UserConfigParams::m_game_mode = CONFIG_CODE_3STRIKES;
            ArenasScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_EASTER)
        {
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_EASTER_EGG);
            UserConfigParams::m_game_mode = CONFIG_CODE_EASTER;
            RaceManager::get()->setNumKarts( RaceManager::get()->getNumLocalPlayers() ); // no AI karts;
            EasterEggScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_SOCCER)
        {
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_SOCCER);
            UserConfigParams::m_game_mode = CONFIG_CODE_SOCCER;
            SoccerSetupScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_GHOST)
        {
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
            UserConfigParams::m_game_mode = CONFIG_CODE_GHOST;
            GhostReplaySelection::getInstance()->push();
        }
        else if (selectedMode == IDENT_LAP_TRIAL)
        {
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_LAP_TRIAL);
            UserConfigParams::m_game_mode = CONFIG_CODE_LAP_TRIAL;
            TracksAndGPScreen::getInstance()->push();
        }
        else if (selectedMode == "locked")
        {
            unlock_manager->playLockSound();
        }
    }
}   // startSelectedMode

// -----------------------------------------------------------------------------
/** Converts the difficulty string into a RaceManager::Difficulty value
 *  and sets this difficulty in the user config and in the race manager.
 */
void RaceSetupScreen::assignDifficulty()
{
    RibbonWidget* difficulty_widget = getWidget<RibbonWidget>("difficulty");
    assert(difficulty_widget != NULL);
    const std::string& difficulty =
        difficulty_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    
    RaceManager::Difficulty diff = RaceManager::convertDifficulty(difficulty);
    UserConfigParams::m_difficulty = diff;
    RaceManager::get()->setDifficulty(diff);
}   // assignDifficulty

// -----------------------------------------------------------------------------
