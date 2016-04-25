#pragma once

#include <Kinect.VisualGestureBuilder.h>
#include <Kinect.h>
#include <iostream>
#include <list>
#include <stdio.h>

#include "renderer.h"

#define FRAME_DEPTH 0
#define FRAME_COLOR 1
#define FRAME_INFRARED 2
#define FRAME_BODY 3

#define VIRTUAL_RECTANGLE_CORNER_L_X 0.15
#define VIRTUAL_RECTANGLE_CORNER_L_Y 0.65

#define VIRTUAL_RECTANGLE_CORNER_R_X 0.35
#define VIRTUAL_RECTANGLE_CORNER_R_Y 0.25

#define THRESHOLD_DISTANCE_SWAP_CANDIDATES 0.005
#define THRESHOLD_DISTANCE_ZOOMING 0.1
#define THRESHOLD_TIMER 1

#define NO_GESTURE 0
#define SELECT 1
#define PANNING 2
#define ZOOM_IN 4
#define ZOOM_OUT 5
#define SELECTION_PROGRESS 6

#define BUFFER_SIZE 10

class KinectSensor {

private:
  IKinectSensor *pKinectSensor;

  IDepthFrameReader *pDepthFrameReader;
  IColorFrameReader *pColorFrameReader;
  IInfraredFrameReader *pInfraredFrameReader;
  IBodyFrameReader *pBodyFrameReader;
  IVisualGestureBuilderFrameReader
      *pVisualGestureBuilderFrameReader[BODY_COUNT];

  ICoordinateMapper *m_pCoordinateMapper;
  IVisualGestureBuilderDatabase *pGestureDatabase;

  void error(std::string e, HRESULT hr);

  

public:
  KinectSensor();
  ~KinectSensor();

  IDepthFrameReader *getDepthFrameReader();
  IColorFrameReader *getColorFrameReader();
  IInfraredFrameReader *getInfraredFrameReader();
  IBodyFrameReader *getBodyFrameReader();
  ICoordinateMapper *getCoordinateMapper();

  IVisualGestureBuilderDatabase *GestureDatabase;
  IGesture *pGesture;

  static Renderer::DisplayMode mode;
  static int gestureType;
  static int timer;
  static float hand_distance;
  static float hand_pos_x;
  static float hand_pos_y;
  static float panning_delta_x;
  static float panning_delta_y;
  static float zoom_delta;
  static bool rightHand_closed;
  static bool leftHand_closed;

  /* Returns the next frame from the depth reader */
  IDepthFrame *getNextDepthFrame();

  /* Returns the next frame from the color reader */
  IColorFrame *getNextColorFrame();

  /* Returns the next frame from the depth reader */
  IInfraredFrame *getNextInfraredFrame();

  /* Returns the next frame from the depth reader */
  IBodyFrame *getNextBodyFrame();

  /* Updates the hand position variable. (Put this into a thread)*/
  static void updateHandPosition();
  static void mapHandToCursor(float *handPosition, int screenWidth,
                              int screenHeight, int *cursor);
  static float *handCoords;
  static bool KeepUpdatingHandPos;
  static float getHandsDistance(IBody *pBody);

  /* Gesture builder functions */
  static void updateGesture(IBody *pBody);
  static std::string getGestureType();
  /* Releases any Kinect interface */
  template <class Interface> void SafeRelease(Interface *&pInterface); // TODO

  static std::list<float> handPosXBuf, handPosYBuf, handPosZBuf;
  static std::list<float> spinePosXBuf, spinePosYBuf, spinePosZBuf;

  static void addHandJoint(Joint handJoint);
  static void addSpineJoint(Joint spineJoint);

  static float getHandJointPosX();
  static float getHandJointPosY();
  static float getHandJointPosZ();

  static float getSpineJointPosX();
  static float getSpineJointPosY();
  static float getSpineJointPosZ();
};
