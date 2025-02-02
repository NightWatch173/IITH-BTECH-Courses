#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <unistd.h>
#include <random>
#include <chrono>
#include <semaphore.h>
#define lli long long int
#define scale 1000000
using namespace std;

sem_t full,empty,mutex;
lli capacity,np,nc,cntp,cntc;										// Capacity, no of producers,no of consumers, no of executions of producer and consumer
double mu_p,mu_c;													// μp, μc
int *buffer;														// buffer of size capacity
lli in,out;
default_random_engine generator(time(NULL));						// Random number generator
exponential_distribution<double> *distribution1,*distribution2; 	// Exponential Distributions
chrono::duration<double> average_time_producer(0); 					
chrono::duration<double> average_time_consumer(0); 				
time_t t;				

string getSysTime()												// Returns a string in following format -> hh:mm:ss
{
	time(&t);
	struct tm * time;
	time = localtime (&t);
	char s[9];
	sprintf(s,"%.2d:%.2d:%.2d",time->tm_hour,time->tm_min,time->tm_sec);
	string str(s);
	return str;
}

void producer(int id){
	chrono::time_point<chrono::system_clock> start_time,end_time;				// Variables for calculating waiting time
	for (int i = 0; i < cntp; ++i){
		usleep((*distribution1)(generator)*scale);
    	start_time = chrono::system_clock::now();
		sem_wait(&empty);
		sem_wait(&mutex);
    	end_time = chrono::system_clock::now();
		buffer[in]=1;
		printf("%dth item produced by thread %d at %s into buffer location %lli \n",i,id,getSysTime().data(),in);
		// cout<<i<<"th item produced by thread "<< id <<" at "<< getSysTime()<<" into buffer location " << in<<endl;
		in=(in+1)%capacity;
    	average_time_producer+=end_time -start_time;
		sem_post(&mutex);
		sem_post(&full);		
	}

}

void consumer(int id){
	chrono::time_point<chrono::system_clock> start_time,end_time;				// Variables for calculating waiting time
	for (int i = 0; i < cntc; ++i){
    	start_time = chrono::system_clock::now();
		sem_wait(&full);	
		sem_wait(&mutex);
		end_time = chrono::system_clock::now();
		buffer[out]=0;
		printf("%dth item consumed by thread %d at %s into buffer location %lli \n",i,id,getSysTime().data(),in);
		out=(out+1)%capacity;
    	average_time_consumer+=end_time-start_time;
		sem_post(&mutex);
		sem_post(&empty);
		usleep((*distribution2)(generator)*scale);
	}
}

int main(){
	ifstream fin("inp-params.txt");
	fin>>capacity>>np>>nc>>cntp>>cntc>>mu_p>>mu_c;
	fin.close();
	buffer = new int[capacity]();
	in=out=0;
	distribution1 = new exponential_distribution<double>(1/mu_p);
	distribution2 = new exponential_distribution<double>(1/mu_c);
	sem_init(&full,0,0);
	sem_init(&empty,0,capacity);
	sem_init(&mutex,0,1);
	vector<thread> producers;														// Array of np producer threads
	vector<thread> consumers;														// Array of nc consumer threads
	for(int i=0;i<np;i++) producers.push_back(thread(producer,i));
	for(int i=0;i<nc;i++) consumers.push_back(thread(consumer,i));
	for(int i=0;i<np;i++) producers[i].join();
	for(int i=0;i<nc;i++) consumers[i].join();
    cout <<"Average time of producers = " << average_time_producer.count()/(np*cntp) << "s\n"; 
    cout <<"Average time of consumers = " << average_time_consumer.count()/(nc*cntc) << "s\n"; 
	delete[] buffer;
    delete distribution1;
    delete distribution2;
	return 0;
}