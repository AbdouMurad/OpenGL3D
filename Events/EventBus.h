#pragma once

#include <glm/glm.hpp>
#include <memory>

#include <typeinfo>
#include <typeindex>

#include "Events.h"



class EventBus {
private:
	struct Handler {
		const void* instance; //who subscribed, so Unsubscribe can find its handlers
		std::function<void(const void*)> callback;
	};
	//type_index is unique index to an type -> std::typeid(type) (unique id for each event type)
	//std::function<void(const void*) -> reference to a function that returns void and takes in const void*
	//const void* is a ptr to any data type as long as it remains unchanged
	std::unordered_map<std::type_index, std::vector<Handler>> handlers;

public:

	template<typename Event, typename T> //dont need to specify T -> compiler can infer it from the function params
	//T::*handler -> handler is a ptr to a member function of T
	void Subscribe(T* instance, void (T::*handler)(const Event&)) {
		//find list of handlers for this event (previously subscribed)
		handlers[typeid(Event)].push_back({
			instance,
			//lambda -> [...] define what var needed from outside
			[instance, handler](const void* event) {
				//calling handler on the instance
				(instance->*handler)(
					*static_cast<const Event*>(event)
				);
			}
		});
	}

	//removes every handler the instance subscribed, call before it's destroyed
	void Unsubscribe(const void* instance) {
		for (auto& [type, list] : handlers) {
			std::erase_if(list, [instance](const Handler& h) { return h.instance == instance; });
		}
	}

	template<typename Event>
	void Emit(const Event& event) {
		auto it = handlers.find(typeid(event));
		if (it == handlers.end()) return;
		for (auto& handler : it->second) handler.callback(&event);
	}
};
