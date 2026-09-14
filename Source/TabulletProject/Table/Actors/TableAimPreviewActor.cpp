#include "TableAimPreviewActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ATableAimPreviewActor::ATableAimPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	SetRootComponent(PreviewMesh);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetGenerateOverlapEvents(false);
	PreviewMesh->SetCastShadow(false);
	PreviewMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMesh(TEXT("/Game/Tabullet/Assets/Arrow/arrow"));
	if (ArrowMesh.Succeeded())
	{
		PreviewMesh->SetStaticMesh(ArrowMesh.Object);
	}
}

void ATableAimPreviewActor::SetPreview(const FVector& Start, const FVector& Direction, float Length)
{
	if (!IsValid(PreviewMesh) || Direction.IsNearlyZero() || Length <= 0.0f)
	{
		HidePreview();
		return;
	}

	const FVector SafeDirection = Direction.GetSafeNormal();
	const float SafeBaseLength = FMath::Max(BaseMeshLength, KINDA_SMALL_NUMBER);
	const FVector Center = Start + SafeDirection * (Length * 0.5f) + FVector::UpVector * HeightOffset;

	SetActorLocation(Center);
	SetActorRotation(SafeDirection.Rotation());
	SetActorScale3D(FVector(Length / SafeBaseLength, WidthScale, WidthScale));
	PreviewMesh->SetVisibility(true);
}

void ATableAimPreviewActor::HidePreview()
{
	if (IsValid(PreviewMesh))
	{
		PreviewMesh->SetVisibility(false);
	}
}
