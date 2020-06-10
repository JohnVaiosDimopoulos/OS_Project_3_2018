all: MyPort PortMaster Monitor Vessel

MyPort:MyPort.o MyPort_funct.o
	gcc -pthread MyPort.o MyPort_funct.o -o MyPort

MyPort.o:MyPort.c
	gcc -c MyPort.c

MyPort_funct.o:MyPort_funct.c
	gcc -c MyPort_funct.c

PortMaster:PortMaster.o PortMaster_funct.o
	gcc -pthread PortMaster.o PortMaster_funct.o -o PortMaster

PortMaster.o:PortMaster.c
	gcc  -c PortMaster.c

PortMaster_funct.o:PortMaster_funct.c
	gcc  -c PortMaster_funct.c

Monitor:Monitor.o Monitor_funct.o
	gcc -pthread Monitor.o Monitor_funct.o -o Monitor

Monitor.o:Monitor.c
	gcc -c Monitor.c

Monitor_funct.o:Monitor_funct.c
	gcc -c Monitor_funct.c

Vessel:Vessel.o Vessel_funct.o
	gcc -pthread Vessel.o Vessel_funct.o -o Vessel

Vessel.o:Vessel.c
	gcc -c Vessel.c

Vessel_funct.o:Vessel_funct.c
	gcc -c Vessel_funct.c
