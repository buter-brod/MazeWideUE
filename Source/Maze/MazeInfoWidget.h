#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/UMG.h"

#include "MazeInfoWidget.generated.h"

struct HUDInfo
{
	unsigned tilesVisited = 0;
	unsigned tilesInQueue = 0;
	unsigned tilesTraveled = 0;
	unsigned lightsPlaced = 0;
	unsigned tilesGenerated = 0;
};

UCLASS()
class MAZE_API UMazeInfoWidget : public UUserWidget
{
	GENERATED_BODY() //-V522

public:
	explicit UMazeInfoWidget(const FObjectInitializer&);
	
	virtual void NativeConstruct() override;

	void Update(const HUDInfo& hudInfo) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* tilesVisitedText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* tilesInQueueText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* tilesTraveledText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* lightsPlacedText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* tilesGeneratedText = nullptr;
};
