#include <stdio.h>
#include "qdbmp.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

//#define DEBUG
// REGARDS TO http://qdbmp.sourceforge.net/

#define PI 3.14159
#define LOGE 2.71828

void RGBtoBW(BMP* bmp);
void invert(BMP* bmp);
BMP* zoom(BMP* bmp, int k);
void reduceLevel(BMP* bmp, int level);
void BritandCntr(BMP* bmp, int brightness, double contrast);
BMP* NaiveGaussianBlur(BMP* bmp, double theta, int radius);
BMP* FastGaussianBlur(BMP* bmp, double theta, int radius);
BMP* naiveRotate(double degrees, BMP* bmp);
BMP* shearRotate(double degrees, BMP* bmp);
BMP* shearRotateShell(double degrees, BMP* bmp);
BMP* SobelEdgeDetection(BMP* bmp, int thrhld, int coefficient);
BMP* AMRotation(BMP* bmp, double theta);
BMP* CannyEdgeDetection(BMP* bmp, int thrhld, int coefficient, double highShr, double lowShr, int radius);
BMP* DoubleThreshold(BMP* bmp, double high, double low, int radius);
int* EgdeConnection(int* img, int width, int height, int radius);
int* IsConnected(int* img, int width, int height, int x, int y, int radius);
BMP* DINTRotation(BMP* bmp, double theta);

int main()
{
	BMP*    bmp;
	char filename[] = "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\test.bmp";
	bmp = BMP_ReadFile(filename);
	BMP_CHECK_ERROR(stderr, -1);
	/////////////////////////////////////////////////////////////////////////
	//Your code in between
	char filepath[100] = "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\INTR\\rotate.bmp";
	BMP* newbmp = DINTRotation(bmp, 30);
	BMP_WriteFile(newbmp, filepath);
	BMP_Free(newbmp);
	/////////////////////////////////////////////////////////////////////////
	/* Save result */
	/*for (double i = 0.1; i <= 0.9; i+=0.1)
	{
		for (double j = i; j <= 0.9; j+=0.1)
		{
			char filepath[100]= "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\CannyC\\";
			int len = strlen(filepath);
			filepath[len] = 'C';
			filepath[len + 1] = 'H';
			itoa((int)(10 * j), filepath + len + 2, 10);
			len = strlen(filepath);
			filepath[len] = 'L';
			itoa((int)(10 * i), filepath + len + 1, 10);
			len = strlen(filepath);
			filepath[len] = '.';
			filepath[len + 1] = 'b';
			filepath[len + 2] = 'm';
			filepath[len + 3] = 'p';
			filepath[len + 4] = 0;
			BMP* newbmp = CannyEdgeDetection(bmp, 50, 2, j, i);
			BMP_WriteFile(newbmp, filepath);
			BMP_Free(newbmp);
		}
	}*/
	/* Free all memory allocated for the image */
	BMP_Free(bmp);

	//BMP_Free(output);
	return 0;
}

void RGBtoBW(BMP* bmp)
{
	UCHAR   r, g, b;
	UINT    width, height;
	UINT    x, y;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	for (x = 0; x < width; x++)
	{
		for (y = 0; y < height; y++)
		{
			BMP_GetPixelRGB(bmp, x, y, &r, &g, &b);
			BMP_SetPixelRGB(bmp, x, y, 0.3*r + 0.59*g + 0.11*b, 0.3*r + 0.59*g + 0.11*b, 0.3*r + 0.59*g + 0.11*b);
		}
	}
}

void invert(BMP* bmp)
{
	UCHAR   r, g, b;
	UINT    width, height;
	UINT    x, y;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	for (x = 0; x < width; x++)
	{
		for (y = 0; y < height; y++)
		{
			BMP_GetPixelRGB(bmp, x, y, &r, &g, &b);
			BMP_SetPixelRGB(bmp, x, y, 255 - r, 255 - g, 255 - b);
		}
	}
}

BMP* zoom(BMP* bmp, int k)
{
	UINT    width, height;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	BMP* newim = BMP_Create(width*k, height*k, BMP_GetDepth(bmp));
	for (UINT iX = 0; iX < width; iX++)
	{
		for (UINT iY = 0; iY < height; iY++)
		{
			UCHAR difr, difg, difb;
			UCHAR tmpr, tmpg, tmpb;
			BMP_GetPixelRGB(bmp, iX, iY, &tmpr, &tmpg, &tmpb);
			BMP_GetPixelRGB(bmp, iX, iY + 1, &difr, &difg, &difb);
			difr = -(tmpr - difr) / k;
			difg = -(tmpg - difg) / k;
			difb = -(tmpb - difb) / k;
			for (int i = 0; i < k; i++)
			{
				BMP_SetPixelRGB(newim, iX*k, iY*k + i, tmpr + difr*i, tmpg + difg*i, tmpb + difb*i);
			}
		}
	}
	for (UINT iY = 0; iY < height*k; iY++)
	{
		for (UINT iX = 0; iX < width; iX++)
		{
			UCHAR difr, difg, difb;
			UCHAR tmpr, tmpg, tmpb;
			BMP_GetPixelRGB(newim, iX*k, iY, &tmpr, &tmpg, &tmpb);
			BMP_GetPixelRGB(newim, iX*k + k, iY, &difr, &difg, &difb);
			difr = -(tmpr - difr) / k;
			difg = -(tmpg - difg) / k;
			difb = -(tmpb - difb) / k;
			for (int i = 0; i < k; i++)
			{
				BMP_SetPixelRGB(newim, iX*k + i, iY, tmpr + difr*i, tmpg + difg*i, tmpb + difb*i);
			}
		}
	}
	return newim;
}

void reduceLevel(BMP* bmp, int level)
{
	UINT    width, height;
	USHORT depth;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	depth = BMP_GetDepth(bmp);
	for (UINT i = 0; i < width; i++)
	{
		for (UINT j = 0; j < height; j++)
		{
			UCHAR r, g, b;
			BMP_GetPixelRGB(bmp, i, j, &r, &g, &b);
			BMP_SetPixelRGB(bmp, i, j, (UCHAR)(r / (255 / level) / (1.0*level - 1) * 255), (UCHAR)(g / (255 / level) / (1.0*level - 1) * 255), (UCHAR)(b / (255 / level) / (1.0*level - 1) * 255));
			BMP_CHECK_ERROR(stdout, -1);
		}
	}
}

void BritandCntr(BMP* bmp, int brightness, double contrast) //Brightness: amount of RGB value you want to add
//Contrast: | 1 means original | <1 less contrast | >1 more contrast
{
	UINT    width, height;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	for (UINT i = 0; i < width; i++)
	{
		for (UINT j = 0; j < height; j++)
		{
			UCHAR Ur, Ug, Ub;
			BMP_GetPixelRGB(bmp, i, j, &Ur, &Ug, &Ub);
			int r, g, b;
			r = (int)(contrast*(Ur - 128)) + 128 + brightness;
			g = (int)(contrast*(Ug - 128)) + 128 + brightness;
			b = (int)(contrast*(Ub - 128)) + 128 + brightness;
			if (r < 0) r = 0;
			if (g < 0) g = 0;
			if (b < 0) b = 0;
			if (r > 255) r = 255;
			if (g > 255) g = 255;
			if (b > 255) b = 255;
			BMP_SetPixelRGB(bmp, i, j, (UCHAR)r, (UCHAR)g, (UCHAR)b);
			BMP_CHECK_ERROR(stdout, -1);
		}
	}
}

double GaussFunction(double theta, int dimension, double* gaussmatrix)
{
	double sum = 0;
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{
			int x = i - (dimension - 1) / 2;
			int y = j - (dimension - 1) / 2;
			double gauss = 1.0 / (2 * PI*theta*theta)*pow(LOGE, 1.0*(-x*x - y*y) / (2 * theta*theta));
			gaussmatrix[i*dimension + j] = gauss;
			sum += gauss;
		}
	}
	return sum;
}

BMP* NaiveGaussianBlur(BMP* bmp, double theta, int radius)
{
	double* gaussmatrix = malloc(sizeof(double)*radius*radius);
	double sum = GaussFunction(theta, radius, gaussmatrix);
#ifdef DEBUG
	for (int i = 0; i < radius; i++)
	{
		for (int j = 0; j < radius; j++)
		{
			printf("%10.4lf", gaussmatrix[i*radius + j]);
		}
		printf("\n");
	}
#endif // DEBUG

	UINT    width, height;
	USHORT depth;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	depth = BMP_GetDepth(bmp);
	BMP* newbmp = BMP_Create(width, height, depth);
	//Go through all pixels
	for (UINT x = 0; x < width; x++)
	{
		for (UINT y = 0; y < height; y++)
		{
			//Fill in colors
			double r = 0, g = 0, b = 0;
			for (int i = 0; i < radius; i++)
			{
				for (int j = 0; j < radius; j++)
				{
					//Reflect boundary
					int xi = x, yi = y;
					if (xi - (radius - 1) / 2 + i < 0)
					{
						xi = -(xi - (radius - 1) / 2 + i);
					}
					else
					{
						xi = (xi - (radius - 1) / 2 + i);
					}
					if (yi - (radius - 1) / 2 + j < 0)
					{
						yi = -(yi - (radius - 1) / 2 + j);
					}
					else
					{
						yi = (yi - (radius - 1) / 2 + j);
					}
					if (xi >= width)
					{
						xi = width - (xi - width) - 1;
					}
					if (yi >= height)
					{
						yi = height - (yi - height) - 1;
					}
					UCHAR R, G, B;
					BMP_GetPixelRGB(bmp, xi, yi, &R, &G, &B);
					r += R*gaussmatrix[i*radius + j];
					g += G*gaussmatrix[i*radius + j];
					b += B*gaussmatrix[i*radius + j];
				}
			}

			//Calculate total
			r /= sum;
			g /= sum;
			b /= sum;
			BMP_SetPixelRGB(newbmp, x, y, (UCHAR)r, (UCHAR)g, (UCHAR)b);
		}
	}
	free(gaussmatrix);
	return newbmp;
}

BMP* FastGaussianBlur(BMP* bmp, double theta, int radius)
{
	double* gaussmatrix = malloc(sizeof(double)*radius*radius);
	double sum = GaussFunction(theta, radius, gaussmatrix);
	sum = 0;
	for (int i = 0; i < radius; i++)
	{
		sum += gaussmatrix[i + radius*(radius / 2)];
	}
#ifdef DEBUG
	for (int i = 0; i < radius; i++)
	{
		for (int j = 0; j < radius; j++)
		{
			printf("%10.4lf", gaussmatrix[i*radius + j]);
		}
		printf("\n");
	}
#endif // DEBUG

	UINT    width, height;
	USHORT depth;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	depth = BMP_GetDepth(bmp);
	BMP* newbmp = BMP_Create(width, height, depth);
	BMP* newbmp1 = BMP_Create(width, height, depth);
	for (UINT x = 0; x < width; x++)
	{
		for (UINT y = 0; y < height; y++)
		{
			//Fill in colors
			double r = 0, g = 0, b = 0;
			for (int i = 0; i < radius; i++)
			{
				//Reflect boundary
				int xi = x;
				if (xi - (radius - 1) / 2 + i < 0)
				{
					xi = -(xi - (radius - 1) / 2 + i);
				}
				else
				{
					xi = (xi - (radius - 1) / 2 + i);
				}
				if (xi >= width)
				{
					xi = width - (xi - width) - 1;
				}
				UCHAR R, G, B;
				BMP_GetPixelRGB(bmp, xi, y, &R, &G, &B);
				r += R*gaussmatrix[i + radius*(radius / 2)];
				g += G*gaussmatrix[i + radius*(radius / 2)];
				b += B*gaussmatrix[i + radius*(radius / 2)];
			}
			//Calculate total
			r /= sum;
			g /= sum;
			b /= sum;
			BMP_SetPixelRGB(newbmp, x, y, (UCHAR)r, (UCHAR)g, (UCHAR)b);
		}
	}

	for (UINT x = 0; x < width; x++)
	{
		for (UINT y = 0; y < height; y++)
		{
			//Fill in colors
			double r = 0, g = 0, b = 0;
			for (int i = 0; i < radius; i++)
			{
				//Reflect boundary
				int yi = y;
				if (yi - (radius - 1) / 2 + i < 0)
				{
					yi = -(yi - (radius - 1) / 2 + i);
				}
				else
				{
					yi = (yi - (radius - 1) / 2 + i);
				}
				if (yi >= height)
				{
					yi = height - (yi - height) - 1;
				}
				UCHAR R, G, B;
				BMP_GetPixelRGB(newbmp, x, yi, &R, &G, &B);
				r += R*gaussmatrix[i + radius*(radius / 2)];
				g += G*gaussmatrix[i + radius*(radius / 2)];
				b += B*gaussmatrix[i + radius*(radius / 2)];
			}
			//Calculate total
			r /= sum;
			g /= sum;
			b /= sum;
			BMP_SetPixelRGB(newbmp1, x, y, (UCHAR)r, (UCHAR)g, (UCHAR)b);
		}
	}
	free(gaussmatrix);
#ifdef DEBUG
	BMP_WriteFile(newbmp, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Output\\Test.bmp");
#endif // DEBUG

	BMP_Free(newbmp);
	return newbmp1;
}

BMP* naiveRotate(double degrees, BMP* bmp)
{
	UINT width, height;
	USHORT depth;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	depth = BMP_GetDepth(bmp);
	UINT lenth = (UINT)(sqrt(width*width + height*height) + 0.5);
	BMP* newim = BMP_Create(lenth, lenth, depth);
	UINT origin = lenth / 2;
	UINT picX = origin - width / 2;
	UINT picY = origin - height / 2;
	UINT cntX = (UINT)(1.0*width / 2 * cos(degrees) + 1.0*height / 2 * sin(degrees));
	UINT cntY = (UINT)(-1.0*width / 2 * sin(degrees) + 1.0*height / 2 * cos(degrees));
	long int disX = origin - cntX;
	long int disY = origin - cntY;
	for (UINT i = 0; i < width; i++)
	{
		for (UINT j = 0; j < height; j++)
		{
			UCHAR r, g, b;
			BMP_GetPixelRGB(bmp, i, j, &r, &g, &b);
			BMP_SetPixelRGB(newim, disX + i*cos(degrees) + j*sin(degrees), disY - i*sin(degrees) + j*cos(degrees), r, g, b);
		}
	}
	return newim;
}

BMP* transpose(BMP* bmp)
{
	UINT width, height;
	USHORT depth;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	depth = BMP_GetDepth(bmp);
	BMP* ans = BMP_Create(height, width, depth);
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			UCHAR r, g, b;
			BMP_GetPixelRGB(bmp, i, j, &r, &g, &b);
			BMP_SetPixelRGB(ans, j, width - i, r, g, b);
		}
	}
	return ans;
}

BMP* r180(BMP* bmp)
{
	UINT width, height;
	USHORT depth;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	depth = BMP_GetDepth(bmp);
	BMP* ans = BMP_Create(width, height, depth);
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			UCHAR r, g, b;
			BMP_GetPixelRGB(bmp, i, j, &r, &g, &b);
			BMP_SetPixelRGB(ans, width - i - 1, height - j - 1, r, g, b);
		}
	}
	return ans;
}

BMP* shearRotate(double degrees, BMP* bmp)
{
	UINT width, height;
	USHORT depth;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	depth = BMP_GetDepth(bmp);
	int outWidth = (int)(tan(degrees / 2)*(height)) + width;
	int outHeight = (int)(sin(degrees)*(width)) + height;
	BMP* newim = BMP_Create(outWidth, outHeight, depth);
	UINT originX = outWidth / 2;
	UINT originY = outHeight / 2;
	UINT picX = originX - width / 2;
	UINT picY = originY - height / 2;
	//perform first X shift
	int disX1 = (int)(-tan(degrees / 2)*(height)) / 2;
	//initialize flag to 0
	int* flag = malloc(sizeof(int)*outHeight*outWidth);
	memset(flag, 0, sizeof(int)*outHeight*outWidth);
	for (UINT i = 0; i < height; i++)
	{
		double prshX = (-tan(degrees / 2));
		int shX = (int)(prshX*i);
		for (UINT j = 0; j < width; j++)
		{
			UCHAR r, g, b;
			BMP_GetPixelRGB(bmp, j, i, &r, &g, &b);
			BMP_SetPixelRGB(newim, (UINT)(picX + j - shX + disX1), (UINT)(picY + i), r, g, b);
			flag[(picX + j - shX + disX1) + (picY + i)*outWidth] = 1;
		}
	}
#ifdef DEBUG
	BMP* debug0 = BMP_Create(outWidth, outHeight, depth);
	for (int i = 0; i < outWidth; i++)
	{
		for (int j = 0; j < outHeight; j++)
		{
			if (flag[i + outWidth*j] == 1) BMP_SetPixelRGB(debug0, (UINT)(i), (UINT)(j), 100, 100, 100);
		}
	}
	BMP_WriteFile(debug0, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Output\\debug0.bmp");
	char filepath1[100] = "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Output\\rotate1.bmp";
	BMP_WriteFile(newim, filepath1);
	BMP_Free(debug0);
#endif // DEBUG
	//perform Y shift
	int disY = (int)(sin(degrees)*(outWidth)) / 2;
	for (int i = 0; i < outWidth / 2; i++)
	{
		double preshY = sin(degrees);
		int shY = (int)(preshY*i);
		for (int j = outHeight - 1; j >= 0; j--)
		{
			UCHAR r, g, b;
			BMP_GetPixelRGB(newim, i, j, &r, &g, &b);
			if (flag[j*outWidth + i] == 1)
			{
				BMP_SetPixelRGB(newim, (UINT)(i), (UINT)(j - shY + disY), r, g, b);
				if ((outWidth*(j - shY + disY) + i) >= 0 && (outWidth*(j - shY + disY) + i) < outWidth*outHeight) flag[outWidth*(j - shY + disY) + i] = 2;
			}
		}
	}
	for (int i = outWidth / 2; i < outWidth; i++)
	{
		double preshY = sin(degrees);
		int shY = (int)(preshY*i);
		for (int j = 0; j < outHeight; j++)
		{
			UCHAR r, g, b;
			BMP_GetPixelRGB(newim, i, j, &r, &g, &b);
			if (flag[j*outWidth + i] == 1)
			{
				BMP_SetPixelRGB(newim, (UINT)(i), (UINT)(j - shY + disY), r, g, b);
				if ((outWidth*(j - shY + disY) + i) >= 0 && (outWidth*(j - shY + disY) + i) < outWidth*outHeight) flag[outWidth*(j - shY + disY) + i] = 2;
			}
		}
	}
#ifdef DEBUG
	BMP* debug1 = BMP_Create(outWidth, outHeight, depth);
#endif // DEBUG
	for (int i = 0; i < outWidth; i++)
	{
		for (int j = 0; j < outHeight; j++)
		{
			if (flag[outWidth*j + i] != 2)
			{
				BMP_SetPixelRGB(newim, (UINT)(i), (UINT)(j), 0, 0, 0);
			}
#ifdef DEBUG
			if (flag[outWidth*j + i] == 2)
			{
				BMP_SetPixelRGB(debug1, (UINT)i, (UINT)j, (UCHAR)100, (UCHAR)100, (UCHAR)100);
			}
#endif // DEBUG
		}
	}
#ifdef DEBUG
	BMP_WriteFile(debug1, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Output\\debug1.bmp");
	BMP_Free(debug1);
	char filepath2[100] = "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Output\\rotate2.bmp";
	BMP_WriteFile(newim, filepath2);
#endif // DEBUG
	//perform second X shift
	int disX2 = (int)(-tan(degrees / 2)*(outHeight / 2));
	for (int i = 0; i < outHeight / 2; i++)
	{
		double prshX = (-tan(degrees / 2));
		int shX = (int)(prshX*i);
		for (int j = 0; j < outWidth; j++)
		{
			UCHAR r, g, b;
			BMP_GetPixelRGB(newim, j, i, &r, &g, &b);
			if (flag[i*outWidth + j] == 2)
			{
				BMP_SetPixelRGB(newim, (UINT)(j - shX + disX2), (UINT)(i), r, g, b);
				if ((outWidth*i + (j - shX + disX2)) >= 0 && (outWidth*i + (j - shX + disX2)) < outWidth*outHeight) flag[(outWidth*i + (j - shX + disX2))] = 3;
			}
		}
	}
	for (int i = outHeight / 2; i < outHeight; i++)
	{
		double prshX = (-tan(degrees / 2));
		int shX = (int)(prshX*i);
		for (int j = outWidth - 1; j >= 0; j--)
		{
			UCHAR r, g, b;
			BMP_GetPixelRGB(newim, j, i, &r, &g, &b);
			if (flag[i*outWidth + j] == 2)
			{
				BMP_SetPixelRGB(newim, (UINT)(j - shX + disX2), (UINT)(i), r, g, b);
				if ((outWidth*i + (j - shX + disX2)) >= 0 && (outWidth*i + (j - shX + disX2)) < outWidth*outHeight) flag[(outWidth*i + (j - shX + disX2))] = 3;
			}
		}
	}
	for (int i = 0; i < outWidth; i++)
	{
		for (int j = 0; j < outHeight; j++)
		{
			if (flag[outWidth*j + i] != 3) BMP_SetPixelRGB(newim, (UINT)(i), (UINT)(j), 0, 0, 0);
		}
	}
#ifdef DEBUG
	BMP* debug2 = BMP_Create(outWidth, outHeight, depth);
	for (int i = 0; i < outWidth; i++)
	{
		for (int j = 0; j < outHeight; j++)
		{
			if (flag[i + outWidth*j] == 3) BMP_SetPixelRGB(debug2, (UINT)(i), (UINT)(j), 100, 100, 100);
		}
	}
	BMP_WriteFile(debug2, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Output\\debug2.bmp");
	BMP_Free(debug2);
	char filepath3[100] = "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Output\\rotate3.bmp";
	BMP_WriteFile(newim, filepath3);
#endif // DEBUG
	free(flag);
	return newim;
}

BMP* shearRotateShell(double degrees, BMP* bmp)
{
	BMP* tmp = bmp;
	//if rotates a large degree, decomposite it into several small rotations
	if (degrees > 30 * 0.0174532925)
	{
		if (degrees <= 90 * 0.0174532925)
		{
			int times = (int)(degrees / (30 * 0.0174532925));
			double left = degrees - (30 * 0.0174532925) * times;
			for (int i = 0; i < times; i++)
			{
				tmp = shearRotate(0.0174532925 * 30, tmp);
				UINT width, height;
				USHORT depth;
				width = BMP_GetWidth(tmp);
				height = BMP_GetHeight(tmp);
				depth = BMP_GetDepth(tmp);
				int minX = width, maxX = -1, minY = height, maxY = -1;
				for (int i = 0; i < width; i++)
				{
					for (int j = 0; j < height; j++)
					{
						UCHAR r, g, b;
						BMP_GetPixelRGB(tmp, i, j, &r, &g, &b);
						if (r != 0 || g != 0 || b != 0)
						{
							if (i < minX) minX = i;
							if (i > maxX) maxX = i;
							if (j < minY) minY = j;
							if (j > maxY) maxY = j;
						}
					}
				}
				BMP* ntmp = BMP_Create(maxX - minX + 1, maxY - minY + 1, depth);
				for (int i = minX; i < maxX + 1; i++)
				{
					for (int j = minY; j < maxY + 1; j++)
					{
						UCHAR r, g, b;
						BMP_GetPixelRGB(tmp, i, j, &r, &g, &b);
						BMP_SetPixelRGB(ntmp, i - minX, j - minY, r, g, b);
					}
				}
#ifdef DEBUG
				BMP_WriteFile(ntmp, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Output\\ntmp.bmp");
#endif // DEBUG
				BMP_Free(tmp);
				tmp = ntmp;
			}
			tmp = shearRotate(left, tmp);
			UINT width, height;
			USHORT depth;
			width = BMP_GetWidth(tmp);
			height = BMP_GetHeight(tmp);
			depth = BMP_GetDepth(tmp);
			int minX = width, maxX = -1, minY = height, maxY = -1;
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					UCHAR r, g, b;
					BMP_GetPixelRGB(tmp, i, j, &r, &g, &b);
					if (r != 0 || g != 0 || b != 0)
					{
						if (i < minX) minX = i;
						if (i > maxX) maxX = i;
						if (j < minY) minY = j;
						if (j > maxY) maxY = j;
					}
				}
			}
			BMP* ntmp = BMP_Create(maxX - minX + 1, maxY - minY + 1, depth);
			for (int i = minX; i < maxX + 1; i++)
			{
				for (int j = minX; j < maxY + 1; j++)
				{
					UCHAR r, g, b;
					BMP_GetPixelRGB(tmp, i, j, &r, &g, &b);
					BMP_SetPixelRGB(ntmp, i - minX, j - minY, r, g, b);
				}
			}
			BMP_Free(tmp);
			tmp = ntmp;
		}
		else
		{
			if (degrees == 90 * 0.0174532925)
			{
				return transpose(tmp);
			}
			else
			{
				if (degrees < 180 * 0.0174532925)
				{
					tmp = transpose(tmp);
					tmp = shearRotateShell(degrees - 90 * 0.0174532925, tmp);
				}
				else
				{
					if (degrees == 180 * 0.0174532925)
					{
						return r180(tmp);
					}
					else
					{
						if (degrees < 270 * 0.0174532925)
						{
							tmp = r180(tmp);
							tmp = shearRotateShell(degrees - 180 * 0.0174532925, tmp);
						}
						else
						{
							if (degrees == 270 * 0.0174532925)
							{
								tmp = r180(tmp);
								tmp = transpose(tmp);
							}
							else
							{
								tmp = r180(tmp);
								tmp = transpose(tmp);
								tmp = shearRotateShell(degrees - 270 * 0.0174532925, tmp);
							}
						}
					}
				}
			}
		}

	}
	else
	{
		tmp = shearRotate(degrees, bmp);
	}
	return tmp;
}

BMP* SobelEdgeDetection(BMP* bmp, int thrhld, int coefficient)
{
	int SobelY[3][3] = { {-1,0,1},{-2,0,2},{-1,0,1} };
	int SobelX[3][3] = { {-1,-2,-1},{0,0,0},{1,2,1} };
	//Convert to Greyscale
	RGBtoBW(bmp);
	BMP_WriteFile(bmp, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\BW.bmp");
	//Gaussian blur
	BMP* tmpbmp = FastGaussianBlur(bmp, 1, 3);
	BMP* swap = bmp;
	bmp = tmpbmp;
	tmpbmp = swap;
	BMP_WriteFile(bmp, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\Blur.bmp");
	//BMP_Free(tmpbmp);
	//Get basic information
	UINT width, height;
	USHORT depth;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	depth = BMP_GetDepth(bmp);
	int* EdgeX = malloc(sizeof(int)*height*width);
	int* EdgeY = malloc(sizeof(int)*height*width);
	memset(EdgeX, 0, sizeof(int)*height*width);
	memset(EdgeY, 0, sizeof(int)*height*width);
	//Convolve with Sobel matrix
	for (int i = 1; i < width - 1; i++)
	{
		for (int j = 1; j < height - 1; j++)
		{
			int sumX = 0, sumY = 0;
			//Calculate gradient
			for (int iX = 0; iX < 3; iX++)
			{
				for (int iY = 0; iY < 3; iY++)
				{
					UCHAR r = 0, g = 0, b = 0;
					BMP_GetPixelRGB(bmp, i + iX - 1, j + iY - 1, &r, &g, &b);
					sumX += r*SobelX[iX][iY];
					sumY += r*SobelY[iX][iY];
				}
			}
			if (abs(sumX) > thrhld && abs(sumX) < 1020) EdgeX[i + j*width] = sumX;
			if (abs(sumY) > thrhld && abs(sumY) < 1020) EdgeY[i + j*width] = sumY;
		}
	}
	BMP* gradX = BMP_Create(width, height, depth);
	BMP* gradY = BMP_Create(width, height, depth);
	BMP* gradA = BMP_Create(width, height, depth);
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int b = (int)sqrt(EdgeX[i + j*width] * EdgeX[i + j*width] + EdgeY[i + j*width] * EdgeY[i + j*width]);
			BMP_SetPixelRGB(gradX, i, j, 128 + EdgeX[i + j*width] / coefficient, 128 + EdgeX[i + j*width] / coefficient, 128 + EdgeX[i + j*width] / coefficient);
			BMP_SetPixelRGB(gradY, i, j, 128 + EdgeY[i + j*width] / coefficient, 128 + EdgeY[i + j*width] / coefficient, 128 + EdgeY[i + j*width] / coefficient);
			BMP_SetPixelRGB(gradA, i, j, b / coefficient, b / coefficient, b / coefficient);
		}
	}
	BMP_WriteFile(gradX, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\gradX.bmp");
	BMP_WriteFile(gradY, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\gradY.bmp");
	BMP_WriteFile(gradA, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\gradA.bmp");
	BMP_Free(gradX);
	BMP_Free(gradY);
	return gradA;
}

BMP* CannyEdgeDetection(BMP* bmp, int thrhld, int coefficient, double highShr, double lowShr, int radius)
{
	int SobelX[3][3] = { { -1,0,1 },{ -2,0,2 },{ -1,0,1 } };
	int SobelY[3][3] = { { -1,-2,-1 },{ 0,0,0 },{ 1,2,1 } };
	//Convert to Greyscale
	RGBtoBW(bmp);
	//BMP_WriteFile(bmp, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\BW.bmp");
	//Gaussian blur
	BMP* tmpbmp = FastGaussianBlur(bmp, 1, 3);
	BMP* swap = bmp;
	bmp = tmpbmp;
	tmpbmp = swap;
	//BMP_WriteFile(bmp, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\Blur.bmp");
	//BMP_Free(tmpbmp);
	//Get basic information
	UINT width, height;
	USHORT depth;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	depth = BMP_GetDepth(bmp);
	int* EdgeX = malloc(sizeof(int)*height*width);
	int* EdgeY = malloc(sizeof(int)*height*width);
	int* Direc = malloc(sizeof(int)*height*width);
	int* GradA = malloc(sizeof(int)*height*width);
	memset(EdgeX, 0, sizeof(int)*height*width);
	memset(EdgeY, 0, sizeof(int)*height*width);
	memset(Direc, -1, sizeof(int)*height*width);
	memset(GradA, 0, sizeof(int)*height*width);
	BMP* Canny = BMP_Create(width, height, depth);
	//Convolve with Sobel matrix
	for (int i = 1; i < width - 1; i++)
	{
		for (int j = 1; j < height - 1; j++)
		{
			int sumX = 0, sumY = 0;
			//Calculate gradient
			for (int iX = 0; iX < 3; iX++)
			{
				for (int iY = 0; iY < 3; iY++)
				{
					UCHAR r = 0, g = 0, b = 0;
					BMP_GetPixelRGB(bmp, i + iY - 1, j + iX - 1, &r, &g, &b);
					sumX += r*SobelX[iX][iY];
					sumY += r*SobelY[iX][iY];
				}
			}
			if (abs(sumX) > thrhld && abs(sumX) < 1020) EdgeX[i + j*width] = sumX;
			if (abs(sumY) > thrhld && abs(sumY) < 1020) EdgeY[i + j*width] = sumY;
			GradA[i + j*width] = sqrt(EdgeX[i + j*width] * EdgeX[i + j*width] + EdgeY[i + j*width] * EdgeY[i + j*width]);
			double theta = 0;
			if (GradA[i + j*width] != 0)
			{
				theta = atan2(EdgeY[i + j*width], EdgeX[i + j*width]);
				//Angle rounding ( long code here )
				{
					if (theta < PI / 8 && theta >= -PI / 8)
					{
						Direc[i + j*width] = 0; // 0 for East
					}
					else
					{
						if (theta >= PI / 8 && theta < PI * 3 / 8)
						{
							Direc[i + j*width] = 1;// 1 for North-East
						}
						else
						{
							if (theta >= PI * 3 / 8 && theta < PI * 5 / 8)
							{
								Direc[i + j*width] = 2;// 2 for North
							}
							else
							{
								if (theta >= PI * 5 / 8 && theta < PI * 7 / 8)
								{
									Direc[i + j*width] = 3;// 3 for North-West
								}
								else
								{
									if (theta >= PI * 7 / 8 || theta < -PI * 7 / 8)
									{
										Direc[i + j*width] = 4;// 4 for West
									}
									else
									{
										if (theta >= -PI * 7 / 8 && theta < -PI * 5 / 8)
										{
											Direc[i + j*width] = 5;// 5 for South-West
										}
										else
										{
											if (theta >= -PI * 5 / 8 && theta < -PI * 3 / 8)
											{
												Direc[i + j*width] = 6;// 6 for South
											}
											else
											{
												if (theta >= -PI * 3 / 8 && theta < -PI * 1 / 8)
												{
													Direc[i + j*width] = 7;// 7 for South-East
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			//
			//printf("%3d", Direc[i + j*width]);
		}
		//	printf("\n");
	}
	// Second Iteration
	for (int i = 1; i < width - 1; i++)
	{
		for (int j = 0; j < height - 1; j++)
		{
			if (Direc[i + j*width] != -1)
			{
				//check neibours
				if (Direc[i + j*width] == 0 || Direc[i + j*width] == 4)
				{
					if (GradA[i + j*width] > GradA[i + 1 + j*width] && GradA[i + j*width] > GradA[i - 1 + j*width] && GradA[i + j*width] != 0)
					{
						BMP_SetPixelRGB(Canny, i, j, (UCHAR)(GradA[i + j*width] / coefficient), (UCHAR)(GradA[i + j*width] / coefficient), (UCHAR)(GradA[i + j*width] / coefficient));
					}
				}
				else
				{
					if (Direc[Direc[i + j*width] == 1 || Direc[i + j*width] == 5])
					{
						if (GradA[i + j*width] > GradA[i + 1 + (j - 1)*width] && GradA[i + j*width] > GradA[i - 1 + (j + 1)*width] && GradA[i + j*width] != 0)
						{
							BMP_SetPixelRGB(Canny, i, j, (UCHAR)(GradA[i + j*width] / coefficient), (UCHAR)(GradA[i + j*width] / coefficient), (UCHAR)(GradA[i + j*width] / coefficient));
						}
					}
					else
					{
						if (Direc[Direc[i + j*width] == 2 || Direc[i + j*width] == 6])
						{
							if (GradA[i + j*width] > GradA[i + (j + 1)*width] && GradA[i + j*width] > GradA[i + (j - 1)*width] && GradA[i + j*width] != 0)
							{
								BMP_SetPixelRGB(Canny, i, j, (UCHAR)(GradA[i + j*width] / coefficient), (UCHAR)(GradA[i + j*width] / coefficient), (UCHAR)(GradA[i + j*width] / coefficient));
							}
						}
						else
						{
							if (Direc[Direc[i + j*width] == 3 || Direc[i + j*width] == 7])
							{
								if (GradA[i + j*width] > GradA[i - 1 + (j - 1)*width] && GradA[i + j*width] > GradA[i + 1 + (j + 1)*width] && GradA[i + j*width] != 0)
								{
									BMP_SetPixelRGB(Canny, i, j, (UCHAR)(GradA[i + j*width] / coefficient), (UCHAR)(GradA[i + j*width] / coefficient), (UCHAR)(GradA[i + j*width] / coefficient));
								}
							}
						}
					}
				}
			}
		}
	}
	free(GradA);
	free(Direc);
	free(EdgeX);
	free(EdgeY);
	Canny = DoubleThreshold(Canny, highShr, lowShr, radius);
	//BMP_WriteFile(Canny, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\Canny.bmp");
	return Canny;
}

BMP* DoubleThreshold(BMP* bmp, double high, double low, int radius)
{
	//CONSTANTS
	double HighShrRate = high;
	double LowShrRate = low;
	//  Generate histogram
	UINT width, height;
	USHORT depth;
	width = BMP_GetWidth(bmp);
	height = BMP_GetHeight(bmp);
	depth = BMP_GetDepth(bmp);
	int* image = malloc(sizeof(int)*height*width);
	memset(image, -1, sizeof(int)*height*width);
	int histogram[256] = { 0 };
	int maxHeight = 0;
	int totalPixel = 0;
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			UCHAR r = 0, g = 0, b = 0;
			BMP_GetPixelRGB(bmp, i, j, &r, &g, &b);
			if (r != 0)
			{
				histogram[r]++;
				if (histogram[r] > maxHeight) maxHeight = histogram[r];
				totalPixel++;
			}
		}
	}
	//Write the histogram to file
	/*BMP* histoBMP = BMP_Create(256, (UINT)(maxHeight*1.2), depth);
	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < (UINT)(maxHeight*1.2); j++)
		{
			if (j <= histogram[i]) BMP_SetPixelRGB(histoBMP, i, (UINT)(maxHeight*1.2) - 1 - j, i, i, i);
		}
	}
	BMP_WriteFile(histoBMP, "C:\\Users\\HP\\Documents\\Visual Studio 2015\\Projects\\ImageProcessing\\Debug\\Tst\\Histo.bmp");
	BMP_Free(histoBMP);
	*/
	//Calculate shreshold according to the histogram
	int countPixel = 0;
	int maxPix = (int)(totalPixel*(1 - HighShrRate));
	int minPix = (int)(totalPixel*(1 - LowShrRate));
	int maxVal = 0;
	int minVal = 0;
	int maxflag = 0, minflag = 0;
	for (int i = 255; i > 0; i--)
	{
		countPixel += histogram[i];
		if (countPixel > maxPix && maxflag == 0)
		{
			maxVal = i;
			maxflag = 1;
		}
		if (countPixel > minPix && minflag == 0)
		{
			minVal = i;
			minflag = 1;
		}
		if (minflag == 1 && maxflag == 1) break;
	}
	printf("totoal pixel= %d\nmax= %d\nmin= %d\n", totalPixel, maxVal, minVal); // DEBUG
	//Double Shreshold
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			UCHAR r, g, b;
			BMP_GetPixelRGB(bmp, i, j, &r, &g, &b);
			if (r == 0) continue;
			if (r >= maxVal)
			{
				image[i + j*width] = 2; // 2 for marked edge
			}
			else
			{
				if (r <= minVal)
				{
					image[i + j*width] = 1; // 1 for marked noise
				}
				else
				{
					image[i + j*width] = 0; // 0 for marked as between
				}
			}
		}
	}
	///////////////////////////D//E//B//U//G//////////////////////////////
	/*for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			printf("%3d", image[i*width + j]);
		}
		printf("\n");
	}*/
	//////////////////////////////////////////////////////////////////////
	//Judge Connectivity
	EgdeConnection(image, width, height, radius);
	//Write BMP
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			if (image[i + j*width] != 2)
			{
				BMP_SetPixelRGB(bmp, i, j, 0, 0, 0);
			}
			else
			{
				if (image[i + j*width] == 2) BMP_SetPixelRGB(bmp, i, j, 255, 255, 255);
			}
		}
	}
	return bmp;
}

int* EgdeConnection(int* img, int width, int height, int radius)
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			if (img[i + j*width] == 0)
			{
				IsConnected(img, width, height, i, j, radius);
			}
		}
	}
	/////////////////////////////D//E//B//U//G////////////////////////////////
	/*printf("\n\n");
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			printf("%3d", img[i*width + j]);
		}
		printf("\n");
	}*/
	/////////////////////////////////////////////////////////////////////////
	return img;
}

int* IsConnected(int* img, int width, int height, int x, int y, int radius)
{
	img[x + y*width] = 3; // 3 for marked temporaryly
	for (int i = 0; i < radius; i++)
	{
		for (int j = 0; j < radius; j++)
		{
			if (!(x - (radius / 2) + i + (y - (radius / 2) + j)*width >= 0 && x - (radius / 2) + i + (y - (radius / 2) + j)*width < height*width)) continue;
			if (img[x - (radius / 2) + i + (y - (radius / 2) + j)*width] == 2)
			{
				img[x + y*width] = 2;
				return img;
			}
		}
	}
	for (int i = 0; i < radius; i++)
	{
		for (int j = 0; j < radius; j++)
		{
			if (!(x - (radius / 2) + i + (y - (radius / 2) + j)*width >= 0 && x - (radius / 2) + i + (y - (radius / 2) + j)*width < height*width)) continue;
			if (img[x - (radius / 2) + i + (y - (radius / 2) + j)*width] == 0)
			{
				img = IsConnected(img, width, height, x - (radius / 2) + i, y - (radius / 2) + j, radius);
			}
			if (img[x - (radius / 2) + i + (y - (radius / 2) + j)*width] == 2)
			{
				img[x + y*width] = 2;
				return img;
			}
		}
	}
	img[x + y*width] = 1;
	return img;
}

double maximum(double a, double b, double c, double d)
{
	double par = a;
	double var[3] = { b,c,d };
	for (int i = 0; i < 3; i++)
		par = (par > var[i]) ? par : var[i];
	return par;
}

double minimum(double a, double b, double c, double d)
{
	double par = a;
	double var[3] = { b,c,d };
	for (int i = 0; i < 3; i++)
		par = (par < var[i]) ? par : var[i];
	return par;
}

double r_compute(double var_x, double var_y, BMP* bmp, double cost, double sint)
{
	UCHAR R[4] = { 0,0,0,0 };
	UCHAR G[4] = { 0,0,0,0 };
	UCHAR B[4] = { 0,0,0,0 };

	BMP_GetPixelRGB(bmp, (unsigned long)var_x, (unsigned long)var_y, R, G, B);
	BMP_GetPixelRGB(bmp, (unsigned long)var_x + 1, (unsigned long)var_y, R + 1, G + 1, B + 1);
	BMP_GetPixelRGB(bmp, (unsigned long)var_x, (unsigned long)var_y + 1, R + 2, G + 2, B + 2);
	BMP_GetPixelRGB(bmp, (unsigned long)var_x + 1, (unsigned long)var_y + 1, R + 3, G + 3, B + 3);

	//Area average of the colors
	return ((1 - var_x + (unsigned long)var_x)*(1 - var_y + (unsigned long)var_y)*R[0] + (var_x - (unsigned long)var_x)*(1 - var_y + (unsigned long)var_y)*R[1] + (1 - var_x + (unsigned long)var_x)*(var_y - (unsigned long)var_y)*R[2] + (var_x - (unsigned long)var_x)*(var_y - (unsigned long)var_y)*R[3]);
}

double g_compute(double var_x, double var_y, BMP* bmp, double cost, double sint)
{
	UCHAR R[4] = { 0,0,0,0 };
	UCHAR G[4] = { 0,0,0,0 };
	UCHAR B[4] = { 0,0,0,0 };

	BMP_GetPixelRGB(bmp, (unsigned long)var_x, (unsigned long)var_y, R, G, B);
	BMP_GetPixelRGB(bmp, (unsigned long)var_x + 1, (unsigned long)var_y, R + 1, G + 1, B + 1);
	BMP_GetPixelRGB(bmp, (unsigned long)var_x, (unsigned long)var_y + 1, R + 2, G + 2, B + 2);
	BMP_GetPixelRGB(bmp, (unsigned long)var_x + 1, (unsigned long)var_y + 1, R + 3, G + 3, B + 3);

	//Area average of the colors
	return ((1 - var_x + (unsigned long)var_x)*(1 - var_y + (unsigned long)var_y)*G[0] + (var_x - (unsigned long)var_x)*(1 - var_y + (unsigned long)var_y)*G[1] + (1 - var_x + (unsigned long)var_x)*(var_y - (unsigned long)var_y)*G[2] + (var_x - (unsigned long)var_x)*(var_y - (unsigned long)var_y)*G[3]);
}

double b_compute(double var_x, double var_y, BMP* bmp, double cost, double sint)
{
	UCHAR R[4] = { 0,0,0,0 };
	UCHAR G[4] = { 0,0,0,0 };
	UCHAR B[4] = { 0,0,0,0 };

	BMP_GetPixelRGB(bmp, (unsigned long)var_x, (unsigned long)var_y, R, G, B);
	BMP_GetPixelRGB(bmp, (unsigned long)var_x + 1, (unsigned long)var_y, R + 1, G + 1, B + 1);
	BMP_GetPixelRGB(bmp, (unsigned long)var_x, (unsigned long)var_y + 1, R + 2, G + 2, B + 2);
	BMP_GetPixelRGB(bmp, (unsigned long)var_x + 1, (unsigned long)var_y + 1, R + 3, G + 3, B + 3);

	//Area average of the colors
	return ((1 - var_x + (unsigned long)var_x)*(1 - var_y + (unsigned long)var_y)*B[0] + (var_x - (unsigned long)var_x)*(1 - var_y + (unsigned long)var_y)*B[1] + (1 - var_x + (unsigned long)var_x)*(var_y - (unsigned long)var_y)*B[2] + (var_x - (unsigned long)var_x)*(var_y - (unsigned long)var_y)*B[3]);
}

BMP* AMRotation(BMP* bmp, double theta)
{
	//Compute some important coefficient
	double cost = cos(theta);
	double sint = sin(theta);
	double pre_width = BMP_GetWidth(bmp);
	double pre_height = BMP_GetHeight(bmp);
	USHORT depth = BMP_GetDepth(bmp);

	//Analysing size of new picture;
	double max_x = maximum((pre_width*cost - pre_height*sint), -pre_height*sint, 0.1 * 0, pre_width*cost);
	double min_x = minimum((pre_width*cost - pre_height*sint), -pre_height*sint, 0.1 * 0, pre_width*cost);
	double max_y = maximum((pre_width*sint + pre_height*cost), pre_height*cost, 0.1 * 0, pre_width*sint);
	double min_y = minimum((pre_width*sint + pre_height*cost), pre_height*cost, 0.1 * 0, pre_width*sint);
	double width = max_x - min_x;
	double height = max_y - min_y;

	BMP* newbmp = BMP_Create(width, height, depth);

	//x`=Ax + (-minx,-miny);
	for (UINT x = 0; x < width; x++)
	{
		for (UINT y = 0; y < height; y++)
		{
			double r = 0, g = 0, b = 0;

			//Compute original coordinate in newbmp system
			double pre_x = cost*(x + min_x) + sint*(y + min_y);
			double pre_y = -sint*(x + min_x) + cost*(y + min_y);

			if ((pre_x > 0 && pre_x < pre_width) && (pre_y > 0 && pre_y < pre_height))
			{
				r = r_compute(pre_x, pre_y, bmp, cost, sint);
				g = g_compute(pre_x, pre_y, bmp, cost, sint);
				b = b_compute(pre_x, pre_y, bmp, cost, sint);
			}
			BMP_SetPixelRGB(newbmp, x, y, (UCHAR)r, (UCHAR)g, (UCHAR)b);
		}
	}

	return newbmp;
}

double ABS(double a) {
	if (a >= 0) return a;
	else return -a;
}

double s(double a) {
	if (a >= 0 && a < 1) return 1 - 2 * a*a + a*a*a;
	if (a >= 1 && a < 2) return 4 - 8 * a + 5 * a*a - 3 * a*a*a;
	if (a >= 2) return 0;
}

double huidu(double row, double column, BMP* bmp)
{
	int width = BMP_GetWidth(bmp);
	int height = BMP_GetHeight(bmp);
	int i = row;
	int j = column;
	double u = row - (double)i;
	double v = column - (double)j;
	/*double a[4] = { s(u + 1),s(u),s(ABS(u - 1)),s(ABS(u - 2)) };
	double B[4][4] = { 0 };
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			UCHAR r, g, b;
			if (i - 1 + x >= 0 && i - 1 + x < width&&j - 1 + y >= 0 && j - 1 + y < height)
			{
				BMP_GetPixelRGB(bmp, i - 1 + x, j - 1 + y, &r, &g, &b);
				B[x][y] = r;
			}
		}
	}
	double c[4] = { s(v + 1),s(v),s(ABS(v - 1)),s(ABS(v - 2)) };
	double d[4] = { 0 };
	double e = 0;
	for (int i = 0; i < 4; i++) {
		d[i] = a[0] * B[0][i] + a[1] * B[1][i] + a[2] * B[2][i] + a[3] * B[3][i];
	}
	for (int i = 0; i < 4; i++) {
		e += d[i] * c[i];
	}
	return e;*/
	double B[2][2] = { 0 };
	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++) {
			UCHAR r, g, b;
			if (i - 1 + x >= 0 && i - 1 + x < width&&j - 1 + y >= 0 && j - 1 + y < height)
			{
				BMP_GetPixelRGB(bmp, i - 1 + x, j - 1 + y, &r, &g, &b);
				B[x][y] = r;
			}
		}
	}
	return (1 - u)*(1 - v)*B[0][0] + u*(1 - v)*B[1][0] + (1 - u)*v*B[0][1] + u*v*B[1][1];

}

BMP* DINTRotation(BMP* bmp, double theta)
{
	RGBtoBW(bmp);
	//Compute some important coefficient
	double cost = cos(theta);
	double sint = sin(theta);
	double pre_width = BMP_GetWidth(bmp);
	double pre_height = BMP_GetHeight(bmp);
	USHORT depth = BMP_GetDepth(bmp);

	//Analysing size of new picture;
	double max_x = maximum((pre_width*cost - pre_height*sint), -pre_height*sint, 0.1 * 0, pre_width*cost);
	double min_x = minimum((pre_width*cost - pre_height*sint), -pre_height*sint, 0.1 * 0, pre_width*cost);
	double max_y = maximum((pre_width*sint + pre_height*cost), pre_height*cost, 0.1 * 0, pre_width*sint);
	double min_y = minimum((pre_width*sint + pre_height*cost), pre_height*cost, 0.1 * 0, pre_width*sint);
	double width = max_x - min_x;
	double height = max_y - min_y;

	BMP* newbmp = BMP_Create(width, height, depth);

	//x`=Ax + (-minx,-miny);
	for (UINT x = 0; x < width; x++)
	{
		for (UINT y = 0; y < height; y++)
		{
			double r = 0, g = 0, b = 0;

			//Compute original coordinate in newbmp system
			double pre_x = cost*(x + min_x) + sint*(y + min_y);
			double pre_y = -sint*(x + min_x) + cost*(y + min_y);
			if ((pre_x > 0 && pre_x < pre_width) && (pre_y > 0 && pre_y < pre_height))
			{
				r = huidu(pre_x, pre_y, bmp);
				BMP_SetPixelRGB(newbmp, x, y, (UCHAR)r, (UCHAR)r, (UCHAR)r);
			}
		}
	}
	return newbmp;
}