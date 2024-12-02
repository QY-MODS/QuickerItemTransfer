#include "Utils.h"

RE::TESObjectREFR* GetMenuContainer()
{
	RE::TESObjectREFR* container = nullptr;

	const auto UI = RE::UI::GetSingleton();
    if (const auto menu = UI ? UI->GetMenu<RE::ContainerMenu>() : nullptr) {
		const auto           refHandle = menu->GetTargetRefHandle();
		RE::TESObjectREFRPtr refr;
		RE::LookupReferenceByHandle(refHandle, refr);

		container = refr.get();
	}

	return container;
}
