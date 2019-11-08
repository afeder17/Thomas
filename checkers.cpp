/*checkers.cpp, the main for the Thomas program. Has three modes. In mode 1, the user plays 
against the Thomas AI as red. In mode 2, the user plays against Thomas as black. In mode 3, 
Thomas plays 10,000 games against a color reversed copy of itself, Hayden, so that the effect of 
strategy changes on gameplay can be observed statistially. In each mode, the AI(s) can be set to 
different difficulty levels corresponding to different evaluation minimax tree depths. In mode 3, the 
first three moves are chosen randomly and turn order switched between games, to increase variety 
of games.

Moves are entered using chess notation, rather than checkers notation, as this is more intuitive to 
use in the terminal environment.*/

#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <cctype>
#include <unistd.h>
#include <thread>
#include "board.h"
#include "AI.h"
#include "AI_r.h"

using namespace std;

//Board dimensions are used for notation conversion
const int DIMEN = 8;

//player vs. Thomas with player as white
void v_AI_w();

//player vs. Thomas with player as black
void v_AI_b();

//function for having Thomas play against a color-reversed copy, Hayden, for strength of gameplay 
//with different changes 
void AI_v_AI();

//get the desired difficulty level
int get_level();

//remove the non-alphanumeric characters from a string
string stripNonAlphaNum(string input);

//get the player's desired move, putting the notation into existing variables
void get_move(string &input, Board *&game, vector<Board> &path, AI &Thomas, bool &go, 
char &col1, int &row1, char &col2, int &row2, int &turns, bool flipped);

//cleans inputted user moves, and performs move undo action if command is entered
void format(string &input, Board *&game, vector<Board> &path, AI &Thomas, int &turns, 
bool flipped);

//perform operations for the end of a player/AI move necessary to maintain game flow
void end_move(char &move, vector<Board> &path, Board *game, int &turns, bool over);

//request move coordinates from Thomas, and then execute that move on the board
void Thomas_turn(char &move, vector<Board> &path, Board *&game, int &turns, bool &over,
AI &Thomas, bool flipped);

//request move coordinates from Hayden, and then execute that move on the board
void Hayden_turn(char &move, vector<Board> &path, Board *&game, int &turns, bool &over,
AI_r &Hayden, bool flipped);

//run the player move input-output sequence, and then execute the player's moves/commands
void player_turn(char &move, vector<Board> &path, Board *&game, int &turns, bool &over,
AI &Thomas, bool flipped);

//runs the print or reverse print functions of the board object depending on a bool for the orientation
void auto_print(Board *&game, bool flipped);

//adjudicate games between Thomas and Hayden when they've gone on for too long
void referee(char &move, Board *&game, int &turns, vector<Board> &path, bool &over, 
bool &tied);

//keep track of wins, losses and draws when Thomas and Hayden play a series of games
void track(char move, Board *&game, bool over, bool &tied, bool &alt, int &T, int &H, int &Tie);

int main() {
    int mode = 0;
    bool chosen = false;
    cout << "Play Thomas as red (enter 1), as black (2), or launch Thomas vs. Hayden experimental"; 
    cout << " mode (3)\n";

    //run input loop until a valid number is entered
    do {
        if (cin >> mode)
            chosen = true;
    } while ((!chosen) || ((mode != 1) && (mode != 2) && (mode != 3)));

    if (mode == 1) {
        v_AI_w(); //player as white
    } else if (mode == 2) {
        v_AI_b(); //player as black
    } else {
        AI_v_AI(); //Thomas vs. Hayden
    }

    system("say game over!");

    return 0;
}

//v_AI_w, run a game between the player and Thomas, creating a board and, turn by turn, asking 
//the player and Thomas for moves, ending the game and declaring a winner when one of them has 
//no possible moves
//parameters: NA
//returns: void
void v_AI_w() {
    Board *game = new Board;
    vector <Board> path;
    path.push_back(*game);
    
    AI Thomas; //declare Thomas and set his difficulty
    Thomas.set_difficulty(get_level());
    Thomas.update_AI(*game); //update Thomas at start because it goes first

    game->print();
    Thomas.intro();
    
    //variables for turn status, flow of game loops
    int turns = 0;
    char move = 'W';
    bool over = false;

    cout << "Enter a move, using board notation for the square to move from and to (i.e. A3  B4), or ";
    cout << "type 'undo' to go back a move\n";
    cin.ignore(100, '\n'); //clear cin for first move
    cin.clear();

    //turn status is changed by turn functions, run game until one indicates it's over
    while (!over) {
        if (move == 'W') {
            player_turn(move, path, game, turns, over, Thomas, false);
        } else {
            Thomas_turn(move, path, game, turns, over, Thomas, false);
        }
    }
}

//v_AI_b, run a game between the player and Thomas, creating a board and, turn by turn, asking 
//the player and Thomas for moves, ending the game and declaring a winner when one of them has 
//no possible moves, flips the colors by printing the board with reversed notation markers, and with 
//the pieces printed as their opposite color equivalents and flipped horizontally, so that the board 
//object treats the board array the same, but it's displayed in reverse colors.
//parameters: NA
//returns: void
void v_AI_b() {
    Board *game = new Board;
    vector <Board> path;
    game->reverse(); //set up game with reversed columnar orientation
    path.push_back(*game);
    
    AI Thomas;
    Thomas.set_difficulty(get_level());
    Thomas.update_AI(*game); //update Thomas at start because it goes first

    game->print_reverse(); //print black pieces as their red equivalents and vice versa
    Thomas.intro();
    
    //variables for turn status, flow of game loops
    int turns = 0;
    char move = 'B';
    bool over = false;

    cout << "Enter a move, using board notation for the square to move from and to (i.e. A3  B4), or ";
    cout << "type 'undo' to go back a move\n";
    cin.ignore(100, '\n'); //clear cin for first move
    cin.clear();

    //turn status is changed by turn functions, run game until one indicates it's over
    while (!over) {
        if (move == 'W') {
            player_turn(move, path, game, turns, over, Thomas, true);
        } else {
            Thomas_turn(move, path, game, turns, over, Thomas, true);
        }
    }
}

//AI_v_AI, similar to the Thomas vs. player functions, but with Thomas playing against another AI, 
//Hayden, and a series of variables to keep track of past games. Additionally, to increase game 
//variation, switches who goes first every game and runs the first three moves randomly, similar to 
//procedure for tournament play
//parameters: NA
//returns: void
void AI_v_AI() {
    int T = 0, H = 0, Tie = 0; //counters for wins and draws
    bool alt = true;
    char move;
    int both = get_level();
    srand(time(NULL));
    for (int x = 0; x < 100; x++) {
    Board *game = new Board;

    //change turn order and random opening procedure
    if (alt) {
        move = 'W';
        game->rando_r(x); //seed each random opening with a time-independent second value
    } else {
        move = 'B';
        game->rando(x); //seed each random opening with a time-independent second value
    }
    
    vector <Board> path;
    path.push_back(*game);
    AI Thomas;
    AI_r Hayden;
    Thomas.set_difficulty(both), Hayden.set_difficulty(both);
    int turns = 0;
    bool over = false, tied = false;

    Thomas.update_AI(*game);
    Hayden.update_AI(*game);
    game->print();

    //run until Thomas, Hayden or referee declare game is over
    while (!over) {
        if (move == 'W') {
            Hayden.update_AI(*game);
            Hayden_turn(move, path, game, turns, over, Hayden, false);
            Thomas.update_AI(*game);
            //system("sleep 1");
        } else if (move == 'B') {
            Thomas.update_AI(*game);
            Thomas_turn(move, path, game, turns, over, Thomas, false);
            Hayden.update_AI(*game);
        }

        referee(move, game, turns, path, over, tied); //adjudicate non-progressing games
    }

    track(move, game, over, tied, alt, T, H, Tie); //keep track of game record
    }
}

//stripNonAlphaNum, strips the non-alphanumeric characters
//parameters: a string to strip
//returns: a processed string
string stripNonAlphaNum(string input) {
    int len = input.size();
    string output = ""; //start return value as the empty string, concatenate the valid chars onto it
    
    for (int i = 0; i < len; i++) {
        if (isalnum(input[i]))
            output = output + input[i]; //concatenate valid characters
    }
    
    return output;
}

//get_move, ask the player for moves/undo commands, execute undo commands and sanitize/
//check validity for proposed moves
//parameters: a ref to the input string, the game object and vector record, a ref to Thomas, variables 
//for the indicated move, the go bool and turn number int, and a bool for whether to flip notation
//returns: void
void get_move(string &input, Board *&game, vector<Board> &path, AI &Thomas,
bool &go, char &col1, int &row1, char &col2, int &row2, int &turns, bool flipped) {
    go = false;
    
    cout << "Player to move:\n";
    
    do {
        format(input, game, path, Thomas, turns, flipped); //sanitize tita, perform undos

        for (size_t i = 0; i < input.size(); i++)
            input[i] = toupper(input[i]);

        //fill in notation variables
        col1 = input[0], row1 = input[1] - 48, col2 = input[2], row2 = input[3] - 48;

        if (flipped) {
            //convert notation for flipped board
            col1 = 'H' - col1 + 'A';
            col2 = 'H' - col2 + 'A';
            row1 = DIMEN + 1 - row1;
            row2 = DIMEN + 1 - row2;
        }
        
        row1 = abs(DIMEN - row1); //convert board notation rows into array positions
        row2 = abs(DIMEN - row2);

        //loop ends with a valid move
        go = game->check_validity(col1, row1, col2, row2, 'W');
        if (!go)
            cout << "Invalid move\n";
    } while (!go);
}

//format, sanitize tita, check for a perform undo commands, loop until input is valid notation
//parameters: a ref to the input string, the game board pointer and vector record, a printable string 
//corresponding to the player's color, a ref to Thomas, an int for the number of turns and a bool for 
//whether the colors are reversed
//returns: void
void format(string &input, Board *&game, vector<Board> &path, AI &Thomas,
int &turns, bool flipped) {
    while (((getline(cin, input) && (input.length() != 4))) || (!isalpha(input[0])) || 
    (!isdigit(input[1])) || (!isalpha(input[2])) || (!isdigit(input[3]))) {
        input = stripNonAlphaNum(input);
        for (size_t i = 0; i < input.size(); i++)
            input[i] = toupper(input[i]);
        
        if (input != "") {
            if (input == "UNDO") {
                if (turns == 0) {
                    cout << "Nothing to undo yet\n"; //cannot undo if nothing has been played
                    input = "";
                } else {
                    delete game;
                    game = new Board(path[turns - 2]); //replace board object with past copy
                    turns -= 2; //set back counter
                    path.pop_back();
                    path.pop_back();
                    Thomas.update_AI(*game);
                    auto_print(game, flipped);

                    cout << "Move undone. Player to move:\n";
                    system("say move undone");
                }
            } else {
                if (((input.length() != 4)) || (!isalpha(input[0])) || (!isdigit(input[1])) || 
                (!isalpha(input[2])) || (!isdigit(input[3]))) {
                    cout << "Invalid notation, use letter-number pairs (i.e. A3 B4)\n"; //valitite input
                    system("say please use correct notation");
                } else {
                    break;
                }
            }
        }
    }
}

//end_move, records a move in the record, updates the turn counter, switches turn status, and prints 
//an end-of-game message if the game has been won
//parameters: a ref to the string indicating whose move it is, the vector record, a pointer to the game 
//board, a ref to the turn number, a bool for whether the game is over
//returns: void
void end_move(char &move, vector<Board> &path, Board *game, int &turns, bool over) {
    //record move
    path.push_back(*game);
    turns++;

    //game is won by last player to be able to move
    if (over) {
        if (move == 'W') {
            cout << "Game over, red wins!\n";
        } else if (move == 'B') {
            cout << "Game over, black wins!\n";
        }
    }

    //switch turn
    if (move == 'W') {
        move = 'B';
    } else if (move == 'B') {
        move = 'W';
    }
}

//Thomas_turn, ask Thomas for its move, execute it on the board, make multiple jumps if possible, 
//then end the turn and change colors
//parameters: a ref to the move string, the vector record, a pointer to the game board, a ref to the 
//turn number, a bool for whether the game is over, a ref to Thomas, and a bool for whether to flip
//returns: void
void Thomas_turn(char &move, vector<Board> &path, Board *&game, int &turns, bool &over,
AI &Thomas, bool flipped) {
    cout << "Thomas goes\n";
    cout << "...\n";
    
    if ((turns > 16) && (((game->same(path[turns - 4])) && (game->same(path[turns - 8]))) ||
    ((game->same(path[turns - 8])) && (game->same(path[turns - 16]))))) {
        Thomas.move(true, true); //if caught in a loop, play a different, slightly worse move
    } else {
        Thomas.move(false, true); //play the best move
    }
    
    game->make_move(Thomas.get_col1(), Thomas.get_row1(), Thomas.get_col2(), 
    Thomas.get_row2()); //make move
    
    auto_print(game, flipped);
    over = game->check_win(move);

    //multiple jumps
    if ((abs(Thomas.get_row2() - Thomas.get_row1()) == 2) && 
    (game->jump_possible(Thomas.get_col2(), Thomas.get_row2(), 'B')) && 
    (!game->kinged())) {
        bool another = false;
        do {
            cout << "Thomas jumps again\n";
            cout << "...\n";
            another = Thomas.multi(true); //jump again if possible after this jump
            game->make_move(Thomas.get_col1(), Thomas.get_row1(), Thomas.get_col2(), 
            Thomas.get_row2());
            auto_print(game, flipped);
            over = game->check_win(move);
        } while (another);
    }

    end_move(move, path, game, turns, over);
}

//Hayden_turn, ask Hayden for its move, execute it on the board, make multiple jumps if possible, 
//then end the turn and change colors
//parameters: a ref to the move string, the vector record, a pointer to the game board, a ref to the 
//turn number, a bool for whether the game is over, a ref to Hayden, and a bool for whether to flip
//returns: void
void Hayden_turn(char &move, vector<Board> &path, Board *&game, int &turns, bool &over,
AI_r &Hayden, bool flipped) {
    cout << "Hayden goes\n";
    cout << "...\n";
    
    if ((turns > 16) && (((game->same(path[turns - 4])) && (game->same(path[turns - 8]))) ||
    ((game->same(path[turns - 8])) && (game->same(path[turns - 16]))))) {
        Hayden.move(true); //if caught in a loop, play a different, slightly worse move
    } else {
        Hayden.move(false); //play the best move
    }
    
    game->make_move(Hayden.get_col1(), Hayden.get_row1(), Hayden.get_col2(), 
    Hayden.get_row2()); //make move
    auto_print(game, flipped);
    over = game->check_win(move);

    //multiple jumps
    if ((abs(Hayden.get_row2() - Hayden.get_row1()) == 2) && 
    (game->jump_possible(Hayden.get_col2(), Hayden.get_row2(), 'W')) && 
    (!game->kinged())) {
        bool another = false;
        do {
            cout << "Hayden jumps again\n";
            cout << "...\n";
            another = Hayden.multi(); //jump again if possible after this jump
            game->make_move(Hayden.get_col1(), Hayden.get_row1(), Hayden.get_col2(), 
            Hayden.get_row2());
            auto_print(game, flipped);
            over = game->check_win(move);
        } while (another);
    }

    end_move(move, path, game, turns, over);
}

//player_turn, prompt the player for their move, execute it, go the board object based multiple jump 
//sequence if possible, then update Thomas
//parameters: a ref to the move string, the vector record, a pointer to the game board, a ref to the 
//turn number, a bool for whether the game is over, a ref to Thomas, and a bool for whether to flip
void player_turn(char &move, vector<Board> &path, Board *&game, int &turns, bool &over,
AI &Thomas, bool flipped) {
    //move execution variables
    char col1, col2;
    int row1, row2;
    string input = "";
    bool go;

    get_move(input, game, path, Thomas, go, col1, row1, col2, row2, turns, flipped);
    game->make_move(col1, row1, col2, row2); //make move
    auto_print(game, flipped);
    over = game->check_win(move);

    //multiple jumps if possible
    if (abs(game->last_row() - row1) == 2) {
        if (flipped) {
            game->multi_hop_r(move); //piece can move again if it can take from its new position
        } else {
            game->multi_hop(move);
        }
        
        over = game->check_win(move);
    }

    end_move(move, path, game, turns, over);
    Thomas.update_AI(*game); //update Thomas
}

//auto_print, print the board in either normal or reversed orientation depending on input
//parameters: a pointer to the game board to print from, a bool for whether to print in reversed mode
//returns: void
void auto_print(Board *&game, bool flipped) {
    if (flipped) {
        game->print_reverse();
    } else {
        game->print();
    }

    system("say ta");
}

//referee, used in AI_v_AI, calls a game if it's gone on for too long without progress
//parameters: a string for the move, a pointer to the game Board, the vector record, a bool ref for 
//whether the game is over, a bool ref for whether the game is a tie
//returns: void
void referee(char &move, Board *&game, int &turns, vector<Board> &path, bool &over, 
bool &tied) {
    //call a draw if pieces are equal and no takes have happened for 200 moves
    if ((turns > 200) && (game->get_num_black() == path[turns - 100].get_num_black()) &&
    (game->get_num_white() == path[turns - 100].get_num_white()) && 
    (game->get_num_black() == game->get_num_white())) {
        over = true;
        tied = true;
        cout << "Draw, by 200-move rule\n";
    }

    //if one player has more pieces, call a win for them if nothing has been taken in 1000 moves
    if ((turns > 400) && (game->get_num_black() == path[turns - 200].get_num_black()) &&
    (game->get_num_white() == path[turns - 200].get_num_white()) &&
    game->get_num_black() != game->get_num_white()) {
        over = true;
        if (game->get_num_white() > game->get_num_black()) {
            cout << "Called win for red\n";
            move = 'B';
        } else {
            cout << "Called win for black\n";
            move = 'W';
        }
    }
}

//track, keep track of wins and losses in AI_v_AI mode, uptiting a displaying counters
//parameters: a string for the move, a pointer to the game Board, a bool for whether the game is 
//over, a bool ref for whether it's tied, a bool for ref for the alternation bool, int refs for the Thomas 
//win, Hayden win, and tie counters
//returns: void
void track(char move, Board *&game, bool over, bool &tied, bool &alt, int &T, int &H, int &Tie) {
    //count a win
    if ((over) && (!tied)) {
        if (move == 'W') {
            T++;
        } else if (move == 'B') {
            H++;
        }
    //count a draw
    } else if ((over) && (tied)) {
        Tie++;
    }

    delete game;
    tied = false, alt = (!alt);

    //print counter values
    cout << endl;
    cout << "Thomas: " << T << endl;
    cout << "Hayden: " << H << endl;
    cout << "Draws: " << Tie << endl;
}

//get_level, ask the player/simulation runner what difficulty level the AI should play at
//parameters: NA
//returns: an int for the difficulty level
int get_level() {
    int mode = 0;
    bool chosen = false;
    cout << "Play on easy (1), medium (2), hard (3), 2-second evaluation (4), or 30-second"
    << " evaluation (5)\n";

    //run loop until input is valid
    do {
        if (cin >> mode)
            chosen = true;
    } while ((!chosen) || ((mode != 1) && (mode != 2) && (mode != 3) && (mode != 4) && (mode != 5)));

    return mode;
}








