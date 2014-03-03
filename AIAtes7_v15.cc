#include "Player.hh"

#include <cmath>

using namespace std;



/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Ates7_v15



struct PLAYER_NAME : public Player {
	/*
		mejora de version:
		#aplico al inicio, un marcage de las zonas peligrosas para los farms cuando aplico dikjstra que actuaran como paredes y las bordeara
		 y funciona como sistema de huida.
		#recuperacion de vida para los knight a partir que su vida baje mas del 70% asta tener el  40% de los knight de la partida,
			iniciamente tenemos el 25% de los knight de la partida, una vez superado el limite, los kinght arriesgan mas disminuyendo el limite
			de recarga de salud.
	*/

    /**
     * Factory: returns a new instance of this class.
     * Do not modify this function.
     */
    static Player* factory () {
        return new PLAYER_NAME;
    }
  
/****************************************************************************************************************/
/*ESTRUCTURAS DE DATOS*/

	//estructura para la matriz de adjacencias
	struct Relacio {
		int num;
		Dir punt;	
	};

	typedef pair<double, Relacio> ArcP;
	typedef pair<double, int> ArcI;
	typedef vector<vector<int> > MatriuInt;
	typedef vector<vector<Relacio> > MatriuRel;
	typedef vector<vector<bool> > MatriuBool;
	
	//estructura que contiene des de 'u' a una opcion que nosotros eligemos la distancia y el camino
	struct Camino {
		int dist;
		Relacio nodo;
	};
	struct CaminoInt {
		int dist;
		int nodo;
	};

	struct compareCaminoPosi
	{  
	  bool operator()(const CaminoInt& l, const CaminoInt& r)  
	  {  
		  return l.dist > r.dist;  
	  }  
	};
	
	//comparacion para las colas de prioridad que contienen caminos
	struct compareCaminoDis
	{  
	  bool operator()(const ArcI& l, const ArcI& r)  
	  {  
		  return l.first > r.first;  
	  }  
	};
	
      
/****************************************************************************************************************/
	
	
	
	void action_Farmers(vector<int> &peligro) { 
		//cout << "action farmers " << endl;
		vector<int> f = farmers(me());
		vector<double> d;
		MatriuInt p;
		vector<int> atencion(rows()*cols(),0);
		int size = f.size(); //para quitar warning
		for(int i = 0; i < size; ++i) {
			pair<bool,vector<int> > pi;
			pi.first = false;
			vector<Relacio> adj;
			Unit u = unit(f[i]);
			int c = ij_num(u.pos.i,u.pos.j); 
			//no utilizo la cola de priorida con int porque necesito la distancia dentro de la estructura para obtener siempre el mas corto
			priority_queue<CaminoInt,vector<CaminoInt>,compareCaminoPosi> caminos,danger;			
			dijkstra_farm(c,d,p,caminos,danger,atencion,peligro);		
			accion(p,c,f[i],caminos);
			
		}
	}	
	
	void action_kinght() {
		//cout << "action knight " << endl;
		vector<int> k = knights(me());
		vector<double> d;
		MatriuInt p;
		vector<bool> atacoya(rows()*cols(),false);//vector para indicar un ataque inminente (se utiliza para scan_ratio1)
		vector<bool> futurolibre(rows()*cols(),false);//vector para indicar las futuras posiciones libres(se utiliza para scan_ratio1)
		vector<bool> futuroocupado(rows()*cols(),false);//vector para indicar las futuras posiciones libres(se utiliza para scan_ratio1)
		vector<int> numatacants = vector<int>((nb_farmers()+nb_knights())*nb_players (),0);//vector para dikstra que indicara el numero max de atacantsa una unidad
		int size = k.size(); //para quitar warning
		for(int i = 0; i < size; ++i) {
			
			pair<bool,vector<int> > pi;
			pi.first = false;

			Unit u = unit(k[i]);
			
				int c = ij_num(u.pos.i,u.pos.j); 
				//no utilizo la cola de priorida con int porque necesito la distancia dentro de la estructura para obtener siempre el mas corto
				priority_queue<CaminoInt,vector<CaminoInt>,compareCaminoPosi> caminos;
				vector<Relacio> ret;
				u_adjacentes(c,ret);
			
				pair<bool,Relacio> pr = scan_ratio1(ret,u,atacoya,futurolibre,futuroocupado);
				if(not pr.first) {
					//si entramos significa que no tenemos a nadie a nuestro alrededor
					
					int numtotkni = nb_knights()*nb_players();
					if( size <= numtotkni*0.5  ) {
						if(u.health >= knights_health()*0.7) {
							dijkstra_kinght(c,d,p,caminos,numatacants);	
							//Tratamos la informacion que nos da el dijkstra
							accion_kinght(p,c,k[i],caminos,futurolibre,futuroocupado);	
						
						}	
					}
					else {
						if(u.health >= knights_health()*0.5) {
							dijkstra_kinght(c,d,p,caminos,numatacants);	
							//Tratamos la informacion que nos da el dijkstra
							accion_kinght(p,c,k[i],caminos,futurolibre,futuroocupado);	
						
						}
					}
						
				}
				else {
					futurolibre[c] = true;
					int direc = pr.second.punt;
					command(k[i], Dir(direc));
				}
			
		}
	}
	
	void accion(MatriuInt &p,int &c,int &farm,priority_queue<CaminoInt,vector<CaminoInt>,compareCaminoPosi> caminos) {
		pair<bool,vector<int> > pi;
		vector<Relacio> adj;
		pi.first = false;		
		CaminoInt desti;
		while(not caminos.empty() and not pi.first) { //caminos
			desti = caminos.top();caminos.pop();
			int num = desti.nodo;
			int size = p[num].size();
			int cont = 0;
			vector<int> nocami;
			u_adjacentesInt_nocami(c,nocami);
			pi = camino(p,c,num,nocami);
		}
	
		int direc;
		if(pi.first) {
			u_adjacentes(c,adj);
			int cont = 0;
			int sizeret = adj.size();
			//conseguimos el adjacente sin comprovar que este vacio
			//giramos sobre nosotros mismo
			bool trobat = false;
			while(cont < sizeret and not trobat) {
				//if(pi.second.size() != 0) {
					if(adj[cont].num == pi.second[0]) {
						direc = adj[cont].punt;
						trobat = true;	
					}
					++cont;
			}
		}
		else {	
			direc = 0;
		}	
		command(farm, Dir(direc));	
	}

	void accion_kinght(MatriuInt &p,int &c,int &farm,priority_queue<CaminoInt,vector<CaminoInt>,compareCaminoPosi> caminos,vector<bool> &futurolibre,vector<bool> &futuroocupado) {
		pair<bool,vector<int> > pi;
		vector<Relacio> adj;
		pi.first = false;		
		CaminoInt desti;
		while(not caminos.empty() and not pi.first) { //caminos
			desti = caminos.top();caminos.pop();
			int num = desti.nodo;
			int size = p[num].size();
			int cont = 0;
			vector<int> nocami;
			u_adjacentesInt_nocami_kinght(c,nocami,futurolibre,futuroocupado);
			pi = camino(p,c,num,nocami);
		}
	
		int direc;
		if(pi.first) {
			u_adjacentes(c,adj);
			int cont = 0;
			int sizeret = adj.size();
			//conseguimos el adjacente sin comprovar que este vacio
			//giramos sobre nosotros mismo
			bool trobat = false;
			while(cont < sizeret and not trobat) {
				//if(pi.second.size() != 0) {
					if(adj[cont].num == pi.second[0]) {
						direc = adj[cont].punt;
						futuroocupado[adj[cont].num] = true;
						trobat = true;	
					}
					++cont;
			}
			futurolibre[c] = true;
			command(farm, Dir(direc));
		}		
	}


	pair<bool,Relacio> scan_ratio1(vector<Relacio> &adj,Unit &uni,vector<bool> &atacoya,vector<bool> &futurolibre,vector<bool> &futuroocupado) {
		pair<bool,Relacio> pr;
		pr.first = false;
		vector<Relacio> ratio_farm,ratio_kinght,ratio_libre;
		Cell cela;
		pair<int,int> ij;
		Relacio r;
		for(int i = 0; i < adj.size(); ++i) {
			ij = num_ij(adj[i].num);
			cela = cell(ij.first,ij.second);
			if(cela.unit != -1) {
				Unit u = unit(cela.unit);
				if(u.player != me()) {
					if(u.type == Knight and not atacoya[adj[i].num]) ratio_kinght.push_back(adj[i]);
					else if(u.type == Farmer and not atacoya[adj[i].num]) ratio_farm.push_back(adj[i]);				
				}
			}
			else {
				bool posible = true;
				for(int j = 0; j < futuroocupado.size(); ++j) {
					if(adj[i].num == futuroocupado[j]) posible = false;
				}		
				if(posible) ratio_libre.push_back(adj[i]);
			}
		}	
		//tratamos la info. para decidir que hacer
		if(ratio_kinght.size() != 0 or ratio_farm.size() != 0) {
			if(uni.health > knights_health()*0.3) { //miramos primero en atacar a los kinght
				if(ratio_kinght.size() != 0) {
					//buscamos el de menor tamaño
					r = ratio_kinght[0];
					pair<int,int> ij1 = num_ij(r.num);
					cela = cell(ij1.first,ij1.second);
					Unit u1 = unit(cela.unit);
					int vida = u1.health;
					for(int i = 1; i < ratio_kinght.size(); ++i) {
						pair<int,int> ij2 = num_ij(ratio_kinght[i].num);
						cela = cell(ij2.first,ij2.second);
						Unit u2 = unit(cela.unit);
						if(vida > u2.health) {
							r = adj[i];
							atacoya[adj[i].num] = true;
							vida = u2.health;					
						}		
					}	
					
					pr.first = true;
					pr.second  = r;

				}	
				if(not pr.first) {
					pr.first = true;
					pr.second = ratio_farm[0];
				}
			
			}
			else {
				if(ratio_farm.size() != 0) {
					pr.first = true;
					pr.second = ratio_farm[0];
				}
				if(not pr.first) {
					//buscamos el de menor tamaño
					r = ratio_kinght[0];
					pair<int,int> ij1 = num_ij(r.num);
					cela = cell(ij1.first,ij1.second);
					Unit u1 = unit(cela.unit);
					int vida = u1.health;
					for(int i = 1; i < ratio_kinght.size(); ++i) {
						pair<int,int> ij2 = num_ij(ratio_kinght[i].num);
						cela = cell(ij2.first,ij2.second);
						Unit u2 = unit(cela.unit);
						if(vida > u2.health) {
							r = adj[i];
							vida = u2.health;					
						}		
					}	
					
					pr.first = true;
					pr.second  = r;
				}	
			}	
		}
		return pr;
	}

	void u_adjacentesInt_nocami(int &num,vector<int> &adj) {
			Pos u;
			pair<int,int> ij = num_ij(num);
			u.i = ij.first; u.j = ij.second;
			
			Relacio r;
			Pos pos = dest(u,Top);//cojemos la posicion de arriba
			if(pos_ok(pos) and cell(pos).type != Wall and cell(pos).unit != -1) { //comprovamos que sea valida
				adj.push_back(ij_num(pos.i,pos.j));	
			}
			pos = dest(u,Right);//cojemos la posicion de derecha
			if(pos_ok(pos) and cell(pos).type != Wall and cell(pos).unit != -1) { //comprovamos que sea valida
				adj.push_back(ij_num(pos.i,pos.j));	
			}
			pos = dest(u,Bottom);//cojemos la posicion de abajo
			if(pos_ok(pos) and cell(pos).type != Wall and cell(pos).unit != -1) { //comprovamos que sea valida
				adj.push_back(ij_num(pos.i,pos.j));		
			}
			pos = dest(u,Left);//cojemos la posicion de izquierda
			if(pos_ok(pos) and cell(pos).type != Wall and cell(pos).unit != -1) { //comprovamos que sea valida
				adj.push_back(ij_num(pos.i,pos.j));	
			}	
	}

	void u_adjacentesInt_nocami_kinght(int &num,vector<int> &adj,vector<bool> &futurolibre,vector<bool> &futuroocupado) {
			Pos u;
			pair<int,int> ij = num_ij(num);
			u.i = ij.first; u.j = ij.second;
			
			Relacio r;
			Pos pos = dest(u,Top);//cojemos la posicion de arriba
			if(pos_ok(pos) and cell(pos).type != Wall ) { //comprovamos que sea valida
				int comp = ij_num(pos.i,pos.j);
				if(cell(pos).unit != -1 and unit(cell(pos).unit).player == me()) {
					if(not futurolibre[comp]) adj.push_back(ij_num(pos.i,pos.j));	
				}
				else {
					if(futuroocupado[comp])	adj.push_back(ij_num(pos.i,pos.j));	
				}
				
			}
			pos = dest(u,Right);//cojemos la posicion de derecha
			if(pos_ok(pos) and cell(pos).type != Wall and cell(pos).unit != -1) { //comprovamos que sea valida
				int comp = ij_num(pos.i,pos.j);
				if(cell(pos).unit != -1 and unit(cell(pos).unit).player == me()) {
					if(not futurolibre[comp]) adj.push_back(ij_num(pos.i,pos.j));	
				}
				else {
					if(futuroocupado[comp])	adj.push_back(ij_num(pos.i,pos.j));	
				}	
			}
			pos = dest(u,Bottom);//cojemos la posicion de abajo
			if(pos_ok(pos) and cell(pos).type != Wall and cell(pos).unit != -1) { //comprovamos que sea valida
				int comp = ij_num(pos.i,pos.j);
				if(cell(pos).unit != -1 and unit(cell(pos).unit).player == me()) {
					if(not futurolibre[comp]) adj.push_back(ij_num(pos.i,pos.j));	
				}
				else {
					if(futuroocupado[comp])	adj.push_back(ij_num(pos.i,pos.j));	
				}	
			}
			pos = dest(u,Left);//cojemos la posicion de izquierda
			if(pos_ok(pos) and cell(pos).type != Wall and cell(pos).unit != -1) { //comprovamos que sea valida
				int comp = ij_num(pos.i,pos.j);
				if(cell(pos).unit != -1 and unit(cell(pos).unit).player == me()) {
					if(not futurolibre[comp]) adj.push_back(ij_num(pos.i,pos.j));	
				}
				else {
					if(futuroocupado[comp])	adj.push_back(ij_num(pos.i,pos.j));	
				}
			}	
	}
		
	bool pos_valida(vector<int> &nocami,int num) {
		//cout << "before pos_ok: " << endl;
		int i = 0;
		int size = nocami.size(); //para quitar warning
		while(i < size) {
			if(num == nocami[i]) return false;	
			++i;
		}
		//cout << "after pos_ok: " << endl;
		return true;
	}
	////////////// Version 2 se le añaden condiciones en el camino
	void camino_i(MatriuInt &p,int &inici,int x,vector<int> &recorrido,bool &ok,vector<int> &nocami) {
		if(x == inici) ok = true;
		else if(p[x].size() != 0) {
			int i = 0;
			while(i < p[x].size() and not ok) {
				if(pos_valida(nocami,p[x][i]))camino_i(p,inici,p[x][i],recorrido,ok,nocami);
				++i;
			}
			if(ok) recorrido.push_back(x);
		}
		else ok = false;	
	}


	pair<bool,vector<int> > camino(MatriuInt& p,int x,int y,vector<int> &nocami) {
		vector<int> recorrido;
		bool ok = false;
		pair<bool,vector<int> > pi;
		if(p[y].size() != 0) {
			int i = 0;
			
			//recorrido.push_back(x);//si quito esta linea me queda al pelo para el Battel Royal
			while(i < p[y].size() and not ok) {
					if(pos_valida(nocami,p[y][i])) camino_i(p,x,p[y][i],recorrido,ok,nocami);
					++i;
			}
			if(ok) recorrido.push_back(y);
			
		}
		pi.first = ok;
		pi.second = recorrido;
		return pi;
	}	
	
	//Para el nodo=num obtenemos todas sus adjacencias y su direccion
	void u_adjacentes(int &num,vector<Relacio> &adj) {
			Pos u;
			pair<int,int> ij = num_ij(num);
			u.i = ij.first; u.j = ij.second;
			
			Relacio r;
			Pos pos = dest(u,Top);//cojemos la posicion de arriba
			if(pos_ok(pos) and cell(pos).type != Wall) { //comprovamos que sea valida
				r.num = ij_num(pos.i,pos.j);
				r.punt = Top;
				adj.push_back(r);	
			}
			pos = dest(u,Right);//cojemos la posicion de derecha
			if(pos_ok(pos) and cell(pos).type != Wall) { //comprovamos que sea valida
				r.num = ij_num(pos.i,pos.j);
				r.punt = Right;
				adj.push_back(r);	
			}
			pos = dest(u,Bottom);//cojemos la posicion de abajo
			if(pos_ok(pos) and cell(pos).type != Wall) { //comprovamos que sea valida
				r.num = ij_num(pos.i,pos.j);
				r.punt = Bottom;
				adj.push_back(r);	
			}
			pos = dest(u,Left);//cojemos la posicion de izquierda
			if(pos_ok(pos) and cell(pos).type != Wall) { //comprovamos que sea valida
				r.num = ij_num(pos.i,pos.j);
				r.punt = Left;
				adj.push_back(r);	
			}	
	}
	
	//de cordenadas i,j --> int 0 <= s < board.size()
	int ij_num(int i, int j) {
		return (rows()*i)+j;
	}
	
	// de un int 0 <= s < board.size() --> cordenadas i,j
	pair<int,int> num_ij(int num) {//first = i, second = j
		pair<int,int> p;
		int m = rows();
		p.second = (num)%m;
		p.first = ((num-p.second)/m);
		return p;
	}
	
	/*
		En la funcion dijkstra falta poner las condiciones de si dentro del ratio no hay caminos aumentar el ratio
	*/
	void dijkstra_farm(int s, vector<double>& d, MatriuInt& p,priority_queue<CaminoInt,vector<CaminoInt>,compareCaminoPosi> &caminos,priority_queue<CaminoInt,vector<CaminoInt>,compareCaminoPosi> &danger,vector<int> &atencion, vector<int> &peligro) {
		int n = rows()*cols();
		d = vector<double>(n, infinite); d[s] = 0;
		MatriuInt ret(n,vector<int>(0));
		p = ret;
		vector<bool> S(n, false);
		priority_queue<ArcI, vector<ArcI>, greater<ArcI> > Q;
		Q.push(ArcI(0, s));
		
		pair<int,int> tunit = num_ij(s);
		Cell cunit = cell(tunit.first,tunit.second);
		Unit iniciunid = unit(cunit.unit);		

		while (not Q.empty() and caminos.size() < 3) {
			int u = Q.top().second; Q.pop();
			if (not S[u]) {
				S[u] = true;
				vector<Relacio> adj;
				u_adjacentes(u,adj);
				
				//para los vectores caminos,danger
				//comprovamos si la casilla nos pertenece
				pair<int,int> t = num_ij(u);
				Cell c = cell(t.first,t.second);
				//si queremos solos atencion[u] < 2 en parejas atencion[u] < 3
				if(c.owner != me() and c.unit == -1 and atencion[u] < 3) {// obtenemos los caminos posibles a conquistar, 
					++atencion[u];
					CaminoInt c;
					c.dist = d[u];
					c.nodo = u;
					caminos.push(c);
				}				
				
				//comprovamos los enemigos que tenemos detro de nuestro ratio
				//comprovamos los peligros dentro del ratio
				if(c.unit != -1) {
					Unit unid = unit(c.unit);
					if(unid.type == Knight and unid.player != me()) {
						CaminoInt c;
						c.dist = d[u];
						c.nodo = u;
						danger.push(c);	
					}
				}	
									
				for (int i = 0; i < adj.size(); ++i) {
					int v = adj[i].num;
					//la segunda parte del or es para los farms que tenga vida inferior al 20% que se la juegen en pasar en zona peligrosa(SUIZIDA)
					if(peligro[v] == 0 or (peligro[v] == 1 and iniciunid.health < farmers_health()*0.2) ) {
						if (d[v] > d[u] + 1) {
							d[v] = d[u] + 1;
							p[v] = vector<int>(0);
							p[v].push_back(u);
							Q.push(ArcI(d[v], v));
						}
						else if(d[v] == d[u] + 1) p[v].push_back(u);
					}
				}
			}
		}
	}
	/*
	Info que obtenemos del dijkstra:
		·Recorrido en amplada dentro de un ratio max;
		·vector de distancias para cada nodo desde el nodo u
		·MatriuRel que contien los posibles caminos posteriores posibles de la misma distancia
		·vector con todos los caminos posibles dentro del ratio (distancia,nodo)
	*/
	
	void dijkstra_kinght(int s, vector<double>& d, MatriuInt& p,priority_queue<CaminoInt,vector<CaminoInt>,compareCaminoPosi> &presas,vector<int> &numatacants) {
		int n = rows()*cols();
		d = vector<double>(n, infinite); d[s] = 0;
		MatriuInt ret(n,vector<int>(0));
		p = ret;
		vector<bool> S(n, false);
		priority_queue<ArcI, vector<ArcI>, greater<ArcI> > Q;
		Q.push(ArcI(0, s));
		while (not Q.empty() and presas.size() < 3) {
			int u = Q.top().second; Q.pop();
			if (not S[u]) {
				S[u] = true;
				
				vector<Relacio> adj;
				u_adjacentes(u,adj);
				
				//para los vectores caminos,danger
				//comprovamos si la casilla nos pertenece
				pair<int,int> t = num_ij(u);
				Cell c = cell(t.first,t.second);
				
				//comprovamos los enemigos que tenemos detro de nuestro ratio
				//comprovamos los peligros dentro del ratio
				if(c.unit != -1) {
					Unit unid = unit(c.unit);
					if(knights(me()).size() < nb_units()*0.4) {
						if(unid.player != me() and numatacants[unid.id] < 5) {
							++numatacants[unid.id];
							CaminoInt c;
							c.dist = d[u];
							c.nodo = u;
							presas.push(c);	
						}					
					}
					else if(knights(me()).size() < nb_units()*0.6) {
						if(unid.player != me() and numatacants[unid.id] < 10) {//cuando pasamos a tener el 50% de las unidades de la partida
							++numatacants[unid.id];
							CaminoInt c;
							c.dist = d[u];
							c.nodo = u;
							presas.push(c);	
						}
					}
					else {
						if(unid.player != me()) {//cuando pasamos a tener el 50% de las unidades de la partida
							++numatacants[unid.id];
							CaminoInt c;
							c.dist = d[u];
							c.nodo = u;
							presas.push(c);	
						}					
					}
						
				}
									
				for (int i = 0; i < adj.size(); ++i) {
					int v = adj[i].num;
					if (d[v] > d[u] + 1) {
						d[v] = d[u] + 1;
						p[v] = vector<int>(0);
						p[v].push_back(u);
						Q.push(ArcI(d[v], v));
					}
					else if(d[v] == d[u] + 1) p[v].push_back(u);
				}
			}
		}
		/*for(int i = 0; i < 40; ++i) cout << d[i] << " ";
		cout << endl;
		for(int i = 0; i < 40; ++i) cout << p[i].size() << " ";
		cout << endl;*/
	}

	/*
	Info que obtenemos del dijkstra:
		·Recorrido en amplada dentro de un ratio max;
		·vector de distancias para cada nodo desde el nodo u
		·MatriuRel que contien los posibles caminos posteriores posibles de la misma distancia
		·vector con todos los caminos posibles dentro del ratio (distancia,nodo)
		·vector con todos los peligros dentro del ratio (distancia,nodo)
	*/
	
	typedef vector<vector<Dir> > DirVal;
	
	pair<int,int> board_pos_inici() {
		vector<int> f = farmers(me());
		int n = f.size();
		
		//cojemos el valor de todas las i,j para para quadrar entre (imin,jmin) y (imax,jmax)
		Pos pmin,pmax;
		int imin,jmin;
		int imax,jmax;
		imin = jmin = infinite;
		imax = jmax = -1;
		int size = f.size();
		for(int i = 1; i < size; ++i) {
			Unit u = unit(f[i]);
			if(u.pos.i < imin) imin = u.pos.i;
			if(u.pos.j < jmin) jmin = u.pos.j;
			if(u.pos.i > imax) imax = u.pos.i;
			if(u.pos.j > jmax) jmax = u.pos.j;
		}
		
		int rowm = rows()/2;
		int colm = cols()/2;
		int pos = 0;
		if(imin < rowm and imax < rowm and jmin < colm and jmax < colm) pos = 0;	
		if(imin < rowm and imax < rowm and jmin > colm and jmax > colm) pos = 1;
		if(imin > rowm and imax > rowm and jmin < colm and jmax < colm) pos = 2;
		if(imin > rowm and imax > rowm and jmin > colm and jmax > colm) pos = 3;
		pair<int,int> coords;//first= i,second = j
		switch(pos) {
			case 0:
				coords.first = rowm/2;
				coords.second = colm/2;
				break;
			case 1:
				coords.first = rowm/2;
				coords.second = colm + (colm/2);
				break;
			case 2:
				coords.first = rowm + (rowm/2);
				coords.second = colm/2;
				break;
			case 3:
				coords.first = rowm + (rowm/2);
				coords.second = colm + (colm/2);
				break;		
		}		
		return coords;
	}	
	
	void action_unit_center() {
		vector<int> f = farmers(me());
		int n = f.size();
		DirVal dirval(n,vector<Dir>(0));
		//cojemos el valor de todas las i,j para para quadrar entre (imin,jmin) y (imax,jmax)
		pair<int,int> coords = board_pos_inici();
		int coordi = coords.first;
		int coordj = coords.second;
		int size = f.size();
		for(int i = 0; i < size; ++i) {
			Unit u = unit(f[i]);
			if(u.pos.i-1 < coordi and u.pos.j-1 < coordj){ //direccion bottom,right;
				dirval[i].push_back(Bottom); dirval[i].push_back(Right);
			}
			if(u.pos.i+1 > coordi and u.pos.j-1 < coordj) {//direccion up,right;
				dirval[i].push_back(Top);dirval[i].push_back(Right);
			}
			if(u.pos.i-1 < coordi and u.pos.j+1 > coordj) {//direccion bottom,left;
				dirval[i].push_back(Bottom);dirval[i].push_back(Left);
			}			
			if(u.pos.i+1 > coordi and u.pos.j+1 > coordj) {//direccion up,left;
				dirval[i].push_back(Top);dirval[i].push_back(Left);		
			}
			if(u.pos.i == coordi and u.pos.j == coordj) {//direccion random;
				vector<Dir> ran(4);
				ran[0] = Bottom; ran[1] = Right;  ran[2] = Left; ran[3] = Top;
				int r = random()%4;					
				dirval[i].push_back(ran[r]);
			}
		}
		
		for(int i = 1; i < size; ++i) {
			if(dirval[i].size() != 0) {
				int r = random()%dirval[i].size();
				command(f[i], Dir(dirval[i][r]));
			}
			else command(f[i], Dir(0));
				
		}
	}	
	
	
    /**
     * Play method.
     * 
     * This method will be invoked once per each round.
     * You have to read the board here to place your actions
     * for this round.
     *
     * In this example, each unit moves in a random direction
     */     
    
	//recorremos el vector de unidades para obtener todos los knight para marcar sus zonas de peligro
	vector<int> zonas_peligro() {
		vector<int> peligro(rows()*cols(),0);
		for(int i = 0; i < nb_units(); ++i) {
			Unit u = unit(i);
			if(u.player != me() and u.type == Knight) {
				int p = ij_num(u.pos.i,u.pos.j);
				++peligro[p];
				//obtenemos sus adjacencias
				Pos pos = dest(u.pos,Top);
				if(pos_ok(pos) and cell(pos).type != Wall) {
					p = ij_num(pos.i,pos.j);
					++peligro[p];			
				} 	
				pos = dest(u.pos,Right);
				if(pos_ok(pos) and cell(pos).type != Wall) {
					p = ij_num(pos.i,pos.j);
					++peligro[p];			
				} 	
				pos = dest(u.pos,Bottom);
				if(pos_ok(pos) and cell(pos).type != Wall) {
					p = ij_num(pos.i,pos.j);
					++peligro[p];			
				} 	
				pos = dest(u.pos,Left);
				if(pos_ok(pos) and cell(pos).type != Wall) {
					p = ij_num(pos.i,pos.j);
					++peligro[p];			
				} 		
			}		
		}
		return peligro;
	}
	
    void play () {
	
		/*
		puedo pasar el vector de caminos y peligros de los farms y tratarlos fuera
		con el vector de presas del knight
		*/
		vector<int> peligro = zonas_peligro();
		//action_Farmers(); 
		action_kinght();
		//action_Farmers(peligro); 
		if(round()>4)action_Farmers(peligro); 
		else action_unit_center();
		
        
    }

};



/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);

