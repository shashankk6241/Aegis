#include "AegisItemDefinitionDataAsset.h"
#include "AegisWeapon.h"

bool FAegisItemInstance::CanStackWith(const FAegisItemInstance& Other) const
{
	if (!IsValid() || !Other.IsValid()) return false;
	if (ItemDefinition != Other.ItemDefinition) return false;
	if (UpgradeLevel != Other.UpgradeLevel) return false;
	return Quantity < ItemDefinition->MaxStackSize;
}

UAegisItemDefinitionDataAsset::UAegisItemDefinitionDataAsset()
{
	MaxStackSize = 1;
	VendorValue = 10;
	TargetSlot = EAegisEquipmentSlot::None;
	AttachSocket = FName("hand_r_socket");
	bConsumeOnUse = true;
}
