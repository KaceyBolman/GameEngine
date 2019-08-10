#include "stdafx.h"
#include "Client.h"

Client* Client::Owner;

Client::Client(void)
{
	Socket = new Winsock();
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HuntThread, (void*)this, 0, NULL);
	Owner = this;

	DropIndex = 0;

	PathDirection[0][0] = 0;
	PathDirection[0][0] = 0;
	PathDirection[0][1] = 1;
	PathDirection[1][0] = -1;
	PathDirection[1][1] = 1;
	PathDirection[2][0] = -1;
	PathDirection[2][1] = 0;
	PathDirection[3][0] = -1;
	PathDirection[3][1] = -1;
	PathDirection[4][0] = 0;
	PathDirection[4][1] = -1;
	PathDirection[5][0] = 1;
	PathDirection[5][1] = -1;
	PathDirection[6][0] = 1;
	PathDirection[6][1] = 0;
	PathDirection[7][0] = 1;
	PathDirection[7][1] = 1;

	ServerResponded = true;
	BreakThreads = false;
	CanMelee = false;
	CanShift = false;
	AbleToHunt = false;
	FarJumpReached = false;
	Disconnecting = false;
	Disconnected = false;
	Dead = false;
	Sniff = false;
	FinishedLogin = false;
	CanPathFind = false;
	ServerResponded = true;
	CanUseXP = false;
	JumpPacketSent = false;
	ClientConfirmedJump = true;
	ServerConfirmedJump = true;
	ServerConfirmedWalk = true;
	ServerConfirmedAttack = true;
	ServerConfirmedSpawn = true;
	int ItemsToPick[] =  { 1088001, 1088000, 721537 /*SkyTokesn*/, 729396 /*FlawlessPack*/,729904 /*CloudSaint`sLet*/,729910 /*CPMiniBag*/,729911 /*CPBag*/,729912 /*CPBackpack*/, 721538 /*earthTokesn*/, 721539 /*SkyTokesn*/, 723340, /*EXPBallScrap*/720172,/*ThunderGem*/ 700101,/*TortoiseGem700071,*//*GloryGem*/700121,/*VioletGem*/ 700051,/*PhoenixGem 700001,*//*DragonGem 700011,*//*FuryGem700021,*//*RainbowGem 700031,*//*KylinGem700041,*//*MoonGem*/700061,/*BoundCPPack*/ 727380, /*Gold*/ 1090020,  /*GoldBullion*/1091000, /*GoldBar*/1091010, /*GoldBars*/1091020, /*SmallJoyStone*/720173  };
	int ItemsToLeave[] =  { 711204, 711205, 711206, 711207, 711208, 711209, 711210, 711211, 723098, 723099, 723100, 723101, 723094, 723095, 723096, 723097, 723102, 723103, 723104, 723105, 711215, 711216, 711217, 711218, 711219, 711220, 711221, 711222,/*Legendrefinary pack*/723693 };
	OKItems.PushBack(ItemsToPick); 
	ForbidItems.PushBack(ItemsToLeave); 

	LastXP = new DateTime();
	LastItemsDrop = new DateTime();
	LastJump = new DateTime();
	LastMapUpdate = new DateTime();
	Lastfatle = new DateTime();
	LastSuperMan = new DateTime();
	LastXPStart = new DateTime();
	LastFatleStrikeUse = new DateTime();
	LastXP = new DateTime();
	LastXPSkillCheck = new DateTime();

	Tasks = new Client::TASKS(this);

	PathEngine = new PathFinder();
}

Client::~Client(void)
{
	BreakThreads = true;
}

int Client::OutgoingPacketLenght;
int Client::OutgoingPacketAddress;
int Client::IncomingPacketLenght;
int Client::IncomingPacketAddress;
DWORD Client::ReceivePacketRetAddr;
DWORD Client::DisconnectRetAddr;
bool Bypassincoming = false;
bool BypassOutgoing = false;
DWORD GameS = 0;

int (WINAPI *Client::pSend)(SOCKET s, const char* buf, int len, int flags) = send;
int (WINAPI *Client::OriginalDisConnect)(SOCKET s) = closesocket;
int (WINAPI *Client::OriginalConnect)(SOCKET s, const sockaddr *name, int len) = connect;
BOOL (WINAPI *Client::Originalisdebuggerpresent)(VOID) = IsDebuggerPresent;

int WINAPI Client::MySend(SOCKET s, const char* buf, int len, int flags)
{
	return pSend(s, buf, len, flags);
}
int WINAPI Client::DetouredConnect(SOCKET s, const sockaddr *name, int len)
{
	sockaddr_in *addr = (sockaddr_in*)name;
	u_short Port = ntohs(addr->sin_port);
	char szPort[32];
	sprintf_s(szPort, "%d", Port);

	if (Port == 9960 || Port == 9959)
	{
		if (!Owner->Socket->SocketConnected)
		{
			Owner->Socket->InitializeSocket();
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Owner->Receive, (void*)Owner, 0, NULL);
		}
	}
	else if(Port == 5816)
	{
		__asm
		{
			PUSH EAX
				MOV EAX,DWORD PTR SS:[EBP+ 0x8]
			MOV GameS, EAX
				POP EAX
		}
		/*cout << inet_ntoa(addr->sin_addr) << endl;
		addr->sin_addr.s_addr = inet_addr("5.10.163.130");
		addr->sin_port = htons(Port);
		return OriginalConnect(s, (const sockaddr*)addr, len); */
	}
	return OriginalConnect(s, name, len); 
}
int WINAPI Client::DetouredDisConnect(SOCKET s)
{
	DWORD Socket = 0;
	__asm
	{
		PUSH EAX
			MOV EAX,DWORD PTR SS:[EBP+ 0x8]
		MOV Socket, EAX
			POP EAX
	}
	if (Socket > 0 && Socket == GameS)
	{
		Owner->Disconnect();
	}
	return OriginalDisConnect(s); 
}
BOOL WINAPI Client::Detouredisdebuggerpresent(VOID)
{
	return false; 
}

Naked void Client::SendPacketFunction(void)
{
	__asm
	{
		MOV OutgoingPacketLenght,ECX
			PUSH ECX
			MOV OutgoingPacketAddress,EAX
			PUSH EAX
	}
	Owner->ProcessOutgoing();
	if (BypassOutgoing)
	{
		__asm
		{
			POP OutgoingPacketLenght
				POP OutgoingPacketLenght
				ret 
		}
	}
	__asm
	{
		MOV ECX, BotProperties::NetworkClass
			CALL BotProperties::SendPacketAddress
			ret 
	}
}

Naked void Client::ReceivePacketFunction(void)
{
	__asm
	{
		pop ReceivePacketRetAddr
			CALL BotProperties::ReceivePacketAddress
			MOV IncomingPacketLenght,EAX
			PUSHAD
			PUSHFD
			TEST IncomingPacketLenght,EAX
			JNZ ProcessPacket
			JMP _ProcessBotPackets
	}
ProcessPacket:
	__asm
	{
		MOV EAX, DWORD PTR SS:[EBP- 0x14]
		MOV IncomingPacketLenght,EAX
			MOV EAX, DWORD PTR SS:[EBP- 0x1C]
		MOV IncomingPacketAddress,EAX
	}
	Owner->ProcessIncoming();
	if (Bypassincoming)
	{
		__asm
		{
			POPFD
				POPAD
				push ReceivePacketRetAddr
				MOV EAX, 0
				ret
		}
	}
	else
	{
		__asm
		{
			POPFD
				POPAD
				push ReceivePacketRetAddr
				ret
		}
	}
_ProcessBotPackets:
	if (Owner->ClientData.Count() > 0)
	{
		char* Buffer = Owner->ClientData[0];
		Owner->ClientData.Remove(0);
		int Address = 0, TLenth = ToUshort(Buffer, 0);
		__asm
		{
			MOV EAX, DWORD PTR SS:[EBP- 0x1C]
			MOV Address, EAX
		}
		memcpy((void*)Address, (void*)Buffer, TLenth);
		__asm
		{
			POPFD
				POPAD
				MOV EAX, TLenth
				MOV DWORD PTR SS:[EBP- 0x14], EAX
				MOV EAX, 1
				push ReceivePacketRetAddr
				ret
		}
	}
	else
	{
		__asm
		{
			POPFD
				POPAD
				push ReceivePacketRetAddr
				ret
		}
	}
}

Naked void Client::DisconnectFunction(void)
{
	__asm
	{
		pop DisconnectRetAddr

			PUSHAD
			PUSHFD
	}
	Owner->ProcessDisconnection();
	__asm
	{
		POPFD
			POPAD
			PUSH 0x100
			push DisconnectRetAddr
			ret
	}
}

void WINAPI Client::HuntThread(Client* Client)
{
	int MonstersAttacked = 0;
	DateTime* LastPathFinding = new DateTime();
	do
	{
		if (Client->FinishedLogin && !Client->Disconnected && !Client->Dead && Client->LastXP->ElapsedSeconds(3))
		{
			if (Client->XP <= 101)
			{
				Client->XP++;
			}
			else if (Client->XP > 99)
			{
				Client->XP = 0;
				Client->CanUseXP = true;
			}
			Client->LastXP->Now();
		}
		if (Client->FinishedLogin && !Client->Disconnected && Client->CanUseXP)
		{
			if (Client->Job >= 20 && Client->Job <= 25)
			{
				if (DateTime::NowTime() > Client->LastXPSkillCheck->Time &&  Client->LastSuperMan->ElapsedSeconds(45))
				{
					Client->MagicAttack(Client, Client->UID, Client->X, Client->Y, 1025);// superman
					Client->LastXPSkillCheck->AddMilliseconds(1500);
				}
			}
			else if (Client->Job >= 50 && Client->Job <= 55)
			{
				if (DateTime::NowTime() > Client->LastXPSkillCheck->Time &&  Client->Lastfatle->ElapsedSeconds(45))
				{
					Client->MagicAttack(Client, Client->UID, Client->X, Client->Y, 6011);// fatal 
					Client->LastXPSkillCheck->AddMilliseconds(800);
				}
			}
		}
		DWORD SleepTime = 60; BYTE Range = 30; Item* TItem = NULL; Monster* Target = NULL;
		if (Client->FinishedLogin && Client->CanMelee)
		{
			if (Client->CanShift)
			{
				if (Client->Lastfatle->ElapsedSeconds(59)) { Client->CanShift = false; }
AgainA:
				Range = 17;
				Target = NULL;
				for each( pair<int, Monster*> Monster in  Client->AllMonsters.SafeDictionary())
				{
					if (max(abs(Client->X - Monster.second->X), abs(Client->Y - Monster.second->Y)) > 18)
					{
						Client->AllMonsters.Remove(Monster.first);
					}
					else if (max(abs(Client->X - Monster.second->X), abs(Client->Y - Monster.second->Y)) <= Range)
					{
						Range = max(abs(Client->X - Monster.second->X), abs(Client->Y - Monster.second->Y));
						Target = Monster.second;
					}
				}
				if (Target != NULL)
				{
					Client->Attack(Client, Target->UID, 2);
					Client->AllMonsters.Remove(Target->UID);
					Client->X = Target->X; Client->Y = Target->Y;
					Sleep(40);
					goto AgainA;
				}
				Sleep(300);
Again:
				Range = 60;
				TItem = NULL;
				for each( pair<int, Item*> I in  Client->LocalItems.SafeDictionary())
				{
					if (max(abs(Client->X - I.second->X), abs(Client->Y - I.second->Y)) < Range)
					{
						Range = max(abs(Client->X - I.second->X), abs(Client->Y - I.second->Y));
						TItem = I.second;
					}
				}
				if (TItem != NULL)
				{
					if (TItem->RedFlagTimes > 3) 
					{
						Client->LocalItems.Remove(TItem->UID);
						goto Again;
					}
					if (max(abs(Client->X - TItem->X), abs(Client->Y - TItem->Y)) > 10)
					{
						POINT* P = Client->GetNextJump(MakePoint(TItem->X, TItem->Y));
						if (P != NULL)
						{
							Client->Jump(P->x, P->y); TItem->RedFlagTimes++;
							Sleep(max(abs(Client->X - P->x), abs(Client->Y - P->y)) * 70);
						}
					}
					else if (max(abs(Client->X - TItem->X), abs(Client->Y - TItem->Y)) > 0)
					{
						Client->JumpPacketSent = false; 
						Client->Jump(TItem->X, TItem->Y); Sleep(25);
						if (Client->JumpPacketSent)
						{
							Sleep(max(abs(Client->X - TItem->X), abs(Client->Y - TItem->Y)) * 11);
							Client->PickItem(Client, TItem);
							Client->LocalItems.Remove(TItem->UID);
							TItem->RedFlagTimes++;
						}
					}
					else
					{
						Client->PickItem(Client, TItem);
						Client->LocalItems.Remove(TItem->UID);
						TItem->RedFlagTimes++;
					}
					Sleep(100);
					goto Again;
				}
				else
				{
					if (max(abs(Client->X - Client->StartPoint->x), abs(Client->Y - Client->StartPoint->y)) < 16)
					{
						Client->TracePoint = Client->EndPoint;
					}
					else if (max(abs(Client->X - Client->EndPoint->x), abs(Client->Y - Client->EndPoint->y)) < 16)
					{
						Client->TracePoint = Client->StartPoint;
					}
					POINT* P = Client->GetNextJump(MakePoint(Client->TracePoint->x, Client->TracePoint->y));
					if (P != NULL)
					{
						Client->Jump(P->x, P->y);
						Sleep(500);
					}
					LastPathFinding->Now();
				}
			}
#pragma region Melee
			else if (Client->CanMelee)
			{
				for each( pair<int, Item*> I in  Client->LocalItems.SafeDictionary())
				{
					if (max(abs(Client->X - I.second->X), abs(Client->Y - I.second->Y)) > 18)
					{
						Client->LocalItems.Remove(I.first);
					}
					else if (max(abs(Client->X - I.second->X), abs(Client->Y - I.second->Y)) < Range)
					{
						Range = max(abs(Client->X - I.second->X), abs(Client->Y - I.second->Y));
						TItem = I.second;
					}
				}
				if (TItem != NULL)
				{
					if (TItem->RedFlagTimes > 5 && max(abs(Client->X - TItem->X), abs(Client->Y - TItem->Y)) <= 11)
					{
						POINT* P = Client->GetRandomCoords(MakePoint(TItem->X, TItem->Y));
						if (P != NULL)
						{
							Client->Jump(P->x, P->y);
							Sleep(40);
						}
					}
					if (max(abs(Client->X - TItem->X), abs(Client->Y - TItem->Y)) > 11)
					{
						POINT* P = Client->GetNextJump(MakePoint(TItem->X, TItem->Y));
						if (P != NULL)
						{
							Client->Jump(P->x, P->y);
							Sleep(max(abs(Client->X - P->x), abs(Client->Y - P->y)) * 20);
						}
					}
					else if (max(abs(Client->X - TItem->X), abs(Client->Y - TItem->Y)) > 0)
					{
						Client->JumpPacketSent = false; 
						Client->Jump(TItem->X, TItem->Y); Sleep(10);
						if (Client->JumpPacketSent)
						{
							Sleep(max(abs(Client->X - TItem->X), abs(Client->Y - TItem->Y)) * 12);
							Client->PickItem(Client, TItem);
							Client->LocalItems.Remove(TItem->UID);
							TItem->RedFlagTimes++;
						}
					}
					else
					{
						Client->PickItem(Client, TItem);
						Client->LocalItems.Remove(TItem->UID);
						TItem->RedFlagTimes++;
					}
					goto FINISH;
				}
				Range = 18;
				for each( pair<int, Monster*> Monster in  Client->AllMonsters.SafeDictionary())
				{
					if (max(abs(Client->X - Monster.second->X), abs(Client->Y - Monster.second->Y)) > 18)
					{
						Client->AllMonsters.Remove(Monster.first);
					}
					else if (max(abs(Client->X - Monster.second->X), abs(Client->Y - Monster.second->Y)) < Range)
					{
						Range = max(abs(Client->X - Monster.second->X), abs(Client->Y - Monster.second->Y));
						Target = Monster.second;
					}
				}
				if (Target != NULL)
				{
					if (max(abs(Client->X - Target->X), abs(Client->Y - Target->Y)) > 13)
					{
						POINT* P = Client->GetNextJump(MakePoint(Target->X, Target->Y));
						if (P != NULL)
						{
							Client->Jump(P->x, P->y);
							Sleep(max(abs(Client->X - P->x), abs(Client->Y - P->y)) * 20);
						}
					}
					else if (max(abs(Client->X - Target->X), abs(Client->Y - Target->Y)) > 1 && max(abs(Client->X - Target->X), abs(Client->Y - Target->Y)) <= 13)
					{
						POINT* P = Client->GetRandomCoords(MakePoint(Target->X, Target->Y));
						if (P != NULL)
						{
							Client->JumpPacketSent = false; 
							Client->Jump(P->x, P->y); Sleep(10);
							if (Client->JumpPacketSent)
							{
								Sleep(300);
								Client->Attack(Client, Target->UID, 2);
								Client->AllMonsters.Remove(Target->UID);
							}
						}
					}
					else
					{
						Sleep(400);
						Client->Attack(Client, Target->UID, 2);
						Client->AllMonsters.Remove(Target->UID);
					}
				}
				else
				{
					if (max(abs(Client->X - Client->StartPoint->x), abs(Client->Y - Client->StartPoint->y)) < 16)
					{
						Client->TracePoint = Client->EndPoint;
					}
					else if (max(abs(Client->X - Client->EndPoint->x), abs(Client->Y - Client->EndPoint->y)) < 16)
					{
						Client->TracePoint = Client->StartPoint;
					}
					POINT* P = Client->GetNextJump(MakePoint(Client->TracePoint->x, Client->TracePoint->y));
					if (P != NULL)
					{
						Client->Jump(P->x, P->y);
						Sleep(300);
					}
				}
			}
#pragma endregion
		}
FINISH:
		Sleep(SleepTime);
	}
	while (!Client->BreakThreads);
}

void WINAPI Client::Receive(Client* Client)
{
	int recvbuflen = DEFAULT_BUFLEN; char Buffer[DEFAULT_BUFLEN];
	int TLength;
	do {
		memset(Buffer, 0, recvbuflen);
		TLength = recv(Client->Socket->hsock, Buffer, recvbuflen, 0);
		if (TLength > 0)
		{
			Client->HandleTask(Buffer, TLength);
		}
		else if (TLength == 0)
		{
			cout << "Connection closed " << endl;
			Owner->Socket->SocketConnected = false;
			Owner->Socket->ShutDown();
		}
		else
		{
			Owner->Socket->SocketConnected = false;
			Owner->Socket->ShutDown();
			break;
		}
	} while (TLength > 0 && !Client->BreakThreads);
}

void Client::LoadMap()
{
	int width = 0, height = 0, MapPointer = 0; DWORD Bytes = 0;
	__asm
	{
		PUSH EAX
			PUSH ECX
			MOV ECX, DWORD PTR DS:[BotProperties::MapDataAddress]
		MOV EAX, DWORD PTR DS:[ECX]
		MOV ECX, DWORD PTR DS:[EAX + 0x24]
		MOV width, ECX
			MOV ECX, DWORD PTR DS:[EAX + 0x28]
		MOV height, ECX
			MOV ECX, DWORD PTR DS:[EAX + 0x48]
		MOV MapPointer, ECX
			POP EAX
			POP ECX
	}
	char* TCells = new char[width * height << 4];
	char* cells = new char[width * height + height];
	if (ReadProcessMemory(GetCurrentProcess(), (void*)MapPointer, TCells, width * height << 4, &Bytes))
	{
		for (int X = 0; X < width; X++)
		{
			for (int Y = 0; Y < height; Y++)
			{
				int index;
				index = Y * width;
				index += X;
				index = index << 4;
				index += 2;
				cells[X * width + Y] = TCells[index];
			}
		}
		MapsHandler::MapData* MData = new MapsHandler::MapData;
		MData->Cell = cells;
		MData->Height = height;
		MData->Width = width;
		if (!MapsHandler::Maps.ContainsKey(Map))
		{
			MapsHandler::Maps.Add(Map, MData);
		}
	}
}

void Client::ProcessDisconnection()
{
	Disconnect();
}

void Client::ProcessOutgoing()
{
	if (OutgoingPacketAddress > 0 && OutgoingPacketLenght > 4 && OutgoingPacketLenght < 5000)
	{
		BypassOutgoing = false;
		SendPacketBuffer = new char[OutgoingPacketLenght];
		memcpy((void *) SendPacketBuffer, (void*)OutgoingPacketAddress, OutgoingPacketLenght);
		if (SendPacketBuffer)
		{
			HandleClientPacket(SendPacketBuffer);
			if (Sniff)
			{
				SniffPacket("Client", SendPacketBuffer, OutgoingPacketLenght);
			}
		}
	}
}

void Client::ProcessIncoming()
{
	if (IncomingPacketAddress > 0 && IncomingPacketLenght > 4 && IncomingPacketLenght < 5000)
	{
		Bypassincoming = false;
		ReceivePacketBuffer = new char[IncomingPacketLenght];
		memcpy((void *) ReceivePacketBuffer, (void*)IncomingPacketAddress, IncomingPacketLenght);
		if (ReceivePacketBuffer)
		{
			HandleServerPacket(ReceivePacketBuffer);
			if (Sniff)
			{
				SniffPacket("Server", ReceivePacketBuffer, IncomingPacketLenght);
			}
		}
	}
}

void Client::ProcessCommand(string Message)
{
	Message.erase(1, Message.length() - 1);
	if (Message == "GUI")
	{
		cout << "GUI Requested!" << endl;
	}
}

void Client::Move(BYTE Dir)
{
	switch (Dir)
	{
	case 0:
		Y += 1;
		break;
	case 2:
		X -= 1;
		break;
	case 4:
		Y -= 1;
		break;
	case 6:
		X += 1;
		break;
	case 1:
		X -= 1;
		Y += 1;
		break;
	case 3:
		X -= 1;
		Y -= 1;
		break;
	case 5:
		X += 1;
		Y -= 1;
		break;
	case 7:
		X += 1;
		Y += 1;
		break;
	}
}

void Client::Move(BYTE Dir, POINT* Point)
{
	int X = Point->x, Y = Point->y;
	switch (Dir)
	{
	case 0:
		Y += 1;
		break;
	case 2:
		X -= 1;
		break;
	case 4:
		Y -= 1;
		break;
	case 6:
		X += 1;
		break;
	case 1:
		X -= 1;
		Y += 1;
		break;
	case 3:
		X -= 1;
		Y -= 1;
		break;
	case 5:
		X += 1;
		Y -= 1;
		break;
	case 7:
		X += 1;
		Y += 1;
		break;
	}
	Point->x = X; Point->y = Y;
}

void Client::AddMonster(Monster* Mon)
{
	POINT* P = new POINT(); P->x =Mon->X; P->y = Mon->Y;
	AllMonsters.Add(Mon->UID, Mon, true);
	AllMonstersCoords.Add(Mon->UID, P);
}

void Client::RemoveMonster(Monster* Mon)
{
	AllMonsters.Remove(Mon->UID);
	AllMonstersCoords.Remove(Mon->UID);
}

void Client::Disconnect()
{
	CanMelee = false;
	AbleToHunt = false;
	Disconnected = true;
	FinishedLogin = false;
	Tasks->OnlineStatus();
	cout << "Client: " << Name << " Disconncted" << endl;
}

void Client::HandleTask(char* Buffer, int BufferLength)
{
	int PacketLenght = ToUshort(Buffer, 0);
	int PacketType = ToUshort(Buffer, 2);
	switch (PacketType)
	{
	case 500://MapTravel
		{
			PathFinder::SafeHouse* House = new PathFinder::SafeHouse();
			int UID = ToInt(Buffer, 4);
			House->Map = ToInt(Buffer, 8);
			House->X = ToUshort(Buffer, 12);
			House->Y = ToUshort(Buffer, 14);
			list<POINT*> Path = this->PathEngine->PathFind(House, MakePoint(this->X, this->Y), House->Map, 0);
			if (Path.size() > 0)
			{
				cout << "Path size: " << Path.size() << endl;
			}
			else
			{
				cout << "Path not found  "<< endl;
			}
			break;
		}
	case 1000://Test
		{
			//TeleportBasedOnQuests(400, 400, 0);
			break;
		}

	default:
		{

			break;
		}
	}
}

void Client::HandleServerPacket(char* Buffer)
{
	int PacketType = ToUshort(Buffer, 2);
	switch (PacketType)
	{
	case 1006:
		{
			this->XP = 0;
			this->CanUseXP = false;
			this->Disconnected = false;
			this->ServerConfirmedSpawn = true;
			this->BreakThreads = false;
			this->UID = ToInt(Buffer, 4);
			this->Job = ToByte(Buffer, 69);
			this->Name = ToString(Buffer, 112, ToByte(Buffer, 111));
			this->Tasks->OnlineStatus();
			break;
		}
	case 10005://Walk 
		{
			int UID = ToInt(Buffer, 8); int Dir = ToByte(Buffer, 4) % 8;
			if (AllMonsters.ContainsKey(UID))
			{
				Monster* M = AllMonsters[UID];
				M->Move(Dir);
				if (AllMonstersCoords.ContainsKey(UID))
				{
					POINT* P = new POINT(); P->x = M->X; P->y = M->Y;
					AllMonstersCoords[UID] = P;
				}
			}
			else if (this->UID == UID)
			{
				ServerConfirmedWalk = true;
			}
			break;
		}
	case 1110:
		{
			this->Map = ToInt(Buffer, 4);
			LoadMap();
			break;
		}
	case 10014:
		{
			int UID = ToInt(Buffer, 8); int Mesh = ToInt(Buffer, 4);
			if (UID == this->UID)
			{
				this->X = ToUshort(Buffer, 92); this->Y = ToUshort(Buffer, 94);
				//Client.SendToClient(Packets.Jump(Client, Client.X, Client.Y));
				ServerConfirmedSpawn = true;
			}
			else if (Mesh < 900 && UID < 510800)
			{
				BYTE Namelen = ToByte(Buffer, 231);
				string Name = ToString(Buffer, 224, Namelen);
				int X = ToUshort(Buffer, 92), Y = ToUshort(Buffer, 94), Dir = ToUshort(Buffer, 98);
				Monster* M = new Monster(UID, X, Y, this->Map, Name);
				AddMonster(M);
			}
			break;
		}
	case 1022:
		{
			BYTE AttackType = ToByte(Buffer, 20); 
			int AttackerUID = ToInt(Buffer, 8), TargetUID = ToInt(Buffer, 12);
			int X = ToUshort(Buffer, 16), Y = ToUshort(Buffer, 18);
			if (AttackType == 14)
			{
				if (AttackerUID == this->UID)
				{
					XP++;
				}
				else if (TargetUID == this->UID)
				{
					AbleToHunt = false;
					CanMelee = false;
					Disconnected = true;
					ServerConfirmedSpawn = true;
					ServerConfirmedJump = true;
					XP = 0;
					Dead = true;

				}
				if (AllMonsters.ContainsKey(TargetUID))
				{
					AllMonsters.Remove(TargetUID);
					AllMonstersCoords.Remove(TargetUID);
				}
			}
			else if (AttackType == 45 && AttackerUID == this->UID)
			{

			}
			break;
		}
	case 1105:
		{
			int AttackerUID = ToInt(Buffer, 4), SkillID = ToUshort(Buffer, 12);
			if (AttackerUID == this->UID)
			{
				if (SkillID == 6011)
				{
					CanUseXP = false;
					Lastfatle->Now();
					CanShift = true;
				}
				else if (SkillID == 1025)
				{
					CanUseXP = false;
					LastSuperMan->Now();
				}
			}
			break;
		}
	case 1101:
		{
			int UID = ToInt(Buffer, 4);
			int ID = ToInt(Buffer, 8);
			int X = ToUshort(Buffer, 12), Y = ToUshort(Buffer, 14);
			if (ToByte(Buffer, 18) == 1)
			{
				if (!LocalItems.ContainsKey(UID))
				{
					//if (ID == 1090020 && !BotProperties.PickGold) { return; }
					if (ConvertToString(ID)[0] == '7' && !OKItems.Contains(ID)) { return; }
					if ((ID % 10 >= 6 && !ForbidItems.Contains(ID)) || OKItems.Contains(ID))
					{
						if (!LocalItems.ContainsKey(UID)) { LocalItems.Add(UID, new Item(UID, ID, X, Y)); }
					}
				}
			}
			else if (ToByte(Buffer, 18) == 2)
			{
				if (LocalItems.ContainsKey(UID))
				{
					LocalItems.Remove(UID);
				}
			}
			break;
		}
	case 1008:
		{
			int UID = ToInt(Buffer, 4);
			int ID = ToInt(Buffer, 8);
			if (ToByte(Buffer, 16) == 1)
			{
				switch (ID)
				{
				case 711521://Note`Do
				case 711522://Note`Re
				case 711523://@@Note`Mi`
				case 711524://@@Note`Fa`
				case 711525://@@Note`So`
				case 711526://@@Note`La`
				case 711527://@@Note`Ti

				case 729400://Note`Do`
				case 729401://Note`Mi`
				case 729402://Note`Re`
				case 729403://Note`Fa`
				case 729404://Note`So`
				case 729405://Note`La`
				case 729406://Note`Ti`
					{
						ItemsDropIndex++;
						DropItem(UID, ItemsDropIndex);
						break;
					}
				default:
					break;
				}
				if (ID == 711661)
				{
					ItemsDropIndex++;
					DropItem(UID, ItemsDropIndex);
				}
			}
			break;
		}
	case 10010:
		{
			switch (ToByte(Buffer,20))
			{
			case 81://Actions
				{
					uint8_t Action = ToByte(Buffer, 8);
					switch (Action)
					{
					case 230:
						{

							break;
						}
					}
				}
			case 85:
				{

					break;
				}
			case 86:
				{
					this->Map = ToInt(Buffer, 8);
					LoadMap();
					FindStartPoint();
					break;
				}
			case 156:
				{
					Bypassincoming = true;
					break;
				}
			case 104:
				{
					this->X = ToUshort(Buffer, 24);
					this->Y = ToUshort(Buffer, 26);
					this->Tasks->HeroInformation();
					break;
				}
			case 137:
				{
					int UID = ToInt(Buffer, 4);
					if (UID == this->UID)
					{
						//this->Map = ToInt(Buffer, 28);
						ServerConfirmedJump = true;
					}
					break;
				}
			case 102:
				{

					break;
				}
			case 96:
				{
					FindStartPoint();
					break;
				}
			}
			break;
		}
	case 1002:
		{

			break;
		}
	case 10017:
		{
			if (ToInt(ReceivePacketBuffer, 4) != Client::UID) { return; }
			int DataType = ToInt(ReceivePacketBuffer, 8);
			if (DataType == 1)
			{
				int UpdateType = ToUshort(ReceivePacketBuffer, 12);
				uint64_t Status = ToLong(ReceivePacketBuffer, 16);
				switch (UpdateType)
				{
				case 0://Hp
					{

						break;
					}
				case 25:
					{
						long Status1 = (long)(Status + 0x800000);
						WriteUInt64(Status1, 16, ReceivePacketBuffer);
						memcpy((void*)IncomingPacketAddress, (void *) ReceivePacketBuffer, IncomingPacketLenght);
						break;
					}
				}
			}
			else if (DataType == 2)
			{
				int UpdateType = ToUshort(ReceivePacketBuffer, 36);
				uint64_t Status = ToLong(ReceivePacketBuffer, 40);
				switch (UpdateType)
				{
				case 0://Hp
					{

						break;
					}
				case 25:
					{
						long Status2 = (long)(Status + 0x800000);
						WriteUInt64(Status2, 40, ReceivePacketBuffer);
						memcpy((void*)IncomingPacketAddress, (void *) ReceivePacketBuffer, IncomingPacketLenght);
						break;
					}
				}
			}

			break;
		}
	default:
		break;
	}
}

void Client::HandleClientPacket(char* Buffer)
{
	int PacketType = ToUshort(Buffer, 2);
	switch (PacketType)
	{
	case 1004:
		{
			int ChatType = ToInt(Buffer, 8);
			int MessageID = ToInt(Buffer, 12);
			string from = ToString(Buffer, 26, ToByte(Buffer, 25));
			string To = ToString(Buffer, (27 + from.length()), ToByte(Buffer, (26 + from.length())));
			string Message = ToString(Buffer, ((29 + from.length()) + To.length()), ToByte(Buffer, (28 + from.length()) + To.length()));
			if (Message.length() > 0 &&  Message[0] == '@')
			{
				ProcessCommand(Message);
				BypassOutgoing = true;
			}
			break;
		}
	case 10005:
		{
			int UID = ToInt(Buffer, 8);
			uint8_t Dir = ToByte(Buffer, 4) % 8;
			DWORD TimeStamp = ToInt(SendPacketBuffer, 16);
			Move(Dir);
			ServerConfirmedWalk = false;

			WalkTimeStamp += 800;
			if ((WalkTimeStamp + JumpTimeStamp) >= (4294967295 - 50000))
			{ 
				WalkTimeStamp = 0;
				cout << " walk TimeStamp reset" << endl;
			}
			WriteUInt32(TimeStamp + WalkTimeStamp, 16, SendPacketBuffer);
			memcpy((void*)OutgoingPacketAddress, (void *) SendPacketBuffer, OutgoingPacketLenght);
			break;
		}
	case 10010:
		{
			switch (ToByte(Buffer, 20))
			{
			case 137:
				{
					JumpPacketSent = true;
					ServerConfirmedJump = false;
					UID = ToInt(Buffer, 4); Map = ToUshort(Buffer, 28);
					int X = ToUshort(Buffer, 8), Y = ToUshort(Buffer, 10);
					LastJump->Now();
					if (this->X > 0 && this->Y > 0)
					{
						WriteUInt16(this->X, 24, SendPacketBuffer);
						WriteUInt16(this->Y, 26, SendPacketBuffer);
					}
					this->UID = UID; this->X = X; this->Y = Y;
					DWORD TimeStamp = ToInt(SendPacketBuffer, 16);
					JumpTimeStamp += 800;
					if ((TimeStamp + JumpTimeStamp) >= (4294967295 - 50000)) 
					{ 
						JumpTimeStamp = 0;
						cout << "Jump TimeStamp reset" << endl;
					}
					WriteUInt32(TimeStamp + JumpTimeStamp, 16, SendPacketBuffer);
					memcpy((void*)OutgoingPacketAddress, (void *) SendPacketBuffer, OutgoingPacketLenght);
					break;
				}
			case 251:
				{
					FinishedLogin = true;
					break;
				}
			case 93:
				{
					XP = 0;
					break;
				}
			case 94://revive
				{
					Dead = false;
					CanUseXP = false;
					Disconnected = false;
					LastXP->Now();
					break;
				}
			case 81://Actions
				{
					uint8_t Action = ToByte(Buffer, 8);
					switch (Action)
					{
					case 230:
						{
							Sniff = true;
							cout << "Client: " << Name << " Sniff Status: "<< Sniff << endl;
							break;
						}
					case 210:
						{
							Sniff = false;
							cout << "Client: " << Name << " Sniff Status: "<< Sniff << endl;
							break;
						}
					case 250:
						{
							AbleToHunt = !AbleToHunt;
							ServerResponded = true;
							CanMelee = false;
							cout << "Client: " << Name << " Hunting status = " << AbleToHunt << endl;
							break;
						}
					case 150:
						{
							CanMelee = !CanMelee;
							cout << "Client: " << Name << " Melee hunting status = " << CanMelee << endl;
							break;
						}
					default:
						{

							break;
						}
					}
					break;
				}
			case 85:
				{
					SniffPacket("Client", SendPacketBuffer, OutgoingPacketLenght);
					break;
				}
			}
			break;
		}
	case 1022:
		{
			int Target = ToInt(Buffer, 12);
			BYTE AttackType = ToByte(Buffer, 20);
			int AttackerUID = ToInt(Buffer, 8);
			if (AttackType == 24 )
			{
				uint16_t SkillId = (uint16_t)(((long)ToByte(Buffer, 24) & 0xFF) | (((long)ToByte(Buffer, 25) & 0xFF) << 8));
				SkillId ^= 0x915d;
				SkillId ^= (uint16_t)UID;
				SkillId = (uint16_t)(SkillId << 0x3 | SkillId >> 0xd);
				SkillId -= 0xeb42;
				Target = ((Target >> 13) | (Target << 19));
				Target ^= 0x5F2D2463;
				Target ^= UID;
				Target -= 0x746F4AE6;
				/////////////////////////////////////////////////
				long xx = (ToByte(Buffer, 16) & 0xFF) | ((ToByte(Buffer, 17) & 0xFF) << 8);
				long yy = (ToByte(Buffer, 18) & 0xFF) | ((ToByte(Buffer, 19) & 0xFF) << 8);
				xx = xx ^ (UID & 0xffff) ^ 0x2ed6;
				xx = ((xx << 1) | ((xx & 0x8000) >> 15)) & 0xffff;
				xx |= 0xffff0000;
				xx -= 0xffff22ee;
				yy = yy ^ (UID & 0xffff) ^ 0xb99b;
				yy = ((yy << 5) | ((yy & 0xF800) >> 11)) & 0xffff;
				yy |= 0xffff0000;
				yy -= 0xffff8922;
				//Console.WriteLine(Target +"|"+ SkillId +"|"+ xx +"|"+ yy);
				if (AttackerUID == UID)
				{
					CanUseXP = false;
				}
			}
			else if (AttackType == 2)
			{
				if (AllMonsters.ContainsKey(Target))
				{

				}
			}
			break;
		}
	default:
		break;
	}

}

void Client::FindStartPoint()
{
	switch (Map)
	{
	case 1015:
		{
			StartPoint = MakePoint(351, 113);
			EndPoint = MakePoint(264, 138);
			TracePoint = StartPoint;
			break;
		}
	case 1002:
		{
			StartPoint = MakePoint(101, 345);
			EndPoint = MakePoint(187, 307);
			TracePoint = StartPoint;
			break;
		}
	case 1000:
		{
			StartPoint = MakePoint(399, 142);
			EndPoint = MakePoint(128, 340);
			TracePoint = StartPoint;
			break;
		}
	default:
		break;
	}
}

void Client::SniffPacket(string Source, char* Buffer, int Length)
{
	string DataStr = "";
	if (Source == "Server")
		DataStr.append("Server -> Client");
	else if (Source == "Client")
		DataStr.append("Client -> Server");
	DataStr.append("; Length : " + ConvertToString(Length) + ", PacketType: " + ConvertToString(ToUshort(Buffer, 2)) + "\n");
	for (int i = 0; i < ceil((double)Length / 16); i++)
	{
		int t = 16;
		if (((i + 1) * 16) > Length)
			t = Length - (i * 16);
		for (int a = 0; a < t; a++)
		{
			DataStr += int_to_hex(Buffer[i * 16 + a]) + " ";
		}
		if (t < 16)
			for (int a = t; a < 16; a++)
				DataStr += "   ";
		DataStr += "     ;";

		for (int a = 0; a < t; a++)
		{
			DataStr += Buffer[i * 16 + a];
		}
		DataStr += "\n";
	}
	//std::replace(DataStr.begin(), DataStr.end(), '0', '.');
	std::transform(DataStr.begin(), DataStr.end(), DataStr.begin(), ::toupper);
	DataStr += "\n";
	std::ofstream outfile;
	outfile.open("E:\\PacketSniffing\\ZPackets.txt", std::ios_base::app);
	outfile << DataStr; 
	outfile.close();
}

POINT* Client::GetRandomCoords(POINT* Point)
{
	POINT* P = new POINT(); 
	BYTE Dir = 0;
	for (BYTE i = 0; i < 30; i++)
	{
		Dir = Random(0, 8);
		P->x = Point->x; P->y = Point->y; Move(Dir, P);
		if (MapsHandler::Valid(this->Map, P->x, P->y) && !AllMonstersCoords.ContainsValue(P))
		{
			return P;
		}
	}
	return NULL;
}

POINT* Client::GetNextJump(POINT* StartPoint)
{
	POINT* SelectedPoint = new POINT();
	int Dir = GetDirection(this->X, this->Y, StartPoint->x, StartPoint->y);
	int X = this->X; int Y = this->Y; bool SpotFound = false, Round2 = false;
	switch (Dir)
	{
	case 0:
	case 2:
	case 4:
	case 6:
		{
Again:
			for (BYTE B = 10; B > 0; B--)
			{
				X = this->X; Y = this->Y;
				X += PathDirection[Dir][0] * B;
				Y += PathDirection[Dir][1] * B;
				SelectedPoint->x = X; SelectedPoint->y = Y;
				if (MapsHandler::Valid(this->Map, X, Y) && !AllMonstersCoords.ContainsValue(SelectedPoint) && max(abs(this->X - X), abs(this->Y - Y)) < 11)
				{
					return SelectedPoint;
				}
			}
			if (!Round2)
			{
				if (Dir != 0)
				{ Dir -= 1; goto Again; }
				else { Dir = 7; }
			}
			else
			{
				if (Dir != 7)
				{ if (Dir != 6) { Dir += 2; goto Again; } else { Dir = 0; } }
				else { Dir = 1; }
			}
			break;
		}
	case 1:
	case 3:
	case 5:
	case 7:
		{
Again2:
			for (BYTE B = 10; B > 0; B--)
			{
				X = this->X; Y = this->Y;
				X += PathDirection[Dir][0] * B;
				Y += PathDirection[Dir][1] * B;
				SelectedPoint->x = X; SelectedPoint->y = Y;
				if (MapsHandler::Valid(this->Map, X, Y) && !AllMonstersCoords.ContainsValue(SelectedPoint) && max(abs(this->X - X), abs(this->Y - Y)) < 11)
				{
					return SelectedPoint;
				}
			}
			if (!Round2)
			{
				if (Dir != 0)
				{ Dir -= 1; goto Again2; }
				else { Dir = 7; }
			}
			else
			{
				if (Dir != 7)
				{ if (Dir != 6) { Dir += 2; goto Again2; } else { Dir = 0; } }
				else { Dir = 1; }
			}
			break;
		}
	}
	return NULL;
}

void Client::Attack(Client* Client, int TargetUID, int AttackType)
{
	char* Pack = new char[40];
	WriteUInt16(40, 0, Pack);
	WriteUInt16(1022, 2, Pack);
	WriteUInt32(GetTickCount(), 4, Pack);
	WriteUInt32(Client->UID, 8, Pack);
	WriteUInt32(TargetUID, 12, Pack);
	WriteUInt16(0, 16, Pack);
	WriteUInt16(0, 18, Pack);
	WriteByte(AttackType, 20, Pack);
	Client->SendToServer(Pack, 40);
}

void Client::SendToServer(char* Buffer, int TLength)
{
	__asm
	{
		PUSH TLength
			PUSH DWORD PTR DS:[Buffer]
		MOV ECX, BotProperties::NetworkClass
			CALL BotProperties::SendPacketAddress
	}
}

void Client::SendTask(char* Buffer, int Length)
{
	this->Socket->Send(Buffer, Length);
}

void Client::Jump(int X, int Y)
{
	if (BotProperties::ClientType == "Realco")
	{
		__asm
		{
			PUSHAD
				PUSHFD
				PUSH Y
				PUSH X
				CALL BotProperties::WalkJumpClass
				MOV ECX,EAX
				CALL BotProperties::JumpAddress
				POPFD
				POPAD
		}
	}
}

void Client::Walk(int X, int Y)
{
	if (BotProperties::ClientType == "Realco")
	{
		__asm
		{
			PUSHAD
				PUSHFD
				PUSH Y
				PUSH X
				CALL BotProperties::WalkJumpClass
				MOV ECX,EAX
				CALL BotProperties::WalkAddress
				POPFD
				POPAD
		}
	}
}

void Client::PathFind(int ToX, int ToY)
{

}

void Client::PickItem(Client* Client, Item* Item)
{
	char* Pack = new char[24];
	WriteUInt16(24, 0, Pack);
	WriteUInt16(1101, 2, Pack);
	WriteUInt32(Item->UID, 4, Pack);
	WriteUInt32(GetTickCount(), 8, Pack);
	WriteUInt16(Item->X, 12, Pack);
	WriteUInt16(Item->Y, 14, Pack);
	WriteUInt16(0, 16, Pack);
	WriteUInt16(3, 18, Pack);
	Client->SendToServer(Pack, 24);
}

void Client::DropItem(int UID, USHORT Index)
{
	if (DropIndex >= 65000 || DropIndex < 0) DropIndex = 0;
	DropIndex++;
	char* Pack = new char[88];
	WriteUInt16(88, 0, Pack);
	WriteUInt16(1009, 2, Pack);
	WriteUInt32(UID, 4, Pack);
	WriteUInt16(DropIndex, 8, Pack);
	WriteUInt16(0, 10, Pack);
	WriteUInt32(37, 12, Pack);
	WriteUInt32(GetTickCount(), 16, Pack);
	SendToServer(Pack, 88);
}

void Client::MagicAttack(Client* Client, int TargetUID, int TX, int TY, int SkillID)
{
	int AttackerUID = Client->UID;

	int ECX = SkillID;
	ECX -= 0x14BE;//-691
	ECX &= 0xFFFF;

	SkillID = ECX >> 3;
	ECX <<= 13;
	SkillID |= ECX;

	ECX = TargetUID;
	ECX += 0x746F4AE6;
	SkillID ^= AttackerUID;
	ECX ^= AttackerUID;
	SkillID ^= 0x915D;
	ECX ^= 0x5F2D2463;

	int XX = ECX >> 19;
	ECX <<= 13;
	XX |= ECX;
	TargetUID = XX;

	ECX = TX;
	ECX += 0xFFFF22EE;
	ECX &= 0xFFFF;

	XX = ECX;
	ECX &= 1;
	XX >>= 1;
	XX &= 0xFFFF;
	ECX <<= 15;
	XX |= ECX;

	ECX = TY;
	ECX += 0xFFFF8922;
	XX ^= AttackerUID;
	ECX &= 0xFFFF;
	XX ^= 0x2ED6;

	int YY = ECX;
	ECX &= 0x1F;
	YY >>= 5;
	YY &= 0xFFFF;
	ECX <<= 0xB;
	YY |= ECX;
	YY ^= AttackerUID;
	YY ^= 0xB99B;
	uint16_t X = (uint16_t)(XX), Y = (uint16_t)(YY);
	SkillID = (uint16_t)(SkillID);

	int TimeStamp = GetTickCount();
	char* Pack = new char[40];
	WriteUInt16(40, 0, Pack);
	WriteUInt16(1022, 2, Pack);
	WriteUInt32(TimeStamp, 4, Pack); 
	WriteUInt32(Client->UID, 8, Pack);
	WriteUInt32(TargetUID, 12, Pack);
	WriteUInt16(X, 16, Pack);
	WriteUInt16(Y, 18, Pack);
	WriteUInt32(0x18, 20, Pack);
	WriteUInt16((uint16_t)SkillID, 24, Pack);
	WriteUInt16((uint16_t)(1 + 0x100 * (TimeStamp % 0x100) ^ 0x3721), 26, Pack);
	Client->SendToServer(Pack, 40);
}

void Client::UsePortal(Client* Client, int ToX, int ToY)
{
	char* Packet = new char[38];
	WriteUInt16(38, 0, Packet);
	WriteUInt16(10010, 2, Packet);
	WriteUInt32(Client->UID, 4, Packet);
	WriteUInt16(ToX, 8, Packet);
	WriteUInt16(ToY, 10, Packet);
	WriteUInt32(0, 12, Packet);
	WriteUInt32(GetTickCount(), 16, Packet);
	WriteUInt16(85, 20, Packet);
	WriteUInt16(Client->Direction, 22, Packet);
	WriteUInt16(ToX, 24, Packet);
	WriteUInt16(ToY, 26, Packet);
	WriteUInt32(0, 28, Packet);
	WriteUInt32(0xffffffff, 32, Packet);
	Client->SendToServer(Packet, 38);
}

void Client::TeleportBasedOnQuests(USHORT TX, USHORT TY, int QID)
{
	char* Pack = new char[38];
	WriteUInt16(38, 0, Pack);
	WriteUInt16(10010, 2, Pack);
	WriteUInt32(UID, 4, Pack);
	WriteUInt16(0, 8, Pack);
	WriteUInt16(0, 10, Pack);
	WriteUInt32(0, 12, Pack);
	WriteUInt16(TX, 16, Pack);
	WriteUInt16(TY, 18, Pack);
	WriteUInt16(85, 20, Pack);
	WriteUInt16(7, 22, Pack);
	WriteUInt16(X, 24, Pack);
	WriteUInt16(Y, 26, Pack);
	WriteUInt32(QID, 32, Pack);
	SendToServer(Pack, 38);
}

#pragma region Tasks

void Client::TASKS::OnlineStatus()
{
	char* Pack = new char[40];
	WriteUInt16(40, 0, Pack);
	WriteUInt16(1, 2, Pack);
	WriteUInt32(Client->UID, 4, Pack);
	WriteByte(Client->Disconnected, 8, Pack);
	WriteByte(Client->Name.length(), 9, Pack);
	WriteString(Client->Name, 10, Pack);
	Client->SendTask(Pack, 40);
}

void Client::TASKS::HeroInformation()
{
	char* Pack = new char[40];
	WriteUInt16(40, 0, Pack);
	WriteUInt16(2, 2, Pack);
	WriteUInt32(Client->UID, 4, Pack);
	WriteUInt16(Client->X, 8, Pack);
	WriteUInt16(Client->Y, 10, Pack);
	WriteUInt32(Client->Map, 12, Pack);
	Client->SendTask(Pack, 40);
}

#pragma endregion