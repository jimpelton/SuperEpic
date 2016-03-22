#pragma once
#include <stdio.h>
#include <iostream>

#include "KinectSensor.h"

float * KinectSensor::handCoords{ new float[3] };
bool KinectSensor::KeepUpdatingHandPos{ true };

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




	while (KeepUpdatingHandPos) {
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


