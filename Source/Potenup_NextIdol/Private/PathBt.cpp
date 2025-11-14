#include "PathBt.h"

#include "Components/Button.h"
#include "DesktopPlatformModule.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Containers/StringConv.h"

namespace
{
    constexpr const TCHAR* Mp4FileFilter = TEXT("MP4 Files (*.mp4)|*.mp4");
}

UPathBt::UPathBt(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UPathBt::NativeConstruct()
{
    Super::NativeConstruct();

    if (UserWeightButton)
    {
        UserWeightButton->OnClicked.AddDynamic(this, &UPathBt::HandleUserWeightClicked);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UserWeightButton is not bound on %s"), *GetName());
    }
}

void UPathBt::HandleUserWeightClicked()
{
    TArray<FString> SelectedFiles;
    if (!TrySelectMp4Files(SelectedFiles))
    {
        return;
    }

    if (SelectedFiles.Num() != ExpectedFileCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Expected %d files but received %d. Adjust ExpectedFileCount or selection."), ExpectedFileCount, SelectedFiles.Num());
        return;
    }

    DispatchUploadRequest(SelectedFiles);
}

bool UPathBt::TrySelectMp4Files(TArray<FString>& OutSelectedFiles) const
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (!DesktopPlatform)
    {
        UE_LOG(LogTemp, Warning, TEXT("DesktopPlatform module is unavailable; cannot open file dialog."));
        return false;
    }

    const FString DialogTitle = TEXT("Select MP4 files to upload");
    const FString DefaultPath = FPaths::DirectoryExists(DefaultBrowseFolder) ? DefaultBrowseFolder : FPaths::ProjectDir();

    const bool bDialogOpened = DesktopPlatform->OpenFileDialog(
        nullptr,
        DialogTitle,
        DefaultPath,
        TEXT(""),
        Mp4FileFilter,
        ExpectedFileCount > 1 ? EFileDialogFlags::Multiple : EFileDialogFlags::None,
        OutSelectedFiles);

    if (!bDialogOpened || OutSelectedFiles.Num() == 0)
    {
        return false;
    }

    return true;
}

bool UPathBt::BuildMultipartPayload(const TArray<FString>& FilePaths, const FString& Boundary, TArray<uint8>& OutPayload) const
{
    OutPayload.Reset();

    auto AppendString = [&OutPayload](const FString& Input)
    {
        FTCHARToUTF8 Converter(*Input);
        OutPayload.Append(reinterpret_cast<const uint8*>(Converter.Get()), Converter.Length());
    };

    for (int32 Index = 0; Index < FilePaths.Num(); ++Index)
    {
        const FString& FilePath = FilePaths[Index];

        if (!FPaths::FileExists(FilePath))
        {
            UE_LOG(LogTemp, Warning, TEXT("File does not exist: %s"), *FilePath);
            return false;
        }

        TArray<uint8> FileData;
        if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to read file: %s"), *FilePath);
            return false;
        }

        const FString FieldName = FString::Printf(TEXT("file%d"), Index + 1);
        const FString FileHeader = FString::Printf(
            TEXT("--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: video/mp4\r\n\r\n"),
            *Boundary,
            *FieldName,
            *FPaths::GetCleanFilename(FilePath));

        AppendString(FileHeader);
        OutPayload.Append(FileData);
        AppendString(TEXT("\r\n"));
    }

    const FString ClosingBoundary = FString::Printf(TEXT("--%s--\r\n"), *Boundary);
    AppendString(ClosingBoundary);

    return true;
}

void UPathBt::DispatchUploadRequest(const TArray<FString>& FilePaths)
{
    const FString Boundary = FString::Printf(TEXT("----UEBoundary-%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits));

    TArray<uint8> Payload;
    if (!BuildMultipartPayload(FilePaths, Boundary, Payload))
    {
        return;
    }

    if (!FHttpModule::Get().IsHttpEnabled())
    {
        UE_LOG(LogTemp, Warning, TEXT("HTTP module is disabled; cannot send upload request."));
        return;
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(UploadEndpoint);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
    Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
    Request->SetContent(Payload);
    Request->OnProcessRequestComplete().BindUObject(this, &UPathBt::HandleUploadResponse);
    Request->ProcessRequest();

    UE_LOG(LogTemp, Log, TEXT("Uploading %d file(s) to %s"), FilePaths.Num(), *UploadEndpoint);
}

void UPathBt::HandleUploadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Upload failed: request unsuccessful."));
        return;
    }

    const int32 StatusCode = Response->GetResponseCode();
    UE_LOG(LogTemp, Log, TEXT("Upload response %d: %s"), StatusCode, *Response->GetContentAsString());
}
