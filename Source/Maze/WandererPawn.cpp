#include "WandererPawn.h"
#include "TileBase.h"
#include <random>
#include <algorithm>

constexpr float heroSpeed = 2000;

Vec2D AWandererPawn::goalUndefined = {std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};

// Sets default values
AWandererPawn::AWandererPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWandererPawn::BeginPlay()
{
	Super::BeginPlay();

	currentMoveQueue.emplace_back(0, 0);
}

void AWandererPawn::Move(const float dt) {

	if (currentMoveQueue.empty())
		return;

	const auto& goalTilePos = currentMoveQueue.front();

	const auto& targetRealPos = mazeField->GetTileRealPosition(goalTilePos);
	const auto& heroPos = GetActorLocation();

	auto distanceVec = (targetRealPos - heroPos);
	distanceVec.Z = 0.f;
	const auto distLen = distanceVec.Length();
	
	auto directionVec = distanceVec;	
	directionVec.Normalize();

	const float dDist = heroSpeed * dt;
	auto newPos = targetRealPos;
	newPos.Z = heroPos.Z;

	if (dDist < distLen) {
		const auto deltaPos = directionVec * dDist;
		newPos = heroPos + deltaPos;
	}	

	SetActorLocation(newPos);
}

bool AWandererPawn::IsNearCenterOfTile() const {

	const auto heroLocation = GetActorLocation();
	const auto* tileInfo = mazeField->GetTileInfoByRealPosition(heroLocation);

	if (!tileInfo){
		return false;
	}

	const auto tileCenterPos = mazeField->GetTileRealPosition(tileInfo->pos);

	auto distToCenterVec = tileCenterPos - heroLocation;
	distToCenterVec.Z = 0;
	const auto distToCenterLen = distToCenterVec.Length();

	constexpr float distToCenterSensitivity = 5.f;
	const bool isNearCenter = distToCenterLen <= distToCenterSensitivity;
	return isNearCenter;
}

bool AWandererPawn::AddToVisitQueue(const Vec2D& position) {

	const auto knownIt = knownTiles.find(position);

	if (knownIt != knownTiles.end())
		return false;
		// already known

	visitQueue.push_back(position);
	knownTiles.insert(position);
	return true;
}

void AWandererPawn::SortVisitQueue() {

	const auto* tile = mazeField->GetTileInfoByRealPosition(GetActorLocation());
	const auto heroPos = tile->pos;

	std::sort(visitQueue.begin(), visitQueue.end(), [heroPos](const Vec2D& pos1, const Vec2D& pos2) {
		return MazeField::CompareDistances(heroPos, pos1, pos2);
	});
}

bool AWandererPawn::SetUpdateFlagForSurroundings() const
{
	const auto* tile = mazeField->GetTileInfoByRealPosition(GetActorLocation());
	const auto& currentTileLogicalPos = tile->pos;
	const auto surroudingPositions = mazeField->GetTileLogicalPositionsInRadius(currentTileLogicalPos, searchRadius);
	const bool updateSet = mazeField->UpdateActorsAtPositions(surroudingPositions);

	return updateSet;
}

bool AWandererPawn::AddSurroundingsToVisitQueue() {

	const auto* tile = mazeField->GetTileInfoByRealPosition(GetActorLocation());
	const auto& currentTileLogicalPos = tile->pos;
	const auto surroudingPositions = mazeField->GetTileLogicalPositionsInRadius(currentTileLogicalPos, searchRadius);
	bool added = false;

	mazeField->SpawnActorsAtPositions(surroudingPositions);

	for (const auto& pos : surroudingPositions) {
		const auto* tileToCheck = mazeField->GetTileInfoByLogicalPosition(pos);

		if (!tileToCheck->visited && tileToCheck->tileType == TileType::FREE)
			added |= AddToVisitQueue(pos);
	}

	return added;
}

std::deque<Vec2D> AWandererPawn::GetPathToTile(const Vec2D& destination) const
{
	const auto* tile = mazeField->GetTileInfoByRealPosition(GetActorLocation());
	const auto& startTileLogicalPos = tile->pos;
	return mazeField->GetPathToTile(startTileLogicalPos, destination, knownTiles);
}

void AWandererPawn::ContinueSearch() {

	auto* tile = mazeField->GetTileInfoByRealPosition(GetActorLocation());
	if (!tile)
	{
		checkf(false, L"AWandererPawn::ContinueSearch error, no tile found");
		return;
	}

	const auto tilePos = tile->pos;
	const bool nearCenter = IsNearCenterOfTile();
	if (nearCenter) {

		if (!tile->IsVisited()) {
			tile->SetVisited();
			visitedCount++;

			mazeField->SetHeroPosition(0, tilePos);

			if (tile->tilePtr)
				tile->tilePtr->OnVisited();

			visitQueue.erase(std::remove_if(visitQueue.begin(),visitQueue.end(), [tilePos](const Vec2D& visitedPos) { return visitedPos == tilePos; }), visitQueue.end());
		}

		if (!currentMoveQueue.empty()) {

			if (currentMoveQueue.front() == tilePos) {
				currentMoveQueue.pop_front();
				AddSurroundingsToVisitQueue();

				traveledCount++;
			}
		}

		if (currentMoveQueue.empty())
			SortVisitQueue();

		while (currentMoveQueue.empty() && !visitQueue.empty()) {
			currentMoveQueue = GetPathToTile(visitQueue.front());
			visitQueue.pop_front();
		}
	}
}

void AWandererPawn::SetMazeField(MazeField* mazeFieldPtr) {

	mazeField = mazeFieldPtr;
}

void AWandererPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	ContinueSearch();
	Move(DeltaTime);
	SetUpdateFlagForSurroundings();
}

void AWandererPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

