#include "TCP.hpp"

/*******************************************************************************
*       MAIN - Begin
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
