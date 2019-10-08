#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include <string>
#include <vector>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    virtual void removeDead();
    virtual void updateStat();
    
    virtual void makeObject(Actor *actor);
    virtual NanchenBlaster* getNanchenBlaster();
    
    virtual bool hitEnemy(int projX, int projY, int projRad, int projDamage);
    virtual bool collidedWithPlayer(Actor* actor);
    
    virtual void enemyDied();
    virtual void playerDestroyedEnemy();
    
    virtual ~StudentWorld();
private:
    std::vector<Actor*> gameObject;
    NanchenBlaster* m_player;
    std::string m_statText;
    
    
    int m_destroyedTotal; //D
    int m_toLevelUp;
    int m_enemyOnScreen;
    int m_maxAlienOnScreen;
    bool m_cleanUp;
    
    
    int m_probabilityBase;
    int m_smallgonHP;
    int m_smoregonHP;
    int m_snagglegonHP;
    
    
    

    
    
};

#endif // STUDENTWORLD_H_
