#ifndef _TCP_H_
#define _TCP_H_

#include <iostream>
#include <sstream>
#include <thread>
#include <queue>
#include <mutex>
#include <signal.h>
#include <cstdlib>
#include <csignal>
#include <vector>
#include <string>
#include <cstdint>

// Raspberry
#include <RF24/RF24.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
/*
* PLOG Installation notes
* $ git clone https://github.com/SergiusTheBest/plog
* mkdir build 
* cd build
* cmake ..
* make -j
* sudo make install
*/

// TCP
#include <tcp/tcp_server.h>
// #include <tcp/tcp_client.h>

/************************************************
  ... NameSpaces
***********************************************/
using std::cout;
using std::mutex;
using std::string;
using std::thread;
/************************************/
/*************************************************************/

/************************************************
* GLOBAL VARIABLES
***********************************************/

//  static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
//  plog::init(plog::verbose, "Log.txt").addAppender(&consoleAppender);

extern std::queue<string> g_tcpRecMsg;
 extern std::queue<string> g_rfRecMsg;

extern mutex mtx;
extern mutex mtxrf;

extern TcpServer tcpServer;
extern server_observer_t observer1, observer2;
/***************************/
void onIncomingMsg1(const Client &client, const char *msg, size_t size);
void onClientDisconnected(const Client &client);
void tcpServerOpen(int p_port);

#endif
