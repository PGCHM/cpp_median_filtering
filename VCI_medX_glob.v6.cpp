/***************************/
/****median filter**********/
/****needs to set Ny, Nx****/
/****Chunming Peng 03/2012**/
/***************************/

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <sys/time.h>
#include <vector>
#include <algorithm>

#include "gdal_priv.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
#include "ogr_api.h"

#define FORWARD     1
#define BACKWARD  (-1)

using namespace std;

unsigned char get_value(unsigned char *image, int ROWNUM, int COLNUM, int row, int col)
{
        if(row >= 0 && col >= 0 && row < ROWNUM && col < COLNUM)
                return *(image+row*COLNUM+col);
        return 0;
}

// 2d median filter implementation based on median definition
void median2d_direct(unsigned char *dst, unsigned char *src, int ROWNUM, int COLNUM, int Ny, int Nx)
{
	std::vector<int> v;
	int middle=((Ny*2+1)*(Nx*2+1)+1)/2;
	for(int row=0; row<ROWNUM; row++)
	{
		for(int col=0; col<COLNUM; col++)
		{
			v.clear();
			for(int r = row-Ny; r <= row+Ny; r++)
			{
				for(int c = col-Nx; c <= col+Nx; c++)
				{
					v.push_back(get_value(src, ROWNUM, COLNUM, r, c));
				}
			}
			std::sort(v.begin(), v.end());
			int med =v[middle-1];
			*(dst+row*COLNUM+col) = med;
		}
	}
}

void median2d(unsigned char *dst, unsigned char *src, int ROWNUM, int COLNUM, int Ny, int Nx)
{
	int med, delta_l;
	int m, n;
	int prev, next;
	int h[256];
	int direction = FORWARD;
	int row1, row2, col1, col2;
	int row, col, r, c;

	int middle=((Ny*2+1)*(Nx*2+1)+1)/2;


	memset(h, 0, sizeof(h));

	// Histogram For (0,0)-element
	for(row = -Ny; row <= Ny; row++)
	{
		for(col = -Nx; col <= Nx; col++)
			h[get_value(src, ROWNUM, COLNUM, row, col)]++;
	}
	// Median
	for(m=0,n=0; n < 256; n++)
	{
		m += h[n];
		if(m>=middle)
			break;
	}
	med=n;
	delta_l = m - h[n];


	// Now, Median Is Defined For (0,0)-element
	// Begin Scanning: direction - FORWARD
	*dst=med;

	// main loop
	for(col=1, row=0; row<ROWNUM; row++)
	{
		// Prepare to Horizontal Scanning
		row1=row-Ny;
		row2=row+Ny;

		for(; col>=0 && col<COLNUM; col += direction)
		{
			// Determine Previous and Next Columns
			// Pay Attention To Defined Direction !!!
			prev = col-direction*(Nx+1);
			next = col+direction*Nx;

			// Now Change Old Histogram
			// New Histogram
			// delete previous

			for(r=row1; r<=row2; r++)
			{
				int value_out = get_value(src, ROWNUM, COLNUM, r, prev);
				int value_in = get_value(src, ROWNUM, COLNUM, r, next);
				if(value_out == value_in)
					continue;
				h[value_out]--;
				if(value_out < med)
					delta_l--;
				h[value_in]++;
				if(value_in < med)
					delta_l++;
			}

			// Update new median
			if(delta_l >= middle)
			{
				while(delta_l>=middle)
				{
					if(h[--med] > 0)
						delta_l -= h[med];
				}
			}
			else
			{
				while(delta_l + h[med] < middle)
				{
					if(h[med] > 0)
						delta_l += h[med];
					med++;
				}
			}
			*(dst+row*COLNUM+col) = med;
		} // end of column loop


		// go back to the last/first pixel of the line
		col -= direction;
		// change direction to the opposite
		direction *= -1;


		// Shift Down One Line
		prev = row1;
		next = row2+1;


		col1 = col - Nx;
		col2 = col + Nx;

		for(c=col1; c<=col2; c++)
		{
			int value_out = get_value(src, ROWNUM, COLNUM, prev, c);
			int value_in = get_value(src, ROWNUM, COLNUM, next, c);
			if(value_out == value_in)
				continue;
			h[value_out]--;
			if(value_out<med)
				delta_l--;
			h[value_in]++;
			if(value_in<med)
				delta_l++;
		}

		if(delta_l>=middle)
		{
			while(delta_l>=middle)
			{
				if(h[--med]>0)
					delta_l -= h[med];
			}
		}
		else
		{
			while(delta_l + h[med] < middle)
			{
				if(h[med]>0)
					delta_l += h[med];
				med++;
			}
		}
		*(dst+(row+1)*COLNUM+col) = med;
		col += direction;
	}
}

int main( int argc, char ** argv )
{
       const char * strFilePath; 
       const char * strFilePath2;

       int nBlocks = 50;
       GDALDataset *poDataset;
       GDALDataset *poDataset2;
       GDALDriver *poDriver;   
       GDALAllRegister();

       argc = GDALGeneralCmdLineProcessor( argc, &argv, 0 );
       if( argc < 4 )
       {
           cerr << "Please orderly input VCI, and OUTPUT file names, and kernel size (e.g. 3 for 3 *3 filtering)." << endl ;
           return -1;
       }

       strFilePath = argv[1]; 
       strFilePath2 = argv[2]; 
       int kernelSize = atoi(argv[3]);

       if( (kernelSize<3)||(kernelSize>13)){
		cerr << "Kernel Size incorrect. choose from 3 to 13. " << endl;
		return -1;
       }

       poDataset = (GDALDataset *) GDALOpen(strFilePath, GA_ReadOnly );
     
       //Open file
       if( poDataset == NULL )
       {
                cerr << "Can not open file " << strFilePath << endl ;
                return -1;
       }


       //Create a new tiff file
       int nBandCount=poDataset->GetRasterCount();
       int nImgSizeX=poDataset->GetRasterXSize();
       int nImgSizeY=poDataset->GetRasterYSize();
       const char *pszFormat = "GTiff";
       poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
       if( poDriver == NULL )
       {
            cerr << "Can not get " << pszFormat << " format driver "<< endl ;
            return -1;
       }

       char **papszMetadata = poDriver->GetMetadata();

       if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATE, FALSE ) )
       //printf( "Driver %s supports Create() method.\n", pszFormat );
       if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATECOPY, FALSE ) )
       //printf( "Driver %s supports CreateCopy() method.\n", pszFormat );
       poDataset2=poDriver->Create(strFilePath2,nImgSizeX,nImgSizeY,nBandCount,GDT_Byte,papszMetadata);
       cerr << "Complete creation of file " << strFilePath2 << endl ;

       //Information
  	   double adfGeoTransform[6]={0,1,0,0,0,1};
       poDataset->GetGeoTransform(adfGeoTransform);
       poDataset2->SetGeoTransform(adfGeoTransform);

       const char *pszProjection = "";
       pszProjection = poDataset->GetProjectionRef();
       poDataset2->SetProjection(pszProjection);

	   //Processing
 	   int nYOff[nBlocks];
	   int nH = nImgSizeY / nBlocks;
	   int nBlockW[nBlocks];
	   int nBlockH[nBlocks];
	   for (int i=0;i<nBlocks;i++)
	   {
	      nBlockW[i] = nImgSizeX;
	      nBlockH[i] = nH;
	   }

	   nBlockH[nBlocks-1] = nImgSizeY - (nBlocks - 1) * nH;

	   nYOff[0] = 0;
       for (int i=1;i<nBlocks;i++)
	   {
	      nYOff[i] = nBlockH[i-1] + nYOff[i-1];
	   }

	   for (int i=0;i<=nBlocks-1;i++)
	   {
	 long nSize = nBlockW[i]*nBlockH[i]*nBandCount;
         unsigned char *ppafScan = new unsigned char [nSize];    
	 printf("ppafScan at location %p. ", (void*)ppafScan);     
         unsigned char *ppbfScan = new unsigned char [nSize];
	 printf("ppbfScan at location %p. ", (void*)ppbfScan);
	 printf("ppaf and ppbf allocated. ");
          
         if(poDataset->RasterIO(GF_Read,0,nYOff[i],nBlockW[i],nBlockH[i],ppafScan,nBlockW[i],nBlockH[i],GDT_Byte,nBandCount,0,0,0,0)==CE_Failure) {
		printf("X: Read geotiff error.\n");
		cerr<< "Read geotiff error." <<endl;
		return -1;
	 }
	 printf("Read geotiff complete. "); 
	 median2d(ppbfScan, ppafScan, nBlockH[i], nBlockW[i], (kernelSize-1)/2, (kernelSize-1)/2);
	 printf("Median filter complete. ");
         if(poDataset2->RasterIO(GF_Write,0,nYOff[i],nBlockW[i],nBlockH[i],ppbfScan,nBlockW[i],nBlockH[i],GDT_Byte,nBandCount,0,0,0,0)==CE_None) 
	     if(i<nBlocks-2){
	 	printf("Write geotiff complete. ");
		if(ppafScan) { delete[] ppafScan; ppafScan=NULL;}   
		printf("mem ppaf removed. ");  
	 	if(ppbfScan) { delete[] ppbfScan; ppbfScan=NULL;}
		printf("mem ppbf removed. ");
	    }else{}
	else{
		cerr<< "Write geotiff error." <<endl;
                return -1;
        }
	 printf("Bulk Calc: %d/%d finished.\n",i+1,nBlocks);
     }

           for (int i=1;i<nBlocks;i++)
           {
	 printf("___Edge Removal___\n");
         long enSize = nBlockW[i]*(2*kernelSize-1)*nBandCount;
         unsigned char *eppafScan = new unsigned char [enSize];
         unsigned char *eppbfScan = new unsigned char [enSize];
	 printf("eppaf and eppbf allocated.\n");

         if(poDataset->RasterIO(GF_Read,0,nYOff[i]-(kernelSize-1),nBlockW[i],(2*kernelSize-1),eppafScan,nBlockW[i],(2*kernelSize-1),GDT_Byte,nBandCount,0,0,0,0)==CE_Failure) {
		cerr<< "Read geotiff error." <<endl;
                return -1;
        }
         median2d(eppbfScan, eppafScan,2*kernelSize-1, nBlockW[i], (kernelSize-1)/2, (kernelSize-1)/2);
         if(poDataset2->RasterIO(GF_Write,0,nYOff[i]-(kernelSize-1)/2,nBlockW[i],kernelSize,eppbfScan+(kernelSize-1)*nBlockW[i]/2,nBlockW[i],kernelSize,GDT_Byte,nBandCount,0,0,0,0)==CE_None) {
		printf("Write geotiff complete. ");
                //if(eppafScan) { delete[] eppafScan; eppafScan=NULL;}
                //printf("mem ppaf removed. ");
                //if(eppbfScan) { delete[] eppbfScan; eppbfScan=NULL;}
                //printf("mem ppbf removed. ");
	 }else{
                cerr<< "Write geotiff error." <<endl;
                return -1;
        }
         printf("Edge Removal: %d of %d finished.\n",i+1,nBlocks);
     }

       if(poDataset != NULL)
       {
            GDALClose(poDataset);
            poDataset = NULL;
       }
       
 
       if(poDataset2 != NULL)
       {
          GDALClose(poDataset2);
          poDataset2 = NULL;
       }
       
       if(poDriver != NULL)
       {
          GDALDeregisterDriver(poDriver);
          poDriver = NULL;
       }

       return EXIT_SUCCESS;
}
