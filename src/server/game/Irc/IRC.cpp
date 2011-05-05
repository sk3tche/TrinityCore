#include "IRC.h"

IrcBot::IrcBot()
{
}

IrcBot::~IrcBot()
{
}

bool IrcBot::Connect()
{
    return true;
}

bool IrcBot::Disconnect()
{
    // close sockets
    return true;
}

bool IrcBot::HookChannel(char const* channel)
{
    // TODO
    // Check if channel actually exists
    _hookedChannels.push_back(channel);
    return true;
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
