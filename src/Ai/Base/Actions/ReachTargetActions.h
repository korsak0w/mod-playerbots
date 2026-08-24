/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_REACHTARGETACTIONS_H
#define PLAYERBOTS_REACHTARGETACTIONS_H

#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "ObjectGuid.h"

class PlayerbotAI;

class ReachTargetAction : public MovementAction
{
public:
    ReachTargetAction(PlayerbotAI* botAI, std::string const name, float distance)
        : MovementAction(botAI, name), distance(distance)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
    std::string const GetTargetName() override;

protected:
    float distance;
};

class CastReachTargetSpellAction : public CastSpellAction
{
public:
    CastReachTargetSpellAction(PlayerbotAI* botAI, std::string const spell, float distance)
        : CastSpellAction(botAI, spell), distance(distance)
    {
    }

    bool isUseful() override;

protected:
    float distance;
};

class ReachMeleeAction : public ReachTargetAction
{
public:
    ReachMeleeAction(PlayerbotAI* botAI) : ReachTargetAction(botAI, "reach melee", sPlayerbotAIConfig.meleeDistance) {}

    bool Execute(Event event) override;

private:
    void ResetFleeingTargetChase();

    ObjectGuid fleeingTargetGuid = ObjectGuid::Empty;
    uint32 fleeingAnchorMapId = 0;
    float fleeingAnchorX = 0.0f;
    float fleeingAnchorY = 0.0f;
    float fleeingAnchorZ = 0.0f;
    bool fleeingAnchorSet = false;
    bool fleeingLeashExceeded = false;
};

class ReachSpellAction : public ReachTargetAction
{
public:
    ReachSpellAction(PlayerbotAI* botAI);
};

class ReachPartyMemberToHealAction : public ReachTargetAction
{
public:
    ReachPartyMemberToHealAction(PlayerbotAI* botAI);

    std::string const GetTargetName() override;
};

class ReachPartyMemberToResurrectAction : public ReachTargetAction
{
public:
    ReachPartyMemberToResurrectAction(PlayerbotAI* botAI);

    std::string const GetTargetName() override;
};

#endif
