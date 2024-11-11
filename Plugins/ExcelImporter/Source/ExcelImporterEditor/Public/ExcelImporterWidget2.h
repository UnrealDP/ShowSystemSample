#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ExcelImportSettings.h"

namespace OpenXLSX {
    class XLWorksheet;  // XLWorksheet sms OpenXLSX 네임스페이스
}


class SExcelImporterWidget2 : public SCompoundWidget
{
    struct FImportStruct
    {
        FString StructName;
        TSoftObjectPtr<UScriptStruct> Inherited;
        TArray<FString> VariableNames;
        TArray<FString> DataTypes;
    };

    struct FExcelFileItem
    {
        FExcelImportSettings ExcelImportSettings;  // 액셀 import 정보
        bool bIsChecked;    // 체크 상태

        FExcelFileItem(const FExcelImportSettings& InExcelImportSettings)
            : ExcelImportSettings(InExcelImportSettings), bIsChecked(false)
        {}

        FString GetFullExcelFilePath() const
        {
            return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / ExcelImportSettings.ExcelFilePath);
        }
        FString GetSheetName() const
        {
            return ExcelImportSettings.SheetName;
        }
        FString GetFullGeneratedCodePath() const
        {
            return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / ExcelImportSettings.GeneratedCodePath);
        }
        FString GetFullDataTablePath() const
        {
            return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / ExcelImportSettings.DataTablePath);
        }
        FString GetStructPrefix() const
        {
            return ExcelImportSettings.StructPrefix;
        }
        TSoftObjectPtr<UScriptStruct> GetInherited() const
        {
            return ExcelImportSettings.InheritedDataStruct;
        }
    };


public:
    SLATE_BEGIN_ARGS(SExcelImporterWidget2) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    void LoadData();
    void LoadExcelFiles();
    EVisibility GetErrorMessageVisibility() const;

    FReply OnGeneratedClicked();
    FReply OnSelectAllButtonClicked();    
    FReply OnUnSelectAllButtonClicked();
    FReply OnCreateDataTableClicked();

    TSharedRef<ITableRow> GenerateFileRow(TSharedPtr<FExcelFileItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);
    ECheckBoxState GetCheckBoxState(TSharedPtr<FExcelFileItem> InItem) const;
    void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FExcelFileItem> InItem);
    void OnFileCheckboxChanged(ECheckBoxState NewState, TSharedPtr<FString> FileName);

    bool ConvertMultipleExcelToCPP(TArray<TSharedPtr<FExcelFileItem>>& ExcelFileItems);
    FString GenerateCPPCode(
        const FString& StructName, 
        const FString& StructPrefix, 
        const TSoftObjectPtr<UScriptStruct> Inherited, 
        const TArray<FString>& VariableNames, 
        const TArray<FString>& DataTypes, 
        const TArray<FImportStruct>& Structs);
    FString GenerateFieldCPPCode(
        const TSoftObjectPtr<UScriptStruct> Inherited,
        const TArray<FString>& VariableNames,
        const TArray<FString>& DataTypes);
    FString GenerateCPPStructCode(
        const FString& StructName,
        const FString& StructPrefix, 
        const TSoftObjectPtr<UScriptStruct> Inherited, 
        const TArray<FString>& VariableNames, 
        const TArray<FString>& DataTypes);
    void GenerateEnumHeader(const FString& FilePath, const TArray<FString>& EnumStrs);

    bool CreateDataTable(const TArray<FString>& ExcelPaths, const TArray<FString>& SheetNames, TArray<FString>& GeneratedCodePaths, const TArray<FString>& DataTablePaths);
    bool CreateDataTableFromExcel(const FString& ExcelFilePath, const FString& SheetName, const FString& GeneratedCodePath, const FString& DataTablePath);
    UScriptStruct* LoadStructFromHeaderPath(const FString& HeaderFilePath);
    UDataTable* CreateNewOrClearDataTable(UScriptStruct* RowStruct, const FString& ClassName, const FString& DataTablePath);
    UDataTable* ClearDataTableRows(const FString& DataTablePath, const FString& DataTableName);
    FTableRowBase* CreateDataTableRowFromExcel(const UScriptStruct* const RowStruct, const TArray<FString>& VariableNames, const OpenXLSX::XLWorksheet& wks, int32 RowIndex);
    void SaveDataTableAsset(UDataTable* DataTable, const FString& DataTablePath);

    FText ErrorMessage;
    UDataTable* ExcelImportSettingsDataTable;
    UDataTable* DataTypeSettingsDataTable;
    TArray<TSharedPtr<FExcelFileItem>> ExcelFiles;  // Excel 파일 리스트
    TSharedPtr<SListView<TSharedPtr<FExcelFileItem>>> ExcelListView;  // SListView 멤버 변수 추가
};
