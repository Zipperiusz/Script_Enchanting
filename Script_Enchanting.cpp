#include "Script_Enchanting.h"

#include "util\Memory.h"
#include "util\Logging.h"
#include "util\Hook.h"
#include "util/Util.h"
#include "util/ScriptUtil.h"
#include "zSpy.h"
#include "Script.h"
gSScriptInit& GetScriptInit()
{
	static gSScriptInit s_ScriptInit;
	return s_ScriptInit;
}

GEBool CanEnchant(PSInventory& inv, GEInt stack, GEU32 quality) {
	GEU32 ItemQuality = inv.GetQuality(stack);
	if ((ItemQuality & quality) == quality) {
		return GEFalse;
	}
	return GETrue;
}

GEBool EnchantWeapon(Entity& item, Entity& target, GEBool replace) {

	PSInventory& Inv = target.Inventory;
	// Staff has 2 possible slots so check if its in left hand
	Entity Staff = Inv.GetItemFromSlot(gESlot_LeftHand);
	Entity Weapon = Inv.GetItemFromSlot(gESlot_BackRight);
	GEInt index;
	if (Staff.Item.IsValid()) {
		index = Inv.FindStackIndex(gESlot_LeftHand);
	}
	else if (Weapon.Item.IsValid()) {
		index = Inv.FindStackIndex(gESlot_BackRight);
	}
	else {
		//Print no item in slot
		return GEFalse;
	}
	gEUseType useType = Inv.GetUseType(index);
	if (!(
		(useType == gEUseType_1H) ||
		(useType == gEUseType_2H) ||
		(useType == gEUseType_Staff) ||
		(useType == gEUseType_Halberd) ||
		(useType == gEUseType_Axe)
		)) {
		//Print wrong item in slot
		return GEFalse;
	}

	GEU32 quality = item.Item.GetQuality();

	if (!CanEnchant(Inv, index, quality)) {
		return GEFalse;
	}

	// Unequip item to make sure stats are applied for effect changing stats
	Inv.UnEquipStack(index);
	// If weapon is broken/forged/sharp make sure it wont erase effect
	GEU32 FinalQuality = (Inv.GetQuality(index) & gEItemQuality_Worn) | quality;
	FinalQuality = (FinalQuality & gEItemQuality_Forged) | quality;
	FinalQuality = (FinalQuality & gEItemQuality_Sharp) | quality;
	GEU32 newIndex;
	// Clear other enchantments or add new without removing others
	if (replace) {
		newIndex = Inv.SetQuality(index, FinalQuality, 1);
	}
	else {
		newIndex = Inv.ApplyQuality(index, FinalQuality, 1);
	}
	Inv.EquipStackToSlot(newIndex, gESlot_BackRight);
	return GETrue;
}

GEBool EnchantShield(Entity& item, Entity& target, GEBool replace) {
	PSInventory& Inv = target.Inventory;
	Entity ItemEntity = Inv.GetItemFromSlot(gESlot_BackLeft);
	if (!ItemEntity.Item.IsValid())
	{
		return GEFalse;
	}
	GEInt index = Inv.FindStackIndex(gESlot_BackLeft);
	gEUseType useType = Inv.GetUseType(index);
	if (useType != gEUseType_Shield) {
		return GEFalse;
	}
	GEU32 quality = item.Item.GetQuality();

	if (!CanEnchant(Inv, index, quality)) {
		return GEFalse;
	}
	// Unequip item to make sure stats are applied for effect changing stats
	Inv.UnEquipStack(index);
	// If shield is broken make sure it wont erase worn effect
	GEU32 FinalQuality = (Inv.GetQuality(index) & gEItemQuality_Worn) | quality;
	GEU32 newIndex;
	// Clear other enchantments or add new without removing others
	if (replace) {
		newIndex = Inv.SetQuality(index, FinalQuality, 1);
	}
	else {
		newIndex = Inv.ApplyQuality(index, FinalQuality, 1);
	}
	Inv.EquipStackToSlot(newIndex, gESlot_BackLeft);
	return GETrue;
}

GEBool RepairItem(Entity& target, gESlot slot) {
	PSInventory& Inv = target.Inventory;
	Entity ItemEntity = Inv.GetItemFromSlot(slot);
	// Staff has 2 possible slots so check if its in left hand
	Entity Staff = Inv.GetItemFromSlot(gESlot_LeftHand);
	Entity equipment = Inv.GetItemFromSlot(slot);
	GEInt index;
	if (Staff.Item.IsValid()) {
		index = Inv.FindStackIndex(gESlot_LeftHand);
	}
	else if (equipment.Item.IsValid()) {
		index = Inv.FindStackIndex(slot);
	}
	else {
		//Print no item in slot
		return GEFalse;
	}
	gEUseType useType = Inv.GetUseType(index);
	// Fixable useTypes
	if (!(
		(useType == gEUseType_1H) ||
		(useType == gEUseType_2H) ||
		(useType == gEUseType_Staff) ||
		(useType == gEUseType_Halberd) ||
		(useType == gEUseType_Axe) ||
		(useType == gEUseType_Shield)
		)) {
		return GEFalse;
	}
	GEU32 CurrentQuality = Inv.GetQuality(index);
	// Check if shield is broken
	if ((CurrentQuality & gEItemQuality_Worn) != gEItemQuality_Worn) {
		//Print item doenst need repair
		return GEFalse;
	}
	// Remove worn effect
	GEU32 FixedQuality = (CurrentQuality & ~gEItemQuality_Worn);
	// Remove item to make sure stats from fixed item are applied
	Inv.UnEquipStack(index);
	GEU32 newIndex = Inv.SetQuality(index, FixedQuality, 1);
	Inv.EquipStackToSlot(newIndex, slot);
	return GETrue;
}

ME_DEFINE_AND_REGISTER_SCRIPT(SetWeaponEnchant) {
	INIT_SCRIPT();	
	if (!EnchantWeapon(SelfEntity, OtherEntity)) {
		StartSaySVM(Entity::GetPlayer(), None, "DoesntWork_0"+ bCString(Entity::GetRandomNumber(3)), 0);
		// Return item if enchant wasnt applied
		if (!OtherEntity.Inventory.IsPermanent(OtherEntity.Inventory.FindStackIndex(SelfEntity.GetTemplate()))) {
			OtherEntity.Inventory.CreateItems(SelfEntity.GetTemplate(), SelfEntity.Item.GetQuality(), 1);
		}
		return 0;
	}

	return 1;
}

ME_DEFINE_AND_REGISTER_SCRIPT(ApplyWeaponEnchant) {
	INIT_SCRIPT();	
	if (!EnchantWeapon(SelfEntity, OtherEntity, GEFalse)) {
		StartSaySVM(Entity::GetPlayer(), None, "DoesntWork_0"+ bCString(Entity::GetRandomNumber(3)), 0);
		//Return item
		if (!OtherEntity.Inventory.IsPermanent(OtherEntity.Inventory.FindStackIndex(SelfEntity.GetTemplate()))) {
			OtherEntity.Inventory.CreateItems(SelfEntity.GetTemplate(), SelfEntity.Item.GetQuality(), 1);
		}
		return 0;
	}

	return 1;
}

ME_DEFINE_AND_REGISTER_SCRIPT(ApplyShieldEnchant) {
	INIT_SCRIPT();
	if (!EnchantShield(SelfEntity, OtherEntity, GEFalse)) {
		StartSaySVM(Entity::GetPlayer(), None, "DoesntWork_0"+ bCString(Entity::GetRandomNumber(3)), 0);
		//Return item
		if (!OtherEntity.Inventory.IsPermanent(OtherEntity.Inventory.FindStackIndex(SelfEntity.GetTemplate()))) {
			OtherEntity.Inventory.CreateItems(SelfEntity.GetTemplate(), SelfEntity.Item.GetQuality(), 1);
		}
		return 0;
	}

	return 1;
}

ME_DEFINE_AND_REGISTER_SCRIPT(SetShieldEnchant) {
	INIT_SCRIPT();
	if (!EnchantShield(SelfEntity, OtherEntity)) {
		StartSaySVM(Entity::GetPlayer(), None, "DoesntWork_0"+ bCString(Entity::GetRandomNumber(3)), 0);
		// Return item if consumable and enchant wasnt possible
		if(!OtherEntity.Inventory.IsPermanent(OtherEntity.Inventory.FindStackIndex(SelfEntity.GetTemplate()))){
			OtherEntity.Inventory.CreateItems(SelfEntity.GetTemplate(), SelfEntity.Item.GetQuality(), 1);
		}
		return 0;
	}

	return 1;
}

ME_DEFINE_AND_REGISTER_SCRIPT(RepairShield) {
	INIT_SCRIPT();
	if (!RepairItem(OtherEntity, gESlot::gESlot_BackLeft)) {
		StartSaySVM(Entity::GetPlayer(), None, "DoesntWork_0"+ bCString(Entity::GetRandomNumber(3)), 0);
		// Return item if enchant wasnt applied
		if(!OtherEntity.Inventory.IsPermanent(OtherEntity.Inventory.FindStackIndex(SelfEntity.GetTemplate()))){
			OtherEntity.Inventory.CreateItems(SelfEntity.GetTemplate(), SelfEntity.Item.GetQuality(), 1);
		}
		return 0;
	}
	return 1;
}

ME_DEFINE_AND_REGISTER_SCRIPT(RepairWeapon) {
	INIT_SCRIPT();
	if (!RepairItem(OtherEntity, gESlot::gESlot_BackRight)) {
		StartSaySVM(Entity::GetPlayer(), None, "DoesntWork_0"+ bCString(Entity::GetRandomNumber(3)), 0);
		// Return item if enchant wasnt applied
		if (!OtherEntity.Inventory.IsPermanent(OtherEntity.Inventory.FindStackIndex(SelfEntity.GetTemplate()))) {
			OtherEntity.Inventory.CreateItems(SelfEntity.GetTemplate(), SelfEntity.Item.GetQuality(), 1);
		}
		return 0;
	}
	return 1;
}

extern "C" __declspec(dllexport)
gSScriptInit const* GE_STDCALL ScriptInit(void)
{
	GetScriptAdmin().LoadScriptDLL("Script_Game.dll");
	GetScriptAdmin().LoadScriptDLL("Script_G3Fixes.dll");
	if (!GetScriptAdmin().IsScriptDLLLoaded("Script_G3Fixes.dll")) {
		GE_FATAL_ERROR_EX("Script_Enchanting", "Missing Script_G3Fixes.dll.");
	}
	spy = new zSpy();
	spy->Send("Enchanting script loaded.");
	return &GetScriptInit();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		::DisableThreadLibraryCalls(hModule);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
