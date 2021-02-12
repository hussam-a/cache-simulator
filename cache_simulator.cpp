/*
Hussam Ashraf El-Araby
Bishoy Boshra Labib
Dec. 9th, 2014
*/

#include <iostream>
#include <iomanip>
#include <cmath>
#include <time.h>
#include <fstream>
using namespace std;

#define		DBG				1
#define		DRAM_SIZE		(64*1024*1024)
#define		CACHE_SIZE		(32*1024)

enum cacheResType { MISS = 0, HIT = 1 };		// return values

struct line { bool valid = 0; int tag; }; line *p;	// an array of type struct line; which stores valid bit and tag

//					Global variables
int type = 0;			// type of memory generator to be used
int lineSize;			// the number of bytes in a cache line
int instNum;			// the number of instructions

int lineNum;			// number of lines in a set-associative cache
int way;				// the number of ways for the set-associative cache
int n;					// a counter for the fully-associative cache
ofstream B;				// Output file

// Random Address Generators
unsigned int m_w = 0xABABAB55;	// must not be zero, nor 0x464fffff
unsigned int m_z = 0x05080902;	// must not be zero, nor 0x9068ffff
unsigned int rand_() {			// returns a 32-bit result
	m_z = 36969 * (m_z & 65535) + (m_z >> 16); m_w = 18000 * (m_w & 65535) + (m_w >> 16); return (m_z << 16) + m_w;   }
unsigned int memGen1() { static unsigned int addr = 0; return (addr++) % (DRAM_SIZE); }
unsigned int memGen2() { return rand_() % (DRAM_SIZE); }
unsigned int memGen3() { static unsigned int addr = 0; return (addr++) % (1024 * 8); }
unsigned int memGen4() { static unsigned int addr = 0; return (addr++) % (1024 * 64); }
unsigned int memGen(){	// according to the value stored in the global variable type, 
	switch (type){		// this function will generate an address using one of the address generators
	case 1: return memGen1(); break;
	case 2: return memGen2(); break;
	case 3: return memGen3(); break;
	case 4: return memGen4(); break;
	}
}

int index, tag;			// used by the cacheResType only; initialized as global to avoid redundant redeclaration

// input	: address
// output	: MISS or HIT
cacheResType cacheSimDM(unsigned int addr)
{	
	// To get the index,
	// 1. ignore bytes within line
	// 2.get the bytes for the index (known by the number of lines)
	index = (addr >> int(log2(lineSize))) % (lineNum);

	// To ge the tag,
	// ignore the bytes within line, and the bytes used for indes
	tag = addr >> int(log2(CACHE_SIZE));

	// if tag is found with a vlid bit, return HIT
	if (tag == p[index].tag && p[index].valid) return HIT;

	// if the function doesn't cease, update the data in the index and return MISS
	p[index].valid = true;
	p[index].tag = tag;
	return MISS;
}
// input	: address
// output	: MISS or HIT
cacheResType cacheSimFA(unsigned int addr)
{
	// tag = address, disregarding bytes within line
	tag = addr >> int(log2(lineSize));
	// If you find a matching tag and a valid bit, return HIT
	for (int i = 0; i < (lineNum); i++) if (p[i].valid && tag == p[i].tag) return HIT;	// compare the tags of all lines; if one found, return HIT
	// If the functions hasn't ceased, store the address in the next free space
	if (n < lineNum){	// This n is a counter, initialized at 0 once a fully-associative cache is declared.
						// It counts lines in order to insert addresses sequentially at the beginning
		p[n++].tag = tag; p[n++].valid = true;	// update data
		return MISS;
	}
	// If none of the above ceased the function, replace a line randomly, and return MISS
	index = rand() % (lineNum);	// generate a random address, within the number of lines in the cache
	p[index].tag = tag; p[index].valid = true;	// replace the data
	return MISS;
}
// input	: address
// output	: MISS or HIT
cacheResType cacheSimSA(unsigned int addr){
	// to get the index,
	// 1. ignore the bytes within line
	// 2. get bytes for index (determined by the number of lines)
	index = (addr >> (int(log2(lineSize)))) % lineNum;
	
	// to get the tag,
	// ignore the bytes within line, and the bytes used for index
	tag = addr >> int(log2(CACHE_SIZE / way));
	for (int i = 0; i < way; i++){	// loop for all ways
		// if you find a matching tag and a vlid bit, return HIT
		if (p[index + i * lineNum].valid && p[index + i * lineNum].tag == tag) return HIT;
	}
	// if the function hasn't ceased, then no HIT was found
	int i = rand() % way;													// generate a random address
	p[index + i * lineNum].tag = tag; p[index + i * lineNum].valid = true;	// update data
	return MISS;															// return MISS
}

// Runs one simulation on Direct-Mapped cache
void DM(){
	int counter = 0;	// number of misses counter
	unsigned int addr;	// memory address
	cacheResType r;		// stores MISS or HIT
	float ratio;		// miss ratio

	p = new line[lineNum];	// point to a dynamic array of size = number of lines (blocks)
	for (int i = 0; i < instNum; i++){	// loop for the number of instructions
		addr = memGen();	// generate address
		if (cacheSimDM(addr) == MISS) counter++;	// update the counter
	}
	ratio = float (counter) / instNum;	// calculate ratio

	B << "line size = " << setw(3) << lineSize << "| miss ratio = " << setw(8) << (100 * ratio ) << " %" << "\n";	// output result
	delete[] p;		// delete the array to reuse it later
}
// Runs one simulation on Fully-Associative cache
void FA()
{
	int counter = 0;	// counter for number of misses
	unsigned int addr;	// memeory address
	cacheResType r;		// stores MISS or HIT
	float ratio;		// miss ratio

	p = new line[lineNum];	// point to a dynamic array of size = number of lines (blocks)
	for (int i = 0; i < instNum; i++){	// loop for the number of instructions
		addr = memGen();	// generate address
		if (cacheSimFA(addr) == MISS) counter++;	// update the counter
	}
	ratio = float(counter) / instNum;	// calculate ratio

	B << "line size = " << setw(3) << lineSize << "| miss ratio = " << setw(8) << (100 * ratio) << " %" << "\n";	// output result
	delete[] p;		// delete the array to reuse it later
}
// Runs one simulation on Set-Associative cache
void SA(){
	int counter = 0;	// counter for number of misses
	unsigned int addr;	// memory address
	cacheResType r;		// stores MISS or HIT
	float ratio;		// miss ratio

	p = new line[CACHE_SIZE / lineSize];	// point to a dynamic array of size = total number of lines in all ways
	for (int i = 0; i < instNum; i++){		// loop for the number of instructions
		addr = memGen();	// generate an address
		if (cacheSimSA(addr) == MISS) counter++;		// update the counter
	}
	ratio = float(counter) / instNum;	// calculate ratio
	B << "associativity = " << way << "| miss ratio = " << setw(8) << (100 * ratio) << " %" << "\n";				// output result
	delete[] p;		// delete the array to reuse it later
}

int main()
{
	srand((unsigned) time(NULL));	// rand() will be used in replacement to randomly select a line in FA cache, or a way in SA cache
	
	instNum = 1000000;												// the number of instructions to be generated
	string inputfile = "simulation.txt"; B.open(inputfile.c_str());	// output file
	
	B << "          Direct-Mapped cache\n\n";
	// will loop 4 times for the memory generators, 4 times each for varying line size
	for (int i = 1; i <= 4; i++){			// loop for the number of memory address generators
		type = i;										// set this global variable to the type of memGen to be used
		B << "using memGen" << i << "()\n";
		for (int j = 0; j < 5; j++){		// loop for the line Size
			lineSize = 8 * pow(2, j);					// update line size
			lineNum = CACHE_SIZE / lineSize;			// set this global variable to number of lines
			/* After having chosen proper values of:
			type	: type of memGen
			lineSize: line size in bytes
			lineNum	: number of lines(blocks)

			call the simulating function*/
			DM();
		}
	}
	
	B << "\n\n\n          Fully-associative cache\n\n";
	// will loop 4 times for the memory generators, 4 times each for varying line size
	for (int i = 1; i <= 4; i++){			// loop for the memeory address generators
		type = i;										// set this global variable to the type of memGen to be used
		B << "using memGen" << i << "()\n";
		for (int j = 0; j < 5; j++){		// loop for the line size
			lineSize = 8 * pow(2, j); n = 0;			// update line size
			lineNum = CACHE_SIZE / lineSize;			// set this global variable to number of lines
			/* After having chosen proper values of:
			type	: type of memGen
			lineSize: line size in bytes
			lineNum	: number of lines(blocks)

			call the simulating function*/
			FA();
		}
	}

	B << "\n\n\n          Set-associative cache\n\n";
	lineSize = 16;												// Set this global variable to the chosen value of line size
	// will loop 4 times for the memory generators, e times each for varying the number of ways
	for (int i = 1; i <= 4; i++){			// loop for the memory address generators
		type = i;										// set this global variable to the type of memGen to be used
		B << "using memGen" << i << "()\n";
		for (int j = 0; j < 3; j++){		// loop for the number of ways
			way = 2 * pow(2, j);						// set this global variable to the current value of NUMBER OF WAYS in the loop
			lineNum = (CACHE_SIZE / (lineSize * way));	// set this global varialbe to the number of lines (blocks) IN ONE WAY
			/* After having chosen proper values of:
			type	: type of memGen
			way		: number of ways
			lineNum	: number of lines(blocks) in one way

			call the simulating function*/
			SA();
		}
	}
	B.close();
}
