#pragma once
#include "stdafx.h"
#include "DateTime.h"
#include "List.h"
#include "Winsock.h"
#include "Item.h"
#include "Monster.h"
#include "MapsHandler.h"
#include "PathFinder.h"

class Client
{
	class TASKS
	{
	public:
		TASKS(Client* Client)
		{
			this->Client = Client;
		}
	public:
		void OnlineStatus();
		void HeroInformation();
	protected:
		Client* Client;
	};
public:
	Client(void);
	~Client(void);
	typedef void(Client::*FUNCTION)();
	void Disconnect();
	void AddMonster(Monster* Mon);
	void RemoveMonster(Monster* Mon);
	void Move(BYTE Dir);
	void Move(BYTE Dir, POINT* Point);
	void HandleTask(char* Buffer, int BufferLength);
	void HandleClientPacket(char* Buffer);
	void HandleServerPacket(char* Buffer);
	void ProcessCommand(string Message);
	void ProcessOutgoing();
	void ProcessIncoming();
	void ProcessDisconnection();
	void LoadMap();
	void SetupPortals();
	void FindStartPoint();
	void SniffPacket(string Source, char* Buffer, int Length);
	void SendToServer(char* Buffer, int Length);
	void SendTask(char* Buffer, int Length);
	void Jump(int X, int Y);
	void Walk(int X, int Y);
	void PathFind(int ToX, int ToY);
	void TeleportBasedOnQuests(USHORT TX, USHORT TY, int QID = 0);
	void Attack(Client* Client, int TargetUID, int AttackType);
	void PickItem(Client* Client, Item* Item);
	void DropItem(int UID, USHORT Index);
	void UsePortal(Client* Client, int ToX, int ToY);
	void MagicAttack(Client* Client, int TargetUID, int TX, int TY, int SkillID);
	int PathDirection[8][2];
	int Portals[504][6];
	std::string Name;
	bool BreakThreads;
	bool CanMelee;
	bool CanShift;
	bool AbleToHunt;
	bool FarJumpReached;
	bool Disconnecting;
	bool Disconnected;
	bool Dead;
	bool FinishedLogin;
	bool CanPathFind;
	bool ServerResponded;
	bool CanUseXP;
	bool JumpPacketSent;
	bool ServerConfirmedJump;
	bool ServerConfirmedWalk;
	bool ServerConfirmedAttack;
	bool ServerConfirmedSpawn;
	bool ClientConfirmedJump;
	bool Sniff;
	int ProccessID;
	int UID;
	int SubUID;
	int DropIndex;
	int LastAttackedMonster;
	int X;
	int ServerX;
	int Y;
	int ServerY;
	int LastX;
	int LastY;
	int SubX; 
	int SubY;
	int ProcessID; 
	int Map; 
	int MainMap; 
	int Count; 
	int XP; 
	int KillsCounter;
	int DataSendTraceSleepTime;
	int hsock;
	long Status1, Status2;
	USHORT ItemsDropIndex;
	int DropItemsTimeStamp;
	BYTE Job;
	BYTE Direction;
	POINT* StartPoint;
	POINT* EndPoint;
	POINT* TracePoint;
	DateTime* LastItemsDrop;
	DateTime* LastJump;
	DateTime* LastMapUpdate;
	DateTime* Lastfatle;
	DateTime* LastSuperMan;
	DateTime* LastXPStart;
	DateTime* LastFatleStrikeUse;
	DateTime* LastXP;
	DateTime* LastXPSkillCheck;
	Dictionary<int, Item*> LocalItems;
	Dictionary<int, Item*> ItemsToDrop;
	Dictionary<int, Monster*> AllMonsters;
	Dictionary<int, POINT*> AllMonstersCoords;
	MapsHandler::Portal* LastPortal;
	PathFinder* PathEngine;
	POINT* GetRandomCoords(POINT* Point);
	POINT* GetNextJump(POINT* StartPoint);
	TASKS* Tasks;
	List<int> OKItems;
	List<int> ForbidItems;
	Winsock* Socket;
	List<char*> ClientData;
	static int OutgoingPacketLenght;
	static int OutgoingPacketAddress; 
	static int IncomingPacketLenght; 
	static int IncomingPacketAddress;
	char* SendPacketBuffer;
	char* ReceivePacketBuffer;
	DWORD JumpTimeStamp;
	DWORD MagicAttackTimeStamp;
	DWORD AttackTimeStamp;
	DWORD WalkTimeStamp;
	static DWORD ReceivePacketRetAddr;
	static DWORD DisconnectRetAddr;
	DWORD DisconnectAddr;
	DWORD AccountServerSocket;
	DWORD GameServerSocket;


public: 
	static void WINAPI HuntThread(Client* C);
	static void WINAPI Receive(Client* Client);
	static void ReceivePacketFunction(void);
	static void SendPacketFunction(void);
	static void DisconnectFunction(void);
	static int WINAPI DetouredConnect(SOCKET s, const sockaddr *name, int len);
	static int (WINAPI *OriginalConnect)(SOCKET s, const sockaddr *name, int len);
	static int (WINAPI *pSend)(SOCKET s, const char* buf, int len, int flags);
	static int WINAPI MySend(SOCKET s, const char* buf, int len, int flags);
	static BOOL (WINAPI *Originalisdebuggerpresent)(VOID);
	static BOOL WINAPI Detouredisdebuggerpresent (VOID);
	static int WINAPI DetouredDisConnect(SOCKET s);
	static int (WINAPI *OriginalDisConnect)(SOCKET s);
	static Client* Owner;

};

