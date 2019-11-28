#include <iostream>
#include <tcp/tcp_server.hpp>
#include <sstream>
#include <thread>
#include <queue>

using std::cout;
using std::string;

void print(Node *nd, string msg)
{
  cout << nd->ip << " : " << msg << std::endl;
}

tcp::server server(9000);

void threadFunction()
{

  while(1)
  {
      while (!server.Menssage.empty())
  {
    cout << "Message" << ' ' << server.Menssage.front();
    server.Menssage.pop();
  }


  }

}

int main(int argc, char **argv)
{
  // pthread_t t1;
  // pthread_attr_t attr;
  // tcp::server server(9000) = (tcp::server)malloc(sizeof(tcp::server));
  // pthread_create(&t1, NULL, &attr, (void*)server);

  //initialize default port number and max connection cout
  // int port = 9000;

  // // check if there are any passed arguments
  // if (argc > 1)
  // {
  //   // initialize string stream from argument
  //   std::istringstream arg_stream(argv[1]);

  //   // bind arguments stream to port int variable if valid
  //   if (!(arg_stream >> port))
  //     std::cerr << "Invalid number " << argv[1] << '\n';
  // }

  // create server instance with specified port number
  // tcp::server server(port);

  // tcp::server server(9000);
  // server(port);

  // set async fn to handle incoming messages
  server.OnMessage(print);

  // pthread_t t1;
  std::thread t(&threadFunction);

  server.Listen();



  // pthread_create(&t1, NULL, threadFunction, NULL);

  // int rc;

  // rc = pthread_create(&thread, NULL, _tcpServer, void*server;

  // if (rc)
  // {
  //   cout << "Error:unable to create thread," << rc << endl;
  //   exit(-1);
  // }

  // Printing content of queue

  // //start listening to connections
  //   pid_t  _childpid;

  // if ((_childpid = fork()) == 0)
  // {
  //   while (1)
  //   {
  //     // cout << ' ' << server.Menssage.size();
  //     while (!server.Menssage.empty())
  //     {
  //       cout << "Message"<<' ' << server.Menssage.front();
  //       server.Menssage.pop();
  //     }
  //   }
  // }

  // int result = server.Listen();

  // if (result != 0)
  //   cout << "Faild to start server !" << std::endl;

  return 0;
}
