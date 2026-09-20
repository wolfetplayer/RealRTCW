/*
===========================================================================

ui_cardgame.c -- ui.qvm side of the Malta hub "War" card mini-game.
See ui_cardgame.h.

===========================================================================
*/

#include "ui_local.h"
#include "ui_cardgame.h"

#define CARDGAME_POOL_SIZE   52
#define CARDGAME_TABLE_SLOTS 24
#define CARDGAME_WAR_SLOTS   6
#define CARDGAME_START_CHIPS 10
#define CARDGAME_NUM_RANKS   13
#define CARDGAME_NUM_SUITS   4

#define CARDGAME_PICK_DELAY_MS      1400  // beat between the player's pick and the opponent's response
#define CARDGAME_AI_FLICKER_MS      350   // how often the opponent's "considering" highlight jumps to a new card
#define CARDGAME_DEAL_STAGGER_MS    60    // per-slot delay so a deal visibly lays cards out one at a time
#define CARDGAME_WAR_ANNOUNCE_MS    2200  // how long the tied cards stay visible before the war table replaces them
#define CARDGAME_FINALE_COUNTDOWN_SEC 5   // seconds shown on the big win/lose banner before auto-restart

// All cardgame audio lives under its own path - dedicated assets, nothing borrowed from stock RTCW sounds.
#define SOUND_CHIP        "sound/cardgame/chip.wav"         // bet +/-
#define SOUND_DEAL        "sound/cardgame/deal.wav"         // Deal pressed, table is laid out
#define SOUND_REVEAL      "sound/cardgame/reveal.wav"       // any card flipping face-up
#define SOUND_DISCARD     "sound/cardgame/discard.wav"      // a war-round blind discard
#define SOUND_WAR_START   "sound/cardgame/war_start.wav"    // a tie triggers War
#define SOUND_WAR_END     "sound/cardgame/war_end.wav"      // War resolves to an actual win/loss
#define SOUND_WIN         "sound/cardgame/win.wav"          // player wins a round
#define SOUND_LOSE        "sound/cardgame/lose.wav"         // player loses a round
#define SOUND_FINALE_WIN  "sound/cardgame/finale_win.wav"   // opponent is out of chips - game finale
#define SOUND_FINALE_LOSE "sound/cardgame/finale_lose.wav"  // player is out of chips - game finale

typedef struct {
	int rank;   // 2-14 (11=J, 12=Q, 13=K, 14=A)
	int suit;   // 0-3
} card_t;

static const char *cardgameRankToken[CARDGAME_NUM_RANKS] = {
	"2", "3", "4", "5", "6", "7", "8", "9", "10", "j", "q", "k", "a"
};
static const char *cardgameSuitToken[CARDGAME_NUM_SUITS] = {
	"clubs", "diamonds", "hearts", "spades"
};

// War uses only the centered middle row of the 24-slot grid (cardSlot9..14 in cardgame.menu).
static const int warSlotLayout[CARDGAME_WAR_SLOTS] = { 9, 10, 11, 12, 13, 14 };

static qhandle_t cardFaceShaders[CARDGAME_POOL_SIZE];
static qhandle_t cardBackShader;

static card_t pool[CARDGAME_POOL_SIZE];
static int    poolOrder[CARDGAME_POOL_SIZE];
static int    poolCursor;   // how many poolOrder[] entries have been dealt into slots so far

static int             sessionChips;
static int             opponentChips;
static int             currentBet;
static int             warDepth;
static cardGamePhase_t phase;
static char             cardgameResultText[128];
static qboolean          hasResult;
static qboolean          resultIsWin;   // paint color for ResultText(), valid once hasResult is set

static card_t playerCard, opponentCard;   // this round's showdown pair

// slotActive[] marks which of the 24 positions are live - all of them for a normal round, or just
// warSlotLayout's 6 during War, which aren't a contiguous 0..N-1 prefix.
static qboolean tableRevealed[CARDGAME_TABLE_SLOTS];
static qboolean tableEliminated[CARDGAME_TABLE_SLOTS];   // war-only: blindly discarded, never revealed
static qboolean slotActive[CARDGAME_TABLE_SLOTS];
static card_t   tableCards[CARDGAME_TABLE_SLOTS];
static int      dealOrderIndex[CARDGAME_TABLE_SLOTS];   // stagger rank within the current deal
static int      dealStartTime;                          // trap_Milliseconds() of the last deal

// the opponent's half of the current exchange, delayed for a beat before it lands
static int      pendingRevealTime;     // 0 = nothing pending
static int      pendingOpponentSlot;   // the true, already-decided pick (never shown directly)
static int      pendingDisplaySlot;    // the slot currently highlighted as "opponent is considering this"
static int      pendingNextFlickerTime;
static qboolean pendingIsWarPick;
static qboolean pendingIsWarFinal;   // pending action is the decisive reveal, not a blind discard

// game finale: one side ran out of chips
static qboolean gameOverIsWin;
static int      gameOverCountdownStart;   // 0 = no finale in progress

static int warAnnounceUntil;   // 0 = not announcing; ms timestamp CG_PHASE_WAR_ANNOUNCE holds until

//=================================== localization ===================================

// Resolved once at UI_Init() into these buffers (English fallback baked in) - not safe to call later.
static char translatedChipsBetting[64]  = "You: %d   Opponent: %d   Bet: %d";
static char translatedChipsPot[64]      = "You: %d   Opponent: %d   Pot: %d";
static char translatedWarStart[64]      = "WAR! POT IS NOW %d CHIPS";
static char translatedWin[64]           = "YOU WON THE BET! PRESS NEXT TO CONTINUE";
static char translatedLose[64]          = "YOU LOST THE BET! PRESS NEXT TO CONTINUE";
static char translatedPressDeal[32]     = "PRESS DEAL TO START";
static char translatedPlaceBet[40]      = "PLACE YOUR BET TO CONTINUE";
static char translatedFinaleWin[48]     = "YOU WIN! RESTARTING IN %d...";
static char translatedFinaleLose[48]    = "YOU LOSE! RESTARTING IN %d...";

static void CardGame_ResolveOne( char *dest, int destSize, const char *key ) {
	const char *translated = TranslateTable_Find( key );
	if ( translated ) {
		Q_strncpyz( dest, translated, destSize );
	}
}

void UI_CardGame_ResolveTranslations( void ) {
	CardGame_ResolveOne( translatedChipsBetting, sizeof( translatedChipsBetting ), "CARDGAME_CHIPS_BETTING" );
	CardGame_ResolveOne( translatedChipsPot, sizeof( translatedChipsPot ), "CARDGAME_CHIPS_POT" );
	CardGame_ResolveOne( translatedWarStart, sizeof( translatedWarStart ), "CARDGAME_WAR_START" );
	CardGame_ResolveOne( translatedWin, sizeof( translatedWin ), "CARDGAME_WIN" );
	CardGame_ResolveOne( translatedLose, sizeof( translatedLose ), "CARDGAME_LOSE" );
	CardGame_ResolveOne( translatedPressDeal, sizeof( translatedPressDeal ), "CARDGAME_PRESS_DEAL" );
	CardGame_ResolveOne( translatedPlaceBet, sizeof( translatedPlaceBet ), "CARDGAME_PLACE_BET" );
	CardGame_ResolveOne( translatedFinaleWin, sizeof( translatedFinaleWin ), "CARDGAME_FINALE_WIN" );
	CardGame_ResolveOne( translatedFinaleLose, sizeof( translatedFinaleLose ), "CARDGAME_FINALE_LOSE" );
}

//=================================== sound ===================================

static void CardGame_PlaySound( const char *path ) {
	trap_S_StartLocalSound( trap_S_RegisterSound( path ), CHAN_LOCAL );
}

//=================================== deck / assets ===================================

static int CardGame_CardIndex( card_t c ) {
	return ( c.suit * CARDGAME_NUM_RANKS ) + ( c.rank - 2 );
}

static void CardGame_BuildAndShuffle( void ) {
	int i, j, n = 0;
	int suit, rank;

	for ( suit = 0; suit < CARDGAME_NUM_SUITS; suit++ ) {
		for ( rank = 2; rank <= 14; rank++ ) {
			pool[n].suit = suit;
			pool[n].rank = rank;
			poolOrder[n] = n;
			n++;
		}
	}

	// Fisher-Yates, same loop as UI_Armory_Randomize (ui_armory.c)
	for ( i = n - 1; i > 0; i-- ) {
		int tmpIndex;

		j = rand() % ( i + 1 );

		tmpIndex = poolOrder[i];
		poolOrder[i] = poolOrder[j];
		poolOrder[j] = tmpIndex;
	}

	poolCursor = 0;
}

// Deals into the given table positions, reshuffling first if the pool is short.
static void CardGame_DealInto( const int *slots, int count ) {
	int i;

	if ( poolCursor + count > CARDGAME_POOL_SIZE ) {
		CardGame_BuildAndShuffle();
	}

	for ( i = 0; i < CARDGAME_TABLE_SLOTS; i++ ) {
		slotActive[i] = qfalse;
	}

	for ( i = 0; i < count; i++ ) {
		int slot = slots[i];

		tableCards[slot] = pool[poolOrder[poolCursor]];
		poolCursor++;
		tableRevealed[slot] = qfalse;
		tableEliminated[slot] = qfalse;
		slotActive[slot] = qtrue;
		dealOrderIndex[slot] = i;
	}
	dealStartTime = trap_Milliseconds();
}

static void CardGame_DealNormalTable( void ) {
	int slots[CARDGAME_TABLE_SLOTS];
	int i;

	for ( i = 0; i < CARDGAME_TABLE_SLOTS; i++ ) {
		slots[i] = i;
	}
	CardGame_DealInto( slots, CARDGAME_TABLE_SLOTS );
}

static void CardGame_DealWarTable( void ) {
	CardGame_DealInto( warSlotLayout, CARDGAME_WAR_SLOTS );
}

static qhandle_t CardGame_FaceShader( card_t c ) {
	int idx = CardGame_CardIndex( c );

	if ( cardFaceShaders[idx] == -1 ) {
		char path[MAX_QPATH];

		Com_sprintf( path, sizeof( path ), "ui/assets/cards/%s_%s.png",
			cardgameRankToken[c.rank - 2], cardgameSuitToken[c.suit] );
		cardFaceShaders[idx] = trap_R_RegisterShaderNoMip( path );
	}
	return cardFaceShaders[idx];
}

// Boot-time sanity check: registers all 52 face shaders and logs anything missing.
static void CardGame_ValidateAssets( void ) {
	int suit, rank;

	if ( !trap_Cvar_VariableValue( "developer" ) ) {
		return;
	}

	for ( suit = 0; suit < CARDGAME_NUM_SUITS; suit++ ) {
		for ( rank = 2; rank <= 14; rank++ ) {
			card_t c;

			c.suit = suit;
			c.rank = rank;
			if ( !CardGame_FaceShader( c ) ) {
				Com_Printf( "CARDGAME: missing/unregistered shader for ui/assets/cards/%s_%s.png\n",
					cardgameRankToken[rank - 2], cardgameSuitToken[suit] );
			}
		}
	}
}

// True once this slot's staggered lay-out delay has elapsed.
static qboolean CardGame_SlotHasLanded( int slot ) {
	return trap_Milliseconds() >= dealStartTime + dealOrderIndex[slot] * CARDGAME_DEAL_STAGGER_MS;
}

//=================================== turn resolution ===================================

static int CardGame_RandomUnrevealedSlot( int excludeSlot ) {
	int candidates[CARDGAME_TABLE_SLOTS];
	int i, n = 0;

	for ( i = 0; i < CARDGAME_TABLE_SLOTS; i++ ) {
		if ( slotActive[i] && i != excludeSlot && !tableRevealed[i] ) {
			candidates[n++] = i;
		}
	}
	if ( n == 0 ) {
		return -1;
	}
	return candidates[rand() % n];
}

static int CardGame_RandomEligibleWarSlot( int excludeSlot ) {
	int candidates[CARDGAME_WAR_SLOTS];
	int i, n = 0;

	for ( i = 0; i < CARDGAME_TABLE_SLOTS; i++ ) {
		if ( slotActive[i] && i != excludeSlot && !tableEliminated[i] ) {
			candidates[n++] = i;
		}
	}
	if ( n == 0 ) {
		return -1;
	}
	return candidates[rand() % n];
}

// How many war slots are still live (not yet blindly discarded).
static int CardGame_CountWarRemaining( void ) {
	int i, remaining = 0;

	for ( i = 0; i < CARDGAME_TABLE_SLOTS; i++ ) {
		if ( slotActive[i] && !tableEliminated[i] ) {
			remaining++;
		}
	}
	return remaining;
}

// One side is out of chips - show the finale banner/sound; UI_CardGame_RunFrame() resets once the countdown ends.
static void CardGame_StartGameOver( qboolean isWin ) {
	gameOverIsWin = isWin;
	gameOverCountdownStart = trap_Milliseconds();
	phase = CG_PHASE_GAME_OVER;
	CardGame_PlaySound( isWin ? SOUND_FINALE_WIN : SOUND_FINALE_LOSE );
}

// Compares the two decisive cards: win/lose pays the pot (currentBet << warDepth), a tie chains another war.
static void CardGame_ResolveShowdown( card_t p, card_t o ) {
	int pot = currentBet << warDepth;
	qboolean wasWar = ( warDepth > 0 );

	playerCard = p;
	opponentCard = o;
	hasResult = qtrue;

	if ( p.rank == o.rank ) {
		warDepth++;
		phase = CG_PHASE_WAR_ANNOUNCE;   // war table deals once UI_CardGame_RunFrame's announce beat ends
		warAnnounceUntil = trap_Milliseconds() + CARDGAME_WAR_ANNOUNCE_MS;
		Com_sprintf( cardgameResultText, sizeof( cardgameResultText ), translatedWarStart, currentBet << warDepth );
		CardGame_PlaySound( SOUND_WAR_START );
		return;
	}

	warDepth = 0;

	if ( p.rank > o.rank ) {
		sessionChips += pot;
		opponentChips -= pot;
		if ( opponentChips < 0 ) {
			opponentChips = 0;
		}
		resultIsWin = qtrue;
		CardGame_PlaySound( SOUND_WIN );
		if ( wasWar ) {
			CardGame_PlaySound( SOUND_WAR_END );
		}
		if ( opponentChips <= 0 ) {
			CardGame_StartGameOver( qtrue );
		} else {
			Q_strncpyz( cardgameResultText, translatedWin, sizeof( cardgameResultText ) );
			phase = CG_PHASE_ROUND_OVER;
		}
	} else {
		sessionChips -= pot;
		opponentChips += pot;
		resultIsWin = qfalse;
		CardGame_PlaySound( SOUND_LOSE );
		if ( wasWar ) {
			CardGame_PlaySound( SOUND_WAR_END );
		}
		if ( sessionChips <= 0 ) {
			CardGame_StartGameOver( qfalse );
		} else {
			Q_strncpyz( cardgameResultText, translatedLose, sizeof( cardgameResultText ) );
			phase = CG_PHASE_ROUND_OVER;
		}
	}
}

// Called once the opponent's delayed half of an exchange lands.
static void CardGame_ResolvePending( void ) {
	int oslot = pendingOpponentSlot;

	pendingRevealTime = 0;

	if ( !pendingIsWarPick ) {
		tableRevealed[oslot] = qtrue;
		CardGame_PlaySound( SOUND_REVEAL );
		CardGame_ResolveShowdown( playerCard, tableCards[oslot] );
		return;
	}

	tableEliminated[oslot] = qtrue;
	phase = CG_PHASE_WAR_TURN;   // hand control back to the player for the next discard or the final reveal
}

// Picks a fresh random "considering" slot for the opponent's hover illusion.
static void CardGame_RerollDisplaySlot( void ) {
	int candidate;

	if ( !pendingIsWarFinal ) {   // final reveal has exactly one live slot - nothing to flicker between
		candidate = pendingIsWarPick ? CardGame_RandomEligibleWarSlot( -1 ) : CardGame_RandomUnrevealedSlot( -1 );
		if ( candidate >= 0 ) {
			pendingDisplaySlot = candidate;
		}
	}
	pendingNextFlickerTime = trap_Milliseconds() + CARDGAME_AI_FLICKER_MS;
}

//=================================== public API ===================================

void UI_CardGame_Reset( void ) {
	int i;

	sessionChips = CARDGAME_START_CHIPS;
	opponentChips = CARDGAME_START_CHIPS;
	currentBet = 0;
	warDepth = 0;
	phase = CG_PHASE_BETTING;
	cardBackShader = 0;
	cardgameResultText[0] = '\0';
	hasResult = qfalse;
	pendingRevealTime = 0;
	gameOverCountdownStart = 0;
	warAnnounceUntil = 0;
	playerCard.rank = 0;
	opponentCard.rank = 0;

	for ( i = 0; i < CARDGAME_TABLE_SLOTS; i++ ) {
		slotActive[i] = qfalse;
	}
	for ( i = 0; i < CARDGAME_POOL_SIZE; i++ ) {
		cardFaceShaders[i] = -1;
	}

	CardGame_BuildAndShuffle();
	CardGame_ValidateAssets();
}

void UI_CardGame_RunFrame( void ) {
	if ( gameOverCountdownStart ) {
		int elapsedSec = ( trap_Milliseconds() - gameOverCountdownStart ) / 1000;
		if ( elapsedSec >= CARDGAME_FINALE_COUNTDOWN_SEC ) {
			UI_CardGame_Reset();
		}
		return;
	}

	if ( warAnnounceUntil ) {
		if ( trap_Milliseconds() >= warAnnounceUntil ) {
			warAnnounceUntil = 0;
			CardGame_DealWarTable();
			phase = CG_PHASE_WAR_TURN;
		}
		return;
	}

	if ( !pendingRevealTime ) {
		return;
	}
	if ( trap_Milliseconds() >= pendingRevealTime ) {
		CardGame_ResolvePending();
		return;
	}
	if ( trap_Milliseconds() >= pendingNextFlickerTime ) {
		CardGame_RerollDisplaySlot();
	}
}

int UI_CardGame_CurrentBet( void ) {
	return currentBet;
}

qboolean UI_CardGame_SetBet( int amount ) {
	if ( phase != CG_PHASE_BETTING || sessionChips <= 0 ) {
		return qfalse;
	}
	if ( amount < 1 ) {
		amount = 1;
	}
	if ( amount > sessionChips ) {
		amount = sessionChips;
	}
	if ( amount == currentBet ) {
		return qfalse;   // already clamped there - don't play the chip sound for a no-op click
	}
	currentBet = amount;
	CardGame_PlaySound( SOUND_CHIP );
	return qtrue;
}

void UI_CardGame_Deal( void ) {
	if ( phase != CG_PHASE_BETTING || currentBet <= 0 ) {
		return;
	}
	hasResult = qfalse;
	cardgameResultText[0] = '\0';
	playerCard.rank = 0;
	opponentCard.rank = 0;
	CardGame_DealNormalTable();
	phase = CG_PHASE_PLAYER_TURN;
	CardGame_PlaySound( SOUND_DEAL );
}

int UI_CardGame_TableSlotCount( void ) {
	int i, n = 0;

	for ( i = 0; i < CARDGAME_TABLE_SLOTS; i++ ) {
		if ( slotActive[i] ) {
			n++;
		}
	}
	return n;
}

qhandle_t UI_CardGame_TableSlotIcon( int slot ) {
	if ( slot < 0 || slot >= CARDGAME_TABLE_SLOTS || !slotActive[slot] || !CardGame_SlotHasLanded( slot ) ) {
		return 0;
	}
	if ( tableRevealed[slot] ) {
		return CardGame_FaceShader( tableCards[slot] );
	}
	if ( !cardBackShader ) {
		cardBackShader = trap_R_RegisterShaderNoMip( "ui/assets/cards/back.png" );
	}
	return cardBackShader;
}

qboolean UI_CardGame_TableSlotIsPlayable( int slot ) {
	if ( slot < 0 || slot >= CARDGAME_TABLE_SLOTS || !slotActive[slot] || !CardGame_SlotHasLanded( slot ) ) {
		return qfalse;
	}
	if ( phase == CG_PHASE_PLAYER_TURN ) {
		return !tableRevealed[slot];
	}
	if ( phase == CG_PHASE_WAR_TURN ) {
		return !tableEliminated[slot];
	}
	return qfalse;
}

// Durable state, independent of phase - unlike "not playable", stays true through the WAR_PENDING beat.
qboolean UI_CardGame_TableSlotIsEliminated( int slot ) {
	if ( slot < 0 || slot >= CARDGAME_TABLE_SLOTS || !slotActive[slot] ) {
		return qfalse;
	}
	return tableEliminated[slot];
}

// The slot currently highlighted as "the opponent is considering this one" - see CardGame_RerollDisplaySlot.
int UI_CardGame_PendingOpponentSlot( void ) {
	return pendingRevealTime ? pendingDisplaySlot : -1;
}

// Player's pick lands immediately; the opponent's response is deferred to UI_CardGame_RunFrame().
void UI_CardGame_PlayerPick( int slot ) {
	if ( slot < 0 || slot >= CARDGAME_TABLE_SLOTS || !slotActive[slot] ) {
		return;
	}

	if ( phase == CG_PHASE_PLAYER_TURN ) {
		int oslot;

		if ( tableRevealed[slot] ) {
			return;
		}
		tableRevealed[slot] = qtrue;
		CardGame_PlaySound( SOUND_REVEAL );
		playerCard = tableCards[slot];

		oslot = CardGame_RandomUnrevealedSlot( slot );
		if ( oslot < 0 ) {
			// window exhausted mid-round (shouldn't normally happen with 24 slots and 2 used/round)
			CardGame_DealNormalTable();
			tableRevealed[slot] = qtrue;
			oslot = CardGame_RandomUnrevealedSlot( slot );
		}

		pendingOpponentSlot = oslot;
		pendingDisplaySlot = oslot;
		pendingIsWarPick = qfalse;
		pendingIsWarFinal = qfalse;
		pendingRevealTime = trap_Milliseconds() + CARDGAME_PICK_DELAY_MS;
		pendingNextFlickerTime = trap_Milliseconds() + CARDGAME_AI_FLICKER_MS;
		phase = CG_PHASE_REVEAL;
		return;
	}

	if ( phase == CG_PHASE_WAR_TURN ) {
		int oslot;

		if ( tableEliminated[slot] || tableRevealed[slot] ) {
			return;
		}

		if ( CardGame_CountWarRemaining() <= 2 ) {
			// only the decisive pair is left - this pick reveals it instead of blindly discarding
			tableRevealed[slot] = qtrue;
			CardGame_PlaySound( SOUND_REVEAL );
			playerCard = tableCards[slot];

			oslot = CardGame_RandomEligibleWarSlot( slot );
			if ( oslot < 0 ) {
				return;   // shouldn't happen: the decisive pair always has exactly one other live slot
			}

			pendingOpponentSlot = oslot;
			pendingDisplaySlot = oslot;
			pendingIsWarPick = qfalse;
			pendingIsWarFinal = qtrue;
			pendingRevealTime = trap_Milliseconds() + CARDGAME_PICK_DELAY_MS;
			pendingNextFlickerTime = trap_Milliseconds() + CARDGAME_AI_FLICKER_MS;
			phase = CG_PHASE_WAR_PENDING;
			return;
		}

		tableEliminated[slot] = qtrue;
		CardGame_PlaySound( SOUND_DISCARD );

		oslot = CardGame_RandomEligibleWarSlot( slot );
		if ( oslot < 0 ) {
			return;   // shouldn't happen: war deals are always even-sized
		}

		pendingOpponentSlot = oslot;
		pendingDisplaySlot = oslot;
		pendingIsWarPick = qtrue;
		pendingIsWarFinal = qfalse;
		pendingRevealTime = trap_Milliseconds() + CARDGAME_PICK_DELAY_MS;
		pendingNextFlickerTime = trap_Milliseconds() + CARDGAME_AI_FLICKER_MS;
		phase = CG_PHASE_WAR_PENDING;
		return;
	}
}

cardGamePhase_t UI_CardGame_Phase( void ) {
	return phase;
}

qboolean UI_CardGame_HasResult( void ) {
	return hasResult;
}

// Meaningless during CG_PHASE_WAR_ANNOUNCE - the tie itself is the news, not a win or loss yet.
qboolean UI_CardGame_ResultIsWin( void ) {
	return resultIsWin;
}

void UI_CardGame_ContinueAfterRound( void ) {
	int i;

	if ( phase != CG_PHASE_ROUND_OVER ) {
		return;
	}
	currentBet = 0;
	hasResult = qfalse;
	cardgameResultText[0] = '\0';
	playerCard.rank = 0;
	opponentCard.rank = 0;
	for ( i = 0; i < CARDGAME_TABLE_SLOTS; i++ ) {
		slotActive[i] = qfalse;   // clear the table - the next deal starts fresh once a new bet is placed
	}
	phase = CG_PHASE_BETTING;
}

// One text region for the betting prompt, the round-over result, and the finale countdown.
const char *UI_CardGame_ResultText( void ) {
	static char finaleText[64];

	if ( phase == CG_PHASE_GAME_OVER ) {
		int elapsedSec = ( trap_Milliseconds() - gameOverCountdownStart ) / 1000;
		int remaining = CARDGAME_FINALE_COUNTDOWN_SEC - elapsedSec;

		if ( remaining < 1 ) {
			remaining = 1;
		}
		Com_sprintf( finaleText, sizeof( finaleText ), gameOverIsWin ? translatedFinaleWin : translatedFinaleLose, remaining );
		return finaleText;
	}
	if ( phase == CG_PHASE_BETTING ) {
		return ( currentBet > 0 ) ? translatedPressDeal : translatedPlaceBet;
	}
	return cardgameResultText;
}

// "You: N   Opponent: N   Bet/Pot: N" - third figure is the current bet, or the live pot once dealt.
const char *UI_CardGame_ChipsText( void ) {
	static char text[96];

	if ( phase == CG_PHASE_BETTING ) {
		Com_sprintf( text, sizeof( text ), translatedChipsBetting, sessionChips, opponentChips, currentBet );
	} else {
		Com_sprintf( text, sizeof( text ), translatedChipsPot, sessionChips, opponentChips, currentBet << warDepth );
	}
	return text;
}

int UI_CardGame_WarDepth( void ) {
	return warDepth;
}
