//Raycasting pac man style game //thanks to the guy on the internet who taught me how to rayCast
//WASD to move, arrows L and R to change direction
//space for ice skateing
// and up and down to place or remove walls
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
#include <queue>
#include <map>
using namespace std;

#include <stdio.h>
#include <Windows.h>

//flags
bool isRunning = false;
bool nMap = true;
bool isScared = false;
int nGhostState = 1;

//mouse control variables
POINT lastMousePos = {0, 0};
bool mouseInitialized = false;
float mouseSensitivity = 0.0009f;
int mouseAccumX = 0;  // Accumulate small movements

//init
void ghostMovement(float fTick, int s, float f, wstring map);

//game
int nScreenX = 160;
int nScreenY = 50;
const int nMapX = 28;
const int nMapY = 31;
const int nGhostCount = 1;         
int nHeartX = 26;
int nHeartY = 9;


//player
int nMass = 70; //kg
int nScore = 0;
int nMaxLives = 3;
int nLives = 3;
int gunEquipped = 1;

//floats

//game
float fDepth = 20.0f;
float g = 9.8f;
float fStep = 0.1f;
float fFudge = 0.0f;
float fError = 0.0f;
float fGhostRange = 12.0f;
float fGhostFudge[4] = {0.01f, 4.0f,0,0};
float nTimer = 0;

//Player
float fPX = 1.0f;
float fPY = 1.0f;
float fPA = 0.0f;
float fFov = 1.8f;
float fSense = 2.2f;
float fPMew = 0.65f;
float fPMewBase = 0.65f;
float fForce = 700.0f;
float fForceBase = 700.0f;
float fMomentum = 0.0f;

//arr
int offX[8] = {0, 1, 0, -1, 1, 1, -1, -1};
int offY[8] = {1, 0, -1, 0, 1, -1, 1, -1};
float fMomentumArr[3] = {0.0f, 0.0f, 0.0f};
float fForceVec[3] = {0.0f, 0.0f, 0.0f};
//float fBadPos[nGhostCount*2] = {25.0f, 3.0f, 7.0f, 7.0f, 25.0f, 6.0f, 25.0f, 20.0f};
float fBadPos[nGhostCount*2] = {15.0f};
float fBadAngle[nGhostCount] = { 0 };
int nRand[nGhostCount] = {0};
pair<int, int> fBadTarget[nGhostCount] = {{0,0}};

vector<pair<int, int>> test;

bool checkResized(HANDLE hCmd) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hCmd, &csbi);

    int testWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int testHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    if (testWidth != nScreenX || testHeight != nScreenY) {return true;}

    return false;
}

void resize(HANDLE hCmd, wchar_t* &frame) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hCmd, &csbi)) {
        CloseHandle(hCmd);
        throw std::runtime_error("Failed to get console screen buffer info");
    }

    delete[] frame;
    nScreenX = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    nScreenY = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    frame = new wchar_t[nScreenX * nScreenY];
}

//functions
bool valid(const wchar_t* map, bool visit[nMapY][nMapX], int cellX, int cellY) {
    if(cellX >= nMapX || cellX < 0) {return false;}
    if(cellY >= nMapY || cellY < 0) {return false;}

    if(visit[cellY][cellX] || map[(cellY*nMapX) + cellX] == '#') {
        return false;
    }

    

    return true;
}

//breadth first search 
pair<int , int> ghostBFS(wstring mapp, int startX, int startY, int targetX, int targetY) {
    if (startX == targetX && startY == targetY) {return {-10, -10};}
    const wchar_t* map = mapp.c_str();
    queue<pair<int, int>> que;
    std::map<pair<int, int>, pair<int, int>> parentCell;
    pair<int, int> index;
    bool visit[nMapY][nMapX] = {false};
    bool bFound = false;
    

    que.push({startY, startX});
    visit[startY][startX] = true;

    test.clear();

    while(!que.empty()){
        pair<int, int> cell = que.front();
        int cellY = cell.first;
        int cellX = cell.second;
    
        que.pop();

        for (int i = 0; i < 8; i++) {
            int testX = cellX + offX[i];
            int testY = cellY + offY[i];

            if(testX >= nMapX) {testX -= nMapX;}
            if(testX < 0) {testX += nMapX;}
            if(testY >= nMapY) {testY -= nMapY;}
            if(testY < 0) {testY += nMapY;}

            if(valid(map, visit, testX, testY)) {
                que.push({testY, testX});
                visit[testY][testX] = true;
                //new position is key and the parent cell is the value
                parentCell.insert({{testY, testX}, {cellY, cellX}});
                if(testY == targetY && testX == targetX) {
                    queue<pair<int, int>> empty;
                    swap(que, empty);
                    index = {testY, testX};
                    bFound = true;
                }
            } 
        }
    }
    if (bFound) {
        bool bTrack = true;
        pair<int, int> ghostIndex;
        int runs = 0;

        while(bTrack) {
            runs++;
            ghostIndex = index;
            index = parentCell[index];
            test.push_back(index);
            if (index.first == startY && index.second == startX) {
                if (runs == 1) {return {-10, -10};}
                bTrack = false;
            }
        }
        return ghostIndex;
    }
    return {-9,-9};
}

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

    // Handle mouse look - completely new approach
    POINT currentMousePos;
    if (GetCursorPos(&currentMousePos)) {
        if (!mouseInitialized) {
            lastMousePos = currentMousePos;
            mouseInitialized = true;
        }
        else {
            // Calculate raw mouse movement
            int deltaX = currentMousePos.x - lastMousePos.x;
            int deltaY = currentMousePos.y - lastMousePos.y;
            
            mouseAccumX += deltaX;
            
            // Apply rotation when we have enough accumulated movement
            if (abs(mouseAccumX) >= 1) {
                fPA += mouseAccumX * mouseSensitivity;
                mouseAccumX = 0; // Reset accumulator
            }
            
            // Update last position
            lastMousePos = currentMousePos;
            
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
            // Reset to screen center
            int centerX = screenWidth / 2;
            int centerY = screenHeight / 2;
            SetCursorPos(centerX, centerY);
            lastMousePos.x = centerX;
            lastMousePos.y = centerY;

        }
    }

    //handles force scaling
    for (int i = 0; i < 2; i++) {
        fForceVec[i] = 1.0f - (abs(fMomentumArr[i])/(fForce));//force factor
    }
    
    //Grabs which keys are pressed and updates momentum accordingly
    if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
        fMomentumArr[0] += cosf(fPA) * fForce * fForceVec[0] * fTick;
        fMomentumArr[1] += sinf(fPA) * fForce * fForceVec[1] * fTick;
    }
    if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
        fMomentumArr[0] -= cosf(fPA) * fForce * fForceVec[0] * fTick;
        fMomentumArr[1] -= sinf(fPA) * fForce * fForceVec[1] * fTick;
    }
    if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
        fMomentumArr[0] += cosf(fPA - (3.1416/2.0)) * fForce * fForceVec[0] * fTick;
        fMomentumArr[1] += sinf(fPA - (3.1416/2.0)) * fForce * fForceVec[1] * fTick;
    }
    if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
        fMomentumArr[0] -= cosf(fPA - (3.1416/2.0)) * fForce * fForceVec[0] * fTick;
        fMomentumArr[1] -= sinf(fPA - (3.1416/2.0)) * fForce * fForceVec[1] * fTick;
    }

    //changes the player angle if requested (keep as fallback)
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
       fPA += fSense * fTick;

    }
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
       fPA -= fSense * fTick;

    }

    //mouse buttons
    // Left click for ray gun
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        // Will use existing ray gun functionality
    }
    
    // Right click for ice skating
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
        fPMew = fPMewBase * 0.05f;
        fForce = 280.0f;
    }
    else if (!(GetAsyncKeyState(VK_SPACE) & 0x8000)) {
        fPMew = fPMewBase;
        fForce = fForceBase;
    }

    //ice skating with space
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        fPMew = fPMewBase * 0.05f;
        fForce = 280.0f;
    }
}

void collision(wstring map, float fTick) { //handles player collisions with walls
    if (map.c_str()[(int)fPY * nMapX + (int)fPX] == '#') {
            //x
            fMomentumArr[0] *= -1; //flip
            fPX +=fMomentumArr[0] * fTick; //move
            if (map.c_str()[(int)fPY * nMapX + (int)fPX] == '#') { //checks
                fMomentumArr[0] *= -1; //if bad unflip
                fPX +=fMomentumArr[0] * fTick; //and unmove
            }
            else {
                fPX -=fMomentumArr[0] * fTick; // if good unmove
            }
            //y
            fMomentumArr[1] *= -1;
            fPY +=fMomentumArr[1] * fTick;
            if (map.c_str()[(int)fPY * nMapX + (int)fPX] == '#') {
                fMomentumArr[1] *= -1;
                fPY +=fMomentumArr[1] * fTick;
            }
            else {
                fPY -=fMomentumArr[1] * fTick;
            }
    }
}

void ghostController(float fTick, int s, float f, wstring map) {
    float tarX;
    float tarY;
    //not finished at all
    switch (s) {
        case 0: //red
            if (isScared) {
                tarX = 15.0f;
                tarY = 15.0f;
            }
            else {
                tarX = fPX;
                tarY = fPY;
            }
            break;
        case 1: //pink
            tarX = fPX;
            tarY = fPY;
            int nFudge = mapAngleToValue(fPA);
            float baseX = cosf(nFudge*(3.1415/2.0));
            float baseY = sinf(nFudge*(3.1415/2.0));
            for (int i = 0; i < 4; i++) {
                if (map.c_str()[(int)tarY * nMapX + (int)tarX] == '#') {
                    tarX -= baseX;
                    tarY -= baseY;
                }
                else {
                    tarX += baseX;
                    tarY += baseY;
                }
            }

            break;
    }

    fBadTarget[s] = ghostBFS(map, fBadPos[(2*s)], fBadPos[(2*s)+1], tarX, tarY);
    if (fBadTarget[s].first == -10) { //if it is one tile away from player
        float fX = fPX-fBadPos[(2*s)];
        float fY = fPY-fBadPos[(2*s)+1];
        float fA = atan2f(fY,fX); //angle from ghost to player

        float fGX = cosf(fA); //basis vecs
        float fGY = sinf(fA);

        fBadPos[(2*s)] += fGX*f*fTick; //update position
        fBadPos[(2*s)+1] += fGY*f*fTick;
    }
    else if (fBadTarget[s].first != -9) { //move than 1 tile and able to reach it
        if (fBadTarget[s].first == nMapX-1 || fBadTarget[s].second == nMapY-1 || fBadTarget[s].first == 0 || fBadTarget[s].second == 0) {
 
        }
        else {
            fBadAngle[s] = atan2f((fBadTarget[s].first + 0.5f) - fBadPos[(2*s)+1], (fBadTarget[s].second + 0.5f) - fBadPos[(2*s)]);
        }
        float fBaseX = cosf(fBadAngle[s]); // basis vecs and stuff
        float fBaseY = sinf(fBadAngle[s]);
        fBadPos[(2*s)] += fBaseX*f*fTick; //update position
        fBadPos[(2*s)+1] += fBaseY*f*fTick;
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
        else if((map.c_str()[(int)fTestRayY * nMapX + (int)fTestRayX] == '#')) {bHit = true;} //stop if hits wall
        else if(isClose(fTestRayX, fTestRayY, fPX, fPY, fGhostFudge[s])) {bHit = true; bLOS = true;} //stop if hits player, update flag
    }

    //determind behavior
    if (!bLOS) { // if pacman cannot see player
        float fAngle = nRand[s] * (3.1415f/2.0); //point in random direction
        float fBaseX = cosf(fAngle); //basis vecs
        float fBaseY = sinf(fAngle);
        float fTest = f;
        fBadPos[(2*s)] += fBaseX*fTest*fTick; //move
        fBadPos[(2*s)+1] += fBaseY*fTest*fTick;
        if(map.c_str()[(int)fBadPos[(2*s)+1] * nMapX + (int)fBadPos[(2*s)]] == '#') { //if hit a wall
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
            if(map.c_str()[(int)(fBadPos[(2*s)+1]) * nMapX + (int)(fBadPos[(2*s)])] == '#') {fDist = 1000.0f;} //check if is wall //sets distance to something absurdly high
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
            //wierd error correction thing
            bool bFix1 = 0;
            bool bFix2 = 0;


            //move the camera is outta bounds // for cool portal like effect thing
            if (fTestRayY < 0.0f) {
                //fRY = nMapY+fRayMag;
                fRY += nMapY;
            }
            if (fTestRayY > (float)nMapY) {
                //fRY = 0.0f-fRayMag;
                bFix1 = 1;
                fRY -= nMapY;
            }
            if (fTestRayX < 0.0f) {
                //fRX = nMapX+fRayMag;
                fRX += nMapX;
                
            }
            if (fTestRayX > (float)nMapX) {
                //fRX = 0.0f-fRayMag;
                bFix2 = 1;
                fRX -= nMapX;
            }
            if (bFix1) {
                fTestRayY = (float)(fRY + (fBaseY * fRayMag));
            }
            if (bFix2) {
                fTestRayX = (float)(fRX + (fBaseX * fRayMag));
            }

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
            int nCoinI = (int)fTestRayY * nMapX + (int)fTestRayX;
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
            if(map[(int)fTestRayY * nMapX + (int)fTestRayX] == '#') {
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
        fRayMag *= (cosf(fRayA-fPA)); //correct fisheye

        //Calculate dynamic aspect ratio for consistent rendering
        float fAspectRatio = (float)nScreenX / (float)nScreenY;
        float fHorizonLine = nScreenY * (0.5f + (0.05f * fAspectRatio));

        //Calculate the top and bottom of objects
        int nTop = fHorizonLine - nScreenY / ((float)fRayMag); //top of wall
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
                float f = nScreenY*1.3f;
                nBTop[c] = (f/fCoinMag[c]) + (float)(nScreenY/2.0f) - nScreenY / ((float)fCoinMag[c]);
                nBBottom[c] = nScreenY - nBTop[c] + 0.6f*(f/fCoinMag[c]) ;
            }
        }

        //fruit
        int nFTop = (40.0f/fFruitMag)+ (float)(nScreenY/1.9f) - nScreenY / ((float)fFruitMag); //top of wall
        int nFBottom = nScreenY - nFTop;// - (10.0f/fFruitMag); //bottom of wall

        

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
           if (bGhost[g] && !isScared) {
                if (fGhostMag[g] <= fDepth / 5.5f)     nShadeB[g] = (rand()%2) ? 8272 : '0'; //cool switching effect
                else if (fGhostMag[g] < fDepth / 4.5f) nShadeB[g] = (rand()%2) ? '*' : 216;
                else if(fGhostMag[g] < fDepth / 3.0f)  nShadeB[g] = (rand()%2) ? ' ' : 'O';
                else if(fGhostMag[g] < fDepth / 2.0f)  nShadeB[g] = (rand()%3) ? ' ' : ':';
                else if(fGhostMag[g] < fDepth )        nShadeB[g] = (rand()%4) ? ' ' : '|';
            }
            else {
                nShadeB[g] = (rand()%3) ? ' ' : '0';
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

            //make all the fruits
            if (j > nFTop && j <= nFBottom) {
                frame[j*(nScreenX) + i] = nShadeF;
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
            
        }
    }
}

wstring makeOldMap() { //makes the map, really just wanted to hide this code away
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
        frame[((int)fPY+1) * nScreenX + (int)fPX] = '@';
        for(auto coord : test) {
            //frame[(coord.first+1) * nScreenX + coord.second] = '+';
        }
        for (int k = 0; k < nGhostCount; k++) {
            frame[((int)(fBadPos[(2*k)+1]+1)) * nScreenX + (int)fBadPos[(2*k)]] = '&';
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
        frame[((int)fPY+1+i) * (nScreenX) + (int)(fPX)+j] = '*';
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
    if (fPX > (nMapX)) {
        fPX = 0;
    }
    if (fPY > nMapY) {
        fPY = 0;
    }
    if (fPX < 0) {
        fPX = nMapX;
    }
    if (fPY < 0) {
        fPY = nMapY;
    }
    //ghosts
    for (int g = 0; g < nGhostCount; g++) { //works for now
        if (fBadPos[(2*g)] > nMapX) {
            fBadPos[(2*g)] = 0;
        }
        if (fBadPos[(2*g)+1] > nMapY) {
            fBadPos[(2*g)+1]= 0;
        }
        if (fBadPos[(2*g)] < 0.0f) {
            fBadPos[(2*g)] = nMapX;
        }
        if (fBadPos[(2*g)+1] < 0.0f) {
            fBadPos[(2*g)+1] = nMapY;
        }
    }

}

void coinControl(wstring& map) {
    if (map.c_str()[(int)fPY * nMapX + (int)fPX] == '.') {
        map[(int)fPY * nMapX + (int)fPX] = ' ';
        nScore++;
    }
}

void berryControl(wstring& map, float fTick) {
    if (map.c_str()[(int)fPY * nMapX + (int)fPX] == 'B') {
        map[(int)fPY * nMapX + (int)fPX] = ' ';
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

wstring makeMap() { //makes the map, really just wanted to hide this code away
    wstring map;
    map += L"############################";
    map += L"#............##............#";
    map += L"#.####.#####.##.#####.####.#";
    map += L"#B####.#####.##.#####.####B#";
    map += L"#.####.#####.##.#####.####.#";
    map += L"#..........................#";
    map += L"#.####.##.########.##.####.#";
    map += L"#.####.##.########.##.####.#";
    map += L"#......##....##....##......#";
    map += L"######.##### ## #####.######";
    map += L"     #.##### ## #####.#     ";
    map += L"     #.##          ##.#     ";
    map += L"     #.## ###  ### ##.#     ";
    map += L"######.## #      # ##.######";
    map += L"      .   #      #   .      ";
    map += L"######.## #      # ##.######";
    map += L"     #.## ######## ##.#     ";
    map += L"     #.##          ##.#     ";
    map += L"     #.## ######## ##.#     ";
    map += L"######.## ######## ##.######";
    map += L"#............##............#";
    map += L"#.####.#####.##.#####.####.#";
    map += L"#.####.#####.##.#####.####.#";
    map += L"#B..##................##..B#";
    map += L"###.##.##.########.##.##.###";
    map += L"###.##.##.########.##.##.###";
    map += L"#......##....##....##......#";
    map += L"#.##########.##.##########.#";
    map += L"#.##########.##.##########.#";
    map += L"#..........................#";
    map += L"############################";

    return map;
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

void rayGun(float fTick, wstring& Map) {
    bool shootWall = (GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
    bool buildWall = (GetAsyncKeyState(VK_DOWN) & 0x8000);
    
    if (gunEquipped && (shootWall || buildWall)) {
        const wchar_t* map = Map.c_str();
        float fRayMag = 0.0f;
        float fRA = fPA;
        float fBX = cosf(fRA);
        float fBY = sinf(fRA);
        float fRX = fPX;
        float fRY = fPY;
        bool bEnd = 0;
        while (fRayMag < 10.0f && !bEnd) {
            fRayMag += fStep;
            float fPosX = (fRX + (fBX * fRayMag));
            float fPosY = (fRY + (fBY * fRayMag));

            if (fPosY< 0.0f) {
                fRY = nMapY+fRayMag;
            }
            else if (fPosY > nMapY) {
                fRY = 0-fRayMag;
            }
            if (fPosX< 0.0f) {
                fRX = nMapX+fRayMag;
            }
            else if (fPosX > nMapX) {
                fRX = 0-fRayMag;
            }
            if(map[(int)fPosY * nMapX + (int)fPosX] == '#' && shootWall) {
                Map[(int)fPosY * nMapX + (int)fPosX] = ' ';
                bEnd = 1;
            }
            if((map[(int)fPosY * nMapX + (int)fPosX] == ' ' || map[(int)fPosY * nMapX + (int)fPosX] == '.') && buildWall && fRayMag > 3.0f)  {
                Map[(int)fPosY * nMapX + (int)fPosX] = '#';
                bEnd = 1;
            }


                
        }

    }
}

int main() {
    // Create Frame  // goofy windows stuff
	wchar_t *frame = new wchar_t[nScreenX*nScreenY];
	HANDLE hCmd = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hCmd);
	DWORD dwBytesWritten = 0;

    // Hide console cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hCmd, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hCmd, &cursorInfo);

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

        if (checkResized(hCmd)) {resize(hCmd, frame);}

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
        //pushinP(map);

        //jew stuff
        coinControl(map);
        berryControl(map, fTick);

        //handle ghost
        for (int g = 0; g < nGhostCount; g++) {
            
            ghostController(fTick, g, 1.8f, map); //1.7f is the base ghost speed

            // end game if ghost colide
            
            if(isClose(fBadPos[(g*2)], fBadPos[(g*2)+1], fPX, fPY, 0.3f)) { //death and lives and stuff
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

        //unadd player indicator from map
        //unpushinP(map);
        
        //handle ray gun 
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            rayGun(fTick, map);
        } else {
            rayGun(fTick, map); // Keep existing up/down arrow functionality
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

    // Restore console cursor
    CONSOLE_CURSOR_INFO restoreCursor;
    GetConsoleCursorInfo(hCmd, &restoreCursor);
    restoreCursor.bVisible = TRUE;
    SetConsoleCursorInfo(hCmd, &restoreCursor);

    std::cout << "gg" << endl;
    std::cout << "Score: " << nScore << endl;
    chrono::duration<float> fTimer = tp2 - tp0;
    double fTime = fTimer.count();
    std::cout << "Time: " << fTime << " seconds" << endl;
    std::cout << "type anything to quit" << endl;
    string thing;
    cin >> thing;

    //return 0 so pc is happy
    return 0;
}