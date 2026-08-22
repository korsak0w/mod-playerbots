/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ReachTargetActions.h"

#include "Creature.h"
#include "Event.h"
#include "Map.h"
#include "MotionMaster.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"

namespace
{
bool IsFleeingOrAssisting(Creature const* creature)
{
    if (creature->HasUnitState(UNIT_STATE_FLEEING))
        return true;

    switch (creature->GetMotionMaster()->GetCurrentMovementGeneratorType())
    {
        case FLEEING_MOTION_TYPE:
        case TIMED_FLEEING_MOTION_TYPE:
        case ASSISTANCE_MOTION_TYPE:
        case ASSISTANCE_DISTRACT_MOTION_TYPE:
            return true;
        default:
            return false;
    }
}
}

bool ReachTargetAction::Execute(Event /*event*/) { return ReachCombatTo(AI_VALUE(Unit*, GetTargetName()), distance); }

bool ReachTargetAction::isUseful()
{
    // do not move while staying
    if (botAI->HasStrategy("stay", botAI->GetState()))
    {
        return false;
    }

    // do not move while casting
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
    {
        return false;
    }
    Unit* target = GetTarget();
    // float dis = distance + CONTACT_DISTANCE;
    return target &&
           !bot->IsWithinCombatRange(target, distance);  // ServerFacade::instance().IsDistanceGreaterThan(AI_VALUE2(float,
                                                         // "distance", GetTargetName()), distance);
}

std::string const ReachTargetAction::GetTargetName() { return "current target"; }

bool ReachMeleeAction::Execute(Event event)
{
    if (sPlayerbotAIConfig.fleeingTargetMaxChaseDistance <= 0.0f)
    {
        ResetFleeingTargetChase();
        return ReachTargetAction::Execute(event);
    }

    Unit* target = AI_VALUE(Unit*, GetTargetName());
    Creature* creature = target ? target->ToCreature() : nullptr;
    Map* map = bot->GetMap();
    bool const limitChase = bot->IsInCombat() && creature && map && map->IsDungeon() && !map->IsRaid() &&
                            !creature->IsDungeonBoss() && !creature->isWorldBoss() &&
                            botAI->ContainsStrategy(STRATEGY_TYPE_MELEE) && IsFleeingOrAssisting(creature);

    if (!limitChase)
    {
        ResetFleeingTargetChase();
        return ReachTargetAction::Execute(event);
    }

    if (!fleeingAnchorSet || fleeingTargetGuid != creature->GetGUID() || fleeingAnchorMapId != bot->GetMapId())
    {
        fleeingTargetGuid = creature->GetGUID();
        fleeingAnchorMapId = bot->GetMapId();
        fleeingAnchorX = bot->GetPositionX();
        fleeingAnchorY = bot->GetPositionY();
        fleeingAnchorZ = bot->GetPositionZ();
        fleeingAnchorSet = true;
        fleeingLeashExceeded = false;
    }

    if (!fleeingLeashExceeded &&
        creature->GetExactDist2d(fleeingAnchorX, fleeingAnchorY) >
            sPlayerbotAIConfig.fleeingTargetMaxChaseDistance)
    {
        fleeingLeashExceeded = true;
        AI_VALUE(LastMovement&, "last movement").clear();
        bot->GetMotionMaster()->Clear(false);
        bot->StopMoving();
    }

    if (!fleeingLeashExceeded)
        return ReachTargetAction::Execute(event);

    if (bot->GetExactDist2d(fleeingAnchorX, fleeingAnchorY) > sPlayerbotAIConfig.followDistance)
    {
        // Normal combat and dungeon navigation can immediately override this return movement.
        MoveTo(fleeingAnchorMapId, fleeingAnchorX, fleeingAnchorY, fleeingAnchorZ, false, false, false, false,
               MovementPriority::MOVEMENT_IDLE, true);
    }

    return true;
}

void ReachMeleeAction::ResetFleeingTargetChase()
{
    fleeingTargetGuid = ObjectGuid::Empty;
    fleeingAnchorMapId = 0;
    fleeingAnchorSet = false;
    fleeingLeashExceeded = false;
}

bool CastReachTargetSpellAction::isUseful()
{
    // do not move while staying
    if (botAI->HasStrategy("stay", botAI->GetState()))
    {
        return false;
    }

    return ServerFacade::instance().IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"),
                                                (distance + sPlayerbotAIConfig.contactDistance));
}

ReachSpellAction::ReachSpellAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach spell", botAI->GetRange("spell"))
{
}

ReachPartyMemberToHealAction::ReachPartyMemberToHealAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach party member to heal", botAI->GetRange("heal"))
{
}

std::string const ReachPartyMemberToHealAction::GetTargetName() { return "party member to heal"; }

ReachPartyMemberToResurrectAction::ReachPartyMemberToResurrectAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach party member to resurrect", botAI->GetRange("spell"))
{
}

std::string const ReachPartyMemberToResurrectAction::GetTargetName() { return "party member to resurrect"; }
