#include <sstream>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
//#include <opencv2\highgui.h>
#include "opencv2/highgui/highgui.hpp"
//#include <opencv2\cv.h>
#include "opencv2/opencv.hpp"
#define PORT_NO 20232
#define IP 193.226.12.217

using namespace std;
using namespace cv;
float distanta(int x,int y, int x1,int y1);
//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 55;
int H_MAX = 180;
int S_MIN = 50;
int S_MAX = 225;
int V_MIN = 0;
int V_MAX = 256;

int H_MIN_2 = 28;
int H_MAX_2 = 160;
int S_MIN_2 = 23;
int S_MAX_2 = 218;
int V_MIN_2 = 228;
int V_MAX_2 = 256;

typedef struct coorda {
	int H_MIN;
	int H_MAX;
	int S_MIN;
	int S_MAX;
	int V_MIN;
	int V_MAX;
} coord;


char move_command;
int sockfd, n;

//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const std::string windowName = "Original Image";
const std::string windowName1 = "HSV Image";
const std::string windowName2 = "Thresholded Image";
const std::string windowName3 = "After Morphological Operations";
const std::string trackbarWindowName = "Trackbars";
const std::string trackbar2WindowName = "Trackbars2";

void run(char cmdlist[]);


void on_mouse(int e, int x, int y, int d, void *ptr)
{
	if (e == EVENT_LBUTTONDOWN)
	{
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	}
}

void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed
}

string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}

void createTrackbars() {
	//create window for trackbars


	namedWindow(trackbarWindowName, 0);
	namedWindow(trackbar2WindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", H_MIN);
	sprintf(TrackbarName, "H_MAX", H_MAX);
	sprintf(TrackbarName, "S_MIN", S_MIN);
	sprintf(TrackbarName, "S_MAX", S_MAX);
	sprintf(TrackbarName, "V_MIN", V_MIN);
	sprintf(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH),
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);


	char Trackbar2Name[50];
	sprintf(Trackbar2Name, "H_MIN_2", H_MIN_2);
	sprintf(Trackbar2Name, "H_MAX_2", H_MAX_2);
	sprintf(Trackbar2Name, "S_MIN_2", S_MIN_2);
	sprintf(Trackbar2Name, "S_MAX_2", S_MAX_2);
	sprintf(Trackbar2Name, "V_MIN_2", V_MIN_2);
	sprintf(Trackbar2Name, "V_MAX_2", V_MAX_2);

	createTrackbar("H_MIN_2", trackbar2WindowName, &H_MIN_2, H_MAX_2, on_trackbar);
	createTrackbar("H_MAX_2", trackbar2WindowName, &H_MAX_2, H_MAX_2, on_trackbar);
	createTrackbar("S_MIN_2", trackbar2WindowName, &S_MIN_2, S_MAX_2, on_trackbar);
	createTrackbar("S_MAX_2", trackbar2WindowName, &S_MAX_2, S_MAX_2, on_trackbar);
	createTrackbar("V_MIN_2", trackbar2WindowName, &V_MIN_2, V_MAX_2, on_trackbar);
	createTrackbar("V_MAX_2", trackbar2WindowName, &V_MAX_2, V_MAX_2, on_trackbar);


}
void drawObject(int x, int y, Mat &frame) {

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

	//UPDATE:JUNE 18TH, 2013
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25 > 0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25 < FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25 > 0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25 < FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);
	//cout << "x,y: " << x << ", " << y;

}
void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed) {

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects < MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area > MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true) {
				putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				//cout << x << "," << y;
				drawObject(x, y, cameraFeed);

			}


		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}
int main(int argc, char* argv[])
{

float d1;
float d2;
	coord cordA;
	coord cordB;

	cordA.H_MIN = 55;
	cordA.H_MAX = 180;
	cordA.S_MIN = 50;
	cordA.S_MAX = 225;
	cordA.V_MIN = 0;
	cordA.V_MAX = 256;

	cordB.H_MIN = 28;
	cordB.H_MAX = 160;
	cordB.S_MIN = 23;
	cordB.S_MAX = 218;
	cordB.V_MIN = 228;
	cordB.V_MAX = 256;

  struct sockaddr_in serv_addr;
	struct hostent *server;
	char command_list[]="fsrls";


	//some boolean variables for different functionality within this
	//program
	bool trackObjects = true;
	bool useMorphOps = true;

	Point p;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
	Mat threshold2;
	//x and y values for the location of the object
	int x = 0, y = 0;
	int x1=0, y1 = 0;
	//create slider bars for HSV filtering
	createTrackbars();
	//video capture object to acquire webcam feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	capture.open("rtmp://172.16.254.99/live/nimic");
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop


	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
 	 perror("socket failed");
        exit(EXIT_FAILURE);
	}
        server = gethostbyname("193.226.12.217");

        serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    	serv_addr.sin_port = htons(PORT_NO);

	  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		printf("ERROR connecting");
		}
		else
		{
		printf("Connected");
	}


	//run(command_list);

 while (1) {


		//store image to matrix
		capture.read(cameraFeed);
		if(!cameraFeed.empty()){

		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV, Scalar(cordA.H_MIN, cordA.S_MIN, cordA.V_MIN), Scalar(cordA.H_MAX, cordA.S_MAX, cordA.V_MAX), threshold);
		inRange(HSV, Scalar(cordB.H_MIN, cordB.S_MIN, cordB.V_MIN), Scalar(cordB.H_MAX, cordB.S_MAX, cordB.V_MAX), threshold2);
		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		if (useMorphOps){
			morphOps(threshold);
			morphOps(threshold2);
		}
		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		//filtered object
		if (trackObjects){
			trackFilteredObject(x, y, threshold, cameraFeed);
			trackFilteredObject(x1, y1, threshold2, cameraFeed);
		}

		d1=distanta(x,y,x1,y1);
		run("l");
		d2=distanta(x,y,x1,y1);
		if(d2<d1)
		{
			run("b");
		}
		else
		{
			run("r");
		}


		//show frames
		imshow(windowName2, threshold);
		imshow(windowName, cameraFeed);
		imshow(windowName1, HSV);
		setMouseCallback("Original Image", on_mouse, &p);
		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		waitKey(30);
		}
		else
		{
			printf("CameraFeed is empty\n");
			exit(1);
		}
printf("D:%f\n",distanta(x,y,x1,y1));
		}





	return 0;
}

void run(char cmdlist[])
{
	int i;
	for(i=0;i<strlen(cmdlist);i++)
	{
		switch(cmdlist[i])
		{
		 case 'f':write(sockfd,"f\n",1);

		          break;

		 case 'b':write(sockfd,"b\n",1);

                          break;
		 case 'l':write(sockfd,"l\n",1);

			  break;

		 case 'r':write(sockfd,"r\n",1);

		          break;

	       	 case 's':write(sockfd,"s\n",1);

			  break;


		default:

		  break;
		}
		sleep(1);
	}
	write(sockfd,"s\n",1);



}

float distanta(int x,int y, int x1,int y1)
{
	return sqrt(((x-x1) * (x-x1)) + ((y-y1) * (y-y1)));
}
