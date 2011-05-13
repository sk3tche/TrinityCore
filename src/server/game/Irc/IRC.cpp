#include "IRC.h"
#include "ChannelMgr.h"
#include "Channel.h"
#include "World.h"
#include "Log.h"
#include "Common.h"
#include <sys/types.h>

#define MAXDATASIZE 512

IrcBot::IrcBot()
{
}

IrcBot::~IrcBot()
{
}

void IrcBot::run()
{
    // Let core breath out 500 ms after core startup :)
    ACE_Based::Thread::Sleep(500);
    sLog->outString("<IrcBot> - Starting IRC");

    // Create a loop while the worldserver thread is running
    while (!World::IsStopped())
    {
        // Initialize socket library
        if (InitSocket())
        {
            sLog->outString("<IrcBot> - Connecting...");
            if (Connect())
            {
                sLog->outString("<IrcBot> - Connected.");
                sLog->outString("<IrcBot> - Logging in to the IRC server...");
                if (Login())
                {
                    sLog->outString("<IrcBot> - Logged in sucessfully. Recieving data...");
                    while (IsConnected() && !World::IsStopped())
                    {
                        SockRecv();
                    }
                }
            }

            sLog->outError("<IrcBot> - Connection has been lost. Disconnecting");
            Disconnect();
            ACE_Based::Thread::Sleep(30 * IN_MILLISECONDS);
        }
        else
        {
            sLog->outError("<IrcBot> - Couldn't initialize socket.");
            ACE_Based::Thread::Sleep(10 * IN_MILLISECONDS);
        }
    }
}

bool IrcBot::Login()
{
    if (SendData(USER, IRC_USER))
    {
        if (SendData(NICK, IRC_NICK))
        {
            SockRecv();
            return true;
        }
        else
            sLog->outError("<IrcBot> - There was an error in SendData(NICK, IRC_NICK)");
    }
    else
        sLog->outError("<IrcBot> - There was an error in SendData(USER, IRC_USER)");

    return false;
}

bool IrcBot::Connect()
{
    // http://rabbit.eng.miami.edu/info/functions/internet.html#sockaddr_in
    _connected = false;

    // Check hostname
    hostent * record = gethostbyname(IRC_SERVER);
    if (record == NULL)
    {
        sLog->outError("<IrcBot> - Could not resolve host irc.projectsjgr.com");
        return false;
    }
    in_addr * addressptr = (in_addr *) record->h_addr;

    sockaddr_in serverInfo;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr = *addressptr;
    serverInfo.sin_port = htons(IRC_PORT);
    memset(&(serverInfo.sin_zero), '\0', 8);

    if (connect(_socket, (sockaddr *) &serverInfo, sizeof(sockaddr)) == -1)
    {
        sLog->outError("<IrcBot> - Cannot connect to irc.projectsjgr.com");
        return false;
    }
    _connected = true;
    return true;
}

bool IrcBot::InitSocket()
{
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0)
    {
        sLog->outError("<IrcBot> - Winsock initialization error");
        return false;
    }
    #endif
    if ((_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        sLog->outError("<IrcBot> - Socket can't be created");
        return false;
    }
    int on = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char const*) &on, sizeof (on)) == -1)
    {
        sLog->outError("<IrcBot> - Invalid Socket");
        return false;
    }
    #ifdef _WIN32
    u_long mode = 0;
    ioctlsocket(_socket, FIONBIO, &mode);
    #else
    fcntl(_socket, F_SETFL, O_NONBLOCK);                // set to non-blocking
    fcntl(_socket, F_SETFL, O_ASYNC);                   // set to asynchronous I/O
    #endif
    return true;
}

void IrcBot::Disconnect()
{
    if (_socket)
    {
        #ifdef _WIN32
        closesocket(_socket);
        #else
        close(_socket);
        #endif
    }

    // Remove all hooked channels
    _hookedChannels.clear();

    _connected = false;
}

bool IrcBot::HookChannel(char const* channel)
{
    if (ChannelMgr* chanMgr = channelMgr(0))
    {
        if (Channel* chn = chanMgr->GetChannel(channel, 0, false))
        {
            _hookedChannels.push_back(channel);
            return true;
        }
        else
            error_msg = "Channel does not exist!";
    }
    return false;
}

bool IrcBot::UnhookChannel(char const* channel)
{
    bool found;
    for (_itr = _hookedChannels.begin(); _itr != _hookedChannels.end(); _itr++)
    {
        if (!stricmp(channel, *_itr))
        {
            found = true;
            _hookedChannels.erase(_itr);
        }
    }

    if (!found)
        error_msg = "Cannot unhook channel, the channel wasn't hooked to begin with!";

    return found;
}

bool IrcBot::IsChannelHooked(char const* channel)
{
    for (_itr = _hookedChannels.begin(); _itr != _hookedChannels.end(); _itr++)
    {
        if (!stricmp(channel, *_itr))
            return true;
    }
    return false;
}

void IrcBot::SayToIRC(char const* channel, Player* player, char const* msg)
{
    std::stringstream ss;
    ss << "<" << channel << "> [" << player->GetName() << "] says: " << msg;
    std::string message = ss.str();
    SendData(PRIVMSG, message.c_str());
}

void IrcBot::SockRecv()
{
    char sizebuffer[MAXDATASIZE];

    memset(sizebuffer, 0, MAXDATASIZE);
    
    int recievedBytes = recv(_socket, sizebuffer, MAXDATASIZE - 1, 0);
    if (recievedBytes == -1)
    {
        sLog->outError("<IrcBot> - Connection lost");
        Disconnect();
    }
    else
    {
        if (-1 == recievedBytes)
            sLog->outError("<IrcBot> - Error while receiving from socket");
        else
        {
            std::string reply;
            std::istringstream iss(sizebuffer);

            while (getline(iss, reply))
            {
                // PING/PONG
                if (!strcmp(reply.substr(0, 4).c_str(), "PING"))
                {
                    reply.replace(1, 1, "O");
                    SendData(NONE, reply.c_str());
                    return;
                }

                // Join Channel
                //  - SJGR has no MOTD, so we need to look for MOTD File is Missing
                //  - Also make sure that it is not a PRIVMSG
                if(reply.find("MOTD File is missing") != std::string::npos &&
                    reply.find("PRIVMSG") == std::string::npos)
                {
                    SendData(IDENTIFY, IRC_PASS);
                    SendData(JOIN, IRC_CHANNEL);
                    return;
                }

                std::vector<char const*> args;
                SplitArgs(reply.c_str(), args);

                if (!stricmp(args[1], "PRIVMSG") && !stricmp(args[2], IRC_CHANNEL) && args[3][1] == '!')
                {
                    std::string command = args[3];
                    command.erase(0, 2); // erase : and ! from the commands
                    
                    std::vector<char const*> params;
                    params.push_back(command.c_str());

                    for (_itr = args.begin() + 4; _itr != args.end(); _itr++)
                        params.push_back(*_itr);

                    // Get nick (In between first : and first !
                    std::string nick = args[0];
                    nick.erase(0, 1);
                    std::stringstream nstream;
                    for (uint32 i = 0; nick[i] != '!'; i++)
                        nstream << nick[i];
                    std::string ntemp = nstream.str();
                    ParseCommand(ntemp.c_str(), params);
                }
            }
        }
    }
}

void IrcBot::SplitArgs(char const* arg, std::vector<char const*> & elems)
{
    std::stringstream ss(arg);
    std::string item;

    while (std::getline(ss, item, ' '))
        elems.push_back(item.c_str());
}

bool IrcBot::SendData(MessageType type, char const* data)
{
    std::stringstream ss;

    switch(type)
    {
        case USER:
            ss << "USER " << IRC_USER_NICK << " 0 * :" << data;
            break;
        case NICK:
            ss << "NICK " << data;
            break;
        case IDENTIFY:
            ss << "PRIVMSG NickServ :IDENTIFY " << data;
            break;
        case JOIN:
            ss << "JOIN " << data;
            break;
        case PRIVMSG:
            ss << "PRIVMSG " << IRC_CHANNEL << " :" << data;
            break;
        default:
            ss << data;
            break;
    }

    if (IsConnected())
    {
        std::string temp = ss.str();
        sLog->outString("<IrcBot> - Sending data: %s", temp.c_str());
        temp += "\r\n";
        char const* message = temp.c_str();
        if (send(_socket, message, strlen(message), 0) == -1)
            return false;
    }

    return true;
}

void IrcBot::ParseCommand(std::string nickName, std::vector<char const*> args)
{
    if (!args.size())
        return;

    if (!stricmp(args[0], "channel"))
    {
        if (args.size() < 3)
            SendData(PRIVMSG, "Not enough arguments! !channel HOOKED_CHANNEL message");
        else if (!IsChannelHooked(args[1]))
            SendData(PRIVMSG, "Channel is not hooked!");
        else
        {
            std::stringstream ss;
            for (_itr = args.begin()+2; _itr != args.end(); _itr++)
                ss << *_itr << " ";
            std::string temp = ss.str();
            if (ChannelMgr* cMgr = channelMgr(0))
            {
                if (Channel* chn = cMgr->GetChannel(args[1], 0, false))
                    chn->SayFromIRC(nickName.c_str(), temp.c_str());
            }
       }
    } 
    else if (!stricmp(args[0], "hook"))
    {
        if (args.size() < 2)
            SendData(PRIVMSG, "Not enough arguments! !hook CHANNEL_TO_HOOK");
        else if (IsChannelHooked(args[1]))
            SendData(PRIVMSG, "Channel is already hooked!");
        else
        {
            if (HookChannel(args[1]))
                SendData(PRIVMSG, "Channel successfully hooked!");
            else
                SendData(PRIVMSG, error_msg.c_str());
        }
    }
    else if (!stricmp(args[0], "unhook"))
    {
        if (args.size() < 2)
            SendData(PRIVMSG, "Not enough arguments! !unhook CHANNEL_TO_UNHOOK");
        else
        {
            if (UnhookChannel(args[1]))
                SendData(PRIVMSG, "Channel successfully unhooked!");
            else
                SendData(PRIVMSG, error_msg.c_str());
        }
    }
    else if (!stricmp(args[0], "hooked"))
    {
        if (_hookedChannels.size() <= 0)
            SendData(PRIVMSG, "No channels are hooked!");
        else
        {
            std::stringstream ss;
            ss << "The following channels are hooked:\n";
            for (_itr = _hookedChannels.begin(); _itr != _hookedChannels.end(); _itr++)
            {
                ss << " - " << *_itr << "\n";
            }
            ss << "End of List";
            std::string temp = ss.str();
            SendData(PRIVMSG, temp.c_str());
        }
    }
}
