#include "Labyrinthe.h"
#include "Chasseur.h"
#include "Gardien.h"
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <ctime>

using namespace std;
#define PI 3.14159265

/**
 * This file contians functions about hunters. These functions is responsible for
 * how the hunter walks, how he fires and how the fireball flies in diffrent situations.
 *
 * The system will automatically call all functions in this file,
 * so that the state of the hunter and fireball will be processed at every moment.
 *
 * bool Chasseur::move_aux (double dx, double dy):
 *
 * 		This function completes a movement of Hunter.
 *
 * 		If hunter walks over a grid where the portal is located, there will be a message
 * 		displayed in the upper left corner. And he can completes teleportation after
 * 		keeping running sur the portals.
 *
 * 		If hunter walks over a grid where there is a medical kit aroud box, then he will
 * 		get 20 hp.
 *
 * bool Chasseur::process_fireball (float dx, float dy):
 *		This function handles the different situations of the fireball during the flight.
 *
 *		If there is nothing in the direction of the flight of fireball, it keeps flying.
 *		If fireball hits a guardian, the HP value of guardian will decreases. Puissance of
 *		fireball increases as distance between hunter and guardian increases.
 *		If fireball missed guardian, but the distance between fireball and guardian is
 *		less than dis1, then guardian will move to the direction where fireball comes from.
 *		When the fireball is going to hit the guard, the guard will dodge randomly in a short distance.
 *
 * void Chasseur::fire (int angle_vertical)
 * 		Initialize the angle of the fireball
 * 		Hunter fires, but fire angle drift when HP goes down.
**/

/**
 * @Author      JiaxuanLIU and JiangnanHUANG
 * @Description Completing a movement of Hunter.
 * 				If there is a portals, hunter will complete teleportation after running sur place for a period of time.
 * 				If the grid is aroud a box, then HP value of hunter augments
 * @DateTime    2020-05-08T15:25:15+0100
 * @param		dx: distance that hunter moves in x direction
 * @param		dy: distance that hunter moves in x direction
 * @return		bool: if hunter moving in direction dx,dy successfully
 */

bool Chasseur::move_aux (double dx, double dy)
{
	int pos_x = (int)(_x / Environnement::scale);
	int pos_y = (int)(_y / Environnement::scale);
	int pos_tp = 0;

	int val_over_tp = ((Labyrinthe *)(_l)) -> get_over_tp(pos_x,pos_y);

	// Teleportation
	if ( -1 !=  val_over_tp )
	{
		message("You have fund a secret, keep running at the same place");
		_wait_to_transmision = _wait_to_transmision-1;
		cout << "_wait_to_transmision" << _wait_to_transmision << endl;

		if (_wait_to_transmision == 0)
		{
			_wait_to_transmision = 150;
			pos_tp = ((Labyrinthe *)(_l)) -> is_exist_in_teleprotation(val_over_tp);
			if (pos_tp != INFINI)
			{
				std::pair<int,int> pos_portal_xy = ((Labyrinthe *)(_l)) -> get_the_other_portal(pos_tp, pos_x, pos_y);

				int pos_portal_x = (int)(pos_portal_xy.first);
				int pos_portal_y = (int)(pos_portal_xy.second);
				_x = pos_portal_x * Environnement::scale;
				_y = pos_portal_y * Environnement::scale;

				message("You entered a new place");

				return true;

			}
		}
	} else {
		message(" ");
	}

	//Eat Medical kit.
	if(((Labyrinthe *)(_l)) -> around_boxs(pos_x,pos_y) == 1)
	{
		  if (lives >= 80) {
				lives = 100;
			} else {
				lives += 20;
			}

			((Labyrinthe *)(_l)) -> set_around_boxs(pos_x,pos_y);
			message ("Get 20 hp! %d", (int) lives);
	}
	if (EMPTY == _l -> data ((int)((_x + dx) / Environnement::scale),
							 (int)((_y + dy) / Environnement::scale)))
	{
		_x += dx;
		_y += dy;
		return true;
	}
	return false;
}

/*
 *	Constructeur.
 */

Chasseur::Chasseur (Labyrinthe* l) : Mover (100, 80, l, 0)
{
	_hunter_fire = new Sound ("sons/hunter_fire.wav");
	_hunter_hit = new Sound ("sons/hunter_hit.wav");
	if (_wall_hit == 0)
		_wall_hit = new Sound ("sons/hit_wall.wav");
}


/**
 * @Author      JiaxuanLIU and JiangnanHUANG
 * @Description This fuction  handles the different situations of the fireball during the flight:
 * 				fireball hits guardian, hits walls, hits treasure or misses guardian or there is noting.
 * @DateTime    2020-05-08T15:44:34+0100
 */

bool Chasseur::process_fireball (float dx, float dy)
{

	// calculer la distance entre le chasseur et le lieu de l'explosion.
	float	x = (_x - _fb -> get_x ()) / Environnement::scale;
	float	y = (_y - _fb -> get_y ()) / Environnement::scale;
	float	dist2 = x*x + y*y;

	// calculer la distance maximum en ligne droite.
	float	dmax2 = (_l -> width ())*(_l -> width ()) + (_l -> height ())*(_l -> height ());

	int pos_of_fb_x = (int)((_fb -> get_x () + dx) / Environnement::scale) ;
	int pos_of_fb_y = (int)((_fb -> get_y () + dy) / Environnement::scale);


	// on bouge que dans le vide!
	for (int i = 1; i < _l -> _nguards; ++i) {
		int pos_of_guard_x = (int)((_l -> _guards [i] -> _x) / Environnement::scale ) ;
		int pos_of_guard_y = (int)((_l -> _guards [i] -> _y) / Environnement::scale);

		// tester la collision avec un gardient.
		if( pos_of_guard_x ==  pos_of_fb_x && pos_of_guard_y == pos_of_fb_y)
		{

			((Gardien *)(_l ->  _guards [i])) -> _guard_hit -> play (1. - dist2/dmax2);
			if (!(((Gardien *)(_l ->  _guards [i])) -> is_dead())) {

				//Puissance d'attaque augmente si la distance entre chasseur et gardien diminue.

				((Gardien *)(_l ->  _guards [i])) -> hit(( 1 - (((Gardien *)(_l ->  _guards [i])) -> get_distance_to_chasseur())/1000) * puissance_attaque);
				message ("Good shoot.");

				if (((Gardien *)(_l ->  _guards [i])) -> get_lives() <= 0) {
					message ("Kill one.");
					((Gardien *)(_l ->  _guards [i])) -> dead();
					((Gardien *)(_l ->  _guards [i])) -> rester_au_sol();
				}
			}

			return false;

		}
		
		//If distance of fireball and guardian <= 10, then guardian will notice this attack.
		//And then he will forwards to the direction where the attack comes from, even if he does't see hunter.

		if ( abs(pos_of_guard_x - pos_of_fb_x) + abs(pos_of_guard_y - pos_of_fb_y) <= 10)
		{
			if (!(((Gardien *)(_l ->  _guards [i])) -> is_dead()))
			{
				((Gardien*)(_l -> _guards[i])) -> change_to_mode_3();
			}
		}

		// If distance of fireball and guardian <= 5, then he will randomly dodge in acertain direction in a small step.
		if (abs(pos_of_guard_x - pos_of_fb_x) + abs(pos_of_guard_y - pos_of_fb_y) <= 5)
		{
			if (!(((Gardien *)(_l ->  _guards [i])) -> is_dead()))
			{
			srand((int) time(NULL));
			float step_x = - 5*sin( ( ((Gardien*)(_l -> _guards[i])) -> _angle+ (rand() % 361) ) * PI / 180);    // you can change the speed of dodging bullets
			float step_y = 5 *cos(  ( ((Gardien*)(_l -> _guards[i])) -> _angle+ (rand() % 361) ) * PI / 180);

			((Gardien *)(_l ->  _guards [i])) -> move(step_x, step_y);
			}
		}
	}


	if (EMPTY == _l -> data ((int)((_fb -> get_x () + dx) / Environnement::scale),
							 (int)((_fb -> get_y () + dy) / Environnement::scale)))
	{
		message ("Woooshh ..... %d", (int) dist2);
		// il y a la place.
		return true;
	}

	// collision...

	// faire exploser la boule de feu avec un bruit fonction de la distance.
	_wall_hit -> play (1. - dist2/dmax2);
	message ("Booom...");

	// teste si on a touché le trésor: juste pour montrer un exemple de la
	// fonction < partie_terminee >.
	if ((int)((_fb -> get_x () + dx) / Environnement::scale) == _l -> _treasor._x &&
		(int)((_fb -> get_y () + dy) / Environnement::scale) == _l -> _treasor._y)
	{
		partie_terminee (true);
	}
	return false;
}


/**
 * @Author      JiaxuanLIU and JiangnanHUANG
 * @Description Initialize the angle of the fireball
 * 				Hunter fires, but fire angle drift when HP goes down.
 */

void Chasseur::fire (int angle_vertical)
{
	message ("Woooshh...");

	int vertical_angle_drift = (float)(rand()%(101 - lives) - (101 - lives)/2)/100 * 20;
	int horizon_angle_drift = (float)(rand()%(101 - lives) - (101 - lives)/2)/100 * 30;

	_hunter_fire -> play ();
	_fb -> init (/* position initiale de la boule */ _x, _y, 10.,
				 /* angles de visée */ angle_vertical + vertical_angle_drift, _angle + horizon_angle_drift);
}


// /*
//  *	Clic droit: par défaut fait tomber le premier gardien.
//  *
//  *	Inutile dans le vrai jeu, mais c'est juste pour montrer
//  *	une utilisation des fonctions < tomber > et < rester_au_sol >
//  */
//  void Chasseur::right_click (bool shift, bool control) {                       // this funtion is not used(old version of Teleportation)

//  	if (shift) {
//  		cout<<"hahaha"<<endl;
//  	} else {

//  		int x_tele = 0;
//  		int y_tele = 0;

//  		while (true) {

//  			srand((unsigned int)(time(NULL)));
//  			x_tele = rand()%(_l -> width ());
//  			y_tele = rand()%(_l -> height ());

//  			if (EMPTY == _l -> data (x_tele, y_tele) && ((Labyrinthe *)(_l)) -> distance_to_tresor (x_tele, y_tele) > 20) {
//  				break;
//  			}
//  		}

//  		_x = x_tele * Environnement::scale;
//  		_y = y_tele * Environnement::scale;
//  	}
//  }
