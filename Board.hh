#ifndef Board_hh
#define Board_hh

#include "Utils.hh"
#include "PosDir.hh"

using namespace std;


/*! \file
 * Contains the Board class itself, and the classes of all the elements that can be on the board.
 */ 

/**
 * Defines if a cell is empty or has any special feature on it
 */
enum CellType {
    Empty, Wall,
    CellTypeSize
};

/**
 * Describes a cell in the board, and its contents
 */
struct Cell {
    ///A CellType defining the kind of cell
    CellType type;
    ///The id of player that last conquered this cell, -1 if none
    int owner;
    ///The id of unit if present, -1 otherwise
    int unit;
    ///The position inside the board
    Pos pos;
};

/**
 * Defines the type of the instances of the class Unit
 */
enum UnitType {
    Knight, Farmer,
    UnitTypeSize
};

/**
 * Defines a unit on the board and its properties
 */
struct Unit {
    ///A UnitType defining the kind of unit
    UnitType type; 
    ///The unique id for this unit in the board
    int id;
    ///The id of the player that owns this unit
    int player;
    ///The current health of the unit, if it reaches 0 it will die
    int health;
    ///The position inside the board    
    Pos pos;
};

/**
 * Forward declarations.
 */
class Action;


/**
 * Represents and manages the game board.
 */

class Board {

    // Allow access to the private part of Board.
    friend class Game;
    friend class SecGame;

    // Game settings
    int nb_players_;
    int nb_rounds_;
    int nb_farmers_;
    int nb_knights_;
    int farmers_health_;
    int knights_health_;
    int farmers_regen_;
    int knights_regen_;
    int damage_min_;
    int damage_max_;
    int rows_;
    int cols_;
    vector<string> names_;

    // Game state
    int round_;
    vector< vector<Cell> > cells_;
    vector<Unit> units_;
    vector<int> score_;
    vector<double>         status_;     // cpu status. <-1: dead, 0..1: %of cpu time limit

    //Aux vectors
    vector< vector<int> > farmers_by_player_;
    vector< vector<int> > knights_by_player_;
    bool vectors_by_player_updated_;
    
    /**
     * Construct a board by reading first round from a stream.
     */
    Board (istream& is);
    /**
     * Print the board to a stream.
     */
    void print (ostream& os) const;
    
    /**
     * Print the board preamble to a stream.
     */
    void print_preamble (ostream& os) const;

    /**
     * Print simplified board to a stream.
     * Just for debug, no interest.
     */
    void print_debug (ostream& os) const;

    /**
     * Computes the next board aplying the given actions as to the current board.
     * It also returns the actual actions performed.
     * b0 is the start board.
     */
    Board next (const vector<Action>& as, Action& actions_done, const Board& b0) const;

    /**
     * Applies a basic move.
     */
    bool move (int player, int id, Dir dir, const Board& b0);

    /**
     * Places a unit in its starting point
     */
    void spawn_unit(int id);

    /**
     * Called from knights(int player) or farmers(int player) to update the auxiliar vectors
     */
    void update_vectors_by_player();
    
    /**
     * Update scores
     */
    void update_scores();
    
public:

    /**
     * Empty constructor.
     */
    Board ()
    {   }

    /**
     * Returns the number of rounds for the game
     */
    inline int nb_rounds () const {
        return nb_rounds_;
    }

    /**
     * Returns the number of players in the game
     */
    inline int nb_players () const {
        return nb_players_;
    }

    /**
     * Returns the number of farmers each player starts with in the game
     */
    inline int nb_farmers () const {
        return nb_farmers_;
    }

    /**
     * Returns the number of knights each player starts with in the game
     */
    inline int nb_knights () const {
        return nb_knights_;
    }

    /**
     * Returns the initial (and maximum) health of the farmers
     */
    inline int farmers_health () const {
        return farmers_health_;
    }

    /**
     * Returns the initial (and maximum) health of the knights
     */
    inline int knights_health () const {
        return knights_health_;
    }
    
    /**
     * Returns the health a farmer recovers per round when not moving
     */
    inline int farmers_regen () const {
        return farmers_regen_;
    }
    
    /**
     * Returns the health a knight recovers per round when not moving
     */
    inline int knights_regen () const {
        return knights_regen_;
    }
    
    /**
     * Returns the minimum damage inflicted by a knight attack
     */
    inline int damage_min () const {
        return damage_min_;
    }
    
    /**
     * Returns the maximum damage inflicted by a knight attack
     */
    inline int damage_max () const {
        return damage_max_;
    }
    
    /**
     * Returns all the farmers of a player
     */
    inline const vector<int>& farmers(int player) {
        if (!vectors_by_player_updated_) update_vectors_by_player();
        return farmers_by_player_[player];    
    }
    
    /**
     * Returns all the knights of a player
     */
    inline const vector<int>& knights(int player) {
        if (!vectors_by_player_updated_) update_vectors_by_player();
        return knights_by_player_[player];
    }
    
    /**
     * Returns the total number of units in the game.
     */
    inline int nb_units () const {
        return (int)units_.size();
    }

    /**
     * Returns the number of rows of the maze in the game.
     */
    inline int rows () const {
        return rows_;
    }

    /**
     * Returns the number of columns of the maze in the game.
     */
    inline int cols () const {
        return cols_;
    }

    /**
     * Returns the current round.
     */
    inline int round () const {
        return round_;
    }

    /**
     * Return whether player is a valid player identifier.
     */
    inline bool player_ok (int player) const {
        return player >= 0 and player < nb_players();
    }

    /**
     * Return whether id is a valid unit identifier.
     */
    inline bool unit_ok (int id) const {
        return id >= 0 and id < (int)units_.size();
    }
    
    /**
     * Return whether (i,j) is a position inside the board.
     */
    inline bool pos_ok (int i, int j) const {
        return i >= 0 and i < rows() and j >= 0 and j < cols();
    }

    /**
     * Return whether p is a position inside the board.
     */
    inline bool pos_ok (const Pos& p) const {
        return pos_ok(p.i, p.j);
    }

    /**
     * Return whether (i,j) + d is a position inside the board.
     * Does not handle wrapping.
     */
    inline bool pos_ok (int i, int j, Dir d) const {
        return pos_ok(Pos(i, j) + d);
    }

    /**
     * Return whether p+d is a position inside the board.
     * Does not handle wrapping.
     */
    inline bool pos_ok (const Pos& p, Dir d) const {
        return pos_ok(p + d);
    }

    /**
     * Returns the position resulting from moving from
     * position p towards direction d taking wrapping into
     * account. Only checks geometry, not walls.
     */
    inline Pos dest (const Pos& p, Dir d) const {
        Pos q = p + d;

             if (q.i == -1) q.i = rows() - 1;
        else if (q.j == -1) q.j = cols() - 1;
        else if (q.i == rows()) q.i = 0;
        else if (q.j == cols()) q.j = 0;

        return q;
    }

    /**
     * Returns the name of a player.
     */
    inline string name (int player) const {
        check(player_ok(player));
        return names_.at(player);
    }

    /**
     * Returns the current score of a player.
     */
    inline int score (int player) const {
        check(player_ok(player));
        return score_.at(player);
    }

    /**
    * Returns a reference to the current cpu status of a player.
    */
    inline double& status (int player) {
        return status_[player];
    }

    /**
     * Returns the cell at (i, j).
     */
    inline const Cell& cell (int i, int j) const {
        check(pos_ok(i, j));
        return cells_[i][j];
    }

    /**
     * Returns the cell at p.
     */
    inline const Cell& cell (const Pos& p) const {
        return cell(p.i, p.j);
    }

    /**
     * Returns the unit with identifier id.
     */
    inline const Unit& unit (int id) const {
        check(unit_ok(id));
        return units_.at(id);
    }

};

#endif
