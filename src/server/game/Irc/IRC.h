
#ifndef _IRC_BOT_H
#define _IRC_BOT_H

#include <string>
#include <vector>
#include <ace/Singleton.h>
#include "Define.h"

#define IRC_SERVER  "irc.projectsjgr.com"
uint16 const IRC_PORT = 6667;
#define IRC_CHANNEL "#ADMIN"
#define IRC_NICK    "TrinIRC"
#define IRC_USER    "Coded by SJGR"
#define IRC_PASS    "ircbot1"

enum MessageType
{
    PRIVMSG = 0,
    USER    = 1,
    NICK    = 2,
    IDENTIFY = 3,
    JOIN    = 4
};

class IrcBot : public ACE_Based::Runnable
{
    friend class ACE_Singleton<IrcBot, ACE_Null_Mutex>;
    public:
        IrcBot();
        ~IrcBot();

        void run();

        void SockRecv();

        bool Connect();
        bool InitSocket();
        bool Disconnect();
        bool IsConnected() { return _connected; }

        void ParseCommand(char const* nick, std::vector<char const*> args);

        // Channels
        bool HookChannel(char const* channel);
        bool UnhookChannel(char const* channel);
        bool IsChannelHooked(char const* channel);
        void SayToIRC(char const* channel, Player* player, char const* msg);
        bool SendData(MessageType type, char const* data);

        void SplitArgs(char const* s, std::vector<char const*> & elems);
    protected:
        int _socket;
    private:
        std::vector<char const*> _hookedChannels;
        std::string error_msg;
        std::vector<char const*>::iterator _itr;
        bool _connected;
};

#define sIrc ACE_Singleton<IrcBot, ACE_Null_Mutex>::instance()
#endif

