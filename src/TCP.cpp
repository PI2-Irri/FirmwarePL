#include "TCP.hpp"

/*******************************************************************************
*       SERVER
******************************************************************************/
void onIncomingMsg1(const Client &client, const char *msg, size_t size)
{
    std::string msgStr = msg;
    PLOG_WARNING << "Observer1 got client msg: " << msgStr;
    if (msgStr.find("quit") != std::string::npos)
    {
        PLOG_WARNING << "Closing server...";
        pipe_ret_t finishRet = tcpServer.finish();
        if (finishRet.success)
            PLOG_WARNING << "Server closed." << std::endl;
        else
            PLOG_WARNING << "Failed closing server: " << finishRet.msg << std::endl;
    }
    else if (msgStr.find("print") != std::string::npos)
        tcpServer.printClients();
    else
    {
        std::string replyMsg = "server got this msg: " + msgStr;
        mtx.lock();
        g_tcpRecMsg.push(msgStr);
        mtx.unlock();
        PLOG_INFO << "Push Message: " << msgStr << std::endl;
        // server.sendToAllClients(replyMsg.c_str(), replyMsg.length());
    }
}

void onClientDisconnected(const Client &client)
{
    PLOG_INFO << "Client: " << client.getIp() << " disconnected: " << client.getInfoMessage() << std::endl;
}
void CloseServer(int s)
{
    PLOG_WARNING << "Closing Server...";
    pipe_ret_t finishServer = tcpServer.finish();
    PLOG_WARNING_IF(finishServer.success) << "Server closed.";
    PLOG_WARNING_IF(!finishServer.success) << "Failed to close Server.";
    exit(0);
}

void tcpServerOpen(int p_port)
{

    pipe_ret_t startServer = tcpServer.start(p_port);
    PLOG_WARNING_IF(startServer.success) << "Server setup succeeded" ;
    PLOG_WARNING_IF(!startServer.success) << "Server setup failed: "<< startServer.msg;

    observer1.incoming_packet_func = onIncomingMsg1;
    observer1.disconnected_func = onClientDisconnected;
    observer1.wantedIp = "127.0.0.1";
    tcpServer.subscribe(observer1);
    
    PLOG_DEBUG << "waiting for clients in port:" << p_port << std::endl;

    while (1)
    {
        Client client = tcpServer.acceptClient(0);
        if (client.isConnected())
        {
            PLOG_WARNING << "Got client with IP: " << client.getIp() << std::endl;
            tcpServer.printClients();
        }
        else
        {
            PLOG_ERROR << "Accepting client failed: " << client.getInfoMessage() << std::endl;
        }
        sleep(1);
    }
}

/*******************************************************************************
*       CLIENT
******************************************************************************/
bool clientOpen = false;

void tcpClientConnect(int p_port)
{
    // signal(SIGINT, sig_exit);
    // client_observer_t observer;
    observer.wantedIp = "127.0.0.1";
    observer.clientIncomingPacketFunc = onIncomingMsg;
    observer.clientDisconnectedFunc = onDisconnection;
    tcpClient.subscribe(observer);

    pipe_ret_t connectRet = tcpClient.connectTo("127.0.0.1", p_port);

    if (connectRet.success)
    {
        PLOG_WARNING << "Client connected successfully" << std::endl;
        clientOpen = true;
    }
    else
        PLOG_WARNING << "Client failed to connect: " << connectRet.msg << std::endl;
}

void sig_exit(int s)
{
    PLOG_WARNING << "Closing client...";
    pipe_ret_t finishRet = tcpClient.finish();

    if (finishRet.success)
    {
        PLOG_WARNING << "Client closed.";
        // clientOpen = false;
    }
    else
    {
        PLOG_WARNING << "Failed to close client.";
    }
    exit(0);
}

void onIncomingMsg(const char *msg, size_t size)
{
    PLOG_WARNING << "Got msg from server: " << msg << std::endl;
}

void onDisconnection(const pipe_ret_t &ret)
{
    PLOG_WARNING << "Server disconnected: " << ret.msg;
    PLOG_WARNING << "Closing client...";
    pipe_ret_t finishRet = tcpClient.finish();
    if (finishRet.success)
        PLOG_WARNING << "Client closed.";
    else
        PLOG_WARNING << "Failed to close client: " << finishRet.msg;
    clientOpen = false;
}

void tcpClientSend(int p_port)
{
    while (1)
    {
        PLOG_NONE << "try to connect";
        tcpClientConnect(p_port);
        int msgretry = 0;

        while (clientOpen)
        {
            while (!g_rfRecMsg.empty())
            {

                mtx.lock(); /**< Mutex Lock*/
                std::string msg = g_rfRecMsg.front();
                // msg << std::endl;
                g_rfRecMsg.pop();
                mtx.unlock(); /**< Mutex unlock*/

                PLOG_INFO << "Pop RF queue:" << msg;

                pipe_ret_t sendRet = tcpClient.sendMsg(msg.c_str(), msg.size());
                while (!sendRet.success && msgretry < 5)
                {
                    PLOG_ERROR << "Failed to send msg: " << sendRet.msg;
                    PLOG_ERROR << "Try to resend: " << msgretry << "/5";
                    sendRet = tcpClient.sendMsg(msg.c_str(), msg.size());
                    msgretry++;

                    // break;
                }
                if (msgretry == 5)
                    break;

                // if (!sendRet.success)
                // {
                // }
                sleep(1);
            }
        }
        sleep(10);
    }
}
