#!/bin/bash

while true; do
    $1 $2 $3
    mv core core_`date +"%Y-%m-%d-%H-%M-%S"`
    sleep 15
done;
