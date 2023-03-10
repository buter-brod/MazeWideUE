#include "MazeInfoWidget.h"

UMazeInfoWidget::UMazeInfoWidget(const FObjectInitializer& initializer) : Super(initializer) {

}

void UMazeInfoWidget::Update(const HUDInfo& hudInfo) const
{
	tilesVisitedText  ->SetText(FText::FromString(FString("TILES VISITED : ") + FString::FromInt(hudInfo.tilesVisited)));
	tilesInQueueText  ->SetText(FText::FromString(FString("TILES IN QUEUE: ") + FString::FromInt(hudInfo.tilesInQueue)));
	tilesTraveledText ->SetText(FText::FromString(FString("TILES TRAVELED: ") + FString::FromInt(hudInfo.tilesTraveled)));
	lightsPlacedText  ->SetText(FText::FromString(FString("LIGHTS PLACED : ") + FString::FromInt(hudInfo.lightsPlaced)));
	tilesGeneratedText->SetText(FText::FromString(FString("TILES CREATED : ") + FString::FromInt(hudInfo.tilesGenerated)));
}

void UMazeInfoWidget::NativeConstruct() {
	Super::NativeConstruct();

}