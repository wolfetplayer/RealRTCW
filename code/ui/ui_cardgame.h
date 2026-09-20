/*
===========================================================================

ui_cardgame.h -- ui.qvm side of the Malta hub "War" card mini-game.
State here is UI-only: not saved, not networked. It resets to a fresh
52-card pool and 10 chips whenever the menu opens, and is discarded on
close - there is nothing here for the server to be the source of truth for.

===========================================================================
*/

#ifndef __UI_CARDGAME_H__
#define __UI_CARDGAME_H__

typedef enum {
	CG_PHASE_BETTING,       // waiting for SetBet + a "deal" uiScript
	CG_PHASE_PLAYER_TURN,   // table dealt, waiting for PlayerPick
	CG_PHASE_REVEAL,        // player's card is up, opponent's pick is pending
	CG_PHASE_WAR_ANNOUNCE,  // tie just landed - tied cards stay up for a beat before the war deal
	CG_PHASE_WAR_TURN,      // waiting for the player's next discard, or the decisive reveal once 2 remain
	CG_PHASE_WAR_PENDING,   // opponent's matching response is pending
	CG_PHASE_ROUND_OVER,    // win/loss text showing, waiting for "continue"
	CG_PHASE_GAME_OVER      // a side is out of chips - win/lose message + countdown, then auto-restart
} cardGamePhase_t;

void UI_CardGame_Reset( void );

// Call from UI_Init(), before UI_FreeTranslateTable() frees the table.
void UI_CardGame_ResolveTranslations( void );

// Call once per frame regardless of active menu - resolves pending opponent actions and finale countdown.
void UI_CardGame_RunFrame( void );

// session / chips
int      UI_CardGame_CurrentBet( void );
qboolean UI_CardGame_SetBet( int amount );   // clamps to [1, chips]
void     UI_CardGame_Deal( void );           // locks in currentBet, deals the 24-slot table

// "You: N   Opponent: N   Bet/Pot: N" readout, already formatted from the translated template
const char *UI_CardGame_ChipsText( void );

// table (feeder-backed grid of face-down/revealed slots)
int       UI_CardGame_TableSlotCount( void );          // 24 normally, 6 during a war sub-deal
qhandle_t UI_CardGame_TableSlotIcon( int slot );        // back shader, or face shader once revealed
qboolean  UI_CardGame_TableSlotIsPlayable( int slot );  // false once used up, or while a reveal is pending
qboolean  UI_CardGame_TableSlotIsEliminated( int slot ); // durable war-discard flag, independent of phase
int       UI_CardGame_PendingOpponentSlot( void );      // slot the opponent is about to act on, or -1

// turn flow
void            UI_CardGame_PlayerPick( int slot );    // flip (normal turn) or blind-discard (war turn)
cardGamePhase_t UI_CardGame_Phase( void );
qboolean        UI_CardGame_HasResult( void );          // true once a showdown has happened this round
void            UI_CardGame_ContinueAfterRound( void ); // dismiss result text, return to BETTING (or GAME_OVER)

// Also carries the betting-phase flow prompt and the finale win/lose countdown text.
const char *UI_CardGame_ResultText( void );
qboolean    UI_CardGame_ResultIsWin( void );   // which color to paint ResultText() in once a showdown lands

// war chain
int UI_CardGame_WarDepth( void );      // 0 = no war in progress

#endif // __UI_CARDGAME_H__
