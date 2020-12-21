#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "Structs.h"
#include <unordered_set>
#include <utility>

class IBaseInterface;
class IExamInterface;
class BehaviorTree;

class Plugin : public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin();

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	bool IsOverlapping(const Rectf& a, const Rectf& b) const;

	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	BehaviorTree* m_pBehaviorTree{ nullptr };
	//std::unordered_set<EntityInfoExtended, EntityInfoExtendedHash, EntityInfoExtendedEqual> m_EnemiesBehindUs{};
	std::vector<Checkpoint> m_Checkpoints{};
	std::unordered_set<Elite::Vector2, Vector2Hash, Vector2Equal> m_HousePositions{};
	EntityInfo m_LocationOfNearestPistol{};
	EntityInfo m_LocationOfNearestMedkit{};
	EntityInfo m_LocationOfNearestFood{};
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}