/*
 * ContextMenu.cpp
 *
 *  Created on: Apr 26, 2016
 *      Author: Gilang M. Hamidy (g.hamidy@samsung.com)
 */

#include "SRIN/Components/ContextMenu.h"
#include <algorithm>

Evas_Object* SRIN::Components::ContextMenu::CreateComponent(Evas_Object* root)
{
	naviframe = root;
	button = elm_button_add(root);
	evas_object_smart_callback_add(button, "clicked", EFL::EvasSmartEventHandler, &eventContextMenuButtonClicked);
	elm_object_style_set(button, "naviframe/more/default");
	return button;
}

void ContextMenu_ItemClickHandler(void* data, Evas_Object* obj, void* eventData)
{
	auto pkg = reinterpret_cast<SRIN::Components::ContextMenu::ContextMenuPackage*>(data);
	pkg->RaiseEvent();
}

void SRIN::Components::ContextMenu::SetText(const std::string& text)
{
}

std::string& SRIN::Components::ContextMenu::GetText()
{

}

SRIN::Components::ContextMenu::ContextMenu() :
	naviframe(nullptr),
	button(nullptr),
	menuShown(false),
	Text(this),
	contextMenu(nullptr)
{
	eventContextMenuButtonClicked += AddEventHandler(ContextMenu::OnContextMenuButtonClicked);
	eventContextMenuDismissed += AddEventHandler(ContextMenu::OnContextMenuDismissed);
}

void SRIN::Components::ContextMenu::AddMenu(MenuItem* menu)
{
	rootMenu.push_back(menu);
}

void SRIN::Components::ContextMenu::AddMenuAt(int index, MenuItem* menu)
{
	if(index >= this->rootMenu.size())
		AddMenu(menu);
	else
	{
		auto pos = this->rootMenu.begin() + index;
		this->rootMenu.insert(pos, menu);
	}
}

void SRIN::Components::ContextMenu::RemoveMenu(MenuItem* menu)
{
	auto pos = std::find(rootMenu.begin(), rootMenu.end(), menu);
	rootMenu.erase(pos);
}

void SRIN::Components::ContextMenu::AddMenu(const std::vector<MenuItem*>& listOfMenus)
{
	for (auto item : listOfMenus)
	{
		this->AddMenu(item);
	}
}

void SRIN::Components::ContextMenu::SetMenu(const std::vector<MenuItem*>& listOfMenus)
{
	rootMenu = listOfMenus;
}

void SRIN::Components::ContextMenu::OnContextMenuButtonClicked(EFL::EvasSmartEvent* ev, Evas_Object* obj, void* eventData)
{
	if(not this->menuShown)
		this->ShowMenu();
	else
		this->HideMenu();
}

void SRIN::Components::ContextMenu::ShowMenu()
{
	BackButtonHandler::Acquire();

	auto contextMenu = elm_ctxpopup_add(this->naviframe);
	evas_object_smart_callback_add(contextMenu, "dismissed", EFL::EvasSmartEventHandler, &eventContextMenuDismissed);
	elm_object_style_set(contextMenu, "dropdown/label");

	this->contextMenu = contextMenu;

	this->menuShown = true;
	for(auto item : rootMenu)
	{
		Evas_Object* img = nullptr;
		if(item->MenuIcon->length())
		{
			img = elm_image_add(contextMenu);
			elm_image_file_set(img, Framework::ApplicationBase::GetResourcePath(item->MenuIcon->c_str()).c_str(), nullptr);
		}
		auto itemPackage = new ContextMenuPackage({this, item});
		this->currentItemPackages.push_back(itemPackage);
		elm_ctxpopup_item_append(contextMenu, item->Text->c_str(), img, ContextMenu_ItemClickHandler, itemPackage);
	}

	Evas_Coord x, y, w , h;
	evas_object_geometry_get(this->button, &x, &y, &w, &h);
	elm_ctxpopup_direction_priority_set(contextMenu,
			ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);


	evas_object_move(contextMenu, x, y + h);

	evas_object_show(contextMenu);
}

void SRIN::Components::ContextMenu::HideMenu()
{
	if(this->contextMenu != nullptr)
	{
		evas_object_del(this->contextMenu);
		this->contextMenu = nullptr;

		for(auto pkg : this->currentItemPackages)
		{
			delete pkg;
		}

		this->currentItemPackages.clear();
	}
	this->menuShown = false;
	BackButtonHandler::Release();
}

bool SRIN::Components::ContextMenu::BackButtonClicked()
{
	elm_ctxpopup_dismiss(this->contextMenu);
	return false;
}

SRIN::Components::ContextMenu::~ContextMenu()
{
	HideMenu();
}

void SRIN::Components::ContextMenu::OnContextMenuDismissed(EFL::EvasSmartEvent* ev,
		Evas_Object* obj, void* eventData) {
	HideMenu();
}

void SRIN::Components::ContextMenu::ContextMenuPackage::RaiseEvent() {
	this->thisRef->RaiseOnClickEvent(this->menuItemRef);
}

void SRIN::Components::ContextMenu::RaiseOnClickEvent(MenuItem* menuItemRef) {
	elm_ctxpopup_dismiss(this->contextMenu);
	menuItemRef->OnMenuItemClick(menuItemRef, nullptr);
}
