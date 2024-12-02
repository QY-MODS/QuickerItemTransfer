#include "Utils.h"
#include "Settings.h"


static void SetupLog() {
    const auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    const auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
#else
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);
#endif
    logger::info("Name of the plugin is {}.", pluginName);
    logger::info("Version of the plugin is {}.", SKSE::PluginDeclaration::GetSingleton()->GetVersion());
}


namespace {
    void TransferItemsOfType(RE::TESObjectREFR* akSource, RE::TESObjectREFR* akTarget, const ItemTypes item_type) {
	    if (!akSource || !akTarget) return;
	    if (!IsItemType(item_type)) return;
		logger::trace("Transferring items of type {}", to_string(item_type));
        const bool bExcludeSpecials = akSource->IsPlayerRef();
        const auto filter_func = GetFilterFunc(item_type);
        std::vector<std::pair<RE::TESBoundObject*, std::int32_t>> forms;
        for (const auto akSource_inv = akSource->GetInventory();
            auto& [item,data] : akSource_inv) {
            if (!filter_func(item)) {
				logger::trace("Item {} is not of type {}", item->GetName(), to_string(item_type));
                continue;
            }
		    if (bExcludeSpecials && (data.second->IsWorn() || data.second->IsFavorited() || data.second->IsQuestObject())) continue;
			logger::trace("Transferring item {} of type {} from {} to {}", item->GetName(), to_string(item_type), akSource->GetName(), akTarget->GetName());
			forms.emplace_back(item, data.first);
	    }
		for (const auto& [item, count] : forms) {
		    akSource->RemoveItem(item, count, RE::ITEM_REMOVE_REASON::kRemove,nullptr, akTarget);
        }
		SKSE::GetTaskInterface()->AddUITask([akTarget, akSource]() {
            RE::SendUIMessage::SendInventoryUpdateMessage(akTarget,nullptr);
            RE::SendUIMessage::SendInventoryUpdateMessage(akSource,nullptr);
        });
    }

    void StartTransfer(RE::StaticFunctionTag*, const int iAction, const int iSubType = 0) {
	    const bool bIsTaking = iAction > 0 && iAction < 10;
	    const auto container = GetMenuContainer();
	    const auto player_ref = RE::PlayerCharacter::GetSingleton()->AsReference();
	    RE::TESObjectREFR* akSource = bIsTaking ? container :player_ref;
	    RE::TESObjectREFR* akTarget = bIsTaking ? player_ref: container;

        if (iAction == 1 || iAction == 12) {
            TransferItemsOfType(akSource, akTarget, kWeapon); // kWeapon
            TransferItemsOfType(akSource, akTarget, kAmmo); // kAmmo
        }
        else if (iAction == 2 || iAction == 13) {
            TransferItemsOfType(akSource, akTarget, kArmor); // kArmor
        }
        else if (iAction == 3 || iAction == 14) {
            TransferItemsOfType(akSource, akTarget, kPotion); // kPotion
        }
        else if (iAction == 4 || iAction == 15) {
            TransferItemsOfType(akSource, akTarget, kScrollItem); // kScrollItem
        }
        else if (iAction == 5 || iAction == 16) {
			if (iSubType == 0) TransferItemsOfType(akSource, akTarget, kRawFood);
			else if (iSubType == 1) TransferItemsOfType(akSource, akTarget, kCookedFood);
			else if (iSubType == 2) TransferItemsOfType(akSource, akTarget, kDrinks);
			else if (iSubType == 3) TransferItemsOfType(akSource, akTarget, kSweets);
        }
        else if (iAction == 6 || iAction == 17) {
            TransferItemsOfType(akSource, akTarget, kIngredient); // kIngredient
        }
        else if (iAction == 7 || iAction == 18) {
            TransferItemsOfType(akSource, akTarget, kBook); // kBook
        }
        else if (iAction == 8 || iAction == 19) {
            TransferItemsOfType(akSource, akTarget, kKey); // kKey
        }
        else if (iAction == 9 || iAction == 20) {
            if (iSubType == 0) TransferItemsOfType(akSource, akTarget, kSoulGem); // kSoulGem
            else if (iSubType == 1) TransferItemsOfType(akSource, akTarget, kOres);
			else if (iSubType == 2) TransferItemsOfType(akSource, akTarget, kGems);
			else if (iSubType == 3) TransferItemsOfType(akSource, akTarget, kLeatherNPelts);
			else if (iSubType == 4) TransferItemsOfType(akSource, akTarget, kBuildingMaterials);
        }
    }

    bool PapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
	    vm->RegisterFunction("StartTransfer", "QuickItemTransfer_Script", StartTransfer);
	    return true;
    }
    
}

void OnMessage(SKSE::MessagingInterface::Message* a_message) {
	if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
		logger::info("Data loaded");
		GetAllFormLists();
	    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions);
	}
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    SetupLog();
    logger::info("Plugin loaded");
    Init(skse);
	SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}