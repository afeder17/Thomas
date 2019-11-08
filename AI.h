/*AI.h, the header file for the AI class, the artificial intelligence playing the game. Utilizes a minimax 
evaluation tree to make decisions, which are then accessed by the main through getters for the 
move notation.

One of two AI objects, this one seen as "Thomas."*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <thread>
#include "board.h"

using namespace std;

#ifndef AI_H
#define AI_H

class AI {
    private:
        /***AI state***/
        
        //name of AI
        string name;

        //variable for difficulty level
        int difficulty;

        //variables for time limit
        long long start_time;
        long long time_limit;
    
        /***Structs***/

        //Game tree node
        struct Node {
            Board *position;
            Node *options;
            int score;
            char row1, row2;
            char col1, col2;
        };

        //stored position node
        struct Mem_node {
            char child_c[180];
            char child_r[180];
            int score;
            char children;
            char depth;
        };

        /***History information***/
        
        //transposition table, stores data from previous move evaluations
        unordered_map<string, Mem_node> *memory = new unordered_map<string, Mem_node>;
        
        //hold last move to cause a beta-cutoff at each level
        char killer_c[100] = {DIMEN + 'A'};
        int killer_r[100] = {DIMEN};

        int history[100][64][64];

        int cutoffs[100];

        /***~Threading~***/

        //thread for background work
        thread background;

        /***Game and move state***/
        
        //board held by AI to make decisions
        Board *state;

        //position variables for chosen move
        char col1, col2;
        int row1, row2;

        //restrictions on possible moves for evaluating multiple jumps
        bool diverge, repeat;

        //hold piece position for divergent multiple jumps
        char d_col;
        int d_row;

        //whether or not to clear transposition table at start of next move
        bool clear;

        /***Constants***/
        
        //board size constant
        const static int DIMEN = 8;

        //other constants
        const static char WHITE_PIECE = 'r', WHITE_KING = 'R';
        const static char BLACK_PIECE = 'b', BLACK_KING = 'B';
        const static char BLANK = ' ', BLACK_SQUARE = '-';
        const static int NA = -32000;
        const static int DIMEN_LESS1 = 7;
        
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
        int maximize(Node *start, char moves_c[], int moves_r[], int count, int depth);

        //return the min value at the node, clean up arrays
        int minimize(Node *start, char moves_c[], int moves_r[], int count, int depth);

        //chooses a move after score evaluations have been made
        int select(Node *start, int count, int make, char color);

        //same as above, but choose a next best move to avoid repetition
        int select_second(Node *start, int count, int make);

        //make a string key for a given Board
        string* make_key(Board &ref, char turn);

        //order moves to be checked for optimal pruning, for black
        void b_order(char moves_c[], int moves_r[], int depth, int count);

        //order moves to be checked for optimal pruning, for white
        void w_order(char moves_c[], int moves_r[], int depth, int count);

        //update killer move arrays with moves that cause beta cutoffs
        void update_killer(char c1, int r1, char c2, int r2, int depth);

        //function to free hash table from pointer, used by concurrent thread
        void concurrent_table_free(unordered_map<string, Mem_node>* temp);

        int deepw(Node *start, int depth);

        int deepb(Node *start, int depth);

        void iterative_deepening(bool sub);

        void manage_memory();

        void clear_history();

        void pre_move(bool sub, bool go);

        void evaluate_move_b(Node *start, char c1, int r1, char c2, int r2, int make, int depth);

        void evaluate_move_w(Node *start, char c1, int r1, char c2, int r2, int make, int depth);

        void choose_move(Node *start, int count, int make, bool sub);

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

};
#endif
