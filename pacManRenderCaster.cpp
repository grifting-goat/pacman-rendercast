//Raycasting pac man style game //thanks to the guy on the internet who taught me how to rayCast
//WASD to move, arrows L and R to change direction
//space for ice skateing
//terminal should be at what ever size the nScreen variables are

//required libs
#pragma comment(lib, "User32.lib")

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <thread>
#include <unordered_set>
using namespace std;

#include <stdio.h>
#include <Windows.h>

//flags
bool isRunning = false;
bool nMap = true;
bool isScared = false;

//int

//game
const int nScreenX = 230;
const int nScreenY = 60;
int nMapX = 28;
int nMapY = 36;
const int nGhostCount = 4;
int nHeartX = 26;
int nHeartY = 9;


//player
int nMass = 70; //kg
int nScore = 0;
int nMaxLives = 3;
int nLives = 3;

//floats

//game
float fDepth = 20.0f;
float g = 9.8f;
float fStep = 0.1f;
float fFudge = 0.0f;
float fError = 0.0f;
float fGhostRange = 12.0f;
float fGhostFudge = 0.2f;
float nTimer = 0;

//Player
float fPX = 2.0f;
float fPY = 2.0f;
float fPA = 0.0f;
float fFov = 1.5f;
float fSense = 2.2f;
float fPMew = 0.65f;
float fPMewBase = 0.65f;
float fForce = 700.0f;
float fForceBase = 700.0f;
float fMomentum = 0.0f;

//vectors
float fMomentumArr[3] = {0.0f, 0.0f, 0.0f};
float fForceVec[3] = {0.0f, 0.0f, 0.0f};
float fBadPos[nGhostCount*2] = {18.0f, 3.0f, 7.0f, 7.0f, 30.0f, 6.0f, 32.0f, 20.0f};
int nRand[nGhostCount] = {rand()%4};

//functiions
bool isClose(float px, float py, float bx, float by, float threshold) {
    float tSquared = threshold * threshold;
    float dx = px - bx;
    float dy = py - by;
    return (dx * dx + dy * dy) < tSquared;
}
int mapAngleToValue(float angle) { //used to map the ghost angles to NWSE system
    if (angle < 0) angle += 2.0f * 3.14159265f;
    if (angle >= 2.0f * 3.14159265f) angle -= 2.0f * 3.14159265f;

    if (angle >= 0 && angle < 3.14159265f / 2.0f) {
        return 0;
    } else if (angle >= 3.14159265f / 2.0f && angle < 3.14159265f) {
        return 1;
    } else if (angle >= 3.14159265f && angle < 3.0f * 3.14159265f / 2.0f) {
        return 2;
    } else if (angle >= 3.0f * 3.14159265f / 2.0f && angle < 2.0f * 3.14159265f) {
        return 3;
    } else {
        // This should never happen if the angle is correctly normalized
        return -1;
    }
}

void hInput(float fTick) { //handles player inputs

    //handles force scaling
    for (int i = 0; i < 2; i++) {
        fForceVec[i] = 1.0f - (abs(fMomentumArr[i])/(fForce));//force factor
    }
    
    //Grabs which keys are pressed and updates momentum accordingly
    if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
        for (int i = 0; i < 2; i++) {
            fMomentumArr[i] += cosf(fPA - ((3.1416/2.0)*i)) * fForce * fForceVec[i] * fTick;
        }
    }
    if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
        for (int i = 0; i < 2; i++) {
            fMomentumArr[i] -= cosf(fPA - ((3.1416/2.0)*i)) * fForce * fForceVec[i] * fTick;
        }
    }
    if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
        for (int i = 0; i < 2; i++) {
            fMomentumArr[i] += cosf(fPA - ((3.1416/2.0)*(i+1))) * fForce * fForceVec[i] * fTick;
        }
    }
    if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
        for (int i = 0; i < 2; i++) {
            fMomentumArr[i] -= cosf(fPA - ((3.1416/2.0)*(i+1))) * fForce * fForceVec[i] * fTick;
        }
    }

    //changes the player angle if requested
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
       fPA += fSense * fTick;

    }
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
       fPA -= fSense * fTick;

    }

    //ice skating??
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        fPMew = fPMewBase * 0.05f;
        fForce = 280.0f;
    }
    else {
        fPMew = fPMewBase;
        fForce = fForceBase;
    }
}

void collision(wstring map, float fTick) { //handles player collisions with walls
    if (map.c_str()[(int)fPX * nMapX + (int)fPY] == '#') {
            //x
            fMomentumArr[0] *= -1; //flip
            fPX +=fMomentumArr[0] * fTick; //move
            if (map.c_str()[(int)fPX * nMapX + (int)fPY] == '#') { //checks
                fMomentumArr[0] *= -1; //if bad unflip
                fPX +=fMomentumArr[0] * fTick; //and unmove
            }
            else {
                fPX -=fMomentumArr[0] * fTick; // if good unmove
            }
            //y
            fMomentumArr[1] *= -1;
            fPY +=fMomentumArr[1] * fTick;
            if (map.c_str()[(int)fPX * nMapX + (int)fPY] == '#') {
                fMomentumArr[1] *= -1;
                fPY +=fMomentumArr[1] * fTick;
            }
            else {
                fPY -=fMomentumArr[1] * fTick;
            }
    }
}

void ghostMovement(float fTick, int s, float f, wstring map) { //handles ghost movement/AI
    //s refers to the ghost number // f refers to the ghost speed
    float fMag = 0.0f;
    bool bLOS = false; //Line of sight? yes no
    bool bHit = false; //hit a thing? y or n
    //vetor stuff // abba // vec from a to b = b - a
    float fX = fPX-fBadPos[(2*s)];
    float fY = fPY-fBadPos[(2*s)+1];
    float fA = atan2f(fY,fX); //angle from ghost to player

    float fGX = cosf(fA); //basis vecs
    float fGY = sinf(fA);

    //cast ray
    while (!bHit && (fMag < fGhostRange)) {
        fMag += fStep; //make ray longer
        float fTestRayX = (float)(fBadPos[(2*s)] + (fGX * fMag)); //new ray position
        float fTestRayY = (float)(fBadPos[(2*s)+1] + (fGY * fMag));
        if (fTestRayX < 0.0f || fTestRayY < 0.0f || fTestRayX > (float)(nMapY) || fTestRayY > (float)(nMapX)) {
                bHit = true; //out of bounds
        }
        else if((map.c_str()[(int)fTestRayX * nMapX + (int)fTestRayY] == '#')) {bHit = true;} //stop if hits wall
        else if(isClose(fTestRayX, fTestRayY, fPX, fPY, fGhostFudge)) {bHit = true; bLOS = true;} //stop if hits player, update flag
    }

    //determind behavior
    if (!bLOS) { // if pacman cannot see player
        float fAngle = nRand[s] * (3.1415f/2.0); //point in random direction
        float fBaseX = cosf(fAngle); //basis vecs
        float fBaseY = sinf(fAngle);
        float fTest = f;
        fBadPos[(2*s)] += fBaseX*fTest*fTick; //move
        fBadPos[(2*s)+1] += fBaseY*fTest*fTick;
        if(map.c_str()[(int)fBadPos[(2*s)] * nMapX + (int)fBadPos[(2*s+1)]] == '#') { //if hit a wall
            fBadPos[(2*s)] -= fBaseX*fTest*fTick; //go back
            fBadPos[(2*s)+1] -= fBaseY*fTest*fTick;
            fBadPos[(2*s)] -= fBaseX*0.3f; //go back even more
            fBadPos[(2*s)+1] -= fBaseY*0.3f;
            nRand[s] = rand()%4; //change direction
            ghostMovement(fTick, s, f, map); //try again
        }
    }
    else  { //if he can
        f *= 1.35f; //ghost gets excited(so starts moving faster)
        vector<float> fDistVec; //holds the different distances
        fA = 0.0f; //starting angle
        for (int i = 0; i < 16; i++) { //loops through 2pi with 16 steps //checks all 16 positions to determine which is the closest to the player;
            float fBaseX = cosf(fA);//basis vecs
            float fBaseY = sinf(fA);
            fBadPos[(2*s)] += fBaseX*f*fTick; //update positon
            fBadPos[(2*s)+1] += fBaseY*f*fTick;
            float fDist = (pow(fPX - fBadPos[(2*s)],2)+pow(fPY - fBadPos[(2*s)+1],2)); //calculates the distance to player
            if(map.c_str()[(int)(fBadPos[(2*s)]) * nMapX + (int)(fBadPos[(2*s)+1])] == '#') {fDist = 1000.0f;} //check if is wall //sets distance to something absurdly high
            fDistVec.push_back(fDist);//adds it to vector
            fBadPos[(2*s)] -= fBaseX*f*fTick;//unupdate position
            fBadPos[(2*s)+1] -= fBaseY*f*fTick;
            fA += (3.142f/8.0f); // updates angle
        }

        //goofy code which should probably be illegal
        
        //returns the index of the shortest/longest distance
        int index = isScared ? max_element(fDistVec.begin(), fDistVec.end()) - fDistVec.begin() : min_element(fDistVec.begin(), fDistVec.end()) - fDistVec.begin();

        fA = (3.142f/8.0f) * (float)index; //back tracks to get corrisponding angle
        nRand[s] = mapAngleToValue(fA); //convers to NWSE in case player breaks LOS
        float fBaseX = cosf(fA); // basis vecs and stuff
        float fBaseY = sinf(fA);
        fBadPos[(2*s)] += fBaseX*f*fTick; //update position
        fBadPos[(2*s)+1] += fBaseY*f*fTick;
    }
}

void rayCaster(wchar_t *frame, wstring Map, float fTick) {
    for(int i = 0; i < nScreenX; i++) { // shots out a ray for ever vertical line on the screen
        //load map in c string
        const wchar_t* map = Map.c_str();
        //handle main ray
        float fRayMag = 0.0f; //maginitude of the ray
        float fRayA = (fPA - fFov/2.0f) + ((float)i / (float)nScreenX) * fFov; // sends out a ray for each pixel
        float fRX = fPX; //initial camera start
        float fRY = fPY;
        int nHit = 0; // determines what was hit

        //normalize angle
        fRayA = fmod(fRayA, 2.0f * 3.14159f);
        if (fRayA < 0) {
            fRayA += 2.0f * 3.14159f;
        }

        //Handle ghosts
        float fGhostDiff[nGhostCount] = {0};
        bool bGhost[nGhostCount] = {0};
        float fGhostMag[nGhostCount] = {0.0f};  //depth(ray magnitude) per ghost

        //handle coins
        vector<float> fCoinMag; //depth(ray magnitude) per coin
        unordered_set<int> nCoinIndexSet;
        vector<int> nCoinIndex;
        bool bCoin = false;
        
        //handle fruit/powerups
        float fFruitMag = 0.0f;

        //corners
        bool nCorner = 0; //is a corner, yes no
        
        //basis vectors
        float fBaseX = cosf(fRayA);
        float fBaseY = sinf(fRayA);
        
        while (!nHit) {
            //check if the Magnitude of the ray is greater than the render distance
            if (fRayMag > fDepth) {
                nHit = 1; //stop if ray to big
            }

            fRayMag += fStep; // incriment ray magnitude

            //calculate the x and y position of the test ray
            float fTestRayX = (float)(fRX + (fBaseX * fRayMag));
            float fTestRayY = (float)(fRY + (fBaseY * fRayMag));

            //move the camera is outta bounds // for cool portal like effect thing
            if (fTestRayY < 0.0f) {
                fRY = nMapX+fRayMag;
            }
            else if (fTestRayY > nMapX) {
                fRY = 0-fRayMag;
            }
            else if (fTestRayX < 0.0f) {
                fRX = nMapY+fRayMag;
            }
            else if (fTestRayX > nMapY) {
                fRX = 0-fRayMag;
            }
            else {
                //other colisions
                // ghosts
                for (int g = 0; g < nGhostCount; g++) { //cycle throuhg all ghosts
                    if(isClose(fBadPos[(g*2)], fBadPos[(g*2)+1], fTestRayX, fTestRayY, 0.3f)) {
                        if (!bGhost[g]) {
                            fGhostMag[g] = fRayMag;
                            float fX = fBadPos[(2*g)]-fRX;
                            float fY = fBadPos[(2*g)+1]-fRY;
                            float fA = atan2f(fY,fX); //angle from cam to ghost

                            //normalize
                                fA = fmod(fA, 2.0f * 3.14159f);
                                if (fA < 0) {
                                    fA += 2.0f * 3.14159f;
                                }

                            fGhostDiff[g] = fabs(fA-fRayA); //checks how aligned the two angles are
                            fError =  fGhostDiff[g];
                        }
                        bGhost[g] = 1;
                    }
                }
                // Coins
                int nCoinI = (int)fTestRayX * nMapX + (int)fTestRayY;
                if (map[nCoinI] == '.' && nCoinIndexSet.find(nCoinI) == nCoinIndexSet.end()) {
                    if(fabs(fTestRayX - ((int)fTestRayX + 0.5f)) < 0.1f && fabs(fTestRayY - ((int)fTestRayY + 0.5f)) < 0.1f) {
                        nCoinIndexSet.insert(nCoinI);
                        nCoinIndex.push_back(nCoinI);
                        fCoinMag.push_back(fRayMag);
                        bCoin = true;
                    }
                }
                // fruit
                if (map[nCoinI] == 'B' && fabs(fTestRayX-(float)((int)fTestRayX+0.5)) < 0.2f && fabs(fTestRayY-(float)((int)fTestRayY+0.5)) < 0.2f &&!fFruitMag) {
                    fFruitMag = fRayMag;
                }
                
                //test for wall colision
                if(map[(int)fTestRayX * nMapX + (int)fTestRayY] == '#') {
                    nHit = 1;
                    //calculate the dot product from the corners of the walls
                    vector<pair<float, float>> pVec;
                    for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++) {
								float vy = (int)fTestRayY + ty - fRY;
								float vx = (int)fTestRayX + tx - fRX;
								float fMag = sqrt(vx*vx + vy*vy); 
								float fDot = (fBaseX * vx / fMag) + (fBaseY * vy / fMag);
                                pVec.push_back(make_pair(fMag, fDot));
							}
                    //no clue how this works had to copy it from the interwebs      
                    sort(pVec.begin(), pVec.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });
                    float fBound = 0.008f;
                    //make the most aligned(closer to 1) into corners
					if (acos(pVec.at(0).second) < fBound) nCorner = true;
					if (acos(pVec.at(1).second) < fBound) nCorner = true;
                }

            }
        }
        fRayMag *= (cosf(fRayA-fPA)); //correct fisheye

        //Calculate the top and bottom of objects
        int nTop = (float)(nScreenY/2.0f) - nScreenY / ((float)fRayMag); //top of wall
        int nBottom = nScreenY - nTop; //bottom of wall

        int nBadTop[nGhostCount] = {0}; //vectors of the ghosts top and bottom
        int nBadBottom[nGhostCount] = {0};

        for (int g = 0; g < nGhostCount; g++) {
            float f = 40.0f;
            nBadTop[g] = ((f/(float)fGhostMag[g]) + (float)(nScreenY/2.0f) - nScreenY / ((float)fGhostMag[g])); //sets ghosts top and bottoms
            nBadBottom[g] = (nScreenY - nBadTop[g] + (f/fGhostMag[g]) + ((rand()%2) ? 0.6f / (fGhostMag[g]/fDepth) : 0));
        }

        //Coin vectors
        vector<int> nBTop(nCoinIndex.size(), 0);
        vector<int> nBBottom(nCoinIndex.size(), 0);
        for (int c = 0; c < nCoinIndex.size(); c++) {
            if (bCoin) {
                float f = 80.0f;
                nBTop[c] = (f/fCoinMag[c]) + (float)(nScreenY/2.0f) - nScreenY / ((float)fCoinMag[c]);
                nBBottom[c] = nScreenY - nBTop[c] + 0.6f*(f/fCoinMag[c]) ;
            }
        }

        //fruit
        int nFTop = (25.0f/fFruitMag)+ (float)(nScreenY/2.0f) - nScreenY / ((float)fFruitMag); //top of wall
        int nFBottom = nScreenY - nFTop - (30.0f/fFruitMag); //bottom of wall

        

        //determine which shade to use based on distance
        short nShadeB[nGhostCount] = {' '}; // for the ghosts
        vector<short> nShadeC(nCoinIndex.size(), '$'); // for coins
        short nShadeF = ' '; // for fruit
        short nShadeM = ' '; // for the walls
        

        //Handle wall shading
        if (nCorner)                            nShadeM = ' ';      //is corner
        else if (fRayMag <= fDepth / 4.5f)		nShadeM = 0x2588;	//nice and close
        else if (fRayMag < fDepth / 3.5f)		nShadeM = 0x2593;
        else if (fRayMag < fDepth / 2.5f)		nShadeM = 0x2592;
        else if (fRayMag < fDepth / 1.5f)		nShadeM = 0x2591;
        else if (fRayMag < fDepth / 0.5f)		nShadeM = 0x00B7;
        else									nShadeM = ' '; // far away

        //handle ghost shading
        for (int g = 0; g < nGhostCount; g++) {
           if (bGhost[g]) {
                if (fGhostMag[g] <= fDepth / 5.5f)     nShadeB[g] = (rand()%2) ? 8272 : '0'; //cool switching effect
                else if (fGhostMag[g] < fDepth / 4.5f) nShadeB[g] = (rand()%2) ? '*' : 216;
                else if(fGhostMag[g] < fDepth / 3.0f)  nShadeB[g] = (rand()%2) ? ' ' : 'O';
                else if(fGhostMag[g] < fDepth / 2.0f)  nShadeB[g] = (rand()%3) ? ' ' : ':';
                else if(fGhostMag[g] < fDepth )        nShadeB[g] = (rand()%4) ? ' ' : '|';
                }
        }

        for(int c = 0; c < nCoinIndex.size(); c++) {
            if(fCoinMag[c] < fDepth / 4.0f)            nShadeC[c] = '$';
            else if(fCoinMag[c] < fDepth / 3.0f)       nShadeC[c] = '!';
            else if(fCoinMag[c] < fDepth / 2.0f)       nShadeC[c] = '*';
            else if(fCoinMag[c] < fDepth)              nShadeC[c] = ' ';
        }
        //Berry Shading
        if(fFruitMag < fDepth / 4.0f)                 nShadeF = '@';
            else if(fFruitMag  < fDepth / 3.0f)       nShadeF = 'B';
            else if(fFruitMag  < fDepth / 2.0f)       nShadeF = 'b';
            else if(fFruitMag  < fDepth)              nShadeF = ' ';
        
        //render the strip
        for (int j = 0; j < nScreenY; j++) {
            //make objects in map && floor && ceiling
            if(j <= nTop) {
                frame[j*(nScreenX) + i] = ' '; //make the ceiling
            }
            else if (j > nTop && j <= nBottom && nHit == 1) {
                frame[j*(nScreenX) + i] = nShadeM; // make the walls
            }
            else { // make floor
                float fFloorDist = 1.0f - (((float)j -nScreenY/2.0f) / ((float)nScreenY / 2.0f));
                if (fFloorDist < 0.25)		nShadeM = '#';
                else if (fFloorDist < 0.5)	nShadeM = 'x';
                else if (fFloorDist < 0.75)	nShadeM = '.';
                else if (fFloorDist < 0.9)	nShadeM = '-';
                else				        nShadeM = ' ';
                //set the floor shade
                frame[j*(nScreenX) + i] = nShadeM;
            }

            //make all coins
            for (int c = 0; c < nCoinIndex.size(); c++) {
                if (j > nBTop[c] && j <= nBBottom[c] && bCoin) {
                    frame[j*(nScreenX) + i] = nShadeC[c];
                }
            }

            //make all ghosts
            for (int g = 0; g < nGhostCount; g++) {
                if (j > nBadTop[g] && j <= nBadBottom[g] && bGhost[g]) {
                    frame[j*(nScreenX) + i] = nShadeB[g]; // make the ghost
                    if ((fabs(fGhostDiff[g]) > 0.015f && fabs(fGhostDiff[g]) < 0.065f)) {
                        int m = (nBadTop[g]+nBadBottom[g])/2;
                        int t = 0.5f / (fGhostMag[g]/fDepth);
                        if (j > m-(2*t) && j <= m+t)  {
                            frame[j*(nScreenX) + i] = ' ';
                        }
                    }
                }
            }
            if (j > nFTop && j <= nFBottom) {
                frame[j*(nScreenX) + i] = nShadeF;
            }

        }
    }
}

wstring makeMap() { //makes the map, really just wanted to hide this code away
    wstring map;
        map += L"############################";
        map += L"#............##............#";
        map += L"#..#####.....##.....#####..#";
        map += L"#..#.........##.........#..#";
        map += L"#..#...##....##....##...#..#";
        map += L"#......##..........##......#";
        map += L"#..........................#";
        map += L"#....B...##########........#";
        map += L"#............##...........B#";
        map += L"#...#........##........#...#";
        map += L"#...#........##........#...#";
        map += L"#...#...#....##....#...#...#";
        map += L"#.......#..........#.......#";
        map += L"#.......####....####.......#";
        map += L"######..#..........#..######";
        map += L"     #..#..........#..#     ";
        map += L"     #..#..##..##..#..#     ";
        map += L"######..#..#....#..#..######";
        map += L"...........#....#...........";
        map += L"######..#..######..#..######";
        map += L"     #..#..........#..#     ";
        map += L"     #..#..........#..#     ";
        map += L"######..#..........#..######";
        map += L"#.......####....####.......#";
        map += L"#.......#..........#.......#";
        map += L"#...#..B#....##....#...#...#";
        map += L"#...#........##........#...#";
        map += L"#...#........##........#...#";
        map += L"#........##########........#";
        map += L"#......................B...#";
        map += L"#..#....##.........##...#..#";
        map += L"#..#....##...##....##...#..#";
        map += L"#..#.........##.........#..#";
        map += L"#..#####.....##.....#####..#";
        map += L"#............##............#";
        map += L"############################";
        return map;
}

void Overlay(wstring map, wstring heart, float fTick, wchar_t* frame) {
    fMomentum = sqrt(pow(fMomentumArr[0],2) + pow(fMomentumArr[1],2) + pow(fMomentumArr[2],2) );
    int angle = fPA/3.1416*180.0; //covert to degrees
    angle %= 360;
    if (angle < 0) {
        angle = 360 + angle;
    }
    //display info
    swprintf_s(frame, nScreenX, L"X=%3.2f, Y=%3.2f, Angle=%d, FPS=%3.2f, P=%3.2f PX=%3.2f, PY=%3.2f, Mew=%3.2f, Score= %d" , fPX, fPY, angle, 1.0f/fTick, fMomentum,fMomentumArr[0], fMomentumArr[1], fPMew,nScore);

    //draw Mini Map
    if (nMap) {
        int i = 0;
        int j = 0;
        for (int nx = 0; nx < nMapX; nx++)
            for (int ny = 0; ny < nMapY; ny++)
            {
                frame[(ny+1)*nScreenX + nx] = map[(ny * nMapX + nx)];
            }
        frame[((int)fPX+1) * nScreenX + (int)fPY] = '@';
        for (int k = 0; k < nGhostCount; k++) {
            frame[((int)(fBadPos[(2*k)]+1)) * nScreenX + (int)fBadPos[(2*k)+1]] = '&';
        }
        double b = 22.5;
        
        if (angle >= 45-b && angle <= 135+b) {
            j = 1;
        }
        if (angle >= 225-b && angle <= 315+b) {
            j = -1;
        }
        if (angle >= 135-b && angle <= 225+b) {
            i = -1;
        }
        if (angle > 315-b || angle < 45+b) {
            i = 1;
        }
        frame[((int)fPX+1+i) * (nScreenX) + (int)(fPY)+j] = '*';
    }
    for (int h = 0; h < (nLives); h++) {
        for (int nx = 0; nx < nHeartX; nx++)
            for (int ny = 0; ny < nHeartY; ny++) {
                if (heart[(ny * nHeartX + nx)] == '#') {
                    frame[((ny+1)*nScreenX) + nScreenX - nx -(h*nMapX)] = 8709;
                }
                
            }
    }

}

void friction(float fTick) { //handle how the players momentum is effected by friction
    for (int i = 0; i < 2; i++) {
        if (abs(fMomentumArr[i]) > 0.05f) { //check if significant
            fMomentumArr[i] += fPMew * (g*(float)nMass) * fTick * (fMomentumArr[i] > 0 ? -1 : 1); //unreadible syntax but clever i think
        }
        else {
            fMomentumArr[i] = 0; //if not significant //make it nothing
        }
    }
}

void mapScrolling() { //handle moving things from one side of map to other 
    //player
    if (fPX > (nMapY)) {
    fPX = 0;
    }
    if (fPY > nMapX) {
        fPY = 0;
    }
    if (fPX < 0) {
        fPX = nMapY;
    }
    if (fPY < 0) {
        fPY = nMapX;
    }
    //ghosts
    for (int g = 0; g < nGhostCount; g++) {
        if (fBadPos[(2*g)] > (nMapY)) {
            fBadPos[(2*g)] = 0;
        }
        if (fBadPos[(2*g)+1] > nMapX) {
            fBadPos[(2*g)+1]= 0;
        }
        if (fBadPos[(2*g)] < 0) {
            fBadPos[(2*g)] = nMapY;
        }
        if (fBadPos[(2*g)+1] < 0) {
            fBadPos[(2*g)+1] = nMapX;
        }
    }

}

void coinControl(wstring& map) {
    if (map.c_str()[(int)fPX * nMapX + (int)fPY] == '.') {
        map[(int)fPX * nMapX + (int)fPY] = ' ';
        nScore++;
    }
}

void berryControl(wstring& map, float fTick) {
    if (map.c_str()[(int)fPX * nMapX + (int)fPY] == 'B') {
        map[(int)fPX * nMapX + (int)fPY] = ' ';
        isScared = true;
        nTimer = 0;
        nScore += 10;
    }
    if (isScared) {
        nTimer += fTick;
        if (nTimer > 12) {
            isScared = false;
            nTimer = 0;
        }
    }
}


wstring makeHeart() {
    wstring heart;
        heart += L"....#####........#####....";
        heart += L"..######################..";
        heart += L"..######################..";
        heart += L"..######################..";
        heart += L"....##################....";
        heart += L"......##############......";
        heart += L"........##########........";
        heart += L".........########.........";
        heart += L"...........####...........";
       
    return heart;
}

int main() {
    // Create Frame  // goofy windows stuff
	wchar_t *frame = new wchar_t[nScreenX*nScreenY];
	HANDLE hCmd = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hCmd);
	DWORD dwBytesWritten = 0;

    //Base map
    wstring map = makeMap();

    //heart picture
    wstring heart = makeHeart();

    //init timeing
    auto tp0 = chrono::system_clock::now();
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    //tell the game to start running
    isRunning = true;

    //main game loop
    while (isRunning) {

        //handle timing
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fTick = elapsedTime.count();

        //scrolling
        mapScrolling();
        
        //contols
        hInput(fTick);

        //handle friction
        friction(fTick); 

        //handle collision
        collision(map, fTick);

        //update position
        fPX += fMomentumArr[0]/nMass * fTick;
        fPY += fMomentumArr[1]/nMass * fTick;

        //jew stuff
        coinControl(map);
        berryControl(map, fTick);

        //handle ghost
        for (int g = 0; g < nGhostCount; g++) {
            ghostMovement(fTick, g, 1.6f, map); //1.7f is the base ghost speed
            // end game if ghost colide
            
            if(isClose(fBadPos[(g*2)], fBadPos[(g*2)+1], fPX, fPY, 0.2f)) { //death and lives and stuff
                if (isScared) {
                    nScore += 25;
                    fBadPos[(g*2)] = 17;
                    fBadPos[(g*2)+1] = 12;
                }
                else {
                    nLives --;
                    if (!nLives) {isRunning = false; break;}
                    fBadPos[(g*2)] = 17.0f;
                    fBadPos[(g*2)+1] = 12.0f;
                    for (int i = 0; i < nScreenX*nScreenY; i++) {frame[i] = ' ';}
                    //render death screen
                    frame[nScreenX * nScreenY - 1] = '\0'; //windows really needs the end character
                    WriteConsoleOutputCharacterW(hCmd, frame, nScreenX * nScreenY, { 0,0 }, &dwBytesWritten);
                    Sleep(1000);
                    fPX = 2.0f;
                    fPY = 2.0f;
                    fPA = 0.0f;
                    for (int n = 0; n < 3 ; n++) {
                        fMomentumArr[n] = 0.0f;
                    }
                    break;
                }
                
            }
            
        }

        //handle 3d projection
        rayCaster(frame, map,fTick);

        //Handle Overlay
        Overlay(map, heart, fTick, frame);

        // Display Frame
        frame[nScreenX * nScreenY - 1] = '\0'; //windows really needs the end character
        WriteConsoleOutputCharacterW(hCmd, frame, nScreenX * nScreenY, { 0,0 }, &dwBytesWritten);
    }

    //display msg when game over
    for (int i = 0; i < nScreenX*nScreenY; i++) {frame[i] = ' ';}
    //render death screen
    frame[nScreenX * nScreenY - 1] = '\0'; //windows really needs the end character
    WriteConsoleOutputCharacterW(hCmd, frame, nScreenX * nScreenY, { 0,0 }, &dwBytesWritten);

    std::cout << "gg" << endl;
    std::cout << "Score: " << nScore << endl;
    chrono::duration<float> fTimer = tp2 - tp0;
    double fTime = fTimer.count();
    std::cout << "Time: " << fTime << " seconds" << endl;
    
    //sleep so ppl can read the score and stuff
    //Sleep(10000);

    //return 0 so pc is happy
    return 0;
}