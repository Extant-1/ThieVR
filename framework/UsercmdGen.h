/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 6574 $ (Revision of last commit) 
 $Date: 2016-02-04 00:07:31 +0000 (Thu, 04 Feb 2016) $ (Date of last commit)
 $Author: stevel $ (Author of last commit)
 
******************************************************************************/

#ifndef __USERCMDGEN_H__
#define __USERCMDGEN_H__

/*
===============================================================================

	Samples a set of user commands from player input.

===============================================================================
*/

const int USERCMD_HZ			= 60;			// 60 frames per second
const int USERCMD_MSEC			= 1000 / USERCMD_HZ;

// ButtonState inputs; originally from UsercmdGen.cpp, left out of SDK by accident
// sourced from http://www.doom3world.org/phpbb2/viewtopic.php?f=26&t=18587&p=170143
typedef enum {
   UB_NONE,

   UB_UP,
   UB_DOWN,
   UB_LEFT,
   UB_RIGHT,
   UB_FORWARD,
   UB_BACK,
   UB_LOOKUP,
   UB_LOOKDOWN,
   UB_STRAFE,
   UB_MOVELEFT,
   UB_MOVERIGHT,

   UB_BUTTON0,
   UB_BUTTON1,
   UB_BUTTON2,
   UB_BUTTON3,
   UB_BUTTON4,
   UB_BUTTON5,
   UB_BUTTON6,
   UB_BUTTON7,

   UB_ATTACK,
   UB_SPEED,
   UB_ZOOM,
   UB_SHOWSCORES,
   UB_MLOOK,

#ifdef QUAKE4
        UB_INGAMESTATS,
        UB_VOICECHAT,
        UB_TOURNEY,
#endif//QUAKE4

   UB_IMPULSE0,
   UB_IMPULSE1,
   UB_IMPULSE2,
   UB_IMPULSE3,
   UB_IMPULSE4,
   UB_IMPULSE5,
   UB_IMPULSE6,
   UB_IMPULSE7,
   UB_IMPULSE8,
   UB_IMPULSE9,
   UB_IMPULSE10,
   UB_IMPULSE11,
   UB_IMPULSE12,
   UB_IMPULSE13,
   UB_IMPULSE14,
   UB_IMPULSE15,
   UB_IMPULSE16,
   UB_IMPULSE17,
   UB_IMPULSE18,
   UB_IMPULSE19,
   UB_IMPULSE20,
   UB_IMPULSE21,
   UB_IMPULSE22,
   UB_IMPULSE23,
   UB_IMPULSE24,
   UB_IMPULSE25,
   UB_IMPULSE26,
   UB_IMPULSE27,
   UB_IMPULSE28,
   UB_IMPULSE29,
   UB_IMPULSE30,
   UB_IMPULSE31,
   UB_IMPULSE32,
   UB_IMPULSE33,
   UB_IMPULSE34,
   UB_IMPULSE35,
   UB_IMPULSE36,
   UB_IMPULSE37,
   UB_IMPULSE38,
   UB_IMPULSE39,
   UB_IMPULSE40,
   UB_IMPULSE41,
   UB_IMPULSE42,
   UB_IMPULSE43,
   UB_IMPULSE44,
   UB_IMPULSE45,
   UB_IMPULSE46,
   UB_IMPULSE47,
   UB_IMPULSE48,
   UB_IMPULSE49,
   UB_IMPULSE50,
   UB_IMPULSE51,
   UB_IMPULSE52,
   UB_IMPULSE53,
   UB_IMPULSE54,
   UB_IMPULSE55,
   UB_IMPULSE56,
   UB_IMPULSE57,
   UB_IMPULSE58,
   UB_IMPULSE59,
   UB_IMPULSE60,
   UB_IMPULSE61,
   UB_IMPULSE62,
   UB_IMPULSE63,

   UB_MAX_BUTTONS
} UserCmdButton;


// usercmd_t->button bits
const int BUTTON_ATTACK			= BIT(0);
const int BUTTON_RUN			= BIT(1);
const int BUTTON_ZOOM			= BIT(2);
const int BUTTON_SCORES			= BIT(3);
const int BUTTON_MLOOK			= BIT(4);
const int BUTTON_5				= BIT(5);
const int BUTTON_6				= BIT(6);
const int BUTTON_7				= BIT(7);

// usercmd_t->impulse commands
enum {
	IMPULSE_0,
	IMPULSE_1,
	IMPULSE_2,
	IMPULSE_3,
	IMPULSE_4,
	IMPULSE_5,
	IMPULSE_6,
	IMPULSE_7,
	IMPULSE_8,
	IMPULSE_9,
	IMPULSE_10,
	IMPULSE_11,
	IMPULSE_12,
	IMPULSE_13,
	IMPULSE_14,
	IMPULSE_15,
	IMPULSE_16,
	IMPULSE_17,
	IMPULSE_18,
	IMPULSE_19,
	IMPULSE_20,
	IMPULSE_21,
	IMPULSE_22,
	IMPULSE_23,
	IMPULSE_24,
	IMPULSE_25,
	IMPULSE_26,
	IMPULSE_27,
	IMPULSE_28,
	IMPULSE_29,
	IMPULSE_30,	// #4286
	IMPULSE_40 = 40,
	IMPULSE_41,
	IMPULSE_42,
	IMPULSE_43,
	IMPULSE_44,
	IMPULSE_45,
	IMPULSE_46,
	IMPULSE_47,
	IMPULSE_48,
	IMPULSE_49,
	IMPULSE_50,
	IMPULSE_51,
	IMPULSE_52,
	IMPULSE_MAX
};

/*
const int IMPULSE_0				= 0;			// weap 0
const int IMPULSE_1				= 1;			// weap 1
const int IMPULSE_2				= 2;			// weap 2
const int IMPULSE_3				= 3;			// weap 3
const int IMPULSE_4				= 4;			// weap 4
const int IMPULSE_5				= 5;			// weap 5
const int IMPULSE_6				= 6;			// weap 6
const int IMPULSE_7				= 7;			// weap 7
const int IMPULSE_8				= 8;			// weap 8
const int IMPULSE_9				= 9;			// weap 9
const int IMPULSE_10			= 10;			// weap 10
const int IMPULSE_11			= 11;			// weap 11
const int IMPULSE_12			= 12;			// weap 12
const int IMPULSE_13			= 13;			// weap reload
const int IMPULSE_14			= 14;			// weap next
const int IMPULSE_15			= 15;			// weap prev
const int IMPULSE_16			= 16;			// <unused>
const int IMPULSE_17			= 17;			// ready to play ( toggles ui_ready )
const int IMPULSE_18			= 18;			// center view
const int IMPULSE_19			= 19;			// show INV/MAP
const int IMPULSE_20			= 20;			// toggle team ( toggles ui_team )
const int IMPULSE_21			= 21;			// <unused>
const int IMPULSE_22			= 22;			// spectate
const int IMPULSE_23			= 23;			// <unused>
const int IMPULSE_24			= 24;			// <unused>
const int IMPULSE_25			= 25;			// <unused>
const int IMPULSE_26			= 26;			// <unused>
const int IMPULSE_27			= 27;			// <unused>
const int IMPULSE_28			= 28;			// vote yes
const int IMPULSE_29			= 29;			// vote no
const int IMPULSE_30			= 30;			// Inventory Grid courtesy Durandall  #4286
const int IMPULSE_40			= 40;			// use vehicle
const int IMPULSE_41			= 41;			// TDM Use/Frob key
const int IMPULSE_42			= 42;			// Inventory prev (unused)
const int IMPULSE_43			= 43;			// Inventory next (unused)
const int IMPULSE_44			= 44;			// Lean forward
const int IMPULSE_45			= 45;			// Lean left
const int IMPULSE_46			= 46;			// Lean right
const int IMPULSE_47			= 47;			// Inventory prev item
const int IMPULSE_48			= 48;			// Inventory next item
const int IMPULSE_49			= 49;			// Inventory prev group
const int IMPULSE_50			= 50;			// Inventory next group
const int IMPULSE_51			= 51;			// Inventory use item
const int IMPULSE_52			= 52;			// Inventory drop item
*/

// Darkmod: Added as a baseoffset for the impulse keys, when used with ButtonState.
// This function requires an int as input which defines the key that should be used,
// and it looks as if the first impulse starts with the number 25.
const int IMPULSE_BUTTON_BASE	= 25;
#define KEY_FROM_IMPULSE(n)		(n + IMPULSE_BUTTON_BASE)

// usercmd_t->flags
const int UCF_IMPULSE_SEQUENCE	= 0x0001;		// toggled every time an impulse command is sent

class usercmd_t {
public:
	int			gameFrame;						// frame number
	int			gameTime;						// game time
	int			duplicateCount;					// duplication count for networking
	byte		buttons;						// buttons
	signed char	forwardmove;					// forward/backward movement
	signed char	rightmove;						// left/right movement
	signed char	upmove;							// up/down movement
	short		angles[3];						// view angles
	short		mx;								// mouse delta x
	short		my;								// mouse delta y
	signed char impulse;						// impulse command
	byte		flags;							// additional flags
	int			sequence;						// just for debugging

public:
	void		ByteSwap();						// on big endian systems, byte swap the shorts and ints
	bool		operator==( const usercmd_t &rhs ) const;
};

typedef enum {
	INHIBIT_SESSION = 0,
	INHIBIT_ASYNC
} inhibit_t;

const int MAX_BUFFERED_USERCMD = 64;

class idUsercmdGen {
public:
	virtual				~idUsercmdGen( void ) {}

	// Sets up all the cvars and console commands.
	virtual	void		Init( void ) = 0;

	// Prepares for a new map.
	virtual void		InitForNewMap( void ) = 0;

	// Shut down.
	virtual void		Shutdown( void ) = 0;

	// Clears all key states and face straight.
	virtual	void		Clear( void ) = 0;

	// Clears view angles.
	virtual void		ClearAngles( void ) = 0;

	// When the console is down or the menu is up, only emit default usercmd, so the player isn't moving around.
	// Each subsystem (session and game) may want an inhibit will OR the requests.
	virtual void		InhibitUsercmd( inhibit_t subsystem, bool inhibit ) = 0;

	// Returns a buffered command for the given game tic.
	virtual usercmd_t	TicCmd( int ticNumber ) = 0;

	// Called async at regular intervals.
	virtual	void		UsercmdInterrupt( void ) = 0;

	// Set a value that can safely be referenced by UsercmdInterrupt() for each key binding.
	virtual	int			CommandStringUsercmdData( const char *cmdString ) = 0;

	// Returns the number of user commands.
	virtual int			GetNumUserCommands( void ) = 0;

	// Returns the name of a user command via index.
	virtual const char *GetUserCommandName( int index ) = 0;

	// Continuously modified, never reset. For full screen guis.
	virtual void		MouseState( int *x, int *y, int *button, bool *down ) = 0;

	// Directly sample a button.
	virtual int			ButtonState( int key ) = 0;

	// Directly sample a keystate.
	virtual int			KeyState( int key ) = 0;

	// Directly sample a usercmd.
	virtual usercmd_t	GetDirectUsercmd( void ) = 0;
};

extern idUsercmdGen	*usercmdGen;

#endif /* !__USERCMDGEN_H__ */