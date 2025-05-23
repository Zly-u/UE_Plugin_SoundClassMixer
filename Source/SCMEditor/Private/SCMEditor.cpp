#include "SCMEditor.h"

#include "Editor.h"
#include "SoundClassMixerCommands.h"

#define LOCTEXT_NAMESPACE "FSCMEditorModule"

void FSCMEditorModule::StartupModule()
{
	FEditorDelegates::PostPIEStarted.AddRaw(this, &FSCMEditorModule::HandleBeginPIE);
	
}


void FSCMEditorModule::ShutdownModule()
{
	FEditorDelegates::PrePIEEnded.AddRaw(this, &FSCMEditorModule::HandleEndPIE);
}


void FSCMEditorModule::HandleBeginPIE(bool bIsSimulating)
{
	if(bIsSimulating)
	{
		return;
	}
	
	USoundClassMixerSubsystem* SCMS = GEditor->GetPIEWorldContext()->World()->GetGameInstance()->GetSubsystem<USoundClassMixerSubsystem>();
	FSoundClassMixerCommands::RegisterCommands(SCMS);
}


void FSCMEditorModule::HandleEndPIE(bool bIsSimulating)
{
	if(bIsSimulating)
	{
		return;
	}
	
	FSoundClassMixerCommands::UnregisterCommands();
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSCMEditorModule, SCMEditor)