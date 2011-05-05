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
    return true;
}

bool IrcBot::HookChannel(const char *channel)
{
    // TODO
    // Check if channel actually exists
    hooked_channels.push_back(channel);
}

bool IrcBot::UnHookChannel(const char *channel)
{
    bool found;
    for(itr = hooked_channels.begin(); itr != hooked_channels.end(); itr++)
    {
        if(!stricmp(channel, *itr))
        {
            found = true;
            hooked_channels.erase(itr);
        }
    }
    return found;
}

bool IrcBot::IsChannelHooked(const char *channel)
{
    for(itr = hooked_channels.begin(); itr != hooked_channels.end(); itr++)
    {
        if(!stricmp(channel, *itr))
            return true;
    }
    return false;
}