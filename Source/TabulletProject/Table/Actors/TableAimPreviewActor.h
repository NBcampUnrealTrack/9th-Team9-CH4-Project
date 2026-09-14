#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TableAimPreviewActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class TABULLETPROJECT_API ATableAimPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ATableAimPreviewActor();
	void SetPreview(const FVector& Start, const FVector& Direction, float Length);

	UFUNCTION(BlueprintCallable, Category = "Table | Aim Preview")
	void HidePreview();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table | Aim Preview")
	TObjectPtr<UStaticMeshComponent> PreviewMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Aim Preview", meta = (ClampMin = "0.01"))
	float BaseMeshLength = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Aim Preview", meta = (ClampMin = "0.01"))
	float WidthScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Aim Preview")
	float HeightOffset = 0.0f;
};
