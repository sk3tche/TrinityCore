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
    SAY_AGGRO                           = 0, // You will sssuffer for this intrusion! (17528)
    SAY_CONFLAGRATION                   = 1, // Burn in the master's flame! (17532)
    EMOTE_ENRAGED                       = 2, // %s becomes enraged!
    SAY_KILL                            = 3, // Halion will be pleased. (17530) - As it should be.... (17529)
};

enum Spells
{
    SPELL_CONFLAGRATION         = 74452,
    SPELL_FLAME_BEACON          = 74453,
    SPELL_CONFLAGRATION_2       = 74454, // Unknown dummy effect
    SPELL_ENRAGE                = 78722,
    SPELL_FLAME_BREATH          = 74403,
};

enum Events
{
    EVENT_ENRAGE                    = 1,
    EVENT_LAND                      = 2,
    EVENT_AIR_PHASE                 = 3,
    EVENT_FLAME_BREATH              = 4,
};

enum MovementPoints
{
    POINT_AIR_PHASE         = 1,
    POINT_LAND              = 2,
};

enum Misc
{
    SOUND_ID_DEATH          = 17531,
};

Position const SavianaRagefireFlyPos  = {3156.324f, 643.1251f, 88.5247f, 4.69f};
Position const SavianaRagefireLandPos = {3155.510f, 683.8440f, 95.5070f, 4.69f};

class boss_saviana_ragefire : public CreatureScript
{
    public:
        boss_saviana_ragefire() : CreatureScript("boss_saviana_ragefire") { }

        struct boss_saviana_ragefireAI : public BossAI
        {
            boss_saviana_ragefireAI(Creature* creature) : BossAI(creature, DATA_SAVINA_RAGEFIRE)
            {
            }

            void Reset()
            {
                BossAI::Reset();
                me->SetReactState(REACT_DEFENSIVE);
                _events.ScheduleEvent(EVENT_ENRAGE, 10000);
                _events.ScheduleEvent(EVENT_FLAME_BREATH, 10000);
                _events.ScheduleEvent(EVENT_AIR_PHASE, 20000);
            }

            void EnterCombat(Unit* victim)
            {
                BossAI::EnterCombat(victim);
                Talk(SAY_AGGRO);
            }

            void JustDied(Unit* killer)
            {
                BossAI::JustDied(killer);
                me->PlayDirectSound(SOUND_ID_DEATH); // Only plays sound, no text
            }

            void MovementInform(uint32 type, uint32 point)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                switch (point)
                {
                    case POINT_AIR_PHASE:
                        DoCast(me, SPELL_CONFLAGRATION, true);
                        Talk(SAY_CONFLAGRATION);
                        _events.ScheduleEvent(EVENT_LAND, 8000);
                        break;
                    case POINT_LAND:
                        me->SetFlying(false);
                        me->RemoveUnitMovementFlag(MOVEMENTFLAG_LEVITATING);
                        me->SetReactState(REACT_AGGRESSIVE);
                        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
                            me->GetMotionMaster()->MovementExpired();
                        DoStartMovement(me->getVictim());
                        break;
                    default:
                        break;
                }
            }

            void JustReachedHome()
            {
                BossAI::JustReachedHome();
                instance->SetBossState(DATA_SAVINA_RAGEFIRE, FAIL);
                me->SetFlying(false);
                me->RemoveUnitMovementFlag(MOVEMENTFLAG_LEVITATING);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
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
                        case EVENT_AIR_PHASE:
                        {
                            me->SetFlying(true);
                            me->AddUnitMovementFlag(MOVEMENTFLAG_LEVITATING);
                            me->SetReactState(REACT_PASSIVE);
                            me->GetMotionMaster()->MovePoint(POINT_AIR_PHASE, SavianaRagefireFlyPos);
                            break;
                        }
                        case EVENT_LAND:
                            me->GetMotionMaster()->MovePoint(POINT_LAND, SavianaRagefireLandPos);
                            _events.ScheduleEvent(EVENT_AIR_PHASE, urand(15000, 20000));
                            break;
                        case EVENT_ENRAGE:
                            DoCast(me, SPELL_ENRAGE);
                            Talk(EMOTE_ENRAGED, me->GetGUID());
                            _events.ScheduleEvent(EVENT_ENRAGE, urand(15000, 20000));
                            break;
                        case EVENT_FLAME_BREATH:
                            DoCastVictim(SPELL_FLAME_BREATH);
                            _events.ScheduleEvent(EVENT_FLAME_BREATH, urand(15000, 20000));
                            break;
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
            return new boss_saviana_ragefireAI(creature);
        }
};

class ConflagrationTargetSelector
{
    public:
        ConflagrationTargetSelector() { }

        bool operator()(Unit* unit)
        {
            return unit->GetTypeId() != TYPEID_PLAYER;
        }
};

// 74452
class spell_saviana_conflagration_init : public SpellScriptLoader
{
    public:
        spell_saviana_conflagration_init() : SpellScriptLoader("spell_saviana_conflagration_init") { }

        class spell_saviana_conflagration_init_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_saviana_conflagration_init_SpellScript);

            void FilterTargets(std::list<Unit*>& unitList)
            {
                unitList.remove_if(ConflagrationTargetSelector());
                uint8 maxSize = uint8(GetCaster()->GetMap()->GetSpawnMode() & 1 ? 6 : 3);
                if (unitList.size() > maxSize)
                    Trinity::RandomResizeList(unitList, maxSize);
            }

            void HandleDummy(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetCaster()->CastSpell(GetHitUnit(), SPELL_FLAME_BEACON, true);
                GetCaster()->CastSpell(GetHitUnit(), SPELL_CONFLAGRATION_2, true);
            }

            void Register()
            {
                OnUnitTargetSelect += SpellUnitTargetFn(spell_saviana_conflagration_init_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_AREA_ENEMY_SRC);
                OnEffect += SpellEffectFn(spell_saviana_conflagration_init_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_saviana_conflagration_init_SpellScript();
        }
};

// 74453
class spell_saviana_conflagration_trigger : public SpellScriptLoader
{
    public:
        spell_saviana_conflagration_trigger() : SpellScriptLoader("spell_saviana_conflagration_trigger") { }

        class spell_saviana_conflagration_trigger_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_saviana_conflagration_trigger_AuraScript);

            void HandlePeriodicTick(AuraEffect const* aurEff)
            {
                PreventDefaultAction();
                uint32 triggerSpellId = GetSpellProto()->EffectTriggerSpell[aurEff->GetEffIndex()];

                if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                    if (Creature* saviana = Unit::GetCreature(*GetCaster(), instance->GetData64(DATA_SAVINA_RAGEFIRE)))
                        GetCaster()->CastSpell(saviana, triggerSpellId, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_saviana_conflagration_trigger_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_saviana_conflagration_trigger_AuraScript();
        }
};

// 74455
class spell_saviana_conflagration_throwback : public SpellScriptLoader
{
    public:
        spell_saviana_conflagration_throwback() : SpellScriptLoader("spell_saviana_conflagration_throwback") { }

        class spell_saviana_conflagration_throwback_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_saviana_conflagration_throwback_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetHitUnit()->CastSpell(GetCaster(), uint32(GetEffectValue()), true);
            }

            void Register()
            {
                OnEffect += SpellEffectFn(spell_saviana_conflagration_throwback_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_saviana_conflagration_throwback_SpellScript();
        }
};

void AddSC_boss_saviana_ragefire()
{
    new boss_saviana_ragefire();
    new spell_saviana_conflagration_init();
    new spell_saviana_conflagration_trigger();
    new spell_saviana_conflagration_throwback();
}
