#pragma once
#include <stdio.h>
#include <iostream>
#include <ctime>
#include <Kinect.VisualGestureBuilder.h>

#include "KinectSensor.h"

int KinectSensor::mode = 0;


float * KinectSensor::handCoords{ new float[3] };
int KinectSensor::gestureType = NO_GESTURE;
int KinectSensor::timer = 0;
bool KinectSensor::rightHand_closed = false;
bool KinectSensor::leftHand_closed = false;
float KinectSensor::hand_pos_x = 0;
float KinectSensor::hand_distance = 0;
bool KinectSensor::KeepUpdatingHandPos = true;

KinectSensor::KinectSensor()
{
	HRESULT hr;


	hr = GetDefaultKinectSensor(&pKinectSensor);
	if (FAILED(hr))
		error("ERROR : getDefaultKinectSensor() : ", hr);

	hr = pKinectSensor->Open();
	if (FAILED(hr))
		error("ERROR : pKinectSensor->open() : ", hr);



	// GET THE DEPTH READER //
	IDepthFrameSource * pDepthFrameSource;
	hr = pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
	if (FAILED(hr))
		error("ERROR : pKinectSensor->getDepthFrameSource() : ", hr);
	hr = pDepthFrameSource->OpenReader(&pDepthFrameReader);
	if (FAILED(hr))
		error("ERROR : pDepthFrameSource->OpenReader() : ", hr);
	SafeRelease<IDepthFrameSource>(pDepthFrameSource);



	// GET THE COLOR READER //
	IColorFrameSource * pColorFrameSource;
	hr = pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
	if (FAILED(hr))
		error("ERROR : pKinectSensor->getColorFrameSource() : ", hr);
	hr = pColorFrameSource->OpenReader(&pColorFrameReader);
	if (FAILED(hr))
		error("ERROR : pColorFrameSource->OpenReader() : ", hr);
	SafeRelease<IColorFrameSource>(pColorFrameSource);



	// GET THE INFRARED READER //
	IInfraredFrameSource * pInfraredFrameSource;
	hr = pKinectSensor->get_InfraredFrameSource(&pInfraredFrameSource);
	if (FAILED(hr))
		error("ERROR : pKinectSensor->getInfraredFrameSource() : ", hr);
	hr = pInfraredFrameSource->OpenReader(&pInfraredFrameReader);
	if (FAILED(hr))
		error("ERROR : pInfraredFrameSource->OpenReader() : ", hr);
	SafeRelease<IInfraredFrameSource>(pInfraredFrameSource);

	// GET THE BODY READER //
	IBodyFrameSource * pBodyFrameSource;
	hr = pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
	if (FAILED(hr))
		error("ERROR : pKinectSensor->getBodyFrameSource() : ", hr);
	hr = pBodyFrameSource->OpenReader(&pBodyFrameReader);
	if (FAILED(hr))
		error("ERROR : pBodyFrameSource->OpenReader() : ", hr);
	SafeRelease<IBodyFrameSource>(pBodyFrameSource);

	hr = pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
	if (FAILED(hr))
		error("ERROR : pKinectSensor->getCoordinateMapper() : ", hr);
}

KinectSensor::~KinectSensor()
{
}

IDepthFrame * KinectSensor::getNextDepthFrame() {
	IDepthFrame * depthFrame = NULL;
	HRESULT hr = pDepthFrameReader->AcquireLatestFrame(&depthFrame);
	if (FAILED(hr))
		error("ERROR : pDepthFrameReader->AcquireLatestFrame() : ", hr);
	return depthFrame;
}

IColorFrame * KinectSensor::getNextColorFrame() {
	IColorFrame * colorFrame = NULL;
	HRESULT hr = pColorFrameReader->AcquireLatestFrame(&colorFrame);
	if (FAILED(hr))
		error("ERROR : pColorFramReader->AcquireLatestFrame() : ", hr);
	return colorFrame;
}

IInfraredFrame * KinectSensor::getNextInfraredFrame() {
	IInfraredFrame * infraredFrame = NULL;
	HRESULT hr = pInfraredFrameReader->AcquireLatestFrame(&infraredFrame);
	if (FAILED(hr))
		error("ERROR : pInfraredFrameReader->AcquireLatestFrame() : ", hr);
	return infraredFrame;
}

IBodyFrame * KinectSensor::getNextBodyFrame() {
	IBodyFrame * bodyFrame = NULL;

	HRESULT hr = pBodyFrameReader->AcquireLatestFrame(&bodyFrame);
	if (FAILED(hr))
		error("ERROR : pBodyFrameReader->AcquireLatestFrame() : ", hr);
	return bodyFrame;
}


IDepthFrameReader * KinectSensor::getDepthFrameReader() {
	return pDepthFrameReader;
}


IColorFrameReader * KinectSensor::getColorFrameReader() {
	return pColorFrameReader;
}


IInfraredFrameReader * KinectSensor::getInfraredFrameReader() {
	return pInfraredFrameReader;
}

IBodyFrameReader * KinectSensor::getBodyFrameReader() {
	return pBodyFrameReader;
}

ICoordinateMapper * KinectSensor::getCoordinateMapper() {
	return m_pCoordinateMapper;
}

template<class Interface>
void KinectSensor::SafeRelease(Interface *& pInterface) {
	if (pInterface != NULL) {
		pInterface->Release();
		pInterface = NULL;
	}
}


void KinectSensor::error(std::string e, HRESULT hr) {
	//std::cout << e;
	std::cout << hr;
	Sleep(5000);
	exit(-1);
}








void  KinectSensor::updateHandPosition() {
	// Initialize the Kinect sensor and the readers
	KinectSensor kinectSensor = KinectSensor();
	IBodyFrame * bodyFrame;
	IBodyFrameReader * bodyreader = kinectSensor.getBodyFrameReader();




	while (1) {
		IBody* ppBodies[BODY_COUNT] = { 0 };

		// Get the frames //
		HRESULT hr = bodyreader->AcquireLatestFrame(&bodyFrame);

		if (!SUCCEEDED(hr) || bodyFrame == NULL)
			continue;

		hr = bodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		bodyFrame->Release();
		bodyFrame = NULL;


		if (!SUCCEEDED(hr))
			continue;

		// Process bodies //
		Joint joints[JointType_Count];
		for (int i = 0; i < BODY_COUNT; i++) {

			IBody * pBody = ppBodies[i];
			if (pBody) {
				BOOLEAN bTracked = false;
				hr = pBody->get_IsTracked(&bTracked);

				if (SUCCEEDED(hr) && bTracked) {

					hr = pBody->GetJoints(_countof(joints), joints);
					Joint handJoint = joints[JointType_HandRight];
					Joint spineJoint = joints[JointType_SpineBase];
					handCoords[0] = handJoint.Position.X - spineJoint.Position.X;
					handCoords[1] = handJoint.Position.Y - spineJoint.Position.Y;
					handCoords[2] = handJoint.Position.Z - spineJoint.Position.Z;

					updateGesture(pBody);

				}
				pBody->Release();
				pBody = NULL;
			}
		}
	}
}



void KinectSensor::mapHandToCursor(float * handPosition, int screenWidth, int screenHeight, int * cursor) {
	float x = handPosition[0] - VIRTUAL_RECTANGLE_CORNER_L_X;
	cursor[0] = (x * screenWidth) / (VIRTUAL_RECTANGLE_CORNER_R_X - VIRTUAL_RECTANGLE_CORNER_L_X);
	cursor[0] = cursor[0] < 0 ? 0 : cursor[0];
	cursor[0] = cursor[0] > screenWidth ? screenWidth : cursor[0];

	float y = handPosition[1] - VIRTUAL_RECTANGLE_CORNER_L_Y;
	cursor[1] = (y * screenWidth) / (VIRTUAL_RECTANGLE_CORNER_R_Y - VIRTUAL_RECTANGLE_CORNER_L_Y);
	cursor[1] = cursor[1] < 0 ? 0 : cursor[1];
	cursor[1] = cursor[1] > screenHeight ? screenHeight : cursor[1];
}



void KinectSensor::updateGesture(IBody * pBody) {
	int hr;
	HandState handState;
	pBody->get_HandRightState(&handState);
	if (handState == HandState_Closed) {

		if (rightHand_closed) {
			if (mode == 0) {
				if(std::abs(hand_pos_x - handCoords[0]) > THRESHOLD_DISTANCE_SWAP_CANDIDATES) {
					gestureType = SWAP_CANDIDATES;
					timer = std::time(nullptr);
				}
				else if (std::time(nullptr) - timer > THRESHOLD_TIMER) {
					gestureType = SELECT;
				}
				else {
					gestureType = SELECTION_PROGRESS;
				}
			}
			else {
				gestureType = PANNING;
			}
			
		}
		else {
			gestureType = PANNING;
			timer = std::time(nullptr);
			//Store hand position x
			hand_pos_x = handCoords[0];
		}
		
		if (mode == 1) {
			pBody->get_HandLeftState(&handState);
			if (handState == HandState_Closed) {
				if (!leftHand_closed) {
					leftHand_closed = true;
					//Store distance
					Joint joints[JointType_Count];
					hr = pBody->GetJoints(_countof(joints), joints);
					Joint rhj = joints[JointType_HandRight];
					Joint lhj = joints[JointType_HandLeft];
					hand_distance = sqrt((rhj.Position.X - lhj.Position.X)*(rhj.Position.X - lhj.Position.X) + (rhj.Position.Y - lhj.Position.Y)*(rhj.Position.Y - lhj.Position.Y));
				}
			}
			else {
				if (leftHand_closed) {
					leftHand_closed = false;
					Joint joints[JointType_Count];
					hr = pBody->GetJoints(_countof(joints), joints);
					Joint rhj = joints[JointType_HandRight];
					Joint lhj = joints[JointType_HandLeft];
					float d = sqrt((rhj.Position.X - lhj.Position.X)*(rhj.Position.X - lhj.Position.X) + (rhj.Position.Y - lhj.Position.Y)*(rhj.Position.Y - lhj.Position.Y));
					if (d - hand_distance < 0) {
						gestureType = ZOOM_OUT;
						Sleep(500);
					}
					else {
						gestureType = ZOOM_IN;
						Sleep(500);
					}
				}
			}
		}
		
		//Set rightHand closed state true
		rightHand_closed = true;
	}
	else {
		//Detect no gesture 
		gestureType = NO_GESTURE;
		hand_pos_x = 0;
		rightHand_closed = false;

		if (mode == 1) {
			// ZOOM IN/OUT
			if (leftHand_closed) {
				leftHand_closed = false;
				Joint joints[JointType_Count];
				hr = pBody->GetJoints(_countof(joints), joints);
				Joint rhj = joints[JointType_HandRight];
				Joint lhj = joints[JointType_HandLeft];
				float d = sqrt((rhj.Position.X - lhj.Position.X)*(rhj.Position.X - lhj.Position.X) + (rhj.Position.Y - lhj.Position.Y)*(rhj.Position.Y - lhj.Position.Y));
				if (d - hand_distance < 0) {
					gestureType = ZOOM_OUT;
					Sleep(500);
				}
				else {
					gestureType = ZOOM_IN;
					Sleep(500);
				}
			}
		}
	}
}

char * KinectSensor::getGestureType() {
	switch (gestureType)
	{
	case NO_GESTURE:
		return "NO_GESTURE";
	case PANNING:
		return "PANNING";
	case SELECT:
		return "SELECT";
	case SWAP_CANDIDATES:
		return "SWAP_CANDIDATES";
	case ZOOM_IN:
		return "ZOOM_IN";
	case ZOOM_OUT:
		return "ZOOM_OUT";
	case SELECTION_PROGRESS:
		return "SELECTION_PROGRESS";
	default:
		return nullptr;
	}
}