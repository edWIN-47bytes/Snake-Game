#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

typedef struct node{
	int x,y;
	struct node *link;
}node;

void borders(int left, int right, int up, int down){

	for(int i=left; i<=right; i++){
		mvaddch(up,i,ACS_HLINE);                    //inner border
		mvaddch(down,i,ACS_HLINE);
	}
	for(int i=up+1; i<=down; i++){
		mvaddch(i,left,ACS_VLINE);
		mvaddch(i,right,ACS_VLINE);
	}

	for(int i=left-1; i<=right+1; i++){
		mvaddch(up-1,i,ACS_HLINE);                 //outer border
		mvaddch(down+1,i,ACS_HLINE);
	}
	for(int i=up; i<=down+1; i++){
		mvaddch(i,left-1,ACS_VLINE);
		mvaddch(i,right+1,ACS_VLINE);
	}
	
    mvaddch(up, left, ACS_ULCORNER);
    mvaddch(up, right, ACS_URCORNER);               //inner box corners
    mvaddch(down, left, ACS_LLCORNER);
    mvaddch(down, right, ACS_LRCORNER);

    mvaddch(up-1, left-1, ACS_ULCORNER);
    mvaddch(up-1, right+1, ACS_URCORNER);           //outer box corners
    mvaddch(down+1, left-1, ACS_LLCORNER);
    mvaddch(down+1, right+1, ACS_LRCORNER);
}


int random_between(int min, int max){
	return min+rand() % (max-min+1);
}

void game_over(int score){
	erase();
	int max_y;
	int max_x;
	getmaxyx(stdscr, max_y, max_x);

	char *gameover="GAME OVER";
	char finalscore[50];
	snprintf(finalscore, sizeof(finalscore), "FINAL SCORE: %d", score);
	char *re = "press 'q' to quit or 'r' to restart";

	int go_x=(max_x-(int)strlen(gameover))/2;
	int fscore_x=(max_x-(int)strlen(finalscore))/2;
	int re_x =(max_x-(int)strlen(re))/2;

	attron(A_BOLD);
	mvprintw(max_y/2-2, go_x, "%s", gameover);
	attroff(A_BOLD);

	attron(A_STANDOUT);
	mvprintw(max_y/2, fscore_x, "%s", finalscore);
	attroff(A_STANDOUT);

	mvprintw(max_y/2+3, re_x, "%s", re);
	refresh();

	int ch;
	nodelay(stdscr, FALSE);
	while ((ch=getch())){
		if (ch=='q') break;
		if (ch=='r'){

			erase();
			refresh();
			endwin();
			execlp("./a.out", "./a.out", NULL);
			exit(0);
		}
	}
}

int main(){
	initscr();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	srand(time(NULL));

	int score= 0;
	bool runs= true;

	// snake initial body
	node *head= malloc(sizeof(node));
	head->x =3;
	head->y = 3;
	node *mid = malloc(sizeof(node));
	mid->x = 2;
	mid->y = 3;
	node *tail = malloc(sizeof(node));
	tail->x = 1;
	tail->y = 3;
	head->link= mid;
	mid->link= tail;
	tail->link = NULL;

	int dir_x = 1;
	int dir_y = 0;
	
	//screen extremes
	int left = 2;
	int right = 75;
	int up = 2;
	int down = 22;
    
    //start food
	int food_x = random_between(left + 1, right - 1);
	int food_y = random_between(up + 1, down - 1);

	while(runs) {
		int input = getch();

		switch(input){
		case KEY_LEFT:
			if (dir_x!=1){
				dir_x= -1;
				dir_y= 0;
			}
			break;
		case KEY_RIGHT:
			if (dir_x!=-1){
				dir_x= 1;
				dir_y= 0;
			}
			break;
		case KEY_UP:
			if (dir_y!= 1){
				dir_x= 0;
				dir_y= -1;
			}
			break;
		case KEY_DOWN:
			if (dir_y!= -1){
				dir_x= 0;
				dir_y= 1;
			}
			break;
		case 'q':
			runs=false;
			continue;
		}

		int prev_x= head->x;
		int prev_y= head->y;
		head->x += dir_x;
		head->y += dir_y;

		node *ptr= head->link;
		while(ptr!=NULL) {
			int temp_x= ptr->x;
			int temp_y= ptr->y;
			ptr->x= prev_x;
			ptr->y= prev_y;
			prev_x= temp_x;
			prev_y= temp_y;
			ptr= ptr->link;
		}

		//wall collision
		if(head->x==left || head->x==right || head->y==up || head->y==down)
			break;

		//self collision
		node *self= head->link;
		while(self!=NULL) {
			if(self->x==head->x && self->y==head->y){
				runs=false;
				break;
			}
			self= self->link;
		}



		if(head->x==food_x && head->y==food_y){
			node *new_tail = malloc(sizeof(node));
			new_tail->x = prev_x;
			new_tail->y = prev_y;
			new_tail->link = NULL;
			tail->link = new_tail;
			tail = new_tail;

			score++;

			//new food 
			bool valid;
			do{
				valid= true;
				food_x= random_between(left + 1, right - 1);
				food_y= random_between(up + 1, down - 1);

				node *chk=head;
				while(chk!=NULL) {
					if (chk->x==food_x && chk->y==food_y) {
						valid= false;
						break;
					}
					chk= chk->link;
				}
			}while(!valid);
		}

		erase();
		mvprintw(0, 33, "Score: %d", score);
		mvaddch(food_y, food_x, '*');
		mvaddch(head->y, head->x, '@');
		node *s = head->link;
		while(s != NULL) {
			mvaddch(s->y, s->x, 'o');
			s= s->link;
		}

		borders(left, right, up, down);
		refresh();
		usleep(150000);
	}

	//freeing memory
	node *tmp;
	while(head != NULL) {
		tmp = head;
		head = head->link;
		free(tmp);
	}

	game_over(score);
	endwin();
	return 0;
}
