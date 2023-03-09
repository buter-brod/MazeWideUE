#include "TileBase.h"

#include "WandererPawn.h"
#include "Kismet/GameplayStatics.h"

constexpr float distToStartAppear = 1000.f;
constexpr float distToFullyAppear = 400.f;

constexpr float zToHideAt = -800.f;

constexpr bool fallAfterMoveAway = false;

ATileBase::ATileBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

ATileBase::~ATileBase() {
}

void ATileBase::BeginPlay()
{
	Super::BeginPlay();
}

void ATileBase::SetTileInfo(TileInfo* tileInfoPtr) {

	tileInfo = tileInfoPtr;
}

bool ATileBase::SetNeedUpdate() {

	if (!needUpdate) {
		needUpdate = true;
		return true;
	}

	return false;
}

void ATileBase::OnVisited() {
}

void ATileBase::Update() {

	if (!hero) {

		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("MainHero"), OutActors);
		hero = dynamic_cast<AWandererPawn*>(*OutActors.begin());

		auto location = GetActorLocation();
		location.Z = zToHideAt;
		SetActorLocation(location);
	}

	// todo: should not iterate all existing tiles in that way. Better approach might be getting X*X tiles grid around hero and work with these only

	auto location = GetActorLocation();
	const auto heroLocation = hero->GetActorLocation();
	const auto distance = FVector::Dist(heroLocation, location);

	location.Z = 0;

	const bool distanceClose = distance <= distToFullyAppear;
	const bool distanceWithinAppearRadius = distance <= distToStartAppear;

	if (distanceClose || tileInfo->visited)
	{
		tileInfo->fullyAppeared = true;
	}
	else if (!tileInfo->fullyAppeared || fallAfterMoveAway) {

		if (distanceWithinAppearRadius) {
			const float outScaleProgress = (distance - distToFullyAppear) / (distToStartAppear - distToFullyAppear);
			location.Z = outScaleProgress * zToHideAt;
		}
		else {
			location.Z = zToHideAt;
		}
	}

	SetActorLocation(location);

	needUpdate = false;
}

void ATileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (needUpdate)
		Update();	
}

