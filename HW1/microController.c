#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#define TEMPERATURE_THRESHOLD   25
#define PRESSURE_THRESHOLD      3

bool isHeaterWorking = false;
int current_temp;
int current_pressure;

pthread_mutex_t tempMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pressMutex = PTHREAD_MUTEX_INITIALIZER; 

typedef struct
{
    char portName[20];
} port;

static void adc_trigger(port* portType)
{
    printf("Port %s adc triggered!\n", portType->portName);
}

static void write_dac(int value)
{
    printf("Value %d written to digital-to-analog pump converter\n", value);
}

static void read_adc(port* portType, int *value)
{
    printf("Adc value readed for %s  and value written do given pointer address\n", portType->portName);
}

static void write_switch(bool value)
{
	isHeaterWorking = value;
}

void* pTaskTemperature(void *arg)
{
    port* tempPort = (port*)arg;
    while(true)
    {
        adc_trigger(tempPort);
        usleep(100*1000); //10 HZ sleep(100*1000)ms
        pthread_mutex_lock(&tempMutex);
        read_adc(tempPort, &current_temp);
        pthread_mutex_unlock(&tempMutex);
        if(current_temp < TEMPERATURE_THRESHOLD && !isHeaterWorking) //Run heater to increase temperature
        {
            printf("Heater is on!\n");
            write_switch(true);
        }
        else if(current_temp >= TEMPERATURE_THRESHOLD && isHeaterWorking) //Shutdown heater so temperature can decrease
        {
            printf("Heater is shutdown!\n");
            write_switch(false);
        }
    }
    return NULL;
}

void* pTaskPressure(void *arg)
{
    port* pressPort = (port*)arg;
    while(true)
    {
        adc_trigger(pressPort);
        usleep(10*1000); //100HZ sleep(10*1000)ms
        pthread_mutex_lock(&pressMutex);
        read_adc(pressPort, &current_pressure);
        pthread_mutex_unlock(&pressMutex);
        if(current_pressure < PRESSURE_THRESHOLD) //Increase pressure by pumping air
            write_dac((PRESSURE_THRESHOLD-current_pressure)*10);
        else if(current_pressure > PRESSURE_THRESHOLD) //Decrease pressure by absorb pressure
            write_dac((PRESSURE_THRESHOLD-current_pressure)*10);
        else
            write_dac(0);
    }
    return NULL;
}

void* pTaskDisplay(void *arg)
{
    while(true)
    {
        pthread_mutex_lock(&tempMutex);
        printf("Display current temp: %d\n", current_temp);
        pthread_mutex_unlock(&tempMutex);

        pthread_mutex_lock(&pressMutex);
        printf("Display current pressure: %d\n", current_pressure);
        pthread_mutex_unlock(&pressMutex);

        usleep(10*1000); //100Hz sleep(10*1000) ms
    }
}

int main(int argc, char const *argv[])
{
    port pressPort = {
        .portName = "Pressure Port"
    };
    port tempPort = {
        .portName = "Temperature Port"
    };
    pthread_t pressureTask;
    pthread_t temperatureTask;
    pthread_t displayTask;

    current_temp = 25;
    current_pressure = 3;

    if(pthread_create(&displayTask, NULL, pTaskDisplay, NULL) != 0) //Create pressure task
    {
        printf("Error while creating display task!\n");
        return EXIT_FAILURE;
    }
    if(pthread_create(&pressureTask, NULL, pTaskPressure, &pressPort) != 0) //Create pressure task
    {
        printf("Error while creating pressure task!\n");
        return EXIT_FAILURE;
    }
    if(pthread_create(&temperatureTask, NULL, pTaskTemperature, &tempPort) != 0) //Create temperature task
    {
        printf("Error while creating temperature task!\n");
        return EXIT_FAILURE;
    }

    pthread_join(displayTask, NULL);
    pthread_join(temperatureTask, NULL);
    pthread_join(pressureTask, NULL);
    return EXIT_SUCCESS;
}
