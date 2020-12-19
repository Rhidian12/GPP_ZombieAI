#pragma once
#include "Blackboard.h"
#include "IExamInterface.h"
#include "BehaviorTree.h"
// ===========================
// ===========================
//	CONDITIONALS
// ===========================
// ===========================
bool AreZombiesInFOV(Blackboard* pBlackboard)
{
#pragma region Blackboard
	std::vector<EntityInfo> entities{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) };

	if (!dataAvailable || entities.empty())
		return false;
#pragma endregion

	for (const auto& entity : entities)
	{
		if (entity.Type == eEntityType::ENEMY)
		{
			return true;
		}
	}
	return false;
}
bool IsGunLoaded(Blackboard* pBlackboard)
{
#pragma region Blackboard
	IExamInterface* pExamInterface{};
	bool dataAvailable{ pBlackboard->GetData("interface",pExamInterface) };

	if (!dataAvailable)
		return false;
#pragma endregion

	auto inventoryCapacity{ pExamInterface->Inventory_GetCapacity() };
	for (unsigned int i{}; i < inventoryCapacity; ++i)
	{
		ItemInfo itemInfo{};
		if (pExamInterface->Inventory_GetItem(i, itemInfo))
			if (itemInfo.Type == eItemType::PISTOL)
				if (pExamInterface->Weapon_GetAmmo(itemInfo) > 0)
					return true;
	}
	return false;
}
bool IsGunNotLoaded(Blackboard* pBlackboard)
{
	return !IsGunLoaded(pBlackboard);
}
// ===========================
// ===========================
//	ACTIONS
// ===========================
// ===========================
// ===========================
// ===========================
//	STEERING BEHAVIOURS
// ===========================
// ===========================
bool Seek(Blackboard* pBlackboard, const Elite::Vector2& target)
{
#pragma region Blackboard
	SteeringPlugin_Output steering{};
	AgentInfo agentInfo{};
	bool dataAvailable{ pBlackboard->GetData("steeringOutput",steering) && pBlackboard->GetData("agentInfo",agentInfo) };

	if (!dataAvailable)
		return false;
#pragma endregion

	steering.LinearVelocity = target - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	return pBlackboard->ChangeData("steeringOutput", steering);
}
BehaviorState Wander(Blackboard* pBlackboard)
{
#pragma region Blackboard
	SteeringPlugin_Output steering{};
	AgentInfo agentInfo{};
	bool dataAvailable{ pBlackboard->GetData("steeringOutput",steering) && pBlackboard->GetData("agentInfo",agentInfo) };

	if (!dataAvailable)
		return BehaviorState::Failure;
#pragma endregion

	const float offset{ 6.f };
	constexpr const float maxAngleChange{ Elite::ToRadians(45.f) }; // max angle change
	const float radius{ 4.f };
	const float angleChange{ maxAngleChange * (Elite::randomFloat(2.f) - 1.f) };
	agentInfo.Orientation += angleChange;

	const Elite::Vector2 circleCentre{ agentInfo.Position + (agentInfo.LinearVelocity.GetNormalized() * offset) };
	const Elite::Vector2 centreToTarget{ radius * cos(agentInfo.Orientation), radius * sin(agentInfo.Orientation) };
	const Elite::Vector2 target{ circleCentre + centreToTarget };

	if (Seek(pBlackboard, target))
		return BehaviorState::Success;
	else
		return BehaviorState::Failure;

}
