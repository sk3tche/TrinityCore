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

class boss_saviana_ragefire : public CreatureScript
{
    public:
        boss_saviana_ragefire() : CreatureScript("boss_saviana_ragefire") { }

        struct boss_saviana_ragefireAI : public BossAI
        {
            boss_saviana_ragefireAI(Creature* creature) : BossAI(creature, DATA_SAVINA_RAGEFIRE)
            {
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_saviana_ragefireAI(creature);
        }
};

void AddSC_boss_saviana_ragefire()
{
    new boss_saviana_ragefire();
}
