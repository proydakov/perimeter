#ifndef __P2P_INTERFACE_H__
#define __P2P_INTERFACE_H__

#include "NetIncludes.h"
#include "ConnectionDP.h"
#include "EventBufferDP.h"
#include "CommonEvents.h"
#include "ServerList.h"

#ifdef PERIMETER_DEBUG
#define LogMsg(...) fprintf(stdout, __VA_ARGS__)
#else
#define LogMsg(...)
#endif

#define _DBG_COMMAND_LIST

// {DF006380-BF70-4397-9A18-51133CEEE3B6}

int InternalServerThread(void* lpParameter);

struct sGameStatusInfo{
    uint8_t maximumPlayers = 0;
    uint8_t currrentPlayers = 0;
    bool flag_gameRun = false;
    int ping = 0;
    int worldID = 0;

	sGameStatusInfo() = default;
    
	sGameStatusInfo(uint8_t _maxPlayers, uint8_t _curPlayers, bool _flag_gameRun, int _ping, int _worldID) {
		set(_maxPlayers, _curPlayers, _flag_gameRun, _ping, _worldID);
	}

	void set(uint8_t _maxPlayers, uint8_t _curPlayers, bool _flag_gameRun, int _ping, int _worldID){
		maximumPlayers=_maxPlayers;
		currrentPlayers=_curPlayers;
		flag_gameRun=_flag_gameRun;
		ping=_ping;
		worldID=_worldID;
	}
};

struct sGameHostInfo {
	GameHostConnection gameHost;
	std::string hostName;
	std::string gameName;
	//bool flag_gameRun;
	sGameStatusInfo gameStatusInfo;
	//void* pData = nullptr;
    
	sGameHostInfo(const GameHostConnection& _gameHost, const char * _hostName, const char * _gameName, const sGameStatusInfo& gsi, void* _pData=nullptr) {
		set(_gameHost, _hostName, _gameName, gsi, _pData);
	}
	void set(const GameHostConnection& _gameHost, const char * _hostName, const char * _gameName, const sGameStatusInfo& gsi, void* _pData=nullptr){
        gameHost=_gameHost;
        hostName=_hostName;
        gameName=_gameName;
        gameStatusInfo=gsi;
        //pData=_pData;
	}
};

#define PERIMETER_IP_PORT_DEFAULT 12987
#define PERIMETER_IP_HOST_DEFAULT "127.0.0.1:12987"


struct PClientData
{
//	PNetCenter* m_pGame;
//	CRITICAL_SECTION  m_csLock;

	char   m_szDescription[PERIMETER_CLIENT_DESCR_SIZE];
	///HANDLE m_hReady;
	bool m_flag_Ready;
	unsigned int clientGameCRC;
	NETID  netidPlayer;

	//OutputEventBuffer out_buffer;
	//InputEventBuffer  in_buffer;
	bool requestPause;
	bool clientPause;
	unsigned int timeRequestPause;

	std::list<netCommand4H_BackGameInformation*> backGameInfList;
	unsigned int lagQuant;
	unsigned int lastExecuteQuant;

	std::vector<netCommand4H_BackGameInformation2> backGameInf2List;
	unsigned int curLastQuant;

	unsigned int lastTimeBackPacket;

	unsigned int missionDescriptionIdx;
	unsigned int confirmQuant;

	PClientData(unsigned int mdIdx, NETID netid, const char* descr="simple client");
	~PClientData();

};

enum e_PPStatus{
	PLAYER,
	AI
};
struct PPlayerData
{
	e_PPStatus	status;
	PClientData* player;

	PPlayerData(e_PPStatus s){
		status = s; player = 0;
	}
};

enum e_PNCInterfaceCommands {
	PNC_INTERFACE_COMMAND_NONE,
	PNC_INTERFACE_COMMAND_CONNECTION_FAILED,
	PNC_INTERFACE_COMMAND_CONNECTION_DROPPED,
	PNC_INTERFACE_COMMAND_INFO_PLAYER_EXIT,
	PNC_INTERFACE_COMMAND_INFO_PLAYER_DISCONNECTED,

	PNC_INTERFACE_COMMAND_CONNECT_OK,
	PNC_INTERFACE_COMMAND_CONNECT_ERR_INCORRECT_VERSION,
	PNC_INTERFACE_COMMAND_CONNECT_ERR_GAME_STARTED,
	PNC_INTERFACE_COMMAND_CONNECT_ERR_GAME_FULL,
	PNC_INTERFACE_COMMAND_CONNECT_ERR_PASSWORD,
	PNC_INTERFACE_COMMAND_CONNECT_ERR,

	PNC_INTERFACE_COMMAND_HOST_TERMINATED_GAME,
	PNC_INTERFACE_COMMAND_CRITICAL_ERROR_GAME_TERMINATED

};
struct sPNCInterfaceCommand {
	e_PNCInterfaceCommands icID;
	std::string textInfo;

	sPNCInterfaceCommand(){
		icID=PNC_INTERFACE_COMMAND_NONE;
	}
	sPNCInterfaceCommand(e_PNCInterfaceCommands _icID, const char* str=0){
		icID=_icID;
		if(str) textInfo=str;
	}
	sPNCInterfaceCommand(const sPNCInterfaceCommand& donor){
		icID=donor.icID;
		textInfo=donor.textInfo;
	}
};

enum e_PNCInternalCommand{
	PNC_COMMAND__START_HOST_AND_CREATE_GAME_AND_STOP_FIND_HOST,
	PNC_COMMAND__STOP_HOST_AND_ABORT_GAME_AND_START_FIND_HOST,

	//PNCC_START_FIND_HOST,
	PNC_COMMAND__CONNECT_2_HOST_AND_STOP_FIND_HOST,
	PNC_COMMAND__DISCONNECT_AND_ABORT_GAME_AND_END_START_FIND_HOST,
	PNC_COMMAND__DISCONNECT_AND_ABORT_GAME_AND_END,

	//Client back commands
	PNC_COMMAND__CLIENT_STARTING_LOAD_GAME,
	PNC_COMMAND__CLIENT_STARTING_GAME,

	//Special command
	PNC_COMMAND__STOP_GAME_AND_ASSIGN_HOST_2_MY,
	PNC_COMMAND__STOP_GAME_AND_WAIT_ASSIGN_OTHER_HOST,

	PNC_COMMAND__END,
	PNC_COMMAND__ABORT_PROGRAM,
	
	PNC_COMMAND__END_GAME,
	PNC_COMMAND__START_FIND_HOST

};

#define PNC_State_Host (0x80)
#define PNC_State_GameRun (0x40)
enum e_PNCState{
	PNC_STATE__NONE=0,

	PNC_STATE__CLIENT_FIND_HOST=1,

	PNC_STATE__CONNECTION=2,
	//PNC_STATE__INFINITE_CONNECTION_2_IP=3,
	PNC_STATE__CLIENT_TUNING_GAME=4,
	PNC_STATE__CLIENT_LOADING_GAME=PNC_State_GameRun|5,
	PNC_STATE__CLIENT_GAME=PNC_State_GameRun|6,

	PNC_STATE__CLIENT_RESTORE_GAME_AFTE_CHANGE_HOST_PHASE_0=PNC_State_GameRun|7,
	PNC_STATE__CLIENT_RESTORE_GAME_AFTE_CHANGE_HOST_PHASE_AB=PNC_State_GameRun|8,

	//special state

	PNC_STATE__END=9,

	PNC_STATE__NET_CENTER_CRITICAL_ERROR=10,

	PNC_STATE__HOST_TUNING_GAME=						PNC_State_Host	|11,
	PNC_STATE__HOST_LOADING_GAME=	PNC_State_GameRun	|PNC_State_Host	|12,
	PNC_STATE__HOST_GAME=			PNC_State_GameRun	|PNC_State_Host|13,

	PNC_STATE__NEWHOST_PHASE_0=PNC_State_GameRun|PNC_State_Host|14,
	PNC_STATE__NEWHOST_PHASE_A=PNC_State_GameRun|PNC_State_Host|15,
	PNC_STATE__NEWHOST_PHASE_B=PNC_State_GameRun|PNC_State_Host|16,

	// Состояние завершения
	PNC_STATE__ENDING_GAME=17
};

enum e_PNCStateClient{
	PNC_STATE_CLIENT__NONE=0,

	PNC_STATE_CLIENT__FIND_HOST=1,


	PNC_STATE_CLIENT__TUNING_GAME=2,
	PNC_STATE_CLIENT__LOADING_GAME=3,
	PNC_STATE_CLIENT__GAME=4,

	PNC_STATE_CLIENT__RESTORE_GAME_AFTE_CHANGE_HOST,
	//special state

	PNC_STATE_CLIENT__END,

	PNC_STATE__CLIENT_NET_CENTER_CRITICAL_ERROR
};

enum e_PNCStateHost{
	PNC_STATE_HOST_NONE=0,


	PNC_STATE_HOST__END,

	PNC_STATE_HOST__NET_CENTER_CRITICAL_ERROR,

	PNC_STATE_HOST__TUNING_GAME=PNC_State_Host|2,
	PNC_STATE_HOST__LOADING_GAME=PNC_State_Host|3,
	PNC_STATE_HOST__GAME=PNC_State_Host|4,

	PNC_STATE_NEWHOST__WAIT_ALL_CLIENT_GAME_STATUS=PNC_State_Host|5,
	PNC_STATE_NEWHOST__WAIT_GAME_DATA=PNC_State_Host|6
};

class PNetCenter {
private:
    ServerList serverList;

public:
	std::list<e_PNCInternalCommand> internalCommandList;
	HANDLE hSecondThreadInitComplete;
	HANDLE hCommandExecuted;
	std::list<sPNCInterfaceCommand> interfaceCommandList;

	//typedef hash_map<NETID, PClientData*> ClientMapType;
	typedef std::vector<PClientData*> ClientMapType;

	e_PNCState m_state;
	const char* getStrState();


	e_PNCStateClient m_clientState;
	e_PNCStateClient m_previsionClientState;
	e_PNCStateHost m_hostState;
	e_PNCStateHost m_previsionHostState;
	void setClientStateAndStorePrevisionState(e_PNCStateClient state){
		m_previsionClientState=m_clientState;
		m_clientState=state;
	}
	void restorePrevisionClientState(){
		m_clientState=m_previsionClientState;
	}

	void setHostStateAndStorePrevisionState(e_PNCStateHost state){
		m_previsionHostState=m_hostState;
		m_hostState=state;
	}
	void restorePrevisionHostState(){
		m_hostState=m_previsionHostState;
	}

	unsigned char m_amountPlayersInHost; //info only
	unsigned char m_amountPlayerInDP;  //info only

	//int m_quantPeriod;
	unsigned long m_nextQuantTime;

    //Use server listing to get servers or broadcast server to listing
    bool publicServerHost;

	//bool flag_missionDescriptionUpdate;
	MissionDescription hostMissionDescription;

	MissionDescription clientMissionDescription; //Only 1


	//for command PNCC_START_HOST_AND_CREATE_GAME_AND_STOP_FIND_HOST
	PlayerData internalPlayerData;

	//string m_missionName;
	
	ClientMapType  m_clients;
	//NETID          m_netidGroupGame; //Этот NETID используется для отправки сообщений всей группе

	//int            m_nServerPause;
	int            m_nQuantCommandCounter;

	//int            m_nWorldID;
	std::string    m_GameName;
	bool           m_bStarted;

	std::list<netCommandGeneral*>   m_CommandList;
	std::list<netCommand4G_ForcedDefeat*> m_DeletePlayerCommand;

	void ClearCommandList();
	void ClearDeletePlayerGameCommand();


	///HANDLE             m_hPlayerListReady;
	//bool m_flag_PlayerListReady;
	//list<PPlayerData>  m_PlayerStartList;


	PNetCenter();
	~PNetCenter();

	void DisconnectAndStartFindHost(void);
	void StopServerAndStartFindHost(void);


	void UpdateBattleData();
	void UpdateCurrentMissionDescription4C();
	void CheckClients();
	void DumpClients();
	void WaitForAllClientsReady(int ms);
	void ResetAllClients();

//	void QuantTimeProc();
	//Host !!!
	void SendEvent(netCommandGeneral& event, NETID netid, bool flag_guaranted=1);
	void PutGameCommand2Queue_andAutoDelete(netCommandGame* pCommand);


///	void SetGameSpeedScale(float scale, NETID netidFrom);

	void Start();
///	void Quant();
	int AddClient(PlayerData& pd, const NETID netid, const char* descr);
	void ClearClients();

	void setNETIDInClientsDate(const int missionDescriptionIdx, NETID netid);
	void DeleteClientByMissionDescriptionIdx(const int missionDescriptionIdx);
	void DeleteClientByNETID(const NETID netid, DWORD dwReason);

	//void StartGame();
	void LLogicQuant();

    SDL_mutex* m_GeneralLock;

	//Game(first) Thread
	InOutNetComBuffer  in_ClientBuf;
	InOutNetComBuffer  out_ClientBuf;
	bool flag_SkipProcessingGameCommand;//Нужно при миграции Host-а

	InOutNetComBuffer  in_HostBuf;
	InOutNetComBuffer  out_HostBuf;
	void clearInOutClientHostBuffers();

	bool ExecuteInternalCommand(e_PNCInternalCommand ic, bool waitExecution);
	bool ExecuteInterfaceCommand(e_PNCInterfaceCommands ic, const char* str=0);

	void CreateGame(const GameHostConnection& connection, const std::string& gameName, const std::string& missionName, const std::string& playerName, const std::string& password="");

	void JoinGame(const GameHostConnection& connection, const std::string& playerName, const std::string& password="");

	void refreshLanGameHostList();
	std::vector<sGameHostInfo*>& getGameHostList();

	void SendEvent(const netCommandGeneral* event);
	void SendEventSync(const netCommandGeneral* event);



	void ServerTimeControl(float scale);
	void changePlayerBelligerent(int idxPlayerData, terBelligerent newBelligerent);
	void changePlayerColor(int idxPlayerData, int newColorIndex);
	void changeRealPlayerType(int idxPlayerData, RealPlayerType newRealPlayerType);
	void changePlayerDifficulty(int idxPlayerData, Difficulty difficulty);
	void changePlayerClan(int idxPlayerData, int clan);
	void changePlayerHandicap(int idxPlayerData, int handicap);
	void changeMap(const char* missionName);


	bool StartLoadTheGame(void);
	void GameIsReady(void);


	void P2PIQuant();
	void HandlerInputNetCommand();

	//NETID m_playerNETID; //Информация на клиенте присланный его NETID


	HANDLE hSecondThread;
	//Second Thread
	bool SecondThread(void);
	void ClearClientData(void){
		ClientMapType::iterator p;
		for(p=m_clients.begin(); p!=m_clients.end(); p++){
			delete *p;
		}
		m_clients.clear();
	}
	bool AddClientToMigratedHost(const NETID _netid, unsigned int _curLastQuant, unsigned int _confirmQuant);


	MissionDescription curMD; //1t
	MissionDescription& getCurrentMissionDescription(void){
		return curMD;
	};
	unsigned int m_numberGameQuant; //Кванты на хосте Кванты считаются с 1-цы!
	void HostReceiveQuant();
	void ClientPredReceiveQuant();


	unsigned int m_quantInterval;

	unsigned int m_beginWaitTime;

	unsigned int lastTimeServerPacket;

	unsigned int TIMEOUT_CLIENT_OR_SERVER_RECEIVE_INFORMATION;
	unsigned int TIMEOUT_DISCONNECT;
	unsigned int MAX_TIME_PAUSE_GAME;

	bool isHost(void){
		if(m_state&PNC_State_Host) return 1;
		else return 0;
	}
	bool isGameRun(void){
		if(m_state&PNC_State_GameRun) return 1;
		else return 0;
	}

    static bool resolveHostAddress(GameHostConnection& address, const std::string& host);

    NETID	m_hostNETID;
    NETID	m_localNETID;
    bool flag_connected;

	void FinishGame(void);
	void StartFindHost(void);

	//DP interface
	bool Init(void);
	int ServerStart(const char* _name, int port);
	void SetConnectionTimeout(int ms);
	void RemovePlayer(NETID netid);
    
	void Close(bool flag_immediatle=1);
	int Connect(const GameHostConnection& host);

	bool isConnected();
	int Send(const char* buffer, int size, NETID netid, bool flag_guaranted=1);

	std::vector<sGameHostInfo*> gameHostList;
	void clearGameHostList(void){
		std::vector<sGameHostInfo*>::iterator p;
		for(p=gameHostList.begin(); p!=gameHostList.end(); p++){
			delete *p;
		}
		gameHostList.erase(gameHostList.begin(), gameHostList.end());
	}


	unsigned int flag_LockIputPacket;
	void LockInputPacket(void);
	void UnLockInputPacket(void);

	std::list<XDPacket> m_DPPacketList;
	bool PutInputPacket2NetBuffer(InOutNetComBuffer& netBuf);
	bool PutInputPacket2NetBuffer(InOutNetComBuffer& netBuf, NETID& returnNETID);

    GameHostConnection hostConnection;

	//Host Date
	unsigned long hostGeneralCommandCounter;
	unsigned int quantConfirmation;

	NETID netidClientWhichWeWait; //netid игрока которому хост при миграции посылает команду прислать игровые комманды; нужен чтобы в случае выхода переслать комманду другому

	//Host info //TODO originally for gamespy, we should use this for public listed hosts in future
	const char* getMissionName();
	const char* getGameName();
	const char* getGameVer();
	int getNumPlayers();
	int getMaxPlayers();
	int getHostPort();
	enum e_perimeterGameState{
		PGS_OPEN_WAITING,
		PGS_CLOSE_WAITING,
		PGS_CLOSED_PLAYING
	};
	e_perimeterGameState getGameState(void){
		switch(m_state){
		case PNC_STATE__HOST_TUNING_GAME:
			return PGS_OPEN_WAITING;
		case PNC_STATE__HOST_GAME:
			return PGS_CLOSED_PLAYING;
		default:
			return PGS_CLOSED_PLAYING;
		}
	}
	std::string gamePassword;
    
	bool hasPassword(void){
		return !gamePassword.empty();
	}

	//Network settings
	bool flag_NetworkSimulation;
	bool flag_HostMigrate;
	bool flag_NoUseDPNSVR;
	int m_DPSigningLevel;//0-none 1-fast signed 2-full signed

	//Pause client
	bool setPause(bool pause);
	bool isPause();

	bool hostPause;
	bool clientPause;
	bool clientInPacketPause;

	//Chat
	void chatMessage(int userID, const char* str);
};



#endif //__P2P_INTERFACE_H__