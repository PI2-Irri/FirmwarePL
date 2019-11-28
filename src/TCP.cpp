#include "TCP.hpp"

/*******************************************************************************
*       SERVER
******************************************************************************/
void onIncomingMsg1(const Client &client, const char *msg, size_t size)
{
    std::string msgStr = msg;
    PLOG_INFO << "Observer1 got client msg: " << msgStr << std::endl;
    if (msgStr.find("quit") != std::string::npos)
    {
        PLOG_INFO << "Closing server..." << std::endl;
        pipe_ret_t finishRet = tcpServer.finish();
        if (finishRet.success)
            PLOG_INFO << "Server closed." << std::endl;
        else
            PLOG_INFO << "Failed closing server: " << finishRet.msg << std::endl;
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

void tcpServerOpen(int p_port)
{
    pipe_ret_t startRet = tcpServer.start(p_port);
    if (startRet.success)
        PLOG_INFO << "Server setup succeeded" << std::endl;
    else
        PLOG_INFO << "Server setup failed: " << startRet.msg << std::endl;

    observer1.incoming_packet_func = onIncomingMsg1;
    observer1.disconnected_func = onClientDisconnected;
    observer1.wantedIp = "127.0.0.1";
    tcpServer.subscribe(observer1);
    PLOG_INFO << "waiting for clients in port:" << p_port << std::endl;

    while (1)
    {
        Client client = tcpServer.acceptClient(0);
        if (client.isConnected())
        {
            PLOG_INFO << "Got client with IP: " << client.getIp() << std::endl;
            tcpServer.printClients();
        }
        else
        {
            PLOG_INFO << "Accepting client failed: " << client.getInfoMessage() << std::endl;
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
    signal(SIGINT, sig_exit);

    // client_observer_t observer;
    observer.wantedIp = "127.0.0.1";
    observer.clientIncomingPacketFunc = onIncomingMsg;
    observer.clientDisconnectedFunc = onDisconnection;
    tcpClient.subscribe(observer);

    pipe_ret_t connectRet = tcpClient.connectTo("127.0.0.1", p_port);
    if (connectRet.success)
    {
        PLOG_INFO << "Client connected successfully" << std::endl;
        clientOpen = true;
    }
    else
        PLOG_INFO << "Client failed to connect: " << connectRet.msg << std::endl;
}

void sig_exit(int s)
{
    PLOG_INFO << "Closing client..." << std::endl;
    pipe_ret_t finishRet = tcpClient.finish();

    if (finishRet.success)
    {
        PLOG_INFO << "Client closed." << std::endl;
    }
    else
    {
        PLOG_INFO << "Failed to close client." << std::endl;
    }
    exit(0);
}

void onIncomingMsg(const char *msg, size_t size)
{
    PLOG_INFO << "Got msg from server: " << msg << std::endl;
}

void onDisconnection(const pipe_ret_t &ret)
{
    PLOG_INFO << "Server disconnected: " << ret.msg << std::endl;
    PLOG_INFO << "Closing client..." << std::endl;
    pipe_ret_t finishRet = tcpClient.finish();
    if (finishRet.success)
    {
        PLOG_INFO << "Client closed." << std::endl;
    }
    else
    {
        PLOG_INFO << "Failed to close client: " << finishRet.msg << std::endl;
    }
    clientOpen = false;
}

void tcpClientSend(int p_port)
{
    while (1)
    {
        tcpClientConnect(p_port);

        while (clientOpen)
        {
            while (!g_rfRecMsg.empty())
            {
                mtx.lock();
                std::string msg = g_rfRecMsg.front();
                g_rfRecMsg.pop();
                mtx.unlock();

                PLOG_VERBOSE << "RF queue:" << msg;

                pipe_ret_t sendRet = tcpClient.sendMsg(msg.c_str(), msg.size());
                if (!sendRet.success)
                {
                    PLOG_VERBOSE << "Failed to send msg: " << sendRet.msg << std::endl;
                    break;
                }
                sleep(1);
            }
        }
        sleep(10);
    }
}
