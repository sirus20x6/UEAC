// Source/astrochicken/UI/ShipManagement/ShipManagementWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShipManagementWidget.generated.h"

class UShipInfrastructureManager;
class UWidgetSwitcher;
class UButton;
class UTextBlock;
class UProgressBar;
class UPowerGridPanel;
class UCompartmentStatusPanel;
class UHardpointPanel;
class UResourcePanel;

/**
 * Main ship management UI - displays all ship systems
 * Provides tabs for Power, Compartments, Hardpoints, Resources, and Alerts
 */
UCLASS()
class ASTROCHICKEN_API UShipManagementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Manual refresh
	UFUNCTION(BlueprintCallable, Category = "Ship Management")
	void RefreshAllPanels();

protected:
	// Main tab switcher
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* TabSwitcher;

	// Tab buttons
	UPROPERTY(meta = (BindWidget))
	UButton* PowerTabButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CompartmentsTabButton;

	UPROPERTY(meta = (BindWidget))
	UButton* HardpointsTabButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ResourcesTabButton;

	UPROPERTY(meta = (BindWidget))
	UButton* AlertsTabButton;

	// Quick status displays (always visible)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShipNameText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* OverallIntegrityBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OverallIntegrityText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PowerBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PowerText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* OxygenBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OxygenText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AlertCountText;

	// Panel containers (named slots for dynamic widgets)
	UPROPERTY(meta = (BindWidget))
	class UNamedSlot* PowerPanelSlot;

	UPROPERTY(meta = (BindWidget))
	class UNamedSlot* CompartmentsPanelSlot;

	UPROPERTY(meta = (BindWidget))
	class UNamedSlot* HardpointsPanelSlot;

	UPROPERTY(meta = (BindWidget))
	class UNamedSlot* ResourcesPanelSlot;

	UPROPERTY(meta = (BindWidget))
	class UNamedSlot* AlertsPanelSlot;

	// Panel widget classes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Management")
	TSubclassOf<UPowerGridPanel> PowerGridPanelClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Management")
	TSubclassOf<UCompartmentStatusPanel> CompartmentStatusPanelClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Management")
	TSubclassOf<UHardpointPanel> HardpointPanelClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Management")
	TSubclassOf<UResourcePanel> ResourcePanelClass;

	// Update interval
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Management")
	float UpdateInterval = 0.5f;

private:
	// Tab button callbacks
	UFUNCTION()
	void OnPowerTabClicked();

	UFUNCTION()
	void OnCompartmentsTabClicked();

	UFUNCTION()
	void OnHardpointsTabClicked();

	UFUNCTION()
	void OnResourcesTabClicked();

	UFUNCTION()
	void OnAlertsTabClicked();

	// Update functions
	void UpdateQuickStatus();
	void UpdateAlertCount();
	void SwitchToTab(int32 TabIndex);
	void UpdateTabButtonStates();

	// Create panel widgets
	void CreatePanels();

	UPROPERTY()
	UShipInfrastructureManager* InfraManager;

	UPROPERTY()
	UPowerGridPanel* PowerPanel;

	UPROPERTY()
	UCompartmentStatusPanel* CompartmentPanel;

	UPROPERTY()
	UHardpointPanel* HardpointPanelWidget;

	UPROPERTY()
	UResourcePanel* ResourcePanelWidget;

	int32 CurrentTabIndex = 0;
	float UpdateAccumulator = 0.0f;
	int32 AlertCount = 0;
};
