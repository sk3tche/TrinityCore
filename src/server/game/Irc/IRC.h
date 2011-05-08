
#ifndef _IRC_BOT_H
#define _IRC_BOT_H

#include <string>
#include <vector>
#include <ace/Singleton.h>
#include "Define.h"

#define IRC_SERVER  "irc.projectsjgr.com"
uint16 const IRC_PORT = 6667;
#define IRC_CHANNEL "#ADMIN"

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

        // Channels
        bool HookChannel(char const* channel);
        bool UnhookChannel(char const* channel);
        bool IsChannelHooked(char const* channel);
		void SayToChannel(char const* channel, uint64 p, char const* msg);
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

