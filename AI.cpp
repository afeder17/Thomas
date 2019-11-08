/*.cpp file for the AI class, the artificial intelligence playing the game. 
Uses a minimax tree to make move decisions, with tree geometry optimized to 
balance runtime and performance.

Has 5 difficulty levels, corresponding to search depths of 3, 5, and 9, and then 2- and 30- second 
search times. Tree uses alpha-beta pruning to increase search efficiency, pruning away vast 
majority of possible game tree leaves by disregarding branches where opponent 
can force a sub-optimal position.*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cctype>
#include <cmath>
#include <ctime>
#include <chrono>
#include <unordered_map>
#include "board.h"
#include "AI.h"

using namespace std;

//constructor
//parameters: NA
//returns: NA
AI::AI() {
    state = new Board;

    name = "Thomas";

    srand(time(NULL));
    
    //default, impossible values for AI's chosen move
    row1 = 8;
    col1 = 'I';
    row2 = 8;
    col2 = 'I';

    diverge = false, repeat = false, clear = false;

    int cutoff_start = -10000;
    for (int i = 0; i < 100; i++) {
        cutoffs[i] = cutoff_start;
        cutoff_start = -cutoff_start;
    }

    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 64; j++) {
            for (int k = 0; k < 64; k++) {
                history[i][j][k] = 0;
            }
        }
    }

    difficulty = 3;
    time_limit = 60;

    memory->reserve(1000000); //reserve starting table space to reduce reallocation time
}

//destructor
//parameters: NA
//returns: NA
AI::~AI() {
    delete state;
    memory->clear();
    delete memory;
}

//intro, prints an intro message
//parameters: NA
//returns: void
void AI::intro() {
    cout << "Hello, I am " << name << ", your AI opponent\n";
    system("say Hello, I am Thomas, your AI opponent");
}

//set_difficulty, sets search depth limit directly or by search time
//parameters: an int for the difficulty level
//returns: void
void AI::set_difficulty(int level) {
    if (level == 1) {
       difficulty = 3;
    } else if (level == 2) {
        difficulty = 5;
    } else if (level == 3) {
        difficulty = 9;
    } else if (level == 4) {
        difficulty = 20;
        time_limit = 2;
    } else {
        difficulty = 20;
        time_limit = 30;
    }
}

//update_AI, updates the board held by the AI to the game board
//parameters: a Board to put in the copy constructor
//returns: void
void AI::update_AI(const Board ref) {
    int pieces = state->get_num_black() + state->get_num_white();
    
    delete state;
    state = new Board(ref); //copy over board data
    
    if (state->get_num_black() + state->get_num_white() < pieces)
        clear = true;
}

//move, AI makes its decision about where to move
//parameters: a bool for whether or not to choose a somewhat sub-optimal move, to break repetitions
//returns: void
void AI::move(bool sub, bool go) {
    int count = 0, make; //number of possible moves, number iterated through so far
    char c1 = 'A', c2 = 'A', taken = 'X'; //position variables
    int r1 = 0, r2 = 0;
    int *moves_r = new int[180]; //arrays with square locations for possible moves
    char *moves_c = new char[180];
    bool restore = false;

    pre_move(sub, go);
    
    //ensure that at least some calculation takes place in timed mode
    while ((difficulty < 8) && (time(NULL) - start_time >= time_limit)) {
        start_time++;
    }

    iterative_deepening(sub);

    Node *start = new Node; //make root Node, with given position as board
    start->position = state;
    fill_b(start, moves_c, moves_r, count, NA, restore); //fill move arrays with possible moves
    start->options = new Node[count]; //create subtrees for each possible move

    cutoffs[1] = -10000; //reset alpha
    
    for (make = 0; make < count; make++) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, true, taken); //fill child node
        evaluate_move_b(start, c1, r1, c2, r2, make, 0);
        if (start->options[make].score > cutoffs[1])
            cutoffs[1] = start->options[make].score; //set alpha

        if ((closeness(*start->options[make].position, 'W') < closeness(*start->position, 'W')) && 
        (start->position->get_num_black() >= start->position->get_num_white()) && 
        (start->position->get_num_white() < 5))
            start->options[make].score += rand () % 6; //weight aggressive king moves 

        delete start->options[make].position;
    }
    
    choose_move(start, count, make, sub);
    
    if (go)
        state->make_move(col1, row1, col2, row2), repeat = false;
    
    delete [] moves_r;
    delete [] moves_c;
    delete [] start->options;
    delete start;
}

//multi, similar to move, but used when a piece has taken and my do so again. Evaluates best 
//possible lines of jumps for the piece, makes decision at each jump, possibly repeatedly
//parameters: NA
//returns: a bool for whether or not futher jump can be made after the selected move
bool AI::multi(bool go) {
    char c1 = col2, c2 = 'A', taken = 'X'; //position variables
    int r1 = row2, r2 = 0;
    int count = 0, make; //number of possible moves, number iterated through so far
    int *moves_r = new int[8]; //arrays with square locations for possible moves
    char *moves_c = new char[8];

    //ensure that at least some calculation takes place in timed mode
    while ((difficulty < 8) && (time(NULL) - start_time >= time_limit)) {
        start_time++;
    }

    pre_move(false, go);

    if (difficulty > 3) {
        difficulty--;
        multi(false);
        difficulty++;
    }
    
    Node *start = new Node; //make root Node, with given position as board value
    start->position = state;

    cutoffs[1] = -10000;

    fill_multi(moves_c, moves_r, count, c1, r1, 'B'); //find possible moves from single square

    start->options = new Node[count];

    for (make = 0; make < count; make++) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, true, taken); //fill child node

        evaluate_move_b(start, c1, r1, c2, r2, make, 0);

        if (start->options[make].score > cutoffs[1])
            cutoffs[1] = start->options[make].score; //set alpha-beta value

        delete start->options[make].position;
    }

    choose_move(start, count, make, false);

    if (go)
        state->make_move(col1, row1, col2, row2);

    delete [] moves_c;
    delete [] moves_r;
    delete [] start->options;
    delete start;

    return ((state->jump_possible(col2, row2, 'B')) && (!state->kinged()));
}

//functions to get notation of decided move

//get_row1, get row of first square of decided move
//parameters: NA
//returns: a char for the row
char AI::get_row1() {
    return row1;
}

//get_col1, get col of first square of decided move
//parameters: NA
//returns: an int for the col
int AI::get_col1() {
    return col1;
}

//get_row2, get row of second square of decided move
//parameters: NA
//returns: a char for the row
char AI::get_row2() {
    return row2;
}

//get_col2, get col of first square of decided move
//parameters: NA
//returns: an int for the col
int AI::get_col2() {
    return col2;
}

//calc, evaluates the positional score of a leaf Node position
//parameters: a ref to a Board object
//returns: and integer for the position's score
int AI::calc(Board &ref) {
    //piece scores
    int black = 0, white = 0;
    char num_b = ref.get_num_black(), num_w = ref.get_num_white(), R, C;

    //(incomplete) check for win or loss based on number of pieces
    if (num_w == 0) {
        return 9900;
    } else if (num_b == 0) {
        return -9900;
    }

    //iterate through the white pieces, giving each a value based on its type and locations
    for (int i = 0; i < num_w; i++) {
        R = ref.get_place_row_w(i);
        C = ref.get_place_col_w(i);

        //kings are worth about 1.5 times as much as regular pieces, and more if close to oppossing 
        //pieces if the player is ahead in pieces
        //iterate through white pieces
        if (ref.get_place_king_w(i)) {
            if (num_w > num_b) {
                white += 336 - (3 * proximity(R, C, 'W', ref, num_b, num_w));
            } else {
                white += 336 - (1 * proximity(R, C, 'W', ref, num_b, num_w));
            }

            if (((R == 2) && (ref.look(R - 2, C) == BLACK_KING)) || ((R == 5) && 
            (ref.look(R + 2, C) == BLACK_KING)) || ((C == 2) && (ref.look(R, C - 2) == BLACK_KING)) ||
            ((C == 5) && (ref.look(R, C + 2) == BLACK_KING))) {
                white += 20;
            }
            
        } else {
            white += 214 - 2 * R;

            if ((C == 0) || (C == DIMEN_LESS1))
                white += 4;

            if (((R < DIMEN_LESS1) && (C > 0) && (ref.look(R + 1, C - 1) == WHITE_PIECE)) || 
            ((R < DIMEN_LESS1) && (C < DIMEN_LESS1) && ref.look(R + 1, C + 1) == WHITE_PIECE))
                white += 4;
        }
    }

    //iterate through black pieces
    for (int i = 0; i < num_b; i++) {
        R = ref.get_place_row_b(i);
        C = ref.get_place_col_b(i);

        if (ref.get_place_king_b(i)) {
            if (num_b > num_w) {
                black += 336 - (3 * proximity(R, C, 'B', ref, num_b, num_w));
            } else {
                black += 336 - (1 * proximity(R, C, 'B', ref, num_b, num_w));
            }

            if (((R == 2) && (ref.look(R - 2, C) == WHITE_KING)) || ((R == 5) && 
            (ref.look(R + 2, C) == WHITE_KING)) || ((C == 2) && (ref.look(R, C - 2) == WHITE_KING)) ||
            ((C == 5) && (ref.look(R, C + 2) == WHITE_KING))) {
                black += 20;
            }

        } else {
            black += 193 + 2 * R;

            if ((C == 0) || (C == DIMEN_LESS1))
                black += 4;

            if (((R > 0) && (C < DIMEN_LESS1) && (ref.look(R - 1, C + 1) == BLACK_PIECE)) || 
            ((R > 0) && (C > 0) && ref.look(R - 1, C - 1) == BLACK_PIECE))
                black += 4;
        }
    }

    //if there is a single piece of a color, assign value to its being in a double corner or close to center
    if ((num_b == 1) && (num_w > 1)) {
        if ((ref.look(6, 7) == BLACK_KING) || (ref.look(7, 6) == BLACK_KING) || 
        (ref.look(0, 1) == BLACK_KING) || (ref.look(1, 0) == BLACK_KING) ||
        (ref.look(6, 5) == BLACK_KING) || (ref.look(5, 6) == BLACK_KING) ||
        (ref.look(1, 2) == BLACK_KING) || (ref.look(2, 1) == BLACK_KING))
            black += 50;
    } else if ((num_w == 1) && (num_b > 1)) {
        if ((ref.look(6, 7) == WHITE_KING) || (ref.look(7, 6) == WHITE_KING) || 
        (ref.look(0, 1) == WHITE_KING) || (ref.look(1, 0) == WHITE_KING) || 
        (ref.look(6, 5) == WHITE_KING) || (ref.look(5, 6) == WHITE_KING) ||
        (ref.look(1, 2) == WHITE_KING) || (ref.look(2, 1) == WHITE_KING))
            white += 50;
    }

    //value trapping an opposing king in a single square corner
    if ((ref.look(0, 7) == WHITE_KING) && (ref.look(2, 5) == BLACK_KING)) {
        black += 25;
    } else if ((ref.look(0, 7) == BLACK_KING) && (ref.look(2, 5) == WHITE_KING)) {
        white += 25;
    }

    if ((ref.look(7, 0) == WHITE_KING) && (ref.look(5, 2) == BLACK_KING)) {
        black += 25;
    } else if ((ref.look(7, 0) == BLACK_KING) && (ref.look(5, 2) == WHITE_KING)) {
        white += 25;
    }

    //value of having more pieces increases exponentially with declining number of pieces
    if (num_w > num_b) {
        int temp = (num_w - num_b) * (17 - num_b) * (17 - num_b);

        if (temp < 5000) {
            white += temp;
        } else {
            white += 5000; //avoid value here higher than winning value
        }
    } else if (num_b > num_w) {
        int temp = (num_b - num_w) * (17 - num_w) * (17 - num_w);

        if (temp < 5000) {
            black += temp;
        } else {
            black += 5000; //avoid value here higher than winning value
        }
    }
    
    return (black - white);
}

//proximity, finds how close a piece is to the nearest opposing king
//parameters: an int for the piece's row, an int for its column, a string for the color of the piece, a ref 
//to a Board object to scan, ints for the numbers of black and white pieces
//returns: an int for the distance to the nearest opposing king
int AI::proximity(int row, int column, char turn, Board &ref, int num_b, int num_w) {
    int dist = 5, x = 0, y = 0;

    if (turn == 'W') {
        for (int i = 0; i < num_b; i++) {
        if (ref.get_place_king_b(i)) {
                x = abs(ref.get_place_col_b(i) - column);
                y = abs(ref.get_place_row_b(i) - row);
    
                if ((x < dist) && (y < dist)) {
                    if (x > y) {
                        dist = x;
                    } else {
                        dist = y;
                    }
                }  
            }
        }
    } else {
        for (int i = 0; i < num_w; i++) {
            if (ref.get_place_king_w(i)) {
                x = abs(ref.get_place_col_w(i) - column);
                y = abs(ref.get_place_row_w(i) - row);

                if ((x < dist) && (y < dist)) {
                    if (x > y) {
                        dist = x;
                    } else {
                        dist = y;
                    }
                }
            }
        }
    }

    return dist; //dist is smaller of column difference and row difference
}

//fill_b, fills move arrays with coordinates, testing all possible movements for each piece in the Board 
//object's black piece position arrays
//parameters: pointer to a Node, a char array of column coordinates, an int array of row coordinates, 
//and a pointer to the count integer
//returns: void
int AI::fill_b(Node *start, char moves_c[], int moves_r[], int &count, int depth, bool &restore) {
    string *look = make_key(*start->position, 'B');
    unordered_map<string, Mem_node>::iterator place = memory->find(*look);
    if (place != memory->end()) {
        count = place->second.children;
        if ((((state->get_num_white() > 3) && (place->second.depth >= depth)) || 
        (place->second.depth == depth)) && (depth != NA)) {
            delete [] moves_c;
            delete [] moves_r;
            return place->second.score;
        } else {
            for (int i = 0; i < 2 * place->second.children; i++) {
                moves_c[i] = place->second.child_c[i];
                moves_r[i] = place->second.child_r[i];
            }
            count = place->second.children;
        }
    } else {
        char c1;
        int r1;
        bool forced = start->position->forced_take('B'); //check for forced jump
        if ((!forced) && (!start->position->any_move('W')) && (depth != NA)) {
            delete [] moves_c;
            delete [] moves_r;
            return -9950 - depth; //check for loss, value modified by depth
        }
        for (int i = 0; i < start->position->get_num_black(); i++) {
            //iterate through each black piece
            c1 = start->position->get_place_col_b(i) + 'A';
            r1 = start->position->get_place_row_b(i);
            if (!forced) {
                if ((r1 < DIMEN_LESS1) && (c1 - 'A' < DIMEN_LESS1) && 
                (start->position->look(r1 + 1, c1 + 1 - 'A') == BLACK_SQUARE)) {
                    moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 1, 
                    moves_r[(count * 2) + 1] = r1 + 1;
                    count++;
                }
                if ((r1 < DIMEN_LESS1) && (c1 - 'A' > 0) &&
                (start->position->look(r1 + 1, c1 - 1 - 'A') == BLACK_SQUARE)) {
                    moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 1, 
                    moves_r[(count * 2) + 1] = r1 + 1;
                    count++;
                }
            } else {
                if ((r1 < DIMEN - 2) && (c1 - 'A' < DIMEN - 2) && 
                ((start->position->look(r1 + 1, c1 + 1 - 'A') == WHITE_PIECE) || 
                (start->position->look(r1 + 1, c1 + 1 - 'A') == WHITE_KING)) &&
                (start->position->look(r1 + 2, c1 + 2 - 'A') == BLACK_SQUARE)) {
                    moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 2, 
                    moves_r[(count * 2) + 1] = r1 + 2;
                    count++;
                }
                if ((r1 < DIMEN - 2) && (c1 - 'A' > 1) && 
                ((start->position->look(r1 + 1, c1 - 1 - 'A') == WHITE_PIECE) || 
                (start->position->look(r1 + 1, c1 - 1 - 'A') == WHITE_KING)) &&
                (start->position->look(r1 + 2, c1 - 2 - 'A') == BLACK_SQUARE)) {
                    moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 2, 
                    moves_r[(count * 2) + 1] = r1 + 2;
                    count++;
                }
            }
            
            //check backwards movements for kings
            if (start->position->get_place_king_b(i)) {
                if (!forced) {
                    if ((r1 > 0) && (c1 - 'A' < DIMEN_LESS1) &&
                    (start->position->look(r1 - 1, c1 + 1 - 'A') == BLACK_SQUARE)) {
                        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 1, 
                        moves_r[(count * 2) + 1] = r1 - 1;
                        count++;
                    }
                    if ((r1 > 0) && (c1 - 'A' > 0) &&
                    (start->position->look(r1 - 1, c1 - 1 - 'A') == BLACK_SQUARE)) {
                        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 1, 
                        moves_r[(count * 2) + 1] = r1 - 1;
                        count++;
                    }
                } else {
                    if ((r1 > 1) && (c1 - 'A' < DIMEN - 2) && 
                    ((start->position->look(r1 - 1, c1 + 1 - 'A') == WHITE_PIECE) || 
                    (start->position->look(r1 - 1, c1 + 1 - 'A') == WHITE_KING)) &&
                    (start->position->look(r1 - 2, c1 + 2 - 'A') == BLACK_SQUARE)) {
                        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 2, 
                        moves_r[(count * 2) + 1] = r1 - 2;
                        count++;
                    }
                    if ((r1 > 1) && (c1 - 'A' > 1) && 
                    ((start->position->look(r1 - 1, c1 - 1 - 'A') == WHITE_PIECE) || 
                    (start->position->look(r1 - 1, c1 - 1 - 'A') == WHITE_KING)) &&
                    (start->position->look(r1 - 2, c1 - 2 - 'A') == BLACK_SQUARE)) {
                        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 2, 
                        moves_r[(count * 2) + 1] = r1 - 2;
                        count++;
                    }
                }
            }
        }

        b_order(moves_c, moves_r, depth, count);
    }

    if (start->position->kinged())
        restore = true;
    
    return NA;
}

//fill_w, fills move arrays with coordinates, testing all possible movements for each piece in the Board 
//object's white piece position arrays
//parameters: pointer to a Node, a char array of column coordinates, an int array of row coordinates, 
//and a pointer to the count integer
//returns: void
int AI::fill_w(Node *start, char moves_c[], int moves_r[], int &count, int depth, bool &restore) {
    string *look = make_key(*start->position, 'W');
    unordered_map<string, Mem_node>::iterator place = memory->find(*look);
    if (place != memory->end()) {
        count = place->second.children;
        if ((((state->get_num_black() > 3) && (place->second.depth >= depth)) || 
        (place->second.depth == depth)) && (depth != NA)) {
            delete [] moves_c;
            delete [] moves_r;
            return place->second.score;
        } else {
            for (int i = 0; i < 2 * place->second.children; i++) {
                moves_c[i] = place->second.child_c[i];
                moves_r[i] = place->second.child_r[i];
            }
            count = place->second.children;
        }
    } else {
        char c1;
        int r1;
        bool forced = start->position->forced_take('W'); //check for forced jump
        if ((!forced) && (!start->position->any_move('B')) && (depth != NA)) {
            delete [] moves_c;
            delete [] moves_r;
            return 9950 + depth; //check for win, value modified by depth
        }
        for (int i = 0; i < start->position->get_num_white(); i++) {
            //iterate through each white piece
            c1 = start->position->get_place_col_w(i) + 'A';
            r1 = start->position->get_place_row_w(i);
            if (!forced) {
                if ((r1 > 0) && (c1 - 'A' < DIMEN_LESS1) &&
                (start->position->look(r1 - 1, c1 + 1 - 'A') == BLACK_SQUARE)) {
                    moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 1, 
                    moves_r[(count * 2) + 1] = r1 - 1;
                    count++;
                }
                if ((r1 > 0) && (c1 - 'A' > 0) &&
                (start->position->look(r1 - 1, c1 - 1 - 'A') == BLACK_SQUARE)) {
                    moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 1, 
                    moves_r[(count * 2) + 1] = r1 - 1;
                    count++;
                }
            } else {
                if ((r1 > 1) && (c1 - 'A' < DIMEN - 2) && 
                ((start->position->look(r1 - 1, c1 + 1 - 'A') == BLACK_PIECE) || 
                (start->position->look(r1 - 1, c1 + 1 - 'A') == BLACK_KING)) &&
                (start->position->look(r1 - 2, c1 + 2 - 'A') == BLACK_SQUARE)) {
                    moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 2, 
                    moves_r[(count * 2) + 1] = r1 - 2;
                    count++;
                }
                if ((r1 > 1) && (c1 - 'A' > 1) && 
                ((start->position->look(r1 - 1, c1 - 1 - 'A') == BLACK_PIECE) || 
                (start->position->look(r1 - 1, c1 - 1 - 'A') == BLACK_KING)) &&
                (start->position->look(r1 - 2, c1 - 2 - 'A') == BLACK_SQUARE)) {
                    moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 2, 
                    moves_r[(count * 2) + 1] = r1 - 2;
                    count++;
                }
            }

            //check backwards movements for kings
            if (start->position->get_place_king_w(i)) {
                if (!forced) {
                    if ((r1 < DIMEN_LESS1) && (c1 - 'A' < DIMEN_LESS1) && 
                    (start->position->look(r1 + 1, c1 + 1 - 'A') == BLACK_SQUARE)) {
                        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 1, 
                        moves_r[(count * 2) + 1] = r1 + 1;
                        count++;
                    }
                    if ((r1 < DIMEN_LESS1) && (c1 - 'A' > 0) &&
                    (start->position->look(r1 + 1, c1 - 1 - 'A') == BLACK_SQUARE)) {
                        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 1, 
                        moves_r[(count * 2) + 1] = r1 + 1;
                        count++;
                    }
                } else {
                    if ((r1 < DIMEN - 2) && (c1 - 'A' < DIMEN - 2) && 
                    ((start->position->look(r1 + 1, c1 + 1 - 'A') == BLACK_PIECE) || 
                    (start->position->look(r1 + 1, c1 + 1 - 'A') == BLACK_KING)) &&
                    (start->position->look(r1 + 2, c1 + 2 - 'A') == BLACK_SQUARE)) {
                        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 2, 
                        moves_r[(count * 2) + 1] = r1 + 2;
                        count++;
                    }
                    if ((r1 < DIMEN - 2) && (c1 - 'A' > 1) && 
                    ((start->position->look(r1 + 1, c1 - 1 - 'A') == BLACK_PIECE) || 
                    (start->position->look(r1 + 1, c1 - 1 - 'A') == BLACK_KING)) &&
                    (start->position->look(r1 + 2, c1 - 2 - 'A') == BLACK_SQUARE)) {
                        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 2, 
                        moves_r[(count * 2) + 1] = r1 + 2;
                        count++;
                    }
                }
            }   
        }
        
        w_order(moves_c, moves_r, depth, count);
    }
    

    if (start->position->kinged())
        restore = true;
    
    return NA;
}

//diverge_b, fills the position arrays similarly to the fill functions, but only looks for jumps from the 
//coordinate placed in the d_col and d_row variables, for when one black piece has multiple options 
//on a second or further jump
//parameters: pointer to a Node, a char array of column coordinates, an int array of row coordinates, 
//and a pointer to the count integer
//returns: void
int AI::diverge_b(Node *start, char moves_c[], int moves_r[], int &count, int depth) {
    diverge = false; //make sure next fill function called is regular
    string *look = make_key(*start->position, 'B');
    unordered_map<string, Mem_node>::iterator place = memory->find(*look);
    if (place != memory->end()) {
        count = place->second.children;
        if ((((state->get_num_white() > 3) && (place->second.depth >= depth)) || 
        (place->second.depth == depth)) && (depth != NA)) {
            delete [] moves_c;
            delete [] moves_r;
            return place->second.score;
        } else {
            for (int i = 0; i < 2 * place->second.children; i++) {
                moves_c[i] = place->second.child_c[i];
                moves_r[i] = place->second.child_r[i];
            }
            count = place->second.children;
        }
    } else {
        char c1 = d_col;
        int r1 = d_row;
        
        //check all possible jumps from a single square
        if (start->position->simple_check(c1, r1, c1 + 2, r1 + 2, 'B')) {
            moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 2, 
            moves_r[(count * 2) + 1] = r1 + 2;
            count++;
        }
        if (start->position->simple_check(c1, r1, c1 + 2, r1 - 2, 'B')) {
            moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 2, 
            moves_r[(count * 2) + 1] = r1 - 2;
            count++;
        }
        if (start->position->simple_check(c1, r1, c1 - 2, r1 + 2, 'B')) {
            moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 2, 
            moves_r[(count * 2) + 1] = r1 + 2;
            count++;
        }
        if (start->position->simple_check(c1, r1, c1 - 2, r1 - 2, 'B')) {
            moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 2, 
            moves_r[(count * 2) + 1] = r1 - 2;
            count++;
        }
    }
    
    return NA;
}

//diverge_w, fills the position arrays similarly to the fill functions, but only looks for jumps from the 
//coordinate placed in the d_col and d_row variables, for when one white piece has multiple options 
//on a second or further jump
//parameters: pointer to a Node, a char array of column coordinates, an int array of row coordinates, 
//and a pointer to the count integer
//returns: void
int AI::diverge_w(Node *start, char moves_c[], int moves_r[], int &count, int depth) {
    diverge = false; //make sure next fill function called is regular
    string *look = make_key(*start->position, 'W');
    unordered_map<string, Mem_node>::iterator place = memory->find(*look);
    if (place != memory->end()) {
        count = place->second.children;
        if (place->second.depth >= depth) {
            delete [] moves_c;
            delete [] moves_r;
            return place->second.score;
        } else {
            for (int i = 0; i < 2 * place->second.children; i++) {
                moves_c[i] = place->second.child_c[i];
                moves_r[i] = place->second.child_r[i];
            }
            count = place->second.children;
        }
    } else {
        char c1 = d_col;
        int r1 = d_row;
        
        //check all possible jumps from a single square
        if (start->position->simple_check(c1, r1, c1 + 2, r1 + 2, 'W')) {
            moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 2, 
            moves_r[(count * 2) + 1] = r1 + 2;
            count++;
        }
        if (start->position->simple_check(c1, r1, c1 + 2, r1 - 2, 'W')) {
            moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 2, 
            moves_r[(count * 2) + 1] = r1 - 2;
            count++;
        }
        if (start->position->simple_check(c1, r1, c1 - 2, r1 + 2, 'W')) {
            moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 2, 
            moves_r[(count * 2) + 1] = r1 + 2;
            count++;
        }
        if (start->position->simple_check(c1, r1, c1 - 2, r1 - 2, 'W')) {
            moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 2, 
            moves_r[(count * 2) + 1] = r1 - 2;
            count++;
        }
    }
    
    return NA;
}

//fill_multi, helper function to multi, similar to the other fill functions, but only looks for jumps from a 
//particular square, indicated in the parameters
//parameters: a char array of column coordinates, and int array of row coordinates, an int ref to  
//count, an int ref to make, a char and int for the particular square, a string for the color of the AI
//returns: void
void AI::fill_multi(char moves_c[], int moves_r[], int &count, char c1, int r1, char color) {
    if (state->check_validity(c1, r1, c1 + 2, r1 + 2, color)) {
        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 2, 
        moves_r[(count * 2) + 1] = r1 + 2;
        count++;
    }
    if (state->check_validity(c1, r1, c1 + 2, r1 - 2, color)) {
        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 2, 
        moves_r[(count * 2) + 1] = r1 - 2;
        count++;
    }
    if (state->check_validity(c1, r1, c1 - 2, r1 + 2, color)) {
        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 2, 
        moves_r[(count * 2) + 1] = r1 + 2;
        count++;
    }
    if (state->check_validity(c1, r1, c1 - 2, r1 - 2, color)) {
        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 - 2, 
        moves_r[(count * 2) + 1] = r1 - 2;
        count++;
    }
}

//moving_b, creates Board objects in the children of the passed Node, copying the start Node's 
//Board and making the corresponding move from the move arrays for black
//parameters: pointer to a Node, a char array of column coordinates, an int array of row coordinates, 
// a pointer to the count integer and chars and ints for four coordinate points
//returns: void
void AI::moving_b(Node *start, char moves_c[], int moves_r[], int &make, char &c1, char &c2, int 
&r1, int &r2, bool copy, char &taken) {
    c1 = moves_c[make * 2];
    r1 = moves_r[make * 2];
    c2 = moves_c[(make * 2) + 1];
    r2 = moves_r[(make * 2) + 1];

    if (abs(r2 - r1) == 2)
        taken = start->position->look((r1 + r2) / 2, ((c1 + c2) / 2) - 'A');

    if (copy) {
        start->options[make].position = new Board(*start->position); //make child move
        start->options[make].position->make_move(c1, r1, c2, r2);
    } else {
        start->position->make_move(c1, r1, c2, r2);
        start->options[make].position = start->position;
    }

    start->options[make].row1 = r1, start->options[make].row2 = r2; //put move coordinates in child
    start->options[make].col1 = c1, start->options[make].col2 = c2;
}

//moving_w, creates Board objects in the children of the passed Node, copying the start Node's 
//Board and making the corresponding move from the move arrays for white
//parameters: pointer to a Node, a char array of column coordinates, an int array of row coordinates, 
// a pointer to the count integer and chars and ints for four coordinate points
//returns: void
void AI::moving_w(Node *start, char moves_c[], int moves_r[], int &make, char &c1, char &c2, int 
&r1, int &r2, bool copy, char &taken) {
    c1 = moves_c[make * 2];
    r1 = moves_r[make * 2];
    c2 = moves_c[(make * 2) + 1];
    r2 = moves_r[(make * 2) + 1];

    if (abs(r2 - r1) == 2)
        taken = start->position->look((r1 + r2) / 2, ((c1 + c2) / 2) - 'A');

    if (copy) {
        start->options[make].position = new Board(*start->position); //make child move
        start->options[make].position->make_move(c1, r1, c2, r2);
    } else {
        start->position->make_move(c1, r1, c2, r2);
        start->options[make].position = start->position;
    }

    start->options[make].row1 = r1, start->options[make].row2 = r2; //put move coordinates in child
    start->options[make].col1 = c1, start->options[make].col2 = c2;
}

//maximize, compares the evaluated scores of the start Node's children, returning the score of the 
//maximum one, and also de-allocating the children
//parameters: pointer to a Node, a char array of column coordinates, an int array of row coordinates, 
//and an int ref to the make counter
//returns: an int for the max score
int AI::maximize(Node *start, char moves_c[], int moves_r[], int count, int depth) {
    Mem_node temp;
    int hold;

    int place[45];
    for (int k = 0; k < count; k++)
        place[k] = k;

    for (int i = 1; i < count; i++) {
        for (int j = i; j > 0; j--) {
            if (start->options[place[j]].score > start->options[place[j - 1]].score) {
                hold = place[j];
                place[j] = place[j - 1];
                place[j - 1] = hold;
            } else {
                break;
            }
        }
    }

    temp.children = count;
    for (int i = 0; i < count; i++) {
        temp.child_c[2 * i] = moves_c[2 * place[i]];
        temp.child_c[(2 * i) + 1] = moves_c[2* place[i] + 1];
        temp.child_r[2 * i] = moves_r[2 * place[i]];
        temp.child_r[(2 * i) + 1] = moves_r[2 * place[i] + 1];
    }

    temp.depth = depth;
    temp.score = start->options[place[0]].score;
    string *access = make_key(*start->position, 'B');
    
    unordered_map<string, Mem_node>::iterator stored = memory->find(*access);
    if (stored != memory->end()) {
            if (stored->second.depth < depth) {
                memory->erase(*access);
                memory->emplace_hint(stored, *access, temp);
            }
    } else {
        memory->emplace(*access, temp);
    }

    delete [] start->options; //delete array of pointers itself
    delete [] moves_r; //delete stored move coordinates
    delete [] moves_c;
    return temp.score;
}

//minimize, compares the evaluated scores of the start Node's children, returning the score of the 
//minimum one, and also de-allocating the children
//parameters: pointer to a Node, a char array of column coordinates, an int array of row coordinates, 
//and an int ref to the make counter
//returns: an int for the min score
int AI::minimize(Node *start, char moves_c[], int moves_r[], int count, int depth) {
    Mem_node temp;
    int hold;

    int place[45];
    for (int k = 0; k < count; k++)
        place[k] = k;

    for (int i = 1; i < count; i++) {
        for (int j = i; j > 0; j--) {
            if (start->options[place[j]].score < start->options[place[j - 1]].score) {
                hold = place[j];
                place[j] = place[j - 1];
                place[j - 1] = hold;
            } else {
                break;
            }
        }
    }

    temp.children = count;
    for (int i = 0; i < count; i++) {
        temp.child_c[2 * i] = moves_c[2 * place[i]];
        temp.child_c[(2 * i) + 1] = moves_c[2* place[i] + 1];
        temp.child_r[2 * i] = moves_r[2 * place[i]];
        temp.child_r[(2 * i) + 1] = moves_r[2 * place[i] + 1];
    }

    temp.depth = depth;
    temp.score = start->options[place[0]].score;
    string *access = make_key(*start->position, 'W');
    
    unordered_map<string, Mem_node>::iterator stored = memory->find(*access);
    if (stored != memory->end()) {
            if (stored->second.depth < depth) {
                memory->erase(*access);
                memory->emplace_hint(stored, *access, temp);
            }
    } else {
        memory->emplace(*access, temp);
    }

    delete [] start->options; //delete array of pointers itself
    delete [] moves_r; //delete stored move coordinates
    delete [] moves_c;
    return temp.score;
}

//closeness, finds the average distance between the kings of the selected color and the nearest 
//opposing king
//parameters: a ref to a Board, and a string for the color of the kings to measure distance of nearest
//opponent from
//returns: a double for the average king minimum distance
double AI::closeness(Board &ref, char color) {
    double total = 0; //sum of distances
    double kings = 0;
    int num_b = ref.get_num_black(), num_w = ref.get_num_white();

    bool hold = false;
    if (ref.look(0, 0) == BLANK)
        hold = true;
    for (int i = 0; i < DIMEN; i++) {
        for (int j = hold; j < DIMEN; j += 2) {
            if ((color == 'W') && (ref.look(i, j) == WHITE_KING)) {
                if (proximity(i, j, 'W', ref, num_b, num_w) > 2)
                    total += proximity(i, j, 'W', ref, num_b, num_w);
                kings++;
            } else if ((color == 'B') && (ref.look(i, j) == BLACK_KING)) {
                if (proximity(i, j, 'B', ref, num_b, num_w) > 2)
                    total += proximity(i, j, 'B', ref, num_b, num_w);
                kings++;
            }
        }
        hold = (!hold);
    }
    
    return (total/kings);
}

//select, helper function to move, finds the optimal move once game-tree evaluations are completed,
//then sets the make variable to the array number of the optimal child
//parameters: a Node pointer, an int ref to count, an int ref to make, a string for the color of the AI
//returns: void
int AI::select(Node *start, int count, int make, char color) {
    (void) color;

    int rando[150];
    int num_same = 0; //number of possible moves with the same score

    int max = NA;
    for (int i = 0; i < count; i++) {
        if (start->options[i].score > max) {
            max = start->options[i].score;
            make = i;
            num_same = 0;
            rando[num_same] = i; //new highest's number added to first position in randomization array
        } else if (start->options[i].score > max) {
            num_same++;
            rando[num_same] = i; //add later tied moves' child array number to randomization array
        }
    }

    //choose randomly among best moves, with extra weighting for ones that progress the game by 
    //reducing closeness value
    return rando[rand() % (num_same + 1)];
}

//select_second, helper function to move, finds a best move if the usual highest-scoring ones are 
//reduced in score by 30, getting a slightly sub-optimal choice to break repetitions, sets make to it
//parameters: a Node pointer, an int ref to count, an int ref to make, a string for the color of the AI
//returns: void
int AI::select_second(Node *start, int count, int make) {
    int rando[150];
    int max = -10001;
    int num_same = 0; //number of possible moves with the same score

    //find max score
    for (int i = 0; i < count; i++) {
        if (start->options[i].score > max)
            max = start->options[i].score;
    }

    //set all max scores to max - 100, so the best sub-optimal move can be chosen, but only if the AI 
    //isn't badly losing
    for (int i = 0; i < count; i++) {
        if ((start->options[i].score >= max - 100) && (max > -120))
            start->options[i].score = max - 100;
    }

    max = -10001;
    for (int i = 0; i < count; i++) {
        start->options[i].score += rand() % 5;
        
        if (start->options[i].score > max) {
            max = start->options[i].score;
            make = i;
            num_same = 0;
            rando[num_same] = i; //new highest's number added to first position in randomization array
        } else if (start->options[i].score == max) {
            num_same++;
            rando[num_same] = i; //add later tied moves' child array number to randomization array
        }
    }

    //choose randomly among best sub-optimal moves, with extra weighting for ones that progress the
    //game by reducing closeness value
    return rando[rand() % (num_same + 1)];
}

string* AI::make_key(Board &ref, char turn) {
    string *hold = ref.get_key();
    hold->back() = turn;
    
    return hold;
}

void AI::b_order(char moves_c[], int moves_r[], int depth, int count) {
    if (depth == NA)
        return;

    char tempc1, tempc2;
    int tempr1, tempr2, priority = 0;
    for (int i = count - 1; i > 0; i--) {
        if ((moves_c[2 * i] == killer_c[depth * 2]) && (moves_c[2 * i + 1] == killer_c[depth * 2 + 1]) && 
        (moves_r[2 * i] == killer_r[depth * 2]) && (moves_r[2 * i + 1] == killer_r[depth * 2 + 1])) {
            tempc1 = moves_c[2 * i];
            tempc2 = moves_c[2 * i + 1];
            moves_c[2 * i] = moves_c[0];
            moves_c[2 * i + 1] = moves_c[1];
            moves_c[0] = tempc1;
            moves_c[1] = tempc2;

            tempr1 = moves_r[2 * i];
            tempr2 = moves_r[2 * i + 1];
            moves_r[2 * i] = moves_r[0];
            moves_r[2 * i + 1] = moves_r[1];
            moves_r[0] = tempr1;
            moves_r[1] = tempr2;

            priority++;

            break;
        }
    }

    for (int i = priority + 1; i < count; i++) {
        for (int j = i; j > priority; j--) {
            if ((history[depth][(moves_c[2 * j] - 'A') * DIMEN + moves_r[2 * j]][(moves_c[2 * j + 1] - 'A') * 
            DIMEN + moves_r[2 * j + 1]]) < (history[depth][(moves_c[2 * (j - 1)] - 'A') * 
            DIMEN + moves_r[2 * (j - 1)]][(moves_c[2 * (j - 1) + 1] - 'A') * 
            DIMEN + moves_r[2 * (j - 1) + 1]])) {
                tempc1 = moves_c[2 * j];
                tempc2 = moves_c[2 * j + 1];
                moves_c[2 * j] = moves_c[2 * (j - 1)];
                moves_c[2 * j + 1] = moves_c[2 * (j - 1) + 1];
                moves_c[2 * (j - 1)] = tempc1;
                moves_c[2 * (j - 1) + 1] = tempc2;
    
                tempr1 = moves_r[2 * j];
                tempr2 = moves_r[2 * j + 1];
                moves_r[2 * j] = moves_r[2 * (j - 1)];
                moves_r[2 * j + 1] = moves_r[2 * (j - 1) + 1];
                moves_r[2 * (j - 1)] = tempr1;
                moves_r[2 * (j - 1) + 1] = tempr2;
            } else {
                break;
            }
        }
    }
}

void AI::w_order(char moves_c[], int moves_r[], int depth, int count) {
    if (depth == NA)
        return;
    
    char tempc1, tempc2;
    int tempr1, tempr2, priority = 0;
    for (int i = count - 1; i > 0; i--) {
        if ((moves_c[2 * i] == killer_c[depth * 2]) && (moves_c[2 * i + 1] == killer_c[depth * 2 + 1]) && 
        (moves_r[2 * i] == killer_r[depth * 2]) && (moves_r[2 * i + 1] == killer_r[depth * 2 + 1])) {
            tempc1 = moves_c[2 * i];
            tempc2 = moves_c[2 * i + 1];
            moves_c[2 * i] = moves_c[0];
            moves_c[2 * i + 1] = moves_c[1];
            moves_c[0] = tempc1;
            moves_c[1] = tempc2;

            tempr1 = moves_r[2 * i];
            tempr2 = moves_r[2 * i + 1];
            moves_r[2 * i] = moves_r[0];
            moves_r[2 * i + 1] = moves_r[1];
            moves_r[0] = tempr1;
            moves_r[1] = tempr2;

            priority++;

            break;
        }
    }

    for (int i = priority + 1; i < count; i++) {
        for (int j = i; j > priority; j--) {
            if ((history[depth][(moves_c[2 * j] - 'A') * DIMEN + moves_r[2 * j]][(moves_c[2 * j + 1] - 'A') * 
            DIMEN + moves_r[2 * j + 1]]) < (history[depth][(moves_c[2 * (j - 1)] - 'A') * 
            DIMEN + moves_r[2 * (j - 1)]][(moves_c[2 * (j - 1) + 1] - 'A') * 
            DIMEN + moves_r[2 * (j - 1) + 1]])) {
                tempc1 = moves_c[2 * j];
                tempc2 = moves_c[2 * j + 1];
                moves_c[2 * j] = moves_c[2 * (j - 1)];
                moves_c[2 * j + 1] = moves_c[2 * (j - 1) + 1];
                moves_c[2 * (j - 1)] = tempc1;
                moves_c[2 * (j - 1) + 1] = tempc2;
    
                tempr1 = moves_r[2 * j];
                tempr2 = moves_r[2 * j + 1];
                moves_r[2 * j] = moves_r[2 * (j - 1)];
                moves_r[2 * j + 1] = moves_r[2 * (j - 1) + 1];
                moves_r[2 * (j - 1)] = tempr1;
                moves_r[2 * (j - 1) + 1] = tempr2;
            } else {
                break;
            }
        }
    }
}

void AI::concurrent_table_free(unordered_map<string, Mem_node>* temp) {
    temp->clear();
    delete temp;
}

inline void AI::update_killer(char c1, int r1, char c2, int r2, int depth) {
    killer_c[2 * depth] = c1, killer_c[2 * depth + 1] = c2;
    killer_r[2 * depth] = r1, killer_r[2 * depth + 1] = r2;
}

int AI::deepb(Node *start, int depth) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[180]; //arrays with square locations for possible moves
    char *moves_c = new char[180];
    bool restore = false;

    if ((depth < difficulty - 10) && (time(NULL) - start_time >= time_limit)) {
        delete [] moves_c;
        delete [] moves_r;
        return 0;
    }

    cutoffs[depth + 1] = -10000;
    
    if (diverge) {
        int cut = diverge_b(start, moves_c, moves_r, count, difficulty - depth); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        int cut = fill_b(start, moves_c, moves_r, count, difficulty - depth, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }

    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node

        evaluate_move_b(start, c1, r1, c2, r2, make, depth);

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);

        if (start->options[make].score > cutoffs[depth + 1])
                cutoffs[depth + 1] = start->options[make].score; //set alpha-beta value

        if (start->options[make].score >= cutoffs[depth]) {
            update_killer(c1, r1, c2, r2, difficulty - depth);
            if (difficulty - depth > 1)
                history[difficulty - depth][r1 * DIMEN + (c1 - 'A')][r2 * DIMEN + (c2 - 'A')]++;
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }
        
        make++;
    }
    if ((depth < difficulty - 10) && (time(NULL) - start_time >= time_limit)) {
        return 0;
    } else {
        return maximize(start, moves_c, moves_r, count, difficulty - depth); //return max of children's scores
    }
}

int AI::deepw(Node *start, int depth) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[180]; //arrays with square locations for possible moves
    char *moves_c = new char[180];
    bool restore = false;

    if ((depth < difficulty - 10) && (time(NULL) - start_time >= time_limit)) {
        delete [] moves_c;
        delete [] moves_r;
        return 0;
    }

    cutoffs[depth + 1] = 10000;

    if (diverge) {
        int cut = diverge_w(start, moves_c, moves_r, count, difficulty - depth); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        int cut = fill_w(start, moves_c, moves_r, count, difficulty - depth, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }

    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_w(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node
        
        evaluate_move_w(start, c1, r1, c2, r2, make, depth);

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);

        if (start->options[make].score < cutoffs[depth + 1])
            cutoffs[depth + 1] = start->options[make].score; //set alpha-beta value

        if (start->options[make].score <= cutoffs[depth]) {
            update_killer(c1, r1, c2, r2, difficulty - depth);
            if (difficulty - depth > 1)
                history[difficulty - depth][r1 * DIMEN + (c1 - 'A')][r2 * DIMEN + (c2 - 'A')]++;
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }

        make++;
    }

    if ((depth < difficulty - 10) && (time(NULL) - start_time >= time_limit)) {
        return 0;
    } else {
        return minimize(start, moves_c, moves_r, count, difficulty - depth); //return max of children's scores
    }
}

void AI::iterative_deepening(bool sub) {
    if (difficulty > 3) {
        difficulty--;
        move(sub, false);
        difficulty++;
    }
}

void AI::manage_memory() {
    if ((clear) || (memory->size() * sizeof(Mem_node) > 4000000000)) {
        unordered_map<string, Mem_node> *temp = memory;
        memory = new unordered_map<string, Mem_node>;
        memory->reserve(4000000000 / sizeof(Mem_node));
        if (background.joinable())
            background.join();

        background = thread(&AI::concurrent_table_free, this, temp);
        background.detach();
        
        clear = false;
    }
}

void AI::clear_history() {
    for (int i = 0; i < 100; i++) {
        killer_c[i] = DIMEN + 'A';
        killer_r[i] = DIMEN;
        for (int j = 0; j < 64; j++) {
            for (int k = 0; k < 64; k++) {
                history[i][j][k] = 0;
            }
        }
    }
}

void AI::pre_move(bool sub, bool go) {
    if (go)
        start_time = time(NULL);

    if (sub)
        repeat = true; //fully score each move from root

    if (go)
        manage_memory();

    if (go)
        clear_history();
}

void AI::evaluate_move_b(Node *start, char c1, int r1, char c2, int r2, int make, int depth) {
    if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'B')) && 
    (!start->options[make].position->kinged())) {
        diverge = true;
        d_col = c2, d_row = r2;
        start->options[make].score = deepb(&start->options[make], depth); //recurse for multiple jumps
    } else if (depth < difficulty - 1) {
        start->options[make].score = deepw(&start->options[make], depth + 1); //evaluate responses
    } else {
        start->options[make].score = calc(*start->options[make].position); //get score for this position
    }

    (void) c1;
}

void AI::evaluate_move_w(Node *start, char c1, int r1, char c2, int r2, int make, int depth) {
    if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'W')) && 
    (!start->options[make].position->kinged())) {
        diverge = true;
        d_col = c2, d_row = r2;
        start->options[make].score = deepw(&start->options[make], depth); //recurse for multiple jumps
    } else if (depth < difficulty - 1) {
        start->options[make].score = deepb(&start->options[make], depth + 1); //evaluate responses
    } else {
        start->options[make].score = calc(*start->options[make].position); //get score for this position
    }

    (void) c1;
}

void AI::choose_move(Node *start, int count, int make, bool sub) {
    if (time(NULL) - start_time < time_limit) {
        int choice;
        if (!sub) {
            choice = select(start, count, make, 'B'); //choose best move, change make value
        } else {
            choice = select_second(start, count, make); //choose sub-optimal move, change make value
        }
        
        //set decided move accessible by getter
        row1 = start->options[choice].row1, row2 = start->options[choice].row2;
        col1 = start->options[choice].col1, col2 = start->options[choice].col2;
    }
}
