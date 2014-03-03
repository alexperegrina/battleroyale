
#ifndef Action_hh
#define Action_hh

#include "Utils.hh"
#include "PosDir.hh"

using namespace std;

/*! \file
 * Contains the class Action and the struct Movement that it uses. 
 */ 

/**
 * A movement is defined by a unit id and direction
 */
struct Movement {
    int i;
    Dir d;
    
    /**
     * Constructor, given a unit id and movement direction
     */
    Movement(int id, Dir dir) : i(id), d(dir) { }
};

/**
 * Class that stores the actions requested by a player in a round.
 */

class Action {

    friend class Game;
    friend class SecGame;
    friend class Board;

    //Set of units that have already performed a movement
    set<int> u_;
    //List of movements to be performed this round
    queue<Movement> q_;

    //Read/write an action to/from a stream
    Action (istream& is);
    void print (ostream& os) const;

public:
    
    Action () { }
    
    /**
     * Adds a movement to the action list. Fails and returns false if a
     * movement is already present for this unit.
     */
    inline bool command (int id, Dir dir) {
        if (u_.find(id) == u_.end()) {
            q_.push(Movement(id, dir));
            u_.insert(id);
            return true;
        } else {
            cerr << "warning: action alread requested for this unit." << endl;
            return false;
        }
    }


};


#endif
