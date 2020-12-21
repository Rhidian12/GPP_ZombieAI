#pragma once
#include "Blackboard.h"
#include "IExamInterface.h"
#include "BehaviorTree.h"
#include "Structs.h"
#include <unordered_set>
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
bool HasMoreThanFourStamina(Blackboard* pBlackboard)
{
	AgentInfo agentInfo{};
	bool dataAvailable{ pBlackboard->GetData("agentInfo",agentInfo) };
	if (!dataAvailable)
		return false;

	if (agentInfo.Stamina >= 4.f)
		return true;
	return false;
}
bool IsFarEnoughToFire(Blackboard* pBlackboard)
{
	AgentInfo agentInfo{};
	std::vector<EntityInfo> entities{};
	bool dataAvailable{ pBlackboard->GetData("agentInfo", agentInfo) && pBlackboard->GetData("entitiesInFOV",entities) };
	if (!dataAvailable && entities.empty())
		return false;

	for (const auto& entity : entities)
	{
		if (entity.Type == eEntityType::ENEMY)
		{
			const float squaredDist{ Elite::DistanceSquared(agentInfo.Position, entity.Location) };
			if (squaredDist >= Elite::Square(agentInfo.FOV_Range * 0.2))
				return true;
		}
	}
	return false;
}
bool HaveFiveSecondsPassed(Blackboard* pBlackboard)
{
	float timerForCheckingOurBack{};
	bool dataAvailable{ pBlackboard->GetData("timerForCheckingOurBack",timerForCheckingOurBack) };
	if (!dataAvailable)
		return false;

	if (timerForCheckingOurBack >= 5.f)
	{
		return true;
	}
	return false;
}
bool HasSprintedForThreeSeconds(Blackboard* pBlackboard)
{
	float timeSpentSprinting{};
	bool dataAvailable{ pBlackboard->GetData("timeSpentSprinting",timeSpentSprinting) };
	if (!dataAvailable)
		return false;

	if (timeSpentSprinting >= 3.f)
		return true;
	return false;
}
bool IsHouseInFOV(Blackboard* pBlackboard)
{
	std::vector<HouseInfo> entities{};
	bool dataAvailable{ pBlackboard->GetData("housesInFOV", entities) };
	if (!dataAvailable)
		return false;

	if (entities.empty())
		return false;
	return true;
}
bool HaveNotAllCheckpointsBeenReached(Blackboard* pBlackboard)
{
	std::vector<Checkpoint>* checkpoints{};
	bool dataAvailable{ pBlackboard->GetData("checkpoints",checkpoints) };
	if (!dataAvailable)
		return false;

	for (const auto& checkpoint : *checkpoints)
		if (!checkpoint.hasBeenReached)
			return true;
	return false;
}
bool HasHouseNotBeenEntered(Blackboard* pBlackboard)
{
	std::unordered_set<Elite::Vector2, Vector2Hash, Vector2Equal>* housePositions{};
	std::vector<HouseInfo> entities{};
	bool dataAvailable{ pBlackboard->GetData("housesInFOV", entities) && pBlackboard->GetData("housePositions",housePositions) };
	if (!dataAvailable)
		return false;

	bool wasHouseInserted{};
	for (const auto& house : entities)
		if (housePositions->insert(house.Center).second)
			wasHouseInserted = pBlackboard->ChangeData("housePositions", housePositions);

	return wasHouseInserted;
}
bool ArePickupsInFOV(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) };
	if (!dataAvailable || entities.empty())
		return false;

	for (const auto& entity : entities)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			return true;
		}
	}
	return false;
}
bool IsHealthAboveHalf(Blackboard* pBlackboard)
{
	AgentInfo agentInfo{};
	bool dataAvailable{ pBlackboard->GetData("agentInfo",agentInfo) };
	if (!dataAvailable)
		return false;

	const float maxHealth{ 10.f };
	if (agentInfo.Health >= maxHealth * 0.5f)
		return true;
	return false;
}
bool IsHealthBelowHalf(Blackboard* pBlackboard)
{
	return !IsHealthAboveHalf(pBlackboard);
}
bool IsPistolInFOV(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestPistol{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface)
				&& pBlackboard->GetData("locationOfNearestPistol",locationOfNearestPistol) };
	if (!dataAvailable || entities.empty())
		return false;

	float smallestDistance{ FLT_MAX };
	bool wasPistolFound{};
	for (const auto& entity : entities)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			ItemInfo itemInfo{};
			if (pInterface->Item_GetInfo(entity, itemInfo))
			{
				if (itemInfo.Type == eItemType::PISTOL)
				{
					const float squaredDistance{ Elite::DistanceSquared(pInterface->Agent_GetInfo().Position,entity.Location) };
					if (squaredDistance < smallestDistance)
					{
						locationOfNearestPistol->Location = entity.Location;
						smallestDistance = squaredDistance;
						wasPistolFound = true;
					}
				}
			}
		}
	}

	if (wasPistolFound)
		return pBlackboard->ChangeData("locationOfNearestPistol", locationOfNearestPistol);
	return false;
}
bool IsPistolInGrabRange(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestPistol{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface) 
					&& pBlackboard->GetData("locationOfNearestPistol",locationOfNearestPistol) };
	if (!dataAvailable || entities.empty())
		return false;

	if (Elite::DistanceSquared(locationOfNearestPistol->Location, pInterface->Agent_GetInfo().Position) <= Elite::Square(pInterface->Agent_GetInfo().GrabRange))
		return true;
	return false;
}
bool IsMedkitInFOV(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestMedkit{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface)
				&& pBlackboard->GetData("locationOfNearestMedkit",locationOfNearestMedkit) };
	if (!dataAvailable || entities.empty())
		return false;

	float smallestDistance{ FLT_MAX };
	bool wasMedkitFound{};
	for (const auto& entity : entities)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			ItemInfo itemInfo{};
			if (pInterface->Item_GetInfo(entity, itemInfo))
			{
				if (itemInfo.Type == eItemType::PISTOL)
				{
					const float squaredDistance{ Elite::DistanceSquared(pInterface->Agent_GetInfo().Position,entity.Location) };
					if (squaredDistance < smallestDistance)
					{
						locationOfNearestMedkit->Location = entity.Location;
						smallestDistance = squaredDistance;
						wasMedkitFound = true;
					}
				}
			}
		}
	}

	if (wasMedkitFound)
		return pBlackboard->ChangeData("locationOfNearestMedkit", locationOfNearestMedkit);
	return false;
}
bool IsMedkitInGrabRange(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestMedkit{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface)
					&& pBlackboard->GetData("locationOfNearestMedkit",locationOfNearestMedkit) };
	if (!dataAvailable || entities.empty())
		return false;

	if (Elite::DistanceSquared(locationOfNearestMedkit->Location, pInterface->Agent_GetInfo().Position) <= Elite::Square(pInterface->Agent_GetInfo().GrabRange))
		return true;
	return false;
}
bool IsFoodInFOV(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestFood{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface)
				&& pBlackboard->GetData("locationOfNearestFood",locationOfNearestFood) };
	if (!dataAvailable || entities.empty())
		return false;

	float smallestDistance{ FLT_MAX };
	bool wasFoodFound{};
	for (const auto& entity : entities)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			ItemInfo itemInfo{};
			if (pInterface->Item_GetInfo(entity, itemInfo))
			{
				if (itemInfo.Type == eItemType::PISTOL)
				{
					const float squaredDistance{ Elite::DistanceSquared(pInterface->Agent_GetInfo().Position,entity.Location) };
					if (squaredDistance < smallestDistance)
					{
						locationOfNearestFood->Location = entity.Location;
						smallestDistance = squaredDistance;
						wasFoodFound = true;
					}
				}
			}
		}
	}

	if (wasFoodFound)
		return pBlackboard->ChangeData("locationOfNearestFood", locationOfNearestFood);
	return false;
}
bool IsFoodInGrabRange(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestFood{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface)
					&& pBlackboard->GetData("locationOfNearestFood",locationOfNearestFood) };
	if (!dataAvailable || entities.empty())
		return false;

	if (Elite::DistanceSquared(locationOfNearestFood->Location, pInterface->Agent_GetInfo().Position) <= Elite::Square(pInterface->Agent_GetInfo().GrabRange))
		return true;
	return false;
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
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("steeringOutput",steering) && pBlackboard->GetData("agentInfo",agentInfo)
					&& pBlackboard->GetData("interface",pInterface) };

	if (!dataAvailable)
		return false;
#pragma endregion

	steering.LinearVelocity = pInterface->NavMesh_GetClosestPathPoint(target) - agentInfo.Position;
	pInterface->Draw_Point(target, 30.f, { 0.f,0.f,1.f });
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	return pBlackboard->ChangeData("steeringOutput", steering);
}
void Seek(Blackboard* pBlackboard, const Elite::Vector2& target, Elite::Vector2& velocity)
{
#pragma region Blackboard
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("agentInfo",agentInfo) && pBlackboard->GetData("interface",pInterface) };

	if (!dataAvailable)
		return;
#pragma endregion

	velocity = pInterface->NavMesh_GetClosestPathPoint(target) - agentInfo.Position;
	velocity.Normalize();
	velocity *= agentInfo.MaxLinearSpeed;
}
bool Evade(Blackboard* pBlackboard, const EnemyInfo& target)
{
	SteeringPlugin_Output steering{};
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("steeringOutput",steering) && pBlackboard->GetData("agentInfo",agentInfo)
					&& pBlackboard->GetData("interface",pInterface) };

	if (!dataAvailable)
		return false;

	const float distanceSquared{ Elite::DistanceSquared(target.Location,agentInfo.Position) };
	const int timeToLookIntoTheFuture{ int(distanceSquared / Elite::Square(agentInfo.MaxLinearSpeed)) };
	const Elite::Vector2 futurePosition{ target.Location + target.LinearVelocity * float(timeToLookIntoTheFuture) };

	Elite::Vector2 path{};
	Seek(pBlackboard, futurePosition, path);
	path = -path;
	steering.LinearVelocity = path;

	if (pBlackboard->ChangeData("steeringOutput", steering))
		return true;
	return false;
}
void Evade(Blackboard* pBlackboard, const EnemyInfo& target, Elite::Vector2& path)
{
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("agentInfo",agentInfo)	&& pBlackboard->GetData("interface",pInterface) };

	if (!dataAvailable)
		return;

	const float distanceSquared{ Elite::DistanceSquared(target.Location,agentInfo.Position) };
	const int timeToLookIntoTheFuture{ int(distanceSquared / Elite::Square(agentInfo.MaxLinearSpeed)) };
	const Elite::Vector2 futurePosition{ target.Location + target.LinearVelocity * float(timeToLookIntoTheFuture) };

	Seek(pBlackboard, futurePosition, path);
	path = -path;
}
BehaviorState Wander(Blackboard* pBlackboard)
{
#pragma region Blackboard
	SteeringPlugin_Output steering{};
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("steeringOutput",steering) && pBlackboard->GetData("agentInfo",agentInfo)
					&& pBlackboard->GetData("interface",pInterface) };

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

	pInterface->Draw_Circle(circleCentre, radius, { 1.f,0.f,0.f });
	pInterface->Draw_Point(target, 3.f, { 0.f,1.f,0.f });

	if (Seek(pBlackboard, target))
		return BehaviorState::Success;
	else
		return BehaviorState::Failure;
}
BehaviorState Sprint(Blackboard* pBlackboard)
{
	AgentInfo agentInfo{};
	float timeSpentSprinting{};
	SteeringPlugin_Output steering{};
	bool dataAvailable{ pBlackboard->GetData("agentInfo",agentInfo) && pBlackboard->GetData("timeSpentSprinting",timeSpentSprinting)
					&& pBlackboard->GetData("steeringOutput",steering) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	if (timeSpentSprinting >= 2.f)
	{
		steering.RunMode = false;
		if (pBlackboard->ChangeData("steeringOutput", steering))
			return BehaviorState::Success;
	}

	steering.RunMode = true;
	if (pBlackboard->ChangeData("steeringOutput", steering))
	{
		//std::cout << "SPRINTING" << std::endl;
		return BehaviorState::Success;
	}

	return BehaviorState::Failure;
}
void AddZombiesToWatchOutForBehindUs(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	std::unordered_set<EntityInfoExtended, EntityInfoExtendedHash, EntityInfoExtendedEqual> enemiesBehindUs{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("enemiesBehindUs", enemiesBehindUs) };
	if (!dataAvailable)
		return;

	for (const auto& entity : entities)
	{
		if (entity.Type == eEntityType::ENEMY)
		{
			EntityInfoExtended temp{};
			temp.entity = entity;
			enemiesBehindUs.insert(temp);
		}
	}

	pBlackboard->ChangeData("enemiesBehindUs", enemiesBehindUs);
}
BehaviorState TurnAroundToCheckForZombies(Blackboard* pBlackboard)
{
	AgentInfo agentInfo{};
	float degreesTurned{};
	float deltaTime{};
	std::unordered_set<EntityInfoExtended, EntityInfoExtendedHash, EntityInfoExtendedEqual> enemiesBehindUs{};
	bool dataAvailable{ pBlackboard->GetData("agentInfo",agentInfo) && pBlackboard->GetData("degreesTurned",degreesTurned)
					&& pBlackboard->GetData("enemiesBehindUs",enemiesBehindUs) && pBlackboard->GetData("deltaTime",deltaTime) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	if (Elite::AreEqual(degreesTurned, 0.f))
		enemiesBehindUs.clear(); // Reset Enemies behind us, no use in running from ghosts

	AddZombiesToWatchOutForBehindUs(pBlackboard);

	SteeringPlugin_Output steering{};
	if (degreesTurned <= 27.f)
	{
		std::cout << degreesTurned << std::endl;

		steering.AutoOrient = false;
		steering.RunMode = false;
		steering.AngularVelocity = agentInfo.MaxAngularSpeed;
		degreesTurned += agentInfo.MaxAngularSpeed * deltaTime;

		//std::cout << "TURNING" << std::endl;

		if (pBlackboard->ChangeData("steeringOutput", steering))
			if (pBlackboard->ChangeData("degreesTurned", degreesTurned))
				return BehaviorState::Success;
		return BehaviorState::Failure;
	}
	else
	{
		steering.AngularVelocity = 0.f;
		steering.AutoOrient = true;
		if (pBlackboard->ChangeData("timeSpentSprinting", 0.f))
			if (pBlackboard->ChangeData("timerForCheckingOurBack", 0.f))
				if (pBlackboard->ChangeData("degreesTurned", 0.f))
					if (pBlackboard->ChangeData("steeringOutput", steering))
						return BehaviorState::Success;
		return BehaviorState::Failure;
	}
}
BehaviorState RunAway(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	SteeringPlugin_Output steering{};
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface)
					&& pBlackboard->GetData("steeringOutput",steering) && pBlackboard->GetData("agentInfo",agentInfo) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	Elite::Vector2 finalEscapePath{};
	for (const auto& entity : entities)
	{
		if (entity.Type == eEntityType::ENEMY)
		{
			EnemyInfo enemyInfo{};
			if (pInterface->Enemy_GetInfo(entity, enemyInfo))
			{
				Elite::Vector2 path{};
				Evade(pBlackboard, enemyInfo, path);
				finalEscapePath += path;
			}
		}
	}

	if (Seek(pBlackboard, finalEscapePath))
	{
		steering.RunMode = true;
		return BehaviorState::Success;
	}
	return BehaviorState::Failure;
}
BehaviorState EnterHouse(Blackboard* pBlackboard)
{
	std::vector<HouseInfo>* houseInFOV{};
	SteeringPlugin_Output steering{};
	AgentInfo agentInfo{};
	bool dataAvailable{ pBlackboard->GetData("housesInFOV",houseInFOV) && pBlackboard->GetData("steeringOutput",steering)
					&& pBlackboard->GetData("agentInfo",agentInfo) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	if (houseInFOV->size() == 1)
	{
		if (Seek(pBlackboard, (*houseInFOV)[0].Center))
		{
			return BehaviorState::Success;
		}
	}
	else
	{
		float smallestDistance{ FLT_MAX };
		Elite::Vector2 closestHouse{};
		for (const auto& house : *houseInFOV)
		{
			const float distanceSquared{ Elite::DistanceSquared(agentInfo.Position,house.Center) };
			if (distanceSquared <= smallestDistance)
			{
				smallestDistance = distanceSquared;
				closestHouse = house.Center;
			}
		}
		if (Seek(pBlackboard, closestHouse))
			return BehaviorState::Success;
	}
	return BehaviorState::Failure;
}
BehaviorState GoToUnvisitedCheckpoint(Blackboard* pBlackboard)
{
	std::vector<Checkpoint>* checkpoints{};
	bool dataAvailable{ pBlackboard->GetData("checkpoints",checkpoints) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	for (const auto& checkpoint : *checkpoints)
	{
		if (!checkpoint.hasBeenReached)
		{
			if (Seek(pBlackboard, checkpoint.position))
				return BehaviorState::Success;
		}
	}
	return BehaviorState::Failure;
}
BehaviorState GoToPistol(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestPistol{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface)
					&& pBlackboard->GetData("locationOfNearestPistol",locationOfNearestPistol) };
	if (!dataAvailable || entities.empty())
		return BehaviorState::Failure;

	if (Seek(pBlackboard,locationOfNearestPistol->Location))
		return BehaviorState::Success;
	return BehaviorState::Failure;
}
BehaviorState GoToMedkit(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestMedkit{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface)
					&& pBlackboard->GetData("locationOfNearestMedkit",locationOfNearestMedkit) };
	if (!dataAvailable || entities.empty())
		return BehaviorState::Failure;

	if (Seek(pBlackboard, locationOfNearestMedkit->Location))
		return BehaviorState::Success;
	return BehaviorState::Failure;
}
BehaviorState GoToFood(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestFood{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface)
					&& pBlackboard->GetData("locationOfNearestFood",locationOfNearestFood) };
	if (!dataAvailable || entities.empty())
		return BehaviorState::Failure;

	if (Seek(pBlackboard, locationOfNearestFood->Location))
		return BehaviorState::Success;
	return BehaviorState::Failure;
}
BehaviorState GrabPistol(Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestPistol{};
	bool dataAvailable{ pBlackboard->GetData("interface",pInterface) && pBlackboard->GetData("locationOfNearestPistol",locationOfNearestPistol ) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	ItemInfo itemInfo{};
	if (pInterface->Item_Grab(*locationOfNearestPistol, itemInfo))
		return BehaviorState::Success;
	return BehaviorState::Failure;
}
BehaviorState GrabMedkit(Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestMedkit{};
	bool dataAvailable{ pBlackboard->GetData("interface",pInterface) && pBlackboard->GetData("locationOfNearestMedkit",locationOfNearestMedkit) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	ItemInfo itemInfo{};
	if (pInterface->Item_Grab(*locationOfNearestMedkit, itemInfo))
		return BehaviorState::Success;
	return BehaviorState::Failure;
}
BehaviorState GrabFood(Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	EntityInfo* locationOfNearestFood{};
	bool dataAvailable{ pBlackboard->GetData("interface",pInterface) && pBlackboard->GetData("locationOfNearestFood",locationOfNearestFood) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	ItemInfo itemInfo{};
	if (pInterface->Item_Grab(*locationOfNearestFood, itemInfo))
		return BehaviorState::Success;
	return BehaviorState::Failure;
}