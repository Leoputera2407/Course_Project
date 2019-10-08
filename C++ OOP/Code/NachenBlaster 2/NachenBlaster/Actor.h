#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
/* ===================Actor Declaration==================*/
class StudentWorld;


class Actor: public GraphObject
{
public:
    Actor(int imageID,int startX, int startY,int startDirection, double size, int depth, StudentWorld* world);
    
    virtual double euclidian_dist(int x1,int y1,int x2,int y2);
    
    virtual bool closeEnough(int x, int y, int rad);
    
    virtual bool stillAlive() const = 0;
    
    virtual void doSomething() = 0;
    
    virtual void destroyed() = 0;
    
    virtual bool isEnemyShip () const = 0;
    
    virtual bool canDamagePlayer() const = 0;
    
    virtual bool isCollectible() const = 0;
    
    virtual void reduceHP(int damage){};
    
    virtual ~Actor(){};
    
    
    
protected:
    virtual StudentWorld* getWorld() const;
private:
    StudentWorld* m_world;
};


/* ===================SpaceCraft Declaration=============*/

class SpaceCraft: public Actor
{
public:
    SpaceCraft(int imageID,int startX, int startY,int startDirection, double size, int depth, int startHP, StudentWorld* world);
    
    virtual bool stillAlive() const = 0;
    
    virtual void doSomething() = 0;
    
    virtual int getHP() const;
    
    virtual void reduceHP(int damage);
    
    virtual void increaseHP(int gainHP);
    
    virtual bool isEnemyShip() const = 0;
    
    virtual bool canDamagePlayer() const;
    
    virtual bool isCollectible() const;
    
    virtual ~SpaceCraft(){};
    
    virtual void destroyed() = 0;
    
private:
    int m_hp;
    int m_torpedoes;
};


/* ===================Nanchen Declaration================*/


class NanchenBlaster: public SpaceCraft
{
public:
    NanchenBlaster(StudentWorld* world);
    
    virtual bool stillAlive () const;
    
    virtual int getCabbage() const;
    
    virtual int getTorpedoes() const;
    
    virtual void gainTorpedoes();
    
    virtual bool isEnemyShip() const;
    
    virtual void doSomething();
    
    virtual ~NanchenBlaster(){};
    
    virtual void destroyed(){};
    
private:
    int m_cabbage;
    int m_torpedoes;
};

/* ==================EnemyShip Declaration================*/
class EnemyShip: public SpaceCraft
{
public:
    EnemyShip(StudentWorld* world, int imageID, int type, double startSpeed, int collisionDamage, int startHP,  int startY);
    
    virtual bool stillAlive () const;
    
    virtual int getSpeed() const;
    
    virtual void boosted();
    
    virtual void setFlightPlan ();
    
    virtual bool checkHitPlayerOrFlyOff();
    
    virtual bool isEnemyShip() const;
    
    virtual void move();
    
    virtual void destroyed();
    
    virtual void doSomething()= 0;
    
    virtual ~EnemyShip(){};
    
private:
    int m_flightPlan;
    int m_flightPath;
    double m_speed;
    int m_collisionDamage;
    int m_type;
    int m_alive;
    
};



/* ===================Smoregon Declaration================*/
class Smoregon: public EnemyShip
{
public:
    Smoregon(StudentWorld* world, int startY, int startHP);
    
    virtual void doSomething();
    
    virtual ~Smoregon(){};

    
};

/* ==================Smallgon Declaration================*/
class Smallgon: public EnemyShip
{
public:
    Smallgon(StudentWorld* world, int startY, int startHP);
    
    virtual void doSomething();
    
    virtual ~Smallgon(){};
    
};


/* ==================Snaggle Declaration================*/
class Snagglegon: public EnemyShip
{
public:
    Snagglegon(StudentWorld* world, int startY, int startHP);
    
    virtual void doSomething();
    
    virtual ~Snagglegon(){};
    
    
};


/* ===================Star Declaration==================*/

class Star: public Actor
{
public:
    Star(StudentWorld* world, int startX, int startY);
    
    virtual bool stillAlive() const;
    
    virtual void doSomething();
    
    virtual bool isEnemyShip() const;
    
    virtual bool canDamagePlayer() const;
    
    virtual bool isCollectible() const;
    
    virtual void destroyed();
    
    virtual ~Star(){};
    
private:
    bool m_alive;
    
};

/* ===================Explosion Declaration==================*/

class Explosion: public Actor
{
public:
    Explosion(StudentWorld* world, int startX, int startY);
    
    virtual bool stillAlive() const;
    
    virtual void doSomething();
    
    virtual bool isEnemyShip() const;
    
    virtual bool canDamagePlayer() const;
    
    virtual bool isCollectible() const;
    
    virtual void destroyed(){};
    
    virtual ~Explosion(){};
    
private:
    int m_lifetime;
    
};



/* ===================Projecticle Declaration=============*/
class Projectile: public Actor
{
public:
    Projectile(int imageID, int type, int startX, int startY,int startDirection, double size, int depth, int damage, StudentWorld* world);
    
    virtual void doSomething() = 0;
    
    virtual bool stillAlive() const;
    
    virtual int getDamage() const;
    
    virtual bool isEnemyShip() const;

    virtual bool  canDamagePlayer() const;
    
    virtual bool isCollectible() const;
    
    virtual bool hitTargetOrFlyOff();
    
    virtual void destroyed();
    
    ~Projectile(){};

private:
    bool m_alive;
    int m_damage;
    int m_type;
    
};
    
    
/* ===================Cabbage Declaration=============*/
class Cabbage: public Projectile
{
    
public:
    Cabbage(StudentWorld* world, int startX, int startY);
    
    virtual void doSomething();
    
    virtual ~Cabbage(){};
    
};


/* ===================Turnip Declaration=============*/
class Turnip: public Projectile
{
public:
    Turnip(StudentWorld* world, int startX, int startY);
    
    virtual void doSomething();
    
    virtual ~Turnip(){};
};

/* ===================FlatTorpedo Declaration=============*/

class FlatTorpedo: public Projectile
{
public:
    FlatTorpedo(StudentWorld* world,int startX, int startY, int startDirection);
    
    virtual bool hitTargetOrFlyOff();
    
    virtual void doSomething();
    
    virtual ~FlatTorpedo(){};
    
private:
    bool m_firedByPlayer;
};




/* ===================Collectible Declaration=============*/

class Collectible: public Actor
{
public:
    Collectible(int imageID, int type, int startX, int startY,StudentWorld* world);
    
    virtual void doSomething() = 0;
    
    virtual bool stillAlive() const;
    
    virtual bool isEnemyShip() const;
    
    virtual bool canDamagePlayer() const;
    
    virtual bool isCollectible() const;
    
    virtual void collectibleGained();
    
    virtual bool hitPlayerOrFlyOff();
    
    virtual void action();
    
    virtual void destroyed();
    
    virtual ~Collectible(){};

private:
    bool m_alive;
    int m_type;
    

};

/* ===================RepairGoodie Declaration=============*/

class RepairGoodie: public Collectible
{
public:
    RepairGoodie(StudentWorld* world, int startX, int startY);
    
    virtual void doSomething();
    
    virtual ~RepairGoodie(){};
    
  
};

/* ===================ExtraLife Declaration=============*/
class ExtraLife: public Collectible
{
public:
    ExtraLife(StudentWorld* world, int startX, int startY);
    
    virtual void doSomething();
    
    virtual ~ExtraLife(){};
    
    
};

/* ===================TorpedoGoodie Declaration=============*/

class TorpedoGoodie: public Collectible
{
public:
    TorpedoGoodie(StudentWorld* world, int startX, int startY);
    
    virtual void doSomething();
    
    virtual ~TorpedoGoodie(){};
    
};






#endif // ACTOR_H_
