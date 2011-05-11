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
-- NPC talk text insert from sniff 
DELETE FROM `creature_text` WHERE `entry`=40429; 
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES 
(40429,0,0, 'Thank you! I could not have held out for much longer.... A terrible thing has happened here.',1,0,100,5,0,17491, 'Sanctum Guardian Xerestrasza'),
(40429,1,0, 'We believed the Sanctum was well-fortified, but we were not prepared for the nature of this assault.',0,0,100,1,0,17492, 'Sanctum Guardian Xerestrasza'),
(40429,2,0, 'The Black dragonkin materialized from thin air, and set upon us before we could react.',0,0,100,1,0,17493, 'Sanctum Guardian Xerestrasza'),
(40429,3,0, 'We did not stand a chance. As my brethren perished around me, I managed to retreat here and bar the entrance.',0,0,100,1,0,17494, 'Sanctum Guardian Xerestrasza'),
(40429,4,0, 'They slaughtered us with cold efficiency, but the true focus of their interest seemed to be the eggs kept here in the Sanctum.',0,0,100,1,0,17495, 'Sanctum Guardian Xerestrasza'),
(40429,5,0, 'The commander of the forces on the ground here is a cruel brute named Zarithrian, but I fear there are greater powers at work.',0,0,100,1,0,17496, 'Sanctum Guardian Xerestrasza'),
(40429,6,0, 'In their initial assault, I caught a glimpse of their true leader, a fearsome full-grown twilight dragon.',0,0,100,1,0,17497, 'Sanctum Guardian Xerestrasza'),
(40429,7,0, 'I know not the extent of their plans, heroes, but I know this:  They cannot be allowed to succeed!',0,0,100,5,0,17498, 'Sanctum Guardian Xerestrasza'),
(40429,8,0, 'Help! I am trapped within this tree!  I require aid!',1,0,100,5,0,17490, 'Sanctum Guardian Xerestrasza');
*/

enum Texts
{
    SAY_XERESTRASZA_EVENT       = 0,
    SAY_XERESTRASZA_EVENT_1     = 1,
    SAY_XERESTRASZA_EVENT_2     = 2,
    SAY_XERESTRASZA_EVENT_3     = 3,
    SAY_XERESTRASZA_EVENT_4     = 4,
    SAY_XERESTRASZA_EVENT_5     = 5,
    SAY_XERESTRASZA_EVENT_6     = 6,
    SAY_XERESTRASZA_EVENT_7     = 7,
    SAY_XERESTRASZA_INTRO       = 8,
};

enum Events
{
    EVENT_XERESTRASZA_EVENT_1   = 1,
    EVENT_XERESTRASZA_EVENT_2   = 2,
    EVENT_XERESTRASZA_EVENT_3   = 3,
    EVENT_XERESTRASZA_EVENT_4   = 4,
    EVENT_XERESTRASZA_EVENT_5   = 5,
    EVENT_XERESTRASZA_EVENT_6   = 6,
    EVENT_XERESTRASZA_EVENT_7   = 7,
};

class npc_xerestrasza : public CreatureScript
{
    public:
        npc_xerestrasza() : CreatureScript("npc_xerestrasza") { }

        struct npc_xerestraszaAI : public ScriptedAI
        {
            npc_xerestraszaAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset()
            {
                _events.Reset();
                _isIntro = false;
                me->RemoveFlag(UNIT_NPC_FLAGS, GOSSIP_OPTION_QUESTGIVER);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_INTRO_TRIGGER)
                    Talk(SAY_XERESTRASZA_INTRO);
                else if (action == ACTION_BALTHARUS_DEATH)
                {
                    me->setActive(true);
                    _isIntro = true;
                    Talk(SAY_XERESTRASZA_EVENT);
                    me->GetMotionMaster()->MovePoint(0, 3151.236f, 379.8733f, 86.31996f);

                    // Timers from sniff
                    _events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_1, 16000);
                    _events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_2, 25000);
                    _events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_3, 32000);
                    _events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_4, 42000);
                    _events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_5, 51000);
                    _events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_6, 61000);
                    _events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_7, 69000);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!_isIntro)
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_XERESTRASZA_EVENT_1:
                            Talk(SAY_XERESTRASZA_EVENT_1);
                            break;
                        case EVENT_XERESTRASZA_EVENT_2:
                            Talk(SAY_XERESTRASZA_EVENT_2);
                            break;
                        case EVENT_XERESTRASZA_EVENT_3:
                            Talk(SAY_XERESTRASZA_EVENT_3);
                            break;
                        case EVENT_XERESTRASZA_EVENT_4:
                            Talk(SAY_XERESTRASZA_EVENT_4);
                            break;
                        case EVENT_XERESTRASZA_EVENT_5:
                            Talk(SAY_XERESTRASZA_EVENT_5);
                            break;
                        case EVENT_XERESTRASZA_EVENT_6:
                            Talk(SAY_XERESTRASZA_EVENT_6);
                            break;
                        case EVENT_XERESTRASZA_EVENT_7:
                            me->SetFlag(UNIT_NPC_FLAGS, GOSSIP_OPTION_QUESTGIVER);
                            Talk(SAY_XERESTRASZA_EVENT_7);
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap _events;
            bool _isIntro;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_xerestraszaAI(creature);
        }
};

// INSERT INTO `areatrigger_scripts` (`entry`,`ScriptName`) VALUES (5867, 'at_baltharus_plateau');
class at_baltharus_plateau : public AreaTriggerScript
{
    public:
        at_baltharus_plateau() : AreaTriggerScript("at_baltharus_plateau") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
        {
            if (InstanceScript* instance = player->GetInstanceScript())
            {
                if (Creature* xerestrasza = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_XERESTRASZA)))
                    xerestrasza->AI()->DoAction(ACTION_INTRO_TRIGGER);

                if (Creature* baltharus = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_BALTHARUS_THE_WARBORN)))
                    baltharus->AI()->DoAction(ACTION_INTRO_TRIGGER);
            }

            return true;
        }
};

void AddSC_ruby_sanctum()
{
    new at_baltharus_plateau();
    new npc_xerestrasza();
}
