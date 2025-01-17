#include "hitStop.h"
static inline bool hitStopping;

inline void SGTM(float a_in) {
	static float* g_SGTM = (float*)REL::ID(511883).address();
	*g_SGTM = a_in;
	using func_t = decltype(SGTM);
	REL::Relocation<func_t> func{ REL::ID(66989) };
	return;
}

/*actual operation in the hitstop.
@param animVariable animation variable related to weapon speed.
@param stopTimeMiliSec how long ni miliseconds is the hitstop.
@param stopSpeed weapon speed during hitstop. Suggested to be lower than 0.2*/
void hitStop::hitStopBehaviorOp(int stopTimeMiliSec, float stopSpeed) {
	auto pc = RE::PlayerCharacter::GetSingleton();
	if (pc) {
		DEBUG("executing stop! by speed {}", stopSpeed);
		pc->SetGraphVariableFloat("SkySA_weaponSpeedMult", stopSpeed);
		pc->SetGraphVariableFloat("SkySA_dwweaponsSpeedMult", stopSpeed);
		std::this_thread::sleep_for(std::chrono::milliseconds(stopTimeMiliSec));
		pc->SetGraphVariableFloat("SkySA_weaponSpeedMult", 1);
		pc->SetGraphVariableFloat("SkySA_dwweaponsSpeedMult", 1);
	}
}

void hitStop::hitStopSGTMOp(int stopTimeMiliSec, float stopSpeed) {
	DEBUG("executing stop! by speed {}", stopSpeed);
	SGTM(stopSpeed);
	std::this_thread::sleep_for(std::chrono::milliseconds(stopTimeMiliSec));
	SGTM(1);
}

void hitStop::hitStopVanillaOp(int stopTimeMiliSec, float stopSpeed) {
	auto pc = RE::PlayerCharacter::GetSingleton();
	if (pc && !hitStopping) {
		DEBUG("executing stop! by speed {}", stopSpeed);
		float orgSpd = pc->GetActorValue(RE::ActorValue::kWeaponSpeedMult);
		float orgSpdL = pc->GetActorValue(RE::ActorValue::kLeftWeaponSpeedMultiply);
		pc->SetActorValue(RE::ActorValue::kWeaponSpeedMult, stopSpeed);
		pc->SetActorValue(RE::ActorValue::kLeftWeaponSpeedMultiply, stopSpeed);
		hitStopping = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(stopTimeMiliSec));
		pc->SetActorValue(RE::ActorValue::kWeaponSpeedMult, orgSpd);
		pc->SetActorValue(RE::ActorValue::kLeftWeaponSpeedMultiply, orgSpdL);
		hitStopping = false;
	}
}

/*asf is a bit special. So we instead of directly setting the speed, subtract
the difference between the current and desired speed. */
void hitStop::hitStopASFOp(int stopTimeMiliSec, float stopSpeed) {
	auto pc = RE::PlayerCharacter::GetSingleton();
	if (pc && !hitStopping) {
		DEBUG("executing stop! by speed {}", stopSpeed);
		float speedDiff = stopSpeed - pc->GetActorValue(RE::ActorValue::kWeaponSpeedMult);
		float speedDiffL = stopSpeed - pc->GetActorValue(RE::ActorValue::kLeftWeaponSpeedMultiply);
		pc->ModActorValue(RE::ActorValue::kWeaponSpeedMult, speedDiff);
		pc->ModActorValue(RE::ActorValue::kLeftWeaponSpeedMultiply, speedDiffL);
		DEBUG("speed right after execution is {}", pc->GetActorValue(RE::ActorValue::kWeaponSpeedMult));
		hitStopping = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(stopTimeMiliSec));
		pc->ModActorValue(RE::ActorValue::kWeaponSpeedMult, -speedDiff);
		pc->ModActorValue(RE::ActorValue::kLeftWeaponSpeedMultiply, -speedDiff);
		hitStopping = false;
	}
}

void hitStop::stopASF(int stopTimeMiliSec, float stopSpeed) {
	DEBUG("stop ASF!");
	std::jthread hitstopThread = std::jthread(hitStopASFOp, stopTimeMiliSec, stopSpeed);
	hitstopThread.detach();
}

void hitStop::stopVanilla(int stopTimeMiliSec, float stopSpeed) {
	DEBUG("stop Vanilla!");
	std::jthread hitstopThread = std::jthread(hitStopVanillaOp, stopTimeMiliSec, stopSpeed);
	hitstopThread.detach();
}

void hitStop::stopMCO(int stopTimeMiliSec, float stopSpeed) {
	DEBUG("stop MCO!");
	std::jthread hitstopThread = std::jthread(hitStopBehaviorOp, stopTimeMiliSec, stopSpeed);
	hitstopThread.detach();
}

void hitStop::stopSGTM(int stopTimeMiliSec, float stopSpeed) {
	DEBUG("stop SGTM!");
	if (RE::PlayerCharacter::GetSingleton()->HasEffectWithArchetype(RE::MagicTarget::Archetype::kSlowTime)) {
		return;
	}
	std::jthread hitstopThread = std::jthread(hitStopSGTMOp, stopTimeMiliSec, stopSpeed);
	hitstopThread.detach();
}


/*call this to start a new hitstop*/
void hitStop::stop(int stopTimeMiliSec, float stopSpeed) {
	DEBUG("finalizing hitstop. stop time: {}, stop speed: {}", stopTimeMiliSec, stopSpeed);
	using method = dataHandler::combatFrameWork;
	switch (dataHandler::GetSingleton()->currFrameWork) {
	case method::MCO: stopMCO(stopTimeMiliSec, stopSpeed); break;
	case method::ASF: stopASF(stopTimeMiliSec, stopSpeed); break;
	case method::Vanilla: stopVanilla(stopTimeMiliSec, stopSpeed); break;
	case method::STGM: stopSGTM(stopTimeMiliSec, stopSpeed); break;
	default: stopVanilla(stopTimeMiliSec, (float) stopSpeed); break;
	}
	
}

