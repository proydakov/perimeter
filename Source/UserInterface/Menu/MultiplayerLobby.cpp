
#include "StdAfx.h"
#include "Config.h"
#include "terra.h"
#include "Universe.h"
#include "GameShell.h"
#include "PerimeterShellUI.h"
#include "MainMenu.h"

#include "BGScene.h"

#include "BelligerentSelect.h"
#include "GameContent.h"
#include "MultiplayerCommon.h"

extern BGScene bgScene;

extern std::string getItemTextFromBase(const char *keyStr);

bool isEditAllowed(const MissionDescription& mission, int number) {
    if (gameShell->getNetClient()->isSaveGame()) {
        return false;
    }
    return mission.activePlayerID == mission.playersData[number].playerID
           || (gameShell->getNetClient()->isHost() && mission.playersData[number].realPlayerType == REAL_PLAYER_TYPE_AI);
}

bool showPlayerControls(const MissionDescription& mission, int number) {
    return 0 <= number && number < mission.playerAmountScenarioMax
    && (mission.gameType_ == GT_loadMPGame || mission.playersData[number].realPlayerType != REAL_PLAYER_TYPE_OPEN)
    && mission.playersData[number].realPlayerType != REAL_PLAYER_TYPE_CLOSE;
}

void fillMultiplayerLobbyList() {
    CListBoxWindow* list = (CListBoxWindow*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_MAP_LIST);
    list->NewItem(1);
    list->Clear();
    if (!gameShell->getNetClient()->isSaveGame()) {
        for (int i = 0; i < multiplayerMaps.size(); i++) {
            std::string name = "MapNames.";
            name += multiplayerMaps[i].missionName();
            name = qdTextDB::instance().getText(name.c_str());
            if (name.empty()) {
                name = multiplayerMaps[i].missionName();
            }
            list->AddString(name.c_str(), 0);
        }
    }
    if (multiplayerMaps.empty()) {
        list->SetCurSel(-1);
    } else {
        list->SetCurSel(0);
    }
}

void onMMMultiplayerGameSpeedSlider(CShellWindow* pWnd, InterfaceEventCode code, int param) {
    if ( code == EVENT_CREATEWND ) {
        CSliderWindow *pSlider = (CSliderWindow*) pWnd;
        pSlider->pos = (1.0f - 0.2f) / 4.8f;
    }
}

void onMMLobbyMapList(CShellWindow* pWnd, InterfaceEventCode code, int param) {
	if ( code == EVENT_PRESSED && intfCanHandleInput() ) {
		if (param >= 0 && param < multiplayerMaps.size()) {
			checkMissionDescription(param, multiplayerMaps, GT_createMPGame);
			std::string missionName = std::string("RESOURCE\\MULTIPLAYER\\") + multiplayerMaps[param].missionName();
			gameShell->getNetClient()->changeMap(missionName.c_str());
		}
/*
        CListBoxWindow* list = (CListBoxWindow*)pWnd; 
		int pos = list->GetCurSel();
		if (pos >= 0 && pos < multiplayerMaps.size()) {
			checkMissionDescription(pos, multiplayerMaps);
			string missionName = string("RESOURCE\\MULTIPLAYER\\") + multiplayerMaps[pos].missionName();
			gameShell->getNetClient()->changeMap(missionName.c_str());

		}
*/
	}		
}

void setFrm(CComboWindow* combo, int number) {
	const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
    if (showPlayerControls(currMission, number)) {
		combo->Show(true);
        combo->Enable(isEditAllowed(currMission, number));
        if (unavailableContentBelligerent(currMission.playersData[number].belligerent, terGameContentSelect)) {
            combo->pos = 0;
            gameShell->getNetClient()->changePlayerBelligerent(number, BELLIGERENT_EXODUS0);
        } else {
            combo->pos = getSelectableBelligerentIndex(currMission.playersData[number].belligerent);
        }
	} else {
		combo->Show(false);
	}
}

void onMMLobbyFrmButton(CShellWindow* pWnd, InterfaceEventCode code, int param) {
    setupFrameButton(pWnd, code, pWnd->ID - SQSH_MM_LOBBY_PLAYER1_FRM_BTN, true);
}

void setSlot(CComboWindow* combo, int number) {
    const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
    const PlayerData& pd = currMission.playersData[number];
    bool isSave = gameShell->getNetClient()->isSaveGame();
	if ( currMission.playerAmountScenarioMax > number ) {
		combo->Show(true);
        bool isHost = gameShell->getNetClient()->isHost();
        bool slotEnable = isHost && currMission.activePlayerID != number;
        if (slotEnable && isSave
        && pd.realPlayerType != REAL_PLAYER_TYPE_OPEN
        && pd.realPlayerType != REAL_PLAYER_TYPE_PLAYER
        && pd.realPlayerType != REAL_PLAYER_TYPE_PLAYER_AI) {
            //Only kicking clients or adding/removing AI to empty player is allowed when loading a save
            slotEnable = false;
        }
        combo->Enable(slotEnable);
		switch (pd.realPlayerType) {
            case REAL_PLAYER_TYPE_OPEN:
				combo->pos = 0;
				break;
            case REAL_PLAYER_TYPE_PLAYER:
                combo->pos = 1;
                break;
			case REAL_PLAYER_TYPE_CLOSE:
				combo->pos = 2;
				break;
			case REAL_PLAYER_TYPE_AI:
				{
					switch (pd.difficulty) {
						case DIFFICULTY_EASY:
							combo->pos = 3;
							break;
						case DIFFICULTY_NORMAL:
							combo->pos = 4;
							break;
						case DIFFICULTY_HARD:
							combo->pos = 5;
							break;
					}
				}
				break;
            case REAL_PLAYER_TYPE_PLAYER_AI:
                combo->pos = 5;
                break;
            default:
                break;
        }
//		combo->pos = currMission.playersData[number].realPlayerType;
	} else {
		combo->Show(false);
	}
}
void setupSlot(CComboWindow* combo, int number, bool direction) {
    if (gameShell->getNetClient()->isSaveGame()) {
        const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
        const PlayerData& pd = currMission.playersData[number];
        switch (pd.realPlayerType) {
            case REAL_PLAYER_TYPE_OPEN:
                gameShell->getNetClient()->changeRealPlayerType(number, REAL_PLAYER_TYPE_PLAYER_AI);
                break;
            case REAL_PLAYER_TYPE_PLAYER:
            case REAL_PLAYER_TYPE_PLAYER_AI:
                gameShell->getNetClient()->changeRealPlayerType(number, REAL_PLAYER_TYPE_OPEN);
                break;
            default:
                break;
        }
    } else {
        int newPos = combo->pos;
        if (direction) {
            newPos++;
            if (newPos >= combo->size) {
                newPos = 0;
            }
        } else {
            newPos--;
            if (newPos < 0) {
                newPos = combo->size - 1;
            }
        }
        switch (newPos) {
            default:
                break;
            case 0:
                gameShell->getNetClient()->changeRealPlayerType(number, REAL_PLAYER_TYPE_OPEN);
                break;
            case 1: //REAL_PLAYER_TYPE_PLAYER
                gameShell->getNetClient()->changeRealPlayerType(number, direction ? REAL_PLAYER_TYPE_CLOSE
                                                                                  : REAL_PLAYER_TYPE_OPEN);
                break;
            case 2:
                gameShell->getNetClient()->changeRealPlayerType(number, REAL_PLAYER_TYPE_CLOSE);
                break;
            case 3:
                gameShell->getNetClient()->changeRealPlayerType(number, REAL_PLAYER_TYPE_AI);
                gameShell->getNetClient()->changePlayerDifficulty(number, DIFFICULTY_EASY);
                break;
            case 4:
                gameShell->getNetClient()->changeRealPlayerType(number, REAL_PLAYER_TYPE_AI);
                gameShell->getNetClient()->changePlayerDifficulty(number, DIFFICULTY_NORMAL);
                break;
            case 5:
                gameShell->getNetClient()->changeRealPlayerType(number, REAL_PLAYER_TYPE_AI);
                gameShell->getNetClient()->changePlayerDifficulty(number, DIFFICULTY_HARD);
                break;
        }
    }
}
void setupSlotButton(CShellWindow* pWnd, InterfaceEventCode code, int number) {
	if( code == EVENT_CREATEWND ) {
		CComboWindow *pCombo = (CComboWindow*) pWnd;
		pCombo->Array.emplace_back( getItemTextFromBase("Open").c_str() );
		pCombo->Array.emplace_back( getItemTextFromBase("Player").c_str() );
		pCombo->Array.emplace_back( getItemTextFromBase("Closed").c_str() );
		pCombo->Array.emplace_back( getItemTextFromBase("AI (Easy)").c_str() );
		pCombo->Array.emplace_back( getItemTextFromBase("AI (Normal)").c_str() );
        pCombo->Array.emplace_back( getItemTextFromBase("AI (Hard)").c_str() );
		pCombo->size = pCombo->Array.size();
		pCombo->pos = 0;
	} else if (code == EVENT_UNPRESSED) {
		setupSlot((CComboWindow*) pWnd, number, true);
	} else if (code == EVENT_RUNPRESSED) {
		setupSlot((CComboWindow*) pWnd, number, false);
	}
}
void onMMLobbySlotButton(CShellWindow* pWnd, InterfaceEventCode code, int param) {
	setupSlotButton(pWnd, code, pWnd->ID - SQSH_MM_LOBBY_PLAYER1_SLOT_BTN);
}

void setClan(CComboWindow* combo, int number) {
    const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
    if (showPlayerControls(currMission, number)) {
		combo->Show(true);
        combo->Enable(isEditAllowed(currMission, number));
		combo->pos = currMission.playersData[number].clan;
	} else {
		combo->Show(false);
	}
}
void setupClan(CComboWindow* combo, int number, bool direction) {
	int newPos = combo->pos;
	if (direction) {
		newPos++;
		if (newPos >= combo->size) {
			newPos = 0;
		}
	} else {
		newPos--;
		if (newPos < 0) {
			newPos = combo->size - 1;
		}
	}
	gameShell->getNetClient()->changePlayerClan(number, newPos);
//	gameShell->getNetClient()->changeRealPlayerType(number, RealPlayerType(newPos));
}
void setupClanButton(CShellWindow* pWnd, InterfaceEventCode code, int number) {
	if( code == EVENT_CREATEWND ) {
		CComboWindow *pCombo = (CComboWindow*) pWnd;
		std::string clan = getItemTextFromBase("Clan");
		char buff[30 + 1];
		for (int i = 0; i < NETWORK_PLAYERS_MAX; i++) {
			sprintf(buff, "%d", (i + 1));
			pCombo->Array.push_back( (clan + buff).c_str() );
		}
		pCombo->size = NETWORK_PLAYERS_MAX;
		pCombo->pos = 0;
	} else if (code == EVENT_UNPRESSED) {
		setupClan((CComboWindow*) pWnd, number, true);
	} else if (code == EVENT_RUNPRESSED) {
		setupClan((CComboWindow*) pWnd, number, false);
	}
}

void onMMLobbyClanButton(CShellWindow* pWnd, InterfaceEventCode code, int param) {
	setupClanButton(pWnd, code, pWnd->ID - SQSH_MM_LOBBY_PLAYER1_CLAN_BTN);
}

void setHC(CComboWindow* combo, int number) {
	const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
    if (showPlayerControls(currMission, number)) {
		combo->Show(true);
        combo->Enable(isEditAllowed(currMission, number));

		int handicap = currMission.playersData[number].handicap;
		if (handicap > 70) {
			combo->pos = 0;
		} else if (handicap > 50) {
			combo->pos = 1;
		} else if (handicap > 30) {
			combo->pos = 2;
		} else {
			combo->pos = 3;
		}
	} else {
		combo->Show(false);
	}
}
void setupHC(CComboWindow* combo, int number, bool direction) {
	int newPos = combo->pos;
	if (direction) {
		newPos++;
		if (newPos >= combo->size) {
			newPos = 0;
		}
	} else {
		newPos--;
		if (newPos < 0) {
			newPos = combo->size - 1;
		}
	}
	int handicap = 100;
	switch (newPos) {
		case 0:
			handicap = 100;
			break;
		case 1:
			handicap = 70;
			break;
		case 2:
			handicap = 50;
			break;
		case 3:
			handicap = 30;
			break;
	}
	gameShell->getNetClient()->changePlayerHandicap(number, handicap);
}
void setupHCButton(CShellWindow* pWnd, InterfaceEventCode code, int number) {
	if( code == EVENT_CREATEWND ) {
		CComboWindow *pCombo = (CComboWindow*) pWnd;
		pCombo->Array.push_back( "100%" );
		pCombo->Array.push_back( "70%" );
		pCombo->Array.push_back( "50%" );
		pCombo->Array.push_back( "30%" );
		pCombo->size = 4;
		pCombo->pos = 0;
	} else if (code == EVENT_UNPRESSED) {
		setupHC((CComboWindow*) pWnd, number, true);
	} else if (code == EVENT_RUNPRESSED) {
		setupHC((CComboWindow*) pWnd, number, false);
	}
}

void onMMLobbyHCButton(CShellWindow* pWnd, InterfaceEventCode code, int param) {
	setupHCButton(pWnd, code, pWnd->ID - SQSH_MM_LOBBY_PLAYER1_HC_BTN);
}

void setClr(CColorComboWindow* combo, CShellWindow* bg, int number) {
	const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
    if (showPlayerControls(currMission, number)) {
		bg->Show(true);
		combo->Show(true);
        combo->Enable(isEditAllowed(currMission, number));
		combo->pos = currMission.playersData[number].colorIndex;
	} else {
		bg->Show(false);
		combo->Show(false);
	}
}
void setupClr(CColorComboWindow* combo, int number, bool direction) {
	int newPos = combo->pos;
	if (direction) {
		newPos++;
		if (newPos >= combo->size) {
			newPos = 0;
		}
	} else {
		newPos--;
		if (newPos < 0) {
			newPos = playerAllowedColorSize - 1;
		}
	}
	gameShell->getNetClient()->changePlayerColor(number, newPos, direction);
}
void setupClrButton(CShellWindow* pWnd, InterfaceEventCode code, int number) {
	if (code == EVENT_UNPRESSED) {
		setupClr((CColorComboWindow*) pWnd, number, true);
	} else if (code == EVENT_RUNPRESSED) {
		setupClr((CColorComboWindow*) pWnd, number, false);
	}
}
void onMMLobbyClrButton(CShellWindow* pWnd, InterfaceEventCode code, int param) {
	setupClrButton(pWnd, code, pWnd->ID - SQSH_MM_LOBBY_PLAYER1_CLR_BTN);
}

void setName(CShellPushButton* btn, int number) {
	const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
    bool isSave = currMission.gameType_ == GT_loadMPGame;
    const PlayerData& pd = currMission.playersData[number];
    if (showPlayerControls(currMission, number)
    && pd.realPlayerType != REAL_PLAYER_TYPE_AI) {
		btn->Show(true);
		btn->Enable(true);
        std::string text = pd.name();
        if (isSave) {
            if (pd.realPlayerType == REAL_PLAYER_TYPE_PLAYER) {
                text = pd.nameInitial() + std::string(" &FFFFFF(") + pd.name() + ")";
            }
        } else {
            text = std::string("&FFFFFF") + text;
        }
        btn->setText(text);
	} else {
		btn->Show(false);
	}
}
void setupName(CComboWindow* combo, int number) {
    gameShell->getNetClient()->changePlayerSeat(number);
}
void setupNameButton(CShellWindow* pWnd, InterfaceEventCode code, int number) {
	if (code == EVENT_UNPRESSED) {
		setupName((CComboWindow*) pWnd, number);
	}
}

void onMMLobbyNameButton(CShellWindow* pWnd, InterfaceEventCode code, int param) {
	setupNameButton(pWnd, code, pWnd->ID - SQSH_MM_LOBBY_PLAYER1_NAME_BTN);
}

void setReady(CShellWindow* wnd, int number) {
	const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
    bool showReady = currMission.playerAmountScenarioMax > number;
    if (showReady
    && currMission.playersData[number].realPlayerType != REAL_PLAYER_TYPE_AI
    && currMission.playersData[number].realPlayerType != REAL_PLAYER_TYPE_PLAYER_AI) {
        showReady = currMission.playersData[number].flag_playerStartReady;
    }
	wnd->Show(showReady);
}

void setLobbyMapListVisible(bool visible) {
	_shellIconManager.GetWnd(SQSH_MM_LOBBY_MAP_LIST)->Show(visible);
	_shellIconManager.GetWnd(SQSH_MM_LOBBY_MAP_LIST_RAMKA1)->Show(visible);
	_shellIconManager.GetWnd(SQSH_MM_LOBBY_MAP_LIST_RAMKA2)->Show(visible);
	_shellIconManager.GetWnd(SQSH_MM_LOBBY_MAP_LIST_RAMKA3)->Show(visible);
	_shellIconManager.GetWnd(SQSH_MM_LOBBY_MAP_LIST_RAMKA4)->Show(visible);
}

void onMMLobbyGameNameButton(CShellWindow* pWnd, InterfaceEventCode code, int param) {
	if (code == EVENT_DRAWWND) {
        CPushButton* btn = reinterpret_cast<CPushButton*>(pWnd);
        btn->setText(gameShell->getNetClient()->m_GameName);
        
		const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
//		((CPushButton*)pWnd)->setText(currMission.missionDescription());

		if (gameShell->getNetClient()->isHost() && !gameShell->getNetClient()->isSaveGame()) {
			setLobbyMapListVisible(true);
			_shellIconManager.GetWnd(SQSH_MM_LOBBY_GAME_MAP)->Show(0);
			((CShowMapWindow*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_HOST_GAME_MAP))->setWorldID( currMission.worldID() );
			int pos = getMultiplayerMapNumber(currMission.missionName());
			if (pos != -1) {
				((CListBoxWindow*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_MAP_LIST))->SetCurSelPassive(pos);
			}
		} else {
			setLobbyMapListVisible(false);
			((CShowMapWindow*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_GAME_MAP))->setWorldID( currMission.worldID() );
			_shellIconManager.GetWnd(SQSH_MM_LOBBY_HOST_GAME_MAP)->Show(0);
		}
		((CTextWindow*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_GAME_MAP_DESCR_TXT))->setText( currMission.missionDescription() );
	
		for (int i = 0; i < NETWORK_PLAYERS_MAX; i++) {
			setFrm(((CComboWindow*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_PLAYER1_FRM_BTN + i)), i);
			setClr(((CColorComboWindow*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_PLAYER1_CLR_BTN + i)), _shellIconManager.GetWnd(SQSH_MM_LOBBY_PLAYER1_CLR_BG + i), i);
			setSlot(((CComboWindow*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_PLAYER1_SLOT_BTN + i)), i);
			setName(((CShellPushButton*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_PLAYER1_NAME_BTN + i)), i);
			setClan(((CComboWindow*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_PLAYER1_CLAN_BTN + i)), i);
			setHC(((CComboWindow*)_shellIconManager.GetWnd(SQSH_MM_LOBBY_PLAYER1_HC_BTN + i)), i);
			setReady(_shellIconManager.GetWnd(SQSH_MM_LOBBY_PLAYER1_READY_BTN + i), i);
		}
	}
}

void onMMLobbyStartButton(CShellWindow* pWnd, InterfaceEventCode code, int param) {
	if (code == EVENT_DRAWWND){
        const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
        bool enable = currMission.playersAmount() > 1;
        //Lock host enable until all slots are closed
        if (gameShell->getNetClient()->isHost() && currMission.gameType_ == GT_loadMPGame) {
            for (int i = 0; i < currMission.playerAmountScenarioMax; ++i) {
                if (currMission.playersData[i].realPlayerType == REAL_PLAYER_TYPE_OPEN) {
                    enable = false;
                    break;
                }
            }
        }
		pWnd->Enable(enable);
	} else if (code == EVENT_UNPRESSED){
        const MissionDescription& currMission = gameShell->getNetClient()->getLobbyMissionDescription();
        bool state = !currMission.playersData[currMission.activePlayerID].flag_playerStartReady;
		gameShell->getNetClient()->StartLoadTheGame(state);
        //Set button state
        CPushButton* btn = reinterpret_cast<CPushButton*>(pWnd);
        btn->setText(qdTextDB::instance().getText(state ? "Interface.Menu.ButtonLabels.CANCEL" : "Interface.Menu.ButtonLabels.READY"));
	}
}
void onMMLobbyBackButton(CShellWindow* pWnd, InterfaceEventCode code, int param) {
	if( code == EVENT_UNPRESSED && intfCanHandleInput() ) {
		if (gameShell->isStartedWithMainmenu()) {
			_shellIconManager.SwitchMenuScreens( pWnd->m_pParent->ID, SQSH_MM_MULTIPLAYER_LIST_SCR );
		} else {
			gameShell->GameContinue = 0;
		}
	}
}
