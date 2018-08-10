#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int n , m;
int raw[100][20];
int raw_length[100];
int tail[100][20];
unsigned long long int** cube;
unsigned long int cube_len[100];
int * filled;
int** used;

const unsigned long long int ONE = 1;

typedef struct {
	int length;
	unsigned long long int mask[2];
} row;

typedef struct {
	int index;
	int bit_index;
	unsigned long long bit;
} queue_item;

queue_item queue[1000];
int queue_len = 0;
int all_filled = 0;
row* solution;

unsigned long long int make_ones(int length, int offset) {
	int i;
	unsigned long long int num = 0;

	for (i = 0; i < length; i++) {
		num = num | (ONE << i);
	}
	num = num << offset;

	return num;
}

void init_cube() {
	int i;
	cube = calloc(n + m, sizeof(unsigned long long int*));
	used = calloc(n + m, sizeof(int*));
	filled = calloc(n + m, sizeof(int));
	for (i = 0; i < (n + m); i++) {
		cube[i] = calloc(100000, sizeof(unsigned long long int));
		used[i] = calloc(100, sizeof(int));
		cube_len[i] = 0;
	}
}

void free_cube() {
	int i;
	for (i = 0; i < (n + m); i++) {
		free(cube[i]);
		free(used[i]);
	}
	free(cube);		
	free(used);
	free(filled);	
}

void init_solution() {
	solution = calloc(n + m, sizeof(row));
}

void free_solution() {
	free(solution);
}

void print_cube() {
	int i, j, all_solved = 1;
	char bit;
	for (i = n; i < (n + m); i++) {
		for (j = 0; j < n; j++) {
			bit = (solution[i].mask[1] & (ONE << j)) ? 'X' : '.';

			//add hyphens if not solved
			if (solution[i].length != 1) {
				if ((bit == '.') && !(solution[i].mask[0] & (ONE << j))) {
					bit = '-';
				}
			}

			printf("%c", bit);
		}
		if (solution[i].length != 1) {
			printf("\t%d\t%d", (i - n + 1), solution[i].length);
			all_solved = 0;
		}
		printf("\n");
	}
	if (all_solved) {
		printf("\nProblem solved!\n");
	}
	else
		printf("\nProblem NOT solved.\n");
}

/*****
* index  - index of row in cube,
* mask   - current mask,
* offset - number of used bits, we have already set
* length_offset - number of used items in raw
*****/

int fill_row(int index, unsigned long long int mask, int offset, int length_offset) {
	int bound, offset_up;
	unsigned long long int mask_up, r, r_sub;

	if (!filled[index]) {
		return 1;
	}

	if (length_offset > raw_length[index]) {
		return 1;
	}

	bound = (index < n) ? m : n;
	r = make_ones(bound,0);

	if (length_offset == raw_length[index]) {
		if (cube_len[index] >= 100000) {
			cube_len[index] = 0;
			filled[index] = 0;
			return 1;
		}
		if (filled[index] > 1) {
			//check mask
			if (mask & solution[index].mask[0])
				return 1;
			if ((mask ^ r) & solution[index].mask[1])
				return 1;
		}
		cube[index][cube_len[index]++] = mask;
		return 0;
	}

	// speed up, check to fit tail
	if ((offset + tail[index][length_offset]) > bound) {
		return 1;
	}

	//?? speed up, if not first try - check submask

	if ((filled[index] > 1) && (offset > 0)) {
		r_sub = make_ones(offset, 0);
		if (mask & solution[index].mask[0])
			return 1;
		if ((mask ^ r_sub) & solution[index].mask[1])
			return 1;
	}

	offset_up = offset + raw[index][length_offset];
	if ((length_offset + 1) < raw_length[index]) {
		offset_up++;
	}

	if (offset_up <= bound) {
		mask_up = mask | make_ones(raw[index][length_offset], offset);
		fill_row(index, mask_up, offset_up, length_offset + 1);
	}

	if ((offset + 1) <= bound) {
		fill_row(index, mask, offset + 1, length_offset);
	}

	return 0;
}

unsigned long long int get_zero_mask(int index) {
	int i;
	int bound = (index < n) ? m : n;
	unsigned long long int res, r;
	r = make_ones(bound, 0);
	res = r;
	for (i = 0; i < solution[index].length; i++) {
		res = res & (cube[index][i] ^ r);
		if (!res) {
			return 0;
		}
	}
	return res;
}

unsigned long long int get_ones_mask(int index) {
	int i;
	int bound = (index < n) ? m : n;
	unsigned long long int res;
	res = make_ones(bound, 0);
	for (i = 0; i < solution[index].length; i++) {
		res = res & cube[index][i];
		if (!res) {
			return 0;
		}
	}
	return res;
}

void add_queue_item(queue_item q) {
	if (!used[q.index][q.bit_index]) {
		//printf("+\t%d\t%d\t%d\n", q.index, q.bit_index, q.bit);
		if (filled[q.index]) {
			used[q.index][q.bit_index] = 1;
		}
		queue[queue_len++] = q;
	}
}

void apply_mask(int index) {
	int i, bit;
	int bound = (index < n) ? m : n;
	unsigned long long int m[2];
	queue_item q, q1;

	m[0] = get_zero_mask(index);
	m[1] = get_ones_mask(index);

	for (bit = 0; bit <= 1; bit++) {
		m[bit] = (solution[index].mask[bit] ^ make_ones(bound, 0)) & m[bit];	
	}

	
	if (!filled[index]) {
		return;
	}
	

	i = 0;
	q.bit_index = (index >= n) ? (index - n) : index;
	q1.index = index;

	while ((m[0] | m[1]) > 0) {
		//if (index == 1) printf("!\t%d %d %d %d\n", index, i, m[1], solution[index].length);
		for (bit = 0; bit <= 1; bit++) {
			if (m[bit] & 1) {
				q.index = ((index >= n) ? 0 : n) + i;
				q.bit = bit;

				q1.bit = bit;
				q1.bit_index = i;

				add_queue_item(q);
				add_queue_item(q1);
			}
		}
		for (bit = 0; bit <= 1; bit++) {
			m[bit] = m[bit] >> 1;
		}
		i++;
	}	
}

void apply_bit(queue_item q) {
	unsigned long long int mask, c, r;
	int i;
	int bound = (q.index < n) ? m : n;

	mask = make_ones(1, q.bit_index);
	r = make_ones(bound, 0);
	mask = (solution[q.index].mask[q.bit] ^ r) & mask;

	if (mask) {

		solution[q.index].mask[q.bit] = solution[q.index].mask[q.bit] | mask;

		for (i = 0; i < solution[q.index].length; i++) {
			c = cube[q.index][i];
			if (!q.bit) {
				c = c ^ r;
			}

			//remove item with swap
			if (!(c & mask)) {
				c = cube[q.index][--solution[q.index].length];
				cube[q.index][solution[q.index].length] = cube[q.index][i];
				cube[q.index][i--] = c;
			}
		}

		apply_mask(q.index);
	}
}

void init_queue() {
	int i;
	for (i = 0; i < (n + m); i++) {
		apply_mask(i);
	}

}

void fill_cube(int t) {
	int i;
	all_filled = 0;
	for (i = 0; i < (n + m); i++) {
		if (filled[i] > 0) {
			continue;
		}
		filled[i] = t;
		fill_row(i, 0, 0, 0);

		if (!filled[i]) {
			all_filled = 1;
		}

		solution[i].length = cube_len[i];
		//printf("%d - %d\n", i, cube_len[i]);
	}
	all_filled = !all_filled;
}

//??set priority based on solution.length - no need

void process_queue() {
	queue_item q;
	while (queue_len > 0) {
		q = queue[--queue_len];
		//printf("-\t%d\t%d\t%d\n", q.index, q.bit_index, q.bit);

		apply_bit(q);
	}
}

void make_tail() {
	int i, j, sum;
	for (i = 0; i < (n + m); i++) {
		sum = 0;
		for (j = 0; j < raw_length[i]; j++) {
			sum = sum + raw[i][j] + 1;
		}
		for (j = 0; j < raw_length[i]; j++) {
			tail[i][j] = sum - 1;
			sum = sum - raw[i][j] - 1;
		}
	}
}

int get_hyphens() {
	int h = 0;
	int i;

	for (i = 0; i < (n + m); i++) {
		if (!filled[i]) {
			h++;
		}
	}

	return h;
}

unsigned long long int get_progress() {
	unsigned long long int p = 0;
	int i;

	for (i = 0; i < (n + m); i++) {
		p = p + solution[i].length;
	}

	return p;
}

int main() {
	int i, j, t, h, h_prev;
	char *s, *buf;
	char raw_num[3];
	unsigned long long p, p_prev;

	s = calloc(100, sizeof(char));

	//suppose input is good, no sanity check
	scanf("%d %d", &n, &m);
	//printf("%d %d\n", n, m);

	for (i = 0; i < (n + m);) {
		gets(s);
		//printf("%s\n", s);
		raw_length[i] = 0;
		buf = s;
		while (sscanf(buf, "%s", raw_num) != -1) {
			raw[i][raw_length[i]++] = atoi(raw_num);
			buf = strstr(buf, raw_num) + strlen(raw_num);
		}
		if (!raw_length[i]) {
			continue;
		}
		i++;
	}

	/***** check input
	for (i = 0; i < (n + m); i++) {
		for (j = 0; j < raw_length[i]; j++) {
			printf("%d\t", raw[i][j]);
		}
		printf("\n");
	}
	*****/

	init_cube();
	init_solution();
	make_tail();

	/*****
	* Suppose we have straight solution,
	* i.e. no need to guess, have stack and branches.
	* but we will provide compatible solution.
	*****/

	/*****
	* +check for progress  
	*****/

	for (t = 1; ; t++) {
		printf("Try #%d\n", t);
		fill_cube(t);
		init_queue();
		process_queue();

		if (all_filled) {
			break;
		}

		if (t == 1) {
			p_prev = get_progress();
			h_prev = get_hyphens();
			//break;
		}

		if (t > 1) {
			h = get_hyphens();
			p = get_progress();
			if (h < h_prev) {
				h_prev = h;
				p_prev = p;
			}
			else {
				if (p < p_prev) {
					p_prev = p;
				}
				else {
					printf("No progress...\n");
					break;
				}
			}
		}
	}

	print_cube();

	free_solution();
	free_cube();
	free(s);

	return 0;
}

