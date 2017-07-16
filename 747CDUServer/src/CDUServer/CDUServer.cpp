// CDUServer.cpp : Defines the entry point for the console application.
//


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
#include "CDUServer.hpp"
#include "../common/PMDG_747QOTSII_SDK.h"
#include "../common/Util.hpp"

using namespace MirageUtil;

CDU747Server::CDU747Server( int port ):
	port_( port ),
	hSimConnect_(  nullptr ),
	serverStat_( STAT_INITIALIZING ),
	needReconnectP3D_( true ),
	isOpendP3D_( false )
{
};

CDU747Server::~CDU747Server(){
	// close the socket
	auto iResult = closesocket( listenSocket_ );
	if( iResult == SOCKET_ERROR ){
		printf( "close failed with error: %d\n", WSAGetLastError() );
		WSACleanup();
		return;
	}

	WSACleanup();

}

int CDU747Server::initListenSock( void ){

	printf( "start initNetwork\n" );

	serverStat_ = STAT_INITIALIZING;

	int iResult;
	size_t DEFAULT_BUFLEN = 1024;
	int recvbuflen = DEFAULT_BUFLEN;

	//----------------------
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData_);
	if( iResult != NO_ERROR ){
		printf( "WSAStartup failed with error: %d\n", iResult );
		return -1;
	}

	addrinfo hints;
	addrinfo* result = nullptr;

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	char strPort[ 32 ];
	_itoa_s( port_, strPort, 32, 10 );
	iResult = getaddrinfo( NULL, strPort, &hints, &result );
	if( iResult != 0 ){
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return -1;
	}

	// Create a SOCKET for connecting to server
	listenSocket_ = socket( result->ai_family, result->ai_socktype, result->ai_protocol );
	if( listenSocket_ == INVALID_SOCKET ){
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return -1;
	}

	// Setup the TCP listening socket
	iResult = bind( listenSocket_, result->ai_addr, (int)result->ai_addrlen );
	if( iResult == SOCKET_ERROR ){
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo( result );
		closesocket( listenSocket_ );
		WSACleanup();
		return -1;
	}

	freeaddrinfo( result );

	serverStat_ = STAT_WAITING;
	printf( "Wait for connection\n" );

	iResult = listen( listenSocket_, SOMAXCONN );
	if( iResult == SOCKET_ERROR ){
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket( listenSocket_ );
		WSACleanup();
		return -1;
	}

	return 0;

}

void CDU747Server::waitNetworkConnect( void ){
	// Accept a client socket
	while( true ){
		SOCKET	connectedSocket;
		connectedSocket = accept( listenSocket_, NULL, NULL );
		if( connectedSocket == INVALID_SOCKET ){
			continue;
		}
		printf( "connected\n" );
		
		std::thread t( CDU747Server::clientConnectLoop, ClientConnectArg( this, connectedSocket ) );
		t.detach();
	}
}

bool CDU747Server::initSimConnect( void ){
	HRESULT hr;

	isOpendP3D_ = false;

	if( SUCCEEDED( SimConnect_Open( &hSimConnect_, "CDU Server", NULL, 0, 0, 0)) ){

		//register CDU screen
		hr = SimConnect_MapClientDataNameToID (hSimConnect_, PMDG_747QOTSII_CDU_0_NAME, PMDG_747QOTSII_CDU_0_ID);
		hr = SimConnect_AddToClientDataDefinition (hSimConnect_, PMDG_747QOTSII_CDU_0_DEFINITION, 0, sizeof(PMDG_747QOTSII_CDU_Screen), 0, 0);
		hr = SimConnect_RequestClientData( hSimConnect_, PMDG_747QOTSII_CDU_0_ID, CDU0_DATA_REQUEST, PMDG_747QOTSII_CDU_0_DEFINITION,
										   SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);
		
		hr = SimConnect_MapClientDataNameToID (hSimConnect_, PMDG_747QOTSII_CDU_1_NAME, PMDG_747QOTSII_CDU_1_ID);
		hr = SimConnect_AddToClientDataDefinition (hSimConnect_, PMDG_747QOTSII_CDU_1_DEFINITION, 0, sizeof(PMDG_747QOTSII_CDU_Screen), 0, 0);
		hr = SimConnect_RequestClientData( hSimConnect_, PMDG_747QOTSII_CDU_1_ID, CDU1_DATA_REQUEST, PMDG_747QOTSII_CDU_1_DEFINITION,
										   SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);
		
		hr = SimConnect_MapClientDataNameToID (hSimConnect_, PMDG_747QOTSII_CDU_2_NAME, PMDG_747QOTSII_CDU_2_ID);
		hr = SimConnect_AddToClientDataDefinition (hSimConnect_, PMDG_747QOTSII_CDU_2_DEFINITION, 0, sizeof(PMDG_747QOTSII_CDU_Screen), 0, 0);
		hr = SimConnect_RequestClientData( hSimConnect_, PMDG_747QOTSII_CDU_2_ID, CDU2_DATA_REQUEST, PMDG_747QOTSII_CDU_2_DEFINITION,
										   SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);

		//register 747 Current stat
		hr = SimConnect_MapClientDataNameToID (hSimConnect_, PMDG_747QOTSII_DATA_NAME, PMDG_747QOTSII_DATA_ID);
		hr = SimConnect_AddToClientDataDefinition (hSimConnect_, PMDG_747QOTSII_DATA_DEFINITION, 0, sizeof(PMDG_747QOTSII_Data), 0, 0);
		hr = SimConnect_RequestClientData(hSimConnect_, PMDG_747QOTSII_DATA_ID, DATA_REQUEST, PMDG_747QOTSII_DATA_DEFINITION,
										  SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);

		//Control setup
		control_.Event = 0;
		control_.Parameter = 0;
		hr = SimConnect_MapClientDataNameToID( hSimConnect_, PMDG_747QOTSII_CONTROL_NAME, PMDG_747QOTSII_CONTROL_ID );
		hr = SimConnect_AddToClientDataDefinition ( hSimConnect_, PMDG_747QOTSII_CONTROL_DEFINITION, 0, sizeof(PMDG_747QOTSII_Control), 0, 0);
		hr = SimConnect_RequestClientData( hSimConnect_, PMDG_747QOTSII_CONTROL_ID, CONTROL_REQUEST, PMDG_747QOTSII_CONTROL_DEFINITION,
										   SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);
		for( auto i = 0; i < 70; ++i ){
			hr = SimConnect_MapClientEventToSimEvent( hSimConnect_, static_cast<EVENT_ID>(i), simConnectEnumToStr( i, EVT_CDU_L_START ).c_str() );
			hr = SimConnect_MapClientEventToSimEvent( hSimConnect_, static_cast<EVENT_ID>(i+70), simConnectEnumToStr( i, EVT_CDU_R_START ).c_str() );
			hr = SimConnect_MapClientEventToSimEvent( hSimConnect_, static_cast<EVENT_ID>(i+140), simConnectEnumToStr( i, EVT_CDU_C_START ).c_str() );
		}


		needReconnectP3D_ = false;
		return true;

	}else{

		return false;

	}
}

void CALLBACK CDU747Server::simConnectDispatch( SIMCONNECT_RECV* pData, DWORD cbData, void *pContext ){
	CDU747Server* self = reinterpret_cast< CDU747Server* >( pContext );

	switch( pData->dwID ){
	 case SIMCONNECT_RECV_ID_CLIENT_DATA: // Receive and process the 747QOTSII data block
		{
			SIMCONNECT_RECV_CLIENT_DATA *pObjData = (SIMCONNECT_RECV_CLIENT_DATA*)pData;

			switch( pObjData->dwRequestID )
			{
			 case CDU0_DATA_REQUEST:
				if( self->isOpendP3D_ ){
					PMDG_747QOTSII_CDU_Screen *pS = (PMDG_747QOTSII_CDU_Screen*)&pObjData->dwData;
					self->screenMutex_.lock();
					memcpy( &( self->screen_[0] ), pS, sizeof(PMDG_747QOTSII_CDU_Screen) );
					self->screenUpdated_ = true;
					self->screenMutex_.unlock();
				}
				break;
				
			 case CDU1_DATA_REQUEST:
				if( self->isOpendP3D_ ){
					PMDG_747QOTSII_CDU_Screen *pS = (PMDG_747QOTSII_CDU_Screen*)&pObjData->dwData;
					self->screenMutex_.lock();
					memcpy( &( self->screen_[1] ), pS, sizeof(PMDG_747QOTSII_CDU_Screen) );
					self->screenUpdated_ = true;
					self->screenMutex_.unlock();
				}
				break;

			 case CDU2_DATA_REQUEST:
				if( self->isOpendP3D_ ){
					PMDG_747QOTSII_CDU_Screen *pS = (PMDG_747QOTSII_CDU_Screen*)&pObjData->dwData;
					self->screenMutex_.lock();
					memcpy( &( self->screen_[2] ), pS, sizeof(PMDG_747QOTSII_CDU_Screen) );
					self->screenUpdated_ = true;
					self->screenMutex_.unlock();
				}
				break;
				
			 case DATA_REQUEST:
				{
					PMDG_747QOTSII_Data *pS = (PMDG_747QOTSII_Data*)&pObjData->dwData;
					memcpy( &( self->currentData_ ), pS, sizeof( PMDG_747QOTSII_Data ) );
					break;
				}
			 case CONTROL_REQUEST:
				{
					// keep the present state of Control area to know if the server had received and reset the command
					PMDG_747QOTSII_Control *pS = (PMDG_747QOTSII_Control*)&pObjData->dwData;
					self->control_ = *pS;
					break;
				}
			}
			break;
		}
	 case SIMCONNECT_RECV_ID_OPEN:
		self->isOpendP3D_ = true;
		break;

	 case SIMCONNECT_RECV_ID_QUIT:
		self->needReconnectP3D_ = true;
		break;

	 default:
		printf("Received:%d\n",pData->dwID);
		break;
	}

}


void CDU747Server::serverLoop( SOCKET& connectedSocket ){

	//MainLoop
	while( true ){

		std::array< char, 1024 >	recvBuffer;
		std::string recvMsg;
		auto result = recv( connectedSocket, &( recvBuffer[0] ), 1024, 0 );
		if( result > 0 ){
			for( unsigned int i = 0; i < result; ++i ){
				recvMsg.push_back( recvBuffer[i] );
			}

			if( recvMsg == "Req:CDU" ){
				sendCDUDisplay( connectedSocket, 0 );
			}else if( recvMsg == "Req:CDU0" ){
				sendCDUDisplay( connectedSocket, 0 );
			}else if( recvMsg == "Req:CDU1" ){
				sendCDUDisplay( connectedSocket, 1 );
			}else if( recvMsg == "Req:CDU2" ){
				sendCDUDisplay( connectedSocket, 2 );
			}else if( recvMsg == "Req:ANNU" ){
				sendAnnunciatorStat( connectedSocket );
			}else{
				auto tok = tokenizeString( recvMsg, ":" );
				if( tok.size() > 1 ){
					if( tok[0] == "CDU0" ){
						auto val = atoi( tok[1].c_str() );
						//printf( "KeyVal:%d", val );

						EVENT_ID ev = static_cast< EVENT_ID >( val );
						SimConnect_TransmitClientEvent( hSimConnect_, 0, ev, MOUSE_FLAG_LEFTSINGLE,
														SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY );
						SimConnect_TransmitClientEvent( hSimConnect_, 0, ev, MOUSE_FLAG_LEFTRELEASE,
														SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY );
					}
					if( tok[0] == "CDU1" ){
						auto val = atoi( tok[1].c_str() ) + 70;
						//printf( "KeyVal:%d", val );

						EVENT_ID ev = static_cast< EVENT_ID >( val );
						SimConnect_TransmitClientEvent( hSimConnect_, 0, ev, MOUSE_FLAG_LEFTSINGLE,
														SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY );
						SimConnect_TransmitClientEvent( hSimConnect_, 0, ev, MOUSE_FLAG_LEFTRELEASE,
														SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY );
					}
					if( tok[0] == "CDU2" ){
						auto val = atoi( tok[1].c_str() ) + 140;
						//printf( "KeyVal:%d", val );

						EVENT_ID ev = static_cast< EVENT_ID >( val );
						SimConnect_TransmitClientEvent( hSimConnect_, 0, ev, MOUSE_FLAG_LEFTSINGLE,
														SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY );
						SimConnect_TransmitClientEvent( hSimConnect_, 0, ev, MOUSE_FLAG_LEFTRELEASE,
														SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY );
					}
				}
			}

		}else if( result == 0 ){
			break;
		}else if( result == SOCKET_ERROR ){
			break;
		}

		//Sleep( 10 );
	}

	closesocket( connectedSocket );

	printf( "Closed\n" );

}

void CDU747Server::pollingSimConnectDatas( void ){
	//MainLoop
	while( true ){
		SimConnect_CallDispatch(hSimConnect_, CDU747Server::simConnectDispatch, this);
		if( needReconnectP3D_ ){
			SimConnect_Close( hSimConnect_ );
			break;
		}

		if( screenUpdated_ ){
			#if 0
			for( auto y = 0; y < CDU_ROWS; ++y ){
				for( auto x = 0; x < CDU_COLUMNS; ++x ){
					PMDG_747QOTSII_CDU_Cell* cell = &( screen_.Cells[x][y] );
					printf( "%c", cell->Symbol );
				}
				printf( "\n" );
			}
			#endif
			screenUpdated_ = false;
		}

		Sleep( 50 );
	}

}

void CDU747Server::sendAnnunciatorStat( SOCKET& connectedSocket ){

	std::string buff;

	char flags = currentData_.CDU_annunEXEC[0] ? 1 << 0 : 0;
	flags |= currentData_.CDU_annunDSPY[0] ? 1 << 1 : 0;
	flags |= currentData_.CDU_annunFAIL[0] ? 1 << 2 : 0;
	flags |= currentData_.CDU_annunMSG[0] ? 1 << 3 : 0;
	flags |= currentData_.CDU_annunOFST[0] ? 1 << 4 : 0;

	buff.push_back( flags );

	auto result = send( connectedSocket, buff.c_str(), buff.size(), 0 );
	if( result == SOCKET_ERROR ){
		printf( "send error\n" );
	}

}
	
void CDU747Server::sendCDUDisplay( SOCKET& connectedSocket, int cduIdx ){

	static char buff[ CDU_ROWS * CDU_COLUMNS * 2 ];

	screenMutex_.lock();
	int cnt = 0;
	for( auto y = 0; y < CDU_ROWS; ++y ){
		for( auto x = 0; x < CDU_COLUMNS; ++x ){
			PMDG_747QOTSII_CDU_Cell* cell = &( screen_[cduIdx].Cells[x][y] );
			buff[cnt] = cell->Symbol;
			++cnt;
			buff[cnt] = cell->Flags;
			++cnt;
		}
	}
	screenMutex_.unlock();

	auto result = send( connectedSocket, buff, CDU_ROWS * CDU_COLUMNS * 2, 0 );
	if( result == SOCKET_ERROR ){
		printf( "send error\n" );
	}

}

void CDU747Server::clientConnectLoop( ClientConnectArg arg ){
	arg.self->serverLoop( arg.connectedSocket );
}

void CDU747Server::run( CDU747Server* self ){
	while( !self->initListenSock() == 0 ){
		Sleep( 1000 );
	}
	self->waitNetworkConnect();
}

void CDU747Server::simConnectLoop( CDU747Server* self ){
	while (true) {
		while (!self->initSimConnect()) {
			Sleep(1000);
		}
		self->needReconnectP3D_ = false;
		self->pollingSimConnectDatas();

		printf( "simConnect end\n" );
	}
}



int main( int argc, char* argv ){

	CDU747Server server( 47474 );
	std::thread t1( CDU747Server::run, &server );
	std::thread t2( CDU747Server::simConnectLoop, &server );
	t1.join();
	t2.join();
	
}

