#pragma once
#include "Blackboard.h"
#include "IExamInterface.h"
#include "BehaviorTree.h"
#include "Structs.h"
#include <unordered_set>
#include "Functions.h"
#include <set>
#include <vector>
// ===========================
// ===========================
//	HELPER FUNCTIONS
// ===========================
// ===========================
bool CalculateNearestMissingItem(Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	std::vector<EntityInfo>* items{};
	EntityInfo nearestMissingItem{};
	bool dataAvailable{ pBlackboard->GetData("interface",pInterface) && pBlackboard->GetData("items",items)
				 && pBlackboard->GetData("nearestMissingItem",nearestMissingItem) };
	if (!dataAvailable || items == nullptr)
		return false;

	bool wasNearestItemChanged{};
	float smallestDistance{ FLT_MAX };
	for (const auto& item : *items)
	{
		const float squaredDistance{ Elite::DistanceSquared(item.Location,pInterface->Agent_GetInfo().Position) };
		if (squaredDistance < smallestDistance)
		{
			smallestDistance = squaredDistance;
			nearestMissingItem = item;
			wasNearestItemChanged = pBlackboard->ChangeData("nearestMissingItem", nearestMissingItem);
		}
	}

	return wasNearestItemChanged;
}
void CheckIfItemWasErased(Blackboard* pBlackboard)
{
	std::vector<EntityInfo>* items{};
	std::vector<ItemInfo>* inventory{};
	bool dataAvailable{ pBlackboard->GetData("inventory",inventory) && pBlackboard->GetData("items",items) };
	if (!dataAvailable || items == nullptr)
		return;

	for (unsigned int i{}; i < inventory->size(); ++i)
	{
		if ((*inventory)[0].ItemHash != 0)
		{
			for (auto& item : *items)
			{
				auto it = std::find_if(items->begin(), items->end(), [inventory, &i](const EntityInfo& a)->bool
					{
						if (a.EntityHash == (*inventory)[i].ItemHash)
							return true;
						return false;
					});
				if (it != items->end())
				{
					items->erase(it);
					pBlackboard->ChangeData("items", items);
				}
			}
		}
	}
}
bool IsInHouse(Blackboard* pBlackboard)
{
	std::vector<HouseInfo> housesInFOV{};
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("housesInFOV",housesInFOV) && pBlackboard->GetData("interface",pInterface) };
	if (!dataAvailable)
		return false;

	if (housesInFOV.empty())
		return false;

	for(const auto& house : housesInFOV)
	{
		const Rectf houseHitbox{ Elite::Vector2{house.Center.x - house.Size.x * 0.5f, house.Center.y - house.Size.y * 0.5f},house.Size.x,house.Size.y };
		const Rectf agentHitbox{ pInterface->Agent_GetInfo().Position,pInterface->Agent_GetInfo().AgentSize,pInterface->Agent_GetInfo().AgentSize };
		return Functions::IsOverlapping(agentHitbox, houseHitbox);
	}
	return false;
}
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
	std::vector<ItemInfo>* inventory{};
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("inventory",inventory) && pBlackboard->GetData("interface",pInterface) };

	if (!dataAvailable)
		return false;
#pragma endregion

	for (size_t i{}; i < inventory->size(); ++i)
	{
		if ((*inventory)[i].ItemHash == 0)
			continue;

		if ((*inventory)[i].Type == eItemType::PISTOL)
		{
			if (pInterface->Weapon_GetAmmo((*inventory)[i]) > 0)
			{
				return true;
			}
		}
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
	std::vector<House>* houses{};
	std::vector<HouseInfo> entities{};
	bool dataAvailable{ pBlackboard->GetData("housesInFOV", entities) && pBlackboard->GetData("houses",houses) };
	if (!dataAvailable)
		return false;

	for (const auto& house : entities)
	{
		auto it = std::find_if(houses->begin(), houses->end(), [house](const House& a)->bool
			{
				if (Elite::AreEqual(house.Center.x, a.position.x) && Elite::AreEqual(house.Center.y, a.position.y))
				{
					return true;
				}
				return false;
			});
		if (it != houses->end())
		{
			continue;
		}
		House temp{ house.Center };
		houses->push_back(temp);
		pBlackboard->ChangeData("houses", houses);
	}

	bool haveAllHousesBeenEntered{ true };
	for (const auto& house : *houses)
	{
		if (!house.hasBeenVisited)
		{
			haveAllHousesBeenEntered = false;
			break;
		}
	}

	if (entities.empty() && haveAllHousesBeenEntered)
	{
		return false;
	}

	return true; // If false hasn't been returned by now, there is a house
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
	std::unordered_set<House, HouseHash, HouseEqual>* houses{};
	std::vector<HouseInfo> entities{};
	bool dataAvailable{ pBlackboard->GetData("housesInFOV", entities) && pBlackboard->GetData("housePositions",houses) };
	if (!dataAvailable)
		return false;

	bool wasHouseInserted{};
	for (const auto& house : entities)
	{
		if (houses->insert(house.Center).second)
		{
			wasHouseInserted = pBlackboard->ChangeData("houses", houses);
		}
	}

	return wasHouseInserted;
}
bool ArePickupsInFOV(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities{};
	IExamInterface* pInterface{};
	std::vector<EntityInfo>* items{};
	ItemsInInventory itemsInInventory{};
	bool dataAvailable{ pBlackboard->GetData("entitiesInFOV",entities) && pBlackboard->GetData("interface",pInterface)
	&& pBlackboard->GetData("items",items) && pBlackboard->GetData("itemsInInventory",itemsInInventory) };
	if (!dataAvailable)
		return false;

	for (const auto& entity : entities)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			ItemInfo itemInfo{};
			if (pInterface->Item_GetInfo(entity, itemInfo))
			{
				if (itemInfo.Type == eItemType::GARBAGE)
				{
					continue;
				}

				switch (itemInfo.Type)
				{
				case eItemType::FOOD:
					if (itemsInInventory.nrOfFood == itemsInInventory.maxNrOfFood)
						continue;
					break;
				case eItemType::PISTOL:
					if (itemsInInventory.nrOfPistols == itemsInInventory.maxNrOfPistols)
						continue;
					break;
				case eItemType::MEDKIT:
					if (itemsInInventory.nrOfMedkits == itemsInInventory.maxNrOfMedkits)
						continue;
					break;
				default:
					break;
				}

				auto it = std::find_if(items->begin(), items->end(), [entity](const EntityInfo& a)->bool
					{
						if (entity.EntityHash == a.EntityHash)
						{
							return true;
						}
						return false;
					});
				if (it != items->end())
				{
					continue;
				}
				items->push_back(entity);
				pBlackboard->ChangeData("items", items);
			}
		}
	}

	if (entities.empty() && items->empty())
	{
		return false;
	}
	return true;
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
bool HasCenterOfHouseNotBeenReached(Blackboard* pBlackboard)
{
	std::vector<House>* houses{};
	IExamInterface* pInterface{};
	bool dataAvailable{ pBlackboard->GetData("interface", pInterface) && pBlackboard->GetData("houses",houses) };
	if (!dataAvailable)
		return false;

	for (auto& house : *houses)
	{
		if (house.hasBeenVisited)
		{
			continue;
		}
		const float houseSize{ 5.f };
		const Rectf houseCenterHitbox{ Elite::Vector2{house.position.x, house.position.y},houseSize,houseSize };
		const Rectf agentHitbox{ pInterface->Agent_GetInfo().Position,pInterface->Agent_GetInfo().AgentSize,pInterface->Agent_GetInfo().AgentSize };
		if (Functions::IsOverlapping(agentHitbox, houseCenterHitbox))
		{
			house.hasBeenVisited = true;
			return false;
		}
	}
	return true;
}
bool IsInventoryNotFull(Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	std::vector<ItemInfo>* inventory{};
	bool dataAvailable{ pBlackboard->GetData("inventory", inventory) };
	if (!dataAvailable)
		return false;

	for (const auto& item : *inventory)
	{
		if (item.ItemHash == 0)
			return true;
	}
	return false;
}
bool IsFirstItemInGrabRange(Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	std::vector<EntityInfo>* items{};
	bool dataAvailable{ pBlackboard->GetData("items",items) && pBlackboard->GetData("interface",pInterface) };
	if (!dataAvailable)
		return false;

	if (items->empty())
		return false;

	return (Elite::DistanceSquared((*items)[0].Location, pInterface->Agent_GetInfo().Position)
		<= Elite::Square(pInterface->Agent_GetInfo().GrabRange));
}
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
	bool dataAvailable{ pBlackboard->GetData("agentInfo",agentInfo) && pBlackboard->GetData("interface",pInterface) };

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

	steering.RunMode = true;



	if (Seek(pBlackboard, finalEscapePath))
	{
		steering.RunMode = true;
		return BehaviorState::Success;
	}
	return BehaviorState::Failure;
}
BehaviorState EnterHouse(Blackboard* pBlackboard)
{
	std::vector<HouseInfo> houseInFOV{};
	SteeringPlugin_Output steering{};
	AgentInfo agentInfo{};
	std::vector<House>* houses{};
	bool dataAvailable{ pBlackboard->GetData("housesInFOV",houseInFOV) && pBlackboard->GetData("steeringOutput",steering)
					&& pBlackboard->GetData("agentInfo",agentInfo) && pBlackboard->GetData("houses",houses) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	for (const auto& house : *houses)
	{
		if (house.hasBeenVisited)
		{
			continue;
		}
		if (Seek(pBlackboard, house.position))
		{
			return BehaviorState::Success;
		}
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
BehaviorState GoToFirstItem(Blackboard* pBlackboard)
{
	std::vector<EntityInfo>* items{};
	bool dataAvailable{ pBlackboard->GetData("items",items) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	if (items->empty())
		return BehaviorState::Failure;

	if (Seek(pBlackboard, (*items)[0].Location))
	{
		return BehaviorState::Success;
	}
	return BehaviorState::Failure;
}
BehaviorState GrabFirstItem(Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	std::vector<ItemInfo>* inventory{};
	std::vector<EntityInfo>* items{};
	ItemsInInventory itemsInInventory{};
	bool dataAvailable{ pBlackboard->GetData("interface",pInterface) && pBlackboard->GetData("inventory",inventory)
					&& pBlackboard->GetData("items",items) && pBlackboard->GetData("itemsInInventory",itemsInInventory) };
	if (!dataAvailable)
		return BehaviorState::Failure;

	for (auto& item : *items)
	{
		ItemInfo itemInfo{};
		if (pInterface->Item_Grab(item, itemInfo))
		{
			switch (itemInfo.Type)
			{
			case eItemType::FOOD:
				++itemsInInventory.nrOfFood;
				break;
			case eItemType::MEDKIT:
				++itemsInInventory.nrOfMedkits;
				break;
			case eItemType::PISTOL:
				++itemsInInventory.nrOfPistols;
				break;
			default:
				break;
			}
			pBlackboard->ChangeData("itemsInInventory", itemsInInventory);

			for (unsigned int i{}; i < inventory->size(); ++i)
			{
				if ((*inventory)[i].ItemHash == 0)
				{
					// no item in the slot
					pInterface->Inventory_AddItem(i, itemInfo);
					(*inventory)[i] = itemInfo;
					pBlackboard->ChangeData("inventory", inventory);
					items->erase(items->begin()); // delete the first element
					CheckIfItemWasErased(pBlackboard);
					return BehaviorState::Success;
				}
			}
		}
	}
	return BehaviorState::Failure;
}
