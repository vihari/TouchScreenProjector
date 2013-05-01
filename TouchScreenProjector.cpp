/*The MIT License (MIT)

Copyright (c) <2013> <Vihari Piratla>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.*/

/*******************************************************************
 *
 *	TouchScreenProjector.cpp 
 *      Purpose: To aid one in interacting right from the projector screen, 
 *               or any surface on to which the computer screen is projected.
 *               For more information, please refer to the description of                                       
 *                  https://github.com/vihari/TouchScreenProjector.
 *
 *      Hardware required: 
 *      1) A web-cam that is facing the projected screen and can see all the corners of the projection.
 *      2) A red laser pointer or a red light emitting pen(or a device attached with red LED, 
 *         having a switch is beneficial).
 *      3) A projecting surface and a projector.
 *      4) A computational module.
 *      6) OpenCV.
 *      5) Windows Operating system.
 * 
 *      Implementation: The implementation is divided into following sub-stages:
 *        1) Mapping Phase: Mapping of camera coordinated to system coordinates:
 *           We have to map the camera coordinates to computer/projector coordinates in order to 
 *           initialize event at the right place. This is done by mapping the projector screen corners
 *           on to the camera coordinates. For this, a black screen will be shown at the start of the code and 
 *           yellow dots at the corner of the screen. One has to focus the red light on these dots for this 
 *           mapping. For more deeper explanation, please refer to the comments of the corresponding code 
 *           and the code.
 *        2) Red Light detection:
 *           This involves capturing the image, thresholding it and getting the position of the  pointer
 *           by extracting the moments of the image.
 *        3) Event initialization:
 *           This is to initialize an event at the mapped position of the pointer detected in the view.
 *
 *      Note: The event Trigger code is specifically, for the windows;one should modify 
 *            the code accordingly for use in other OS. Also the memory management is done with 
 *            Visual C++ in mind, with other compilers this code may have software aging if not 
 *            segmentation fault
 *      To use this code one should have the environment set up with OpenCV.
 * 
 *      Email: viharipiratla@gmail.com              
-----------*/

#include "stdafx.h"
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>

/*void sleep(long d)
  {
  clock_t start=clock();
  while(clock() - start < d);
  }*/

/*Thresholding phase*/
inline IplImage* getThreshold(IplImage* src){

  IplImage* Temp = cvCreateImage(cvGetSize(src),8,3);
  cvCvtColor(src,Temp,CV_BGR2HSV);

  IplImage* ThreshImage = cvCreateImage(cvGetSize(src),8,1);
  //cvShowImage("HSV",Temp);

  //Threshold for red color
  cvInRangeS(Temp,cvScalar(0, 0, 120), cvScalar(20,255,100000),ThreshImage);

  cvReleaseImage(&Temp);
  return(ThreshImage);
  //cvDestroyWindow("HSV");
}

inline IplImage* getThresholdGreen(IplImage* src){

  IplImage* Temp = cvCreateImage(cvGetSize(src),8,3);
  cvCvtColor(src,Temp,CV_BGR2HSV);

  IplImage* ThreshImage = cvCreateImage(cvGetSize(src),8,1);
  //cvShowImage("HSV",Temp);

  cvInRangeS(Temp,cvScalar(50, 150, 0), cvScalar(70,200,100),ThreshImage);

  cvReleaseImage(&Temp);
  return(ThreshImage);
  //cvDestroyWindow("HSV");
}

/*The locations where the yellow dots have to be displayed, 
better if one sets these coordinates to the resolution of their system*/
CvPoint drdots[4] = {{40,40},{1000,40},{40,740},{1000,740}};
CvPoint dots[4] = {{40,40},{1000,40},{40,740},{1000,740}};
CvPoint cal[4];
float a1,b1,c1,a2,b2,c2,a3,b3;

/*Calibration phase*/
int calibrate(void){
  cvNamedWindow("Calibration",CV_WINDOW_AUTOSIZE);
  cvMoveWindow("Calibration",0,0);
  cvSetWindowProperty("Calibration",CV_WINDOW_FULLSCREEN,CV_WINDOW_FULLSCREEN);
  //HWND win_handle = FindWindow(0,L"Calibration");

  //SetWindowLong(win_handle,GWL_STYLE,GetWindowLong(win_handle,GWL_EXSTYLE)|WS_EX_TOPMOST);

  printf( "running calibration\n" );
  CvCapture* capture = 0;
  capture = cvCaptureFromCAM(0);
  CvSize sz;
  sz.width = 1024;sz.height = 768;

  IplImage* show = cvCreateImage(sz,8,3);
  cvZero(show);
  IplImage *frame = NULL;
  IplImage* ThreshImage = NULL;
  CvPoint pt;

  cvCircle(show,dots[0],5,cvScalar(255,255,255),1,8,0);
  cvCircle(show,dots[1],5,cvScalar(255,255,255),1,8,0);
  cvCircle(show,dots[2],5,cvScalar(255,255,255),1,8,0);
  cvCircle(show,dots[3],5,cvScalar(255,255,255),1,8,0);
  cvShowImage("Calibration",show);
  cvWaitKey(20);

  while(-1){
    ThreshImage = NULL;
    frame = cvQueryFrame(capture);
		
    if(!frame){
      printf("Unable to capture the frame...\n");
      break;
    }
		
    ThreshImage = getThreshold(frame);

    CvMoments *Moments = (CvMoments*)malloc(sizeof(CvMoments));

    cvMoments(ThreshImage,Moments,1);
		
    double m10 = cvGetSpatialMoment(Moments,1,0);
		
    double m01 = cvGetSpatialMoment(Moments,0,1);
		
    double m00	= cvGetCentralMoment(Moments,0,0);

    pt.x = m10/m00;
    pt.y = m01/m00;
    if((pt.x >cvGetSize(frame).width/2)||(pt.y > cvGetSize(frame).height/2))
      continue;
    else if((pt.x>0)&&(pt.y>0)) break;

  }

  cal[0].x = pt.x;
  cal[0].y = pt.y;

  /*Notify the detection to the user.*/
  cvCircle(show,dots[0],20,cvScalar(0,255,255),1,8,0);
  cvShowImage("Calibration",show);
  cvWaitKey(2);

  while(-1){
    ThreshImage = NULL;
    frame = cvQueryFrame(capture);
		
    if(!frame){
      printf("Unable to capture the frame...\n");
      break;
    }
		
    ThreshImage = getThreshold(frame);

    CvMoments *Moments = (CvMoments*)malloc(sizeof(CvMoments));

    cvMoments(ThreshImage,Moments,1);
		
    double m10 = cvGetSpatialMoment(Moments,1,0);
		
    double m01 = cvGetSpatialMoment(Moments,0,1);
		
    double m00	= cvGetCentralMoment(Moments,0,0);

    pt.x = m10/m00;
    pt.y = m01/m00;
    if((pt.x <cvGetSize(frame).width/2)||(pt.y > cvGetSize(frame).height/2))
      continue;
    else if((pt.x>0)&&(pt.y>0)) break;

  }

  cal[1].x = pt.x;
  cal[1].y = pt.y;
  cvCircle(show,dots[1],20,cvScalar(0,255,255),1,8,0);
  cvShowImage("Calibration",show);
  cvWaitKey(2);

  while(-1){
    ThreshImage = NULL;
    frame = cvQueryFrame(capture);
		
    if(!frame){
      printf("Unable to capture the frame...\n");
      break;
    }
		
    ThreshImage = getThreshold(frame);

    CvMoments *Moments = (CvMoments*)malloc(sizeof(CvMoments));

    cvMoments(ThreshImage,Moments,1);
		
    double m10 = cvGetSpatialMoment(Moments,1,0);
		
    double m01 = cvGetSpatialMoment(Moments,0,1);
		
    double m00	= cvGetCentralMoment(Moments,0,0);

    pt.x = m10/m00;
    pt.y = m01/m00;
    if((pt.x >cvGetSize(frame).width/2)||(pt.y < cvGetSize(frame).height/2))
      continue;
    else if((pt.x>0)&&(pt.y>0)) break;

  }

  cal[2].x = pt.x;
  cal[2].y = pt.y;
  cvCircle(show,dots[2],20,cvScalar(0,255,255),1,8,0);
  cvShowImage("Calibration",show);
  cvWaitKey(2);

  while(-1){
    ThreshImage = NULL;
    frame = cvQueryFrame(capture);
		
    if(!frame){
      printf("Unable to capture the frame...\n");
      break;
    }
		
    ThreshImage = getThreshold(frame);

    CvMoments *Moments = (CvMoments*)malloc(sizeof(CvMoments));

    cvMoments(ThreshImage,Moments,1);
		
    double m10 = cvGetSpatialMoment(Moments,1,0);
		
    double m01 = cvGetSpatialMoment(Moments,0,1);
		
    double m00	= cvGetCentralMoment(Moments,0,0);

    pt.x = m10/m00;
    pt.y = m01/m00;
    if((pt.x < cvGetSize(frame).width/2)||(pt.y < cvGetSize(frame).height/2))
      continue;
    else if((pt.x>0)&&(pt.y>0)){
      printf("%lf,%lf\n",pt.x,pt.y);
      break;
    }

  }

  cal[3].x = pt.x;
  cal[3].y = pt.y;
  cvCircle(show,dots[3],20,cvScalar(0,255,255),1,8,0);
  cvShowImage("Calibration",show);
  cvWaitKey(2);

  float matrix[8][8] = { 
    { -1, -1, -1, -1, 0, 0, 0, 0 },
    { -cal[0].x, -cal[1].x, -cal[2].x, -cal[3].x, 0, 0, 0, 0 },
    { -cal[0].y, -cal[1].y, -cal[2].y, -cal[3].y, 0,0,0,0 },
    { 0,0,0,0,-1,-1,-1,-1 },
    { 0,0,0,0, -cal[0].x, -cal[1].x, -cal[2].x, -cal[3].x },
    { 0,0,0,0, -cal[0].y, -cal[1].y, -cal[2].y, -cal[3].y },
    { cal[0].x * dots[0].x, cal[1].x * dots[1].x, cal[2].x * dots[2].x, cal[3].x * dots[3].x, cal[0].x * dots[0].y, cal[1].x * dots[1].y, cal[2].x * dots[2].y, cal[3].x * dots[3].y },
    { cal[0].y * dots[0].x, cal[1].y * dots[1].x, cal[2].y * dots[2].x, cal[3].y * dots[3].x, cal[0].y * dots[0].y, cal[1].y * dots[1].y, cal[2].y * dots[2].y, cal[3].y * dots[3].y },
  };

  float bb[8] = { -dots[0].x, -dots[1].x, -dots[2].x, -dots[3].x, -dots[0].y, -dots[1].y, -dots[2].y, -dots[3].y };

  // gauﬂ-elimination
  /*Gauss-Jordon elimination to solve for the mapping coefficients
   Where matrix matrix is the equations to be solved
   and bb is the right hand side values.
   cal is the camera coordinates and dots is projector/computer coordinates
   Here we are trying to solve system of equations
   corrX = (a1 * X + b1 * Y + c1 ) / (a3 * X + b3 * Y + 1 );
   corrY = (a2 * X + b2 * Y + c2 ) / (a3 * X + b3 * Y + 1 );
   where corrX and corrY are both computer coordinates and X,Y are camera coordinates
   because there are 8 unknowns, we need atleast 4 X equations and 4Y equations that is 4 points.
   All that follows is just an elimination to find these a's and b's please ignore, 
   if you cannot understand.
  */

  for( int j = 1; j < 4; j ++ )
    {

      for( int i = 1; i < 8; i ++ )
	{
	  matrix[i][j] = - matrix[i][j] + matrix[i][0];
	}
      bb[j] = -bb[j] + bb[0];
      matrix[0][j] = 0;

    }


  for( int i = 2; i < 8; i ++ )
    {
      matrix[i][2] = -matrix[i][2] / matrix[1][2] * matrix[1][1] + matrix[i][1];
    }
  bb[2] = - bb[2] / matrix[1][2] * matrix[1][1] + bb[1];
  matrix[1][2] = 0;


  for( int i = 2; i < 8; i ++ )
    {
      matrix[i][3] = -matrix[i][3] / matrix[1][3] * matrix[1][1] + matrix[i][1];
    }
  bb[3] = - bb[3] / matrix[1][3] * matrix[1][1] + bb[1];
  matrix[1][3] = 0;


  for( int i = 3; i < 8; i ++ )
    {
      matrix[i][3] = -matrix[i][3] / matrix[2][3] * matrix[2][2] + matrix[i][2];
    }
  bb[3] = - bb[3] / matrix[2][3] * matrix[2][2] + bb[2];
  matrix[2][3] = 0;

  for( int j = 5; j < 8; j ++ )
    {
      for( int i = 4; i < 8; i ++ )
	{
	  matrix[i][j] = -matrix[i][j] + matrix[i][4];
	}
      bb[j] = -bb[j] + bb[4];
      matrix[3][j] = 0;
    }


  for( int i = 5; i < 8; i ++ )
    {
      matrix[i][6] = -matrix[i][6] / matrix[4][6] * matrix[4][5] + matrix[i][5];
    }

  bb[6] = - bb[6] / matrix[4][6] * matrix[4][5] + bb[5];
  matrix[4][6] = 0;


  for( int i = 5; i < 8; i ++ )
    {
      matrix[i][7] = -matrix[i][7] / matrix[4][7] * matrix[4][5] + matrix[i][5];
    }
  bb[7] = - bb[7] / matrix[4][7] * matrix[4][5] + bb[5];
  matrix[4][7] = 0;
	
  for( int i = 6; i < 8; i ++ )
    {
      matrix[i][7] = -matrix[i][7] / matrix[5][7] * matrix[5][6] + matrix[i][6];
    }
  bb[7] = - bb[7] / matrix[5][7] * matrix[5][6] + bb[6];
  matrix[5][7] = 0;



  matrix[7][7] = - matrix[7][7]/matrix[6][7]*matrix[6][3] + matrix[7][3];
  bb[7] = -bb[7]/matrix[6][7]*matrix[6][3] + bb[3];
  matrix[6][7] = 0;


  printf( "data dump" );

  printf( "bb" );
  for( int j= 0; j < 8 ; j ++ )
    {
      printf( "%lf\t", bb[j] );
    }

  printf("\n");
  printf("Cal:\n");

  for( int j= 0; j < 4 ; j ++ )
    {
      printf( "%lf,%lf\t", cal[j].x,cal[j].y );
    }

  printf("\n");

  b3 =  bb[7] /matrix[7][7];
  a3 = (bb[3]-(matrix[7][3]*b3))/matrix[6][3];
  b2 = (bb[6]-(matrix[7][6]*b3+matrix[6][6]*a3))/matrix[5][6];
  a2 = (bb[5]-(matrix[7][5]*b3+matrix[6][5]*a3+matrix[5][5]*b2))/matrix[4][5];
  c2 = (bb[4]-(matrix[7][4]*b3+matrix[6][5]*a3+matrix[5][4]*b2+matrix[4][4]*a2))/matrix[3][4];
  b1 = (bb[2]-(matrix[7][2]*b3+matrix[6][2]*a3+matrix[5][2]*b2+matrix[4][2]*a2+matrix[3][2]*c2))/matrix[2][2];
  a1 = (bb[1]-(matrix[7][1]*b3+matrix[6][1]*a3+matrix[5][1]*b2+matrix[4][1]*a2+matrix[3][1]*c2+matrix[2][1]*b1))/matrix[1][1];
  c1 = (bb[0]-(matrix[7][0]*b3+matrix[6][0]*a3+matrix[5][0]*b2+matrix[4][0]*a2+matrix[3][0]*c2+matrix[2][0]*b1+matrix[1][0]*a1))/matrix[0][0];

  cvDestroyWindow("Calibration");

  /*To ensure the values are not NaN, according to the IEEE std, NaN can be checked this way.*/
  if( b3 != b3 ) return 1;
  if( b2 != b2 ) return 1;
  if( a2 != a2 ) return 1;
  if( c2 != c2 ) return 1;
  if( a3 != a3 ) return 1;
  if( b1 != b1 ) return 1;
  if( a1 != a1 ) return 1;
  if( c1 != c1 ) return 1;
  printf( "calibrated OK" );
  //cvReleaseImage(&show);
  return 0;
}

POINT correctIt(float X, float Y )
{
  float corrX = (a1 * X + b1 * Y + c1 ) / (a3 * X + b3 * Y + 1 );
  float corrY = (a2 * X + b2 * Y + c2 ) / (a3 * X + b3 * Y + 1 );

  POINT corr;
  corr.x = corrX;
  corr.y = corrY+20;

  //println( (int)corrX + ":" + (int)corrY );
  return corr;
}

int main(){

  CvCapture* capture = 0;
  capture = cvCaptureFromCAM(0);
	
  // Couldn't get a device? Throw an error and quit
  if(!capture)
    {
      printf("Could not initialize capturing...\n");
      return -1;
    }

  IplImage* dframe;
  double x = 0,y = 0;
  dframe = cvQueryFrame(capture);

  CvSize sz;
  sz.height =768;sz.width = 1024;
  IplImage* TrajectoryImage = cvCreateImage(cvSize(sz.width,sz.height),8,3);
  cvZero(TrajectoryImage);
	
  int count1 = 1,count2 = 1,count3 = 1,count4 = 1;
  double a;double b,xd,yd;
  CvSize tr,tl,br,bl;
  CvSize size = cvGetSize(dframe);

  //sleep(14000);Z
  //keybd_event("^(2)",0,0,0);

  calibrate();
		
  cvNamedWindow("Debug1",0);
  cvNamedWindow("Debug2",0);
  cvNamedWindow("Debug3",0);
  cvNamedWindow("Debug4",0);
  POINT corr = {0,0};
  int count = 0;
	
  while(TRUE){
    IplImage* ThreshImage ;
    IplImage* ThreshImageGreen ;
    IplImage* frame ;
    frame = cvQueryFrame(capture);
		
    if(!frame){
      printf("Unable to capture the frame...\n");
      break;
    }
		
    ThreshImage = getThreshold(frame);
    ThreshImageGreen = getThresholdGreen(frame);

    CvMoments *Moments = (CvMoments*)malloc(sizeof(CvMoments));

    cvMoments(ThreshImage,Moments,1);
    //Here in we are extracting the moments from the binary image that has been thresholded. 

    double m10 = cvGetSpatialMoment(Moments,1,0);
    /*We are here extracting the x mean refer http://en.wikipedia.org/wiki/Moment_%28mathematics%29. 
      But is not normalized.*/

    double m01 = cvGetSpatialMoment(Moments,0,1);
    /*Extracting the mean of y over here...*/

    double m00	= cvGetCentralMoment(Moments,0,0);
    /*To normalize the values*/

    POINT last = corr;
	    
    double lastx = x;
    double lasty = y;
    x = m10/m00;
    y = m01/m00;

    corr = correctIt(x,y);
    //This is to normalize.

    if(last.x>0 && last.y>0 && corr.x>0 && corr.y>0&&((corr.x-last.x)<100)&&((corr.y-last.y)<100)){
      cvLine(TrajectoryImage, cvPoint(corr.x+20,corr.y-25), cvPoint(last.x+20, last.y-25), cvScalar(255,255,0), 5);
      /*To draw the trajectory of the point; say to write something or draw;*/
      POINT p;
			
      GetCursorPos(&p);
      p.x = corr.x;
      p.y = corr.y;
      SetCursorPos(p.x,p.y);
			
      //			mouse_event(MOUSEEVENTF_LEFTUP, 10, 10, 0, 0);
      //mouse_event(MOUSEEVENTF_LEFTDOWN, 10, 10, 0, 0);
      //sleep(15);
      //mouse_event(MOUSEEVENTF_LEFTUP, 10, 10, 0, 0);
      /*Event Initiation.*/
      mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
      //
    }

    int c = cvWaitKey(10);
    /*To flush the scribble window.*/
    if(c=='c')
      cvZero(TrajectoryImage);

    cvShowImage("Debug1",ThreshImage);		
    cvShowImage("Debug2",TrajectoryImage);		
    cvShowImage("Debug3",frame);	
    cvShowImage("Debug4",ThreshImageGreen);	
    cvWaitKey(1);
		
    cvReleaseImage(&ThreshImage);
    delete Moments;
    //cvReleaseImage(&frame);
  }
}

