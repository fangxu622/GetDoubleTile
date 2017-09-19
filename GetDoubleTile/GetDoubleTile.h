#include <iostream>
#include <string>
#include <vector>
#include <io.h>
#include <sstream>
/*************************************GDAL头文件**********************************/
#include "gdal.h"
#include "gdal_priv.h"
#include "gdal_alg.h"
#include "ogr_srs_api.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "ogrsf_frmts.h"

#include "ogr_spatialref.h"
/*************************************GDAL头文件**********************************/
using namespace std;

struct ImgPoint
{
	int Colum;
	int Row;
};
struct Coordinate
{
	double Latitude;
	double Longitude;
};
struct  Envolop
{
	double LatMin;//纬度最小
	double LatMax;
	double LonMin;//经度最小
	double LonMax;
};
struct tilePose
{
	ImgPoint startP;//瓦片起始像素位置
	Coordinate startC;//瓦片起始经纬度
	int height;//瓦片高度
	int width;
	bool nodata;//瓦片是否是空值
	tilePose()
	{
		nodata = false;
	}
};

class GetDoubleTile
{
public:
	string preResultdir;
	double adfpreGeoTransform[6];
	string datatype;
	GDALDataType eMeta;
	string SSproj;
	Envolop imgRange;
	GDALDataset *preDatset;
	vector<tilePose> pretile;
	int validHeight;
	int validWidth;
	int XtileNum;
	int YtileNum;
	ImgPoint preStartP;
	int Block;

public:
	GetDoubleTile();
	GetDoubleTile(string _preResultdir, int _block);
	~GetDoubleTile();
	void ReadRaster(string preImg);
	void getEnvolop();
	void setPose();
	void getTile();
	void Projection2ImageRowCol(double *adfGeoTransform, Coordinate coord, ImgPoint &point);
	void ImageRowCol2Projection(double *adfGeoTransform, ImgPoint point, Coordinate &coord);
	void getTileimg(int iterm);
	void getShpLabel(OGRLayer *poLayer, int iterm);
};