#include <iostream>
#include <string>
#include <vector>
#include <io.h>
#include <sstream>
/*************************************GDALͷ�ļ�**********************************/
#include "gdal.h"
#include "gdal_priv.h"
#include "gdal_alg.h"
#include "ogr_srs_api.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "ogrsf_frmts.h"

#include "ogr_spatialref.h"
/*************************************GDALͷ�ļ�**********************************/
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
	double LatMin;//γ����С
	double LatMax;
	double LonMin;//������С
	double LonMax;
};
struct tilePose
{
	ImgPoint startP;//��Ƭ��ʼ����λ��
	Coordinate startC;//��Ƭ��ʼ��γ��
	int height;//��Ƭ�߶�
	int width;
	bool nodata;//��Ƭ�Ƿ��ǿ�ֵ
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