/*
Copyright (c) 2014 Miroslav Andel IXEL AB
miroslav.andel@ixel.se
All rights reserved.
*/

#include "UserData.h"
#include <stdio.h> //printf
#include <math.h>

#define CANVAS_SIZE 500.0f
#define DEG90_TO_RAD 1.570796327f
#define DEG180_TO_RAD 3.141592654f

UserData::UserData()
{
    mTheta = 0.0f;
    mPhi = DEG180_TO_RAD;
    //mColor = RED;
    mRed = 1.0f;
    mGreen = 0.0f;
    mBlue = 0.0f;
    mTimeStamp = -1.0f;
}

UserData::~UserData()
{
}

void UserData::setColor(float r, float g, float b){
    mRed = r;
    mGreen = g;
    mBlue = b;
}

void UserData::set(float theta, float phi, float r, float g, float b, float timeStamp)
{
    mTheta = theta;
    mPhi = phi;
    //mColor = color;
    mTimeStamp = timeStamp;
    mRed = r;
    mGreen = g;
    mBlue = b;
}

float lastX;
float lastY;
bool firstRun = true;
float speed = 0.03;

void UserData::setCartesian2d(int x, int y, float timeStamp)
{
	float s, t;

	if (firstRun) {
		s = (static_cast<float>(x) / CANVAS_SIZE)*2.0f - 1.0f;
		t = (static_cast<float>(y) / CANVAS_SIZE)*2.0f - 1.0f;

		lastX = static_cast<float>(x);
		lastY = static_cast<float>(y);

		firstRun = false;
	}
	else {
		float diffX = static_cast<float>(x) - lastX;
		float diffY = static_cast<float>(y) - lastY;

		float length = sqrt(diffX*diffX + diffY*diffY);

		float difference = speed / length;
		
		if (difference > 1.0) {
			s = (static_cast<float>(x) / CANVAS_SIZE)*2.0f - 1.0f;
			t = (static_cast<float>(y) / CANVAS_SIZE)*2.0f - 1.0f;
		}
		else {
			float newX = lastX + (difference*diffX);
			float newY = lastY +(difference*diffY);

			s = (newX / CANVAS_SIZE)*2.0f - 1.0f;
			t = (newY / CANVAS_SIZE)*2.0f - 1.0f;

			lastX = newX;
			lastY = newY;
		}
		
	}
	
	float r2 = s*s + t*t;
    if( r2 <= 1.0f )
    {
        mPhi = sqrt(r2) * DEG90_TO_RAD;
        mTheta = atan2f(s,t);
    }
    
    //fprintf(stderr, "phi: %f theta: %f\n", mPhi, mTheta);
    
    //mColor = color;
   /* mRed = r;
    mGreen = g;
    mBlue = b;*/
    mTimeStamp = timeStamp;
}
