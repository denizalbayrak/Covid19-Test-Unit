#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

//Variables 
#define waitingUnitCapacity  3 
#define unit_number 8 
#define patients_count 25

//Functions
void *hcstaff(void *number);
void *patient(void *number);
void randwait(int secs);
void *unitControl(void *number);

//Define the semaphores

//avaibleunit is for patients to enter the waiting unit
sem_t avaibleunit[unit_number];	

//patientComes hcstaff stopped ventilating
sem_t patientComes[unit_number];

//flag to hcstaff ventilating or waiting and announcing to patients
sem_t unitAvaibleflag[unit_number]; 

//hcstaff is ventilating	
sem_t hcstaffVentilating[unit_number]; 

//unit use count to avoid starvation
int reusableunit[unit_number];	
//Seat Count for each unit of hospital	
int seatCount[unit_number];		
//Flag to stop 
int allDone=0;	

int main(int argc, char *argv[])
{
    printf("\n∿Have A Good Day. Welcome to the hospital simulation.∿\n\n");
	printf("All Units Are Being Vented.\n\n");
    //creating patient and hcstaff thread
    pthread_t hcstafftid[unit_number];	
	pthread_t patientstid[patients_count]; 


	int hcstaffNumber[unit_number];
	int patientNumber[patients_count];

	for(int i = 0;i < patients_count;i++)

		patientNumber[i]=i;

	for(int i = 0;i < unit_number;i++)			

		hcstaffNumber[i]=i;

	for(int i=0;i < unit_number;i++)
		pthread_create(&hcstafftid[i],NULL,hcstaff,(void *)&hcstaffNumber[i]);
	sleep(1);

	for(int i=0;i <patients_count;i++){
		pthread_create(&patientstid[i],NULL,patient,(void *)&patientNumber[i]);
		sleep(2);
    }
    for (int i = 0; i < patients_count; i++) 
    		pthread_join(patientstid[i],NULL);
	allDone = 1;

	for (int i = 0; i < unit_number; i++){

		sem_post(&hcstaffVentilating[i]);        	

		sem_post(&avaibleunit[i]);		
	} 
    	for (int i = 0; i < unit_number; i++) 

        	pthread_join(hcstafftid[i],NULL);

	printf("\nAll the patients of the day were tested.\n");
	printf("\nShutting down simulation. I wish you a good day.\n\n");
	exit(0);	
	
}



void *hcstaff(void *number){

	//number of hcstaff
	int num = *(int *)number; 	
	int i;     int count;

    //initial states
        //patients and avaible unit check
	sem_init(&patientComes[num],0,0);
	sem_init(&unitAvaibleflag[num],0,0);
	sem_init(&hcstaffVentilating[num],0,0);

	pthread_t ttid;				

	seatCount[num]=waitingUnitCapacity;
	reusableunit[num]=0;
	
	while(!allDone){
		//sem_init(&avaibleunit[num+1],0,0);	
		//printf("* unit %d is empty. The unit is being ventilated. * \n",num+1,waitingUnitCapacity-seatCount[num+1]);

		sem_wait(&hcstaffVentilating[num]);
		printf("Staff %d stopped ventilating, waiting for the unit to fill up.\n",num+1);
		

     /* |---> */pthread_create(&ttid,NULL,unitControl,number);	

        //creating new thread for each unit 
        //check whether the unit is available
		sem_wait(&avaibleunit[num]);
        //kill the thread that created 
		pthread_cancel(ttid);	

		if(!allDone){

			count = waitingUnitCapacity-seatCount[num];   //variable for keeping patient number in the unit

			
			printf("\nPatients are in unit %d and unit is full.  \n!Test has started! \n\n",num+1,count+1);
			sleep(5);

			printf("\n* Unit %d is empty. Unit is being ventilated. * \n\n",num+1);
            printf("");

			for(i = 0 ; i < count; i ++ )	
				sem_post(&patientComes[num]);	

			reusableunit[num]++;
			sleep(2);

			for(i = 0 ; i < count; i ++ )
				sem_post(&unitAvaibleflag[num]);

			seatCount[num]=waitingUnitCapacity;			

		}

		else{
			printf("Staff %d is ventilating unit.\n",num+1);		
		}
		
	}

}


void *patient(void *number){

	int num = *(int *)number;
	int i = 0;
	int j;         int min;	

    //patient came to the hospital for unit
	printf("Patient %d arrived to hospital. Waiting for avaible unit.\n",num+1);
	sleep(1);		
	while(1){

		min = 999;
		for(j= 0; j < unit_number ; j++){
			if(min >= reusableunit[j])
				min = reusableunit[j];

		}

		if(seatCount[i]>0 && reusableunit[i] == min){
			seatCount[i]--;
			if(seatCount[i]==(waitingUnitCapacity-1)){
				sem_post(&hcstaffVentilating[i]);
				sleep(1);

			}

			printf("Patient %d is in unit %d[#%d].\n",num+1,i+1,waitingUnitCapacity-seatCount[i]);

			if(seatCount[i]==0)

				sem_post(&avaibleunit[i]);
			break;	

		}
        //waiting at the outdoor waiting hole until there is an avaible unit
		else if(i == unit_number - 1){	
			i = -1;
			sleep(3);

		}
		i++;
	}	
	sem_wait(&patientComes[i]);	//waiting patients to test and leave.
	//randwait(2);	
	printf("Patient %d has left the hospital.\n",num+1);
	sem_wait(&unitAvaibleflag[i]);	//waiting until unit is avaible for patient.
}

void *unitControl(void *number){
	int num = *(int *)number;	
	int seat = seatCount[num];

	while(1){
        //if there are no more patients coming to the unit LAST PATIENTS
		sleep(5);			

		if(seat == seatCount[num] && seat != waitingUnitCapacity ){

			printf("\nUnit %d won't be full, !!!The last people, let's start! Please, pay attention to your social distance and hygiene; use a mask!!!\n",num+1);   

			sem_post(&avaibleunit[num]);

		}

		else{	

			seat = seatCount[num];

		}



	}



}



void randwait(int secs) {
	int len = rand() % (secs+1);
     	sleep(len);

}