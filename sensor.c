#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <wiringPi.h>

#include "sensor.h"

float sensor_read() {
        DIR *dir;
    struct dirent *dirent;
    char dev[16];      // Dev ID
    char devPath[128]; // Path to device
    char buf[256];     // Data from device
    char tempData[6];   // Temp C * 1000 reported by device
    char path[] = "/sys/bus/w1/devices";

    dir = opendir (path);
    if (dir != NULL)
    {
        while ((dirent = readdir (dir)))
            // 1-wire devices are links beginning with 28-
            if (dirent->d_type == DT_LNK &&
                strstr(dirent->d_name, "28-") != NULL) {
                strcpy(dev, dirent->d_name);
                printf("\nDevice: %s\n", dev);
            }
        (void) closedir (dir);
    }
    else
    {
        perror ("Couldn't open the w1 devices directory");
        return 1;
    }


    // Assemble path to OneWire device
    sprintf(devPath, "%s/%s/w1_slave", path, dev);

    // Read temp continuously
    // Opening the device's file triggers new reading
    int fd = open(devPath, O_RDONLY);
    if(fd == -1)
    {
        perror ("Couldn't open the w1 device.");
        return 1;
    }


    // start wiringPI API
    if (wiringPiSetup() == -1)
    {	    
        perror ("Couldn't open the wiringPi");
        return 1;
    }

    // mark GPIO 17 as output ( WiringPi Pin 0 )
    pinMode(0, OUTPUT);

    // LED on
    digitalWrite(0, 1);

    while((read(fd, buf, 256)) > 0)
    {
        strncpy(tempData, strstr(buf, "t=") + 2, 5);
     
        // to make sure that the tempData has only five digits	
        tempData[5] = '\0'; 
	 printf("TEMPDATA: %s \n", tempData);
        	
        //LED off	
        digitalWrite(0, 0);

	// For whatever reason, sometimes the string has six digits
        //if  (len > 5) {
	  //printf("STRING > 5: %s \n", tmpData);
	  // cut last digit by replacing it with the null terminator
	  //tmpData[len - 2] = '\0';
//	}

        // convert to float
       return strtof(tempData, NULL)/1000;
    }
    close(fd);
    return 0;
}
