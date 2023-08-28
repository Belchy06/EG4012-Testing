#pragma once

#include "libvmaf/libvmaf.h"

#define VMAFAPI

typedef int(VMAFAPI* PFNVMAFINIT)(VmafContext** InVmaf, VmafConfiguration InCfg);
typedef int(VMAFAPI* PFNVMAFUSEFEATURESFROMMODEL)(VmafContext* InVmaf, VmafModel* InModel);
typedef int(VMAFAPI* PFNVMAFUSEFEATURESFROMMODELCOLLECTION)(VmafContext* InVmaf, VmafModelCollection* InModelCollection);
typedef int(VMAFAPI* PFNVMAFUSEFEATURE)(VmafContext* InVmaf, const char* InFeatureName, VmafFeatureDictionary* InOptsDic);
typedef int(VMAFAPI* PFNVMAFIMPORTFEATURESCORE)(VmafContext* InVmaf, const char* InFeatureName, double InValue, unsigned InIndex);
typedef int(VMAFAPI* PFNVMAFREADPICTURE)(VmafContext* InVmaf, VmafPicture* InRef, VmafPicture* InDist, unsigned InIndex);
typedef int(VMAFAPI* PFNVMAFSCOREATINDEX)(VmafContext* InVmaf, VmafModel* InModel, double* InScore, unsigned InIndex);
typedef int(VMAFAPI* PFNVMAFSCOREATINDEXMODELCOLLECTION)(VmafContext* InVmaf, VmafModelCollection* InModelCollection, VmafModelCollectionScore* InScore, unsigned InIndex);
typedef int(VMAFAPI* PFNVMAFFEATURESCOREATINDEX)(VmafContext* InVmaf, const char* InFeatureName, double* InScore, unsigned InIndex);
typedef int(VMAFAPI* PFNVMAFSCOREPOOLED)(VmafContext* InVmaf, VmafModel* InModel, enum VmafPoolingMethod InPoolMethod, double* InScore, unsigned InIndexLow, unsigned InIndexHigh);
typedef int(VMAFAPI* PFNVMAFSCOREPOOLEDMODELCOLLECTION)(VmafContext* InVmaf, VmafModelCollection* InModelCollection, enum VmafPoolingMethod InPoolMethod, VmafModelCollectionScore* InScore, unsigned InIndexLow, unsigned InIndexHigh);
typedef int(VMAFAPI* PFNVMAFFEATURESCOREPOOLED)(VmafContext* InVmaf, const char* InFeatureName, enum VmafPoolingMethod InPoolMethod, double* InScore, unsigned InIndexLow, unsigned InIndexHigh);
typedef int(VMAFAPI* PFNVMAFCLOSE)(VmafContext* InVmaf);
typedef int(VMAFAPI* PFNVMAFWRITEOUTPUT)(VmafContext* InVmaf, const char* InOutputPath, enum VmafOutputFormat InFmt);
typedef int(VMAFAPI* PFNVMAFMODELLOAD)(VmafModel** OutModel, VmafModelConfig* InCfg, const char* Version);
typedef int(VMAFAPI* PFNVMAFMODELLOADFROMPATH)(VmafModel** OutModel, VmafModelConfig* InCfg, const char* InPath);
typedef int(VMAFAPI* PFNVMAFMODELFEATUREOVERLOAD)(VmafModel* InModel, const char* InFeatureName, VmafFeatureDictionary* InOptsDict);
typedef int(VMAFAPI* PFNVMAFMODELDESTROY)(VmafModel* InModel);
typedef int(VMAFAPI* PFNVMAFMODELCOLLECTIONLOAD)(VmafModel** OutModel, VmafModelCollection** OutModelCollection, VmafModelConfig* InCfg, const char* InVersion);
typedef int(VMAFAPI* PFNVMAFMODELCOLLECTIONLOADFROMPATH)(VmafModel** OutModel, VmafModelCollection** OutModelCollection, VmafModelConfig* InCfg, const char* InPath);
typedef int(VMAFAPI* PFNVMAFMODELCOLLECTIONFEATUREOVERLOAD)(VmafModel* InModel, VmafModelCollection** OutModelCollection, const char* InFeatureName, VmafFeatureDictionary* InOptsDict);
typedef int(VMAFAPI* PFNVMAFMODELCOLLECTIONDESTROY)(VmafModelCollection* InModelCollection);

struct VMAF_API_FUNCTION_LIST
{
	PFNVMAFINIT							  VmafInit;
	PFNVMAFUSEFEATURESFROMMODEL			  VmafUseFeaturesFromModel;
	PFNVMAFUSEFEATURESFROMMODELCOLLECTION VmafUseFeaturesFromModelCollection;
	PFNVMAFUSEFEATURE					  VmafUseFeature;
	PFNVMAFIMPORTFEATURESCORE			  VmafImportFeatureScore;
	PFNVMAFREADPICTURE					  VmafReadPicture;
	PFNVMAFSCOREATINDEX					  VmafScoreAtIndex;
	PFNVMAFSCOREATINDEXMODELCOLLECTION	  VmafScoreAtIndexModelCollection;
	PFNVMAFFEATURESCOREATINDEX			  VmafFeatureScoreAtIndex;
	PFNVMAFSCOREPOOLED					  VmafScorePooled;
	PFNVMAFSCOREPOOLEDMODELCOLLECTION	  VmafScorePooledModelCollection;
	PFNVMAFFEATURESCOREPOOLED			  VmafFeatureScorePooled;
	PFNVMAFCLOSE						  VmafClose;
	PFNVMAFWRITEOUTPUT					  VmafWriteOutput;
	PFNVMAFMODELLOAD					  VmafModelLoad;
	PFNVMAFMODELLOADFROMPATH			  VmafModelLoadFromPath;
	PFNVMAFMODELFEATUREOVERLOAD			  VmafModelFeatureOverload;
	PFNVMAFMODELDESTROY					  VmafModelDestroy;
	PFNVMAFMODELCOLLECTIONLOAD			  VmafModelCollectionLoad;
	PFNVMAFMODELCOLLECTIONLOADFROMPATH	  VmafModelCollectionLoadFromPath;
	PFNVMAFMODELCOLLECTIONFEATUREOVERLOAD VmafModelCollectionFeatureOverload;
	PFNVMAFMODELCOLLECTIONDESTROY		  VmafModelCollectionDestroy;
};

class VMAF
{
public:
	// Attempts to load the shared library for the VMAF API
	static void* OpenVMAFLibrary();

	// Attempts to retrieve the list of function pointers for the VMAF API shared library
	static bool LoadVMAFFunctions(void* InLibrary, VMAF_API_FUNCTION_LIST* InFuncList);

	// Closes a previously-loaded VMAF shared library
	static void CloseVMAFLibrary(void* InLibrary);
};
