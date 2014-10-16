/*
Copyright (c) 2014 Miroslav Andel IXEL AB
miroslav.andel@ixel.se
All rights reserved.
*/

#ifndef _USER_DATA_
#define _USER_DATA_

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

    void setTeam(int id); // decides team depending on the id
    void getTeam();
    
    inline float getPhi() { return mPhi; }
    inline float getTheta() { return mTheta; }
    inline float getTimeStamp() { return mTimeStamp; }
    //inline UserColor getColor() { return mColor; }
    inline float getRed() { return mRed; }
    inline float getGreen() { return mGreen; }
    inline float getBlue() { return mBlue; }
	inline float getS() { return s; }
	inline float getT() { return t; }
    
		
private:
	float mTheta, mPhi;
    float mTimeStamp;
    float mRed, mGreen, mBlue;
	float s, t;

    int team; // if we have to teams to be able to compete. 
};

#endif