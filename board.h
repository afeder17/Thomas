/*.h file for the Board class, used to store the game state and evaluate possible game trees. Holds 
the rules of American checkers, determining move legality, whether a game has been won, and 
hosting an interface for the user to make multiple jumps.*/

#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

#ifndef BOARD_H
#define BOARD_H

class Board {
    private:
        //board constants
        const static char WHITE_PIECE = 'r', WHITE_KING = 'R';
        const static char BLACK_PIECE = 'b', BLACK_KING = 'B';
        const static char BLANK = ' ';
        const static char BLACK_SQUARE = '-';
    
        const static int DIMEN = 8, START_NUM = 12;

        //struct holding the array position and type of a piece
        struct Place {
            int row;
            int column;
            bool king;
        };

        //game data
        char game_board[DIMEN][DIMEN];

        //hashing key
        string *key;
        
        int *last_move; //records last move as array coordinates

        bool just_kinged; //records whether a piece was kinged at the last move

        //arrays of the locations of the pieces of each color
        Place *black_places;
        Place *white_places;

        //the number of pieces of each color
        int num_black, num_white;
        
        //private functions
        //sets up initial board
        void fill_board();

        //helper function to fill_board, sets up checkerboard pattern
        void checker_board();

        //checks that a proposed move for a white king is valid
        bool white_king_valid(int col1, int row1, int col2, int row2);
        
        //checks that a proposed move for a white piece is valid
        bool white_piece_valid(int col1, int row1, int col2, int row2);

        //checks that a proposed move for a black king is valid
        bool black_king_valid(int col1, int row1, int col2, int row2);

        //checks that a proposed move for a black piece is valid
        bool black_piece_valid(int col1, int row1, int col2, int row2);

        //checks if it's possible for a piece to move one square
        bool move_possible(char col, int row, char turn);

        //helper function to make_move, converts regular pieces to kings if called for
        void king_maker();

        //puts the locations of all the pieces in their respective location arrays
        void locate();

        //efficiently updates location array data, but only following a call of locate
        void update(int col1, int row1, int col2, int row2, char taken);

        //undo the changes in the location arrays made at the last move
        void reverse_update(int col1, int row1, int col2, int row2, char taken);

    public:
        //public functions
        //constructor
        Board();

        //copy constructor
        Board(const Board &other);

        //destructor
        ~Board();

        //modify board for reverse orientation
        void reverse();

        //play a random first three moves to create a tournament opening
        void rando(int seed);

        //play a random first three moves to create a tournament opening, in reverse turn order
        void rando_r(int seed);

        //print board with white on bottom of display
        void print();

        //print board with black on bottom of display
        void print_reverse();

        //checks that a move is valid, returns a bool if so
        bool check_validity(char column1, int row1, char column2, int row2, char turn);

        //checks that a move is valid without scanning for takes, returns a bool if so
        bool simple_check(char column1, int row1, char column2, int row2, char turn);

        //makes a move, modifying the board according to the entered notation
        void make_move(char column1, int row1, char column2, int row2);

        //makes a move reversal
        void reverse_move(char column1, int row1, char column2, int row2, char taken, bool restore);

        //checks if the game is over, returns a bool if so
        bool check_win(char turn);

        //gets the column a piece was last moved to
        int last_col();

        //gets the row a piece was last moved to
        int last_row();

        //recursive function for multiple jumps
        void multi_hop(char turn);

        //recursive function for multiple jumps on a reversed board
        void multi_hop_r(char turn);

        //checks if any of the pieces of a certain color have any possible moves
        bool anything_possible(char turn);

        //checks if any opposing piece can be moved a single square
        bool any_move(char turn);

        //checks if any opposing piece can jump
        bool any_jump(char turn);

        //checks if it's possible for a piece to take
        bool jump_possible(char col, int row, char turn);

        //helper function to move_possible, checks for forced takes
        bool forced_take(char turn);

        //gets value at particular index array, for access by AI
        char square(int row, int column);

        //gets value at a particular index array, but without protections for out of bounds
        char look(int row, int column);

        //gets status of just_kinged bool
        bool kinged();

        //get an int corresponding to a row in the places array of a particular color
        int get_place_row(int num, char color);

        //get an int corresponding to a row in the white places array
        int get_place_row_w(int num);

        //get an int corresponding to a row in the black places array
        int get_place_row_b(int num);

        //get an int corresponding to a column in the places array of a particular color
        int get_place_col(int num, char color);

        //get an int corresponding to a column in the white places array
        int get_place_col_w(int num);

        //get an int corresponding to a column in the black places array
        int get_place_col_b(int num);

        //get a bool corresponding to whether a piece is a king in the places array of a particular color
        int get_place_king(int num, char color);

        //get a bool corresponding to whether a piece is a king in the white places array
        int get_place_king_w(int num);

        //get a bool corresponding to whether a piece is a king in the black places array
        int get_place_king_b(int num);

        //get number of black pieces
        int get_num_black();

        //get number of white pieces
        int get_num_white();

        //finds number of possible jumps for a piece, given its location
        int num_jumps(char col, int row);

        //determines whether the Board is identical to another one
        bool same(Board ref2);

        //get a pointer to the Board's hashing key
        string* get_key();

};

#endif
