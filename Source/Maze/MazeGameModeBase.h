#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MazeField.h"
#include "WandererPawn.h"
#include "MazeGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MAZE_API AMazeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	virtual void Tick(float dt) override;

	std::shared_ptr<MazeField> mazeField;

	explicit AMazeGameModeBase(const FObjectInitializer& ObjectInitializer);

private:
	AWandererPawn* hero = nullptr;
	float mazeTimer = 0.f;
};
