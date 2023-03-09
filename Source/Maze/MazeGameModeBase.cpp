#include "MazeGameModeBase.h"
#include "Kismet/GameplayStatics.h"

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