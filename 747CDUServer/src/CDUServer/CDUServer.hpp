// CDUServer.cpp : Defines the entry point for the console application.
//

#if !defined( CDUSERVER_HPP_INCLUDED__ )
#define CDUSERVER_HPP_INCLUDED__

#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#include <thread>
#include <memory>
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <mutex>

#include "SimConnect.h"
#include "../common/PMDG_747QOTSII_SDK.h"
#include "../common/Util.hpp"

using namespace MirageUtil;

class CDU747Server{
 public:

	enum EVENT_ID{
		EVENT_CDU_L_L1,
		EVENT_CDU_L_L2,
		EVENT_CDU_L_L3,
		EVENT_CDU_L_L4,
		EVENT_CDU_L_L5,
		EVENT_CDU_L_L6,
		EVENT_CDU_L_R1,
		EVENT_CDU_L_R2,
		EVENT_CDU_L_R3,
		EVENT_CDU_L_R4,
		EVENT_CDU_L_R5,
		EVENT_CDU_L_R6,
		EVENT_CDU_L_INIT_REF,
		EVENT_CDU_L_RTE,
		EVENT_CDU_L_DEP_ARR,
		EVENT_CDU_L_ATC,
		EVENT_CDU_L_VNAV,
		EVENT_CDU_L_FIX,
		EVENT_CDU_L_LEGS,
		EVENT_CDU_L_HOLD,
		EVENT_CDU_L_FMCCOMM,
		EVENT_CDU_L_PROG,
		EVENT_CDU_L_EXEC,
		EVENT_CDU_L_MENU,
		EVENT_CDU_L_NAV_RAD,
		EVENT_CDU_L_PREV_PAGE,
		EVENT_CDU_L_NEXT_PAGE,
		EVENT_CDU_L_1,
		EVENT_CDU_L_2,
		EVENT_CDU_L_3,
		EVENT_CDU_L_4,
		EVENT_CDU_L_5,
		EVENT_CDU_L_6,
		EVENT_CDU_L_7,
		EVENT_CDU_L_8,
		EVENT_CDU_L_9,
		EVENT_CDU_L_DOT,
		EVENT_CDU_L_0,
		EVENT_CDU_L_PLUS_MINUS,
		EVENT_CDU_L_A,
		EVENT_CDU_L_B,
		EVENT_CDU_L_C,
		EVENT_CDU_L_D,
		EVENT_CDU_L_E,
		EVENT_CDU_L_F,
		EVENT_CDU_L_G,
		EVENT_CDU_L_H,
		EVENT_CDU_L_I,
		EVENT_CDU_L_J,
		EVENT_CDU_L_K,
		EVENT_CDU_L_L,
		EVENT_CDU_L_M,
		EVENT_CDU_L_N,
		EVENT_CDU_L_O,
		EVENT_CDU_L_P,
		EVENT_CDU_L_Q,
		EVENT_CDU_L_R,
		EVENT_CDU_L_S,
		EVENT_CDU_L_T,
		EVENT_CDU_L_U,
		EVENT_CDU_L_V,
		EVENT_CDU_L_W,
		EVENT_CDU_L_X,
		EVENT_CDU_L_Y,
		EVENT_CDU_L_Z,
		EVENT_CDU_L_SPACE,
		EVENT_CDU_L_DEL,
		EVENT_CDU_L_SLASH,
		EVENT_CDU_L_CLR,
		EVENT_CDU_L_BRITENESS,

		EVENT_CDU_R_L1,
		EVENT_CDU_R_L2,
		EVENT_CDU_R_L3,
		EVENT_CDU_R_L4,
		EVENT_CDU_R_L5,
		EVENT_CDU_R_L6,
		EVENT_CDU_R_R1,
		EVENT_CDU_R_R2,
		EVENT_CDU_R_R3,
		EVENT_CDU_R_R4,
		EVENT_CDU_R_R5,
		EVENT_CDU_R_R6,
		EVENT_CDU_R_INIT_REF,
		EVENT_CDU_R_RTE,
		EVENT_CDU_R_DEP_ARR,
		EVENT_CDU_R_ATC,
		EVENT_CDU_R_VNAV,
		EVENT_CDU_R_FIX,
		EVENT_CDU_R_LEGS,
		EVENT_CDU_R_HOLD,
		EVENT_CDU_R_FMCCOMM,
		EVENT_CDU_R_PROG,
		EVENT_CDU_R_EXEC,
		EVENT_CDU_R_MENU,
		EVENT_CDU_R_NAV_RAD,
		EVENT_CDU_R_PREV_PAGE,
		EVENT_CDU_R_NEXT_PAGE,
		EVENT_CDU_R_1,
		EVENT_CDU_R_2,
		EVENT_CDU_R_3,
		EVENT_CDU_R_4,
		EVENT_CDU_R_5,
		EVENT_CDU_R_6,
		EVENT_CDU_R_7,
		EVENT_CDU_R_8,
		EVENT_CDU_R_9,
		EVENT_CDU_R_DOT,
		EVENT_CDU_R_0,
		EVENT_CDU_R_PLUS_MINUS,
		EVENT_CDU_R_A,
		EVENT_CDU_R_B,
		EVENT_CDU_R_C,
		EVENT_CDU_R_D,
		EVENT_CDU_R_E,
		EVENT_CDU_R_F,
		EVENT_CDU_R_G,
		EVENT_CDU_R_H,
		EVENT_CDU_R_I,
		EVENT_CDU_R_J,
		EVENT_CDU_R_K,
		EVENT_CDU_R_L,
		EVENT_CDU_R_M,
		EVENT_CDU_R_N,
		EVENT_CDU_R_O,
		EVENT_CDU_R_P,
		EVENT_CDU_R_Q,
		EVENT_CDU_R_R,
		EVENT_CDU_R_S,
		EVENT_CDU_R_T,
		EVENT_CDU_R_U,
		EVENT_CDU_R_V,
		EVENT_CDU_R_W,
		EVENT_CDU_R_X,
		EVENT_CDU_R_Y,
		EVENT_CDU_R_Z,
		EVENT_CDU_R_SPACE,
		EVENT_CDU_R_DEL,
		EVENT_CDU_R_SLASH,
		EVENT_CDU_R_CLR,
		EVENT_CDU_R_BRITENESS,

		EVENT_CDU_C_L1,
		EVENT_CDU_C_L2,
		EVENT_CDU_C_L3,
		EVENT_CDU_C_L4,
		EVENT_CDU_C_L5,
		EVENT_CDU_C_L6,
		EVENT_CDU_C_R1,
		EVENT_CDU_C_R2,
		EVENT_CDU_C_R3,
		EVENT_CDU_C_R4,
		EVENT_CDU_C_R5,
		EVENT_CDU_C_R6,
		EVENT_CDU_C_INIT_REF,
		EVENT_CDU_C_RTE,
		EVENT_CDU_C_DEP_ARR,
		EVENT_CDU_C_ATC,
		EVENT_CDU_C_VNAV,
		EVENT_CDU_C_FIX,
		EVENT_CDU_C_LEGS,
		EVENT_CDU_C_HOLD,
		EVENT_CDU_C_FMCCOMM,
		EVENT_CDU_C_PROG,
		EVENT_CDU_C_EXEC,
		EVENT_CDU_C_MENU,
		EVENT_CDU_C_NAV_RAD,
		EVENT_CDU_C_PREV_PAGE,
		EVENT_CDU_C_NEXT_PAGE,
		EVENT_CDU_C_1,
		EVENT_CDU_C_2,
		EVENT_CDU_C_3,
		EVENT_CDU_C_4,
		EVENT_CDU_C_5,
		EVENT_CDU_C_6,
		EVENT_CDU_C_7,
		EVENT_CDU_C_8,
		EVENT_CDU_C_9,
		EVENT_CDU_C_DOT,
		EVENT_CDU_C_0,
		EVENT_CDU_C_PLUS_MINUS,
		EVENT_CDU_C_A,
		EVENT_CDU_C_B,
		EVENT_CDU_C_C,
		EVENT_CDU_C_D,
		EVENT_CDU_C_E,
		EVENT_CDU_C_F,
		EVENT_CDU_C_G,
		EVENT_CDU_C_H,
		EVENT_CDU_C_I,
		EVENT_CDU_C_J,
		EVENT_CDU_C_K,
		EVENT_CDU_C_L,
		EVENT_CDU_C_M,
		EVENT_CDU_C_N,
		EVENT_CDU_C_O,
		EVENT_CDU_C_P,
		EVENT_CDU_C_Q,
		EVENT_CDU_C_R,
		EVENT_CDU_C_S,
		EVENT_CDU_C_T,
		EVENT_CDU_C_U,
		EVENT_CDU_C_V,
		EVENT_CDU_C_W,
		EVENT_CDU_C_X,
		EVENT_CDU_C_Y,
		EVENT_CDU_C_Z,
		EVENT_CDU_C_SPACE,
		EVENT_CDU_C_DEL,
		EVENT_CDU_C_SLASH,
		EVENT_CDU_C_CLR,
		EVENT_CDU_C_BRITENESS,
	};

	enum DATA_REQUEST_ID{
		DATA_REQUEST,
		CONTROL_REQUEST,
		AIR_PATH_REQUEST,
		CDU0_DATA_REQUEST,
		CDU1_DATA_REQUEST,
		CDU2_DATA_REQUEST,
	};

	enum SERVER_STAT{
		STAT_INITIALIZING,
		STAT_WAITING,
		STAT_CONNECTED,
		STAT_CLOSED,
		STAT_ERR
	};

	struct ClientConnectArg{
		ClientConnectArg( CDU747Server* p, SOCKET& s ):self(p),connectedSocket( s ){};
		CDU747Server* self;
		SOCKET			connectedSocket;
	};
	
 public:
	CDU747Server( int port );
	virtual ~CDU747Server();

 public:
	virtual int	initListenSock( void );
	void		waitNetworkConnect( void );
	bool		initSimConnect( void );
	static void CALLBACK	simConnectDispatch( SIMCONNECT_RECV* pData, DWORD cbData, void *pContext );
	virtual void serverLoop( SOCKET& connectedSocket );
	virtual void pollingSimConnectDatas( void );
	virtual void sendAnnunciatorStat( SOCKET& connectedSocket );
	virtual void sendCDUDisplay( SOCKET& connectedSocket, int cduIdx );
	static void clientConnectLoop( ClientConnectArg arg );
	static void run( CDU747Server* self );
	static void simConnectLoop( CDU747Server* self );

 protected:
	WSADATA			wsaData_;

	SOCKET			listenSocket_;

	std::string		address_;
	int				port_;
	HANDLE  hSimConnect_;

	PMDG_747QOTSII_CDU_Screen screen_[3];
	bool screenUpdated_;
	std::mutex screenMutex_;

	static const int CELL_WIDTH = 30;
	static const int CELL_HEIGHT = 45;

	PMDG_747QOTSII_Control control_;
	PMDG_747QOTSII_Data currentData_;
	
	bool needReconnectP3D_;
	bool isOpendP3D_;

	SERVER_STAT serverStat_;
};

int main( int argc, char* argv );

#endif