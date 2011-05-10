/*
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptPCH.h"
#include "ruby_sanctum.h"

/*
ClientToServer: CMSG_AREATRIGGER (0x00B4) Length: 4 Time: 06/30/2010 23:00:50
Area Trigger ID: 5867

ServerToClient: SMSG_PLAY_SOUND (0x02D2) Length: 4 Time: 06/30/2010 23:00:45
Sound ID: 17490

ServerToClient: SMSG_EMOTE (0x0103) Length: 12 Time: 06/30/2010 23:00:45
Emote ID: 5
GUID: Full: 0xF130009DED00BB55 Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 40429 Low: 3976248149

ServerToClient: SMSG_MESSAGECHAT (0x0096) Length: 116 Time: 06/30/2010 23:00:45
Type: MonsterYell
Language: Universal
GUID: Full: 0xF130009DED00BB55 Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 40429 Low: 3976248149
Unk Int32: 0
Name Length: 29
Name: Sanctum Guardian Xerestrasza
Receiver GUID: Full: 0xF130009DED00BB55 Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 40429 Low: 3976248149
Receiver Name Length: 53
Receiver Name: Help! I am trapped within this tree!  I require aid!


ServerToClient: SMSG_PLAY_SOUND (0x02D2) Length: 4 Time: 06/30/2010 23:00:54
Sound ID: 17525

ServerToClient: SMSG_MESSAGECHAT (0x0096) Length: 123 Time: 06/30/2010 23:00:54
Type: MonsterYell
Language: Universal
GUID: Full: 0xF130009B4700BB4A Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191230282
Unk Int32: 0
Name Length: 22
Name: Baltharus the Warborn
Receiver GUID: Full: 0xF130009B4700BB4A Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191230282
Receiver Name Length: 67
Receiver Name: Your power wanes, ancient one.... Soon you will join your friends.
*/
class at_baltharus_plateau : public AreaTriggerScript
{
    public:
        at_baltharus_plateau() : AreaTriggerScript("at_baltharus_plateau") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
        {
            if (InstanceScript* instance = player->GetInstanceScript())
            {
                //if (Creature* xerestrasza = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_XERESTRASZA)))
                //    xerestrasza->AI()->DoAction(ACTION_INTRO_TRIGGER);

                if (Creature* baltharus = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_BALTHARUS_THE_WARBORN)))
                    baltharus->AI()->DoAction(ACTION_INTRO_TRIGGER);
            }

            return true;
        }
};

void AddSC_ruby_sanctum()
{
    new at_baltharus_plateau();
}
