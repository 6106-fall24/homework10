#!/bin/bash

./mark_phase
telerun --utils ./mark_phase mark_args.txt | grep HW10
