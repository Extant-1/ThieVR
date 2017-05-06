/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5945 $ (Revision of last commit) 
 $Date: 2014-04-04 19:27:40 +0000 (Fri, 04 Apr 2014) $ (Date of last commit)
 $Author: grayman $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: IdleSleepState.cpp 5945 2014-04-04 19:27:40Z grayman $");

#include "IdleSleepState.h"
#include "IdleState.h"
#include "AlertIdleState.h"
#include "../Memory.h"
//#include "../Tasks/RandomHeadturnTask.h"
//#include "../Tasks/AnimalPatrolTask.h"
#include "../Tasks/SingleBarkTask.h"
#include "../Tasks/RepeatedBarkTask.h"
#include "../Tasks/MoveToPositionTask.h"
#include "../Tasks/IdleAnimationTask.h"
#include "../Tasks/WaitTask.h"
#include "ObservantState.h"
#include "../Library.h"

namespace ai
{

// Get the name of this state
const idStr& IdleSleepState::GetName() const
{
	static idStr _name(STATE_IDLE_SLEEP);
	return _name;
}

bool IdleSleepState::CheckAlertLevel(idAI* owner)
{
	if (owner->AI_AlertIndex > ERelaxed)
	{
		// Alert index is too high, switch to the higher State
		owner->GetMind()->PushState(owner->backboneStates[EObservant]);
		return false;
	}

	// Alert Index is matching, return OK
	return true;
}

void IdleSleepState::Init(idAI* owner)
{
	// Init base class first
	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("IdleSleepState initialised.\r");
	assert(owner);

	// Memory shortcut
	Memory& memory = owner->GetMemory();
	memory.alertClass = EAlertNone;
	memory.alertType = EAlertTypeNone;

	if (owner->HasSeenEvidence() && !owner->spawnArgs.GetBool("disable_alert_idle", "0"))
	{
		owner->GetMind()->SwitchState(STATE_ALERT_IDLE);
		return;
	}

	if (owner->GetMoveType() != MOVETYPE_SLEEP && 
		(!owner->ReachedPos(memory.idlePosition, MOVE_TO_POSITION) 
			|| owner->GetCurrentYaw() != memory.idleYaw))
	{
		// we need to get to the bed first before starting to sleep, back to idle state
		owner->GetMind()->SwitchState(owner->backboneStates[ERelaxed]);
		return;
	}

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;

	_startSleeping = owner->spawnArgs.GetBool("sleeping", "0");
	_startSitting = owner->spawnArgs.GetBool("sitting", "0");

	_alertLevelDecreaseRate = 0.01f;

	// owner->SheathWeapon();

	owner->actionSubsystem->ClearTasks();
	owner->senseSubsystem->ClearTasks();
	owner->commSubsystem->ClearTasks();

	InitialiseMovement(owner);

	int idleBarkIntervalMin = SEC2MS(owner->spawnArgs.GetInt("sleep_bark_interval_min", "10"));
	int idleBarkIntervalMax = SEC2MS(owner->spawnArgs.GetInt("sleep_bark_interval_max", "30"));

	owner->commSubsystem->AddCommTask(
		CommunicationTaskPtr(new RepeatedBarkTask("snd_sleeping", idleBarkIntervalMin, idleBarkIntervalMax))
	);

	// Let the AI update their weapons (make them nonsolid)
	owner->UpdateAttachmentContents(false);
}

// Gets called each time the mind is thinking
void IdleSleepState::Think(idAI* owner)
{
	Memory& memory = owner->GetMemory();

	if (_startSleeping && owner->GetMoveType() == MOVETYPE_ANIM)
	{
		if (owner->ReachedPos(memory.idlePosition, MOVE_TO_POSITION) 
			&& owner->GetCurrentYaw() == memory.idleYaw)
		{
			owner->AI_LAY_DOWN_LEFT = owner->spawnArgs.GetBool("lay_down_left", "1");
			owner->LayDown();
		}
	}
	else if (owner->GetMoveType() == MOVETYPE_GET_UP_FROM_LYING)
	{
		owner->GetMind()->SwitchState(owner->backboneStates[ERelaxed]);
		owner->commSubsystem->ClearTasks();
		return;
	}

	UpdateAlertLevel();

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;
}


StatePtr IdleSleepState::CreateInstance()
{
	return StatePtr(new IdleSleepState);
}

void IdleSleepState::OnChangeTarget(idAI* owner)
{
	// re-initialize movement subsystem to catch new path_corners
	InitialiseMovement(owner);
}

// Register this state with the StateLibrary
StateLibrary::Registrar idleSleepStateRegistrar(
	STATE_IDLE_SLEEP, // Task Name
	StateLibrary::CreateInstanceFunc(&IdleSleepState::CreateInstance) // Instance creation callback
);

} // namespace ai
