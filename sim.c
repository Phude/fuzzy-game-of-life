#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
// == MATH ==
unsigned randseed;
unsigned randint() {
	randseed = (randseed * 1103515245) + 12345; 
	return randseed;
};

#define NUM_COLORS 26

#define BOARD_WIDTH 100
#define BOARD_HEIGHT 50
#define BOARD_SIZE (BOARD_WIDTH * BOARD_HEIGHT)

char board_buffer1[BOARD_SIZE];
char board_buffer2[BOARD_SIZE];

char *front_buffer; // active state of the board
char *back_buffer;  // writes are made to this buffer

const unsigned reproduce_table[9] = {
	0,  // 0
	1, // 1
	100, // 2
	9950, // 3
	1, // 4
	1, // 5
	1, // 6
	1,  // 7
	1,  // 8
};

const unsigned survival_table[9] = {
	0,  // 0
	1, // 1
	9860, // 2
	9990, // 3
	50, // 4
	100, // 5
	200, // 6
	8888,  // 7
	1,  // 8
};

void swap_buffers() {
	char *tmp = back_buffer;
	back_buffer = front_buffer;
	front_buffer = tmp;
}

int idx(int x, int y) {
	if (x < 0)
		x = BOARD_WIDTH - x;
	if (y < 0)
		y = BOARD_HEIGHT - y;
	x = x % BOARD_WIDTH;
	y = y % BOARD_HEIGHT;
	int index = x + (BOARD_WIDTH * y);
	assert(index < BOARD_SIZE);
	return index;
}

int read_cell(int x, int y) {
	return front_buffer[idx(x, y)];
}

void write_cell(int value, int x, int y) {
	back_buffer[idx(x, y)] = value;
}

void update_cell(int x, int y) {
	char neighbors[256];
	int living_neighbor_count = 0;
	char my_color = read_cell(x, y);

	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			char neighbor_color = read_cell(x+i, y+j);
			neighbors[neighbor_color] += 1;
			if (neighbor_color != 0) {
				++living_neighbor_count;
			}
		}
	}

	// we overcounted didn't we?
	neighbors[my_color] -= 1;
	if (my_color != 0) {
		living_neighbor_count -= 1;
	}

	// now make the changes
	int chance;
	if (my_color == 0) {
		chance = reproduce_table[living_neighbor_count];
	}
	else {
		chance = survival_table[living_neighbor_count];
	}

//	printf("CHANCE: %d, NEIGHBORS: %d\n", chance, living_neighbor_count);

	unsigned roll = randint() % (10 * 1000);
	if (roll < chance) {
		write_cell(1, x, y);
	}
	else {
		write_cell(0, x, y);
	}
}

int step() {
	int living_cell_count = 0;
	// count neighbors for each cell
	for (int j = 0; j < BOARD_HEIGHT; ++j) {
		for (int i = 0; i < BOARD_WIDTH; ++i) {
			update_cell(i, j);
			if (read_cell(i, j) != 0) {
				++living_cell_count;
			}
		}
	}

	swap_buffers();
	return living_cell_count;
}

void init_board(int initial_seed) {
	front_buffer = board_buffer1;
	back_buffer = board_buffer2;

	randseed = initial_seed;
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if ((randint() % 10) == 0)
			front_buffer[i] = 1;
	}
}

char colormap[NUM_COLORS] = "QWERTYUIOPASDFGHJKLZXCVBNM";

#define FPS 16

#define MAX_EPOCH (1000 * 1000)
#define VISUAL_MODE
int main() {
	init_board(time(NULL));

	int epoch = 0;

	while (epoch < MAX_EPOCH) {
#ifdef VISUAL_MODE
		// print board
		printf("\033[2J\n");
		printf("\033[0;0H");
		for (int j = 0; j < BOARD_HEIGHT; ++j) {
			for (int i = 0; i < BOARD_WIDTH; ++i) {
				char c = ' ';
				char cell_color = read_cell(i, j);
				if (cell_color != 0)
					c = colormap[cell_color - 1];
				putchar(c);
			}
			putchar('\n');
		}
		usleep((1000 * 1000) / FPS);
#endif

		// update gamestate
		if (step() == 0) {
			break;
		}
		++epoch;
	}
	printf("survived %d epochs\n", epoch);
}