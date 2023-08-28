#include "ovb_common/platform/platform.h"
#include "ovb_recv/vmaf/vmaf.h"

void* VMAF::OpenVMAFLibrary()
{
	return Platform::GetDllHandle("libvmaf.dll");
}

bool VMAF::LoadVMAFFunctions(void* InLibrary, VMAF_API_FUNCTION_LIST* InFuncList)
{
	if (InLibrary == nullptr || InFuncList == nullptr)
	{
		return false;
	}

	InFuncList->VmafInit = (PFNVMAFINIT)(Platform::GetDllExport(InLibrary, "vmaf_init"));
	InFuncList->VmafUseFeaturesFromModel = (PFNVMAFUSEFEATURESFROMMODEL)(Platform::GetDllExport(InLibrary, "vmaf_use_features_from_model"));
	InFuncList->VmafUseFeaturesFromModelCollection = (PFNVMAFUSEFEATURESFROMMODELCOLLECTION)(Platform::GetDllExport(InLibrary, "vmaf_use_features_from_model_collection"));
	InFuncList->VmafUseFeature = (PFNVMAFUSEFEATURE)(Platform::GetDllExport(InLibrary, "vmaf_use_feature"));
	InFuncList->VmafImportFeatureScore = (PFNVMAFIMPORTFEATURESCORE)(Platform::GetDllExport(InLibrary, "vmaf_import_feature_score"));
	InFuncList->VmafReadPicture = (PFNVMAFREADPICTURE)(Platform::GetDllExport(InLibrary, "vmaf_read_pictures"));
	InFuncList->VmafScoreAtIndex = (PFNVMAFSCOREATINDEX)(Platform::GetDllExport(InLibrary, "vmaf_score_at_index"));
	InFuncList->VmafScoreAtIndexModelCollection = (PFNVMAFSCOREATINDEXMODELCOLLECTION)(Platform::GetDllExport(InLibrary, "vmaf_score_at_index_model_collection"));
	InFuncList->VmafFeatureScoreAtIndex = (PFNVMAFFEATURESCOREATINDEX)(Platform::GetDllExport(InLibrary, "vmaf_feature_score_at_index"));
	InFuncList->VmafScorePooled = (PFNVMAFSCOREPOOLED)(Platform::GetDllExport(InLibrary, "vmaf_score_pooled"));
	InFuncList->VmafScorePooledModelCollection = (PFNVMAFSCOREPOOLEDMODELCOLLECTION)(Platform::GetDllExport(InLibrary, "vmaf_score_pooled_model_collection"));
	InFuncList->VmafFeatureScorePooled = (PFNVMAFFEATURESCOREPOOLED)(Platform::GetDllExport(InLibrary, "vmaf_feature_score_pooled"));
	InFuncList->VmafClose = (PFNVMAFCLOSE)(Platform::GetDllExport(InLibrary, "vmaf_close"));
	InFuncList->VmafWriteOutput = (PFNVMAFWRITEOUTPUT)(Platform::GetDllExport(InLibrary, "vmaf_write_output"));
	InFuncList->VmafModelLoad = (PFNVMAFMODELLOAD)(Platform::GetDllExport(InLibrary, "vmaf_model_load"));
	InFuncList->VmafModelLoadFromPath = (PFNVMAFMODELLOADFROMPATH)(Platform::GetDllExport(InLibrary, "vmaf_model_load_from_path"));
	InFuncList->VmafModelFeatureOverload = (PFNVMAFMODELFEATUREOVERLOAD)(Platform::GetDllExport(InLibrary, "vmaf_model_feature_overload"));
	InFuncList->VmafModelDestroy = (PFNVMAFMODELDESTROY)(Platform::GetDllExport(InLibrary, "vmaf_model_destroy"));
	InFuncList->VmafModelCollectionLoad = (PFNVMAFMODELCOLLECTIONLOAD)(Platform::GetDllExport(InLibrary, "vmaf_model_collection_load"));
	InFuncList->VmafModelCollectionLoadFromPath = (PFNVMAFMODELCOLLECTIONLOADFROMPATH)(Platform::GetDllExport(InLibrary, "vmaf_model_collection_load_from_path"));
	InFuncList->VmafModelCollectionFeatureOverload = (PFNVMAFMODELCOLLECTIONFEATUREOVERLOAD)(Platform::GetDllExport(InLibrary, "vmaf_model_collection_feature_overload"));
	InFuncList->VmafModelCollectionDestroy = (PFNVMAFMODELCOLLECTIONDESTROY)(Platform::GetDllExport(InLibrary, "vmaf_model_collection_destroy"));

	return true;
}

void VMAF::CloseVMAFLibrary(void* InLibrary)
{
	Platform::FreeDllHandle(InLibrary);
}