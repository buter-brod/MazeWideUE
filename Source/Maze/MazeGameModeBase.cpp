#include "MazeGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "MazeHUD.h"

constexpr float mazeServiceInterval = 2.f;

AMazeGameModeBase::AMazeGameModeBase(const FObjectInitializer& ObjectInitializer) : AGameModeBase(ObjectInitializer)
{
	MazeField::InitSubclasses();
	PrimaryActorTick.bCanEverTick = true;
}

void AMazeGameModeBase::Tick(float dt) {

	mazeTimer -= dt;

	if (mazeTimer <= 0.f) {
		mazeTimer = mazeServiceInterval;
		mazeField->Update();

		AMazeHUD* hud = Cast<AMazeHUD> (GetWorld()->GetFirstPlayerController()->GetHUD());
		HUDInfo hudInfo;
		hudInfo.tilesVisited = hero->GetVisitedCount();
		hudInfo.tilesInQueue = hero->GetVisitQueueSize();
		hudInfo.tilesTraveled = hero->GetTraveledCount();
		hudInfo.lightsPlaced = mazeField->GetLightsCount();
		hudInfo.tilesGenerated = mazeField->GetTilesGenerated();
		hud->Update(hudInfo);
	}
}

void AMazeGameModeBase::BeginPlay(){

	mazeField = std::make_shared<MazeField>();
	mazeField->Init(GetWorld());

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("MainHero"), OutActors);
	hero = dynamic_cast<AWandererPawn*>(*OutActors.begin());

	hero->SetMazeField(mazeField.get());
	
}