#if !defined(_FREngine_H_)
#define _FREngine_H_

typedef struct _tagSRect
{
    int     left;
    int     top;
    int     width;
    int     height;
}SRect;

typedef struct _tagSPoint
{
    int     x;
    int     y;
}SPoint;

typedef struct _tagPoint2D
{
    float   x;
    float   y;
}Point2D;


typedef struct _tagSImg
{
	int nH;
	int nW;
	unsigned char* pbImage;
}SImg;

typedef struct _tagARM_FEATURE{
 float rInitSim;
 int  m_pnFeature[0xFA];
}ARM_Feature;


typedef struct _tagModelingPoints
{
	int nModelNum;
	Point2D pxModelPoints[27];
	float rPan;
        int fPan;
}ModelingPoints;


struct Time{
 int nShot, nFDTime, nFMTime, nFFTime, nFATime;
};

typedef struct _tagDistNo
{
	float rDist;
	int nNo;
}DistNo;

typedef struct _tagPerson1
{
	int nFeatureNum;
	ARM_Feature* pxFeatures;
}Person1;

typedef struct _tagPersonDatabase1
{
	int nPersonNum;
	Person1* pxPersons;
}PersonDatabase1;


typedef struct _tagModelResult
{
    SRect xFaceRegion;
    int nModelNum;
    SPoint xFaceModel[27];
}ModelResult;

typedef struct _tagArmRect
{
	float rX;
	float rY;
	float m_rRate;
}ArmRect;

typedef struct _tagDetectionResult
{
	ArmRect xFaceRect;
	float rPan;
	Point2D pxModelPoints[6];
}DetectionResult;

typedef struct _tagMFResult
{
	int nModelNum;
	Point2D pxModelPoints[27];
	ARM_Feature xFeature;
	float rModelConfidence;
    int  fPan;
}MFResult;

#ifdef __cplusplus
extern "C" {
#endif
	int GetCurrentHWID(char* szID, int size);
	int SetActivation(char* activationKey);
	int CreateEngine(char* szDicPath, const char* szDatabaseFilePath);
	int ReleaseEngine();
	void* EngineMalloc(int nSize);
	void EngineFree(void* ptr);

	//PersonRetrieval
	DetectionResult* FaceDetectionGray(unsigned char* pbImage, int nHeight, int nWidth, int nDetectMinPercent, int* pnFaceCount);
	DetectionResult* FaceDetection(SImg* psImage, int nDetectMinPercent, int* pnFaceCount);
	MFResult* MFExtraction(SImg* psImage, DetectionResult* pxDetectionResult, int nFaceNum);
	MFResult* MFExtractionGray(unsigned char* pbImage, int nHeight, int nWidth, DetectionResult* pxDetectionResult, int nFaceNum);
	MFResult* MFExtractionGrayFromLandmark(unsigned char* pbImage, int nHeight, int nWidth, ModelingPoints* pxModelingPoints, int nFaceNum);
	MFResult* MFExtractionFromLandmark(SImg* psImage, ModelingPoints* pxModelingPoints, int nFaceNum);
	DistNo* Identify(ARM_Feature* pxFeature, PersonDatabase1* pxDatabase, int* pnUpdate, float* prMatchingTH, float* prUpdateMin, float* prUpdateMax);
	DistNo* IdentifySet(Person1* pxPerson, PersonDatabase1* pxDatabase, int* pnUpdate, float* prMatchingTH, float* prUpdateMin, float* prUpdateMax);
	DistNo* IdentifyCPU(char* pbProbFeature, char* pbGalleryFeatures, int nGalleryNum);
	DistNo* IdentifyGPU(char* pbProbFeature);
	void EnrollGPUData(char* pbGalleryFeatures, int nGalleryNum);
	void ReleaseGPUEnrollData();
	void CreateGPUMemoryForConvert(int nMaxH, int nMaxW, int nChannelNo);
	void ReleaseGPUMemoryForConvert(int nChannelNo);

	int ConvertYUV2RGB(unsigned char* data, int width, int height, unsigned char* dstData, int nChannelNo);
	void ConvertYUV2RGBCPU(unsigned char* data, int width, int height, unsigned char* dstData);
#ifdef __cplusplus
} //extern "C"
#endif

#endif //(_FREngine_H_)
