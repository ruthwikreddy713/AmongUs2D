#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <shader.h>
#include <iostream>
#include <parts.h>
#include <ctime>
#include <cstdlib>
#include <cmath>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#include "glm/ext.hpp"
#include <vector>
#include <queue>
int startx,starty,endx,endy;
int size,sizeofeachgrid;
int impbuttonx,impbuttony;
Mazegrid * Maze;
int *breaklines;
int countdown=100;
int countdowntime=0;
int taskbuttonx;
int taskbuttony;
int tasksdone=0;
int wmovementsmooth=0;
int smovementsmooth=0;
bool impdead=false;
bool taskact=false;
int amovementsmooth=0;
int dmovementsmooth=0;
int playerx,playery;  
int tasks=2;
int impostorx,impostory;
bool taskbuttonactiv=true;
std::vector <int> Adj[225]; //Adjacency list for graph Assuming 15*15 grid
bool impbuttonactiv=true;
int health = 5;
int score = 0;
int mazepathupdated=0;
int lastinp=-1;
float lastimpmov=0.0f;
int predecessor[225];   //For BFS path 15*15 grid :p
struct powerupsandob
{
	bool active;
	int xcor;
	int ycor;
	bool type;
};
//This returns the immediate best step
int BFSshortpath(int source,int destn){
	std::queue <int> bfsqueue;
	bool Vis[size*size];  //Assuming 15*15 grid for Adj list
	for(int i=0;i<size*size;i++){
		predecessor[i]=-1;
		Vis[i]=false;
	}
	bfsqueue.push(source);
	Vis[source]=true;
	while(!bfsqueue.empty()){
		int curr = bfsqueue.front();
		bfsqueue.pop();
		for(int i=0;i<Adj[curr].size();i++){
			if(Vis[Adj[curr][i]]==false){
				Vis[Adj[curr][i]]=true;
				predecessor[Adj[curr][i]]=curr;
				bfsqueue.push(Adj[curr][i]);
				if(Adj[curr][i]==destn){
					int op=destn;
					while(predecessor[op]!=source){
						op=predecessor[op];
						if(predecessor[op]==source){
							return op;
						}
					}
					return destn;
				}
			}
		}
	}
}
struct powerupsandob pupsandob[15];
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 900;
float deltaTime=0.0f;
float lastFrame=0.0f;
void ChooseStartandEnd(){
	startx=size-1;
	starty=rand()%size;
	Maze[size*starty+startx].avaipath[3]=true; //Opening the right side wall so  we can start there
	endx = 0;
	endy=rand()%size;
	Maze[size*endy+endx].avaipath[2]=true; //Opening Left side of a random element from left side of the maze
	//cout << startx << starty << endl;
	playery=starty;
	playerx=startx;
	impostorx=endx;
	impostory=endy;
	impbuttonx = 6 + rand()%3;
	impbuttony = 6 + rand()%3;
	taskbuttonx = 3+ rand()%3;
	taskbuttony = 3+ rand()%3;
	for(int i=0;i<15;i++){
		pupsandob[i].active=true;
		if(i%2==0){
			pupsandob[i].type=true;
		}
		else{
			pupsandob[i].type=false;
		}
		pupsandob[i].xcor = i;
		pupsandob[i].ycor = rand()%size;
		std::cout << pupsandob[i].xcor << "  " << pupsandob[i].ycor << std::endl;
	}
	//impbuttonx=6;
	//impbuttony=6;
}
void updatingAdjacencylistofgraph(){
	for(int i=0;i<size*size;i++){
		int mazecorx = i % size;
		int mazecory = i/size;
		if(Maze[i].avaipath[0]==true &&mazecory>0){
			Adj[i].push_back(size*(mazecory-1)+mazecorx);
		}
		if(Maze[i].avaipath[1]==true && mazecory<14){
			Adj[i].push_back(size*(mazecory+1)+mazecorx);
		}
		if(Maze[i].avaipath[2]==true && mazecorx>0){
			Adj[i].push_back(size*mazecory+mazecorx-1);
		}
		if(Maze[i].avaipath[3]==true && mazecorx <14){
			Adj[i].push_back(size*mazecory+mazecorx+1);
		}
	}
	for(int i=0;i<size*size;i++){
		std::cout << "Adjacent to  " << i << "Are  " ;
		for(int j=0;j<Adj[i].size();j++){
			std::cout << Adj[i][j] << "  ";
		}
		std::cout << std::endl;
	}
}
void UpdateMazepath(){
	//cout << "Hi" << endl;
	int count = 0;  //Number of walls broken
	while(1 && !mazepathupdated){
		if(count==size*size){
			mazepathupdated=1;
			for(int i=0;i<size*size;i++){
				Maze[i].broken = 0;                 //We no longer nneed if broken property
			}
			break;
		}
		if(count==0){
			ChooseStartandEnd();
			count++;
			//Randomly choosing a block and we start breaking from there
			breaklines = new int [size*size];
			int rx=rand()%size,ry=rand()%size;
			breaklines[0]= size*ry+rx; //Storing the value now we try to break neighbours of this square
			Maze[size*ry+rx].broken=1;
		}
		//Iterating until we break a wall
		bool brokewall=false;
		while(1){
			int randombrokengrid = rand()%count;
			int gridpos=breaklines[randombrokengrid];
			//the position is stored as size*y+x 
			//extracting pos;
			int ycor=gridpos/size;
			int xcor=gridpos%size;
			//Has 4 neighbours randomly choosing one
			int neighpos=rand()%4;
			if(neighpos==0){
				//Neighbor below it
				//Checking if border condition or is broken before
				if(ycor==0 || Maze[size*(ycor-1)+xcor].broken==1){
					continue;
				}
				else{  //Breaking the neighbout
					Maze[size*(ycor-1)+xcor].broken=1;
					breaklines[count++]=size*(ycor-1)+xcor;
					brokewall=true;
					Maze[size*(ycor-1)+xcor].avaipath[1]=true;
					Maze[size*ycor+xcor].avaipath[0]=true;
				}		
			}
			else if(neighpos==1){
				//Neighbor Above it
				//Checking if border condition or is broken before
				if(ycor==size-1 || Maze[size*(ycor+1)+xcor].broken==1){
					continue;
				}
				else{  //Breaking the neighbout
					Maze[size*(ycor+1)+xcor].broken=1;
					breaklines[count++]=size*(ycor+1)+xcor;
					brokewall=true;
					Maze[size*(ycor+1)+xcor].avaipath[0]=true;
					Maze[size*ycor+xcor].avaipath[1]=true;
				}	
			}
			else if(neighpos==2){
				//Neighbor on left chek
				//Checking if border condition or is broken before
				if(xcor==0 || Maze[size*(ycor)+xcor-1].broken==1){
					continue;
				}
				else{  //Breaking the neighbout
					Maze[size*(ycor)+xcor-1].broken=1;
					breaklines[count++]=size*(ycor)+xcor-1;
					brokewall=true;
					Maze[size*(ycor)+xcor-1].avaipath[3]=true;
					Maze[size*ycor+xcor].avaipath[2]=true;
				}	
			}
			else if(neighpos==3){
				//Neighbor on Right chek
				//Checking if border condition or is broken before
				if(xcor==size-1 || Maze[size*(ycor)+xcor+1].broken==1){
					continue;
				}
				else{  //Breaking the neighbout
					Maze[size*(ycor)+xcor+1].broken=1;
					breaklines[count++]=size*(ycor)+xcor+1;
					brokewall=true;
					Maze[size*(ycor)+xcor+1].avaipath[2]=true;
					Maze[size*ycor+xcor].avaipath[3]=true;
				}	
			}
		if(brokewall)
			break;
		}
	}
}
void key_callback(GLFWwindow *window,int key, int scancode, int action, int mods);
int main()
{
	size=15;
	srand(time(NULL));
	sizeofeachgrid=50;
	Maze = new Mazegrid[size*size];
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "A0", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    UpdateMazepath();
    updatingAdjacencylistofgraph();
    Shader newshader(0);
    float hvertices[]={
    	//Triangle1
    	50.0f,50.0f,0.0f,  1.0f,1.0f,1.0f,
    	100.0f,50.0f,0.0f, 1.0f,1.0f,1.0f,
    };
    float vvertices[]={
    	50.0f,50.0f,0.0f,1.0f,1.0f,1.0f,
    	50.0f,100.0f,0.0f,1.0f,1.0f,1.0f,
    };
    float pvertices[6*18 + 33*18 + 6*18 + 18*18];
    int k=0;
    pvertices[k++]=65.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=85.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=80.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    //traingle2
    pvertices[k++]=65.0f;
    pvertices[k++]=80.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=85.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=85.0f;
    pvertices[k++]=80.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    //Semicirle
    for(float ang=0.0;ang<3.3;ang+=0.1){
    	//center
    	pvertices[k++]=75.0f;
    	pvertices[k++]=80.0f;
    	pvertices[k++]=0.0f;
    	//color
    	pvertices[k++]=0.0f;
    	pvertices[k++]=1.0f;
    	pvertices[k++]=1.0f;
    	//Vertex1
    	pvertices[k++]=75.0+10*cos(ang);
    	pvertices[k++]=80.0+10*sin(ang);
    	pvertices[k++]=0.0f;
    	//color
    	pvertices[k++]=0.0f;
    	pvertices[k++]=1.0f;
    	pvertices[k++]=1.0f;
    	//Vertex2
    	pvertices[k++]=75.0+10*cos(ang+0.1);
    	pvertices[k++]=80.0+10*sin(ang+0.1);
    	pvertices[k++]=0.0f;
    	//color
    	pvertices[k++]=0.0f;
    	pvertices[k++]=1.0f;
    	pvertices[k++]=1.0f;    	
    }
    //Spectacles
    pvertices[k++]=65.0f;
    pvertices[k++]=81.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=75.0f;
    pvertices[k++]=81.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=89.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=75.0f;
    pvertices[k++]=89.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=89.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=75.0f;
    pvertices[k++]=81.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    //Backpack
    pvertices[k++]=85.0f;
    pvertices[k++]=68.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.9f;
    pvertices[k++]=0.9f;
    pvertices[k++]=90.0f;
    pvertices[k++]=68.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.9f;
    pvertices[k++]=0.9f;
    pvertices[k++]=85.0f;
    pvertices[k++]=78.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.9f;
    pvertices[k++]=0.9f;
    //traingle2
    pvertices[k++]=85.0f;
    pvertices[k++]=78.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.9f;
    pvertices[k++]=0.9f;
    pvertices[k++]=90.0f;
    pvertices[k++]=68.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.9f;
    pvertices[k++]=0.9f;
    pvertices[k++]=90.0f;
    pvertices[k++]=78.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.9f;
    pvertices[k++]=0.9f;
    //Legs
    pvertices[k++]=65.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=68.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=55.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    //traingle2
    pvertices[k++]=65.0f;
    pvertices[k++]=55.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=68.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=68.0f;
    pvertices[k++]=55.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f; 
    //Leg 2 
    pvertices[k++]=82.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=85.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=82.0f;
    pvertices[k++]=55.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    //traingle2
    pvertices[k++]=82.0f;
    pvertices[k++]=55.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=85.0f;
    pvertices[k++]=65.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=85.0f;
    pvertices[k++]=55.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=0.0f;
    pvertices[k++]=1.0f;
    pvertices[k++]=1.0f; 
    /*float pvertices[]={
    	55.0f,55.0f,0.0f, 0.0f,1.0f,1.0f,
    	55.0f,95.0f,0.0f, 0.0f,1.0f,1.0f,
    	95.0f,55.0f,0.0f, 0.0f,1.0f,1.0f,
    	95.0f,95.0f,0.0f, 0.0f,1.0f,1.0f,
    	95.0f,55.0f,0.0f, 0.0f,1.0f,1.0f,
    	55.0f,95.0f,0.0f, 0.0f,1.0f,1.0f,	
    };*/
    k=0;
    float ivertices[6*18 + 33*18 + 6*18 + 18*18];
    ivertices[k++]=65.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=85.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=80.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    //traingle2
    ivertices[k++]=65.0f;
    ivertices[k++]=80.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=85.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=85.0f;
    ivertices[k++]=80.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    //Semicirle
    for(float ang=0.0;ang<3.3;ang+=0.1){
        //center
        ivertices[k++]=75.0f;
        ivertices[k++]=80.0f;
        ivertices[k++]=0.0f;
        //color
        ivertices[k++]=1.0f;
        ivertices[k++]=0.0f;
        ivertices[k++]=0.0f;
        //Vertex1
        ivertices[k++]=75.0+10*cos(ang);
        ivertices[k++]=80.0+10*sin(ang);
        ivertices[k++]=0.0f;
        //color
        ivertices[k++]=1.0f;
        ivertices[k++]=0.0f;
        ivertices[k++]=0.0f;
        //Vertex2
        ivertices[k++]=75.0+10*cos(ang+0.1);
        ivertices[k++]=80.0+10*sin(ang+0.1);
        ivertices[k++]=0.0f;
        //color
        ivertices[k++]=1.0f;
        ivertices[k++]=0.0f;
        ivertices[k++]=0.0f;        
    }
    //Spectacles
    ivertices[k++]=75.0f;
    ivertices[k++]=81.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=85.0f;
    ivertices[k++]=81.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=75.0f;
    ivertices[k++]=89.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=85.0f;
    ivertices[k++]=89.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=75.0f;
    ivertices[k++]=89.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=85.0f;
    ivertices[k++]=81.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=1.0f;
    //Backpack
    ivertices[k++]=60.0f;
    ivertices[k++]=68.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.9f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=68.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.9f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=60.0f;
    ivertices[k++]=78.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.9f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    //traingle2
    ivertices[k++]=60.0f;
    ivertices[k++]=78.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.9f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=68.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.9f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=78.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.9f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    //Legs
    ivertices[k++]=65.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=68.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=55.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    //traingle2
    ivertices[k++]=65.0f;
    ivertices[k++]=55.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=68.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=68.0f;
    ivertices[k++]=55.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f; 
    //Leg 2 
    ivertices[k++]=82.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=85.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=82.0f;
    ivertices[k++]=55.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    //traingle2
    ivertices[k++]=82.0f;
    ivertices[k++]=55.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=85.0f;
    ivertices[k++]=65.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=85.0f;
    ivertices[k++]=55.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=1.0f;
    ivertices[k++]=0.0f;
    ivertices[k++]=0.0f; 
    /*float ivertices[]={
    	55.0f,55.0f,0.0f, 1.0f,0.0f,0.0f,
    	55.0f,95.0f,0.0f, 1.0f,0.0f,0.0f,
    	95.0f,55.0f,0.0f, 1.0f,0.0f,0.0f,
    	95.0f,95.0f,0.0f, 1.0f,0.0f,0.0f,
    	95.0f,55.0f,0.0f, 1.0f,0.0f,0.0f,
    	55.0f,95.0f,0.0f, 1.0f,0.0f,0.0f,	    	
    };*/
    /*std::vector<float> ibverticesv;
    for(float ang=0.0f;ang <= 2*3.1417;ang+=0.01f){
    	float x = 75.0f + 15.0f*cos(ang);
    	ibverticesv.push_back(x);
    	float y = 75.0f + 15.0f*sin(ang);
    	ibverticesv.push_back(y);
    	float z = 0.0f;
    	ibverticesv.push_back(z);
    	ibverticesv.push_back(0.0f);
    	ibverticesv.push_back(1.0f);
    	ibverticesv.push_back(0.0f);
    	float xn = 75.0f + 15.0f * cos(ang+0.01f);
    	ibverticesv.push_back(xn);
    	float yn= 75.0f + 15.0f * sin(ang + 0.01f);
    	ibverticesv.push_back(yn);
    	float zn = 0.0f;
    	ibverticesv.push_back(zn);
    	ibverticesv.push_back(0.0f);
    	ibverticesv.push_back(1.0f);
    	ibverticesv.push_back(0.0f);
    	ibverticesv.push_back(75.0f);
    	ibverticesv.push_back(75.0f);
    	ibverticesv.push_back(0.0f);
    	ibverticesv.push_back(0.0f);
    	ibverticesv.push_back(1.0f);
    	ibverticesv.push_back(0.0f);  
    };
    float * ibvertices = &ibverticesv[0];*/
    float ibvertices[18*80];
    int j=0;
    for(float ang=0.0;ang<8.0;ang+=0.1){
    	//center
    	ibvertices[j++]=75.0f;
    	ibvertices[j++]=75.0f;
    	ibvertices[j++]=0.0f;
    	//color
    	ibvertices[j++]=0.0f;
    	ibvertices[j++]=1.0f;
    	ibvertices[j++]=0.0f;
    	//Vertex1
    	ibvertices[j++]=75.0+15*cos(ang);
    	ibvertices[j++]=75.0+15*sin(ang);
    	ibvertices[j++]=0.0f;
    	//color
    	ibvertices[j++]=0.0f;
    	ibvertices[j++]=1.0f;
    	ibvertices[j++]=0.0f;
    	//Vertex2
    	ibvertices[j++]=75.0+15*cos(ang+0.1);
    	ibvertices[j++]=75.0+15*sin(ang+0.1);
    	ibvertices[j++]=0.0f;
    	//color
    	ibvertices[j++]=0.0f;
    	ibvertices[j++]=1.0f;
    	ibvertices[j++]=0.0f;    	
    }
    /*float ibvertices[]={
    	75.0f,75.0f,0.0f,0.0f,1.0f,0.0f,
    	75.0f,95.0f,0.0f,0.0f,1.0f,0.0f,
    	95.0f,75.0f,0.0f,0.0f,1.0f,0.0f,
    };*/
    float tbvertices[18*80];
    j=0;
    for(float ang=0.0;ang<8.0;ang+=0.1){
    	//center
    	tbvertices[j++]=75.0f;
    	tbvertices[j++]=75.0f;
    	tbvertices[j++]=0.0f;
    	//color
    	tbvertices[j++]=1.0f;
    	tbvertices[j++]=1.0f;
    	tbvertices[j++]=0.0f;
    	//Vertex1
    	tbvertices[j++]=75.0+15*cos(ang);
    	tbvertices[j++]=75.0+15*sin(ang);
    	tbvertices[j++]=0.0f;
    	//color
    	tbvertices[j++]=1.0f;
    	tbvertices[j++]=1.0f;
    	tbvertices[j++]=0.0f;
    	//Vertex2
    	tbvertices[j++]=75.0+15*cos(ang+0.1);
    	tbvertices[j++]=75.0+15*sin(ang+0.1);
    	tbvertices[j++]=0.0f;
    	//color
    	tbvertices[j++]=1.0f;
    	tbvertices[j++]=1.0f;
    	tbvertices[j++]=0.0f;    	
    }
    j=0;
    /*float tbvertices[]={
    	95.0f,95.0f,0.0f,1.0f,1.0f,1.0f,
    	75.0f,95.0f,0.0f,1.0f,1.0f,1.0f,
    	95.0f,75.0f,0.0f,1.0f,1.0f,1.0f,    	
    };*/

    float taskvert[18*80];
    for(float ang=0.0;ang<8.0;ang+=0.1){
    	//center
    	taskvert[j++]=75.0f;
    	taskvert[j++]=75.0f;
    	taskvert[j++]=0.0f;
    	//color
    	taskvert[j++]=0.831f;
    	taskvert[j++]=0.686f;
    	taskvert[j++]=0.216f;
    	//Vertex1
    	taskvert[j++]=75.0+10*cos(ang);
    	taskvert[j++]=75.0+10*sin(ang);
    	taskvert[j++]=0.0f;
    	//color
    	taskvert[j++]=0.831f;
    	taskvert[j++]=0.686f;
    	taskvert[j++]=0.216f;
    	//Vertex2
    	taskvert[j++]=75.0+10*cos(ang+0.1);
    	taskvert[j++]=75.0+10*sin(ang+0.1);
    	taskvert[j++]=0.0f;
    	//color
    	taskvert[j++]=0.831f;
    	taskvert[j++]=0.686f;
    	taskvert[j++]=0.216f;   	
    }
    /*float taskvert[]={
    	55.0f,55.0f,0.0f, 0.2f,0.3f,0.4f,
    	95.0f,55.0f,0.0f, 0.2f,0.3f,0.4f,
    	75.0f,95.0f,0.0f, 0.2f,0.3f,0.4f,
    };*/
    j=0;
    float obsvert[18*80+8*18];
    for(float ang=0.0;ang<8.0;ang+=0.1){
    	//center
    	obsvert[j++]=75.0f;
    	obsvert[j++]=75.0f;
    	obsvert[j++]=0.0f;
    	//color
    	obsvert[j++]=0.798f;
    	obsvert[j++]=0.007f;
    	obsvert[j++]=0.007f;
    	//Vertex1
    	obsvert[j++]=75.0+15*cos(ang);
    	obsvert[j++]=75.0+15*sin(ang);
    	obsvert[j++]=0.0f;
    	//color
    	obsvert[j++]=0.798f;
    	obsvert[j++]=0.007f;
    	obsvert[j++]=0.007f;
    	//Vertex2
    	obsvert[j++]=75.0+15*cos(ang+0.1);
    	obsvert[j++]=75.0+15*sin(ang+0.1);
    	obsvert[j++]=0.0f;
    	//color
    	obsvert[j++]=0.798f;
    	obsvert[j++]=0.007f;
    	obsvert[j++]=0.007f;	
    }
    //line width some width
    obsvert[j++]=75.0f+10.0f;
    obsvert[j++]=75.0f+12.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;

    obsvert[j++]=75.0f+12.0f;
    obsvert[j++]=75.0f+10.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    //
    obsvert[j++]=75.0f-12.0f;
    obsvert[j++]=75.0f-10.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    //triangle2
    obsvert[j++]=75.0f+12.0f;
    obsvert[j++]=75.0f+10.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    //
    obsvert[j++]=75.0f-12.0f;
    obsvert[j++]=75.0f-10.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    //
    obsvert[j++]=75.0f-10.0f;
    obsvert[j++]=75.0f-12.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;


    //Line 2
    obsvert[j++]=75.0f-10.0f;
    obsvert[j++]=75.0f+12.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;

    obsvert[j++]=75.0f-12.0f;
    obsvert[j++]=75.0f+10.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    //
    obsvert[j++]=75.0f+12.0f;
    obsvert[j++]=75.0f-10.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    //triangle2
    obsvert[j++]=75.0f-12.0f;
    obsvert[j++]=75.0f+10.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    //
    obsvert[j++]=75.0f+12.0f;
    obsvert[j++]=75.0f-10.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    //
    obsvert[j++]=75.0f+10.0f;
    obsvert[j++]=75.0f-12.0f;
    obsvert[j++]=0.0f;
    //color
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    obsvert[j++]=1.0f;
    float healthvertices[18*2];
    j=0;
    healthvertices[j++]= 50.0f;
    healthvertices[j++]= 810.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]= 60.0f;
    healthvertices[j++]= 810.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]= 60.0f;
    healthvertices[j++]= 830.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]= 50.0f;
    healthvertices[j++]= 810.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]= 50.0f;
    healthvertices[j++]= 830.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]= 60.0f;
    healthvertices[j++]= 830.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=0.0f;
    healthvertices[j++]=1.0f;
    healthvertices[j++]=1.0f;
    j=0;
    float scorevert[18*80];
    for(float ang=0.0;ang<8.0;ang+=0.1){
        //center
        scorevert[j++]=135.0f;
        scorevert[j++]=825.0f;
        scorevert[j++]=0.0f;
        //color
        scorevert[j++]=0.831f;
        scorevert[j++]=0.686f;
        scorevert[j++]=0.216f;
        //Vertex1
        scorevert[j++]=135.0f+7*cos(ang);
        scorevert[j++]=825.0f+7*sin(ang);
        scorevert[j++]=0.0f;
        //color
        scorevert[j++]=0.831f;
        scorevert[j++]=0.686f;
        scorevert[j++]=0.216f;
        //Vertex2
        scorevert[j++]=135.0+7*cos(ang+0.1);
        scorevert[j++]=825.0+7*sin(ang+0.1);
        scorevert[j++]=0.0f;
        //color
        scorevert[j++]=0.831f;
        scorevert[j++]=0.686f;
        scorevert[j++]=0.216f;       
    }       
    /*float obsvert[]={
    	55.0f,95.0f,0.0f, 1.0f,0.3f,0.0f,
    	95.0f,95.0f,0.0f, 1.0f,0.3f,0.0f,
    	75.0f,55.0f,0.0f, 1.0f,0.3f,0.0f,
    };*/
    unsigned int VBO,VAO,VBO2,VAO2,VBO3,VAO3,VBO4,VAO4,VBO5,VAO5,VBO6,VAO6,VBO7,VAO7,VBO8,VAO8,VBO9,VAO9,VBO10,VAO10;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hvertices), hvertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glGenVertexArrays(1,&VAO2);
    glGenBuffers(1,&VBO2);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER,VBO2);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vvertices),vvertices,GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glGenVertexArrays(1,&VAO3);
    glGenBuffers(1,&VBO3);
    glBindVertexArray(VAO3);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pvertices), pvertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    //Impostor
    glGenVertexArrays(1,&VAO4);
    glGenBuffers(1,&VBO4);
    glBindVertexArray(VAO4);
    glBindBuffer(GL_ARRAY_BUFFER, VBO4);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ivertices), ivertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    //Button to kill Impostor
    glGenVertexArrays(1,&VAO5);
    glGenBuffers(1,&VBO5);
    glBindVertexArray(VAO5);
    glBindBuffer(GL_ARRAY_BUFFER, VBO5);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ibvertices), ibvertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    //Button to generate tasks
    glGenVertexArrays(1,&VAO6);
    glGenBuffers(1,&VBO6);
    glBindVertexArray(VAO6);
    glBindBuffer(GL_ARRAY_BUFFER, VBO6);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tbvertices), tbvertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    //Powerups
    glGenVertexArrays(1,&VAO7);
    glGenBuffers(1,&VBO7);
    glBindVertexArray(VAO7);
    glBindBuffer(GL_ARRAY_BUFFER, VBO7);
    glBufferData(GL_ARRAY_BUFFER, sizeof(taskvert), taskvert, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
   	//Obstacles
    glGenVertexArrays(1,&VAO8);
    glGenBuffers(1,&VBO8);
    glBindVertexArray(VAO8);
    glBindBuffer(GL_ARRAY_BUFFER, VBO8);
    glBufferData(GL_ARRAY_BUFFER, sizeof(obsvert), obsvert, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    //Health Bar
    glGenVertexArrays(1,&VAO9);
    glGenBuffers(1,&VBO9);
    glBindVertexArray(VAO9);
    glBindBuffer(GL_ARRAY_BUFFER, VBO9);
    glBufferData(GL_ARRAY_BUFFER, sizeof(healthvertices), healthvertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    //Scorebar
    glGenVertexArrays(1,&VAO10);
    glGenBuffers(1,&VBO10);
    glBindVertexArray(VAO10);
    glBindBuffer(GL_ARRAY_BUFFER, VBO10);
    glBufferData(GL_ARRAY_BUFFER, sizeof(scorevert), scorevert, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
   	glUseProgram(newshader.ID);
   glm::mat4 projection = glm::ortho(0.0f,850.0f,0.0f,850.0f,-1.0f,1.0f);
   glfwSetKeyCallback(window,key_callback);
    newshader.setMat4("projection", projection);
    Shader lightingShader(1);
    unsigned int lightVAO;
    glGenVertexArrays(1,&lightVAO);
    glBindVertexArray(lightVAO);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    while (!glfwWindowShouldClose(window))
    {
        //Lighting Shit
       // lightingShader.use();
       // lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
       // lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        //lightingShader.setVec3("lightPos", 75.0f,75.0f,0.0f);
        //lightingShader.setVec3("viewPos", 75.0f,75.0f,0.5f);
    	std::cout << "Score " <<score << " Health " << health << " Countdown " << countdown << " Taskdone/Total " << tasksdone << " / " << tasks  << std::endl;
    	if(lastinp!=-1){
    		if(lastinp==0){
    			playery--;
    		}
    		else if(lastinp==1){
    			playery++;
    		}
    		else if (lastinp==2){
    			playerx--;
    			if(playerx==-1 && playery==endy && tasksdone==2){
    				if(impdead==true)
    					break;
    				else
    					playerx=0;
    			}
    			else if(playerx==-1 && playery==endy){
    				playerx=0;
    			}
    		}
    		else if (lastinp==3){
    			playerx++;
    		}
    		else if (lastinp==4){
    			//std::cout << "here" << std::endl;
    			//std::cout << playerx << playery << impbuttonx << impbuttony << std::endl;
    			if(playerx==impbuttonx && playery==impbuttony && !impdead){
    				impdead=true;
    				impbuttonactiv=false;
    				tasksdone++;
    			}
    			if(playerx==taskbuttonx && playery==taskbuttony && !taskact){
    				taskact=true;
    				taskbuttonactiv=false;
    				tasksdone++;
    			}
    		}
    		lastinp=-1;
    	}

    	if(taskact==true){
	    	for(int i=0;i<15;i++){
	    		if(pupsandob[i].type==true && pupsandob[i].active==true && playerx==pupsandob[i].xcor && playery==pupsandob[i].ycor){
	    			pupsandob[i].active=false;
	    			score+=10;
	    		}
	    		else if(pupsandob[i].type==false && pupsandob[i].active==true && playerx==pupsandob[i].xcor && playery==pupsandob[i].ycor){
	    			pupsandob[i].active=false;
	    			health--;
	    		}
	    	}
    	}
    	if(health==0){
    		break;
    	}
        float curr=glfwGetTime();
        deltaTime=curr-lastFrame;
        lastFrame=curr;
        if(curr- countdowntime>1.0f){
            countdowntime=curr;
            countdown--;
            if(countdown==0){
                break;
            }
        }
        if(curr- lastimpmov > 0.5f && !impdead){
        	lastimpmov=curr;
        	int source = size*impostory+impostorx;
        	int destn = size*playery+playerx;
        	if(source==destn){
        		impdead=true;
        		//std::cout << "Game Over" << std::endl;
        		break;
        	}
        	int nxstep = BFSshortpath(source,destn);
        	impostorx = nxstep%size;
        	impostory=nxstep/size;
        	//std::cout << source << "  " << destn << " nxtstep " << nxstep <<" posn " << impostorx << "  " << impostory << std::endl;
        }
        if(playerx==impostorx && playery==impostory && !impdead){
        	break;
        }
        //std::cout << deltaTime << std::endl;
        //processInput(window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        newshader.use();
        float ang = 3.1417/2;
        for(int i=0;i<size*size;i++){
        	glm::mat4 model = glm::mat4(1.0f);
	        int xcor = i % size;
	        int ycor = i / size;
	        if(Maze[i].avaipath[0]==false || Maze[i].avaipath[1]==false){
	        	glBindVertexArray(VAO);
	        	if(Maze[i].avaipath[0]==false){
	        		model=glm::mat4(1.0f);
	        		model = glm::translate(model,glm::vec3(xcor*sizeofeachgrid,ycor*sizeofeachgrid,0.0f));
	        		newshader.setMat4("model",model);
	        		glDrawArrays(GL_LINES,0,2);
	        	}
	        	if(Maze[i].avaipath[1]==false){
	        		model=glm::mat4(1.0f);
	        		model = glm::translate(model,glm::vec3(xcor*sizeofeachgrid,(ycor+1)*sizeofeachgrid,0.0f));
	        		newshader.setMat4("model",model);
	        		glDrawArrays(GL_LINES,0,2);	        	
	        	}
	        }
	        if(Maze[i].avaipath[2]==false || Maze[i].avaipath[3]==false){
	        	glBindVertexArray(VAO2);
	        	if(Maze[i].avaipath[2]==false){
	        		model=glm::mat4(1.0f);
	        		model = glm::translate(model,glm::vec3(xcor*sizeofeachgrid,ycor*sizeofeachgrid,0.0f));
	        		newshader.setMat4("model",model);
	        		glDrawArrays(GL_LINES,0,2);
	        	}
	        	if(Maze[i].avaipath[3]==false){
	        		model=glm::mat4(1.0f);
	        		model = glm::translate(model,glm::vec3((xcor+1)*sizeofeachgrid,(ycor)*sizeofeachgrid,0.0f));
	        		newshader.setMat4("model",model);
	        		glDrawArrays(GL_LINES,0,2);	        	
	        	}	        	
	        }
	        //glm::mat4 model = glm::mat4(1.0f);
	       	//model = glm::rotate(model, glm::radians(ang) , glm::vec3(1.0f,1.0f,0.0f));
	       	//std::cout << model << std::endl;
	        //std::cout<<glm::to_string(model)<<std::endl;
	        //newshader.setMat4("model",model);
	        //glDrawArrays(GL_LINES,0,2);
	        if(i==size*size-1){
        //Player
		        glBindVertexArray(VAO3);
		        glm::mat4 model = glm::mat4(1.0f);
		        model =  glm::translate(model,glm::vec3(playerx*sizeofeachgrid,playery*sizeofeachgrid,0.0f));
		        newshader.setMat4("model",model);
		        glDrawArrays(GL_TRIANGLES,0,3*44);
		        if(!impdead){
			        glBindVertexArray(VAO4);
			        model = glm::mat4(1.0f);
			        model =  glm::translate(model,glm::vec3(impostorx*sizeofeachgrid,impostory*sizeofeachgrid,0.0f));
			        newshader.setMat4("model",model);
			        glDrawArrays(GL_TRIANGLES,0,3*44);
		    	}
		        if(impbuttonactiv){
			        glBindVertexArray(VAO5);
			        model = glm::mat4(1.0f);
			        model=glm::translate(model,glm::vec3(impbuttonx*sizeofeachgrid,impbuttony*sizeofeachgrid,0.0f));
			        newshader.setMat4("model",model);
			        glDrawArrays(GL_TRIANGLES,0,80*3);	
		    	}
		    	if(taskbuttonactiv){
			        glBindVertexArray(VAO6);
			        model = glm::mat4(1.0f);
			        model=glm::translate(model,glm::vec3(taskbuttonx*sizeofeachgrid,taskbuttony*sizeofeachgrid,0.0f));
			        newshader.setMat4("model",model);
			        glDrawArrays(GL_TRIANGLES,0,80*3);		
		        }	        	        		        
	        }
        }
        if(taskact==true){
        	glm::mat4 model = glm::mat4(1.0f);
        	for(int i=0;i<15;i++){
        		if(pupsandob[i].type==true && pupsandob[i].active==true){
	        		glBindVertexArray(VAO7);
	        		model = glm::mat4(1.0f);
	        		model=glm::translate(model,glm::vec3(pupsandob[i].xcor*sizeofeachgrid,pupsandob[i].ycor*sizeofeachgrid,0.0f));
	        		newshader.setMat4("model",model);
	        		glDrawArrays(GL_TRIANGLES,0,80*3);
	        		//std::cout << pupsandob[i].xcor << "    " << pupsandob[i].ycor << " -  " << i << std::endl;
        		}
        		if(pupsandob[i].type==false && pupsandob[i].active==true){
	        		glBindVertexArray(VAO8);
	        		model = glm::mat4(1.0f);
	        		model=glm::translate(model,glm::vec3(pupsandob[i].xcor*sizeofeachgrid,pupsandob[i].ycor*sizeofeachgrid,0.0f));
	        		newshader.setMat4("model",model);
	        		glDrawArrays(GL_TRIANGLES,0,88*3);
	        		//std::cout << pupsandob[i].xcor << "    " << pupsandob[i].ycor << " -  " << i << std::endl;
        		}        		
        	}
        }
        //Health Bar
        glBindVertexArray(VAO9);
        for(int i=0;i<health;i++){
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model,glm::vec3(i*12.0f,0.0f,0.0f));
            newshader.setMat4("model",model);
            glDrawArrays(GL_TRIANGLES,0,6);
        }
        //Score Bar
        glBindVertexArray(VAO10);
        for(int i=10; i <= score;i+=10){
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model,glm::vec3((i/10)*15.0f,0.0f,0.0f));
                    newshader.setMat4("model",model);
                    glDrawArrays(GL_TRIANGLES,0,80*3);
        }
	    //glBindVertexArray(VAO2);
	    //model=glm::mat4(1.0f);
	    //newshader.setMat4("model",model);
	    //glDrawArrays(GL_LINES,0,2);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteVertexArrays(1, &VAO3);
    glDeleteBuffers(1, &VBO3);
    glDeleteVertexArrays(1, &VAO4);
    glDeleteBuffers(1, &VBO4);
    glDeleteVertexArrays(1, &VAO5);
    glDeleteBuffers(1, &VBO5);
    glDeleteVertexArrays(1, &VAO6);
    glDeleteBuffers(1, &VBO6);
    glDeleteVertexArrays(1, &VAO7);
    glDeleteBuffers(1, &VBO7);
    glfwTerminate();
    return 0;
}
void key_callback(GLFWwindow *window,int key, int scancode, int action, int mods)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key==GLFW_KEY_W && action==GLFW_PRESS){
    	if(playery < size-1 && Maze[playery*size + playerx].avaipath[1]==true){
    		lastinp=1;
    	}
    }
    if (key==GLFW_KEY_S && action==GLFW_PRESS){
     	if(playery > 0 && Maze[playery*size + playerx].avaipath[0]==true){
    		lastinp=0;
    	}	
    }
    if (key==GLFW_KEY_A && action==GLFW_PRESS){
    	if(playerx >= 0 && Maze[playery*size+playerx].avaipath[2]==true){
    		lastinp=2;
    	}
    }
    if (key==GLFW_KEY_D && action==GLFW_PRESS){
    	if(playerx < size-1 && Maze[playery*size+playerx].avaipath[3]==true){
    		lastinp=3;
    	}
    }
    if (key==GLFW_KEY_K && action==GLFW_PRESS){
    	lastinp=4;
    }

}	
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
