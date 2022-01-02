#include "C:/Users/Ty/Documents/SKSEPlugins/Build/Plugins/dTryHitStop/CMakeFiles/dTryHitStop.dir/Debug/cmake_pch.hxx"
#include "onHitEventHandler.h"
#include "stopHandler.h"
#include "dataHandler.h"
using EventResult = RE::BSEventNotifyControl;
using HitFlag = RE::TESHitEvent::Flag;
EventResult onHitEventHandler::ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) {
	if (!a_event || !a_eventSource) {
		ERROR("Event Source not found");
		return EventResult::kContinue;
	}

	DEBUG("onhit event triggers!");
	if (a_event->projectile || !a_event->cause || !a_event->target) {
		DEBUG("invalid hit event!");
		return EventResult::kContinue;
	}

	if (a_event->cause->IsPlayerRef()) {
		playerMeleeHit(a_event);
	}

	return EventResult::kContinue;
}

void onHitEventHandler::playerMeleeHit(const RE::TESHitEvent* a_event) {
	DEBUG("player melee hit!");
	auto target = a_event->target.get();
	if (!target) {
		DEBUG("Target Actor Not Found!");
		return;
	}

	if (isObject(target)) {
		if (!dataHandler::stopOnObject) {
			DEBUG("object, no hit stop!");
			return;
		}
		DEBUG("stop on object");
		stopHandler::objectStop();
		return;
	}

	//stop on alive target.
	if (!dataHandler::stopOnCreature) {
		DEBUG("hit creature, no hitstop!");
		return;
	}

	//if not actor, shorcircuits here.
	auto targetActor = target->As<RE::Actor>();
	if (!isAlive(targetActor)) {
		if (!dataHandler::stopOnDead) {
			DEBUG("dead actor, no hit stop!");
			return;
		}
		DEBUG("stop on dead");
		stopHandler::creatureStop();
		return;
	}


	if (a_event->flags.any(RE::TESHitEvent::Flag::kBashAttack)) {
		if (!dataHandler::stopOnBash) {
			DEBUG("bash hit, no hit stop!");
			return;
		}
		DEBUG("stop on bash");
		stopHandler::bashStop();
		return;
	}

	if (a_event->flags.any(RE::TESHitEvent::Flag::kHitBlocked)) {
		if (!dataHandler::stopOnBlocked) {
			DEBUG("hit blocked, no hit stop!");
			return;
		}
		DEBUG("stop on blocked");
		stopHandler::blockedStop();
		return;
	}

	//iff all above are checked, it can only be a living creature.
	stopHandler::creatureStop();
}



bool onHitEventHandler::isObject(RE::TESObjectREFR* a_obj) {
	if (a_obj->formType == RE::FormType::ActorCharacter) {
		DEBUG("Target is actor!");
		return false;
	}
	return true;
}


bool onHitEventHandler::isAlive(RE::Actor* a_target) {				//stolen from Maxsu's OHAF

	if (a_target->IsDead()) {
		DEBUG("Target Actor is Dead!");
		return false;
	}
	if (a_target->IsInKillMove()) {
		DEBUG("Target Actor Is in Kill Move!");
		return false;
	}
	return true;
}
