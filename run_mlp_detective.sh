#!/bin/bash

./mlp_detective
telerun --utils ./mlp_detective mlp_detective_args.txt | grep HW10
