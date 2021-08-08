/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod LagCompManager Extension
 * Copyright (C) 2021 A1mDev. All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "extension.h"

#define GAMEDATA_FILE "l4d2_lagcomp_manager"

CGlobalVars *gpGlobals = NULL;

IServerGameEnts *gameents = NULL;
ICvar *icvar = NULL;
IGameConfig *g_pGameConf = NULL;

CLagCompensationManager* CLagCompensationManager::LagCompInstance = NULL;
int CLagCompensationManager::offset_m_AdditionalEntitiesPool = 0;

LagCompManager g_LagCompManager;
SMEXT_LINK(&g_LagCompManager);

bool LagCompManager::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late)
{
	size_t maxlen = maxlength;
	gpGlobals = ismm->GetCGlobals();
	GET_V_IFACE_ANY(GetServerFactory, gameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	//GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	g_pCVar = icvar;
	
	return true;
}

bool LagCompManager::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	char conf_error[255];
	if (!gameconfs->LoadGameConfigFile(GAMEDATA_FILE, &g_pGameConf, conf_error, sizeof(conf_error))) {
		if (!strlen(conf_error)) {
			snprintf(error, maxlength, "[LagCompManager] Could not read %s.txt: %s", GAMEDATA_FILE, conf_error);
		} else {
			snprintf(error, maxlength, "[LagCompManager] Could not read %s.txt.", GAMEDATA_FILE);
		}
		return false;
	}
	
	char *addr = NULL;
	
	if (!g_pGameConf->GetAddress("lagcompensation", (void **)&addr) || !addr) {
		snprintf(error, maxlength, "[LagCompManager] Couldn't find \"lagcompensation\" instance!");
		return false;
	}
	
	CLagCompensationManager::LagCompInstance = reinterpret_cast<CLagCompensationManager *>(addr);
	
	if (!g_pGameConf->GetOffset("CLagCompensationManager->m_AdditionalEntities", &CLagCompensationManager::offset_m_AdditionalEntitiesPool) || !CLagCompensationManager::offset_m_AdditionalEntitiesPool) {
		snprintf(error, maxlength, "[LagCompManager] Unable to get offset for \"CLagCompensationManager->m_AdditionalEntities\"!");
		return false;
	}
	
	return true;
}

void LagCompManager::SDK_OnAllLoaded()
{
	sharesys->AddNatives(myself, g_CheckNatives);
	sharesys->RegisterLibrary(myself, "l4d2_lagcompmanager_test");
}

void LagCompManager::SDK_OnUnload()
{
	gameconfs->CloseGameConfigFile(g_pGameConf);
}

cell_t LagComp_ShowAllEntities(IPluginContext *pContext, const cell_t *params)
{
	for (int i = CLagCompensationManager::LagCompInstance->m_AdditionalEntitiesPool().FirstInorder(); \
		i != CLagCompensationManager::LagCompInstance->m_AdditionalEntitiesPool().InvalidIndex(); \
		i = CLagCompensationManager::LagCompInstance->m_AdditionalEntitiesPool().NextInorder(i)
	) {
		//CBaseEntity *pAddEntity = CLagCompensationManager::LagCompInstance->m_AdditionalEntitiesPool()[i].Get();
		edict_t *pAddEdict = gamehelpers->GetHandleEntity(CLagCompensationManager::LagCompInstance->m_AdditionalEntitiesPool()[i]);
		CBaseEntity *pAddEntity = gameents->EdictToBaseEntity(pAddEdict);
		if (!pAddEntity) {
			rootconsole->ConsolePrint("[Error] Invalid entity m_AdditionalEntities[%d]: address: %08x", i, pAddEntity);
			continue;
		}

		int index = gamehelpers->EntityToBCompatRef(pAddEntity);
		const char *classname = gamehelpers->GetEntityClassname(pAddEntity);
		rootconsole->ConsolePrint("m_AdditionalEntities[%d]: entity index: %d, name: %s, address: %08x", i, index, classname, pAddEntity);
	}

	return 1;
}

cell_t LagComp_FindEntity(IPluginContext *pContext, const cell_t *params)
{
	int target = params[1];
	
	cell_t *addr;
	pContext->LocalToPhysAddr(params[2], &addr);
	*addr = 0;
	
	//this + 88
	CBaseEntity *pEntity = UTIL_GetCBaseEntity(target, false);
	if (pEntity == NULL) {
		return pContext->ThrowNativeError("The specified object is invalid '%d'!", target);
	}
	
	EHANDLE eh = pEntity;
	short unsigned int iIndex = CLagCompensationManager::LagCompInstance->m_AdditionalEntitiesPool().Find(eh);
	if (iIndex != CLagCompensationManager::LagCompInstance->m_AdditionalEntitiesPool().InvalidIndex()) {
		//CBaseEntity *pGetEntity = CLagCompensationManager::LagCompInstance->m_AdditionalEntitiesPool()[iIndex].Get();
		edict_t *pGetEdict = gamehelpers->GetHandleEntity(CLagCompensationManager::LagCompInstance->m_AdditionalEntitiesPool()[iIndex]);
		CBaseEntity *pGetEntity = gameents->EdictToBaseEntity(pGetEdict);
		if (pGetEntity == NULL) {
			return pContext->ThrowNativeError("An invalid object was received from the array! Index: %d", iIndex);
		}
		
		*addr = gamehelpers->EntityToBCompatRef(pGetEntity);
		return 1;
	}

	return 0;
}

sp_nativeinfo_t g_CheckNatives[] = 
{
	{"LagComp_ShowAllEntities",		LagComp_ShowAllEntities},
	{"LagComp_FindEntity",			LagComp_FindEntity},
	{NULL,							NULL},
};
