#pragma once

#include "CoreMinimal.h"

#include <array>
#include <map>
#include <deque>
#include <set>

class ATileBase;
class APointLight;

#define TCHARIFYSTDSTRING(X) UTF8_TO_TCHAR(X.c_str())

enum class TileType {
	HOLE,
	FREE,
	BLOCKED,
	UNDEFINED
};

struct Vec2D {

	int x = 0;
	int y = 0;

	Vec2D(){}
	Vec2D(int xx, int yy) : x(xx), y(yy){}

	inline bool operator==(const Vec2D& second) const {
		return x == second.x && y == second.y;
	}

	inline bool operator!=(const Vec2D& second) const {
		return !(*this == second);
	}

	inline bool operator< (const Vec2D& second) const {
		if (y != second.y)
			return y < second.y;

		return x < second.x;
	}
};

inline Vec2D operator+(const Vec2D& first, const Vec2D& second) {

	return {first.x + second.x, first.y + second.y};
}

struct TileInfo {

	Vec2D pos;
	TileType tileType = TileType::UNDEFINED;
	bool visited = false;
	bool fullyAppeared = false;
	ATileBase* tilePtr = nullptr;
	
	bool IsVisited() const {return visited;}

	void SetVisited() {
		if (!visited)
			visited = true;
	}

	~TileInfo();
};

constexpr unsigned blockSideSize = 100;

typedef std::array<std::array<TileInfo, blockSideSize>, blockSideSize> TilesBlock;

class MAZE_API MazeField
{
public:
	~MazeField();

	static void InitSubclasses();
	void Init(UWorld* pWorld);

	bool SpawnActorsAtPositions(const std::vector<Vec2D>& positions);
	bool UpdateActorsAtPositions(const std::vector<Vec2D>& positions);

	void Update();
	void UpdateLights();

	std::vector<Vec2D> GetTileLogicalPositionsInRadius(const Vec2D& center, unsigned radius) const;

	static Vec2D GetBlockOffsetForLogicalPosition(const Vec2D& pos);

	std::pair<Vec2D, TilesBlock&> GetOrCreateBlockForLogicalPosition(const Vec2D& pos);

	TileInfo* GetTileInfoByLogicalPosition(const Vec2D& pos);
	TileInfo* GetTileInfoByRealPosition(const FVector actorPosition);
	static FVector GetTileRealPosition(const Vec2D& where);

	void SetHeroPosition(int heroId, const Vec2D& pos);

	std::deque<Vec2D> GetPathToTile(const Vec2D& startTileLogicalPos, const Vec2D& destination, const std::set<Vec2D>& knownTiles);

	static bool CompareDistances(const Vec2D& origin, const Vec2D& pos1, const Vec2D& pos2);

	static TSubclassOf<class ATileBase> TileClassFree;
	static TSubclassOf<class ATileBase> TileClassBlocked;

protected:

	bool EnsureTileActorSpawned(TileInfo& tileInfo);
	static TileInfo GenerateTileInfo(TileInfo& newTileInfo, const Vec2D& where, TileType what = TileType::UNDEFINED);
	void GenerateBlock(const Vec2D where);

private:

	std::map<int, Vec2D> heroesPositions;

	std::map<Vec2D, TilesBlock> blocksMap;
	UWorld* world = nullptr;

	std::map<Vec2D, APointLight*> lights;
};
