import peasy.*;
import processing.dxf.*;
PeasyCam cam;
PMatrix3D world;

/*
 
 DodecaRGB is an interactive light gadget assembled from 12 PCBs with addressable LEDs. Once put
 together, the model can be programmed to do animations or other things. The prototype was
 developed for CCC Camp 2023 in Germany, and some early kits were sold. 
 
 A dodecahedron is a 3d shape with 12 sides, each side is a pentagon made up of 5 equal edges.
 The top and bottom of the pentagon are parallel. 
 
 All of the LEDs are connected in a continuous strand, and each PCB has inputs and outputs on 
 each side (labeled A-E), which need to be connected together in a specific order. This sketch
 helped in figuring out how to best do that and work out the math involved.

 This Processing sketch renders the DodecaRGB model in 3D, with the layout of LEDs and 
 sides numbered. There is a menu displayed and a few options to change the view mode and write
 the data file (if not present).
 
 This sketch can output the points with calculated X,Y,Z coordiates for all 312 LEDs (26 per side), 
 both as a JSON file and a C++ header file containing an array. This is useful for
 programming the firmware and developing your own animations.

 Requires Processing v4.3 or later

 References:
 - Hackaday page: https://hackaday.io/project/192557-dodecargb
 - Homepage: (Jeremy Seitz): https://somebox.com/projects
 - Firmware: https://github.com/somebox/DodecaRGB-firmware
 - Maths: https://math.stackexchange.com/questions/1320661/efficient-construction-of-a-dodecahedron
 
 */

int led_counter = 0;
boolean write_points = false;
boolean record_dfx = false;

// Appearance config
color side_color = #8888AA;
color led_color  = #EEFFDD;
color label_color = #333333;
color brighter_color = #AAAADD;

// interactive settings
boolean xray=false;   // see-through
boolean show_axes=false; // display the X,Y,Z axes lines
int view_mode=0; // 0..5, different views of the model for documentation
int last_second = 0; // tracks the time
String view_modes[] = {"normal", "bottom", "top", "build seq","3d points"};
int build_step = 0; 

float xv = 1.1071;   // angle between faces: PI - 2*atan((1+sqrt(5))/2), or around 63.4 degrees
float zv = 2*PI/20;  // pentagon top/bottom halves must be rotated 18 degrees in either direction to fit 
float radius = 200;    
int NUM_LEDS = 102*12;
// The rotations of each pentagon face, 0-4 (72 degree increments)
// This is important for aligning the PCBs correctly.
int side_rotation[] = {0, 3, 4, 4, 4, 4, 2, 2, 2, 2, 2, 0};

// structure used to track info about each LED for coordinates and outputting data.
class LedPoint {
  public int index, side, label_num = 0;
  public float x, y, z;

  // constructor
  public LedPoint(float x, float y, float z, int side, int label_num) {
    this.x = x;
    this.y = y;
    this.z = z;
    this.side = side;
    this.label_num = label_num;
    this.index = led_counter++;
  }
}
ArrayList<LedPoint> leds = new ArrayList();


// Function to draw a pentagon, labels and LEDs in the current coordinates.
void drawPentagon(int sideNumber) {
  pushMatrix();
  
  // rotate current side into position
  float ro = TWO_PI/5;
  if (sideNumber == 0) {  // top
    rotateZ(-zv-ro*2);
  } else if (sideNumber > 0 && sideNumber < 6) {  // top half
    rotateZ(ro*(sideNumber)+zv-ro);
    rotateX(xv);
  } else if (sideNumber >= 6 && sideNumber < 11) { // bottom half
    rotateZ(ro*(sideNumber)-zv+ro*3);
    rotateX(PI - xv);
  } else if (sideNumber == 11) { // bottom
    rotateX(PI);
    rotateZ(zv);
  }

  // Center the pentagon face in the canvas, used for drawing
  translate(0, 0, radius*1.31); // roughly atan(2+sqrt(5)), trial-and-error 

  // draw the pentagon
  beginShape();
  fill(side_color,xray ? 50 : 255);
  stroke(0);
  float z = 0; // Distance between the center and side plane
  float angle = TWO_PI / 5;
  if (sideNumber >= 6 && sideNumber < 11) {
    rotateZ(zv);
  } else {
    rotateZ(-zv);
  }
  for (int i = 0; i < 5; i++) {
    // Calculate the x, y, z coordinates of each point on the pentagon
    float x = radius * cos(angle * i);
    float y = radius * sin(angle * i);
    vertex(x, y, z);
  }
  endShape(CLOSE);

  // adust for side rotations due to model assembly
  rotateZ(ro*side_rotation[sideNumber]);  // config of face rotations

  buildLedsFromComponentPlacementCSV(sideNumber);

  popMatrix();
}


// draws a single LED at the current coordinates and labels it,
// while recording the position and info for later.
void drawLED(int led_num, int sideNumber) {
  pushMatrix();
  // draw a box for the LED
  fill(led_color, xray ? 70 : 255);
  if (xray == true){
    stroke(led_color);
  } else {
    noStroke();
  }
  translate(0, 0, 6);
  box(10, 10, 5);
  // label the led number
  textSize(10);
  textAlign(CENTER, CENTER);
  fill(xray ? 255 : label_color, xray ? 150 : 255);
  text(led_num, 0, -25, 2);  // Specify a z-axis value

  // during first pass, record the calculated X,Y,Z coordinates of the model 
  // for saving later as JSON and C header files.
  if (write_points) {
    float x = modelX(0, 0, 0);
    float y = modelY(0, 0, 0);
    float z = modelZ(0, 0, 0);
    leds.add(new LedPoint(x, y, z, sideNumber, led_num));
  }

  popMatrix();
}


int data_written = 0;
void show_HUD(){
  cam.beginHUD();
  textMode(MODEL);
  textSize(20);
  fill(255);
  textAlign(LEFT, TOP);
 
  text("[v] view mode: "+view_modes[view_mode], 40, 30);
  text("[a] show axes: "+show_axes, 40, 50);
  text("[x] x-ray mode: "+xray, 40, 70);
  text("[w] write data: "+(data_written > 0 ? "DONE!" : ""), 40, 90);
  text("fps "+round(frameRate), 40, 110);
  cam.endHUD();
}

void keyPressed() {
  switch (key) {
  case 'v':  // cycle between view modes
    view_mode = (view_mode+1) % 5;
    break;
  case 'x':
    xray = !xray;
    break;
  case 'a':
    show_axes = !show_axes;
    break;
  case 'w':  // write data points and reset things
    write_points = true;
    record_dfx = true;
    leds.clear();
    led_counter = 0;
    view_mode = 0;
    data_written = 5; // delay is seconds for confirmation message in HUD
    break;
  }
}

void exportJSON() {
  JSONArray doc = new JSONArray();
  int i = 0;
  for (LedPoint led : leds) {
    JSONObject pos = new JSONObject();
    pos.setInt("index", led.index);
    pos.setFloat("x", round(led.x*100)/100.0);
    pos.setFloat("y", round(led.y*100)/100.0);
    pos.setFloat("z", round(led.z*100)/100.0);
    pos.setInt("led_num", led.label_num);
    pos.setInt("side", led.side);
    doc.setJSONObject(i, pos);
    i++;
  }
  saveJSONArray(doc, "data/points.json");
}

void exportCArray() {
  // open a text file in data dir
  PrintWriter output = createWriter("data/points.h");
  output.println("/*\r\nThis text and code below was generated from a Processing sketch\n"+
    "which outputs all of the points in the DodecaLED model. See README.md for more info.");
  output.println("Generated on "+day()+"."+month()+"."+year()+" - "+hour()+":"+minute());
  output.println("radius: "+radius+" num_leds:"+NUM_LEDS);
  output.println("--------------");
  output.println("format: index, x, y, z, led_label, side_number");
  output.println("*/");
  
  for (LedPoint led : leds) {
    // write each point to the file as a C array
    // XYZ points[0] = {0, 0, 0};
    output.println("LED_Point(" + led.index +
                     ", "+round(led.x*100)/100.0 +
                     ", "+round(led.y*100)/100.0 +
                     ", "+round(led.z*100)/100.0 +
                     ", "+led.label_num +
                     ", "+led.side +
                     "),");
  }
  output.flush(); // Write the remaining data
  output.close(); // Finish the file
}

void setup() {
  size(800, 800, P3D);
  world = getMatrix(world);
  cam = new PeasyCam(this, 1000);
  cam.setMinimumDistance(200);
  cam.setMaximumDistance(1000);
}

// 
// connection points:
// side 0 = (bottom) our 12,13,14 (B) to side 1 in 21,22,23 (E)
// side 1 out 24,25,26 (A) to side 2 IN 21,23,23 (E)
// side 2 out 12,13,14 (B) to side 3 IN 21,23,23 (E)
// side 3 out 12,13,14 (B) to side 4 IN 21,23,23 (E)
// side 4 out 12,13,14 (B) to side 5 IN 21,23,23 (E)
// side 5 out 15,16,17 (C) to side 6 IN 18,19,20 (D)
// --> JST jumps between hemispheres
// side 6 out 24,25,26 (A) to side 7 IN 15,16,17 (C)
// side 7 out 24,25,26 (A)to side 8 IN 15,16,17 (C)
// side 8 out 24,25,26 (A)to side 9 IN 15,16,17 (C)
// side 9 out 24,25,26 (A)to side 10 IN 15,16,17 (C)
// side 10 out 12,13,14 (B) to side 11 (top) IN 21,22,23 (E)

String stripit(String s){
  return s.replace('"',' ').replace('m',' ').trim();
}

Table table;
//  for each row, there's a pushMatrix(), then we load the x,y coordinates of the LED, 
// then we apply an X,Y offset, then we rotateZ(), translate() with scale, and capture 
// the world coordinates, followed by a popMatrix(). 
// This ensures each operation happens in the right coordinate space.
void buildLedsFromComponentPlacementCSV(int sideNumber){
  println("load csv");
  table = loadTable("../../data/PickAndPlace_PCB_DodecaRGB_v2_2024-11-22.csv", "header,tsv");
  println(table.getRowCount() + " total rows in table");
  float scale = 5.15;
  int led_count = 0;
  for (TableRow row : table.rows()) {

    String label = stripit(row.getString("Designator"));
    if (label.startsWith("LED")){
      float x = Float.parseFloat(stripit(row.getString("Mid X")));
      float y = Float.parseFloat(stripit(row.getString("Mid Y")));
      int led_num = Integer.parseInt(label.substring(3)); // find number in "LEDxxx"
      //println(led_num + " (" + x + "," + y + ")");
      pushMatrix();
      x += 0.2;
      y -= 55.884;
      rotateZ(PI/10);
      translate(x*scale, y*scale, 0);
      drawLED(led_num, sideNumber); // LEDs 12-26
      led_count++;
      popMatrix();
    }
    
    //println(row.getColumnTitle(0));  
    //println(row.getColumnTitle(1));  
    //println(row.getColumnTitle(2));  
  }
  println(led_count + " LEDs counted");
}

void draw() {
  if (record_dfx) {
    beginRaw(DXF, "data/dodecaRGB.dxf");
  }
  background(0);
  lights();
  directionalLight(100, 100, 100, 400, 400, 0);
  ambientLight(50,50,50);

  if (show_axes==true) draw_axes();
  if (view_mode==4){
    draw_points();   
  } else {
    draw_model();
  }
  if (!record_dfx){
    show_HUD();
  }
  if (millis()/1000 > last_second){  // simple timer to handle UI updates every second
    last_second = millis()/1000;
    build_step = (build_step+1) % 12;   // build animation view
    data_written = max(0, data_written - 1);  // when writing files
  }
  if (write_points) {
    exportJSON();
    exportCArray();
    write_points = false;
  }
  if (record_dfx) {
    endRaw();
    record_dfx = false;
  }  
}



void draw_model() {
  pushMatrix();
  // Draw the dodecahedron
  for (int i = 0; i < 12; i++) {
    if (view_mode==1 && i>5) continue;
    if (view_mode==2 && i<6) continue;
    if (view_mode==3 && build_step < i) continue;
    drawPentagon(i);
  }
  popMatrix();
}


// verify the written data displays correctly, and run a simulated LED animation on the Z axis
float z_pos = 0;
int target = 60; // used for animation, fading points when they are near the counter
JSONArray doc;
void draw_points() {
  if (doc == null){
    doc = loadJSONArray("data/points.json");
  }
  if (doc == null) {
    textAlign(CENTER, CENTER);
    textSize(100);
    text("no data file.\nPress 'w' to write.", 0, 0);
  } else {
    // display data
    for (int i=0; i<doc.size(); i++) {
      JSONObject p = doc.getJSONObject(i);
      float x = p.getFloat("x");
      float y = p.getFloat("y");
      float z = p.getFloat("z");
      color c = color(80, 80, 80);
      float d = (z_pos-z);
      if (abs(d) < target) {
        // fade pixels in and out based on z distance
        float val = map(target - abs(d), 0, target, 80, 255);  
        c = color(val, val, val/2);
      }
      z_pos = (z_pos+0.005);
      if (z_pos > 350) z_pos = -350;
  
      noStroke();
      if (xray == true){
        fill(c,80);
      } else {
        fill(c);
      }
      pushMatrix();
      translate(x, y, z);
      sphere(5);
      popMatrix();
    }
  }
}


void draw_axes() {
  pushMatrix();
  stroke(200,150);
  textSize(60);
  fill(255);
  int label_loc = 500;

  line(-1000, 0, 0, 1000, 0, 0); // x axis
  text("X", label_loc, 0, 0);  // Specify a z-axis value

  line(0, -1000, 0, 0, 1000, 0); // y axis
  text("Y", 0, label_loc, 0);  // Specify a z-axis value

  line(0, 0, -1000, 0, 0, 1000); // z axis
  text("Z", 0, 0, label_loc);  // Specify a z-axis value

  popMatrix();
}
