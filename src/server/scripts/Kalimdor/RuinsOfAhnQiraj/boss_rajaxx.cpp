/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
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

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ruins_of_ahnqiraj.h"

enum Yells
{
    // The time of our retribution is at hand! Let darkness reign in the hearts of our enemies! Sound: 8645 Emote: 35
    SAY_ANDOROV_INTRO         = -1509003,   // Before for the first wave
    SAY_ANDOROV_ATTACK        = -1509004,   // Beginning the event
    SAY_ANDOROV_WAVE_1        = -1509001,   // When the first wave comes text: Kill first, ask questions later... Incoming! emote: 45 sound: 8653
    SAY_WAVE3                 = -1509005,
    SAY_WAVE4                 = -1509006,
    SAY_WAVE5                 = -1509007,
    SAY_WAVE6                 = -1509008,
    SAY_WAVE7                 = -1509009,
    SAY_INTRO                 = -1509010,
    SAY_UNK1                  = -1509011,
    SAY_UNK2                  = -1509012,
    SAY_UNK3                  = -1509013,
    SAY_DEATH                 = -1509014,
    SAY_CHANGEAGGRO           = -1509015,
    SAY_KILLS_ANDOROV         = -1509016,
    SAY_COMPLETE_QUEST        = -1509017                        // Yell when realm complete quest 8743 for world event
    // Warriors, Captains, continue the fight! Sound: 8640
};

enum Spells
{
    SPELL_DISARM            = 6713,
    SPELL_FRENZY            = 8269,
    SPELL_THUNDERCRASH      = 25599
};

enum Events
{
    EVENT_DISARM            = 1,        // 03:58:27, 03:58:49
    EVENT_THUNDERCRASH      = 2,        // 03:58:29, 03:58:50
    EVENT_CHANGE_AGGRO      = 3,
};

//Waypoints
/*
Waypoint 1: X: 9122.947 Y: -1600.251 Z: -27.77769
Waypoint 2: X: 9113.947 Y: -1596.751 Z: -22.27769
Waypoint 3: X: 9112.697 Y: -1595.501 Z: -22.02769
Waypoint 4: X: 9110.197 Y: -1592.001 Z: -22.02769
Waypoint 5: X: 9110.197 Y: -1587.751 Z: -22.02769
Waypoint 6: X: 9110.197 Y: -1583.501 Z: -22.02769
Waypoint 7: X: 9099.697 Y: -1575.251 Z: -22.02769
Waypoint 8: X: 9083.197 Y: -1579.501 Z: -22.02769
Waypoint 9: X: 9074.697 Y: -1579.501 Z: -22.02769
Waypoint 10: X: 9066.447 Y: -1581.501 Z: -22.02769
Waypoint 11: X: 9063.197 Y: -1579.501 Z: -22.77769
Waypoint 12: X: 9062.197 Y: -1578.251 Z: -22.77769
Waypoint 13: X: 9060.197 Y: -1578.251 Z: -22.77769
Waypoint 14: X: 9058.197 Y: -1578.251 Z: -22.77769
Waypoint 15: X: 9055.947 Y: -1578.251 Z: -22.77769
Waypoint 16: X: 9053.947 Y: -1578.251 Z: -22.77769
Waypoint 17: X: 9051.947 Y: -1578.251 Z: -22.77769
Waypoint 18: X: 9049.697 Y: -1578.251 Z: -22.77769
Waypoint 19: X: 9047.697 Y: -1578.251 Z: -22.77769
Waypoint 20: X: 9045.697 Y: -1578.251 Z: -22.77769
Waypoint 21: X: 9043.447 Y: -1578.251 Z: -22.77769
Waypoint 22: X: 9041.447 Y: -1578.251 Z: -22.77769
Waypoint 23: X: 9039.447 Y: -1578.251 Z: -22.52769
Waypoint 24: X: 9037.197 Y: -1578.251 Z: -22.52769
Waypoint 25: X: 9033.197 Y: -1577.251 Z: -22.27769
*/
// http://www.youtube.com/watch?v=FOQaLSI5-Go

class boss_rajaxx : public CreatureScript
{
    public:
        boss_rajaxx() : CreatureScript("boss_rajaxx") { }

        struct boss_rajaxxAI : public BossAI
        {
            boss_rajaxxAI(Creature* creature) : BossAI(creature, BOSS_RAJAXX)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset()
            {
                BossAI::Reset();
                enraged = false;
                events.ScheduleEvent(EVENT_DISARM, 10000);
                events.ScheduleEvent(EVENT_THUNDERCRASH, 12000);
            }

            void JustDied(Unit* killer)
            {
                //SAY_DEATH
                BossAI::JustDied(killer);
            }

            void EnterCombat(Unit* victim)
            {
                BossAI::EnterCombat(victim);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STAT_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DISARM:
                            DoCastVictim(SPELL_DISARM);
                            events.ScheduleEvent(EVENT_DISARM, 22000);
                            break;
                        case EVENT_THUNDERCRASH:
                            DoCast(me, SPELL_THUNDERCRASH);
                            events.ScheduleEvent(EVENT_THUNDERCRASH, 21000);
                            break;
                        default:
                            break;
                    }
                }
 
                DoMeleeAttackIfReady();
            }
            private:
                InstanceScript* instance;
                EventMap events;
                bool enraged;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_rajaxxAI (creature);
        }
};

void AddSC_boss_rajaxx()
{
    new boss_rajaxx();
}
