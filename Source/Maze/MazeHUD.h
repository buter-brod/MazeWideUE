#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MazeInfoWidget.h"
#include "MazeHUD.generated.h"


UCLASS()
class MAZE_API AMazeHUD : public AHUD
{
	GENERATED_BODY()

	AMazeHUD();

	void DrawHUD() override;
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> MazeInfoWidgetClass;

public:
	void Update(const HUDInfo& hudInfo);

private:
	UMazeInfoWidget* mazeWidget = nullptr;
	
};
