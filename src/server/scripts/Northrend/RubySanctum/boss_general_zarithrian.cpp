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

#include "ScriptPCH.h"
#include "ruby_sanctum.h"

class boss_general_zarithrian : public CreatureScript
{
    public:
        boss_general_zarithrian() : CreatureScript("boss_general_zarithrian") { }

        struct boss_general_zarithrianAI : public BossAI
        {
            boss_general_zarithrianAI(Creature* creature) : BossAI(creature, DATA_GENERAL_ZARITHRIAN)
            {
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_general_zarithrianAI(creature);
        }
};

void AddSC_boss_general_zarithrian()
{
    new boss_general_zarithrian();
}
