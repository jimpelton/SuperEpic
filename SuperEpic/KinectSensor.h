#pragma once

#include <stdio.h>
#include <iostream>
#include <Kinect.h>

#define FRAME_DEPTH		0
#define FRAME_COLOR		1
#define FRAME_INFRARED	2
#define FRAME_BODY		3

#define VIRTUAL_RECTANGLE_CORNER_L_X 0.15
#define VIRTUAL_RECTANGLE_CORNER_L_Y 0.55

#define VIRTUAL_RECTANGLE_CORNER_R_X 0.35
#define VIRTUAL_RECTANGLE_CORNER_R_Y 0.25


class KinectSensor
{
	
private:
	IKinectSensor * pKinectSensor;

	IDepthFrameReader * pDepthFrameReader;
	IColorFrameReader * pColorFrameReader;
	IInfraredFrameReader * pInfraredFrameReader;
	IBodyFrameReader * pBodyFrameReader;

	ICoordinateMapper * m_pCoordinateMapper;

	

	void error(std::string e, HRESULT hr);

public:
	

	KinectSensor();
	~KinectSensor();

	IDepthFrameReader * getDepthFrameReader();
	IColorFrameReader * getColorFrameReader();
	IInfraredFrameReader * getInfraredFrameReader();
	IBodyFrameReader * getBodyFrameReader();
	ICoordinateMapper * getCoordinateMapper();

	
	/* Returns the next frame from the depth reader */
	IDepthFrame * getNextDepthFrame();

	/* Returns the next frame from the color reader */
	IColorFrame * getNextColorFrame();

	/* Returns the next frame from the depth reader */
	IInfraredFrame * getNextInfraredFrame();

	/* Returns the next frame from the depth reader */
	IBodyFrame * getNextBodyFrame();

	
	/* Updates the hand position variable. (Put this into a thread)*/
	static void updateHandPosition();
	static void mapHandToCursor(float * handPosition, int screenWidth, int screenHeight, int * cursor);
	static float* handCoords;
																	/* Releases any Kinect interface */
	template<class Interface>
	void SafeRelease(Interface *& pInterface);//TODO

};

