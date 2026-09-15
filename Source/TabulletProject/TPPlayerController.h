#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TPPlayerState.h"
#include "TPPlayerController.generated.h"

class AFlickTableBase;
class ATableBulletPiece;
class UTableFlickInputComponent;
class ATPGameState;
class UUserWidget;
class UInputMappingContext;
class UInputAction;
class ACameraActor;
/**
 * 
 */
UCLASS()
class TABULLETPROJECT_API ATPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATPPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnRep_PlayerState() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Table | Flick")
	void ServerRequestFlick(AFlickTableBase* Table, ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Lobby")
	void ServerSetLobbyReady(bool bReady);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Lobby")
	void ServerSelectLobbyCharacter(ETPCharacterType CharacterType);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ToggleLobbyReady();

	UFUNCTION(BlueprintPure, Category = "Lobby")
	FText GetLobbyReadyButtonText() const;

	UFUNCTION(BlueprintPure, Category = "Lobby")
	FText GetLobbyPlayerNameText(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Lobby")
	FText GetLobbyPlayerCharacterText(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Lobby")
	FText GetLobbyPlayerReadyText(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Lobby")
	FText GetLobbyPlayerCountText() const;

	UFUNCTION(BlueprintPure, Category = "Lobby")
	FText GetLobbyStatusText() const;

	void SetSelectedLobbyCharacterType(ETPCharacterType NewCharacterType);

	UFUNCTION(BlueprintPure, Category = "Lobby")
	ETPCharacterType GetSelectedLobbyCharacterType() const { return SelectedLobbyCharacterType; }

	UFUNCTION(Client, Reliable)
	void ClientFlickRequestRejected();

	UFUNCTION(BlueprintPure, Category = "Turn")
	bool IsMyTurn() const;

	UFUNCTION(BlueprintPure, Category = "Table | Input")
	UTableFlickInputComponent* GetTableFlickInputComponent() const { return TableFlickInputComponent; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	void RefreshMouseInputMode();
	
	void ShowCrosshair();
	void HideCrosshair();

	// T키: 생존 시엔 ViewModeComponent로 탑다운 전환, 사망 후엔 고정 TopView 카메라로 전환
	void HandleTopViewInput();

	// F키: 생존 시엔 ViewModeComponent로 1인칭 전환, 사망 후엔 고정 DeathQuarterView 카메라로 전환
	void HandleDeathQuarterViewInput();

protected:
	static FText GetCharacterTypeText(ETPCharacterType CharacterType);
	const ATPPlayerState* GetLobbyPlayerStateAt(int32 SlotIndex) const;

	void BindGameStateInputEvents();
	AFlickTableBase* FindFlickTable();
	bool IsTopDownViewMode() const;
	bool ShouldEnableTableInput() const;
	bool IsGameViewportFocused() const;
	void ApplyPendingInputMode();
	void SwitchToFixedCamera(ACameraActor* TargetCamera);

	UFUNCTION()
	void HandleLocalPawnDeathVisual();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table | Input")
	TObjectPtr<UTableFlickInputComponent> TableFlickInputComponent;

	UPROPERTY()
	TObjectPtr<ATPGameState> BoundGameState;

	UPROPERTY()
	TObjectPtr<AFlickTableBase> CachedFlickTable;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	// T/F 시점 전환 입력 (생존/사망 공통 - Pawn이 아니라 Controller가 소유)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> ViewMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> TopViewAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DeathQuarterViewAction;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float DeathCameraBlendTime = 0.75f;

	UPROPERTY()
	TObjectPtr<UUserWidget> CrosshairWidgetInstance;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby", meta = (AllowPrivateAccess = "true"))
	ETPCharacterType SelectedLobbyCharacterType = ETPCharacterType::None;

	// 마지막으로 SetInputMode에 넘긴 모드. 같은 모드를 다시 적용하지 않기 위한 값
	bool bHasAppliedInputMode = false;
	bool bLastAppliedTopDownCursor = false;

	// 포커스가 없어 아직 적용하지 못한 입력 모드
	bool bInputModeDirty = false;
	bool bPendingTopDownCursor = false;
};
