/*
 * SidebarView.cpp
 *
 *  Created on: Feb 22, 2016
 *      Author: Gilang M. Hamidy (g.hamidy@samsung.com)
 */

#include "SRIN/Components/SidebarView.h"

using namespace SRIN::Components;

LIBAPI SidebarView::SidebarView() :
	Framework::ViewBase()
{
	layout = leftPanel = background = currentContent = nullptr;
	drawerButtonClick += { this, &SidebarView::OnDrawerButtonClick };
}

LIBAPI Evas_Object* SidebarView::CreateView(Evas_Object* root)
{
	// Create base layout
	layout = elm_layout_add(root);
	elm_layout_theme_set(layout, "layout", "drawer", "panel");

	// Create panel layout for sidebar
	leftPanel = elm_panel_add(layout);
	elm_panel_scrollable_set(leftPanel, EINA_TRUE);
	elm_panel_orient_set(leftPanel, ELM_PANEL_ORIENT_LEFT);
	elm_panel_hidden_set(leftPanel, EINA_TRUE);

	// Create sidebar content from subclass
	auto sidebarContent = CreateSidebar(leftPanel);
	elm_object_content_set(leftPanel, sidebarContent);

	// Set the panel to base layout
	elm_object_part_content_set(layout, "elm.swallow.left", leftPanel);

	return layout;
}

LIBAPI void SidebarView::Attach(ViewBase* view)
{
	if (currentContent != nullptr)
		Detach();

	currentContent = view->Create(layout);
	elm_object_part_content_set(layout, "elm.swallow.content", currentContent);
}

LIBAPI void SidebarView::Detach()
{
	elm_object_part_content_unset(layout, "elm.swallow.content");
	currentContent = nullptr;
}

LIBAPI Evas_Object* SidebarView::GetTitleLeftButton(CString* buttonPart)
{
	auto btn = elm_button_add(layout);
	evas_object_smart_callback_add(btn, "clicked", SmartEventHandler, &drawerButtonClick);

	DrawerButtonStyle(btn);
	//*buttonPart = "drawers";
	return btn;
}

LIBAPI void SidebarView::OnDrawerButtonClick(ElementaryEvent* eventSource, Evas_Object* objSource, void* eventData)
{
	ToggleSidebar();
}

Evas_Object* SidebarView::GetTitleRightButton(CString* buttonPart)
{
	return nullptr;
}

LIBAPI CString SRIN::Components::SidebarView::GetContentStyle()
{
	return nullptr; //"drawers";
}

void SRIN::Components::SidebarView::DrawerButtonStyle(Evas_Object* button)
{
	elm_object_style_set(button, "naviframe/drawers");
}

void SRIN::Components::SidebarView::ToggleSidebar()
{
	if (!elm_object_disabled_get(leftPanel))
		elm_panel_toggle(leftPanel);
}