/*******************************************************************************
 *       Irri Firmware Raspberry pi 3B+
 *
 *    Alterações:
 *    --------------------------------------------------------------------------
 *    - 27/11/19 - Arthur Faria Campos
 *    - Criação
 *    --------------------------------------------------------------------------
 *
 ******************************************************************************/
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
#include <Irri.hpp>
#include <TCP.hpp>

// Raspberry
// #include <RF24/RF24.h>
// #include <plog/Log.h>
// #include <plog/Appenders/ColorConsoleAppender.h>

// TCP
#include <tcp/tcp_server.h>
// #include <tcp/tcp_client.h>
// file mainIrri.cpp

/************************************************
  ... NameSpaces
***********************************************/
using std::cout;
using std::mutex;
using std::string;
using std::thread;
/************************************/
/************************************************
  ... Global Variables
***********************************************/
// thread _tSendNrf24;
// thread _tReceiveNrf24;
// thread _tTcpServer;
// thread _tTcpClient;
mutex mtx;
mutex mtxrf;

// pthread_mutex_t _lock;

//Message queue
std::queue<string> g_tcpRecMsg;
std::queue<string> g_rfRecMsg;

TcpServer tcpServer;
// TcpClient tcpClient;

Client client;
server_observer_t observer1, observer2;

/************************************/
/*******************************************************************************
*       MAIN - Begin
******************************************************************************/
// plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
// plog::init(plog::verbose, "Log.txt").addAppender(&consoleAppender);
int main(int argc, char *argv[])
{
  // Initialize Logger
  static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
  plog::init(plog::verbose, "Log.txt").addAppender(&consoleAppender);
  PLOG_INFO << "IRRI says: Hello Log World!";

  nrf24Setup();

  //Catch Signal
  // signal(SIGTSTP, &signalHandler); // ^z to perform ControllerHub Test Routine
  std::thread tReceiveNrf24(nrf24Read);        // Main Loop
  std::thread tSendNrf24(nrf24SendTcpQueue);   // spawn new thread that calls bar(0)
  std::thread tTcpServer(tcpServerOpen, 9001); // spawn new thread that calls foo()
  // std::thread tTcpClient(tcpServerOpen, 9000); // spawn new thread that calls foo()

  PLOG_INFO << "Aplication Started\n";

  // synchronize threads:
  tReceiveNrf24.join(); // pauses until first finishes
  tSendNrf24.join();    // pauses until second finishes
  tTcpServer.join();

  PLOG_INFO << "All Thread Complete.\n";

  return 0;
}
/*******************************************************************************
*       MAIN - End
******************************************************************************/