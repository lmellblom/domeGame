/*
Copyright (c) 2014 Miroslav Andel IXEL AB
miroslav.andel@ixel.se
All rights reserved.
*/

#ifndef _USER_DATA_
#define _USER_DATA_
#include <btBulletDynamicsCommon.h>
#include "sgct.h" // need glm from here

class UserData
{
public:
    //enum UserColor { RED = 0, YELLOW, GREEN, CYAN, BLUE };
    
	UserData();
	~UserData();
    void set(float theta, float phi, float r, float g, float b, float timeStamp);
    void setCartesian2d(int x, int y, float timeStamp);
    void setColor(float r, float g, float b); 
	void setTimeStamp(float t);
    void setPlayerDirection(btQuaternion dir); 

    void setPingTime(float t);
    void setPingPosition(glm::vec3 pos) ;


    btVector3 calculatePosition();

    void setTeam(int id); // decides team depending on the id
    int getTeam();
    
    inline float getPhi() { return mPhi; }
    inline float getTheta() { return mTheta; }
    inline float getTimeStamp() { return mTimeStamp; }
    //inline UserColor getColor() { return mColor; }
    inline float getRed() { return mRed; }
    inline float getGreen() { return mGreen; }
    inline float getBlue() { return mBlue; }
	inline float getS() { return s; }
	inline float getT() { return t; }
    inline btQuaternion getPlayerDirection() {return direction; }
    bool exists; 

    inline float getPingTime(){return pingedTime;}
    inline glm::vec3 getPingPosition(){return pingedPosition;}
		
private:
	float mTheta, mPhi;
    float mTimeStamp;
    float mRed, mGreen, mBlue;
	float s, t;
    btQuaternion direction; 

    // for the ping
    float pingedTime; 
    glm::vec3 pingedPosition; 

    int team; // if we have to teams to be able to compete. 
};

#endif