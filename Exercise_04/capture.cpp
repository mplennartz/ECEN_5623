#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

//#include <unistd.h>
//#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define FRAMES_TO_CAPTURE 10

using namespace cv;
using namespace std;

enum _tran { GAUSS = 0, GRAY, HOUGH };
string t_tran[3] = {"GAUSS","GRAY","HOUGH_CIRCLES" };

// Custom structures
typedef struct
{
	_tran		 transform;
	unsigned int h_resl;
	unsigned int v_resl;
} _test;

int selection_delays[9];
_test test[3];

pthread_t main_thread;

pthread_t thread_analysis;
pthread_t thread_transform;
pthread_t thread_timer;

pthread_attr_t 	main_sched_attr;
pthread_attr_t 	timer_sched_attr;
pthread_attr_t 	transform_sched_attr;

struct sched_param transform_param;
struct sched_param nrt_param;
struct sched_param timer_param;
struct sched_param main_param;


sem_t sem_image_capture;

int mode,selection,continue_to_run;
int rt_max_prio, rt_min_prio, min;
int exit_early;

string dev;

void print_opts();
void setup_resl(){
	cout << "Starting setup" << endl;
	test[0].h_resl = 80;
	test[0].v_resl = 60;
	test[1].h_resl = 320;
	test[1].v_resl = 240;
	test[2].h_resl = 1280;
	test[2].v_resl = 960;
	test[0].transform = GAUSS;
	test[1].transform = GRAY;
	test[2].transform = HOUGH;
	selection = -1;
	continue_to_run = 1;
	exit_early = 0;
	
//	selection_delay[0] = ; //Finish analysis
//	selection_delay[1] = ; //Finish analysis
//	selection_delay[2] = ; //Finish analysis
//	selection_delay[3] = ; //Finish analysis
//	selection_delay[4] = ; //Finish analysis
//	selection_delay[5] = ; //Finish analysis
//	selection_delay[6] = ; //Finish analysis
//	selection_delay[7] = ; //Finish analysis
//	selection_delay[8] = ; //Finish analysis
}

void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_OTHER\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }
}

void *Soft_timer( void *threadid );
void *Analyze_transform( void *threadid );
void *Preform_transform( void *threadid );

int main( int argc, char* argv[] )
{
	string log = "[ MAIN    ] ";
	int rc;
	int result,ii;
	string input,cut;
	print_scheduler();
	//Setup selectable resoultions
	setup_resl();
/*
    if(argc > 1){
        sscanf(argv[1], "%d", &dev);
        printf("using %s\n", argv[1]);
    }else if(argc == 1){
        printf("using default\n");
    }else{
        printf("usage: capture [dev]\n");
        exit(-1);
    }
*/
	for(ii=1;ii<argc;ii++){
		input = argv[ii];
//		cout << log << ii << " : " << input << " : " << input.length() << endl;
		if( input.compare(0,5,"mode=") == 0){
			cut = input.substr(5,input.length()-5);
			mode = atoi(cut.c_str());
			cout << log << "Set mode = " << mode << endl;
		}
		if( input.compare(0,4,"sel=") == 0){
			cut = input.substr(4,input.length()-4);
			selection = atoi(cut.c_str());
			cout << log << "Set selection = " << selection << endl;
		}
	}
	if( selection < 0 || selection > 8 ){
		cout << log << "ERROR!! No valid selection made." << endl << endl;
		print_opts();
		return -1;
	}


	//Read input mode
	//Mode 0 - Print out
	//Mode 1 - Run operation
	//Mode 2 - Analysis
	//Mode 3 - Debug
	if( mode == 1 ){
		cout << log << "Mode selected - Run operation" << endl;
   		cout << log << "Before adjustments to scheduling policy: " << endl;
		print_scheduler();
		//Scheduler setup
		pthread_attr_init(&timer_sched_attr);
		pthread_attr_init(&transform_sched_attr);
		pthread_attr_init(&main_sched_attr);

		pthread_attr_setinheritsched(&timer_sched_attr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setinheritsched(&transform_sched_attr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setinheritsched(&main_sched_attr, PTHREAD_EXPLICIT_SCHED);

		pthread_attr_setschedpolicy(&timer_sched_attr, SCHED_FIFO);
		pthread_attr_setschedpolicy(&transform_sched_attr, SCHED_FIFO);
		pthread_attr_setschedpolicy(&main_sched_attr, SCHED_FIFO);

		rt_max_prio = sched_get_priority_max(SCHED_FIFO);
		rt_min_prio = sched_get_priority_min(SCHED_FIFO);

		rc=sched_getparam(getpid(), &nrt_param);

		main_param.sched_priority = rt_max_prio;
		timer_param.sched_priority = rt_max_prio-1;
		transform_param.sched_priority = rt_max_prio-2;

		if (sched_setscheduler(getpid(), SCHED_FIFO, &main_param)){
			cout << log << "ERROR; sched_setscheduler rc is " << rc << endl; 
			perror(NULL); 
			exit(-1);
		}

		//Checking scheduler
   		cout << log << "After adjustments to scheduling policy: " << endl;
		print_scheduler();

		pthread_attr_setschedparam(&timer_sched_attr, &timer_param);
		pthread_attr_setschedparam(&transform_sched_attr, &transform_param);
		pthread_attr_setschedparam(&main_sched_attr, &main_param);

		//Create timer thread
		if( pthread_create( &thread_timer, 
							&timer_sched_attr, 
							Soft_timer, 
							(void*)0				) 
		){
			cout << log << "ERROR!! Could not create timer thread!" << endl;
			perror(NULL);
			return -1;
		}
		if( pthread_create( &thread_transform, 
							&transform_sched_attr, 
							Preform_transform, 
							(void*)0				) 
		){
			cout << log << "ERROR!! Could not create transform thread!" << endl;
			perror(NULL);
			return -1;
		}

		pthread_join( thread_timer, NULL );
		pthread_join( thread_transform, NULL );

		if(pthread_attr_destroy( &timer_sched_attr ) != 0){
			perror("attr destroy");
		}
		if(pthread_attr_destroy( &transform_sched_attr ) != 0){
			perror("attr destroy");
		}

		sem_destroy(&sem_image_capture);
		rc=sched_setscheduler(getpid(), SCHED_OTHER, &nrt_param);
	}else if( mode == 2 ){
		cout << log << "Mode selected - Analysis" << endl;

		//Create thread
		if( pthread_create( &thread_analysis, 
							NULL, 
							Analyze_transform, 
							(void*)0				) 
		){
			cout << log << "ERROR!! Could not create thread!" << endl;
			perror(NULL);
			return -1;
		}
		pthread_join( thread_analysis, NULL );

	}else if( mode == 3 ){
		cout << log << "Mode selected - Debug" << endl;
		for(ii=0;ii<9;ii++){
			cout << "\tSelection " << ii << " : " << endl;
			cout << "\t\ttransform  : " << t_tran[test[ii/3].transform ] << endl;
			cout << "\t\tresolution : " << test[ii%3].h_resl << "x"<<test[ii%3].v_resl << endl;
		}
	}else{
		cout << log << "ERROR!! No valid mode selected" << endl << endl;
		print_opts();
	}

	cout << log << "Done, exiting." << endl;
	return 0;
}
void *Soft_timer( void *threadid ){
	string log = "[ TIMER   ] ";
	
	cout << log << "Setup hold time" << endl;
	usleep(100000);
	while(continue_to_run){	
		usleep(200000);							//define from analysis 
//		usleep( selection_delay[selection] );							
		sem_post( &sem_image_capture );
		cout << log << "Post semaphore" << endl; //Move to sys call
	}
	pthread_exit( NULL );
}
void *Preform_transform( void *threadid ){
	string log = "[ PREFORM ] ";
	int jj;
    struct timespec start, finish;
	Mat capture,gray,display;
    vector<Vec3f> circles;
	int dv = 0;
	int hard_cnt = 500;
	int filter = selection/3;

	cout << log << "Filter : " << t_tran[filter] << endl;

    VideoCapture cam;

	namedWindow("Standard", WINDOW_AUTOSIZE);
//	namedWindow("Standard", WINDOW_NORMAL);

    cam.set( CV_CAP_PROP_FRAME_WIDTH, test[selection%3].h_resl );   // <-- C++ syntax
    cam.set( CV_CAP_PROP_FRAME_HEIGHT, test[selection%3].v_resl );  // <-- C++ syntax

	cam.open(dv);

	if( !cam.isOpened() ){
        cout << log << "ERROR!! Video capture 0 did not correctly open" << endl;
        exit(-1);
    }

	cout << log << "Successfully opened cams" << endl;
	cout << log << "Camera width  : " << cam.get(CV_CAP_PROP_FRAME_WIDTH) << endl;
	cout << log << "Camera height : " << cam.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
	cout << log << "Filter : " << t_tran[filter] << endl;

	while(1){
		cout << log << "Wait for semaphore lock" << endl; //Move to sys call
		sem_wait( &sem_image_capture );
		cout << log << "Aquired semaphore lock" << endl;	//Move to sys call
		if( !cam.read(capture)){ 
        	cout << log << "ERROR!! Video capture 0 did not correctly read" << endl;
			break;
		}
		if(filter == 1){
        	cvtColor(capture, display, COLOR_BGR2GRAY);
		}else if(filter == 0){		
			GaussianBlur(capture, display, Size(9,9), 2, 2);
		}else if(filter == 2){
		    // Converts an image from one color to another (gray)
		    cvtColor(capture, gray, COLOR_BGR2GRAY);
		    // Blurs an image using Gaussian filter
		    GaussianBlur(gray, gray, Size(9,9), 2, 2);
		    // Finds circles in a grayscale image using the Hough transform
		    HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows/8, 100, 50, 0, 0);
			for( size_t i = 0; i < circles.size(); i++ ){
				Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
				int radius = cvRound(circles[i][2]);
				// circle center
				circle( capture, center, 3, Scalar(0,255,0), -1, 8, 0 );
				// circle outline
				circle( capture, center, radius, Scalar(0,0,255), 3, 8, 0 );
			}
			display = capture.clone();
		}else{
			display = capture.clone();	
		}
		imshow( "Standard", display);
		// 'q' will halt this thread and timer
        char c = cvWaitKey(30);
        if( c == 'q' ){
			printf("break sig\n"); 
			break;
		}
	}

	//have Soft_timer thread finish
	continue_to_run = 0;

	pthread_exit( NULL );

}

void *Analyze_transform( void *threadid ){
	string log = "[ ANALYZE ] ";
	int jj;
    struct timespec start, finish;
	Mat capture,gray,display;
    vector<Vec3f> circles;
	int dv = 0;
	int hard_cnt = 500;
	int filter = selection/3;

    VideoCapture cam;

	namedWindow("Standard", WINDOW_AUTOSIZE);
//	namedWindow("Standard", WINDOW_NORMAL);

    cam.set( CV_CAP_PROP_FRAME_WIDTH, test[selection%3].h_resl );   // <-- C++ syntax
    cam.set( CV_CAP_PROP_FRAME_HEIGHT, test[selection%3].v_resl );  // <-- C++ syntax

	cam.open(dv);

	if( !cam.isOpened() ){
        cout << log << "ERROR!! Video capture 0 did not correctly open" << endl;
        exit(-1);
    }

	cout << log << "Successfully opened cams" << endl;
	cout << log << "Camera width  : " << cam.get(CV_CAP_PROP_FRAME_WIDTH) << endl;
	cout << log << "Camera height : " << cam.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;

	cout << log << "Running analysis" << endl;
	//Start time
	clock_gettime(CLOCK_MONOTONIC, &start);

	cout << log << "Filter : " << t_tran[filter] << endl;
	for(jj=0;jj<hard_cnt;jj++){
		if( !cam.read(capture)){ 
        	cout << log << "ERROR!! Video capture 0 did not correctly read" << endl;
			break;
		}
		if(filter == 1){
        	cvtColor(capture, display, COLOR_BGR2GRAY);
		}else if(filter == 0){		
			GaussianBlur(capture, display, Size(9,9), 2, 2);
		}else if(filter == 2){
		    // Converts an image from one color to another (gray)
		    cvtColor(capture, gray, COLOR_BGR2GRAY);
		    // Blurs an image using Gaussian filter
		    GaussianBlur(gray, gray, Size(9,9), 2, 2);
		    // Finds circles in a grayscale image using the Hough transform
		    HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows/8, 100, 50, 0, 0);
			for( size_t i = 0; i < circles.size(); i++ ){
				Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
				int radius = cvRound(circles[i][2]);
				// circle center
				circle( capture, center, 3, Scalar(0,255,0), -1, 8, 0 );
				// circle outline
				circle( capture, center, radius, Scalar(0,0,255), 3, 8, 0 );
			}
			display = capture.clone();
		}else{
			display = capture.clone();	
		}
		imshow( "Standard", display);
        char c = cvWaitKey(30);
        if( c == 'q' ){
			exit_early = 1;
			cout << log << "break sig" << endl;
			break;
		}
	}
	//Finish time
	clock_gettime(CLOCK_MONOTONIC, &finish);

    cam.release();
	destroyAllWindows();

	cout << log << "Start  " << start.tv_sec << " [ " << start.tv_nsec << " ]" << endl;
	cout << log << "Finish " << finish.tv_sec << " [ " << finish.tv_nsec << " ]" << endl;
	if(exit_early == 0){
		long long s,n,d;
		s = finish.tv_sec - start.tv_sec;
		n = finish.tv_nsec - start.tv_nsec;
		d = s*1000000000 + n;

		cout << log << "seconds  " << s << endl;
		cout << log << "nanosecs " << n << endl;
		cout << log << "diffence " << d << " nanosecs" << endl;
		cout << log << "Average run time (ave take from 500 cycles): " << d/hard_cnt << " nanosecs" << endl;
	}else{
		cout << log << "Test exited early, results not valid" << endl;
	}

	pthread_exit( NULL );

	//have Soft_timer thread finish
	continue_to_run = 0;
}

void print_opts(){
	cout << "Print out of command line options" << endl;
	cout << endl;
	cout << "\t+--------+-----------+------------+" << endl;
	cout << "\t| Select | Transform | Resolution |" << endl;
	cout << "\t+--------+-----------+------------+" << endl;
	cout << "\t|   0    |  Gauss    |  80x60     |" << endl;
	cout << "\t|   1    |  Gauss    |  320x240   |" << endl;
	cout << "\t|   2    |  Gauss    |  1280x960  |" << endl;
	cout << "\t+--------+-----------+------------+" << endl;
	cout << "\t|   3    |  Gray     |  80x60     |" << endl;
	cout << "\t|   4    |  Gray     |  320x240   |" << endl;
	cout << "\t|   5    |  Gray     |  1280x960  |" << endl;
	cout << "\t+--------+-----------+------------+" << endl;
	cout << "\t|   6    |  Hough    |  80x60     |" << endl;
	cout << "\t|   7    |  Hough    |  320x240   |" << endl;
	cout << "\t|   8    |  Hough    |  1280x960  |" << endl;
	cout << "\t+--------+-----------+------------+" << endl;
	cout << endl;
	cout << "Req: Selection from table: Ex. sel=3 for Gray(80x60)" << endl;
	cout << "Req: Mode from:" << endl;
	cout << "\tmode=1 : Run operation" << endl;
	cout << "\tmode=2 : Analysis" << endl;
	cout << "\tmode=3 : Debug" << endl;
	cout << "Opt: Camera select : Ex. dev=/dev/video1" << endl;
	cout << "\tdefault : dev=/dev/video0" << endl;
	cout << endl;
	cout << "Ex. ./capture dev=/dev/video1 mode=2 sel=8" << endl;
	cout << endl;
}