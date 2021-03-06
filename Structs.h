#pragma once
#include "stdafx.h"
#include "Exam_HelperStructs.h"
struct EntityInfoExtended : EntityInfo
{
	EntityInfo entity;
	std::string hash;
};
struct EntityInfoExtendedHash
{
	size_t operator()(const EntityInfoExtended& entity) const
	{
		return std::hash<std::string>()(entity.hash);
	}
};
struct EntityInfoExtendedEqual
{
	bool operator()(const EntityInfoExtended& a, const EntityInfoExtended& b) const
	{
		return a.EntityHash == b.EntityHash;
	}
};
struct Vector2Hash
{
	// https://stackoverflow.com/questions/5928725/hashing-2d-3d-and-nd-vectors
	size_t operator()(const Elite::Vector2& vector) const
	{
		const int largePrimeOne{ 73856093 };
		const int largePrimeTwo{ 19349663 };
		const int hashTableSize{ 199 };
		return (int(vector.x * largePrimeOne) ^ int(vector.y * largePrimeTwo)) % hashTableSize;
	}
};
struct Vector2Equal
{
	bool operator()(const Elite::Vector2& a, const Elite::Vector2& b) const
	{
		return Elite::AreEqual(a.x, b.x) && Elite::AreEqual(a.y, b.y);
	}
};
struct Rectf final
{
	Rectf()
		: Rectf{ {0.f,0.f},0.f,0.f }
	{
	}
	Rectf(const Elite::Vector2& bottomLeft, float width, float height)
		: bottomLeft{ bottomLeft }
		, width{ width }
		, height{ height }
	{
	}

	Elite::Vector2 bottomLeft;
	float width;
	float height;
};
struct Checkpoint
{
	Checkpoint(const Elite::Vector2& position)
		: position{ position }
		, hasBeenReached{}
	{
	}
	bool hasBeenReached{};
	Elite::Vector2 position{};
};
struct Item 
{
	Item()
		: Item{ {},eItemType::GARBAGE }
	{
	}
	Item(const Elite::Vector2& position, const eItemType& type)
		: position{position}
		, type{ type }
		, hasBeenPickedUp{}
	{
	}
	eItemType type{};
	Elite::Vector2 position{};
	bool hasBeenPickedUp{};
};
struct House
{
	House(const Elite::Vector2& position)
		: position{ position }
		, hasBeenVisited{}
	{
	}
	Elite::Vector2 position{};
	bool hasBeenVisited{};
};
struct HouseHash
{
	size_t operator()(const House& a) const
	{
		const int largePrimeOne{ 73856093 };
		const int largePrimeTwo{ 19349663 };
		const int hashTableSize{ 199 };
		return (int(a.position.x * largePrimeOne) ^ int(a.position.y * largePrimeTwo)) % hashTableSize;
	}
};
struct HouseEqual
{
	bool operator()(const House& a, const House& b) const
	{
		return (Elite::AreEqual(a.position.x, b.position.x) && Elite::AreEqual(a.position.y, b.position.y)) && (a.hasBeenVisited == b.hasBeenVisited);
	}
};
struct ItemsInInventory
{
	int nrOfPistols{};
	int nrOfMedkits{};
	int nrOfFood{};

	int maxNrOfPistols{ 2 };
	int maxNrOfMedkits{ 2 };
	int maxNrOfFood{ 1 };
};