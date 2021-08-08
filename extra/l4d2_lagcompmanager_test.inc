#if defined _l4d2_lagcompmanager_test
	#endinput
#endif
#define _l4d2_lagcompmanager_test

/**
 * @brief Displays in the server console all entities that are in the array for the lag compensation
 * @remarks Basically this lagcomp array is always empty, so don't be surprised you won't see anything in the console
 * @remarks This works, for example, for melee weapons or for the boomer's ability, at this moment, entities should appear in this array
 *
 * @noreturn
 */
native void LagComp_ShowAllEntities();

/**
 * @brief Finds the specified entity in the array for the lag compensation
 *
 * @param iFindEntityIndex			The index of the entity that was found, for additional check
 *
 * @return							True if the entity was found, false otherwise.
*/
native bool LagComp_FindEntity(int iEntity, int &iFindEntityIndex);


public Extension __ext_l4d2_lagcompmanager_test =
{
	name = "L4D2 LagCompManager Test",
	file = "l4d2_lagcompmanager_test.ext",

#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};
