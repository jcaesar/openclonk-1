/**
	Sequence
	Cutscene to be watched by all players.
	Start calling global func StartSequence, stop using StopSequence
	
	Can also be used as a trigger object for UserActions.
	
	@author Sven
*/

local seq_name;
local seq_progress;
local started;


/* Start and stop */

public func Start(string name, int progress, ...)
{
	if (started) 
		Stop();
	// Force global coordinates for the script execution.
	SetPosition(0, 0);
	// Store sequence name and progress.
	this.seq_name = name;
	this.seq_progress = progress;
	// Call init function of this scene - difference to start function is that it is called before any player joins.
	var fn_init = Format("~%s_Init", seq_name);
	if (!Call(fn_init, ...))
		GameCall(fn_init, this, ...);
	// Join all players: disable player controls and call join player of this scene.
	for (var i = 0; i < GetPlayerCount(C4PT_User); ++i)
	{
		var plr = GetPlayerByIndex(i, C4PT_User);
		JoinPlayer(plr);
	}
	started = true;
	// Sound effect.
	Sound("UI::Ding", true);
	// Call start function of this scene.
	var fn_start = Format("%s_Start", seq_name);
	if (!Call(fn_start, ...))
		GameCall(fn_start, this, ...);
	return true;
}

protected func InitializePlayer(int plr)
{
	if (seq_name)
	{
		// Scripted sequence
		JoinPlayer(plr);
	}
	else
	{
		// Editor-made sequence
		if (trigger && trigger.Trigger == "player_join") OnTrigger(nil, plr);
	}
	return true;
}

protected func InitializePlayers(int plr)
{
	if (!seq_name)
	{
		// Editor-made sequence
		if (trigger && trigger.Trigger == "game_start") OnTrigger();
	}
	return true;
}

public func RemovePlayer(int plr)
{
	if (seq_name)
	{
		// Scripted sequence
		// Called by sequence if it ends and by engine if player leaves.
		var fn_remove = Format("~%s_RemovePlayer", seq_name);
		if (!Call(fn_remove, plr))
			GameCall(fn_remove, this, plr);
	}
	else
	{
		// Editor-made sequence
		if (trigger && trigger.Trigger == "player_remove") OnTrigger(nil, plr);
	}
	return true;
}

public func JoinPlayer(int plr)
{
	var j = 0, crew;
	while (crew = GetCrew(plr, j++))
	{
		//if (crew == GetCursor(plr)) crew.Sequence_was_cursor = true; else crew.Sequence_was_cursor = nil;
		crew->SetCrewEnabled(false);
		crew->CancelUse();
		if (crew->GetMenu()) 
			if (!crew->GetMenu()->~Uncloseable()) 
				crew->CancelMenu();
		crew->MakeInvincible();
		crew->SetCommand("None");
		crew->SetComDir(COMD_Stop);
		crew.Sequence_stored_breath = crew->GetBreath();
	}
	// Per-player sequence callback.
	var fn_join = Format("~%s_JoinPlayer", seq_name);
	if (!Call(fn_join, plr))
		GameCall(fn_join, this, plr);
	return true;
}

public func Stop(bool no_remove)
{
	if (started)
	{
		SetViewTarget(nil);
		// Reenable crew and reset cursor.
		for (var i = 0; i<GetPlayerCount(C4PT_User); ++i)
		{
			var plr = GetPlayerByIndex(i, C4PT_User);
			var j = 0, crew;
			while (crew = GetCrew(plr, j++))
			{
				crew->SetCrewEnabled(true);
				crew->ClearInvincible();
				// just in case clonk was underwater
				var breath_diff = crew.Sequence_stored_breath - crew->GetBreath();
				crew.Sequence_stored_breath = nil;
				if (breath_diff) crew->DoBreath(breath_diff + 100); // give some bonus breath for the distraction
				//if (crew.Sequence_was_cursor) SetCursor(plr, crew);
			}
			// Ensure proper cursor.
			if (!GetCursor(plr)) 
				SetCursor(plr, GetCrew(plr));
			if (crew = GetCursor(plr)) 
				SetPlrView(plr, crew);
			// Per-player sequence callback.
			RemovePlayer(plr);
		}
		Sound("UI::Ding", true);
		started = false;
		// Call stop function of this scene.
		var fn_init = Format("~%s_Stop", seq_name);
		if (!Call(fn_init))
			GameCall(fn_init, this);
	}
	if (!no_remove) 
		RemoveObject();
	return true;
}

protected func Destruction()
{
	Stop(true);
	return;
}


/*-- Sequence callbacks --*/

public func ScheduleNext(int delay, next_idx)
{
	return ScheduleCall(this, this.CallNext, delay, 1, next_idx);
}

public func ScheduleSame(int delay) { return ScheduleNext(delay, seq_progress); }

public func CallNext(next_idx)
{
	// Start conversation context.
	// Update dialogue progress first.
	if (GetType(next_idx)) 
		seq_progress = next_idx; 
	else 
		++seq_progress;
	// Then call relevant functions.
	var fn_progress = Format("%s_%d", seq_name, seq_progress);
	if (!Call(fn_progress))
		GameCall(fn_progress, this);
	return true;
}


/*-- Force view on target --*/

// Force all player views on given target
public func SetViewTarget(object view_target)
{
	ClearScheduleCall(this, this.UpdateViewTarget);
	if (view_target)
	{
		UpdateViewTarget(view_target);
		ScheduleCall(this, this.UpdateViewTarget, 30, 999999999, view_target);
	}
	else
	{
		for (var i = 0; i < GetPlayerCount(C4PT_User); ++i)
		{
			var plr = GetPlayerByIndex(i, C4PT_User);
			SetPlrView(plr, GetCursor(plr));
		}
	}
	return true;
}

private func UpdateViewTarget(object view_target)
{
	// Force view of all players on target.
	if (!view_target) 
		return;
	for (var i=0; i < GetPlayerCount(C4PT_User); ++i)
	{
		var plr = GetPlayerByIndex(i, C4PT_User);
		SetPlrView(plr, view_target);
	}
	return;
}



/*-- Message function forwards --*/

public func MessageBoxAll(string message, object talker, bool as_message, ...)
{
	return Dialogue->MessageBoxAll(message, talker, as_message, ...);
}

private func MessageBox(string message, object clonk, object talker, int for_player, bool as_message, ...)
{
	return Dialogue->MessageBox(message, clonk, talker, for_player, as_message, ...);
}


/*-- Helper Functions --*/

// Helper function to get a speaker in sequences.
public func GetHero(object nearest_obj)
{
	// Prefer object stored as hero - if not assigned, find someone close to specified object.
	if (!this.hero)
	{
		if (nearest_obj)
			this.hero = nearest_obj->FindObject(Find_ID(Clonk), Find_Not(Find_Owner(NO_OWNER)), nearest_obj->Sort_Distance());
		else
			this.hero = FindObject(Find_ID(Clonk), Find_Not(Find_Owner(NO_OWNER)));
	}
	// If there is still no hero, take any clonk. Let the NPCs do the sequence among themselves.
	// (to prevent null pointer exceptions if all players left during the sequence)
	if (!this.hero) 
		this.hero = FindObject(Find_ID(Clonk));
	// Might return nil if all players are gone and there are no NPCs. Well, there was noone to listen anyway.
	return this.hero;
}

// Scenario section overload: automatically transfers all player clonks.
public func LoadScenarioSection(name, ...)
{
	// Store objects: All clonks and sequence object
	this.save_objs = [];
	AddSectSaveObj(this);
	var iplr, plr;
	for (iplr = 0; iplr < GetPlayerCount(C4PT_User); ++iplr)
	{
		plr = GetPlayerByIndex(iplr, C4PT_User);
		for (var icrew = 0, crew; icrew < GetCrewCount(iplr); ++icrew)
			if (crew = GetCrew(plr, icrew))
				AddSectSaveObj(crew);
	}
	var save_objs = this.save_objs;
	// Load new section
	var result = inherited(name, ...);
	// Restore objects
	for (var obj in save_objs) 
		if (obj) 
			obj->SetObjectStatus(C4OS_NORMAL);
	if (this) 
		this.save_objs = nil;
	// Recover HUD
	for (iplr = 0; iplr < GetPlayerCount(C4PT_User); ++iplr)
	{
		plr = GetPlayerByIndex(iplr, C4PT_User);
		var HUDcontroller = FindObject(Find_ID(GUI_Controller), Find_Owner(plr));
		if (!HUDcontroller) HUDcontroller = CreateObjectAbove(GUI_Controller, 10, 10, plr);
	}
	return result;
}

// Flag obj and any contained stuff for scenario saving.
public func AddSectSaveObj(object obj)
{
	if (!obj) 
		return false;
	this.save_objs[GetLength(this.save_objs)] = obj;
	var cont, i = 0;
	while (cont = obj->Contents(i++)) 
		AddSectSaveObj(cont);
	return obj->SetObjectStatus(C4OS_INACTIVE);
}


/*-- Global helper functions --*/

// Starts the specified sequence at indicated progress.
global func StartSequence(string name, int progress, ...)
{
	var seq = CreateObject(Sequence, 0, 0, NO_OWNER);
	seq->Start(name, progress, ...);
	return seq;
}

// Stops the currently active sequence.
global func StopSequence()
{
	var seq = FindObject(Find_ID(Sequence));
	if (!seq) 
		return false;
	return seq->Stop();
}

// Returns the currently active sequence.
global func GetActiveSequence()
{
	var seq = FindObject(Find_ID(Sequence));
	return seq;
}


/* User-made sequences from the editor */

local Name="$Name$";
local Description="$Description$";
local trigger, condition, action, action_progress_mode, action_allow_parallel;
local active=true;
local check_interval=12;
local deactivate_after_action; // If true, finished is set to true after the first execution and the trigger deactivated
local Visibility=VIS_Editor;

// finished: Disables the trigger. true if trigger has run and deactivate_after_action is set to true.
// Note that this flag is not saved in scenarios, so saving as scenario and reloading will re-enable all triggers (for editor mode)
local finished;

public func Definition(def)
{
	// EditorActions
	if (!def.EditorActions) def.EditorActions = {};
	def.EditorActions.Test = { Name="$Test$", Command="OnTrigger(nil, nil, true)" };
	// EditorProps
	if (!def.EditorProps) def.EditorProps = {};
	def.EditorProps.active = { Name="$Active$", Type="bool", Set="SetActive" };
	def.EditorProps.finished = { Name="$Finished$", Type="bool", Set="SetFinished" };
	def.EditorProps.trigger = { Name="$Trigger$", Type="enum", OptionKey="Trigger", Set="SetTrigger", Options = [
		{ Name="$None$" },
		{ Name="$PlayerEnterRegionRect$", EditorHelp="$PlayerEnterRegionHelp$", Value={ Trigger="player_enter_region_rect", Rect=[-20, -20, 40, 40] }, ValueKey="Rect", Delegate={ Type="rect", Color=0xff8000, Relative=true, Set="SetTriggerRect", SetRoot=true } },
		{ Name="$PlayerEnterRegionCircle$", EditorHelp="$PlayerEnterRegionHelp$", Value={ Trigger="player_enter_region_circle", Radius=25 }, ValueKey="Radius", Delegate={ Type="circle", Color=0xff8000, Relative=true, Set="SetTriggerRadius", SetRoot=true } },
		{ Name="$ObjectEnterRegionRect$", EditorHelp="$ObjectEnterRegionHelp$", Value={ Trigger="object_enter_region_rect", Rect=[-20, -20, 40, 40] }, Delegate={ Name="$ObjectEnterRegionRect$", EditorHelp="$ObjectEnterRegionHelp$", Type="proplist", EditorProps = {
			ID = { Name="$ID$", EditorHelp="$IDHelp$", Type="def", Set="SetTriggerID", SetRoot=true },
			Rect = { Type="rect", Color=0xff8000, Relative=true, Set="SetTriggerRect", SetRoot=true }
			} } },
		{ Name="$ObjectEnterRegionCircle$", EditorHelp="$ObjectEnterRegionHelp$", Value={ Trigger="object_enter_region_circle", Radius=25 }, Delegate={ Name="$ObjectEnterRegionCircle$", EditorHelp="$ObjectEnterRegionHelp$", Type="proplist", EditorProps = {
			ID = { Name="$ID$", EditorHelp="$IDHelp$", Type="def", Set="SetTriggerID", SetRoot=true },
			Radius = { Type="circle", Color=0xff8000, Relative=true, Set="SetTriggerRadius", SetRoot=true }
			} } },
		{ Name="$GameStart$", Value={ Trigger="game_start" } },
		{ Name="$PlayerJoin$", Value={ Trigger="player_join" } },
		{ Name="$PlayerRemove$", Value={ Trigger="player_remove" } },
		{ Name="$GoalsFulfilled$", Value={ Trigger="goals_fulfilled" } },
		{ Group="$ClonkDeath$", Name="$AnyClonkDeath$", Value={ Trigger="any_clonk_death" } },
		{ Group="$ClonkDeath$", Name="$PlayerClonkDeath$", Value={ Trigger="player_clonk_death" } },
		{ Group="$ClonkDeath$", Name="$NeutralClonkDeath$", Value={ Trigger="neutral_clonk_death" } },
		{ Group="$ClonkDeath$", Name="$SpecificClonkDeath$", Value={ Trigger="specific_clonk_death" }, ValueKey="Object", Delegate={ Type="object", Filter="IsClonk" } },
		{ Name="$Construction$", Value={ Trigger="construction" }, ValueKey="ID", Delegate={ Type="def", Filter="IsStructure", EmptyName="$Anything$" } },
		{ Name="$Production$", Value={ Trigger="production" }, ValueKey="ID", Delegate={ Type="def", EmptyName="$Anything$" } },
		] };
	def.EditorProps.condition = UserAction.Evaluator.Condition;
	def.EditorProps.action = UserAction.Prop;
	def.EditorProps.action_progress_mode = UserAction.PropProgressMode;
	def.EditorProps.action_allow_parallel = UserAction.PropParallel;
	def.EditorProps.deactivate_after_action = { Name="$DeactivateAfterAction$", Type="bool" };
}

public func SetTrigger(proplist new_trigger)
{
	trigger = new_trigger;
	// Set trigger: Restart any specific trigger timers
	if (active && !finished)
	{
		StopTrigger();
		StartTrigger();
	}
	return true;
}

public func SetTriggerRect(array new_trigger_rect)
{
	if (trigger && trigger.Rect)
	{
		trigger.Rect = new_trigger_rect;
		SetTrigger(trigger); // restart trigger
	}
	return true;
}

public func SetTriggerRadius(int new_trigger_radius)
{
	if (trigger)
	{
		trigger.Radius = new_trigger_radius;
		SetTrigger(trigger); // restart trigger
	}
	return true;
}

public func SetTriggerID(id new_id)
{
	if (trigger)
	{
		trigger.ID = new_id;
		SetTrigger(trigger); // restart trigger
	}
	return true;
}

public func SetAction(new_action, new_action_progress_mode, new_action_allow_parallel)
{
	action = new_action;
	action_progress_mode = new_action_progress_mode;
	action_allow_parallel = new_action_allow_parallel;
	return true;
}

public func SetActive(bool new_active, bool force_triggers)
{
	if (active == new_active && !force_triggers) return true;
	active = new_active;
	if (active && !finished)
	{
		// Activated: Start trigger
		StartTrigger();
	}
	else
	{
		// Inactive or inactive by editor run: Stop trigger
		StopTrigger();
	}
	return true;
}

public func SetFinished(bool new_finished)
{
	finished = new_finished;
	return SetActive(active, true);
}

public func SetDeactivateAfterAction(bool new_val)
{
	deactivate_after_action = new_val;
	return true;
}

public func StartTrigger()
{
	if (!trigger) return false;
	SetGraphics("Active");
	var fn = trigger.Trigger;
	var id_search;
	if (trigger.ID) id_search = Find_ID(trigger.ID);
	if (fn == "player_enter_region_rect")
	{
		this.search_mask = Find_And(Find_InRect(trigger.Rect[0], trigger.Rect[1], trigger.Rect[2], trigger.Rect[3]), Find_OCF(OCF_Alive), Find_Func("IsClonk"), Find_Not(Find_Owner(NO_OWNER)));
		AddTimer(this.EnterRegionTimer, check_interval);
	}
	else if (fn == "player_enter_region_circle")
	{
		this.search_mask = Find_And(Find_Distance(trigger.Radius), Find_OCF(OCF_Alive), Find_Func("IsClonk"), Find_Not(Find_Owner(NO_OWNER)));
		AddTimer(this.EnterRegionTimer, check_interval);
	}
	else if (fn == "object_enter_region_rect")
	{
		this.search_mask = Find_And(Find_InRect(trigger.Rect[0], trigger.Rect[1], trigger.Rect[2], trigger.Rect[3]), id_search);
		AddTimer(this.EnterRegionTimer, check_interval);
	}
	else if (fn == "object_enter_region_circle")
	{
		this.search_mask = Find_And(Find_Distance(trigger.Radius), Find_OCF(OCF_Alive), Find_Func("IsClonk"), id_search);
		AddTimer(this.EnterRegionTimer, check_interval);
	}
	else return false;
	return true;
}

public func StopTrigger()
{
	SetGraphics();
	// Remove any timers that may have been added
	RemoveTimer(this.EnterRegionRectTimer);
	return true;
}

private func EnterRegionTimer()
{
	for (var clonk in FindObjects(this.search_mask))
	{
		if (!clonk) continue; // deleted by previous execution
		OnTrigger(clonk, clonk->GetOwner());
		if (active != true) break; // deactivated by trigger
	}
}

public func OnTrigger(object triggering_clonk, int triggering_player, bool is_editor_test)
{
	// Editor test: Triggered by first player
	if (is_editor_test)
	{
		if (GetPlayerCount(C4PT_User)) triggering_player = GetPlayerByIndex();
	}
	// Check condition
	if (!UserAction->EvaluateCondition(action, this, triggering_clonk, triggering_player)) return false;
	// Only one action at the time
	if (!action_allow_parallel) StopTrigger();
	// Execute action
	return UserAction->EvaluateAction(action, this, triggering_clonk, triggering_player, action_progress_mode, action_allow_parallel, this.OnActionFinished);
}

private func OnActionFinished(context)
{
	// Callback from EvaluateAction: Action finished. Deactivate action if desired.
	if (deactivate_after_action)
		SetFinished(true);
	else if (active && !finished && !action_allow_parallel)
		StartTrigger();
	return true;
}

public func OnClonkDeath(object clonk, int killer)
{
	// Is this a clonk death trigger?
	if (!trigger || !clonk) return false;
	var t = trigger.Trigger;
	if (!WildcardMatch(t, "*_clonk_death")) return false;
	// Specific trigger check
	if (t == "player_clonk_death")
	{
		if (clonk->GetOwner() == NO_OWNER) return false;
	}
	else if (t == "neutral_clonk_death")
	{
		if (clonk->GetOwner() != NO_OWNER) return false;
	}
	else if (t == "specific_clonk_death")
	{
		if (trigger.Object != clonk) return false;
	}
	// OK, trigger it!
	return OnTrigger(clonk, killer);
}

public func OnConstructionFinished(object structure, int constructing_player)
{
	// Is this a structure finished trigger?
	if (!trigger || !structure) return false;
	if (trigger.Trigger != "construction") return false;
	if (trigger.ID) if (structure->GetID() != trigger.ID) return false;
	// OK, trigger it!
	return OnTrigger(structure, constructing_player);
}

public func OnProductionFinished(object product, int producing_player)
{
	// Is this a structure finished trigger?
	if (!trigger || !product) return false;
	if (trigger.Trigger != "production") return false;
	if (trigger.ID) if (product->GetID() != trigger.ID) return false;
	// OK, trigger it!
	return OnTrigger(product, producing_player);
}

public func OnGoalsFulfilled()
{
	// All goals fulfilled: Return true if any action is executed (stops regular GameOver)
	if (!trigger) return false;
	if (trigger.Trigger != "goals_fulfilled") return false;
	return OnTrigger();
}

public func SetName(string new_name, ...)
{
	if (new_name == GetID()->GetName())
	{
		Message("");
	}
	else
	{
		if (trigger)
			Message(Format("@<c ff8000>%s</c>", new_name));
		else
			Message(Format("@<c 808080>%s</c>", new_name));
	}
	return inherited(new_name, ...);
}

/*-- Saving --*/

// No scenario saving.
public func SaveScenarioObject(props, ...)
{
	if (!_inherited(props, ...)) return false;
	// Do not save script-created sequences
	if (this.seq_name) return false;
	// Save editor-made sequences
	if (save_scenario_dup_objects && finished) // finished flag only copied for object duplication; not saved in savegames
		props->AddCall("Active", this, "SetFinished", finished);
	if (!active) props->AddCall("Active", this, "SetActive", active);
	if (trigger) props->AddCall("Trigger", this, "SetTrigger", trigger);
	if (condition) props->AddCall("Condition", this, "SetCondition", condition);
	if (action || action_progress_mode || action_allow_parallel) props->AddCall("Action", this, "SetAction", action, action_progress_mode, action_allow_parallel);
	if (deactivate_after_action) props->AddCall("DeactivateAfterAction", this, "SetDeactivateAfterAction", deactivate_after_action);
	return false;
}
