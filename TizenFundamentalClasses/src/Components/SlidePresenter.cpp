/*
 * Tizen Fundamental Classes - TFC
 * Copyright (c) 2016-2017 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *    Components/SlidePresenter.cpp
 *
 * Created on:  Sep 14, 2016
 * Author: 		Gilang Mentari Hamidy (g.hamidy@samsung.com)
 * Contributor: Kevin Winata (k.winata@samsung.com)
 */

#include "TFC/Components/SlidePresenter.h"
#include "TFC/Framework/Application.h"

#include <algorithm>

#define FILE_EDC_SLIDEPRESENTER "TFC/edc/SlidePresenter.edj"

void TFC::Components::SlidePresenter::OnLayoutResize(EFL::EvasEventSourceInfo objSource, void* event_data)
{
	Evas_Coord w, h;

	evas_object_geometry_get(objSource.obj, NULL, NULL, &w, &h);

	for(auto page : this->pageList)
	{
		evas_object_size_hint_min_set(page, w, h);
		evas_object_size_hint_max_set(page, w, h);
	}

	elm_scroller_page_size_set(this->scroller, w, h);
}

void TFC::Components::SlidePresenter::OnPageScrolled(Evas_Object* source, void* event_data)
{
	int pageH, pageV;
	elm_scroller_current_page_get(source, &pageH, &pageV);

	auto item = elm_index_item_find(this->index, this->pageList[pageH]);
	elm_index_item_selected_set(item, EINA_TRUE);
	eventPageChanged(this, pageH);
}

Evas_Object* TFC::Components::SlidePresenter::CreateComponent(
		Evas_Object* root) {
	auto layout = elm_layout_add(root);
	auto edj_path = Framework::ApplicationBase::GetResourcePath(FILE_EDC_SLIDEPRESENTER);
	elm_layout_file_set(layout, edj_path.c_str(), "slide_presenter");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//evas_object_event_callback_add(layout, EVAS_CALLBACK_RESIZE, EFL::EvasObjectEventHandler, &this->eventLayoutResizeInternal);
	eventLayoutResizeInternal.Bind(layout, EVAS_CALLBACK_RESIZE);

	this->scroller = elm_scroller_add(layout);
	elm_scroller_loop_set(this->scroller, EINA_FALSE, EINA_FALSE);
	evas_object_size_hint_weight_set(this->scroller, 0.5, 0.5);
	evas_object_size_hint_align_set(this->scroller, 0.5, 0.5);
	elm_scroller_page_relative_set(this->scroller, 1.0, 0.0);
	elm_scroller_policy_set(this->scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(this->scroller, 1, 0);
	elm_object_scroll_lock_y_set(this->scroller, EINA_TRUE);
	evas_object_size_hint_weight_set(this->scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(this->scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(this->scroller);
	//evas_object_smart_callback_add(this->scroller, "scroll,page,changed", EFL::EvasSmartEventHandler, &this->eventPageScrolledInternal);
	eventPageScrolledInternal.Bind(this->scroller, "scroll,page,changed");

	elm_object_part_content_set(layout, "scroller", this->scroller);

	// Box inside scroller
	this->box = elm_box_add(this->scroller);
	elm_box_horizontal_set(this->box, EINA_TRUE);
	elm_object_content_set(this->scroller, this->box);
	evas_object_size_hint_weight_set(this->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(this->box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(this->box);
	elm_box_homogeneous_set(this->box, EINA_TRUE);

	// Index page
	this->index = elm_index_add(layout);
	evas_object_size_hint_weight_set(this->index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(this->index, 0.5, 1.0);
	elm_object_style_set(this->index, "pagecontrol");
	elm_index_horizontal_set(this->index, EINA_TRUE);
	elm_index_autohide_disabled_set(this->index, EINA_TRUE);
	elm_object_part_content_set(layout, "index", this->index);
	//evas_object_smart_callback_add(this->index, "changed", EFL::EvasSmartEventHandler, &this->eventIndexSelectedInternal);
	eventIndexSelectedInternal.Bind(this->index, "changed");

	return layout;
}

TFC::Components::SlidePresenter::SlidePresenter() :
	box(nullptr), scroller(nullptr), index(nullptr){

	this->eventLayoutResizeInternal += EventHandler(SlidePresenter::OnLayoutResize);
	this->eventPageScrolledInternal += EventHandler(SlidePresenter::OnPageScrolled);
	this->eventIndexSelectedInternal += EventHandler(SlidePresenter::OnIndexSelected);
}

TFC::Components::SlidePresenter::~SlidePresenter() {

}


void TFC::Components::SlidePresenter::InsertPage(Evas_Object* page) {
	this->pageList.push_back(page);
	elm_box_pack_end(this->box, page);
	auto indexItem = elm_index_item_append(this->index, nullptr, nullptr, page);
	evas_object_show(page);

	if(this->pageList.size() == 1)
		elm_index_item_selected_set(indexItem, EINA_TRUE);
}

void TFC::Components::SlidePresenter::InsertPageAt(Evas_Object* page,
		int index) {
	if(this->pageList.size() >= index)
		InsertPage(page);
	else
	{
		auto iter = this->pageList.begin() + index;
		auto before = *iter;
		this->pageList.insert(iter, page);
		elm_box_pack_before(this->box, page, before);
		auto indexItem = elm_index_item_append(this->index, nullptr, nullptr, page);
		evas_object_show(page);

		if(this->pageList.size() == 1)
			elm_index_item_selected_set(indexItem, EINA_TRUE);
	}
}

void TFC::Components::SlidePresenter::Remove(int index) {
	auto iter = this->pageList.begin() + index;
	auto obj = *iter;
	evas_object_del(obj);
	elm_box_recalculate(this->box);
}

void TFC::Components::SlidePresenter::OnIndexSelected(Evas_Object* source, void* event_data)
{
	auto page = elm_object_item_data_get((Elm_Object_Item*)event_data);
	auto it = std::find(this->pageList.begin(), this->pageList.end(), page);
	if(it != this->pageList.end())
	{
		int index = it - this->pageList.begin();
		elm_scroller_page_show(this->scroller, index, 0);
//		eventPageChanged(this, index);
	}
}
