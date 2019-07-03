/*--
	Generic, not game-specific behaviour of the Clonk
	
	This should serve as a base definition for humanoid characters that behave like a Clonk,
	but do not need all of the OC Clonk behaviour, or want to define a different behaviour
	entirely.

--*/


func Construction()
{
	_inherited(...);

	// Assuming every Clonk/Humanoid has at least a "Walk" action
	SetAction("Walk");
	SetDir(Random(2));
	// Broadcast for rules (this should be called last, ideally, but it was never used anywhere)
	GameCallEx("OnClonkCreation", this);
}


/* When adding to the crew of a player */

protected func Recruitment(int iPlr)
{
	// Broadcast for crew
	GameCallEx("OnClonkRecruitment", this, iPlr);
	
	return _inherited(iPlr,...);
}

protected func DeRecruitment(int iPlr) {
	// Broadcast for crew
	GameCallEx("OnClonkDeRecruitment", this, iPlr);
	
	return _inherited(iPlr,...);
}

/* Events */

protected func Death(int killed_by)
{
	// this must be done first, before any goals do funny stuff with the clonk
	_inherited(killed_by,...);
	
	// Info-broadcasts for dying clonks.
	GameCallEx("OnClonkDeath", this, killed_by);
	
	// The broadcast could have revived the clonk.
	if (GetAlive())
		return false;
	
	// Some effects on dying.
	this->~DeathEffects(killed_by, ...);

	return true;
}

protected func Destruction(...)
{
	_inherited(...);
	// If the clonk wasn't dead yet, he will be now.
	// Always kill clonks first. This will ensure relaunch scripts, enemy kill counters, etc. are called
	// even if clonks die in some weird way that causes direct removal
	// (To prevent a death callback, you can use SetAlive(false); RemoveObject();)
	if (GetAlive()) { this.silent_death=true; Kill(); }
	return true;
}

protected func ControlCommand(szCommand, pTarget, iTx, iTy, pTarget2, Data)
{
	// RejectConstruction Callback for building via Drag'n'Drop form a building menu
	// TODO: Check if we still need this with the new system
	if(szCommand == "Construct")
	{
		if(Data->~RejectConstruction(iTx - GetX(), iTy - GetY(), this) )
		{
			return 1;
		}
	}
	// No overloaded command
	return _inherited(szCommand, pTarget, iTx, iTy, pTarget2, Data, ...);
}


/* Transformation */

public func Redefine(idTo)
{
	// save data of activity
	var phs=GetPhase(),act=GetAction();
	// Transform
	ChangeDef(idTo);
	// restore action
	var chg=SetAction(act);
	if (!chg) SetAction("Walk");
	if (chg) SetPhase(phs);
	// Done
	return 1;
}

public func QueryCatchBlow(object obj)
{
	var fx;
	var index = 0;
	// Blocked by object effects?
	while (fx = GetEffect("*", obj, index++))
		if (EffectCall(obj, fx, "QueryHitClonk", this))
			return true;
	// Blocked by Clonk effects?
	index = 0;
	while (fx = GetEffect("*Control*", this, index++))
		if (EffectCall(this, fx, "QueryCatchBlow", obj))
			return true;
	// Default blocking.
	return _inherited(obj, ...);
}


protected func CheckStuck()
{
	// Prevents getting stuck on middle vertex
	if(!GetXDir()) if(Abs(GetYDir()) < 5)
		if(GBackSolid(0, 3))
			SetPosition(GetX(), GetY() + 1);
}

/* Max energy */

func SetMaxEnergy(int new_max_energy)
{
	// Update max energy, inform HUD adapter and clamp current energy
	this.MaxEnergy = new_max_energy;
	var current_energy = GetEnergy();
	if (current_energy > this.MaxEnergy/1000)
		DoEnergy(this.MaxEnergy - current_energy*1000, true);
	else
		this->~OnEnergyChange();
	return true;
}

protected func OnEnergyChange(int change, int cause, int caused_by)
{
	if (change < 0 && GetCursor(GetOwner()) == this)
		PlayRumble(GetOwner(), Min(300 + 1000 * -change / this.MaxEnergy, 1000), 150);
	return _inherited(change, cause, caused_by, ...);
}


/* Scenario saving */

func SaveScenarioObject(props)
{
	if (!_inherited(props, ...)) return false;

	// Direction is randomized at creation and there's no good way to find
	// out if the user wanted that specific direction. So just always save
	// it, because that's what scenario designer usually wants.
	if (!props->HasProp("Dir")) props->AddCall("Dir", this, "SetDir", GetConstantNameByValueSafe(GetDir(),"DIR_"));
	// Custom portraits for dialogues
	if (this.portrait) props->AddCall("Portrait", this, "SetPortrait", this.portrait);
	return true;
}

func SetPortrait(proplist custom_portrait)
{
	this.portrait = custom_portrait;
	return true;
}

/* Properties */

local NoBurnDecay = true;
local FireproofContainer = true; // Don't burn down all tools/resources when the clonk dips into lava for a short time
local BorderBound = C4D_Border_Sides;

func Definition(def) {
	
	UserAction->AddEvaluator("Action", "Clonk", "$SetMaxContentsCount$", "$SetMaxContentsCountHelp$", "clonk_set_max_contents_count", [def, def.EvalAct_SetMaxContentsCount], { }, { Type="proplist", Display="{{Target}}: {{MaxContentsCount}}", EditorProps = {
		Target = UserAction->GetObjectEvaluator("IsClonk", "Clonk"),
		MaxContentsCount = new UserAction.Evaluator.Integer { Name="$MaxContentsCount$", EmptyName = Format("$Default$ (%d)", def.MaxContentsCount) }
		} } );
		
	// Turn around
	if (!def.EditorActions) def.EditorActions = {};
	def.EditorActions.turn_around = { Name="$TurnAround$", EditorHelp="$TurnAroundHelp$", Command="SetDir(1-GetDir())" };
	
	_inherited(def);
}

private func EvalAct_SetMaxContentsCount(proplist props, proplist context)
{
	// Set max contents count. nil defaults to Clonk.MaxContentsCount.
	var clonk = UserAction->EvaluateValue("Object", props.Target, context);
	var number = BoundBy(UserAction->EvaluateValue("Integer", props.MaxContentsCount, context) ?? clonk->GetID().MaxContentsCount, 0, 10);
	if (clonk) clonk->~SetMaxContentsCount(number);
}

/* --- Status --- */

// These assume that certain actions exist in the ActMap,
// but they should be basic enough to be regarded as
// something that any Clonk/humanoid has or can do

public func IsClonk() { return true; }

// TODO: Make this more sophisticated, read turn animation and other
// adaptions
public func IsJumping(){return WildcardMatch(GetAction(), "*Jump*");}
public func IsWalking(){return GetProcedure() == "WALK";}
public func IsSwimming(){return GetProcedure() == "SWIM";}
