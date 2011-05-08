#include "IRC.h"
#include "ChannelMgr.h"
#include "Channel.h"

IrcBot::IrcBot()
{}

IrcBot::~IrcBot()
{}

bool IrcBot::Connect()
{
    // sScriptMgr->OnIrcConnect();
    return true;
}

bool IrcBot::Disconnect()
{
    // sScriptMgr->OnIrcDisonnect();
    return true;
}

bool IrcBot::HookChannel(char const* channel)
{
    if(ChannelMgr *c = channelMgr(0))
    {
        if(Channel *chn = c->GetChannel(channel, 0, false))
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

    if(!found)
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
