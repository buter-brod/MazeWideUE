#include "MazeField.h"
#include "Kismet/KismetMathLibrary.h"
#include "TileBase.h"
#include "Engine/PointLight.h"
#include <algorithm>
#include <random>

TSubclassOf<class ATileBase> MazeField::TileClassFree;
TSubclassOf<class ATileBase> MazeField::TileClassBlocked;

constexpr float holeChance = 0.1f;
constexpr float blockedChance = 0.3f;

constexpr bool createLongCorridorOnAxes = false;

constexpr float tileSize = 100.f;

constexpr int lightZonesPerBlockSide = 20;
static_assert(blockSideSize % lightZonesPerBlockSide == 0 && (blockSideSize / lightZonesPerBlockSide - 1) % 2 == 0);

constexpr unsigned singleLightRadius = (blockSideSize / lightZonesPerBlockSide - 1) / 2; 
//e.g. for blockside = 100, lightRadius will be 2, it means 25 tiles coverage.

std::mt19937& localRnd() {

	static std::random_device rd;
	static std::mt19937 rnd(rd());
	return rnd;
}

TileInfo::~TileInfo() {

	if (IsValid(tilePtr))
		tilePtr->Destroy();
}

void MazeField::InitSubclasses()
{
	static ConstructorHelpers::FObjectFinder<UBlueprint> blueprint_finder_tile_free(TEXT("Blueprint'/Game/TileEmpty.TileEmpty'"));
	static ConstructorHelpers::FObjectFinder<UBlueprint> blueprint_finder_tile_blocker(TEXT("Blueprint'/Game/TileBlocker.TileBlocker'"));

	TileClassFree = (UClass*) blueprint_finder_tile_free.Object->GeneratedClass;
	TileClassBlocked = (UClass*) blueprint_finder_tile_blocker.Object->GeneratedClass;
}

MazeField::~MazeField()
{
	for (const auto& lightPair : lights){
	
		if (IsValid(lightPair.second))
			lightPair.second->Destroy();
	}
}

void MazeField::Init(UWorld* pWorld) {

	world = pWorld;

	constexpr auto blockSideNegative = -static_cast<int>(blockSideSize);

	GenerateBlock({0, 0});
	GenerateBlock({blockSideNegative, 0});
	GenerateBlock({blockSideNegative, blockSideNegative});
	GenerateBlock({0, blockSideNegative});
}

FVector MazeField::GetTileRealPosition(const Vec2D& where) {

	return {where.x * tileSize, where.y * tileSize, 0};
}

std::vector<Vec2D> getOrthoDirectionsInRandomOrder() {

	std::vector<Vec2D> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
	std::shuffle(directions.begin(), directions.end(), localRnd());

	return directions;
}

std::set<Vec2D> GetBlockTilePositions(const Vec2D blockOffset) {

	std::set<Vec2D> allPositions;

	for (int x = blockOffset.x; x < blockOffset.x + static_cast<int>(blockSideSize); ++x) {
		for (int y = blockOffset.y; y < blockOffset.y + static_cast<int>(blockSideSize); ++y) {

			allPositions.emplace(x, y);
		}
	}

	return allPositions;
}

std::deque<Vec2D> MazeField::GetPathToTile(const Vec2D& startTileLogicalPos, const Vec2D& destination, const std::set<Vec2D>& knownTiles) {

	std::deque<Vec2D> checkQueue;
	checkQueue.push_back(startTileLogicalPos);

	std::map<Vec2D, Vec2D> parentMap;
	std::deque<Vec2D> pathVec;

	while (!checkQueue.empty()) {

		const Vec2D checkPos = checkQueue.front();
		checkQueue.pop_front();

		if (checkPos == destination)
		{
			Vec2D backwardIt = checkPos;
			while (backwardIt != startTileLogicalPos)
			{
				pathVec.push_back(backwardIt);
				backwardIt = parentMap.at(backwardIt);
			}
			std::reverse(pathVec.begin(), pathVec.end());
		}

		else {
			const auto randomDirections = getOrthoDirectionsInRandomOrder();
			for (const auto& direction : randomDirections)
			{
				const auto neighborPos = checkPos + direction;
				if (!knownTiles.empty() && knownTiles.find(neighborPos) == knownTiles.end())
					continue;
					// cannot use this tile due to restrictions

				if (neighborPos == startTileLogicalPos || parentMap.count(neighborPos) > 0)
					continue;

				const auto* neighborInfo = GetTileInfoByLogicalPosition(neighborPos);
				if (!neighborInfo)
					continue;

				if (neighborInfo->tileType != TileType::FREE)
					continue;

				parentMap.emplace(neighborPos, checkPos);
				checkQueue.push_back(neighborPos);
			}
		}		
	}

	return pathVec;
}

bool MazeField::CompareDistances(const Vec2D& origin, const Vec2D& pos1, const Vec2D& pos2) {

	const int pos1DistSquare =
		std::pow(pos1.x - origin.x, 2) +
		std::pow(pos1.y - origin.y, 2);

	const int pos2DistSquare =
		std::pow(pos2.x - origin.x, 2) +
		std::pow(pos2.y - origin.y, 2);

	// sqrt not needed if we need just understand which distance is longer
	return pos1DistSquare < pos2DistSquare;
}

void MazeField::SetHeroPosition(int heroId, const Vec2D& pos) {

	heroesPositions[heroId] = pos;
}

void MazeField::Update() {

	UpdateLights();
}

void MazeField::UpdateLights() {

	for (const auto& [pos, block] : blocksMap) {

		for (int lzStartx = 0; lzStartx < blockSideSize; lzStartx += singleLightRadius * 2 + 1) {
			for (int lzStarty = 0; lzStarty < blockSideSize; lzStarty += singleLightRadius * 2 + 1) {

				const Vec2D lightZoneBegin(lzStartx, lzStarty);
				const Vec2D lightZoneRelativeCenter(lightZoneBegin.x + singleLightRadius, lightZoneBegin.y + singleLightRadius);

				if (lights.count({lightZoneRelativeCenter.x + pos.x, lightZoneRelativeCenter.y + pos.y}) > 0)
					continue;
					// already has light here

				std::vector<Vec2D> lightZonePositions;

				for (int x = 0; x <= singleLightRadius * 2; ++x) {
					for (int y = 0; y <= singleLightRadius * 2; ++y) {
						lightZonePositions.emplace_back(lightZoneBegin.x + x, lightZoneBegin.y + y);
					}
				}

				bool canPutLight = true;

				const TileInfo* anyVisitedOne = nullptr;
				for (const auto& lzCellPos : lightZonePositions)
				{
					const auto& tileInfo = block[lzCellPos.y][lzCellPos.x];
					if (tileInfo.IsVisited() && !anyVisitedOne) {
						anyVisitedOne = &tileInfo;
						break;
					}
				}
				
				if (!anyVisitedOne)
					canPutLight = false;
				
				if (canPutLight)
				{
					const auto& tileInfo = block[lightZoneRelativeCenter.y][lightZoneRelativeCenter.x];
					if (tileInfo.tilePtr)
					{
						const FRotator rotator;
						const FActorSpawnParameters spawnParams;
						FVector spawnLocation = tileInfo.tilePtr->GetActorLocation();
						spawnLocation.Z = 150;

						const auto light = world->SpawnActor<APointLight>(APointLight::StaticClass(), spawnLocation, rotator, spawnParams);
						light->SetRadius(200);
						light->SetLightColor(FLinearColor(1.f, 1.f, 0.f));
						lights[tileInfo.pos] = light;						
					}
				}
			}
		}
	}
}

Vec2D MazeField::GetBlockOffsetForLogicalPosition(const Vec2D& pos) {

	constexpr auto blockSideSizeI = static_cast<int>(blockSideSize);

	int blockStartX = (pos.x / blockSideSizeI) * blockSideSize;
	int blockStartY = (pos.y / blockSideSizeI) * blockSideSize;

	if (pos.x < 0 && pos.x % blockSideSizeI != 0)
		blockStartX -= blockSideSize;

	if (pos.y < 0 && pos.y % blockSideSizeI != 0)
		blockStartY -= blockSideSize;

	const Vec2D blockOffset = {blockStartX, blockStartY};
	return blockOffset;
}

std::pair<Vec2D, TilesBlock&> MazeField::GetOrCreateBlockForLogicalPosition(const Vec2D& pos) {

	const auto blockOffset = GetBlockOffsetForLogicalPosition(pos);
	const auto existingBlockIt = blocksMap.find(blockOffset);
	if (existingBlockIt == blocksMap.end()) {
		GenerateBlock(blockOffset);
	}

	auto& block = blocksMap.at(blockOffset);
	return {blockOffset, block};
}

TileInfo* MazeField::GetTileInfoByLogicalPosition(const Vec2D& pos) {

	GetOrCreateBlockForLogicalPosition({24, -100});

	const auto [blockPos, block] = GetOrCreateBlockForLogicalPosition(pos);
	auto* tileInfo = &block[pos.y - blockPos.y][pos.x - blockPos.x];
	if (tileInfo->pos != pos)
	{
		checkf(false, TEXT("MazeField::GetTileInfoByLogicalPosition fatal error, block internal position doesn't match its matrix position"));
		return nullptr;
	}
	
	return tileInfo;
}

bool MazeField::UpdateActorsAtPositions(const std::vector<Vec2D>& positions) {

	bool updated = false;

	for (const auto& pos : positions) {
		const TileInfo* tileInfo = GetTileInfoByLogicalPosition(pos);
		if (!tileInfo)
			continue;
		if (!tileInfo->tilePtr)
			continue;

		updated |= tileInfo->tilePtr->SetNeedUpdate();
	}

	return updated;
}

bool MazeField::SpawnActorsAtPositions(const std::vector<Vec2D>& positions) {
	
	bool spawned = false;

	for (const auto& pos : positions) {

		TileInfo* tileInfo = GetTileInfoByLogicalPosition(pos);
		if (!tileInfo)
			continue;

		spawned |= EnsureTileActorSpawned(*tileInfo);
	}

	return spawned;
}

std::vector<Vec2D> MazeField::GetTileLogicalPositionsInRadius(const Vec2D& center, const unsigned radius) const
{
	std::vector<Vec2D> tilePositions;

	const int leftTopX = center.x - radius;
	const int leftTopY = center.y - radius;

	const int rightBottomX = center.x + radius;
	const int rightBottomY = center.y + radius;

	const size_t tilesCount = std::pow(radius * 2 + 1, 2);
	tilePositions.reserve(tilesCount);

	for (int x = leftTopX; x <= rightBottomX; ++x) {
		for (int y = leftTopY; y <= rightBottomY; ++y) {

			tilePositions.emplace_back(x, y);
		}
	}

	return tilePositions;
}

TileInfo* MazeField::GetTileInfoByRealPosition(const FVector actorPosition) {

	for (auto& [blockStart, block] : blocksMap) {
	
		const auto blockPosStart = blockStart;

		const auto realOriginX = -tileSize / 2.f + blockPosStart.x * tileSize;
		const auto realOriginY = -tileSize / 2.f + blockPosStart.y * tileSize;

		const auto realEndX = realOriginX + blockSideSize * tileSize;
		const auto realEndY = realOriginY + blockSideSize * tileSize;

		if (actorPosition.X >= realOriginX &&
			actorPosition.Y >= realOriginY &&
			actorPosition.X < realEndX &&
			actorPosition.Y < realEndY) 
		{
			const int xOnBlock = (actorPosition.X - realOriginX) / tileSize;
			const int yOnBlock = (actorPosition.Y - realOriginY) / tileSize;
			auto* cell = &block[yOnBlock][xOnBlock];

			return cell;
		}
	}

	checkf(false, L"MazeField::GetTileInfoByRealPosition error, tile not found");	
	return nullptr;
}

bool MazeField::EnsureTileActorSpawned(TileInfo& tileInfo) {

	if (tileInfo.tilePtr)
		return false;

	if (tileInfo.tileType == TileType::HOLE) 
		return false;

	const FRotator rotator;
	const FActorSpawnParameters spawnParams;
	const FVector spawnLocation = GetTileRealPosition(tileInfo.pos);

	const decltype(TileClassFree) tileClass = tileInfo.tileType == TileType::FREE ? TileClassFree : TileClassBlocked;
	const auto tileActor = world->SpawnActor<ATileBase>(tileClass, spawnLocation, rotator, spawnParams);
	tileInfo.tilePtr = tileActor;

	tileActor->SetTileInfo(&tileInfo);
	return true;
}

TileInfo MazeField::GenerateTileInfo(TileInfo& newTileInfo, const Vec2D& where, TileType what) {

	if (what == TileType::UNDEFINED)
	{
		if constexpr (createLongCorridorOnAxes && (where.x == 0 || where.y == 0)) {
			what = TileType::FREE;
		}
		else {
			const bool isHole = UKismetMathLibrary::RandomBoolWithWeight(holeChance);
			if (isHole) {
				what = TileType::HOLE;
			}
			else {
				const bool blocked = UKismetMathLibrary::RandomBoolWithWeight(blockedChance);
				what = blocked ? TileType::BLOCKED : TileType::FREE;
			}
		}
	}

	newTileInfo.tileType = what;
	newTileInfo.pos = where;
	
	return newTileInfo;
}

void MazeField::GenerateBlock(const Vec2D where) {

	auto& newBlock = blocksMap[where];
	
	for (int y = 0; y < blockSideSize; ++y){
		for (int x = 0; x < blockSideSize; ++x){
			auto& newTileInfo = newBlock[y][x];

			const TileType tileType = (x == 0 && y == 0) ? TileType::FREE : TileType::UNDEFINED;
			newTileInfo = GenerateTileInfo(newTileInfo, {where.x + x, where.y + y}, tileType);
		}
	}
}