#include <string>
#include <vector>

#define IRC_SERVER  "irc.projectsjgr.com"
#define IRC_PORT    6667
#define IRC_CHANNEL "#ADMIN"

class IrcBot : public ACE_Based::Runnable
{
    public:
        IrcBot();
        ~IrcBot();

        void run();

        bool Connect();
        bool Disconnect();
        bool IsConnected() { return _connected; }

        // Channels
        bool HookChannel(char const* channel);
        bool UnhookChannel(char const* channel);
        bool IsChannelHooked(char const* channel);
    private:
        bool _connected; 
        std::vector<char const*> _hookedChannels;
        std::string error_msg;
        std::vector<char const*>::iterator _itr;
};