code

equ	trap_Print							-1
equ	trap_Error							-2
equ	trap_Milliseconds					-3
equ	trap_Cvar_Register					-4
equ	trap_Cvar_Update					-5
equ	trap_Cvar_Set						-6
equ trap_Cvar_VariableStringBuffer		-7
equ	trap_Argc							-8
equ	trap_Argv							-9
equ	trap_Args							-10
equ	trap_FS_FOpenFile					-11
equ	trap_FS_Read						-12
equ	trap_FS_Write						-13 
equ	trap_FS_FCloseFile					-14
equ	trap_SendConsoleCommand				-15
equ	trap_AddCommand						-16
equ	trap_SendClientCommand				-17
equ	trap_UpdateScreen					-18
equ	trap_CM_LoadMap						-19
equ	trap_CM_NumInlineModels				-20
equ	trap_CM_InlineModel					-21
equ	trap_CM_LoadModel					-22
equ	trap_CM_TempBoxModel				-23
equ	trap_CM_PointContents				-24
equ	trap_CM_TransformedPointContents	-25
equ	trap_CM_BoxTrace					-26
equ	trap_CM_TransformedBoxTrace			-27
equ trap_CM_CapsuleTrace				-28
equ trap_CM_TransformedCapsuleTrace		-29
equ trap_CM_TempCapsuleModel			-30
equ	trap_CM_MarkFragments				-31
equ	trap_S_StartSound					-32
equ trap_S_StartSoundEx					-33
equ	trap_S_StartLocalSound				-34
equ	trap_S_ClearLoopingSounds			-35
equ	trap_S_AddLoopingSound				-36
equ	trap_S_UpdateEntityPosition			-37
equ trap_S_GetVoiceAmplitude			-38
equ	trap_S_Respatialize					-39
equ	trap_S_RegisterSound				-40
equ	trap_S_StartBackgroundTrack			-41
equ	trap_S_FadeBackgroundTrack			-42
equ	trap_S_FadeAllSound					-43
equ	trap_S_StartStreamingSound			-44
equ	trap_S_GetCurrentSoundTime			-45
equ	trap_R_LoadWorldMap					-46
equ	trap_R_RegisterModel				-47
equ	trap_R_RegisterSkin					-48
equ	trap_R_RegisterShader				-49
equ trap_R_GetSkinModel					-50
equ trap_R_GetShaderFromModel			-51
equ trap_R_RegisterFont					-52
equ	trap_R_ClearScene					-53
equ	trap_R_AddRefEntityToScene			-54
equ trap_GetEntityToken					-55
equ	trap_R_AddPolyToScene				-56
equ	trap_R_AddPolysToScene				-57
equ	trap_RB_ZombieFXAddNewHit			-58
equ	trap_R_AddLightToScene				-59
equ trap_R_AddCoronaToScene				-60
equ trap_R_SetFog						-61
equ	trap_R_RenderScene					-62
equ	trap_R_SetColor						-63
equ	trap_R_DrawStretchPic				-64
equ trap_R_DrawStretchPicGradient		-65
equ	trap_R_ModelBounds					-66
equ	trap_R_LerpTag						-67
equ	trap_GetGlconfig					-68
equ	trap_GetGameState					-69
equ	trap_GetCurrentSnapshotNumber		-70
equ	trap_GetSnapshot					-71
equ	trap_GetServerCommand				-72
equ	trap_GetCurrentCmdNumber			-73
equ	trap_GetUserCmd						-74
equ	trap_SetUserCmdValue				-75
equ	trap_R_RegisterShaderNoMip			-76
equ	trap_MemoryRemaining				-77
equ trap_Key_IsDown						-78
equ trap_Key_GetCatcher					-79
equ trap_Key_SetCatcher					-80
equ trap_Key_GetKey						-81
equ trap_PC_AddGlobalDefine				-82
equ	trap_PC_LoadSource					-83
equ trap_PC_FreeSource					-84
equ trap_PC_ReadToken					-85
equ trap_PC_SourceFileAndLine			-86
equ trap_S_StopBackgroundTrack			-87
equ trap_RealTime						-88
equ trap_SnapVector						-89
equ trap_RemoveCommand					-90

; trap_R_LightForPoint ; not currently used (sorry, trying to keep CG_MEMSET @ 100)

equ trap_SendMoveSpeedsToGame			-91
equ trap_CIN_PlayCinematic				-92
equ trap_CIN_StopCinematic				-93
equ trap_CIN_RunCinematic 				-94
equ trap_CIN_DrawCinematic				-95
equ trap_CIN_SetExtents					-96
equ trap_R_RemapShader					

; trap_S_AddRealLoopingSound ; not currently used (sorry, trying to keep CG_MEMSET @ 100)

equ trap_S_StopLoopingSound				-98
equ trap_S_StopStreamingSound			-99
equ trap_loadCamera						-100
equ trap_startCamera					-101
equ trap_stopCamera						-102
equ trap_getCameraInfo					-103

equ	memset						-111
equ	memcpy						-112
equ	strncpy						-113
equ	sin							-114
equ	cos							-115
equ	atan2						-116
equ	sqrt						-117
equ floor						-118
equ	ceil						-119
equ	testPrintInt				-120
equ	testPrintFloat				-121
equ acos						-122

equ trap_UI_Popup				-123
equ trap_UI_ClosePopup			-124
equ trap_UI_LimboChat			-125
equ trap_GetModelInfo			-126

; New in iortcw
equ trap_Alloc					-901
equ trap_R_inPVS                -902

