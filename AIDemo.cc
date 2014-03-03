#include "Player.hh"

#include <cmath>

using namespace std;



/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Demo



struct PLAYER_NAME : public Player {


    /**
     * Factory: returns a new instance of this class.
     * Do not modify this function.
     */
    static Player* factory () {
        return new PLAYER_NAME;
    }


    /**
     * Attributes for your player can be defined here.
     */     
    //vector<Dir> dirs;


    /**
     * Play method.
     * 
     * This method will be invoked once per each round.
     * You have to read the board here to place your actions
     * for this round.
     *
     * In this example, each unit moves in a random direction
     */     
    
    void play () {

        const vector<int> f = farmers(me());
        for (int i = 0; i < sze(f); ++i) {
            command(f[i], Dir(1 + random() % 4));
        }
        
        const vector<int>& k = knights(me());
        for (int i = 0; i < sze(k); ++i) {
            command(k[i], Dir(1 + random() % 4));
        }
        
    }

};



/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);

