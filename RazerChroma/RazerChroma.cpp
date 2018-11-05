
#include "RazerChroma.h"
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <Audioclient.h>
#include <time.h>
#include <string>
#include <fstream>
#include <windows.h>


INIT Init;
UNINIT UnInit;
CREATEEFFECT CreateEffect;
CREATEKEYBOARDEFFECT CreateKeyboardEffect;
CREATEMOUSEEFFECT CreateMouseEffect;
CREATEHEADSETEFFECT CreateHeadsetEffect;
CREATEMOUSEPADEFFECT CreateMousepadEffect;
CREATEKEYPADEFFECT CreateKeypadEffect;
SETEFFECT SetEffect;
DELETEEFFECT DeleteEffect;
QUERYDEVICE QueryDevice;

bool RazerChroma::initialize() {
	HMODULE chromaSDKModule = LoadLibrary(CHROMASDKDLL);
	if (chromaSDKModule == nullptr) {
		std::cout << "Chroma sdk module fail to load to library." << std::endl;
		return FALSE;
	}

	if (Init == nullptr)
	{
		auto Result = RZRESULT_INVALID;
		Init = reinterpret_cast<INIT>(GetProcAddress(chromaSDKModule, "Init"));
		if (Init) {
			Result = Init();
			if (Result == RZRESULT_SUCCESS) {
				CreateEffect = reinterpret_cast<CREATEEFFECT>(GetProcAddress(chromaSDKModule, "CreateEffect"));
				CreateKeyboardEffect = reinterpret_cast<CREATEKEYBOARDEFFECT>(GetProcAddress(chromaSDKModule, "CreateKeyboardEffect"));
				CreateMouseEffect = reinterpret_cast<CREATEMOUSEEFFECT>(GetProcAddress(chromaSDKModule, "CreateMouseEffect"));
				CreateHeadsetEffect = reinterpret_cast<CREATEHEADSETEFFECT>(GetProcAddress(chromaSDKModule, "CreateHeadsetEffect"));
				CreateMousepadEffect = reinterpret_cast<CREATEMOUSEPADEFFECT>(GetProcAddress(chromaSDKModule, "CreateMousepadEffect"));
				CreateKeypadEffect = reinterpret_cast<CREATEKEYPADEFFECT>(GetProcAddress(chromaSDKModule, "CreateKeypadEffect"));
				SetEffect = reinterpret_cast<SETEFFECT>(GetProcAddress(chromaSDKModule, "SetEffect"));
				DeleteEffect = reinterpret_cast<DELETEEFFECT>(GetProcAddress(chromaSDKModule, "DeleteEffect"));
				QueryDevice = reinterpret_cast<QUERYDEVICE>(GetProcAddress(chromaSDKModule, "QueryDevice"));

				if (CreateEffect &&
					CreateKeyboardEffect &&
					CreateMouseEffect &&
					CreateHeadsetEffect &&
					CreateMousepadEffect &&
					CreateKeypadEffect &&
					SetEffect &&
					DeleteEffect &&
					QueryDevice)
				{
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

COLORREF led1[95], led2[19]; //row: 6, col: 20, 16
int score;
int time1 = 0;

//spectrum lighting - rbg values
int x = 255, y = 0, z = 0, storeX = 0, storeY = 255, storeZ = 255;
void rgbChange() {
	if (x >= 255 && y < 255 && z == 0) { y++; storeY--; }
	else if (storeX < 255 && y >= 255 && z < 255) { x--; storeX++; }
	else if (x < 255 && y >= 255 && z < 255) { z++; storeZ--; }
	else if (x < 255 && storeY < 255 && z >= 255) { storeY++; y--; }
	else if (x < 255 && y < 255 && z >= 255) { storeX--; x++; }
	else if (x >= 255 && y < 255 && storeZ < 255) { storeZ++; z--; }
}


//pull the background
void pull() {
	for (int i = 0; i < 31; i++) {
		if (i < 16) {
			led1[i] = led1[i + 1];
		}
		else {
			led1[i + 64] = led1[i + 65];
		}
	}
}

//pull the obstical
void obsticals(bool obstical) {
	if (obstical) {
		led1[30] = RGB(255, 235, 0);
		//led1[30] = RGB(255, 0 , 0);
	}

	if (led1[18] == RGB(255, 235, 0)) {
		if (score != 9) score++;
		std::cout << "                             [+]      " << "Lifes: " << score << "              Play time: " << time1 / 833 << " sec" << std::endl;
		if (led1[19] == RGB(0, 255, 255)) {
			led1[17] = led1[18];
			led1[18] = RGB(0, 0, 0);
		}
	}

	for (int i = 0; i < 15; i++) {
		if (led1[i + 17] != RGB(0, 255, 255)) {
			led1[i + 16] = led1[i + 17];
		}
	}
}

int timer = 0, timer2 = 0, timer3 = 0, timer4 = 0, timer5 = 0, timer6 = 0, timer7 = 0;

ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE layer1 = {};


bool RazerChroma::keyboardEffect() {

	for (int row = 0; row < 6; row++) {
		for (int col = 0; col < 16; col++) {
			int d = (16 * (5 - row)) + col - 1;
			layer1.Color[row][col] = led1[d];
		}
	}



	RZRESULT resultKeyboard1 = CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &layer1, nullptr);
	//RZRESULT resultKeyboard2 = CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &layer2, nullptr);

	return TRUE;
}

bool jumping = false, storeJumping = false, falling = false;
int i = 19;
bool buf1 = true;
void jump() {


	if (jumping == true && buf1) {
		storeJumping = true;
		buf1 = false;
		i = 34;
		led1[19] = RGB(0, 0, 0);
	}
	else if (i == 34 && falling == false) { i = 50; led1[34] = RGB(0, 0, 0); falling = true; }
	else if (i == 50) { i = 34; led1[50] = RGB(0, 0, 0); }
	else if (i == 34 && falling == true) { i = 19; led1[34] = RGB(0, 0, 0); falling = false; buf1 = true; }

}

bool failed = false;
void fail() {
	for (int s = 0; s < 15; s++) {
		led1[s + 64] = RGB(255, 0, 0);
	}
	failed = true;
}


COLORREF w = RGB(255, 255, 255), a = RGB(0, 0, 0);
//set and initialize the background;
void set(int r, int g, int b) {
	led1[94] = RGB(r, g, b);
	led1[15] = RGB(r, g, b);

	if (led1[i] == RGB(255, 235, 0)) {
		fail();

		score--;
		std::cout << "                             [-]      " << "Lifes: " << score << "              Play time: " << time1 / 833 << " sec" << std::endl;
	}
	led1[i] = RGB(0, 255, 255);

	if (score == 0) { layer1.Color[5][19] = w; layer1.Color[4][18] = a; }
	else if (score == 1) { layer1.Color[4][18] = w; layer1.Color[4][19] = a; layer1.Color[5][19] = a; }
	else if (score == 2) { layer1.Color[4][19] = w; layer1.Color[4][20] = a; layer1.Color[4][18] = a; }
	else if (score == 3) { layer1.Color[4][20] = w; layer1.Color[3][18] = a; layer1.Color[4][19] = a; }
	else if (score == 4) { layer1.Color[3][18] = w; layer1.Color[3][19] = a; layer1.Color[4][20] = a; }
	else if (score == 5) { layer1.Color[3][19] = w; layer1.Color[3][20] = a; layer1.Color[3][18] = a; }
	else if (score == 6) { layer1.Color[3][20] = w; layer1.Color[2][18] = a; layer1.Color[3][19] = a; }
	else if (score == 7) { layer1.Color[2][18] = w; layer1.Color[2][19] = a; layer1.Color[3][20] = a; }
	else if (score == 8) { layer1.Color[2][19] = w; layer1.Color[2][20] = a; layer1.Color[2][18] = a; }
	else if (score == 9) { layer1.Color[2][20] = w; layer1.Color[2][19] = a; }


}




RazerChroma rzchroma;

bool think = FALSE;
int gap = 1200;
bool storage = true;
bool paused = false;
int level = 220;
int gap1 = 550;

int frequency_of_primes(int n) {
	int i, j;
	int freq = n - 1;
	for (i = 2; i <= n; ++i) for (j = sqrt(i); j > 1; --j) if (i%j == 0) { --freq; break; }
	return freq;
}


void run() {

	if (failed) timer6++;
	timer++;
	timer2++;
	timer3++;
	timer4++;
	timer5++;
	if (timer > 1600) {
		x = 0;
		y = 0;
		z = 0;
		timer = 0;
	}
	if (timer3 > gap) {
		think = false;
		timer3 = 0;
	}
	if (timer2 > 100) {
		//rzchroma.keyboardEffect();
		pull();
		timer2 = 0;
	}

	if (timer5 > 180) {
		jump();
		timer5 = 0;
		jumping = false;
	}
	if (timer4 > level) {
		obsticals(think);
		timer4 = 0;
	}
	if (timer6 > 400) {
		for (int s = 0; s < 15; s++) {
			led1[s + 64] = RGB(0, 0, 0);
		}
		timer6 = 0;
		failed = false;
	}
	if (x == 0 && y == 0 && z == 0 && timer > 300) {
		x = 0; y = 255; z = 0;
	}
	if (think == false && timer3 > (gap - 1)) {
		think = true;
		gap = rand() % gap1 + 650;
	}
	set(x, y, z);
	time1++;
}



bool running = true;

int main()
{
	//std::ifstream Reader("Speedstor Run.txt");             //Open file

	std::string Art = "    ___________________________________________________________________________________________________\n   /                                                                                                   \\ \n   |      _________                        .___        __                 __________                   |\n   |     /   _____/_____   ____   ____   __| _/_______/  |_  ___________  \\______   \\__ __  ____       |\n   |     \\_____  \\\\____ \\_/ __ \\_/ __ \\ / __ |/  ___/\\   __\\/  _ \\_  __ \\  |       _/  |  \\/    \\      |\n   |     /        \\  |_> >  ___/\\  ___// /_/ |\\___ \\  |  | (  <_> )  | \\/  |    |   \\  |  /   |  \\     |\n   |    /_______  /   __/ \\___  >\\___  >____ /____  > |__|  \\____/|__|     |____|_  /____/|___|  /     |\n   |            \\/|__|        \\/     \\/     \\/    \\/                              \\/           \\/      |\n   |                                                                                                   |\n   |                                                                                                   |\n   |                                           Need to know:                                           |\n   |           The main objective of the game is basically to dodge the orange obsticales              |\n   |           and gain points. Points will be displayed on the numpad, and if you don't               |\n   |           have a numpad, DON'T FRET, you can just simply read the console itself.                 |\n   |                                                                                                   |\n   |                              Press arrow keys or SPACEBAR to jump.                                |\n   |                                         And Esc to pause.                                         |\n   |                                                                                                   |\n   |                                           Point system:                                           |\n   |           You gain a point when you jump over an obstical, and loses a point when                 |\n   |           get hit. Points could be added up to 9, and you lose the game when you                  |\n   |           dip below 0, wish you the best of luck.                                                 |\n   |                                                                                                   |\n   \\___________________________________________________________________________________________________/\n\n\n";



	//getFileContents(Reader);       //Get file

	std::cout << Art << std::endl;               //Print it to the screen

	//Reader.close();                           //Close file


	if (rzchroma.initialize()) {
		clock_t t;
		int f;

		while (running) {
			if ((time1 / 833) > 32) { level = 215; gap1 = 400; }
			if ((time1 / 833) > 64) { level = 190; gap1 = 350; }
			if ((time1 / 833) > 78) { level = 182; gap1 = 290; }
			if ((time1 / 833) > 100) { level = 130; gap1 = 220; }
			if ((time1 / 833) > 120) { level = 98; }
			if ((time1 / 833) > 180) { level = 95; }
			if ((time1 / 833) > 200) { level = 90; }
			if ((time1 / 833) > 220) { level = 80; }
			if ((time1 / 833) > 300) { level = 70; }


			if (GetKeyState(VK_ESCAPE) < -1 && storage) {
				paused = !paused;
				storage = false;

			}



			if (GetKeyState(VK_RIGHT) < -1 || GetKeyState(VK_UP) < -1 || GetKeyState(VK_LEFT) < -1 || GetKeyState(VK_DOWN) < -1 || GetKeyState(VK_SPACE) < -1) {
				jumping = true;
				//std::cout << jumping << std::endl;
			}

			t = clock();
			//printf("Calculating...\n");
			f = frequency_of_primes(99999);
			//printf("The number of primes lower than 100,000 is: %d\n", f);
			t = clock() - t;
			//printf("It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);

			if (!paused) {
				while (t >= 1) {
					rzchroma.keyboardEffect();
					run();
					t--;
				}
			}

			storage = true;

			if (score < 0) { running = false; }
		}

	}
	else std::cout << "Failed to Setup Razer Chroma" << std::endl;

	//std::ifstream la("Try Again.txt");             //Open file

	std::string wa = "\n\n______________________________________________________________________________________________________________\n\n\n    __     __               _ _          _           _       _                                  _         \n    \\ \\   / /              | (_)        | |         | |     | |                                (_)        \n     \\ \\_/ ___  _   _    __| |_  ___  __| |    _ __ | |____ | |_ _ __ _   _    __ _  __ _  __ _ _ _ __    \n      \\   / _ \\| | | |  / _` | |/ _ \\/ _` |   | '_ \\| |_  / | __| '__| | | |  / _` |/ _` |/ _` | | '_ \\   \n       | | (_) | |_| | | (_| | |  __| (_| |_  | |_) | |/ /  | |_| |  | |_| | | (_| | (_| | (_| | | | | |_ \n       |_|\\___/ \\__,_|  \\__,_|_|\\___|\\__,_( ) | .__/|_/___|  \\__|_|   \\__, |  \\__,_|\\__, |\\__,_|_|_| |_(_)\n                                          |/  | |                      __/ |         __/ |                \n                                              |_|                     |___/         |___/                 \n											  \n______________________________________________________________________________________________________________\n";

	std::cout << wa << std::endl << std::endl << "                                  [+]    " << "Total play time: " << time1 / 833 << " sec   [+]\n" << std::endl << "______________________________________________________________________________________________________________" << std::endl;               //Print it to the screen
	//
	//la.close();                           //Close file

	std::system("PAUSE");

	return 0;
}