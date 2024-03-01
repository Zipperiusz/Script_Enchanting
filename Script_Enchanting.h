#ifndef SCRIPTLEARNPOINTS_H_INCLUDED
#define SCRIPTLEARNPOINTS_H_INCLUDED

#include "Script.h"
#include "util\Memory.h"
#include "util\Logging.h"
#include "util\Hook.h"
#include "util/Util.h"
#include "util/ScriptUtil.h"
#include "zSpy.h"

gSScriptInit &GetScriptInit();

#include "Game.h"

GEBool CanEnchant(PSInventory &inv, GEInt stack, GEU32 quality);
GEBool EnchantWeapon(Entity& item, Entity& target, GEBool replace = GETrue);
GEBool EnchantShield(Entity& item, Entity& target, GEBool replace = GETrue);
GEBool RepairItem(Entity &target, gESlot slot);
#endif