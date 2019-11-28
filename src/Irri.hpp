#ifndef _IRRI_H_
#define _IRRI_H_

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
// using namespace std;
/************************************/

/*******************************************************************************
*      STRUCTS
******************************************************************************/
// Structs //
typedef struct ActuatorCommandTag
{
    bool status;
    uint16_t timer;
} TActuatorCommand;

typedef struct ActuatorDataTag
{
    uint16_t water_comsumption;
    uint8_t reservoir_level;
} TActuatorData;

typedef struct ContextTag
{
    int moisture;
    float temperature;
    int battery;
    int timer;
} TContext;

typedef struct inGroundTag
{
    ContextTag tag;
    uint64_t rfAdress;
} TinGround;
/*************************************************************/

/************************************************
* GLOBAL VARIABLES
***********************************************/
// RF24 Params //
/*
* Pipes Description:
* 0 - ControlHub to ControllerHub 
* 1 - ControllerHub to ControlHub
* 2 - InGround1 to ControlHub
* 3 - InGround2 to ControlHub
* 4 - InGround3 to ControlHub
* 5 - ControlHub broadcast to InGround Sensors
*/
const uint64_t pipes[6] =
    {
        0xF0F0F0F0D2LL, 0xF0F0F0F0E1LL,
        0xF0F0F0F0E2LL, 0xF0F0F0F0E3LL,
        0xF0F0F0F0F1, 0xF0F0F0F0F2};

//  static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
//  plog::init(plog::verbose, "Log.txt").addAppender(&consoleAppender);

extern std::queue<string> g_tcpRecMsg;
extern std::queue<string> g_rfRecMsg;
extern mutex mtx;
extern mutex mtxrf;


/***************************/

void nrf24Setup();
void configureRadio();
void configurePipes();
void nrf24SendTcpQueue();
void nrf24Read();
void testControllerHub();
void nrf24Restart();
// vector<string> splitDelimiter(const string &str, char delimiter);
TActuatorCommand actuatorCommandParser(const string &str);

#endif
