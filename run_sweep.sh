#!/bin/bash

./sweep_phase
telerun --utils ./sweep_phase sweep_args.txt | grep HW10
