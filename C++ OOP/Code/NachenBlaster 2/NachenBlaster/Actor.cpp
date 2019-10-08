#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"


/* ===================Actor method===================*/


Actor::Actor(int imageID,int startX, int startY,int startDirection, double size, int depth, StudentWorld* world)
: GraphObject(imageID, startX, startY, startDirection, size, depth){
    m_world = world;
}

double Actor::euclidian_dist(int x1, int y1, int x2, int y2){
    double x = x1 - x2;
    double y = y1 - y2;
    double dist;
    
    dist = pow(x, 2) + pow(y, 2);
    dist = sqrt(dist);
    
    return dist;
}

//This will be used to check whether a collision occurs, using the euclid_dist function above. Just need to furnish the other parties' x,y coordinates and radius.
bool Actor::closeEnough(int x, int y, int rad)
{
    if(euclidian_dist(getX(), getY(), x, y) < 0.75*(getRadius() +rad))
        return true;
    return false;
}


StudentWorld* Actor::getWorld() const {
    return m_world;
}



/* ===================SpaceCraft Declaration=============*/

SpaceCraft:: SpaceCraft(int imageID,int startX, int startY,int startDirection, double size, int depth, int startHP, StudentWorld* world)
: Actor(imageID, startX, startY, startDirection, size, depth,world){
    
    m_hp = startHP;
}


int SpaceCraft::getHP() const{
    return m_hp;
}


void SpaceCraft::reduceHP(int damage){
    m_hp = m_hp - damage;
}


void SpaceCraft::increaseHP(int gainHP){
    m_hp = m_hp + gainHP;
}


bool SpaceCraft::isEnemyShip() const {
    return false;
}

bool SpaceCraft::canDamagePlayer () const {
    return true;
}

bool SpaceCraft::isCollectible() const {
    return false;
}


/* ===================Nanchen method===================*/

NanchenBlaster::NanchenBlaster(StudentWorld* world)
:SpaceCraft(IID_NACHENBLASTER, 0, 128, 0, 1.0, 0, 50, world)
{
    m_cabbage = 30;
    m_torpedoes = 0;
}

bool NanchenBlaster::stillAlive () const {
    //The only way nanchenblaster die is when HP<0
    if(getHP() <= 0){
        return false;
    }
    return true;
}


int NanchenBlaster::getCabbage() const{
    return m_cabbage;
}

int NanchenBlaster::getTorpedoes() const{
    return m_torpedoes;
}

void NanchenBlaster::gainTorpedoes(){
    m_torpedoes += 5;
}

void NanchenBlaster::doSomething(){
    if(stillAlive()){
        int ch;
        if (getWorld()->getKey(ch))
        {
            int x = getX();
            int y = getY();
            // user hit a key during this tick!
            switch (ch)
            {
                case KEY_PRESS_UP:
                    if(y < VIEW_HEIGHT - 6)
                        moveTo(x, y + 6);
                    break;
                case KEY_PRESS_DOWN:
                    if(y >= 6)
                        moveTo(x, y - 6);
                    break;
                case KEY_PRESS_LEFT:
                    if(x >= 6)
                        moveTo(x - 6, y);
                    break;
                case KEY_PRESS_RIGHT:
                    if(getX() < VIEW_WIDTH - 6)
                        moveTo(x + 6, y);
                    break;
                case KEY_PRESS_SPACE:
                    if(m_cabbage >=5){
                        //If there's at least 5 cabbage power, upon pressing SPACE, a new Cabbage projectile will be created 12 pixels to the right of player.
                        getWorld()->makeObject(new Cabbage(getWorld(), getX() + 12, getY()));
                        //Sound effect
                        getWorld()->playSound(SOUND_PLAYER_SHOOT);
                        //Reduce cabbage by 5 per shot.
                        m_cabbage -=5;
                    }
                    break;
                case KEY_PRESS_TAB:
                    if(m_torpedoes > 0){
                        //If there's torpedo power, upon pressing TAB, a new Torpedo projectile will be created at 12 pixels to the right of player.
                        getWorld()->makeObject(new FlatTorpedo(getWorld(), getX() + 12, getY(), 0));
                        //Sound effect
                        getWorld()->playSound(SOUND_TORPEDO);
                        //Reduce one torpedo power, when shot.
                        m_torpedoes--;
                    }
                    break;
            }
        }
        //After every tick, if cabbage power is < 30, replenish by 1.
        if(m_cabbage < 30)
            m_cabbage++;
    }
}

bool NanchenBlaster::isEnemyShip() const{
    return false;
}



/* ===================Smoregon Method================*/


EnemyShip::EnemyShip(StudentWorld* world, int imageID, int type, double startSpeed, int collisionDamage, int startHP,  int startY)
:SpaceCraft(imageID, VIEW_WIDTH-1 , startY, 0, 1.5, 1, startHP,world){
    m_flightPlan = 0;
    m_flightPath = 0;
    //Enemyships need to specify its type, which will help this class to determine the approriate algorithms for that enemyship.
    m_type = type;
    m_speed = startSpeed;
    m_collisionDamage = collisionDamage;
    m_alive = true;
    
}

bool EnemyShip::stillAlive () const{
    if(!m_alive||getHP() <= 0 || getX() <0)
        return false;
    return true;
    
}


int EnemyShip::getSpeed() const{
    return m_speed;
}


//Will be used when Smoregon gets boosted. Only if type = Smoregon, will the flightpath, flightplan and speed be changed.
void EnemyShip::boosted() {
    switch(m_type){
        case IID_SMOREGON:
            m_speed = 5;
            m_flightPath = 1;
            m_flightPlan = VIEW_WIDTH;
            break;
        default:
            break;
    }
}

//Choosing flightPlan & flightPath for alien ships.
void EnemyShip::setFlightPlan(){
    //All enemyships move pretty similarly
    if(getY() >= VIEW_HEIGHT - 1){
        m_flightPlan = randInt(1, 32);
        m_flightPath = 0;
    }
    if(getY() <= 0){
        m_flightPlan = randInt(1, 32);
        m_flightPath = 2;
    }
    //However, Snagglegon would move diagonally until it reached the top or bottom of the screen.
    if(m_type == IID_SNAGGLEGON)
        m_flightPlan = VIEW_HEIGHT;
    else {
        //Smoregon and smallgon would have a nother special condition where if flighplan is 0, it will randomly assign a flight path with a randomly assigned flight plan.
        if(m_flightPlan == 0)
        {
            m_flightPlan = randInt(1, 32);
            m_flightPath = randInt(0, 2);
        }
    }
    

}

//Enemyships have similar ways of being eliminated.
bool EnemyShip::checkHitPlayerOrFlyOff(){
    //1. Collide with player
    if(getWorld()->collidedWithPlayer(this)){
        NanchenBlaster* m_player = getWorld()->getNanchenBlaster();
        //Damage player based on its collsion_damage.
        m_player->reduceHP(m_collisionDamage);
        //Get destroyed, gain scores, update studentWorld and explosions.
        destroyed();
        return true;
    }
    //2. Flew Off the map
    if(getX() < 0){
        //Death without triggering drop and scoring points.
        m_alive = false;
        getWorld()->enemyDied();
        return true;
    }
    return false;
    
}

bool EnemyShip::isEnemyShip() const{
    return true;
}

//Destroyed function is called when player destroyed the enemyships. It'll trigger the chance of a random drop(based on the enemy ship type), makes an explosion
void EnemyShip::destroyed(){
    m_alive = false;
    //Update StudentWorld
    getWorld()->playerDestroyedEnemy();
    //Sound effect.
    getWorld()->playSound(SOUND_DEATH);
    //Make Explosion
    getWorld()->makeObject(new Explosion(getWorld(), getX(), getY()));
    //Determine chance of drops based on enemy ship types.
    switch(m_type){
        case IID_SMOREGON:
            if(randInt(1,3) == 1){
                if(randInt(1,2) == 1)
                    getWorld()->makeObject(new RepairGoodie(getWorld(), getX(), getY()));
                else
                    getWorld()->makeObject(new TorpedoGoodie(getWorld(), getX(), getY()));
            }
            getWorld()->increaseScore(250);
            break;
        case IID_SNAGGLEGON:
            if(randInt(1,6) == 1){
                getWorld()->makeObject(new ExtraLife(getWorld(), getX(), getY()));
            }
            getWorld()->increaseScore(1000);
            break;
        default:
            getWorld()->increaseScore(250);
            break;
    }
    
}


//Determine enemyships movements.
void EnemyShip::move(){
    
    int x = getX();
    int y = getY();

    switch (m_flightPath){
            //Left and Down
        case 0:
            moveTo(x-m_speed, y-m_speed);
            break;
            //Left
        case 1:
            moveTo(x-m_speed, y);
            break;
            //Left and Up
        case 2:
            moveTo(x-m_speed, y+m_speed);
            break;
    }
     m_flightPlan--;
}





/* ===================Smoregon Method================*/


Smoregon::Smoregon(StudentWorld* world, int startY, int startHP)
:EnemyShip(world, IID_SMOREGON,IID_SMOREGON, 2, 5, startHP, startY)
{}


void Smoregon::doSomething(){
    if(stillAlive()){
        NanchenBlaster* m_player = getWorld()->getNanchenBlaster();
        
        if(checkHitPlayerOrFlyOff())
            return;
            
        
        setFlightPlan();
        
        //Will random select special action if player is in front of them and is wihin 4 pixels up or below them.
        if(std::abs(m_player->getY()-getY()) <= 4 && getX() - m_player->getX() >= 0)
        {
            //Porbabiltiy as assinged in the spec
            int probability = (20/(getWorld()->getLevel()))+5;
            //1 in probability chance to shoot turnip
            if(randInt(1,probability) == 1){
                getWorld()->makeObject(new Turnip(getWorld(), getX() - 14, getY()));
                getWorld()->playSound(SOUND_ALIEN_SHOOT);
            }
            //1 in probability chance to boost
            if(randInt(1,probability) == 1){
                boosted();
            }
        }
        
        move();

        if(checkHitPlayerOrFlyOff())
            return;
       
    }
}


/* ===================Smallgon Method================*/


Smallgon::Smallgon(StudentWorld* world, int startY, int startHP)
:EnemyShip(world, IID_SMALLGON, IID_SMALLGON, 2, 5, startHP, startY)
{}


void Smallgon::doSomething(){
    if(stillAlive()){
        NanchenBlaster* m_player = getWorld()->getNanchenBlaster();
        
        if(checkHitPlayerOrFlyOff())
            return;
        
        setFlightPlan();
        
         //Will random select special action if player is in front of them and is wihin 4 pixels up or below them.
        if(std::abs(m_player->getY()-getY()) <= 4 && getX() - m_player->getX() >= 0){
            int probability = (20/(getWorld()->getLevel()))+5;
            //1 in probability shoot turnip
            if(randInt(1,probability) == 1){
                getWorld()->makeObject(new Turnip(getWorld(), getX() - 14, getY()));
                getWorld()->playSound(SOUND_ALIEN_SHOOT);
            }
            
        }
        
        move();
        
        if(checkHitPlayerOrFlyOff())
            return;
        
    }
}

/* ===================Snagglegon Method================*/


Snagglegon::Snagglegon(StudentWorld* world, int startY, int startHP)
:EnemyShip(world, IID_SNAGGLEGON, IID_SNAGGLEGON, 1.75, 15, startHP, startY)
{}


void Snagglegon::doSomething(){
    if(stillAlive()){
        NanchenBlaster* m_player = getWorld()->getNanchenBlaster();
        
        if(checkHitPlayerOrFlyOff())
            return;
        
        setFlightPlan();
        
         //Will random select special action if player is in front of them and is wihin 4 pixels up or below them.
        if(std::abs(m_player->getY()-getY()) <= 4 && getX() - m_player->getX() >= 0){
            int probability = (15/(getWorld()->getLevel()))+10;
            //1 in probability shoot flat torpedo
            if(randInt(1,probability) == 1){
                //Make torpedo with 180 start direction
                getWorld()->makeObject(new FlatTorpedo(getWorld(), getX(), getY(), 180));
                getWorld()->playSound(SOUND_TORPEDO);
            }
            
        }

        move();
        
        if(checkHitPlayerOrFlyOff())
            return;
        
    }
}



/* ===================Star method========================*/

Star::Star(StudentWorld* world, int startX, int startY)
:Actor(IID_STAR, startX, startY, 0, ((static_cast<double>(randInt(5, 50)))/100), 3, world)
{
    m_alive = true;
}

bool Star::stillAlive() const{
    return m_alive;
}

void Star::doSomething(){
    if(stillAlive()){
        if(getX() < 0){
            destroyed();
            return;
        }
        moveTo(getX() - 1 , getY());
        if(getX() < 0){
            destroyed();
            return;
        }
    }
}

bool Star::isEnemyShip() const{
    return false;
}

bool Star::canDamagePlayer () const{
    return false;
}

bool Star::isCollectible() const {
    return false;
}


void Star::destroyed(){
    m_alive = false;
}

/* ===============Explosion method=======================*/

Explosion::Explosion(StudentWorld* world, int startX, int startY)
:Actor(IID_EXPLOSION, startX, startY, 0, 1, 0, world)
{
    m_lifetime = 4;
}

bool Explosion::stillAlive() const{
    //Dies after 4 ticks
    if(m_lifetime <= 0)
        return false;
    return true;
}

void Explosion::doSomething(){
    if(stillAlive()){
        double curr_Size = getSize();
        setSize(curr_Size * 1.5);
        m_lifetime--;
    }
}

bool Explosion::isEnemyShip() const{
    return false;
}

bool Explosion::canDamagePlayer() const{
    return false;
}

bool Explosion::isCollectible() const {
    return false;
}


/* ===================Projecticle Method=============*/
Projectile::Projectile(int imageID, int type, int startX, int startY,int startDirection, double size, int depth, int damage, StudentWorld* world)
:Actor(imageID, startX, startY, startDirection, size, depth, world)
{
    m_alive = true;
    m_damage = damage;
    m_type = type;
}

bool Projectile::stillAlive() const{
    return m_alive;
}


int Projectile:: getDamage() const{
    return m_damage;
}

bool Projectile::isEnemyShip() const{
    return false;
}

bool Projectile::canDamagePlayer() const{
    return true;
}

bool Projectile::isCollectible() const {
    return false;
}


//Check if projectile flew off the map or hit targets
bool Projectile::hitTargetOrFlyOff(){
    NanchenBlaster* m_player = getWorld()->getNanchenBlaster();
    
    if(getX()> VIEW_WIDTH || getX() < 0){
        return true;
    }
    //As different projectile types have different behaviors when hitting targets.
    switch(m_type){
        case IID_CABBAGE:
            //Hits enemies, damaging enemies will be done in StudentWorld's hitEnemy function
            if(getWorld()->hitEnemy(getX(), getY(), getRadius(), getDamage()))
                return true;
            break;
        case IID_TURNIP:
            //Damage players
            if(getWorld()->collidedWithPlayer(this)){
                getWorld()->playSound(SOUND_BLAST);
                //Reduce player's HP
                m_player->reduceHP(getDamage());
                return true;
            }
            break;
        default:
            break;
    }
    return false;
    
    
    
    
}

void Projectile::destroyed(){
    m_alive = false;
}




/* ===================Cabbage Method=============*/
Cabbage::Cabbage(StudentWorld* world,int startX, int startY)
:Projectile(IID_CABBAGE,IID_CABBAGE, startX, startY, 0, 0.5, 1, 2, world)
{}

void Cabbage::doSomething(){
    if(stillAlive()){
        
        if(hitTargetOrFlyOff()){
            destroyed();
            return;
        }
        
        //Move right by 8 pixels
        moveTo(getX() + 8 , getY());
        //Spins
        int dir = getDirection() + 20;
        setDirection(dir);
        
        if(hitTargetOrFlyOff()){
            destroyed();
            return;
        }
    }
}




/* ===================Turnip Method=============*/
Turnip::Turnip(StudentWorld* world,int startX, int startY)
:Projectile(IID_TURNIP,IID_TURNIP,startX, startY, 0, 0.5, 1, 2, world)
{}

void Turnip::doSomething(){
    if(stillAlive()){
        if(hitTargetOrFlyOff()){
            destroyed();
            return;
        }
        
        //Move left by 6 pixels
        moveTo(getX() - 6 , getY());
        //Spins
        int dir = getDirection() + 20;
        setDirection(dir);
        
        if(hitTargetOrFlyOff()){
            destroyed();
            return;
        }
    }
}
                       
                       
/* ===================FlatTorpedo Method===============*/
FlatTorpedo::FlatTorpedo(StudentWorld* world,int startX, int startY, int startDirection)
:Projectile(IID_TORPEDO, IID_TORPEDO,startX, startY, startDirection, 0.5, 1, 8, world)
{
    //Start direction will figure out who fired the Flat Torpedo.
    if(startDirection == 0) //0, facing right, is always shot by player
        m_firedByPlayer = true;
    else
        m_firedByPlayer = false;//180, facing left, is always shot by enemy
}

bool FlatTorpedo::hitTargetOrFlyOff(){
    NanchenBlaster* m_player = getWorld()->getNanchenBlaster();
    
    if(getX()> VIEW_WIDTH || getX() < 0){
        return true;
    }
    //If fired by player and it hits enemies, damaging enemies will be done in StudentWorld's hitEnemy function
    if(m_firedByPlayer && getWorld()->hitEnemy(getX(), getY(), getRadius(), getDamage())){
        return true;
    }
    //Not shot by player-> Snaggelgon shot it. Will damage players
    if(!m_firedByPlayer && getWorld()->collidedWithPlayer(this)){
        m_player->reduceHP(getDamage());
        return true;
    }
    
    return false;

}


void FlatTorpedo::doSomething(){
   if(stillAlive()){
       
       if(hitTargetOrFlyOff()){
           //Fly off or hit target, set state to dead.
           destroyed();
           return;
       }
       
       //Decide Movement
       if(m_firedByPlayer){
           //Shot by player: move right 8 pixels.
           moveTo(getX() + 8 , getY());
       }else{
           //Shot by enemy : move left 8 pixels.
           moveTo(getX() - 8 , getY());
       }
       
       if(hitTargetOrFlyOff()){
           destroyed();
           return;
       }

       
   }
}

                       
                       

/* ==================Collectible Method==================*/
Collectible::Collectible(int imageID, int type, int startX, int startY,StudentWorld* world)
:Actor(imageID, startX, startY, 0, 0.5, 1, world){
    m_alive = true;
    m_type = type;
}

bool Collectible::stillAlive() const{
    return m_alive;
}


bool Collectible::isEnemyShip() const{
    return false;
}

bool Collectible::canDamagePlayer() const{
    return false;
}

bool Collectible::isCollectible() const {
    return true;
}

void Collectible::destroyed(){
    m_alive = false;
}

//Decide action based on goodie type.
void Collectible:: collectibleGained(){
    
    NanchenBlaster* m_player = getWorld()->getNanchenBlaster();
    
    if(getWorld()->collidedWithPlayer(this)){
        getWorld()->playSound(SOUND_GOODIE);
        getWorld()->increaseScore(100);
        
        switch(m_type){
            //Repair goodie add 10 HP if player's HP is <= 40, preventing it from exceeding 50. If >40, add remaining.
            case IID_REPAIR_GOODIE:
                if(m_player->getHP() <= 40)
                    m_player->increaseHP(10);
                else{
                    int addHP = m_player->getHP() % 10;
                    m_player->increaseHP(addHP);
                }
                break;
            //Life goodie increases lives
            case IID_LIFE_GOODIE:
                getWorld()->incLives();
                break;
            //Life goodie increase Torpedoes by 5
            case IID_TORPEDO_GOODIE:
                m_player->gainTorpedoes();
                break;
        }
    }
}

//Return true if either it went off the map or collides with player.
bool Collectible::hitPlayerOrFlyOff(){
    if(getX()< 0 || getY() <0){
        return true;
    }
    if(getWorld()->collidedWithPlayer(this)){
        //Helper function to decide goodie algortihm
        collectibleGained();
        return true;
    }
    return false;
}



//Collectible pretty much move in similar way.
void Collectible::action(){
 
    if(hitPlayerOrFlyOff())
    {
        destroyed();
        return;
    }
    
    moveTo(getX() - 0.75 , getY() - 0.75);
    
    if(hitPlayerOrFlyOff())
    {
        destroyed();
        return;
    }
    
}




/* ==================Repair Goodie Method==================*/

RepairGoodie::RepairGoodie(StudentWorld* world,int startX, int startY)
:Collectible(IID_REPAIR_GOODIE,IID_REPAIR_GOODIE, startX, startY, world)
{
}
    
void RepairGoodie::doSomething(){
    if(stillAlive())
        action();
}

/* ==================ExtraLife Method==================*/

ExtraLife::ExtraLife(StudentWorld* world,int startX, int startY)
:Collectible(IID_LIFE_GOODIE,IID_LIFE_GOODIE, startX, startY, world)
{
}

void ExtraLife::doSomething(){
    if(stillAlive())
        action();

}

/* ===================TorpedoGoodie Declaration=============*/

TorpedoGoodie::TorpedoGoodie(StudentWorld* world,int startX, int startY)
:Collectible(IID_TORPEDO_GOODIE,IID_TORPEDO_GOODIE,startX, startY, world)
{
}

void TorpedoGoodie::doSomething(){
    if(stillAlive())
        action();
}











// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp
