#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "Blackboard.h"
#include "BehaviorTree.h"
#include "Behaviours.h"
#include <unordered_set>

Plugin::~Plugin()
{
	SAFE_DELETE(m_pBehaviorTree);
}
//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Version00";
	info.Student_FirstName = "Rhidian";
	info.Student_LastName = "De Wit";
	info.Student_Class = "2DAE01";

	// == Create Blackboard ==
	Blackboard* pBlackboard{ new Blackboard{} };

	// == Add Data To Blackboard ==
	pBlackboard->AddData("agentInfo", m_pInterface->Agent_GetInfo());
	pBlackboard->AddData("entitiesInFOV", GetEntitiesInFOV());
	pBlackboard->AddData("housesInFOV", GetHousesInFOV());
	SteeringPlugin_Output steeringOutput{};
	pBlackboard->AddData("steeringOutput", steeringOutput);
	pBlackboard->AddData("interface", m_pInterface);
	float timerForCheckingOurBack{};
	pBlackboard->AddData("timerForCheckingOurBack", timerForCheckingOurBack);
	float timeSpentSprinting{};
	pBlackboard->AddData("timeSpentSprinting", timeSpentSprinting);
	float degreesTurned{};
	pBlackboard->AddData("degreesTurned", degreesTurned);
	std::unordered_set<EntityInfoExtended, EntityInfoExtendedHash, EntityInfoExtendedEqual> enemiesBehindUs{};
	pBlackboard->AddData("enemiesBehindUs", enemiesBehindUs);
	float deltaTime{};
	pBlackboard->AddData("deltaTime", deltaTime);
	float timerForWander{ 1.f };
	pBlackboard->AddData("timerForWander", timerForWander);
	std::vector<std::pair<bool, Elite::Vector2>> checkpoints{};
	auto worldInfo{ m_pInterface->World_GetInfo() };
	const Elite::Vector2 leftBot{ worldInfo.Center.x - worldInfo.Dimensions.x * 0.5f,worldInfo.Center.y - worldInfo.Dimensions.y * 0.5f };
	const int checkPointsXDirection{ int(worldInfo.Dimensions.x / m_pInterface->Agent_GetInfo().FOV_Range) };
	const float FOVRange{ m_pInterface->Agent_GetInfo().FOV_Range };
	for (int x{ 1 }; x < checkPointsXDirection; ++x)
	{
		for (int y{}; y < 2; ++y)
		{
			if (y == 0)
			{
				const Elite::Vector2 checkpoint{ leftBot.x + (FOVRange * x),leftBot.y + FOVRange };
				checkpoints.push_back(std::make_pair(false,checkpoint));
			}
			else
			{
				const Elite::Vector2 checkpoint{ leftBot.x + (FOVRange * x),worldInfo.Dimensions.y * 0.5f - FOVRange };
				checkpoints.push_back(std::make_pair(false, checkpoint));
			}
		}
	}
	pBlackboard->AddData("checkpoints", checkpoints);

	m_pBehaviorTree = new BehaviorTree
	{ pBlackboard,
		new BehaviorSelector{
			{
				new BehaviorSequence{ // == DANGER ACTIONS ==
					{
						new BehaviorConditional{AreZombiesInFOV},
						new BehaviorSelector{
							{
								new BehaviorSequence{
									{
										new BehaviorConditional{IsGunLoaded},
										new BehaviorConditional{IsFarEnoughToFire},
										//new BehaviorAction{FireGunAtZombie}
									}},
								new BehaviorSequence{
									{
										new BehaviorConditional{IsGunNotLoaded},
										new BehaviorAction{RunAway}
									}}
							}}
					}},
				new BehaviorSequence{ // == LOOK FOR HOUSE ==
					{
						new BehaviorConditional{IsHouseInFOV},
						new BehaviorAction{EnterHouse}
					}},
				new BehaviorSequence{ // == SEARCH PATTERN ==
					{
						new BehaviorConditional{HaveNotAllCheckpointsBeenReached},
					}},
					//new BehaviorSequence{ // == FIND PICKUPS ==
					//	{
					//		new BehaviorConditional{ArePickupsNearby},
					//		new BehaviorSelector{
					//			{
					//				new BehaviorSequence{
					//					{
					//						new BehaviorConditional{IsHealthAboveHalf},
					//						new BehaviorSelector{
					//							{
					//								new BehaviorSequence{
					//									{
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsAmmoNearby},
					//												new BehaviorAction{GoToAmmoPickup}
					//											}},
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsAmmoInGrabRange},
					//												new BehaviorAction{GrabAmmo}
					//											}}
					//									}},
					//								new BehaviorSequence{
					//									{
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsHealthNearby},
					//												new BehaviorAction{GoToHealthPickup}
					//											}},
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsHealthInGrabRange},
					//												new BehaviorAction{GrabHealth}
					//											}}
					//									}},
					//								new BehaviorSequence{
					//									{
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsFoodNeaby},
					//												new BehaviorAction{GoToFoodPickup}
					//											}},
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsFoodInGrabRange},
					//												new BehaviorAction{GrabFood}
					//											}}
					//									}}
					//							}},

					//					}},
					//				new BehaviorSequence{
					//					{
					//						new BehaviorConditional{IsHealthBelowHalf},
					//						new BehaviorSelector{
					//							{
					//								new BehaviorSequence{
					//									{
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsAmmoNearby},
					//												new BehaviorAction{GoToAmmoPickup}
					//											}},
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsAmmoInGrabRange},
					//												new BehaviorAction{GrabAmmo}
					//											}}
					//									}},
					//								new BehaviorSequence{
					//									{
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsHealthNearby},
					//												new BehaviorAction{GoToHealthPickup}
					//											}},
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsHealthInGrabRange},
					//												new BehaviorAction{GrabHealth}
					//											}}
					//									}},
					//								new BehaviorSequence{
					//									{
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsFoodNeaby},
					//												new BehaviorAction{GoToFoodPickup}
					//											}},
					//										new BehaviorSequence{
					//											{
					//												new BehaviorConditional{IsFoodInGrabRange},
					//												new BehaviorAction{GrabFood}
					//											}}
					//									}}
					//							}},

					//					}}
					//			}}
					//	}},
							//new BehaviorSequence{ == PROJECT FOR LATER ==
							//{
							//		new BehaviorConditional{HaveFiveSecondsPassed},
							//		new BehaviorAction{Sprint},
							//		new BehaviorConditional{HasSprintedForThreeSeconds},
							//		new BehaviorAction{TurnAroundToCheckForZombies},
							//}},
							new BehaviorAction{Wander} // == WANDER AROUND ==
				}}
	};
}
//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
#pragma region DEMO
	//auto steering = SteeringPlugin_Output();

	////Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	//auto agentInfo = m_pInterface->Agent_GetInfo();

	//auto nextTargetPos = m_Target; //To start you can use the mouse position as guidance

	//auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	//auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	//for (auto& e : vEntitiesInFOV)
	//{
	//	if (e.Type == eEntityType::PURGEZONE)
	//	{
	//		PurgeZoneInfo zoneInfo;
	//		m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
	//		std::cout << "Purge Zone in FOV:" << e.Location.x << ", "<< e.Location.y <<  " ---EntityHash: " << e.EntityHash << "---Radius: "<< zoneInfo.Radius << std::endl;
	//	}
	//}
	//
	////INVENTORY USAGE DEMO
	////********************

	//if (m_GrabItem)
	//{
	//	ItemInfo item;
	//	//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
	//	//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
	//	//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
	//	//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
	//	if (m_pInterface->Item_Grab({}, item))
	//	{
	//		//Once grabbed, you can add it to a specific inventory slot
	//		//Slot must be empty
	//		m_pInterface->Inventory_AddItem(0, item);
	//	}
	//}

	//if (m_UseItem)
	//{
	//	//Use an item (make sure there is an item at the given inventory slot)
	//	m_pInterface->Inventory_UseItem(0);
	//}

	//if (m_RemoveItem)
	//{
	//	//Remove an item from a inventory slot
	//	m_pInterface->Inventory_RemoveItem(0);
	//}

	////Simple Seek Behaviour (towards Target)
	//steering.LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	//steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	//steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed

	//if (Distance(nextTargetPos, agentInfo.Position) < 2.f)
	//{
	//	steering.LinearVelocity = Elite::ZeroVector2;
	//}

	////steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	//steering.AutoOrient = true; //Setting AutoOrientate to TRue overrides the AngularVelocity

	//steering.RunMode = m_CanRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)

	//							 //SteeringPlugin_Output is works the exact same way a SteeringBehaviour output

	//							 //@End (Demo Purposes)
	//m_GrabItem = false; //Reset State
	//m_UseItem = false;
	//m_RemoveItem = false;

	//return steering;

#pragma endregion
#pragma region UpdateBlackboard
	m_pBehaviorTree->GetBlackboard()->ChangeData("agentInfo", m_pInterface->Agent_GetInfo());
	m_pBehaviorTree->GetBlackboard()->ChangeData("entitiesInFOV", GetEntitiesInFOV());
	m_pBehaviorTree->GetBlackboard()->ChangeData("housesInFOV", GetHousesInFOV());
	m_pBehaviorTree->GetBlackboard()->ChangeData("deltaTime", dt);
	float timerForCheckingOurBack{};
	if (!HaveFiveSecondsPassed(m_pBehaviorTree->GetBlackboard()))
	{
		m_pBehaviorTree->GetBlackboard()->GetData("timerForCheckingOurBack", timerForCheckingOurBack);
		m_pBehaviorTree->GetBlackboard()->ChangeData("timerForCheckingOurBack", (timerForCheckingOurBack + dt));
	}
	else
	{
		float timeSpentSprinting{};
		m_pBehaviorTree->GetBlackboard()->GetData("timeSpentSprinting", timeSpentSprinting);
		m_pBehaviorTree->GetBlackboard()->ChangeData("timeSpentSprinting", (timeSpentSprinting + dt));
	}
	float timerForWander{};
	m_pBehaviorTree->GetBlackboard()->GetData("timerForWander", timerForWander);
	if (timerForWander <= 1.f)
	{
		m_pBehaviorTree->GetBlackboard()->ChangeData("timerForWander", (timerForWander + dt));
	}

#pragma endregion

	m_pBehaviorTree->Update(dt);
	SteeringPlugin_Output steering{};
	m_pBehaviorTree->GetBlackboard()->GetData("steeringOutput", steering);

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });

	std::vector<std::pair<bool, Elite::Vector2>> checkpoints{};
	m_pBehaviorTree->GetBlackboard()->GetData("checkpoints", checkpoints);
	for (const auto& point : checkpoints)
		m_pInterface->Draw_Point(point.second, 5.f, { 1.f,0.f,0.f });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}
