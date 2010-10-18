/*--
	Scroll: Fireball
	Author: Mimmo

	Hurl a fiery ball into your enemies.
--*/


public func ControlUse(object pClonk, int ix, int iy)
{
	AddEffect("Fireball", 0, 100, 1, 0, GetID(), pClonk->GetOwner(), Angle(0,0,ix,iy),pClonk->GetX(), pClonk->GetY());
	RemoveObject();
	return 1;
}



public func FxFireballStart(pTarget, iEffectNumber, iTemp, owner, angle, x, y)
{
	if(iTemp) return;
	x+=Sin(angle, 10)+RandomX(-1, 1);
	y+=-Cos(angle, 10)+RandomX(-1, 1);
	EffectVar(0, pTarget, iEffectNumber)=owner;
	EffectVar(1, pTarget, iEffectNumber)=angle;
	EffectVar(2, pTarget, iEffectNumber)=x;
	EffectVar(3, pTarget, iEffectNumber)=y;
}

public func FxFireballTimer(pTarget, iEffectNumber, iEffectTime)
{
	var angle=EffectVar(1, pTarget, iEffectNumber);
	var x=EffectVar(2, pTarget, iEffectNumber);
	var y=EffectVar(3, pTarget, iEffectNumber);

	if	(	iEffectTime>67  ||
	 		GBackSolid(x,y) ||
	 		FindObject(
	 		Find_Hostile(EffectVar(0, pTarget, iEffectNumber)),
	 		Find_OCF(OCF_Alive),
	 		Find_NoContainer(),
	 		Find_OnLine(x+Sin(angle, -4),y-Cos(angle, 4),x+Sin(angle, -4),y-Cos(angle, 4))
	 		)
	 	)
	{
		CreateObject(Dynamite,x,y,-1)->Explode(14);
		for(var i=0; i<=3;i++) CreateObject(Dynamite,x+Sin(i*120 +x,13),y-Cos(i*120 +x,13),-1)->Explode(6+Random(4));
		return -1;
	}	
	else if(iEffectTime < 70)
	{
		//if(!Random(10)) if(Random(2))angle++; else angle--;
		angle+=Sin(iEffectTime*30,18);
		x+=Sin(angle, 6);
		y+=-Cos(angle, 6);
		EffectVar(2, pTarget, iEffectNumber)=x;
		EffectVar(3, pTarget, iEffectNumber)=y;
		for(var i=0;i<2;++i)
		{
			var c=HSL(Random(32), 200+Random(25), Random(100));
			var rx=RandomX(-1, 1);
			var ry=RandomX(-1, 1);
			CreateParticle("Magic", x+rx, y+ry, 0, 0, 30+Random(10), c);
			var rx=RandomX(-2, 2);
			var ry=RandomX(-2, 2);
			CreateParticle("Blast", x+rx, y+ry, 0, 0, 20+Random(8), c);
		}
	}

	return 1;
	
	
}

local Name = "$Name$";
local Description = "$Description$";
