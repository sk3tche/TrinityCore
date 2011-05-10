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
    SAY_INTRO                   = 0,    // Your power wanes, ancient one.... Soon you will join your friends.
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
                        Talk(SAY_INTRO);

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

// Spellscript 74511. Add should spawn with same health as caster

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

/*
ServerToClient: SMSG_SPELL_START (0x0131) Length: 38 Time: 06/30/2010 21:34:46
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 76221
Cast Flags: Unknown1, Unknown3
Time: 0
Target Flags: Unit
Target GUID: Full: 0xF13000685800A51B Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 26712 Low: 1476437275

ServerToClient: SMSG_SPELL_START (0x0131) Length: 38 Time: 06/30/2010 21:34:53
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 76221
Cast Flags: Unknown1, Unknown3
Time: 0
Target Flags: Unit
Target GUID: Full: 0xF13000685800A51B Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 26712 Low: 1476437275

Enter combat 21:36:13
Update Type: Values
GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Block Count: 5
Block Value 18: 43113058/2.143118E-37
Block Value 19: 83886080/6.018531E-36
Block Value 20: 0/0
Block Value 21: 0/0
Block Value 22: 0/0
Block Value 59: 2112/2.959542E-42

ServerToClient: SMSG_MESSAGECHAT (0x0096) Length: 91 Time: 06/30/2010 21:36:13
Type: MonsterYell
Language: Universal
GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Unk Int32: 0
Name Length: 22
Name: Baltharus the Warborn
Receiver GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Receiver Name Length: 35
Receiver Name: Ah, the entertainment has arrived.

ServerToClient: SMSG_SPELL_START (0x0131) Length: 37 Time: 06/30/2010 21:36:24
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 40504
Cast Flags: Unknown1
Time: 0
Target Flags: Unit
Target GUID: Full: 0x50000000291DA62 Flags: Flag01, Flag04 Type: NoEntry1 Entry: 0 Low: 43113058

ServerToClient: SMSG_SPELL_START (0x0131) Length: 37 Time: 06/30/2010 21:36:25
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 74502
Cast Flags: Unknown1
Time: 0
Target Flags: Unit
Target GUID: Full: 0x500000001D34D74 Flags: Flag01, Flag04 Type: NoEntry1 Entry: 0 Low: 30625140

ServerToClient: SMSG_SPELL_START (0x0131) Length: 31 Time: 06/30/2010 21:36:32
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 75125
Cast Flags: Unknown1
Time: 0
Target Flags: Self

ServerToClient: SMSG_SPELL_START (0x0131) Length: 37 Time: 06/30/2010 21:36:48
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 40504
Cast Flags: Unknown1
Time: 0
Target Flags: Unit
Target GUID: Full: 0x50000000291DA62 Flags: Flag01, Flag04 Type: NoEntry1 Entry: 0 Low: 43113058

ServerToClient: SMSG_SPELL_START (0x0131) Length: 37 Time: 06/30/2010 21:36:51
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 74502
Cast Flags: Unknown1
Time: 0
Target Flags: Unit
Target GUID: Full: 0x500000001D34D74 Flags: Flag01, Flag04 Type: NoEntry1 Entry: 0 Low: 30625140

ServerToClient: SMSG_SPELL_START (0x0131) Length: 31 Time: 06/30/2010 21:36:56
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 75125
Cast Flags: Unknown1
Time: 0
Target Flags: Self

ServerToClient: SMSG_SPELL_START (0x0131) Length: 31 Time: 06/30/2010 21:37:07
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 74509
Cast Flags: Unknown1, Unknown3
Time: 1000
Target Flags: Self

ServerToClient: SMSG_SPELL_GO (0x0132) Length: 95 Time: 06/30/2010 21:37:08
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 74509
Cast Flags: Unknown3, Unknown7, Unknown12
Time: -1966424103
Hit Count: 6
Hit GUID 0: Full: 0x50000000291DA62 Flags: Flag01, Flag04 Type: NoEntry1 Entry: 0 Low: 43113058
Hit GUID 1: Full: 0x50000000002499E Flags: Flag01, Flag04 Type: NoEntry1 Entry: 0 Low: 149918
Hit GUID 2: Full: 0xF1404803B1000755 Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Pet Entry: 4719537 Low: 2969569109
Hit GUID 3: Full: 0xF1404B5E3D000758 Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Pet Entry: 4939325 Low: 1023412056
Hit GUID 4: Full: 0x500000002E8E6E0 Flags: Flag01, Flag04 Type: NoEntry1 Entry: 0 Low: 48817888
Hit GUID 5: Full: 0x500000002F344FE Flags: Flag01, Flag04 Type: NoEntry1 Entry: 0 Low: 49497342

ServerToClient: SMSG_PLAY_SOUND (0x02D2) Length: 4 Time: 06/30/2010 21:37:08
Sound ID: 17524

ServerToClient: SMSG_SPELL_START (0x0131) Length: 31 Time: 06/30/2010 21:37:08
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 74511
Cast Flags: Unknown1
Time: 0
Target Flags: Self

ServerToClient: SMSG_SPELL_GO (0x0132) Length: 47 Time: 06/30/2010 21:37:08
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 74511
Cast Flags: Unknown7, Unknown8
Time: -1966422898
Hit Count: 0
Miss Count: 0
Target Flags: DestinationLocation
Target GUID: Full: 0x00000000 Flags: None Type: NoEntry1 Entry: 0 Low: 0
Position: X: 3153.091 Y: 387.059 Z: 86.36932
Unk Byte 2: 1

ServerToClient: SMSG_SPELL_START (0x0131) Length: 31 Time: 06/30/2010 21:37:08
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 34098
Cast Flags: Unknown1
Time: 0
Target Flags: Self

ServerToClient: SMSG_MESSAGECHAT (0x0096) Length: 89 Time: 06/30/2010 21:37:08
Type: MonsterYell
Language: Universal
GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Unk Int32: 0
Name Length: 22
Name: Baltharus the Warborn
Receiver GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Text Length: 33
Text: Twice the pain and half the fun.
Chat Tag: None

ServerToClient: SMSG_SPELL_START (0x0131) Length: 37 Time: 06/30/2010 21:37:12
Caster GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Caster Unit GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Cast Count: 0
Spell ID: 40504
Cast Flags: Unknown1
Time: 0
Target Flags: Unit
Target GUID: Full: 0x50000000291DA62 Flags: Flag01, Flag04 Type: NoEntry1 Entry: 0 Low: 43113058

ServerToClient: SMSG_PLAY_SOUND (0x02D2) Length: 4 Time: 06/30/2010 21:38:55
Sound ID: 17521

ServerToClient: SMSG_MESSAGECHAT (0x0096) Length: 87 Time: 06/30/2010 21:38:55
Type: MonsterYell
Language: Universal
GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Unk Int32: 0
Name Length: 22
Name: Baltharus the Warborn
Receiver GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Receiver Name Length: 31
Receiver Name: Baltharus leaves no survivors!

Eller     This world has enough heroes. (soundid 17522)

ServerToClient: SMSG_PLAY_SOUND (0x02D2) Length: 4 Time: 06/30/2010 21:38:57
Sound ID: 17523

ServerToClient: SMSG_MESSAGECHAT (0x0096) Length: 88 Time: 06/30/2010 21:38:57
Type: MonsterYell
Language: Universal
GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Unk Int32: 0
Name Length: 22
Name: Baltharus the Warborn
Receiver GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Receiver Name Length: 32
Receiver Name: I... didn't see that coming....

ServerToClient: SMSG_DESTROY_OBJECT (0x00AA) Length: 9 Time: 06/30/2010 21:40:03
GUID: Full: 0xF130009B4700C1DF Flags: Flag01, Flag10, Flag20, Flag40, Flag80 Type: Unit Entry: 39751 Low: 1191231967
Despawn Animation: True

*/

/*
SAI?

class npc_baltarhus_the_warborn_clone : public CreatureScript
{
    public:
        npc_baltarhus_the_warborn_clone() : CreatureScript("npc_baltarhus_the_warborn_clone") { }

        struct npc_baltarhus_the_warborn_cloneAI : public ScriptedAI
        {
            npc_baltarhus_the_warborn_cloneAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void EnterCombat(Unit* who)
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

*/

void AddSC_boss_baltharus_the_warborn()
{
    new boss_baltharus_the_warborn();
    //new npc_baltarhus_the_warborn_clone();
    new spell_baltharus_enervating_brand();
    new spell_baltharus_summon_clone();
}
