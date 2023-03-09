#pragma once

#include "CoreMinimal.h"
#include "MazeField.h"
#include "GameFramework/Pawn.h"
#include <set>
#include <queue>
#include "WandererPawn.generated.h"

UCLASS()
class MAZE_API AWandererPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AWandererPawn();

	void SetMazeField(MazeField* mazeFieldPtr);
	

	unsigned GetSearchRadius() const {return searchRadius;}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool IsNearCenterOfTile() const;
	void ContinueSearch();
	void Move(const float dt);
	bool AddToVisitQueue(const Vec2D& position);
	bool AddSurroundingsToVisitQueue();
	bool SetUpdateFlagForSurroundings() const;

	void SortVisitQueue();
	std::deque<Vec2D> GetPathToTile(const Vec2D& destination) const;

private:
	MazeField* mazeField = nullptr;

	std::set<Vec2D> knownTiles;
	std::deque<Vec2D> visitQueue;
	std::deque<Vec2D> currentMoveQueue;

	unsigned searchRadius = 8;

	static Vec2D goalUndefined;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
