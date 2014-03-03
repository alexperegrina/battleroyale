#include <algorithm>

#include "Board.hh"
#include "Action.hh"

#include <sstream>

#include <cctype>

using namespace std;

Board::Board (istream& is) {
    string s;

    string version;
    is >> s >> version;
    assert(s == "battleroyale");
    assert(version == "v1");

    is >> s >> nb_players_;
    assert(s == "nb_players");
    assert(nb_players_ == 2 || nb_players_ == 4);

    is >> s >> nb_rounds_;
    assert(s == "nb_rounds");
    assert(nb_rounds_ >= 1);

    is >> s >> nb_farmers_;
    assert(s == "nb_farmers");
    assert(nb_farmers_ >= 0);
    
    is >> s >> nb_knights_;
    assert(s == "nb_knights");
    assert(nb_knights_ >= 0);
    
    is >> s >> farmers_health_;
    assert(s == "farmers_health");
    assert(farmers_health_ >= 1);

    is >> s >> knights_health_;
    assert(s == "knights_health");
    assert(knights_health_ >= 1);
    
    is >> s >> farmers_regen_;
    assert(s == "farmers_regen");
    assert(farmers_regen_ >= 0);

    is >> s >> knights_regen_;
    assert(s == "knights_regen");
    assert(knights_regen_ >= 0);
    
    is >> s >> damage_min_;
    assert(s == "damage_min");
    assert(damage_min_ >= 0);
    
    is >> s >> damage_max_;
    assert(s == "damage_max");
    assert(damage_max_ >= 0);

    is >> s >> rows_;
    assert(s == "rows");
    assert(rows_ >= 4);

    is >> s >> cols_;
    assert(s == "cols");
    assert(cols_ >= 4);

    names_ = vector<string>(nb_players_);
    is >> s;
    assert(s == "names");
    for (int pl = 0; pl < nb_players_; ++pl) {
        is >> names_[pl];
    }

    is >> s >> round_;
    if (s == "?") cerr << "ERROR: Number of names does not match number of players" << endl;
    assert(s == "round");
    assert(round_ < nb_rounds_);

    cells_ = vector< vector<Cell> >(rows_, vector<Cell>(cols_));
    for (int i = 0; i < rows_; ++i) {
        for (int j = 0; j < cols_; ++j) {
            cells_[i][j].unit = -1;
            cells_[i][j].pos = Pos(i, j);
            cells_[i][j].owner = -1;
            cells_[i][j].type = Empty;
            char c;
            is >> c;
            c = toupper(c);
            //cerr << c;
            switch (c) {
                case '0':
                    cells_[i][j].owner = 0;
                    break;
                case '1':
                    cells_[i][j].owner = 1; 
                    break;
                case '2': 
                    cells_[i][j].owner = 2;
                    break;
                case '3': 
                    cells_[i][j].owner = 3; 
                    break;
                case '.': 
                    break; //Empty cell
                case 'X': 
                    cells_[i][j].type = Wall;
                    break;
                default:
                    cerr << "Unexpected '" << c << "' in board definition" << endl;
                    assert(false);
            } 
        }
        //cerr << endl;
    }
    
    is >> s;
    assert(s == "score");
    score_ = vector<int>(nb_players_);
    for (int i = 0; i < nb_players_; ++i) {
        is >> score_[i];
    }

    is >> s;
    assert(s == "status");
    status_ = vector<double>(nb_players_);
    for (int i = 0; i < nb_players_; ++i) {
        is >> status_[i];
    }
    

    units_ = vector<Unit>(nb_players_ * (nb_farmers_ + nb_knights_));
    
    unsigned int id = 0;
    char type;
    while (is >> type) {
        int i, j;
        int player;
        int health;
        
        is >> player >> i >> j >> health;
        
        assert(player >= 0 && player < nb_players_);
        assert(health > 0);
        assert(i >= 0 && i < rows_);
        assert(j >= 0 && j < cols_);        
        
        units_[id].id = id;
        units_[id].player = player;
        units_[id].health = health;
        units_[id].pos.i = i;
        units_[id].pos.j = j;
        cells_[i][j].unit = id;
        switch(type) {
            case 'f':
                units_[id].type = Farmer;
                assert(health <= farmers_health_);
                break;
            case 'k':
                units_[id].type = Knight;
                assert(health <= knights_health_);
                break;
            default:
                assert(false);
                break;
        }
        
        id++;
        
        if (id >= units_.size()) break;
    }
    
    if (id == 0) { //Empty board given, generate units
        //cerr << "Empty board given, generating units" << endl;
        for (int player = 0; player < nb_players_; ++player) {
            for (int rep = 0; rep < nb_farmers_; ++rep) {
                units_[id].type = Farmer;
                units_[id].id = id;
                units_[id].player = player;
                units_[id].health = farmers_health_;
                spawn_unit(id);
                id++;
            }
            for (int rep = 0; rep < nb_knights_; ++rep) {
                units_[id].type = Knight;
                units_[id].id = id;
                units_[id].player = player;
                units_[id].health = knights_health_;
                spawn_unit(id);
                id++;
            }
            
        }
    }
    else if (id < units_.size()) { //Half-full board given, error
        cerr << "Read " << id << " units, expected " << units_.size() << endl;
        assert(false);
    }
    else { //Full board given, ok
        //cerr << "Full board given" << endl;
    }
        
    farmers_by_player_ = vector< vector<int> >(nb_players_);
    knights_by_player_ = vector< vector<int> >(nb_players_);
    vectors_by_player_updated_ = false;

    //cerr << "Board constructor ok" << endl;
    
}


void Board::spawn_unit(int id) {
    Unit& u = units_[id];
    int mini, minj, maxi, maxj;
    switch(u.player) {
        default: 
            assert(0);
        case 0:
            mini = 1;
            maxi = rows_/2 - 1;
            minj = 1;
            maxj = cols_/2 - 1;
            break;
        case 1:
            mini = rows_/2 + 1;
            maxi = rows_ - 2;
            maxj = cols_ - 2;
            minj = cols_/2 + 1;
            break;
        case 2:
            mini = 1;
            maxi = rows_/2 - 1;
            maxj = cols_ - 2;
            minj = cols_/2 + 1;
            break;
        case 3:
            mini = rows_/2 + 1;
            maxi = rows_ - 2;
            minj = 1;
            maxj = cols_/2 - 1;
            break;
    }
    
    int i, j;
    do {
        i = mini + random() % (maxi-mini);
        j = minj + random() % (maxj-minj);
    } while(cells_[i][j].type != Empty || cells_[i][j].unit != -1);

    if (pos_ok(u.pos.i,u.pos.j)) cells_[u.pos.i][u.pos.j].unit = -1;
    
    cells_[i][j].unit = id;
    
    u.pos.i = i;
    u.pos.j = j;
    
    if (u.type == Farmer) {
        cells_[i][j].owner = u.player;
    }
    
}

void Board::update_vectors_by_player() {
    for (int p = 0; p < nb_players(); ++p) {
        farmers_by_player_[p].clear();
        knights_by_player_[p].clear();
    }
    for (int id = 0; id < nb_units(); ++id) {
        const Unit& u = units_[id];
        switch (u.type) {
            case Farmer:
                farmers_by_player_[u.player].push_back(id);
                break;
            case Knight:
                knights_by_player_[u.player].push_back(id);
                break;
            default:
                //cerr << "Unknown type: " << u.type << endl;
                assert(0);
        }
    }
    vectors_by_player_updated_ = true;
}

void Board::update_scores() {
    //Update scores
    for (int player = 0; player < nb_players(); ++player) {
        score_[player] = 0;
    }
    for (int i = 0; i < rows_; ++i) {
        for (int j = 0; j < cols_; ++j) {
            int o = cells_[i][j].owner;
            if (o >= 0) score_[o]++;
        }
    }
}

void Board::print_preamble (ostream& os) const {
    os << "battleroyale v1" << endl;
    os << "nb_players " << nb_players() << endl;
    os << "nb_rounds " << nb_rounds() << endl;
    os << "nb_farmers " << nb_farmers() << endl;
    os << "nb_knights " << nb_knights() << endl;
    os << "farmers_health " << farmers_health() << endl;
    os << "knights_health " << knights_health() << endl;
    os << "farmers_regen " << farmers_regen() << endl;
    os << "knights_regen " << knights_regen() << endl;
    os << "damage_min " << damage_min() << endl;
    os << "damage_max " << damage_max() << endl;
    os << "rows " << rows() << endl;
    os << "cols " << cols() << endl;
    os << "names";
    for (int player = 0; player < nb_players(); ++player) os << " " << name(player);
    os << endl << endl;
}



void Board::print (ostream& os) const {
    os << endl;
    
    os << "round " << round() << endl;
    
    for (int i = 0; i < rows_; ++i) {
        for (int j = 0; j < cols_; ++j) {
            const Cell& c = cells_[i][j];
            if (c.type == Wall) os << 'X';
            else if (c.owner < 0) os << '.';
            else os << c.owner;
        }
        os << endl;
    }

    os << "score";
    for (int i = 0; i < nb_players(); ++i) os << " " << score_[i];
    os << endl;
    
    os << "status";
    for (int i = 0; i < nb_players(); ++i) os << " " << status_[i];
    os << endl;
    
    for (int id = 0; id < nb_units(); ++id) {

        string s;
        if (unit(id).type == Knight) s = "k";
        else if (unit(id).type == Farmer) s = "f";
        else assert(0);

        int i, j, health, player;
        os  << s << " "
            << unit(id).player << " "    
            << unit(id).pos.i << " "
            << unit(id).pos.j << " "
            << unit(id).health << endl;

    }
    os << endl;
}




void Board::print_debug (ostream& os) const {
    print(os);
}




Board Board::next (const vector<Action>& as, Action& actions_done, const Board& b0) const {

    // b will be the new board we shall return
    Board b = *this;

    // increment the round
    ++b.round_;

    // randomize turns
    vector<int> turns(nb_players());
    for (int player = 0; player < nb_players(); ++player) {
        turns[player] = player;
    }
    random_shuffle(turns.begin(), turns.end());

    // move each unit
    vector<bool> moved(nb_units(), false);
    for (int turn = 0; turn < nb_players(); ++turn) {
        int player = turns[turn];
        queue<Movement> q = as[player].q_;
        while (not q.empty()) {
            Movement m = q.front();  q.pop();
            int id = m.i;
            Dir dir = m.d;
            if (!unit_ok(id)) {
                cerr << "Id out of range" << endl;
                continue;
            }
            if (dir != None and dir != Left and dir != Right and dir != Top and dir != Bottom) {
                cerr << "Direction not valid" << endl;
                continue;
            }
            if (moved[id]) continue;
            moved[id] = true;
            bool ok = b.move(player, id, dir, b0);
            if (ok) {
                actions_done.q_.push(Movement(id, dir));
            }
        }
    }

    //Heal units that have not moved
    for (int id = 0; id < nb_units(); id++) {
        if (moved[id]) continue;
        Unit& u = b.units_[id];
        switch (u.type) {
            case Farmer:
                u.health += farmers_regen_;
                if (u.health > farmers_health_) u.health = farmers_health_;
                break;
            case Knight:
                u.health += knights_regen_;
                if (u.health > knights_health_) u.health = knights_health_;
                break;
            default:
                assert(0);
        }
    }
    
    b.update_scores();
    
    return b;
}



bool Board::move (int player, int id, Dir d, const Board& b0) {

    if (d == None) return false;

    Unit& u1 = units_[id];
    if (u1.player != player) return false; //Trying to move another player's unit
                
    Pos p1 = u1.pos;
    if (not pos_ok(p1)) return false;

    Pos p2 = dest(p1, d);
    if (not pos_ok(p2)) return false;

    Cell& c1 = cells_[p1.i][p1.j];
    Cell& c2 = cells_[p2.i][p2.j];

    if (c2.type == Wall) return false;

    if (c2.unit != -1) { //There is a unit at our destination
        
        if (u1.type != Knight) return false; //This unit can not attack
            
        Unit& u2 = units_[c2.unit];
        if (u2.player == u1.player) return false; //Trying to attack your own unit
        
        int damage = damage_min_ + random() % (damage_max_ - damage_min_);
        u2.health -= damage;
        if (u2.health <= 0) {
            u2.player = u1.player;
            u2.health = (u2.type == Farmer)? farmers_health_ : knights_health_;
            spawn_unit(u2.id);
            vectors_by_player_updated_ = false;
        }
        
        return true;
        
    } else { //We can move there

        c2.unit = c1.unit;
        c1.unit = -1;
        u1.pos = p2;
        
        if (u1.type == Farmer) {
            c2.owner = u1.player;
        }
        
        return true;

    }

    
}

