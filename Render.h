#ifndef Render_H
#define Render_H

#include "sgct.h"
#include "Webserver.h"
#include "UserData.h"
#include "Quad.h"
#include "Simulation.h"
#include "Game.h"

#include <iostream>

class Render {
	public:
		void myInitFun();
		void myDrawFun();
		void myPreSyncFun();
		void myPostSyncFun();
		void myEncodeFun();
		void myDecodeFun();
		void myCleanUpFun();
		void keyCallback(int key, int action);

		void renderAvatars();
		void renderSkyBox();
		void renderConnectionInfo();
		void renderBalls();
		void renderFootball();
		void renderGoal();

	protected:

};

#endif  // Render_H