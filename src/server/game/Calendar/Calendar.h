/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#ifndef TRINITY_CALENDAR_H
#define TRINITY_CALENDAR_H

#include "Common.h"

// TODO - Get correct values
enum CalendarEventType
{
    CALENDAR_TYPE_RAID          = 0,
    CALENDAR_TYPE_DUNGEON       = 1,
    CALENDAR_TYPE_PVP           = 2,
    CALENDAR_TYPE_MEETING       = 3,
    CALENDAR_TYPE_OTHER         = 4,
};

// TODO - Get correct values
enum CalendarInviteStatus
{
    CALENDAR_STATUS_INVITED     = 0,
    CALENDAR_STATUS_ACCEPTED    = 1,
    CALENDAR_STATUS_DECLINED    = 2,
    CALENDAR_STATUS_TENTATIVE   = 3,
    CALENDAR_STATUS_OUT         = 4,
    CALENDAR_STATUS_STANDBY     = 5,
    CALENDAR_STATUS_CONFIRMED   = 6,
};

struct CalendarEvent
{
    uint64 id;
    uint64 creator_guid;
    std::string name;
    std::string description;
    uint8 type;
    uint8 unk;
    uint32 dungeonID;
    uint32 unkTime;
    uint32 time;
    uint32 flags;
    uint32 guildID;
};

struct CalendarInvite
{
    uint64 id;
    uint64 event_id;
    uint8 status;
    uint8 rank;
    uint8 type;
    uint8 unk2;
    uint8 unk3;
    std::string text;
    uint64 creator_guid;
    uint32 time;
    uint64 target_guid;
};

typedef UNORDERED_MAP<uint64, CalendarInvite> CalendarInviteMap;
typedef UNORDERED_MAP<uint64, CalendarEvent> CalendarEventMap;

#endif
