#include "MazeHUD.h"

AMazeHUD::AMazeHUD() {
}

void AMazeHUD::DrawHUD() {
	Super::DrawHUD();
}

void AMazeHUD::BeginPlay() {
	Super::BeginPlay();

	if (MazeInfoWidgetClass)
	{
		mazeWidget = CreateWidget<UMazeInfoWidget>(GetWorld(), MazeInfoWidgetClass);
		if (mazeWidget)
		{
			mazeWidget->AddToViewport(0);
		}
	}
}

void AMazeHUD::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

void AMazeHUD::Update(const HUDInfo& hudInfo) {
	if (mazeWidget)
		mazeWidget->Update(hudInfo);
}
