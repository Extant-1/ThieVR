/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 6634 $ (Revision of last commit) 
 $Date: 2016-10-12 20:38:38 +0000 (Wed, 12 Oct 2016) $ (Date of last commit)
 $Author: grayman $ (Author of last commit)
 
******************************************************************************/

#ifndef __AI_RANGED_COMBAT_TASK_H__
#define __AI_RANGED_COMBAT_TASK_H__

#include "CombatTask.h"

namespace ai
{

// Define the name of this task
#define TASK_RANGED_COMBAT "RangedCombat"

class RangedCombatTask;
typedef boost::shared_ptr<RangedCombatTask> RangedCombatTaskPtr;

class RangedCombatTask :
	public CombatTask
{
	//int _lastCombatBarkTime; // grayman #4394
public:
	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	// Creates a new Instance of this task
	static RangedCombatTaskPtr CreateInstance();

	virtual void OnFinish(idAI* owner);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);
};

} // namespace ai

#endif /* __AI_RANGED_COMBAT_TASK_H__ */
