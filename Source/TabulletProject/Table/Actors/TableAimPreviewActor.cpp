#include "TableAimPreviewActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TabulletProject/Table/Core/TableLog.h"
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
	PreviewMesh->SetRenderInMainPass(true);
	PreviewMesh->SetRenderInDepthPass(true);
	PreviewMesh->SetReceivesDecals(false);
	PreviewMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMesh(TEXT("/Game/Tabullet/Assets/Arrow/arrow.arrow"));
	if (ArrowMesh.Succeeded())
	{
		PreviewMesh->SetStaticMesh(ArrowMesh.Object);

		const FVector MeshSize = ArrowMesh.Object->GetBounds().BoxExtent * 2.0f;
		FVector LocalLengthAxis = FVector::ForwardVector;
		BaseMeshLength = MeshSize.X;

		if (MeshSize.Y > BaseMeshLength)
		{
			LocalLengthAxis = FVector::RightVector;
			BaseMeshLength = MeshSize.Y;
		}

		if (MeshSize.Z > BaseMeshLength)
		{
			LocalLengthAxis = FVector::UpVector;
			BaseMeshLength = MeshSize.Z;
		}

		PreviewMesh->SetRelativeRotation(FQuat::FindBetweenNormals(LocalLengthAxis, FVector::ForwardVector));
	}
	else
	{
		UE_LOG(LogTable, Error, TEXT("Failed to load table aim preview mesh: /Game/Tabullet/Assets/Arrow/arrow.arrow"));
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
	const FVector MeshDirection = bReverseMeshDirection ? -SafeDirection : SafeDirection;
	SetActorRotation(MeshDirection.Rotation());
	const float UniformScale = Length / SafeBaseLength;
	SetActorScale3D(FVector(UniformScale));
	PreviewMesh->UpdateBounds();
	AddActorWorldOffset(Center - PreviewMesh->Bounds.Origin);
	PreviewMesh->UpdateBounds();
	PreviewMesh->SetHiddenInGame(false);
	PreviewMesh->SetVisibility(true, true);
	PreviewMesh->MarkRenderStateDirty();
}

void ATableAimPreviewActor::HidePreview()
{
	if (IsValid(PreviewMesh))
	{
		PreviewMesh->SetVisibility(false, true);
	}

}
