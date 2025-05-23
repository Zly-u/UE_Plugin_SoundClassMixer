#pragma once


class FSCMEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    void         HandleBeginPIE(bool bIsSimulating);
    void         HandleEndPIE(bool bIsSimulating);
};
