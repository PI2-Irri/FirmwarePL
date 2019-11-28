// /*******************************************************************************
//  *       Irri Firmware Raspberry pi 3B+
//  *
//  *    Alterações:
//  *    --------------------------------------------------------------------------
//  *    - 27/11/19 - Arthur Faria Campos
//  *    - Criação
//  *    --------------------------------------------------------------------------
//  *
//  ******************************************************************************/
#include "Irri.hpp"

// /************************************/
// /************************************************
//   ... Global Variables
// ***********************************************/
// // RF24 Params //
// /*
// * Pipes Description:
// * 0 - ControlHub to ControllerHub
// * 1 - ControllerHub to ControlHub
// * 2 - InGround1 to ControlHub
// * 3 - InGround2 to ControlHub
// * 4 - InGround3 to ControlHub
// * 5 - ControlHub broadcast to InGround Sensors
// */
// // pthread_mutex_t _lock;

// //Message queue
// std::queue<string> g_tcpRecMsg;
// TcpServer tcpServer;
// // TcpClient tcpClient;

// Client client;
// server_observer_t observer1, observer2;
// // queue<ActuatorData> controllerHubData;
// // queue<inGroundTag> inGroundData;
// /************************************/
std::queue<TActuatorData> controllerHubData;
std::queue<TinGround> inGroundData;

RF24 radio(26, 22);
bool rfBegin;
uint8_t rfpipe = 1;
bool testCHub = false;

void nrf24Setup()
{
    // Radio Setup
    rfBegin = radio.begin();
    // if
    PLOG_INFO_IF(rfBegin) << "configureRadio: RF24 started!";
    PLOG_FATAL_IF(!rfBegin) << "configureRadio: RF24 couldn't begin :(";
    configureRadio();
    configurePipes();
    radio.printDetails();
    radio.startListening();
}

// vector<string> splitDelimiter(const string &str, char delimiter);

TActuatorCommand actuatorCommandParser(const string &str)
{
    TActuatorCommand command;
    std::string delims = ":,";
    std::vector<std::string> results;
    size_t lastOffset = 0;

    while (true)
    {
        size_t offset = str.find_first_of(delims, lastOffset);
        results.push_back(str.substr(lastOffset, offset - lastOffset));
        if (offset == std::string::npos)
            break;
        else
            lastOffset = offset + 1; // add one to skip the delimiter
    }

    for (size_t i = 0; i < results.size(); i++)
    {
        if (results[i] == "status")
        {
            if (results[i + 1] == "on")
                command.status = true;
            else
                command.status = false;
        }
        else if (results[i] == "timer")
            command.timer = (uint16_t)std::stoi(results[i + 1]);
        std::cout << results[i] << std::endl;
    }

    return command;
}

void nrf24SendTcpQueue()
{
    while (1)
    {
        while (!g_tcpRecMsg.empty())
        {
            mtx.lock();
            std::string msg = g_tcpRecMsg.front();
            g_tcpRecMsg.pop();
            mtx.unlock();

            PLOG_VERBOSE << "Queue:" << msg;
            TActuatorCommand cmd = actuatorCommandParser(msg);
            bool sent;

            mtx.lock();
            PLOG_FATAL_IF(radio.failureDetected) << "RF24 FAILED!!!!!";
            if (radio.failureDetected)
                nrf24Restart();
            radio.stopListening();
            sent = radio.write(&cmd, sizeof(cmd));
            radio.startListening();
            mtx.unlock();

            if (sent)
                cout << "SENT.\n";
            else
                cout << "FAILED :(\n";
        }
    }
}

void nrf24Restart()
{

    radio.failureDetected = false;
    // Perform Radio Setup
    rfBegin = radio.begin();
    PLOG_INFO_IF(rfBegin) << "RF24 started after failure!";
    PLOG_FATAL_IF(!rfBegin) << "RF24 couldn't begin after failure :((";
    configureRadio();
    configurePipes();
    radio.printDetails();
    radio.startListening();
    PLOG_WARNING << "Radio reset successfuly after failureDetected.";
}

void nrf24Read()
{

    PLOG_INFO << "MAIN LOOP STARTED";
    while (1)
    {
        // Failure Detection Routine
        mtx.lock();

        PLOG_FATAL_IF(radio.failureDetected) << "RF24 FAILED!!!!!";

        if (radio.failureDetected)
            nrf24Restart();

        // Iterate over pipes for incoming messages
        while (radio.available(&rfpipe))
        {
            if (rfpipe == 1) // Message from ControllerHub
            {
                // Read Message
                TActuatorData rdata;
                radio.read(&rdata, sizeof(rdata));
                // Push message into Queue
                // controllerHubData.push(rdata); -- save in file
                std::string msg = "type:0,water_consumption:" + std::to_string(rdata.water_comsumption) + ",reservoir_level:" + std::to_string(rdata.reservoir_level) + "\n";
                // mtxrf.lock();
                g_rfRecMsg.push(msg);
                // mtxrf.unlock();
                PLOG_VERBOSE << "ControllerHub Pipe " << (int)rfpipe << ": Recv: " << rdata.water_comsumption << " litres and reservoir level is " << (int)rdata.reservoir_level;
            }
            else if (rfpipe > 1 && rfpipe < 5) // Message from InGround Sensors
            {
                // Read Message
                struct ContextTag rtag;
                struct inGroundTag rdata;
                radio.read(&rtag, sizeof(rtag));
                // Push message into Queue
                rdata.tag = rtag;
                // rdata.rfAdress = pipes[pipe];
                rdata.rfAdress = rfpipe;
                // inGroundData.push(rdata); -- save in file
                std::string msg = "type:1,temperature:" + std::to_string(rdata.tag.temperature) + ",ground_humidity:" + std::to_string(rdata.tag.moisture) + ",battery_level:" + std::to_string(rdata.tag.battery) + ",rf_address:" + std::to_string(rdata.rfAdress) + "\n";
                // mtxrf.lock();
                g_rfRecMsg.push(msg);
                // mtxrf.unlock();
                PLOG_VERBOSE << "InGround Pipe " << (int)rfpipe << ": Recv: " << rtag.moisture << "% RH, " << rtag.temperature << " Celsius and " << rtag.battery << "% battery";
            }
        }

        mtx.unlock();

        delayMicroseconds(20);
        // Next Pipe...
        rfpipe += 1;
        rfpipe > 5 ? rfpipe = 1 : rfpipe = rfpipe;

        // Check flag for ControllerHub Testing
        if (testCHub)
            testControllerHub();
    }
}

//configureRadio: Configure RF24 radio
void configureRadio()
{
    radio.setAutoAck(true);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_HIGH);
    radio.setChannel(76);
    radio.setCRCLength(RF24_CRC_16);
    radio.setRetries(5, 15); // 5*250us delay with 15 retries
}
//configurePipes: Configure RF24 Pipes
void configurePipes()
{
    for (uint8_t i = 1; i < 6; i++)
        radio.openReadingPipe(i, pipes[i]);
    radio.openWritingPipe(pipes[0]);
}

void testControllerHub()
{
    int status, timer;
    TActuatorCommand cmd;
    bool sent;

    testCHub = false;
    std::cout << "** SEND TO CONTROLLER HUB **\n";
    std::cout << "Status (0-OFF / 1-ON) > ";
    std::cin >> status;
    std::cout << "Timer (s) > ";
    std::cin >> timer;
    cmd.status = (bool)status;
    cmd.timer = (uint16_t)timer;
    radio.stopListening();
    sent = radio.write(&cmd, sizeof(cmd));
    radio.startListening();
    if (sent)
        cout << "SENT.\n";
    else
        cout << "FAILED :(\n";
}

void signalHandler(int signum)
{
    testCHub = true;
}