/*
===========================================================================

ui_armory.h -- ui.qvm side of the hub armory point-budget loadout builder.
Holds the in-progress build (which roster weapons / equipment are picked)
for the currently open "armory_loadout" menu. Nothing here is saved or
networked - it only matters while the menu is open, and is discarded
(reset) on open/cancel; a successful "confirm" sends the pick list to the
server, which is the sole source of truth for what actually gets granted.

===========================================================================
*/

#ifndef __UI_ARMORY_H__
#define __UI_ARMORY_H__

void UI_Armory_Reset( void );
void UI_Armory_LoadRosterForCurrentMap( void );

// Must run before UI_FreeTranslateTable() frees the table.
void UI_Armory_ResolveEquipTranslations( void );
void UI_Armory_ResolveWeaponDescTranslations( void );

int UI_Armory_WeaponCount( void );
const char  *UI_Armory_WeaponName( int index );
qhandle_t   UI_Armory_WeaponIcon( int index );
qboolean    UI_Armory_WeaponPicked( int index );
void        UI_Armory_ToggleWeapon( int index );

int UI_Armory_EquipCount( void );
const char  *UI_Armory_EquipName( int index );
qhandle_t   UI_Armory_EquipIcon( int index );
qboolean    UI_Armory_EquipPicked( int index );
void        UI_Armory_ToggleEquip( int index );

// Click highlights only; a separate "+" button adds the highlighted item, so a description can show first.
void        UI_Armory_SelectWeapon( int index );
void        UI_Armory_SelectEquip( int index );
void        UI_Armory_AddSelectedWeapon( void );
void        UI_Armory_AddSelectedEquip( void );
qhandle_t   UI_Armory_SelectedWeaponIcon( void );
const char  *UI_Armory_SelectedWeaponDesc( void );
qhandle_t   UI_Armory_SelectedEquipIcon( void );
const char  *UI_Armory_SelectedEquipDesc( void );

// Combined "current build" list: picked weapons then picked equipment, same select-then-act flow ("-Remove" button).
int UI_Armory_BuildCount( void );
const char  *UI_Armory_BuildName( int index );
qhandle_t   UI_Armory_BuildIcon( int index );
void        UI_Armory_RemoveBuildIndex( int index );
void        UI_Armory_SelectBuild( int index );
void        UI_Armory_RemoveSelectedBuild( void );

int UI_Armory_PointsTotal( void );
int UI_Armory_PointsUsed( void );

void UI_Armory_ApplyRecommended( void );
void UI_Armory_Randomize( void );

// Fills out (size outSize) with "sp_loadout_confirm <weapons> <equip>\n"
void UI_Armory_BuildConfirmCommand( char *out, int outSize );

#endif // __UI_ARMORY_H__
