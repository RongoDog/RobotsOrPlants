CC = gcc
CCOPTS = -c -g -Wall -pthread -lpigpio -lrt
LINKOPTS = -g -pthread -lpigpio -lrt 

EXEC=flame_fighter
OBJECTS=main.o flame_sensors.o motor_controller.o temperature_sensor.o ultra_sonic_sensor.o

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(LINKOPTS) -o $@ $^

%.o:%.c
	$(CC) $(CCOPTS) -o $@ $^

clean:
	- $(RM) $(EXEC)
	- $(RM) $(OBJECTS)
	- $(RM) *~
	- $(RM) core.*

pretty: 
	indent *.c *.h -kr
