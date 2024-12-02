#include "Settings.h"
#include "FormReader.h"

void GetAllFormLists()
{
	GetAllRawFood();
	GetAllCookedFood();
	GetAllSweets();
	GetAllDrinks();
	GetAllOres();
	GetAllGems();
	GetAllLeatherNPelts();
	GetAllBuildingMaterials();
}

void GetFormList(const RE::FormID a_localid, std::set<FormID>& a_formset) {

	const auto formid = FormReader::GetForm("Quick Item Transfer.esp", a_localid);
	const auto formlist = RE::TESForm::LookupByID<RE::BGSListForm>(formid);
	if (!formlist) {
		logger::error("Form list with formid {:x} not found", formid);
		return;
	}
	for (const auto& item : formlist->forms) {
		a_formset.insert(item->GetFormID());
	}
}
