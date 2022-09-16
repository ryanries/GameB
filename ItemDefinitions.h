#pragma once

// IMPORTANT: Do not create an item with an Id of 0.

ITEM gArmor00 = {
    .Id = 1,
    .Name = "Commoner's Clothes",
    .Class = ITEMCLASS_ARMOR,
    .Description = "Your mother sewed these garments just for you, on your 18th birthday. They're your favorite shade of blue.",
    .Value = 2,
    .Damage = 0,
    .Defense = 1
};

ITEM gWeapon00 = {
    .Id = 2,
    .Name = "Walking Stick",
    .Class = ITEMCLASS_WEAPON,
    .Description = "You found this stick by the shore and were impressed by its sturdiness and straightness. It's a pretty good stick.",
    .Value = 3,
    .Damage = 1,
    .Defense = 0
};

ITEM gPotion00 = {
    .Id = 3,
    .Name = "Pork Dumpling",
    .Class = ITEMCLASS_POTION,
    .Description = "A delicious pork dumpling wrapped in leaves. It has notes of scallions and garlic.",
    .Value = 1
    // TODO: ACTION
};