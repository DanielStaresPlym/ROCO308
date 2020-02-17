/*
Phil Culverhouse Oct 2016 (c) Plymouth University
James Rogers Jan 2020     (c) Plymouth University

This demo code will move eye and neck servos with kepresses.
Use this code as a base for your assignment.

*/

#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "owl-pwm.h"
#include "owl-comms.h"
#include "owl-cv.h"
#include "neck_pan.h"
#include "eye_scan.h"
#include "chameleon.h"
#include "eye_roll.h"
#include "annoyed_eye_roll.h"
#include "point_focus.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    //Setup TCP coms
    ostringstream CMDstream; // string packet
    string CMD;
    string PiADDR = "10.0.0.10";
    int PORT=12345;
    SOCKET u_sock = OwlCommsInit(PORT, PiADDR);

    //Set servo positions to their center-points
    Rx = RxC; Lx = LxC;
    Ry = RyC; Ly = LyC;
    Neck= NeckC;

    // move servos to centre of field
    CMDstream.str("");
    CMDstream.clear();
    CMDstream << Rx << " " << Ry << " " << Lx << " " << Ly << " " << Neck;
    CMD = CMDstream.str();
    string RxPacket= OwlSendPacket (u_sock, CMD.c_str());

    Mat Frame, Left, Right;

    //Open video feed
    string source = "http://10.0.0.10:8080/stream/video.mjpeg";
    VideoCapture cap (source);
    if (!cap.isOpened())
    {
        cout  << "Could not open the input video: " << source << endl;
        return -1;
    }

    // program mode
    enum { NONE, NECK_PAN, EYE_PAN, CHAMELEON, EYE_ROLL, A_EYE_ROLL, FOCUS } mode = NONE;

    //main program loop
    while (1){
        if (!cap.read(Frame))
        {
            cout  << "Could not open the input video: " << source << endl;
            break;
        }

        //flip input image as it comes in reversed
        Mat FrameFlpd;
        flip(Frame,FrameFlpd,1);

        // Split into LEFT and RIGHT images from the stereo pair sent as one MJPEG iamge
        Left= FrameFlpd(Rect(0, 0, 640, 480)); // using a rectangle
        Right=FrameFlpd(Rect(640, 0, 640, 480)); // using a rectangle

        //Draw a circle in the middle of the left and right image (usefull for aligning both cameras)
        circle(Left,Point(Left.size().width/2,Left.size().height/2),10,Scalar(255,255,255),1);
        circle(Right,Point(Right.size().width/2,Right.size().height/2),10,Scalar(255,255,255),1);

        //Display left and right images
        imshow("Left",Left);
        imshow("Right", Right);

        //Read keypress and move the corresponding motor
        int key = waitKey(50);
        switch (key){
        case 'w': //up
            Ry=Ry+5;
            break;
        case 's'://down
            Ry=Ry-5;
            break;
        case 'a'://left
            Rx=Rx-5;
            break;
        case 'd'://right
            Rx=Rx+5;
            break;
        case 'i': //up
            Ly=Ly-5;
            break;
        case 'k'://down
            Ly=Ly+5;
            break;
        case 'j'://left
            Lx=Lx-5;
            break;
        case 'l'://right
            Lx=Lx+5;
            break;
        case 'e'://right
            Neck=Neck+5;
            break;
        case 'q'://left
            Neck=Neck-5;
            break;
        case 'x': //eye pan
            mode = EYE_PAN;
            break;
        case 'n': //neck pan
             mode = NECK_PAN;
             break;
        case 'c': //Chameleon
            mode = CHAMELEON;
            break;
        case 'r': //Annoyed eye roll
            mode = A_EYE_ROLL;
            break;
        case 27: //STOP ALL (ESCAPE KEY)
            mode = NONE;
            // Center everything
            Rx = RxC; Lx = LxC;
            Ry = RyC; Ly = LyC;
            Neck= NeckC;
            break;
        case 'f': //focus on point
            mode = FOCUS;
            break;
        case 'o': //Rolls Eyes
            mode = EYE_ROLL;
            break;
        }

        switch (mode) {
            case FOCUS:
                point_focus(Rx, Lx, Neck);
                break;
            case A_EYE_ROLL:
                annoyed_eye_roll(Rx, Ry, Lx, Ly, Neck);
                break;
            case EYE_ROLL:
                eye_roll(Rx, Ry, Lx, Ly);
                break;
            case CHAMELEON:
                chameleon_eyes(Rx, Ry, Lx, Ly);
                break;
            case EYE_PAN:
                pan_eyes(Rx, Lx);
                break;
            case NECK_PAN:
                pan_sin(Neck);
                break;
            case NONE:
                break;
        }

        //Send new motor positions to the owl servos
        CMDstream.str("");
        CMDstream.clear();
        CMDstream << Rx << " " << Ry << " " << Lx << " " << Ly << " " << Neck;
        CMD = CMDstream.str();
        printf("main() %s", CMD.c_str());
        RxPacket= OwlSendPacket (u_sock, CMD.c_str());

    } // END cursor control loop

    // close windows down
    destroyAllWindows();


#ifdef _WIN32
    RxPacket= OwlSendPacket (u_sock, CMD.c_str());
    closesocket(u_sock);
#else
    OwlSendPacket (clientSock, CMD.c_str());
    close(clientSock);
#endif
    exit(0); // exit here for servo testing only
}
