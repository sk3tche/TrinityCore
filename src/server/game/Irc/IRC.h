/*
 * Copyright (C) 2011-2011 ShitJustGotReal <http://www.projectsjgr.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
#define IRC_USER_NICK   "tib"   // Trinity IRC Bot (TIB)
#define IRC_USER    "Coded by SJGR"
#define IRC_PASS    "ircbot1"

enum MessageType
{
    NONE        = 0,
    PRIVMSG     = 1,
    USER        = 2,
    NICK        = 3,
    IDENTIFY    = 4,
    JOIN        = 5,
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
        bool Login();
        bool InitSocket();
        void Disconnect();
        bool IsConnected() { return _connected; }

        void ParseCommand(std::string nickName, char const* cmd, std::vector<char const*> args);

        // Channels
        bool HookChannel(char const* channel);
        bool UnhookChannel(char const* channel);
        bool IsChannelHooked(char const* channel);
        void SayToIRC(char const* channel, Player* player, char const* msg);
        bool SendData(MessageType type, char const* data);

        void SplitArgs(char const* arg, std::vector<char const*> &elems);

    private:
        std::vector<char const*> _hookedChannels;
        std::string error_msg;
        std::vector<char const*>::iterator _itr;
        bool _connected;
        int _socket;
};

#define sIrc ACE_Singleton<IrcBot, ACE_Null_Mutex>::instance()
#endif

