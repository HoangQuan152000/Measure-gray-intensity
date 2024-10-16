#pragma once
#include "xvtCV/xvtDefine.h"
#include <string>
#include <vector>

#define XVT_VERSION_MAJOR 0
#define XVT_VERSION_MINOR 2
#define XVT_VERSION_PATCH 8
#define XVT_VERSION_NUMBER 1
#define XVT_VERSION_COMIT 4f95c17
#define XVT_VERSION_DIRTY 
#define XVT_BRANCH 

namespace xvt {

class XVTCV_EXPORTS VersionInfo
{
public:
    auto GetVersionInfo(std::vector<VersionInfo const*>* list = nullptr)const->std::string;
    auto GetDependenciesInfo(std::vector<VersionInfo const*>* list=nullptr)const->std::string;

    std::string mMajor;
    std::string mMinor;
    std::string mPatch;
    std::string mNumber;
    std::string mCommit;
    std::string mDirty;
    std::string mBranch;
    std::string mName;
    //Pointer to submodule dependencies
    std::vector<VersionInfo const*> mDependencies{};

    mutable bool mIsPrinted = false;
};

extern XVTCV_EXPORTS const VersionInfo xvtCV_VersionInfo;
extern XVTCV_EXPORTS const VersionInfo opencv_VersionInfo;
extern XVTCV_EXPORTS const VersionInfo zlib_VersionInfo;
using VersionInfoCPtrList = std::vector<VersionInfo const*>;

}
