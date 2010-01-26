/*
 * OpenClonk, http://www.openclonk.org
 *
 * Copyright (c) 1998-2000  Matthes Bender
 * Copyright (c) 2001, 2004-2005, 2007  Sven Eberhardt
 * Copyright (c) 2008  Günther Brammer
 * Copyright (c) 2001-2009, RedWolf Design GmbH, http://www.clonk.de
 *
 * Portions might be copyrighted by other authors who have contributed
 * to OpenClonk.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * See isc_license.txt for full license and disclaimer.
 *
 * "Clonk" is a registered trademark of Matthes Bender.
 * See clonk_trademark_license.txt for full license.
 */

/* Loads all standard graphics from Graphics.c4g */

#ifndef INC_C4GraphicsResource
#define INC_C4GraphicsResource

#include <C4Group.h>
#include <C4GroupSet.h>
#include <C4Surface.h>
#include <C4FacetEx.h>

namespace C4GUI { class Resource; }

class C4GraphicsResource
	{
	private:
		bool fInitialized;
	public:
		C4GraphicsResource();
		~C4GraphicsResource();
	protected:
		C4Surface sfcControl;
		int32_t idSfcControl; // id of source group of control surface
		int32_t idPalGrp;     // if of source group of pal file
		// ID of last group in main group set that was already registered into the Files-set
		// used to avoid doubled entries by subsequent calls to RegisterMainGroups
		int32_t idRegisteredMainGroupSetFiles;
	public:
		C4GroupSet Files;
		BYTE GamePalette[256*3];
		BYTE AlphaPalette[256*3]; // TODO: alphapal: Why *3?
		C4FacetID fctPlayer;
		C4FacetID fctFlag;
		C4FacetID fctCrew;
		C4FacetID fctScore;
		C4FacetID fctWealth;
		C4FacetID fctRank;
		int32_t iNumRanks;
		C4FacetID fctFire;
		C4FacetID fctBackground;
		C4FacetID fctCaptain;
		C4FacetID	fctMouseCursor;
		bool fOldStyleCursor; // if set, offsets need to be applied to some cursor facets
		C4FacetID fctSelectMark;
		C4FacetID fctOptions;
		C4FacetID fctMenu;
		C4FacetID fctUpperBoard;
		C4FacetID fctLogo;
		C4FacetID fctConstruction;
		C4FacetID fctEnergy;
		C4FacetID fctMagic;
		C4FacetID fctArrow;
		C4FacetID fctExit;
		C4FacetID fctHand;
		C4FacetID fctGamepad;
		C4FacetID fctBuild;
		C4Facet fctCursor;
		C4Facet fctDropTarget;
		C4Facet fctInsideSymbol;
		C4Facet fctKeyboard;
		C4Facet fctMouse;
		C4Facet fctCommand;
		C4Facet fctKey;
		C4Facet fctOKCancel;
		C4FacetID fctCrewClr; // ColorByOwner-surface of fctCrew
		C4FacetID fctFlagClr; // ColorByOwner-surface of fctFlag
		C4FacetID fctPlayerClr; // ColorByOwner-surface of fctPlayer
		C4FacetID fctPlayerGray; // grayed out version of fctPlayer

		// fonts
		CStdFont FontTiny;     // used for logs
		CStdFont FontRegular;  // normal font - just refed from graphics system
		CStdFont FontCaption;  // used for title bars
		CStdFont FontTitle;    // huge font for titles
		CStdFont FontTooltip;  // normal, non-shadowed font (same as BookFont)
	public:
		int32_t GetColorIndex(int32_t iColor, bool fLast=false);
		void Default();
		void Clear();
		bool InitFonts(); // init fonts only (early init done by loader screen)
		void ClearFonts(); // clear fonts ()
		bool Init(bool fInitGUI);

		bool IsInitialized() { return fInitialized; } // return whether any gfx are loaded (so dlgs can be shown)

		bool RegisterGlobalGraphics(); // register global Graphics.c4g into own group set
		bool RegisterMainGroups();     // register new groups of Game.GroupSet into own group set
		void CloseFiles();             // free group set

		bool ReloadResolutionDependantFiles(); // reload any files that depend on the current resolution

	protected:
		bool LoadFile(C4FacetID& fct, const char *szName, C4GroupSet &rGfxSet, int32_t iWdt = C4FCT_Full, int32_t iHgt = C4FCT_Full, bool fNoWarnIfNotFound=false);
		bool LoadFile(C4Surface& sfc, const char *szName, C4GroupSet &rGfxSet, int32_t &ridCurrSfc);
		bool FindLoadRes(C4Group *pSecondFile, const char *szName);
		bool LoadCursorGfx();

		friend class C4GUI::Resource;
		friend class C4StartupGraphics;
	};

extern C4GraphicsResource GraphicsResource;
#define GfxR (&(::GraphicsResource))
#endif
