// Copyright © Bedrockbreaker 2025. MIT License

#include "MementoSaveSystemEditor.h"

#include "Editor.h"
#include "Engine/GameInstance.h"
#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "SaveSystem/MSaveManager.h"
#include "Types/SlateEnums.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SMSaveGraph.h"

IMPLEMENT_MODULE(FMementoSaveSystemEditorModule, MementoSaveSystemEditor);

void FMementoSaveSystemEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("FMementoSaveSystemEditorModule::StartupModule"));
	FEditorDelegates::PostPIEStarted.AddRaw(this, &FMementoSaveSystemEditorModule::OnPIEStarted);
	FEditorDelegates::PrePIEEnded.AddRaw(this, &FMementoSaveSystemEditorModule::OnPIEEnded);

	FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(
			"MSaveSystemGraphTab", FOnSpawnTab::CreateRaw(this, &FMementoSaveSystemEditorModule::SpawnSaveGraphTab))
		.SetDisplayName(NSLOCTEXT("MementoSaveSystem", "SaveGraphTab", "Save Graph"));
}

void FMementoSaveSystemEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("FMementoSaveSystemEditorModule::ShutdownModule"));
	FEditorDelegates::PostPIEStarted.RemoveAll(this);
	FEditorDelegates::PrePIEEnded.RemoveAll(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("MSaveSystemGraphTab");
}

void FMementoSaveSystemEditorModule::OnPIEStarted(bool bIsSimulating)
{
	UE_LOG(LogTemp, Warning, TEXT("FMementoSaveSystemEditorModule::OnPIEStarted"));
	if (!GEditor) return;
	UWorld* World = GEditor->PlayWorld;
	if (!World) return;
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance) return;
	UMSaveManager* SaveManager = GameInstance->GetSubsystem<UMSaveManager>();
	if (!SaveManager) return;

	SaveManager->OnSaveSlotUpdated.AddRaw(this, &FMementoSaveSystemEditorModule::OnSaveSlotUpdated);
}

void FMementoSaveSystemEditorModule::OnPIEEnded(bool bIsSimulating)
{
	UE_LOG(LogTemp, Warning, TEXT("FMementoSaveSystemEditorModule::OnPIEEnded"));
	if (!GEditor) return;
	UWorld* World = GEditor->PlayWorld;
	if (!World) return;
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance) return;
	UMSaveManager* SaveManager = GameInstance->GetSubsystem<UMSaveManager>();
	if (!SaveManager) return;

	SaveManager->OnSaveSlotUpdated.RemoveAll(this);
}

void FMementoSaveSystemEditorModule::OnSaveSlotUpdated(UMSaveGame* SaveGame)
{
	if (!SaveGraphWidget) return;
	SaveGraphWidget->RefreshGraph(SaveGame);
}

TSharedRef<SDockTab> FMementoSaveSystemEditorModule::SpawnSaveGraphTab(const FSpawnTabArgs& SpawnTabArgs)
{
	SaveGraphWidget = SNew(SMSaveGraph);

	// clang-format off
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)
	[
		SNew(SScrollBox)
		.Orientation(Orient_Vertical)
		+ SScrollBox::Slot()
		[
			SNew(SScrollBox)
			.Orientation(Orient_Horizontal)
			+ SScrollBox::Slot()
			[
				SaveGraphWidget.ToSharedRef()
			]
		]
	];
	// clang-format on
}
