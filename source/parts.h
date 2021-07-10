#include<iostream>
#include<stdlib.h>
#include <cmath>
class Mazegrid {
	public:
		int broken;     //Used to check if we broke it
		bool avaipath[4];   //Is there a way to move to up,down,left,right
		Mazegrid(){
			broken=0;
			for(int i=0;i<4;i++)
				avaipath[i]=false;   //Initially there are walls all 4 sides  //0 Down 1 Up 2 Left 3 Right
		}
};
class Player {
	public:
		int xcor,ycor; //Coordinates in the previous mentioned way 
		int tomove; // Destination or used in moving like left right etc
		Player(int xpos,int ypos,int size){  //Generating Ball at initial position
			tomove=2;
			xcor=20.0 + 10.0* xpos;
			ycor=20.0 + 10.0*ypos;
		}
};