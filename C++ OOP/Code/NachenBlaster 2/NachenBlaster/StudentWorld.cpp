#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <iostream> // defines the overloads of the << operator
#include <sstream>  // defines the type std::ostringstream
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
    return new StudentWorld(assetDir);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetDir)
: GameWorld(assetDir)
{
    m_cleanUp = false;
}


//Initializer to be called at the start.
int StudentWorld::init()
{
    //Initiliaze 30 stars of random sizes randomly on the screen.
    m_player = new NanchenBlaster(this);
    for(int i = 1; i <=30 ; i++){
        int x = randInt(0, VIEW_WIDTH-1);
        int y = randInt(0, VIEW_HEIGHT-1);
        gameObject.push_back(new Star(this, x, y));
    }
    //Determine how many enemies to kill to level up based on the spec
    m_toLevelUp = 6 + (4* getLevel());
    //Set destroyedTotal to 0.
    m_destroyedTotal = 0;
    //Determine max aliens of screen based on the spec.
    m_maxAlienOnScreen = 4+ (0.5 * getLevel());
    //Current enemy on screen is zero.
    m_enemyOnScreen = 0;
    
    //Total probability to spawn enemy as given in spec
    m_probabilityBase = 60 + 20 + (5*getLevel()) + 5+ (10*getLevel());
    //Determine enemy ships HP as per the spec.
    m_smoregonHP = m_smallgonHP =  5 * (1 +(getLevel() - 1) * 0.1);
    m_snagglegonHP = 10 * (1 +(getLevel() - 1) * 0.1);

    
    
    return GWSTATUS_CONTINUE_GAME;
}


int StudentWorld::move()
{
    //1 in 15 chance of creating a star at random Y-position.
    if(randInt(1,15) == 1){
        gameObject.push_back(new Star(this, VIEW_WIDTH - 1, randInt(0,VIEW_HEIGHT-1)));
    }
    
    //Seed enemyship as per the condition given in spec
    if(m_enemyOnScreen < min(m_toLevelUp - m_destroyedTotal, m_maxAlienOnScreen )){
        //Choose the types of enemies based on equation in the spec.
        int startY = randInt(0, VIEW_HEIGHT-1);
        int result = randInt(1,m_probabilityBase);
        
        if(result <= 60)
            gameObject.push_back(new Smallgon(this,startY ,m_smallgonHP));
        else if(result <= (60 + 20 + (5*getLevel())))
            gameObject.push_back(new Smoregon(this,startY ,m_smoregonHP));
        else
            gameObject.push_back(new Snagglegon(this,startY ,m_snagglegonHP));
      
        m_enemyOnScreen++;
//        int enemies=0;
//        for (auto p = gameObject.begin(); p != gameObject.end(); ++p){
//            if( (*p)->isEnemyShip())
//                ++enemies;
//        }
//        
//        std::cerr<<"Player added. Now, n. of enemies in gameobject: "<<enemies<<std::endl;
    }
    
    //Player do soemthing
    m_player->doSomething();
    vector<Actor*> ::iterator it = gameObject.begin();
    //Iterate through the container and let each actor act.
    while(it != gameObject.end()){
        if((*it)->stillAlive()){
            (*it)->doSomething();
            //If player dies given the action of the actor, return GWSTATUS_PLAYER_DIED
            if(!m_player->stillAlive()){
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }
       }
        it++;
    }
    
    removeDead();
    updateStat();
    if(m_destroyedTotal < m_toLevelUp){
        return GWSTATUS_CONTINUE_GAME;
    }
    else{ //Level up if level up condition is met.
        return GWSTATUS_FINISHED_LEVEL;
    }
}

//Removes all actor to reset level or when it's gameover.
void StudentWorld::cleanUp()
{
    //Delete player
    delete m_player;
    vector<Actor*> ::iterator it = gameObject.begin();
    //Loop through and delete each actor in the container.
    while(it != gameObject.end()){
        delete *it;
        it = gameObject.erase(it);
    }
    m_cleanUp = true;
}

//Remove all dead actors
void StudentWorld::removeDead(){
    vector<Actor*> ::iterator it = gameObject.begin();
    //Loop through container
    while(it != gameObject.end()){
        //If actor is dead
        if(!(*it)->stillAlive()){
            //Delete actor
            delete *it;
            it = gameObject.erase(it);
        }else
            it++;
    }
}


void StudentWorld::updateStat(){
    ostringstream oss;
   
    oss<<"Lives: "<<getLives()<<"  Health:  "<<(m_player->getHP()*100)/50<<"%  Score: "<<getScore()<<"  Level: "<<getLevel()<<"  Cabbages: "<<(m_player->getCabbage()*100)/30<<"%  Torpedoes: "<<m_player->getTorpedoes()<<"  ";
    string m_statText = oss.str();
    setGameStatText(m_statText);
}

//Needed to create new game object and added to container
void StudentWorld::makeObject(Actor *actor){
    gameObject.push_back(actor);
}

//Checking if player's projectile hit enemy
bool StudentWorld::hitEnemy(int projX, int projY, int projRad, int projDamage)
{
    vector<Actor*> ::iterator it = gameObject.begin();
    //Loop thru container
    for (; it != gameObject.end(); it++){
        //If actor collides with projectile & actor is enemy ship & actor is still alive
        if ((*it)->closeEnough(projX, projY, projRad) && (*it)->isEnemyShip() && (*it)->stillAlive())
        {
            //Reduce actor's health by projectile damage
            (*it)->reduceHP(projDamage);
            
            //If it doesn't die from resulting damage
            if((*it)->stillAlive())
                //Play blast sound
                playSound(SOUND_BLAST);
            else{
                //Else, actor gets destroyed
                (*it)->destroyed();
//                std::cerr<<"enemy shot down"<<std::endl;
            }
            
            return true;
        }
    }
    return false;
}

//Return true if recognized objects (canDamagePlayer or isCollectible) collide with player.
bool StudentWorld::collidedWithPlayer(Actor* actor){
    int playerX = m_player->getX();
    int playerY = m_player->getY();
    int playerRad = m_player->getRadius();
    
    if(actor->closeEnough(playerX, playerY, playerRad) && (actor->canDamagePlayer() || actor->isCollectible()) && m_player->stillAlive())
    {
        return true;
    }
    return false;
    
}

void StudentWorld::enemyDied(){
    m_enemyOnScreen--;
}

void StudentWorld::playerDestroyedEnemy(){
    m_destroyedTotal++;
    m_enemyOnScreen--;
}

NanchenBlaster* StudentWorld::getNanchenBlaster(){
    return m_player;
}

StudentWorld::~StudentWorld(){
    //If cleanUp clean hasn't been called, calls cleanUp function. Otherwise, do nothing.
    if(!m_cleanUp)
        cleanUp();
}
