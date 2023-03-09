#pragma once

#include "CoreMinimal.h"
#include "MazeField.h"
#include "WandererPawn.h"
#include "GameFramework/Actor.h"
#include "TileBase.generated.h"

struct TileInfo;
class APointLight;

UCLASS()
class MAZE_API ATileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ATileBase();
	virtual ~ATileBase() override;
	bool SetNeedUpdate();

	void Update();
	void OnVisited();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	AWandererPawn* hero = nullptr;
	TileInfo* tileInfo = nullptr;
	bool needUpdate = false;
	bool lightOn = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetTileInfo(TileInfo* tileInfoPtr);

};
