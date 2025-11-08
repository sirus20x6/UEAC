#!/bin/bash
clear
make astrochicken && \
make astrochickenEditor && \
cd /thearray/git/UnrealEngine/Engine/Binaries/Linux && \
./UnrealEditor && \
cd /thearray/git/UnrealEngine/astrochicken
