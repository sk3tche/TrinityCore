#include "ScriptPCH.h"

class player_script_sjgr : public PlayerScript
{
    public:
        player_script_sjgr() : PlayerScript("player_script_sjgr") {}

        void OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type)
        {
            // Check that duel have ended properly (fled or won)
            if (type != DUEL_INTERUPTED)
            {
                winner->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
                loser->HandleEmoteCommand(EMOTE_ONESHOT_BEG);
            }
        }

        //void OnPVPKill(Player* /*killer*/, Player* /*killed*/) { }
        //void OnLevelChanged(Player* /*player*/, uint8 /*newLevel*/) { }
        //void OnLogin(Player* /*player*/) { }
        //void OnLogout(Player* /*player*/) { }
        //void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Channel* /*channel*/) { }
};

void AddSC_SJGR_scripts()
{
    new player_script_sjgr();
}
