/*AI.h, the header file for the AI class, the artificial intelligence playing the game. Utilizes a minimax 
evaluation tree to make decisions, which are then accessed by the main through getters for the 
move notation.

One of two AI objects, this one seen as "Thomas."*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include "board.h"

using namespace std;

#ifndef AI_H
#define AI_H

class AI {
    private:
        //board held by AI to make decisions
        Board *state;

        //name of AI
        string name;

        //variable for difficulty level
        int difficulty;

        //position variables for chosen move
        char col1, col2;
        int row1, row2;

        //restrictions on possible moves for evaluating multiple jumps
        bool diverge, repeat;

        //hold piece position for divergent multiple jumps
        char d_col;
        int d_row;

        //variables for alpha-beta pruning
        int level1_max, level2_min, level3_max, level4_min, level5_max, level6_min, level7_max,
        level8_min, level9_max, level10_min, level11_max, level12_min;

        //rough measure of tree complexity at each move
        int complexity;

        //whether or not to clear transposition table at start of next move
        bool clear;

        //board size constant
        const static int DIMEN = 8;

        //constants
        const static char WHITE_PIECE = 'r', WHITE_KING = 'R';
        const static char BLACK_PIECE = 'b', BLACK_KING = 'B';
        const static char BLANK = ' ', BLACK_SQUARE = '-';
        const static int NA = -INT_MAX;

        struct Node {
            Board *position;
            int score;
            Node *options;
            char row1, row2;
            int col1, col2;
        };

        struct Mem_node {
            int children;
            char child_c[90];
            int child_r[90];
            int depth;
            int score;
        };

        //transposition table, stores data from previous move evaluations
        unordered_map<string, Mem_node> memory;
        
        //evaluates how favorable a position is to the AI
        int calc(Board &ref);

        //calculates the distance of a piece from the nearest opposing piece
        int proximity(int row, int column, char turn, Board &ref, int num_b, int num_w);

        //finds the average distance between the kings of a given color and the nearest opposing piece
        double closeness(Board &ref, char color);

        //fill the move option arrays for the max node deep functions
        int fill_b(Node *start, char moves_c[], int moves_r[], int &count, int depth, bool &restore);

        //fill the move option arrays for the min node deep functions
        int fill_w(Node *start, char moves_c[], int moves_r[], int &count, int depth, bool &restore);
        
        //fill the move option arrays from only one starting square
        void fill_multi(char moves_c[], int moves_r[], int &count, char c1, int r1, char color);

        //fill the move options arrays for the max node deep functions for diverging jumps
        int diverge_b(Node *start, char moves_c[], int moves_r[], int &count, int depth);

        //fill the move options arrays for the min node deep functions for diverging jumps
        int diverge_w(Node *start, char moves_c[], int moves_r[], int &count, int depth);

        //make the moves on the board from the move arrays for the max nodes
        void moving_b(Node *start, char moves_c[], int moves_r[], int &make, char &c1, char &c2, 
        int &r1, int &r2, bool copy, char &taken);

        //make the moves on the board from the move arrays for the min nodes
        void moving_w(Node *start, char moves_c[], int moves_r[], int &make, char &c1, char &c2, 
        int &r1, int &r2, bool copy, char &taken);

        //return the max value at the node, clean up arrays
        int maximize(Node *start, char moves_c[], int moves_r[], int &make, int depth);

        //return the min value at the node, clean up arrays
        int minimize(Node *start, char moves_c[], int moves_r[], int &make, int depth);

        //evaluates future possible moves, at 1st level down, called by move() for divergent multi jumps
        int deep0(Node *start);
        
        //evaluates future possible moves, 2nd level down
        int deep1(Node* start);

        //evaluates future possible moves, 3rd level down
        int deep2(Node *start);

        //evaluates future possible moves, 4th level down
        int deep3(Node *start);

        //evaluates future possible moves, 5th level down
        int deep4(Node *start);

        //evaluates future possible moves, 6th level down
        int deep5(Node *start);

        //evaluates future possible moves, 7th level down
        int deep6(Node *start);

        //evaluates future possible moves, 8th level down
        int deep7(Node *start);

        //evaluates future possible moves, 9th level down
        int deep8(Node *start);

        //evaluates future possible moves, 10th level down
        int deep9(Node *start);

        //evaluates future possible moves, 11th level down
        int deep10(Node *start);

        //evaluates future possible moves, 12th level down
        int deep11(Node *start);

        //evaluates future possible moves, 13th level down
        int deep12(Node *start);

        //chooses a move after score evaluations have been made
        void select(Node *start, int &count, int &make, char color);

        //same as above, but choose a next best move to avoid repetition
        void select_second(Node *start, int &count, int &make);

        //make a string key for a given Board
        string* make_key(Board &ref, char turn);

        //counter for the numbers of possibilities evaluated
        long tree;

    public:
        //constructor
        AI();

        //destructor
        ~AI();

        //intro message
        void intro();

        //set difficulty
        void set_difficulty(int level);

        //updates the board held by the AI to the game board
        void update_AI(const Board ref);

        //AI makes its decision about where to move
        void move(bool sub, bool go);

        //makes additional jumps on a turn
        bool multi(bool go);

        //functions to get notation of decided move
        //get row of first square
        char get_row1();

        //get column of first square
        int get_col1();

        //get row of second square
        char get_row2();

        //get column of second square
        int get_col2();

        //roughly find complexity of position by multiplying number of possible moves for each side
        int find_complexity();

};
#endif
