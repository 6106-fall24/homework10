CXX = g++
CXXFLAGS = -O3 -Wall -Wextra -std=c++20 -march=x86-64
C = gcc
CFLAGS = -O3 -Wall -Wextra -march=x86-64

all: mlp_detective sweep mark bulk_prefetch

mlp_detective: mlp_detective.cpp
	$(CXX) $(CXXFLAGS) -o mlp_detective mlp_detective.cpp

sweep: sweep_phase.c
	$(C) $(CFLAGS) -o sweep_phase sweep_phase.c -lm

mark: mark_phase.c
	$(C) $(CFLAGS) -o mark_phase mark_phase.c -lm

bulk_prefetch: bulk_prefetch.c
	$(C) $(CFLAGS) -o bulk_prefetch bulk_prefetch.c

clean:
	rm -f mlp_detective sweep_phase mark_phase bulk_prefetch
