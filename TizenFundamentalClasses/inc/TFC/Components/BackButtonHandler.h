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
 *    Components/BackButtonHandler.h
 *
 * Wrapper to handle back-button click on device.
 *
 * Created on: 	 Apr 27, 2016
 * Author: 		 Gilang Mentari Hamidy (g.hamidy@samsung.com)
 * Contributors: Kevin Winata (k.winata@samsung.com)
 */

#ifndef TFC_COMPONENTS_BACKBUTTONHANDLER_H_
#define TFC_COMPONENTS_BACKBUTTONHANDLER_H_

#include "TFC/Framework/Application.h"

namespace TFC {
namespace Components {

/**
 * Class that can be inherited to override back button default behavior.
 * It provides a way to manipulate UIApplicationBase's back button stack with Acquire() & Release() method.
 * When the handler is acquired, every back button press event will call a custom function BackButtonClicked().
 */
class LIBAPI BackButtonHandler : public virtual EventClass
{
public:
	/**
	 * Destructor of BackButtonHandler.
	 */
	virtual ~BackButtonHandler();
protected:
	/**
	 * Constructor for BackButtonHandler.
	 */
	BackButtonHandler();

	/**
	 * Method to acquire handler of back button.
	 * It calls AcquireExclusiveBackButtonPressed() method from UIApplicationBase which pushes the handler to the stack.
	 */
	void Acquire();

	/**
	 * Method to release handler of back button.
	 * It calls ReleaseExclusiveBackButtonPressed() method from UIApplicationBase which pops the handler to the stack.
	 *
	 * @note It will be automatically called when the back button is pressed, so if you want to continue
	 * 		 handle back button you need to re-acquire it on BackButtonClicked().
	 */
	void Release();

	/**
	 * Abstract method that will be called when the handler is acquired and back button is pressed.
	 * Has to be implemented.
	 *
	 * @return true if the application should be closed after clicking back
	 * 		   false to cancel closing the application
	 */
	virtual bool BackButtonClicked() = 0;
private:
	class Wrapper : public EventClass
	{
		BackButtonHandler* handler;
	public:
		Wrapper(BackButtonHandler* handler);
		void Call();
	};

	friend class Wrapper;
	Wrapper* obj;
	bool acquired;
	bool BackButtonClickedInternal();
};

}
}


#endif /* TFC_COMPONENTS_BACKBUTTONHANDLER_H_ */
