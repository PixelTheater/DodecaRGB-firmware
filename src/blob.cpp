#include <Blob.h>

/* 
Blobs are points that orbit a sphere at given radius. 
As they orbit, they leave a trail of color behind them.
Each blob has a color, and a radius.
They also have velocity in the A and C angles.
Every loop, the angles are updated by the velocity by calling the tick() function
*/
Blob::Blob(uint16_t unique_id){
  this->blob_id = unique_id;
  this->reset();
}

void Blob::reset(){
  this->age = 0;
  this->lifespan = random(this->max_age/2)+this->max_age/2;
  this->radius = random(90,130);
  this->av = random(-this->max_accel * 1000, this->max_accel * 1000) / 1000.0;
  this->cv = random(-this->max_accel * 1000, this->max_accel * 1000) / 1000.0;
  this->a = random(TWO_PI*1000)/1000.0 - PI;  // rotation angle of blob
  this->c = random(TWO_PI*10000)/10000.0 - PI;  // rotation angle of blob
  float force_av = random(80, 100)/1000.0 * (random(2) == 0 ? -1.0 : 1.0);
  float force_cv = random(80, 100)/1000.0 * (random(2) == 0 ? -1.0 : 1.0);
  this->applyForce(force_av, force_cv);
}

int Blob::x(){ return sphere_r * sin(this->c) * cos(this->a); }
int Blob::y(){ return sphere_r * sin(this->c) * sin(this->a); }
int Blob::z(){ return sphere_r * cos(this->c); }

void Blob::applyForce(float af, float cf){
  this->av += af;
  if (this->av > this->max_accel) this->av = this->max_accel;
  if (this->av < -this->max_accel) this->av = -this->max_accel;
  this->cv += cf;
  if (this->cv > this->max_accel) this->cv = this->max_accel;
  if (this->cv < -this->max_accel) this->cv = -this->max_accel;
}

void Blob::applyForce(float fx, float fy, float fz){
  // convert x,y,z to angles
  float af = atan2(fy, fx);
  float cf = atan2(sqrt(fx*fx + fy*fy), fz);
  this->applyForce(af, cf);
}

void Blob::tick(){
  // as a blob orbits, it will tend towards the equator of the sphere. here we apply a small force
  // to the angles to keep them from going too far north or south
  float force_av = this->av * 1.001;
  this->c = fmod(this->c + PI, TWO_PI) - PI; // Normalize c to be within [-PI, PI]
  float force_cv = 0.0005 * (this->c - PI/2);
  if (this->c < -PI/2){
    force_cv = -0.0005 * (this->c + PI/2);
  }
  this->applyForce(force_av, force_cv);

  // animate angles with velocity
  this->age++;
  this->a += this->av; 
  this->c += this->cv;
  //this->av *= 0.9995; 
  if (abs(this->cv) < 0.005){
    float af = random(-max_accel*1000,max_accel*1000);
    float cf = random(-max_accel*1000,max_accel*1000);
    this->applyForce(af/2000.0, cf/1000.0);
  }
  if (this->lifespan - this->age < max_age/20){
    this->radius *= 0.99;
  }
  if (this->age > this->lifespan){
    this->reset();
  }
}