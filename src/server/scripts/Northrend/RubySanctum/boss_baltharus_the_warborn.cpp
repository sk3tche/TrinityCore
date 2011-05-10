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

enum Texts
{
    SAY_BALTHARUS_INTRO         = 0,    // Your power wanes, ancient one.... Soon you will join your friends.
    SAY_AGGRO                   = 1,    // Ah, the entertainment has arrived.
    SAY_SLAY                    = 2,    // Baltharus leaves no survivors! - This world has enough heroes.
    SAY_CLONE                   = 3,    // Twice the pain and half the fun.
    SAY_DEATH                   = 4,    // I... didn't see that coming....
};

enum Spells
{
    SPELL_BARRIER_CHANNEL       = 76221,
    SPELL_ENERVATING_BRAND      = 74502,
    SPELL_SIPHONED_MIGHT        = 74507,
    SPELL_CLEAVE                = 40504,
    SPELL_BLADE_TEMPEST         = 75125,
    SPELL_CLONE                 = 74511,
    SPELL_REPELLING_WAVE        = 74509,
    SPELL_CLEAR_DEBUFFS         = 34098,
    SPELL_SPAWN_EFFECT          = 64195,
};

enum Events
{
    EVENT_BLADE_TEMPEST         = 0,
    EVENT_CLEAVE                = 1,
    EVENT_ENERVATING_BRAND      = 2,
    EVENT_INTRO_TALK            = 3,
    EVENT_OOC_CHANNEL           = 4,
};

enum Actions
{
    ACTION_CLONE                = 0,
};

class boss_baltharus_the_warborn : public CreatureScript
{
    public:
        boss_baltharus_the_warborn() : CreatureScript("boss_baltharus_the_warborn") { }

        struct boss_baltharus_the_warbornAI : public BossAI
        {
            boss_baltharus_the_warbornAI(Creature* creature) : BossAI(creature, DATA_BALTHARUS_THE_WARBORN)
            {
            }

            void Reset()
            {
                _Reset();
                _isOutOfCombat = true;
                _events.ScheduleEvent(EVENT_OOC_CHANNEL, 0);
                _cloneCount = RAID_MODE<uint8>(1, 2, 2, 3);
            }

            void DoAction(int32 const action)
            {
                switch(action)
                {
                    case ACTION_INTRO_TRIGGER:
                        _events.ScheduleEvent(EVENT_INTRO_TALK, 9000);
                        break;
                    case ACTION_CLONE:
                    {
                        DoCast(me, SPELL_REPELLING_WAVE);
                        Talk(SAY_CLONE);
                        DoCast(me, SPELL_CLONE);
                        DoCast(me, SPELL_CLEAR_DEBUFFS);
                        --_cloneCount;
                        break;
                    }
                    default:
                        break;
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                _isOutOfCombat = false;
                _events.ScheduleEvent(EVENT_CLEAVE, 11000);
                _events.ScheduleEvent(EVENT_ENERVATING_BRAND, 13000);
                _events.ScheduleEvent(EVENT_BLADE_TEMPEST, 15000);
                Talk(SAY_AGGRO);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                if (Creature* xerestrasza = Unit::GetCreature(*me, instance->GetData64(DATA_XERESTRASZA)))
                    xerestrasza->AI()->DoAction(ACTION_BALTHARUS_DEATH);
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage)
            {
                if (GetDifficulty() != RAID_DIFFICULTY_10MAN_NORMAL)
                {
                    if (me->HealthBelowPctDamaged(66, damage) && _cloneCount == 2)
                        DoAction(ACTION_CLONE);
                    else if (me->HealthBelowPctDamaged(33, damage) && _cloneCount == 1)
                        DoAction(ACTION_CLONE);
                }
                else if (GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC)
                {
                    if (me->HealthBelowPctDamaged(75, damage) && _cloneCount == 3)
                        DoAction(ACTION_CLONE);
                    else if (me->HealthBelowPctDamaged(50, damage) && _cloneCount == 2)
                        DoAction(ACTION_CLONE);
                    else if (me->HealthBelowPctDamaged(25, damage) && _cloneCount == 1)
                        DoAction(ACTION_CLONE);
                }
                else
                {
                    if (me->HealthBelowPctDamaged(50, damage) && _cloneCount == 1)
                        DoAction(ACTION_CLONE);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() && !_isOutOfCombat)
                    return;

                _events.Update(diff);

                // These events occur out of combat
                if (_isOutOfCombat)
                {
                    if (_events.ExecuteEvent() == EVENT_OOC_CHANNEL)
                    {
                        if (Creature* channelTarget = Unit::GetCreature(*me, instance->GetData64(DATA_CRYSTAL_CHANNEL_TARGET)))
                            DoCast(channelTarget, SPELL_BARRIER_CHANNEL);
                        _events.ScheduleEvent(EVENT_OOC_CHANNEL, 7000);
                    }
                    else if (_events.ExecuteEvent() == EVENT_INTRO_TALK)
                        Talk(SAY_BALTHARUS_INTRO);

                    return;
                }

                if (me->HasUnitState(UNIT_STAT_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CLEAVE:
                            DoCastVictim(SPELL_CLEAVE);
                            _events.ScheduleEvent(EVENT_CLEAVE, 24000);
                            break;
                        case EVENT_ENERVATING_BRAND:
                            for (uint8 i = 0; i < RAID_MODE<uint8>(4, 8, 8, 10); i++)
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 45.0f, true))
                                    DoCast(target, SPELL_ENERVATING_BRAND);
                            _events.ScheduleEvent(EVENT_ENERVATING_BRAND, 26000);
                            break;
                        case EVENT_BLADE_TEMPEST:
                            DoCast(me, SPELL_BLADE_TEMPEST);
                            _events.ScheduleEvent(EVENT_BLADE_TEMPEST, 24000);
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
            uint8 _cloneCount;
            bool _isOutOfCombat;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_baltharus_the_warbornAI(creature);
        }
};

// 74502
class spell_baltharus_enervating_brand : public SpellScriptLoader
{
    public:
        spell_baltharus_enervating_brand() : SpellScriptLoader("spell_baltharus_enervating_brand") { }

        class spell_baltharus_enervating_brand_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_baltharus_enervating_brand_AuraScript);

            void HandleTriggerSpell(AuraEffect const* aurEff)
            {
                PreventDefaultAction();
                if (Unit* caster = GetCaster())
                {
                    uint32 triggerSpellId = GetSpellProto()->EffectTriggerSpell[aurEff->GetEffIndex()];
                    caster->CastSpell(caster, triggerSpellId, true);
                    if (Unit* originalCaster = aurEff->GetBase()->GetUnitOwner())
                        if (caster->GetDistance(originalCaster) <= 12.0f)
                            caster->CastSpell(originalCaster, SPELL_SIPHONED_MIGHT, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_baltharus_enervating_brand_AuraScript::HandleTriggerSpell, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_baltharus_enervating_brand_AuraScript();
        }
};

// 74511
class spell_baltharus_summon_clone : public SpellScriptLoader
{
    public:
        spell_baltharus_summon_clone() : SpellScriptLoader("spell_baltharus_summon_clone") { }

        class spell_baltharus_summon_clone_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_baltharus_summon_clone_SpellScript);

            void HandleSummon(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                InstanceScript* instance = caster->GetInstanceScript();
                if (!instance)
                    return;

                Creature* baltharus = Unit::GetCreature(*caster, instance->GetData64(DATA_BALTHARUS_THE_WARBORN));
                if (!baltharus)
                    return;

                uint32 entry = uint32(GetSpellInfo()->EffectMiscValue[effIndex]);
                SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(uint32(GetSpellInfo()->EffectMiscValueB[effIndex]));
                uint32 duration = uint32(GetSpellDuration(GetSpellInfo()));

                Position pos;
                caster->GetPosition(&pos);
                TempSummon* summon = caster->GetMap()->SummonCreature(entry, pos, properties, duration, caster);
                if (!summon)
                    return;

                summon->SetHealth(caster->GetHealth());
                summon->CastSpell(summon, SPELL_SPAWN_EFFECT, true);

                summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellInfo()->Id);
                summon->SetCreatorGUID(caster->GetGUID());
                baltharus->AI()->JustSummoned(summon);
            }

            void Register()
            {
                OnEffect += SpellEffectFn(spell_baltharus_summon_clone_SpellScript::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_baltharus_summon_clone_SpellScript();
        }
};

class npc_baltarhus_the_warborn_clone : public CreatureScript
{
    public:
        npc_baltarhus_the_warborn_clone() : CreatureScript("npc_baltarhus_the_warborn_clone") { }

        struct npc_baltarhus_the_warborn_cloneAI : public ScriptedAI
        {
            npc_baltarhus_the_warborn_cloneAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                _events.ScheduleEvent(EVENT_CLEAVE, urand(5000, 10000));
                _events.ScheduleEvent(EVENT_BLADE_TEMPEST, urand(18000, 25000));
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STAT_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CLEAVE:
                            DoCastVictim(SPELL_CLEAVE);
                            _events.ScheduleEvent(EVENT_CLEAVE, 24000);
                            break;
                        case EVENT_BLADE_TEMPEST:
                            DoCastVictim(SPELL_BLADE_TEMPEST);
                            _events.ScheduleEvent(EVENT_BLADE_TEMPEST, 24000);
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_baltarhus_the_warborn_cloneAI(creature);
        }
};

void AddSC_boss_baltharus_the_warborn()
{
    new boss_baltharus_the_warborn();
    new npc_baltarhus_the_warborn_clone();
    new spell_baltharus_enervating_brand();
    new spell_baltharus_summon_clone();
}
