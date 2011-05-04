#include <string>
#include <vector>

#include "Common.h"

#define IRC_SERVER  "irc.projectsjgr.com"
#define IRC_PORT    6667
#define IRC_CHANNEL "#ADMIN"

class IrcBot
{
    public:

        IrcBot();
        ~IrcBot();

        bool Connect();
        bool Disconnect();
        bool IsConnected() { return connected; }

        // Channels
        bool HookChannel(const char * channel);
        bool UnHookChannel(const char * channel);
        bool IsChannelHooked(const char * channel);
    private:
        bool connected; 
        std::vector<const char *> hooked_channels;
        std::string error_msg;
        std::vector<const char*>::iterator itr;
}