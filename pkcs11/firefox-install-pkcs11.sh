#!/bin/bash 

if [ "$(which firefox)" ] 
then 
	firefox CHANGEME/clauerPK11inst.xpi &
else
  echo "No puedo encontrar firefox"
	exit -1 
fi
