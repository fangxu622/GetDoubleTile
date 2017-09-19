// GetDoubleTile.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "GetDoubleTile.h"
#include "GetNumTif.h"
#include "gdalwarper.h"
#include <io.h>
using namespace std;
void getShpLabel(string proj, double *adfGeotransform, int width, int height, OGRLayer *poLayer, int iterm);
void cutShpbyFisher(string fisherShpfile, string cutShp, string pathDir);
void cutShpbyCCcode(string shpDir, int numShp, vector<string> ccthre);
int main(int argc, char** argv)
{
	if (argc<2)
	{
		cout << "输入参数示例：GetDoubleTile.exe E:\\test.tif 2048" << endl;
		return 0;
	}

	//argv[1] = "D:\\VS2017\\14.tif";
	//argv[2] = "256";

	GDALAllRegister();
	OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	CPLSetConfigOption("SHAPE_ENCODING", "");
	string preimgFile = argv[1]/*"C:\\江苏江阴\\2013.tif"*/;
	int patchsize = atoi(argv[2]);
	int cutpos = preimgFile.find_last_of('\\');
	string fisherName(preimgFile.substr(0, cutpos + 1));
	fisherName += "Fisher.shp";
	char* FisherShpfile = const_cast<char*>(fisherName.c_str());/*"C:\\fisher.shp"*/;
	int pos1 = preimgFile.find_last_of('\\');
	int pos2 = preimgFile.find_last_of('.');
	//文件名，不包括后缀
	string	PreName(preimgFile.substr(pos1 + 1, pos2 - pos1 - 1));
	string Pathdir = (preimgFile.substr(0, pos1 + 1));
	string Resultdir = Pathdir + "LCABlock\\";
	string cmd = "md " + Resultdir;
	//system(cmd.c_str());

	string preResultdir = Pathdir + PreName;
	GetDoubleTile *test = new GetDoubleTile(preResultdir, patchsize);//初始化类	
	test->ReadRaster(preimgFile);
	test->getEnvolop();
	test->getTile();//获得切块

	string projs = test->preDatset->GetProjectionRef(); //获得空间参考

	// 得到shp文件的处理器
	OGRSFDriver* poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
	//判断要生成的shp文件是否存在，存在就删掉
	if ((_access(FisherShpfile, 0)) != -1)
	{
		poDriver->DeleteDataSource(FisherShpfile);
	}
	//创建shp文件
	OGRDataSource* poDS = poDriver->CreateDataSource(FisherShpfile, NULL);
	char *proj = const_cast<char*>(projs.c_str());
	OGRSpatialReference other;
	other.importFromWkt(&proj);
	//创建图层
	OGRLayer* poLayer = poDS->CreateLayer("Polygon", &other, wkbMultiPolygon, NULL);
	//创建字段
	//   区域编号
	OGRFieldDefn oFieldLabel("Label", OFTInteger);
	poLayer->CreateField(&oFieldLabel);
	for (int i = 0; i<test->pretile.size(); i++)
	{
		int width = test->pretile[i].width;
		int height = test->pretile[i].height;
		double adfGeotransform[6];
		memcpy(adfGeotransform, test->adfpreGeoTransform, sizeof(double) * 6);
		adfGeotransform[0] = adfGeotransform[0] + (test->pretile[i].startP.Colum)*adfGeotransform[1] + (test->pretile[i].startP.Row)*adfGeotransform[2];
		adfGeotransform[3] = adfGeotransform[3] + (test->pretile[i].startP.Colum)*adfGeotransform[4] + (test->pretile[i].startP.Row)*adfGeotransform[5];
		cout << "处理第" << i << "块" << endl;
		getShpLabel(projs, adfGeotransform, width, height, poLayer, i);
	}
	OGRDataSource::DestroyDataSource(poDS);

	delete test;
	system("pause");
}
void getShpLabel(string proj, double *adfGeotransform, int width, int height, OGRLayer *poLayer, int iterm)
{
	int *labels = new int[width*height];
	for (int i = 0; i<width*height; i++)
	{
		labels[i] = iterm;
	}
	GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName("MEM");
	GDALDataset *preds = poDriver->Create("", width, height, 1, GDT_Int32, NULL);
	preds->SetProjection(proj.c_str());
	preds->SetGeoTransform(adfGeotransform);
	preds->RasterIO(GF_Write, 0, 0, width, height, labels, width, height, GDT_Int32, 1, NULL, 0, 0, 0);
	GDALRasterBandH hdstBand = (GDALRasterBandH)preds->GetRasterBand(1);
	char **papszOptions = NULL;
	papszOptions = CSLSetNameValue(papszOptions, "8CONNECTED", "8");
	GDALPolygonize(hdstBand, NULL, (OGRLayerH)poLayer, 0, papszOptions, NULL, NULL); //调用栅格矢量化
	GDALClose(preds);
	delete[] labels; labels = NULL;
}
void cutShpbyFisher(string fisherShpfile, string cutShp, string pathDir)
{
	OGRDataSource *poDSCut;
	OGRDataSource* poFisher;
	poDSCut = OGRSFDriverRegistrar::Open(cutShp.c_str(), FALSE);//shape文件存放的路径
	poFisher = OGRSFDriverRegistrar::Open(fisherShpfile.c_str(), TRUE);
	OGRLayer  *poLayer;
	int pos1 = cutShp.find_last_of('\\');
	int pos2 = cutShp.find_last_of('.');
	//文件名，不包括后缀
	string	shpkeyname(cutShp.substr(pos1 + 1, pos2 - pos1 - 1));
	char *shpKeyName = const_cast<char*>(shpkeyname.c_str());
	poLayer = poDSCut->GetLayerByName(shpKeyName);

	OGRLayer *fisherLayer = poFisher->GetLayer(0);
	int numFeature = fisherLayer->GetFeatureCount();
	string LayerName = fisherLayer->GetName();

	for (int i = 0; i<numFeature; i++)
	{
		cout << "正在切割第" << i << "块shp..." << endl;
		stringstream ss;
		ss << i;
		string PatchshpName = pathDir + "RegBlock_" + ss.str() + ".shp";
		// 得到shp文件的处理器
		OGRSFDriver* poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
		//判断要生成的shp文件是否存在，存在就删掉
		if ((_access(PatchshpName.c_str(), 0)) != -1)
		{
			poDriver->DeleteDataSource(PatchshpName.c_str());
		}
		//创建shp文件
		OGRDataSource* poDS = poDriver->CreateDataSource(PatchshpName.c_str(), NULL);
		OGRSpatialReference* spatial = fisherLayer->GetSpatialRef();
		//创建图层
		OGRLayer* patchLayer = poDS->CreateLayer("Polygon", spatial, wkbMultiPolygon, NULL);

		string pszQuery = "SELECT * FROM " + LayerName + " WHERE \"Label\"=" + ss.str();
		OGRLayer* sqlLayer = poFisher->ExecuteSQL(pszQuery.c_str(), NULL, NULL);
		OGRErr err = poLayer->Clip(sqlLayer, patchLayer);
		OGRDataSource::DestroyDataSource(poDS);
	}
	OGRDataSource::DestroyDataSource(poDSCut);
	OGRDataSource::DestroyDataSource(poFisher);
}

vector<string> split(string str, string pattern)
{
	string::size_type pos;
	vector<string> result;
	str += pattern;
	int size = str.size();
	for (int i = 0; i<size; i++)
	{
		pos = str.find(pattern, i);
		if (pos<size)
		{
			string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}

	}
	return result;
}
void cutShpbyCCcode(string shpDir, int numShp, vector<string> ccthre)
{
	for (int i = 0; i<numShp; i++)
	{

		//判断要生成的shp文件是否存在，存在就删掉
		stringstream ss;
		ss << i;
		string ccPatchdir = shpDir + ss.str();
		string cmd = "md " + ccPatchdir;
		system(cmd.c_str());//创建一个文件夹，名称和shp名字一样
		string shpFile = shpDir + "RegBlock_" + ss.str() + ".shp";//打开要分别导出地类的shp文件
		OGRDataSource *srcDS = OGRSFDriverRegistrar::Open(shpFile.c_str(), TRUE);
		OGRLayer* srcLayer = srcDS->GetLayer(0);
		for (int ccid = 0; ccid<ccthre.size(); ccid++)
		{
			cout << "处理第" << i << "块" << "第" << ccid << "类" << endl;
			stringstream s1;
			s1 << ccid;
			string ccClassShp = ccPatchdir + "\\" + s1.str() + "_floor.shp";
			OGRSFDriver* poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
			if ((_access(ccClassShp.c_str(), 0)) != -1)
			{
				poDriver->DeleteDataSource(ccClassShp.c_str());
			}
			OGRDataSource* poDSIgnore = poDriver->CreateDataSource(ccClassShp.c_str(), NULL);
			string ccthreshold = ccthre[ccid];
			vector<string> ccVector;
			ccVector = split(ccthreshold, " ");
			string layerName = srcLayer->GetName();
			string sql = "SELECT * FROM " + (string)layerName + " WHERE ";
			if (ccVector.size() == 1)
			{
				sql += "(CC=\'" + ccVector[0] + "\')";
			}
			if (ccVector.size() == 2)
			{
				sql += "(CC>=\'" + ccVector[0] + "\') AND " + "(CC<=\'" + ccVector[ccVector.size() - 1] + "\')";
			}
			char* sqlSelect = const_cast<char*>(sql.c_str());
			OGRLayer* tempLayer = srcDS->ExecuteSQL(sqlSelect, NULL, NULL);
			if (tempLayer == NULL)
				continue;
			OGRLayer *poLayer = poDSIgnore->CopyLayer(tempLayer, "Polygon");
			poLayer->SyncToDisk();
			srcDS->ReleaseResultSet(tempLayer);
			OGRDataSource::DestroyDataSource(poDSIgnore);
		}
		OGRDataSource::DestroyDataSource(srcDS);
	}

}