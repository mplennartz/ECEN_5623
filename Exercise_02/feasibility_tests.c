/*********************************************************************************************************
Feasibility_tests.c

   Author:   Sam Siewert
   Modified: Martin Lennartz
   
   Base code given for Exercise 2, part 4.
   
   Modified by adding EDF and LLF completion tests as well as a wrapper function to run all tests.
*********************************************************************************************************/

// Included libraries
#include <math.h>
#include <stdio.h>

// Global program definitions 
#define TRUE 1
#define FALSE 0
#define U32_T unsigned int

// Example service formatting
// Kept original format
// Consider creating struct with num_services, period, wcet, deadline
// could create array and loop through all of the examples with this format
U32_T ex4_period[] = {2, 4, 16};
U32_T ex4_wcet[] = {1, 1, 4};

U32_T ex5_period[] = {2, 5, 10};
U32_T ex5_wcet[] = {1, 2, 1};

U32_T ex6_period[] = {2, 5, 7, 13};
U32_T ex6_wcet[] = {1, 1, 1, 2};
U32_T ex6_deadline[] = {2, 3, 7, 15}; // required for deadline monotonic testing

U32_T ex7_period[] = {3, 5, 15};
U32_T ex7_wcet[] = {1, 2, 4};

U32_T ex8_period[] = {2, 5, 7, 13};
U32_T ex8_wcet[] = {1, 1, 1, 2};

U32_T ex9_period[] = {6, 8, 12, 24};
U32_T ex9_wcet[] = {1, 2, 4, 6};

// File access for printing timing info
// Comma delimited so it can be analyzed/parsed
FILE* fp;
char* _file = "timing.csv";

// Function definitions
void  wrapper_function                  ( U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[] );
int   EDF_completion_time_feasibility   ( U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[] );
int   LLF_completion_time_feasibility   ( U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[] );
int   RM__completion_time_feasibility   ( U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[] );
int   RM__scheduling_point_feasibility  ( U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[] );

int main(void)
{
    int i;
    U32_T numServices;        // local variable calc/passed to functions
    fp = fopen( _file, "w");  // open timing file for writing
   
    fprintf(fp,",ex04,\n");  // reference example for timing file
    // Kept original formmating for consistency 
    // consider accessing values from variable array instead of hardcoding
    printf("Ex-4 U=%4.2f& (C1=1, C2=1, C3=4; T1=2, T2=4, T3=16; T=D): \n",((1.0/2.0) + (1.0/4.0) + (4.0/16.0))*100.00);                     
    //Calc number of services
	  numServices = sizeof(ex4_period)/sizeof(U32_T);
    //Wrapper function cals RM compl & schedule feasibility, EDF compl, and LLF completion tests
    wrapper_function(numServices, ex4_period, ex4_wcet, ex4_period);

    //All other examples -- same format as above
    fprintf(fp,",ex05,\n");
    printf("Ex-5 U=%4.2f% (C1=1, C2=2, C3=1; T1=2, T2=5, T3=10; T=D): \n",((1.0/2.0) + (2.0/5.0) + (1.0/10.0))*100.00);
    numServices = sizeof(ex5_period)/sizeof(U32_T);
    wrapper_function(numServices, ex5_period, ex5_wcet, ex5_period);  

    fprintf(fp,",ex06,\n");
    printf("Ex-6 U=%4.2f% (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): \n",((1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0))*100.00);
    numServices = sizeof(ex6_period)/sizeof(U32_T);
    wrapper_function(numServices, ex6_period, ex6_wcet, ex6_period);  
    //Running rate-monotonic functions where deadline does not nec equal period
    if(RM__completion_time_feasibility(numServices, ex6_period, ex6_wcet, ex6_deadline) == TRUE)
        printf("\tDM completion : FEASIBLE\n");
    else
        printf("\tDM completion : INFEASIBLE\n");
    if(RM__scheduling_point_feasibility(numServices, ex6_period, ex6_wcet, ex6_deadline) == TRUE)
        printf("\tDM scheduling point : FEASIBLE\n");
    else
        printf("\tDM scheduling point : INFEASIBLE\n");

    fprintf(fp,",ex07,\n");
    printf("Ex-7 U=%4.2f% (C1=1, C2=2, C3=4; T1=3, T2=5, T3=15; T=D): \n",((1.0/3.0) + (2.0/5.0) + (4.0/15.0))*100.00);
    numServices = sizeof(ex7_period)/sizeof(U32_T);
    wrapper_function(numServices, ex7_period, ex7_wcet, ex7_period);  

    fprintf(fp,",ex08,\n");
    printf("Ex-8 U=%4.2f% (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): \n",((1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0))*100.00);
    numServices = sizeof(ex8_period)/sizeof(U32_T);
    wrapper_function(numServices, ex8_period, ex8_wcet, ex8_period);  

    fprintf(fp,",ex09,\n");
    printf("Ex-9 U=%4.2f% (C1=1, C2=2, C3=4, C4=6; T1=6, T2=8, T3=12, T4=24; T=D): \n",((1.0/6.0) + (2.0/8.0) + (4.0/12.0) + (6.0/24.0))*100.00);
    numServices = sizeof(ex9_period)/sizeof(U32_T);
    wrapper_function(numServices, ex9_period, ex9_wcet, ex9_period);  

  fclose(fp); // closing timing file

  return 0;
}
/******************************************************************************************************
  wrapper_function
    calls all of the testing functions 
******************************************************************************************************/
void wrapper_function( U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]){
  if(RM__completion_time_feasibility(numServices, period, wcet, period) == TRUE)
      printf("\tRM completion : FEASIBLE\n");
  else
      printf("\tRM completion : INFEASIBLE\n");
  if(RM__scheduling_point_feasibility(numServices, period, wcet, period) == TRUE)
      printf("\tRM scheduling point : FEASIBLE\n");
  else
      printf("\tRM scheduling point : INFEASIBLE\n");

  if(EDF_completion_time_feasibility(numServices, period, wcet, period) == TRUE)
      printf("\tEDF completion : FEASIBLE\n");
  else
      printf("\tEDF completion : INFEASIBLE\n");

  if(LLF_completion_time_feasibility(numServices, period, wcet, period) == TRUE)
      printf("\tLLF completion : FEASIBLE\n");
  else
      printf("\tLLF completion : INFEASIBLE\n");
}
/******************************************************************************************************
  Custom earliest deadline first completion test
    steps 1 clk at a time, updating timing to deadlines at each clk
    process --
      determine absolute deadline for each service
      calculate remaining capacity for each service
      determine priority; absolute deadline - current time
      run task for one clock; reduce remaing capcity
      check for any missed deadlines; check if any service has remaining capacity at deadline
      update absolue deadlines
      return to determine priority
******************************************************************************************************/
int EDF_completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]){
  int ii;                   // counter
  U32_T min;                // min = smallest diff until absolute deadline; recalculated each step
  U32_T task;               // which service to run; 99 = idle
  U32_T LCM = 1;            // least common multiplier, FIXME not currently using least
  U32_T clk = 0;            // counter to step LCM clks 
  U32_T abdl[numServices];  // absolute deadline for each service
  U32_T ttdl[numServices];  // time to deadline for each service
  U32_T rcap[numServices];  // remaining capacity for each service

  fprintf(fp,",EDF,");      // write test to timing file

  // assume feasible until we find otherwise
  int set_feasible=TRUE;

  for(ii=0; ii<numServices; ii++){
    LCM *= period[ii]; // FIXME may run longer than needed
    abdl[ii] = deadline[ii];
    rcap[ii] = wcet[ii];
//    printf("task %1d - absolule deadline = %3d\n",ii,abdl[ii]);
  }
//  printf("LCM = %3d\n",LCM);
  while(clk < LCM){
    //Determine priority
    min = 99;
    task = 99;
    for(ii=0; ii<numServices; ii++){
      ttdl[ii] = abdl[ii] - clk;
      if( ttdl[ii] < min && rcap[ii] > 0){
        min = ttdl[ii];
        task = ii;
      }
    }   
    //run task
    if(task != 99){
      rcap[task] -= 1;
//      printf("[%3d] starting task %1d\n",clk,task);
      fprintf(fp,"%0d,",task);
    }else{
//      printf("[%3d] idle\n",clk);
      fprintf(fp,"%0d,",task);
    }
    clk++;

    //Check for missed deadline
    for(ii=0; ii<numServices; ii++){
      if(abdl[ii] == clk && rcap[ii] > 0){
        set_feasible=FALSE;
      }
    }
    if(set_feasible == FALSE){
      break;
    }
    //Update absolute deadlines
    for(ii=0; ii<numServices; ii++){
      if(abdl[ii] == clk){
        abdl[ii] = clk + deadline[ii];
        rcap[ii] = wcet[ii];
//        printf("task %1d - absolule deadline = %3d\n",ii,abdl[ii]);
      }
    } 
  }

  fprintf(fp,"\n");
  return set_feasible;
}
/******************************************************************************************************
  Custom least latincy first completion test
    steps 1 clk at a time, updating timing to deadlines at each clk
    process --
      determine absolute deadline for each service
      calculate remaining capacity for each service
      determine priority; absolute deadline - current time - remaining capacity
      run task for one clock; reduce remaing capcity
      check for any missed deadlines; check if any service has remaining capacity at deadline
      update absolue deadlines
      return to determine priority
******************************************************************************************************/
int LLF_completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]){
  int ii;                   // counter
  U32_T min;                // min = smallest diff until absolute deadline; recalculated each step
  U32_T task;               // which service to run; 99 = idle
  U32_T LCM = 1;            // least common multiplier, FIXME not currently using least
  U32_T clk = 0;            // counter to step LCM clks 
  U32_T abdl[numServices];  // absolute deadline for each service
  U32_T ttdl[numServices];  // time to deadline for each service
  U32_T rcap[numServices];  // remaining capacity for each service

  fprintf(fp,",LLF,");



  // assume feasible until we find otherwise
  int set_feasible=TRUE;

  for(ii=0; ii<numServices; ii++){
    LCM *= period[ii]; // FIXME may run longer than needed
    abdl[ii] = deadline[ii];
    rcap[ii] = wcet[ii];
//    printf("task %1d - absolule deadline = %3d\n",ii,abdl[ii]);
  }
//  printf("LCM = %3d\n",LCM);
  while(clk < LCM){
    //Determine priority
    min = 99;
    task = 99;
    for(ii=0; ii<numServices; ii++){
      ttdl[ii] = abdl[ii] - clk - rcap[ii];
      if( ttdl[ii] < min && rcap[ii] > 0){
        min = ttdl[ii];
        task = ii;
      }
    }   
    //run task
    if(task != 99){
      rcap[task] -= 1;
//      printf("[%3d] starting task %1d\n",clk,task);
      fprintf(fp,"%0d,",task);
    }else{
//      printf("[%3d] idle\n",clk);
      fprintf(fp,"%0d,",task);
    }
    clk++;

    //Check for missed deadline
    for(ii=0; ii<numServices; ii++){
      if(abdl[ii] == clk && rcap[ii] > 0){
        set_feasible=FALSE;
      }
    }
    if(set_feasible == FALSE){
      break;
    }
    //Update absolute deadlines
    for(ii=0; ii<numServices; ii++){
      if(abdl[ii] == clk){
        abdl[ii] = clk + deadline[ii];
        rcap[ii] = wcet[ii];
//        printf("task %1d - absolule deadline = %3d\n",ii,abdl[ii]);
      }
    } 
  }


  fprintf(fp,"\n");
  return set_feasible;
}
int RM__completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]){
   int i, j;      // counters
   U32_T an;      // 
   U32_T anext;   // 

   // assume feasible until we find otherwise
   int set_feasible=TRUE;

   //printf("numServices=%d\n", numServices);

   for (i=0; i < numServices; i++){
      an=0; anext=0;
      for (j=0; j <= i; j++){
         an+=wcet[j];
      }
      //printf("i=%d, an=%d\n", i, an);
      while(1){
         anext=wcet[i];
         for (j=0; j < i; j++){
            anext += ceil(((double)an)/((double)period[j]))*wcet[j];
         }
         if (anext == an){
            break;
         }else{
            an=anext;
         }
      //printf("an=%d, anext=%d\n", an, anext);
      }
      //printf("an=%d, deadline[%d]=%d\n", an, i, deadline[i]);
      if (an > deadline[i]){
         set_feasible=FALSE;
      }
   }
   return set_feasible;
}

int RM__scheduling_point_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]){
   int rc = TRUE, i, j, k, l, status, temp;

   for (i=0; i < numServices; i++){ // iterate from highest to lowest priority
      status=0;
      for (k=0; k<=i; k++){
         for (l=1; l <= (floor((double)period[i]/(double)period[k])); l++){
            temp=0;
            for (j=0; j<=i; j++){
               temp += wcet[j] * ceil((double)l*(double)period[k]/(double)period[j]);
            }
            if (temp <= (l*period[k])){
               status=1;
               break;
            }
         }
         if (status){
            break;
         }
      }
      if (!status){ 
         rc=FALSE;
      }
   }
   return rc;
}
