/*.cpp file for the Board class, which holds game state and enforces rules. Position data is stored as 
a 2d array, with a small array for the previous move notation and variable for whether that move 
created a king.

Object holds functions for determining whether a proposed move is legal, and seperate functions for
making moves. Function to implement moves takes in character variables corresponding to column, 
but integer variables directly corresponding to subarray number in 2d array. This is a little confusing.
Functions to make moves do not themselves check legality, and should only be used after checking 
whther they are allowed.

For use by AI, contains arrays of pieces with their types and positions, accessible through getters, 
and getters to look at particular squares on the board. Piece data is updated after each move.

User plays multiple jumps directly through Board, and Board can make opening random moves on 
itself, to increase game variability.

Board prints position with colored ASCI characters, and can print a reversed board by changing 
colors, switching corresponding characters, and printing rows in horizontally reversed order.*/

#include <iostream>
#include <cstdlib>
#include <string>
#include <cctype>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include "board.h"

using namespace std;

//constructor
//parameters: NA
//returns: NA
Board::Board() {
    fill_board(); //set board
    
    for (int i = 0; i < 3; i++) {
        last_move[i] = DIMEN; //fill last move array with default, impossible values
    }

    just_kinged = false;

    for (int i = 0; i < 34; i++)
        key.push_back(' ');

    int place = 0;
    for (int i = 0; i < DIMEN; i++) {
        for (int j = 0; j < DIMEN; j++) {
            if (game_board[i][j] != BLANK) {
                key[place] = game_board[i][j];
                place++;
            }
        }
    }

    num_black = START_NUM;
    num_white = START_NUM;

    locate(); //find the information for the Place arrays
}

//copy constructor
//parameters: a reference to another Board object
//returns: NA
Board::Board(const Board &other) {
    //copy board array
    for (int i = 0; i < DIMEN; i++) {
        for (int j = 0; j < DIMEN; j++) {
            game_board[i][j] = other.game_board[i][j];
        }
    }

    for (int i = 0; i < 3; i++) {
        last_move[i] = other.last_move[i];
    }

    just_kinged = other.just_kinged;

    for (int i = 0; i < 34; i++)
        key.push_back(' ');

    int place = 0;
    for (int i = 0; i < DIMEN; i++) {
        for (int j = 0; j < DIMEN; j++) {
            if (game_board[i][j] != BLANK) {
                key[place] = game_board[i][j];
                place++;
            }
        }
    }

    num_black = other.num_black;
    num_white = other.num_white;

    locate();
}

//reverse, sets up the board in a reverse orientation, but only in the column orientation
//parameters: NA
//returns: NA
void Board::reverse() {
    //reset pattern of black and white squares
    int type = false; //bool to alternate between square colors
    for (int i = 0; i < DIMEN; i++) {
        for (int j = 0; j < DIMEN; j++) {
            if (type) {
                game_board[i][j] = BLANK;
                type = false;
            } else {
                game_board[i][j] = BLACK_SQUARE;
                type = true;
            }
        }
        if (type) {
            type = false;
        } else {
            type = true;
        }
    }

    //set black pieces
    int hold = 0; //change initial alternating value to reverse board
    for (int i = 0; i < ((DIMEN / 2) - 1); i++) {
        for (int j = hold; j < DIMEN; j += 2) {
            game_board[i][j] = BLACK_PIECE;
        }
        if (hold == 1) {
            hold = 0;
        } else {
            hold = 1;
        }
    }

    //set white pieces
    int start = DIMEN - (DIMEN / 2) + 1;
    for (int i = start; i < DIMEN; i++) {
        for (int j = hold; j < DIMEN; j += 2) {
            game_board[i][j] = WHITE_PIECE;
        }
        if (hold == 1) {
            hold = 0;
        } else {
            hold = 1;
        }
    }

    locate();  //find the information for the Place arrays
}

//rando, plays three random moves to start the game, producing a wider number of possible games 
//in AI vs AI mode
//parameters: an integer to add to the unix time seed, to increase randomness when multiple games
//occur within a second
//returns: NA
void Board::rando(int seed) {
    char c1, c2;
    int r1, r2;
    srand(time(NULL) + seed);

    //first move
    do {
        c1 = (rand() % DIMEN) + 'A';
        r1 = rand() % DIMEN;
        c2 = (rand() % DIMEN) + 'A';
        r2 = rand() % DIMEN;
    } while (!check_validity(c1, r1, c2, r2, 'W'));
    make_move(c1, r1, c2, r2);

    //second move
    do {
        c1 = (rand() % DIMEN) + 'A';
        r1 = rand() % DIMEN;
        c2 = (rand() % DIMEN) + 'A';
        r2 = rand() % DIMEN;
    } while (!check_validity(c1, r1, c2, r2, 'B'));
    make_move(c1, r1, c2, r2);

    //third move
    do {
        c1 = (rand() % DIMEN) + 'A';
        r1 = rand() % DIMEN;
        c2 = (rand() % DIMEN) + 'A';
        r2 = rand() % DIMEN;
    } while (!check_validity(c1, r1, c2, r2, 'W'));
    make_move(c1, r1, c2, r2);
}

//rando_r, the same process as rando, but in a reversed move order
//parameters: an integer to add to the unix time seed, to increase randomness when multiple games
//occur within a second
//returns: NA
void Board::rando_r(int seed) {
    char c1, c2;
    int r1, r2;
    srand(time(NULL) + seed);

    //first move
    do {
        c1 = (rand() % DIMEN) + 'A';
        r1 = rand() % DIMEN;
        c2 = (rand() % DIMEN) + 'A';
        r2 = rand() % DIMEN;
    } while (!check_validity(c1, r1, c2, r2, 'B'));
    make_move(c1, r1, c2, r2);

    //second move
    do {
        c1 = (rand() % DIMEN) + 'A';
        r1 = rand() % DIMEN;
        c2 = (rand() % DIMEN) + 'A';
        r2 = rand() % DIMEN;
    } while (!check_validity(c1, r1, c2, r2, 'W'));
    make_move(c1, r1, c2, r2);

    //third move
    do {
        c1 = (rand() % DIMEN) + 'A';
        r1 = rand() % DIMEN;
        c2 = (rand() % DIMEN) + 'A';
        r2 = rand() % DIMEN;
    } while (!check_validity(c1, r1, c2, r2, 'B'));
    make_move(c1, r1, c2, r2);
}

//fill_board, sets up board
//parameters: NA
//returns: void
void Board::fill_board() {
    checker_board(); //square colors
    //black pieces
    bool hold = true;
    for (int i = 0; i < ((DIMEN / 2) - 1); i++) {
        for (int j = hold; j < DIMEN; j += 2) {
            game_board[i][j] = BLACK_PIECE;
        }
        hold = (!hold);
    }

    //white pieces
    int start = DIMEN - (DIMEN / 2) + 1;
    for (int i = start; i < DIMEN; i++) {
        for (int j = hold; j < DIMEN; j += 2) {
            game_board[i][j] = WHITE_PIECE;
        }
        hold = (!hold);
    }
}

//checker_board, helper function to fill_board, sets up checkerboard pattern of board
//parameters: NA
//returns: void
void Board::checker_board() {
    bool type = true;
    for (int i = 0; i < DIMEN; i++) {
        for (int j = 0; j < DIMEN; j++) {
            if (type) {
                game_board[i][j] = BLANK;
            } else {
                game_board[i][j] = BLACK_SQUARE;
            }
            type = (!type); //alternate each square
        }
        type = (!type); //alternate each row
    }
}

//print, displays board (note: output color changes may impact portability)
//parameters: NA
//returns: void
void Board::print() {
    int numbers = DIMEN;
    
    cout << "\033[0m";
    cout << "   A  B  C  D  E  F  G  H\n"; //column markers
    cout << " |––––––––––––––––––––––––|\n";
    for (int i = 0; i < DIMEN; i++) {
        cout << numbers << "|";
        for (int j = 0; j < DIMEN; j++) {
            cout << "[";
            if ((i == last_move[1]) && (j == last_move[0])) {
                cout << "\033[1m\033[34m"; //highlight spot a piece last moved from
            } else if ((game_board[i][j] == WHITE_PIECE) || (game_board[i][j] == WHITE_KING)) {
                cout << "\033[1m\033[31m"; //piece color
            } else {
                cout << "\033[1m\033[30m"; //piece color
            }
            cout << game_board[i][j] << "\033[0m" << ']';
        }
        cout << '|' << numbers-- << "\n |––––––––––––––––––––––––|\n";
    }
    cout << "   A  B  C  D  E  F  G  H\n";
}

//print_reverse, displays board with reversed piece characters and colors. This, paired with a board 
//having been flipped in the column orientation and a change of turn order, allows the player to play 
//as black against Thomas.
//parameters: NA
//returns: void
void Board::print_reverse() {
    int numbers = 1;
    
    cout << "\033[0m";
    cout << "   H  G  F  E  D  C  B  A\n"; //column markers
    cout << " |––––––––––––––––––––––––|\n";
    for (int i = 0; i < DIMEN; i++) {
        cout << numbers << "|";
        for (int j = 0; j < DIMEN; j++) {
            cout << "[";
            if ((i == last_move[1]) && (j == last_move[0])) {
                cout << "\033[1m\033[34m"; //highlight spot a piece last moved from
            } else if ((game_board[i][j] == BLACK_PIECE) || (game_board[i][j] == BLACK_KING)) {
                cout << "\033[1m\033[31m"; //reversed piece color
            } else {
                cout << "\033[1m\033[30m"; //reversed piece color
            }

            //print each piece as its color reverse
            if (game_board[i][j] == BLACK_PIECE) {
                cout << WHITE_PIECE;
            } else if (game_board[i][j] == BLACK_KING) {
                cout << WHITE_KING;
            } else if (game_board[i][j] == WHITE_PIECE) {
                cout << BLACK_PIECE;
            } else if (game_board[i][j] == WHITE_KING) {
                cout << BLACK_KING;
            } else {
                cout << game_board[i][j];
            }

            cout << "\033[0m" << ']';
        }
        cout << '|' << numbers++ << "\n |––––––––––––––––––––––––|\n";
    }
    cout << "   H  G  F  E  D  C  B  A\n";
}

//check_validity, checks that a proposed move is valid
//parameters: 2 chars for columns, and 2 ints for rows of notations, string for the turn
//returns: a bool indicating if the move is valid
bool Board::check_validity(char column1, int row1, char column2, int row2, char turn) {
    char hold1 = toupper(column1);
    char hold2 = toupper(column2);

    //Check that the positions are actually on the board
    if (((hold1 - 'A') > 7) || ((hold1 - 'A') < 0) || (row1 > 7) || (row1 < 0) || ((hold2 - 'A') > 7) || 
    ((hold2 - 'A') < 0) || (row2 > 7) || (row2 < 0))
        return false;

    if ((abs(row2 - row1) != 2) && (forced_take(turn)))
        return false;

    int col1 = hold1 - 'A'; //convert user-inputed chars to ints
    int col2 = hold2 - 'A';

    //Make sure that player is trying to move their own piece
    if ((turn == 'W') && (((game_board[row1][col1] != WHITE_PIECE) && 
    (game_board[row1][col1] != WHITE_KING)) || (game_board[row2][col2] != BLACK_SQUARE))) {
        return false;
    } else if ((turn == 'B') && (((game_board[row1][col1] != BLACK_PIECE) && 
    (game_board[row1][col1] != BLACK_KING)) || (game_board[row2][col2] != BLACK_SQUARE))) {
        return false;
    }

    //check that the particular piece can be moved that way
    if ((turn == 'W') && (game_board[row1][col1] == WHITE_PIECE)) {
        return white_piece_valid(col1, row1, col2, row2); //white regular piece
    } else if ((turn == 'W') && (game_board[row1][col1] == WHITE_KING)) {
        return white_king_valid(col1, row1, col2, row2); //white king
    } else if ((turn == 'B') && (game_board[row1][col1] == BLACK_PIECE)) {
        return black_piece_valid(col1, row1, col2, row2); //black piece
    } else if ((turn == 'B') && (game_board[row1][col1] == BLACK_KING)) {
        return black_king_valid(col1, row1, col2, row2); //black king
    }

    return false; //return false if move does not meet conditions of legality for its piece
}

//make_move, changes values on the board array to make a move
//parameters: 2 chars for the notation columns, and 2 ints for the rows
//returns: void
void Board::make_move(char column1, int row1, char column2, int row2) {
    int col1 = toupper(column1) - 'A';
    int col2 = toupper(column2) - 'A';
    char taken = 'X';
    
    game_board[row2][col2] = game_board[row1][col1];
    game_board[row1][col1] = BLACK_SQUARE;

    last_move[0] = col1, last_move[1] = row1, last_move[2] = col2, last_move[3] = row2;

    //remove taken piece
    if (abs(row2 - row1) == 2) {
        if ((row2 - row1 == -2) && (col2 - col1 == -2)) {
            taken = game_board[row1 - 1][col1 - 1];
            game_board[row1 - 1][col1 - 1] = BLACK_SQUARE;
        } else if ((row2 - row1 == -2) && (col2 - col1 == 2)) {
            taken = game_board[row1 - 1][col1 + 1];
            game_board[row1 - 1][col1 + 1] = BLACK_SQUARE;
        } else if ((row2 - row1 == 2) && (col2 - col1 == -2)) {
            taken = game_board[row1 + 1][col1 - 1];
            game_board[row1 + 1][col1 - 1] = BLACK_SQUARE;
        } else if ((row2 - row1 == 2) && (col2 - col1 == 2)) {
            taken = game_board[row1 + 1][col1 + 1];
            game_board[row1 + 1][col1 + 1] = BLACK_SQUARE;
        }
    }
    
    king_maker();
    update(col1, row1, col2, row2, taken);

    key[((row1 * 8) + col1) / 2] = game_board[row1][col1];
    key[((row2 * 8) + col2) / 2] = game_board[row2][col2];

    if (abs(row2 - row1) == 2)
        key[((((row1 + row2) / 2) * 8) / 2) + (((col1 + col2) / 2)) / 2] = BLACK_SQUARE;
}

void Board::reverse_move(char column1, int row1, char column2, int row2, char taken, 
bool restore) {
    char hold1 = toupper(column1);
    char hold2 = toupper(column2);
    int col1 = hold1 - 'A';
    int col2 = hold2 - 'A';

    game_board[row1][col1] = game_board[row2][col2];
    game_board[row2][col2] = BLACK_SQUARE;

    if (just_kinged == true) {
        if (game_board[row1][col1] == WHITE_KING) {
            game_board[row1][col1] = WHITE_PIECE;
        } else if (game_board[row1][col1] == BLACK_KING) {
            game_board[row1][col1] = BLACK_PIECE;
        }
    }

    if (abs(row2 - row1) == 2) {
        if ((row2 - row1 == -2) && (col2 - col1 == -2)) {
            game_board[row1 - 1][col1 - 1] = taken;
        } else if ((row2 - row1 == -2) && (col2 - col1 == 2)) {
            game_board[row1 - 1][col1 + 1] = taken;
        } else if ((row2 - row1 == 2) && (col2 - col1 == -2)) {
            game_board[row1 + 1][col1 - 1] = taken;
        } else if ((row2 - row1 == 2) && (col2 - col1 == 2)) {
            game_board[row1 + 1][col1 + 1] = taken;
        }
    }

    if (restore) {
        just_kinged = true;
    } else {
        just_kinged = false;
    }
    
    reverse_update(col1, row1, col2, row2, taken);

    key[((row1 * 8) + col1) / 2] = game_board[row1][col1];
    key[((row2 * 8) + col2) / 2] = game_board[row2][col2];

    if (abs(row2 - row1) == 2)
        key[((((row1 + row2) / 2) * 8) / 2) + (((col1 + col2) / 2)) / 2] = taken;
}

//king_maker, helper function to make_move, scans back rows of board, converting men to kings
//parameters: NA
//returns: void
void Board::king_maker() {
    if (game_board[0][last_move[2]] == WHITE_PIECE) {
        game_board[0][last_move[2]] = WHITE_KING;
        just_kinged = true;
        return;
    }
    
    if (game_board[DIMEN_LESS1][last_move[2]] == BLACK_PIECE) {
        game_board[DIMEN_LESS1][last_move[2]] = BLACK_KING;
        just_kinged = true;
        return;
    }
    
    just_kinged = false;
}

//check_win, checks if the game has been won
//parameters: a string for the turn
//returns: a bool for whether the game is over
bool Board::check_win(char turn) {
    return (!anything_possible(turn));
}

//last_col, gets column a piece was last moved to
//parameters: NA
//returns: int for the array postion of the column
int Board::last_col() {
    return last_move[2];
}

//last_row, gets row a piece was last moved to
//parameters: NA
//returns: int for the array position of the row
int Board::last_row() {
    return last_move[3];
}

//multi_hop, recursive function for multiple takes in a row
//parameters: a string for the turn
//returns: void
void Board::multi_hop(char turn) {
    if ((!jump_possible((last_move[2] + 'A'), last_move[3], turn)) || (just_kinged))
        return; //don't ask about another hop if not possible

    char col1, col2;
    int row1, row2;
    string input, output = "";
    bool go = false;
    do {
        cout << "Next jump?\n";
        while (((getline(cin, input) && (input.length() != 4))) || (!isalpha(input[0])) || 
        (!isdigit(input[1])) || (!isalpha(input[2])) || (!isdigit(input[3]))) {
            int len = input.size();
            for (int i = 0; i < len; i++) {
                if (isalnum(input[i]))
                    output = output + input[i];
            }
            input = output;
            
            if (input != "") {
                if (((input.length() != 4)) || (!isalpha(input[0])) || (!isdigit(input[1])) || 
                (!isalpha(input[2])) || (!isdigit(input[3]))) {
                    cout << "Invalid notation, use letter-number pairs (i.e. A3 B4)\n"; //validate input
                } else {
                    break;
                }
            }
        }

        //fill in notation variables
        col1 = input[0], row1 = input[1] - 48, col2 = input[2], row2 = input[3] - 48;
        
        row1 = abs(DIMEN - row1); //convert row input to array positions
        row2 = abs(DIMEN - row2);
        int col1int = toupper(col1) - 'A'; //convert column to an int
        
        go = check_validity(col1, row1, col2, row2, turn);
        if ((col1int != last_move[2]) || (row1 != last_move[3])) {
            go = false; //make sure we're moving the right piece
        }
    } while (!go);
    make_move(col1, row1, col2, row2);
    print();
    
    if (abs(row2 - row1) == 2)
        multi_hop(turn); //recursive call if another hop possible
}

//multi_hop_r, recursive function for multiple takes in a row, but with reversed board notation
//parameters: a string for the turn
//returns: void
void Board::multi_hop_r(char turn) {
    if ((!jump_possible((last_move[2] + 'A'), last_move[3], turn)) || (just_kinged))
        return; //don't ask about another hop if not possible

    char col1, col2;
    int row1, row2;
    string input, output = "";
    bool go = false;
    do {
        cout << "Next jump?\n";
        while (((getline(cin, input) && (input.length() != 4))) || (!isalpha(input[0])) || 
        (!isdigit(input[1])) || (!isalpha(input[2])) || (!isdigit(input[3]))) {
            int len = input.size();
            for (int i = 0; i < len; i++) {
                if (isalnum(input[i]))
                    output = output + input[i];
            }
            input = output;
            
            if (input != "") {
                if (((input.length() != 4)) || (!isalpha(input[0])) || (!isdigit(input[1])) || 
                (!isalpha(input[2])) || (!isdigit(input[3]))) {
                    cout << "Invalid notation, use letter-number pairs (i.e. A3 B4)\n"; //validate input
                } else {
                    break;
                }
            }
        }

        //fill in notation variables
        col1 = input[0], row1 = input[1] - 48, col2 = input[2], row2 = input[3] - 48;

        //convert notation for flipped board
        col1 = 'H' - col1 + 'A';
        col2 = 'H' - col2 + 'A';
        row1 = 9 - row1;
        row2 = 9 - row2;
                
        row1 = abs(DIMEN - row1); //convert row input to array positions
        row2 = abs(DIMEN - row2);
        int col1int = toupper(col1) - 'A'; //convert column to an int
        
        go = check_validity(col1, row1, col2, row2, turn);
        if ((col1int != last_move[2]) || (row1 != last_move[3])) {
            go = false; //make sure we're moving the right piece
        }
    } while (!go);
    make_move(col1, row1, col2, row2);
    print();
    
    if (abs(row2 - row1) == 2)
        multi_hop_r(turn); //recursive call if another hop possible
}

//jump_possible, checks if it's possible for a particular piece to take in any direction
//parameters: a char for the piece's column, an int for the row, a string for the turn
//returns: a bool if a take is possible
bool Board::jump_possible(char col, int row, char turn) {
    if (turn == 'W') {
        if (col < 'G') {
            if ((row > 1) && ((game_board[row - 1][col + 1 - 'A'] == BLACK_PIECE) || 
            (game_board[row - 1][col + 1 - 'A'] == BLACK_KING)) && 
            (game_board[row - 2][col + 2 - 'A'] == BLACK_SQUARE))
                return true; //check up to the right
            if ((row < DIMEN - 2) && ((game_board[row + 1][col + 1 - 'A'] == BLACK_PIECE) || 
            (game_board[row + 1][col + 1 - 'A'] == BLACK_KING)) &&
            (game_board[row + 2][col + 2 - 'A'] == BLACK_SQUARE) &&
            (game_board[row][col - 'A'] == WHITE_KING))
                return true; //check down to the right
        }
        if (col > 'B') {
            if ((row > 1) && ((game_board[row - 1][col - 1 - 'A'] == BLACK_PIECE) || 
            (game_board[row - 1][col - 1 - 'A'] == BLACK_KING)) &&
            (game_board[row - 2][col - 2 - 'A'] == BLACK_SQUARE))
                return true; //check up to the left
            if ((row < DIMEN - 2) && ((game_board[row + 1][col - 1 - 'A'] == BLACK_PIECE) || 
            (game_board[row + 1][col - 1 - 'A'] == BLACK_KING)) &&
            (game_board[row + 2][col - 2 - 'A'] == BLACK_SQUARE) &&
            (game_board[row][col - 'A'] == WHITE_KING))
                return true; //check down to the left
        }
    } else {
        if (col < 'G') {
            if ((row > 1) && ((game_board[row - 1][col + 1 - 'A'] == WHITE_PIECE) || 
            (game_board[row - 1][col + 1 - 'A'] == WHITE_KING)) &&
            (game_board[row - 2][col + 2 - 'A'] == BLACK_SQUARE) &&
            (game_board[row][col - 'A'] == BLACK_KING))
                return true; //check up to the right
            if ((row < DIMEN - 2) && ((game_board[row + 1][col + 1 - 'A'] == WHITE_PIECE) || 
            (game_board[row + 1][col + 1 - 'A'] == WHITE_KING)) &&
            (game_board[row + 2][col + 2 - 'A'] == BLACK_SQUARE))
                return true; //check down to the right
        }
        if (col > 'B') {
            if ((row > 1) && ((game_board[row - 1][col - 1 - 'A'] == WHITE_PIECE) || 
            (game_board[row - 1][col - 1 - 'A'] == WHITE_KING)) &&
            (game_board[row - 2][col - 2 - 'A'] == BLACK_SQUARE) &&
            (game_board[row][col - 'A'] == BLACK_KING))
                return true; //check up to the left
            if ((row < DIMEN - 2) && ((game_board[row + 1][col - 1 - 'A'] == WHITE_PIECE) || 
            (game_board[row + 1][col - 1 - 'A'] == WHITE_KING)) &&
            (game_board[row + 2][col - 2 - 'A'] == BLACK_SQUARE))
                return true; //check down to the left
        }
    }

    return false;
}

//move_possible, checks if it's possible for a particular piece to move one space in any direction
//parameters: a char for the piece's column, an int for the row, a string for the turn
//returns: a bool if a move is possible
bool Board::move_possible(char col, int row, char turn) {
    if (turn == 'W') {
        if (col < 'H') {
            if ((row > 0) && (game_board[row - 1][col + 1 - 'A'] == BLACK_SQUARE))
                return true; //check up to the right
            if ((row < DIMEN_LESS1) && (game_board[row + 1][col + 1 - 'A'] == BLACK_SQUARE) &&
            (game_board[row][col - 'A'] == WHITE_KING))
                return true; //check down to the right
        }
        if (col > 'A') {
            if ((row > 0) && (game_board[row - 1][col - 1 - 'A'] == BLACK_SQUARE))
                return true; //check up to the left
            if ((row < DIMEN_LESS1) && (game_board[row + 1][col - 1 - 'A'] == BLACK_SQUARE) &&
            (game_board[row][col - 'A'] == WHITE_KING))
                return true; //check down to the left
        }
    } else {
        if (col < 'H') {
            if ((row > 0) && (game_board[row - 1][col + 1 - 'A'] == BLACK_SQUARE) &&
            (game_board[row][col - 'A'] == BLACK_KING))
                return true; //check up to the right
            if ((row < DIMEN_LESS1) && (game_board[row + 1][col + 1 - 'A'] == BLACK_SQUARE))
                return true; //check down to the right
        }
        if (col > 'A') {
            if ((row > 0) && (game_board[row - 1][col - 1 - 'A'] == BLACK_SQUARE) &&
            (game_board[row][col - 'A'] == BLACK_KING))
                return true; //check up to the left
            if ((row < DIMEN_LESS1) && (game_board[row + 1][col - 1 - 'A'] == BLACK_SQUARE))
                return true; //check down to the left
        }
    }

    return false;
}

bool Board::move_possible_w(char col, int row) {
    if (col > 'A') {
        if ((row > 0) && (game_board[row - 1][col - 1 - 'A'] == BLACK_SQUARE))
            return true; //check up to the left
        if ((row < DIMEN_LESS1) && (game_board[row + 1][col - 1 - 'A'] == BLACK_SQUARE) &&
        (game_board[row][col - 'A'] == WHITE_KING))
            return true; //check down to the left
    }

    if (col < 'H') {
        if ((row > 0) && (game_board[row - 1][col + 1 - 'A'] == BLACK_SQUARE))
            return true; //check up to the right
        if ((row < DIMEN_LESS1) && (game_board[row + 1][col + 1 - 'A'] == BLACK_SQUARE) &&
        (game_board[row][col - 'A'] == WHITE_KING))
            return true; //check down to the right
    }

    return false;
}

bool Board::move_possible_b(char col, int row) {
    if (col > 'A') {
        if ((row > 0) && (game_board[row - 1][col - 1 - 'A'] == BLACK_SQUARE) &&
        (game_board[row][col - 'A'] == BLACK_KING))
            return true; //check up to the left
        if ((row < DIMEN_LESS1) && (game_board[row + 1][col - 1 - 'A'] == BLACK_SQUARE))
            return true; //check down to the left
    }

    if (col < 'H') {
        if ((row > 0) && (game_board[row - 1][col + 1 - 'A'] == BLACK_SQUARE) &&
        (game_board[row][col - 'A'] == BLACK_KING))
            return true; //check up to the right
        if ((row < DIMEN_LESS1) && (game_board[row + 1][col + 1 - 'A'] == BLACK_SQUARE))
            return true; //check down to the right
    }

    return false;
}

//anything_possible, checks if a player has any possible moves
//parameters: a string for the other player's color
//returns: a bool for whether there's any possible moves
bool Board::anything_possible(char turn) {
    return ((any_move(turn)) || (any_jump(turn)));
}

//any_move, helper function to anything_possible, checks whether there's any possible single space 
//moves for a particular player
//parameters: a string for the other player's color
//returns: a bool for whether any single space move is possible
bool Board::any_move(char turn) {
    if (turn == 'W') {
        for (int i = num_black - 1; i > -1; i--) {
            if (move_possible_b((get_place_col_b(i) + 'A'), get_place_row_b(i)))
                return true; //check each piece
        }
    } else {
        for (int i = num_white - 1; i > -1; i--) {
            if (move_possible_w((get_place_col_w(i) + 'A'), get_place_row_w(i)))
                return true; //check each piece
        }
    }

    return false; //false if nothing is found
}

//any_jump, checks whether there's any possible taking moves for a particular player
//parameters: a string for the other player's color
//returns: a bool for whether any taking move is possible
bool Board::any_jump(char turn) {
    if (turn == 'W') {
        for (int i = 0; i < get_num_black(); i++) {
            if (jump_possible((get_place_col_b(i) + 'A'), get_place_row_b(i), 'B'))
                return true; //check each piece
        }
    } else {
        for (int i = 0; i < get_num_white(); i++) {
            if (jump_possible((get_place_col_w(i) + 'A'), get_place_row_w(i), 'W'))
                return true; //check each piece
        }
    }

    return false; //false if nothing is found
}

//forced_take, checks whether a particular player is forced to take a piece
//parameters: a string for the color of the player
//returns: a bool for whether there is a forced take
bool Board::forced_take(char turn) {
    //switch around colors when calling any_jump
    if (turn == 'W') {
        return any_jump('B');
    } else {
        return any_jump('W');
    }
}

//square, a getter for the content of a particular square on the board array, but with a safety check
//so one does not go outside the bounds of the array, returning BLANK if so
//parameters: an int for the row of the square, and another for the column
//returns: content of the indicated board position
char Board::square(int row, int column) {
    if ((row < 0) || (row > DIMEN_LESS1) || (column < 0) || (column > (DIMEN_LESS1))) {
        return BLANK;
    } else {
        return game_board[row][column];
    }
}

//locate, scans through the board array, placing the position and type of each piece in an array of 
//Place instances for each respective player
//parameters: NA
//returns: void
void Board::locate() {
    num_white = 0, num_black = 0;

    for (int j = DIMEN_LESS1; j > -1; j--) {
        for (int i = 0; i < DIMEN; i++) {
            //check whether it's a black square first to increase efficiency
            if (game_board[i][j] != BLACK_SQUARE) {
                if (game_board[i][j] == WHITE_PIECE) {
                    white_places[num_white].row = i;
                    white_places[num_white].column = j;
                    white_places[num_white].king = false;
                    num_white++;
                } else if (game_board[i][j] == WHITE_KING) {
                    white_places[num_white].row = i;
                    white_places[num_white].column = j;
                    white_places[num_white].king = true;
                    num_white++;
                }
            }
        }
    }

    for (int j = 0; j < DIMEN; j++) {
        for (int i = 0; i < DIMEN; i++) {
            //check whether it's a black square first to increase efficiency
            if (game_board[i][j] != BLACK_SQUARE) {
                if (game_board[i][j] == BLACK_PIECE) {
                    black_places[num_black].row = i;
                    black_places[num_black].column = j;
                    black_places[num_black].king = false;
                    num_black++;
                }else if (game_board[i][j] == BLACK_KING) {
                    black_places[num_black].row = i;
                    black_places[num_black].column = j;
                    black_places[num_black].king = true;
                    num_black++;
                }
            }
        }
    }
}


void Board::update(int col1, int row1, int col2, int row2, char taken) {
    if (abs(row2 - row1) == 2) {
        int col = (col1 + col2)/2;
        int row = (row1 + row2)/2;
    
        if ((taken == WHITE_PIECE) || (taken == WHITE_KING)) {
            for (int i = 0; i < num_white; i++) {
                if ((white_places[i].column == col) && (white_places[i].row == row)) {
                    white_places[i] = white_places[num_white - 1];
                    num_white--;
                    break;
                }
            }
        } else {
            for (int i = 0; i < num_black; i++) {
                if ((black_places[i].column == col) && (black_places[i].row == row)) {
                    black_places[i] = black_places[num_black - 1];
                    num_black--;
                    break;
                }
            }
        }
    }

    if ((game_board[row2][col2] == WHITE_PIECE) || (game_board[row2][col2] == WHITE_KING)) {
        for (int i = 0; i < num_white; i++) {
            if ((white_places[i].row == row1) && (white_places[i].column == col1)) {
                white_places[i].column = col2;
                white_places[i].row = row2;
                if (game_board[row2][col2] == WHITE_KING)
                    white_places[i].king = true;
                break;
            }
        }
    } else {
        for (int i = 0; i < num_black; i++) {
            if ((black_places[i].row == row1) && (black_places[i].column == col1)) {
                black_places[i].column = col2;
                black_places[i].row = row2;
                if (game_board[row2][col2] == BLACK_KING)
                    black_places[i].king = true;
                break;
            }
        }
    }
}

void Board::reverse_update(int col1, int row1, int col2, int row2, char taken) {
    if (abs(row2 - row1) == 2) {
        int col = (col1 + col2)/2;
        int row = (row1 + row2)/2;
        
        if ((taken == WHITE_PIECE) || (taken == WHITE_KING)) {
            white_places[num_white].column = col;
            white_places[num_white].row = row;
            if (taken == WHITE_PIECE) {
                white_places[num_white].king = false;
            } else {
                white_places[num_white].king = true;
            }

            num_white++;

            Place temp;

            temp = white_places[num_white - 1];
            white_places[num_white - 1] = white_places[0];
            white_places[0] = temp;
        } else {
            black_places[num_black].column = col;
            black_places[num_black].row = row;
            if (taken == BLACK_PIECE) {
                black_places[num_black].king = false;
            } else {
                black_places[num_black].king = true;
            }

            num_black++;

            Place temp;

            temp = black_places[num_black - 1];
            black_places[num_black - 1] = black_places[0];
            black_places[0] = temp;
        }
    }

    if ((game_board[row1][col1] == WHITE_PIECE) || (game_board[row1][col1] == WHITE_KING)) {
        for (int i = 0; i < num_white; i++) {
            if ((white_places[i].column == col2) && (white_places[i].row == row2)) {
                white_places[i].column = col1;
                white_places[i].row = row1;
                if (game_board[row1][col1] == WHITE_PIECE) {
                    white_places[i].king = false;
                } else {
                    white_places[i].king = true;
                }
                break;
            }
        }
    } else {
        for (int i = 0; i < num_black; i++) {
            if ((black_places[i].column == col2) && (black_places[i].row == row2)) {
                black_places[i].column = col1;
                black_places[i].row = row1;
                if (game_board[row1][col1] == BLACK_PIECE) {
                    black_places[i].king = false;
                } else {
                    black_places[i].king = true;
                }
                break;
            }
        }
    }
}


//get_place_row, gets the row coordinate of a particular piece in one of the Place arrays
//parameters: an int for the place in the array, and a string for the array to be accessed
//returns: an int for the coordinate
int Board::get_place_row(int num, char color) {
    if (color == 'W') {
        return white_places[num].row;
    } else {
        return black_places[num].row;
    }
}

//get_place_col, gets the col coordinate of a particular piece in one of the Place arrays
//parameters: an int for the place in the array, and a string for the array to be accessed
//returns: an int for the coordinate
int Board::get_place_col(int num, char color) {
    if (color == 'W') {
        return white_places[num].column;
    } else {
        return black_places[num].column;
    }
}

//get_place_king, gets whether a piece in one of the Place arrays is a king
//parameters: an int for the place in the array, and a string for the array to be accessed
//returns: a bool for whether it's a king
int Board::get_place_king(int num, char color) {
    if (color == 'W') {
        return white_places[num].king;
    } else {
        return black_places[num].king;
    }
}

//num_jumps, checks how many possible taking moves there are from a particular square
//parameters: a char for the column, an int for the row
int Board::num_jumps(char col, int row) {
    int num = 0;
    char color;
    
    //check the color of the piece
    if ((game_board[row][col - 'A'] == WHITE_KING) || 
    (game_board[row][col - 'A'] == WHITE_PIECE)) {
        color = 'W';
    } else {
        color = 'B';
    }
    
    if (simple_check(col, row, (col + 2), (row + 2), color))
        num++;
    if (simple_check(col, row, (col + 2), (row - 2), color))
        num++;
    if (simple_check(col, row, (col - 2), (row + 2), color))
        num++;
    if (simple_check(col, row, (col - 2), (row - 2), color))
        num++;

    return num;
}

//simple_check, checks that a proposed move is valid without scanning for takes or capitalizing 
//column inputs (be very careful with this one)
//parameters: 2 chars for columns, and 2 ints for rows of notations, string for the turn
//returns: a bool indicating if the move is valid
bool Board::simple_check(char column1, int row1, char column2, int row2, char turn) {
    //Check that the positions are actually on the board
    if ((column2 - 'A' > DIMEN_LESS1) || (column2 - 'A' < 0) || (row2 > DIMEN_LESS1) || (row2 < 0))
        return false;

    //Make sure that player is moving to an emptry square
    if (game_board[row2][column2 - 'A'] != BLACK_SQUARE)
        return false;

    //check that the particular piece can be moved that way
    if (turn == 'W') {
        if (game_board[row1][column1 - 'A'] == WHITE_PIECE) {
            return white_piece_valid(column1 - 'A', row1, column2 - 'A', row2); //white regular piece
        } else {
            return white_king_valid(column1 - 'A', row1, column2 - 'A', row2); //white king
        }
    } else {
        if (game_board[row1][column1 - 'A'] == BLACK_PIECE) {
            return black_piece_valid(column1 - 'A', row1, column2 - 'A', row2); //black piece
        } else {
            return black_king_valid(column1 - 'A', row1, column2 - 'A', row2); //black king
        }
    }
}

//same, compares the board to another to see if they are identical
//parameters: another board to compare to
//returns: a bool indicating they are the same
bool Board::same(Board ref2) {
    bool hold = false;
    if (game_board[0][0] == BLANK)
        hold = true;

    for (int i = 0; i < DIMEN; i++) {
        for (int j = hold; j < DIMEN; j += 2) {
            if (game_board[i][j] != ref2.look(i, j)) {
                return false;
            }
        }
        hold = (!hold);
    }

    return true;
}

//white_king_valid, helper function to check_validity and simple_check, checks that a proposed 
//move for a white king is valid
//parameters: two ints for the column and row of the first coordinate, and two more for the second
//returns: a bool for whether the move is valid
bool Board::white_king_valid(int col1, int row1, int col2, int row2) {
    if ((abs(row2 - row1) == 1) && (abs(col2 - col1) == 1)) {
        return true; //single space move in any direction
    } else if ((row2 - row1 == -2) && (col2 - col1 == -2) && //taking moves
    ((game_board[row1 - 1][col1 - 1] == BLACK_PIECE) || 
    (game_board[row1 - 1][col1 - 1] == BLACK_KING))) {
        return true;
    } else if ((row2 - row1 == -2) && (col2 - col1 == 2) &&
    ((game_board[row1 - 1][col1 +1] == BLACK_PIECE) || 
    (game_board[row1 - 1][col1 + 1] == BLACK_KING))) {
        return true;
    } else if ((row2 - row1 == 2) && (col2 - col1 == -2) &&
    ((game_board[row1 + 1][col1 - 1] == BLACK_PIECE) || 
    (game_board[row1 + 1][col1 - 1] == BLACK_KING))) {
        return true;
    } else if ((row2 - row1 == 2) && (col2 - col1 == 2) &&
    ((game_board[row1 + 1][col1 + 1] == BLACK_PIECE) || 
    (game_board[row1 + 1][col1 + 1] == BLACK_KING))) {
        return true;
    }

    return false;
}

//white_piece_valid, helper function to check_validity and simple_check, checks that a proposed 
//move for a white piece is valid
//parameters: two ints for the column and row of the first coordinate, and two more for the second
//returns: a bool for whether the move is valid
bool Board::white_piece_valid(int col1, int row1, int col2, int row2) {
    if ((row2 - row1 == -1) && (abs(col2 - col1) == 1)) {
        return true; //one move forward
    } else if ((row2 - row1 == -2) && (abs(col2 - col1) == 2)) {
        //taking moves
        if ((col2 - col1 == 2) && ((game_board[row1 - 1][col1 + 1] == BLACK_PIECE) ||
        (game_board[row1 - 1][col1 + 1] == BLACK_KING))) {
            return true;
        } else if ((col2 - col1 == -2) && ((game_board[row1 - 1][col1 - 1] == BLACK_PIECE) ||
        (game_board[row1 - 1][col1 - 1] == BLACK_KING))) {
            return true;
        }
    }

    return false;
}

//black_king_valid, helper function to check_validity and simple_check, checks that a proposed 
//move for a black king is valid
//parameters: two ints for the column and row of the first coordinate, and two more for the second
//returns: a bool for whether the move is valid
bool Board::black_king_valid(int col1, int row1, int col2, int row2) {
    if ((abs(row2 - row1) == 1) && (abs(col2 - col1) == 1)) {
        return true; //single space move in any direction
    } else if ((row2 - row1 == -2) && (col2 - col1 == -2) && //taking moves
    ((game_board[row1 - 1][col1 - 1] == WHITE_PIECE) ||
    (game_board[row1 - 1][col1 - 1] == WHITE_KING))) {
        return true;
    } else if ((row2 - row1 == -2) && (col2 - col1 == 2) &&
    ((game_board[row1 - 1][col1 + 1] == WHITE_PIECE) ||
    (game_board[row1 - 1][col1 + 1] == WHITE_KING))) {
        return true;
    } else if ((row2 - row1 == 2) && (col2 - col1 == -2) &&
    ((game_board[row1 + 1][col1 - 1] == WHITE_PIECE) ||
    (game_board[row1 + 1][col1 - 1] == WHITE_KING))) {
        return true;
    } else if ((row2 - row1 == 2) && (col2 - col1 == 2) &&
    ((game_board[row1 +1][col1 + 1] == WHITE_PIECE) ||
    (game_board[row1 + 1][col1 + 1] == WHITE_KING))) {
        return true;
    }

    return false;
}

//black_piece_valid, helper function to check_validity and simple_check, checks that a proposed 
//move for a black piece is valid
//parameters: two ints for the column and row of the first coordinate, and two more for the second
//returns: a bool for whether the move is valid
bool Board::black_piece_valid(int col1, int row1, int col2, int row2) {
    if ((row2 - row1 == 1) && (abs(col2 - col1) == 1)) {
        return true;
    } else if ((row2 - row1 == 2) && (abs(col2 - col1) == 2)) {
        //taking moves
        if ((col2 - col1 == 2) && ((game_board[row1 + 1][col1 + 1] == WHITE_PIECE) ||
        game_board[row1 + 1][col1 + 1] == WHITE_KING)) {
            return true;
        } else if ((col2 - col1 == -2) && ((game_board[row1 + 1][col1 - 1] == WHITE_PIECE) ||
        game_board[row1 + 1][col1 - 1] == WHITE_KING)) {
            return true;
        }
    }

    return false;
}


