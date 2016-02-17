/*
 * Core.h
 *
 *  Created on: Feb 12, 2016
 *      Author: gilang
 */

#ifndef CORE_H_
#define CORE_H_

#define LOG_TAG "SRINFW"
#include <dlog.h>

#ifdef LIBBUILD
#define LIBAPI __attribute__((__visibility__("default")))
#else
#define LIBAPI
#endif

typedef const char* CString;

template<class DefiningClass, class ValueType, ValueType (DefiningClass::* GetFunc)(), void (DefiningClass::* SetFunc)(const ValueType&)>
class Property {
private:
	DefiningClass* instance;
public:
	Property(DefiningClass* inst) : instance(inst) { }
	operator ValueType() { return (instance->*GetFunc)(); }
	void operator=(const ValueType& val) { (instance->*SetFunc)(val); }
	ValueType &operator->() const { return instance->*GetFunc(); }
};

template<class DefiningClass, class ValueType, ValueType DefiningClass::* LocalVar>
class SimpleProperty {
private:
	DefiningClass* instance;
public:
	SimpleProperty(DefiningClass* inst) : instance(inst) { }
	operator ValueType() { return instance->*LocalVar; }
	void operator=(const ValueType& val) { instance->*LocalVar = (val); }
};

template<class DefiningClass, class ValueType>
class ReadOnlyProperty {
private:
	DefiningClass* instance;
	ValueType (DefiningClass::* GetFunc)();
public:
	ReadOnlyProperty(DefiningClass* inst, ValueType (DefiningClass::* getFunc)()) : instance(inst), GetFunc(getFunc) { }
	operator ValueType() { return instance->*GetFunc(); }
};


class Event;

class EventClass {};
typedef void (EventClass::*EventHandler)(Event* eventSource, void* objSource, void* eventData);

class Event {
public:
	EventClass* instance;
	EventHandler eventHandler;
	EventClass* eventSource;
	CString eventLabel;

	Event();
	Event(EventClass* eventSource);
	Event(EventClass* instance, EventHandler eventHandler, CString eventLabel = nullptr);

	void operator+=(const Event& other);
	void Invoke(void* eventInfo);
	void operator()(void* objSource, void* eventData);
};

#define AddEventHandler(EVENT_METHOD) ::Event(this, (::EventHandler) & EVENT_METHOD)


#endif /* CORE_H_ */