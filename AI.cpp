/*.cpp file for the AI class, the artificial intelligence playing the game. Uses a minimax tree to make 
move decisions, with tree geometry optimized to balance runtime and performance.

Has 4 difficulty levels, corresponding to search depths of 3, 5, 7, and 12 moves.

Tree uses alpha-beta pruning to increase search efficiency, pruning away vast majority of possible 
game tree leaves by disregarding branches where opponent can force a sub-optimal position.*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cctype>
#include <cmath>
#include <queue>
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

    difficulty = 12;

    srand(time(NULL));
    
    //default, impossible values for AI's chosen move
    row1 = 8;
    col1 = 'I';
    row2 = 8;
    col2 = 'I';

    diverge = false, repeat = false, clear = false;

    //set default values for minimax pruning values
    level1_max = -10000, level2_min = 10000, level3_max = -10000, level4_min = 10000, 
    level5_max = -10000, level6_min = 10000, level7_max = -10000, level8_min = 10000,
    level9_max = -10000, level10_min = 10000, level11_max = -10000;

    //values for AI to keep track of tree shape, relative depth each turn
    complexity = 0, tree = 0;

    memory.reserve(20000000); //reserve starting table space to reduce reallocation time
}

//destructor
//parameters: NA
//returns: NA
AI::~AI() {
    delete state;
    memory.clear();
}

//intro, prints an intro message
//parameters: NA
//returns: void
void AI::intro() {
    cout << "Hello, I am " << name << ", your AI opponent\n";
}

//set_difficulty, sets number of game tree levels AI looks down each turn, 1 for 3 levels, 2 for 5 levels,
//3 for 7 levels, and 4 for 12 levels
//parameters: an int for the difficulty level
//returns: void
void AI::set_difficulty(int level) {
    if (level == 1) {
        difficulty = 3;
    } else if (level == 2) {
        difficulty = 5;
    } else if (level == 3) {
        difficulty = 7;
    } else {
        difficulty = 10;
        move(false, false);
        difficulty = 12;
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
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[104]; //arrays with square locations for possible moves
    char *moves_c = new char[104];
    bool restore = false;

    if (difficulty > 3) {
        difficulty--;
        move(false, false);
        difficulty++;
    }

    if (clear)
        memory.clear(), clear = false;
    
    Node *start = new Node; //make root Node, with given position as board value
    start->position = state;

    //measure, limit tree complexity
    tree = 0, level1_max = -10000;

    if (sub)
        repeat = true; //fully score each move from root

    fill_b(start, moves_c, moves_r, count, INT_MAX, restore); //fill move arrays with possible moves
    
    start->options = new Node[count]; //create subtrees for each possible move
    
    for (make = 0; make < count; make++) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, true, taken); //fill child node

        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'B')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep0(&start->options[make]); //evaluate multiple jump options
        } else {
            start->options[make].score = deep1(&start->options[make]); //evaluate possible responses
        }

        if (start->options[make].score > level1_max)
            level1_max = start->options[make].score; //set alpha-beta value
    }

    if (!sub) {
        select(start, count, make, 'B'); //choose best move, change make value
    } else {
        select_second(start, count, make); //choose sub-optimal move, change make value
    }

    //set decided move accessible by getter
    row1 = start->options[make].row1, row2 = start->options[make].row2;
    col1 = start->options[make].col1, col2 = start->options[make].col2;

    if (go)
        state->make_move(col1, row1, col2, row2);

    repeat = false;
    delete [] moves_r;
    delete [] moves_c;
    delete [] start->options;

    //cerr << tree << endl;
}

//multi, similar to move, but used when a piece has taken and my do so again. Evaluates best 
//possible lines of jumps for the piece, makes decision at each jump, possibly repeatedly
//parameters: NA
//returns: a bool for whether or not futher jump can be made after the selected move
bool AI::multi(bool go) {
    char c1 = col2, c2, taken = 'X'; //position variables
    int r1 = row2, r2;
    int count = 0, make; //number of possible moves, number iterated through so far
    int *moves_r = new int[8]; //arrays with square locations for possible moves
    char *moves_c = new char[8];

    if (difficulty > 3) {
        difficulty--;
        multi(false);
        difficulty++;
    }
    
    Node *start = new Node; //make root Node, with given position as board value
    start->position = state;

    //measure, limit tree complexity
    tree = 0, level1_max = -10000;

    fill_multi(moves_c, moves_r, count, c1, r1, 'B'); //find possible moves from single square

    start->options = new Node[count]; //create subtrees for each possible move

    for (make = 0; make < count; make++) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, true, taken); //fill child node

        if ((start->options[make].position->jump_possible(c2, r2, 'B')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep0(&start->options[make]); //evaluate multiple jump options
        } else {
            start->options[make].score = deep1(&start->options[make]); //evaluate possible responses
        }
    }

    select(start, count, make, 'B'); //choose best move, change make value

    row1 = start->options[make].row1, row2 = start->options[make].row2;
    col1 = start->options[make].col1, col2 = start->options[make].col2;

    if (go)
        state->make_move(col1, row1, col2, row2);

    delete [] moves_c;
    delete [] moves_r;
    delete [] start->options;

    return ((state->jump_possible(col2, row2, 'B')) && (!state->kinged()));
}

//deep0, evaluate moves for black at root level, only used for multiple black jumps
//parameters: a Node to make moves from and evaluate
//returns: an int for the max value among the Node's children
int AI::deep0(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;

    if (diverge) {
        int cut = diverge_b(start, moves_c, moves_r, count, difficulty - 0); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level1_max = -10000;
        int cut = fill_b(start, moves_c, moves_r, count, difficulty - 0, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node
        
        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'B')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep0(&start->options[make]); //recurse for multiple jumps
        } else {
            start->options[make].score = deep1(&start->options[make]); //evaluate possible responses
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);

        if (start->options[make].score > level1_max)
            level1_max = start->options[make].score; //set alpha-beta value

        make++;
    }

    return maximize(start, moves_c, moves_r, make, difficulty - 0); //return max of children's scores
}

//deep1, evaluate moves for white one level down
//parameters: a Node to make moves from and evaluate
//returns: an int for the min value among the Node's children
int AI::deep1(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;

    if (diverge) {
        int cut = diverge_w(start, moves_c, moves_r, count, difficulty - 1); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level2_min = 10000;
        int cut = fill_w(start, moves_c, moves_r, count, difficulty - 1, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_w(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node
        
        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'W')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep1(&start->options[make]); //recurse for multiple jumps
        } else {
            start->options[make].score = deep2(&start->options[make]); //evaluate possible responses
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);
        
        if (start->options[make].score < level2_min)
            level2_min = start->options[make].score; //set alpha-beta value
        
        if ((!repeat) && (start->options[make].score < level1_max)) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }

        make++;
    }

    return minimize(start, moves_c, moves_r, make, difficulty - 1); //return min of children's scores
}

//deep2, evaluate moves for black two levels down, only look this far down for difficulty 3
//parameters: a Node to make moves from and evaluate
//returns: an int for the max value among the Node's children
int AI::deep2(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;

    if (diverge) {
        int cut = diverge_b(start, moves_c, moves_r, count, difficulty - 2); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level3_max = -10000;
        int cut = fill_b(start, moves_c, moves_r, count, difficulty - 2, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node
        
        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'B')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep2(&start->options[make]); //recurse for multiple jumps
        } else if (difficulty > 3) {
            start->options[make].score = deep3(&start->options[make]); //evaluate possible responses
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score for this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);

        if (start->options[make].score > level3_max)
            level3_max = start->options[make].score; //set alpha-beta value

        if (start->options[make].score >= level2_min) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }

        make++;
    }

    return maximize(start, moves_c, moves_r, make, difficulty - 2); //return max of children's scores
}

//deep3, evaluate moves for white three levels down
//parameters: a Node to make moves from and evaluate
//returns: an int for the min value among the Node's children
int AI::deep3(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;

    if (diverge) {
        int cut = diverge_w(start, moves_c, moves_r, count, difficulty - 3); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level4_min = 10000;
        int cut = fill_w(start, moves_c, moves_r, count, difficulty - 3, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_w(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node

        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'W')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep3(&start->options[make]); //recurse for multiple jumps
        } else if (difficulty > 4) {
            start->options[make].score = deep4(&start->options[make]); //evaluate possible responses
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score for this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);

        if (start->options[make].score < level4_min)
            level4_min = start->options[make].score; //set alpha-beta value

        if (start->options[make].score <= level3_max) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }

        make++;
    }

    return minimize(start, moves_c, moves_r, make, difficulty - 3); //return min of children's scores
}

//deep4, evaluate moves for black four levels down, only look this far down for difficulty 5
//parameters: a Node to make moves from and evaluate
//returns: an int for the max value among the Node's children
int AI::deep4(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;
    
    if (diverge) {
        int cut = diverge_b(start, moves_c, moves_r, count, difficulty - 4); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level5_max = -10000;
        int cut = fill_b(start, moves_c, moves_r, count, difficulty - 4, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node

        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'B')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep4(&start->options[make]); //recurse for multiple jumps
        } else if (difficulty > 5) {
            start->options[make].score = deep5(&start->options[make]); //evaluate possible responses
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score for this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);

        if (start->options[make].score > level5_max)
            level5_max = start->options[make].score; //set alpha-beta value

        if (start->options[make].score >= level4_min) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }

        make++;
    }

    return maximize(start, moves_c, moves_r, make, difficulty - 4); //return max of children's scores
}

//deep5, evaluate moves for white five levels down
//parameters: a Node to make moves from and evaluate
//returns: an int for the min value among the Node's children
int AI::deep5(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;

    if (diverge) {
        int cut = diverge_w(start, moves_c, moves_r, count, difficulty - 5); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level6_min = 10000;
        int cut = fill_w(start, moves_c, moves_r, count, difficulty - 5, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_w(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node

        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'W')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep5(&start->options[make]); //recurse for multiple jumps
        } else if (difficulty > 6) {
            start->options[make].score = deep6(&start->options[make]); //evaluate possible responses
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score for this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);

        if (start->options[make].score < level6_min)
            level6_min = start->options[make].score; //set alpha-beta value

        if (start->options[make].score <= level5_max) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }
            
        make++;
    }

    return minimize(start, moves_c, moves_r, make, difficulty - 5); //return min of children's scores
}

//deep6, evaluate moves for black six levels down, only look this far down for difficulty 7
//parameters: a Node to make moves from and evaluate
//returns: an int for the max value among the Node's children
int AI::deep6(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;
    
    if (diverge) {
        int cut = diverge_b(start, moves_c, moves_r, count, difficulty - 6); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level7_max = -10000;
        int cut = fill_b(start, moves_c, moves_r, count, difficulty - 6, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node

        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'B')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep6(&start->options[make]); //recurse for multiple jumps
        } else if (difficulty > 7) {
            start->options[make].score = deep7(&start->options[make]); //evaluate possible responses
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score of this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);
        
        if (start->options[make].score > level7_max)
            level7_max = start->options[make].score; //set alpha-beta value

        if (start->options[make].score >= level6_min) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }
        
        make++;
    }

    return maximize(start, moves_c, moves_r, make, difficulty - 6); //return max of children's scores
}

//deep7, evaluate moves for white seven levels down
//parameters: a Node to make moves from and evaluate
//returns: an int for the min value among the Node's children
int AI::deep7(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;

    if (diverge) {
        int cut = diverge_w(start, moves_c, moves_r, count, difficulty - 7); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level8_min = 10000;
        int cut = fill_w(start, moves_c, moves_r, count, difficulty - 7, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_w(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node
        
        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'W')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep7(&start->options[make]); //recurse for multiple jumps
        } else if (difficulty > 8) {
            start->options[make].score = deep8(&start->options[make]); //evaluate possible responses
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score for this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);
        
        if (start->options[make].score < level8_min)
            level8_min = start->options[make].score; //set alpha-beta value

        if (start->options[make].score <= level7_max) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }

        make++;
    }

    return minimize(start, moves_c, moves_r, make, difficulty - 7); //return min of children's scores
}

//deep8, evaluate moves for black eight levels down
//parameters: a Node to make moves from and evaluate
//returns: an int for the max value among the Node's children
int AI::deep8(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;
    
    if (diverge) {
        int cut = diverge_b(start, moves_c, moves_r, count, difficulty - 8); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level9_max = -10000;
        int cut = fill_b(start, moves_c, moves_r, count, difficulty - 8, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node

        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'B')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep8(&start->options[make]); //recurse for multiple jumps
        } else if (difficulty > 9) {
            start->options[make].score = deep9(&start->options[make]); //evaluate possible responses
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score for this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);
        
        if (start->options[make].score > level9_max)
                level9_max = start->options[make].score; //set alpha-beta value
                
        if (start->options[make].score >= level8_min) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }
        
        make++;
    }

    return maximize(start, moves_c, moves_r, make, difficulty - 8); //return max of children's scores
}

//deep9, evaluate moves for white nine levels down
//parameters: a Node to make moves from and evaluate
//returns: an int for the min value among the Node's children
int AI::deep9(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;

    if (diverge) {
        int cut = diverge_w(start, moves_c, moves_r, count, difficulty - 9); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level10_min = 10000;
        int cut = fill_w(start, moves_c, moves_r, count, difficulty - 9, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_w(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node
        
        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'W')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep9(&start->options[make]); //recurse for multiple jumps
        } else if (difficulty > 10) {
            start->options[make].score = deep10(&start->options[make]); //evaluate possible responses
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score for this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);
        
        if (start->options[make].score < level10_min)
            level10_min = start->options[make].score; //set alpha-beta value

        if (start->options[make].score <= level9_max) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }

        make++;
    }

    return minimize(start, moves_c, moves_r, make, difficulty - 9); //return min of children's scores
}

//deep10, evaluate moves for black ten levels down
//parameters: a Node to make moves from and evaluate
//returns: an int for the max value among the Node's children
int AI::deep10(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;
    
    if (diverge) {
        int cut = diverge_b(start, moves_c, moves_r, count, difficulty - 10); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level11_max = -10000;
        int cut = fill_b(start, moves_c, moves_r, count, difficulty - 10, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node

        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'B')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep10(&start->options[make]); //recurse for multiple jumps
        } else if (difficulty > 11) {
            start->options[make].score = deep11(&start->options[make]); //evaluate possible responses
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score for this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);

        if (start->options[make].score > level11_max)
                level11_max = start->options[make].score; //set alpha-beta value

        if (start->options[make].score >= level10_min) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }
        
        make++;
    }

    return maximize(start, moves_c, moves_r, make, difficulty - 10); //return max of children's scores
}

//deep11, evaluate moves for white nine levels down
//parameters: a Node to make moves from and evaluate
//returns: an int for the min value among the Node's children
int AI::deep11(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;

    if (diverge) {
        int cut = diverge_w(start, moves_c, moves_r, count, difficulty - 11); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        level12_min = 10000;
        int cut = fill_w(start, moves_c, moves_r, count, difficulty - 11, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_w(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node
        
        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'W')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep11(&start->options[make]); //recurse for multiple jumps
        } else if (difficulty > 12) {
            start->options[make].score = deep12(&start->options[make]); //evaluate possible responses
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score for this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);

        if (start->options[make].score < level12_min)
            level12_min = start->options[make].score; //set alpha-beta value

        if (start->options[make].score <= level11_max) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }

        make++;
    }

    return minimize(start, moves_c, moves_r, make, difficulty - 11); //return min of children's scores
}

//deep12, evaluate moves for black twelve levels down
//parameters: a Node to make moves from and evaluate
//returns: an int for the max value among the Node's children
int AI::deep12(Node *start) {
    int count = 0, make = 0; //number of possible moves, number iterated through so far
    char c1, c2, taken = 'X'; //position variables
    int r1, r2;
    int *moves_r = new int[90]; //arrays with square locations for possible moves
    char *moves_c = new char[90];
    bool restore = false;
    
    if (diverge) {
        int cut = diverge_b(start, moves_c, moves_r, count, difficulty - 12); //moves for a multi jump turn
        if (cut != NA)
            return cut;
    } else {
        int cut = fill_b(start, moves_c, moves_r, count, difficulty - 12, restore); //fill in possible moves
        if (cut != NA)
            return cut;
    }
    
    start->options = new Node[count]; //create subtrees for each possible move

    while (make < count) {
        moving_b(start, moves_c, moves_r, make, c1, c2, r1, r2, false, taken); //fill child Node

        if ((abs(r2 - r1) == 2) && (start->options[make].position->jump_possible(c2, r2, 'B')) && 
        (!start->options[make].position->kinged())) {
            diverge = true;
            d_col = c2, d_row = r2;
            start->options[make].score = deep12(&start->options[make]); //recurse for multiple jumps
        } else {
            start->options[make].score = calc(*start->options[make].position); //get score for this position
        }

        start->position->reverse_move(c1, r1, c2, r2, taken, restore);

        if (start->options[make].score >= level12_min) {
            for (int i = make + 1; i < count; i++) {
                start->options[i].score = start->options[make].score;
                start->options[i].position = NULL; //prune unevaluated branches after a disqualifying child
            }
            make = count - 1;
        }
        
        make++;
    }

    return maximize(start, moves_c, moves_r, make, difficulty - 12); //return max of children's scores
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
    /*string *key = make_key(ref, turn);
    if (memory.count(*key))
        return memory.find(*key)->second.score;*/

    //piece scores
    int black = 0, white = 0, R, C;
    int num_b = ref.get_num_black(), num_w = ref.get_num_white();

    //(incomplete) check for win or loss based on number of pieces
    if (num_w == 0) {
        return 9900;
    } else if (num_b == 0) {
        return -9900;
    }

    //interate through the white pieces, giving each a value based on its type and locations
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
                white += 8;
            }

            white -= 4 * (abs(4 - R) + abs(4 - C));
        } else {
            white += 207 - R;

            if ((C == 0) || (C == DIMEN - 1)) {
                white += 10;
            }
        }
    }

    //interate through black pieces
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
                black += 8;
            }

            black -= 4 * (abs(4 - R) + abs(4 - C));
        } else {
            black += 200 + R;

            if ((C == 0) || (C == DIMEN - 1)) {
                black += 10;
            }
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
        black += 10;
    } else if ((ref.look(0, 7) == BLACK_KING) && (ref.look(2, 5) == WHITE_KING)) {
        white += 10;
    }

    if ((ref.look(7, 0) == WHITE_KING) && (ref.look(5, 2) == BLACK_KING)) {
        black += 10;
    } else if ((ref.look(7, 0) == BLACK_KING) && (ref.look(5, 2) == WHITE_KING)) {
        white += 10;
    }

    //value of having more pieces increases exponentially with declining number of pieces
    if (num_w > num_b) {
        int temp = 17 - num_b;
        temp = (num_w - num_b) * temp * temp;

        if (temp < 5000) {
            white += temp;
        } else {
            white += 5000; //avoid value here higher than winning value
        }
    } else if (num_b > num_w) {
        int temp = 17 - num_w;
        temp = (num_b - num_w) * temp * temp;

        if (temp < 5000) {
            black += temp;
        } else {
            black += 5000; //avoid value here higher than winning value
        }
    }

    tree++;

    /*Mem_node temp;
    temp.depth = 0;
    temp.score = black - white;
    memory.emplace(*key, temp);*/
    
    
    return (black - white); //score is difference in value of black and white positions
}

//proximity, finds how close a piece is to the nearest opposing piece
//parameters: an int for the piece's row, an int for its column, a string for the color of the piece, a ref 
//to a Board object to scan, ints for the numbers of black and white pieces
//returns: an int for the distance to the nearest opposing piece
int AI::proximity(int row, int column, char turn, Board &ref, int num_b, int num_w) {
    int dist = 7, x = 0, y = 0;

    if (turn == 'W') {
        for (int i = 0; i < num_b; i++) {
            x = abs(ref.get_place_col_b(i) - column);
            y = abs(ref.get_place_row_b(i) - row);

            if ((x < dist) && (y < dist)) {
                if (x > y) {
                    dist = x;
                } else {
                    dist = y;
                }

                if (dist == 1)
                    return 1; //save time if adjacent
            }  
        }
    } else {
        for (int i = 0; i < num_w; i++) {
            x = abs(ref.get_place_col_w(i) - column);
            y = abs(ref.get_place_row_w(i) - row);

            if ((x < dist) && (y < dist)) {
                if (x > y) {
                    dist = x;
                } else {
                    dist = y;
                }

                if (dist == 1)
                    return 1; //save time if adjacent
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
    if (memory.count(*look)) {
        unordered_map<string, Mem_node>::iterator place = memory.find(*look);
        count = place->second.children;
        if (place->second.depth >= depth) {
            return place->second.score;
        } else {
            for (int i = 0; i < 2 * place->second.children; i++) {
                moves_c[i] = place->second.child_c[i];
                moves_r[i] = place->second.child_r[i];
            }
        }
    } else {
        char c1;
        int r1;
        bool forced = start->position->forced_take('B'); //check for forced jump
        if ((!forced) && (!start->position->any_move('W')))
            return -10000 + depth; //check for loss, value modified by depth
        for (int i = 0; i < start->position->get_num_black(); i++) {
            //iterate through each black piece
            c1 = start->position->get_place_col_b(i) + 'A';
            r1 = start->position->get_place_row_b(i);
            if (!forced) {
                if ((r1 < DIMEN - 1) && (c1 - 'A' < DIMEN - 1) && 
                (start->position->look(r1 + 1, c1 + 1 - 'A') == BLACK_SQUARE)) {
                    moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 1, 
                    moves_r[(count * 2) + 1] = r1 + 1;
                    count++;
                }
                if ((r1 < DIMEN - 1) && (c1 - 'A' > 0) &&
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
                    if ((r1 > 0) && (c1 - 'A' < DIMEN - 1) &&
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
    if (memory.count(*look)) {
        unordered_map<string, Mem_node>::iterator place = memory.find(*look);
        count = place->second.children;
        if (place->second.depth >= depth) {
            return place->second.score;
        } else {
            for (int i = 0; i < 2 * place->second.children; i++) {
                moves_c[i] = place->second.child_c[i];
                moves_r[i] = place->second.child_r[i];
            }
        }
    } else {
        char c1;
        int r1;
        bool forced = start->position->forced_take('W'); //check for forced jump
        if ((!forced) && (!start->position->any_move('B')))
            return 10000 - depth; //check for win, value modified by depth
        for (int i = 0; i < start->position->get_num_white(); i++) {
            //iterate through each white piece
            c1 = start->position->get_place_col_w(i) + 'A';
            r1 = start->position->get_place_row_w(i);
            if (!forced) {
                if ((r1 > 0) && (c1 - 'A' < DIMEN - 1) &&
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
                    if ((r1 < DIMEN - 1) && (c1 - 'A' < DIMEN - 1) && 
                    (start->position->look(r1 + 1, c1 + 1 - 'A') == BLACK_SQUARE)) {
                        moves_c[count * 2] = c1, moves_r[count * 2] = r1, moves_c[(count * 2) + 1] = c1 + 1, 
                        moves_r[(count * 2) + 1] = r1 + 1;
                        count++;
                    }
                    if ((r1 < DIMEN - 1) && (c1 - 'A' > 0) &&
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
    if (memory.count(*look)) {
        unordered_map<string, Mem_node>::iterator place = memory.find(*look);
        count = place->second.children;
        if (place->second.depth >= depth) {
            return place->second.score;
        } else {
            for (int i = 0; i < 2 * place->second.children; i++) {
                moves_c[i] = place->second.child_c[i];
                moves_r[i] = place->second.child_r[i];
            }
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
    if (memory.count(*look)) {
        unordered_map<string, Mem_node>::iterator place = memory.find(*look);
        count = place->second.children;
        if (place->second.depth >= depth) {
            return place->second.score;
        } else {
            for (int i = 0; i < 2 * place->second.children; i++) {
                moves_c[i] = place->second.child_c[i];
                moves_r[i] = place->second.child_r[i];
            }
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

    if (abs(r2 - r1) == 2) {
        if ((r2 - r1 == -2) && (c2 - c1 == -2)) {
            taken = start->position->look(r1 - 1, c1 - 1 - 'A');
        } else if ((r2 - r1 == -2) && (c2 - c1 == 2)) {
            taken = start->position->look(r1 - 1, c1 + 1 - 'A');
        } else if ((r2 - r1 == 2) && (c2 - c1 == -2)) {
            taken = start->position->look(r1 + 1, c1 - 1 - 'A');
        } else if ((r2 - r1 == 2) && (c2 - c1 == 2)) {
            taken = start->position->look(r1 + 1, c1 + 1 - 'A');
        }
    }

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

    if (abs(r2 - r1) == 2) {
        if ((r2 - r1 == -2) && (c2 - c1 == -2)) {
            taken = start->position->look(r1 - 1, c1 - 1 - 'A');
        } else if ((r2 - r1 == -2) && (c2 - c1 == 2)) {
            taken = start->position->look(r1 - 1, c1 + 1 - 'A');
        } else if ((r2 - r1 == 2) && (c2 - c1 == -2)) {
            taken = start->position->look(r1 + 1, c1 - 1 - 'A');
        } else if ((r2 - r1 == 2) && (c2 - c1 == 2)) {
            taken = start->position->look(r1 + 1, c1 + 1 - 'A');
        }
    }

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
int AI::maximize(Node *start, char moves_c[], int moves_r[], int &make, int depth) {
    Mem_node temp;
    int hold;

    int *place = new int[make];
    for (int k = 0; k < make; k++)
        place[k] = k;

    for (int i = 1; i < make; i++) {
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

    temp.children = make;
    for (int i = 0; i < make; i++) {
        temp.child_c[2 * i] = moves_c[2 * place[i]];
        temp.child_c[(2 * i) + 1] = moves_c[2* place[i] + 1];
        temp.child_r[2 * i] = moves_r[2 * place[i]];
        temp.child_r[(2 * i) + 1] = moves_r[2 * place[i] + 1];
    }

    temp.depth = depth;
    temp.score = start->options[place[0]].score;
    string *access = make_key(*start->position, 'B');
    
    if (memory.count(*access)) {
            if (memory.find(*access)->second.depth < depth)
                memory.erase(*access);
    }
    
    memory.emplace(*access, temp);

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
int AI::minimize(Node *start, char moves_c[], int moves_r[], int &make, int depth) {
    Mem_node temp;
    int hold;

    int *place = new int[make];
    for (int k = 0; k < make; k++)
        place[k] = k;

    for (int i = 1; i < make; i++) {
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

    temp.children = make;
    for (int i = 0; i < make; i++) {
        temp.child_c[2 * i] = moves_c[2 * place[i]];
        temp.child_c[(2 * i) + 1] = moves_c[2* place[i] + 1];
        temp.child_r[2 * i] = moves_r[2 * place[i]];
        temp.child_r[(2 * i) + 1] = moves_r[2 * place[i] + 1];
    }

    temp.depth = depth;
    temp.score = start->options[place[0]].score;
    string *access = make_key(*start->position, 'W');

    if (memory.count(*access)) {
            if (memory.find(*access)->second.depth < depth)
                memory.erase(*access);
    }
    
    memory.emplace(*access, temp);

    delete [] start->options; //delete array of pointers itself
    delete [] moves_r; //delete stored move coordinates
    delete [] moves_c;
    return temp.score;
}

//find_complexity, produces a rough estimate of the game-tree branching factor at the current 
//position, by multiplying together the number of moves available to each player, so the AI can limit 
//evaluation depth when complexity is high
//parameters: NA
//returns: an int corresponding to the game-tree complexity
int AI::find_complexity() {
    int count1 = 0, count2 = 0; //counter for moves available to each player

    //dummy Node, Board and move arrays to use the fill functions
    Node *placeholder = new Node;
    placeholder->position = new Board(*state);
    int *moves_r = new int[90];
    char *moves_c = new char[90];
    bool restore;

    fill_b(placeholder, moves_c, moves_r, count1, 0, restore); //find and count possible moves
    fill_w(placeholder, moves_c, moves_r, count2, 0, restore);

    delete [] moves_c;
    delete [] moves_r;

    return (count1 * count2); //complexity is approximated by multiple of two counts
}

//closeness, finds the average distance between the kings of the selected color and the nearest 
//opposing piece
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
                total += proximity(i, j, 'W', ref, num_b, num_w);
                kings++;
            } else if ((color == 'B') && (ref.look(i, j) == BLACK_KING)) {
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
void AI::select(Node *start, int &count, int &make, char color) {
    int rando[150];
    int num_same = 0; //number of possible moves with the same score

    int max = NA;
    for (int i = 0; i < count; i++) {
        if ((closeness(*start->options[i].position, color) < closeness(*start->position, color)) && 
        (start->position->get_num_black() >= start->position->get_num_white()))
            start->options[i].score += 5; //weight aggressive moves
        if (start->options[i].score > max) {
            max = start->options[i].score;
            make = i;
            num_same = 0;
            rando[num_same] = i; //new highest's number added to first position in randomization array
        } else if (start->options[i].score > max) {
            num_same++;
            rando[num_same] = i; //add later tied moves' child array number to randomization array
        }

        delete start->options[i].position; //delete each Board of each child
    }

    //choose randomly among best moves, with extra weighting for ones that progress the game by 
    //reducing closeness value
    make = rando[rand() % (num_same + 1)];
}

//select_second, helper function to move, finds a best move if the usual highest-scoring ones are 
//reduced in score by 30, getting a slightly sub-optimal choice to break repetitions, sets make to it
//parameters: a Node pointer, an int ref to count, an int ref to make, a string for the color of the AI
//returns: void
void AI::select_second(Node *start, int &count, int &make) {
    int rando[150];
    int max = NA;
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

    max = NA;
    for (int i = 0; i < count; i++) {
        if (start->options[i].score > max) {
            max = start->options[i].score;
            make = i;
            num_same = 0;
            rando[num_same] = i; //new highest's number added to first position in randomization array
        } else if (start->options[i].score == max) {
            num_same++;
            rando[num_same] = i; //add later tied moves' child array number to randomization array
        }

        delete start->options[i].position;
    }

    //choose randomly among best sub-optimal moves, with extra weighting for ones that progress the
    //game by reducing closeness value
    make = rando[rand() % (num_same + 1)];
}

string* AI::make_key(Board &ref, char turn) {
    string *hold = ref.get_key();
    hold->back() = turn;
    
    return hold;
}














