#include <Blob.h>

/* 
Blobs are points that orbit a sphere at given radius. 
As they orbit, they leave a trail of color behind them.
Each blob has a color, and a radius.
They also have velocity in the A and C angles.
Every loop, the angles are updated by the velocity by calling the tick() function
*/
Blob::Blob(){
  this->reset();
}

void Blob::reset(){
  this->lifespan = random(30000+4000);
  this->av = 0;
  this->cv = 0;
  this->a = random(UINT16_MAX) - UINT16_MAX/2;  // rotation angle of blob, using FastLED fast trif functions
  this->c = random(UINT16_MAX) - UINT16_MAX/2;  // rotation angle of blob, using FastLED fast trif functions
  this->applyForce(random(50,300), random(50, 300));
  this->color = CHSV((millis()/500)%255, 240, 150+random(50));
  this->radius = random(80,130);
  this->age = 0;
  this->lifespan = random(max_age)+500;
}

const float FTRIG = 65535.0 / (TWO_PI);
int Blob::x(){ return sphere_r * (sin16(c+32768) / 32768.0) * (cos16(a+32768) / 32768.0); }
int Blob::y(){ return sphere_r * (sin16(c+32768) / 32768.0) * (sin16(a+32768) / 32768.0); }
int Blob::z(){ return sphere_r * (cos16(c+32768) / 32768.0); }

void Blob::applyForce(float af, float cf){
  this->av += af;
  this->av = constrain(this->av, max_accel*-1, max_accel);
  this->cv += cf;
  this->cv = constrain(this->cv, max_accel*-1, max_accel);
}

void Blob::tick(){
  // animate angles with velocity
  this->age++;
  this->a = this->a + av;
  this->c = this->c + cv;
  if (random(300)==1){
    float af = random(30,80) * (random(2)==1 ? 1 : -1);
    float cf = random(30,60) * (random(2)==1 ? 1 : -1);
    this->applyForce(af, cf);
  }
  if (this->lifespan - this->age < max_age/10){
    this->radius *= 0.97;
  }
  if (this->age > this->lifespan){
    this->reset();
  }
}