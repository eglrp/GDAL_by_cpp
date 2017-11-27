// hello_GDAL.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include<iostream>
#include "../gdal/gdal_priv.h"
#include "../gdal/gdal_alg.h"
#include "../gdal/gdal_priv.h"   
#include "../gdal/ogrsf_frmts.h"
using namespace std;

int main()
{
	callGDAL();
	callOGR();
	return 0;
}

void callGDAL()
{
	//ע��GDAL
	GDALAllRegister();

	//դ�����ݼ�
	GDALDataset *poDataset;
	char* filePath = "C:\\code\\For_Gdal\\mosaic.tif";

	//��һ��դ��
	poDataset = (GDALDataset *)GDALOpen(filePath, GA_ReadOnly);

	//��GDALDataset�ķ�����������
	char const *dd = poDataset->GetDriver()->GetDescription();
}

void callOGR()
{
	//ע��OGR
	OGRRegisterAll();

	//Driver
	const char *pszDriverName = "ESRI Shapefile";
	OGRSFDriver *poDriver = (OGRSFDriver *)OGRGetDriverByName(pszDriverName);

	//ʸ������Դ
	OGRDataSource *poDatasource = poDriver->CreateDataSource("path", NULL);//��������Դ

	OGRLayer *lyr = poDatasource->GetLayer(0);
}


//�����¶�ͼ http://blog.csdn.net/liminlu0314/article/details/8498985


#pragma region ������ֵ�߷���	GDALContourGenerate(������); http://blog.csdn.net/hong__fang/article/details/51605030

int _tmain(int argc, _TCHAR* argv[])
{
	string imgPath = "F:\\2.tif";
	string shpPath = "C:\\contour.shp";


	CalContourByGdal(imgPath, shpPath, 500);

	cout << "�������" << endl;
	getchar();

	return 0;
}

bool CalContourByGdal(string imgPath, string shpPath, double dfContourInterval)
{
	OGRRegisterAll();
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	GDALDataset * pInDataset = (GDALDataset *)GDALOpen(imgPath.c_str(), GA_ReadOnly);
	if (pInDataset == NULL)
	{
		printf("��ȡͼ��ʧ�ܣ�");
		return FALSE;
	}

	int nDemWidth = pInDataset->GetRasterXSize(); //��ȡͼ���  
	int nDemHeight = pInDataset->GetRasterYSize(); //��ȡͼ���  
	int Count = pInDataset->GetRasterCount(); //������  


											  //��ȡͼ�����ݲ���
	GDALRasterBand *pInRasterBand = pInDataset->GetRasterBand(1);
	float *pData = new float[nDemWidth*nDemHeight]();

	CPLErr err = pInRasterBand->RasterIO(GF_Read, 0, 0, nDemWidth, nDemHeight, pData, nDemWidth, nDemHeight, GDT_Float32, 0, 0);
	if (err == CE_Failure)
	{
		cout << "��ȡDEMͼ������ʱ������" << endl;
		GDALDestroyDriverManager();
		return 0;
	}

	//�ж�ͼ�����Ƿ����쳣ֵ������ȡ�쳣ֵʵ��ֵ
	float fNoData = 0;
	int nIdx;
	for (int i = 0; i < nDemHeight; i++)
	{
		for (int j = 0; j < nDemWidth; j++)
		{
			nIdx = i*nDemWidth + j;
			if (pData[nIdx] <= -9999)
			{
				fNoData = pData[nIdx];
			}
		}
	}


	//����ʸ��ͼ
	const char *pszDriverName = "ESRI Shapefile";
	OGRSFDriver *poDriver;
	GDALAllRegister();
	poDriver = (OGRSFDriver *)OGRGetDriverByName(pszDriverName);
	if (poDriver == NULL)
	{
		printf("%s driver not available.\n", pszDriverName);
		exit(1);
	}
	OGRDataSource *poDS;
	poDS = poDriver->CreateDataSource(shpPath.c_str(), NULL);//��������Դ
	if (poDS == NULL)
	{
		printf("Creation of output file failed.\n");
		exit(1);
	}

	OGRLayer *poLayer;
	OGRSpatialReference *poSpatialRef = new OGRSpatialReference(pInDataset->GetProjectionRef());
	poLayer = poDS->CreateLayer("Elevation", poSpatialRef, wkbLineString, NULL); //����ͼ��
	if (poLayer == NULL)
	{
		printf("Layer creation failed.\n");
		exit(1);
	}


	OGRFieldDefn ofieldDef1("Elevation", OFTInteger);    //��ʸ��ͼ�д����߳�ֵ�ֶ�
	if (poLayer->CreateField(&ofieldDef1) != OGRERR_NONE)
	{
		cout << "����ʸ��ͼ�����Ա�ʧ�ܣ�" << endl;
		GDALClose((GDALDatasetH)poDS);
		GDALClose(pInDataset);
		return 0;
	}

	//����ͼ�񲨶�����ʸ��ͼ�ȸ���
	if (fNoData == 0)
		GDALContourGenerate(pInRasterBand, dfContourInterval, 0, 0, NULL, false, 0, (OGRLayerH)poLayer, -1, 0, NULL, NULL);
	else //���쳣ֵʱ�������쳣ֵ���д���
		GDALContourGenerate(pInRasterBand, dfContourInterval, 0, 0, NULL, true, fNoData, (OGRLayerH)poLayer, -1, 0, NULL, NULL);

	GDALClose(poDS);
	GDALClose(pInDataset);


	if (pData != NULL)
	{
		delete[] pData;
		pData = NULL;
	}

}
#pragma endregion