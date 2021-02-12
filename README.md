# cache-simulator

A C++ program designed to simulate a direct-mapped, fully-associative, and set-associative caches

## Authors

- Hussam Elaraby (https://github.com/hussam-a)
- Bishoy Boshra (bishoyboshra@aucegypt.edu)

## Cache Variations Simulated

**Direct-mapped cache**

- Cache size : 32 KiB
- Line size : 8, 16, 32, 64, 128

**Fully-associative cache**

- Cache size : 32 kiB
- Line size : 8, 16, 32, 64, 128

**Set-associative cache**

- Cache size : 32 kiB
- Line size : 16B
- Associativity : 2, 4, 8

## Implementation

Caches are represented using an array of type struct line { int tag; bool valid; };
This array is dynamic; declared with a given size, and deleted after each simulation

## Simulation

1. Calculate the necessary data (number of lines, line size, number of ways for Set associative)
2. Set the line pointer to point to a new line of size = number of lines
3. Run the simulation function for a number of times = instNum (1,000,000)
4. For each run
   - generate a random address using one of the functions
   - check if this address is in the cache array along with a vlid bit
   - if found, return HIT; else, update the data and return MISS
5. Repeat the steps for different parameters, using different address generating functions
